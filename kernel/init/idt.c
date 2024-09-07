#include <boot/idt.h>
#include <kernel/interrupts.h>

#define IDT_ENTRY_SIZE 256

#define IDT_ATTR(dpl)   ((0x70 << 5) | (dpl & 3) << 13 | 1 << 15)

uint64_t _idt[IDT_ENTRY_SIZE];
uint16_t _idt_limit = sizeof(_idt) - 1;

void _set_idt_entry(uint32_t vector, uint16_t seg_selector, void(*isr)(), uint8_t dpl) {
    uintptr_t offset = (uintptr_t)isr;
    _idt[vector] = (offset & 0xFFFF0000) | IDT_ATTR(dpl);
    _idt[vector] <<= 32;
    _idt[vector] |= (seg_selector << 16) | (offset & 0x0000FFFF);
}

void _init_idt()
{
    _set_idt_entry(FAULT_DIVISION_ERROR, 0x08, _asm_isr_0, 0);  //除以0异常 处理
}