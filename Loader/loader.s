MONITOR_LOAD_ADDRESS				EQU		0x7e00
Org MONITOR_LOAD_ADDRESS

IMAGE_SIZE:	dd	IMAGE_SIZE_C

BITS 32

	mov		eax, [LAST_MEMORY_ADDRESS]
	sub		eax, STACK_SIZE * 10
	mov     [IMAGE_LOAD_ADDRESS], eax

	mov		eax, [LAST_MEMORY_ADDRESS]
	mov		[MEMORY_SIZE_ADDRESS], eax
	 
	mov		esi, VERSION_STRING
	mov		edi, 3 * 160 + 0 * 2 + VIDEO_BASE_ADDRESS
	mov		eax, COLOR_GOLUBOY
	call	print

; максимальный размер загрузчика 256 секторов !!!

	mov		esi, SERIAL_CONNECTION_STRING
	mov		edi, 4 * 160 + 0 * 2 + VIDEO_BASE_ADDRESS
	mov		eax, COLOR_INFO
	call	print

	call	initSerial

	mov		eax, 4 * 160 + SERIAL_CONNECTION_STRING_LENGTH * 2 + VIDEO_BASE_ADDRESS
	call	serial_connection
	cmp		al, 0
	je		connectionFalse

	mov		esi, DONE_STRING
	mov		edi, 4 * 160 + SERIAL_CONNECTION_STRING_LENGTH * 2 + VIDEO_BASE_ADDRESS
	mov		eax, COLOR_DONE
	call	print
	jmp		monitorCode

connectionFalse:
	mov		esi, FAILED_STRING
	mov		edi, 4 * 160 + SERIAL_CONNECTION_STRING_LENGTH * 2 + VIDEO_BASE_ADDRESS
	mov		eax, COLOR_FAILED
	call	print

	mov		esi, LOAD_SYSTEM_FROM_HDD
	mov		edi, 5 * 160 + 0 * 2 + VIDEO_BASE_ADDRESS
	mov		eax, COLOR_INFO
	call	print

	mov		eax, SYSTEM_IMAGE_HDD_SECTORS
	mov		ecx, 1
	mov		edi, [IMAGE_LOAD_ADDRESS]
	call	readsectors	
	cmp		ecx, 0
	je		errorReadSystemImage
	mov		ecx, [IMAGE_LOAD_ADDRESS]
	mov		ecx, DWORD [ecx]
	push	ecx 
	pop		DWORD [imageSize]
	shr		DWORD [imageSize], 9
	inc		DWORD [imageSize]
	inc		DWORD [imageSize]
	inc		DWORD [imageSize]
	inc		DWORD [imageSize]

	mov		DWORD [readSectorNumber], SYSTEM_IMAGE_HDD_SECTORS
	inc		DWORD [readSectorNumber]

	mov		ecx, [IMAGE_LOAD_ADDRESS]
	add		ecx, 512
	mov		DWORD [readSectorAddress], ecx

readSystemImageLoop:
	cmp		DWORD [imageSize], 1	
	jle		readSystemImageEnd
	mov		eax, DWORD [readSectorNumber]
	mov		ecx, 1
	mov		edi, DWORD [readSectorAddress]
	call	readsectors	
	cmp		ecx, 0
	je		errorReadSystemImage
	dec		DWORD [imageSize]
	inc		DWORD [readSectorNumber]
	add		DWORD [readSectorAddress], 512
	jmp		readSystemImageLoop

readSystemImageEnd:
	mov		esi, DWORD [IMAGE_LOAD_ADDRESS]
	mov		DWORD [SYSTEM_DATA_ADDRESS], esi

	mov		esi, DONE_STRING
	mov		edi, 5 * 160 + LOAD_SYSTEM_FROM_HDD_STRING_LENGTH * 2 + VIDEO_BASE_ADDRESS
	mov		eax, COLOR_DONE
	call	print

	jmp		runSystem

errorReadSystemImage:
	mov		esi, FAILED_STRING
	mov		edi, 5 * 160 + LOAD_SYSTEM_FROM_HDD_STRING_LENGTH * 2 + VIDEO_BASE_ADDRESS
	mov		eax, COLOR_FAILED
	call	print
	jmp		$

imageSize			dd	0
readSectorNumber	dd	0
readSectorAddress	dd	0

COMMAND_REBOOT			equ	1
COMMAND_LOAD_AND_RUN	equ	2
COMMAND_LOAD_AND_WRITE	equ	3

monitorCode:

	mov		eax, [IMAGE_LOAD_ADDRESS]
	call	serial_getCommand
	mov		eax, [IMAGE_LOAD_ADDRESS]

	cmp		BYTE [eax], COMMAND_REBOOT
	je		rebooting
	cmp		BYTE [eax], COMMAND_LOAD_AND_RUN
	je		loadAndRun
	cmp		BYTE [eax], COMMAND_LOAD_AND_WRITE
	je		loadAndWrite

	mov		esi, UNKNOWN_COMMAND_STRING
	mov		edi, 5 * 160 + 0 * 2 + VIDEO_BASE_ADDRESS
	mov		eax, COLOR_FAILED
	call	print
	jmp		$

rebooting:
	mov		esi, COMMAND_REBOOT_STRING
	mov		edi, 5 * 160 + 0 * 2 + VIDEO_BASE_ADDRESS
	mov		eax, COLOR_INFO
	call	print
	jmp		reboot

reboot:	
	mov		ecx, 0x1000000
rebootingTimeOut:
	dec		ecx
	cmp		ecx, 0
	jne		rebootingTimeOut

	mov		al, 0x8d
	out		0x70, al
	in		al, 0x70
rebootLoop:
	in      al,0x64
	test    al,2
	jnz	    rebootLoop
	mov     al,0xfe
	out     0x64,al
	jmp		$

loadAndRun:
	add		eax, 5
	mov		DWORD [SYSTEM_DATA_ADDRESS], eax
;	mov		esi, LOAD_AND_RUN_STRING
;	mov		edi, 5 * 160 + 0 * 2 + VIDEO_BASE_ADDRESS
;	call	print
	jmp		runSystem

loadAndWrite:
	add		eax, 5
	mov		DWORD [SYSTEM_DATA_ADDRESS], eax

	mov		esi, DWORD [SYSTEM_DATA_ADDRESS]
	add		esi, 4
	mov		eax, DWORD [SYSTEM_DATA_ADDRESS]
	mov		ecx, DWORD [eax]
	call	calcCRC16

	mov		ecx, DWORD [SYSTEM_DATA_ADDRESS]
	mov		ecx, DWORD [ecx]
	mov		ebx, DWORD [SYSTEM_DATA_ADDRESS]
	add		ebx, 4
	add		ebx, ecx

	cmp		eax, DWORD [ebx]
	jne		runSystemCrcFalse

	mov		esi, WRITE_SYSTEM_IMAGE_STRING
	mov		edi, 6 * 160 + 0 * 2 + VIDEO_BASE_ADDRESS
	mov		eax, COLOR_INFO
	call	print

	mov		eax, DWORD [SYSTEM_DATA_ADDRESS]
	push	DWORD [eax]
	pop		DWORD [imageSize]
	add		DWORD [imageSize], 8
	shr		DWORD [imageSize], 9
	inc		DWORD [imageSize]
	mov		DWORD [readSectorNumber], SYSTEM_IMAGE_HDD_SECTORS
	push	DWORD [SYSTEM_DATA_ADDRESS]
	pop		DWORD [readSectorAddress]

writeSystemImageLoop:
	cmp		DWORD [imageSize], 0	
	je		writeSystemImageEnd
	mov		eax, DWORD [readSectorNumber]
	mov		ecx, 1
	mov		esi, DWORD [readSectorAddress]
	call	writesectors	
	cmp		ecx, 0
	je		errorWriteSystemImage
	dec		DWORD [imageSize]
	inc		DWORD [readSectorNumber]
	add		DWORD [readSectorAddress], 512
	jmp		writeSystemImageLoop

errorWriteSystemImage:
	mov		esi, FAILED_STRING
	mov		edi, 6 * 160 + WRITE_SYSTEM_IMAGE_STRING_LENGTH * 2 + VIDEO_BASE_ADDRESS
	mov		eax, COLOR_FAILED
	call	print
	jmp		monitorCode

writeSystemImageEnd:
	mov		esi, DONE_STRING
	mov		edi, 6 * 160 + WRITE_SYSTEM_IMAGE_STRING_LENGTH * 2 + VIDEO_BASE_ADDRESS
	mov		eax, COLOR_DONE
	call	print
	jmp		reboot

SYSTEM_DATA_ADDRESS	dd	0

runSystem:
	mov		esi, TEST_IMAGE_CRC_STRING
	mov		edi, 6 * 160 + 0 * 2 + VIDEO_BASE_ADDRESS
	mov		eax, COLOR_INFO
	call	print		

	mov		esi, DWORD [SYSTEM_DATA_ADDRESS]
	add		esi, 4
	mov		eax, DWORD [SYSTEM_DATA_ADDRESS]
	mov		ecx, DWORD [eax]
	call	calcCRC16

	mov		ecx, DWORD [SYSTEM_DATA_ADDRESS]
	mov		ecx, DWORD [ecx]	
	mov		ebx, DWORD [SYSTEM_DATA_ADDRESS]
	add		ebx, 4
	add		ebx, ecx

	cmp		eax, DWORD [ebx]
	je		runSystemCrcTrue

runSystemCrcFalse:
	mov		esi, FAILED_STRING
	mov		edi, 6 * 160 + TEST_IMAGE_CRC_STRING_LENGTH * 2 + VIDEO_BASE_ADDRESS
	mov		eax, COLOR_FAILED
	call	print		
	jmp		$

runSystemCrcTrue:
	mov		esi, DONE_STRING
	mov		edi, 6 * 160 + TEST_IMAGE_CRC_STRING_LENGTH * 2 + VIDEO_BASE_ADDRESS
	mov		eax, COLOR_DONE
	call	print		

	add		DWORD [SYSTEM_DATA_ADDRESS], 4
	mov		eax, DWORD [SYSTEM_DATA_ADDRESS]
	mov		ebx, DWORD [eax]	
	cmp		bx, 0x5a4d
	je		_peFormat
	cmp		ebx, 0x464c457f
	je		_elfFormat
	jmp		binFormat
_peFormat:
	jmp		peFormat
_elfFormat:
	jmp		elfFormat

IMAGE_LOAD_ADDRESS	dd	0

STACK_SIZE					EQU		1024 * 1024
SYSTEM_IMAGE_HDD_SECTORS	EQU		1000

VERSION_STRING							db 'monitor 1.3', 0
SERIAL_CONNECTION_STRING				db 'serial connection ... ',0
SERIAL_CONNECTION_STRING_LENGTH			equ	($ - SERIAL_CONNECTION_STRING - 1)
DONE_STRING								db 'done', 0
FAILED_STRING							db 'failed', 0
COMMAND_REBOOT_STRING					db 'reboot ...                     ', 0
UNKNOWN_COMMAND_STRING					db 'unknown command                ', 0
LOAD_SYSTEM_FROM_HDD					db 'load image from hdd ... ', 0
LOAD_SYSTEM_FROM_HDD_STRING_LENGTH		equ	($ - LOAD_SYSTEM_FROM_HDD - 1)
SYSTEM_START_STRING						db 'start the system ... ', 0
TEST_IMAGE_CRC_STRING					db 'test checksum of the image ... ', 0
TEST_IMAGE_CRC_STRING_LENGTH			equ	($ - TEST_IMAGE_CRC_STRING - 1)
ERROR_READ_SYSTEM_IMAGE_STRING			db 'Error reading system image', 0
WRITE_SYSTEM_IMAGE_STRING				db 'writing system image ... ', 0
WRITE_SYSTEM_IMAGE_STRING_LENGTH		equ	($ - WRITE_SYSTEM_IMAGE_STRING - 1)

%include "const.inc"
%include "display.s"
%include "serial.s"
%include "hdd.s"
%include "hddWrite.s"
%include "binary.s"
%include "pe.s"
%include "elf.s"

IMAGE_SIZE_C equ ($-$$) / 512 + 1
