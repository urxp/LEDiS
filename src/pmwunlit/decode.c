#include<stdio.h>
#include<stdlib.h>

typedef unsigned char   byte;
typedef unsigned char   dword;

size_t decode_bufsize;
size_t decode_filesize;

byte * output_buffer;
byte * decode_bufbase;
byte * decode_bufend;
byte * decode_bufthreshold;
FILE * decode_handle;

void decode();


int main(){

    int i;

    decode_bufsize = 0x6F450;
    decode_filesize = 0x3728F;

    decode_handle = fopen("RALLY.EXE", "rb");
    fseek(decode_handle, 0x2800/* + 0x1A25F*/, SEEK_SET);

    output_buffer = calloc(decode_bufsize, sizeof(byte));

    decode_bufbase = output_buffer;
    decode_bufend = output_buffer + decode_bufsize;
    decode_bufthreshold = output_buffer + decode_bufsize - 16;


    decode();

    fclose(decode_handle);

    
    for(i = 0; i < 0x6F450; i++){
        
        printf("%c", output_buffer[i]);
    }

    return 0;
}
