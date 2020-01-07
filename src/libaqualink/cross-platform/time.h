#ifndef AQ_TIME_H_
#define AQ_TIME_H_

#if defined (WIN32)

#include <time.h>

char* strptime(const char* s, const char* format, struct tm* tm);

#else // defined (WIN32)

#ifndef _GNU_SOURCE
#define _GNU_SOURCE 1
#endif // _GNU_SOURCE

#ifndef _XOPEN_SOURCE
#define _XOPEN_SOURCE 1
#endif // _XOPEN_SOURCE

#ifndef __USE_XOPEN
#define __USE_XOPEN 1
#endif // __USE_XOPEN

#include <time.h>

// strptime is available on Linux systems

#endif // defined (WIN32)

#endif // AQ_TIME_H_
