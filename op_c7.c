#include"bin2inc.h"

boolean op_c7(void){

    char * mod_rm_str;
    char * o_size;
    char * mnems[8] = { "mov", 0, 0, 0, 0, 0, 0, 0 };
    char * mnemonic;
    char * imm_str;
    
    address_mode = read_byte();
    mod_rm_str = mod_rm();


    mnemonic = mnems[(address_mode >> 3) & 7];

    if(!mnemonic) return boolean(0);

    if(operand_size == 32){
        o_size = "dword "; 
        fx = le_checkFixup(current_va.obj_n, current_va.offset);
        if((fx->type == 7)&&(fx->size == 4)){
            read_dword();
            imm_str = getLabel(fx->object_n, fx->target);
        }
        else{
            imm_str = hexbyte(read_dword());
        }
    }
    else if(operand_size == 16){
        o_size = "word "; 
        fx = le_checkFixup(current_va.obj_n, current_va.offset);
        if(fx->size != 0){
            printf("[DISASM] ::: %s %s %d\n", __FILE__, __FUNCTION__, __LINE__);
            printf("[DISASM] [TODO] 16-bit offset fixup\n");
            return boolean(0);
        }
        else{
            imm_str = hexbyte(read_word());
        }
    }
    else return boolean(0);
         
    

    if(((address_mode >> 6) & 3) == 3) o_size = "";

    sprintf(ibuffer, iFORMAT"%s%s, %s",
        mnemonic, o_size, mod_rm_str, imm_str);

    mark_instruction();// comment_instruction();
    // push next instruction offset to stack
    IDisasm.pushAddress(&current_va);

    return boolean(1);
}