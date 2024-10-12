#include <mm/kalloc.h>

#include <kernel/common.h>
#include <kernel/spike.h>
#include <mm/dmm.h>
#include <libc/string.h>

extern uint8_t __kernel_heap_start;
static heap_context_t __kalloc_kheap;

int kalloc_init() {
    __kalloc_kheap.start = &__kernel_heap_start;
    __kalloc_kheap.brk = NULL;
    __kalloc_kheap.max_addr = (void*)K_STACK_START;

    // dmm_init 为 内核堆的起始地址分配一个可用的页      |   4KiB   |
    //                                     起始地址 ^           ^末地址
    if (!dmm_init(&__kalloc_kheap)) {
        return 0;
    }

    // 头标志位用来查看开头第一个内存块是否被使用？
    SW(__kalloc_kheap.start, PACK(4, M_ALLOCATED));// 可能是空指针吧   // 初始的header_1   标记当前块大小为 4 下一个header_2则位置+4

    // 第一个内存块
    SW((uintptr_t)__kalloc_kheap.start + WSIZE, PACK(0, M_ALLOCATED)); // 初始化一个大小为0的header_2 如下所示
    
    // __kalloc_kheap.brk等同于start+WSIZE，因为grow_heap会重新复写__kalloc_kheap.start+WSIZE位置的内容(也就是brk), 所以即使上面被标记为[已分配]也无所谓
    __kalloc_kheap.brk = (void*)((uintptr_t)__kalloc_kheap.brk + WSIZE);    // 指向__kalloc_kheap.start + WSIZE

    // 扩展堆区到 HEAP_INIT_SIZE，此操作会修改__kalloc_kheap.brk位置的数据，修改为header数据，所以上方的start+WSIZE的内容无须在意
    return grow_heap(&__kalloc_kheap, HEAP_INIT_SIZE) != NULL;

/*
总的数据结构大概是这个样式:


1. before grow_heap:

 Header  header(占用4字节，标记内存段大小为0)
+------++---------------+
|  4B  ||  4B    size=0 |
+------++---------------+
        ^
        | __kalloc_kheap.brk

2. after grow_heap:

grow_heap会分配多4字节，也就是+WSIZE用来存放header，防止实际的size小于理论上的size
grow_heap会重新修改header，所以不必担心header的问题
footer会在这段内存被使用的时候删除，减少不必要的内存占用, 所以不必担心size!=HEAP_INIT_SIZE的情况

 Header  header             data
+------++------+-----------------------------+
|  4B  ||  4B  | size=HEAP_INIT_SIZE + WSIZE |
+------++------+-----------------------------+
                                             ^
                                             | __kalloc_kheap.brk

WSIZE = 4Bytes
因为 header + data = HEAP_INIT_SIZE
所以实际上的data = HEAP_INIT_SIZE - WSIZE
data < HEAP_INIT_SIZE

所以需要加上WSIZE, 用于抵消header的内存占用

                    |
                    V

 Header  header         data          footer(内存空闲时存在)
+------++------+---------------------+-------+
|  4B  ||  4B  | size=HEAP_INIT_SIZE | WSIZE |
+------++------+---------------------+-------+
                                     ^
                                     | __kalloc_kheap.brk

                    |
                    V

 Header  header         data          footer next_header
+------++------+---------------------+-------++-------+
|  4B  ||  4B  | size=HEAP_INIT_SIZE | WSIZE || WSIZE |
+------++------+---------------------+-------++-------+
                                     ^
                                     | __kalloc_kheap.brk

3. use data:

 Header  header             data             next_header
+------++------+-----------------------------++-------+
|  4B  ||  4B  |     size=HEAP_INIT_SIZE     || WSIZE |
+------++------+-----------------------------++-------+
                                              ^
                                              | __kalloc_kheap.brk

*/


}

void* __malloc_internal(heap_context_t* heap, size_t size)
{
    // Simplest first fit approach.

    if (!size) {
        return NULL;
    }

    uint8_t* ptr = heap->start;
    // round to largest 4B aligned value
    //  and space for header
    size = ROUNDUP(size + WSIZE, BOUNDARY);
    while (ptr < (uint8_t*)heap->brk) {
        uint32_t header = LW(ptr);  // 获得头数据
        size_t chunk_size = CHUNK_S(header);
        
        // 如果到 堆区的末尾 则退出，堆区末尾通常大小为0，且被标记为[已分配]
        if (!chunk_size && CHUNK_A(header)) {
            break;
        }
        
        // 如果当前内存块的大小大于或等于 所需的size大小 且 [未分配]
        if (chunk_size >= size && !CHUNK_A(header)) {
            // found!
            place_chunk(ptr, size); // 分割块 并 分配
            return BPTR(ptr);
        }
        ptr += chunk_size;
    }

    // if heap is full (seems to be!), then allocate more space (if it's
    // okay...)
    if ((ptr = grow_heap(heap, size))) {
        place_chunk(ptr, size);
        return BPTR(ptr);
    }

    // Well, we are officially OOM!
    return NULL;
}

// 分配 堆区 内存
void* malloc(size_t size) {
    return __malloc_internal(&__kalloc_kheap, size);
}

// 重新 分配指定数量的 堆区 内存 即 nmemb * size (会清空内存)
void* calloc(size_t nmemb, size_t per_size) {
    size_t pd = nmemb * per_size;

    // overflow detection
    // 检测是否内存溢出
    if (pd < per_size || pd < nmemb) {
        return NULL;
    }

    void* ptr = malloc(pd); // 分配内存
    if (!ptr) {             // 如果分配失败，则返回 NULL
        return NULL;
    }

    return memset(ptr, 0, pd);  // 清空内存
}

// 释放 堆区 内存
void free(void* ptr) {
    if (!ptr) {
        return;
    }

    uint8_t* chunk_ptr = (uint8_t*)ptr - WSIZE;
    uint32_t hdr = LW(chunk_ptr);       // 当前header
    size_t sz = CHUNK_S(hdr);           // 当前内存块大小
    uint8_t* next_hdr = chunk_ptr + sz; // 下一个header

    // make sure the ptr we are 'bout to free makes sense
    //   the size trick is stolen from glibc's malloc/malloc.c:4437 ;P
    
    assert_msg(((uintptr_t)ptr < (uintptr_t)(-sz)) && !((uintptr_t)ptr & 0x3),
               "free(): invalid pointer");
    
    assert_msg(sz > WSIZE,
               "free(): invalid size");

    SW(chunk_ptr, hdr & ~M_ALLOCATED);              // 标记当前内存块头部 为 [未分配]
    SW(FPTR(chunk_ptr, sz), hdr & ~M_ALLOCATED);    // 标记当前内存块脚部 为 [未分配]
    SW(next_hdr, LW(next_hdr) | M_PREV_FREE);       // 标记下一个内存块的头部的 "前一内存块" 为 [未分配]
    
    coalesce(chunk_ptr);    // 合并 当前 内存块 的 前后 空闲内存块
}

