
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
#include "hardware/aqualink_master_controller.h"
#include "hardware/controllers/rs_controller.h"
#include "logging/logging.h"
#include "serial/aq_serial_threaded.h"
#include "threads/thread_utils.h"

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
	initialise_aqualinkrs_controller(&aqualink_master_controller, RS8);

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
			ran_successfully = EXIT_SUCCESS;
		}
	}

	shutdown_logging(&aqualink_default_logger);

	return ran_successfully;
}

#ifdef OLD

// printf("AqualinkD %s\n", VERSION);

/*
if (argc < 2 || access(argv[1], F_OK) == -1) {
	fprintf(stderr, "ERROR, first param must be valid serial port, ie:-\n\t%s /dev/ttyUSB0\n\n", argv[0]);
	fprintf(stderr, "Optional parameters are -d (debug) & -p <number> (log # packets) & -i <ID> & -r (raw) ie:=\n\t%s /dev/ttyUSB0 -d -p 1000 -i 0x08\n\n", argv[0]);
	return 1;

	///FIXME support : -p <number> (log # packets) & -i <ID> & -r (raw)



	}
	*/



	if (_playback_file) {
		rs_fd = open(argv[1], O_RDONLY | O_NOCTTY | O_NONBLOCK | O_NDELAY);
		if (rs_fd < 0) {
			ERROR("Unable to open file: %s\n", argv[1]);
			displayLastSystemError(argv[1]);
			return -1;
		}
	}
	else {
		rs_fd = init_serial_port(argv[1]);
	}

	signal(SIGINT, intHandler);
	signal(SIGTERM, intHandler);

	NOTICE("Logging serial information!");
	if (logLevel < LOG_DEBUG)
		printf("Please wait.");

	while (_keepRunning == true) {
		if (rs_fd < 0) {
			ERROR("ERROR, serial port disconnect");
		}

		//packet_length = get_packet(rs_fd, packet_buffer);
		packet_length = get_packet(rs_fd, packet_buffer);

		if (packet_length == -1) {
			// Unrecoverable read error. Force an attempt to reconnect.
			ERROR("ERROR, on serial port");
			_keepRunning = false;
		}
		else if (packet_length == 0) {
			// Nothing read
		}
		else if (packet_length > 0) {

			//TRACE("Received Packet for ID 0x%02hhx of type %s", packet_buffer[PKT_DEST], get_packet_type(packet_buffer, packet_length));
			if (logLevel > LOG_NOTICE)
				printPacket(lastID, packet_buffer, packet_length);

			if (getProtocolType(packet_buffer) == PENTAIR) {
				found = false;
				for (i = 0; i <= pent_sindex; i++) {
					if (pent_slog[i].ID == packet_buffer[PEN_PKT_FROM]) {
						found = true;
						break;
					}
				}
				if (found == false) {
					pent_slog[pent_sindex].ID = packet_buffer[PEN_PKT_FROM];
					pent_slog[pent_sindex].inuse = true;
					pent_sindex++;
				}
			}
			else {
				if (packet_buffer[PKT_DEST] != DEV_MASTER) {
					found = false;
					for (i = 0; i <= sindex; i++) {
						if (slog[i].ID == packet_buffer[PKT_DEST]) {
							found = true;
							break;
						}
					}
					if (found != true && sindex < SLOG_MAX) {
						slog[sindex].ID = packet_buffer[PKT_DEST];
						slog[sindex].inuse = false;
						sindex++;
					}
				}

				if (packet_buffer[PKT_DEST] == DEV_MASTER /*&& packet_buffer[PKT_CMD] == CMD_ACK*/) {
					//NOTICE("ID is in use 0x%02hhx %x", lastID, lastID);
					for (i = 0; i <= sindex; i++) {
						if (slog[i].ID == lastID) {
							slog[i].inuse = true;
							break;
						}
					}
				}

				lastID = packet_buffer[PKT_DEST];
			}
			received_packets++;
		}

		if (logPackets != 0 && received_packets >= logPackets) {
			_keepRunning = false;
		}
		if (logLevel < LOG_DEBUG)
			advance_cursor();

		//sleep(1);
	}

	DEBUG("\n");
	if (logLevel < LOG_DEBUG)
		printf("\n");

	if (sindex >= SLOG_MAX)
		ERROR("Ran out of storage, some ID's were not captured, please increase SLOG_MAX and recompile");
	NOTICE("Jandy ID's found");
	for (i = 0; i < sindex; i++) {
		//NOTICE("ID 0x%02hhx is %s %s", slog[i].ID, (slog[i].inuse == true) ? "in use" : "not used",
		//           (slog[i].inuse == false && canUse(slog[i].ID) == true)? " <-- can use for Aqualinkd" : "");
		if (logLevel >= LOG_DEBUG || slog[i].inuse == true || canUse(slog[i].ID) == true) {
			NOTICE("ID 0x%02hhx is %s %s", slog[i].ID, (slog[i].inuse == true) ? "in use" : "not used",
				(slog[i].inuse == false) ? canUseExtended(slog[i].ID) : getDevice(slog[i].ID));
		}
	}

	if (pent_sindex > 0) {
		NOTICE("\n");
		NOTICE("Pentair ID's found");
	}
	for (i = 0; i < pent_sindex; i++) {
		NOTICE("ID 0x%02hhx is %s %s", pent_slog[i].ID, (pent_slog[i].inuse == true) ? "in use" : "not used",
			(pent_slog[i].inuse == false) ? canUseExtended(pent_slog[i].ID) : getPentairDevice(pent_slog[i].ID));
	}

	NOTICE("\n\n");

#endif // OLD