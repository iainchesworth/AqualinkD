#include "../threads.h"

#if defined (WIN32)

#if !defined(__STDC_NO_THREADS__) || __STDC_NO_THREADS__

#pragma comment(lib, "tinycthread.lib")

#else // !defined(__STDC_NO_THREADS__) || __STDC_NO_THREADS__

// Nothing here due to C11 support for <thread.h>

#endif // !defined(__STDC_NO_THREADS__) || __STDC_NO_THREADS__

#endif // defined (WIN32)
