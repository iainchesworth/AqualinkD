#include "options.h"

#include <assert.h>
#include <getopt.h>
#include <stdio.h>
#include <stdlib.h>
#include <syslog.h>
#include "config/config_helpers.h"
#include "utils.h"
#include "version.h"

void printHelp()
{
	printf("%s %s\n", AQUALINKD_NAME, AQUALINKD_VERSION);
	printf("\t-h         (this message)\n");
	printf("\t-d         (do not deamonize)\n");
	printf("\t-c <file>  (Configuration file)\n");
	printf("\t-v         (Debug logging)\n");
	printf("\t-vv        (Serial Debug logging)\n");
	printf("\t-rsd       (RS485 debug)\n");
	printf("\t-rsrd      (RS485 raw debug)\n");
}

// For long options that don't have a corresponding short option, the flag should 
// be a value thats not part of the optstring.
//
// If one chooses a value higher than 255 then it can't possibly be a short option.

enum aqualink_option_flags
{
	OPTION_FLAG_HELP = 'h',
	OPTION_FLAG_NO_DAEMONIZE = 'd',
	OPTION_FLAG_CONFIG_FILE = 'c',
	OPTION_FLAG_DEBUG = 'v',

	// Long options without a corresponding short option.
	OPTION_FLAG_RSD = 0x100,
	OPTION_FLAG_RSRD = 0x101
};

void handleOptions(int argc, char* argv[])
{
	assert(0 == _config_parameters);

	static const struct option aqualink_long_options[] =
	{
		{ "help",         no_argument,       0, OPTION_FLAG_HELP},
		{ "no-daemonize", no_argument,       0, OPTION_FLAG_NO_DAEMONIZE},
		{ "config-file",  required_argument, 0, OPTION_FLAG_CONFIG_FILE},
		{ "debug",        no_argument,       0, OPTION_FLAG_DEBUG},
		{ "rsd",          no_argument,       0, OPTION_FLAG_RSD},
		{ "rsrd",         no_argument,       0, OPTION_FLAG_RSRD},
		{0, 0, 0, 0}
	};

	static char* aqualink_short_options = "hdc:v";

	int ch = 0;

	while ((ch = getopt_long(argc, argv, aqualink_short_options, aqualink_long_options, 0)) != -1)
	{
		// check to see if a single character or long option came through
		switch (ch)
		{
		case OPTION_FLAG_NO_DAEMONIZE: // short option 'd' / long option "no-daemonize"
			CFG_Set_Daemonize(false);
			break;

		case OPTION_FLAG_CONFIG_FILE: // short option 'c' / long option "config-file"
			CFG_Set_ConfigFile(optarg);
			break;

		case OPTION_FLAG_DEBUG: // short option 'v' / long option "debug"
			if (LOG_DEBUG_SERIAL == CFG_LogLevel())
			{
				// Already at the highest level of debug logging...do nothing.
				logMessage(LOG_DEBUG, "Already at highest level of debugging...don't need to specify more verbosity");
			}
			else if (LOG_DEBUG == CFG_LogLevel())
			{
				// There has been more than one "-v" option so increment the logging level.
				CFG_Set_LogLevel(LOG_DEBUG_SERIAL);
			}
			else
			{
				CFG_Set_LogLevel(LOG_DEBUG);
			}
			break;

		case OPTION_FLAG_RSD: // long option "rsd"
			CFG_Set_DebugRsProtocolPackets(true);
			break;

		case OPTION_FLAG_RSRD: // long option "rsrd"
			CFG_Set_LogRawRsBytes(true);
			break;

		case OPTION_FLAG_HELP: // short option 'h' / long option "help"
		default:  // any unknown options
			printHelp();
			exit(EXIT_SUCCESS);
		}
	}
}