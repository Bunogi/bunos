.section .text
.global _x86_do_syscall
_x86_do_syscall:
    push %ebp
    mov %esp, %ebp

    // abi: Don't have to preserve ecx or eax
    pushl %ebx

    movl 12(%esp), %eax // arg count
    movl 16(%esp), %ebx // argument pointer
    movl 20(%esp), %ecx // code

    cmpl $1, %eax
    je single_arg
    // ...etc

    // Should never get here
    call abort

    //syscall abi:
    //Return value of the syscall is always returned in eax.
    //If the code is negative, an error occurred.
    
single_arg:
    movl %ecx, %eax
    movl (%ebx), %ebx
    int $0x80

    jmp restore

restore:    
    popl %ebx
    leave
    ret
