[BITS 32]
[section .text]

[EXTERN staticSerialPort1IrqHandler]
[EXTERN staticSerialPort2IrqHandler]
[EXTERN staticSerialPort3IrqHandler]

[GLOBAL asmSerialPort1InterruptHandler]
[GLOBAL asmSerialPort2InterruptHandler]
[GLOBAL asmSerialPort3InterruptHandler]

asmSerialPort1InterruptHandler:
	pushfd
	pushad

	call	staticSerialPort1IrqHandler

	mov		al, 0x20
	out		0x20, al

	popad
	popfd
	iretd

asmSerialPort2InterruptHandler:
	pushfd
	pushad
	
	call	staticSerialPort2IrqHandler

	mov		al, 0x20
	out		0x20, al
	
	popad
	popfd
	iretd

asmSerialPort3InterruptHandler:
	pushfd
	pushad

	call	staticSerialPort3IrqHandler

	mov		al, 0x20
	out		0x20, al

	popad
	popfd
	iretd
