#include <stdint.h>

/* GDT Entry Structure */
struct gdt_entry {
    uint16_t limit_low;
    uint16_t base_low;
    uint8_t  base_middle;
    uint8_t  access;
    uint8_t  granularity;
    uint8_t  base_high;
} __attribute__((packed));

/* GDT Pointer Structure */
struct gdt_ptr {
    uint16_t limit;
    uint32_t base;
} __attribute__((packed));

/* GDT with 6 entries: 
 * 0: Null descriptor
 * 1: Kernel Code Segment
 * 2: Kernel Data Segment
 * 3: Kernel Stack (can use same as data)
 * 4: User Code Segment
 * 5: User Data Segment
 * 6: User Stack (can use same as user data)
 */
struct gdt_entry gdt[7];
struct gdt_ptr gp;

/* External assembly function to load GDT */
extern void gdt_flush(uint32_t);

/* Set a GDT entry */
static void gdt_set_gate(int num, uint32_t base, uint32_t limit, uint8_t access, uint8_t gran)
{
    gdt[num].base_low = (base & 0xFFFF);
    gdt[num].base_middle = (base >> 16) & 0xFF;
    gdt[num].base_high = (base >> 24) & 0xFF;

    gdt[num].limit_low = (limit & 0xFFFF);
    gdt[num].granularity = (limit >> 16) & 0x0F;
    
    gdt[num].granularity |= gran & 0xF0;
    gdt[num].access = access;
}

void gdt_install(void)
{
    gp.limit = (sizeof(struct gdt_entry) * 7) - 1;
    gp.base = (uint32_t)&gdt;

    /* Null descriptor */
    gdt_set_gate(0, 0, 0, 0, 0);

    /* Kernel Code Segment:  Base=0, Limit=4GB, Access=0x9A, Granularity=0xCF
     * Access:  Present=1, DPL=00 (ring 0), Type=1010 (code, exec/read)
     * Granularity: 4KB blocks, 32-bit */
    gdt_set_gate(1, 0, 0xFFFFFFFF, 0x9A, 0xCF);

    /* Kernel Data Segment: Base=0, Limit=4GB, Access=0x92, Granularity=0xCF
     * Access: Present=1, DPL=00 (ring 0), Type=0010 (data, read/write)
     * Granularity:  4KB blocks, 32-bit */
    gdt_set_gate(2, 0, 0xFFFFFFFF, 0x92, 0xCF);

    /* Kernel Stack (same as kernel data) */
    gdt_set_gate(3, 0, 0xFFFFFFFF, 0x92, 0xCF);

    /* User Code Segment: Base=0, Limit=4GB, Access=0xFA, Granularity=0xCF
     * Access:  Present=1, DPL=11 (ring 3), Type=1010 (code, exec/read) */
    gdt_set_gate(4, 0, 0xFFFFFFFF, 0xFA, 0xCF);

    /* User Data Segment: Base=0, Limit=4GB, Access=0xF2, Granularity=0xCF
     * Access:  Present=1, DPL=11 (ring 3), Type=0010 (data, read/write) */
    gdt_set_gate(5, 0, 0xFFFFFFFF, 0xF2, 0xCF);

    /* User Stack (same as user data) */
    gdt_set_gate(6, 0, 0xFFFFFFFF, 0xF2, 0xCF);

    /* Load the GDT */
    gdt_flush((uint32_t)&gp);
}
