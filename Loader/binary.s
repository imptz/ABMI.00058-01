;
;
;	ме пюанрючыхи лндскэ. йнд ме хглемем дкъ пюанрш я дюммшлх юбрнлюрхвеяйнцн нопедекемхъ назелю нгс
;
;

BIN_FORMAT_START_ADDRESS	EQU	0x00100000

binFormat:
	mov		esi, BIN_FORMAT_STRING
	mov		edi, 8 * 160 + 0 * 2 + VIDEO_BASE_ADDRESS
	mov		eax, COLOR_INFO
	call	print

	sub		DWORD [SYSTEM_DATA_ADDRESS], 4
	mov		eax, DWORD [SYSTEM_DATA_ADDRESS]
	mov		ecx, DWORD [eax]

	add		DWORD [SYSTEM_DATA_ADDRESS], 4
	
	mov     esi, DWORD [SYSTEM_DATA_ADDRESS]
	mov     edi, BIN_FORMAT_START_ADDRESS
	cld
	rep		movsb   

	jmp		BIN_FORMAT_START_ADDRESS

BIN_FORMAT_STRING	db 'BIN format', 0

