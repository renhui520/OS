#ifndef __OS_INTERRUPTS_H__
#define __OS_INTERRUPTS_H__


#define FAULT_DIVISION_ERROR            0
#define FAULT_TRAP_DEBUG_EXCEPTION      1
#define INT_NMI                         2
#define TRAP_BREAKPOINT                 3
#define TRAP_OVERFLOW                   4
#define FAULT_BOUND_EXCEED              5
#define FAULT_INVALID_OPCODE            6
#define FAULT_NO_MATH_PROCESSOR         7
#define ABORT_DOUBLE_FAULT              8
#define FAULT_RESERVED_0                9
#define FAULT_INVALID_TSS               10
#define FAULT_SEG_NOT_PRESENT           11
#define FAULT_STACK_SEG_FAULT           12
#define FAULT_GENERAL_PROTECTION        13
#define FAULT_PAGE_FAULT                14
#define FAULT_RESERVED_1                15
#define FAULT_X87_FAULT                 16
#define FAULT_ALIGNMENT_CHECK           17
#define ABORT_MACHINE_CHECK             18
#define FAULT_SIMD_FP_EXCEPTION         19
#define FAULT_VIRTUALIZATION_EXCEPTION  20
#define FAULT_CONTROL_PROTECTION        21

// AWAOS related
#define AWA_SYS_PANIC                32

#define EX_INTERRUPT_BEGIN              200
// APIC related
#define APIC_ERROR_IV                   200
#define APIC_LINT0_IV                   201
#define APIC_TIMER_IV                   202
#define APIC_SPIV_IV                    203

#define RTC_TIMER_IV                    210


#define PC_AT_IRQ_RTC                   8
#define PC_AT_IRQ_KBD_BUF_FULL          1


#ifndef __ASM__

#include <hal/cpu.h>

typedef struct {
    gp_regs registers;
    unsigned int vector;
    unsigned int err_code;
    unsigned int eip;
    unsigned int cs;
    unsigned int eflags;
    unsigned int esp;
    unsigned int ss;
} __attribute__((packed)) isr_param;

typedef void (*int_subscriber)(isr_param*);


// region和endregion是一对用于标记它们之间是代码段，方便vscode折叠代码
#pragma region ISR_DECLARATION

void _asm_isr_0();
void _asm_isr_1();
void _asm_isr_2();
void _asm_isr_3();
void _asm_isr_4();
void _asm_isr_5();
void _asm_isr_6();
void _asm_isr_7();
void _asm_isr_8();
void _asm_isr_9();
void _asm_isr_10();
void _asm_isr_11();
void _asm_isr_12();
void _asm_isr_14();
void _asm_isr_15();
void _asm_isr_16();
void _asm_isr_17();
void _asm_isr_18();
void _asm_isr_19();
void _asm_isr_20();
void _asm_isr_21();

void _asm_isr_32();
void _asm_isr_200();
void _asm_isr_201();
void _asm_isr_202();
void _asm_isr_203();

void _asm_isr_210();

#pragma endregion

void intr_subscribe(const uint8_t vector, int_subscriber);

void intr_unsubscribe(const uint8_t vector, int_subscriber);

void intr_set_fallback_handler(int_subscriber);

void intr_handler(isr_param* param);
void intr_routine_init();

void intr_other_init();

#endif


#endif