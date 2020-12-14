/* Declare constants for the multiboot header. */
.set ALIGN,    1<<0             /* align loaded modules on page boundaries */
.set MEMINFO,  1<<1             /* provide memory map */
.set FLAGS,    ALIGN | MEMINFO  /* this is the Multiboot 'flag' field */
.set MAGIC,    0x1BADB002       /* 'magic number' lets bootloader find the header */
.set CHECKSUM, -(MAGIC + FLAGS) /* checksum of above, to prove we are multiboot */
/* 
Declare a multiboot header that marks the program as a kernel. These are magic
values that are documented in the multiboot standard. The bootloader will
search for this signature in the first 8 KiB of the kernel file, aligned at a
32-bit boundary. The signature is in its own section so the header can be
forced to be within the first 8 KiB of the kernel file.
*/
/*
The linker script specifies _start as the entry point to the kernel and the
bootloader will jump to this position once the kernel has been loaded. It
doesn't make sense to return from this function as the bootloader is gone.
*/
.section .multiboot.data
.long MAGIC
.long FLAGS
.long CHECKSUM

.section .bootstrap_stack, "aw", @nobits
.align 16
stack_bottom:
.skip 32768 # 32K
stack_top:

.section .bss, "aw", @nobits
	.align 4096
boot_page_directory:
	.skip 4096
boot_page_table1:
	.skip 4096

.section .multiboot.text, "ax"
.global _start
.type _start, @function
_start:
	cli

	// subtract 0xC0000000 in order to convert between the virtual address of the boot_page_table
	// and the physical address it is actually stored at.
	// edi: Physical pointer to current page
	// esi: Page pointing to 
	// ecx: Used to loop 1023 times
	movl $(boot_page_table1 - 0xC0000000), %edi
	movl $0, %esi
	movl $1023, %ecx // 1024 entries

.loop_continue:	
	// Skip mapping anything that isn't the kernel (cause page fault when writing beyond the kernel's limits)

	cmpl $(_kernel_start - 0xC0000000), %eax
	jl .next_page
	cmpl $(_kernel_end - 0xC0000000), %esi
	jge .rest_of_setup

	// Map this address as present, writable
	movl %esi, %edx
	orl $0x003, %edx
	movl %edx, (%edi)

.next_page:
	addl $4096, %esi
	addl $4, %edi
	loop .loop_continue
.rest_of_setup:
	// Map VGA memory to the last page for fun. This means that the virtual address is
	// 0xC0000000 + 0x003FF000 = 0xC03FF000 (3FF is 1023 in hex)
	movl $(0x000B8000 | 0x0003), boot_page_table1 - 0xC0000000 + 1023 * 4

	// Use the first boot page table for 0x00000 000
	// Identity map the kernel to prevent a page fault when writing to the page table.
	movl $(boot_page_table1 - 0xC0000000 + 0x003), boot_page_directory - 0xC0000000 + 0
	// Use the first page directory entry(boot_page_table1) for 0xC0000xxx
	// at offset 0xC00 (768 * 4)
	movl $(boot_page_table1 - 0xC0000000 + 0x003), boot_page_directory - 0xC0000000 + 768 * 4

	// Point the cr3 register to the boot_page_directory to set it up
	movl $(boot_page_directory - 0xC0000000), %ecx
	movl %ecx, %cr3

	//Enable paging and write protect.
	movl %cr0, %ecx
	orl $0x80010000, %ecx
	movl %ecx, %cr0

	// Get the address of 4f into ecx
	lea _start_high_half, %ecx
	jmp *%ecx

.section .text
_start_high_half:
	//Don't need this one anymore after jumping to high virtual memory
	movl $0, boot_page_directory + 0

	// Update the page table
	mov %cr3, %ecx
	mov %ecx, %cr3
 
	//Set up the initial stack
	mov $stack_top, %esp
	mov $0, %ebp // To tell stack traces where to stop

	call kernel_main
 
	cli
1:	hlt
	jmp 1b
 
/*
Set the size of the _start symbol to the current location '.' minus its start.
This is useful when debugging or when you implement call tracing.
*/
//.size _start, . - _start

//Page table
