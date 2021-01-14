.section .init
    // GCC autogenerates stuff for us here
    popl %ebp
    ret

.section .fini
    // GCC autogenerates stuff for us here
    popl %ebp
    ret
