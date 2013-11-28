peFormat:
	mov		esi, UNPUCKING_STRING
	mov		edi, 7 * 160 + 0 * 2 + VIDEO_BASE_ADDRESS
	mov		eax, COLOR_INFO
	call	print		

	mov		eax, DWORD [SYSTEM_DATA_ADDRESS]
	add		eax, 0x0000003c
	mov		ebx, DWORD [eax]
	add		ebx, DWORD [SYSTEM_DATA_ADDRESS]
	xor		edx,edx
	mov		dx, [ebx + 6]

	mov		edi, [ebx + 0x34]
	push	edi

	add		ebx, 0xf8

	mov		DWORD [KERNEL_LAST_ADDRESS], 0	

moveLoop:
	cmp     edx,0
	je      moveEnd
	mov     ecx, [ebx + 0x10]
	mov     eax, [ebx + 0x0c]

	pop     edi
	push    edi
	add     edi,eax

	mov     esi, [ebx + 0x14]
	add		esi, DWORD [SYSTEM_DATA_ADDRESS]
	cld
	mov		DWORD [KERNEL_LAST_ADDRESS], edi 

	cmp		ecx, 0
	jne		moveS

	mov     ecx, [ebx + 0x08]
	xor		eax, eax
	add		DWORD [KERNEL_LAST_ADDRESS], ecx
	rep		stosb
	jmp		moveS_

moveS:
	add		DWORD [KERNEL_LAST_ADDRESS], ecx
	rep		movsb   

moveS_:

	add     ebx, 40
	dec     edx  
	jmp     moveLoop
moveEnd:

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
	push	edx 

	mov		esi, DONE_STRING
	mov		edi, 7 * 160 + UNPUCKING_STRING_LENGTH * 2 + VIDEO_BASE_ADDRESS
	mov		eax, COLOR_DONE
	call	print		

	mov		esi, SYSTEM_START_STRING
	mov		edi, 8 * 160 + 0 * 2 + VIDEO_BASE_ADDRESS
	mov		eax, COLOR_INFO
	call	print		


;	push	DWORD [START_FREE_MEMORY_ADDRESS]
;	pop		DWORD [KERNEL_LAST_ADDRESS]

	pop		edx
	jmp     edx

UNPUCKING_STRING			db 'PE format detected, unpacking ... ', 0
UNPUCKING_STRING_LENGTH		equ	($ - UNPUCKING_STRING - 1)
