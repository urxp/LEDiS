#ifndef __BIN2INC_H
#define __BIN2INC_H

#include<stdio.h>
#include<stdlib.h>
#include<string.h>

//#define PACKED

#ifdef PACKED
    #define __packed__      __attribute__((packed))
#else
    #define __packed__
#endif

typedef unsigned char   	byte;
typedef signed char   	    signed_byte;
typedef unsigned short  	word;
typedef signed short        signed_word;
typedef unsigned int    	dword;
typedef signed int    	    signed_dword;
typedef unsigned long long 	qword;
typedef unsigned int    	boolean;
typedef void * 				pointer;

typedef struct IRbyte           IRbyte;
typedef struct object_info      object_info;
typedef struct fixup_struct     fixup_struct;
typedef struct disasm_struct    DisasmInterface;
typedef struct virtual_address  VirtualAddress;

struct virtual_address{

    byte    obj_n;
    dword   offset;
} __packed__;


struct disasm_struct {

    FILE *          fd;
    char            InstructionString[256];
    VirtualAddress  vAddress;
    void            (*init)(void);
    void            (*close)(void);
    void            (*loadInstruction)(dword);
    void            (*pushAddress)(VirtualAddress *);
    boolean         (*popAddress)(void);
    void            (*disassemble)(void);
};

#define boolean(b)      ((boolean)!!(b))



struct object_info {

    char    * MapName;
    dword   RelBaseAddress;
    dword   offset;
    dword   size;
    boolean isCode;
    dword   bitsMode;
    dword   DisBytes;
    dword   DataBytes;
    dword   EntryAddress;
    dword   BSS;
    IRbyte  * IR;

};

struct fixup_struct {
	byte object_n;
	byte type;
	byte size;
    dword target;
} __packed__;

struct IRbyte {
    byte    flags;
    dword   data;
} __packed__;

boolean le_initLinearExecutable(char *);

dword le_getNumberOfObjects(void);
pointer le_getFixupPageTable(void);
pointer le_getFixupRecordTable(void);
dword le_getNumberOfPagesForObject(dword);
dword le_getPageSize(void);
dword le_getBaseAddressForObject(dword);
qword le_getEntry(void);
boolean le_checkBSSForObject(dword);
dword le_getObjectPagesForObject(dword);
dword le_getVirtualSizeForObject(dword);
dword le_getFixupsSize(void);
dword le_getSize(void);
pointer le_getBuffer(void);
boolean le_isCodeForObject(dword);
dword le_getBitsModeForObject(dword);


void le_createFixup(dword, dword, fixup_struct *);
fixup_struct * le_checkFixup(dword, dword);
fixup_struct * le_getFixup(dword, dword);
void le_createLabel(dword, dword);
boolean le_checkLabel(dword, dword);

void processFixups(void);
char * getLabel(dword obj, dword offset);
char * getObjectSegmentMapName(dword obj);
char * hexbyte(dword val);
dword getBitsMode();

void disassemble(void);
void checkInfixLabels(void);
pointer locateCode(void);
byte read_byte(void);
word read_word(void);
dword read_dword(void);
void mark_instruction(void);
void markAsData(VirtualAddress *, VirtualAddress *);
void comment_instruction(void);
void dump15(void);
boolean checkAddress(void);
dword getAlignment(dword);

char * getRegister(byte reg);
char * getSegRegister(byte reg);
char * getJCC(byte);

char *  mod_rm(void);
char *  mod_reg(void);
char *  mod_sreg(void);
byte    mod_mod(void);
char * o_size_str(void);

void pushJumpTable(VirtualAddress *);

#define iFORMAT "%-8s"
extern boolean prefix_66;
extern char ibuffer[256];
extern DisasmInterface  IDisasm;
extern VirtualAddress   current_va;
extern fixup_struct * fx;
extern char * seg_ds;
extern char * seg_ss;
extern char * seg_seg;
extern char * rep_prefix;
extern dword imm32;
extern dword id;
extern byte ib;
extern char * tmp_str;
extern signed_byte displacement_8;
extern signed_word displacement_16;
extern signed_dword displacement_32;
extern byte operand_size;
extern byte address_size;
extern byte address_mode;

boolean op_00(void);
boolean op_01(void);
boolean op_02(void);
boolean op_03(void);
boolean op_08(void);
boolean op_09(void);
boolean op_0a(void);
boolean op_0b(void);
boolean op_20(void);
boolean op_21(void);
boolean op_22(void);
boolean op_23(void);
boolean op_28(void);
boolean op_29(void);
boolean op_2a(void);
boolean op_2b(void);
boolean op_30(void);
boolean op_31(void);
boolean op_32(void);
boolean op_33(void);
boolean op_38(void);
boolean op_39(void);
boolean op_3a(void);
boolean op_3b(void);
boolean op_66(void);
boolean op_69(void);
boolean op_6b(void);
boolean op_80(void);
boolean op_81(void);
boolean op_83(void);
boolean op_84(void);
boolean op_85(void);
boolean op_88(void);
boolean op_89(void);
boolean op_8a(void);
boolean op_8b(void);
boolean op_8d(void);
boolean op_c0(void);
boolean op_c1(void);
boolean op_c4(void);
boolean op_c6(void);
boolean op_c7(void);
boolean op_d0(void);
boolean op_d1(void);
boolean op_f6(void);
boolean op_f7(void);
boolean op_ff(void);

#endif // __BIN2INC_H