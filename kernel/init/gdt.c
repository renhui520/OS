#include <boot/gdt.h>
#include <stdint.h>

//指定 GDT 中描述符的数量
#define GDT_ENTRY 5

uint64_t _gdt[GDT_ENTRY];
uint16_t _gdt_limit = sizeof(_gdt) - 1;

//拼凑 构建 一个全局描述符
void _set_gdt_entry(uint32_t index, uint32_t base, uint32_t limit, uint32_t flags) {
    _gdt[index] = SEG_BASE_H(base) | flags | SEG_LIM_H(limit) | SEG_BASE_M(base);
    _gdt[index] <<= 32;
    _gdt[index] |= SEG_BASE_L(base) | SEG_LIM_L(limit);
}

void
_init_gdt() {
    _set_gdt_entry(0, 0, 0, 0); // 空描述符
    _set_gdt_entry(1, 0, 0xfffff, SEG_R0_CODE);//权限0 内核级权限 代码段
    _set_gdt_entry(2, 0, 0xfffff, SEG_R0_DATA);//权限0 内核级权限 数据段
    _set_gdt_entry(3, 0, 0xfffff, SEG_R3_CODE);//权限3 用户级权限 代码段
    _set_gdt_entry(4, 0, 0xfffff, SEG_R3_DATA);//权限3 用户级权限 数据段
}
