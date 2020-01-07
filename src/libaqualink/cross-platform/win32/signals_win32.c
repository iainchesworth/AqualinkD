#include "../signals.h"

#if defined (WIN32)

#include <signal.h>

#include "threads/thread_utils.h"

void configure_termination_signals()
{
    signal(SIGINT, termination_handler);
    signal(SIGTERM, termination_handler);
    signal(SIGABRT, termination_handler);
}

#endif // defined WIN32
