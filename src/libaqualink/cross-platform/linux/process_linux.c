#include "../process.h"

#if !defined(WIN32)

#include <sys/types.h>
#include <unistd.h>

int GetPid()
{
	return getpid();
}

#endif // !defined(WIN32)
