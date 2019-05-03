cc = clang-9

LDFLAGS = -Wl,--strip-all

ifeq ($(cc), clang-9)
	LDFLAGS += -fuse-ld=lld-9
endif


pmwu: pmwu.o
	$(cc) -o $@ $< $(LDFLAGS)

%.o: %.c
	$(cc) -o $@ -c $< -O2

.phony: clean

clean:
	@rm -f *.o pmwu
