#ifndef AQ_SERIAL_MESSAGE_UNKNOWN_H_
#define AQ_SERIAL_MESSAGE_UNKNOWN_H_

#include <stdint.h>

#include "hardware/devices/hardware_device_types.h"
#include "serial/aq_serial_types.h"

typedef struct tagAQ_Unknown_Packet
{
	struct
	{
		uint8_t DLE;
		uint8_t STX;
	}
	Header;

	HardwareDeviceId Destination;
	SerialData_Commands Command;

	// There may be a payload of 0..n bytes (length is indeterminate)

	struct
	{
		uint8_t Checksum;
		uint8_t DLE;
		uint8_t ETX;
	}
	Terminator;
}
AQ_Unknown_Packet;

extern const unsigned int AQ_UNKNOWN_PACKET_LENGTH;

#endif // AQ_SERIAL_MESSAGE_UNKNOWN_H_
