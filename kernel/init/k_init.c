#include <tty/tty.h>
#include <libc/stdio.h>
#include <libc/stdlib.h>

void init(void)
{
   // tty必须要在分页映射后才可使用，因为linker脚本把kernel下的文件全都分到高半核了，不在同一个段，没法使用
   tty_init((void*)0xb8000UL);
   tty_clear();
   tty_set_theme(VGA_COLOR_GREEN, VGA_COLOR_BLACK);
   tty_put_str("Kernel Initing...\n");
   tty_put_str("AWA OS Kernel booting...\n");
   
   uint16_t a = tty_get_theme();
   char buf[500];

   // itoa(1234567891, buf, 10);
   snprintf(buf, 500, "Num: %d", 100);

   tty_put_str(buf);

   // tty_put_str("a\n");
   // tty_put_str("a\n");
   // tty_put_str("a\n");
   // tty_put_str("a\n");
   // tty_put_str("a\n");
   // tty_put_str("a\n");
   // tty_put_str("a\n");
   // tty_put_str("a\n");
   // tty_put_str("a\n");
   // tty_put_str("a\n");
   // tty_put_str("a\n");
   // tty_put_str("a\n");
   // tty_put_str("a\n");
   // tty_put_str("a\n");
   // tty_put_str("a\n");
   // tty_put_str("a\n");
   // tty_put_str("a\n");
   // tty_put_str("a\n");
   // tty_put_str("a\n");
   // tty_put_str("a\n");
   // tty_put_str("a\n");
   // tty_put_str("a\n");
   // tty_put_str("a\n");
   // tty_put_str("a\n");
   // tty_put_str("a\n");
   // tty_put_str("a\n");
   // tty_put_str("a\n");
   // tty_put_str("a\n");
   // tty_put_str("a\n");
   // tty_put_str("a\n");
   // tty_put_str("a\n");
   // tty_put_str("a\n");
   // tty_put_str("a\n");
   // tty_put_str("a\n");
   // tty_put_str("a\n");
   // tty_put_str("a\n");
   // tty_put_str("b\n");
   // tty_put_str("c\n");
   // tty_put_str("d\n");
}
