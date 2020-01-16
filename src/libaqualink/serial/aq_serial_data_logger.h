#ifndef AQ_SERIAL_DATA_LOGGER_H_
#define AQ_SERIAL_DATA_LOGGER_H_

#include <stdbool.h>

#include "logging/logging.h"
#include "logging/logging_message.h"

extern Logger aq_serial_data_logger;

void aq_serial_logger_initialise(void);
void aq_serial_data_logger_error_handler(const char error_message[]);
void aq_serial_data_logger_formatter(char buffer[], unsigned int buffer_length, LoggingMessage logMessage);

void log_serial_packet(const unsigned char* packet_buffer, unsigned int packet_buffer_length, bool packet_is_bad);

#endif // AQ_SERIAL_DATA_LOGGER_H_
