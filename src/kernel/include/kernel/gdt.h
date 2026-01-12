#pragma once

#include <stdint.h>

/* GDT Segment Selectors */
#define KERNEL_CODE_SEGMENT 0x08
#define KERNEL_DATA_SEGMENT 0x10
#define KERNEL_STACK_SEGMENT 0x18
#define USER_CODE_SEGMENT   0x20
#define USER_DATA_SEGMENT   0x28
#define USER_STACK_SEGMENT  0x30

struct gdt_entry {
    uint16_t limit_low;
    uint16_t base_low;
    uint8_t  base_middle;
    uint8_t  access;
    uint8_t  granularity;
    uint8_t  base_high;
} __attribute__((packed));

struct gdt_ptr {
    uint16_t limit;
    uint32_t base;
} __attribute__((packed));

extern struct gdt_entry gdt[7];

void gdt_install(void);
