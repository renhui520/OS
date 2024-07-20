
;----BIOS把启动区加载到内存的该位置，所以需设置地址偏移量
section mbr vstart=0x7c00

;----定义loader起始地址
LOADER_BASE_ADDR	equ	0x900
LOADER_BASE_SECTOR	equ	0x02	;所在LBA

;----设置堆栈地址
mov sp,0x7c00

;----卷屏中断，目的是清屏
mov ax,0x0600
mov bx,0x0700
mov cx,0
mov dx,0x184f
int 0x10

;----直接往显存中写数据
mov ax,0xb800
mov gs,ax
mov byte [gs:0x00],'m'
mov byte [gs:0x02],'b'
mov byte [gs:0x04],'r'

;----读取硬盘（第2扇区）并加载到内存（0x900)
mov eax,LOADER_BASE_SECTOR	;起始扇区lba地址，LBA=(柱面号*磁头数+磁头号)*扇区数+扇区编号-1
mov bx,LOADER_BASE_ADDR    ;写入的内存地址，之后用
mov cx,1        ;待读入的扇区数
call read_disk
jmp LOADER_BASE_ADDR

;----读硬盘方法，eax为lba扇区号，bx为待写入内存地址，cx为读入的扇区数
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
	out dx,al
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
	mov al,0x20
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
	
	mov dx,0x1f0
	.go_on_read:
		in ax,dx
		mov [bx],ax
		add bx,2
		loop .go_on_read
		ret
	
;----512字节的最后两字节是启动区标识
times 510-($-$$) db 0
db 0x55,0xaa
