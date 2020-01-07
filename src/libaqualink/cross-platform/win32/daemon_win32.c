#include "../daemon.h"

#if defined (WIN32)

bool daemonize(main_function_t main_function)
{
	return false;
}

#endif // defined (WIN32)