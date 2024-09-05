#define __LIBC__

#include <libc/string.h>
#include <libc/stdio.h>

#include <stdint.h>



/*---------------------抄过来的--------------------*/

#define NUMBUFSIZ 24

static const char flag_chars[] = "#0- +";

#define FLAG_ALT                (1<<0)
#define FLAG_ZERO               (1<<1)
#define FLAG_LEFTJUSTIFY        (1<<2)
#define FLAG_SPACEPOSITIVE      (1<<3)
#define FLAG_PLUSPOSITIVE       (1<<4)
#define FLAG_NUMERIC            (1<<5)
#define FLAG_SIGNED             (1<<6)
#define FLAG_NEGATIVE           (1<<7)
#define FLAG_ALT2               (1<<8)
#define FLAG_CAPS               (1<<9)

void __sprintf_internal(char* buffer, char* fmt, size_t max_len, va_list args)
{
    char numbuf[NUMBUFSIZ];
    uint32_t ptr = 0;
    for (; *fmt; ++fmt) {
        if (max_len && ptr >= max_len - 1) {
            break;
        }
        
        if (*fmt != '%') {
            buffer[ptr++] = *fmt;
            continue;
        }

        // process flags
        int flags = 0;
        for (++fmt; *fmt; ++fmt) {
            const char* flagc = strchr(flag_chars, *fmt);
            if (flagc) {
                flags |= 1 << (flagc - flag_chars);
            } else {
                break;
            }
        }

        // process width
        int width = -1;
        if (*fmt >= '1' && *fmt <= '9') {
            for (width = 0; *fmt >= '0' && *fmt <= '9'; ) {
                width = 10 * width + *fmt++ - '0';
            }
        } else if (*fmt == '*') {
            width = va_arg(args, int);
            ++fmt;
        }

        // process precision
        int precision = -1;
        if (*fmt == '.') {
            ++fmt;
            if (*fmt >= '0' && *fmt <= '9') {
                for (precision = 0; *fmt >= '0' && *fmt <= '9'; ) {
                    precision = 10 * precision + *fmt++ - '0';
                }
            } else if (*fmt == '*') {
                precision = va_arg(args, int);
                ++fmt;
            }
            if (precision < 0) {
                precision = 0;
            }
        }

        // process main conversion character
        int base = 10;
        unsigned long num = 0;
        int length = 0;
        char* data = "";
    again:
        switch (*fmt) {
        case 'l':
        case 'z':
            length = 1;
            ++fmt;
            goto again;
        case 'd':
        case 'i': {
            long x = length ? va_arg(args, long) : va_arg(args, int);
            int negative = x < 0 ? FLAG_NEGATIVE : 0;
            num = negative ? -x : x;
            flags |= FLAG_NUMERIC | FLAG_SIGNED | negative;
            break;
        }
        case 'u':
        format_unsigned:
            num = length ? va_arg(args, unsigned long) : va_arg(args, unsigned);
            flags |= FLAG_NUMERIC;
            break;
        case 'x':
            base = 16;
            goto format_unsigned;
        case 'X':
            flags = flags | FLAG_CAPS;
            base = 16;
            goto format_unsigned;
        case 'p':
            num = (uintptr_t) va_arg(args, void*);
            base = 16;
            flags |= FLAG_ALT | FLAG_ALT2 | FLAG_NUMERIC;
            break;
        case 's':
            data = va_arg(args, char*);
            break;
        case 'c':
            data = numbuf;
            numbuf[0] = va_arg(args, int);
            numbuf[1] = '\0';
            break;
        default:
            data = numbuf;
            numbuf[0] = (*fmt ? *fmt : '%');
            numbuf[1] = '\0';
            if (!*fmt) {
                fmt--;
            }
            break;
        }

        if (flags & FLAG_NUMERIC) {
            data = itoa(num, numbuf, base);
            int i = 0;
            char c;
            while ((flags & FLAG_CAPS) && (c = data[i]))
            {
                data[i] = c & ~((c & 0x40) >> 1);
                i++;
            }
        }

        const char* prefix = "";
        if ((flags & FLAG_NUMERIC) && (flags & FLAG_SIGNED)) {
            if (flags & FLAG_NEGATIVE) {
                prefix = "-";
            } else if (flags & FLAG_PLUSPOSITIVE) {
                prefix = "+";
            } else if (flags & FLAG_SPACEPOSITIVE) {
                prefix = " ";
            }
        } else if ((flags & FLAG_NUMERIC) && (flags & FLAG_ALT)
                   && (base == 16 || base == -16)
                   && (num || (flags & FLAG_ALT2))) {
            prefix = "0x";
        }

        int len;
        if (precision >= 0 && !(flags & FLAG_NUMERIC)) {
            len = strnlen(data, precision);
        } else {
            len = strlen(data);
        }
        int zeros;
        if ((flags & FLAG_NUMERIC) && precision >= 0) {
            zeros = precision > len ? precision - len : 0;
        } else if ((flags & FLAG_NUMERIC) && (flags & FLAG_ZERO)
                   && !(flags & FLAG_LEFTJUSTIFY)
                   && len + (int) strlen(prefix) < width) {
            zeros = width - len - strlen(prefix);
        } else {
            zeros = 0;
        }
        width -= len + zeros + strlen(prefix);
        for (; !(flags & FLAG_LEFTJUSTIFY) && width > 0; --width) {
            buffer[ptr++] = ' ';
        }
        for (; *prefix; ++prefix) {
            buffer[ptr++] = *prefix;
        }
        for (; zeros > 0; --zeros) {
            buffer[ptr++] = '0';
        }
        for (; len > 0; ++data, --len) {
            buffer[ptr++] = *data;
        }
        for (; width > 0; --width) {
            buffer[ptr++] = ' ';
        }
    }
    buffer[ptr++] = '\0';
}
/*-------------------------------------------以上不是我写的:( */

//改天有空再写自己的
// void __sprintf_internal(char* buffer, char* fmt, size_t max_len, va_list args)
// {
//     if (buffer == NULL || fmt == NULL) return;
//     char* buf = buffer;
//     const char* fmt_ptr = fmt;
//     while (*fmt_ptr != '\0')
//     {
//         if (*fmt_ptr == '%')
//         {
//             fmt_ptr++;
//             switch (*fmt_ptr)
//             {
//             case 'd':
                
//                 break;
//             }
//         }
        
//     }
    
// }

void sprintf(char* buffer, const char* fmt, ...)
{
    va_list args;
    va_start(args, fmt);
    __sprintf_internal(buffer, fmt, 0, args);
    va_end(args);
}

void snprintf(char* buffer, size_t n, char* fmt, ...)
{
    va_list args;
    va_start(args, fmt);
    __sprintf_internal(buffer, fmt, n, args);
    va_end(args);
}


