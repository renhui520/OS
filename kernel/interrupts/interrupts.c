#include <kernel/interrupts.h>
#include <tty/tty.h>
#include <kernel/sysio.h>

void intr_routine_divide_zero(const isr_param* param);

static int_subscriber subscribers[256];

static int_subscriber fallback = (int_subscriber) 0;

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
    fallback = subscribers;
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

void intr_init()
{
    intr_subscribe(0, intr_routine_divide_zero);
}
// static void 
// __print_panic_msg(const char* msg, const isr_param* param) 
// {
//     kprint_panic("  INT %u: (%x) [%p: %p] %s",
//             param->vector,
//             param->err_code,
//             param->cs,
//             param->eip,
//             msg);
// }

void intr_routine_divide_zero(const isr_param* param)
{
    kprintf_panic("     [%s] Divide by zero\n", "ERROR!");
    while(1);   //不写这个会不断重启!!!
}
void intr_routine_init()
{

}