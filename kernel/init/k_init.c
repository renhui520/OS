#include <tty/tty.h>
#include <libc/stdio.h>
#include <libc/stdlib.h>
#include <kernel/sysio.h>

#include <kernel/interrupts.h>


#include <mm/pmm.h>
#include <mm/vmm.h>

#include <boot/multiboot.h>


#define MEM_1MB 0x100000


// multiboot用于与grub交互
multiboot_info_t* _k_init_mb_info;


// 创建kprintf函数
// KINFO KWARN KERROR 作为LOG等级参数
// kprintf(KINFO "awa");
LOG_MODULE("OS")

void init(void)
{
#pragma region TTY init
   // tty必须要在分页映射后才可使用，因为linker脚本把kernel下的文件全都分到高半核了，不在同一个段，没法使用
   tty_init((void*)0xb8000UL);   // 设置文本内容缓冲区
   tty_clear();
   tty_set_theme(VGA_COLOR_WHITE, VGA_COLOR_BLACK);
#pragma endregion

   // kernel initing...
   kprintf(KLOG "Kernel Initing...\n");
   
#pragma region Interrupts init
   // 初始化 中断处理 函数
   kprintf(KINIT "Initializing interrupts...\n");
   intr_init();
   kprintf(KINIT "Interrupts initialized.\n");
#pragma endregion

#pragma region Mermory Manager init
   // (_k_init_mb_info->mem_upper << 10)将 KiB 转化为 字节
   // 1KiB = 2^10字节 1MiB = 2^20字节
   // 所以 mem_upper << 10 ----> mem_upper * 2^10
   // 标记所有页为 [已占用]
   kprintf(KINIT "Memory Manager initializing...\n");

   kprintf(KINIT "pmm(Phyisic Memory Manager) initializing...\n");
   kprintf(KINIT "Marking all memory as used...\n");
   pmm_init(MEM_1MB + (_k_init_mb_info->mem_upper << 10));
   kprintf(KINIT "pmm initialized.\n");

   kprintf(KINIT "vmm(Virtual Memory Manager) initializing...\n");
   vmm_init();
   kprintf(KINIT "vmm initialized.\n");
   
   kprintf(KINIT "Memory Manager initialized.\n");
   
#pragma endregion   
   
   // kprintf("MEM_UPPER: %d  Byte\n", _k_init_mb_info->mem_upper << 10);
 
   // 检测是否开启VBE
   if (_k_init_mb_info->flags & MULTIBOOT_INFO_VBE_INFO)
   {
      // 好像VBE还没开启
      kprintf("VBE Address(from multiboot): %d\n", _k_init_mb_info->framebuffer_addr);
      kprintf("vbe_mode_info: %d\n", _k_init_mb_info->vbe_mode_info);
      kprintf("vbe_mode: %d\n", _k_init_mb_info->vbe_mode);
   } else {
      kprintf("VBE Mode is not enabled.\n");
   }


   for (int i = 0; i < 2; i++)
   {
      kprintf("Hello Wolrd!!!Hello Everyone! Times: %d\n", i+1);
   }
   
   // int a = 1/0;   // 启用了 -Werror 这种情况无法通过编译器，所以无法编译 要触发这种错误就只能另寻他法了

   // 触发 1 / 0 异常   这样可以
   // __asm__("int $0");
}
