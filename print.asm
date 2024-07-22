extern	DISPLAY	;外部函数
[BITS 16]
[section .text]
global	_start
global	myprint

_start:
	call DISPLAY	;asm call c function
	jmp $
myprint:
	mov ax,0xb800
	mov es,ax
	mov byte [es:0xb0], 'C'
	mov byte [es:0xb1], 0x06
	mov byte [es:0xb2], 'K'
	mov byte [es:0xb3], 0x07
	mov byte [es:0xb4], 'e'
	mov byte [es:0xb5], 0x06
	mov byte [es:0xb6], 'r'
	mov byte [es:0xb7], 0x05
	mov byte [es:0xb8], 'n'
	mov byte [es:0xb9], 0x04
	mov byte [es:0xba], 'e'
	mov byte [es:0xbb], 0x03
	mov byte [es:0xbc], 'l'
	mov byte [es:0xbd], 0x02
	ret
