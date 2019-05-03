#include<stdio.h>
#include<stdlib.h>
#include"bin2inc.h"



dword fixups_counter;


dword position, object_n, current_page_within_object;
void * FixupPageTable;
void * FixupRecordTable;



static boolean readFixup(){

	dword TRGOFF, SRCOFF_CNT, target_object_n;


	byte ATp = *(byte *)(FixupRecordTable + position);
	position++;

    boolean FixupToAliasFlag        = boolean(ATp & 0x10);
    boolean SourceListFlag          = boolean(ATp & 0x20);


	byte RTp = *(byte *)(FixupRecordTable + position);
	position++;


	boolean InternalRef 				= boolean((RTp & 0x03) == 0);
	boolean InternalRefByOrd 			= boolean((RTp & 0x03) == 1);
	boolean InternalRefByName 			= boolean((RTp & 0x03) == 2);
	boolean InternalRefViaEntryTable 	= boolean((RTp & 0x03) == 3);
	boolean Additive_Fixup 				= boolean(RTp & 0x04);
	boolean Zero_Check 					= boolean(RTp & 0x08);
	dword TargetOffsetSize 			= (RTp & 0x10) ? 32 : 16;
	dword AdditiveSize 				= (RTp & 0x20) ? 32 : 16;
	dword ObjectModuleOrd 			= (RTp & 0x40) ? 16 : 8;
	dword ImportOrdSize 			= (RTp & 0x80) ? 8 : TargetOffsetSize;


	if(FixupToAliasFlag){

		printf("[TODO] Fixup to 16:16 Alias Flag is set ...\n");
		printf("       ... don't know what to do!\n");
		return boolean(0);
	}

	if(SourceListFlag){
		// src_cnt byte
		SRCOFF_CNT = *(byte *)(FixupRecordTable + position);
		position++;

		printf("[TODO] Source List Flag is set ...\n");
		printf("       ... don't know what to do!\n");
		return boolean(0);
	}
	else{
		// src_off signed_word
		SRCOFF_CNT = *(word *)(FixupRecordTable + position);
		if(SRCOFF_CNT & 0x8000) SRCOFF_CNT |= 0xFFFF0000;
		position += 2;
	}

	if(Additive_Fixup){

		printf("[TODO] Additive Fixup Flag is set [size: %u] ...\n", AdditiveSize);
		printf("       ... don't know what to do!\n");
		return boolean(0);
	}

	if(Zero_Check){

		printf("Bit 3 of Target Flags should be 0!\n");
		//return boolean(0);
	}


	switch(ATp & 0x0F){
		case 0:
			// 00h = Byte fixup (8-bits). 
			printf("ATp & 0x0F :: %02xh\n", (ATp & 0x0F));
			break;
		case 1:
			// 01h = (undefined)
			printf("ATp & 0x0F :: %02xh\n", (ATp & 0x0F));
			break;
		case 2:
			// 16-bit Selector fixup (16-bits).
			if(ObjectModuleOrd == 16){
				target_object_n = *(word *)(FixupRecordTable + position);
				position += 2;
			}
			else{
				target_object_n = *(byte *)(FixupRecordTable + position);
				position++;
			}

			le_createFixup(object_n, (current_page_within_object - 1) * le_getPageSize() + SRCOFF_CNT, &(fixup_struct){
				.object_n = target_object_n,
				.type = 2,
				.target = 0,
				.size = 2
			});

			break;
		case 3:
			// 03h = 16:16 Pointer fixup (32-bits). 
			printf("ATp & 0x0F :: %02xh\n", (ATp & 0x0F));
			break;
		case 4:
			// 04h = (undefined)
			printf("ATp & 0x0F :: %02xh\n", (ATp & 0x0F));
			break;
		case 5:
			// 05h = 16-bit Offset fixup (16-bits).
			printf("ATp & 0x0F :: %02xh\n", (ATp & 0x0F));
			break;
		case 6:
			// 06h = 16:32 Pointer fixup (48-bits). 
			printf("ATp & 0x0F :: %02xh\n", (ATp & 0x0F));
			break;
		case 7:
			// 07h = 32-bit Offset fixup (32-bits). 
			if(ObjectModuleOrd == 16){
				target_object_n = *(word *)(FixupRecordTable + position);
				position += 2;
			}
			else{
				target_object_n = *(byte *)(FixupRecordTable + position);
				position++;
			}

			if(TargetOffsetSize == 32){

				TRGOFF = *(dword *)(FixupRecordTable + position);
				position += 4;
			}
			else{

				TRGOFF = *(word *)(FixupRecordTable + position);
				position += 2;
			}

			le_createFixup(object_n, (current_page_within_object - 1) * le_getPageSize() + SRCOFF_CNT, &(fixup_struct){
				.object_n = target_object_n,
				.type = 7,
				.target = TRGOFF,
				.size = 4
			});
			
			le_createLabel(target_object_n, TRGOFF);
			break;
		case 8:
			// 08h = 32-bit Self-relative offset fixup (32-bits). 
			printf("ATp & 0x0F :: %02xh\n", (ATp & 0x0F));
			break;
		default:
			printf("err unknown source type %02xh\n", (ATp & 0x0F));
			return boolean(0);
	}
	
	return boolean(1);
}

void processFixups(void){

    dword __true                = 1; 
    dword current_page          = 1;
    dword pages_object_n;

    dword number_of_objects     = le_getNumberOfObjects();
    FixupPageTable              = le_getFixupPageTable();
    FixupRecordTable            = le_getFixupRecordTable();

    position                    = 0;
    current_page_within_object  = 1;
    object_n                    = 1;
    pages_object_n              = le_getNumberOfPagesForObject(object_n);


/*
	while(1){

		if(position == *(dword *)(FixupPageTable + 4*current_page)){

			current_page++;
			current_page_within_object++;
		
			if(current_page_within_object > pages_object_n){

				object_n++;
				pages_object_n = le_getNumberOfPagesForObject(object_n);
				current_page_within_object = 1;
			}
					
			if(object_n > number_of_objects) break;
		}
		else if(!readFixup()) break;
	}
*/

    while(__true){

        if((position == *(dword *)(FixupPageTable + 4*current_page))
        ||(!readFixup()&&--__true)){

            current_page++;
            current_page_within_object++;

            if((current_page_within_object > pages_object_n)
            &&(!(++object_n > number_of_objects)||--__true)){

                pages_object_n = le_getNumberOfPagesForObject(object_n);
                current_page_within_object = 1;
            }
        }
    }
}
