#ifndef AQ_THREADS_H_
#define AQ_THREADS_H_

#if defined (WIN32)

#if !defined(__STDC_NO_THREADS__) || __STDC_NO_THREADS__

#include <tinycthread.h>

#else // !defined(__STDC_NO_THREADS__) || __STDC_NO_THREADS__

#include <threads.h>

#endif // !defined(__STDC_NO_THREADS__) || __STDC_NO_THREADS__

#else // defined (WIN32)

#include <threads.h>

#endif // defined (WIN32)

#endif // AQ_THREADS_H_
