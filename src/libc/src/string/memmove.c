#include <string.h>

void	*memmove(void *dst, const void *src, size_t n)
{
	if (dst < src)
		while (n-- != 0)
			*(char*)dst++ = *(char*)src++;
	else if (dst != src)
		while (n-- != 0)
			((char*)dst)[n] = ((char*)src)[n];

	return dst;
}
