#include "thread_utils.h"

#include <signal.h>
#include <stdatomic.h>
#include <stdbool.h>

#include "cross-platform/threads.h"
#include "logging/logging.h"
#include "utility/utils.h"

static cnd_t termination_cv;
static mtx_t termination_mtx;

static _Atomic bool termination_inited = false;
static _Atomic bool aqualink_is_stopping = false;

bool initialise_termination_handler()
{
	if (termination_inited)
	{
		DEBUG("Multiple calls attempting to initialise termination handler");
	}
	else if (thrd_success != cnd_init(&termination_cv))
	{
		DEBUG("Failed while initiialising termination condition variable");
	}
	else if (thrd_success != mtx_init(&termination_mtx, mtx_plain))
	{
		DEBUG("Failed while initiialising termination mutex");
	}
	else
	{
		termination_inited = true;
	}

	return termination_inited;
}

void trigger_application_termination()
{
	INFO("Terminating application");
	cnd_broadcast(&termination_cv);
}

void termination_handler(int signum)
{
	INFO("Received signal %d...terminating", signum);
	cnd_broadcast(&termination_cv);
}

bool test_for_termination()
{
	return aqualink_is_stopping;
}

bool wait_for_termination()
{
	bool ret = false;

	// Block the calling thread until the termination handler condition variable is set.
	if ((termination_inited) && (thrd_success == cnd_wait(&termination_cv, &termination_mtx)))
	{
		aqualink_is_stopping = true;
		ret = true;
	}
	else
	{
		DEBUG("Attempted to wait for termination before termination handler was init'ed");
	}

	return ret;
}

void cleanup_termination_handler()
{
	DEBUG("Cleaning up termination condition variable");
	cnd_destroy(&termination_cv);
}
