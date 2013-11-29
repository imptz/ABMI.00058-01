elfFormat:
	mov		esi, ELF_PARSING_STRING
	mov		edi, 7 * 160 + 0 * 2 + VIDEO_BASE_ADDRESS
	mov		eax, COLOR_INFO
	call	print	

	jmp $

	mov		eax, DWORD [SYSTEM_DATA_ADDRESS]
	add		eax, 0x0000003c
	mov		ebx, DWORD [eax]
	add		ebx, DWORD [SYSTEM_DATA_ADDRESS]
	xor		edx,edx
	mov		dx, [ebx + 6]
	mov		edi, [ebx + 0x34]
	push	edi

	add		ebx, 0xf8

	; sohranenie adresa nachala obraza
	mov		eax, DWORD [ebx + 0x0c]
	add		eax, edi
;	mov		DWORD [ADDRESS_POINTER_IMAGE_START], eax
;	mov		DWORD [ADDRESS_POINTER_IMAGE_LENGTH], 0
	; =============================================

elf_moveLoop:
	cmp     edx,0
	je      moveEnd
	mov     ecx, [ebx + 0x10]
	mov     eax, [ebx + 0x0c]

	pop     edi
	push    edi
	add     edi,eax

	; dlinna obraza
;	mov		DWORD [ADDRESS_POINTER_IMAGE_LENGTH], edi
;	add		DWORD [ADDRESS_POINTER_IMAGE_LENGTH], ecx
	; =============================================

	mov     esi, [ebx + 0x14]
	add		esi, DWORD [SYSTEM_DATA_ADDRESS]
	cld
	rep		movsb   
	add     ebx, 40
	dec     edx  
	jmp     moveLoop
elf_moveEnd:

;=================================================
;pushad
;mov		ebx, ADDRESS_POINTER_IMAGE_START
;mov		ecx, 4
;mov		edi, 10 * 160 + 0 * 2 + VIDEO_BASE_ADDRESS
;call	printMemory
;mov		ebx, ADDRESS_POINTER_IMAGE_LENGTH
;mov		ecx, 4
;mov		edi, 10 * 160 + 20 * 2 + VIDEO_BASE_ADDRESS
;call	printMemory
;popad
;=================================================

	pop     edx
	push	eax
	mov		eax, DWORD [SYSTEM_DATA_ADDRESS]
	add		eax, 0x0000003c
	mov		ebx, DWORD [eax]
	mov		eax, DWORD [SYSTEM_DATA_ADDRESS]
	add		eax, 0x00000028
	add		ebx, eax
	pop		eax

	mov     ecx,[ebx]
	add     edx, ecx
	
	jmp     edx

ELF_PARSING_STRING			db 'ELF format detected, parsing ... ', 0
ELF_PARSING_STRING_LENGTH	equ	($ - ELF_PARSING_STRING - 1)
