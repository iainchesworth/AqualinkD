#include "string_utils.h"

#include <ctype.h>
#include <string.h>

int strcmpi(const char* first, const char* second)
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

int strcmpni(const char* first, const char* second, size_t max)
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
