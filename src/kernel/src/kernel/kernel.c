#include <kernel.h>
#include <kernel/terminal.h>
#include <kernel/gdt.h>
#include <kernel/printk.h>
#include <stdio.h>

static void test_terminal(void)
{
	int		i = 0;

	while (i <= 42)
		printf("%d\n", i++);
}

void kernel_main(void)
{
	terminal_initialize();
	gdt_install();
	
	/* Test printk functionality */
	printk("KFS-2 Kernel initialized\n");
	printk("GDT installed at address 0x00000800\n");
	
	/* Print kernel stack information */
	print_kernel_stack();
	
	test_terminal();
}
