#include"bin2inc.h"

boolean op_d1(void){

    char * mod_rm_str;
    char * o_size;
    char * mnems[8] = { "rol", "ror", "rcl", "rcr", "shl", "shr", 0, "sar" };
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

    sprintf(ibuffer, iFORMAT"%s%s, %s",
        mnemonic, o_size, mod_rm_str, hexbyte(1));

    mark_instruction();// comment_instruction();
    // push next instruction offset to stack
    IDisasm.pushAddress(&current_va);

    return boolean(1);
}