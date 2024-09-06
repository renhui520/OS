#ifndef __TTY_H_
#define __TTY_H_

#include <stdint.h>

#define VGA_COLOR_BLACK 0
#define VGA_COLOR_BLUE 1
#define VGA_COLOR_GREEN 2
#define VGA_COLOR_CYAN 3
#define VGA_COLOR_RED 4
#define VGA_COLOR_MAGENTA 5
#define VGA_COLOR_BROWN 6
#define VGA_COLOR_LIGHT_GREY 7
#define VGA_COLOR_DARK_GREY 8
#define VGA_COLOR_LIGHT_BLUE 9
#define VGA_COLOR_LIGHT_GREEN 10
#define VGA_COLOR_LIGHT_CYAN 11
#define VGA_COLOR_LIGHT_RED 12
#define VGA_COLOR_LIGHT_MAGENTA 13
#define VGA_COLOR_LIGHT_BROWN 14
#define VGA_COLOR_WHITE 15

void tty_init(void* vga_buffer);
void tty_set_theme(uint16_t fg, uint16_t bg);
void tty_put_char(const char chr);
void tty_put_str(const char* str);
void tty_clear();
void tty_clear_line(unsigned int y);
void tty_set_pos(unsigned int x, unsigned int y);

// get information about the tty
uint16_t tty_get_theme();
void tty_get_pos(unsigned int* x, unsigned int* y);

#endif