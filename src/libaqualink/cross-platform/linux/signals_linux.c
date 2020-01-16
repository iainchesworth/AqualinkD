#include "../signals.h"

#if !defined (WIN32)

#include <signal.h>

#include "logging/logging.h"
#include "threads/thread_utils.h"

void configure_termination_signals()
{
	struct sigaction new_action, old_action;

	new_action.sa_handler = termination_handler;
	sigemptyset(&new_action.sa_mask);
	new_action.sa_flags = 0;

	sigaction(SIGINT, 0, &old_action);
	if (SIG_IGN != old_action.sa_handler) { sigaction(SIGINT, &new_action, 0); }
	sigaction(SIGHUP, 0, &old_action);
	if (SIG_IGN != old_action.sa_handler) { sigaction(SIGHUP, &new_action, 0); }
	sigaction(SIGTERM, 0, &old_action);
	if (SIG_IGN != old_action.sa_handler) { sigaction(SIGTERM, &new_action, 0); }
}

#endif // !defined WIN32
