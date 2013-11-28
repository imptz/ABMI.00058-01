[BITS 32]
[section .text]
[GLOBAL defaultProcessorInterruptHandler]
[GLOBAL defaultHardware0InterruptHandler]
[GLOBAL defaultHardware1InterruptHandler]

defaultProcessorInterruptHandler:
	iretd

defaultHardware0InterruptHandler:
	pushfd
	push	eax
	
	mov		al,0x20
	out		0x20,al
	pop		eax
	popfd
	iretd

defaultHardware1InterruptHandler:
	pushfd
	push	eax

	mov		al,0x20
	out		0xa0,al
	pop		eax
	popfd
	iretd
