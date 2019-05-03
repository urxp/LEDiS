#include"bin2inc.h"

boolean op_83(void){

    char * mod_rm_str;
    char * o_size;
    char * mnems[8] = { "add", "or", "adc", "sbb", "and", "sub", 0, "cmp" };
    char * mnemonic;
    char * imm_str;
    
    address_mode = read_byte();
    mod_rm_str = mod_rm();

    if(operand_size == 32){
        o_size = "dword "; 
        imm_str = hexbyte((signed_byte)read_byte());
    }
    else if(operand_size == 16){
        o_size = "word "; 
        imm_str = hexbyte(0xffff & (signed_byte)read_byte());
    }
    else return boolean(0);
         
    mnemonic = mnems[(address_mode >> 3) & 7];

    if(!mnemonic) return boolean(0);

    if(((address_mode >> 6) & 3) == 3) o_size = "";

    sprintf(ibuffer, iFORMAT"%s%s, byte %s",
        mnemonic, o_size, mod_rm_str, imm_str);

    mark_instruction();// comment_instruction();
    // push next instruction offset to stack
    IDisasm.pushAddress(&current_va);

    return boolean(1);
}