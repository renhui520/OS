#ifndef __OS_COMMON_H__
#define __OS_COMMON_H__

#define K_STACK_SIZE            (64 << 10)                              //内核栈大小为64KB
#define K_STACK_START           ((0xFFBFFFFFU - K_STACK_SIZE) + 1)      //内核栈起始地址

#endif