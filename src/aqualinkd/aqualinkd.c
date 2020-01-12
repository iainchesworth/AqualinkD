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
#include "aqualink.h"

#include <stdarg.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include "config/config.h"
#include "config/config_helpers.h"
#include "cross-platform/daemon.h"
#include "cross-platform/signals.h"
#include "cross-platform/threads.h"
#include "logging/logging.h"
#include "logging/logging_sink_basic_file.h"
#include "logging/logging_utils.h"
#include "hardware/controllers/rs_controller.h"
#include "hardware/simulators/pda_simulator.h"
#include "hardware/simulators/rs_keypadsimulator.h"
#include "serial/aq_serial_threaded.h"
#include "string/string_utils.h"
#include "threads/thread_utils.h"
#include "version/version.h"
#include "web/aq_web.h"

#include "options.h"
#include "utils.h"

static bool main_loop()
{
	thrd_t serial_worker_thread, webserver_worker_thread;
	bool ran_successfully = false;

	// 1. Initialise configuration parameters and global data sets.
	if (!initialise_termination_handler())
	{
		ERROR("Failed to create worker synchronisation primitives");
	}

	// 2. Create various "server" threads.
	else if (thrd_success != thrd_create(&serial_worker_thread, &serial_thread, (void*)&wait_for_termination))
	{
		ERROR("Failed to start serial worker thread");
	}
	else if (thrd_success != thrd_create(&webserver_worker_thread, &webserver_thread, (void*)&wait_for_termination))
	{
		ERROR("Failed to start web worker thread");
	}
	else
	{
		INFO("Serial worker thread is running");

		// 3. Go!
		configure_termination_signals();

		INFO("Running AqualinkD...");
		if (!wait_for_termination())
		{
			ERROR("Failed when attempting to block-wait for termination handler");
		}
		else
		{
			// 4. Clean up shop and terminate worker threads.
			INFO("Cleaning up and closing down");
			cleanup_termination_handler();

			ran_successfully = true;
		}
	}

	return ran_successfully;
}

int main(int argc, char* argv[])
{
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

	// Finally, process any settings from the configuration file.
	handle_configuration_file_options();

	// Do all the configuration goodness...
	{
		set_verbosity(&aqualink_default_logger, CFG_LogLevel());

		if (0 == CFG_LogFile())
		{
			// Configure the output file logger by configuring the userdata and registering the sink.  Note that this
			// will implicitly initialise the sink.

			LoggingSinkBasicFileUserData* aqualink_basic_file_user_data = (LoggingSinkBasicFileUserData*)malloc(sizeof(LoggingSinkBasicFileUserData));
			aqualink_basic_file_user_data->Filename = CFG_ConfigFile();

			aqualink_default_logger_sink_file.UserData = aqualink_basic_file_user_data;
			register_logging_sink(&aqualink_default_logger.Sinks, &aqualink_default_logger_sink_file);
		}
	}

	// Initialise the master controller (and turn on the simulator).
	rs_controller_initialise(RS8);
	rs_controller_enable_rs6_simulator();
	rs_controller_enable_pda_simulator();
	rs_controller_enable_onetouch_simulator();

	INFO("%s %s", AQUALINKD_NAME, AQUALINKD_VERSION);

	NOTICE("Config level             = %s", logging_level_to_string(CFG_LogLevel()));
	NOTICE("Config socket_port       = %d", CFG_SocketPort());
	NOTICE("Config serial_port       = %s", CFG_SerialPort());
	NOTICE("Config web_directory     = %s", CFG_WebDirectory());
	NOTICE("Config insecure          = %s", bool2text(CFG_Insecure()));
	NOTICE("Config device_id         = 0x%02hhx", CFG_DeviceId());
	NOTICE("Config read_all_devices  = %s", bool2text(CFG_ReadAllDevices()));
	NOTICE("Config use_aux_labels    = %s", bool2text(CFG_UsePanelAuxLabels()));
	NOTICE("Config override frz prot = %s", bool2text(CFG_OverrideFreezeProtect()));

#ifndef MG_DISABLE_MQTT
	NOTICE("Config mqtt_server       = %s", CFG_MqttServer());
	NOTICE("Config mqtt_dz_sub_topic = %s", CFG_MqttDzSubTopic());
	NOTICE("Config mqtt_dz_pub_topic = %s", CFG_MqttDzPubTopic());
	NOTICE("Config mqtt_aq_topic     = %s", CFG_MqttAqTopic());
	NOTICE("Config mqtt_user         = %s", CFG_MqttUser());
	NOTICE("Config mqtt_passwd       = %s", CFG_MqttPassword());
	NOTICE("Config mqtt_ID           = %s", CFG_MqttId());
	NOTICE("Config idx water temp    = %d", CFG_DzIdxAirTemp());
	NOTICE("Config idx pool temp     = %d", CFG_DzIdxPoolWaterTemp());
	NOTICE("Config idx spa temp      = %d", CFG_DzIdxSpaWaterTemp());
	NOTICE("Config idx SWG Percent   = %d", CFG_DzIdxSwgPercent());
	NOTICE("Config idx SWG PPM       = %d", CFG_DzIdxSwgPpm());
	NOTICE("Config PDA Mode          = %s", bool2text(CFG_PdaMode()));
	NOTICE("Config PDA Sleep Mode    = %s", bool2text(CFG_PdaSleepMode()));
	NOTICE("Config force SWG         = %s", bool2text(CFG_ForceSwg()));
#endif // MG_DISABLE_MQTT

	NOTICE("Config no_daemonize      = %s", bool2text(CFG_NoDaemonize()));
	NOTICE("Config file              = %s", CFG_ConfigFile());
	NOTICE("Config light_pgm_mode    = %.2f", CFG_LightProgrammingMode());
	NOTICE("Log file                 = %s", CFG_LogFile());
	NOTICE("Debug RS485 protocol     = %s", bool2text(CFG_DebugRsProtocolPackets()));
	NOTICE("Read Pentair Packets     = %s", bool2text(CFG_ReadPentairPackets()));
	NOTICE("Display warnings in web  = %s", bool2text(CFG_DisplayWarningsWeb()));

	if (CFG_SwgZeroIgnore() > 0)
	{
		NOTICE("Ignore SWG 0 msg count   = %d", CFG_SwgZeroIgnore());
	}

	int button_index, pump_index;

	for (button_index = FilterPump; button_index < ButtonTypeCount; ++button_index)
	{
		/*
		NOTICE("Config BTN %-13s = label %-15s | VSP ID %-4s  | dzidx %d | %s", 
			aqualink_master_controller.Buttons[button_index].name,
			aqualink_master_controller.Buttons[button_index].label,
			"None",
			aqualink_master_controller.Buttons[button_index].dz_idx,
			(button_index > 0 && (button_index == CFG_LightProgrammingButtonPool() || button_index == CFG_LightProgrammingButtonSpa()) ? "Programmable" : ""));
			*/
	}

	if (CFG_NoDaemonize() == true)
	{
		NOTICE("Running %s interactively...", AQUALINKD_NAME);

		if (!main_loop())
		{
			ERROR("Failed to start %s interactively...exiting", AQUALINKD_NAME);
			return EXIT_FAILURE;
		}
	}
	else
	{
		NOTICE("Running %s as a daemon...", AQUALINKD_NAME);

		if (!daemonize(main_loop))
		{
			ERROR("Failed to start %s as a daemon...exiting", AQUALINKD_NAME);
			return EXIT_FAILURE;
		}
	}

	// Shutdown and destroy the simulators and RS controller
	rs_controller_disable_pda_simulator();
	rs_controller_disable_rs6_simulator();
	rs_controller_destroy();

	shutdown_logging(&aqualink_default_logger);

	return EXIT_SUCCESS;
}
