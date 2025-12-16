.section .text
.global gdt_flush
.type gdt_flush, @function

gdt_flush:
    movl 4(%esp), %eax    # Get the pointer to the GDT descriptor
    lgdt (%eax)           # Load the GDT

    # Reload segment registers
    movw $0x10, %ax       # 0x10 is the offset to kernel data segment
    movw %ax, %ds
    movw %ax, %es
    movw %ax, %fs
    movw %ax, %gs
    movw %ax, %ss

    # Far jump to reload CS (code segment)
    ljmp $0x08, $.flush   # 0x08 is the offset to kernel code segment
.flush:
    ret
