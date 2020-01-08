#include "../daemon.h"

#if defined (WIN32)

#include "logging/logging.h"
#include "version/version.h"

bool daemonize(main_function_t main_function)
{
	NOTICE("On Windows...  Running %s as interactive process (rather than daemon)", AQUALINKD_NAME);
	return main_function();
}

#endif // defined (WIN32)
