CC = clang
BYTES = #-DSHOW_INPUT_BYTES
CFLAGS = -DPACKED -m32 -O3
CFLAGS += $(BYTES)
Makefile = Makefile

segs = obj_1.inc obj_2.inc obj_3.inc bss_3.inc
ops = op_00.o op_01.o op_02.o op_03.o \
		op_08.o op_09.o op_0a.o op_0b.o \
		op_20.o op_21.o op_22.o op_23.o op_28.o op_29.o op_2a.o op_2b.o \
		op_30.o op_31.o op_32.o op_33.o \
		op_38.o op_39.o op_3a.o op_3b.o op_66.o \
		op_69.o op_6b.o op_80.o op_81.o op_83.o \
		op_84.o op_85.o \
		op_88.o op_89.o op_8a.o op_8b.o op_8d.o \
		op_c0.o op_c1.o op_c4.o op_c6.o op_c7.o \
		op_d0.o op_d1.o op_f6.o op_f7.o op_ff.o

all: qfgiv.exe qfgiv.le

qfgiv.exe: qfgiv.wlink qfgiv.obj 
	wlink @ $< name $@

qfgiv.le: qfgiv.wlink qfgiv.obj
	wlink @ $< op nostub name $@ 

qfgiv.obj: qfgiv.asm 
#	wasm -fo=$@ $<
	nasm -f obj -o $@ $< -O0
#	ml.exe /omf /c /Fo$@ $<

qfgiv.asm: $(Makefile) $(segs)
	@touch $@

%.inc: bin2inc
	./bin2inc

bin2inc: bin2inc.o fixups.o le.o disasm.o disasm_switch.o address_mode.o $(ops)
	$(CC) -o $@ $^ -Wl,--strip-all 

%.o: %.c bin2inc.h $(Makefile)
	$(CC) -c -o $@ $< $(CFLAGS)


.PHONY: clean

clean:
	@rm -f $(segs) *.o qfgiv.obj qfgiv.exe qfgiv.le bin2inc
