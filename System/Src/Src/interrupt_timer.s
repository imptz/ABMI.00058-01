[BITS 32]
[section .text]

[EXTERN staticTimerIrqHandler]

[GLOBAL asmTimerInterruptHandler]

asmTimerInterruptHandler:
	pushfd
	pushad

	call	staticTimerIrqHandler

	mov		al, 0x20
	out		0x20, al

	popad
	popfd
	iretd
