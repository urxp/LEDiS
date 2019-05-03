decode: __asm_decode.o __c_decode.o
	clang-9 -o $@ $^

__asm_%.o: %.asm
	nasm -o $@ -f elf $<

__c_%.o: %.c 
	clang-9 -o $@ -c $<


.PHONY: clean

clean:
	rm -f *.o decode