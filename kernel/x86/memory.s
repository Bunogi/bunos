.section .text
.global _x86_refresh_page_directory
_x86_refresh_page_directory:
    //abi: eax does not need preserving
    movl 4(%esp), %eax // arg 1: address of page directory
    movl %eax, %cr3
    ret
