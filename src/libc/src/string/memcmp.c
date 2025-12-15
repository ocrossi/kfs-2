#include <string.h>

int	memcmp(const void *s1, const void *s2, size_t n)
{
	const unsigned char	*const	u_s1 = s1;
	const unsigned char	*const	u_s2 = s2;
	size_t						i;

	for (i = 0; i < n && u_s1[i] == u_s2[i]; ++i);

	return (i == n) ? 0 : u_s1[i] - u_s2[i];
}
