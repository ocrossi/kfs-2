#include <string.h>

void	*ft_memset(void *dst, int c, size_t len)
{
	for (size_t i = 0; i < len; ++i)
		((unsigned char*)dst)[i] = c;

	return dst;
}
