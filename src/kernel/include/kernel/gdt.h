#pragma once

#include <stdint.h>

/* GDT Segment Selectors */
#define KERNEL_CODE_SEGMENT 0x08
#define KERNEL_DATA_SEGMENT 0x10
#define KERNEL_STACK_SEGMENT 0x18
#define USER_CODE_SEGMENT   0x20
#define USER_DATA_SEGMENT   0x28
#define USER_STACK_SEGMENT  0x30

/* GDT Entry Structure (forward declaration for external access) */
struct gdt_entry;

/* External reference to the GDT array */
extern struct gdt_entry gdt[7];

void gdt_install(void);
