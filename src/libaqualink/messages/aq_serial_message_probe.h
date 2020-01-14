#ifndef AQ_SERIAL_MESSAGE_PROBE_H_
#define AQ_SERIAL_MESSAGE_PROBE_H_

#include <stdint.h>

#include "hardware/devices/hardware_device_types.h"
#include "serial/aq_serial_types.h"

typedef struct tagAQ_Probe_Packet
{
	struct
	{
		uint8_t DLE;
		uint8_t STX;
	}
	Header;

	HardwareDeviceId Destination;
	SerialData_Commands Command;

	struct
	{
		uint8_t Checksum;
		uint8_t DLE;
		uint8_t ETX;
	}
	Terminator;
}
AQ_Probe_Packet;

extern const unsigned int AQ_PROBE_PACKET_LENGTH;

#endif // AQ_SERIAL_MESSAGE_PROBE_H_
