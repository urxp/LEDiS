#include<stdio.h>
#include<stdlib.h>
#include<string.h>
#include"bin2inc.h"

extern DisasmInterface IDisasm;
extern object_info * ObjectMap;


FILE *  vAddressStack;
dword   vAddressCount = 0;  
dword   disasm_ptr = 0;
VirtualAddress current_va;

char * jcc[16] = {
    /* 0 */  "jo", /* 1 */ "jno", /* 2 */  "jb", /* 3 */ "jae",
    /* 4 */  "je", /* 5 */ "jne", /* 6 */ "jbe", /* 7 */  "ja",
    /* 8 */  "js", /* 9 */ "jns", /* a */  "jp", /* b */ "jnp",
    /* c */  "jl", /* d */ "jge", /* e */ "jle", /* f */  "jg"
};


char * getJCC(byte j){

    return (j<16) ? jcc[j] : (void *)0;
}

pointer locateCode(){

    pointer code;

    code = le_getBuffer() + ObjectMap[IDisasm.vAddress.obj_n - 1].offset;
    code = code + IDisasm.vAddress.offset;

    return code;
}

char * o_size_str(){
    if(operand_size == 32) return "dword ";
    if(operand_size == 16) return "word ";
    if(operand_size == 8) return "byte ";

    return (void *)0;
}

dword getBitsMode(){

    return ObjectMap[current_va.obj_n - 1].bitsMode;
}

pointer locateCurrent(){

    pointer code;

    code = le_getBuffer() + ObjectMap[current_va.obj_n - 1].offset;
    code = code + current_va.offset;

    return code;
}

byte read_byte(){

    pointer code = locateCurrent();
    current_va.offset += 1;

    return *(byte *)code;
}

word read_word(){

    pointer code = locateCurrent();
    current_va.offset += 2;

    return *(word *)code;
}

dword read_dword(){

    pointer code = locateCurrent();
    current_va.offset += 4;

    return *(dword *)code;
}

dword getAlignment(dword offset){

    dword shift = 0;
    dword t_offset = offset;

    while(!(t_offset & 1)){
        t_offset >>= 1;
        shift++;
    }

    return 1 << shift;
}

void mark_instruction(void){

    byte ins_size;
    IRbyte * info_byte;

    info_byte = ObjectMap[IDisasm.vAddress.obj_n - 1].IR;
    info_byte += IDisasm.vAddress.offset;

    ins_size = current_va.offset - IDisasm.vAddress.offset;

    info_byte->flags |= 0x4 + (ins_size << 4);
    info_byte->data = disasm_ptr;

    ObjectMap[current_va.obj_n - 1].DisBytes += ins_size;
}

void comment_instruction(void){

    //return;

    IRbyte * info_byte;

    info_byte = ObjectMap[IDisasm.vAddress.obj_n - 1].IR;
    info_byte += IDisasm.vAddress.offset;
    
    info_byte->flags |= 0x8;
}

void init(void){

    IDisasm.fd = fopen("$__disasm.tmp", "w+b");
    vAddressStack = fopen("$__adstack.tmp", "w+b");
}

void close(void){

    fclose(IDisasm.fd);
    fclose(vAddressStack);
}

void loadInstruction(dword offset){

    byte size;

    fseek(IDisasm.fd, offset, SEEK_SET);

    fread(&size, 1, 1, IDisasm.fd);

    fread(IDisasm.InstructionString, size, 1, IDisasm.fd);

    IDisasm.InstructionString[size] = 0;
}

void pushAddress(VirtualAddress * va){

    //if((ObjectMap[va->obj_n - 1].size) <= va->offset) return;

    fseek(vAddressStack, vAddressCount * sizeof(VirtualAddress), SEEK_SET);
    fwrite(va, sizeof(VirtualAddress), 1, vAddressStack);
    vAddressCount++;
}

boolean popAddress(){

    if(vAddressCount == 0) return boolean(0);

    vAddressCount--;

    fseek(vAddressStack, vAddressCount * sizeof(VirtualAddress), SEEK_SET);

    fread(&IDisasm.vAddress, sizeof(VirtualAddress), 1, vAddressStack);

    return boolean(1);
}


char * getVALabel(VirtualAddress * va){

    return getLabel(va->obj_n, va->offset);
}

void dump15(){

    pointer code = locateCode();
    dword i;

    dword i_max = ObjectMap[IDisasm.vAddress.obj_n - 1].size - IDisasm.vAddress.offset;

    printf("[DISASM] obj_n: %d, offset: 0x%08x (%s)\n",
        IDisasm.vAddress.obj_n, IDisasm.vAddress.offset, getVALabel(&IDisasm.vAddress));
    printf("[DISASM] >> ");

    if(i_max > 15) i_max = 15;

    for(i = 0; i < i_max; i++){
        printf("%02x ", *(byte *)(code + i));
    }
    printf("\n");
}

boolean checkAddress(void){

    // RALLY.LE specific
    // [TODO] not returning function call followed by data
    if((IDisasm.vAddress.obj_n == 1)&&(IDisasm.vAddress.offset == 0x5041A))
        return boolean(1);
    //if((IDisasm.vAddress.obj_n == 1)&&(IDisasm.vAddress.offset == 0x591DE))
    //    return boolean(1);
    if((IDisasm.vAddress.obj_n == 1)&&(IDisasm.vAddress.offset == 0x6E486))
        return boolean(1);


    return boolean(ObjectMap[IDisasm.vAddress.obj_n - 1].IR[IDisasm.vAddress.offset].flags & 0x4);
}

void checkInfixLabels(void){

    dword obj_num, off, off0, obj_size;
    IRbyte * current_i;

    for(obj_num = 1; obj_num < le_getNumberOfObjects(); obj_num++){

        if(ObjectMap[obj_num-1].isCode){

            current_i = (void *)0;
            obj_size = ObjectMap[obj_num-1].size;
    
            for(off = 0; off < obj_size; off++){

                if(current_i){
                    if(off0 == ((current_i->flags&0xf0)>>4)){
                        current_i = (void *)0;
                    }
                }

                if(ObjectMap[obj_num-1].IR[off].flags & 0x1){

                    if(current_i && off0){
                        current_i->flags |= 0x8;
                        if(ObjectMap[obj_num-1].IR[off].flags & 0x4){
                            ObjectMap[obj_num-1].IR[off].flags |= 0x8;
                        }
                    }
                }

                if((ObjectMap[obj_num-1].IR[off].flags & 0x4)){
                    off0 = 0;
                    current_i = ObjectMap[obj_num-1].IR + off;
                }

                if(current_i) off0++;
            }
        }
    }
}

void markAsData(VirtualAddress * va_start, VirtualAddress * va_end){

    dword diff_bytes;

    if(va_start->obj_n == va_end->obj_n){
        if(va_start->offset <= va_end->offset){
            diff_bytes = va_end->offset - va_start->offset + 1;
            ObjectMap[va_start->obj_n - 1].DataBytes += diff_bytes;
        }
    }
}

void pushJumpTable(VirtualAddress * va){

l_strt:

    fx = le_checkFixup(va->obj_n, va->offset);

    if(fx->size == 4){

        if((ObjectMap[va->obj_n - 1].IR[va->offset].flags & 0x8) == 0){

            IDisasm.pushAddress(&(VirtualAddress){ 
                .obj_n = fx->object_n,
                .offset = fx->target
            });

            ObjectMap[va->obj_n - 1].IR[va->offset].flags |= 0x8;
            ObjectMap[va->obj_n - 1].DataBytes += 4;
        }

        va->offset += 4;
        goto l_strt;
    }
    else return;
}


DisasmInterface IDisasm = {

    .init               = &init,
    .close              = &close,
    .loadInstruction    = &loadInstruction,
    .pushAddress        = &pushAddress,
    .popAddress         = &popAddress,
    .disassemble        = &disassemble
};