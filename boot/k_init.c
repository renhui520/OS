#include <boot/multiboot.h>
#include <tty/tty.h>

void init(void);

void init(void)
{
    tty_set_theme(VGA_COLOR_GREEN, VGA_COLOR_BLACK);
    tty_put_str("Kernel Initing...");

}
