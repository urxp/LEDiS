#include"bin2inc.h"

boolean op_ff(void){

    char * mod_rm_str;
    char * o_size;
    char * mnems[8] = { "inc", "dec", "call", "call", "jmp", "jmp", "push", 0 };
    char * mnemonic;
    char * imm_str;
    
    address_mode = read_byte();
    mnemonic = mnems[(address_mode >> 3) & 7];
    if(!mnemonic) return boolean(0);
    mod_rm_str = mod_rm();

    
    if(operand_size == 32){
        o_size = "dword "; 
    }
    else if(operand_size == 16){
        o_size = "word "; 
    }
    else return boolean(0);

    if(((address_mode >> 6) & 3) == 3) o_size = "";

    switch((address_mode >> 3)&7){
    case 2:
        if(fx && (fx->size == 4)){
            pushJumpTable(&(VirtualAddress){
                .obj_n = fx->object_n,
                .offset = fx->target
            });
        }

        sprintf(ibuffer, iFORMAT"%s%s",
            mnemonic, o_size, mod_rm_str);
        break;
    case 3:
        sprintf(ibuffer, iFORMAT"far %sfar %s",
            mnemonic, o_size, mod_rm_str);
        break;
    case 4:
        if(fx && (fx->size == 4)){
            pushJumpTable(&(VirtualAddress){
                .obj_n = fx->object_n,
                .offset = fx->target
            });
        }

        sprintf(ibuffer, iFORMAT"near %s%s",
            mnemonic, o_size, mod_rm_str);
        mark_instruction();// comment_instruction();
        return boolean(1);
    case 5:
        sprintf(ibuffer, iFORMAT"far %sfar %s",
            mnemonic, o_size, mod_rm_str);
        mark_instruction();// comment_instruction();
        return boolean(1);
    case 0:
    case 1:
    case 6:
        sprintf(ibuffer, iFORMAT"%s%s",
            mnemonic, o_size, mod_rm_str);
        break;
    default:
        return boolean(0);
    }

    mark_instruction();// comment_instruction();
    // push next instruction offset to stack
    IDisasm.pushAddress(&current_va);

    return boolean(1);
}