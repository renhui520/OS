#include <mm/dmm.h>

#include <mm/vmm.h>
#include <mm/page.h>
#include <kernel/spike.h>

void dmm_init(heap_context_t* heap)
{
    heap->brk = heap->start;
}

// 将当前上限扩展到x个字节
void* brk(heap_context_t* heap, size_t size)
{
    if (size == 0)
    {
        return heap->brk;
    }

    void* current_brk = heap->brk;

    // The upper bound of our next brk of heap given the size.
    // This will be used to calculate the page we need to allocate.
    void* next = (void*)((uintptr_t)current_brk + ROUNDUP(size, BOUNDARY));

    // any invalid situations
    if (next >= heap->max_addr || next < current_brk)
    {
        return NULL;
    }

    uintptr_t diff = PG_ALIGN(next) - PG_ALIGN(current_brk);
    if (diff)
    {
        // if next do require new pages to be allocated
        if (!vmm_alloc_pages((void*)(PG_ALIGN(current_brk) + PG_SIZE),
                             diff,
                             PG_PREM_RW))
        {
            // for debugging
            // assert_msg(0, "unable to brk");
            return NULL;
        }
    }

    heap->brk = (void*)((uintptr_t)heap->brk + size);
    return current_brk;
}

// 设置上限至新的地址
// int sbrk(heap_context_t* heap, void* addr)
// {

// }



