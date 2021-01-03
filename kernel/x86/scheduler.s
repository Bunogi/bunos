.section .text
.global _x86_task_switch
_x86_task_switch:
    movl new_esp, %esp
    push new_eip
    ret
