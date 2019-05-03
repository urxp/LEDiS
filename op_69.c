#include"bin2inc.h"

boolean op_69(void){

    char * reg_str;
    char * rm_str;
    char * imm_str;

    address_mode = read_byte();

    reg_str = mod_reg();
    rm_str = mod_rm();

    fx = le_checkFixup(current_va.obj_n, current_va.offset);
    if((fx->type ==7)&&(fx->size == 4)){
        read_dword();
        imm_str = getLabel(fx->object_n, fx->target);
    }
    else if(fx->size == 0){ // no fixup
        if(operand_size == 32){
            imm_str = hexbyte(read_dword());
        }
        else if(operand_size == 16){
            imm_str = hexbyte(read_word());
        }
        else return boolean(0);
    }
    else return boolean(0);

    sprintf(ibuffer, iFORMAT"%s, %s, %s", "imul", reg_str, rm_str, imm_str);

    mark_instruction();// comment_instruction();
    // push next instruction offset to stack
    IDisasm.pushAddress(&current_va);

    return boolean(1);
}