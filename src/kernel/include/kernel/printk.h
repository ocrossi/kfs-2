#pragma once

#include <stdarg.h>

/* Kernel-specific printf function */
int printk(const char* restrict fmt, ...);

/* Print kernel stack information */
void print_kernel_stack(void);
