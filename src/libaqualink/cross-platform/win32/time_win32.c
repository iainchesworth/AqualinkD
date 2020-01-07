#include "../time.h"

#if defined (WIN32)

#include <time.h>

char* strptime(const char* s, const char* format, struct tm* tm)
{
	return 0;
}

#endif // defined (WIN32)
