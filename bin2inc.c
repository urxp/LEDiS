#include"bin2inc.h"
#include<stdio.h>
#include<stdlib.h>

#ifdef SHOW_INPUT_BYTES
	#define GLOBAL_HEX
#endif

#define GLOBAL_STATS
//#define GLOBAL_POSITIONS

#ifdef GLOBAL_STATS
	#include<sys/time.h>
#endif

#define STACK_SIZE 0x1000

extern dword fixups_counter;
extern object_info * ObjectMap;
extern FILE * fixup_fd;
extern DisasmInterface IDisasm;

char * indent = "";
dword fix_indent = 0;

dword GLOBAL_DB_MAX = 0x10;
#ifdef GLOBAL_POSITIONS
	dword GLOBAL_POSITIONS_INTERVAL = 0x080;
	dword GLOBAL_PAGE_SIZE = 0x1000;
#endif
const char * GLOBAL_LABEL_PREFIX = "___";

#ifdef GLOBAL_STATS
struct timeval __global_timer, __local_timer;
qword global_timer, local_timer;

qword timeDiff(struct timeval * start){

	qword useconds0 = 1000000 * start->tv_sec + start->tv_usec;
    gettimeofday(&__local_timer, (void *)0);
	qword useconds1 = 1000000 * __local_timer.tv_sec + __local_timer.tv_usec;

	return useconds1 - useconds0;
}

char * formatTime(qword useconds){

	char * ft = calloc(32, 1);

	dword seconds0 = useconds / 1000000;
	dword mseconds0 = useconds / 1000 % 1000;
	dword useconds0 = useconds % 1000000;

	if(useconds < 10000){

		sprintf(ft, "%d.%03d ms", mseconds0, useconds0);

		return ft;
	}

	sprintf(ft, "%d.%03d s", seconds0, mseconds0);

	return ft;
}
#endif



char * getObjectSegmentMapName(dword object_n){

	return ObjectMap[object_n - 1].MapName;
}

char * getObjectMapName(dword object_n){

	return ObjectMap[object_n - 1].MapName;
}

dword getObjectSize(dword object_n){

	return ObjectMap[object_n - 1].size;
}

dword getObjectOffset(dword object_n){

	return ObjectMap[object_n - 1].offset;
}

dword getObjectSegmentBaseAddress(dword object_n){

	return ObjectMap[object_n - 1].RelBaseAddress;
}

boolean getEntryStatus(dword object_n, dword address){

	if(ObjectMap[object_n - 1].EntryAddress != -1){

		return boolean(address == -1)
		? boolean(1)
		: boolean(ObjectMap[object_n - 1].EntryAddress == address); 
	}

	return boolean(0);
}

dword getEntryAddress(dword object_n){

	if(getEntryStatus(object_n, -1)) return ObjectMap[object_n - 1].EntryAddress;

	return -1;
}

boolean hasBSS(dword object_n){

	return boolean(ObjectMap[object_n - 1].BSS);
}


char * formatAddress(dword value){

	char * addressStr = calloc(12, 1); 

	sprintf(addressStr, "%08x", value);

	return addressStr;
}




char * hexbyte(dword val){

	char * result = calloc(12, 1);
	char * result0 = calloc(12, 1);

	sprintf(result, "%x", val);

	if(val < 10) return result;

	while(val){

		if(((val > 15) && (val < 160))
		||(val < 10)){

			sprintf(result0, "%sh", result);
			sprintf(result, "%s", result0);
			return result;
		}
		
		val >>= 8;
	}

	sprintf(result0, "0%sh", result);
	sprintf(result, "%s", result0);
	
	return result;
}

char * getLabel(dword obj, dword offset){

	char * label;


#define lname(a, n) 	\
	case a:				\
		return n;


	if(obj == 1){ // SIERRA.LE specific
		switch(offset){
		lname(0x00051d34, "main");
		lname(0x0006a830, "memset");
		lname(0x0006a9a1, "printf");
		lname(0x0006ad0a, "open");
		lname(0x0006afa0, "close");
		lname(0x0006b368, "read");
		lname(0x0006b441, "int386");
		lname(0x0006be01, "int386x");
		lname(0x0006be22, "malloc");
		lname(0x0006bea4, "free");
		lname(0x0006c1f8, "fopen");
		lname(0x0006c2b4, "fclose");
		lname(0x0006e642, "getenv");
		lname(0x0006eb20, "__CMain");
		lname(0x0006eb70, "__InitRtns");
		lname(0x0006ebb3, "__FiniRtns");
		lname(0x0006f9cd, "memcpy");
		default:
			break;
		}
	}
/*
	if(obj == 4){ // RALLY.LE specific
		switch(offset){
		lname(0x000052a4, "__IsTable");
		default:
			break;
		}
	}
*/

#undef lname


	label = calloc(32, 1);
	sprintf(label, "%s%s", GLOBAL_LABEL_PREFIX, hexbyte(getObjectSegmentBaseAddress(obj) + offset));

	return label;
}

#ifdef GLOBAL_STATS
char * speed(dword bytes, qword us){

	char * bspeed = calloc(32, 1);

	dword Ki = 1024;
	dword KB = Ki;
	dword MB = Ki*KB;
	dword GB = Ki*MB;

	double seconds_inverse = 1000000.0 / us;
	double speed_bs = seconds_inverse * bytes;

	if(speed_bs < (double)KB){
		sprintf(bspeed, "%.2f B/s", speed_bs);	
		return bspeed;
	}

	if(speed_bs < (double)MB){
		sprintf(bspeed, "%.2f KB/s", speed_bs / KB);	
		return bspeed;
	}

	if(speed_bs < (double)GB){
		sprintf(bspeed, "%.2f MB/s", speed_bs / MB);	
		return bspeed;
	}

	sprintf(bspeed, "%.2f GB/s", speed_bs / GB);	
	return bspeed;
}
#endif





void insert_dups(FILE * fd, dword qms){

	if(qms > 0) fprintf(fd, "resb\t%s\n", hexbyte(qms));
}
















struct {

	FILE * 		fd;
	dword 		obj_n;
	dword 		i;
	dword 		db_written;
	dword 		page_number;
	pointer 	in_buff;
} le;




void fn_fixup(){ // fixup

	dword diff;
	fixup_struct * fixup;

	fixup = le_getFixup(le.obj_n, le.i);

	switch(fixup->size){
	case 1:
		fprintf(le.fd, "\n%sdb\t%s", indent, getLabel(fixup->object_n, fixup->target));
		le.db_written++;
		break;
	case 2:
		if(fixup->type == 2){
			fprintf(le.fd, "\n%sdw\t%s", indent, getObjectSegmentMapName(fixup->object_n));
		}
		else {
			fprintf(le.fd, "\n%sdw\t%s", indent, getLabel(fixup->object_n, fixup->target));
		}
		le.i+=1;
		le.db_written = 0;
#ifdef GLOBAL_POSITIONS
		if(le.i % GLOBAL_PAGE_SIZE == 0) le.page_number++;
		if(le.i % GLOBAL_POSITIONS_INTERVAL == 0)
			fprintf(le.fd, "\n;%d:%s+1", le.page_number, formatAddress(getObjectSegmentBaseAddress(le.obj_n) + le.i));
#endif
		break;
	case 4:
		if(fixup->type == 8){
			fprintf(le.fd, "\n%sdd\t%s-4-$", indent, getLabel(fixup->object_n, fixup->target));
		}
		else {
			fprintf(le.fd, "\n%sdd\t%s", indent, getLabel(fixup->object_n, fixup->target));
		}
		le.i+=3;
		le.db_written = 0;
#ifdef GLOBAL_POSITIONS
		if(le.i % GLOBAL_PAGE_SIZE < 3) le.page_number++;
		diff = le.i % GLOBAL_POSITIONS_INTERVAL;
		if(diff < 3)
			fprintf(le.fd, "\n;%d:%s+%d", le.page_number, formatAddress(getObjectSegmentBaseAddress(le.obj_n) + le.i - diff), 3 - diff);
#endif			
		break;
	default:
		printf("[bin2inc] err invalid fixup size %d\n", fixup->size);
	}
}


void fn_nofixup(){

	char indt[16];
	char * dlmtr;

	if(le.db_written % GLOBAL_DB_MAX != 0){
		dlmtr = ",";
	}
	else {
		sprintf(indt, "\n%sdb\t", indent);
		dlmtr = indt;
	}

	fprintf(le.fd, "%s%s", dlmtr, hexbyte(((byte *)le.in_buff)[le.i]));
	le.db_written++;
}

void fn_bytes(){

	char addr[16];
	char * dlmtr;

	if(le.db_written % GLOBAL_DB_MAX != 0){
		dlmtr = " ";
	}
	else {
		sprintf(addr,"\n;%08x: ", getObjectSegmentBaseAddress(le.obj_n) + le.i);
		dlmtr = addr;
	};
	fprintf(le.fd, "%s%02x", dlmtr, ((byte *)le.in_buff)[le.i]);
	le.db_written++;
}

void (*f_fixup[2])(void) = { &fn_nofixup, &fn_fixup };

void fn_noinstruction(){

	f_fixup[!!(ObjectMap[le.obj_n - 1].IR[le.i].flags & 0x2)]();
}

void fn_instruction(){

	//indent = "";

	IDisasm.loadInstruction(ObjectMap[le.obj_n - 1].IR[le.i].data);

	dword le_i = le.i;

	dword skip;
	
	skip = (ObjectMap[le.obj_n - 1].IR[le.i].flags & 0xF0) >> 4;

	le.db_written = 0;
	while(skip--){
#ifdef GLOBAL_HEX
		fn_bytes();
#endif
		le.i++;
	}
	
	fix_indent = le.i;

	le.i = le_i;

	if(!!(ObjectMap[le.obj_n - 1].IR[le.i].flags & 0x8)){
		// comment out instruction, put raw bytes instead
		fprintf(le.fd, "\n\t\t;%s", IDisasm.InstructionString);
		le.db_written = 0;
		indent = "\t\t";
		fn_noinstruction();
	}
	else{
		
		skip = (ObjectMap[le.obj_n - 1].IR[le.i].flags & 0xF0) >> 4;
		le.i += (skip - 1);
		le.db_written = 0;

		fprintf(le.fd, "\n\t\t%s", IDisasm.InstructionString);
	}
}

void (*f_instruction[2])(void) = { &fn_noinstruction, &fn_instruction };

void fn_nolabel(){

	f_instruction[!!(ObjectMap[le.obj_n - 1].IR[le.i].flags & 0x4)]();
}

void fn_label(){

	fprintf(le.fd, "\n%s:", getLabel(le.obj_n, le.i));
	le.db_written = 0;

	fn_nolabel();
}


void (*f_label[2])(void) = { &fn_nolabel, &fn_label };





void bin2inc_2(dword obj_n, char * section_name){
	printf("[bin2inc] generating %s.inc\n", section_name);


	char filename[64] = {0};

	
	
	

	dword buff_size, qms, bss_end;

	if(hasBSS(obj_n)){

		buff_size	= le_getSize() - getObjectOffset(obj_n);
		bss_end 	= getObjectSize(obj_n) - STACK_SIZE;
	}
	else {		
		buff_size 	= getObjectSize(obj_n);
		bss_end		= 0;	
	}

	sprintf(filename, "%s.inc", section_name);

	le.in_buff 			= le_getBuffer() + getObjectOffset(obj_n);
	le.fd				= fopen(filename, "w");
	le.db_written		= 0;
	le.page_number		= 0;
	le.obj_n 			= obj_n;

	fprintf(le.fd, ";OBJECT#%d", obj_n);

//#ifdef GLOBAL_STATS
//	gettimeofday(&__local_timer, (void *)0);
//#endif

	for(le.i = 0; le.i < buff_size; le.i++){

		if(le.i == fix_indent){
			le.db_written = 0;
			indent = "";
		}
		
#ifdef GLOBAL_POSITIONS
		if(le.i % GLOBAL_PAGE_SIZE == 0) le.page_number++;

		if(le.i % GLOBAL_POSITIONS_INTERVAL == 0){
			
			fprintf(le.fd, "\n;%d:%s", le.page_number, formatAddress(getObjectSegmentBaseAddress(obj_n) + le.i));
			le.db_written = 0;
		}
#endif

		if(getEntryStatus(obj_n, le.i)){
		
			fprintf(le.fd, "\n..start:");
			le.db_written = 0;
		}

		f_label[ObjectMap[obj_n - 1].IR[le.i].flags & 0x1]();
	}

//#ifdef GLOBAL_STATS
//	local_timer = timeDiff(&__local_timer);
//	printf("          ... processed %u bytes in %s (%s)\n", buff_size, formatTime(local_timer), speed(buff_size, local_timer));
//#endif

	fclose(le.fd);

	if(bss_end != 0){

		sprintf(filename, "bss_%d.inc", obj_n);
		printf("[bin2inc] generating bss_%u.inc\n", obj_n);

		qms			= 0;
		le.fd		= fopen(filename, "w");

		fprintf(le.fd, ";UNINITIALIZED DATA\n");

//#ifdef GLOBAL_STATS
//		buff_size 	= bss_end - le.i;
//		gettimeofday(&__local_timer, (void *)0);
//#endif

		for(; le.i < bss_end; le.i++){

			if(le_checkLabel(obj_n, le.i)){

				insert_dups(le.fd, qms);
				qms = 0;

				fprintf(le.fd, "%s:\t", getLabel(obj_n, le.i));
			}

			qms++;
		}

//#ifdef GLOBAL_STATS
//		local_timer = timeDiff(&__local_timer);
//		printf("          ... processed %u bytes in %s (%s)\n", buff_size, formatTime(local_timer), speed(buff_size, local_timer));
//#endif

		insert_dups(le.fd, qms);
		fclose(le.fd);
	}



	dword codeBytes = ObjectMap[obj_n - 1].size - ObjectMap[obj_n - 1].DataBytes;

	double code_ratio = 100.0 * (double)ObjectMap[obj_n - 1].DisBytes / (double)codeBytes;
	double data_ratio = 100.0 * (double)ObjectMap[obj_n - 1].DataBytes / (double)ObjectMap[obj_n - 1].size;

	printf("          ... disassembled %u/%u bytes (%.3f %%)\n", ObjectMap[obj_n - 1].DisBytes, codeBytes, code_ratio);
	printf("          ... data %u/%u bytes (%.3f %%)\n", ObjectMap[obj_n - 1].DataBytes, ObjectMap[obj_n - 1].size, data_ratio);
}

























































int main(int argc, char * argv[]){

	dword object_n;

	fixup_fd = fopen("$__fixups.tmp", "wb+");
	IDisasm.init();

#ifdef GLOBAL_STATS
	gettimeofday(&__global_timer, (void *)0);
#endif

    le_initLinearExecutable("SIERRA.LE");

    printf("[bin2inc] generating fixups and labels\n");

    processFixups();

	printf("          ... %d fixups\n", fixups_counter);
//#ifdef GLOBAL_STATS
//	local_timer = timeDiff(&__global_timer);
//	printf("          ... %d fixups in %s\n", fixups_counter, formatTime(local_timer));
//#endif


	// disassembling [TODO]
	// # __InitRtns
	IDisasm.pushAddress(&(VirtualAddress){ .obj_n = 1, .offset = 0x00001040 });
	IDisasm.pushAddress(&(VirtualAddress){ .obj_n = 1, .offset = 0x000053b0 });
	IDisasm.pushAddress(&(VirtualAddress){ .obj_n = 1, .offset = 0x00005c50 });
	IDisasm.pushAddress(&(VirtualAddress){ .obj_n = 1, .offset = 0x0000d930 });
	IDisasm.pushAddress(&(VirtualAddress){ .obj_n = 1, .offset = 0x000218c0 });
	IDisasm.pushAddress(&(VirtualAddress){ .obj_n = 1, .offset = 0x00025940 });
	IDisasm.pushAddress(&(VirtualAddress){ .obj_n = 1, .offset = 0x00029680 });
	IDisasm.pushAddress(&(VirtualAddress){ .obj_n = 1, .offset = 0x000309a0 });
	IDisasm.pushAddress(&(VirtualAddress){ .obj_n = 1, .offset = 0x000320a0 });
	IDisasm.pushAddress(&(VirtualAddress){ .obj_n = 1, .offset = 0x00033a20 });
	IDisasm.pushAddress(&(VirtualAddress){ .obj_n = 1, .offset = 0x00041040 });
	IDisasm.pushAddress(&(VirtualAddress){ .obj_n = 1, .offset = 0x00041b60 });
	IDisasm.pushAddress(&(VirtualAddress){ .obj_n = 1, .offset = 0x000420f0 });
	IDisasm.pushAddress(&(VirtualAddress){ .obj_n = 1, .offset = 0x0004a8d0 });
	IDisasm.pushAddress(&(VirtualAddress){ .obj_n = 1, .offset = 0x000518b0 });
	IDisasm.pushAddress(&(VirtualAddress){ .obj_n = 1, .offset = 0x00053080 });
	IDisasm.pushAddress(&(VirtualAddress){ .obj_n = 1, .offset = 0x0005be40 });
	IDisasm.pushAddress(&(VirtualAddress){ .obj_n = 1, .offset = 0x000618c0 });
	IDisasm.pushAddress(&(VirtualAddress){ .obj_n = 1, .offset = 0x00063520 });
	IDisasm.pushAddress(&(VirtualAddress){ .obj_n = 1, .offset = 0x0006a977 });
	IDisasm.pushAddress(&(VirtualAddress){ .obj_n = 1, .offset = 0x0006e48b });
	IDisasm.pushAddress(&(VirtualAddress){ .obj_n = 1, .offset = 0x0006b7f0 });
	IDisasm.pushAddress(&(VirtualAddress){ .obj_n = 1, .offset = 0x0006bf84 });
	IDisasm.pushAddress(&(VirtualAddress){ .obj_n = 1, .offset = 0x000709e3 });
	IDisasm.pushAddress(&(VirtualAddress){ .obj_n = 1, .offset = 0x00071403 });
	IDisasm.pushAddress(&(VirtualAddress){ .obj_n = 1, .offset = 0x00070e1c });
	IDisasm.pushAddress(&(VirtualAddress){ .obj_n = 1, .offset = 0x00071b6a });
	// __InitRtns #
	// # __FiniRtns
	IDisasm.pushAddress(&(VirtualAddress){ .obj_n = 1, .offset = 0x0006ccff });
	IDisasm.pushAddress(&(VirtualAddress){ .obj_n = 1, .offset = 0x00070a28 });
	// __FiniRtns #

	// 'main ?'
	IDisasm.pushAddress(&(VirtualAddress){ .obj_n = 1, .offset = 0x0005168c });


	IDisasm.pushAddress(&(VirtualAddress){ .obj_n = 1, .offset = 0x0004cca0 });
	IDisasm.pushAddress(&(VirtualAddress){ .obj_n = 1, .offset = 0x0004ccfc });
	IDisasm.pushAddress(&(VirtualAddress){ .obj_n = 1, .offset = 0x00054674 });
	IDisasm.pushAddress(&(VirtualAddress){ .obj_n = 1, .offset = 0x000546ac });
	IDisasm.pushAddress(&(VirtualAddress){ .obj_n = 1, .offset = 0x0004cd20 });
	IDisasm.pushAddress(&(VirtualAddress){ .obj_n = 1, .offset = 0x0004cda0 });
	IDisasm.pushAddress(&(VirtualAddress){ .obj_n = 1, .offset = 0x000286fc });
	IDisasm.pushAddress(&(VirtualAddress){ .obj_n = 1, .offset = 0x0003435c });
	IDisasm.pushAddress(&(VirtualAddress){ .obj_n = 1, .offset = 0x000344ec });
	IDisasm.pushAddress(&(VirtualAddress){ .obj_n = 1, .offset = 0x000342a4 });
	IDisasm.pushAddress(&(VirtualAddress){ .obj_n = 1, .offset = 0x0001d5f4 });
	IDisasm.pushAddress(&(VirtualAddress){ .obj_n = 1, .offset = 0x0001d160 });
	IDisasm.pushAddress(&(VirtualAddress){ .obj_n = 1, .offset = 0x0001d264 });
	IDisasm.pushAddress(&(VirtualAddress){ .obj_n = 1, .offset = 0x0001d0e0 });
	IDisasm.pushAddress(&(VirtualAddress){ .obj_n = 1, .offset = 0x0001d068 });
	IDisasm.pushAddress(&(VirtualAddress){ .obj_n = 1, .offset = 0x0001d02c });
	IDisasm.pushAddress(&(VirtualAddress){ .obj_n = 1, .offset = 0x0001d044 });
	IDisasm.pushAddress(&(VirtualAddress){ .obj_n = 1, .offset = 0x0001d464 });
	IDisasm.pushAddress(&(VirtualAddress){ .obj_n = 1, .offset = 0x0001d484 });
	IDisasm.pushAddress(&(VirtualAddress){ .obj_n = 1, .offset = 0x0003212c });
	IDisasm.pushAddress(&(VirtualAddress){ .obj_n = 1, .offset = 0x000320ac });
	IDisasm.pushAddress(&(VirtualAddress){ .obj_n = 1, .offset = 0x0001b58c });
	IDisasm.pushAddress(&(VirtualAddress){ .obj_n = 1, .offset = 0x0001c580 });
	IDisasm.pushAddress(&(VirtualAddress){ .obj_n = 1, .offset = 0x0001c74c });
	IDisasm.pushAddress(&(VirtualAddress){ .obj_n = 1, .offset = 0x0001b560 });
	IDisasm.pushAddress(&(VirtualAddress){ .obj_n = 1, .offset = 0x0001ceb4 });
	IDisasm.pushAddress(&(VirtualAddress){ .obj_n = 1, .offset = 0x0001cf48 });
	IDisasm.pushAddress(&(VirtualAddress){ .obj_n = 1, .offset = 0x0001cf60 });
	IDisasm.pushAddress(&(VirtualAddress){ .obj_n = 1, .offset = 0x0001cfc8 });
	IDisasm.pushAddress(&(VirtualAddress){ .obj_n = 1, .offset = 0x00054f90 });
	IDisasm.pushAddress(&(VirtualAddress){ .obj_n = 1, .offset = 0x00054ed0 });
	IDisasm.pushAddress(&(VirtualAddress){ .obj_n = 1, .offset = 0x00054b94 });
	IDisasm.pushAddress(&(VirtualAddress){ .obj_n = 1, .offset = 0x0001cd54 });
	IDisasm.pushAddress(&(VirtualAddress){ .obj_n = 1, .offset = 0x0001ce04 });
	IDisasm.pushAddress(&(VirtualAddress){ .obj_n = 1, .offset = 0x00021080 });
	IDisasm.pushAddress(&(VirtualAddress){ .obj_n = 1, .offset = 0x0004f548 });
	IDisasm.pushAddress(&(VirtualAddress){ .obj_n = 1, .offset = 0x0001a2ec });
	IDisasm.pushAddress(&(VirtualAddress){ .obj_n = 1, .offset = 0x0001b29c });
	IDisasm.pushAddress(&(VirtualAddress){ .obj_n = 1, .offset = 0x000296e4 });
	IDisasm.pushAddress(&(VirtualAddress){ .obj_n = 1, .offset = 0x00063f74 });
	IDisasm.pushAddress(&(VirtualAddress){ .obj_n = 1, .offset = 0x00021d40 });
	IDisasm.pushAddress(&(VirtualAddress){ .obj_n = 1, .offset = 0x00033790 });
	IDisasm.pushAddress(&(VirtualAddress){ .obj_n = 1, .offset = 0x00011d2c });
	IDisasm.pushAddress(&(VirtualAddress){ .obj_n = 1, .offset = 0x000136dc });
	IDisasm.pushAddress(&(VirtualAddress){ .obj_n = 1, .offset = 0x00010410 });
	IDisasm.pushAddress(&(VirtualAddress){ .obj_n = 1, .offset = 0x0001dcb0 });
	IDisasm.pushAddress(&(VirtualAddress){ .obj_n = 1, .offset = 0x000182c4 });
	IDisasm.pushAddress(&(VirtualAddress){ .obj_n = 1, .offset = 0x0003f650 });
	IDisasm.pushAddress(&(VirtualAddress){ .obj_n = 1, .offset = 0x0003fa4c });
	IDisasm.pushAddress(&(VirtualAddress){ .obj_n = 1, .offset = 0x00018328 });
	IDisasm.pushAddress(&(VirtualAddress){ .obj_n = 1, .offset = 0x00030750 });
	IDisasm.pushAddress(&(VirtualAddress){ .obj_n = 1, .offset = 0x000307a0 });
	IDisasm.pushAddress(&(VirtualAddress){ .obj_n = 1, .offset = 0x000308fc });
	IDisasm.pushAddress(&(VirtualAddress){ .obj_n = 1, .offset = 0x0005100c });
	IDisasm.pushAddress(&(VirtualAddress){ .obj_n = 1, .offset = 0x0005106c });
	IDisasm.pushAddress(&(VirtualAddress){ .obj_n = 1, .offset = 0x0004d190 });
	IDisasm.pushAddress(&(VirtualAddress){ .obj_n = 1, .offset = 0x00051218 });
	IDisasm.pushAddress(&(VirtualAddress){ .obj_n = 1, .offset = 0x000512b8 });
	IDisasm.pushAddress(&(VirtualAddress){ .obj_n = 1, .offset = 0x00051164 });
	IDisasm.pushAddress(&(VirtualAddress){ .obj_n = 1, .offset = 0x000510b8 });
	IDisasm.pushAddress(&(VirtualAddress){ .obj_n = 1, .offset = 0x000510cc });
	IDisasm.pushAddress(&(VirtualAddress){ .obj_n = 1, .offset = 0x000151e8 });
	IDisasm.pushAddress(&(VirtualAddress){ .obj_n = 1, .offset = 0x0005c748 });
	IDisasm.pushAddress(&(VirtualAddress){ .obj_n = 1, .offset = 0x000010a8 });
	IDisasm.pushAddress(&(VirtualAddress){ .obj_n = 1, .offset = 0x0006352c });
	IDisasm.pushAddress(&(VirtualAddress){ .obj_n = 1, .offset = 0x00022f24 });
	IDisasm.pushAddress(&(VirtualAddress){ .obj_n = 1, .offset = 0x00023438 });
	IDisasm.pushAddress(&(VirtualAddress){ .obj_n = 1, .offset = 0x00022fa8 });
	IDisasm.pushAddress(&(VirtualAddress){ .obj_n = 1, .offset = 0x00023010 });
	IDisasm.pushAddress(&(VirtualAddress){ .obj_n = 1, .offset = 0x00023094 });
	IDisasm.pushAddress(&(VirtualAddress){ .obj_n = 1, .offset = 0x00023118 });
	IDisasm.pushAddress(&(VirtualAddress){ .obj_n = 1, .offset = 0x000231a4 });
	IDisasm.pushAddress(&(VirtualAddress){ .obj_n = 1, .offset = 0x000231c8 });
	IDisasm.pushAddress(&(VirtualAddress){ .obj_n = 1, .offset = 0x000231f0 });
	IDisasm.pushAddress(&(VirtualAddress){ .obj_n = 1, .offset = 0x00023218 });
	IDisasm.pushAddress(&(VirtualAddress){ .obj_n = 1, .offset = 0x000232ac });
	IDisasm.pushAddress(&(VirtualAddress){ .obj_n = 1, .offset = 0x00023334 });
	IDisasm.pushAddress(&(VirtualAddress){ .obj_n = 1, .offset = 0x00021d40 });
	IDisasm.pushAddress(&(VirtualAddress){ .obj_n = 1, .offset = 0x000233bc });
	IDisasm.pushAddress(&(VirtualAddress){ .obj_n = 1, .offset = 0x00023524 });
	IDisasm.pushAddress(&(VirtualAddress){ .obj_n = 1, .offset = 0x0002379c });
	IDisasm.pushAddress(&(VirtualAddress){ .obj_n = 1, .offset = 0x00023bac });
	IDisasm.pushAddress(&(VirtualAddress){ .obj_n = 1, .offset = 0x00023838 });
	IDisasm.pushAddress(&(VirtualAddress){ .obj_n = 1, .offset = 0x000239e8 });
	IDisasm.pushAddress(&(VirtualAddress){ .obj_n = 1, .offset = 0x000235d0 });
	IDisasm.pushAddress(&(VirtualAddress){ .obj_n = 1, .offset = 0x00025bbc });
	IDisasm.pushAddress(&(VirtualAddress){ .obj_n = 1, .offset = 0x000259d4 });
	IDisasm.pushAddress(&(VirtualAddress){ .obj_n = 1, .offset = 0x000259f0 });
	IDisasm.pushAddress(&(VirtualAddress){ .obj_n = 1, .offset = 0x00025a14 });
	IDisasm.pushAddress(&(VirtualAddress){ .obj_n = 1, .offset = 0x00025a4c });
	IDisasm.pushAddress(&(VirtualAddress){ .obj_n = 1, .offset = 0x00025b94 });
	IDisasm.pushAddress(&(VirtualAddress){ .obj_n = 1, .offset = 0x00025abc });
	IDisasm.pushAddress(&(VirtualAddress){ .obj_n = 1, .offset = 0x00025af8 });
	IDisasm.pushAddress(&(VirtualAddress){ .obj_n = 1, .offset = 0x00025b34 });
	IDisasm.pushAddress(&(VirtualAddress){ .obj_n = 1, .offset = 0x00025b64 });
	IDisasm.pushAddress(&(VirtualAddress){ .obj_n = 1, .offset = 0x00064438 });
	IDisasm.pushAddress(&(VirtualAddress){ .obj_n = 1, .offset = 0x00040ba0 });
	IDisasm.pushAddress(&(VirtualAddress){ .obj_n = 1, .offset = 0x0002d220 });
	IDisasm.pushAddress(&(VirtualAddress){ .obj_n = 1, .offset = 0x0002e074 });
	IDisasm.pushAddress(&(VirtualAddress){ .obj_n = 1, .offset = 0x000302c8 });
	IDisasm.pushAddress(&(VirtualAddress){ .obj_n = 1, .offset = 0x0002e320 });
	IDisasm.pushAddress(&(VirtualAddress){ .obj_n = 1, .offset = 0x0002edac });
	IDisasm.pushAddress(&(VirtualAddress){ .obj_n = 1, .offset = 0x00030064 });
	IDisasm.pushAddress(&(VirtualAddress){ .obj_n = 1, .offset = 0x00042638 });
	IDisasm.pushAddress(&(VirtualAddress){ .obj_n = 1, .offset = 0x000430b4 });
	IDisasm.pushAddress(&(VirtualAddress){ .obj_n = 1, .offset = 0x000433f4 });
	IDisasm.pushAddress(&(VirtualAddress){ .obj_n = 1, .offset = 0x00021d40 });
	IDisasm.pushAddress(&(VirtualAddress){ .obj_n = 1, .offset = 0x00028730 });
	IDisasm.pushAddress(&(VirtualAddress){ .obj_n = 1, .offset = 0x00015174 });
	IDisasm.pushAddress(&(VirtualAddress){ .obj_n = 1, .offset = 0x000152f8 });
	IDisasm.pushAddress(&(VirtualAddress){ .obj_n = 1, .offset = 0x000148c4 });
	IDisasm.pushAddress(&(VirtualAddress){ .obj_n = 1, .offset = 0x000154a4 });
	IDisasm.pushAddress(&(VirtualAddress){ .obj_n = 1, .offset = 0x0003a40c });
	IDisasm.pushAddress(&(VirtualAddress){ .obj_n = 1, .offset = 0x00039ef8 });
	IDisasm.pushAddress(&(VirtualAddress){ .obj_n = 1, .offset = 0x0003a524 });
	IDisasm.pushAddress(&(VirtualAddress){ .obj_n = 1, .offset = 0x00000048 });
	IDisasm.pushAddress(&(VirtualAddress){ .obj_n = 1, .offset = 0x00061b9c });
	IDisasm.pushAddress(&(VirtualAddress){ .obj_n = 1, .offset = 0x00049864 });
	IDisasm.pushAddress(&(VirtualAddress){ .obj_n = 1, .offset = 0x0001da14 });
	IDisasm.pushAddress(&(VirtualAddress){ .obj_n = 1, .offset = 0x0001b374 });
	IDisasm.pushAddress(&(VirtualAddress){ .obj_n = 1, .offset = 0x0001b3bc });
	IDisasm.pushAddress(&(VirtualAddress){ .obj_n = 1, .offset = 0x0001b6a4 });
	IDisasm.pushAddress(&(VirtualAddress){ .obj_n = 1, .offset = 0x0001c674 });
	IDisasm.pushAddress(&(VirtualAddress){ .obj_n = 1, .offset = 0x0001c990 });
	IDisasm.pushAddress(&(VirtualAddress){ .obj_n = 1, .offset = 0x0001b7d4 });
	IDisasm.pushAddress(&(VirtualAddress){ .obj_n = 1, .offset = 0x0001c6e0 });
	IDisasm.pushAddress(&(VirtualAddress){ .obj_n = 1, .offset = 0x0001cad4 });
	IDisasm.pushAddress(&(VirtualAddress){ .obj_n = 1, .offset = 0x0001ba74 });
	IDisasm.pushAddress(&(VirtualAddress){ .obj_n = 1, .offset = 0x00068e38 });
	IDisasm.pushAddress(&(VirtualAddress){ .obj_n = 1, .offset = 0x0001b3fc });
	IDisasm.pushAddress(&(VirtualAddress){ .obj_n = 1, .offset = 0x0001e8ac });
	IDisasm.pushAddress(&(VirtualAddress){ .obj_n = 1, .offset = 0x0004cb70 });
	IDisasm.pushAddress(&(VirtualAddress){ .obj_n = 1, .offset = 0x0004fac0 });
	IDisasm.pushAddress(&(VirtualAddress){ .obj_n = 1, .offset = 0x0004cff8 });
	IDisasm.pushAddress(&(VirtualAddress){ .obj_n = 1, .offset = 0x00051154 });
	IDisasm.pushAddress(&(VirtualAddress){ .obj_n = 1, .offset = 0x0004cc6c });
	IDisasm.pushAddress(&(VirtualAddress){ .obj_n = 1, .offset = 0x00054b70 });


	IDisasm.pushAddress(&(VirtualAddress){ .obj_n = 1, .offset = 0xda0 });
	IDisasm.pushAddress(&(VirtualAddress){ .obj_n = 1, .offset = 0xea0 });
	IDisasm.pushAddress(&(VirtualAddress){ .obj_n = 1, .offset = 0xef0 });
	IDisasm.pushAddress(&(VirtualAddress){ .obj_n = 1, .offset = 0x4068 });
	IDisasm.pushAddress(&(VirtualAddress){ .obj_n = 1, .offset = 0x5470 });


	IDisasm.pushAddress(&(VirtualAddress){ .obj_n = 1, .offset = 0 });
	IDisasm.pushAddress(&(VirtualAddress){ .obj_n = 2, .offset = 0 });





	IDisasm.pushAddress(&(VirtualAddress){ .obj_n = 1, .offset = 0x81e0 });
	IDisasm.pushAddress(&(VirtualAddress){ .obj_n = 1, .offset = 0x8310 });
	IDisasm.pushAddress(&(VirtualAddress){ .obj_n = 1, .offset = 0x84c0 });
	IDisasm.pushAddress(&(VirtualAddress){ .obj_n = 1, .offset = 0x8510 });
	IDisasm.pushAddress(&(VirtualAddress){ .obj_n = 1, .offset = 0x8530 });
	IDisasm.pushAddress(&(VirtualAddress){ .obj_n = 1, .offset = 0xe72c });
	IDisasm.pushAddress(&(VirtualAddress){ .obj_n = 1, .offset = 0xe830 });
	IDisasm.pushAddress(&(VirtualAddress){ .obj_n = 1, .offset = 0xe850 });
	IDisasm.pushAddress(&(VirtualAddress){ .obj_n = 1, .offset = 0xe890 });
	IDisasm.pushAddress(&(VirtualAddress){ .obj_n = 1, .offset = 0xe8a0 });
	IDisasm.pushAddress(&(VirtualAddress){ .obj_n = 1, .offset = 0xe8ec });
	IDisasm.pushAddress(&(VirtualAddress){ .obj_n = 1, .offset = 0xeaf0 });
	IDisasm.pushAddress(&(VirtualAddress){ .obj_n = 1, .offset = 0x183e0 });
	IDisasm.pushAddress(&(VirtualAddress){ .obj_n = 1, .offset = 0x18734 });
	IDisasm.pushAddress(&(VirtualAddress){ .obj_n = 1, .offset = 0x188b8 });
	IDisasm.pushAddress(&(VirtualAddress){ .obj_n = 1, .offset = 0x188d4 });
	IDisasm.pushAddress(&(VirtualAddress){ .obj_n = 1, .offset = 0x188f4 });
	IDisasm.pushAddress(&(VirtualAddress){ .obj_n = 1, .offset = 0x2164c });
	IDisasm.pushAddress(&(VirtualAddress){ .obj_n = 1, .offset = 0x216d4 });
	IDisasm.pushAddress(&(VirtualAddress){ .obj_n = 1, .offset = 0x21778 });
	IDisasm.pushAddress(&(VirtualAddress){ .obj_n = 1, .offset = 0x21790 });
	IDisasm.pushAddress(&(VirtualAddress){ .obj_n = 1, .offset = 0x219b0 });
	IDisasm.pushAddress(&(VirtualAddress){ .obj_n = 1, .offset = 0x284ec });
	IDisasm.pushAddress(&(VirtualAddress){ .obj_n = 1, .offset = 0x28c00 });
	IDisasm.pushAddress(&(VirtualAddress){ .obj_n = 1, .offset = 0x28f88 });
	IDisasm.pushAddress(&(VirtualAddress){ .obj_n = 1, .offset = 0x2a430 });
	IDisasm.pushAddress(&(VirtualAddress){ .obj_n = 1, .offset = 0x2a460 });
	IDisasm.pushAddress(&(VirtualAddress){ .obj_n = 1, .offset = 0x2a4a0 });
	IDisasm.pushAddress(&(VirtualAddress){ .obj_n = 1, .offset = 0x2a4e0 });
	IDisasm.pushAddress(&(VirtualAddress){ .obj_n = 1, .offset = 0x2a580 });
	IDisasm.pushAddress(&(VirtualAddress){ .obj_n = 1, .offset = 0x2c45f });
	IDisasm.pushAddress(&(VirtualAddress){ .obj_n = 1, .offset = 0x310c4 });
	IDisasm.pushAddress(&(VirtualAddress){ .obj_n = 1, .offset = 0x32a2c });
	IDisasm.pushAddress(&(VirtualAddress){ .obj_n = 1, .offset = 0x34760 });
	IDisasm.pushAddress(&(VirtualAddress){ .obj_n = 1, .offset = 0x34920 });
	IDisasm.pushAddress(&(VirtualAddress){ .obj_n = 1, .offset = 0x3494c });
	IDisasm.pushAddress(&(VirtualAddress){ .obj_n = 1, .offset = 0x34980 });
	IDisasm.pushAddress(&(VirtualAddress){ .obj_n = 1, .offset = 0x349b4 });
	IDisasm.pushAddress(&(VirtualAddress){ .obj_n = 1, .offset = 0x349d8 });
	IDisasm.pushAddress(&(VirtualAddress){ .obj_n = 1, .offset = 0x34a1c });
	IDisasm.pushAddress(&(VirtualAddress){ .obj_n = 1, .offset = 0x34a64 });
	IDisasm.pushAddress(&(VirtualAddress){ .obj_n = 1, .offset = 0x34aa8 });
	IDisasm.pushAddress(&(VirtualAddress){ .obj_n = 1, .offset = 0x34af0 });
	IDisasm.pushAddress(&(VirtualAddress){ .obj_n = 1, .offset = 0x34bb0 });
	IDisasm.pushAddress(&(VirtualAddress){ .obj_n = 1, .offset = 0x34c74 });
	IDisasm.pushAddress(&(VirtualAddress){ .obj_n = 1, .offset = 0x34cbc });
	IDisasm.pushAddress(&(VirtualAddress){ .obj_n = 1, .offset = 0x34d08 });
	IDisasm.pushAddress(&(VirtualAddress){ .obj_n = 1, .offset = 0x34d6c });
	IDisasm.pushAddress(&(VirtualAddress){ .obj_n = 1, .offset = 0x34dd8 });
	IDisasm.pushAddress(&(VirtualAddress){ .obj_n = 1, .offset = 0x3a6d0 });
	IDisasm.pushAddress(&(VirtualAddress){ .obj_n = 1, .offset = 0x3a710 });
	IDisasm.pushAddress(&(VirtualAddress){ .obj_n = 1, .offset = 0x40b30 });
	IDisasm.pushAddress(&(VirtualAddress){ .obj_n = 1, .offset = 0x40b70 });
	IDisasm.pushAddress(&(VirtualAddress){ .obj_n = 1, .offset = 0x40c80 });
	IDisasm.pushAddress(&(VirtualAddress){ .obj_n = 1, .offset = 0x41ab0 });
	IDisasm.pushAddress(&(VirtualAddress){ .obj_n = 1, .offset = 0x41fa0 });
	IDisasm.pushAddress(&(VirtualAddress){ .obj_n = 1, .offset = 0x41fe0 });
	IDisasm.pushAddress(&(VirtualAddress){ .obj_n = 1, .offset = 0x43fd6 });
	IDisasm.pushAddress(&(VirtualAddress){ .obj_n = 1, .offset = 0x43fe6 });
	IDisasm.pushAddress(&(VirtualAddress){ .obj_n = 1, .offset = 0x4402d });
	IDisasm.pushAddress(&(VirtualAddress){ .obj_n = 1, .offset = 0x4405b });
	IDisasm.pushAddress(&(VirtualAddress){ .obj_n = 1, .offset = 0x4427b });
	IDisasm.pushAddress(&(VirtualAddress){ .obj_n = 1, .offset = 0x44296 });
	IDisasm.pushAddress(&(VirtualAddress){ .obj_n = 1, .offset = 0x442a1 });
	IDisasm.pushAddress(&(VirtualAddress){ .obj_n = 1, .offset = 0x442e4 });
	IDisasm.pushAddress(&(VirtualAddress){ .obj_n = 1, .offset = 0x44311 });
	IDisasm.pushAddress(&(VirtualAddress){ .obj_n = 1, .offset = 0x44358 });
	IDisasm.pushAddress(&(VirtualAddress){ .obj_n = 1, .offset = 0x443b2 });
	IDisasm.pushAddress(&(VirtualAddress){ .obj_n = 1, .offset = 0x44400 });
	IDisasm.pushAddress(&(VirtualAddress){ .obj_n = 1, .offset = 0x44447 });
	IDisasm.pushAddress(&(VirtualAddress){ .obj_n = 1, .offset = 0x444c1 });
	IDisasm.pushAddress(&(VirtualAddress){ .obj_n = 1, .offset = 0x489c0 });
	IDisasm.pushAddress(&(VirtualAddress){ .obj_n = 1, .offset = 0x48a00 });
	IDisasm.pushAddress(&(VirtualAddress){ .obj_n = 1, .offset = 0x48a70 });
	IDisasm.pushAddress(&(VirtualAddress){ .obj_n = 1, .offset = 0x48a90 });
	IDisasm.pushAddress(&(VirtualAddress){ .obj_n = 1, .offset = 0x48dd8 });
	IDisasm.pushAddress(&(VirtualAddress){ .obj_n = 1, .offset = 0x4a820 });
	IDisasm.pushAddress(&(VirtualAddress){ .obj_n = 1, .offset = 0x4a880 });
	IDisasm.pushAddress(&(VirtualAddress){ .obj_n = 1, .offset = 0x4a8c0 });
	IDisasm.pushAddress(&(VirtualAddress){ .obj_n = 1, .offset = 0x4c914 });
	IDisasm.pushAddress(&(VirtualAddress){ .obj_n = 1, .offset = 0x513d0 });
	IDisasm.pushAddress(&(VirtualAddress){ .obj_n = 1, .offset = 0x513e0 });
	IDisasm.pushAddress(&(VirtualAddress){ .obj_n = 1, .offset = 0x513f0 });
	IDisasm.pushAddress(&(VirtualAddress){ .obj_n = 1, .offset = 0x51400 });
	IDisasm.pushAddress(&(VirtualAddress){ .obj_n = 1, .offset = 0x53110 });
	IDisasm.pushAddress(&(VirtualAddress){ .obj_n = 1, .offset = 0x54a30 });
	IDisasm.pushAddress(&(VirtualAddress){ .obj_n = 1, .offset = 0x54b20 });
	IDisasm.pushAddress(&(VirtualAddress){ .obj_n = 1, .offset = 0x56496 });
	IDisasm.pushAddress(&(VirtualAddress){ .obj_n = 1, .offset = 0x564a5 });
	IDisasm.pushAddress(&(VirtualAddress){ .obj_n = 1, .offset = 0x564b4 });
	IDisasm.pushAddress(&(VirtualAddress){ .obj_n = 1, .offset = 0x564c3 });
	IDisasm.pushAddress(&(VirtualAddress){ .obj_n = 1, .offset = 0x564d4 });
	IDisasm.pushAddress(&(VirtualAddress){ .obj_n = 1, .offset = 0x564e5 });
	IDisasm.pushAddress(&(VirtualAddress){ .obj_n = 1, .offset = 0x564f4 });
	IDisasm.pushAddress(&(VirtualAddress){ .obj_n = 1, .offset = 0x56505 });
	IDisasm.pushAddress(&(VirtualAddress){ .obj_n = 1, .offset = 0x5665e });
	IDisasm.pushAddress(&(VirtualAddress){ .obj_n = 1, .offset = 0x56669 });
	IDisasm.pushAddress(&(VirtualAddress){ .obj_n = 1, .offset = 0x56672 });
	IDisasm.pushAddress(&(VirtualAddress){ .obj_n = 1, .offset = 0x56765 });
	IDisasm.pushAddress(&(VirtualAddress){ .obj_n = 1, .offset = 0x56770 });
	IDisasm.pushAddress(&(VirtualAddress){ .obj_n = 1, .offset = 0x56779 });
	IDisasm.pushAddress(&(VirtualAddress){ .obj_n = 1, .offset = 0x5bea8 });
	IDisasm.pushAddress(&(VirtualAddress){ .obj_n = 1, .offset = 0x62b99 });
	IDisasm.pushAddress(&(VirtualAddress){ .obj_n = 1, .offset = 0x62b9e });
	IDisasm.pushAddress(&(VirtualAddress){ .obj_n = 1, .offset = 0x62ba3 });
	IDisasm.pushAddress(&(VirtualAddress){ .obj_n = 1, .offset = 0x62ba8 });
	IDisasm.pushAddress(&(VirtualAddress){ .obj_n = 1, .offset = 0x62baf });
	IDisasm.pushAddress(&(VirtualAddress){ .obj_n = 1, .offset = 0x6a430 });
	IDisasm.pushAddress(&(VirtualAddress){ .obj_n = 1, .offset = 0x6a530 });
	IDisasm.pushAddress(&(VirtualAddress){ .obj_n = 1, .offset = 0x6a8c8 });
	IDisasm.pushAddress(&(VirtualAddress){ .obj_n = 1, .offset = 0x6a8db });
	IDisasm.pushAddress(&(VirtualAddress){ .obj_n = 1, .offset = 0x6a9ed });
	IDisasm.pushAddress(&(VirtualAddress){ .obj_n = 1, .offset = 0x6aa0f });
	IDisasm.pushAddress(&(VirtualAddress){ .obj_n = 1, .offset = 0x6ab49 });
	IDisasm.pushAddress(&(VirtualAddress){ .obj_n = 1, .offset = 0x6abcf });
	IDisasm.pushAddress(&(VirtualAddress){ .obj_n = 1, .offset = 0x6ac87 });
	IDisasm.pushAddress(&(VirtualAddress){ .obj_n = 1, .offset = 0x6b60f });
	IDisasm.pushAddress(&(VirtualAddress){ .obj_n = 1, .offset = 0x6b61a });
	IDisasm.pushAddress(&(VirtualAddress){ .obj_n = 1, .offset = 0x6b71f });
	IDisasm.pushAddress(&(VirtualAddress){ .obj_n = 1, .offset = 0x6b727 });
	IDisasm.pushAddress(&(VirtualAddress){ .obj_n = 1, .offset = 0x6b737 });
	IDisasm.pushAddress(&(VirtualAddress){ .obj_n = 1, .offset = 0x6c9ec });
	IDisasm.pushAddress(&(VirtualAddress){ .obj_n = 1, .offset = 0x6cd30 });
	IDisasm.pushAddress(&(VirtualAddress){ .obj_n = 1, .offset = 0x6e6d2 });
	IDisasm.pushAddress(&(VirtualAddress){ .obj_n = 1, .offset = 0x6e9c8 });
	IDisasm.pushAddress(&(VirtualAddress){ .obj_n = 1, .offset = 0x6ea26 });
	IDisasm.pushAddress(&(VirtualAddress){ .obj_n = 1, .offset = 0x6ea37 });
	IDisasm.pushAddress(&(VirtualAddress){ .obj_n = 1, .offset = 0x6ea43 });
	IDisasm.pushAddress(&(VirtualAddress){ .obj_n = 1, .offset = 0x6ea4a });
	IDisasm.pushAddress(&(VirtualAddress){ .obj_n = 1, .offset = 0x6ea51 });
	IDisasm.pushAddress(&(VirtualAddress){ .obj_n = 1, .offset = 0x70246 });
	IDisasm.pushAddress(&(VirtualAddress){ .obj_n = 1, .offset = 0x7027c });
	IDisasm.pushAddress(&(VirtualAddress){ .obj_n = 1, .offset = 0x702a1 });
	IDisasm.pushAddress(&(VirtualAddress){ .obj_n = 1, .offset = 0x702fb });
	IDisasm.pushAddress(&(VirtualAddress){ .obj_n = 1, .offset = 0x70318 });
	IDisasm.pushAddress(&(VirtualAddress){ .obj_n = 1, .offset = 0x70331 });
	IDisasm.pushAddress(&(VirtualAddress){ .obj_n = 1, .offset = 0x70339 });
	IDisasm.pushAddress(&(VirtualAddress){ .obj_n = 1, .offset = 0x70379 });
	IDisasm.pushAddress(&(VirtualAddress){ .obj_n = 1, .offset = 0x703b9 });
	IDisasm.pushAddress(&(VirtualAddress){ .obj_n = 1, .offset = 0x70404 });
	IDisasm.pushAddress(&(VirtualAddress){ .obj_n = 1, .offset = 0x7043b });
	IDisasm.pushAddress(&(VirtualAddress){ .obj_n = 1, .offset = 0x7046f });
	IDisasm.pushAddress(&(VirtualAddress){ .obj_n = 1, .offset = 0x7047b });
	IDisasm.pushAddress(&(VirtualAddress){ .obj_n = 1, .offset = 0x7048b });
	IDisasm.pushAddress(&(VirtualAddress){ .obj_n = 1, .offset = 0x7049d });
	IDisasm.pushAddress(&(VirtualAddress){ .obj_n = 1, .offset = 0x704a4 });
	IDisasm.pushAddress(&(VirtualAddress){ .obj_n = 1, .offset = 0x704b0 });
	IDisasm.pushAddress(&(VirtualAddress){ .obj_n = 1, .offset = 0x704d2 });
	IDisasm.pushAddress(&(VirtualAddress){ .obj_n = 1, .offset = 0x704f8 });
	IDisasm.pushAddress(&(VirtualAddress){ .obj_n = 1, .offset = 0x70501 });
	IDisasm.pushAddress(&(VirtualAddress){ .obj_n = 1, .offset = 0x70643 });
	IDisasm.pushAddress(&(VirtualAddress){ .obj_n = 1, .offset = 0x70680 });
	IDisasm.pushAddress(&(VirtualAddress){ .obj_n = 1, .offset = 0x706af });
	IDisasm.pushAddress(&(VirtualAddress){ .obj_n = 1, .offset = 0x706e7 });
	IDisasm.pushAddress(&(VirtualAddress){ .obj_n = 1, .offset = 0x70715 });
	IDisasm.pushAddress(&(VirtualAddress){ .obj_n = 1, .offset = 0x70728 });
	IDisasm.pushAddress(&(VirtualAddress){ .obj_n = 1, .offset = 0x70730 });
	IDisasm.pushAddress(&(VirtualAddress){ .obj_n = 1, .offset = 0x70770 });
	IDisasm.pushAddress(&(VirtualAddress){ .obj_n = 1, .offset = 0x70795 });
	IDisasm.pushAddress(&(VirtualAddress){ .obj_n = 1, .offset = 0x707b8 });
	IDisasm.pushAddress(&(VirtualAddress){ .obj_n = 1, .offset = 0x707ff });
	IDisasm.pushAddress(&(VirtualAddress){ .obj_n = 1, .offset = 0x70821 });
	IDisasm.pushAddress(&(VirtualAddress){ .obj_n = 1, .offset = 0x70830 });
	IDisasm.pushAddress(&(VirtualAddress){ .obj_n = 1, .offset = 0x70b12 });
	IDisasm.pushAddress(&(VirtualAddress){ .obj_n = 1, .offset = 0x70c0a });
	IDisasm.pushAddress(&(VirtualAddress){ .obj_n = 1, .offset = 0x70c68 });
	IDisasm.pushAddress(&(VirtualAddress){ .obj_n = 1, .offset = 0x70c85 });
	IDisasm.pushAddress(&(VirtualAddress){ .obj_n = 1, .offset = 0x70c93 });
	IDisasm.pushAddress(&(VirtualAddress){ .obj_n = 1, .offset = 0x70cab });
	IDisasm.pushAddress(&(VirtualAddress){ .obj_n = 1, .offset = 0x70cc7 });
	IDisasm.pushAddress(&(VirtualAddress){ .obj_n = 1, .offset = 0x70cd4 });
	IDisasm.pushAddress(&(VirtualAddress){ .obj_n = 1, .offset = 0x70ce0 });
	IDisasm.pushAddress(&(VirtualAddress){ .obj_n = 1, .offset = 0x70d02 });
	IDisasm.pushAddress(&(VirtualAddress){ .obj_n = 1, .offset = 0x70d15 });
	IDisasm.pushAddress(&(VirtualAddress){ .obj_n = 1, .offset = 0x70d21 });
	IDisasm.pushAddress(&(VirtualAddress){ .obj_n = 1, .offset = 0x70d39 });
	IDisasm.pushAddress(&(VirtualAddress){ .obj_n = 1, .offset = 0x70d48 });
	IDisasm.pushAddress(&(VirtualAddress){ .obj_n = 1, .offset = 0x71545 });
	IDisasm.pushAddress(&(VirtualAddress){ .obj_n = 1, .offset = 0x715ba });
	IDisasm.pushAddress(&(VirtualAddress){ .obj_n = 1, .offset = 0x715c7 });
	IDisasm.pushAddress(&(VirtualAddress){ .obj_n = 1, .offset = 0x718a4 });
	IDisasm.pushAddress(&(VirtualAddress){ .obj_n = 1, .offset = 0x71b20 });
	IDisasm.pushAddress(&(VirtualAddress){ .obj_n = 1, .offset = 0x71b2c });
	IDisasm.pushAddress(&(VirtualAddress){ .obj_n = 1, .offset = 0x71d5b });
	IDisasm.pushAddress(&(VirtualAddress){ .obj_n = 1, .offset = 0x71efa });
	IDisasm.pushAddress(&(VirtualAddress){ .obj_n = 1, .offset = 0x71f8f });
	IDisasm.pushAddress(&(VirtualAddress){ .obj_n = 1, .offset = 0x71fcc });
	IDisasm.pushAddress(&(VirtualAddress){ .obj_n = 1, .offset = 0x71fe7 });
	IDisasm.pushAddress(&(VirtualAddress){ .obj_n = 1, .offset = 0x7204e });
	IDisasm.pushAddress(&(VirtualAddress){ .obj_n = 1, .offset = 0x720b5 });
	IDisasm.pushAddress(&(VirtualAddress){ .obj_n = 1, .offset = 0x7211c });
	IDisasm.pushAddress(&(VirtualAddress){ .obj_n = 1, .offset = 0x72171 });
	IDisasm.pushAddress(&(VirtualAddress){ .obj_n = 1, .offset = 0x72184 });
	IDisasm.pushAddress(&(VirtualAddress){ .obj_n = 1, .offset = 0x72430 });
	IDisasm.pushAddress(&(VirtualAddress){ .obj_n = 1, .offset = 0x72900 });
	IDisasm.pushAddress(&(VirtualAddress){ .obj_n = 1, .offset = 0x72934 });
	IDisasm.pushAddress(&(VirtualAddress){ .obj_n = 1, .offset = 0x7297c });
	IDisasm.pushAddress(&(VirtualAddress){ .obj_n = 1, .offset = 0x72980 });
	IDisasm.pushAddress(&(VirtualAddress){ .obj_n = 1, .offset = 0x729b8 });
	IDisasm.pushAddress(&(VirtualAddress){ .obj_n = 1, .offset = 0x72a35 });
	IDisasm.pushAddress(&(VirtualAddress){ .obj_n = 1, .offset = 0x72a4e });
	IDisasm.pushAddress(&(VirtualAddress){ .obj_n = 1, .offset = 0x72ab0 });
	IDisasm.pushAddress(&(VirtualAddress){ .obj_n = 1, .offset = 0x72ad3 });
	IDisasm.pushAddress(&(VirtualAddress){ .obj_n = 1, .offset = 0x72b40 });
	IDisasm.pushAddress(&(VirtualAddress){ .obj_n = 1, .offset = 0x72b4d });
	IDisasm.pushAddress(&(VirtualAddress){ .obj_n = 1, .offset = 0x72bb2 });
	IDisasm.pushAddress(&(VirtualAddress){ .obj_n = 1, .offset = 0x72bbf });


	pushJumpTable2(
		&(VirtualAddress){ .obj_n = 3, .offset = 0x6674 },
		&(VirtualAddress){ .obj_n = 3, .offset = 0x6a74 }
	);


	// objects
	pushObject(&(VirtualAddress){ .obj_n = 1, .offset = 0x0de0 });
	pushObject(&(VirtualAddress){ .obj_n = 1, .offset = 0x0e00 });
	pushObject(&(VirtualAddress){ .obj_n = 1, .offset = 0x0e30 });

	pushObject(&(VirtualAddress){ .obj_n = 1, .offset = 0x43b0 });
	pushObject(&(VirtualAddress){ .obj_n = 1, .offset = 0x43e0 });
	pushObject(&(VirtualAddress){ .obj_n = 1, .offset = 0x4440 });
	pushObject(&(VirtualAddress){ .obj_n = 1, .offset = 0x44a0 });
	pushObject(&(VirtualAddress){ .obj_n = 1, .offset = 0x4500 });
	pushObject(&(VirtualAddress){ .obj_n = 1, .offset = 0x4560 });
	pushObject(&(VirtualAddress){ .obj_n = 1, .offset = 0x45c0 });
	pushObject(&(VirtualAddress){ .obj_n = 1, .offset = 0x4620 });
	pushObject(&(VirtualAddress){ .obj_n = 1, .offset = 0x4680 });
	pushObject(&(VirtualAddress){ .obj_n = 1, .offset = 0x46e0 });
	pushObject(&(VirtualAddress){ .obj_n = 1, .offset = 0x4740 });
	pushObject(&(VirtualAddress){ .obj_n = 1, .offset = 0x47a0 });
	pushObject(&(VirtualAddress){ .obj_n = 1, .offset = 0x4800 });
	pushObject(&(VirtualAddress){ .obj_n = 1, .offset = 0x4860 });
	pushObject(&(VirtualAddress){ .obj_n = 1, .offset = 0x48c0 });
	pushObject(&(VirtualAddress){ .obj_n = 1, .offset = 0x4920 });
	pushObject(&(VirtualAddress){ .obj_n = 1, .offset = 0x4980 });
	pushObject(&(VirtualAddress){ .obj_n = 1, .offset = 0x49e0 });
	pushObject(&(VirtualAddress){ .obj_n = 1, .offset = 0x4a40 });

	pushObject(&(VirtualAddress){ .obj_n = 1, .offset = 0x55d0 });
	pushObject(&(VirtualAddress){ .obj_n = 1, .offset = 0x55f0 });
	pushObject(&(VirtualAddress){ .obj_n = 1, .offset = 0x5610 });
	pushObject(&(VirtualAddress){ .obj_n = 1, .offset = 0x5630 });
	pushObject(&(VirtualAddress){ .obj_n = 1, .offset = 0x5650 });
	pushObject(&(VirtualAddress){ .obj_n = 1, .offset = 0x5670 });

	pushObject(&(VirtualAddress){ .obj_n = 1, .offset = 0x8220 });
	pushObject(&(VirtualAddress){ .obj_n = 1, .offset = 0x8240 });

	pushObject(&(VirtualAddress){ .obj_n = 1, .offset = 0x10230 });
	pushObject(&(VirtualAddress){ .obj_n = 1, .offset = 0x10290 });
	pushObject(&(VirtualAddress){ .obj_n = 1, .offset = 0x102b0	});

	pushObject(&(VirtualAddress){ .obj_n = 1, .offset = 0x14720 });
	pushObject(&(VirtualAddress){ .obj_n = 1, .offset = 0x14760 });
	pushObject(&(VirtualAddress){ .obj_n = 1, .offset = 0x147a0	});

	pushObject(&(VirtualAddress){ .obj_n = 1, .offset = 0x18420 });
	pushObject(&(VirtualAddress){ .obj_n = 1, .offset = 0x18470 });
	pushObject(&(VirtualAddress){ .obj_n = 1, .offset = 0x184a0	});

	pushObject(&(VirtualAddress){ .obj_n = 1, .offset = 0x187c0	});
	pushObject(&(VirtualAddress){ .obj_n = 1, .offset = 0x20630	});
	pushObject(&(VirtualAddress){ .obj_n = 1, .offset = 0x21120	});
	pushObject(&(VirtualAddress){ .obj_n = 1, .offset = 0x21190	});
	pushObject(&(VirtualAddress){ .obj_n = 1, .offset = 0x21cf0	});
	pushObject(&(VirtualAddress){ .obj_n = 1, .offset = 0x21d10	});
	pushObject(&(VirtualAddress){ .obj_n = 1, .offset = 0x25f70	});
	pushObject(&(VirtualAddress){ .obj_n = 1, .offset = 0x28bd0	});
	pushObject(&(VirtualAddress){ .obj_n = 1, .offset = 0x2a440	});
	pushObject(&(VirtualAddress){ .obj_n = 1, .offset = 0x31430	});
	pushObject(&(VirtualAddress){ .obj_n = 1, .offset = 0x33860	});
	pushObject(&(VirtualAddress){ .obj_n = 1, .offset = 0x339b0	});
	pushObject(&(VirtualAddress){ .obj_n = 1, .offset = 0x345f0	});
	pushObject(&(VirtualAddress){ .obj_n = 1, .offset = 0x34610	});
	pushObject(&(VirtualAddress){ .obj_n = 1, .offset = 0x3bd60	});
	pushObject(&(VirtualAddress){ .obj_n = 1, .offset = 0x41a50	});
	pushObject(&(VirtualAddress){ .obj_n = 1, .offset = 0x4a860	});
	pushObject(&(VirtualAddress){ .obj_n = 1, .offset = 0x4d0c0	});
	pushObject(&(VirtualAddress){ .obj_n = 1, .offset = 0x4e260	});
	pushObject(&(VirtualAddress){ .obj_n = 1, .offset = 0x51850	});
	pushObject(&(VirtualAddress){ .obj_n = 1, .offset = 0x51de0	});
	pushObject(&(VirtualAddress){ .obj_n = 1, .offset = 0x51e10	});
	pushObject(&(VirtualAddress){ .obj_n = 1, .offset = 0x51e70	});
	pushObject(&(VirtualAddress){ .obj_n = 1, .offset = 0x54890	});
	pushObject(&(VirtualAddress){ .obj_n = 1, .offset = 0x61840	});


	// start object ?
	pushObject(
		&(VirtualAddress){ .obj_n = 1, .offset = 0x51e10 });


/*
	le_createLabel(4, 0x1a294);

	markAsData(
		&(VirtualAddress){ .obj_n = 1, .offset = 0x3 },
		&(VirtualAddress){ .obj_n = 1, .offset = 0xf });
*/

	IDisasm.disassemble();
	checkInfixLabels();

	// output

	for(object_n = 1; object_n <= le_getNumberOfObjects(); object_n++){
		
		bin2inc_2(object_n, getObjectMapName(object_n));
	}

#ifdef GLOBAL_STATS
	global_timer = timeDiff(&__global_timer);
	printf("[bin2inc] total time %s\n", formatTime(global_timer));
#endif

	IDisasm.close();
	fclose(fixup_fd);
	//remove("$__fixups.tmp");

    return 0;
}
