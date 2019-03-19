
#include "libcmini.h"


void *memchr(const void *src, int c, size_t n)
{
	const unsigned char *to = (const unsigned char *)src;

	while(n && (*(unsigned char *)to != (unsigned char)c))
	{
		n--;
		to = (unsigned char *)to + 1;
	}

	return (n ? (void *)to : NULL);
}

