#include"bin2inc.h"

extern dword            disasm_ptr;

boolean prefix_66;
char ibuffer[256];
fixup_struct * fx;
char * seg_ds, * seg_ss, * seg_seg, * rep_prefix;
dword imm32, id;
byte ib;
char * tmp_str;
signed_byte displacement_8;
signed_word displacement_16;
signed_dword displacement_32;
byte operand_size;
byte address_size;
byte address_mode;

char * tmp0;
char * tmp1;
char * tmp2;

void disassemble(void){
#define FORMAT "%-8s"

    char * buffer = ibuffer;
    byte str_size;
    signed_dword rel32;
    signed_byte imm8, rel8;
    
    word imm16, iw;
    
    byte op;

    while(IDisasm.popAddress()){

        if(checkAddress()) continue;

        seg_seg = "";
        seg_ds = "";
        seg_ss = "";
        rep_prefix = "";
        current_va = IDisasm.vAddress;
        operand_size = getBitsMode();
        address_size = operand_size;
        prefix_66 = boolean(0);
        fx = (void *)0;

switch_start:
        op = read_byte();
        switch(op){
        case 0x00:
            if(!op_00()) goto l_def;
            break;
        case 0x01:
            if(!op_01()) goto l_def;
            break;
        case 0x02:
            if(!op_02()) goto l_def;
            break;
        case 0x03:
            if(!op_03()) goto l_def;
            break;
        case 0x04:
            ib = read_byte();
            sprintf(buffer, FORMAT"%s, %s", "add", "al", hexbyte(ib));
            mark_instruction();
            // push next instruction offset to stack
            IDisasm.pushAddress(&current_va);
            break;
        case 0x05:
            fx = le_checkFixup(current_va.obj_n, current_va.offset);
            if(prefix_66){
                iw = read_word();
                sprintf(buffer, FORMAT"%s, %s", "add", "ax", hexbyte(iw));
            }
            else {
                id = read_dword();
                if(fx->size == 4){
                    tmp_str = getLabel(fx->object_n, fx->target);
                }
                else {
                    tmp_str = hexbyte(id);
                }
                sprintf(buffer, FORMAT"%s, %s", "add", "eax", tmp_str);
            }
            mark_instruction();
            // push next instruction offset to stack
            IDisasm.pushAddress(&current_va);
            break;
        case 0x06:
            sprintf(buffer, FORMAT"%s", "push", "es");
            mark_instruction();
            // push next instruction offset to stack
            IDisasm.pushAddress(&current_va);
            break;
        case 0x07:
            sprintf(buffer, FORMAT"%s", "pop", "es");
            mark_instruction();
            // push next instruction offset to stack
            IDisasm.pushAddress(&current_va);
            break;
        case 0x08:
            if(!op_08()) goto l_def;
            break;
        case 0x09:
            if(!op_09()) goto l_def;
            break;
        case 0x0a:
            if(!op_0a()) goto l_def;
            break;
        case 0x0b:
            if(!op_0b()) goto l_def;
            break;
        case 0x0c:
            ib = read_byte();
            sprintf(buffer, FORMAT"%s, %s", "or", "al", hexbyte(ib));
            mark_instruction();
            // push next instruction offset to stack
            IDisasm.pushAddress(&current_va);
            break;
        case 0x0d:
            if(prefix_66){
                iw = read_word();
                sprintf(buffer, FORMAT"%s, %s", "or", "ax", hexbyte(iw));
            }
            else {
                id = read_dword();
                sprintf(buffer, FORMAT"%s, %s", "or", "eax", hexbyte(id));
            }
            mark_instruction();
            // push next instruction offset to stack
            IDisasm.pushAddress(&current_va);
            break;
        case 0x0e:
            sprintf(buffer, FORMAT"%s", "push", "cs");
            mark_instruction();
            // push next instruction offset to stack
            IDisasm.pushAddress(&current_va);
            break;
        case 0x0f:
            op = read_byte();
            switch(op){
            case 0x00:
                address_mode = read_byte();
                if(((address_mode>>3)&7) == 5){
                    operand_size = 16;
                    sprintf(buffer, FORMAT"%s", "verw", mod_rm());
                    mark_instruction(); //comment_instruction();
                }
                else goto l_def;
                
                IDisasm.pushAddress(&current_va);
                break;
            case 0x01:
                address_mode = read_byte();
                if(address_mode == 0x25){
                    sprintf(buffer, FORMAT"%s", "smsw", mod_rm());
                    mark_instruction(); comment_instruction();
                }
                else if(address_mode == 0x0c){
                    sprintf(buffer, FORMAT"%s", "sidt", mod_rm());
                    mark_instruction();
                }
                else goto l_def;
                
                IDisasm.pushAddress(&current_va);
                break;
            case 0x03:
                address_mode = read_byte();
                tmp0 = mod_reg();
                tmp1 = mod_rm();

                sprintf(buffer, FORMAT"%s, %s", "lsl", 
                        tmp0, tmp1);
                mark_instruction();
                IDisasm.pushAddress(&current_va);
                break;
            case 0x20:
                address_mode = read_byte();
                if(address_mode == 0xc0){
                    sprintf(buffer, FORMAT"%s, %s", "mov", "eax", "cr0");
                    mark_instruction();
                    IDisasm.pushAddress(&current_va);
                    break;
                }
                if(address_mode == 0xc1){
                    sprintf(buffer, FORMAT"%s, %s", "mov", "ecx", "cr0");
                    mark_instruction();
                    IDisasm.pushAddress(&current_va);
                    break;
                }
                goto l_def; 
                break;
            case 0x22:
                address_mode = read_byte();
                if(address_mode == 0xc0){
                    sprintf(buffer, FORMAT"%s, %s", "mov", "cr0", "eax");
                    mark_instruction();
                    IDisasm.pushAddress(&current_va);
                    break;
                }
                if(address_mode == 0xc1){
                    sprintf(buffer, FORMAT"%s, %s", "mov", "cr0", "ecx");
                    mark_instruction();
                    IDisasm.pushAddress(&current_va);
                    break;
                }
                goto l_def; 
                break;
            case 0x80:
            case 0x81:
            case 0x82:
            case 0x83:
            case 0x84:
            case 0x85:
            case 0x86:
            case 0x87:
            case 0x88:
            case 0x89:
            case 0x8a:
            case 0x8b:
            case 0x8c:
            case 0x8d:
            case 0x8e:
            case 0x8f:
                if(operand_size != 32) goto l_def;
                rel32 = read_dword();
                mark_instruction();
                IDisasm.pushAddress(&current_va);
                current_va.offset += rel32;
                le_createLabel(current_va.obj_n, current_va.offset);
                sprintf(buffer, FORMAT"near %s", getJCC(op&15),
                    getLabel(current_va.obj_n, current_va.offset));
                IDisasm.pushAddress(&current_va);
                break;
            case 0x94:
                if(prefix_66) goto l_def;
                switch(read_byte()){
                case 0xc0:
                    sprintf(buffer, FORMAT"%s", "setz", "al");
                    mark_instruction();
                    IDisasm.pushAddress(&current_va);
                    break;
                case 0xc2:
                    sprintf(buffer, FORMAT"%s", "setz", "dl");
                    mark_instruction();
                    IDisasm.pushAddress(&current_va);
                    break;
                case 0xc3:
                    sprintf(buffer, FORMAT"%s", "setz", "bl");
                    mark_instruction();
                    IDisasm.pushAddress(&current_va);
                    break;
                default:
                    goto l_def;
                }
                break;
            case 0x95:
                if(prefix_66) goto l_def;
                switch(read_byte()){
                case 0x87:
                    address_mode = 0x87;
                    sprintf(buffer, FORMAT"byte %s", "setnz", mod_rm());
                    mark_instruction();
                    IDisasm.pushAddress(&current_va);
                    break;
                case 0xc0:
                    sprintf(buffer, FORMAT"%s", "setnz", "al");
                    mark_instruction();
                    IDisasm.pushAddress(&current_va);
                    break;
                case 0xc2:
                    sprintf(buffer, FORMAT"%s", "setnz", "dl");
                    mark_instruction();
                    IDisasm.pushAddress(&current_va);
                    break;
                case 0xc3:
                    sprintf(buffer, FORMAT"%s", "setnz", "bl");
                    mark_instruction();
                    IDisasm.pushAddress(&current_va);
                    break;
                case 0xc4:
                    sprintf(buffer, FORMAT"%s", "setnz", "ah");
                    mark_instruction();
                    IDisasm.pushAddress(&current_va);
                    break;
                default:
                    goto l_def;
                }
                break;
            case 0x97:
                if(prefix_66) goto l_def;
                switch(read_byte()){
                case 0xc0:
                    sprintf(buffer, FORMAT"%s", "seta", "al");
                    mark_instruction();
                    IDisasm.pushAddress(&current_va);
                    break;
                case 0xc2:
                    sprintf(buffer, FORMAT"%s", "seta", "dl");
                    mark_instruction();
                    IDisasm.pushAddress(&current_va);
                    break;
                default:
                    goto l_def;
                }
                break;
            case 0x9c:
                if(prefix_66) goto l_def;
                switch(read_byte()){
                case 0xc0:
                    sprintf(buffer, FORMAT"%s", "setl", "al");
                    mark_instruction();
                    IDisasm.pushAddress(&current_va);
                    break;
                default:
                    goto l_def;
                }
                break;
            case 0x9e:
                if(prefix_66) goto l_def;
                switch(read_byte()){
                case 0xc0:
                    sprintf(buffer, FORMAT"%s", "setle", "al");
                    mark_instruction();
                    IDisasm.pushAddress(&current_va);
                    break;
                default:
                    goto l_def;
                }
                break;
            case 0x9f:
                if(prefix_66) goto l_def;
                switch(read_byte()){
                case 0xc0:
                    sprintf(buffer, FORMAT"%s", "setg", "al");
                    mark_instruction();
                    IDisasm.pushAddress(&current_va);
                    break;
                default:
                    goto l_def;
                }
                break;
            case 0xa0:
                sprintf(buffer, FORMAT"%s", "push", "fs");
                mark_instruction();
                IDisasm.pushAddress(&current_va);
                break;
            case 0xa1:
                sprintf(buffer, FORMAT"%s", "pop", "fs");
                mark_instruction();
                IDisasm.pushAddress(&current_va);
                break;
            case 0xa4:
                if(prefix_66) goto l_def;
                switch(read_byte()){
                case 0xc2:
                    ib = read_byte();
                    sprintf(buffer, FORMAT"%s, %s, %s", "shld", 
                        "edx", "eax", hexbyte(ib));
                    mark_instruction();
                    IDisasm.pushAddress(&current_va);
                    break;
                case 0xcb:
                    ib = read_byte();
                    sprintf(buffer, FORMAT"%s, %s, %s", "shld", 
                        "ebx", "ecx", hexbyte(ib));
                    mark_instruction();
                    IDisasm.pushAddress(&current_va);
                    break;
                case 0xf0:
                    ib = read_byte();
                    sprintf(buffer, FORMAT"%s, %s, %s", "shld", 
                        "eax", "esi", hexbyte(ib));
                    mark_instruction();
                    IDisasm.pushAddress(&current_va);
                    break;
                case 0xf8:
                    ib = read_byte();
                    sprintf(buffer, FORMAT"%s, %s, %s", "shld", 
                        "eax", "edi", hexbyte(ib));
                    mark_instruction();
                    IDisasm.pushAddress(&current_va);
                    break;
                default:
                    goto l_def;
                }
                break;
            case 0xa5:
                if(prefix_66) goto l_def;
                switch(read_byte()){
                case 0xc2:
                    sprintf(buffer, FORMAT"%s, %s, %s", "shld", 
                        "edx", "eax", "cl");
                    mark_instruction();
                    IDisasm.pushAddress(&current_va);
                    break;
                case 0xc3:
                    sprintf(buffer, FORMAT"%s, %s, %s", "shld", 
                        "ebx", "eax", "cl");
                    mark_instruction();
                    IDisasm.pushAddress(&current_va);
                    break;
                default:
                    goto l_def;
                }
                break;
            case 0xa8:
                sprintf(buffer, FORMAT"%s", "push", "gs");
                mark_instruction();
                IDisasm.pushAddress(&current_va);
                break;
            case 0xa9:
                sprintf(buffer, FORMAT"%s", "pop", "gs");
                mark_instruction();
                IDisasm.pushAddress(&current_va);
                break;
            case 0xab:
                if(prefix_66) goto l_def;
                switch(read_byte()){
                case 0xfe:
                    sprintf(buffer, FORMAT"%s, %s", "bts", 
                        "esi", "edi");
                    mark_instruction();
                    IDisasm.pushAddress(&current_va);
                    break;
                default:
                    goto l_def;
                }
                break;
            case 0xac:
                if(prefix_66) goto l_def;
                switch(read_byte()){
                case 0xc8:
                    ib = read_byte();
                    sprintf(buffer, FORMAT"%s, %s, %s", "shrd", 
                        "eax", "ecx", hexbyte(ib));
                    mark_instruction();
                    IDisasm.pushAddress(&current_va);
                    break;
                case 0xca:
                    ib = read_byte();
                    sprintf(buffer, FORMAT"%s, %s, %s", "shrd", 
                        "edx", "ecx", hexbyte(ib));
                    mark_instruction();
                    IDisasm.pushAddress(&current_va);
                    break;
                case 0xd0:
                    ib = read_byte();
                    sprintf(buffer, FORMAT"%s, %s, %s", "shrd", 
                        "eax", "edx", hexbyte(ib));
                    mark_instruction();
                    IDisasm.pushAddress(&current_va);
                    break;
                default:
                    goto l_def;
                }
                break;
            case 0xad:
                if(prefix_66) goto l_def;
                switch(read_byte()){
                case 0xc3:
                    sprintf(buffer, FORMAT"%s, %s, %s", "shrd", 
                        "ebx", "eax", "cl");
                    mark_instruction();
                    IDisasm.pushAddress(&current_va);
                    break;
                case 0xc6:
                    sprintf(buffer, FORMAT"%s, %s, %s", "shrd", 
                        "esi", "eax", "cl");
                    mark_instruction();
                    IDisasm.pushAddress(&current_va);
                    break;
                case 0xd0:
                    sprintf(buffer, FORMAT"%s, %s, %s", "shrd", 
                        "eax", "edx", "cl");
                    mark_instruction();
                    IDisasm.pushAddress(&current_va);
                    break;
                case 0xda:
                    sprintf(buffer, FORMAT"%s, %s, %s", "shrd", 
                        "edx", "ebx", "cl");
                    mark_instruction();
                    IDisasm.pushAddress(&current_va);
                    break;
                default:
                    goto l_def;
                }
                break;
            case 0xaf:
                address_mode = read_byte();
                sprintf(buffer, FORMAT"%s, %s", "imul", 
                    mod_reg(), mod_rm());
                mark_instruction();
                IDisasm.pushAddress(&current_va);
                break;
            case 0xb3:
                if(prefix_66) goto l_def;
                switch(read_byte()){
                case 0xc6:
                    sprintf(buffer, FORMAT"%s, %s", "btr", 
                        "esi", "eax");
                    mark_instruction();
                    IDisasm.pushAddress(&current_va);
                    break;
                default:
                    goto l_def;
                }
                break;
            case 0xb4:
                address_mode = read_byte();
                tmp0 = mod_reg();
                tmp1 = mod_rm();

                sprintf(buffer, FORMAT"%s, %s", "lfs", 
                        tmp0, tmp1);
                mark_instruction();
                IDisasm.pushAddress(&current_va);
                break;
            case 0xb5:
                address_mode = read_byte();
                tmp0 = mod_reg();
                tmp1 = mod_rm();

                sprintf(buffer, FORMAT"%s, %s", "lgs", 
                        tmp0, tmp1);
                mark_instruction();
                IDisasm.pushAddress(&current_va);
                break;
            case 0xb6:
                address_mode = read_byte();
                tmp2 = ((address_mode >> 6) == 3) ? "" : "byte ";
                tmp0 = mod_reg();
                operand_size = 8;
                tmp1 = mod_rm();

                sprintf(buffer, FORMAT"%s, %s%s", "movzx", 
                        tmp0, tmp2, tmp1);
                mark_instruction();
                IDisasm.pushAddress(&current_va);
                break;
            case 0xb7:
                address_mode = read_byte();
                tmp2 = ((address_mode >> 6) == 3) ? "" : "word ";
                operand_size = 32;
                tmp0 = mod_reg();
                operand_size = 16;
                tmp1 = mod_rm();

                sprintf(buffer, FORMAT"%s, %s%s", "movzx", 
                        tmp0, tmp2, tmp1);
                mark_instruction();
                IDisasm.pushAddress(&current_va);
                break;
            case 0xbd:
                if(prefix_66) goto l_def;
                switch(read_byte()){
                case 0xc6:
                    sprintf(buffer, FORMAT"%s, %s", "bsr", 
                        "eax", "esi");
                    mark_instruction();
                    IDisasm.pushAddress(&current_va);
                    break;
                case 0xc8:
                    sprintf(buffer, FORMAT"%s, %s", "bsr", 
                        "ecx", "eax");
                    mark_instruction();
                    IDisasm.pushAddress(&current_va);
                    break;
                default:
                    goto l_def;
                }
                break;
            case 0xbe:
                address_mode = read_byte();
                tmp2 = ((address_mode >> 6) == 3) ? "" : "byte ";
                tmp0 = mod_reg();
                operand_size = 8;
                tmp1 = mod_rm();

                sprintf(buffer, FORMAT"%s, %s%s", "movsx", 
                        tmp0, tmp2, tmp1);
                mark_instruction();
                IDisasm.pushAddress(&current_va);
                break;
            case 0xbf:
                address_mode = read_byte();
                tmp2 = ((address_mode >> 6) == 3) ? "" : "word ";
                operand_size = 32;
                tmp0 = mod_reg();
                operand_size = 16;
                tmp1 = mod_rm();

                sprintf(buffer, FORMAT"%s, %s%s", "movsx", 
                        tmp0, tmp2, tmp1);
                mark_instruction();
                IDisasm.pushAddress(&current_va);
                break;
            default:
                goto l_def;
            }
            break;
        case 0x11:
            address_mode = read_byte();
            sprintf(buffer, FORMAT"%s, %s", "adc", mod_rm(), mod_reg());
            mark_instruction();
            // push next instruction offset to stack
            IDisasm.pushAddress(&current_va);
            break;
        case 0x13:
            address_mode = read_byte();
            sprintf(buffer, FORMAT"%s, %s", "adc", mod_reg(), mod_rm());
            mark_instruction();
            // push next instruction offset to stack
            IDisasm.pushAddress(&current_va);
            break;
        case 0x17:
            sprintf(buffer, FORMAT"%s", "pop", "ss");
            mark_instruction();
            // push next instruction offset to stack
            IDisasm.pushAddress(&current_va);
            break;
        case 0x19:
            address_mode = read_byte();
            sprintf(buffer, FORMAT"%s, %s", "sbb", mod_rm(), mod_reg());
            mark_instruction();
            // push next instruction offset to stack
            IDisasm.pushAddress(&current_va);
            break;
        case 0x1b:
            address_mode = read_byte();
            sprintf(buffer, FORMAT"%s, %s", "sbb", mod_reg(), mod_rm());
            mark_instruction();
            // push next instruction offset to stack
            IDisasm.pushAddress(&current_va);
            break;
        case 0x1e:
            sprintf(buffer, FORMAT"%s", "push", "ds");
            mark_instruction();
            // push next instruction offset to stack
            IDisasm.pushAddress(&current_va);
            break;
        case 0x1f:
            sprintf(buffer, FORMAT"%s", "pop", "ds");
            mark_instruction();
            // push next instruction offset to stack
            IDisasm.pushAddress(&current_va);
            break;
        case 0x20:
            if(!op_20()) goto l_def;            
            break;
        case 0x21:
            if(!op_21()) goto l_def;            
            break;
        case 0x22:
            if(!op_22()) goto l_def;            
            break;
        case 0x23:
            if(!op_23()) goto l_def;            
            break;
        case 0x24:
            ib = read_byte();
            sprintf(buffer, FORMAT"%s, %s", "and", "al", hexbyte(ib));
            mark_instruction();
            // push next instruction offset to stack
            IDisasm.pushAddress(&current_va);     
            break;
        case 0x25:
            if(prefix_66){
                iw = read_word();
                sprintf(buffer, FORMAT"%s, %s", "and", "ax", hexbyte(iw));
            }
            else {
                id = read_dword();
                sprintf(buffer, FORMAT"%s, %s", "and", "eax", hexbyte(id));
            }
            mark_instruction();
            // push next instruction offset to stack
            IDisasm.pushAddress(&current_va);
            break;
        case 0x26:
            seg_seg = "es:";
            seg_ds  = "es:";
            seg_ss  = "es:";
            goto switch_start;
        case 0x28:
            if(!op_28()) goto l_def;            
            break;
        case 0x29:
            if(!op_29()) goto l_def;            
            break;
        case 0x2a:
            if(!op_2a()) goto l_def;            
            break;
        case 0x2b:
            if(!op_2b()) goto l_def;            
            break;
        case 0x02c:
            ib = read_byte();
            sprintf(buffer, FORMAT"%s, %s", "sub", "al", hexbyte(ib));
            mark_instruction();
            // push next instruction offset to stack
            IDisasm.pushAddress(&current_va);
            break;
        case 0x2d:
            if(prefix_66){
                iw = read_word();
                sprintf(buffer, FORMAT"%s, %s", "sub", "ax", hexbyte(iw));
            }
            else {
                id = read_dword();
                sprintf(buffer, FORMAT"%s, %s", "sub", "eax", hexbyte(id));
            }
            mark_instruction();
            // push next instruction offset to stack
            IDisasm.pushAddress(&current_va);
            break;
        case 0x2e:
            seg_seg = "cs:";
            seg_ds = "cs:";
            seg_ss = "cs:";
            goto switch_start;
        case 0x30:
            if(!op_30()) goto l_def;            
            break;
        case 0x31:
            if(!op_31()) goto l_def;            
            break;
        case 0x32:
            if(!op_32()) goto l_def;            
            break;
        case 0x33:
            if(!op_33()) goto l_def;            
            break;
        case 0x34:
            ib = read_byte();
            sprintf(buffer, FORMAT"%s, %s",
                "xor", "al", hexbyte(ib));
            mark_instruction();
            // push next instruction offset to stack
            IDisasm.pushAddress(&current_va);
            break;
        case 0x35:
            if(prefix_66){
                iw = read_word();
                sprintf(buffer, FORMAT"%s, %s", "xor", "ax", hexbyte(iw));
            }
            else {
                id = read_dword();
                sprintf(buffer, FORMAT"%s, %s", "xor", "eax", hexbyte(id));
            }
            mark_instruction();
            // push next instruction offset to stack
            IDisasm.pushAddress(&current_va);
            break;
        case 0x36:
            seg_seg = "ss:";
            seg_ds = "ss:";
            seg_ss = "ss:";
            goto switch_start;
        case 0x38:
            if(!op_38()) goto l_def;            
            break;
        case 0x39:
            if(!op_39()) goto l_def;            
            break;
        case 0x3a:
            if(!op_3a()) goto l_def;            
            break;
        case 0x3b:
            if(!op_3b()) goto l_def;            
            break;
        case 0x3c:
            ib = read_byte();
            sprintf(buffer, FORMAT"%s, %s", "cmp", "al", hexbyte(ib));
            mark_instruction();
            IDisasm.pushAddress(&current_va);
            break;
        case 0x3d:
            if(prefix_66){
                iw = read_word();
                sprintf(buffer, FORMAT"%s, word %s", "cmp", "ax", hexbyte(iw));
            }
            else {
                fx = le_checkFixup(current_va.obj_n, current_va.offset);
                id = read_dword();
                if(fx->size == 4){
                    tmp_str = getLabel(fx->object_n, fx->target);
                    sprintf(buffer, FORMAT"%s, %s", "cmp", "eax", tmp_str);
                }
                else {
                    sprintf(buffer, FORMAT"%s, dword %s", "cmp", "eax", hexbyte(id));
                }
            }
            mark_instruction();
            // push next instruction offset to stack
            IDisasm.pushAddress(&current_va);
            break;
        case 0x3e:
            seg_seg = "ds:";
            seg_ds = "ds:";
            seg_ss = "ds:";
            goto switch_start;
        case 0x40:
        case 0x41:
        case 0x42:
        case 0x43:
        case 0x44:
        case 0x45:
        case 0x46:
        case 0x47:
            sprintf(buffer, FORMAT"%s", "inc", getRegister(op&7));
            mark_instruction();
            IDisasm.pushAddress(&current_va);
            break;
        case 0x48:
        case 0x49:
        case 0x4a:
        case 0x4b:
        case 0x4c:
        case 0x4d:
        case 0x4e:
        case 0x4f:
            sprintf(buffer, FORMAT"%s", "dec", getRegister(op&7));
            mark_instruction();
            IDisasm.pushAddress(&current_va);
            break;
        case 0x50:
        case 0x51:
        case 0x52:
        case 0x53:
        case 0x54:
        case 0x55:
        case 0x56:
        case 0x57:
            sprintf(buffer, FORMAT"%s", "push", getRegister(op&7));
            mark_instruction();
            IDisasm.pushAddress(&current_va);
            break;
        case 0x58:
        case 0x59:
        case 0x5a:
        case 0x5b:
        case 0x5c:
        case 0x5d:
        case 0x5e:
        case 0x5f:
            sprintf(buffer, FORMAT"%s", "pop", getRegister(op&7));
            mark_instruction();
            IDisasm.pushAddress(&current_va);
            break;
        case 0x60:
            if(operand_size == 32){
                sprintf(buffer, FORMAT, "pushad");
            }
            else if(operand_size == 16){
                sprintf(buffer, FORMAT, "pusha");
                if(address_size == 32) comment_instruction();
            }
            else goto l_def;
            mark_instruction();
            IDisasm.pushAddress(&current_va);
            break;
        case 0x61:
            if(operand_size == 32){
                sprintf(buffer, FORMAT, "popad");
            }
            else if(operand_size == 16){
                sprintf(buffer, FORMAT, "popa");
                if(address_size == 32) comment_instruction();
            }
            else goto l_def;
            mark_instruction();
            IDisasm.pushAddress(&current_va);
            break;
        case 0x65:
            seg_seg = "gs:";
            seg_ds  = "gs:";
            seg_ss  = "gs:";
            goto switch_start;
        case 0x66:
            if(!op_66()) goto l_def;
            goto switch_start;
        case 0x67:
            switch(read_byte()){
            case 0xe3:
                rel8 = read_byte();

                mark_instruction();
                IDisasm.pushAddress(&current_va);

                // get jmp instruction offset
                current_va.offset += rel8;
                // set label for jmp
                le_createLabel(current_va.obj_n, current_va.offset);
                sprintf(buffer, FORMAT"%s", "jcxz", getLabel(current_va.obj_n, current_va.offset));
                IDisasm.pushAddress(&current_va);
                break;
            default:
                goto l_def;
            }
            break;
        case 0x68:
            if(prefix_66) goto l_def;
            fx = le_checkFixup(current_va.obj_n, current_va.offset);
            id = read_dword();

            if(fx->size == 4){
                sprintf(buffer, FORMAT"%s",
                    "push", getLabel(fx->object_n, fx->target));
            }
            else {
                sprintf(buffer, FORMAT"%s", "push", hexbyte(id));
            }

            mark_instruction();
             // push next instruction offset to stack
            IDisasm.pushAddress(&current_va);
            break;
        case 0x69:
            if(!op_69()) goto l_def;
            break;
        case 0x6a:
            if(prefix_66) goto l_def;
            ib = read_byte();
            sprintf(buffer, FORMAT"byte %s", "push", hexbyte((signed_byte)ib));
            mark_instruction();
            // push next instruction offset to stack
            IDisasm.pushAddress(&current_va);
            break;
        case 0x6b:
            if(!op_6b()) goto l_def;
            break;
        case 0x6e:
            sprintf(buffer, FORMAT, "outsb");
            mark_instruction();
            // push next instruction offset to stack
            IDisasm.pushAddress(&current_va);
            break;
        case 0x70:
        case 0x71:
        case 0x72:
        case 0x73:
        case 0x74:
        case 0x75:
        case 0x76:
        case 0x77:
        case 0x78:
        case 0x79:
        case 0x7a:
        case 0x7b:
        case 0x7c:
        case 0x7d:
        case 0x7e:
        case 0x7f:
            rel8 = read_byte();
            mark_instruction();
            IDisasm.pushAddress(&current_va);
            current_va.offset += rel8;
            le_createLabel(current_va.obj_n, current_va.offset);
            sprintf(buffer, FORMAT"short %s", getJCC(op&15),
                getLabel(current_va.obj_n, current_va.offset));
            IDisasm.pushAddress(&current_va);
            break;
        case 0x80:
            if(!op_80()) goto l_def;
            break;
        case 0x81:
            if(!op_81()) goto l_def;      
            break;
        case 0x83:
            if(!op_83()) goto l_def;         
            break;
        case 0x84:
            if(!op_84()) goto l_def;
            break;
        case 0x85:
            if(!op_85()) goto l_def;
            break;
        case 0x86:
            operand_size = 8;
        case 0x87:
            address_mode = read_byte();
            sprintf(buffer, FORMAT"%s, %s", 
                "xchg", mod_reg(), mod_rm());
            mark_instruction();
            IDisasm.pushAddress(&current_va);
            break;
        case 0x88:  // MOV 
            if(!op_88()) goto l_def;
            break;
        case 0x89:  // MOV 
            if(!op_89()) goto l_def;
            break;
        case 0x8a:
            if(!op_8a()) goto l_def;
            break;
        case 0x8b:
            if(!op_8b()) goto l_def;
            break;
        case 0x8c:
            address_mode = read_byte();
            sprintf(buffer, FORMAT"%s, %s", "mov", 
                mod_rm(), mod_sreg());
            mark_instruction(); comment_instruction();
            IDisasm.pushAddress(&current_va);
            break;
        case 0x8d:
            if(!op_8d()) goto l_def;
            break;
        case 0x8e:
            address_mode = read_byte();
            sprintf(buffer, FORMAT"%s, %s", "mov", 
                mod_sreg(), mod_rm());
            mark_instruction(); comment_instruction();
            IDisasm.pushAddress(&current_va);
            break;
        case 0x8f:
            address_mode = read_byte();
            if(((address_mode>>3)&7) == 0){

                tmp0 = ((address_mode>>6) == 3) ? "" :
                        (operand_size == 32) ? "dword " : "word ";

                sprintf(buffer, FORMAT"%s%s", "pop", tmp0, mod_rm());
                mark_instruction();
                IDisasm.pushAddress(&current_va);
            }
            else goto l_def;
            break;
        case 0x90:
            sprintf(buffer, FORMAT, "nop");
            mark_instruction();
            // push next instruction offset to stack
            IDisasm.pushAddress(&current_va);
            break;
        case 0x91:
            if(prefix_66) goto l_def;
            sprintf(buffer, FORMAT"%s, %s", "xchg", "ecx", "eax");
            mark_instruction();
            // push next instruction offset to stack
            IDisasm.pushAddress(&current_va);
            break;
        case 0x92:
            if(prefix_66) goto l_def;
            sprintf(buffer, FORMAT"%s, %s", "xchg", "edx", "eax");
            mark_instruction();
            // push next instruction offset to stack
            IDisasm.pushAddress(&current_va);
            break;
        case 0x93:
            if(prefix_66) goto l_def;
            sprintf(buffer, FORMAT"%s, %s", "xchg", "ebx", "eax");
            mark_instruction();
            // push next instruction offset to stack
            IDisasm.pushAddress(&current_va);
            break;
        case 0x96:
            if(prefix_66) goto l_def;
            sprintf(buffer, FORMAT"%s, %s", "xchg", "esi", "eax");
            mark_instruction();
            // push next instruction offset to stack
            IDisasm.pushAddress(&current_va);
            break;
		case 0x98:
            if(prefix_66){
				sprintf(buffer, FORMAT, "cbw");
			}
            else {
				sprintf(buffer, FORMAT, "cwde");
			}
			mark_instruction();
            // push next instruction offset to stack
            IDisasm.pushAddress(&current_va);
            break;
        case 0x99:
            if(prefix_66){
				sprintf(buffer, FORMAT, "cwd");
			}
            else {
				sprintf(buffer, FORMAT, "cdq");
			}
			mark_instruction();
            // push next instruction offset to stack
            IDisasm.pushAddress(&current_va);
            break;
        case 0x9b:
            if(prefix_66) goto l_def;
            sprintf(buffer, FORMAT, "wait");
            mark_instruction();
            // push next instruction offset to stack
            IDisasm.pushAddress(&current_va);
            break;
        case 0x9c:
            if(prefix_66){
                sprintf(buffer, FORMAT, "pushfw");
            }
            else {
                sprintf(buffer, FORMAT, "pushfd");
            }
            mark_instruction();
            // push next instruction offset to stack
            IDisasm.pushAddress(&current_va);
            break;
        case 0x9d:
            if(prefix_66){
                sprintf(buffer, FORMAT, "popfw");
            }
            else {
                sprintf(buffer, FORMAT, "popfd");
            }
            mark_instruction();
            // push next instruction offset to stack
            IDisasm.pushAddress(&current_va);
            break;
		case 0x9e:
            if(prefix_66) goto l_def;
            sprintf(buffer, FORMAT, "sahf");
            mark_instruction();
            // push next instruction offset to stack
            IDisasm.pushAddress(&current_va);
            break;
        case 0xa0:
            fx = le_checkFixup(current_va.obj_n, current_va.offset);
            if(fx->size == 4){
                current_va.offset += 4;
                sprintf(buffer, FORMAT"%s, [%s%s]", 
                    "mov", "al", seg_ds, getLabel(fx->object_n, fx->target));
            }
            else {
                sprintf(buffer, FORMAT"%s, [%s%08x]", 
                    "mov", "al", seg_ds, read_dword());
            }
            mark_instruction();
            // push next instruction offset to stack
            IDisasm.pushAddress(&current_va);
            break;
        case 0xa1:
            fx = le_checkFixup(current_va.obj_n, current_va.offset);
            if(fx->size == 4){
                current_va.offset += 4;
                if(prefix_66){
                    sprintf(buffer, FORMAT"%s, [%s%s]", 
                        "mov", "ax", seg_ds, getLabel(fx->object_n, fx->target));
                }
                else {
                    sprintf(buffer, FORMAT"%s, [%s%s]", 
                        "mov", "eax", seg_ds, getLabel(fx->object_n, fx->target));
                }
            }
            else {
                if(prefix_66){
                    sprintf(buffer, FORMAT"%s, [%s%08x]", 
                        "mov", "ax", seg_ds, read_dword());
                }
                else {
                    sprintf(buffer, FORMAT"%s, [%s%08x]", 
                        "mov", "eax", seg_ds, read_dword());
                }
            }
            mark_instruction();
            // push next instruction offset to stack
            IDisasm.pushAddress(&current_va);
            break;
        case 0xa2:
            if(prefix_66) goto l_def;
            fx = le_checkFixup(current_va.obj_n, current_va.offset);
            if(fx->size == 4){
                read_dword();
                sprintf(buffer, FORMAT"[%s%s], %s", "mov", 
                    seg_ds, getLabel(fx->object_n, fx->target), "al");
            }
            else goto l_def;
            mark_instruction();
            // push next instruction offset to stack
            IDisasm.pushAddress(&current_va);
            break;
        case 0xa3:
            fx = le_checkFixup(current_va.obj_n, current_va.offset);
            if(fx->size == 4){
                read_dword();
                if(prefix_66){
                    sprintf(buffer, FORMAT"[%s%s], %s", 
                        "mov", seg_ds, getLabel(fx->object_n, fx->target), "ax");
                }
                else {
                    sprintf(buffer, FORMAT"[%s%s], %s", 
                        "mov", seg_ds, getLabel(fx->object_n, fx->target), "eax");
                }
            }
            else {
                if(prefix_66){
                    sprintf(buffer, FORMAT"[%s%s], %s", 
                        "mov", seg_ds, hexbyte(read_dword()), "ax");
                }
                else {
                    sprintf(buffer, FORMAT"[%s%s], %s", 
                        "mov", seg_ds, hexbyte(read_dword()), "eax");
                }
            }
            mark_instruction();
            // push next instruction offset to stack
            IDisasm.pushAddress(&current_va);
            break;
        case 0xa4:
            if(prefix_66) comment_instruction();
            if(rep_prefix[0]) rep_prefix = "rep ";
            sprintf(buffer, "%s"FORMAT, rep_prefix, "movsb");
            mark_instruction();
            // push next instruction offset to stack
            IDisasm.pushAddress(&current_va);
            break;
        case 0xa5:
            if(rep_prefix[0]) rep_prefix = "rep ";
            if(prefix_66){
                sprintf(buffer, "%s"FORMAT, rep_prefix, "movsw");
            }
            else {
                sprintf(buffer, "%s"FORMAT, rep_prefix, "movsd");
            }
            mark_instruction();
            // push next instruction offset to stack
            IDisasm.pushAddress(&current_va);
            break;
        case 0xa6:
            sprintf(buffer, "%s"FORMAT, rep_prefix, "cmpsb");
            mark_instruction();
            // push next instruction offset to stack
            IDisasm.pushAddress(&current_va);
            break;
        case 0xa8:  // test AL, IB 
            ib = read_byte();
            sprintf(buffer, FORMAT"%s, %s", "test", "al", hexbyte(ib));
            mark_instruction();
            // push next instruction offset to stack
            IDisasm.pushAddress(&current_va);
            break;
        case 0xa9:  // test
            if(prefix_66){
                iw = read_word();
                sprintf(buffer, FORMAT"%s, %s", "test", "ax", hexbyte(iw));
                mark_instruction();
                IDisasm.pushAddress(&current_va);
            }
            else {
                id = read_dword();
                sprintf(buffer, FORMAT"%s, %s", "test", "eax", hexbyte(id));
                mark_instruction();
                IDisasm.pushAddress(&current_va);
            }
            break;
        case 0xaa:
            if(rep_prefix[0]) rep_prefix = "rep ";
            sprintf(buffer, "%s"FORMAT, rep_prefix, "stosb");
            mark_instruction();
            // push next instruction offset to stack
            IDisasm.pushAddress(&current_va);
            break;
        case 0xab:
            if(rep_prefix[0]) rep_prefix = "rep ";
            if(operand_size == 32){
                sprintf(buffer, "%s"FORMAT, rep_prefix, "stosd");
            }
            else if(operand_size == 16){
                sprintf(buffer, "%s"FORMAT, rep_prefix, "stosw");
            }
            else goto l_def;
            mark_instruction();
            // push next instruction offset to stack
            IDisasm.pushAddress(&current_va);
            break;
        case 0xac:
            if(rep_prefix[0]) rep_prefix = "rep ";
            sprintf(buffer, "%s"FORMAT, rep_prefix, "lodsb");
            mark_instruction();
            // push next instruction offset to stack
            IDisasm.pushAddress(&current_va);
            break;
        case 0xad:
            if(rep_prefix[0]) rep_prefix = "rep ";
            if(prefix_66){
                sprintf(buffer, "%s"FORMAT, rep_prefix, "lodsw");
            }
            else {
                sprintf(buffer, "%s"FORMAT, rep_prefix, "lodsd");
            }
            mark_instruction();
            // push next instruction offset to stack
            IDisasm.pushAddress(&current_va);
            break;
        case 0xae:
            sprintf(buffer, "%s"FORMAT, rep_prefix, "scasb");
            mark_instruction();
            // push next instruction offset to stack
            IDisasm.pushAddress(&current_va);
            break;
        case 0xaf:
            if(prefix_66){
                sprintf(buffer, "%s"FORMAT, rep_prefix, "scasw");
            }
            else {
                sprintf(buffer, "%s"FORMAT, rep_prefix, "scasd");
            }
            mark_instruction();
            // push next instruction offset to stack
            IDisasm.pushAddress(&current_va);
            break;
        case 0xb0:
        case 0xb1:
        case 0xb2:
        case 0xb3:
        case 0xb4:
        case 0xb5:
        case 0xb6:
        case 0xb7:
            operand_size = 8;
            ib = read_byte();
            sprintf(buffer, FORMAT"%s, %s", "mov", 
                getRegister(op&7), hexbyte(ib));
            mark_instruction();
            IDisasm.pushAddress(&current_va);
            break;
        case 0xb8:
        case 0xb9:
        case 0xba:
        case 0xbb:
        case 0xbc:
        case 0xbd:
        case 0xbe:
        case 0xbf:
            fx = le_checkFixup(current_va.obj_n, current_va.offset);
            if(operand_size == 32){
                id = read_dword();
                if(fx&&(fx->size == 4)){
                    tmp_str = getLabel(fx->object_n, fx->target);
                }
                else {
                    tmp_str = hexbyte(id);
                }
            }
            else if(operand_size == 16){
                iw = read_word();
                if(fx&&(fx->type == 2)&&(fx->size == 2)){
                    tmp_str = getObjectSegmentMapName(fx->object_n);
                }
                else {
                    tmp_str = hexbyte(iw);
                }
            }
            else goto l_def;
            sprintf(buffer, FORMAT"%s, %s", "mov", 
                getRegister(op&7), tmp_str);
            mark_instruction();
            if(*seg_seg) comment_instruction();
            IDisasm.pushAddress(&current_va);
            break;
        case 0xc0:
            if(!op_c0()) goto l_def;
            break;
        case 0xc1:
            if(!op_c1()) goto l_def;
            break;
        case 0xc2:  // RET
            iw = read_word();
            sprintf(buffer, FORMAT"%s", "ret", hexbyte(iw));
            mark_instruction();
            break;
        case 0xc3:  // RET
            sprintf(buffer, FORMAT, "retn");
            mark_instruction();
            break;
        case 0xc4:
            if(!op_c4()) goto l_def;
            break;
        case 0xc6:
            if(!op_c6()) goto l_def;
            break;
        case 0xc7:
            if(!op_c7()) goto l_def;
            break;
		case 0xc8:  // ENTER
			iw = read_word();
			ib = read_byte();
            sprintf(buffer, FORMAT"%s, %s", "enter", hexbyte(iw), hexbyte(ib));
            mark_instruction();
            // push next instruction offset to stack
            IDisasm.pushAddress(&current_va);
            break;
		case 0xc9:  // LEAVE
            sprintf(buffer, FORMAT, "leave");
            mark_instruction();
            // push next instruction offset to stack
            IDisasm.pushAddress(&current_va);
            break;
        case 0xcb:  // RETF
            sprintf(buffer, FORMAT, "retf");
            mark_instruction();
            break;
        case 0xcc:  // INT3
            sprintf(buffer, FORMAT, "int3");
            mark_instruction();
            // push next instruction offset to stack
            IDisasm.pushAddress(&current_va);
            break;
        case 0xcd:  // INT ib
            ib = read_byte();
            //sprintf(buffer, FORMAT"dword [%s], %s\n\t\t"FORMAT"%s", 
            //    "mov", "int_fn", hexbyte(ib), "call", "soft_int");
            sprintf(buffer, FORMAT"%s", "int", hexbyte(ib));
            mark_instruction();
            // push next instruction offset to stack
            IDisasm.pushAddress(&current_va);
            break;
        case 0xcf:  // IRET
            sprintf(buffer, FORMAT, "iret");
            mark_instruction();
            break;
        case 0xd0:
            if(!op_d0()) goto l_def;
            break;
        case 0xd1:
            if(!op_d1()) goto l_def;
            break;
        case 0xd2:
            switch(read_byte()){
            case 0xc0:
                if(prefix_66) goto l_def;
                sprintf(buffer, FORMAT"%s, %s", "rol",
                    "al", "cl");
                mark_instruction();
                // push next instruction offset to stack
                IDisasm.pushAddress(&current_va);
                break;
            case 0xc8:
                if(prefix_66) goto l_def;
                sprintf(buffer, FORMAT"%s, %s", "ror",
                    "al", "cl");
                mark_instruction();
                // push next instruction offset to stack
                IDisasm.pushAddress(&current_va);
                break;
            default:
                goto l_def;
            }
            break;
        case 0xd3:
            switch(read_byte()){
            case 0xc0:
                if(prefix_66){
                    sprintf(buffer, FORMAT"%s, %s", "rol",
                    "ax", "cl");
                    mark_instruction();
                    // push next instruction offset to stack
                    IDisasm.pushAddress(&current_va);
                }
                else goto l_def;
                break;
            case 0xca:
                if(prefix_66){
                    sprintf(buffer, FORMAT"%s, %s", "ror",
                    "dx", "cl");
                    mark_instruction();
                    // push next instruction offset to stack
                    IDisasm.pushAddress(&current_va);
                }
                else goto l_def;
                break;
            case 0xe0:
                if(prefix_66) goto l_def;
                sprintf(buffer, FORMAT"%s, %s", "shl",
                    "eax", "cl");
                mark_instruction();
                // push next instruction offset to stack
                IDisasm.pushAddress(&current_va);
                break;
            case 0xe2:
                if(prefix_66) goto l_def;
                sprintf(buffer, FORMAT"%s, %s", "shl",
                    "edx", "cl");
                mark_instruction();
                // push next instruction offset to stack
                IDisasm.pushAddress(&current_va);
                break;
            case 0xe3:
                if(!prefix_66) goto l_def;
                sprintf(buffer, FORMAT"%s, %s", "shl",
                    "bx", "cl");
                mark_instruction();
                // push next instruction offset to stack
                IDisasm.pushAddress(&current_va);
                break;
            case 0xe8:
                if(prefix_66) goto l_def;
                sprintf(buffer, FORMAT"%s, %s", "shr",
                    "eax", "cl");
                mark_instruction();
                // push next instruction offset to stack
                IDisasm.pushAddress(&current_va);
                break;
            case 0xea:
                if(prefix_66) goto l_def;
                sprintf(buffer, FORMAT"%s, %s", "shr",
                    "edx", "cl");
                mark_instruction();
                // push next instruction offset to stack
                IDisasm.pushAddress(&current_va);
                break;
            case 0xf8:
                if(prefix_66) goto l_def;
                sprintf(buffer, FORMAT"%s, %s", "sar",
                    "eax", "cl");
                mark_instruction();
                // push next instruction offset to stack
                IDisasm.pushAddress(&current_va);
                break;
            case 0xfa:
                if(prefix_66) goto l_def;
                sprintf(buffer, FORMAT"%s, %s", "sar",
                    "edx", "cl");
                mark_instruction();
                // push next instruction offset to stack
                IDisasm.pushAddress(&current_va);
                break;
            case 0xfb:
                if(prefix_66) goto l_def;
                sprintf(buffer, FORMAT"%s, %s", "sar",
                    "ebx", "cl");
                mark_instruction();
                // push next instruction offset to stack
                IDisasm.pushAddress(&current_va);
                break;
            default:
                goto l_def;
            }
            break;
        case 0xd4:
            if(prefix_66) goto l_def;
            ib = read_byte();
            sprintf(buffer, FORMAT"%s", "aam", hexbyte(ib));
            mark_instruction();
            // push next instruction offset to stack
            IDisasm.pushAddress(&current_va);
            break;
        case 0xd8:
            switch(read_byte()){
			case 0x05:
                address_mode = 0x05;
                sprintf(buffer, FORMAT"dword %s", "fadd", mod_rm());
                mark_instruction();
                // push next instruction offset to stack
                IDisasm.pushAddress(&current_va);
                break;
            case 0x0c:
                address_mode = 0x0c;
                sprintf(buffer, FORMAT"dword %s", "fmul", mod_rm());
                mark_instruction();
                // push next instruction offset to stack
                IDisasm.pushAddress(&current_va);
                break;
			case 0x0d:
                address_mode = 0x0d;
                sprintf(buffer, FORMAT"dword %s", "fmul", mod_rm());
                mark_instruction();
                // push next instruction offset to stack
                IDisasm.pushAddress(&current_va);
                break;
			case 0x1c:
                address_mode = 0x1c;
                sprintf(buffer, FORMAT"dword %s", "fcomp", mod_rm());
                mark_instruction();
                // push next instruction offset to stack
                IDisasm.pushAddress(&current_va);
                break;
			case 0x1d:
                address_mode = 0x1d;
                sprintf(buffer, FORMAT"dword %s", "fcomp", mod_rm());
                mark_instruction();
                // push next instruction offset to stack
                IDisasm.pushAddress(&current_va);
                break;
			case 0x2d:
                address_mode = 0x2d;
                sprintf(buffer, FORMAT"dword %s", "fsubr", mod_rm());
                mark_instruction();
                // push next instruction offset to stack
                IDisasm.pushAddress(&current_va);
                break;
			case 0x34:
                address_mode = 0x34;
                sprintf(buffer, FORMAT"dword %s", "fdiv", mod_rm());
                mark_instruction();
                // push next instruction offset to stack
                IDisasm.pushAddress(&current_va);
                break;
			case 0x35:
                address_mode = 0x35;
                sprintf(buffer, FORMAT"dword %s", "fdiv", mod_rm());
                mark_instruction();
                // push next instruction offset to stack
                IDisasm.pushAddress(&current_va);
                break;
			case 0x3c:
                address_mode = 0x3c;
                sprintf(buffer, FORMAT"dword %s", "fdivr", mod_rm());
                mark_instruction();
                // push next instruction offset to stack
                IDisasm.pushAddress(&current_va);
                break;
            case 0x44:
                address_mode = 0x44;
                sprintf(buffer, FORMAT"dword %s", "fadd", mod_rm());
                mark_instruction();
                // push next instruction offset to stack
                IDisasm.pushAddress(&current_va);
                break;
            case 0x4c:
                address_mode = 0x4c;
                sprintf(buffer, FORMAT"dword %s", "fmul", mod_rm());
                mark_instruction();
                // push next instruction offset to stack
                IDisasm.pushAddress(&current_va);
                break;
			case 0x5c:
                address_mode = 0x5c;
                sprintf(buffer, FORMAT"dword %s", "fcomp", mod_rm());
                mark_instruction();
                // push next instruction offset to stack
                IDisasm.pushAddress(&current_va);
                break;
            case 0x64:
                address_mode = 0x64;
                sprintf(buffer, FORMAT"dword %s", "fsub", mod_rm());
                mark_instruction();
                // push next instruction offset to stack
                IDisasm.pushAddress(&current_va);
                break;
            case 0x74:
                address_mode = 0x74;
                sprintf(buffer, FORMAT"dword %s", "fdiv", mod_rm());
                mark_instruction();
                // push next instruction offset to stack
                IDisasm.pushAddress(&current_va);
                break;
            case 0x7c:
                address_mode = 0x7c;
                sprintf(buffer, FORMAT"dword %s", "fdivr", mod_rm());
                mark_instruction();
                // push next instruction offset to stack
                IDisasm.pushAddress(&current_va);
                break;
			case 0x80:
                address_mode = 0x80;
                sprintf(buffer, FORMAT"dword %s", "fadd", mod_rm());
                mark_instruction();
                // push next instruction offset to stack
                IDisasm.pushAddress(&current_va);
                break;
			case 0x81:
                address_mode = 0x81;
                sprintf(buffer, FORMAT"dword %s", "fadd", mod_rm());
                mark_instruction();
                // push next instruction offset to stack
                IDisasm.pushAddress(&current_va);
                break;
			case 0x82:
                address_mode = 0x82;
                sprintf(buffer, FORMAT"dword %s", "fadd", mod_rm());
                mark_instruction();
                // push next instruction offset to stack
                IDisasm.pushAddress(&current_va);
                break;
			case 0x83:
                address_mode = 0x83;
                sprintf(buffer, FORMAT"dword %s", "fadd", mod_rm());
                mark_instruction();
                // push next instruction offset to stack
                IDisasm.pushAddress(&current_va);
                break;
			case 0x84:
                address_mode = 0x84;
                sprintf(buffer, FORMAT"dword %s", "fadd", mod_rm());
                mark_instruction();
                // push next instruction offset to stack
                IDisasm.pushAddress(&current_va);
                break;
			case 0x8a:
                address_mode = 0x8a;
                sprintf(buffer, FORMAT"dword %s", "fmul", mod_rm());
                mark_instruction();
                // push next instruction offset to stack
                IDisasm.pushAddress(&current_va);
                break;
			case 0x8b:
                address_mode = 0x8b;
                sprintf(buffer, FORMAT"dword %s", "fmul", mod_rm());
                mark_instruction();
                // push next instruction offset to stack
                IDisasm.pushAddress(&current_va);
                break;
			case 0x8c:
                address_mode = 0x8c;
                sprintf(buffer, FORMAT"dword %s", "fmul", mod_rm());
                mark_instruction();
                // push next instruction offset to stack
                IDisasm.pushAddress(&current_va);
                break;
            case 0x99:
                address_mode = 0x99;
                sprintf(buffer, FORMAT"dword %s", "fcomp", mod_rm());
                mark_instruction();
                // push next instruction offset to stack
                IDisasm.pushAddress(&current_va);
                break;
			case 0x9a:
                address_mode = 0x9a;
                sprintf(buffer, FORMAT"dword %s", "fcomp", mod_rm());
                mark_instruction();
                // push next instruction offset to stack
                IDisasm.pushAddress(&current_va);
                break;
			case 0x98:
                address_mode = 0x98;
                sprintf(buffer, FORMAT"dword %s", "fcomp", mod_rm());
                mark_instruction();
                // push next instruction offset to stack
                IDisasm.pushAddress(&current_va);
                break;
			case 0x9b:
                address_mode = 0x9b;
                sprintf(buffer, FORMAT"dword %s", "fcomp", mod_rm());
                mark_instruction();
                // push next instruction offset to stack
                IDisasm.pushAddress(&current_va);
                break;
			case 0x9c:
                address_mode = 0x9c;
                sprintf(buffer, FORMAT"dword %s", "fcomp", mod_rm());
                mark_instruction();
                // push next instruction offset to stack
                IDisasm.pushAddress(&current_va);
                break;
			case 0xa0:
                address_mode = 0xa0;
                sprintf(buffer, FORMAT"dword %s", "fsub", mod_rm());
                mark_instruction();
                // push next instruction offset to stack
                IDisasm.pushAddress(&current_va);
                break;
			case 0xa2:
                address_mode = 0xa2;
                sprintf(buffer, FORMAT"dword %s", "fsub", mod_rm());
                mark_instruction();
                // push next instruction offset to stack
                IDisasm.pushAddress(&current_va);
                break;
			case 0xa6:
                address_mode = 0xa6;
                sprintf(buffer, FORMAT"dword %s", "fsub", mod_rm());
                mark_instruction();
                // push next instruction offset to stack
                IDisasm.pushAddress(&current_va);
                break;
			case 0xa8:
                address_mode = 0xa8;
                sprintf(buffer, FORMAT"dword %s", "fsubr", mod_rm());
                mark_instruction();
                // push next instruction offset to stack
                IDisasm.pushAddress(&current_va);
                break;
			case 0xaa:
                address_mode = 0xaa;
                sprintf(buffer, FORMAT"dword %s", "fsubr", mod_rm());
                mark_instruction();
                // push next instruction offset to stack
                IDisasm.pushAddress(&current_va);
                break;
            case 0xb0:
                address_mode = 0xb0;
                sprintf(buffer, FORMAT"dword %s", "fdiv", mod_rm());
                mark_instruction();
                // push next instruction offset to stack
                IDisasm.pushAddress(&current_va);
                break;
			case 0xb2:
                address_mode = 0xb2;
                sprintf(buffer, FORMAT"dword %s", "fdiv", mod_rm());
                mark_instruction();
                // push next instruction offset to stack
                IDisasm.pushAddress(&current_va);
                break;
			case 0xb3:
                address_mode = 0xb3;
                sprintf(buffer, FORMAT"dword %s", "fdiv", mod_rm());
                mark_instruction();
                // push next instruction offset to stack
                IDisasm.pushAddress(&current_va);
                break;
			case 0xb4:
                address_mode = 0xb4;
                sprintf(buffer, FORMAT"dword %s", "fdiv", mod_rm());
                mark_instruction();
                // push next instruction offset to stack
                IDisasm.pushAddress(&current_va);
                break;
			case 0xc1:
                sprintf(buffer, FORMAT"%s, %s", "fadd", "st0", "st1");
                mark_instruction();
                // push next instruction offset to stack
                IDisasm.pushAddress(&current_va);
                break;
			case 0xc2:
                sprintf(buffer, FORMAT"%s, %s", "fadd", "st0", "st2");
                mark_instruction();
                // push next instruction offset to stack
                IDisasm.pushAddress(&current_va);
                break;
            case 0xc3:
                sprintf(buffer, FORMAT"%s, %s", "fadd", "st0", "st3");
                mark_instruction();
                // push next instruction offset to stack
                IDisasm.pushAddress(&current_va);
                break;
			case 0xc4:
                sprintf(buffer, FORMAT"%s, %s", "fadd", "st0", "st4");
                mark_instruction();
                // push next instruction offset to stack
                IDisasm.pushAddress(&current_va);
                break;
			case 0xc5:
                sprintf(buffer, FORMAT"%s, %s", "fadd", "st0", "st5");
                mark_instruction();
                // push next instruction offset to stack
                IDisasm.pushAddress(&current_va);
                break;
			case 0xc6:
                sprintf(buffer, FORMAT"%s, %s", "fadd", "st0", "st6");
                mark_instruction();
                // push next instruction offset to stack
                IDisasm.pushAddress(&current_va);
                break;
			case 0xc8:
                sprintf(buffer, FORMAT"%s, %s", "fmul", "st0", "st0");
                mark_instruction();
                // push next instruction offset to stack
                IDisasm.pushAddress(&current_va);
                break;
            case 0xc9:
                sprintf(buffer, FORMAT"%s, %s", "fmul", "st0", "st1");
                mark_instruction();
                // push next instruction offset to stack
                IDisasm.pushAddress(&current_va);
                break;
            case 0xca:
                sprintf(buffer, FORMAT"%s, %s", "fmul", "st0", "st2");
                mark_instruction();
                // push next instruction offset to stack
                IDisasm.pushAddress(&current_va);
                break;
			case 0xcb:
                sprintf(buffer, FORMAT"%s, %s", "fmul", "st0", "st3");
                mark_instruction();
                // push next instruction offset to stack
                IDisasm.pushAddress(&current_va);
                break;
			case 0xcc:
                sprintf(buffer, FORMAT"%s, %s", "fmul", "st0", "st4");
                mark_instruction();
                // push next instruction offset to stack
                IDisasm.pushAddress(&current_va);
                break;
            case 0xcd:
                sprintf(buffer, FORMAT"%s, %s", "fmul", "st0", "st5");
                mark_instruction();
                // push next instruction offset to stack
                IDisasm.pushAddress(&current_va);
                break;
			case 0xe2:
                sprintf(buffer, FORMAT"%s, %s", "fsub", "st0", "st2");
                mark_instruction();
                // push next instruction offset to stack
                IDisasm.pushAddress(&current_va);
                break;
			case 0xe5:
                sprintf(buffer, FORMAT"%s, %s", "fsub", "st0", "st5");
                mark_instruction();
                // push next instruction offset to stack
                IDisasm.pushAddress(&current_va);
                break;
			case 0xe9:
                sprintf(buffer, FORMAT"%s, %s", "fsubr", "st0", "st1");
                mark_instruction();
                // push next instruction offset to stack
                IDisasm.pushAddress(&current_va);
                break;
			case 0xea:
                sprintf(buffer, FORMAT"%s, %s", "fsubr", "st0", "st2");
                mark_instruction();
                // push next instruction offset to stack
                IDisasm.pushAddress(&current_va);
                break;
            case 0xeb:
                sprintf(buffer, FORMAT"%s, %s", "fsubr", "st0", "st3");
                mark_instruction();
                // push next instruction offset to stack
                IDisasm.pushAddress(&current_va);
                break;
            case 0xf0:
                sprintf(buffer, FORMAT"%s, %s", "fdiv", "st0", "st0");
                mark_instruction();
                // push next instruction offset to stack
                IDisasm.pushAddress(&current_va);
                break;
            case 0xf1:
                sprintf(buffer, FORMAT"%s, %s", "fdiv", "st0", "st1");
                mark_instruction();
                // push next instruction offset to stack
                IDisasm.pushAddress(&current_va);
                break;
            case 0xf2:
                sprintf(buffer, FORMAT"%s, %s", "fdiv", "st0", "st2");
                mark_instruction();
                // push next instruction offset to stack
                IDisasm.pushAddress(&current_va);
                break;
            case 0xf3:
                sprintf(buffer, FORMAT"%s, %s", "fdiv", "st0", "st3");
                mark_instruction();
                // push next instruction offset to stack
                IDisasm.pushAddress(&current_va);
                break;
			case 0xf4:
                sprintf(buffer, FORMAT"%s, %s", "fdiv", "st0", "st4");
                mark_instruction();
                // push next instruction offset to stack
                IDisasm.pushAddress(&current_va);
                break;
            case 0xf8:
                sprintf(buffer, FORMAT"%s, %s", "fdivr", "st0", "st0");
                mark_instruction();
                // push next instruction offset to stack
                IDisasm.pushAddress(&current_va);
                break;
            default:
                goto l_def;
            }   
            break;
        case 0xd9:
            address_mode = read_byte();
			if(address_mode == 0x15){
                sprintf(buffer, FORMAT"dword %s", "fst", mod_rm());
                mark_instruction();
                // push next instruction offset to stack
                IDisasm.pushAddress(&current_va);
                break;
            }
            if(address_mode == 0x17){
                sprintf(buffer, FORMAT"dword %s", "fst", mod_rm());
                mark_instruction();
                // push next instruction offset to stack
                IDisasm.pushAddress(&current_va);
                break;
            }
            if(address_mode == 0x2c){
                sprintf(buffer, FORMAT"word %s", "fldcw", mod_rm());
                mark_instruction();
                // push next instruction offset to stack
                IDisasm.pushAddress(&current_va);
                break;
            }
            if(address_mode == 0x2d){
                sprintf(buffer, FORMAT"word %s", "fldcw", mod_rm());
                mark_instruction();
                // push next instruction offset to stack
                IDisasm.pushAddress(&current_va);
                break;
            }
            if(address_mode == 0x3c){
                sprintf(buffer, FORMAT"word %s", "fnstcw", mod_rm());
                mark_instruction();
                // push next instruction offset to stack
                IDisasm.pushAddress(&current_va);
                break;
            }
			if(address_mode == 0x54){
                sprintf(buffer, FORMAT"dword %s", "fst", mod_rm());
                mark_instruction();
                // push next instruction offset to stack
                IDisasm.pushAddress(&current_va);
                break;
            }
            if(address_mode == 0x6c){
                sprintf(buffer, FORMAT"word %s", "fldcw", mod_rm());
                mark_instruction();
                // push next instruction offset to stack
                IDisasm.pushAddress(&current_va);
                break;
            }
            if(address_mode == 0x6d){
                sprintf(buffer, FORMAT"word %s", "fldcw", mod_rm());
                mark_instruction();
                // push next instruction offset to stack
                IDisasm.pushAddress(&current_va);
                break;
            }
            if(address_mode == 0x75){
                sprintf(buffer, FORMAT"%s", "fnstenv", mod_rm());
                mark_instruction();
                // push next instruction offset to stack
                IDisasm.pushAddress(&current_va);
                break;
            }
            if(address_mode == 0x7c){
                sprintf(buffer, FORMAT"word %s", "fnstcw", mod_rm());
                mark_instruction();
                // push next instruction offset to stack
                IDisasm.pushAddress(&current_va);
                break;
            }
			if(address_mode == 0x90){
                sprintf(buffer, FORMAT"dword %s", "fst", mod_rm());
                mark_instruction();
                // push next instruction offset to stack
                IDisasm.pushAddress(&current_va);
                break;
            }
			if(address_mode == 0x92){
                sprintf(buffer, FORMAT"dword %s", "fst", mod_rm());
                mark_instruction();
                // push next instruction offset to stack
                IDisasm.pushAddress(&current_va);
                break;
            }
            if(address_mode == 0xc0){
                sprintf(buffer, FORMAT"%s", "fld", "st0");
                mark_instruction();
                // push next instruction offset to stack
                IDisasm.pushAddress(&current_va);
                break;
            }
            if(address_mode == 0xc1){
                sprintf(buffer, FORMAT"%s", "fld", "st1");
                mark_instruction();
                // push next instruction offset to stack
                IDisasm.pushAddress(&current_va);
                break;
            }
            if(address_mode == 0xc2){
                sprintf(buffer, FORMAT"%s", "fld", "st2");
                mark_instruction();
                // push next instruction offset to stack
                IDisasm.pushAddress(&current_va);
                break;
            }
			if(address_mode == 0xc3){
                sprintf(buffer, FORMAT"%s", "fld", "st3");
                mark_instruction();
                // push next instruction offset to stack
                IDisasm.pushAddress(&current_va);
                break;
            }
            if(address_mode == 0xc9){
                sprintf(buffer, FORMAT"%s, %s", "fxch", "st0", "st1");
                mark_instruction();
                // push next instruction offset to stack
                IDisasm.pushAddress(&current_va);
                break;
            }
            if(address_mode == 0xca){
                sprintf(buffer, FORMAT"%s, %s", "fxch", "st0", "st2");
                mark_instruction();
                // push next instruction offset to stack
                IDisasm.pushAddress(&current_va);
                break;
            }
            if(address_mode == 0xcb){
                sprintf(buffer, FORMAT"%s, %s", "fxch", "st0", "st3");
                mark_instruction();
                // push next instruction offset to stack
                IDisasm.pushAddress(&current_va);
                break;
            }
            if(address_mode == 0xcc){
                sprintf(buffer, FORMAT"%s, %s", "fxch", "st0", "st4");
                mark_instruction();
                // push next instruction offset to stack
                IDisasm.pushAddress(&current_va);
                break;
            }
            if(address_mode == 0xcd){
                sprintf(buffer, FORMAT"%s, %s", "fxch", "st0", "st5");
                mark_instruction();
                // push next instruction offset to stack
                IDisasm.pushAddress(&current_va);
                break;
            }
			if(address_mode == 0xce){
                sprintf(buffer, FORMAT"%s, %s", "fxch", "st0", "st6");
                mark_instruction();
                // push next instruction offset to stack
                IDisasm.pushAddress(&current_va);
                break;
            }
            if(address_mode == 0xcf){
                sprintf(buffer, FORMAT"%s, %s", "fxch", "st0", "st7");
                mark_instruction();
                // push next instruction offset to stack
                IDisasm.pushAddress(&current_va);
                break;
            }
            if(address_mode == 0xe0){
                sprintf(buffer, FORMAT, "fchs");
                mark_instruction();
                // push next instruction offset to stack
                IDisasm.pushAddress(&current_va);
                break;
            }
            if(address_mode == 0xe1){
                sprintf(buffer, FORMAT, "fabs");
                mark_instruction();
                // push next instruction offset to stack
                IDisasm.pushAddress(&current_va);
                break;
            }
            if(address_mode == 0xe4){
                sprintf(buffer, FORMAT, "ftst");
                mark_instruction();
                // push next instruction offset to stack
                IDisasm.pushAddress(&current_va);
                break;
            }
            if(address_mode == 0xe8){
                sprintf(buffer, FORMAT, "fld1");
                mark_instruction();
                // push next instruction offset to stack
                IDisasm.pushAddress(&current_va);
                break;
            }
            if(address_mode == 0xec){
                sprintf(buffer, FORMAT, "fldlg2");
                mark_instruction();
                // push next instruction offset to stack
                IDisasm.pushAddress(&current_va);
                break;
            }
            if(address_mode == 0xed){
                sprintf(buffer, FORMAT, "fldln2");
                mark_instruction();
                // push next instruction offset to stack
                IDisasm.pushAddress(&current_va);
                break;
            }
            if(address_mode == 0xee){
                sprintf(buffer, FORMAT, "fldz");
                mark_instruction();
                // push next instruction offset to stack
                IDisasm.pushAddress(&current_va);
                break;
            }
            if(address_mode == 0xf1){
                sprintf(buffer, FORMAT, "fyl2x");
                mark_instruction();
                // push next instruction offset to stack
                IDisasm.pushAddress(&current_va);
                break;
            }
            if(address_mode == 0xf6){
                sprintf(buffer, FORMAT, "fdecstp");
                mark_instruction();
                // push next instruction offset to stack
                IDisasm.pushAddress(&current_va);
                break;
            }
            if(address_mode == 0xf7){
                sprintf(buffer, FORMAT, "fincstp");
                mark_instruction();
                // push next instruction offset to stack
                IDisasm.pushAddress(&current_va);
                break;
            }
            if(address_mode == 0xfa){
                sprintf(buffer, FORMAT, "fsqrt");
                mark_instruction();
                // push next instruction offset to stack
                IDisasm.pushAddress(&current_va);
                break;
            }
            if(address_mode == 0xfc){
                sprintf(buffer, FORMAT, "frndint");
                mark_instruction();
                // push next instruction offset to stack
                IDisasm.pushAddress(&current_va);
                break;
            }
            if(address_mode == 0xfe){
                sprintf(buffer, FORMAT, "fsin");
                mark_instruction();
                // push next instruction offset to stack
                IDisasm.pushAddress(&current_va);
                break;
            }
            if(address_mode == 0xff){
                sprintf(buffer, FORMAT, "fcos");
                mark_instruction();
                // push next instruction offset to stack
                IDisasm.pushAddress(&current_va);
                break;
            }
            if(((address_mode>>3) & 7) == 3){
                sprintf(buffer, FORMAT"dword %s", "fstp", mod_rm());
                mark_instruction();
                // push next instruction offset to stack
                IDisasm.pushAddress(&current_va);
                break;
            }
            if(address_mode >= 0xc0) goto l_def;
            if(((address_mode >> 3) & 7) == 0){
                sprintf(buffer, FORMAT"dword %s", "fld", mod_rm());
                mark_instruction();
                // push next instruction offset to stack
                IDisasm.pushAddress(&current_va);
            }
            else goto l_def;
            break;
        case 0xdb:
            address_mode = read_byte();
            if(address_mode == 0x2c){
                sprintf(buffer, FORMAT"tword %s", "fld", mod_rm());
                mark_instruction();
                // push next instruction offset to stack
                IDisasm.pushAddress(&current_va);
                break;
            }
            if(address_mode == 0x2e){
                sprintf(buffer, FORMAT"tword %s", "fld", mod_rm());
                mark_instruction();
                // push next instruction offset to stack
                IDisasm.pushAddress(&current_va);
                break;
            }
            if(address_mode == 0x3c){
                sprintf(buffer, FORMAT"tword %s", "fstp", mod_rm());
                mark_instruction();
                // push next instruction offset to stack
                IDisasm.pushAddress(&current_va);
                break;
            }
            if(address_mode == 0x6c){
                sprintf(buffer, FORMAT"tword %s", "fld", mod_rm());
                mark_instruction();
                // push next instruction offset to stack
                IDisasm.pushAddress(&current_va);
                break;
            }
            if(address_mode == 0x7c){
                sprintf(buffer, FORMAT"tword %s", "fstp", mod_rm());
                mark_instruction();
                // push next instruction offset to stack
                IDisasm.pushAddress(&current_va);
                break;
            }
            if(address_mode == 0xe1){
                sprintf(buffer, FORMAT, "fndisi");
                mark_instruction();
                // push next instruction offset to stack
                IDisasm.pushAddress(&current_va);
                break;
            }
            if(address_mode == 0xe2){
                sprintf(buffer, FORMAT, "fnclex");
                mark_instruction();
                // push next instruction offset to stack
                IDisasm.pushAddress(&current_va);
                break;
            }
            if(address_mode == 0xe3){
                sprintf(buffer, FORMAT, "fninit");
                mark_instruction();
                // push next instruction offset to stack
                IDisasm.pushAddress(&current_va);
                break;
            }
            if(((address_mode >> 3) & 7) == 0){
                sprintf(buffer, FORMAT"dword %s", "fild", mod_rm());
                mark_instruction();
                // push next instruction offset to stack
                IDisasm.pushAddress(&current_va);
            }
            else if(((address_mode >> 3) & 7) == 3){
                sprintf(buffer, FORMAT"dword %s", "fistp", mod_rm());
                mark_instruction();
                // push next instruction offset to stack
                IDisasm.pushAddress(&current_va);
            }
            else goto l_def;
            break;
        case 0xdc:
            address_mode = read_byte();
			if(address_mode == 0x05){
                sprintf(buffer, FORMAT"qword %s", "fadd", mod_rm());
                mark_instruction();
                // push next instruction offset to stack
                IDisasm.pushAddress(&current_va);
                break;
            }
			if(address_mode == 0x1d){
                sprintf(buffer, FORMAT"qword %s", "fcomp", mod_rm());
                mark_instruction();
                // push next instruction offset to stack
                IDisasm.pushAddress(&current_va);
                break;
            }
			if(address_mode == 0x2d){
                sprintf(buffer, FORMAT"qword %s", "fsubr", mod_rm());
                mark_instruction();
                // push next instruction offset to stack
                IDisasm.pushAddress(&current_va);
                break;
            }
			if(address_mode == 0x44){
                sprintf(buffer, FORMAT"qword %s", "fadd", mod_rm());
                mark_instruction();
                // push next instruction offset to stack
                IDisasm.pushAddress(&current_va);
                break;
            }
            if(address_mode == 0x5d){
                sprintf(buffer, FORMAT"qword %s", "fcomp", mod_rm());
                mark_instruction();
                // push next instruction offset to stack
                IDisasm.pushAddress(&current_va);
                break;
            }
			if(address_mode == 0x64){
                sprintf(buffer, FORMAT"qword %s", "fsub", mod_rm());
                mark_instruction();
                // push next instruction offset to stack
                IDisasm.pushAddress(&current_va);
                break;
            }
			if(address_mode == 0x6c){
                sprintf(buffer, FORMAT"qword %s", "fsubr", mod_rm());
                mark_instruction();
                // push next instruction offset to stack
                IDisasm.pushAddress(&current_va);
                break;
            }
			if(address_mode == 0x84){
                sprintf(buffer, FORMAT"qword %s", "fadd", mod_rm());
                mark_instruction();
                // push next instruction offset to stack
                IDisasm.pushAddress(&current_va);
                break;
            }
			if(address_mode == 0x9c){
                sprintf(buffer, FORMAT"qword %s", "fcomp", mod_rm());
                mark_instruction();
                // push next instruction offset to stack
                IDisasm.pushAddress(&current_va);
                break;
            }
            if(address_mode == 0xa4){
                sprintf(buffer, FORMAT"qword %s", "fsub", mod_rm());
                mark_instruction();
                // push next instruction offset to stack
                IDisasm.pushAddress(&current_va);
                break;
            }
			if(address_mode == 0xac){
                sprintf(buffer, FORMAT"qword %s", "fsubr", mod_rm());
                mark_instruction();
                // push next instruction offset to stack
                IDisasm.pushAddress(&current_va);
                break;
            }
			if(address_mode == 0xc5){
                sprintf(buffer, FORMAT"%s, %s", "fadd", "st5", "st0");
                mark_instruction();
                // push next instruction offset to stack
                IDisasm.pushAddress(&current_va);
                break;
            }
			if(address_mode == 0xcd){
                sprintf(buffer, FORMAT"%s, %s", "fmul", "st5", "st0");
                mark_instruction();
                // push next instruction offset to stack
                IDisasm.pushAddress(&current_va);
                break;
            }
			if(address_mode == 0xe9){
                sprintf(buffer, FORMAT"%s, %s", "fsub", "st1", "st0");
                mark_instruction();
                // push next instruction offset to stack
                IDisasm.pushAddress(&current_va);
                break;
            }
            if(address_mode >= 0xf0){
                if(address_mode == 0xf4){
                    sprintf(buffer, FORMAT"%s, %s", "fdivr", "st4", "st0");
                    mark_instruction();
                    // push next instruction offset to stack
                    IDisasm.pushAddress(&current_va);
                    break;
                }
                else goto l_def;
            } 
            if(address_mode >= 0xf8) goto l_def;
 
            if(((address_mode >> 3) & 7) == 1){
                if(address_mode >= 0xc8) goto l_def;
                sprintf(buffer, FORMAT"qword %s", "fmul", mod_rm());
                mark_instruction();
                // push next instruction offset to stack
                IDisasm.pushAddress(&current_va);
                break;
            }
            if(((address_mode >> 3) & 7) == 6){
                sprintf(buffer, FORMAT"qword %s", "fdiv", mod_rm());
                mark_instruction();
                // push next instruction offset to stack
                IDisasm.pushAddress(&current_va);
                break;
            }
            if(((address_mode >> 3) & 7) == 7){
                sprintf(buffer, FORMAT"qword %s", "fdivr", mod_rm());
                mark_instruction();
                // push next instruction offset to stack
                IDisasm.pushAddress(&current_va);
                break;
            }
            goto l_def;
            break;
        case 0xdd:
            address_mode = read_byte();
            if(address_mode == 0x17){
                sprintf(buffer, FORMAT"qword %s", "fst", mod_rm());
                mark_instruction();
                // push next instruction offset to stack
                IDisasm.pushAddress(&current_va);
                break;
            }
            if(address_mode == 0x20){
                sprintf(buffer, FORMAT"%s", "frstor", mod_rm());
                mark_instruction();
                // push next instruction offset to stack
                IDisasm.pushAddress(&current_va);
                break;
            }
            if(address_mode == 0x30){
                sprintf(buffer, FORMAT"%s", "fnsave", mod_rm());
                mark_instruction();
                // push next instruction offset to stack
                IDisasm.pushAddress(&current_va);
                break;
            }
            if(address_mode == 0x54){
                sprintf(buffer, FORMAT"qword %s", "fst", mod_rm());
                mark_instruction();
                // push next instruction offset to stack
                IDisasm.pushAddress(&current_va);
                break;
            }
            if(address_mode == 0x7d){
                sprintf(buffer, FORMAT"word %s", "fnstsw", mod_rm());
                mark_instruction();
                // push next instruction offset to stack
                IDisasm.pushAddress(&current_va);
                break;
            }
			if(address_mode == 0x94){
                sprintf(buffer, FORMAT"qword %s", "fst", mod_rm());
                mark_instruction();
                // push next instruction offset to stack
                IDisasm.pushAddress(&current_va);
                break;
            }
            if(address_mode == 0xd8){
                sprintf(buffer, FORMAT"%s", "fstp", "st0");
                mark_instruction();
                // push next instruction offset to stack
                IDisasm.pushAddress(&current_va);
                break;
            }
            if(address_mode == 0xd9){
                sprintf(buffer, FORMAT"%s", "fstp", "st1");
                mark_instruction();
                // push next instruction offset to stack
                IDisasm.pushAddress(&current_va);
                break;
            }
			if(address_mode == 0xda){
                sprintf(buffer, FORMAT"%s", "fstp", "st2");
                mark_instruction();
                // push next instruction offset to stack
                IDisasm.pushAddress(&current_va);
                break;
            }
			if(address_mode == 0xdb){
                sprintf(buffer, FORMAT"%s", "fstp", "st3");
                mark_instruction();
                // push next instruction offset to stack
                IDisasm.pushAddress(&current_va);
                break;
            }
			if(address_mode == 0xdc){
                sprintf(buffer, FORMAT"%s", "fstp", "st4");
                mark_instruction();
                // push next instruction offset to stack
                IDisasm.pushAddress(&current_va);
                break;
            }
			if(address_mode == 0xdd){
                sprintf(buffer, FORMAT"%s", "fstp", "st5");
                mark_instruction();
                // push next instruction offset to stack
                IDisasm.pushAddress(&current_va);
                break;
            }
			if(address_mode == 0xde){
                sprintf(buffer, FORMAT"%s", "fstp", "st6");
                mark_instruction();
                // push next instruction offset to stack
                IDisasm.pushAddress(&current_va);
                break;
            }
            if(address_mode >= 0xd8) goto l_def;
            if(((address_mode >> 3) & 7) == 0){
                sprintf(buffer, FORMAT"qword %s", "fld", mod_rm());
                mark_instruction();
                // push next instruction offset to stack
                IDisasm.pushAddress(&current_va);
            }
            else if(((address_mode >> 3) & 7) == 3){
                sprintf(buffer, FORMAT"qword %s", "fstp", mod_rm());
                mark_instruction();
                // push next instruction offset to stack
                IDisasm.pushAddress(&current_va);
            }
            else goto l_def;
            break;
        case 0xde:
            switch(read_byte()){
            case 0xc1:
                sprintf(buffer, FORMAT"%s, %s", "faddp", "st1", "st0");
                mark_instruction();
                // push next instruction offset to stack
                IDisasm.pushAddress(&current_va);
                break;
			case 0xc2:
                sprintf(buffer, FORMAT"%s, %s", "faddp", "st2", "st0");
                mark_instruction();
                // push next instruction offset to stack
                IDisasm.pushAddress(&current_va);
                break;
            case 0xc3:
                sprintf(buffer, FORMAT"%s, %s", "faddp", "st3", "st0");
                mark_instruction();
                // push next instruction offset to stack
                IDisasm.pushAddress(&current_va);
                break;
            case 0xc4:
                sprintf(buffer, FORMAT"%s, %s", "faddp", "st4", "st0");
                mark_instruction();
                // push next instruction offset to stack
                IDisasm.pushAddress(&current_va);
                break;
			case 0xc5:
                sprintf(buffer, FORMAT"%s, %s", "faddp", "st5", "st0");
                mark_instruction();
                // push next instruction offset to stack
                IDisasm.pushAddress(&current_va);
                break;
			case 0xc6:
                sprintf(buffer, FORMAT"%s, %s", "faddp", "st6", "st0");
                mark_instruction();
                // push next instruction offset to stack
                IDisasm.pushAddress(&current_va);
                break;
            case 0xc9:
                sprintf(buffer, FORMAT"%s, %s", "fmulp", "st1", "st0");
                mark_instruction();
                // push next instruction offset to stack
                IDisasm.pushAddress(&current_va);
                break;
            case 0xca:
                sprintf(buffer, FORMAT"%s, %s", "fmulp", "st2", "st0");
                mark_instruction();
                // push next instruction offset to stack
                IDisasm.pushAddress(&current_va);
                break;
            case 0xcb:
                sprintf(buffer, FORMAT"%s, %s", "fmulp", "st3", "st0");
                mark_instruction();
                // push next instruction offset to stack
                IDisasm.pushAddress(&current_va);
                break;
			case 0xcc:
                sprintf(buffer, FORMAT"%s, %s", "fmulp", "st4", "st0");
                mark_instruction();
                // push next instruction offset to stack
                IDisasm.pushAddress(&current_va);
                break;
			case 0xcd:
                sprintf(buffer, FORMAT"%s, %s", "fmulp", "st5", "st0");
                mark_instruction();
                // push next instruction offset to stack
                IDisasm.pushAddress(&current_va);
                break;
			case 0xce:
                sprintf(buffer, FORMAT"%s, %s", "fmulp", "st6", "st0");
                mark_instruction();
                // push next instruction offset to stack
                IDisasm.pushAddress(&current_va);
                break;
			case 0xd9:
                sprintf(buffer, FORMAT, "fcompp");
                mark_instruction();
                // push next instruction offset to stack
                IDisasm.pushAddress(&current_va);
                break;
			case 0xe1:
                sprintf(buffer, FORMAT"%s, %s", "fsubrp", "st1", "st0");
                mark_instruction();
                // push next instruction offset to stack
                IDisasm.pushAddress(&current_va);
                break;
			case 0xe2:
                sprintf(buffer, FORMAT"%s, %s", "fsubrp", "st2", "st0");
                mark_instruction();
                // push next instruction offset to stack
                IDisasm.pushAddress(&current_va);
                break;
			case 0xe9:
                sprintf(buffer, FORMAT"%s, %s", "fsubp", "st1", "st0");
                mark_instruction();
                // push next instruction offset to stack
                IDisasm.pushAddress(&current_va);
                break;
            case 0xea:
                sprintf(buffer, FORMAT"%s, %s", "fsubp", "st2", "st0");
                mark_instruction();
                // push next instruction offset to stack
                IDisasm.pushAddress(&current_va);
                break;
			case 0xeb:
                sprintf(buffer, FORMAT"%s, %s", "fsubp", "st3", "st0");
                mark_instruction();
                // push next instruction offset to stack
                IDisasm.pushAddress(&current_va);
                break;
            case 0xed:
                sprintf(buffer, FORMAT"%s, %s", "fsubp", "st5", "st0");
                mark_instruction();
                // push next instruction offset to stack
                IDisasm.pushAddress(&current_va);
                break;
            case 0xf0:
                sprintf(buffer, FORMAT"%s, %s", "fdivrp", "st0", "st0");
                mark_instruction();
                // push next instruction offset to stack
                IDisasm.pushAddress(&current_va);
                break;
			case 0xf1:
                sprintf(buffer, FORMAT"%s, %s", "fdivrp", "st1", "st0");
                mark_instruction();
                // push next instruction offset to stack
                IDisasm.pushAddress(&current_va);
                break;
            case 0xf2:
                sprintf(buffer, FORMAT"%s, %s", "fdivrp", "st2", "st0");
                mark_instruction();
                // push next instruction offset to stack
                IDisasm.pushAddress(&current_va);
                break;
            case 0xf3:
                sprintf(buffer, FORMAT"%s, %s", "fdivrp", "st3", "st0");
                mark_instruction();
                // push next instruction offset to stack
                IDisasm.pushAddress(&current_va);
                break;
			case 0xf4:
                sprintf(buffer, FORMAT"%s, %s", "fdivrp", "st4", "st0");
                mark_instruction();
                // push next instruction offset to stack
                IDisasm.pushAddress(&current_va);
                break;
            case 0xf8:
                sprintf(buffer, FORMAT"%s, %s", "fdivp", "st0", "st0");
                mark_instruction();
                // push next instruction offset to stack
                IDisasm.pushAddress(&current_va);
                break;
            case 0xf9:
                sprintf(buffer, FORMAT"%s, %s", "fdivp", "st1", "st0");
                mark_instruction();
                // push next instruction offset to stack
                IDisasm.pushAddress(&current_va);
                break;
            default:
                goto l_def;
            }   
            break;
        case 0xdf:
            address_mode = read_byte();
            if(address_mode == 0x7c){
                sprintf(buffer, FORMAT"qword %s", "fistp", mod_rm());
                mark_instruction();
                // push next instruction offset to stack
                IDisasm.pushAddress(&current_va);
                break;
            }
			if(address_mode == 0xe0){
                sprintf(buffer, FORMAT"%s", "fnstsw", "ax");
                mark_instruction();
                // push next instruction offset to stack
                IDisasm.pushAddress(&current_va);
                break;
            }
            if(((address_mode >> 3) & 7) == 0){
                sprintf(buffer, FORMAT"word %s", "fild", mod_rm());
                mark_instruction();
                // push next instruction offset to stack
                IDisasm.pushAddress(&current_va);
            }
            else goto l_def;
            break;
        case 0xe4:  // IN ib
            ib = read_byte();
            sprintf(buffer, FORMAT"%s, %s", "in", "al", hexbyte(ib));
            mark_instruction();
            // push next instruction offset to stack
            IDisasm.pushAddress(&current_va);
            break;
        case 0xe0:  // 
            displacement_8 = read_byte();
            mark_instruction();
            // get jmp instruction offset
            IDisasm.pushAddress(&current_va);
            current_va.offset += displacement_8;
            // set label for jmp
            le_createLabel(current_va.obj_n, current_va.offset);
            sprintf(buffer, FORMAT"%s", "loopne", getLabel(current_va.obj_n, current_va.offset));
            IDisasm.pushAddress(&current_va);
            break;
        case 0xe2:  // 
            displacement_8 = read_byte();
            mark_instruction();
            // get jmp instruction offset
            IDisasm.pushAddress(&current_va);
            current_va.offset += displacement_8;
            // set label for jmp
            le_createLabel(current_va.obj_n, current_va.offset);
            sprintf(buffer, FORMAT"%s", "loop", getLabel(current_va.obj_n, current_va.offset));
            IDisasm.pushAddress(&current_va);
            break;
        case 0xe3:  // LOOP signExt 8
            displacement_8 = read_byte();
            mark_instruction();
            // get jmp instruction offset
            IDisasm.pushAddress(&current_va);
            current_va.offset += displacement_8;
            // set label for jmp
            le_createLabel(current_va.obj_n, current_va.offset);
            sprintf(buffer, FORMAT"%s", "jecxz", getLabel(current_va.obj_n, current_va.offset));
            IDisasm.pushAddress(&current_va);
            break;
        case 0xe6:  // OUT ib, al
            ib = read_byte();
            sprintf(buffer, FORMAT"%s, %s", "out", hexbyte(ib), "al");
            mark_instruction();
            IDisasm.pushAddress(&current_va);
            break;
        case 0xe8:  // CALL signExt 32
            fx = le_checkFixup(current_va.obj_n, current_va.offset);
            displacement_32 = read_dword();
            mark_instruction();
			if(!le_checkLabel(current_va.obj_n, current_va.offset)){
               	IDisasm.pushAddress(&current_va);
			}
            if(fx&&(fx->type == 8)){
                current_va.obj_n = fx->object_n;
                current_va.offset = fx->target;
            }
            else {
                current_va.offset += displacement_32;
            }
            le_createLabel(current_va.obj_n, current_va.offset);
            sprintf(buffer, FORMAT"near %s", "call", getLabel(current_va.obj_n, current_va.offset));
            IDisasm.pushAddress(&current_va);
            break;
        case 0xe9:  // JMP signExt 16/32
            if(address_size == 32){
                displacement_32 = read_dword();
                mark_instruction();
                // get jmp instruction offset
                current_va.offset += displacement_32;
                // set label for jmp
                le_createLabel(current_va.obj_n, current_va.offset);
                sprintf(buffer, FORMAT"near %s", "jmp", getLabel(current_va.obj_n, current_va.offset));
            } 
            else if(address_size == 16){
                displacement_16 = read_word();
                mark_instruction();
                // get jmp instruction offset
                current_va.offset += displacement_16;
                // set label for jmp
                le_createLabel(current_va.obj_n, current_va.offset);
                sprintf(buffer, FORMAT"near %s", "jmp", getLabel(current_va.obj_n, current_va.offset));
            }
            else goto l_def;
            IDisasm.pushAddress(&current_va);
            break;
        case 0xeb:  // JMP signExt 8
            displacement_8 = read_byte();
            mark_instruction();
            // get jmp instruction offset
            current_va.offset += displacement_8;
            // set label for jmp
            le_createLabel(current_va.obj_n, current_va.offset);
            sprintf(buffer, FORMAT"short %s", "jmp",
                getLabel(current_va.obj_n, current_va.offset));
            IDisasm.pushAddress(&current_va);
                break;
        case 0xec:  // IN al, dx
            sprintf(buffer, FORMAT"%s, %s", "in", "al", "dx");
            mark_instruction();
            IDisasm.pushAddress(&current_va);
            break;
        case 0xed:  // IN eax, dx
            if(prefix_66){
                sprintf(buffer, FORMAT"%s, %s", "in", "ax", "dx");
            }
            else {
                sprintf(buffer, FORMAT"%s, %s", "in", "ax", "edx");
            }
            mark_instruction();
            IDisasm.pushAddress(&current_va);
            break;
        case 0xee:  // OUT dx, al
            sprintf(buffer, FORMAT"%s, %s", "out", "dx", "al");
            mark_instruction();
            IDisasm.pushAddress(&current_va);
            break;
        case 0xef:  // OUT dx, eax
            if(prefix_66){
                sprintf(buffer, FORMAT"%s, %s", "out", "dx", "ax");
            }
            else {
                sprintf(buffer, FORMAT"%s, %s", "out", "dx", "eax");
            }
            mark_instruction();
            IDisasm.pushAddress(&current_va);
            break;
        case 0xf2:
            rep_prefix = "repne ";
            goto switch_start;
        case 0xf3:
            rep_prefix = "repe ";
            goto switch_start;
        case 0xf5:  // CMC
            sprintf(buffer, FORMAT, "cmc");
            mark_instruction();
            IDisasm.pushAddress(&current_va);
            break;
        case 0xf6:
            if(!op_f6()) goto l_def;
            break;
        case 0xf7:
            if(!op_f7()) goto l_def;
            break;
        case 0xf8:  // CLC
            sprintf(buffer, FORMAT, "clc");
            mark_instruction();
            IDisasm.pushAddress(&current_va);
            break;
        case 0xf9:  // STC
            sprintf(buffer, FORMAT, "stc");
            mark_instruction();
            IDisasm.pushAddress(&current_va);
            break;
        case 0xfa:  // CLI
            sprintf(buffer, FORMAT, "cli");
            mark_instruction();
            IDisasm.pushAddress(&current_va);
            break;
        case 0xfb:  // STI
            sprintf(buffer, FORMAT, "sti");
            mark_instruction();
            IDisasm.pushAddress(&current_va);
            break;
        case 0xfc:  // CLD
            sprintf(buffer, FORMAT, "cld");
            mark_instruction();
            IDisasm.pushAddress(&current_va);
            break;
        case 0xfd:  // STDD
            sprintf(buffer, FORMAT, "std");
            mark_instruction();
            IDisasm.pushAddress(&current_va);
            break;
        case 0xfe:
            if(prefix_66) goto l_def;
            switch(read_byte()){
            case 0x03:
                address_mode = 0x03;
                sprintf(buffer, FORMAT"byte %s",
                    "inc", mod_rm());
                mark_instruction();
                // push next instruction offset to stack
                IDisasm.pushAddress(&current_va);
                break;
            case 0x05:
                address_mode = 0x05;
                sprintf(buffer, FORMAT"byte %s",
                    "inc", mod_rm());
                mark_instruction();
                // push next instruction offset to stack
                IDisasm.pushAddress(&current_va);
                break;
            case 0x0d:
                address_mode = 0x0d;
                sprintf(buffer, FORMAT"byte %s",
                    "dec", mod_rm());
                mark_instruction();
                // push next instruction offset to stack
                IDisasm.pushAddress(&current_va);
                break;
            case 0x40:
                address_mode = 0x40;
                sprintf(buffer, FORMAT"byte %s",
                    "inc", mod_rm());
                mark_instruction();
                // push next instruction offset to stack
                IDisasm.pushAddress(&current_va);
                break;
            case 0x48:
                address_mode = 0x48;
                sprintf(buffer, FORMAT"byte %s",
                    "dec", mod_rm());
                mark_instruction();
                // push next instruction offset to stack
                IDisasm.pushAddress(&current_va);
                break;
            case 0x80:
                address_mode = 0x80;
                sprintf(buffer, FORMAT"byte %s",
                    "inc", mod_rm());
                mark_instruction();
                // push next instruction offset to stack
                IDisasm.pushAddress(&current_va);
                break;
            case 0x82:
                address_mode = 0x82;
                sprintf(buffer, FORMAT"byte %s",
                    "inc", mod_rm());
                mark_instruction();
                // push next instruction offset to stack
                IDisasm.pushAddress(&current_va);
                break;
            case 0x88:
                address_mode = 0x88;
                sprintf(buffer, FORMAT"byte %s",
                    "dec", mod_rm());
                mark_instruction();
                // push next instruction offset to stack
                IDisasm.pushAddress(&current_va);
                break;
            case 0xc0:
                sprintf(buffer, FORMAT"%s", "inc", "al");
                mark_instruction();
                // push next instruction offset to stack
                IDisasm.pushAddress(&current_va);
                break;
            case 0xc1:
                sprintf(buffer, FORMAT"%s", "inc", "cl");
                mark_instruction();
                // push next instruction offset to stack
                IDisasm.pushAddress(&current_va);
                break;
            case 0xc2:
                sprintf(buffer, FORMAT"%s", "inc", "dl");
                mark_instruction();
                // push next instruction offset to stack
                IDisasm.pushAddress(&current_va);
                break;
            case 0xc3:
                sprintf(buffer, FORMAT"%s", "inc", "bl");
                mark_instruction();
                // push next instruction offset to stack
                IDisasm.pushAddress(&current_va);
                break;
            case 0xc4:
                sprintf(buffer, FORMAT"%s", "inc", "ah");
                mark_instruction();
                // push next instruction offset to stack
                IDisasm.pushAddress(&current_va);
                break;
            case 0xc5:
                sprintf(buffer, FORMAT"%s", "inc", "ch");
                mark_instruction();
                // push next instruction offset to stack
                IDisasm.pushAddress(&current_va);
                break;
            case 0xc6:
                sprintf(buffer, FORMAT"%s", "inc", "dh");
                mark_instruction();
                // push next instruction offset to stack
                IDisasm.pushAddress(&current_va);
                break;
            case 0xc7:
                sprintf(buffer, FORMAT"%s", "inc", "bh");
                mark_instruction();
                // push next instruction offset to stack
                IDisasm.pushAddress(&current_va);
                break;
            case 0xc8:
                sprintf(buffer, FORMAT"%s", "dec", "al");
                mark_instruction();
                // push next instruction offset to stack
                IDisasm.pushAddress(&current_va);
                break;
            case 0xc9:
                sprintf(buffer, FORMAT"%s", "dec", "cl");
                mark_instruction();
                // push next instruction offset to stack
                IDisasm.pushAddress(&current_va);
                break;
            case 0xca:
                sprintf(buffer, FORMAT"%s", "dec", "dl");
                mark_instruction();
                // push next instruction offset to stack
                IDisasm.pushAddress(&current_va);
                break;
			case 0xcb:
                sprintf(buffer, FORMAT"%s", "dec", "bl");
                mark_instruction();
                // push next instruction offset to stack
                IDisasm.pushAddress(&current_va);
                break;
            case 0xcc:
                sprintf(buffer, FORMAT"%s", "dec", "ah");
                mark_instruction();
                // push next instruction offset to stack
                IDisasm.pushAddress(&current_va);
                break;
            case 0xcd:
                sprintf(buffer, FORMAT"%s", "dec", "ch");
                mark_instruction();
                // push next instruction offset to stack
                IDisasm.pushAddress(&current_va);
                break;
            case 0xcf:
                sprintf(buffer, FORMAT"%s", "dec", "bh");
                mark_instruction();
                // push next instruction offset to stack
                IDisasm.pushAddress(&current_va);
                break;
            default:
                goto l_def;
            }
            break;
        case 0xff:
            if(!op_ff()) goto l_def;
            break;
        default:
        l_def:
            printf("[DISASM] Dont know how to disassemble next instruction ...\n");
            dump15();
            continue;
        }

        str_size = strlen(buffer);

        fseek(IDisasm.fd, disasm_ptr, SEEK_SET);
        fwrite(&str_size, 1, 1, IDisasm.fd);
        fwrite(buffer, str_size, 1, IDisasm.fd);
        disasm_ptr += 1 + str_size;
    }
#undef FORMAT
}