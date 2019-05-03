#include<stdio.h>
#include<stdlib.h>


#define __ARR(args...)       (byte[]){ args }           
#define __386_CODE(i...)    \
    i386code(sizeof(__ARR(i)), __ARR(i))


#define BYTE_BITS           8
#define BYTE_MASK           0xFF    
#define IS_BYTE(v)          (!((v) & ~BYTE_MASK))
#define IS_SIGNED_BYTE(v)   (IS_BYTE(v) && ((v) >> (BYTE_BITS - 1)))
#define SIGNEX16(v)         ((v) | (IS_SIGNED_BYTE(v) * ~BYTE_MASK))


#define SEG_MAX         0x10000
#define seg_cs(T)       *(T *)&((byte *)physicalAddress(seg_cs))
#define seg_ds(T)       *(T *)&((byte *)physicalAddress(seg_ds))
#define seg_es(T)       *(T *)&((byte *)physicalAddress(seg_es))
#define seg_ss(T)       *(T *)&((byte *)physicalAddress(seg_ss))

typedef unsigned char   byte;
typedef unsigned short  word;
typedef word            r16;
typedef word            seg;


#define al     ((byte *)&ax)[0]
#define ah     ((byte *)&ax)[1]
#define bl     ((byte *)&bx)[0]
#define bh     ((byte *)&bx)[1]
#define cl     ((byte *)&cx)[0]
#define ch     ((byte *)&cx)[1]
#define dl     ((byte *)&dx)[0]
#define dh     ((byte *)&dx)[1]

seg program_segment_prefix, program_segment;
seg seg_cs, seg_ds, seg_es, seg_ss;
r16 ax, bx, cx, dx, si, di, bp, sp, ip;
byte flag_cf, flag_df, flag_if, flag_zf;


void * physicalAddress(seg);
void setCodeSize(size_t);


void * memory;
size_t m_max;
size_t m_min;
size_t m_alloc;


byte * segments[64];

size_t i_size;

byte * i386code(size_t n, byte * i){

    setCodeSize(n);

    return i;
}


size_t fsize(FILE * fd){

    size_t save_p = ftell(fd);
    size_t result;

    fseek(fd, 0, SEEK_END);
    result = ftell(fd);
    fseek(fd, save_p, SEEK_SET);

    return result;
}

int bin_fread(void * output_buffer, char * input_file_name){

    int result;
    FILE * bin = fopen(input_file_name, "rb");   
    result = fread(output_buffer, 1, fsize(bin), bin);
    fclose(bin);

    return result;
}

int bin_fwrite(void * input_buffer, size_t bytes, char * output_file_name){

    int result;
    FILE * bin = fopen(output_file_name, "wb");   
    result = fwrite(input_buffer, 1, bytes, bin);
    fclose(bin);

    return result;
}




void movsb(byte mode){

    byte p = (flag_df == 0) ? 1 : -1;

    if(mode == 16){

        seg_es(byte)[di] = seg_ds(byte)[si];
        di += p;
        si += p;
    }
}

void movsw(byte mode){

    byte p = (flag_df == 0) ? 2 : -2;

    if(mode == 16){

        seg_es(word)[di] = seg_ds(word)[si];
        di += p;
        si += p;
    }
}

void rep_movsw(byte mode){

    if(mode == 16){

        while(cx != 0){
            
            movsw(mode);
            cx--;
        }    
    }
}



void stosb(byte mode){

    byte p = (flag_df == 0) ? 1 : -1;

    if(mode == 16){

        seg_es(byte)[di] = al;
        di += p;
    }
}


void lodsb(byte mode){

    byte p = (flag_df == 0) ? 1 : -1;

    if(mode == 16){

        al = seg_ds(byte)[si];
        si += p;
    }
}

void lodsw(byte mode){

    byte p = (flag_df == 0) ? 2 : -2;

    if(mode == 16){

        ax = seg_ds(word)[si];
        si += p;
    }
}




size_t getCodeSize(){ return i_size; }
void setCodeSize(size_t s){ i_size = s; }


void halt386(){

    bin_fwrite(memory, m_alloc, "out.bin");
    printf("[386] Halts...\n");

    exit(EXIT_FAILURE);
}



void dump_15(){

    byte * ptr = physicalAddress(seg_cs) + ip;
    int n = 15;

    printf("[386]");
    while((ptr < (byte *)(memory + m_alloc)) && n--) printf(" %02X", *ptr++);
    printf("\n");
}


void emulate386(){

    size_t CodeSize = getCodeSize();

    
    byte * code = physicalAddress(seg_cs) + ip;

    if(code >= (byte *)(memory + m_alloc)){
        
        printf("[386] Out of memory range.\n");
        halt386();
    }
    
    switch(code[0]){
        case 0x01:
            if((code[1] == 0x16)&&(*(word*)(code + 2) == 0x15C)){

                seg_ds(word)[0x15C] += dx;
                ip += 4;
            }
            else goto l_default;
            break;
        case 0x06:  // push es
            sp -= 2;
            seg_ss(word)[sp] = seg_es;
            ++ip;
            break;
        case 0x07:  // pop es
            seg_es = seg_ss(word)[sp];
            sp += 2;
            ++ip;
            break;
        case 0x0E:  // push cs
            sp -= 2;
            seg_ss(word)[sp] = seg_cs;
            ++ip;
            break;
        case 0x16:  // push ss
            sp -= 2;
            seg_ss(word)[sp] = seg_ss;
            ++ip;
            break;
        case 0x1E:  // push ds
            sp -= 2;
            seg_ss(word)[sp] = seg_ds;
            ++ip;
            break;
        case 0x1F:  // pop ds
            seg_ds = seg_ss(word)[sp];
            sp += 2;
            ++ip;
            break;
        case 0x26:
            if(*(word *)(code + 1) == 0x18A){
                al = seg_es(byte)[(word)(bx+di)];
                ip += 3;
            }
            else if(*(word *)(code + 1) == 0x1701){
                seg_es(word)[bx] += dx;
                ip += 3;
            }
            else goto l_default;
            break;
        case 0x2E:
            if((*(word *)(code + 1) == 0x2EFF)&&(*(word *)(code + 3) == 0x15A)){

                ip = seg_cs(word)[0x15A];
                seg_cs = seg_cs(word)[0x15A + 0x2];
            }
            else goto l_default;
            break;
        case 0x32:
            if(code[1] == 0xC9){
                cl = 0;
                ip +=2;
            }
            else if(code[1] == 0xF6){
                dh = 0;
                ip +=2;
            }
            else goto l_default;
            break;
        case 0x33:
            if(code[1] == 0xDB){
                
                bx = 0;
                ip +=2;
            }
            else goto l_default;
            break;
        case 0x4B:
            bx--;
            ++ip;
            break;
        case 0x50:  // push ax
            sp -= 2;
            seg_ss(word)[sp] = ax;
            ++ip;
            break;
        case 0x51:  // push cx
            sp -= 2;
            seg_ss(word)[sp] = cx;
            ++ip;
            break;
        case 0x52:  // push dx
            sp -= 2;
            seg_ss(word)[sp] = dx;
            ++ip;
            break;
        case 0x53:  // push bx
            sp -= 2;
            seg_ss(word)[sp] = bx;
            ++ip;
            break;
        case 0x55:  // push bp
            sp -= 2;
            seg_ss(word)[sp] = bp;
            ++ip;
            break;
        case 0x56:  // push si
            sp -= 2;
            seg_ss(word)[sp] = si;
            ++ip;
            break;
        case 0x57:  // push di
            sp -= 2;
            seg_ss(word)[sp] = di;
            ++ip;
            break;
        case 0x5A:  // pop dx
            dx = seg_ss(word)[sp];
            sp += 2;
            ++ip;
            break;
        case 0x5F:  // pop di
            di = seg_ss(word)[sp];
            sp += 2;
            ++ip;
            break;
        case 0x72:  // jb
            ip += 2;
            if(flag_cf == 1) ip += SIGNEX16(code[1]);
            break;
        case 0x73:  // jae
            ip += 2;
            if(flag_cf == 0) ip += SIGNEX16(code[1]);
            break;
        case 0x75:  // jne
            if(code[1] == 0x5){
                
                ip += 2;
                if(flag_zf == 0) ip += 5;
            }
            else goto l_default;
            break;
        case 0x80:
            if(*(word *)(code + 1) == 0x1D1){
                cl += (1 + flag_cf);
                ip += 3;
            }
            else if(*(word *)(code + 1) == 0x7C6){
                dh += 7;
                ip += 3;
            }
            else if(*(word *)(code + 1) == 0x1EB){
                flag_cf = !bl;
                bl -= 1;
                ip += 3;
            }
            else goto l_default;
            break;
        case 0x81:
            if((code[1] == 0xC2)&&(*(word*)(code + 2) == 0x3D7)){
                dx += 0x3D7;
                ip += 4;
            }
            else goto l_default;
            break;
        case 0x83:
            if(*(word *)(code + 1) == 0xFC1){
                cx += 0x0F;
                ip += 3;
            }
            else goto l_default;
            break;
        case 0x8A:
            if(code[1] == 0xC8){
                cl = al;
                ip += 2;    
            }
            else if(code[1] == 0xCE){
                cl = dh;
                ip += 2;    
            }
            else if(code[1] == 0xD8){
                bl = al;
                ip += 2;    
            }
            else goto l_default;
            break;
        case 0x8B:
            if(code[1] == 0xD8){
                bx = ax;
                ip += 2;
            }
            else if(code[1] == 0xE8){
                bp = ax;
                ip += 2;
            }
            else if(code[1] == 0xF7){
                si = di;
                ip += 2;    
            } 
            else goto l_default;
            break;
        case 0x8E:
            if(code[1] == 0xD2){
                seg_ss = dx;
                ip += 2;
            }
            else goto l_default;
            break;
        case 0x9A:
            if((*(word *)(code + 1) == 0x392)&&(*(word *)(code + 3) == program_segment)){
                sp -=2;
                seg_ss(word)[sp] = seg_cs;
                sp -=2;
                seg_ss(word)[sp] = ip;

                ip = 0x392;
                seg_cs = program_segment;
            }
            else goto l_default;
            break;
        case 0xA4:  // movsb
            movsb(16);
            ++ip;
            break;
        case 0xAA:
            stosb(16);
            ++ip;
            break;
        case 0xAC:  // lodsb
            lodsb(16);
            ++ip;
            break;
        case 0xAD:  // lodsw
            lodsw(16);
            ++ip;
            break;
        case 0xB1:
            if(code[1] == 2){
                cl = 2;
                ip += 2;
            }
            else if(code[1] == 3){
                cl = 3;
                ip += 2;
            }
            else if(code[1] == 4){
                cl = 4;
                ip += 2;
            }
            else if(code[1] == 0x1A){
                cl = 0x1A;
                ip += 2;
            }
            else goto l_default;
            break;
        case 0xB2:
            if(code[1] == 0x10){
                dl = 0x10;
                ip += 2;
            }
            else goto l_default;
            break;
        case 0xB6:
            if(code[1] == 2){
                dh = 2;
                ip += 2;
            }
            else goto l_default;
            break;
        case 0xB8:
            if(*(word *)(code + 1) == 0x1D9){
                ax = 0x1D9;
                ip += 3;
            }
            else goto l_default;
            break;
        case 0xB9: // mov cx, 165Eh
            if(*(word *)(code + 1) == 0x165E){
                cx = 0x165E;
                ip += 3;
            }
            else goto l_default;
            break;
        case 0xBC:
            if(*(word *)(code + 1) == 0x400){
                sp = 0x400;
                ip += 3;
            }
            else goto l_default;
            break;
        case 0xBE: // mov si, 222h
            if(*(word *)(code + 1) == 0x222){
                si = 0x222;
                ip += 3;
            }
            else goto l_default;
            break;
        case 0xBF: // mov di, 100h
            if(*(word *)(code + 1) == 0x100){
                di = 0x100;
                ip += 3;
            }
            else goto l_default;
            break;
        case 0xC3: // ret
            
            ip = seg_ss(word)[sp];
            sp += 2;
            break;
        case 0xCB:
            ip = seg_ss(word)[sp];
            sp += 2;
            seg_cs = seg_ss(word)[sp];
            sp += 2;
            break;
        case 0xD0:
            if(code[1] == 0xD6){
                dh <<=1;
                dh += flag_cf;
                ip += 2;
            }
            else if(code[1] == 0xD7){
                bh <<=1;
                bh += flag_cf;
                ip += 2;
            }
            else if(code[1] == 0xE1){
                cl <<=1;
                ip += 2;
            }
            else goto l_default;
            break;
        case 0xD1:
            if(code[1] == 0xED){
                
                flag_cf = bp & 1;
                bp >>= 1;
                
                ip += 2;
            }
            else goto l_default;
            break;
        case 0xE2:            
            ip += 2;
            if(--cx != 0) ip += SIGNEX16(code[1]);
            break;
        case 0xE8: // call
                ip += 3;
                sp -= 2;
                seg_ss(word)[sp] = ip; 
                ip += *(word *)(code + 1);
            break;
        case 0xEB: // jmp
            ip += 2;
            ip += SIGNEX16(code[1]);
            break;
        case 0xF3:
            if(code[1] == 0xA5){
                rep_movsw(16);
                ip += 2;
            }
            else goto l_default;
            break;
        case 0xF7:
            if(code[1] == 0xDB){
                bx = 0 - bx;
                ip += 2;
            }
            else goto l_default;
            break; 
        case 0xFA:
            flag_if = 0;
            ++ip;
            break;
        case 0xFB:
            flag_if = 1;
            ++ip;
            break;
        case 0xFC:  // cld
            flag_df = 0;
            ++ip;
            break;
        case 0xFE:
            if(code[1] == 0xC6){
                
                dh++;
                flag_zf = !dh;
                
                ip += 2;
            }
            else if(code[1] == 0xCA){
                
                dl--;
                flag_zf = !dl;
                
                ip += 2;
            }
            else goto l_default;
            break;    
            
            
            
            
        default:
        l_default:
            printf("[386] Don't know how to emulate instruction %X\n", code[0]);
            printf("[386] cs:ip = %04x:%04x\n", seg_cs, ip);
            dump_15();
            halt386();
    }
    
    setCodeSize(0);
}














void * physicalAddress(seg seg_sel){

    if(seg_sel < program_segment_prefix){

        printf("[386] ::: Invalid segment selector value %04Xh\n", seg_sel);
        exit(EXIT_FAILURE);
    }

    return memory + ((seg_sel - program_segment_prefix) << 4);
}









int main(int argc, char * argv[]){

    program_segment_prefix = 0x100;
    program_segment = program_segment_prefix + 0x10;

    seg_ds = program_segment_prefix;
    seg_cs = program_segment + 0;
    ip = 0x5E;
    seg_ss = program_segment + 0x3D7;
    sp = 0x100;


    m_max = 0x10 * seg_ss + sp + 0x10 * 0x06B4;
    m_min = 0x10 * program_segment_prefix;
    m_alloc = m_max - m_min;

    
    printf("[PMWU] Allocating %u bytes of memory...\n", m_alloc);
    memory = calloc(m_alloc, sizeof(byte));
    


    int bytes = bin_fread(physicalAddress(seg_cs), "pmwu.bin");









    printf("[PMWU] Preparing stub program to decompress...\n");
    while((seg_cs != seg_ss)&&(ip != 0x1D9)) emulate386();
    
    //printf("[386] cs:ip = %04x:%04x\n", seg_cs, ip);
    
    printf("[PMWU] Decompressing stub program...\n");
    while((seg_cs != (word)(program_segment + 0x21C))&&(ip != 0x47D)) emulate386();
    
    //printf("[386] cs:ip = %04x:%04x\n", seg_cs, ip);
    printf("[PMWU] Running stub program...\n");

    while(1) emulate386();













l_end:

    //printf("[386] di=%x(%u)\n", di, di);

    bin_fwrite(memory, m_alloc, "out.bin");
    //bin_fwrite(physicalAddress(program_segment), di - 0x100, "out.bin");

    return 0;
}
