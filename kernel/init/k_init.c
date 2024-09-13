#include <tty/tty.h>
#include <libc/stdio.h>
#include <libc/stdlib.h>
#include <kernel/sysio.h>

#include <kernel/interrupts.h>


// memcpy memset...
#include <libc/string.h>

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

#define VBE_WIDTH 1024
#define VBE_HEIGHT 768
#define VBE_BPP 32   //??

#define VBE_COLOR_RED      0x00ff0000
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

struct framebuffer fb;

// 绘制像素
void draw_pixel(uint16_t x, uint16_t y, uint32_t color)
{
   uint32_t offset = y * fb.pitch + (x * (fb.bpp / 8));
   *(uint32_t *)((char *)fb.base + offset) = color;
}

// 绘画一行，但是memset只能处理uint8_t的类型，uint32_t类型数据只能写入开头的0，很麻烦，得重新写一个专门的程序
// void draw_line(uint32_t color, uint32_t line_y, struct framebuffer *fb)
// {
//    uint32_t offset = line_y * fb->pitch;
//    memset((void*)fb->base + offset, color, fb->width * (fb->bpp / 8));
// }

void draw_line(uint16_t x, uint16_t y, uint16_t width, uint32_t color)
{
   uint32_t offset = y * fb.pitch + (x * (fb.bpp / 8));
   for (size_t i = 0; i < width; i++)
   {
      *(uint32_t *)((char*)fb.base + offset + i * (fb.bpp / 8)) = color;
   }
}

// Have to fix this
// BUG!!!!

// void draw_line(uint32_t x, uint32_t y, uint32_t color, uint32_t width)
// {
//    uint32_t offset = y * fb.pitch + (x * (fb.bpp / 8));
//    asm volatile (
//       "xor %%eax, %%eax\n\t"
//       "movl %1, %%eax\n\t"
//       "xor %%ecx, %%ecx\n\t"
//       "movl %2, %%ecx\n\t"
//       "xor %%edi, %%edi\n\t"
//       "leal (%3), %%edi\n\t"
//       "rep stosd\n\t"
//       :
//       : "r"(color), "r"(width), "r"((uint32_t)fb.base + offset)
//       : "eax", "ecx", "edi"
//    );
// }


void draw_box(uint16_t x, uint16_t y, uint16_t width, uint16_t height, uint32_t color)
{
   for (uint16_t i = 0; i < height; i++)
   {
      draw_line(x, y + i, width, color);
   }
}

void draw_framebox(uint16_t x, uint16_t y, uint16_t width, uint16_t height, uint16_t border_width, uint32_t color)
{
   for (uint16_t i = 0; i < border_width; i++)
   {
      // 顶部
      draw_line(x, y + i, width, color);
      // 底部
      draw_line(x, y + height - 1 - border_width + i, width, color);
   }
   for (uint16_t i = 0; i < height - 2 * border_width; i++)
   {
      for (uint16_t j = 0; j < border_width; j++)
      {
         // 左侧
         draw_pixel(x + j, y + border_width + i, color);
         // 右侧
         draw_pixel((x + width - border_width) + j, y + border_width + i, color);
      }
   }
   
   
}

void vbe_init()
{
   // 获取帧缓冲区信息
   fb.base = (void *)(uintptr_t)_k_init_mb_info->framebuffer_addr;
   fb.pitch = _k_init_mb_info->framebuffer_pitch;
   fb.width = _k_init_mb_info->framebuffer_width;
   fb.height = _k_init_mb_info->framebuffer_height;
   fb.bpp = _k_init_mb_info->framebuffer_bpp;
   fb.type = _k_init_mb_info->framebuffer_type;

}
// 主函数
void vbe_main(void)
{
   vbe_init();

   draw_box(50, 50, 100, 100, VBE_COLOR_RED);
   draw_box(160, 50, 100, 100, VBE_COLOR_BLUE);
   draw_box(160, 160, 100, 100, VBE_COLOR_GREEN);
   draw_box(50, 160, 100, 100, VBE_COLOR_YELLOW);

   draw_box(550, 50, 100, 100, VBE_COLOR_RED);
   draw_box(660, 50, 100, 100, VBE_COLOR_BLUE);
   draw_box(660, 160, 100, 100, VBE_COLOR_GREEN);
   draw_box(550, 160, 100, 100, VBE_COLOR_YELLOW);

   draw_line(140, 300, 400, VBE_COLOR_CYAN);

   // 画一个 矩形边框
   draw_framebox(5, 5, VBE_WIDTH - 10, VBE_HEIGHT - 10, 5, VBE_COLOR_GREEN);
   while (1)
   {
   }
}
#endif
