LOADER_BASE_ADDR    equ 0x900
SECTION loader vstart=LOADER_BASE_ADDR  ;将代码段放在0x900位置

;KERNEL_BASE_ADDR 从loader jmp到的物理位置，同时也是kernel.bin从硬盘中加载的物理位置
KERNEL_BASE_ADDR equ 0x1500

;从第九号扇区开始写入kernel.bin
KERNEL_BASE_SECTOR equ 0x9 ;kernel.bin在第9号扇区开始


;进入保护模式
jmp protect_mode
;-----------------------gdt表-------------------------
LOADER_STACK_TOP equ 0x90000	;设置栈顶	因为栈是从高地址往低地址增长

;------------------

;创建页表并初始化（页目录和页表）
PAGE_DIR_TABLE_POS equ 0x100000		;页表，分页模式用

;------------------

gdt:
;0描述符
	dd	0x00000000
	dd	0x00000000
;1描述符(4GB代码段描述符)
	dd	0x0000ffff
	dd	0x00cf9800
;2描述符(4GB数据段描述符)
	dd	0x0000ffff
	dd	0x00cf9200
;3描述符(28Kb的视频段描述符)
	dd	0x80000007
	dd	0x00c0920b

lgdt_value:
	dw $-gdt-1	;高16位表示表的最后一个字节的偏移（表的大小-1） 
	dd gdt		;低32位表示起始位置（GDT的物理地址）

SELECTOR_CODE	equ	0x0001<<3
SELECTOR_DATA	equ	0x0002<<3
SELECTOR_VIDEO	equ	0x0003<<3

protect_mode:
;进入32位
	lgdt [lgdt_value]
	in al,0x92
	or al,0000_0010b
	out 0x92,al
	cli
	mov eax,cr0
	or eax,1
	mov cr0,eax
	
	jmp dword SELECTOR_CODE:main
	
[bits 32]
;正式进入32位
main:
mov ax,SELECTOR_DATA
mov ds,ax
mov es,ax
mov ss,ax
mov esp,LOADER_STACK_TOP
mov ax,SELECTOR_VIDEO
mov gs,ax

mov byte [gs:0xa0],'3'
mov byte [gs:0xa2],'2'
mov byte [gs:0xa4],'m'
mov byte [gs:0xa6],'o'
mov byte [gs:0xa8],'d'


;--------分页模式--------

call setup_page

;重新加载 gdt，因为已经变成了虚拟地址方式
sgdt [lgdt_value]
mov ebx,[lgdt_value+2]
or dword [ebx+0x18+4],0xc0000000
add dword [lgdt_value+2],0xc0000000
add esp,0xc0000000

;页目录表起始地址存入 cr3 寄存器
mov eax,PAGE_DIR_TABLE_POS
mov cr3,eax

;开启分页
mov eax,cr0
or eax,0x80000000
mov cr0,eax

;重新加载 gdt
lgdt [lgdt_value]

mov byte [gs:0x1e0],'p'
mov byte [gs:0x1e2],'a'
mov byte [gs:0x1e4],'g'
mov byte [gs:0x1e6],'e'
mov byte [gs:0x1ea],'o'
mov byte [gs:0x1ec],'n'

;------------------------


jmp $

;-------------开启分页模式函数--------------

setup_page:
;先把页目录占用的空间逐字清零
	mov ecx,4096
	mov esi,0
.clear_page_dir:
	mov byte [PAGE_DIR_TABLE_POS+esi],0
	inc esi
	loop .clear_page_dir
	
;开始创建页目录项（PDE）
.create_pde:
	mov eax,PAGE_DIR_TABLE_POS
	add eax,0x1000; 此时eax为第一个页表的位置及属性
	mov ebx,eax
	or eax,111b
	mov [PAGE_DIR_TABLE_POS],eax
	mov [PAGE_DIR_TABLE_POS+0xc00],eax
	sub eax,0x1000
	mov [PAGE_DIR_TABLE_POS+4*1023],eax

;开始创建页表项（PTE）
	mov ecx,256
	mov esi,0
	mov edx,111b
.create_pte:
	mov [ebx+esi*4],edx
	add edx,4096
	inc esi
	loop .create_pte
	
;创建内核其他页表的页目录项（PDE）
	mov eax,PAGE_DIR_TABLE_POS
	add eax,0x2000
	or eax,111b
	mov ebx,PAGE_DIR_TABLE_POS
	mov ecx,254
	mov esi,769
.create_kernel_pde:
	mov [ebx+esi*4],eax
	inc esi
	add eax,0x1000
	loop .create_kernel_pde
	ret

;-------------------------------------------

;									^
;-------------------------------------------------------进入32位保护模式|

mov ax, 0xB800
mov es, ax

;mov byte [gs:0x00], 'h' ;不写前面的内容，这样也行

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

mov eax,KERNEL_BASE_SECTOR;将kernel写在第9号扇区	;起始扇区lba地址，LBA=(柱面号*磁头数+磁头号)*扇区数+扇区编号-1
mov bx,KERNEL_BASE_ADDR    ;写入的内存地址，之后用
mov cx,1        ;待读入的扇区数		在第二扇区开始往后面读，读多少取决于待读入扇区数，2+n

call read_disk
jmp KERNEL_BASE_ADDR

;读硬盘
read_disk:
	mov esi,eax	;备份
	mov di,cx	;备份
	
;第一步，设置要读取的扇区数
	mov dx,0x1f2
	mov al,cl
	out dx,al
	mov eax,esi	;恢复
	
;第二步，设置LBA地址
	mov cl,8
	;0-7位写入0x1f3
	mov dx,0x1f3
	out dx,al	;待读入扇区号
	;8-15位写入0x1f4
	mov dx,0x1f4
	shr eax,cl
	out dx,al
	;16-23位写入0x1f5
	mov dx,0x1f5
	shr eax,cl
	out dx,al
	;24-27位写入0x1f6
	mov dx,0x1f6
	shr eax,cl
	and al,0x0f	;lba的24-27位
	or al,0xe0	;另外4位为1110，表示lba模式
	out dx,al
	
;第三步，写入读命令
	mov dx,0x1f7
	mov al,0x20	;读指令
	out dx,al

;第四步，检测硬盘状态
.not_ready:
	nop
	in al,dx
	and al,0x88	;第4位为1表示准备好，第7位为1表示忙
	cmp al,0x08
	jnz .not_ready
	
;第五步，读数据
	mov ax,di
	mov dx,256
	mul dx
	mov cx,ax
	
	mov dx,0x1f0	;读数据
	.go_on_read:
		in ax,dx
		mov [bx],ax
		add bx,2
		loop .go_on_read
		ret



