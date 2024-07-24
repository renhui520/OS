extern	DISPLAY	;外部函数
[BITS 16]
[section .text]
global	_start
global	myprint
global 	putchar
global print_string

_start:
	call DISPLAY	;asm call c function


; 定义一个函数，用来输出字符串
print_string:
    pusha           ; 保存寄存器
    mov si, [esp+8] ; 从ESP+8处取字符串地址
print_loop:
    lodsb           ; 加载一个字节到AL寄存器，并递增SI
    cmp al, 0       ; 检查是否到达字符串的结尾
    je print_end    ; 如果是，则退出循环
    mov ah, 0x0E    ; 设置AH寄存器为0x0E，准备调用BIOS中断10h
    int 0x10        ; 调用BIOS中断10h
    jmp print_loop  ; 继续下一个字符
print_end:
    popa            ; 恢复寄存器
    ret             ; 返回


putchar:
    ; 从栈顶获取要打印的字符
    mov al, [esp + 4]
    mov ah, 0x0E  ; BIOS中断10h的功能码0x0E用于输出字符
    int 0x10       ; 调用BIOS中断
    ret

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
	
	;系统调用实现打印
	;mov ah, 0x0E ; 设置功能号为0x0E（输出字符到屏幕）
	;mov al, 'A' ; 直接将字符'A'加载到AL寄存器

	;; 调用BIOS中断10h
	;int 0x10
	;ret
