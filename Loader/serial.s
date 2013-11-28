%define SERIAL_CONNECTION_WAIT_COUNT 384 - 196
%define SERIAL_GET_BYTE_WAIT_COUNT 10000
%define COM_PORT 0x03f8

%macro OUT_PORT 2
  mov    dx,%1
  mov    al,%2
  out    dx,al
%endmacro

%macro IN_PORT 1
  mov    dx,%1
  in     al,dx
%endmacro

; ожидает получения байта и возвращает его в al
ReadByte:
 IN_PORT  COM_PORT + 5
 test     al,0x01
 je       ReadByte 
 IN_PORT  COM_PORT
 ret

; ожидает освобождения регистра передачи и передает байт из ah
WriteByte:
 IN_PORT  COM_PORT + 5
 test     al,0x20
 je       WriteByte 
 OUT_PORT  COM_PORT, ah
 ret

; инициализирует последовательный порт
initSerial:

	OUT_PORT  COM_PORT + 1, 0x00 
	OUT_PORT  COM_PORT + 3, 0x80 
	OUT_PORT  COM_PORT + 0, 0x01 
	OUT_PORT  COM_PORT + 1, 0x00
	OUT_PORT  COM_PORT + 3, 0x03 
	OUT_PORT  COM_PORT + 2, 0x00 
	OUT_PORT  COM_PORT + 4, 0x00

	ret

; ожидает получения байта
; если байт не получен за SERIAL_GET_BYTE_WAIT_COUNT, то возвращает в ah 0
; иначе возвращается в al - полученный байт, в ah - 1
serial_getByte:
	push	ecx
	mov		ecx, SERIAL_GET_BYTE_WAIT_COUNT
serial_getByteWait:
	IN_PORT COM_PORT + 5
	test    al,0x01
	jne      serial_getByteProccess
	dec		ecx
	cmp		ecx, 0
	jne		serial_getByteWait
	mov		ah, 0
	pop		ecx
	ret

serial_getByteProccess:
	IN_PORT  COM_PORT	
	mov		ah, 1
	pop		ecx
	ret

; ожидает установки соединения 
; если соединение не устанавливается за SERIAL_CONNECTION_WAIT_COUNT, то возвращает в al 0
; если соединение устанавливается, то возвращает в al 1

%define SERIAL_CONNECTION_CODE 0x12345678
%define SERIAL_CONNECTION_CODE_ANSWER 0x87654321

serial_connectionCode:	dd	0x00000000
serial_connectionCodeCount:	dd	0x00000000
serial_connectionWaitCount:	dd	0x00000000

counterPosition:	dd	0x00000000

serial_connection:
	mov		DWORD [counterPosition], eax
	mov		DWORD [serial_connectionWaitCount], SERIAL_CONNECTION_WAIT_COUNT
	xor		eax, eax
	mov		DWORD [serial_connectionCodeCount], eax
	mov		DWORD [serial_connectionCode], eax
serial_connectionWait:

	push	eax
	mov		eax, DWORD [serial_connectionWaitCount]
	shr		eax, 6
	inc		eax
	push	eax
	push	DWORD [counterPosition]
	push	COLOR_INFO
	call	printNumber
	pop		eax
	
	mov		eax, DWORD [serial_connectionCodeCount]
	cmp		eax, 4
	je		serial_connectionProccess
	call	serial_getByte
	cmp     ah,0x01
	jne     serial_connectionPreWait
	inc		DWORD [serial_connectionCodeCount]
	mov		ebx, DWORD [serial_connectionCode]
	shl		ebx, 8
	mov		bl, al
	mov		DWORD [serial_connectionCode], ebx
	jmp		serial_connectionWait

serial_connectionPreWait:
	dec		DWORD [serial_connectionWaitCount]
	mov		ecx, DWORD [serial_connectionWaitCount]
	cmp		ecx, 0
	jne		serial_connectionWait
	mov		al, 0
	ret

serial_connectionProccess:
	mov		eax, DWORD [serial_connectionCode]

	cmp		eax, SERIAL_CONNECTION_CODE
	jne		serial_connectionProccessFalse

	mov		ebx, SERIAL_CONNECTION_CODE_ANSWER
	mov		ah, bl
	shr		ebx, 8

 	call	WriteByte
	mov		ah, bl
	shr		ebx, 8
	call	WriteByte
	mov		ah, bl
	shr		ebx, 8
 	call	WriteByte
	mov		ah, bl
 	call	WriteByte
	mov		al, 1
	ret

serial_connectionProccessFalse:
	mov		al, 0
	ret

; input param: eax - FRAME_ADDRESS

FRAME_ADDRESS		dd	0
FRAME_COMMAND		dd	0
FRAME_DATA_LENGTH	dd	0
FRAME_DATA			dd	0
crc					dw	0

serial_getCommand:

	mov		DWORD [FRAME_ADDRESS], eax

serial_getCommand_start:
	mov		eax, DWORD [FRAME_ADDRESS]
	mov		DWORD [loading_data_flag], 0
	mov		DWORD [FRAME_COMMAND], eax

	inc		eax
	mov		DWORD [FRAME_DATA_LENGTH], eax
	add		eax, 4
	mov		DWORD [FRAME_DATA], eax

	mov		eax, [FRAME_COMMAND]
	mov		BYTE [eax], 0
	mov		eax, [FRAME_DATA_LENGTH]
	mov		DWORD [eax], 0
	mov		WORD [crc], 0

	; get command
	call	serial_getByte
	cmp		ah, 0
	je		serial_getCommand_start
	mov		edi, DWORD [FRAME_COMMAND]
	mov		BYTE [edi], al

	mov		esi, CLEAR_STRING
	mov		edi, 5 * 160 + 0 * 2 + VIDEO_BASE_ADDRESS
	call	print	

	; get data length
	mov		ebx, [FRAME_DATA_LENGTH]
serial_getCommand_dataLength:
	call	serial_getByte
	cmp		ah, 0
	je		serial_getCommand_start
	mov		BYTE [ebx], al
	inc		ebx
	mov		edi, [FRAME_DATA_LENGTH]
	add		edi, 4
	cmp		ebx, edi

	jne		serial_getCommand_dataLength

	mov		edi, [FRAME_DATA_LENGTH]
	cmp		DWORD [edi], 0
	je		serial_getCommand_crc


	mov		esi, LOAD_IMAGE_STRING
	mov		edi, 5 * 160 + 0 * 2 + VIDEO_BASE_ADDRESS
	mov		eax, COLOR_INFO
	call	print

	mov		DWORD [loading_data_flag], 1

	; get data
	mov		ebx, [FRAME_DATA]
	mov		edi, [FRAME_DATA_LENGTH]
	mov		ecx, [edi]
	add		ecx, ebx

serial_getCommand_data:

;mov		edi, ebx
;sub		edi, [FRAME_DATA]
;push	edi
;push	5 * 160 + LOAD_IMAGE_STRING_LENGTH * 2 + VIDEO_BASE_ADDRESS
;push	COLOR_KRASNIY
;call	printNumberInt

	call	serial_getByte
	cmp		ah, 0
	je		serial_getCommand_start
	mov		BYTE [ebx], al
	inc		ebx
	cmp		ebx, ecx
	jne		serial_getCommand_data


	; get data crc
serial_getCommand_crc:
	mov		ebx, crc
serial_getCommand_crcLoop:
	call	serial_getByte
	cmp		ah, 0
	je		serial_getCommand_start
	mov		BYTE [ebx], al
	inc		ebx
	cmp		ebx, crc + 2
	jne		serial_getCommand_crcLoop

	mov		ecx, DWORD [FRAME_DATA_LENGTH]
	mov		ecx, DWORD [ecx]
	add		ecx, 5
	mov		esi, [FRAME_ADDRESS]
	call	calcCRC16

	cmp		ax, WORD [crc]
	je		serial_getCommand_return
	
	cmp		DWORD [loading_data_flag], 1
	jne		serial_getCommand_crcEnd
	mov		esi, FAILED_STRING
	mov		edi, 5 * 160 + LOAD_IMAGE_STRING_LENGTH * 2 + VIDEO_BASE_ADDRESS
	mov		eax, COLOR_FAILED
	call	print		

serial_getCommand_crcEnd:
	mov		DWORD [loading_data_flag], 0
	jmp		serial_getCommand_start

serial_getCommand_return:

	mov		ebx, SERIAL_CONNECTION_CODE_ANSWER
	mov		ah, bl
	shr		ebx, 8

 	call	WriteByte
	mov		ah, bl
	shr		ebx, 8
	call	WriteByte
	mov		ah, bl
	shr		ebx, 8
 	call	WriteByte
	mov		ah, bl
 	call	WriteByte

	cmp		DWORD [loading_data_flag], 1
	jne		serial_getCommandEnd__
	mov		esi, DONE_STRING
	mov		edi, 5 * 160 + LOAD_IMAGE_STRING_LENGTH * 2 + VIDEO_BASE_ADDRESS
	mov		eax, COLOR_DONE
	call	print		

serial_getCommandEnd__:
	ret

LOAD_IMAGE_STRING					db 'loading image ... ', 0
LOAD_IMAGE_STRING_LENGTH			equ	($ - LOAD_IMAGE_STRING - 1)
CLEAR_STRING						db '                                             ', 0

loading_data_flag	dd	0

%include "crc.s"

