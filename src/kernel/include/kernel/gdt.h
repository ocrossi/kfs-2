#pragma once

/* GDT Segment Selectors */
#define KERNEL_CODE_SEGMENT 0x08
#define KERNEL_DATA_SEGMENT 0x10
#define KERNEL_STACK_SEGMENT 0x18
#define USER_CODE_SEGMENT   0x20
#define USER_DATA_SEGMENT   0x28
#define USER_STACK_SEGMENT  0x30

void gdt_install(void);
