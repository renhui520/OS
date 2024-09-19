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


extern uint8_t __kernel_start;
extern uint8_t __kernel_end;
extern uint8_t __init_hhk_end;

#define VGA_BUFFER_VADDR    0xB0000000UL
#define VGA_BUFFER_PADDR    0xB8000UL
#define VGA_BUFFER_SIZE     4096


/*
   
   TODO: 
         2. 取消之前的1MiB空间映射

*/


// multiboot用于与grub交互
multiboot_info_t* _k_init_mb_info;


// 创建kprintf函数
// KINFO KWARN KERROR 作为LOG等级参数
// kprintf(KINFO "awa");
LOG_MODULE("OS")

// 内存管理 映射 初始化？
void setup_memory(multiboot_memory_map_t *map, size_t map_size);

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

   // 所有页 [已占用]
   pmm_init(MEM_1MB + (_k_init_mb_info->mem_upper << 10));
   kprintf(KINIT "pmm initialized.\n");

   kprintf(KINIT "vmm(Virtual Memory Manager) initializing...\n");
   vmm_init();
   kprintf(KINIT "vmm initialized.\n");
   
   kprintf(KINIT "Memory Manager initialized.\n");

   /*--------------------------------------------*/

   kprintf(KINIT "Setting up memory...\n");

   // 计算 位图 大小
   unsigned int map_size = _k_init_mb_info->mmap_length / sizeof(multiboot_memory_map_t);

   // 按照 Memory map 标识可用的物理页
   setup_memory((multiboot_memory_map_t *)_k_init_mb_info->mmap_addr, map_size);

   kprintf(KINIT "Done ! \n");

   tty_put_str("================================================================================\n");

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
      kprintf("Frame Buffer_addr: 0x%x\n", _k_init_mb_info->framebuffer_addr);
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
void setup_memory(multiboot_memory_map_t *map, size_t map_size)
{

   // First pass, to mark the physical pages
   for (unsigned int i = 0; i < map_size; i++)
   {
      multiboot_memory_map_t mmap = map[i];
      kprintf("[MM] Base: 0x%x, len: %u KiB, type: %u\n",
              map[i].addr,      // 内存区域的起始地址
              map[i].len >> 10, // 字节 转化为 KiB
              map[i].type);         // 内存区域类型 也就是分页属性:
                            // 例如 MULTIBOOT_MEMORY_AVAILABLE

      // 标记 内存 为 可用
      if (mmap.type == MULTIBOOT_MEMORY_AVAILABLE)
      {
         // 整数向上取整除法
         uintptr_t pg = map[i].addr + 0x0fffU;

         // pg >> PG_SIZE_BITS 计算起始页数
         // map[i].len_low >> PG_SIZE_BITS 计算所占页数
         // 连续标记多个页为可用
         pmm_mark_chunk_free(pg >> PG_SIZE_BITS, map[i].len >> PG_SIZE_BITS);
         kprintf(KLOG "[MM] Freed %u pages start from 0x%x\n",
                 map[i].len >> PG_SIZE_BITS, // 计算页数，因为1页=4KiB=(1<<12)字节
                 pg & ~0x0fffU);                 // 进行掩码操作保证地址是对齐到页面大小 (4KiB) 的
      }
   }

   // 将内核占据的页，包括前1MB，hhk_init 设为已占用
   size_t pg_count = V2P(&__kernel_end) >> PG_SIZE_BITS;             // 虚拟内存 ==> 物理内存 并计算占用页数
   pmm_mark_chunk_occupied(0, pg_count);                             // 连续标记多个页 [已占用]
   kprintf(KLOG "[MM] Allocated %d pages for kernel.\n", pg_count); // 输出 [已占用] 页数

   size_t vga_buf_pgs = VGA_BUFFER_SIZE >> PG_SIZE_BITS; // 计算 VGA 缓冲区 页数

   // 首先，标记VGA部分为已占用
   // VGA_BUFFER_PADDR >> PG_SIZE_BITS 计算页数
   pmm_mark_chunk_occupied(VGA_BUFFER_PADDR >> PG_SIZE_BITS, vga_buf_pgs); // 连续标记多个 "VGA缓冲区" 页 [已占用]

   // 重映射VGA文本缓冲区（以后会变成显存，i.e., framebuffer）
   for (size_t i = 0; i < vga_buf_pgs; i++)
   {
      // i << PG_SIZE_BITS 实际上应该和 VGA_BUFFER_PADDR 一致
      // 因为 i = vga_buf_pgs, vga_buf_pgs = VGA_BUFFER_SIZE >> PG_SIZE_BITS
      // 所以 i = VGA_BUFFER_PADDR >> PG_SIZE_BITS
      // 所以 i << PG_SIZE_BITS = VGA_BUFFER_PADDR
      vmm_map_page(                                         // 将 虚拟地址 与 物理地址 建立映射关系
          (void *)(VGA_BUFFER_VADDR + (i << PG_SIZE_BITS)), // 虚拟地址    循环映射多个4KiB页表
          (void *)(VGA_BUFFER_PADDR + (i << PG_SIZE_BITS)), // 物理地址
          PG_PREM_RW                                        // 页属性 [可读写]
      );
   }


   // WARN!!!!!      运行到vmm_map_page这里会强制重启！gdb下似乎会导致l2pte访问失败？???

   // 更新VGA缓冲区位置至虚拟地址
   tty_init((void *)VGA_BUFFER_VADDR);
   // tty_init((void *)VGA_BUFFER_PADDR);    // 仍然能通过这个访问   但是后面会将这部分映射删除，所以今后可能访问不了

   kprintf(KLOG "[MM] Mapped VGA to %p.\n", VGA_BUFFER_VADDR);
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
