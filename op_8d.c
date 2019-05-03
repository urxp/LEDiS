#include"bin2inc.h"


boolean op_8d(void){

    address_mode = read_byte();

    if(address_mode == 0x40){

        if(read_byte() == 0){
            // lea eax, [eax + 0]
            sprintf(ibuffer, "align %s\n;%s",
                hexbyte(getAlignment(current_va.offset)),
                "aligned with dummy instruction lea eax, [eax + 0]");
            
            comment_instruction();
            goto l_end;
        }

        current_va.offset--;       
    }
    
    sprintf(ibuffer, iFORMAT"%s, %s", "lea", mod_reg(), mod_rm());

l_end:
    mark_instruction();
    // push next instruction offset to stack
    IDisasm.pushAddress(&current_va);

    return boolean(1);
}