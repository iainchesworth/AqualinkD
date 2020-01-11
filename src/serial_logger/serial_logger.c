/*
 * Copyright (c) 2017 Shaun Feakes - All rights reserved
 *
 * You may use redistribute and/or modify this code under the terms of
 * the GNU General Public License version 2 as published by the
 * Free Software Foundation. For the terms of this license,
 * see <http://www.gnu.org/licenses/>.
 *
 * You are free to use this software under the terms of the GNU General
 * Public License, but WITHOUT ANY WARRANTY; without even the implied
 * warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
 * See the GNU General Public License for more details.
 *
 *  https://github.com/sfeakes/aqualinkd
 */

#include <fcntl.h>
#include <signal.h>
#include <stdarg.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "config/config.h"
#include "config/config_helpers.h"
#include "cross-platform/signals.h"
#include "cross-platform/threads.h"
#include "hardware/controllers/rs_controller.h"
#include "logging/logging.h"
#include "serial/aq_serial_threaded.h"
#include "threads/thread_utils.h"
#include "version/version.h"

#include "options.h"

int main(int argc, char* argv[]) 
{
	int ran_successfully = EXIT_FAILURE;

	// Log only NOTICE messages and above. Debug and info messages
	// will not be logged to syslog.
	initialize_logging(&aqualink_default_logger);
	set_verbosity(&aqualink_default_logger, Notice);

	// Add the default console sink
	register_logging_sink(&aqualink_default_logger.Sinks, &aqualink_default_logging_sink_console);

	// Initialize the daemon's parameters.
	initialise_config_parameters();

	// Process any options on the command line.
	handleOptions(argc, argv);

	// Initialise the master controller (and turn on the simulator).
	rs_controller_initialise(RS8);

	NOTICE("AqualinkD - Serial Logger v%s", AQUALINKD_VERSION);
	NOTICE("");
	NOTICE("Logging serial information...");
	
	//
	// SERIAL LOGGER MAIN LOOP
	//
	//    - Spawns the serial thread (which runs the "serial processor" that handles logging).
	//    - Waits for a termination signal which causes the main thread and child thread to end.
	//	  - Prints the discovered hardware for the user to see.
	//

	thrd_t serial_worker_thread;
	
	if (!initialise_termination_handler())
	{
		ERROR("Failed to create worker synchronisation primitives");
	}
	else if (thrd_success != thrd_create(&serial_worker_thread, &serial_thread, (void*)&wait_for_termination))
	{
		ERROR("Failed to start serial worker thread");
	}
	else
	{
		configure_termination_signals();

		if (!wait_for_termination())
		{
			ERROR("Failed when attempting to block-wait for termination handler");
		}
		else
		{
			rs_controller_print_detected_devices();
			ran_successfully = EXIT_SUCCESS;
		}
	}

	shutdown_logging(&aqualink_default_logger);

	return ran_successfully;
}
