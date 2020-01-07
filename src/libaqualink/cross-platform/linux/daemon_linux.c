#include "../daemon.h"

#if !defined (WIN32)

#include <sys/file.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <errno.h>
#include <fcntl.h>
#include <stdio.h>
#include <unistd.h>

#include "config/config_helpers.h"
#include "logging/logging.h"

bool daemonize(main_function_t main_function)
{
	bool daemonised = true;
	pid_t process_id, sid;
	int pid_file, rc;
	FILE* fp;

	if (-1 == (pid_file = open(CFG_PidFile(), O_CREAT | O_RDWR, 0666)))
	{
		///FAILED TO OPEN PIDFILE FOR WRITING
		ERROR("Failed to open PID file (%s)...cannot daemonise", CFG_PidFile());
		daemonised = false;
	}
	else if (-1 == (rc = flock(pid_file, LOCK_EX | LOCK_NB)))
	{
		///FAILED TO ADD AN ADVISATORY LOCK - ANOTHER INSTANCE?
		ERROR("Failed to lock PID file...is another instance is already running?");
		daemonised = false;
	}
	else if (-1 == (process_id = fork()))
	{
		///FAILED TO FORK THE PROCESS
		ERROR("Failed to fork child process - error %d (%s)\n", errno, strerror(errno));
		daemonised = false;
	}
	else if (0 == process_id)
	{
		///CHILD PROCESS --> run the main_loop().

		umask(0);

		if (0 > (sid = setsid()))
		{
			// Return failure
			ERROR("Failed while forking child process - error %d (%s)", errno, strerror(errno));
			daemonised = false;
		}
		else
		{
			// Change the current working directory to root.
			chdir("/");

			// Close stdin. stdout and stderr
			close(STDIN_FILENO);
			close(STDOUT_FILENO);
			close(STDERR_FILENO);

			// This is the first instance
			daemonised = main_function();
		}
	}
	else if (0 == (fp = fopen(CFG_PidFile(), "w")))
	{
		ERROR("Can't open PID file (%s) to write process id (%d)", CFG_PidFile(), process_id);
		daemonised = false;
	}
	else if (0 > fprintf(fp, "%d", process_id))
	{
		ERROR("Can't write process id (%d) to PID file (%s)", process_id, CFG_PidFile());
		daemonised = false;

		// Close the FILE pointer to the PID file (as it was openned successfully)
		fclose(fp);
	}
	else
	{
		DEBUG("Child process id: %d ", process_id);

		// Close the FILE pointer to the PID file (as it was openned successfully)
		fclose(fp);
	}

	return daemonised;
}

#endif // !defined (WIN32)
