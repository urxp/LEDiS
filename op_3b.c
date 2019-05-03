#include"bin2inc.h"

boolean op_3b(void){

    address_mode = read_byte();

    sprintf(ibuffer, iFORMAT"%s, %s", "cmp", mod_reg(), mod_rm());

    mark_instruction();
    // push next instruction offset to stack
    IDisasm.pushAddress(&current_va);

    return boolean(1);
}