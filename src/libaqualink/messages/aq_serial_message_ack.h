#ifndef AQ_SERIAL_MESSAGE_ACK_H_
#define AQ_SERIAL_MESSAGE_ACK_H_

#include <stdint.h>

#include "hardware/devices/hardware_device_types.h"
#include "serial/aq_serial_types.h"

typedef struct tagAQ_Ack_Packet
{
	struct
	{
		uint8_t DLE;
		uint8_t STX;
	}
	Header;

	HardwareDeviceId Destination;
	SerialData_Commands Command;

	SerialData_AckTypes AckType;
	unsigned char CommandBeingAcked;

	struct
	{
		uint8_t Checksum;
		uint8_t DLE;
		uint8_t ETX;
	}
	Terminator;
}
AQ_Ack_Packet;

extern const unsigned int AQ_ACK_PACKET_LENGTH;

#endif // AQ_SERIAL_MESSAGE_ACK_H_
