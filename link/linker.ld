ENTRY(start_)

SECTIONS {
    
    . = 0x100000;
    
    .hhk_init_text BLOCK(4K) :
    {
        * (.multiboot)
        build/obj/boot/*.o (.hhk_init)
        build/obj/boot/*.o (.text)
    }

    .hhk_init_bss BLOCK(4K) :
    {
        build/obj/boot/*.o (.bss)
    }
    
    .hhk_init_data BLOCK(4K) :
    {
        build/obj/boot/*.o (.data)
    }

    .hhk_init_rodata BLOCK(4K) :
    {
        build/obj/boot/*.o (.rodata)
    }

    . += 0xC0000000;

    .text BLOCK(4K) : AT ( ADDR(.text) - 0xC0000000)
    {
        __kernel_start = .;
        build/obj/kernel/*.o (.text)
    }
    
    .bss BLOCK(4K) : AT ( ADDR(.bss) - 0xC0000000)
    {
        build/obj/kernel/*.o (.bss)
    }

    .data BLOCK(4K) : AT ( ADDR(.data) - 0xC0000000)
    {
        build/obj/kernel/*.o (.data)
    }
    
    .rodata BLOCK(4K) : AT ( ADDR(.rodata) - 0xC0000000)
    {
        build/obj/kernel/*.o (.rodata)
    }
    
    .kpg BLOCK(4K) : AT ( ADDR(.kpg) - 0xC0000000)
    {
    	build/obj/boot/*.o (.kpg)
        build/obj/kernel/*.o (.kpg)
    }

    __kernel_end = ALIGN(4K);
    __kernel_heap_start = ALIGN(4K);

}
