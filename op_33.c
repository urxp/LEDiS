#include"bin2inc.h"

boolean op_33(void){

    address_mode = read_byte();

    sprintf(ibuffer, iFORMAT"%s, %s", "xor", mod_reg(), mod_rm());

    mark_instruction();
    // push next instruction offset to stack
    IDisasm.pushAddress(&current_va);

    return boolean(1);
}