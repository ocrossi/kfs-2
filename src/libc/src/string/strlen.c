#include <string.h>

size_t	strlen(const char *str)
{
	const char *const start = str;

	while (*str++ != '\0');

	return str - start;
}
