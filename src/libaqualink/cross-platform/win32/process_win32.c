#include "../process.h"

#if defined(WIN32)

#include <Windows.h>

int GetPid()
{
	return GetCurrentProcessId();
}

#endif // defined(WIN32)
