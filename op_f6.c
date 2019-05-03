#include"bin2inc.h"

boolean op_f6(void){

    byte ib;
    char * mod_rm_str;
    char * o_size;
    char * mnems[8] = { "test", 0, "not", "neg", "mul", "imul", "div", "idiv" };
    char * mnemonic;

    operand_size = 8;
    
    address_mode = read_byte();
    o_size = (((address_mode >> 6) & 3) == 3) ? "" : "byte "; 
    mod_rm_str = mod_rm();
    
         
    mnemonic = mnems[(address_mode >> 3) & 7];

    if(!mnemonic) return boolean(0);

    switch((address_mode >> 3)&7){
    case 2:
    case 3:
    case 4:
    case 5:
    case 6:
    case 7:
        sprintf(ibuffer, iFORMAT"%s%s",
            mnemonic, o_size, mod_rm_str);
        break;
    case 0:
        ib = read_byte();
        sprintf(ibuffer, iFORMAT"%s%s, %s",
            mnemonic, o_size, mod_rm_str, hexbyte(ib));
        break;
    default:
        return boolean(0);
    }

    mark_instruction();// comment_instruction();
    // push next instruction offset to stack
    IDisasm.pushAddress(&current_va);

    return boolean(1);
}