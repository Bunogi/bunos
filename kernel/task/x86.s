.section .text
.global x86_idle
x86_idle:
    sti
    hlt
    //When an interrupt happens, we go to the next instruction, so we have to jump back
    jmp x86_idle


.global x86_syscall_enter
x86_syscall_enter:

    pushl %ebp // target_process argument!
    pushl %edx // arg3
    pushl %ecx // arg2
    pushl %ebx // arg1
    pushl %eax // syscall num

    //stack ends here
    movl $0, %ebp
    cld // ABI: clear DF

    call do_syscall
