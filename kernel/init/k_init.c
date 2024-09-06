#include <tty/tty.h>
#include <libc/stdio.h>
#include <libc/stdlib.h>
#include <kernel/sysio.h>

#include <kernel/interrupts.h>

// 创建kprintf函数
// KINFO KWARN KERROR 作为LOG等级参数
// kprintf(KINFO "awa");
LOG_MODULE("OS")

void init(void)
{
   // 初始化 中断处理 函数
   intr_init();

   // tty必须要在分页映射后才可使用，因为linker脚本把kernel下的文件全都分到高半核了，不在同一个段，没法使用
   tty_init((void*)0xb8000UL);   // 设置文本内容缓冲区
   tty_clear();
   tty_set_theme(VGA_COLOR_WHITE, VGA_COLOR_BLACK);

   // kernel initing...
   kprintf(KLOG "Kernel Initing...\n");
   kprintf(KLOG "AWA OS Kernel booting...\n");
   
   for (int i = 0; i < 2; i++)
   {
      kprintf("awa I Love U Creator!!! Times: %d\n", i+1);
   }
   
   // int a = 1/0;   // 启用了 -Werror 这种情况无法通过编译器，所以无法编译 要触发这种错误就只能另寻他法了

   // 触发 1 / 0 异常   这样可以
   __asm__("int $0");
}
