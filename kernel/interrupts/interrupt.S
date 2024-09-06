#define __ASM__
#include <kernel/interrupts.h>

.macro isr_template vector, no_error_code=1	//定义一个宏，表示isr_template接受vector和no_error_code两个参数
	.global _asm_isr_\vector		 //定义一个全局函数
	.type _asm_isr_\vector, @function //同上,vector会被替换为相应的中断号，例如0，则为_asm_isr_0
	_asm_isr_\vector:
		.if \no_error_code	//if判断
			pushl $0x0
		.endif
		pushl $\vector
		jmp interrupt_wrapper	//跳转到中断处理程序
.endm

.section .text
    isr_template FAULT_DIVISION_ERROR
    isr_template FAULT_GENERAL_PROTECTION, no_error_code=0
    isr_template FAULT_PAGE_FAULT, no_error_code=0

    isr_template AWA_SYS_PANIC

    isr_template APIC_ERROR_IV
    isr_template APIC_LINT0_IV
    isr_template APIC_TIMER_IV
    isr_template APIC_SPIV_IV
    isr_template RTC_TIMER_IV
    
	interrupt_wrapper:	//处理中断的函数
        pushl %esp
        pushl %esi
        pushl %ebp
        pushl %edi
        pushl %edx
        pushl %ecx
        pushl %ebx
        pushl %eax

        movl %esp, %eax
        andl $0xfffffff0, %esp
        subl $16, %esp
        movl %eax, (%esp)

        call intr_handler
        popl %esp

        popl %eax
        popl %ebx
        popl %ecx
        popl %edx
        popl %edi
        popl %ebp
        popl %esi
        popl %esp

        addl $8, %esp

        iret