/*
 * Copyright (c) 2017 Shaun Feakes - All rights reserved
 *
 * You may use redistribute and/or modify this code under the terms of
 * the GNU General Public License version 2 as published by the
 * Free Software Foundation. For the terms of this license,
 * see <http://www.gnu.org/licenses/>.
 *
 * You are free to use this software under the terms of the GNU General
 * Public License, but WITHOUT ANY WARRANTY; without even the implied
 * warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
 * See the GNU General Public License for more details.
 *
 *  https://github.com/sfeakes/aqualinkd
 */

#include <assert.h>
#include <stdbool.h>
#include <stdio.h>

#include "config/config.h"
#include "config/config_helpers.h"
#include "cross-platform/serial.h"
#include "logging/logging.h"
#include "aq_serial.h"
#include "aq_serial_checksums.h"
#include "aq_serial_data_logger.h"
#include "aq_serial_reader.h"
#include "aq_serial_types.h"
#include "utils.h"

void send_packet(SerialDevice serial_device, unsigned char* packet, int length);

bool _pda_mode = false;

void set_pda_mode(bool mode)
{
	if (mode) {
		NOTICE("AqualinkD is using PDA mode");
	}

	_pda_mode = mode;
}
bool pda_mode()
{
	return _pda_mode;
}

const char* get_packet_type(unsigned char* packet, int length)
{
	static char buf[15];

	if (length <= 0)
	{
		return "";
	}

	switch (packet[PKT_CMD]) {
	case CMD_ACK:
		return "Ack";
		break;
	case CMD_STATUS:
		return "Status";
		break;
	case CMD_MSG:
		return "Message";
		break;
	case CMD_MSG_LONG:
		return "Lng Message";
		break;
	case CMD_PROBE:
		return "Probe";
		break;
	case CMD_GETID:
		return "GetID";
		break;
	case CMD_PERCENT:
		return "AR %%";
		break;
	case CMD_PPM:
		return "AR PPM";
		break;
	case CMD_PDA_0x05:
		return "PDA Unknown";
		break;
	case CMD_PDA_0x1B:
		return "PDA Init (*guess*)";
		break;
	case CMD_PDA_HIGHLIGHT:
		return "PDA Hlight";
		break;
	case CMD_PDA_CLEAR:
		return "PDA Clear";
		break;
	case CMD_PDA_SHIFTLINES:
		return "PDA Shiftlines";
		break;
	case CMD_PDA_HIGHLIGHTCHARS:
		return "PDA C_HlightChar";
		break;
	case CMD_IAQ_MSG:
		return "iAq Message";
		break;
	case CMD_IAQ_MENU_MSG:
		return "iAq Menu";
		break;
	default:
		sprintf(buf, "Unknown '0x%02hhx'", packet[PKT_CMD]);
		return buf;
		break;
	}
}

protocolType getProtocolType(const unsigned char* packet) 
{
	if (packet[0] == DLE)
	{
		return JANDY;
	}
	
	if (packet[0] == PP1)
	{
		return PENTAIR;
	}

	return P_UNKNOWN;
}

#ifndef PLAYBACK_MODE

// Send an ack packet to the Aqualink RS8 master device.
// serial_device: the file descriptor of the serial port connected to the device
// command: the command byte to send to the master device, NUL if no command
// 
// NUL = '\x00'
// DLE = '\x10'
// STX = '\x02'
// ETX = '\x03'
// 
// masterAddr = '\x00'          # address of Aqualink controller
// 
//msg = DLE+STX+dest+cmd+args
//msg = msg+self.checksum(msg)+DLE+ETX
//      DLE+STX+DEST+CMD+ARGS+CHECKSUM+DLE+ETX

void print_hex(char* pk, int length)
{
	int i = 0;
	for (i = 0; i < length; i++)
	{
		printf("0x%02hhx|", pk[i]);
	}
	printf("\n");
}

/*
 unsigned char tp[] = {PCOL_PENTAIR, 0x07, 0x0F, 0x10, 0x08, 0x0D, 0x55, 0x55, 0x5B, 0x2A, 0x2B, 0x00, 0x00, 0x00, 0x00, 0x64, 0x00, 0x00, 0x00};
 send_command(0, tp, 19);
 Should produce
{0xFF, 0x00, 0xFF, 0xA5, 0x07, 0x0F, 0x10, 0x08, 0x0D, 0x55, 0x55, 0x5B, 0x2A, 0x2B, 0x00, 0x00, 0x00, 0x00, 0x64, 0x00, 0x00, 0x00, 0x02, 0x9E};
 <-------  headder ----> <-- type to from type-> <len> <------------------------------ data ----------------------------------------> <checksum>
*/

void send_pentair_command(SerialDevice serial_device, const unsigned char* packet_buffer, int size)
{
	unsigned char packet[AQ_MAXPKTLEN];
	int i = 0;

	packet[0] = NUL;
	packet[1] = PP1;
	packet[2] = PP2;
	packet[3] = PP3;
	packet[4] = PP4;

	for (i = 5; i - 5 < size; i++) {
		if (i == 6) {
			packet[i] = packet_buffer[i - 5];
		}
		else if (i == 9) {
			packet[i] = (unsigned char)size - 5;
		}
		else {
			packet[i] = packet_buffer[i - 5];
		}
	}

	packet[++i] = NUL;  // Checksum
	packet[++i] = NUL;  // Checksum
	generate_pentair_checksum(&packet[1], i);
	packet[++i] = NUL;

	send_packet(serial_device, packet, i);
}

void send_jandy_command(SerialDevice serial_device, const unsigned char* packet_buffer, int size)
{
	unsigned char packet[AQ_MAXPKTLEN];
	int i = 0;

	packet[0] = NUL;
	packet[1] = DLE;
	packet[2] = STX;

	for (i = 3; i - 3 < size; i++) {
		packet[i] = packet_buffer[i - 3];
	}

	packet[++i] = DLE;
	packet[++i] = ETX;
	packet[++i] = NUL;

	packet[i - 3] = generate_jandy_checksum(packet, i);

	send_packet(serial_device, packet, ++i);
}

void send_command(SerialDevice serial_device, const unsigned char* packet_buffer, int size)
{
	if (packet_buffer[0] == PCOL_JANDY) {
		send_jandy_command(serial_device, &packet_buffer[1], size - 1);
		return;
	}
	if (packet_buffer[0] == PCOL_PENTAIR) {
		send_pentair_command(serial_device, &packet_buffer[1], size - 1);
		return;
	}
}

void send_packet(SerialDevice serial_device, unsigned char* packet, int length)
{
	int nwrite, i;
	for (i = 0; i < length; i += nwrite) {
		nwrite = write_to_serial_device(serial_device, packet + i, length - i);
		if (nwrite < 0)
		{
			ERROR("write to serial port failed");
		}
	}

	if (CFG_LogLevel() > Debug) 
	{
		TRACE("Serial send %d bytes", length - 2);
		log_serial_packet(&packet[1], length - 2, false);
	}
}

void _send_ack(SerialDevice serial_device, unsigned char ack_type, unsigned char command)
{
	const int length = 11;
	// Default null ack with checksum generated, don't mess with it, just over right                    
	unsigned char ackPacket[] = { NUL, DLE, STX, DEV_MASTER, CMD_ACK, NUL, NUL, 0x13, DLE, ETX, NUL };

	// Update the packet and checksum if command argument is not NUL.
	if (command != NUL || ack_type != NUL) {
		//ackPacket[5] = 0x00 normal, 0x03 some pause, 0x01 some pause ending  (0x01 = Screen Busy (also return from logn message))
		ackPacket[5] = ack_type;
		ackPacket[6] = command;
		ackPacket[7] = generate_jandy_checksum(ackPacket, length - 1);
	}

	send_packet(serial_device, ackPacket, length);
}

void send_ack(SerialDevice serial_device, unsigned char command)
{
	_send_ack(serial_device, ACK_NORMAL, command);
}

// ack_typ should only be ACK_PDA, ACK_NORMAL, ACK_SCREEN_BUSY, ACK_SCREEN_BUSY_DISPLAY
void send_extended_ack(SerialDevice serial_device, unsigned char ack_type, unsigned char command)
{
	_send_ack(serial_device, ack_type, command);
}

int get_packet(SerialDevice serial_device, unsigned char* packet)
{
	return serial_getnextpacket(serial_device, packet);
}
int get_packet_lograw(SerialDevice serial_device, unsigned char* packet)
{
	return serial_getnextpacket(serial_device, packet);
}

#else // PLAYBACKMODE

#endif
