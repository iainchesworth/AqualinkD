#include "aq_serial_threaded.h"

#include <errno.h>
#include <fcntl.h>
#include <stdbool.h>
#include <string.h>
#include <syslog.h>
#include <termios.h>
#include <threads.h>
#include <unistd.h>
#include "config/config_helpers.h"
#include "logging/logging.h"
#include "threads/thread_utils.h"
#include "aq_serial.h"
#include "aq_serial_data_logger.h"
#include "aq_serial_messages.h"
#include "aq_serial_statemachine.h"
#include "utils.h"

static struct termios original_serial_config, aqualink_serial_config;

bool set_interface_attributes(int serial_device)
{
	bool returnCode = false;

	if (0 > tcgetattr(serial_device, &original_serial_config))
	{
		WARN("Failed to get serial port attributes");
		WARN("tcgetattr() error: %d - %s", errno, strerror(errno));
	}
	else
	{
		//
		// FIXME - It's a bad idea to simply initialize a struct termios structure to a chosen set of attributes and pass it directly 
		// to tcsetattr. Your program may be run years from now, on systems that support members not documented in this manual.  The 
		// way to avoid setting these members to unreasonable values is to avoid changing them.
		//
		//    - http://kirste.userpage.fu-berlin.de/chemnet/use/info/libc/libc_12.html#SEC237
		//

		aqualink_serial_config.c_cflag = B9600 | CS8 | CLOCAL | CREAD;
		aqualink_serial_config.c_iflag = IGNPAR;  // Ignore framing and parity errors
		aqualink_serial_config.c_lflag = 0;       // Turn off line processing;  
		aqualink_serial_config.c_oflag = 0;		  // Turn off output processing.

		aqualink_serial_config.c_cc[VMIN] = 1;	  // One input byte is enough to return from read()
		aqualink_serial_config.c_cc[VTIME] = 0;	  // No timeout i.e. block until VMIN characters have been transferred


		if ((0 > cfsetispeed(&aqualink_serial_config, B9600)) || (0 > cfsetospeed(&aqualink_serial_config, B9600)))
		{
			ERROR("Failed to set serial port baud rate");
			ERROR("cfset[i|o]speed() error: %d - %s", errno, strerror(errno));
		}
		else if (0 > tcsetattr(serial_device, TCSAFLUSH, &aqualink_serial_config))
		{
			ERROR("Failed to set serial port baud attributes");
			ERROR("tcsetattr() error: %d - %s", errno, strerror(errno));
		}
		else
		{
			DEBUG("Serial port configured successfully");
			returnCode = true;
		}
	}

	return returnCode;
}

int serial_thread(void* termination_handler_ptr)
{
	TRACE("Serial worker thread is starting");
	
	int serial_device = -1, returnCode = 0;

	SerialThread_States state = ST_INIT;

	do
	{
		switch (state)
		{
		case ST_INIT:
			TRACE("ST_INIT");
			{
				// Initialise the serial device tty config structures!
				memset(&original_serial_config, 0, sizeof(original_serial_config));
				memset(&aqualink_serial_config, 0, sizeof(aqualink_serial_config));

				if (-1 == (serial_device = open(CFG_SerialPort(), O_RDWR | O_NOCTTY | O_SYNC)))
				{
					ERROR("Failed to open serial port: %s", CFG_SerialPort());
					ERROR("    open() error: %d - %s", errno, strerror(errno));

					// There is literally nothing that can be done now
					TRACE("Transition: ST_INIT --> ST_TERMINATE");
					trigger_application_termination();
					state = ST_TERMINATE;
				}
				else if (!isatty(serial_device))
				{
					ERROR("Serial device is not a TTY");
					ERROR("    isatty() error: %d - %s", errno, strerror(errno));

					// There is literally nothing that can be done now
					TRACE("Transition: ST_INIT --> ST_TERMINATE");
					trigger_application_termination();
					state = ST_TERMINATE;
				}
				else if (!set_interface_attributes(serial_device))
				{
					ERROR("Failed configuring serial port for communications");
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
				else
				{

				}
			}
			break;

		case ST_WRITEPACKET:
			TRACE("ST_WRITEPACKET");
			{
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

	// Make sure that we close the serial device file descriptor.
	tcsetattr(serial_device, TCSANOW, &original_serial_config);
	close(serial_device);

	return returnCode;
}
