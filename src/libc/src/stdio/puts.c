#include <stdio.h>
#include <string.h>
#if defined(__is_libk)
#include <kernel/terminal.h>
#endif

int puts(const char *string)
{
#if defined(__is_libk)
	return terminal_write(string, strlen(string));
#else
	// TODO: Implement stdio and write syscall
	return EOF;
#endif
}
