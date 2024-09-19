#define ASM_FILE        1


// #define __VBE__         // 开启图形模式


#include <boot/multiboot.h>

#define KPG_SIZE 24*1024 // 24KiB

#ifdef __VBE__
// MULTIBOOT_VIDEO_MODE                 开启视频模式设置
// MULTIBOOT_INFO_VBE_INFO              返回VBE信息
// MULTIBOOT_INFO_FRAMEBUFFER_INFO      返回视频缓冲区地址？
#define MULTIBOOT_HEADER_FLAGS          MULTIBOOT_MEMORY_INFO | \
                                        MULTIBOOT_PAGE_ALIGN | \
                                        MULTIBOOT_VIDEO_MODE
                                        //MULTIBOOT_INFO_VBE_INFO
                                        //MULTIBOOT_INFO_FRAMEBUFFER_INFO
#else   /* !__VBE__ */

#define MULTIBOOT_HEADER_FLAGS          MULTIBOOT_MEMORY_INFO | \
                                        MULTIBOOT_PAGE_ALIGN

#endif  /* __VBE__ */

/* Multiboot header. */
/* 仅需MAGIC FLAGS CHECKSUM即可识别 */
.section .multiboot
        .long   MULTIBOOT_HEADER_MAGIC/* magic */
        .long   MULTIBOOT_HEADER_FLAGS/* flags */
        .long   CHECKSUM(MULTIBOOT_HEADER_FLAGS)/* checksum */
#ifdef __VBE__
        .long   0
        .long   0
        .long   0
        .long   0
        .long   0
        .long   0       // Linear graphics please?
        .long   1024    // Preferred width
        .long   768     // Preferred height
        .long   32      // Preferred pixel depth(32-bit color)
#endif  /* __VBE__ */


.section .bss
        .global mb_info
        .align 16       // 对齐？
        mb_info:        // 用于存放multiboot
                .skip 4096      // 预留4K
        stack_bottom:   // 栈 ：为程序传参提供栈
                .skip 16318 * 2, 0      // 16*1024*2预留32K
        stack_top:


.section .kpg
        .global _k_ptd // 内核 页目录
        _k_ptd:
                .skip KPG_SIZE, 0 // 预留内核页大小   24K

.section .hhk_init              // 高半核初始化
        .global start_
        .type start_, @function

        start_:
                cld     // 禁用所有FLAGS
                cli     // 禁用中断

                /* 初始化栈指针 */
                movl    $stack_top, %esp
                subl    $16, %esp       // 目的是在栈上预留 16 字节的空间。这通常是为了存储一些临时变量或者参数
                
                /* 传参 mb_info地址 */
                movl    $mb_info, 4(%esp)
                /* 传入 multiboot结构体 指针 */
                movl    %ebx, (%esp)
                call    _save_multiboot_info // 保存multiboot结构体到mb_info

                /* Push the magic value. */
                //pushl   %eax
                
                //分页 初始化
                movl    $(KPG_SIZE), 4(%esp)
                movl    $(_k_ptd - 0xC0000000), (%esp) // 将页目录地址 传给hhk_init
                call    _hhk_init       //初始化 分页
                
                movl    (%esp), %eax
                andl    $0xfffff000, %eax       // 保留高 20位，清除低12位，确保只保留页目录的基地址部分
                movl    %eax, %cr3

                movl    %cr0, %eax
                orl     $0x80000000, %eax        // 将 CR0.PG 设置为1
                movl    %eax, %cr0              // 开启分页
                //因为使用的是multiboot+grub所以无须设置PE位

                addl 	$16, %esp       // 将栈指针（%esp）加上 16 字节。目的是释放之前预留的 16 字节空间

                pushl   $hhk_entry_     // 进入高半核
                ret

                /* 
                        push $function_addr
                        ret
                        等同于
                        call function
                 */