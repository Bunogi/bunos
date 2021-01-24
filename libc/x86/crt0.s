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

    addl $8, %esp
    
    push %eax
    call exit
    
