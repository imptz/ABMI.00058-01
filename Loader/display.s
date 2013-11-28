COLOR_SINIY			EQU	0x0100
COLOR_ZELENIY		EQU	0x0200
COLOR_GOLUBOY		EQU	0x0300
COLOR_KRASNIY		EQU	0x0400
COLOR_ROZOVIY		EQU	0x0500
COLOR_ORANZHEVIY	EQU	0x0600
COLOR_BELIY			EQU	0x0700
COLOR_SERIY			EQU	0x0800

COLOR_INFO			EQU	COLOR_BELIY
COLOR_FAILED		EQU	COLOR_KRASNIY
COLOR_DONE			EQU	COLOR_ZELENIY

VIDEO_BASE_ADDRESS	EQU	0x000b8000

clearScreen:
	mov     eax, COLOR_BELIY
	mov		al, 0x20
	shl		eax, 16
	add		eax, COLOR_BELIY
	mov		al, 0x20

	mov		edi, VIDEO_BASE_ADDRESS
	mov		ecx, 40 * 25
	cld
	rep		stosd
	ret

; esi - string
; edi - y * 160 + x * 2 + VIDEO_BASE_ADDRESS
print:
	cld
_printString_12:
	lodsb
	cmp     al,0
	je      _printString_22
	stosw
	jmp		_printString_12
_printString_22: 		
	ret


TablesToStr     db      0x30,0x31,0x32,0x33,0x34,0x35,0x36,0x37,0x38,0x39,0x61,0x62,0x63,0x64,0x65,0x66
bufferIntToStr  db      0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0

byteToStr:
	push	ebx
	push	ecx
	push	edx

	mov     esi,bufferIntToStr+16
	mov     ebx,TablesToStr
      
	mov     ecx,8
	dec     esi
	mov     dl,al
	and     al,0x0f
	xlatb
	mov     [esi],byte al
	mov     al,dl
	shr     al,4
	dec     esi
	xlatb
	mov     [esi],byte al

	pop		edx
	pop		ecx
	pop		ebx
	ret


; ebx - address
; ecx - length
; edi - position = y * 160 + x * 2 + VIDEO_BASE_ADDRESS
printMemory:
	pushad
	pushfd
	add		ecx, ebx
	xor		edx, edx
printMemory_nextByte:	
	mov     al, byte [ebx]
	call	byteToStr
	add     edi, 2
	call	print
	inc     ebx
	cmp     ebx, ecx
	je      printMemory_End
	inc		edx
	cmp     edx, 16
	jne     printMemory_nextByte
	add     edi, (80 - 16 * 3) * 2
	xor		edx, edx
	jmp     printMemory_nextByte
printMemory_End:	
	popfd
	popad
	ret  

; Params (stack):
; number											- 4 bytes
; position = y * 160 + x * 2 + VIDEO_BASE_ADDRESS	- 4 bytes
; color
printNumber:
	push	eax
	push	esi
	push	edi
	mov     eax, DWORD [esp + 24]
	mov     edi, DWORD [esp + 20]
	call	byteToStr
	mov     eax, DWORD [esp + 16]
	call	print
	pop		edi
	pop		esi
	pop		eax
	
	ret		12

; Params (stack):
; number											- 4 bytes
; position = y * 160 + x * 2 + VIDEO_BASE_ADDRESS	- 4 bytes
; color
printNumberInt:
	push	eax
	push	edi
	push	esi

	mov     eax, DWORD [esp + 24]
	call	byteToStr
	mov     edi, DWORD [esp + 20]
	add		edi, 12
	mov     eax, DWORD [esp + 16]
	call	print

	mov     eax, DWORD [esp + 24]
	shr		eax, 8
	call	byteToStr
	mov     edi, DWORD [esp + 20]
	add		edi, 8
	mov     eax, DWORD [esp + 16]
	call	print

	mov     eax, DWORD [esp + 24]
	shr		eax, 16
	call	byteToStr
	mov     edi, DWORD [esp + 20]
	add		edi, 4
	mov     eax, DWORD [esp + 16]
	call	print

	mov     eax, DWORD [esp + 24]
	shr		eax, 24
	call	byteToStr
	mov     edi, DWORD [esp + 20]
	mov     eax, DWORD [esp + 16]
	call	print

	pop		esi
	pop		edi
	pop		eax
	ret		12
