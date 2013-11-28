[BITS 32]
[EXTERN main]
[GLOBAL _start]
_start:
	mov		esp, 1024*1024*200
	call	main