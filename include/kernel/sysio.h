#ifndef __OS_SYSIO_H__
#define __OS_SYSIO_H__

#include <stdarg.h>

#define _LEVEL_INFO     "1"
#define _LEVEL_NOTICE   "2"
#define _LEVEL_WARN     "3"
#define _LEVEL_ERROR    "4"
#define _LEVEL_INIT     "5"
#define _LEVEL_LOG      "6"
#define _LEVEL_DEBUG    "7"
#define _LEVEL_NONE     "8"

#define KDEBUG   "\x1b" _LEVEL_DEBUG
#define KINFO    "\x1b" _LEVEL_INFO
#define KNOTICE  "\x1b" _LEVEL_NOTICE
#define KWARN    "\x1b" _LEVEL_WARN
#define KERROR   "\x1b" _LEVEL_ERROR
#define KINIT    "\x1b" _LEVEL_INIT
#define KLOG     "\x1b" _LEVEL_LOG
#define KNONE    "\x1b" _LEVEL_NONE

//static使函数局部可见，当调用这个宏的时候会生成一个kprintf函数，且还会由于static避免与其他文件的kprintf名称冲突
#define LOG_MODULE(module)                  \
    static void kprintf(const char* fmt, ...) \
    {                                       \
        va_list args;                       \
        va_start(args, fmt);                \
        __kprintf(module, fmt, args);       \
        va_end(args);                       \
    }


void __kprintf(const char* component, const char* fmt, va_list args);
void kprintf_panic(const char* fmt, ...);

#endif