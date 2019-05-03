#include"bin2inc.h"

boolean op_89(void){

    address_mode = read_byte();

    sprintf(ibuffer, iFORMAT"%s, %s", "mov", mod_rm(), mod_reg());

    mark_instruction();// comment_instruction();
    // push next instruction offset to stack
    IDisasm.pushAddress(&current_va);

    return boolean(1);
}