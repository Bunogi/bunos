.section .init
.global _init
_init:
    push %ebp
    movl %esp, %ebp
    // GCC puts stuff here
    
.section .fini
.global _fini
_fini:
    push %ebp
    movl %esp, %ebp
    // GCC puts stuff here
