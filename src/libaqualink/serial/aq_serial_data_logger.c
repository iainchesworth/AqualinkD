#include "aq_serial_data_logger.h"

#include <assert.h>
#include <stdlib.h>

#include "config/config_helpers.h"
#include "logging/logging.h"
#include "logging/logging_levels.h"
#include "logging/logging_error_handler.h"
#include "logging/logging_formatter.h"
#include "logging/logging_message.h"
#include "logging/logging_sink.h"
#include "logging/logging_sink_basic_file.h"

#include "aqualink.h"

const char AQUALINK_SERIAL_DATA_LOGGER_NAME[] = "Aqualink Serial Data Logger";
const char INDIVIDUAL_BYTE_OUTPUT_FORMAT[] = "0x%02hhx|";

LoggingSinkBasicFileUserData aq_serial_data_logger_file_sink_user_data =
{
	.Filename = 0,
	.File = 0
};

LoggingSink aq_serial_data_logger_file_sink =
{
	.Config =
	{
		.SinkIsInitialised = false
		//.SinkWriterMutex <-- cannot be inited this way.
	},

	.Initialise = &logging_sink_basic_file_initialise,	// DEFAULT - Basic File Sink
	.Formatter = &logging_sink_basic_file_formatter,	// DEFAULT - Basic File Sink
	.Pattern = &logging_sink_basic_file_pattern,		// DEFAULT - Basic File Sink
	.Writer = &logging_sink_basic_file_writer,			// DEFAULT - Basic File Sink
	.Flush = &logging_sink_basic_file_flush,			// DEFAULT - Basic File Sink
	.Close = &logging_sink_basic_file_close,			// DEFAULT - Basic File Sink

	.UserData = &aq_serial_data_logger_file_sink_user_data
};

Logger aq_serial_data_logger =
{
	.Name = AQUALINK_SERIAL_DATA_LOGGER_NAME,
	.Verbosity = Trace,
	.Sinks = { 0, 0, 0 },
	.ErrorHandler = &aq_serial_data_logger_error_handler,
	.Formatter = &aq_serial_data_logger_formatter
};

void aq_serial_logger_initialise()
{
	aq_serial_data_logger_file_sink_user_data.Filename = CFG_RawSerial_LogFile();
	register_logging_sink(&aq_serial_data_logger.Sinks, &aq_serial_data_logger_file_sink);
}

void aq_serial_data_logger_error_handler(const char error_message[])
{
	UNREFERENCED_PARAMETER(error_message);

	///FIXME
}

void aq_serial_data_logger_formatter(char buffer[], unsigned int buffer_length, LoggingMessage logMessage)
{
	assert(0 != buffer);

	switch(logMessage.Level)
	{
	case Trace:
	case Debug:
	case Info:
	case Notice:
	case Warning:
	case Error:
	case Critical:
	case Off:
	default:
		{
			// NOTE: Logging here is the "RAW BYTES"
			//
			// Format - 0x00|0x01|0x02|0x03|0x04......
			snprintf(buffer, buffer_length, "%s", INDIVIDUAL_BYTE_OUTPUT_FORMAT);  // Overwrite the provided format string (whatever it was)
		}
		break;
	}	
}

void log_serial_packet(const unsigned char* packet_buffer, unsigned int packet_buffer_length, bool packet_is_bad)
{
	const unsigned int decoded_packet_buffer_length = (packet_buffer_length * 5) + 1;  // Each serial byte decodes into five when printed i.e. "0x00|".
	char* decoded_packet_buffer = (char*)malloc(decoded_packet_buffer_length);

	if (0 == decoded_packet_buffer)
	{
		WARN("Failed to allocate memory for decoded packet buffer in serial data logger");
	}
	else
	{
		memset(decoded_packet_buffer, 0, decoded_packet_buffer_length);

		unsigned int buffer_index;

		for (buffer_index = 0; buffer_index < packet_buffer_length; ++buffer_index)
		{
			sprintf(&(decoded_packet_buffer[5 * buffer_index]), INDIVIDUAL_BYTE_OUTPUT_FORMAT, packet_buffer[buffer_index]);
		}

		const char* PACKET_IS_BAD = (packet_is_bad) ? "BAD PACKET" : "";
		
		DEBUG("%s%8.8s Packet | HEX: %s", PACKET_IS_BAD, "JANDY", decoded_packet_buffer);
	}

	if (0 != decoded_packet_buffer)
	{
		free(decoded_packet_buffer);
	}
}
