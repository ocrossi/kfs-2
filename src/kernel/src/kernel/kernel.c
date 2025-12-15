#include <kernel.h>
#include <kernel/terminal.h>
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

	test_terminal();
}
