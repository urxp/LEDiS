#include"bin2inc.h"

boolean op_6b(void){

    char * reg_str;
    char * rm_str;
    char * imm_str;

    address_mode = read_byte();

    reg_str = mod_reg();
    rm_str = mod_rm();
    if(operand_size == 32){
        imm_str = hexbyte((signed_byte)read_byte());
    }
    else if(operand_size == 16){
        imm_str = hexbyte(0xffff & (signed_byte)read_byte());
    }
    else return boolean(0);

    sprintf(ibuffer, iFORMAT"%s, %s, byte %s", "imul", reg_str, rm_str, imm_str);

    mark_instruction();// comment_instruction();
    // push next instruction offset to stack
    IDisasm.pushAddress(&current_va);

    return boolean(1);
}