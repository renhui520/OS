#define __LIBC__

#define MAX_BUFFER_SIZE 1024

#include <kernel/sysio.h>

#include <tty/tty.h>
#include <libc/stdio.h>

static char buffer[MAX_BUFFER_SIZE];

void __kprintf(const char* component, const char* fmt, va_list args)
{
    if (!fmt) return;
    char log_level = '0';
    char expanded_fmt[1024];
    uint16_t theme = tty_get_theme();

    if (*fmt == '\x1b')
    {
        log_level = *(++fmt);
        fmt++;
    }
    switch (log_level)
    {
    case '0':
        snprintf(expanded_fmt, 1024, "[%s] (%s) %s", "INFO", component, fmt);
        break;
    case '1':
        tty_set_theme(VGA_COLOR_BROWN, theme >> 12);
        snprintf(expanded_fmt, 1024, "[%s] (%s) %s", "WARN", component, fmt);
        break;
    case '2':
        tty_set_theme(VGA_COLOR_LIGHT_RED, theme >> 12);
        snprintf(expanded_fmt, 1024, "[%s] (%s) %s", "EROR", component, fmt);
        break;
    default:
        tty_set_theme(VGA_COLOR_GREEN, theme >> 12);
        snprintf(expanded_fmt, 1024, "[%s] (%s) %s", "LOG", component, fmt);
        break;
    }

    __sprintf_internal(buffer, expanded_fmt, 1024, args);
    tty_put_str(buffer);
    tty_set_theme(theme >> 8, theme >> 12);
}

void kprintf_panic(const char* fmt, ...) {
    va_list args;
    va_start(args, fmt);
    
    tty_set_theme(VGA_COLOR_WHITE, VGA_COLOR_RED);
    tty_clear_line(10);
    tty_clear_line(11);
    tty_clear_line(12);
    

    __sprintf_internal(buffer, fmt, MAX_BUFFER_SIZE, args);
    

    tty_set_pos(0, 10);
    tty_put_str("============================");
    tty_set_pos(0, 11);
    tty_put_str(buffer);
    tty_set_pos(0, 12);
    tty_put_str("============================");

    va_end(args);
}