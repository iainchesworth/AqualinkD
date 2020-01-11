#include "aq_serial_threaded.h"

#include <stdbool.h>
#include <string.h>

#include "cross-platform/threads.h"
#include "config/config_helpers.h"
#include "cross-platform/serial.h"
#include "logging/logging.h"
#include "threads/thread_utils.h"
#include "aq_serial.h"
#include "aq_serial_data_logger.h"
#include "aq_serial_messages.h"
#include "aq_serial_reader.h"
#include "aq_serial_writer.h"
#include "aq_serial_writer_queue.h"
#include "utils.h"

int serial_thread(void* termination_handler_ptr)
{
	TRACE("Serial worker thread is starting");
	
	const int MAXIMUM_NUMBER_OF_WRITER_ENTRIES = 10;

	SerialThread_States state = ST_INIT;
	SerialDevice serial_device = SERIALDEVICE_INVALID;

	int returnCode = 0;

	do
	{
		switch (state)
		{
		case ST_INIT:
			TRACE("ST_INIT");
			{
				if (SERIALDEVICE_INVALID == (serial_device = initialise_serial_device()))
				{
					// There is literally nothing that can be done now
					TRACE("Transition: ST_INIT --> ST_TERMINATE");
					trigger_application_termination();
					state = ST_TERMINATE;
				}
				else if (!set_interface_attributes(serial_device))
				{
					ERROR("Failed configuring serial port for communications");
				}
				else if (!initialise_serial_writer_send_queue(MAXIMUM_NUMBER_OF_WRITER_ENTRIES))
				{
					WARN("Failed initialising the serial writer queue; can receive but not send packets");
				}
				else
				{
					// Initialise the serial data logger, if required.
					if (CFG_LogRawRsBytes())
					{
						aq_serial_logger_initialise();
					}

					// Everything is ready to go...start reading data
					TRACE("Transition: ST_INIT --> ST_READPACKET");
					state = ST_READPACKET;
				}
			}
			break;

		case ST_READPACKET:
			TRACE("ST_READPACKET");
			{
				unsigned char packet[AQ_MAXPKTLEN];
				memset(&packet, 0, AQ_MAXPKTLEN);

				const int packet_length = serial_getnextpacket(serial_device, packet);

				if (0 > packet_length)
				{
					// There was an error while reading data from the serial port (resulting in a -1 error code).
					WARN("There was an error while reading data from the serial port...attempting recovery");
					TRACE("Transition: ST_READPACKET --> ST_RECOVERY");
					state = ST_RECOVERY;
				}
				else if (0 == packet_length)
				{
					// There was no data returned by the serial port.  This is weird because we specifcally block while waiting for data.
					TRACE("Weird...had a read() return 0 bytes");
				}
				else if (!process_aqualink_packet(packet, packet_length))
				{
					// Failed to process the packet.
					WARN("There was an error while processing the packet data...attempting recovery");
					TRACE("Transition: ST_READPACKET --> ST_RECOVERY");
					state = ST_RECOVERY;
				}
				else if (MAXIMUM_NUMBER_OF_WRITER_ENTRIES != serial_writer_send_queue_empty_entries())
				{
					// There is at least one or more packets of data to send.  Let's do that.
					TRACE("Transition: ST_READPACKET --> ST_WRITEPACKET");
					state = ST_WRITEPACKET;
				}
				else
				{
					// Everything's done!
				}
			}
			break;

		case ST_WRITEPACKET:
			TRACE("ST_WRITEPACKET");
			{
				const int length_written = serial_sendnextpacket(serial_device);

				if (0 > length_written)
				{
					// There was an error while writing data to the serial port (resulting in a -1 error code).
					WARN("There was an error while writing data from the serial port...attempting recovery");
					TRACE("Transition: ST_READPACKET --> ST_RECOVERY");
					state = ST_RECOVERY;
				}
				else if (0 == length_written)
				{
					// There was no data sent to the serial port.  This is weird because we specifcally block while writing data.
					TRACE("Weird...had a write() return 0 bytes");
				}
				else if (MAXIMUM_NUMBER_OF_WRITER_ENTRIES == serial_writer_send_queue_empty_entries())
				{
					// There are no more packets to send so transitionback to waiting for a new packet.
					TRACE("Transition: ST_WRITEPACKET --> ST_READPACKET");
					state = ST_READPACKET;
				}
				else
				{
					// There is at least one or more packets of data to send...go round and do that.
				}
			}
			break;

		case ST_RECOVERY:
			TRACE("ST_RECOVERY");
			{
			}
			break;

		case ST_TERMINATE:
			TRACE("ST_TERMINATE");
			{
			}
			break;
		}
	} 
	while (ST_TERMINATE != state);


	// Terminate the serial data logger if it was running.
	if (CFG_LogRawRsBytes())
	{
		shutdown_logging(&aq_serial_data_logger);
	}

	close_serial_device(serial_device);

	return returnCode;
}
