#include "thread_utils.h"

#include <signal.h>
#include <stdbool.h>
#include <threads.h>
#include "utils.h"

static cnd_t termination_cv;
static mtx_t termination_mtx;
static bool termination_inited = false;

bool initialise_termination_handler()
{
	if (termination_inited)
	{
		logMessage(LOG_DEBUG, "Thread_Utils.c | initialise_termination_handler | Multiple calls attempting to initialise termination handler\n");
	}
	else if (thrd_success != cnd_init(&termination_cv))
	{
		logMessage(LOG_DEBUG, "Thread_Utils.c | initialise_termination_handler | Failed while initiialising termination condition variable\n");
	}
	else if (thrd_success != mtx_init(&termination_mtx, mtx_plain))
	{
		logMessage(LOG_DEBUG, "Thread_Utils.c | initialise_termination_handler | Failed while initiialising termination mutex\n");
	}
	else
	{
		termination_inited = true;
	}

	return termination_inited;
}

void termination_handler(int signum)
{
	logMessage(LOG_INFO, "Aqualink.c | termination_handler() | Received signal %d...terminating\n", signum);
	cnd_broadcast(&termination_cv);
}

bool wait_for_termination()
{
	bool ret = false;

	// Block the calling thread until the termination handler condition variable is set.
	if ((termination_inited) && (thrd_success == cnd_wait(&termination_cv, &termination_mtx)))
	{
		ret = true;
	}
	else
	{
		logMessage(LOG_DEBUG, "Thread_Utils.c | wait_for_termination | Attempted to wait for termination before termination handler was init'ed\n");
	}

	return ret;
}

void cleanup_termination_handler()
{
	logMessage(LOG_DEBUG, "Thread_Utils.c | cleanup_termination_handler | Cleaning up termination condition variable\n");
	cnd_destroy(&termination_cv);
}
