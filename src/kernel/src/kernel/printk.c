#include <kernel/printk.h>
#include <stdio.h>
#include <stdarg.h>
#include <stdint.h>

/* printk: kernel-specific printf function
 * This is essentially an alias to printf but provides
 * a kernel-specific namespace for logging
 */
int printk(const char* restrict fmt, ...)
{
	va_list args;
	int ret;

	va_start(args, fmt);
	/* We reuse the existing printf implementation */
	ret = printf(fmt, args);
	va_end(args);

	return ret;
}

/* Helper function to print a 32-bit value in hexadecimal */
static void print_hex(uint32_t value)
{
	const char hex_chars[] = "0123456789ABCDEF";
	char buffer[11];
	int i;

	buffer[0] = '0';
	buffer[1] = 'x';
	
	for (i = 9; i >= 2; i--) {
		buffer[i] = hex_chars[value & 0xF];
		value >>= 4;
	}
	buffer[10] = '\0';

	printk("%s", buffer);
}

/* Print kernel stack information
 * This function prints the current stack pointer and stack contents
 * in a human-friendly format
 */
void print_kernel_stack(void)
{
	uint32_t esp;
	uint32_t ebp;
	uint32_t *stack_ptr;
	int i;
	extern uint32_t stack_bottom;  /* Defined in boot.S */
	extern uint32_t stack_top;     /* Defined in boot.S */

	/* Get current stack pointer (ESP) and base pointer (EBP) */
	__asm__ volatile("movl %%esp, %0" : "=r"(esp));
	__asm__ volatile("movl %%ebp, %0" : "=r"(ebp));

	printk("\n=== Kernel Stack Information ===\n");
	printk("Stack Bottom (lowest address):  ");
	print_hex((uint32_t)&stack_bottom);
	printk("\n");
	
	printk("Stack Top (highest address):    ");
	print_hex((uint32_t)&stack_top);
	printk("\n");
	
	printk("Current ESP (Stack Pointer):    ");
	print_hex(esp);
	printk("\n");
	
	printk("Current EBP (Base Pointer):     ");
	print_hex(ebp);
	printk("\n");
	
	printk("Stack size: %d bytes\n", (uint32_t)&stack_top - (uint32_t)&stack_bottom);
	printk("Stack used: %d bytes\n", (uint32_t)&stack_top - esp);
	
	/* Print the top of the stack (most recent values) */
	printk("\n=== Top of Stack (16 values) ===\n");
	stack_ptr = (uint32_t *)esp;
	
	for (i = 0; i < 16 && (uint32_t)stack_ptr < (uint32_t)&stack_top; i++) {
		printk("[ESP+%d]  ", i * 4);
		print_hex((uint32_t)stack_ptr);
		printk(": ");
		print_hex(*stack_ptr);
		printk("\n");
		stack_ptr++;
	}
	
	printk("=================================\n\n");
}
