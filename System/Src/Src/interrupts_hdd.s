[BITS 32]
[section .text]

[EXTERN staticHddIrqHandler]

[GLOBAL asmHddInterruptHandler]

asmHddInterruptHandler:
	pushfd
	pushad
	
	call	staticHddIrqHandler

	mov		al, 0x20
	out		0x20, al
	
	popad
	popfd
	iretd
