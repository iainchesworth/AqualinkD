#ifndef AQ_THREADS_H_
#define AQ_THREADS_H_

#if defined (WIN32)

#include <tinycthread.h>

#else // defined (WIN32)

#include <threads.h>

#endif // defined (WIN32)

#endif // AQ_THREADS_H_
