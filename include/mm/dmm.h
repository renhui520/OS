#ifndef __OS_DMM_H__
#define __OS_DMM_H__

#include <stdint.h>
#include <stddef.h>

/* ------------------------专门为 place_chunk coalesce grow_heap 所准备的----------------------- */
// 已分配
#define M_ALLOCATED 0x1     // 当前内存块   [已分配]
#define M_PREV_ALLOCATED 0x0// 前一个内存块  [已分配]

// 未分配
#define M_NOT_ALLOCATED 0x0 // 当前内存块   [未分配]
#define M_PREV_FREE 0x2     // 前一个内存块  [未分配]

// 检测
#define CHUNK_S(header) ((header) & ~0x3)           //提取内存块的大小
#define CHUNK_PF(header) ((header)&M_PREV_FREE)     //判断 前一个内存块 是否 [未分配]
#define CHUNK_A(header) ((header)&M_ALLOCATED)      //判断 当前内存块 是否 [已分配]

// 打包
#define PACK(size, flags) (((size) & ~0x3) | (flags))   //打包 内存块大小 和 分配标志 (也就是往内存低位写入flags)

#define SW(p, w) (*((uint32_t*)(p)) = w)            //将w写入p
#define LW(p) (*((uint32_t*)(p)))                   //从地址p中读取一个uint32_t(32位整数)

#define HPTR(bp) ((uint32_t*)(bp)-1)                //获取(返回) 内存块bp的header指针
#define BPTR(bp) ((uint8_t*)(bp) + WSIZE)           //获取(返回) 内存块bp的 起始 指针
#define FPTR(hp, size) ((uint32_t*)((uintptr_t)hp + size - WSIZE))//获取(返回) 内存块bp的footer指针
#define NEXT_CHK(hp) ((uint8_t*)(hp) + CHUNK_S(LW(hp)))//获取(返回) 内存块bp的下一个内存块的 起始位置

/* ----------------------------------------------------------------------------------------- */


#define BOUNDARY 4  //内存块 边界对齐 大小
#define WSIZE 4     //内存块 大小(单位: 字节)

#define HEAP_INIT_SIZE 4096 //堆初始大小


typedef struct 
{
    void* start;    // 堆 的起始地址
    void* brk;      // 当前 堆 的末端地址
    void* max_addr; // 堆 的最大可使用地址
} heap_context_t;



int dmm_init(heap_context_t* heap);  // 动态内存初始化，为堆分配内存页


// 采用4字节对齐
int sbrk(heap_context_t* heap, void* addr);   // 设置上限至新的地址
void* brk(heap_context_t* heap, size_t size);   // 将当前上限扩展到x个字节
void place_chunk(uint8_t* ptr, size_t size); // 分割空闲内存块，分配出一块大小为size的内存块
void* coalesce(uint8_t* chunk_ptr); // 合并相邻空闲块
void* grow_heap(heap_context_t* heap, size_t size); // 增长堆区


#endif