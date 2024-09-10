#include <stdint.h>
#include <boot/multiboot.h>
#include <mm/page.h>

// FOR VBE 开启VBE测试功能
// #define __VBE__


//检查 multiboot flags 是否 等于 某个标志位（info）
//若是 返回非0
//若否 返回 0
#define present(flags, info)      ((flags) & (info))

//PT_ADDR获得偏移一个页表(跳过0号页目录表)的地址
#define PT_ADDR(ptd, pt_index)                          ((uint32_t*)ptd + (pt_index + 1) * 1024)
/*
pde_index为ptd的索引 指向 pde的页表pt
pde指向页表 也是页目录项
pde_index范围 [0, 1024) 即 ptd[0][1024]为页目录 超过范围则是其他PT
例如ptd[0][1024] 则是 ptd[1][0]
实际上是 SET_PDE(ptd, pde_index, pde)   ptd[0][pde_index] = pde
ptd[0][pde_index] = pde 代表ptd第0个页表(页目录)的第 pde_index 项 等于 pde

    |   PD   |  4K  0~1023
    |   PT   |  4K
    |   PT   |  4K
    |   PT   |  4K */


/**
 * @brief 将 具体页表 转化为 页目录索引(PDE) 保存在 "页目录页表"的特定位置
 * @param ptd 页目录基地址
 * @param pde_index 页目录项索引(页目录也表的"特定位置") 即在页目录表(PT 0)中的页表项ptd[0][pde_index]
 * @param pde 具体页表地址(NEW_L1_ENTRY)
 * @note pde_index 表示 线性地址(32位)中的 高10位
*/
#define SET_PDE(ptd, pde_index, pde)                    *((uint32_t*)ptd + pde_index) = pde;

/**
 * @brief 将 具体页表项 转化为 页表项(PTE) 保存在 "具体页表"
 * @param ptd 页目录基地址
 * @param pt_index 具体第几个页表 并非 页目录第几个选项 而是实际页 如：0(页目录)，1(PT)，2(PT)，3(PT)中的1,2,3
 * @param pte_index 具体页表中的第几个 页表项
 * @param pte 具体物理页 基地址(NEW_L2_ENTRY)
 * @note pte_index 表示 线性地址(32位)中的 中10位
 * @note SET_PTE会默认向后偏移一个页表, 所以0为页表1，1为页表2, 以此类推
 * */
#define SET_PTE(ptd, pt_index, pte_index, pte)         *(PT_ADDR(ptd, pt_index) + pte_index) = pte;

// 获得 符号(变量？) 的 线性地址
#define sym_val(sym)                                 (uintptr_t)(&sym)

//计算内核和hhk_init所需的页表数量
//（kernel_end - kernel_start + 0x1000 - 1） / 4KiB 得到页表数量
#define KERNEL_PAGE_COUNT           ((sym_val(__kernel_end) - sym_val(__kernel_start) + 0x1000 - 1) >> 12);

//sym_val获取__init_hhk_end的物理地址 并减去 0x100000 从而得出所使用的内存数量 0x1000为页大小4KiB, ">> 12"相当于除以4KiB, 最后得出所需页数
// +0x1000-1：0x1000为4K即1页，-1即不完整，并不是一页，所以加上0x1000-1可以让他达到下一页但是并没有增加4K,可能只差0x1就达到4K，即下下页
#define HHK_PAGE_COUNT              ((sym_val(__init_hhk_end) - 0x100000 + 0x1000 - 1) >> 12)
//因为linker.ld的原因+= 0xc0000000 所以 0x100000 + 0xC0000000 = 0xC0000100
//起始地址变为0xc0000100，所以__kernel_start = 0xc0000100
//所以不能使用减去 0xc0000000 而是应该减去 0xc0000100

// |    |    |+++           没有 +0x1000-1 之前
// |    |    |     |+++     加 0x1000 之后
// |    |    |     |++      加 0x1000-1 之后
//  看似没差别，但是如果是这样就不能直接+0x1000了！
// |    |    |              加 +0x1000-1 之前
// |    |    |     |        加 0x1000 之后
// |    |    |+++++         加 +0x1000-1 之前
// 除以4K之后这样无法得到一个多余的页，这样可以极大程度地减少浪费

#define ONE_MiB_PAGE_COUNT         256  //0x100=256 256个页表项刚好为1MiB 256 * 4KiB = 1MiB

//-----------------------------定义页目录中的索引
// use table #1
#define PG_TABLE_IDENTITY           0

// use table #2-4
// hence the max size of kernel is 8MiB
#define PG_TABLE_KERNEL             1   //内核页表的start地址，后续会有循环递增 直到第四个

// use table #5
#define PG_TABLE_STACK              4   //栈页表的start地址 即 kernel_end的地址

// Provided by linker (see linker.ld)
extern uint8_t __kernel_start;
extern uint8_t __kernel_end;
extern uint8_t __init_hhk_end;
// extern uint8_t _k_stack;


/*-----------分页-----------*/
void _init_page(uint32_t* ptd)
{
    //跳过第0个页表项(即页目录)，将第一个页表项放进ptd 0号索引 (0是第0个页表)
    //ptd开始这一页用来存储页目录 所以跳过
    //设置第0个页目录项，指向第1个页表
    //ptd[0][0] = (ptd[1])的地址
    //ptd+1024U => 第一个页表 SET_PTE默认会向后偏移一个页表
    SET_PDE(ptd, 
            0,  //0x0地址换算PDE为0
            NEW_L1_ENTRY(PG_PREM_RW, ptd + 1024U) // ptd + 1024U ==> PG_TABLE_IDENTITY
            ) // +1024表示跳过第一个页表项，即指向下一个页表
    for (uint32_t i = 0; i < ONE_MiB_PAGE_COUNT + HHK_PAGE_COUNT; i++)
    {
        //映射低1MB的内存地址 和 hhk_init(即boot文件夹下的内容) 所占用的地址
        //即0x0 到 0x100000 + HHK_PAGE_COUNT页数 的地址
        SET_PTE(ptd, 
                PG_TABLE_IDENTITY, //具体页表
                i,  //0x0地址换算PTE为0，所以从i=0开始
                NEW_L2_ENTRY(PG_PREM_RW, i << PG_SIZE_BITS) //从物理地址0x0开始映射
                )//一页4KB
    }

    
    //---- 至此我们仍未将 内核 映射 ----
    //---- 我们所映射的仅仅只是内核代码的0x100000处到 hhk 高半核的内存区域


    //---- 到这里我们才正式开始映射内核！----
    // --- 将内核重映射至高半区 ---
    
    // 这里是一些计算，主要是计算应当映射进的 页目录 与 页表 的条目索引（Entry Index）
    // __kernel_start >= 0xc0100000 >> 22 =0x300 = 768  E [0, 1024)
    uint32_t kernel_pde_index = L1_INDEX(sym_val(__kernel_start));  // 内核代码的页目录索引
    uint32_t kernel_pte_index = L2_INDEX(sym_val(__kernel_start));  // 内核代码的页表索引
    uint32_t kernel_pg_counts = KERNEL_PAGE_COUNT;                  // kernel所占的页数
    
    // 将内核所需要的页表注册进页目录
    //  当然，就现在而言，我们的内核只占用不到50个页（每个页表包含1024个页）
    //  这里分配了3个页表（12MiB），未雨绸缪。
    //PG_TABLE_STACK - PG_TABLE_KERNEL = 3
    //所以 i = 0, 1, 2 刚好 3个 与前面定义符合(看注释)
    for (uint32_t i = 0; i < PG_TABLE_STACK - PG_TABLE_KERNEL; i++)
    {
        // 分三个 页表
        SET_PDE(
            ptd,
            kernel_pde_index + i,   //线性地址 前20位   //之所以选择在kernel_pde_index + i位置开始创建页，是为了避免今后与用户页发生冲突
            NEW_L1_ENTRY(PG_PREM_RW, PT_ADDR(ptd, PG_TABLE_KERNEL + i))//PT_ADDR跳过0号页表，最终获得2+i即二号页表
        )
        //  | 1 | 0  | 0  | 0  | 0  | 0   | 1  |
        //  0   1    2    3    4    5   kernel
        //  A
        //  |_0号索引指向，第0个页表，主要保存页目录索引，也就是页目录表
    }
    
    // 首先，检查内核的大小是否可以fit进我们这几个表（12MiB）
    // (PG_TABLE_STACK - PG_TABLE_KERNEL) * PG_MAX_ENTRIES = 总共可用的页数
    // 总共可用的页数(物理页) = 3 * 1024 = 3072(物理页) 每个物理页大小被分割为4KiB，所以3072 * 4KiB = 12MiB
    if (kernel_pg_counts > (PG_TABLE_STACK - PG_TABLE_KERNEL) * PG_MAX_ENTRIES) {
        // ERROR: require more pages
        //  here should do something else other than head into blocking
        while (1);
    }
    
    // 计算内核.text段的物理地址
    uintptr_t kernel_pm = V2P(&__kernel_start);
    
    // 重映射内核至高半区地址（>=0xC0000000）
    for (uint32_t i = 0; i < kernel_pg_counts; i++)
    {
        // 正式开始映射内核 代码地址 ==> 高半核地址(也就是往高内存地址写入经处理后的原内存地址)
        SET_PTE(
            ptd, 
            PG_TABLE_KERNEL, //SET_PTE会默认偏移一个页表，根据上面SET_PDE设置为2+，所以1开始很合理
            kernel_pte_index + i,   //线性地址 中20位
            NEW_L2_ENTRY(PG_PREM_RW, kernel_pm + (i << PG_SIZE_BITS))   // 具体内核物理地址
        )
    }

    // 最后一个entry用于循环映射
    //PG_MAX_ENTRIES-1为entry
    // 循环映射最后(第1023个)一个页表，页表指向自己(也就是ptd), 他用来记录ptd(也就是页目录)的起始地址
    SET_PDE(
        ptd,
        PG_LAST_TABLE,/* 最后一个页表 1023代表 第1024个entry */
        NEW_L1_ENTRY(T_SELF_REF_PERM, ptd)/*将ptd起始地址。。。。(懵了)*/
    );

    
#pragma region VBE for test

#ifdef __VBE__
    uint32_t VBE_pde_index = L1_INDEX(0xFD000000);  // 内核代码的页目录索引
    uint32_t VBE_pte_index = L2_INDEX(0xFD000000);  // 内核代码的页表索引
    // uint32_t VBE_pg_counts = KERNEL_PAGE_COUNT;                  // kernel所占的页数
    
    SET_PDE(
        ptd,
        VBE_pde_index,                                       // 线性地址 前20位   //之所以选择在kernel_pde_index + i位置开始创建页，是为了避免今后与用户页发生冲突
        NEW_L1_ENTRY(PG_PREM_RW, PT_ADDR(ptd, 10)) // PT_ADDR跳过0号页表，最终获得2+i即二号页表
    )
    for (int i = 0; i < 1000; i++)
    {
        /* code */
        SET_PTE(
            ptd,
            10,
            VBE_pte_index + i,
            NEW_L2_ENTRY(PG_PREM_RW, 0xFD000000 + (i << PG_SIZE_BITS)))
    }
#endif /* __VBE__ */

#pragma endregion
    
}

/*------------multiboot------------*/

// 复制 mmap_addr 到 destination(destination+current) ??
uint32_t __save_subset(uint8_t* destination, uint8_t* base, unsigned int size) {
    unsigned int i = 0;
    for (; i < size; i++)
    {
        *(destination + i) = *(base + i);
    }
    return i;
}

/*
 * 存储multiboot信息到一个固定的位置，防止找不到
 * info信息
 * destination目标起始地址
*/
void 
_save_multiboot_info(multiboot_info_t* info, uint8_t* destination) {

    uint32_t current = 0;
    uint8_t* info_b = (uint8_t*) info;  //指向传给info的地址
    for (; current < sizeof(multiboot_info_t); current++)
    {
        *(destination + current) = *(info_b + current); //在目标起始地址后逐个写入multiboot的info
    }

    //计算destination的mmap_addr即mmap_addr中存储的mmap地址
    //并指向 [destination+current] 后的地址     下一行代码会望这个地址写入mmap_addr
    ((multiboot_info_t*) destination)->mmap_addr = (uintptr_t)destination + current;
    //通过__save_subset函数将 [mmap_addr指向的mmap] 复制到 [destination+current] 后
    //并增加current偏移量
    current += __save_subset(destination + current, (uint8_t*)info->mmap_addr, info->mmap_length);

    //同上，只不过是保存驱动器信息
    if (present(info->flags, MULTIBOOT_INFO_DRIVE_INFO)) {
        ((multiboot_info_t*) destination)->drives_addr = (uintptr_t)destination + current;
        current += __save_subset(destination + current, (uint8_t*)info->drives_addr, info->drives_length);
    }
}

void _hhk_init(uint32_t* ptd, uint32_t kpg_size)
{
    uint8_t * kpg = (uint8_t*)ptd;  //强转，保证每次递增增长1字节，而不是4字节
    for(uint32_t i = 0; i < kpg_size; i++)
    {
        *(kpg + i) = 0;//清空kpg的每一项
    }

    _init_page(ptd);
}
