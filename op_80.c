#include"bin2inc.h"

boolean op_80(void){

    byte ib;
    char * mod_rm_str;
    char * o_size;
    char * mnems[8] = { "add", "or", "adc", 0, "and", "sub", "xor", "cmp" };
    char * mnemonic;

    operand_size = 8;
    
    address_mode = read_byte();
    o_size = (((address_mode >> 6) & 3) == 3) ? "" : "byte "; 
    mod_rm_str = mod_rm();
    ib = read_byte();
         
    mnemonic = mnems[(address_mode >> 3) & 7];

    if(!mnemonic) return boolean(0);

    sprintf(ibuffer, iFORMAT"%s%s, %s",
        mnemonic, o_size, mod_rm_str, hexbyte(ib));

    mark_instruction();// comment_instruction();
    // push next instruction offset to stack
    IDisasm.pushAddress(&current_va);

    return boolean(1);
}