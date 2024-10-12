#ifndef __OS_COMMON_H__
#define __OS_COMMON_H__

/*
 * 操作系统的通用配置(默认配置)
 *
*/


#define MAX_BUFFER_SIZE 1024    // 最大缓冲区大小

#define K_STACK_SIZE            (64 << 10)                              //内核栈大小为64KB
#define K_STACK_START           ((0xFFBFFFFFU - K_STACK_SIZE) + 1)      //内核栈起始地址

#define VGA_BUFFER_VADDR    0xB0000000UL    // VGA 缓冲区虚拟地址
#define VGA_BUFFER_PADDR    0xB8000UL       // VGA 缓冲区物理地址
#define VGA_BUFFER_SIZE     4096            // VGA 缓冲区大小

#endif