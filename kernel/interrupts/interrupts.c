#include <kernel/interrupts.h>
#include <tty/tty.h>
#include <kernel/sysio.h>
#include <hal/io.h>

// 具体中断处理
void intr_routine_divide_0(isr_param* param);
void intr_routine_divide_6(isr_param* param);
void intr_routine_divide_14(isr_param* param);



static int_subscriber subscribers[256];

static int_subscriber fallback = (int_subscriber) 0;


static void 
__print_panic_msg(const char* msg, const isr_param* param) 
{
    kprintf_panic("  INT %u: (%x) [%p: %p] %s",
            param->vector,
            param->err_code,
            param->cs,
            param->eip,
            msg);
}


void intr_subscribe(const uint8_t vector, int_subscriber subscriber)
{
    subscribers[vector] = subscriber;
}
void intr_unsubscribe(const uint8_t vector, int_subscriber subscriber)
{
    if (subscribers[vector] == subscriber)
    {
        subscribers[vector] = (int_subscriber)0;
    }
    
}
void intr_set_fallback_handler(int_subscriber subscriber)
{
    fallback = subscriber;
}
void intr_handler(isr_param* param)
{
    int_subscriber subscriber = subscribers[param->vector];
    if (subscriber)
    {
        subscriber(param);
        goto done;
    }
    if (fallback)
    {
        fallback(param);
        goto done;
    }
    

done:
    return;
}

// --禁用PIC--
#define PIC1_DATA    0x21
#define PIC2_DATA    0xA1

#define ICW1_DISABLE    0xFF // Disable PIC
//禁用PIC 解决中断重启问题
void disable_8259A()
{
    outb(PIC1_DATA, ICW1_DISABLE);
    outb(PIC2_DATA, ICW1_DISABLE);
}

void intr_routine_init()
{
    // TODO: 6 号中断处理
    intr_subscribe(0, intr_routine_divide_0);
    // intr_subscribe(6, intr_routine_divide_6);
    intr_subscribe(14, intr_routine_divide_14);
}

void intr_other_init()
{

}


void intr_routine_divide_0(isr_param* param)
{
    __print_panic_msg("Divide by zero\n", param);
    // kprintf_panic("     [%s] Divide by zero\n", "ERROR!");
    while(1);   //不写这个会不断重启!!!
}

void intr_routine_divide_14(isr_param* param)
{
    __print_panic_msg("NULL Pointer!!!!\n", param);
    // kprintf_panic("     [%s] Divide by zero\n", "ERROR!");
    while(1);   //不写这个会不断重启!!!
}
