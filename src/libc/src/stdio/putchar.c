#include <stdio.h>
#if defined(__is_libk)
#include <kernel/terminal.h>
#endif

int putchar(int i_c)
{
	const char	c = (char)i_c;
	int			ret = i_c;

#if defined(__is_libk)
	ret = terminal_write(&c, sizeof(c));
	if (ret == -1)
		ret = EOF;
#else
	(void)c;
	// TODO: Implement stdio and the write syscall
	ret = EOF;
#endif

	return i_c;
}
