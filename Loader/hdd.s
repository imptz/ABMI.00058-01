; -----------------------------------------------------------------------------
; readsectors -- Read sectors on the hard drive
; IN:	EAX = starting sector to read
;	ECX = number of sectors to read (1 - 256)
;	EDI = memory location to store sectors
; OUT:	EAX = EAX + number of sectors that were read
;	ECX = number of sectors that were read (0 on error)
;	EDI = EDI + (number of sectors * 512)
;	All other registers preserved
readsectors:
	push edx
	push ecx
	push ebx
	push eax

	push ecx		; Save RCX for use in the read loop
	mov ebx, ecx		; Store number of sectors to read
	cmp ecx, 256
	jg readsectors_fail	; Over 256? Fail!
	jne readsectors_skip	; Not 256? No need to modify CL
	xor ecx, ecx		; 0 translates to 256
readsectors_skip:
	push eax		; Save RAX since we are about to overwrite it
	mov dx, 0x01F2		; 0x01F2 - Sector count Port 7:0
	mov al, cl		; Read CL sectors
	out dx, al
	pop eax			; Restore RAX which is our sector number
	inc dx			; 0x01F3 - LBA Low Port 7:0
	out dx, al
	inc dx			; 0x01F4 - LBA Mid Port 15:8
	shr eax, 8
	out dx, al
	inc dx			; 0x01F5 - LBA High Port 23:16
	shr eax, 8
	out dx, al
	inc dx			; 0x01F6 - Device Port. Bit 6 set for LBA mode, Bit 4 for device (0 = master, 1 = slave), Bits 3-0 for LBA "Extra High" (27:24)
	shr eax, 8
	and al, 00001111b 	; Clear bits 4-7 just to be safe
	or al, 01110000b	; Turn bit 6 on since we want to use LBA addressing, leave device at 0 (master)
	out dx, al
	inc dx			; 0x01F7 - Command Port
	mov al, 0x20		; Read sector(s). 0x24 if LBA48
	out dx, al

	mov ecx, 4
readsectors_wait:
	in al, dx		; Read status from 0x01F7
	test al, 0x80		; BSY flag set?
	jne readsectors_retry
	test al, 0x08		; DRQ set?
	jne readsectors_dataready
readsectors_retry:
	dec ecx
	jg readsectors_wait
readsectors_nextsector:
	in al, dx		; Read status from 0x01F7
	test al, 0x80		; BSY flag set?
	jne readsectors_nextsector
	test al, 0x21		; ERR or DF set?
	jne readsectors_fail

readsectors_dataready:
	sub dx, 7		; Data port (0x1F0)
	mov ecx, 256		; Read 
	rep insw		; Copy a 512 byte sector to RDI
	add dx, 7		; Set DX back to status register (0x01F7)
	in al, dx		; Delay ~400ns to allow drive to set new values of BSY and DRQ
	in al, dx
	in al, dx
	in al, dx

	dec ebx			; RBX is the "sectors to read" counter
	cmp ebx, 0
	jne readsectors_nextsector

	pop ecx
	pop eax
	pop ebx
	add eax, ecx
	pop ecx
	pop edx
ret

readsectors_fail:
	pop ecx
	pop eax
	pop ebx
	pop ecx
	pop edx
	xor ecx, ecx		; Set RCX to 0 since nothing was read
ret

