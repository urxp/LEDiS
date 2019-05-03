CPU 686

extern decode_bufbase
extern decode_bufend
extern decode_bufthreshold
extern decode_bufsize
extern decode_filesize
extern decode_handle

extern fread

global decode

section .text

decode:

        mov edi,[decode_bufbase]
        mov esi,[decode_bufend]

        call decode_read

        movsb
        xor dl,dl

        ret


        jmp short decode_decode


decode_extended:
        call decode_getbit
        jnc short decode_length

        call decode_getbit
        adc cl,1
        shl cl,1

loc_0:
        call decode_getbit
        rcl bh,1
        loop loc_0


decode_length:
        mov dh,2
        mov cl,4

loc_1:
        inc dh
        call decode_getbit
        jc short decode_movestringdh

        loop loc_1

        call decode_getbit
        jnc short decode_length3bit

        lodsb
        mov cl,al
        add ecx,15
        jmp short decode_movestring


decode_length3bit:
        xor dh,dh
        mov cl,3

loc_2:
        call decode_getbit
        rcl dh,1
        loop loc_2

        add dh,7


decode_movestringdh:
        mov cl,dh
        jmp short decode_movestring

decode_movestring2:
        mov cl,2

decode_movestring:
        neg ebx
        dec ebx

loc_3:
        mov al, [edi+ebx]
        stosb
        loop loc_3


decode_decode:
        cmp esi,[decode_bufthreshold]
        jb short loc_4

        call decode_read

loc_4:
        call decode_getbit
        jc short code

        movsb
        jmp decode_decode


code:
        xor ebx,ebx
        lodsb
        mov bl,al

        call decode_getbit
        jc short decode_extended

        call decode_getbit
        jc short decode_code11

        dec ebx
        jns decode_movestring2


        ret


decode_code11:
        mov cl,3

loc_5:
        call decode_getbit
        rcl bh,1
        loop loc_5

        jmp short decode_movestring2

decode_getbit:
        dec dl
        jns short loc_6

        lodsd
        mov ebp,eax
        mov dl,31

loc_6:
        shl ebp,1

        ret


decode_read:
        push edx
        push edi

        mov edi,[decode_bufbase]
        push edi

        mov ecx,[decode_bufend]
        sub ecx,esi
        ;mov edx,ecx
        ;add edx, edi
        mov eax,ecx
        rep movsb

        pop esi

        lea edx,[esi+eax]
        mov ecx,[decode_bufsize]
        sub ecx,eax

        mov eax,[decode_filesize]
        cmp ecx,eax
        jbe short loc_7

        mov ecx,eax

loc_7:
        mov ebx,[decode_handle]


        push ebx
        push ecx
        push 1
        push edx
        call fread
        add esp, 16

        sub [decode_filesize], eax

        pop edi
        pop edx
        xor ecx,ecx
        ret