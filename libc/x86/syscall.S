.section .text
.global _x86_syscall_no_args
_x86_syscall_no_args:
    movl 4(%esp), %eax
    int $0x80
    ret

.global _x86_syscall_one_arg
_x86_syscall_one_arg:
    pushl %ebx // abi 
    movl 16(%esp), %ebx
    movl 8(%esp), %eax
    int $0x80
    popl %ebx
    ret

.global _x86_syscall_two_args
_x86_syscall_two_args:
    pushl %ebx // abi 
    movl 16(%esp), %ecx
    movl 12(%esp), %ebx
    movl 8(%esp), %eax
    int $0x80
    popl %ebx
    ret

.global _x86_syscall_three_args
_x86_syscall_three_args:
    pushl %ebx // abi 
    movl 20(%esp), %edx
    movl 16(%esp), %ecx
    movl 12(%esp), %ebx
    movl 8(%esp), %eax
    int $0x80
    popl %ebx
    ret
