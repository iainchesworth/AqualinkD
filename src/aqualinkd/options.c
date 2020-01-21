#include "options.h"

#include <assert.h>
#include <getopt.h>
#include <stdio.h>
#include <stdlib.h>
#include "config/config_helpers.h"
#include "hardware/devices/hardware_device_types.h"
#include "logging/logging.h"
#include "utility/utils.h"

#include "../aqualinkd-version.h"

void printHelp()
{
	printf("\n");
	printf("Usage: %s [options]\n", AQUALINKD_NAME);
	printf("\n");
	printf("A daemon to control Jandy Aqualink RS pool equipment from any home automation hub or web browser.\n");
	printf("\n");
	printf("Options:\n");
	printf("\n");
	printf("\t-c, --config-file <file>            Daemon configuration file (default: %s)\n", CFG_ConfigFile());
	printf("\t-D, --debug                         Enable debug level logging\n");
	printf("\t-h, --help                          Print usage information\n");
	printf("\t-i, --id <id>                       Sets the RS485 device id (default: %02x)\n", (int)(CFG_DeviceId().Type & CFG_DeviceId().Instance));
	printf("\t    --insecure                      Disable support for SSL for web connections (not recommended)\n");
	printf("\t    --log-file <file>               Daemon log file (default: %s)\n", CFG_LogFile());
	printf("\t-n, --no-daemonize                  Prevent %s from running as a daemon\n", AQUALINKD_NAME);
	printf("\t-p, --pid-file <file>               Path to use for daemon PID file (default: %s)\n", CFG_PidFile());
	printf("\t    --playback-mode <file>          Playback recorded serial data from file (default: %s)\n", CFG_RawSerial_LogFile());
	printf("\t    --record-mode <file>            Record raw serial data direct to file (default: %s)\n", CFG_RawSerial_LogFile());
	printf("\t-s, --serial-port <file>            Serial port/device to connect with (default: %s)\n", CFG_SerialPort());
	printf("\t    --trace                         Enable trace level logging\n");
	printf("\t-w, --web-root <dir>                Sets the location of the web asserts (default: %s)\n", CFG_WebDirectory());
	printf("\t-v, --version                       Print version information and quit\n");
	printf("\n");
}

void printVersion()
{
	printf("\n");
	printf("Version: %s\n", AQUALINKD_VERSION);
	printf("Built: %s\n", __TIMESTAMP__);
}

void printIncompatibleModesSpecified()
{
	printf("\n");
	printf("Cannot use \"--playback-mode\" with \"--record-mode\" together.  Options are mutually exclusive.\n");
}

// For long options that don't have a corresponding short option, the flag should 
// be a value thats not part of the optstring.
//
// If one chooses a value higher than 255 then it can't possibly be a short option.

enum aqualink_option_flags
{
	OPTION_FLAG_CONFIG_FILE = 'c',
	OPTION_FLAG_DEBUG = 'D',
	OPTION_FLAG_HELP = 'h',
	OPTION_FLAG_ID = 'i',
	OPTION_FLAG_NO_DAEMONIZE = 'n',
	OPTION_FLAG_PID_FILE = 'p',
	OPTION_FLAG_SERIAL_DEVICE = 's',
	OPTION_FLAG_VERSION = 'v',
	OPTION_FLAG_WEBROOT = 'w',
	
	// Long options without a corresponding short option.
	OPTION_FLAG_INSECURE = 0x100,
	OPTION_FLAG_LOG_FILE = 0x101,
	OPTION_FLAG_PLAYBACK_MODE = 0x102,
	OPTION_FLAG_RECORD_MODE = 0x103,
	OPTION_FLAG_TRACE = 0x104
};

void handleOptions(int argc, char* argv[])
{
	static const struct option aqualink_long_options[] =
	{
		{ "config-file",			required_argument,	0, OPTION_FLAG_CONFIG_FILE },
		{ "debug",					no_argument,		0, OPTION_FLAG_DEBUG },
		{ "help",					no_argument,		0, OPTION_FLAG_HELP },
		{ "id",                     required_argument,  0, OPTION_FLAG_ID },
		{ "insecure",				no_argument,		0, OPTION_FLAG_INSECURE },
		{ "log-file",				required_argument,	0, OPTION_FLAG_LOG_FILE },
		{ "no-daemonize",			no_argument,		0, OPTION_FLAG_NO_DAEMONIZE },
		{ "pid-file",				required_argument,	0, OPTION_FLAG_PID_FILE },
		{ "playback-mode",			required_argument,	0, OPTION_FLAG_PLAYBACK_MODE },
		{ "record-mode",			required_argument,	0, OPTION_FLAG_RECORD_MODE },
		{ "serial-port",			required_argument,	0, OPTION_FLAG_SERIAL_DEVICE },
		{ "trace",					no_argument,		0, OPTION_FLAG_TRACE },
		{ "version",				no_argument,		0, OPTION_FLAG_VERSION },
		{ "web-root",				required_argument,  0, OPTION_FLAG_WEBROOT },

		{0, 0, 0, 0}
	};

	static char* aqualink_short_options = "c:Dhi:np:s:vw:";

	int ch = 0;

	while ((ch = getopt_long(argc, argv, aqualink_short_options, aqualink_long_options, 0)) != -1)
	{
		// check to see if a single character or long option came through
		switch (ch)
		{
		case OPTION_FLAG_CONFIG_FILE: // short option 'c' / long option "config-file"
			CFG_Set_ConfigFile(optarg);
			break;

		case OPTION_FLAG_DEBUG: // short option 'D' / long option "debug"
			CFG_Set_LogLevel(Debug);
			set_verbosity(&aqualink_default_logger, Debug);
			break;

		case OPTION_FLAG_ID: // short option 'i' / long option "id"
			{
				HardwareDeviceId deviceId;
				deviceId.Type = (HardwareDeviceTypes)(atoi(optarg) & 0xFC);
				deviceId.Instance = (HardwareDeviceInstanceTypes)(atoi(optarg) & 0x03);
				CFG_Set_DeviceId(deviceId);
			}
			break;

		case OPTION_FLAG_INSECURE: // long option "insecure"
			CFG_Set_Insecure(true);
			break;

		case OPTION_FLAG_LOG_FILE: // long option "log-file"
			CFG_Set_LogFile(optarg);
			break;

		case OPTION_FLAG_NO_DAEMONIZE: // short option 'n' / long option "no-daemonize"
			CFG_Set_NoDaemonize(true);
			break;

		case OPTION_FLAG_PID_FILE: // short option 'p' / long option "pid-file"
			CFG_Set_PidFile(optarg);
			break;

		case OPTION_FLAG_PLAYBACK_MODE: // long option "playback-mode"
			if (CFG_RecordMode())
			{
				printIncompatibleModesSpecified();
				exit(EXIT_FAILURE);
			}
			else 
			{
				CFG_Set_PlaybackMode(true);
				CFG_Set_RawSerial_LogFile(optarg);
			}
			break;

		case OPTION_FLAG_RECORD_MODE: // long option "record-mode"
			if (CFG_PlaybackMode())
			{
				printIncompatibleModesSpecified();
				exit(EXIT_FAILURE);
			}
			else
			{
				CFG_Set_RecordMode(true);
				CFG_Set_RawSerial_LogFile(optarg);
			}
			break;

		case OPTION_FLAG_SERIAL_DEVICE: // short option 's' / long option "serial-port"
			CFG_Set_SerialPort(optarg);
			break;

		case OPTION_FLAG_TRACE: // long option "trace"
			CFG_Set_LogLevel(Trace);
			set_verbosity(&aqualink_default_logger, Trace);
			break;

		case OPTION_FLAG_VERSION: // short option 'v' / long option "version"
			printVersion();
			exit(EXIT_SUCCESS);

		case OPTION_FLAG_WEBROOT: // short option 'w' / long option "web-root"
			CFG_Set_WebDirectory(optarg);
			break;

		case OPTION_FLAG_HELP: // short option 'h' / long option "help"
		default:  // any unknown options
			printHelp();
			exit(EXIT_SUCCESS);
		}
	}
}
