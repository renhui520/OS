#include <tty/tty.h>

#define TTY_WIDTH 80
#define TTY_HEIGHT 25

static uint32_t tty_x = 0;
static uint16_t tty_y = 0;

static uint8_t* tty_vga_buffer = (uint8_t*)0xB8000;
static uint16_t tty_theme_color = VGA_COLOR_BLACK;

void tty_set_theme(uint16_t fg, uint16_t bg)
{
    tty_theme_color = (bg << 4 | fg) << 8;
}

void tty_put_char(char chr)
{
    *(tty_vga_buffer + tty_x + tty_y * TTY_WIDTH) = (tty_theme_color | chr);
    tty_x++;
}

void tty_put_str(char* str)
{
    while(*str != '\0')
    {
        tty_put_char(*str);
        str++;
    }
}
