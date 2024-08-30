.section .text
    .global hhk_entry_
    hhk_entry_:
        subl $16, %esp
        
        
        call init


        addl $16, %esp
        cli
    j_:
        hlt
        jmp j_