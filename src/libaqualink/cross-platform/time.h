#ifndef AQ_TIME_H_
#define AQ_TIME_H_

#include <time.h>

#if defined (WIN32)

char* strptime(const char* s, const char* format, struct tm* tm);
int nanosleep(const struct timespec* requested_delay, struct timespec* remaining_delay);

#else // defined (WIN32)

// strptime() is available on Linux systems
// nanosleep() is available on Linux systems

#endif // defined (WIN32)

#endif // AQ_TIME_H_
