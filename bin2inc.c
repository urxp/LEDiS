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
const char * GLOBAL_LABEL_PREFIX = "dr@";

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


	if(obj == 1){ // RALLY.LE specific
		switch(offset){
		lname(0x00000003, "___begtext");
		lname(0x0000018c, "main_");
		lname(0x0004ea14, "__CHK");
		lname(0x0004ea24, "__GRO");
		lname(0x0004ea27, "__STK");
		lname(0x0004ea45, "__STKOVERFLOW");
		lname(0x0004ea54, "stricmp");
		lname(0x0004ea9b, "printf");
		lname(0x0004eabd, "getenv");
		lname(0x0004eb1e, "_null_exit_rtn");
		lname(0x0004eb1f, "exit_");
		lname(0x0004eb37, "_exit");
		lname(0x0004eb4d, "int386");
		lname(0x00053d0c, "ConsoleName");
		lname(0x00053d10, "around");
		lname(0x00053d92, "not_pharlap");
		lname(0x00053dca, "know_ext1");
		lname(0x00053dcc, "not_intel");
		lname(0x00053de9, "rat9");
		lname(0x00053dfd, "rat10");
		lname(0x00053e25, "know_extender");
		lname(0x00053e0e, "not_DOS4G");
		lname(0x00053e7a, "noparm");
		lname(0x00053ef0, "zerobss");
		lname(0x00053f27, "__exit_");
		lname(0x00053f29, "__do_exit_with_msg__");
		lname(0x00053f4c, "ok");
		lname(0x00053f64, "__GETDS");
		lname(0x00053f6d, "__saved_DS");
		lname(0x00053f6f, "___GETDSEnd_");
		lname(0x00053f70, "__open_flags");
		lname(0x00054041, "__doopen");
		lname(0x00054105, "_fsopen");
		lname(0x0005413e, "fopen");
		lname(0x00054148, "close_file");
		lname(0x000541b0, "freopen");
		lname(0x000541f2, "fread");
		lname(0x000543cd, "fclose");
		lname(0x000543fa, "__shutdown_stream");
		lname(0x0005440f, "__hex");
		lname(0x0005441b, "__MkTmpFile");
		lname(0x00054480, "__doclose");
		lname(0x00054510, "strupr");
		lname(0x00054540, "strcmp");
		lname(0x000545e1, "__update_buffer");
		lname(0x00054623, "fseek");
		lname(0x000559c8, "fgetc");
		lname(0x00055a51, "__filbuf");
		lname(0x00055a80, "__fill_buffer");
		lname(0x00055b2c, "outp");
		lname(0x00055b36, "mem_putc");
		lname(0x00055b49, "vsprintf");
		lname(0x00055b5e, "fwrite");
		lname(0x00055d1e, "__CHP");
		lname(0x00055d40, "memset");
		lname(0x00055d7d, "utoa");
		lname(0x00055dc7, "itoa");
		lname(0x00055df0, "__STOSB");
		lname(0x00055e27, "__STOSD");
		lname(0x00055e93, "initrandnext");
		lname(0x00055e99, "rand");
		lname(0x00055ebd, "srand");
		lname(0x00055ecd, "atoi");
		lname(0x00055f3a, "cget_string");
		lname(0x00055f63, "uncget_string");
		lname(0x00055f67, "vsscanf");
		lname(0x00055f8e, "sscanf");
		lname(0x00055faf, "inp");
		lname(0x00055fb7, "ceil");
		lname(0x00055fd2, "int386x");
		lname(0x00055feb, "fputc");
		lname(0x0005608f, "open");
		lname(0x000560b3, "sopen");
		lname(0x0005629c, "__set_binary");
		lname(0x00056301, "filelength");
		lname(0x0005633e, "close");
		lname(0x0005635e, "strlwr");
		lname(0x00056383, "cget_file");
		lname(0x000563d7, "uncget_file");
		lname(0x000563df, "vfscanf");
		lname(0x00056406, "fscanf");
		lname(0x00056427, "malloc");
		lname(0x0005650a, "free");
		lname(0x0005658c, "__init_387_emulator");
		lname(0x00056591, "__fini_387_emulator");
		lname(0x00056596, "cget_stdin");
		lname(0x000565f2, "uncget_stdin");
		lname(0x000565fc, "vscanf");
		lname(0x00056623, "scanf");
		lname(0x0005663e, "kbhit_");
		lname(0x00056655, "_gettextposition");
		lname(0x00056687, "_settextposition");
		lname(0x0005671f, "putchar");
		lname(0x00056799, "strlen");
		lname(0x000567b2, "putch");
		lname(0x00056e84, "__exit_with_msg");
		lname(0x00056ea6, "__null_FPE_rtn");
		lname(0x00056ea7, "file_putc");
		lname(0x00056e89, "__fatal_runtime_error");
		lname(0x00056eb8, "__fprtf");
		lname(0x00056f38, "strnicmp");
		lname(0x00056f8b, "segread");
		lname(0x00056fb4, "_dos_getdate");
		lname(0x00056fcf, "_dos_gettime");
		lname(0x00056fe9, "memcpy");
		lname(0x0005700e, "_fmemcpy");
		lname(0x00057036, "strcpy");
		lname(0x00057055, "strcat");
		lname(0x00057146, "__CMain");
		lname(0x00057196, "__InitRtns");
		lname(0x000571e1, "__FiniRtns");
		lname(0x00057230, "__Init_Argv");
		lname(0x000572f1, "_SplitParms");
		lname(0x00057400, "tolower");
		lname(0x0005740e, "__set_EDOM");
		lname(0x00057413, "__set_errno");
		lname(0x0005741f, "__set_ERANGE");
		lname(0x00057426, "__set_EINVAL");
		lname(0x00057436, "__set_doserrno");
		lname(0x00057442, "__allocfp");
		lname(0x000574e8, "__freefp");
		lname(0x00057521, "__purgefp");
		lname(0x0005753f, "__chktty");
		lname(0x00057570, "__ioalloc");
		lname(0x000575e7, "__qread");
		lname(0x00057606, "getpid");
		lname(0x0005760c, "__flush");
		lname(0x000576e5, "ftell");
		lname(0x00057727, "lseek");
		lname(0x0005779f, "remove");
		lname(0x0005776a, "__close");
		lname(0x000577a4, "tell");
		lname(0x0005a151, "dr_int9h_handler");
		lname(0x0005bda9, "flushall");
		lname(0x0005bdae, "__flushall");
		lname(0x0005bddd, "getche");
		lname(0x0005bdfa, "__prtf");
		lname(0x0005c199, "getprintspecs");
		lname(0x0005c2d7, "evalflags");
		lname(0x0005c336, "far_strlen");
		lname(0x0005c35a, "far_other_strlen");
		lname(0x0005c37d, "fmt4hex");
		lname(0x0005c3db, "FixedPoint_Format");
		lname(0x0005c4d0, "float_format");
		lname(0x0005c4d7, "formstring");
		lname(0x0005c9c0, "zupstr");
		lname(0x0005c9d8, "__qwrite");
		lname(0x0005ca4c, "__get_errno_ptr");
		lname(0x0005ca52, "__get_doserrno_ptr");
		lname(0x0005ca58, "cget");
		lname(0x0005ca5f, "uncget");
		lname(0x0005ca67, "__scnf");
		lname(0x0005ccae, "get_opt");
		lname(0x0005cda3, "scan_white");
		lname(0x0005cde2, "scan_char");
		lname(0x0005ce77, "scan_string");
		lname(0x0005cf9a, "report_scan");
		lname(0x0005cff8, "makelist");
		lname(0x0005d03b, "scan_arb");
		lname(0x0005d135, "scan_float");
		lname(0x0005d44c, "scan_int");
		lname(0x0005d63c, "radix_value");
		lname(0x0005d663, "cgetw");
		lname(0x0005d689, "floor");
		lname(0x0005d6c6, "__int386x");
		lname(0x0005d723, "_DoINTR");
		lname(0x0005daa4, "isatty");
		lname(0x0005dacb, "_dosret0");
		lname(0x0005dad2, "_dosretax");
		lname(0x0005dad6, "__set_errno_dos");
		lname(0x0005db47, "__IOMode");
		lname(0x0005db9c, "__SetIOMode");
		lname(0x0005dbb1, "ungetc");
		lname(0x0005dc34, "__MemAllocator");
		lname(0x0005dcdc, "__MemFree");
		lname(0x0005dde7, "__unlink");
		lname(0x0005de38, "__FreeDPMIBlocks");
		lname(0x0005de94, "__ReAllocDPMIBlock");
		lname(0x0005df93, "__LinkUpNewMHeap");
		lname(0x0005e007, "__LastFree");
		lname(0x0005e057, "RationalAlloc");
		lname(0x0005e128, "__CreateNewNHeap");
		lname(0x0005e1d8, "__ExpandDGROUP");
		lname(0x0005e324, "__AdjustAmount");
		lname(0x0005e3a1, "__nmemneed");
		lname(0x0005e3a4, "__setEFGfmt");
		lname(0x0005e3c0, "__sys_init_387_emulator");
		lname(0x0005e545, "__sys_fini_387_emulator");
		lname(0x0005e61b, "__save_8087");
		lname(0x0005e620, "__rest_8087");
		lname(0x0005e624, "__init_8087");
		lname(0x0005e655, "_fpreset");
		lname(0x0005e65f, "__chk8087_");
		lname(0x0005e6a9, "_getvideoconfig");
		lname(0x0005e6cb, "_CalcNumPages");
		lname(0x0005e740, "_GetState");
		lname(0x0005e86c, "_InitState");
		lname(0x0005e8f5, "_GrInit");
		lname(0x0005e966, "_CursorOn");
		lname(0x0005e994, "_CursorOff");
		lname(0x0005e9fd, "TextCursor");
		lname(0x0005ea22, "GraphCursor");
		lname(0x0005eac1, "_GraphMode");
		lname(0x0005eae5, "_GrProlog");
		lname(0x0005eb09, "_GrEpilog");
		lname(0x0005eb1a, "_InitDevice");
		lname(0x0005eb26, "_FiniDevice");
		lname(0x0005eb2f, "_StartDevice");
		lname(0x0005eb38, "_ResetDevice");
		lname(0x0005eb41, "toupper");
		lname(0x0005eb4f, "__EnterWVIDEO");
		lname(0x0005eb75, "__setenvp");
		lname(0x0005ec49, "stackavail_");
		lname(0x0005ec52, "__CommonInit_");
		lname(0x0005ecbb, "__InitFiles");
		lname(0x0005ed32, "__full_io_exit");
		lname(0x0005ed3e, "fcloseall");
		lname(0x0005ed43, "docloseall");
		lname(0x0005ed9b, "fflush");
		lname(0x0005edab, "unlink");
		lname(0x0006812a, "_no_support_loaded_");
		lname(0x0006813c, "_Alphabet");
		lname(0x00068161, "ultoa");
		lname(0x000681ab, "ltoa");
		lname(0x000681c6, "__FDFS");
		lname(0x00068212, "modf");
		lname(0x0006822b, "_heapenable_");
		lname(0x0006823b, "sbrk");
		lname(0x00068303, "__brk");
		lname(0x000683ce, "_EFG_Format");
		lname(0x000684b8, "forcedecpt");
		lname(0x00068517, "__cnvs2d");
		lname(0x000688a0, "__int7_pl3");
		lname(0x000688c4, "__int7_X32VM");
		lname(0x00068910, "__int7");
		lname(0x0006b74e, "__hook387");
		lname(0x0006b822, "__unhook387");
		lname(0x0006b87c, "__init_80x87");
		lname(0x0006b8a3, "_SysMonType");
		lname(0x0006b992, "DCCEmulate");
		lname(0x0006ba89, "CheckMONO");
		lname(0x0006bb0d, "CheckCGA");
		lname(0x0006bb22, "ChkCursorReg");
		lname(0x0006bb5c, "_InitSegments");
		lname(0x0006bc60, "_getplotaction");
		lname(0x0006bc8c, "_setplotaction");
		lname(0x0006bcdd, "_L1SLine");
		lname(0x0006bd1e, "_L1Line");
		lname(0x0006bdcf, "_L0Line");
		lname(0x0006bec4, "_L2setcolor");
		lname(0x0006be9f, "_getcolor");
		lname(0x0006bea6, "_setcolor");
		lname(0x0006bee2, "__int23_handler");
		lname(0x0006bf77, "__int_ctrl_break_handler");
		lname(0x0006bfef, "__DPMI_hosted_");
		lname(0x0006c083, "__restore_int23");
		lname(0x0006c234, "__restore_int");
		lname(0x0006c239, "__restore_int_ctrl_break");
		lname(0x0006c271, "__grab_int23");
		lname(0x0006c38c, "__grab_int_ctrl_break");
		lname(0x0006c4a7, "strncmp");
		lname(0x0006ca1d, "_SetMaxPrec");
		lname(0x0006ca23, "DoEFormat");
		lname(0x0006cac6, "DoFFormat");
		lname(0x0006cb29, "AdjField");
		lname(0x0006cb9a, "_FtoS");
		lname(0x0006cdd4, "strtod");
		lname(0x0006cfc7, "_dos_setvect");
		lname(0x0006cff4, "_SuperVGAType");
		lname(0x0006d3a2, "TestForVESA");
		lname(0x0006d42e, "_SetSVGAType");
		lname(0x0006d435, "_FastMap");
		lname(0x0006d4de, "_RMAlloc");
		lname(0x0006d564, "_RMFree");
		lname(0x0006d593, "_RMInterrupt");
		lname(0x0006d64c, "_RMInterrupt2");
		lname(0x0006d6d9, "_L1OutCode");
		lname(0x0006d71d, "line_inter");
		lname(0x0006d780, "_L0LineClip");
		lname(0x0006d912, "block_inter");
		lname(0x0006d96e, "_L0BlockClip");
		lname(0x0006d9eb, "_L0DrawLine");
		lname(0x0006dba8, "__sigabort");
		lname(0x0006dbb2, "__sigfpe_handler");
		lname(0x0006dbe7, "signal");
		lname(0x0006dc9c, "raise");
		lname(0x0006dd5e, "_chain_intr");
		lname(0x0006dd77, "_dos_getvect");
		lname(0x0006dda9, "__Nan_Inf");
		lname(0x0006de3c, "IF@LOG");
		lname(0x0006de7f, "IF@LOG2");
		lname(0x0006de83, "IF@LOG10");
		lname(0x0006de87, "log");
		lname(0x0006de93, "log10");
		lname(0x0006de9f, "log2");
		lname(0x0006deab, "_Scale");
		lname(0x0006deec, "_Scale10V");
		lname(0x0006e009, "__cvt");
		lname(0x0006e1e6, "__ModF");
		lname(0x0006e276, "__ZBuf2F");
		lname(0x0006e312, "_GetStackLow_");
		lname(0x0006e318, "abort");
		lname(0x0006e31e, "__terminate");
		lname(0x0006e330, "__grab_fpe");
		lname(0x0006e335, "__log87_err");
		lname(0x0006e372, "__CmpBigInt");
		lname(0x0006e3ac, "__Rnd2Int");
		lname(0x0006e417, "__Bin2String");
		lname(0x0006e538, "fdiv_main_routine");
		lname(0x0006e64f, "__fdiv_fpr");
		lname(0x0006eaeb, "__fdivp_sti_st");
		lname(0x0006eafe, "__fdivrp_sti_st");
		lname(0x0006eb11, "__fdiv_chk");
		lname(0x0006eb24, "__fdiv_m32");
		lname(0x0006eb70, "__fdiv_m64");
		lname(0x0006ebbc, "__fdiv_m32r");
		lname(0x0006ec08, "__fdiv_m64r");
		lname(0x0006ec54, "frexp");
		lname(0x0006ecc3, "__GrabFP87");
		lname(0x0006ecf4, "__math1err");
		lname(0x0006ecfe, "__math2err");
		lname(0x0006edd9, "verify_pentium_fdiv_bug_");
		lname(0x0006ee2e, "__Init_FPE_handler");
		lname(0x0006ee8b, "__Fini_FPE_handler");
		lname(0x0006eee8, "__Enable_FPE");
		lname(0x0006eefd, "__FPEHandler");
		lname(0x0006ef0a, "__FPE2Handler");
		lname(0x0006f148, "_set_matherr");
		lname(0x0006f14e, "__rterrmsg");
		lname(0x0006f196, "_matherr");
		lname(0x0006f1c6, "__Phar_hook_init");
		lname(0x0006f289, "__Phar_hook_fini");
		lname(0x0006f2b6, "__DOS4G_hook_init");
		lname(0x0006f2e3, "__DOS4G_hook_fini");
		lname(0x0006f2fc, "__Ergo_hook_init");
		lname(0x0006f33a, "__Ergo_hook_fini");
		lname(0x0006f354, "__Intel_hook_init");
		lname(0x0006f3ac, "__Intel_hook_fini");
		lname(0x0006f3ae, "__matherr");
		lname(0x0006f3b3, "__get_std_stream");
		lname(0x0006f3d2, "fputs");
		lname(0x0006f44d, "matherr_");
		default:
			break;
		}
	}

	if(obj == 4){ // RALLY.LE specific
		switch(offset){
		lname(0x000052a4, "__IsTable");
		lname(0x000053a8, "_lst_mask");
		lname(0x000053d0, "__HugeValue");
		lname(0x000053d8, "__BigPow10Table");
		lname(0x00005428, "_Msgs");
		lname(0x00019f90, "___atexit");
		lname(0x00019f94, "___int23_exit");
		lname(0x00019f98, "___FPE_handler_exit");
		lname(0x00019fdc, "__D16Infoseg");
		lname(0x00019fd8, "__GDAptr");
		lname(0x00019fde, "__x386_zero_base_selector");
		lname(0x00019fe0, "__argc");
		lname(0x00019fe4, "__argv");
		lname(0x0001a004, "_next");
		lname(0x0001a008, "___nheapbeg");
		lname(0x0001a00c, "___MiniHeapRover");
		lname(0x0001a010, "___LargestSizeB4MiniHeapRover");
		lname(0x0001a014, "__8087");
		lname(0x0001a015, "__real87");
		lname(0x0001a018, "__dynend");
		lname(0x0001a01c, "__curbrk");
		lname(0x0001a020, "__LpCmdLine");
		lname(0x0001a024, "__LpPgmName");
		lname(0x0001a028, "__psp");
		lname(0x0001a02c, "__STACKLOW");
		lname(0x0001a030, "__STACKTOP");
		lname(0x0001a034, "__ASTACKSIZ");
		lname(0x0001a038, "__ASTACKPTR");
		lname(0x0001a03c, "__cbyte");
		lname(0x0001a044, "__child");
		lname(0x0001a048, "__no87");
		lname(0x0001a04a, "__Extender");
		lname(0x0001a04b, "__ExtenderSubtype");
		lname(0x0001a04c, "__X32VM");
		lname(0x0001a04d, "__Envptr");
		lname(0x0001a051, "__Envseg");
		lname(0x0001a053, "__osmajor");
		lname(0x0001a054, "__osminor");
		lname(0x0001a055, "__FPE_handler");
		lname(0x0001a05c, "___iob");
		lname(0x0001a265, "__fmode");
		lname(0x0001a270, "_environ");
		lname(0x0001a274, "___env_mask");
		lname(0x0001a294, "_xlat");
		lname(0x0001a2a8, "___umaskval");
		lname(0x0001a2ac, "___NFiles");
		lname(0x0001a2b0, "___init_mode");
		lname(0x0001a300, "__iomode");
		lname(0x0001a350, "__AdapTab");
		lname(0x0001a374, "__MonTab");
		lname(0x0001a398, "__MemoryTab");
		lname(0x0001a3a8, "__StartUp");
		lname(0x0001a3a9, "__GrMode");
		lname(0x0001a3aa, "__ErrorStatus");
		lname(0x0001a3b8, "__CursState");
		lname(0x0001a3ba, "__GrCursor");
		lname(0x0001a3bc, "__CurrActivePage");
		lname(0x0001a3c4, "__CurrColor");
		lname(0x0001a3df, "__LineStyle");
		lname(0x0001a3e3, "__PlotAct");
		lname(0x0001a43b, "__BiosSeg");
		lname(0x0001a43d, "__MonoSeg");
		lname(0x0001a43f, "__CgaSeg");
		lname(0x0001a441, "__EgaSeg");
		lname(0x0001a443, "__RomSeg");
		lname(0x0001a445, "__BiosOff");
		lname(0x0001a449, "__MonoOff");
		lname(0x0001a44d, "__CgaOff");
		lname(0x0001a451, "__EgaOff");
		lname(0x0001a455, "__RomOff");
		lname(0x0001a460, "___WD_Present");
		lname(0x0001ae0c, "___EFG_printf");
		lname(0x0001ae10, "___EFG_scanf");
		lname(0x0001ae14, "___heap_enabled");
		lname(0x0001ae18, "__amblksiz");
		lname(0x0001ae98, "_hooked");
		lname(0x0001ae99, "_has_wgod_emu");
		lname(0x0001ae9c, "__8087cw");
		lname(0x0001aea0, "___Save8087");
		lname(0x0001aea4, "___Rest8087");
		lname(0x0001b8d0, "___old_int23");
		lname(0x0001b8d6, "___old_int_ctrl_break");
		lname(0x0001b8dc, "___old_pm_int23");
		lname(0x0001b8e2, "___old_pm_int_ctrl_break");
		lname(0x0001b8ec, "_MaxPrec");
		lname(0x0001b8f0, "_SignalTable");
		lname(0x0001b924, "___abort");
		lname(0x0001b928, "__chipbug");
		lname(0x0001b988, "___PMSeg");
		lname(0x0001b98a, "___PMAddr");
		lname(0x0001b98e, "___RMAddr");
		lname(0x0001bbb0, "__RWD_matherr");
		lname(0x0001bbb4, "___FPE_int");
		lname(0x0001bbb5, "___IRQ_num");
		lname(0x0001bbb6, "___IRQ_int");
		lname(0x0001bbb7, "___MST_pic");
		lname(0x0001bd20, "_edata");
		lname(0x000ce644, "___MiniHeapFreeRover");
		lname(0x000ce648, "___ClosedStreams");
		lname(0x000ce64c, "___OpenStreams");
		lname(0x000ce7a8, "_errno");
		lname(0x000ce7ac, "__doserrno");
		lname(0x000ce7b0, "___nheap_clean");
		lname(0x000ce7b8, "__Screen");
		lname(0x000ce7c4, "__TextPos");
		lname(0x000ce7c8, "__ConfigBuffer");
		lname(0x000ce802, "__DBCSPairs");
		lname(0x000ce80c, "__SVGAType");
		lname(0x000ce814, "__CursorShape");
		lname(0x000ce80e, "__StackSeg");
		lname(0x000ce810, "__DefMode");
		lname(0x000ce812, "__DefTextRows");
		lname(0x000ce816, "__Tx_Col_Min");
		lname(0x000ce818, "__Tx_Row_Max");
		lname(0x000ce81a, "__Tx_Col_Max");
		lname(0x000ce81c, "__Tx_Row_Min");
		lname(0x000ce820, "__IsDBCS");
		lname(0x000ce824, "____Argc");
		lname(0x000ce828, "____Argv");
		lname(0x000ce82c, "___historical_splitparms");
		lname(0x000cf2dc, "___minreal");
		lname(0x000cf2e0, "_FPArea");
		lname(0x000cf360, "_my_stack");
		lname(0x000cf7e8, "_end");
		default:
			break;
		}
	}

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
		fprintf(le.fd, "\n%sdd\t%s", indent, getLabel(fixup->object_n, fixup->target));
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
		sprintf(addr,"\n;%08x: ", le.i);
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

    le_initLinearExecutable("RALLY.LE");

    printf("[bin2inc] generating fixups and labels\n");

    processFixups();

	printf("          ... %d fixups\n", fixups_counter);
//#ifdef GLOBAL_STATS
//	local_timer = timeDiff(&__global_timer);
//	printf("          ... %d fixups in %s\n", fixups_counter, formatTime(local_timer));
//#endif


	// disassembling [TODO]
	IDisasm.pushAddress(&(VirtualAddress){ .obj_n = 1, .offset = 0 });
	IDisasm.pushAddress(&(VirtualAddress){ .obj_n = 1, .offset = 0xdc });IDisasm.pushAddress(&(VirtualAddress){ .obj_n = 1, .offset = 0x534 });IDisasm.pushAddress(&(VirtualAddress){ .obj_n = 1, .offset = 0x5a8 });
	IDisasm.pushAddress(&(VirtualAddress){ .obj_n = 1, .offset = 0x754 });
	IDisasm.pushAddress(&(VirtualAddress){ .obj_n = 1, .offset = 0x11a4 });
	IDisasm.pushAddress(&(VirtualAddress){ .obj_n = 1, .offset = 0x8774 });
	IDisasm.pushAddress(&(VirtualAddress){ .obj_n = 1, .offset = 0x15288 });
	IDisasm.pushAddress(&(VirtualAddress){ .obj_n = 1, .offset = 0x1a4f0 });
	IDisasm.pushAddress(&(VirtualAddress){ .obj_n = 1, .offset = 0x1a558 });
	IDisasm.pushAddress(&(VirtualAddress){ .obj_n = 1, .offset = 0x2601c });
	IDisasm.pushAddress(&(VirtualAddress){ .obj_n = 1, .offset = 0x2a604 });
	IDisasm.pushAddress(&(VirtualAddress){ .obj_n = 1, .offset = 0x2a654 });
	IDisasm.pushAddress(&(VirtualAddress){ .obj_n = 1, .offset = 0x2aaf8 });
	IDisasm.pushAddress(&(VirtualAddress){ .obj_n = 1, .offset = 0x2f78c });
	IDisasm.pushAddress(&(VirtualAddress){ .obj_n = 1, .offset = 0x2f7e4 });
	IDisasm.pushAddress(&(VirtualAddress){ .obj_n = 1, .offset = 0x2f8ec });
	IDisasm.pushAddress(&(VirtualAddress){ .obj_n = 1, .offset = 0x32148 });
	IDisasm.pushAddress(&(VirtualAddress){ .obj_n = 1, .offset = 0x328ec });
	IDisasm.pushAddress(&(VirtualAddress){ .obj_n = 1, .offset = 0x33ea4 });
	IDisasm.pushAddress(&(VirtualAddress){ .obj_n = 1, .offset = 0x350f4 });
	IDisasm.pushAddress(&(VirtualAddress){ .obj_n = 1, .offset = 0x356c0 });
	IDisasm.pushAddress(&(VirtualAddress){ .obj_n = 1, .offset = 0x3595c });
	IDisasm.pushAddress(&(VirtualAddress){ .obj_n = 1, .offset = 0x36758 });
	IDisasm.pushAddress(&(VirtualAddress){ .obj_n = 1, .offset = 0x392f4 });
	IDisasm.pushAddress(&(VirtualAddress){ .obj_n = 1, .offset = 0x3933c });
	IDisasm.pushAddress(&(VirtualAddress){ .obj_n = 1, .offset = 0x39548 });
	IDisasm.pushAddress(&(VirtualAddress){ .obj_n = 1, .offset = 0x3e9a0 });
	IDisasm.pushAddress(&(VirtualAddress){ .obj_n = 1, .offset = 0x43ad0 });
	IDisasm.pushAddress(&(VirtualAddress){ .obj_n = 1, .offset = 0x46370 });
	IDisasm.pushAddress(&(VirtualAddress){ .obj_n = 1, .offset = 0x48c80 });
	IDisasm.pushAddress(&(VirtualAddress){ .obj_n = 1, .offset = 0x48cd8 });
	IDisasm.pushAddress(&(VirtualAddress){ .obj_n = 1, .offset = 0x49044 });
	IDisasm.pushAddress(&(VirtualAddress){ .obj_n = 1, .offset = 0x495d5 });
	IDisasm.pushAddress(&(VirtualAddress){ .obj_n = 1, .offset = 0x49621 });
	IDisasm.pushAddress(&(VirtualAddress){ .obj_n = 1, .offset = 0x4966a });
	IDisasm.pushAddress(&(VirtualAddress){ .obj_n = 1, .offset = 0x49682 });
	IDisasm.pushAddress(&(VirtualAddress){ .obj_n = 1, .offset = 0x496b6 });
	IDisasm.pushAddress(&(VirtualAddress){ .obj_n = 1, .offset = 0x496cb });
	IDisasm.pushAddress(&(VirtualAddress){ .obj_n = 1, .offset = 0x4a151 });
	IDisasm.pushAddress(&(VirtualAddress){ .obj_n = 1, .offset = 0x4e118 });
	IDisasm.pushAddress(&(VirtualAddress){ .obj_n = 1, .offset = 0x4ea0c });
	IDisasm.pushAddress(&(VirtualAddress){ .obj_n = 1, .offset = 0x4ea24 });
	le_createLabel(1, 0x4ea24);
	IDisasm.pushAddress(&(VirtualAddress){ .obj_n = 1, .offset = 0x4f044 });
	IDisasm.pushAddress(&(VirtualAddress){ .obj_n = 1, .offset = 0x4f4ac });
	IDisasm.pushAddress(&(VirtualAddress){ .obj_n = 1, .offset = 0x4f5a4 });
	IDisasm.pushAddress(&(VirtualAddress){ .obj_n = 1, .offset = 0x4f8a8 });
	IDisasm.pushAddress(&(VirtualAddress){ .obj_n = 1, .offset = 0x4f998 });
	IDisasm.pushAddress(&(VirtualAddress){ .obj_n = 1, .offset = 0x4f9d8 });
	IDisasm.pushAddress(&(VirtualAddress){ .obj_n = 1, .offset = 0x4f9f8 });
	IDisasm.pushAddress(&(VirtualAddress){ .obj_n = 1, .offset = 0x4fb6c });
	IDisasm.pushAddress(&(VirtualAddress){ .obj_n = 1, .offset = 0x500d6 });
	IDisasm.pushAddress(&(VirtualAddress){ .obj_n = 1, .offset = 0x50338 });
	IDisasm.pushAddress(&(VirtualAddress){ .obj_n = 1, .offset = 0x5052f });
	IDisasm.pushAddress(&(VirtualAddress){ .obj_n = 1, .offset = 0x506ca });
	IDisasm.pushAddress(&(VirtualAddress){ .obj_n = 1, .offset = 0x506d2 });
	IDisasm.pushAddress(&(VirtualAddress){ .obj_n = 1, .offset = 0x5072b });
	IDisasm.pushAddress(&(VirtualAddress){ .obj_n = 1, .offset = 0x5073f });
	IDisasm.pushAddress(&(VirtualAddress){ .obj_n = 1, .offset = 0x50777 });
	IDisasm.pushAddress(&(VirtualAddress){ .obj_n = 1, .offset = 0x50a48 });
	IDisasm.pushAddress(&(VirtualAddress){ .obj_n = 1, .offset = 0x50a60 });
	IDisasm.pushAddress(&(VirtualAddress){ .obj_n = 1, .offset = 0x51a04 });
	IDisasm.pushAddress(&(VirtualAddress){ .obj_n = 1, .offset = 0x51b54 });
	IDisasm.pushAddress(&(VirtualAddress){ .obj_n = 1, .offset = 0x52704 });
	IDisasm.pushAddress(&(VirtualAddress){ .obj_n = 1, .offset = 0x528c4 });
	IDisasm.pushAddress(&(VirtualAddress){ .obj_n = 1, .offset = 0x52970 });
	IDisasm.pushAddress(&(VirtualAddress){ .obj_n = 1, .offset = 0x52a1c });
	IDisasm.pushAddress(&(VirtualAddress){ .obj_n = 1, .offset = 0x52b34 });
	IDisasm.pushAddress(&(VirtualAddress){ .obj_n = 1, .offset = 0x53134 });
	IDisasm.pushAddress(&(VirtualAddress){ .obj_n = 1, .offset = 0x53168 });
	IDisasm.pushAddress(&(VirtualAddress){ .obj_n = 1, .offset = 0x53338 });
	IDisasm.pushAddress(&(VirtualAddress){ .obj_n = 1, .offset = 0x53388 });
	IDisasm.pushAddress(&(VirtualAddress){ .obj_n = 1, .offset = 0x5345c });
	IDisasm.pushAddress(&(VirtualAddress){ .obj_n = 1, .offset = 0x534cc });
	IDisasm.pushAddress(&(VirtualAddress){ .obj_n = 1, .offset = 0x53538 });
	IDisasm.pushAddress(&(VirtualAddress){ .obj_n = 1, .offset = 0x535c8 });
	IDisasm.pushAddress(&(VirtualAddress){ .obj_n = 1, .offset = 0x5386c });
	IDisasm.pushAddress(&(VirtualAddress){ .obj_n = 1, .offset = 0x54148 });
	IDisasm.pushAddress(&(VirtualAddress){ .obj_n = 1, .offset = 0x541b0 });
	le_createLabel(1, 0x541b0);
	IDisasm.pushAddress(&(VirtualAddress){ .obj_n = 1, .offset = 0x54940 });
	IDisasm.pushAddress(&(VirtualAddress){ .obj_n = 1, .offset = 0x54974 });
	IDisasm.pushAddress(&(VirtualAddress){ .obj_n = 1, .offset = 0x54a7c });
	IDisasm.pushAddress(&(VirtualAddress){ .obj_n = 1, .offset = 0x54b60 });
	IDisasm.pushAddress(&(VirtualAddress){ .obj_n = 1, .offset = 0x55284 });
	IDisasm.pushAddress(&(VirtualAddress){ .obj_n = 1, .offset = 0x55408 });
	IDisasm.pushAddress(&(VirtualAddress){ .obj_n = 1, .offset = 0x554c4 });
	IDisasm.pushAddress(&(VirtualAddress){ .obj_n = 1, .offset = 0x55748 });
	IDisasm.pushAddress(&(VirtualAddress){ .obj_n = 1, .offset = 0x557d4 });
	IDisasm.pushAddress(&(VirtualAddress){ .obj_n = 1, .offset = 0x5583c });
	IDisasm.pushAddress(&(VirtualAddress){ .obj_n = 1, .offset = 0x55884 });
	IDisasm.pushAddress(&(VirtualAddress){ .obj_n = 1, .offset = 0x558c0 });
	IDisasm.pushAddress(&(VirtualAddress){ .obj_n = 1, .offset = 0x558c8 });
	IDisasm.pushAddress(&(VirtualAddress){ .obj_n = 1, .offset = 0x55970 });
	IDisasm.pushAddress(&(VirtualAddress){ .obj_n = 1, .offset = 0x55978 });
	IDisasm.pushAddress(&(VirtualAddress){ .obj_n = 1, .offset = 0x55980 });
	IDisasm.pushAddress(&(VirtualAddress){ .obj_n = 1, .offset = 0x55988 });
	IDisasm.pushAddress(&(VirtualAddress){ .obj_n = 1, .offset = 0x559b0 });
	IDisasm.pushAddress(&(VirtualAddress){ .obj_n = 1, .offset = 0x55b36 });
	IDisasm.pushAddress(&(VirtualAddress){ .obj_n = 1, .offset = 0x55f3a });
	IDisasm.pushAddress(&(VirtualAddress){ .obj_n = 1, .offset = 0x55f63 });
	IDisasm.pushAddress(&(VirtualAddress){ .obj_n = 1, .offset = 0x5629c });
	le_createLabel(1, 0x5629c);
	IDisasm.pushAddress(&(VirtualAddress){ .obj_n = 1, .offset = 0x56383 });
	IDisasm.pushAddress(&(VirtualAddress){ .obj_n = 1, .offset = 0x563d7 });
	IDisasm.pushAddress(&(VirtualAddress){ .obj_n = 1, .offset = 0x5658c });
	IDisasm.pushAddress(&(VirtualAddress){ .obj_n = 1, .offset = 0x56591 });
	IDisasm.pushAddress(&(VirtualAddress){ .obj_n = 1, .offset = 0x56596 });
	IDisasm.pushAddress(&(VirtualAddress){ .obj_n = 1, .offset = 0x565f2 });
	IDisasm.pushAddress(&(VirtualAddress){ .obj_n = 1, .offset = 0x56e84 });
	le_createLabel(1, 0x56e84);
	IDisasm.pushAddress(&(VirtualAddress){ .obj_n = 1, .offset = 0x56ea7 });
	IDisasm.pushAddress(&(VirtualAddress){ .obj_n = 1, .offset = 0x56fb4 });
	le_createLabel(1, 0x56fb4);
	IDisasm.pushAddress(&(VirtualAddress){ .obj_n = 1, .offset = 0x57230 });
	IDisasm.pushAddress(&(VirtualAddress){ .obj_n = 1, .offset = 0x57426 });
	le_createLabel(1, 0x57426);
	IDisasm.pushAddress(&(VirtualAddress){ .obj_n = 1, .offset = 0x57521 });
	IDisasm.pushAddress(&(VirtualAddress){ .obj_n = 1, .offset = 0x57910 });
	IDisasm.pushAddress(&(VirtualAddress){ .obj_n = 1, .offset = 0x57b6c });
	IDisasm.pushAddress(&(VirtualAddress){ .obj_n = 1, .offset = 0x57b8c });
	IDisasm.pushAddress(&(VirtualAddress){ .obj_n = 1, .offset = 0x582d0 });
	IDisasm.pushAddress(&(VirtualAddress){ .obj_n = 1, .offset = 0x58350 });
	IDisasm.pushAddress(&(VirtualAddress){ .obj_n = 1, .offset = 0x58684 });
	IDisasm.pushAddress(&(VirtualAddress){ .obj_n = 1, .offset = 0x586d4 });
	IDisasm.pushAddress(&(VirtualAddress){ .obj_n = 1, .offset = 0x58780 });
	IDisasm.pushAddress(&(VirtualAddress){ .obj_n = 1, .offset = 0x587b8 });
	IDisasm.pushAddress(&(VirtualAddress){ .obj_n = 1, .offset = 0x5880c });
	IDisasm.pushAddress(&(VirtualAddress){ .obj_n = 1, .offset = 0x58c7e });
	IDisasm.pushAddress(&(VirtualAddress){ .obj_n = 1, .offset = 0x594c0 });
	IDisasm.pushAddress(&(VirtualAddress){ .obj_n = 1, .offset = 0x59923 });
	IDisasm.pushAddress(&(VirtualAddress){ .obj_n = 1, .offset = 0x59e52 });
	IDisasm.pushAddress(&(VirtualAddress){ .obj_n = 1, .offset = 0x5d723 });
	le_createLabel(1, 0x5d723);
	IDisasm.pushAddress(&(VirtualAddress){ .obj_n = 1, .offset = 0x5dacb });
	le_createLabel(1, 0x5dacb);
	IDisasm.pushAddress(&(VirtualAddress){ .obj_n = 1, .offset = 0x5dbb1 });
	IDisasm.pushAddress(&(VirtualAddress){ .obj_n = 1, .offset = 0x5de94 });
	le_createLabel(1, 0x5de94);
	IDisasm.pushAddress(&(VirtualAddress){ .obj_n = 1, .offset = 0x5e3a4 });
	IDisasm.pushAddress(&(VirtualAddress){ .obj_n = 1, .offset = 0x5e3c0 });
	IDisasm.pushAddress(&(VirtualAddress){ .obj_n = 1, .offset = 0x5e545 });
	IDisasm.pushAddress(&(VirtualAddress){ .obj_n = 1, .offset = 0x5e61b });
	IDisasm.pushAddress(&(VirtualAddress){ .obj_n = 1, .offset = 0x5e620 });
	IDisasm.pushAddress(&(VirtualAddress){ .obj_n = 1, .offset = 0x5e624 });
	IDisasm.pushAddress(&(VirtualAddress){ .obj_n = 1, .offset = 0x5e655 });
	le_createLabel(1, 0x5e655);
	IDisasm.pushAddress(&(VirtualAddress){ .obj_n = 1, .offset = 0x5e65f });
	IDisasm.pushAddress(&(VirtualAddress){ .obj_n = 1, .offset = 0x5e6a9 });
	le_createLabel(1, 0x5e6a9);
	IDisasm.pushAddress(&(VirtualAddress){ .obj_n = 1, .offset = 0x5e8f5 });
	le_createLabel(1, 0x5e8f5);
	IDisasm.pushAddress(&(VirtualAddress){ .obj_n = 1, .offset = 0x5eae5 });
	le_createLabel(1, 0x5eae5);
	IDisasm.pushAddress(&(VirtualAddress){ .obj_n = 1, .offset = 0x5eb1a });
	le_createLabel(1, 0x5eb1a);
	IDisasm.pushAddress(&(VirtualAddress){ .obj_n = 1, .offset = 0x5eb26 });
	le_createLabel(1, 0x5eb26);
	IDisasm.pushAddress(&(VirtualAddress){ .obj_n = 1, .offset = 0x5eb75 });
	IDisasm.pushAddress(&(VirtualAddress){ .obj_n = 1, .offset = 0x5ecbb });
	IDisasm.pushAddress(&(VirtualAddress){ .obj_n = 1, .offset = 0x5ed32 });
	IDisasm.pushAddress(&(VirtualAddress){ .obj_n = 1, .offset = 0x5ed3e });
	le_createLabel(1, 0x5ed3e);
	IDisasm.pushAddress(&(VirtualAddress){ .obj_n = 1, .offset = 0x5ef2c });
	IDisasm.pushAddress(&(VirtualAddress){ .obj_n = 1, .offset = 0x61da4 });
	IDisasm.pushAddress(&(VirtualAddress){ .obj_n = 1, .offset = 0x65648 });
	IDisasm.pushAddress(&(VirtualAddress){ .obj_n = 1, .offset = 0x662dc });
	IDisasm.pushAddress(&(VirtualAddress){ .obj_n = 1, .offset = 0x666c5 });
	IDisasm.pushAddress(&(VirtualAddress){ .obj_n = 1, .offset = 0x66e32 });
	IDisasm.pushAddress(&(VirtualAddress){ .obj_n = 1, .offset = 0x6678e });
	IDisasm.pushAddress(&(VirtualAddress){ .obj_n = 1, .offset = 0x67014 });
	IDisasm.pushAddress(&(VirtualAddress){ .obj_n = 1, .offset = 0x67104 });
	IDisasm.pushAddress(&(VirtualAddress){ .obj_n = 1, .offset = 0x67741 });
	IDisasm.pushAddress(&(VirtualAddress){ .obj_n = 1, .offset = 0x6786a });
	IDisasm.pushAddress(&(VirtualAddress){ .obj_n = 1, .offset = 0x67db0 });
	IDisasm.pushAddress(&(VirtualAddress){ .obj_n = 1, .offset = 0x67dcb });
	IDisasm.pushAddress(&(VirtualAddress){ .obj_n = 1, .offset = 0x67e47 });
	IDisasm.pushAddress(&(VirtualAddress){ .obj_n = 1, .offset = 0x67f2d });
	IDisasm.pushAddress(&(VirtualAddress){ .obj_n = 1, .offset = 0x6822b });
	le_createLabel(1, 0x6822b);
	IDisasm.pushAddress(&(VirtualAddress){ .obj_n = 1, .offset = 0x6823b });
	le_createLabel(1, 0x6823b);
	IDisasm.pushAddress(&(VirtualAddress){ .obj_n = 1, .offset = 0x683ce });
	IDisasm.pushAddress(&(VirtualAddress){ .obj_n = 1, .offset = 0x68517 });
	IDisasm.pushAddress(&(VirtualAddress){ .obj_n = 1, .offset = 0x688a0 });
	IDisasm.pushAddress(&(VirtualAddress){ .obj_n = 1, .offset = 0x688c4 });
	le_createLabel(1, 0x688c4);
	IDisasm.pushAddress(&(VirtualAddress){ .obj_n = 1, .offset = 0x68910 });
	IDisasm.pushAddress(&(VirtualAddress){ .obj_n = 1, .offset = 0x68934 });
	IDisasm.pushAddress(&(VirtualAddress){ .obj_n = 1, .offset = 0x68a68 });
	IDisasm.pushAddress(&(VirtualAddress){ .obj_n = 1, .offset = 0x68aa4 });
	IDisasm.pushAddress(&(VirtualAddress){ .obj_n = 1, .offset = 0x6a5b2 });
	IDisasm.pushAddress(&(VirtualAddress){ .obj_n = 1, .offset = 0x6a5b7 });
	IDisasm.pushAddress(&(VirtualAddress){ .obj_n = 1, .offset = 0x6b694 });
	IDisasm.pushAddress(&(VirtualAddress){ .obj_n = 1, .offset = 0x6b87b });
	IDisasm.pushAddress(&(VirtualAddress){ .obj_n = 1, .offset = 0x6b87c });
	IDisasm.pushAddress(&(VirtualAddress){ .obj_n = 1, .offset = 0x6bd1e });
	le_createLabel(1, 0x6bd1e);
	IDisasm.pushAddress(&(VirtualAddress){ .obj_n = 1, .offset = 0x6be9f });
	le_createLabel(1, 0x6be9f);
	IDisasm.pushAddress(&(VirtualAddress){ .obj_n = 1, .offset = 0x6bee2 });
	IDisasm.pushAddress(&(VirtualAddress){ .obj_n = 1, .offset = 0x6bf77 });
	IDisasm.pushAddress(&(VirtualAddress){ .obj_n = 1, .offset = 0x6c234 });
	IDisasm.pushAddress(&(VirtualAddress){ .obj_n = 1, .offset = 0x6ca1d });
	le_createLabel(1, 0x6ca1d);
	IDisasm.pushAddress(&(VirtualAddress){ .obj_n = 1, .offset = 0x6cdd4 });
	IDisasm.pushAddress(&(VirtualAddress){ .obj_n = 1, .offset = 0x6d42e });
	le_createLabel(1, 0x6d42e);
	IDisasm.pushAddress(&(VirtualAddress){ .obj_n = 1, .offset = 0x6d435 });
	le_createLabel(1, 0x6d435);
	IDisasm.pushAddress(&(VirtualAddress){ .obj_n = 1, .offset = 0x6d912 });
	IDisasm.pushAddress(&(VirtualAddress){ .obj_n = 1, .offset = 0x6d96e });
	le_createLabel(1, 0x6d96e);
	IDisasm.pushAddress(&(VirtualAddress){ .obj_n = 1, .offset = 0x6dba8 });
	IDisasm.pushAddress(&(VirtualAddress){ .obj_n = 1, .offset = 0x6dbe7 });
	le_createLabel(1, 0x6dbe7);
	IDisasm.pushAddress(&(VirtualAddress){ .obj_n = 1, .offset = 0x6dd5e });
	IDisasm.pushAddress(&(VirtualAddress){ .obj_n = 1, .offset = 0x6de7f });
	IDisasm.pushAddress(&(VirtualAddress){ .obj_n = 1, .offset = 0x6de87 });
	le_createLabel(1, 0x6de87);
	IDisasm.pushAddress(&(VirtualAddress){ .obj_n = 1, .offset = 0x6de93 });
	le_createLabel(1, 0x6de93);
	IDisasm.pushAddress(&(VirtualAddress){ .obj_n = 1, .offset = 0x6de9f });
	le_createLabel(1, 0x6de9f);
	IDisasm.pushAddress(&(VirtualAddress){ .obj_n = 1, .offset = 0x6e1e6 });
	le_createLabel(1, 0x6e1e6);
	IDisasm.pushAddress(&(VirtualAddress){ .obj_n = 1, .offset = 0x6e318 });
	le_createLabel(1, 0x6e318);
	IDisasm.pushAddress(&(VirtualAddress){ .obj_n = 1, .offset = 0x6e335 });
	IDisasm.pushAddress(&(VirtualAddress){ .obj_n = 1, .offset = 0x6e64f });
	le_createLabel(1, 0x6e64f);
	IDisasm.pushAddress(&(VirtualAddress){ .obj_n = 1, .offset = 0x6eb11 });
	le_createLabel(1, 0x6eb11);
	IDisasm.pushAddress(&(VirtualAddress){ .obj_n = 1, .offset = 0x6eb24 });
	le_createLabel(1, 0x6eb24);
	IDisasm.pushAddress(&(VirtualAddress){ .obj_n = 1, .offset = 0x6ebbc });
	le_createLabel(1, 0x6ebbc);
	IDisasm.pushAddress(&(VirtualAddress){ .obj_n = 1, .offset = 0x6ec08 });
	le_createLabel(1, 0x6ec08);
	IDisasm.pushAddress(&(VirtualAddress){ .obj_n = 1, .offset = 0x6edd9 });
	IDisasm.pushAddress(&(VirtualAddress){ .obj_n = 1, .offset = 0x6ee2e });
	IDisasm.pushAddress(&(VirtualAddress){ .obj_n = 1, .offset = 0x6ee8b });
	IDisasm.pushAddress(&(VirtualAddress){ .obj_n = 1, .offset = 0x6eefd });
	IDisasm.pushAddress(&(VirtualAddress){ .obj_n = 1, .offset = 0x6f148 });
	le_createLabel(1, 0x6f148);
	IDisasm.pushAddress(&(VirtualAddress){ .obj_n = 1, .offset = 0x6f1c6 });
	IDisasm.pushAddress(&(VirtualAddress){ .obj_n = 1, .offset = 0x6f289 });
	IDisasm.pushAddress(&(VirtualAddress){ .obj_n = 1, .offset = 0x6f2e3 });
	IDisasm.pushAddress(&(VirtualAddress){ .obj_n = 1, .offset = 0x6f33a });

	IDisasm.pushAddress(&(VirtualAddress){ .obj_n = 2, .offset = 0 });

	pushJumpTable(&(VirtualAddress){ .obj_n = 1, .offset = 0x4fd6c });
	pushJumpTable(&(VirtualAddress){ .obj_n = 4, .offset = 0x1bbb0 });
	pushJumpTable(&(VirtualAddress){ .obj_n = 4, .offset = 0x1bc14 });

	le_createLabel(4, 0x1a294);

	markAsData(
		&(VirtualAddress){ .obj_n = 1, .offset = 0x3 },
		&(VirtualAddress){ .obj_n = 1, .offset = 0xf });
	markAsData(
		&(VirtualAddress){ .obj_n = 1, .offset = 0x5e0 },
		&(VirtualAddress){ .obj_n = 1, .offset = 0x753 });
	markAsData(
		&(VirtualAddress){ .obj_n = 1, .offset = 0x2f1f0 },
		&(VirtualAddress){ .obj_n = 1, .offset = 0x2f71b });
	markAsData(
		&(VirtualAddress){ .obj_n = 1, .offset = 0x486b0 },
		&(VirtualAddress){ .obj_n = 1, .offset = 0x48b1f });
	markAsData(
		&(VirtualAddress){ .obj_n = 1, .offset = 0x49e0f },
		&(VirtualAddress){ .obj_n = 1, .offset = 0x4a0c1 });
	markAsData(
		&(VirtualAddress){ .obj_n = 1, .offset = 0x4a22c },
		&(VirtualAddress){ .obj_n = 1, .offset = 0x4e0db });
	markAsData(
		&(VirtualAddress){ .obj_n = 1, .offset = 0x4fbb6 },
		&(VirtualAddress){ .obj_n = 1, .offset = 0x4fd6b });
	markAsData(
		&(VirtualAddress){ .obj_n = 1, .offset = 0x4fdb4 },
		&(VirtualAddress){ .obj_n = 1, .offset = 0x4ff98 });
	markAsData(
		&(VirtualAddress){ .obj_n = 1, .offset = 0x50420 },
		&(VirtualAddress){ .obj_n = 1, .offset = 0x50465 });
	markAsData(
		&(VirtualAddress){ .obj_n = 1, .offset = 0x508b0 },
		&(VirtualAddress){ .obj_n = 1, .offset = 0x508eb });
	markAsData(
		&(VirtualAddress){ .obj_n = 1, .offset = 0x53c9a },
		&(VirtualAddress){ .obj_n = 1, .offset = 0x53d0f });
	markAsData(
		&(VirtualAddress){ .obj_n = 1, .offset = 0x54720 },
		&(VirtualAddress){ .obj_n = 1, .offset = 0x54863 });
	markAsData(
		&(VirtualAddress){ .obj_n = 1, .offset = 0x55d58 },
		&(VirtualAddress){ .obj_n = 1, .offset = 0x55d7c });
	markAsData(
		&(VirtualAddress){ .obj_n = 1, .offset = 0x577c0 },
		&(VirtualAddress){ .obj_n = 1, .offset = 0x577cb });
	markAsData(
		&(VirtualAddress){ .obj_n = 1, .offset = 0x588c0 },
		&(VirtualAddress){ .obj_n = 1, .offset = 0x58c41 });
	markAsData(
		&(VirtualAddress){ .obj_n = 1, .offset = 0x58d30 },
		&(VirtualAddress){ .obj_n = 1, .offset = 0x591b7 });
	markAsData( // looks like table of interrupt call functions
		&(VirtualAddress){ .obj_n = 1, .offset = 0x5d7a4 },
		&(VirtualAddress){ .obj_n = 1, .offset = 0x5daa3 });
	markAsData(
		&(VirtualAddress){ .obj_n = 1, .offset = 0x65f90 },
		&(VirtualAddress){ .obj_n = 1, .offset = 0x662db });
	markAsData(
		&(VirtualAddress){ .obj_n = 1, .offset = 0x6813c },
		&(VirtualAddress){ .obj_n = 1, .offset = 0x68160 });
		markAsData(
		&(VirtualAddress){ .obj_n = 1, .offset = 0x68528 },
		&(VirtualAddress){ .obj_n = 1, .offset = 0x685df });
	markAsData(
		&(VirtualAddress){ .obj_n = 1, .offset = 0x6a8c7 },
		&(VirtualAddress){ .obj_n = 1, .offset = 0x6a946 });
	markAsData(
		&(VirtualAddress){ .obj_n = 1, .offset = 0x6aa7a },
		&(VirtualAddress){ .obj_n = 1, .offset = 0x6aad3 });
	markAsData(
		&(VirtualAddress){ .obj_n = 1, .offset = 0x6ae76 },
		&(VirtualAddress){ .obj_n = 1, .offset = 0x6aebb });
	markAsData(
		&(VirtualAddress){ .obj_n = 1, .offset = 0x6b0ea },
		&(VirtualAddress){ .obj_n = 1, .offset = 0x6b1af });
	markAsData(
		&(VirtualAddress){ .obj_n = 1, .offset = 0x6b3cc },
		&(VirtualAddress){ .obj_n = 1, .offset = 0x6b3f3 });
	markAsData(
		&(VirtualAddress){ .obj_n = 1, .offset = 0x6c8b0 },
		&(VirtualAddress){ .obj_n = 1, .offset = 0x6c96f });
	markAsData(
		&(VirtualAddress){ .obj_n = 1, .offset = 0x6e486 },
		&(VirtualAddress){ .obj_n = 1, .offset = 0x6e535 });

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
