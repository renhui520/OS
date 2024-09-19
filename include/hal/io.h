#include <stdint.h>

static inline uint8_t io_inb(int port)
{
    uint8_t data;
    asm volatile(
        "inb %w1,%0"
        : "=a"(data)
        : "d"(port)
        : "memory"
    );
    return data;
}


static inline void outb(int port, uint8_t data)
{
    asm volatile (
        "outb %0, %w1"  // 将 value 输出到 port
        :
        : "a"(data), "d"(port)
        : "memory"
    );
}