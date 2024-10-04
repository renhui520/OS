#include <mm/dmm.h>

#include <mm/vmm.h>
#include <mm/page.h>
#include <kernel/spike.h>

/*

1、空闲块

    +---------------+
    | size | PF | A |   <-------header
    +---------------+
    |               |
    |               |
    |      空闲      |
    |               |
    |               |
    +---------------+
    | size | PF | A |   <-------footer
    +---------------+

    PF = 1 前一个区块空闲
    A = 1  当前区块已分配

    header永远等于footer
      header = footer

2、已分配块

    +---------------+
    | size | PF | A |   <-------header
    +---------------+
    |               |
    |               |
    |    有效载荷    |
    |               |
    |               |
    +---------------+   <-------无footer


================================================

    +----------------+                                       +------------------------+
    | size1 | ?? | 0 |   <-------header_1                    | size1 + size2 | ?? | 0 |   <-------header_1
    +----------------+                                       +------------------------+
    |                |                                       |                        |
    |                |                                       |                        |
    |      空闲       |                                       |                        |
    |                |                                       |                        |
    |                |                                       |                        |
    +----------------+                                       |                        |
    | size1 | ?? | 0 |   <-------footer_1                    |                        |
    +----------------+                               ====>   |          空闲           |
    | size2 | 1  | 0 |   <-------header_2                    |                        |
    +----------------+                                       |                        |
    |                |                                       |                        |
    |                |                                       |                        |
    |      空闲       |                                       |                        |
    |                |                                       |                        |
    |                |                                       |                        |
    +----------------+                                       +------------------------+
    | size2 | 1  | 0 |   <-------footer_2                    | size1 + size2 | ?? | 0 |   <-------footer_1
    +----------------+                                       +------------------------+
*/



// 1、brk中无return则正常，有则重启一遍
// 2、brk函数中无任何内容 则重启; 仅存在return时，重启!
// 3、dmm_init函数存在时，则重启
// 4、单独把dmm_init函数注释掉，则不重启
// 5、dmm.h中的dmm_init声明不对，目前测试了一下，如果改为int且return则不重启

// bug未知，但是暂且解决了
int dmm_init(heap_context_t* heap)
{
    heap->brk = heap->start;

    // 为 kalloc 分配一个适合的页
    return vmm_alloc_page(heap->brk, PG_PREM_RW) != NULL; // 没有这个就算编译过了，运行到setmemory也会重启
}

// 设置上限至新的地址
int sbrk(heap_context_t* heap, void* addr)
{
    return brk(heap, ((uintptr_t)(addr)) - ((uintptr_t)(heap->brk))) != NULL;
}

// 将当前上限扩展到x个字节
void* brk(heap_context_t* heap, size_t size)
{
    // 如果 size为0则无须扩展
    if (size == 0)
    {
        return heap->brk;
    }

    // 保存当前的 brk 指针值
    void* current_brk = heap->brk;

    // 计算新的 brk 指针值，向上取整到 BOUNDARY 对齐
    // 这个值用于计算需要分配的页面
    void* next = (void*)((uintptr_t)current_brk + ROUNDUP(size, BOUNDARY));

    // 检查是否超出最大地址范围或小于当前 brk 指针
    // 如果是，则返回 NULL 表示失败
    if (next >= heap->max_addr || next < current_brk)
    {
        return NULL;
    }

    // 计算需要分配的页面大小差值
    uintptr_t diff = PG_ALIGN(next) - PG_ALIGN(current_brk);

    // 因为传参进来的size会加上 WSIZE, 所以必须要判断以下，不然会出错
    if (diff)
    {
        // 分配新的页面
        // 注意：PG_ALIGN(current_brk) + PG_SIZE 是从对齐后的当前 brk 指针开始分配
        if (!vmm_alloc_pages((void *)(PG_ALIGN(current_brk) + PG_SIZE),
                            diff,
                            PG_PREM_RW))
        {
            // for debugging
            // assert_msg(0, "unable to brk");

            // 分配失败
            return NULL;
        }
    }
    
    // 更新堆的 brk 指针
    heap->brk = (void *)((uintptr_t)heap->brk + size);

    // 返回原来的 brk 指针
    return current_brk;
}




// 还没看

// 从一个大的内存块中分割出一块 大小为size的内存块 (并标记?)
void place_chunk(uint8_t* ptr, size_t size)
{
    uint32_t header = *((uint32_t*)ptr);
    size_t chunk_size = CHUNK_S(header);
    *((uint32_t*)ptr) = PACK(size, CHUNK_PF(header) | M_ALLOCATED);
    uint8_t* n_hdrptr = (uint8_t*)(ptr + size);
    uint32_t diff = chunk_size - size;

    if (!diff) {
        // if the current free block is fully occupied
        uint32_t n_hdr = LW(n_hdrptr);
        // notify the next block about our avaliability
        SW(n_hdrptr, n_hdr & ~0x2);
    } else {
        // if there is remaining free space left
        uint32_t remainder_hdr = PACK(diff, M_NOT_ALLOCATED | M_PREV_ALLOCATED);
        SW(n_hdrptr, remainder_hdr);
        SW(FPTR(n_hdrptr, diff), remainder_hdr);

        /*
            | xxxx |      |         |

                        |
                        v
                        
            | xxxx |                |
        */
        coalesce(n_hdrptr);
    }
}


// 合并相邻空闲块
void* coalesce(uint8_t* chunk_ptr)
{
    uint32_t hdr = LW(chunk_ptr);
    uint32_t pf = CHUNK_PF(hdr);
    uint32_t sz = CHUNK_S(hdr);

    uint32_t n_hdr = LW(chunk_ptr + sz);

    if (CHUNK_A(n_hdr) && pf)
    {
        // case 1: prev is free
        uint32_t prev_ftr = LW(chunk_ptr - WSIZE);
        size_t prev_chunk_sz = CHUNK_S(prev_ftr);
        uint32_t new_hdr = PACK(prev_chunk_sz + sz, CHUNK_PF(prev_ftr));
        SW(chunk_ptr - prev_chunk_sz, new_hdr);
        SW(FPTR(chunk_ptr, sz), new_hdr);
        chunk_ptr -= prev_chunk_sz;
    }
    else if (!CHUNK_A(n_hdr) && !pf)
    {
        // case 2: next is free
        size_t next_chunk_sz = CHUNK_S(n_hdr);
        uint32_t new_hdr = PACK(next_chunk_sz + sz, pf);
        SW(chunk_ptr, new_hdr);
        SW(FPTR(chunk_ptr, sz + next_chunk_sz), new_hdr);
    }
    else if (!CHUNK_A(n_hdr) && pf)
    {
        // case 3: both free
        uint32_t prev_ftr = LW(chunk_ptr - WSIZE);
        size_t next_chunk_sz = CHUNK_S(n_hdr);
        size_t prev_chunk_sz = CHUNK_S(prev_ftr);
        uint32_t new_hdr =
            PACK(next_chunk_sz + prev_chunk_sz + sz, CHUNK_PF(prev_ftr));
        SW(chunk_ptr - prev_chunk_sz, new_hdr);
        SW(FPTR(chunk_ptr, sz + next_chunk_sz), new_hdr);
        chunk_ptr -= prev_chunk_sz;
    }

    // (fall through) case 4: prev and next are not free
    return chunk_ptr;
}


// 增长堆区
void* grow_heap(heap_context_t* heap, size_t size)
{
    // 检测是否可以增长
    void* start = brk(heap, WSIZE + size);  // 之所以加上 WSIZE 是因为 kalloc开头会占用4字节空间(可能是用于NULL指针位置)
    if (!start)
    {
        return NULL;
    }
    size = ROUNDUP(size, BOUNDARY);

    heap->brk = (void*)((uintptr_t)heap->brk - WSIZE);

    uint32_t old_marker = *((uint32_t*)start);  // 获得写入到当前start地址中的header数据?
    uint32_t free_hdr = PACK(size, CHUNK_PF(old_marker)/*查看是否[已分配]*/);   // 标记32位地址中最后两位
    SW(start, free_hdr);    // 往内存中写入新的header数据
    SW(FPTR(start, size), free_hdr);    // 往内存中写入新的footer数据
    SW(NEXT_CHK(start), PACK(0, M_ALLOCATED | M_PREV_FREE));    // 往下一个块的头(header) 写入新的header数据 标记前一块内存[未分配]

    return coalesce(start);

}
