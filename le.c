#include"bin2inc.h"
#include<stdio.h>
#include<stdlib.h>

pointer LE;
dword LE_size;
object_info * ObjectMap;
dword fixups_counter;

fixup_struct fixup_null;
fixup_struct fixup_tmp;

extern DisasmInterface IDisasm;

FILE * fixup_fd;

static char * createObjectName(dword obj_n){

    char * str = calloc(32, sizeof(byte));

    sprintf(str, "obj_%d", obj_n);

    return str;
}




boolean le_initLinearExecutable(char * le_file){

    dword i, check;
    dword entry[2];
    FILE * fd;

    fd = fopen(le_file, "rb");
    if(!fd) return boolean(0);

    fseek(fd, 0, 2);
    LE_size = sizeof(byte) * ftell(fd);
    fseek(fd, 0, 0);

    LE = calloc(LE_size, sizeof(byte));
    check = fread(LE, sizeof(byte), LE_size, fd);
    fclose(fd);

    if(LE_size != check * sizeof(byte)){

        free(LE);
        return boolean(0);
    }

    fixup_null = (fixup_struct){0};

    fixups_counter = 0;

    ObjectMap = calloc(le_getNumberOfObjects(), sizeof(object_info));

    *(qword *)entry = le_getEntry();

    for(i = 1; i <= le_getNumberOfObjects(); i++){

        ObjectMap[i - 1] = (object_info){
            .MapName        = createObjectName(i),
            .RelBaseAddress = le_getBaseAddressForObject(i),
            .offset         = le_getObjectPagesForObject(i),
            .size           = le_getVirtualSizeForObject(i),
            .isCode         = le_isCodeForObject(i),
            .bitsMode       = le_getBitsModeForObject(i),
            .DisBytes       = 0,
            .DataBytes      = 0,
            .EntryAddress   = (entry[0] == i) ? entry[1] : -1,
            .BSS            = le_checkBSSForObject(i),
            .IR             = calloc(le_getVirtualSizeForObject(i), sizeof(IRbyte))
        };
    }

    return boolean(1);
}




dword le_getSize(){

    return LE_size;
}

pointer le_getBuffer(){

    return LE;
}

dword le_getPageSize(){

    return *(dword *)(LE + 0x28);
}

dword le_getObjectTableOffset(dword obj_n){
 
    dword Base = *(dword *)(LE + 0x40);

    return (obj_n == 0 ) ? Base : Base + (obj_n - 1) * 0x18;
}

boolean le_checkBSSForObject(dword obj_n){

    return (le_getNumberOfPagesForObject(obj_n) * le_getPageSize() < le_getVirtualSizeForObject(obj_n));
}

qword le_getEntry(){

    IDisasm.pushAddress(&(VirtualAddress){ 
                .obj_n = *(dword *)(LE + 0x18),
                .offset = *(dword *)(LE + 0x1c) });

    return (qword)*(dword *)(LE + 0x18) + ((qword)*(dword *)(LE + 0x1c) << 32);
}


dword le_getNumberOfObjects(){

    return *(dword *)(LE + 0x44);
}

dword le_getFixupPageTableOffset(){

    return *(dword *)(LE + 0x68);
}

dword le_getFixupRecordTableOffset(){

    return *(dword *)(LE + 0x6C);
}

dword le_getFixupsSize(){

    return *(dword *)(LE + 0x30);
}


boolean le_isCodeForObject(dword obj_n){

    pointer ObjectTable = LE + le_getObjectTableOffset(obj_n);
    boolean is_code = boolean(0);

    if(*(dword *)(ObjectTable + 0x8) & 0x00000004) is_code = boolean(1);

    return is_code;
}

dword le_getBitsModeForObject(dword obj_n){

    pointer ObjectTable = LE + le_getObjectTableOffset(obj_n);
    dword bits_mode = 16;

    if(*(dword *)(ObjectTable + 0x8) & 0x00002000) bits_mode = 32;

    return bits_mode;
}

dword le_getNumberOfPagesForObject(dword obj_n){

    pointer ObjectTable = LE + le_getObjectTableOffset(obj_n);

    return *(dword *)(ObjectTable + 0x10);
}

dword le_getIndexOfFirstPageForObject(dword obj_n){

    pointer ObjectTable = LE + le_getObjectTableOffset(obj_n);

    return *(dword *)(ObjectTable + 0x0c);
}

dword le_getVirtualSizeForObject(dword obj_n){

    pointer ObjectTable = LE + le_getObjectTableOffset(obj_n);

    return *(dword *)(ObjectTable + 0x0);
}

dword le_getBaseAddressForObject(dword obj_n){

    pointer ObjectTable = LE + le_getObjectTableOffset(obj_n);

    return *(dword *)(ObjectTable + 0x4);
}

dword le_getObjectPagesForObject(dword obj_n){

    dword DataOffset = *(dword *)(LE + 0x80); // from beginning of file
    dword LE_Offset = 0;

    return  (DataOffset - LE_Offset) + (le_getIndexOfFirstPageForObject(obj_n) - 1) * le_getPageSize();
}

object_info * le_getObjectMap(){

    return ObjectMap;
}

void le_createLabel(dword obj_n, dword offset){

    ObjectMap[obj_n - 1].IR[offset].flags |= 1;
}

void le_createFixup(dword obj_n, dword offset, fixup_struct * sFixup){

    ObjectMap[obj_n - 1].IR[offset].flags |= 2;

    ObjectMap[obj_n - 1].IR[offset].data = fixups_counter++;
    fwrite(sFixup, sizeof(fixup_struct), 1, fixup_fd);
}

boolean le_checkLabel(dword object_n, dword offset){

    return boolean(ObjectMap[object_n - 1].IR[offset].flags & 1);
}



fixup_struct * le_getFixup(dword object_n, dword offset){

    fseek(
        fixup_fd,
        ObjectMap[object_n - 1].IR[offset].data * sizeof(fixup_struct),
        SEEK_SET);

    fread(&fixup_tmp, sizeof(fixup_struct), 1, fixup_fd);

    return &fixup_tmp;
}

fixup_struct * le_checkFixup(dword object_n, dword offset){

    boolean check = boolean(ObjectMap[object_n - 1].IR[offset].flags & 2);

    if(check){
    
        return le_getFixup(object_n, offset);
    }

    return &fixup_null;
}


pointer le_getFixupPageTable(void){

    return LE + le_getFixupPageTableOffset();
}

pointer le_getFixupRecordTable(void){

    return LE + le_getFixupRecordTableOffset();
}