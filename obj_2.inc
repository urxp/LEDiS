;OBJECT#2
___90000h:
		push    ebp
		mov     ebp, esp
		sub     esp, byte 0eh
		push    ebx
		push    ecx
		push    edi
		push    es
		mov     ax, 0
		mov     ecx, 1
		int     31h
		mov     [ebp-2], ax
		jb      near ___900a0h
		;mov     bx, ds
		db	66h,8ch,0dbh
		mov     ax, 6
		int     31h
		jb      short ___900a0h
		shl     ecx, 10h
		mov     cx, dx
		add     ecx, [ebp+8]
		mov     [ebp-6], ecx
		;mov     bx, ds
		db	66h,8ch,0dbh
		;mov     es, bx
		db	66h,8eh,0c3h
		lea     edi, [ebp-0eh]
		mov     ax, 0bh
		int     31h
		jb      short ___900a0h
		lea     edx, [ebp-0eh]
		mov     eax, [ebp+0ch]
		mov     [edx], ax
		mov     ax, [ebp+0eh]
		and     al, 0fh
		mov     bl, [edx+6]
		and     bl, 70h
		or      al, bl
		mov     [edx+6], al
		mov     ax, [ebp-6]
		mov     [edx+2], ax
		mov     ax, [ebp-4]
		mov     [edx+4], al
		mov     [edx+7], ah
		mov     ax, [ebp+10h]
		cmp     ax, byte 0
		je      short ___90085h
		and     byte [edx+5], 0f1h
		or      byte [edx+5], 0ah
___90085h:
		;mov     bx, ds
		db	66h,8ch,0dbh
		;mov     es, bx
		db	66h,8eh,0c3h
		mov     bx, [ebp-2]
		lea     edi, [ebp-0eh]
		mov     ax, 0ch
		int     31h
		jb      short ___900a0h
		mov     dx, [ebp-2]
		jmp     short ___900a3h
___900a0h:
		xor     dx, dx
___900a3h:
		xor     eax, eax
		mov     ax, dx
		pop     es
		pop     edi
		pop     ecx
		pop     ebx
		add     esp, byte 0eh
		pop     ebp
		retn    