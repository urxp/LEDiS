#include"bin2inc.h"

boolean op_00(void){

    operand_size = 8;

    address_mode = read_byte();

    sprintf(ibuffer, iFORMAT"%s, %s", "add", mod_rm(), mod_reg());
    mark_instruction();// comment_instruction();
    IDisasm.pushAddress(&current_va);

    return boolean(1);
}