#include <kernel/printk.h>
#include <stdio.h>
#include <stdarg.h>
#include <stdint.h>
#include <stddef.h>
#include <limits.h>
#include <string.h>

/* Helper to print data */
static int print_data(const char* data, size_t length)
{
	const unsigned char* bytes = (const unsigned char*) data;

	for (size_t i = 0; i < length; i++)
		if (putchar(bytes[i]) == EOF)
			return -1;

	return (int)length;
}

/* Helper to print a single character */
static int print_char(char c)
{
	if (putchar(c) == EOF)
		return -1;
	return 1;
}

/* Helper to print a string */
static int print_string(const char *str)
{
	size_t len = strlen(str);
	return print_data(str, len);
}

/* Helper to print an integer */
static int print_int(int value)
{
	char representation[11];
	size_t len = 0;
	char sign;
	unsigned int abs_value;

	if (value < 0) {
		sign = '-';
		/* Handle INT_MIN correctly by converting to unsigned */
		abs_value = (unsigned int)(-(value + 1)) + 1;
	}
	else {
		sign = '\0';
		abs_value = (unsigned int)value;
	}

	/* Handle zero case explicitly */
	if (abs_value == 0) {
		representation[sizeof(representation) - len++ - 1] = '0';
	} else {
		while (abs_value != 0) {
			representation[sizeof(representation) - len++ - 1] = '0' + abs_value % 10;
			abs_value /= 10;
		}
	}

	if (sign != '\0')
		representation[sizeof(representation) - len++ - 1] = sign;

	return print_data(representation + sizeof(representation) - len, len);
}

/* Helper to print an unsigned integer */
static int print_uint(unsigned int value)
{
	char representation[11];
	size_t len = 0;

	/* Handle zero case explicitly */
	if (value == 0) {
		representation[sizeof(representation) - len++ - 1] = '0';
	} else {
		while (value != 0) {
			representation[sizeof(representation) - len++ - 1] = '0' + value % 10;
			value /= 10;
		}
	}

	return print_data(representation + sizeof(representation) - len, len);
}

/* Helper to print a pointer in hexadecimal */
static int print_pointer(void *ptr)
{
	const char hex_chars[] = "0123456789abcdef";
	char buffer[11]; /* "0x" + 8 hex digits + '\0' */
	uint32_t value = (uint32_t)ptr;
	int i;

	buffer[0] = '0';
	buffer[1] = 'x';
	
	for (i = 9; i >= 2; i--) {
		buffer[i] = hex_chars[value & 0xF];
		value >>= 4;
	}
	buffer[10] = '\0';

	return print_string(buffer);
}

/* printk: kernel-specific printf function
 * Simplified printf implementation for kernel logging
 * Supports %c, %s, %d, %i, %u, %p, %% format specifiers
 */
int printk(const char* restrict fmt, ...)
{
	va_list args;
	int written = 0;
	const char *p;

	va_start(args, fmt);

	for (p = fmt; *p != '\0'; p++) {
		if (*p != '%') {
			if (print_char(*p) < 0) {
				written = -1;
				break;
			}
			written++;
		} else {
			p++; /* Skip '%' */
			if (*p == '\0')
				break;

			switch (*p) {
				case 'c': {
					char c = (char) va_arg(args, int);
					int ret = print_char(c);
					if (ret < 0) {
						written = -1;
						goto end;
					}
					written += ret;
					break;
				}
				case 's': {
					const char *s = va_arg(args, const char*);
					int ret;
					/* Handle NULL pointer by printing "(null)" */
					if (s == NULL) {
						ret = print_string("(null)");
					} else {
						ret = print_string(s);
					}
					if (ret < 0) {
						written = -1;
						goto end;
					}
					written += ret;
					break;
				}
				case 'd':
				case 'i': {
					int i = va_arg(args, int);
					int ret = print_int(i);
					if (ret < 0) {
						written = -1;
						goto end;
					}
					written += ret;
					break;
				}
				case 'u': {
					unsigned int u = va_arg(args, unsigned int);
					int ret = print_uint(u);
					if (ret < 0) {
						written = -1;
						goto end;
					}
					written += ret;
					break;
				}
				case 'p': {
					void *p_val = va_arg(args, void*);
					int ret = print_pointer(p_val);
					if (ret < 0) {
						written = -1;
						goto end;
					}
					written += ret;
					break;
				}
				case '%': {
					if (print_char('%') < 0) {
						written = -1;
						goto end;
					}
					written++;
					break;
				}
				default:
					/* Unknown format specifier, just print it */
					if (print_char('%') < 0 || print_char(*p) < 0) {
						written = -1;
						goto end;
					}
					written += 2;
					break;
			}
		}
	}

end:
	va_end(args);
	return written;
}

/* Helper function to print a 32-bit value in hexadecimal */
static int print_hex(uint32_t value)
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

	return print_string(buffer);
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
	
	printk("Stack size: %u bytes\n", (unsigned int)((uint32_t)&stack_top - (uint32_t)&stack_bottom));
	printk("Stack used: %u bytes\n", (unsigned int)((uint32_t)&stack_top - esp));
	
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
