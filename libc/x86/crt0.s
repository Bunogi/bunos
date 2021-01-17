.section .text
.global _start
_start: 
    // Root stack frame
    movl $0, %ebp
    pushl %ebp
    pushl %ebp
    movl %esp, %ebp

    //FIXME: Init standard library here

    call _init

    //FIXME: Real values for argc and argv
    pushl $0
    pushl $0

    call main

    //FIXME: Call exit instead of this madness
1:  
    hlt
    jmp 1b

    //FIXME: Is this right?
    call _fini
    
