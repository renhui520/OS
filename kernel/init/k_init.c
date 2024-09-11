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
      kprintf("VBE Control Info: %d\n", _k_init_mb_info->vbe_control_info);
      kprintf("VBE Mode Info: %d\n", _k_init_mb_info->vbe_mode_info);
      kprintf("VBE Mode: %d\n", _k_init_mb_info->vbe_mode);
      kprintf("VBE Interface seg: %d\n", _k_init_mb_info->vbe_interface_seg);
      kprintf("VBE Interface off: %d\n", _k_init_mb_info->vbe_interface_off);
      kprintf("VBE Interface len: %d\n", _k_init_mb_info->vbe_interface_len);
   } else {
      kprintf("VBE Mode is not enabled.\n");
   }
   if (_k_init_mb_info->flags & MULTIBOOT_INFO_FRAMEBUFFER_INFO)
   {
      kprintf("Frame Buffer_addr: %d\n", _k_init_mb_info->framebuffer_addr);
      kprintf("Frame Buffer width: %d\n", _k_init_mb_info->framebuffer_width);
      kprintf("Frame Buffer height: %d\n", _k_init_mb_info->framebuffer_height);
      if (_k_init_mb_info->framebuffer_addr == 0xb8000UL)
      {
         kprintf(KWARN "VBE is not enabled.");
      }
      
   }
   


   // for (int i = 0; i < 2; i++)
   // {
   //    kprintf("Hello Wolrd!!!Hello Everyone! Times: %d\n", i+1);
   // }
   
   // int a = 1/0;   // 启用了 -Werror 这种情况无法通过编译器，所以无法编译 要触发这种错误就只能另寻他法了

   // 触发 1 / 0 异常   这样可以
   // __asm__("int $0");
}




// #define __VBE__

#ifdef __VBE__

   /*

   RGB
   0x00ff0000 RED
   0x0000ff00 GREEN
   0x000000ff BLUE

   颜色格式(2种)
   0xRRGGBB
   0x00RRGGBB  00位用来凑数，刚好32位

   RR = RED
   GG = GREEN
   BB = BLUE

   */


#define VBE_COLOR_REG      0x00ff0000
#define VBE_COLOR_GREEN    0x0000ff00
#define VBE_COLOR_BLUE     0x000000ff

#define VBE_COLOR_BLACK    0x00000000
#define VBE_COLOR_WHITE    0x00ffffff

#define VBE_COLOR_YELLOW   0x00ffff00
#define VBE_COLOR_MAGENTA  0x00ff00ff
#define VBE_COLOR_CYAN     0x0000ffff

// 图形模式 的处理？ 搞不懂，放一旁先
// 好像还没映射内存，所以无法访问 X(
struct framebuffer
{
   void *base;
   uint32_t pitch;
   uint32_t width;
   uint32_t height;
   uint8_t bpp;
   uint8_t type;
} __attribute__((aligned(4)));

// 绘制像素
void draw_pixel(uint32_t x, uint32_t y, uint32_t color, struct framebuffer *fb)
{
   uint32_t offset = y * fb->pitch + x * (fb->bpp / 8);
   *(uint32_t *)((char *)fb->base + offset) = color;
}

void vbe_init(struct framebuffer *fb)
{
   // 获取帧缓冲区信息
   fb->base = (void *)(uintptr_t)_k_init_mb_info->framebuffer_addr;
   fb->pitch = _k_init_mb_info->framebuffer_pitch;
   fb->width = _k_init_mb_info->framebuffer_width;
   fb->height = _k_init_mb_info->framebuffer_height;
   fb->bpp = _k_init_mb_info->framebuffer_bpp;
   fb->type = _k_init_mb_info->framebuffer_type;

}
// 主函数
void vbe_main(void)
{
   struct framebuffer fb;
   vbe_init(&fb);

   // 绘制像素
   for (size_t i = 0; i < 100; i++)
   {
      /* code */
      draw_pixel(100 + i, 100, VBE_COLOR_BLUE, &fb); // 绘制一个蓝色像素
      draw_pixel(100 + i, 101, VBE_COLOR_BLUE, &fb); // 绘制一个蓝色像素
      draw_pixel(100 + i, 102, VBE_COLOR_BLUE, &fb); // 绘制一个蓝色像素
      draw_pixel(100 + i, 103, VBE_COLOR_BLUE, &fb); // 绘制一个蓝色像素
      draw_pixel(100 + i, 104, VBE_COLOR_BLUE, &fb); // 绘制一个蓝色像素
      draw_pixel(100 + i, 105, VBE_COLOR_BLUE, &fb); // 绘制一个蓝色像素
   }

   for (size_t i = 0; i < 100; i++)
   {
      /* code */
      draw_pixel(150 + i, 150, VBE_COLOR_YELLOW, &fb); // 绘制一个蓝色像素
      draw_pixel(150 + i, 151, VBE_COLOR_YELLOW, &fb); // 绘制一个蓝色像素
      draw_pixel(150 + i, 152, VBE_COLOR_YELLOW, &fb); // 绘制一个蓝色像素
      draw_pixel(150 + i, 153, VBE_COLOR_YELLOW, &fb); // 绘制一个蓝色像素
      draw_pixel(150 + i, 154, VBE_COLOR_YELLOW, &fb); // 绘制一个蓝色像素
      draw_pixel(150 + i, 155, VBE_COLOR_YELLOW, &fb); // 绘制一个蓝色像素
   }

   for (size_t i = 0; i < 100; i++)
   {
      /* code */
      draw_pixel(200 + i, 200, VBE_COLOR_REG, &fb); // 绘制一个红色像素
      draw_pixel(200 + i, 201, VBE_COLOR_REG, &fb); // 绘制一个红色像素
      draw_pixel(200 + i, 202, VBE_COLOR_REG, &fb); // 绘制一个红色像素
      draw_pixel(200 + i, 203, VBE_COLOR_REG, &fb); // 绘制一个红色像素
      draw_pixel(200 + i, 204, VBE_COLOR_REG, &fb); // 绘制一个红色像素
      draw_pixel(200 + i, 205, VBE_COLOR_REG, &fb); // 绘制一个红色像素
   }

   for (size_t i = 0; i < 100; i++)
   {
      /* code */
      draw_pixel(250 + i, 250, VBE_COLOR_MAGENTA, &fb); // 绘制一个红色像素
      draw_pixel(250 + i, 251, VBE_COLOR_MAGENTA, &fb); // 绘制一个红色像素
      draw_pixel(250 + i, 252, VBE_COLOR_MAGENTA, &fb); // 绘制一个红色像素
      draw_pixel(250 + i, 253, VBE_COLOR_MAGENTA, &fb); // 绘制一个红色像素
      draw_pixel(250 + i, 254, VBE_COLOR_MAGENTA, &fb); // 绘制一个红色像素
      draw_pixel(250 + i, 255, VBE_COLOR_MAGENTA, &fb); // 绘制一个红色像素
   }

   for (size_t i = 0; i < 100; i++)
   {
      draw_pixel(300 + i, 300, VBE_COLOR_GREEN, &fb); // 绘制一个绿色像素
      draw_pixel(300 + i, 301, VBE_COLOR_GREEN, &fb); // 绘制一个绿色像素
      draw_pixel(300 + i, 302, VBE_COLOR_GREEN, &fb); // 绘制一个绿色像素
      draw_pixel(300 + i, 303, VBE_COLOR_GREEN, &fb); // 绘制一个绿色像素
      draw_pixel(300 + i, 304, VBE_COLOR_GREEN, &fb); // 绘制一个绿色像素
      draw_pixel(300 + i, 305, VBE_COLOR_GREEN, &fb); // 绘制一个绿色像素
   }

   for (size_t i = 0; i < 100; i++)
   {
      draw_pixel(350 + i, 350, VBE_COLOR_WHITE, &fb); // 绘制一个绿色像素
      draw_pixel(350 + i, 351, VBE_COLOR_WHITE, &fb); // 绘制一个绿色像素
      draw_pixel(350 + i, 352, VBE_COLOR_WHITE, &fb); // 绘制一个绿色像素
      draw_pixel(350 + i, 353, VBE_COLOR_WHITE, &fb); // 绘制一个绿色像素
      draw_pixel(350 + i, 354, VBE_COLOR_WHITE, &fb); // 绘制一个绿色像素
      draw_pixel(350 + i, 355, VBE_COLOR_WHITE, &fb); // 绘制一个绿色像素
                                                 /* code */
   }

   while (1)
   {
   }
}
#endif
