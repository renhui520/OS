#include <mm/pmm.h>
#include <mm/page.h>

/*

0xFF ==> 1111 1111  用于标记 位图中8位都为 [已占用] 被用来标记所有页已占用
0x80 ==> 1000 0000  常用 ">>" 位移 配合来标记物理页是否可用

PPN(Physic Page Number)
bitmap[PPN / 8] & (0x80 >> (PPN % 8))

*/

#define MARK_PAGE(pnn)                  \
    uint32_t group = (pnn / 8);         \
    uint32_t msk = (0x80 >> (pnn % 8)); \
// pnn / 8无须取整，因为索引从0开始，默认就存在了向上取整的特性

#define MARK_CHUNK(start_pnn, page_count)             \
    uint32_t group = start_ppn / 8;                   \
    uint32_t offset = start_ppn % 8;                  \
    uint32_t group_count = (page_count + offset) / 8; \
    uint32_t remainder = (page_count + offset) % 8;   \
    uint32_t leading_shifts =                         \
        (page_count + offset) < 8 ? page_count : 8 - offset;


#define PM_BMP_MAX_SIZE (128*1024)  // 128KB 刚好能够存储 2^20 个页?
#define LOOKUP_START 1 // 我们跳过位于0x0的页。我们不希望空指针是指向一个有效的内存空间。


uintptr_t max_pg;       // 最大的页数
size_t pg_lookup_ptr;   // 指向 正在管理的页面
uint8_t bitmap[PM_BMP_MAX_SIZE];    // 位图


void pmm_init(uintptr_t mem_upper_lim)
{
    max_pg = (PG_ALIGN(mem_upper_lim) >> 12);   // PG_ALIGN(mem_upper_lim) >> 10 ==> KiB --> '>>' 2 ==> / 4
    pg_lookup_ptr = LOOKUP_START;
    for (size_t i = 0; i < PM_BMP_MAX_SIZE; i++)
    {
        bitmap[i] = 0xFFU;
    }
}
void pmm_mark_page_free(uintptr_t ppn)
{
    MARK_PAGE(ppn);
    bitmap[group] &= ~msk;
}
void pmm_mark_page_occupied(uintptr_t ppn)
{
    MARK_PAGE(ppn);
    bitmap[group] |= msk;
}
void pmm_mark_chunk_free(uintptr_t start_ppn, size_t page_count)
{
    MARK_CHUNK(start_ppn, page_count);

    //技术有限，以下代码搬来的，没看懂，以后再看，感觉是magic

    // nasty bit level hacks but it reduce # of iterations.
    bitmap[group] &= ~(((1U << leading_shifts) - 1) << (8 - offset - leading_shifts));

    group++;

    // prevent unsigned overflow
    for (uint32_t i = 0; group_count !=0 && i < group_count - 1; i++, group++) {
        bitmap[group] = 0;
    }

    bitmap[group] &=
      ~(((1U << (page_count > 8 ? remainder : 0)) - 1) << (8 - remainder));
}
void pmm_mark_chunk_occupied(uintptr_t start_ppn, size_t page_count)
{
    MARK_CHUNK(start_ppn, page_count);

    //同上

    bitmap[group] |= (((1U << leading_shifts) - 1) << (8 - offset - leading_shifts));

    group++;

    // prevent unsigned overflow
    for (uint32_t i = 0; group_count !=0 && i < group_count - 1; i++, group++) {
        bitmap[group] = 0xFFU;
    }

    bitmap[group] |=
        (((1U << (page_count > 8 ? remainder : 0)) - 1) << (8 - remainder));
}

// void* pmm_alloc_page(void)
// {
//     uintptr_t suitable_page = (uintptr_t) NULL; // 初始化为空，若没找到合适的位置，仍然返回空
//     size_t pg_last_ptr = pg_lookup_ptr;
//     size_t upper_lim = max_pg;
//     uint8_t chunk = 0;

//     while (!suitable_page && pg_lookup_ptr < upper_lim)
//     {
//         // 每 8 位 为 1 chunk
//         chunk = bitmap[pg_lookup_ptr >> 3]; // ">> 3" ==> "/ 8"

//         // 如果整个 块 非“全满”！
//         if (chunk != 0xFFU)
//         {
//             // i 不能为 0！！！
//             for (size_t i = (pg_lookup_ptr % 8); i < 8; i++, pg_lookup_ptr++)
//             {
//                 // 如果为 0 即 空闲，标记为 [已占用]，并取此处的指针所在页位置
//                 if (!(chunk & (0x80 >> (i % 8))))
//                 {
//                     pmm_mark_page_occupied(pg_lookup_ptr);
//                     suitable_page = (pg_lookup_ptr << 12);  // << 12 ==> * 4KiB
//                     break;
//                 }
//             }            
//         } else {
//             pg_lookup_ptr += 8;
            
//             // 若 页指针 超过 顶部限制，而且 上一个指针位置不为 LOOKUP_START
//             // 则从头开始重新寻找
//             if (pg_lookup_ptr >= upper_lim && pg_last_ptr != LOOKUP_START)
//             {
//                 upper_lim = pg_last_ptr;         // 最高限制 等于 最后一个指针所在位置
//                 pg_lookup_ptr = LOOKUP_START;   // 超过限制，回到开头 1
//                 pg_last_ptr = LOOKUP_START;      // 并且上一个指针指向的位置不为 1，回到 1
//             }
//         }
//     }
//     return (void*) suitable_page;
// }

void* pmm_alloc_page(void)
{
    uintptr_t suitable_page = (uintptr_t) NULL; // 初始化为空，若没找到合适的位置，仍然返回空
    size_t pg_last_ptr = pg_lookup_ptr;
    size_t upper_lim = max_pg;
    uint8_t chunk = 0;

    while (!suitable_page && pg_lookup_ptr < upper_lim)
    {
        // 每 8 位 为 1 chunk
        chunk = bitmap[pg_lookup_ptr >> 3]; // ">> 3" ==> "/ 8"

        // 如果整个 块 为“全满”！
        if (chunk == 0xFFU)
        {
            pg_lookup_ptr += 8;
            // 若 页指针 超过 顶部限制，而且 上一个指针位置不为 LOOKUP_START
            // 则从头开始重新寻找
            if (pg_lookup_ptr >= upper_lim && pg_last_ptr != LOOKUP_START)
            {
                upper_lim = pg_last_ptr;      // 最高限制 等于 最后一个指针所在位置
                pg_lookup_ptr = LOOKUP_START; // 超过限制，回到开头 1
                pg_last_ptr = LOOKUP_START;   // 并且上一个指针指向的位置不为 1，回到 1
            }
            continue;
        }
        // i 不能为 0！！！
        for (size_t i = (pg_lookup_ptr % 8); i < 8; i++, pg_lookup_ptr++)
        {
            // 如果为 0 即 空闲，标记为 [已占用]，并取此处的指针所在页位置
            if (!(chunk & (0x80 >> (i % 8))))
            {
                pmm_mark_page_occupied(pg_lookup_ptr);
                suitable_page = (pg_lookup_ptr << 12);  // << 12 ==> * 4KiB
                break;
            }
        }
    }
    return (void*) suitable_page;
}




int pmm_free_page(void* page)
{
    uintptr_t occupied_page = (uintptr_t)page >> 12;
    if (occupied_page && occupied_page < max_pg)
    {
        pmm_mark_page_free(occupied_page);
    }
    return 0;
}


