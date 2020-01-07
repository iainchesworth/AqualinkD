#include "string_utils.h"

#include <ctype.h>
#include <string.h>

int aq_stricmp(const char* first, const char* second)
{
	while (*first && *second)
	{
		if (toupper(*first) != toupper(*second))
		{
			break;
		}
		first++;
		second++;
	}
	return toupper(*first) == toupper(*second);
}

int aq_strnicmp(const char* first, const char* second, size_t max)
{
	while (*first && *second && max)
	{
		if (toupper(*first) != toupper(*second))
		{
			break;
		}

		max--;
		first++;
		second++;
	}

	if (0 == max)
	{
		return 1;  /* they are equal already */
	}

	return toupper(*first) == toupper(*second); /* depends on the maxth character is equality */
}

char* aq_stristr(const char* haystack, const char* needle)
{
	do
	{
		const char* h = haystack;
		const char* n = needle;
		while (tolower((unsigned char)*h) == tolower((unsigned char)*n) && *n)
		{
			h++;
			n++;
		}
		if (*n == 0)
		{
			return (char*)haystack;
		}
	} while (*haystack++);

	return 0;
}
