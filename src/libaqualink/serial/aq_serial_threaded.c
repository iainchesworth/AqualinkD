#include "aq_serial_threaded.h"

#include <errno.h>
#include <fcntl.h>
#include <stdbool.h>
#include <string.h>
#include <syslog.h>
#include <termios.h>
#include <threads.h>
#include <unistd.h>
#include "aq_serial.h"
#include "aq_serial_messages.h"
#include "aq_serial_statemachine.h"
#include "utils.h"

static struct termios original_serial_config, aqualink_serial_config;

bool set_interface_attributes(int serial_device)
{
	bool returnCode = false;

	if (0 > tcgetattr(serial_device, &original_serial_config))
	{
		logMessage(LOG_WARNING, "AQ_Serial_Threaded.c | serial_thread | Failed to get serial port attributes\n");
		logMessage(LOG_WARNING, "tcgetattr() error: %d - %s\n", errno, strerror(errno));
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
			logMessage(LOG_ERR, "AQ_Serial_Threaded.c | serial_thread | Failed to set serial port baud rate\n");
			logMessage(LOG_ERR, "cfset[i|o]speed() error: %d - %s\n", errno, strerror(errno));
		}
		else if (0 > tcsetattr(serial_device, TCSAFLUSH, &aqualink_serial_config))
		{
			logMessage(LOG_ERR, "AQ_Serial_Threaded.c | serial_thread | Failed to set serial port baud attributes\n");
			logMessage(LOG_ERR, "tcsetattr() error: %d - %s\n", errno, strerror(errno));
		}
		else
		{
			logMessage(LOG_DEBUG, "AQ_Serial_Threaded.c | serial_thread | Serial port configured successfully\n");
			returnCode = true;
		}
	}

	return returnCode;
}

int serial_thread(void* termination_handler_ptr)
{
	logMessage(LOG_DEBUG_SERIAL, "AQ_Serial_Threaded.c | serial_thread | Serial worker thread is starting\n");

	static const char* serial_device_tty = "/dev/ttyS8";

	int serial_device = -1, returnCode = 0;

	SerialThread_States state = ST_INIT;

	do
	{
		switch (state)
		{
		case ST_INIT:
			logMessage(LOG_DEBUG_SERIAL, "AQ_Serial_Threaded.c | serial_thread | ST_INIT\n");
			{
				// Initialise the serial device tty config structures!
				memset(&original_serial_config, 0, sizeof(original_serial_config));
				memset(&aqualink_serial_config, 0, sizeof(aqualink_serial_config));

				if (-1 == (serial_device = open(serial_device_tty, O_RDWR | O_NOCTTY | O_SYNC)))
				{
					logMessage(LOG_ERR, "AQ_Serial_Threaded.c | serial_thread | Failed to open serial port\n");
					logMessage(LOG_ERR, "open() error: %d - %s\n", errno, strerror(errno));
				}
				else if (!isatty(serial_device))
				{
					logMessage(LOG_ERR, "AQ_Serial_Threaded.c | serial_thread | Serial device is not a TTY\n");
					logMessage(LOG_ERR, "isatty() error: %d - %s\n", errno, strerror(errno));
				}
				else if (!set_interface_attributes(serial_device))
				{
					logMessage(LOG_ERR, "AQ_Serial_Threaded.c | serial_thread | Failed configuring serial port for communications\n");
				}
				else
				{
					// Everything is ready to go...start reading data
					logMessage(LOG_DEBUG_SERIAL, "AQ_Serial.c | serial_thread | Transition: ST_INIT --> ST_READPACKET\n");
					state = ST_READPACKET;
				}
			}
			break;

		case ST_READPACKET:
			logMessage(LOG_DEBUG_SERIAL, "AQ_Serial_Threaded.c | serial_thread | ST_READPACKET\n");
			{
				unsigned char packet[AQ_MAXPKTLEN];
				memset(&packet, 0, AQ_MAXPKTLEN);

				const int packet_length = serial_getnextpacket(serial_device, packet);
				
				if (0 > packet_length)
				{
					// There was an error while reading data from the serial port (resulting in a -1 error code).
					logMessage(LOG_WARNING, "AQ_Serial.c | serial_thread | There was an error while reading data from the serial port...attempting recovery\n");
					logMessage(LOG_DEBUG_SERIAL, "AQ_Serial.c | serial_thread | Transition: ST_READPACKET --> ST_RECOVERY\n");
					state = ST_RECOVERY;
				}
				else if (0 == packet_length)
				{
					// There was no data returned by the serial port.  This is weird because we specifcally block while waiting for data.
					logMessage(LOG_DEBUG_SERIAL, "AQ_Serial.c | serial_thread | Weird...had a read() return 0 bytes\n");
				}
				else if (!process_aqualink_packet(packet, packet_length))
				{
					// Failed to process the packet.
					logMessage(LOG_WARNING, "AQ_Serial.c | serial_thread | There was an error while processing the packet data...attempting recovery\n");
					logMessage(LOG_DEBUG_SERIAL, "AQ_Serial.c | serial_thread | Transition: ST_READPACKET --> ST_RECOVERY\n");
					state = ST_RECOVERY;
				}
				else
				{

				}
			}
			break;

		case ST_WRITEPACKET:
			logMessage(LOG_DEBUG_SERIAL, "AQ_Serial_Threaded.c | serial_thread | ST_WRITEPACKET\n");
			{
			}
			break;

		case ST_RECOVERY:
			logMessage(LOG_DEBUG_SERIAL, "AQ_Serial_Threaded.c | serial_thread | ST_RECOVERY\n");
			{
			}
			break;

		case ST_TERMINATE:
			logMessage(LOG_DEBUG_SERIAL, "AQ_Serial_Threaded.c | serial_thread | ST_TERMINATE\n");
			{
			}
			break;
		}
	} 
	while (ST_TERMINATE != state);

	// Make sure that we close the serial device file descriptor.
	tcsetattr(serial_device, TCSANOW, &original_serial_config);
	close(serial_device);

	return returnCode;
}
