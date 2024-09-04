#include <tty/tty.h>
#include <libc/string.h>

#define TTY_WIDTH 80
#define TTY_HEIGHT 25

static uint32_t tty_x = 0;
static uint16_t tty_y = 0;

//用uint8_t*会出现问题，应为uint8_t*每次只能访问一个字节，需要访问两次才能正确设置
static uint16_t* tty_vga_buffer = (uint16_t*)0xB8000;
static uint16_t tty_theme_color = VGA_COLOR_BLACK;

void tty_init(void* vga_buffer)
{
    tty_vga_buffer = (uint16_t*)vga_buffer;
}

void tty_set_theme(uint16_t fg, uint16_t bg)
{
    tty_theme_color = (bg << 4 | fg) << 8;
}

void tty_put_char(const char chr)
{
    switch (chr)
    {
    case '\t':
        tty_x += 4;
        break;
    case '\n':
        // tty_x = 0;
        tty_y++;
    case '\r':
        tty_x = 0;
        break; 
    default:
        *(tty_vga_buffer + tty_x + tty_y * TTY_WIDTH) = (tty_theme_color | chr);
        tty_x++;
        break;
    }

    // 超过最大宽度，自动换行
    if (tty_x > TTY_WIDTH)
    {
        tty_x = 0;
        tty_y++;
    }
    //超过最大高度，自动滚屏
    if (tty_y > TTY_HEIGHT-1)
    {
        tty_scroll_up();
    }
    
    
}

void tty_put_str(const char* str)
{ 
    while(*str != '\0')
    {
        tty_put_char(*str);
        str++;
    }
}

void tty_scroll_up(void)
{
    size_t no_last_line = TTY_WIDTH * (TTY_HEIGHT - 1);
    
    // no_last_line * 2是因为每个字符占两个字节，tty需要2个字节组成一个颜色字符 (颜色 | 字符)
    // 将文本整体向上移动一行？
    memcpy(tty_vga_buffer, tty_vga_buffer + TTY_WIDTH, no_last_line * 2);

    for (size_t i = 0; i < TTY_WIDTH; i++)
    {
        // 清除最后一行所有信息
        *(tty_vga_buffer + no_last_line + i) = tty_theme_color;
    }
    tty_y = tty_y == 0 ? 0 : TTY_HEIGHT - 1; 

}

void tty_clear(void)
{
    tty_theme_color = VGA_COLOR_BLACK;
    for (uint32_t i = 0; i < TTY_HEIGHT * TTY_WIDTH; i++)
    {
        *(tty_vga_buffer + i) = tty_theme_color;
        tty_x = 0;
        tty_y = 0;
    } 
}

// get information about tty
uint16_t tty_get_theme()
{
    return tty_theme_color;
}

