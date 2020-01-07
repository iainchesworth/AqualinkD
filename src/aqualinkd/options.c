#include "options.h"

#include <assert.h>
#include <getopt.h>
#include <stdio.h>
#include <stdlib.h>
#include "config/config_helpers.h"
#include "logging/logging.h"
#include "version/version.h"
#include "utils.h"

void printHelp()
{
	printf("\n");
	printf("Usage: %s [options]\n", AQUALINKD_NAME);
	printf("\n");
	printf("A daemon to control Jandy Aqualink RS pool equipment from any home automation hub or web browser.\n");
	printf("\n");
	printf("Options:\n");
	printf("\n");
	printf("\t-c, --config-file <file>                    Daemon configuration file (default: %s)\n", CFG_ConfigFile());
	printf("\t-D, --debug                                 Enable debug level logging\n");
	printf("\t-h, --help                                  Print usage information\n");
	printf("\t    --log-file <file>                       Daemon log file (default: %s)\n", CFG_LogFile());
	printf("\t    --log-raw-serial                        Log all received serial bytes as-received without decoding\n");
	printf("\t    --log-raw-serial-file <file>            Raw serial log file (default: %s)\n", CFG_LogRawRsBytes_LogFile());
	printf("\t    --log-serial                            Log decoded serial payloads\n");
	printf("\t    --log-serial-file <file>                Serial log file (default: %s)\n", CFG_LogRawRsBytes_LogFile());
	printf("\t-n, --no-daemonize                          Prevent %s from running as a daemon\n", AQUALINKD_NAME);
	printf("\t-p, --pid-file <file>                       ath to use for daemon PID file (default: %s)\n", CFG_PidFile());
	printf("\t-s, --serial-port <file>                    Serial port/device to connect with (default: %s)\n", CFG_SerialPort());
	printf("\t    --trace                                 Enable trace level logging\n");
	printf("\t-v, --version                               Print version information and quit\n");
	printf("\n");
}

void printVersion()
{
	printf("\n");
	printf("Version: %s\n", AQUALINKD_VERSION);
	printf("Built: %s\n", __TIMESTAMP__);
	printf("\n");
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
	OPTION_FLAG_NO_DAEMONIZE = 'n',
	OPTION_FLAG_PID_FILE = 'p',
	OPTION_FLAG_SERIAL_DEVICE = 's',
	OPTION_FLAG_VERSION = 'v',
	
	// Long options without a corresponding short option.
	OPTION_FLAG_LOG_FILE = 0x100,
	OPTION_FLAG_LOG_RAW_SERIAL = 0x101,
	OPTION_FLAG_LOG_RAW_SERIAL_FILE = 0x102,
	OPTION_FLAG_LOG_SERIAL = 0x103,
	OPTION_FLAG_LOG_SERIAL_FILE = 0x104,
	OPTION_FLAG_TRACE = 0x105
};

void handleOptions(int argc, char* argv[])
{
	static const struct option aqualink_long_options[] =
	{
		{ "config-file",			required_argument,	0, OPTION_FLAG_CONFIG_FILE },
		{ "debug",					no_argument,		0, OPTION_FLAG_DEBUG },
		{ "help",					no_argument,		0, OPTION_FLAG_HELP },
		{ "log-file",				required_argument,	0, OPTION_FLAG_LOG_FILE },
		{ "log-raw-serial",			no_argument,		0, OPTION_FLAG_LOG_RAW_SERIAL },
		{ "log-raw-serial-file",	required_argument,	0, OPTION_FLAG_LOG_RAW_SERIAL_FILE },
		{ "log-serial",				no_argument,		0, OPTION_FLAG_LOG_SERIAL },
		{ "log-serial-file",		required_argument,	0, OPTION_FLAG_LOG_SERIAL_FILE },
		{ "no-daemonize",			no_argument,		0, OPTION_FLAG_NO_DAEMONIZE },
		{ "pid-file",				required_argument,	0, OPTION_FLAG_PID_FILE },
		{ "serial-port",			required_argument,	0, OPTION_FLAG_SERIAL_DEVICE },
		{ "trace",					no_argument,		0, OPTION_FLAG_TRACE },
		{ "version",				no_argument,		0, OPTION_FLAG_VERSION },

		{0, 0, 0, 0}
	};

	static char* aqualink_short_options = "c:Dhnp:s:v";

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
			break;

		case OPTION_FLAG_LOG_FILE: // long option "log-file"
			CFG_Set_LogFile(optarg);
			break;

		case OPTION_FLAG_LOG_RAW_SERIAL: // long option "log-raw-serial"
			CFG_Set_LogRawRsBytes(true);
			break;

		case OPTION_FLAG_LOG_RAW_SERIAL_FILE: // long option "log-raw-serial-file"
			break;

		case OPTION_FLAG_LOG_SERIAL: // long option "log-serial"
			CFG_Set_DebugRsProtocolPackets(true);
			break;

		case OPTION_FLAG_LOG_SERIAL_FILE: // long option "log-serial-file"
			break;

		case OPTION_FLAG_NO_DAEMONIZE: // short option 'n' / long option "no-daemonize"
			CFG_Set_NoDaemonize(true);
			break;

		case OPTION_FLAG_PID_FILE: // short option 'p' / long option "pid-file"
			CFG_Set_PidFile(optarg);
			break;

		case OPTION_FLAG_SERIAL_DEVICE: // short option 's' / long option "serial-port"
			CFG_Set_SerialPort(optarg);
			break;

		case OPTION_FLAG_TRACE: // long option "trace"
			CFG_Set_LogLevel(Trace);
			break;

		case OPTION_FLAG_VERSION: // short option 'v' / long option "version"
			printVersion();
			exit(EXIT_SUCCESS);

		case OPTION_FLAG_HELP: // short option 'h' / long option "help"
		default:  // any unknown options
			printHelp();
			exit(EXIT_SUCCESS);
		}
	}
}