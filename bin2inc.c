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
		lname(0x0006eb20, "__CMain");
		lname(0x0006eb70, "__InitRtns");
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
	// #__InitRtns
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




	IDisasm.pushAddress(&(VirtualAddress){ .obj_n = 1, .offset = 0 });
	IDisasm.pushAddress(&(VirtualAddress){ .obj_n = 2, .offset = 0 });
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
