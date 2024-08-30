#define ASM_FILE        1
#include <boot/multiboot.h>

#define MULTIBOOT_HEADER_FLAGS                  MULTIBOOT_MEMORY_INFO | MULTIBOOT_PAGE_ALIGN
#define KPG_SIZE 24*1024 // 24KiB


/* Multiboot header. */
/* 仅需MAGIC FLAGS CHECKSUM即可识别 */
.section .multiboot
        .long   MULTIBOOT_HEADER_MAGIC/* magic */
        .long   MULTIBOOT_HEADER_FLAGS/* flags */
        .long   CHECKSUM(MULTIBOOT_HEADER_FLAGS)/* checksum */

.section .bss
        .global mb_info
        .align 16

        mb_info:
                .skip 4096
        stack_bottom:
                .skip 16318 * 2, 0      // 16*1024*2预留32K
        stack_top:


.section .kpg
        .global _k_ptd // 内核 页目录
        _k_ptd:
                .skip KPG_SIZE, 0 //预留内核页大小

.section .hhk_init
        .global start_
        .type start_, @function

        start_:
                cld
                cli

                /* Initialize the stack pointer. */
                movl    $stack_top, %esp
                subl    $16, %esp
                
                /* 传参 mb_info地址 */
                movl    $mb_info, 4(%esp)
                /* Push the pointer to the Multiboot information structure. */
                movl    %ebx, (%esp)
                call    _save_multiboot_info

                /* Push the magic value. */
                //pushl   %eax
                
                movl    $(KPG_SIZE), 4(%esp)
                movl    $(_k_ptd - 0xC0000000), (%esp) // 将页目录地址 传给hhk_init
                call    _hhk_init
                
                movl    (%esp), %eax
                andl    0xfffff000, %eax
                movl    %eax, %cr3

                movl    %cr0, %eax
                orl     0x80000000, %eax        // 将 CR0.PG 设置为1
                movl    %eax, %cr0              // 开启分页
                //因为使用的是multiboot+grub所以无须设置PE位

                addl 	$16, %esp

                pushl   $hhk_entry_
                ret
