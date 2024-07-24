LOADER_BASE_ADDR    equ 0x900
SECTION loader vstart=LOADER_BASE_ADDR  ;将代码段放在0x900位置

;KERNEL_BASE_ADDR 从loader jmp到的物理位置，同时也是kernel.bin从硬盘中加载的物理位置
KERNEL_BASE_ADDR equ 0x1500

;从第九号扇区开始写入kernel.bin
KERNEL_BASE_SECTOR equ 0x9 ;kernel.bin在第9号扇区开始



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



