.section .text
    .global hhk_entry_
    hhk_entry_:
        subl    $16, %esp
        
        // 加载GDT
        call    _init_gdt
        movl    $_gdt, 2(%esp)
        movw    _gdt_limit, %ax
        movw    %ax, (%esp)
        lgdt    (%esp)

            // 更新 段寄存器
        movw    $0x10, %cx
        movw    %cx, %ds
        movw    %cx, %es
        movw    %cx, %fs
        movw    %cx, %gs
        movw    %cx, %ss
        
            // 更新 CS:EIP指针
        pushw    $0x08
        pushl    $_after_gdt
        retf

    _after_gdt:
    /**
      * @ TODO: 实现 IDT
    */

        // 加载IDT  目前没写中断向量表
        //call     _init_idt
        //subl    $6, %esp    //预留6字节空间给IDT
        //movl    _idt, 2(%esp)
        //movw    _idt_limit, %ax
        //movw    %ax, (%esp)
        //lidt    (%esp)
        //addl    $6, %esp    //释放6字节空间 (%esp)=2字节 2(%esp)=4字节

        //kernel init
        call    init

/*

retf(远返回)指令用于从当前的 CS:EIP 指针跳转到另一个 CS:EIP 指针
这里没有指定索引值，因此默认从堆栈中弹出两个值：先是 EIP，然后是 CS
这意味着处理器将从堆栈中弹出 _after_gdt 标签的地址作为新的 EIP 值，并弹出 0x08 作为新的 CS 值。处理器将使用新的 CS:EIP 继续执行指令

retf(远返回) 弹出堆栈中的两个值，第一个默认被设置为EIP，第二个默认被设置为CS

*/



        cli
    j_:
        hlt
        jmp j_