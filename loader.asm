LOADER_BASE_ADDR    equ 0x900
SECTION loader vstart=LOADER_BASE_ADDR  ;将代码段放在0x900位置

mov ax, 0xB800
mov es, ax

;mov byte [gs:0x00], 'h' ;这样也行

mov byte [es:0xa0], 'L'
mov byte [es:0xa1], 0x06
mov byte [es:0xa2], 'o'
mov byte [es:0xa3], 0x06
mov byte [es:0xa4], 'a'
mov byte [es:0xa5], 0x06
mov byte [es:0xa6], 'd'
mov byte [es:0xa7], 0x06
mov byte [es:0xa8], 'e'
mov byte [es:0xa9], 0x06
mov byte [es:0xaa], 'r'
mov byte [es:0xab], 0x06

jmp $

