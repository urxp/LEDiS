#include"bin2inc.h"

char * addr16[8] = { "bx+si", "bx+di", "bp+si", "bp+di", "si", "di", "bp", "bx" };


char * sregs[8] = { "es", "cs", "ss", "ds", "fs", "gs", 0, 0 };
char * regs_8[8] = { "al", "cl", "dl", "bl", "ah", "ch", "dh", "bh" };
char * regs_16[8] = { "ax", "cx", "dx", "bx", "sp", "bp", "si", "di" };
char * regs_32[8] = { "eax", "ecx", "edx", "ebx", "esp", "ebp", "esi", "edi" };

char am_buffer[256];
char sib_buffer[256];
char * tmp_str_2;

char * seg_def;

char * getOperandSizeString(){

    if(operand_size == 8) return "byte";
    if(operand_size == 16) return "word";
    if(operand_size == 32) return "dword";

    return (void *)0;
}

char * getRegister(byte reg){

    if(operand_size == 8) return regs_8[reg];
    if(operand_size == 16) return regs_16[reg];
    if(operand_size == 32) return regs_32[reg];

    return (void *)0;
}

char * getSegRegister(byte reg){

    return (reg<8) ? sregs[reg]: (void *)0;
}

byte mod_mod(void){

    return (address_mode&0xc0) >> 6;
}

char * get_SIB(){ // FIX

    byte SIB = read_byte();
    seg_def = seg_ss;

    if((SIB&0x07) == 5){

        if(mod_mod()==0){
            if(((SIB&0x38)>>3) != 4){

                seg_def = seg_ds;

                fx = le_checkFixup(current_va.obj_n, current_va.offset);
                displacement_32 = read_dword();
                tmp_str = ((fx->type == 7)&&(fx->size == 4))
                    ? getLabel(fx->object_n, fx->target)
                    : hexbyte(displacement_32);

                if((SIB>>6)==1){
                    sprintf(sib_buffer, "NoSplit %s*%d+%s", 
                        regs_32[(SIB&0x38)>>3], 1 << (SIB>>6), tmp_str);
                }
                else {
                    sprintf(sib_buffer, "%s*%d+%s", 
                        regs_32[(SIB&0x38)>>3], 1 << (SIB>>6), tmp_str);
                }

                return sib_buffer;
            }
        }
        if(mod_mod()){
            if(((SIB&0x38)>>3) != 4){

                sprintf(sib_buffer, "ebp+%s*%d", 
                    regs_32[(SIB&0x38)>>3], 1 << (SIB>>6));

                return sib_buffer;
            }
        }
    }
    else {

        if((SIB&0x07) != 4){
            // BASE != esp
            seg_def = seg_ds;
        }

        if(((SIB&0x38)>>3) != 4){
            // INDEX != esp

            sprintf(sib_buffer, "%s+%s*%d", 
                regs_32[SIB&0x07], regs_32[(SIB&0x38)>>3], 1 << (SIB>>6));  
            
            return sib_buffer;
        }
        else{

            sprintf(sib_buffer, "%s", regs_32[SIB&0x07]);

            return sib_buffer;
        }
    }

    return (void *)0;
}


char * mod_00(void){

    seg_def = seg_ds;

    if(address_size == 32){

        if((address_mode&0x07) == 4){
            
            tmp_str = get_SIB();
        }
        else if((address_mode&0x07) == 5){
            
            fx = le_checkFixup(current_va.obj_n, current_va.offset);
            displacement_32 = read_dword();
            tmp_str = ((fx->type == 7)&&(fx->size == 4))
                ? getLabel(fx->object_n, fx->target)
                : hexbyte(displacement_32);
        }
        else {    
            tmp_str = regs_32[address_mode&0x07];
        }

        sprintf(am_buffer, "[%s%s]", seg_def, tmp_str);

        return am_buffer;
    }
    else if(address_size == 16){

        if(((address_mode&7) == 2)||((address_mode&7) == 3)){
            seg_def = seg_ss;
        }

        if((address_mode&0x07) == 6){
            
            //fx = le_checkFixup(current_va.obj_n, current_va.offset);
            displacement_16 = read_word();
            //tmp_str = ((fx->type == 7)&&(fx->size == 4))
            //    ? getLabel(fx->object_n, fx->target)
            //    : hexbyte(displacement_16);
            tmp_str = hexbyte(displacement_16);
        }
        else {    
            tmp_str = addr16[address_mode&0x07];
        }

        sprintf(am_buffer, "[%s%s]", seg_def, tmp_str);

        return am_buffer;
    }

    return (void *)0;
}

char * mod_01(void){

    seg_def = seg_ds;

    if(address_size == 32){
        
        if((address_mode&0x07) != 4){

            if((address_mode&0x07) == 5) seg_def = seg_ss;

            tmp_str = regs_32[address_mode&0x07];
        }
        else {
            tmp_str = get_SIB();
        }

        displacement_8 = read_byte();
        if(displacement_8 < 0){
            sprintf(am_buffer, "[%s%s-%s]", 
                seg_def, tmp_str, hexbyte(-displacement_8));
        }
        else {
            sprintf(am_buffer, "[%s%s+%s]", 
                seg_def, tmp_str, hexbyte(displacement_8));
        }

        return am_buffer;
    }
    else if(address_size == 16){

        if(((address_mode&7) == 2)
        ||((address_mode&7) == 3)
        ||((address_mode&7) == 6)){
            seg_def = seg_ss;
        }

        displacement_8 = read_byte();
        tmp_str = addr16[address_mode&0x07];
   
        sprintf(am_buffer, "[%s%s+%s]", seg_def, tmp_str, hexbyte(displacement_8));

        return am_buffer;
    }

    return (void *)0;
}

char * mod_10(void){

    seg_def = seg_ds;

    if(address_size == 32){
        
        if((address_mode&0x07) != 4){

            if((address_mode&0x07) == 5) seg_def = seg_ss;

            tmp_str_2 = regs_32[address_mode&0x07];
        }
        else {
            tmp_str_2 = get_SIB();
        }

        fx = le_checkFixup(current_va.obj_n, current_va.offset);
        displacement_32 = read_dword();
        tmp_str = ((fx->type == 7)&&(fx->size == 4))
            ? getLabel(fx->object_n, fx->target)
            : hexbyte(displacement_32);
        sprintf(am_buffer, "[%s%s+%s]", seg_def, tmp_str_2, tmp_str);

        return am_buffer;
    }
    else if(address_size == 16){

        if(((address_mode&7) == 2)
        ||((address_mode&7) == 3)
        ||((address_mode&7) == 6)){
            seg_def = seg_ss;
        }

        displacement_16 = read_word();
        tmp_str = addr16[address_mode&0x07];
   
        sprintf(am_buffer, "[%s%s+%s]", seg_def, tmp_str, hexbyte(displacement_16));

        return am_buffer;
    }

    return (void *)0;
}

char * mod_11(void){

    return getRegister(address_mode&0x07);
}



char * mod_rm(void){

    switch(mod_mod()){
    case 0x0:
        return mod_00();
    case 0x1:
        return mod_01();
    case 0x2:
        return mod_10();
    case 0x3:
        return mod_11();
    default:
        printf("[DISASM] %s %s %d\n", __FILE__, __FUNCTION__, __LINE__);
        printf("[DISASM] err you shouldn't reach this state!\n");
    }

    return (void *)0;
}

char * mod_reg(void){

    return getRegister((address_mode&0x38) >> 3);
}

char * mod_sreg(void){

    return getSegRegister((address_mode&0x38) >> 3);
}