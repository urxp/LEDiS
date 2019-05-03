#include"bin2inc.h"

boolean op_66(void){

    prefix_66 = !prefix_66;

    operand_size = operand_size ^ (16|32);

    return boolean(1);
}