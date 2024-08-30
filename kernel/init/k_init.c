#include <tty/tty.h>

void init(void);

void init(void)
{
   // tty必须要在分页映射后才可使用，因为linker脚本把kernel下的文件全都分到高半核了，不在同一个段，没法使用
   tty_init((void*)0xb8000UL);
   tty_set_theme(VGA_COLOR_GREEN, VGA_COLOR_BLACK);
   tty_put_char('a');
   tty_put_str("Kernel Initing...");

}
