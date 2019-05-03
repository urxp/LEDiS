;OBJECT#2
dr@80000h:
;00000000: 1e
		push    ds
;00000001: 06
		push    es
;00000002: 60
		pusha   
;00000003: b0 20
		mov     al, 20h
;00000005: e6 20
		out     20h, al
dr@80007h:
;00000007: 2e 8b 16 00 00
		mov     dx, [cs:0]
;0000000c: 83 c2 02
		add     dx, byte 2
;0000000f: ec
		in      al, dx
;00000010: a8 01
		test    al, 1
;00000012: 74 03
		je      short dr@80017h
;00000014: e9 a0 00
		jmp     near dr@800b7h
dr@80017h:
;00000017: 88 c3
		mov     bl, al
;00000019: 80 e3 07
		and     bl, 7
;0000001c: 80 fb 02
		cmp     bl, 2
;0000001f: 75 3b
		jne     short dr@8005ch
;00000021: 2e 8b 0e 0a 00
		mov     cx, [cs:0ah]
dr@80026h:
;00000026: 2e 8e 06 06 00
		;mov     es, [cs:6]
		db	2eh,8eh,6,6,0
;0000002b: 2e 8b 3e 08 00
		mov     di, [cs:8]
;00000030: 53
		push    bx
;00000031: 26 8b 1d
		mov     bx, [es:di]
;00000034: 83 c7 02
		add     di, byte 2
;00000037: 26 8b 15
		mov     dx, [es:di]
;0000003a: 39 d3
		cmp     bx, dx
;0000003c: 5b
		pop     bx
;0000003d: 74 1d
		je      short dr@8005ch
;0000003f: 83 c7 02
		add     di, byte 2
;00000042: 81 e2 ff 0f
		and     dx, 0fffh
;00000046: 01 d7
		add     di, dx
;00000048: 26 8a 05
		mov     al, [es:di]
;0000004b: 2e 8b 3e 08 00
		mov     di, [cs:8]
;00000050: 26 ff 45 02
		inc     word [es:di+2]
;00000054: 2e 8b 16 00 00
		mov     dx, [cs:0]
;00000059: ee
		out     dx, al
;0000005a: e2 ca
		loop    dr@80026h
dr@8005ch:
;0000005c: 80 fb 04
		cmp     bl, 4
;0000005f: 75 34
		jne     short dr@80095h
dr@80061h:
;00000061: 2e 8b 16 00 00
		mov     dx, [cs:0]
;00000066: ec
		in      al, dx
;00000067: 2e 8e 06 02 00
		;mov     es, [cs:2]
		db	2eh,8eh,6,2,0
;0000006c: 2e 8b 3e 04 00
		mov     di, [cs:4]
;00000071: 26 8b 0d
		mov     cx, [es:di]
;00000074: 83 c7 04
		add     di, byte 4
;00000077: 81 e1 ff 0f
		and     cx, 0fffh
;0000007b: 01 cf
		add     di, cx
;0000007d: 26 88 05
		mov     [es:di], al
;00000080: 2e 8b 3e 04 00
		mov     di, [cs:4]
;00000085: 26 ff 05
		inc     word [es:di]
;00000088: 2e 8b 16 00 00
		mov     dx, [cs:0]
;0000008d: 83 c2 05
		add     dx, byte 5
;00000090: ec
		in      al, dx
;00000091: a8 01
		test    al, 1
;00000093: 75 cc
		jne     short dr@80061h
dr@80095h:
;00000095: 80 fb 06
		cmp     bl, 6
;00000098: 75 09
		jne     short dr@800a3h
;0000009a: 2e 8b 16 00 00
		mov     dx, [cs:0]
;0000009f: 83 c2 05
		add     dx, byte 5
;000000a2: ec
		in      al, dx
dr@800a3h:
;000000a3: 80 fb 00
		cmp     bl, 0
;000000a6: 74 03
		je      short dr@800abh
;000000a8: e9 5c ff
		jmp     near dr@80007h
dr@800abh:
;000000ab: 2e 8b 16 00 00
		mov     dx, [cs:0]
;000000b0: 83 c2 06
		add     dx, byte 6
;000000b3: ec
		in      al, dx
;000000b4: e9 50 ff
		jmp     near dr@80007h
dr@800b7h:
;000000b7: 61
		popa    
;000000b8: 07
		pop     es
;000000b9: 1f
		pop     ds
;000000ba: cf
		iret    