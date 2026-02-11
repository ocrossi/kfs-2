# KFS-2: Building a Bare-Metal Kernel from Scratch

Welcome, students! This document will guide you through understanding and building a minimal operating system kernel from scratch. By the end of this guide, you will understand how to compile, link, and boot your own kernel using GRUB on an i386 (x86 32-bit) architecture.

---

## Table of Contents

1. [Project Overview](#project-overview)
2. [Main Components](#main-components)
3. [Build System and Docker Workflow](#build-system-and-docker-workflow)
4. [The Bootable Section: Deep Dive into GRUB](#the-bootable-section-deep-dive-into-grub)
5. [The Multiboot Specification](#the-multiboot-specification)
6. [The Boot Assembly Code (`boot.S`)](#the-boot-assembly-code-boots)
7. [The Linker Script](#the-linker-script)
8. [Running the Kernel](#running-the-kernel)
9. [Adding the Global Descriptor Table (GDT)](#adding-the-global-descriptor-table-gdt)

---

## Project Overview

This project, **kfs-2** (Kernel From Scratch), is an educational bare-metal kernel targeting the **i686-elf** (32-bit x86) platform. It demonstrates:

- How to set up a cross-compiler toolchain (GCC, Binutils, GDB)
- How to write assembly code that interfaces with the GRUB bootloader
- How to structure a minimal C kernel with VGA text-mode output
- How to create a bootable ISO image using GRUB

The kernel boots using the **Multiboot** specification, which allows GRUB to load the kernel into memory and transfer control to it in 32-bit protected mode.

> **Note on Naming**: The repository is named `kfs-2` while the generated files use `kfs-1` naming (e.g., `kfs-1.iso`, `kfs-1.bin`). This is the original project configuration and is intentional.

---

## Main Components

The project is organized into several key components:

```
kfs-2/
├── Dockerfile           # Docker container for reproducible builds
├── Makefile             # Top-level build orchestration
├── dist/                # Output directory for the bootable ISO
└── src/
    ├── build.sh         # Main build script
    ├── build_iso.sh     # ISO creation script using grub-mkrescue
    ├── build_toolchain.sh  # Cross-compiler toolchain setup
    ├── config.sh        # Build configuration variables
    ├── install_dependencies.sh  # System dependencies installer
    ├── install_headers.sh       # Header installation script
    ├── common/          # Helper scripts (host detection, arch mapping)
    ├── kernel/          # THE KERNEL (main focus)
    │   ├── Makefile
    │   ├── include/     # Kernel headers (terminal.h, vga.h, kernel.h)
    │   └── src/
    │       ├── kernel/kernel.c       # Main kernel entry point
    │       └── arch/i386/            # Architecture-specific code
    │           ├── boot.S            # Multiboot header & entry point
    │           ├── terminal.c        # VGA text terminal driver
    │           ├── linker.ld         # Linker script
    │           ├── crti.S / crtn.S   # C runtime initialization
    │           └── make.config
    ├── libc/            # Minimal C library implementation
    │   ├── Makefile
    │   ├── include/     # Standard headers (stdio.h, string.h, etc.)
    │   └── src/         # Implementation (printf, memcpy, etc.)
    └── sysroot/         # System root for the cross-compilation
        └── boot/grub/grub.cfg  # GRUB configuration file
```

### 1. **Kernel** (`src/kernel/`)

The kernel is the heart of the operating system. It contains:

- **`boot.S`**: Assembly code that defines the Multiboot header and kernel entry point
- **`kernel.c`**: The `kernel_main()` function, where your C code begins
- **`terminal.c`**: VGA text mode driver for displaying output
- **`linker.ld`**: Linker script that defines memory layout

### 2. **LibC** (`src/libc/`)

A minimal freestanding C library (`libk`) providing basic functions:
- `printf()`, `putchar()`, `puts()` for output
- `memcpy()`, `memmove()`, `memset()`, `memcmp()`, `strlen()` for memory/string operations
- `abort()` for halting execution

### 3. **Toolchain** (`src/build_toolchain.sh`)

A cross-compiler is essential because we're building code for a different target (i686-elf) than our host machine. The toolchain includes:

- **Binutils 2.41**: Assembler (`as`), linker (`ld`), and other binary tools
- **GCC 13.2.0**: C/C++ compiler
- **GDB 14.1**: Debugger

All tools are prefixed with `i686-elf-` (e.g., `i686-elf-gcc`).

---

## Build System and Docker Workflow

### Why Docker?

Building a cross-compiler and kernel requires specific tools and versions. Docker ensures a **reproducible build environment** across different machines.

### Build Flow

```
┌─────────────────────────────────────────────────────────────────┐
│                         make all                                 │
└────────────────────────────┬────────────────────────────────────┘
                             │
                             ▼
┌─────────────────────────────────────────────────────────────────┐
│   1. Build Docker Image (kfs-1)                                  │
│      • Install dependencies (build-essential, grub, xorriso)     │
│      • Build cross-compiler toolchain (binutils, gcc, gdb)       │
└────────────────────────────┬────────────────────────────────────┘
                             │
                             ▼
┌─────────────────────────────────────────────────────────────────┐
│   2. Run Container: ./build_iso.sh                               │
│      • Run ./build.sh (compile kernel & libc)                    │
│      • Run grub-mkrescue to create bootable ISO                  │
└────────────────────────────┬────────────────────────────────────┘
                             │
                             ▼
┌─────────────────────────────────────────────────────────────────┐
│   3. Copy ISO to dist/kfs-1.iso                                  │
└─────────────────────────────────────────────────────────────────┘
```

### Building the Project

```bash
# Build the kernel ISO (this will take time on first run)
make all

# Run the kernel in QEMU
make qemu-run

# Check shell scripts for issues
make check-scripts

# View all available make targets
make help
```

### The `build.sh` Script

```bash
#!/bin/sh
set -e

. ./install_headers.sh    # Install headers to sysroot

for PROJECT in $PROJECTS  # PROJECTS="libc kernel"
do
  (cd "$PROJECT" && DESTDIR="$SYSROOT" $MAKE install)
done
```

This script:
1. Installs header files from `libc` and `kernel` to the sysroot
2. Builds `libc` (produces `libk.a`)
3. Builds the kernel binary (`kfs-1.bin`)

---

## The Bootable Section: Deep Dive into GRUB

### What is GRUB?

**GRUB (Grand Unified Bootloader)** is a widely-used bootloader that:

1. Loads at system startup from the BIOS/UEFI
2. Presents a menu to select an operating system
3. Loads the selected kernel into memory
4. Transfers control to the kernel

### How GRUB Finds and Loads Our Kernel

#### 1. The GRUB Configuration (`sysroot/boot/grub/grub.cfg`)

```
menuentry "kfs-1" {
    multiboot /boot/kfs-1.bin
}
```

This configuration tells GRUB:
- Create a menu entry named "kfs-1"
- Use the **multiboot** protocol to load `/boot/kfs-1.bin`

#### 2. The `grub-mkrescue` Command

The `build_iso.sh` script uses:

```bash
grub-mkrescue -v -o "$iso_dist_dir/$iso_name" "$iso_content_dir"
```

This command:
- Takes the `sysroot/` directory as input
- Creates a bootable **El Torito** ISO image
- Embeds GRUB itself into the ISO
- Makes the ISO bootable on both BIOS and UEFI systems

#### 3. The Boot Process (Step by Step)

```
┌──────────────────────────────────────────────────────────────────┐
│ BIOS/UEFI                                                        │
│   • POST (Power-On Self-Test)                                    │
│   • Locates boot device (CD-ROM with our ISO)                    │
│   • Loads GRUB's first stage bootloader                          │
└─────────────────────────────┬────────────────────────────────────┘
                              │
                              ▼
┌──────────────────────────────────────────────────────────────────┐
│ GRUB Stage 1                                                     │
│   • Loads GRUB Stage 2 from the ISO filesystem                   │
└─────────────────────────────┬────────────────────────────────────┘
                              │
                              ▼
┌──────────────────────────────────────────────────────────────────┐
│ GRUB Stage 2                                                     │
│   • Reads grub.cfg                                               │
│   • Displays boot menu                                           │
│   • Searches for Multiboot header in kfs-1.bin                   │
│   • Loads kernel at specified memory address                     │
│   • Sets up 32-bit protected mode                                │
│   • Disables interrupts                                          │
│   • Jumps to kernel entry point (_start)                         │
└─────────────────────────────┬────────────────────────────────────┘
                              │
                              ▼
┌──────────────────────────────────────────────────────────────────┐
│ Our Kernel (_start in boot.S)                                    │
│   • Sets up the stack                                            │
│   • Calls _init (global constructors)                            │
│   • Calls kernel_main()                                          │
│   • Halts the CPU                                                │
└──────────────────────────────────────────────────────────────────┘
```

---

## The Multiboot Specification

The **Multiboot Specification** is a standard interface between bootloaders and operating systems. GRUB implements this specification.

### Why Multiboot?

- **Standardization**: Any Multiboot-compliant bootloader can load any Multiboot-compliant kernel
- **Simplified kernel development**: The bootloader handles hardware initialization
- **Information passing**: The bootloader provides memory maps, boot device info, etc.

### The Multiboot Header

The kernel must contain a **Multiboot header** within the first **8 KiB** of the kernel file. Here's how it's defined in `boot.S`:

```asm
/* Constants for the multiboot header */
.set ALIGN,    1<<0             /* Align modules on page boundaries */
.set MEMINFO,  1<<1             /* Provide memory map */
.set FLAGS,    ALIGN | MEMINFO  /* Combined flags */
.set MAGIC,    0x1BADB002       /* Magic number (required) */
.set CHECKSUM, -(MAGIC + FLAGS) /* Checksum must make sum = 0 */

.section .multiboot
.align 4
.long MAGIC
.long FLAGS
.long CHECKSUM
```

### Multiboot Header Fields

| Field      | Value        | Purpose                                      |
|------------|--------------|----------------------------------------------|
| MAGIC      | `0x1BADB002` | Identifies this as a Multiboot kernel        |
| FLAGS      | `0x00000003` | Requests page-aligned modules + memory info  |
| CHECKSUM   | `-(MAGIC+FLAGS)` | Must make `MAGIC + FLAGS + CHECKSUM = 0` |

### Machine State After GRUB Loads the Kernel

When GRUB jumps to `_start`, the CPU is in the following state:

| Register/State | Value/State                    |
|----------------|--------------------------------|
| Mode           | 32-bit protected mode          |
| A20 Gate       | Enabled                        |
| Paging         | Disabled                       |
| Interrupts     | Disabled                       |
| EAX            | `0x2BADB002` (Multiboot magic) |
| EBX            | Pointer to Multiboot info struct |
| ESP            | Undefined (kernel must set it) |

---

## The Boot Assembly Code (`boot.S`)

Let's walk through `src/kernel/src/arch/i386/boot.S`:

### 1. The Multiboot Header Section

```asm
.section .multiboot
.align 4
.long MAGIC
.long FLAGS
.long CHECKSUM
```

- **`.section .multiboot`**: Creates a special section for the header
- **`.align 4`**: Ensures 32-bit (4-byte) alignment
- The three `.long` values form the required Multiboot header

### 2. The Stack Setup

```asm
.section .bss
.align 16
stack_bottom:
.skip 16384    # Reserve 16 KiB for the stack
stack_top:
```

- **`.bss` section**: Contains uninitialized data (doesn't increase file size)
- **16-byte alignment**: Required by the System V ABI for x86
- Stack grows downward, so we start at `stack_top`

### 3. The Entry Point

```asm
.section .text
.global _start
.type _start, @function
_start:
    mov $stack_top, %esp    # Set up the stack pointer

    call _init              # Call global constructors
    call kernel_main        # Call C kernel entry point

    cli                     # Disable interrupts
1:  hlt                     # Halt the CPU
    jmp 1b                  # Loop forever if we wake up
```

Key points:
- **`mov $stack_top, %esp`**: Essential! Without a stack, we can't call C functions
- **`call _init`**: Runs C++ global constructors and GCC initialization code
- **`call kernel_main`**: Transfers control to your C code
- **`cli; hlt; jmp 1b`**: Safely halts the system when kernel_main returns

---

## The Linker Script

The linker script (`src/kernel/src/arch/i386/linker.ld`) tells the linker how to arrange code and data in memory.

```ld
ENTRY(_start)             /* Entry point of the kernel */

SECTIONS
{
    . = 2M;               /* Load kernel at 2 MiB physical address */

    .text BLOCK(4K) : ALIGN(4K)
    {
        *(.multiboot)     /* Multiboot header MUST come first */
        *(.text)          /* Code section */
    }

    .rodata BLOCK(4K) : ALIGN(4K)
    {
        *(.rodata)        /* Read-only data (strings, constants) */
    }

    .data BLOCK(4K) : ALIGN(4K)
    {
        *(.data)          /* Initialized global variables */
    }

    .bss BLOCK(4K) : ALIGN(4K)
    {
        *(COMMON)
        *(.bss)           /* Uninitialized data + stack */
    }
}
```

### Why 2 MiB?

- Traditional kernels loaded at 1 MiB, but UEFI systems may use that memory
- 2 MiB is a safer choice for modern systems
- The Multiboot header is in the `.text` section, which starts at 2 MiB

### Why 4K Alignment?

- Prepares for paging: x86 pages are 4 KiB
- Allows setting different permissions per section (read, write, execute)

---

## Running the Kernel

### Using QEMU

```bash
# Build and run
make qemu-run

# Or manually:
qemu-system-i386 -cdrom dist/kfs-1.iso
```

### What You Should See

The kernel initializes the VGA terminal and prints numbers 0 through 42 to the screen.

---

## Adding the Global Descriptor Table (GDT)

The **Global Descriptor Table (GDT)** is a critical x86 data structure that defines memory segments and their properties. While GRUB sets up a basic GDT for us, implementing our own gives us full control.

### What is the GDT?

The GDT is an array of **segment descriptors** that define:
- **Base address**: Where the segment starts in memory
- **Limit**: The size of the segment
- **Access rights**: Read, write, execute permissions
- **Privilege level**: Ring 0 (kernel) to Ring 3 (user)

### Why Do We Need It?

1. **Protected mode requirement**: x86 protected mode requires a GDT
2. **Memory protection**: Separate kernel and user space memory
3. **Task switching**: Different segments for different tasks

### Implementation Steps

#### 1. Create GDT Header (`src/kernel/include/kernel/gdt.h`)

```c
#ifndef _KERNEL_GDT_H
#define _KERNEL_GDT_H

#include <stdint.h>

/* GDT segment descriptor structure (8 bytes) */
struct gdt_entry {
    uint16_t limit_low;     /* Limit bits 0-15 */
    uint16_t base_low;      /* Base bits 0-15 */
    uint8_t  base_middle;   /* Base bits 16-23 */
    uint8_t  access;        /* Access flags */
    uint8_t  granularity;   /* Granularity + limit bits 16-19 */
    uint8_t  base_high;     /* Base bits 24-31 */
} __attribute__((packed));

/* GDT pointer structure (used by lgdt instruction) */
struct gdt_ptr {
    uint16_t limit;         /* Size of GDT - 1 */
    uint32_t base;          /* Address of first GDT entry */
} __attribute__((packed));

/* Access byte flags */
#define GDT_ACCESS_PRESENT     0x80  /* Segment is present */
#define GDT_ACCESS_RING0       0x00  /* Kernel privilege level */
#define GDT_ACCESS_RING3       0x60  /* User privilege level */
#define GDT_ACCESS_EXECUTABLE  0x08  /* Code segment */
#define GDT_ACCESS_DIRECTION   0x04  /* Direction/Conforming bit */
#define GDT_ACCESS_READWRITE   0x02  /* Readable (code) / Writable (data) */
#define GDT_ACCESS_ACCESSED    0x01  /* CPU sets this when segment is accessed */

/* Descriptor type bit (must be 1 for code/data segments) */
#define GDT_ACCESS_DESCRIPTOR  0x10  /* S bit: 1 = code/data, 0 = system */

/* Combined access byte values for common segment types */
#define GDT_ACCESS_CODE_SEGMENT  0x9A  /* Present + Ring0 + Descriptor + Exec + Read */
#define GDT_ACCESS_DATA_SEGMENT  0x92  /* Present + Ring0 + Descriptor + Write */

/* Granularity byte flags */
#define GDT_GRAN_4K            0x80  /* Limit is in 4K blocks */
#define GDT_GRAN_32BIT         0x40  /* 32-bit protected mode */
#define GDT_GRAN_16BIT         0x00  /* 16-bit protected mode */

/* Initialize the GDT */
void gdt_init(void);

#endif /* _KERNEL_GDT_H */
```

#### 2. Create GDT Implementation (`src/kernel/src/arch/i386/gdt.c`)

```c
#include <kernel/gdt.h>

/* We need 3 entries: null, kernel code, kernel data */
#define GDT_ENTRIES 3

static struct gdt_entry gdt[GDT_ENTRIES];
static struct gdt_ptr   gp;

/* External assembly function to load the GDT */
extern void gdt_flush(uint32_t);

/* Helper function to set a GDT entry */
static void gdt_set_entry(int num, uint32_t base, uint32_t limit,
                          uint8_t access, uint8_t gran)
{
    /* Set base address */
    gdt[num].base_low    = (base & 0xFFFF);
    gdt[num].base_middle = (base >> 16) & 0xFF;
    gdt[num].base_high   = (base >> 24) & 0xFF;

    /* Set limit */
    gdt[num].limit_low   = (limit & 0xFFFF);
    gdt[num].granularity = (limit >> 16) & 0x0F;

    /* Set granularity and access flags */
    gdt[num].granularity |= (gran & 0xF0);
    gdt[num].access       = access;
}

void gdt_init(void)
{
    /* Set up the GDT pointer */
    gp.limit = (sizeof(struct gdt_entry) * GDT_ENTRIES) - 1;
    gp.base  = (uint32_t)&gdt;

    /* Entry 0: Null descriptor (required by x86) */
    gdt_set_entry(0, 0, 0, 0, 0);

    /* Entry 1: Kernel Code Segment
     * Base: 0x00000000, Limit: 0xFFFFFFFF (4GB)
     * Access: 0x9A = Present + Ring 0 + Descriptor + Executable + Readable
     * Granularity: 4K blocks, 32-bit mode */
    gdt_set_entry(1, 0, 0xFFFFFFFF,
                  GDT_ACCESS_CODE_SEGMENT,
                  GDT_GRAN_4K | GDT_GRAN_32BIT);

    /* Entry 2: Kernel Data Segment
     * Base: 0x00000000, Limit: 0xFFFFFFFF (4GB)
     * Access: 0x92 = Present + Ring 0 + Descriptor + Writable
     * Granularity: 4K blocks, 32-bit mode */
    gdt_set_entry(2, 0, 0xFFFFFFFF,
                  GDT_ACCESS_DATA_SEGMENT,
                  GDT_GRAN_4K | GDT_GRAN_32BIT);

    /* Load the GDT */
    gdt_flush((uint32_t)&gp);
}
```

#### 3. Create GDT Assembly (`src/kernel/src/arch/i386/gdt_flush.S`)

```asm
.global gdt_flush
.type gdt_flush, @function

gdt_flush:
    /* Load the GDT pointer passed as argument */
    mov 4(%esp), %eax       /* Get pointer to gdt_ptr from stack */
    lgdt (%eax)             /* Load the GDT */

    /* Reload segment registers with new GDT selectors
     * Selector format: index * 8 + RPL (Request Privilege Level)
     * Kernel code segment: 1 * 8 = 0x08
     * Kernel data segment: 2 * 8 = 0x10 */
    mov $0x10, %ax          /* Data segment selector */
    mov %ax, %ds
    mov %ax, %es
    mov %ax, %fs
    mov %ax, %gs
    mov %ax, %ss

    /* Far jump to reload CS register
     * We can't directly mov to CS, must use far jump */
    ljmp $0x08, $.flush     /* Jump to code segment selector 0x08 */

.flush:
    ret
```

#### 4. Update Kernel Makefile

Add to `src/kernel/src/arch/i386/make.config`:

```makefile
KERNEL_ARCH_OBJS=\
$(ARCHOBJDIR)/boot.o \
$(ARCHOBJDIR)/terminal.o \
$(ARCHOBJDIR)/gdt.o \
$(ARCHOBJDIR)/gdt_flush.o \
```

#### 5. Update `boot.S` to Call GDT Initialization

Modify the `_start` function:

```asm
_start:
    mov $stack_top, %esp    /* Set up stack */

    /* Initialize the GDT before anything else */
    call gdt_init

    call _init              /* Global constructors */
    call kernel_main        /* Enter C kernel */

    cli
1:  hlt
    jmp 1b
```

Or, alternatively call `gdt_init()` at the beginning of `kernel_main()` in C.

#### 6. Update `kernel.c`

```c
#include <kernel.h>
#include <kernel/terminal.h>
#include <kernel/gdt.h>
#include <stdio.h>

void kernel_main(void)
{
    /* Initialize GDT first (or do it in boot.S) */
    gdt_init();

    terminal_initialize();
    printf("GDT initialized successfully!\n");

    /* ... rest of kernel code ... */
}
```

### Understanding the GDT Structure

```
┌───────────────────────────────────────────────────────────────────┐
│                    GDT Entry (8 bytes)                            │
├───────────────────────────────────────────────────────────────────┤
│ Byte 7 │ Byte 6 │ Byte 5 │ Byte 4 │ Byte 3-2 │ Byte 1-0          │
│ Base   │ Gran+  │ Access │ Base   │ Base     │ Limit             │
│ 31:24  │ Limit  │ Byte   │ 23:16  │ 15:0     │ 15:0              │
│        │ 19:16  │        │        │          │                   │
└───────────────────────────────────────────────────────────────────┘

Access Byte:
┌─────┬─────┬─────┬─────┬─────┬─────┬─────┬─────┐
│  P  │ DPL │ DPL │  S  │  E  │ DC  │ RW  │  A  │
│     │ (1) │ (0) │     │     │     │     │     │
└─────┴─────┴─────┴─────┴─────┴─────┴─────┴─────┘
P   = Present bit (must be 1 for valid segments)
DPL = Descriptor Privilege Level (0=kernel, 3=user)
S   = Descriptor type (1=code/data, 0=system)
E   = Executable bit (1=code, 0=data)
DC  = Direction/Conforming bit
RW  = Readable/Writable bit
A   = Accessed bit (set by CPU)

Granularity Byte:
┌─────┬─────┬─────┬─────┬─────┬─────┬─────┬─────┐
│  G  │ DB  │  L  │ AVL │       Limit 19:16      │
└─────┴─────┴─────┴─────┴─────┴─────┴─────┴─────┘
G   = Granularity (0=byte, 1=4KB pages)
DB  = Size (0=16-bit, 1=32-bit)
L   = Long mode (0 for 32-bit)
AVL = Available for system use
```

### Segment Selectors

When loading segment registers (CS, DS, ES, etc.), you use **segment selectors**, not direct GDT indices:

```
Selector = (GDT_index * 8) | RPL

Where:
- GDT_index: Entry number in the GDT (0, 1, 2, ...)
- RPL: Requested Privilege Level (0-3)

For our GDT:
- Null: 0 * 8 = 0x00
- Kernel Code: 1 * 8 = 0x08
- Kernel Data: 2 * 8 = 0x10
```

### What Happens When You Load the GDT

1. **`lgdt` instruction**: Loads the GDT register with the address and size of your GDT
2. **Reload data segments**: Immediately reload DS, ES, FS, GS, SS with the new data segment selector
3. **Far jump**: Required to reload CS because you can't `mov` to CS directly
4. **Protection active**: CPU now uses your GDT for all memory accesses

---

## Next Steps After GDT

Once you have a working GDT, you can proceed to:

1. **Interrupt Descriptor Table (IDT)**: Handle CPU exceptions and hardware interrupts
2. **Programmable Interrupt Controller (PIC)**: Configure hardware interrupt routing
3. **Keyboard driver**: Read input from the keyboard
4. **Memory management**: Implement paging and a physical memory allocator
5. **User mode**: Create user space segments (Ring 3) and switch between kernel/user mode

---

## Additional Resources

- [OSDev Wiki - GDT Tutorial](https://wiki.osdev.org/GDT_Tutorial)
- [OSDev Wiki - Bare Bones](https://wiki.osdev.org/Bare_Bones)
- [Multiboot Specification](https://www.gnu.org/software/grub/manual/multiboot/multiboot.html)
- [Intel Software Developer's Manual Vol. 3](https://www.intel.com/content/www/us/en/developer/articles/technical/intel-sdm.html)

---

## Summary

In this guide, you learned:

1. **Project structure**: How the kernel, libc, and toolchain are organized
2. **Build process**: Docker-based reproducible builds with cross-compilation
3. **GRUB bootloading**: How `grub-mkrescue` creates a bootable ISO and how GRUB loads the kernel
4. **Multiboot**: The standard interface between bootloader and kernel
5. **Boot assembly**: How `boot.S` sets up the stack and transfers to C
6. **Linker script**: Memory layout for the kernel binary
7. **GDT implementation**: The theory and practice of setting up your own GDT

Happy kernel hacking! 🔧
