cpu 386

group  dgroup obj_3 _bss stack

section obj_1 public use32 align=1 class=code
%include "obj_1.inc"

section obj_2 public use32 align=1 class=data
%include "obj_2.inc"

section obj_3 public use32 align=1 class=data
%include "obj_3.inc"

section _bss public use32 align=1 class=bss
%include "bss_3.inc"

section stack stack use32 align=1 class=stack
    resb 1000h
