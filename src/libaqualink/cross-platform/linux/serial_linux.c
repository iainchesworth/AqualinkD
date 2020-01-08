#include "../serial.h"

#if !defined (WIN32)

#include <sys/types.h>
#include <sys/stat.h>
#include <errno.h>
#include <fcntl.h>
#include <termios.h>
#include <unistd.h>

#include "config/config_helpers.h"
#include "logging/logging.h"

const SerialDevice SERIALDEVICE_INVALID = -1;

static struct termios original_serial_config, aqualink_serial_config;

SerialDevice initialise_serial_device()
{
	SerialDevice serial_device;

	// Initialise the serial device tty config structures!
	memset(&original_serial_config, 0, sizeof(original_serial_config));
	memset(&aqualink_serial_config, 0, sizeof(aqualink_serial_config));

	if (-1 == (serial_device = open(CFG_SerialPort(), O_RDWR | O_NOCTTY | O_SYNC)))
	{
		ERROR("Failed to open serial port: %s", CFG_SerialPort());
		ERROR("    open() error: %d - %s", errno, strerror(errno));
	}
	else if (!isatty(serial_device))
	{
		ERROR("Serial device is not a TTY");
		ERROR("    isatty() error: %d - %s", errno, strerror(errno));
	}
	else
	{
		// Initialisation was successful.
		DEBUG("Successfully opened serial device: %s", CFG_SerialPort());
	}

	return serial_device;
}

bool set_interface_attributes(SerialDevice serial_device)
{
	bool returnCode = false;

	if (0 > tcgetattr(serial_device, &original_serial_config))
	{
		WARN("Failed to get serial port attributes");
		WARN("    tcgetattr() error: %d - %s", errno, strerror(errno));
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
			ERROR("    cfset[i|o]speed() error: %d - %s", errno, strerror(errno));
		}
		else if (0 > tcsetattr(serial_device, TCSAFLUSH, &aqualink_serial_config))
		{
			ERROR("Failed to set serial port baud attributes");
			ERROR("    tcsetattr() error: %d - %s", errno, strerror(errno));
		}
		else
		{
			DEBUG("Serial port configured successfully");
			returnCode = true;
		}
	}

	return returnCode;
}

int read_from_serial_device(SerialDevice serial_device, unsigned char buffer[], unsigned int buffer_length)
{
	return read(serial_device, buffer, buffer_length);
}

int write_to_serial_device(SerialDevice serial_device, const unsigned char buffer[], unsigned int buffer_length)
{
	return write(serial_device, buffer, buffer_length);
}

void close_serial_device(SerialDevice serial_device)
{
	// Make sure that we close the serial device file descriptor.
	tcsetattr(serial_device, TCSANOW, &original_serial_config);
	close(serial_device);
}

#endif // !defined (WIN32)
