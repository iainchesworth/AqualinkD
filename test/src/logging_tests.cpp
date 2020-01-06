#include <string>
#include <sstream>
#include <gtest/gtest.h>

#include "logging/logging.h"
#include "serial/aq_serial_data_logger.h"


TEST(Logging, Default_Logger)
{
	const char* TRACE_STRING = "TRACE string example";
	const char* DEBUG_STRING = "DEBUG string example";
	std::string stdout_output;

	initialize_logging(&aqualink_default_logger);

	set_verbosity(&aqualink_default_logger, Trace);

	testing::internal::CaptureStdout();
	TRACE(TRACE_STRING);
	stdout_output = testing::internal::GetCapturedStdout();
	ASSERT_TRUE(0 == strncmp(TRACE_STRING, stdout_output.c_str(), strlen(TRACE_STRING)));
	ASSERT_TRUE(strlen(TRACE_STRING) + 1 == stdout_output.length());

	testing::internal::CaptureStdout();
	DEBUG(DEBUG_STRING);
	stdout_output = testing::internal::GetCapturedStdout();
	ASSERT_TRUE(0 == strncmp(DEBUG_STRING, stdout_output.c_str(), strlen(DEBUG_STRING)));
	ASSERT_TRUE(strlen(DEBUG_STRING) + 1 == stdout_output.length());

	set_verbosity(&aqualink_default_logger, Off);

	testing::internal::CaptureStdout();
	TRACE(TRACE_STRING);
	stdout_output = testing::internal::GetCapturedStdout();
	ASSERT_FALSE(0 == strncmp(TRACE_STRING, stdout_output.c_str(), strlen(TRACE_STRING)));

	testing::internal::CaptureStdout();
	DEBUG(DEBUG_STRING);
	stdout_output = testing::internal::GetCapturedStdout();
	ASSERT_FALSE(0 == strncmp(DEBUG_STRING, stdout_output.c_str(), strlen(DEBUG_STRING)));

	shutdown_logging(&aqualink_default_logger);
}

TEST(Logging, AQ_Serial_Data_Logger)
{
	initialize_logging(&aq_serial_data_logger);
	
	aq_serial_logger_initialise();
	
	DEBUG_TO(&aq_serial_data_logger, "%d", 0xAA);
	DEBUG_TO(&aq_serial_data_logger, "%d", 0x55);
	DEBUG_TO(&aq_serial_data_logger, "%d", 0xAA);
	DEBUG_TO(&aq_serial_data_logger, "%d", 0x55);

	shutdown_logging(&aq_serial_data_logger);
}
