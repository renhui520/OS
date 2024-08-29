#define ASM_FILE        1
#include <boot/multiboot.h>

#define MULTIBOOT_HEADER_FLAGS                  MULTIBOOT_MEMORY_INFO | MULTIBOOT_PAGE_ALIGN
#define KPG_SIZE 24*1024 // 24KiB


        /* Multiboot header. */
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
                
                movl    $mb_info, 4(%esp)
                movl    %ebx, (%esp)
                call    _save_multiboot_info

		//没搞懂 不知道为什么无法打印
		//call init
                /* Push the pointer to the Multiboot information structure. */
                //pushl   %ebx
                /* Push the magic value. */
                //pushl   %eax
                
                movl    $(KPG_SIZE), 4(%esp)
                movl    $(_k_ptd - 0xC0000000), (%esp)
                call    _hhk_init
                addl 	$16, %esp
