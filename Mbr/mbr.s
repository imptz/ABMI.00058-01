; ----- Master Boot Record -------------------------------------------
 
Org 0x7c00
BITS 16

main:
	mov	ax, SEGMENTS
	mov	ds, ax
	mov	es, ax
	mov	fs, ax
	mov	gs, ax
	mov	ss, ax
	mov	sp, REAL_MODE_STACK_ADDRESS

clearScreen:
	push	es
	mov     bx,0xb800
	mov     es,bx
	xor     di,di
	mov     ax,COLOR_BELIY
	mov     cx,2000
	cld
clearScreen_next:
	stosw
	loop    clearScreen_next		
	pop	es

	mov	si, VERSION_STRING
	mov	di, 0 * 160 + 0 * 2
	mov     ax, COLOR_GOLUBOY
	call	print

getMemoryMap:

	mov	si, GET_MEMORY_MAP_STRING
	mov	di, 1 * 160 + 0 * 2
	mov	ax, COLOR_INFO
	call	print

	sti
	xor	ebx, ebx
	mov	[E820_ITEMS_COUNTER_ADDRESS], ebx
	mov	di, E820_MEMORY_MAP_ADDRESS
	
nextItem:
	mov	eax, E820_FUNCTION_CODE
	mov	edx, E820_SIGNATURE
	mov	ecx, E820_ITEM_SIZE
	int	0x15
	jc	getMemoryMapFailed
	
	cmp	eax, E820_SIGNATURE
	jne	getMemoryMapFailed
	
	mov	eax, [E820_ITEMS_COUNTER_ADDRESS]
	cmp	eax, E820_MAX_ITEMS_COUNTER
	jnl	getMemoryMapDone
	
	inc	dword [E820_ITEMS_COUNTER_ADDRESS]
	add	di, E820_ITEM_SIZE
	
	test	ebx, ebx
	jnz	nextItem
getMemoryMapDone:
	jmp	calcMemorySize

getMemoryMapFailed:
	cli
	mov	si, FAILED_STRING
	mov	di, 1 * 160 + GET_MEMORY_MAP_STRING_LENGTH * 2
	mov	ax, COLOR_FAILED
	call	print
	jmp	$

calcMemorySize:

	mov	eax, [E820_ITEMS_COUNTER_ADDRESS]
	cmp	eax, E820_MAX_BIG_MEMORY_ITEMS_NUMBER
	jle	getMemoryMapFailed

	mov	eax, dword [E820_MAX_BIG_MEMORY_ITEMS_OFFSET]
	add	eax, 0x00100000
	mov	dword [LAST_MEMORY_ADDRESS], eax

	mov	si, DONE_STRING
	mov	di, 1 * 160 + GET_MEMORY_MAP_STRING_LENGTH * 2
	mov	ax, COLOR_DONE
	call	print

	mov	si, LOAD_MONITOR_STRING
	mov	di, 2 * 160 + 0 * 2
	mov	ax, COLOR_INFO
	call	print	

readMonitor:
	mov	ah, 2
	mov	al, 1
	mov	cx, MONITOR_SECTOR_NUMBER
	mov	dx, MONITOR_DRIVE_NUMBER
	mov	bx, MONITOR_LOAD_ADDRESS
	int	0x13
	jc	readMonitorFailed
	cmp	ah, 0
	jne	readMonitorFailed

	cmp	DWORD [MONITOR_LOAD_ADDRESS], 1
	jle	startImage
	
	mov	eax, DWORD [MONITOR_LOAD_ADDRESS]
	sub	eax, 1
	mov	ah, 2
	mov	cx, MONITOR_SECTOR_NUMBER + 1
	mov	dx, MONITOR_DRIVE_NUMBER
	mov	bx, MONITOR_LOAD_ADDRESS + 512
	int	0x13
	jc	readMonitorFailed
	cmp	ah, 0
	jne	readMonitorFailed

startImage:
	mov	si, DONE_STRING
	mov	di, 2 * 160 + LOAD_MONITOR_STRING_LENGTH * 2
	mov	ax, COLOR_DONE
	call	print

	cli
	mov     al,0x80
	out     0x70,al

	jmp	start32

start32:
	; ----- copy gdt_table to address 0x00000000 ---------------------
	mov     si,_gdt
	mov	di, GDT_ADDRESS
	mov     ecx,(_gdt_end-_gdt)
	rep     movsb

	; ----- protected mode init --------------------------------------
	o32 lgdt    [addressGDTLoad]
	mov     eax,cr0
	or      eax,0x00000001
	mov     cr0,eax
	jmp     8:bootPM

BITS 32

bootPM:
	mov     eax,16
	mov     ds,ax
	mov     es,ax
	mov     ss,ax

	call    OBF_8042
	mov     al,0xD1
	out     0x64,al
	call    OBF_8042
	mov     al,0x03
	out     0x60,al
	call    OBF_8042

	mov	eax, [LAST_MEMORY_ADDRESS]
	sub	eax, 4
	mov     esp, eax

	jmp	MONITOR_LOAD_ADDRESS + 4
	
readMonitorFailed:

	mov	si, FAILED_STRING
	mov	di, 2 * 160 + LOAD_MONITOR_STRING_LENGTH * 2
	mov	ax, COLOR_FAILED
	call	print	

	jmp $


OBF_8042:
	in      al,0x64
	test    al,2
	jnz	OBF_8042
	retn

BITS 16

print:
	push	bx
	push	es
	mov     bx,0xb800
	mov     es,bx
	cld
_print_12:
	lodsb
	cmp     al,0
	je      _print_22
	stosw
	jmp	_print_12
_print_22: 	
	pop	es
	pop	bx
	ret

VIDEO_BASE_ADDRESS	EQU	0xb8000

;COLOR_SINIY		EQU	0x0100
COLOR_ZELENIY		EQU	0x0200
COLOR_GOLUBOY		EQU	0x0300
COLOR_KRASNIY		EQU	0x0400
;COLOR_ROZOVIY		EQU	0x0500
;COLOR_ORANZHEVIY	EQU	0x0600
COLOR_BELIY		EQU	0x0700
;COLOR_SERIY		EQU	0x0800

COLOR_INFO		EQU	COLOR_BELIY
COLOR_FAILED		EQU	COLOR_KRASNIY
COLOR_DONE		EQU	COLOR_ZELENIY

SEGMENTS		EQU	0

E820_ITEM_SIZE				EQU	20
E820_ITEMS_COUNTER_ADDRESS		EQU	4096
E820_MAX_ITEMS_COUNTER			EQU	50
E820_MEMORY_MAP_ADDRESS			EQU	8192

E820_FUNCTION_CODE			EQU	0x0000e820
E820_SIGNATURE				EQU	'PAMS'

E820_MAX_BIG_MEMORY_ITEMS_NUMBER	EQU	3
E820_MAX_BIG_MEMORY_ITEMS_OFFSET	EQU	E820_MEMORY_MAP_ADDRESS + 20 * 3 + 8

MONITOR_SECTOR_NUMBER			EQU	2
MONITOR_DRIVE_NUMBER			EQU	0x80

DONE_STRING			db	'done', 0
FAILED_STRING			db	'failed', 0
VERSION_STRING			db	'mbr 2.3', 0
GET_MEMORY_MAP_STRING		db	'determining the size of RAM ... ', 0
GET_MEMORY_MAP_STRING_LENGTH	equ	($ - GET_MEMORY_MAP_STRING - 1)
LOAD_MONITOR_STRING		db	'load monitor ... ', 0
LOAD_MONITOR_STRING_LENGTH	equ	($ - LOAD_MONITOR_STRING - 1)

_gdt:
	_gdt_seg_null:
		dd     0x00000000
		dd     0x00000000
	_gdt_seg_code:
		dw     0xffff
		dw     0x0000
		db     0x00
		db     0x9a
		db     0xcf
		db     0x00
	_gdt_seg_data:
		dw     0xffff
		dw     0x0000
		db     0x00
		db     0x92
		db     0xcf
		db     0x00
_gdt_end:	

addressGDTLoad:		dw	_gdt_end - _gdt
addressGDTLoad_:	dd	GDT_ADDRESS	
IMAGE_LOAD_ADDRESS	dd	0
STACK_SIZE		EQU	1024 * 1024

GDT_ADDRESS		EQU	4096
REAL_MODE_STACK_ADDRESS	EQU	1024 * 16
LAST_MEMORY_ADDRESS	EQU	REAL_MODE_STACK_ADDRESS + 4

MONITOR_LOAD_ADDRESS	EQU	0x7e00

TIMES 510-($-$$) DB 0 

DW 0xAA55
