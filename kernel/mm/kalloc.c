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

    SW(__kalloc_kheap.start, PACK(4, M_ALLOCATED));// 可能是空指针吧   // 初始的header_1   标记当前块大小为 4 下一个header_2则位置+4
    SW((uintptr_t)__kalloc_kheap.start + WSIZE, PACK(0, M_ALLOCATED)); // 初始化一个大小为0的header_2 如下所示
    __kalloc_kheap.brk = (void*)((uintptr_t)__kalloc_kheap.brk + WSIZE);

    return grow_heap(&__kalloc_kheap, HEAP_INIT_SIZE) != NULL;
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
        uint32_t header = *((uint32_t*)ptr);
        size_t chunk_size = CHUNK_S(header);
        if (!chunk_size && CHUNK_A(header)) {
            break;
        }
        if (chunk_size >= size && !CHUNK_A(header)) {
            // found!
            place_chunk(ptr, size);
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

void* malloc(size_t size) {
    return __malloc_internal(&__kalloc_kheap, size);
}

void* calloc(size_t n, size_t elem) {
    size_t pd = n * elem;

    // overflow detection
    if (pd < elem || pd < n) {
        return NULL;
    }

    void* ptr = malloc(pd);
    if (!ptr) {
        return NULL;
    }

    return memset(ptr, 0, pd);
}

void free(void* ptr) {
    if (!ptr) {
        return;
    }

    uint8_t* chunk_ptr = (uint8_t*)ptr - WSIZE;
    uint32_t hdr = LW(chunk_ptr);
    size_t sz = CHUNK_S(hdr);
    uint8_t* next_hdr = chunk_ptr + sz;

    // make sure the ptr we are 'bout to free makes sense
    //   the size trick is stolen from glibc's malloc/malloc.c:4437 ;P
    
    assert_msg(((uintptr_t)ptr < (uintptr_t)(-sz)) && !((uintptr_t)ptr & 0x3),
               "free(): invalid pointer");
    
    assert_msg(sz > WSIZE,
               "free(): invalid size");

    SW(chunk_ptr, hdr & ~M_ALLOCATED);
    SW(FPTR(chunk_ptr, sz), hdr & ~M_ALLOCATED);
    SW(next_hdr, LW(next_hdr) | M_PREV_FREE);
    
    coalesce(chunk_ptr);
}

