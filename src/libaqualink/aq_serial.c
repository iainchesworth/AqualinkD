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

#include <stdio.h>
#include <stdarg.h>
#include <termios.h>
#include <fcntl.h>
#include <unistd.h>
#include <string.h>
#include <errno.h>
#include <string.h>
#include <sys/ioctl.h>
#include <stdbool.h>

#include "aq_serial.h"
#include "aq_serial_statemachine.h"
#include "utils.h"
#include "config.h"
#include "packetLogger.h"

static struct termios _oldtio;

void send_packet(int fd, unsigned char* packet, int length);

bool _pda_mode = false;

void set_pda_mode(bool mode)
{
	if (mode) {
		logMessage(LOG_NOTICE, "AqualinkD is using PDA mode\n");
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
		return "";

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

// Generate and return checksum of packet.
int generate_checksum(unsigned char* packet, int length)
{
	int i, sum, n;

	n = length - 3;
	sum = 0;
	for (i = 0; i < n; i++)
		sum += (int)packet[i];
	return(sum & 0x0ff);
}

bool check_jandy_checksum(unsigned char* packet, int length)
{
	if (generate_checksum(packet, length) == packet[length - 3])
		return true;

	return false;
}

bool check_pentair_checksum(unsigned char* packet, int length)
{
	int i, sum, n;
	n = packet[8] + 9;
	sum = 0;
	for (i = 3; i < n; i++) {
		sum += (int)packet[i];
	}

	// Check against calculated length
	if (sum == (packet[length - 2] * 256 + packet[length - 1]))
		return true;

	// Check against actual # length
	if (sum == (packet[n] * 256 + packet[n + 1])) {
		logMessage(LOG_ERR, "Pentair checksum is accurate but length is not\n");
		return true;
	}

	return false;
}

void generate_pentair_checksum(unsigned char* packet, int length)
{
	int i, sum, n;
	n = packet[8] + 9;
	sum = 0;
	for (i = 3; i < n; i++) {
		sum += (int)packet[i];
	}

	packet[n + 1] = (unsigned char)(sum & 0xFF);        // Low Byte
	packet[n] = (unsigned char)((sum >> 8) & 0xFF); // High Byte

}

protocolType getProtocolType(unsigned char* packet) {
	if (packet[0] == DLE)
		return JANDY;
	else if (packet[0] == PP1)
		return PENTAIR;

	return P_UNKNOWN;
}

#ifndef PLAYBACK_MODE
/*
Open and Initialize the serial communications port to the Aqualink RS8 device.
Arg is tty or port designation string
returns the file descriptor
*/
int init_serial_port(const char* tty)
{
	long BAUD = B9600;
	long DATABITS = CS8;
	long STOPBITS = 0;
	long PARITYON = 0;
	long PARITY = 0;

	struct termios newtio;       //place for old and new port settings for serial port

	int fd = open(tty, O_RDWR | O_NOCTTY | O_NONBLOCK | O_NDELAY);
	if (fd < 0) {
		logMessage(LOG_ERR, "Unable to open port: %s\n", tty);
		return -1;
	}

	logMessage(LOG_DEBUG_SERIAL, "Openeded serial port %s\n", tty);

	int flags = fcntl(fd, F_GETFL, 0);
	fcntl(fd, F_SETFL, flags | O_NONBLOCK | O_NDELAY);
	newtio.c_cc[VMIN] = 0;
	newtio.c_cc[VTIME] = 1;
	logMessage(LOG_DEBUG_SERIAL, "Set serial port %s to non blocking mode\n", tty);

	tcgetattr(fd, &_oldtio); // save current port settings
							
	// set new port settings for canonical input processing
	newtio.c_cflag = BAUD | DATABITS | STOPBITS | PARITYON | PARITY | CLOCAL | CREAD;
	newtio.c_iflag = IGNPAR;
	newtio.c_lflag = 0;       // ICANON;  
	newtio.c_oflag = 0;

	tcflush(fd, TCIFLUSH);
	tcsetattr(fd, TCSANOW, &newtio);

	logMessage(LOG_DEBUG_SERIAL, "Set serial port %s io attributes\n", tty);

	return fd;
}

/* close tty port */
void close_serial_port(int fd)
{
	tcsetattr(fd, TCSANOW, &_oldtio);
	close(fd);
	logMessage(LOG_DEBUG_SERIAL, "Closed serial port\n");
}

// Send an ack packet to the Aqualink RS8 master device.
// fd: the file descriptor of the serial port connected to the device
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

void send_pentair_command(int fd, unsigned char* packet_buffer, int size)
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

	send_packet(fd, packet, i);
}

void send_jandy_command(int fd, unsigned char* packet_buffer, int size)
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

	packet[i - 3] = generate_checksum(packet, i);

	send_packet(fd, packet, ++i);
}

void send_command(int fd, unsigned char* packet_buffer, int size)
{
	if (packet_buffer[0] == PCOL_JANDY) {
		send_jandy_command(fd, &packet_buffer[1], size - 1);
		return;
	}
	if (packet_buffer[0] == PCOL_PENTAIR) {
		send_pentair_command(fd, &packet_buffer[1], size - 1);
		return;
	}
}

void send_packet(int fd, unsigned char* packet, int length)
{
	int nwrite, i;
	for (i = 0; i < length; i += nwrite) {
		nwrite = write(fd, packet + i, length - i);
		if (nwrite < 0)
			logMessage(LOG_ERR, "write to serial port failed\n");
	}

	if (getLogLevel() >= LOG_DEBUG_SERIAL) {
		logMessage(LOG_DEBUG_SERIAL, "Serial send %d bytes\n", length - 2);
		logPacket(&packet[1], length - 2);
	}
}

void _send_ack(int fd, unsigned char ack_type, unsigned char command)
{
	const int length = 11;
	// Default null ack with checksum generated, don't mess with it, just over right                    
	unsigned char ackPacket[] = { NUL, DLE, STX, DEV_MASTER, CMD_ACK, NUL, NUL, 0x13, DLE, ETX, NUL };

	// Update the packet and checksum if command argument is not NUL.
	if (command != NUL || ack_type != NUL) {
		//ackPacket[5] = 0x00 normal, 0x03 some pause, 0x01 some pause ending  (0x01 = Screen Busy (also return from logn message))
		ackPacket[5] = ack_type;
		ackPacket[6] = command;
		ackPacket[7] = generate_checksum(ackPacket, length - 1);
	}

	send_packet(fd, ackPacket, length);
}

void send_ack(int fd, unsigned char command)
{
	_send_ack(fd, ACK_NORMAL, command);
}

// ack_typ should only be ACK_PDA, ACK_NORMAL, ACK_SCREEN_BUSY, ACK_SCREEN_BUSY_DISPLAY
void send_extended_ack(int fd, unsigned char ack_type, unsigned char command)
{
	_send_ack(fd, ack_type, command);
}

int _get_packet(int fd, unsigned char* packet, bool rawlog);

int get_packet(int fd, unsigned char* packet)
{
	return _get_packet(fd, packet, false);
}
int get_packet_lograw(int fd, unsigned char* packet)
{
	return _get_packet(fd, packet, true);
}

int _get_packet(int fd, unsigned char* packet, bool rawlog)
{
	assert((0 <= fd));			// Look for a "valid" file descriptor
	assert((NULL != packet));	// Ensure the destination packet buffer is not NULL

	static const int MAXIMUM_PAYLOAD_LENGTH = AQ_MAXPKTLEN - 4;  // 64 bytes minus DLE+STX / DLE+ETX
	static const int MINIMUM_PAYLOAD_LENGTH = 3;				 // Ignore DLE+STX | DEST + CMD + CSUM | Ignore DLE+ETX
	static const int ERROR_WHILE_RECEIVING_PACKET = 0;
	static const int ERROR_INVALID_PARAMETERS = 0;
	static const int MAXIMUM_RETRY_COUNT = 20;

	SerialFsm_States state = ST_WAITFOR_PACKETSTART;

	unsigned char rawPacketBytes[AQ_MAXPKTLEN];
	unsigned char byte, prevByte;

	int bytesRead, returnCode = ERROR_WHILE_RECEIVING_PACKET;
	int packetPayloadBytesRead = 0, retryCounter = 0, totalPacketBytesRead = 0;

	memset(rawPacketBytes, 0, AQ_MAXPKTLEN);

	do
	{
		switch (state)
		{
		case ST_WAITFOR_PACKETSTART:
			logMessage(LOG_DEBUG, "AQ_Serial.c | _get_packet | ST_WAITFOR_PACKETSTART\n");
			{
				// Get the next byte....note that this is non-blocking and will fall through.
				bytesRead = read(fd, &byte, 1);

				if ((bytesRead < 0) && (EAGAIN == errno))
				{
					// Nothing to do...so stop looking for serial data.
					logMessage(LOG_DEBUG, "AQ_Serial.c | _get_packet | Transition: ST_WAITFOR_PACKETSTART --> ST_TERMINATE\n");
					state = ST_TERMINATE;
				}
				else if ((1 == bytesRead) && (DLE == byte))
				{
					// This might be the first byte of a new packet...wait and check for the STX byte.
					prevByte = byte;
				}
				else if ((1 == bytesRead) && (STX == byte) && (DLE == prevByte))
				{
					// Valid packet start...transition and receive the packet payload and terminators.
					logMessage(LOG_DEBUG, "AQ_Serial.c | _get_packet | Transition: ST_WAITFOR_PACKETSTART --> ST_RECEIVE_PACKETPAYLOAD\n");
					state = ST_RECEIVE_PACKETPAYLOAD;
				}
				else
				{
					// Something unexpected/unplanned has happened....log and transition to error.
					logMessage(LOG_WARNING, "Unknown/unexpected error occured in ST_WAITFOR_PACKETSTART\n");
					logPacketError(rawPacketBytes, AQ_MAXPKTLEN);
					logMessage(LOG_DEBUG, "AQ_Serial.c | _get_packet | Transition: ST_WAITFOR_PACKETSTART --> ST_ERROR_OCCURRED\n");
					state = ST_ERROR_OCCURRED;
				}
			}
			break;

		case ST_RECEIVE_PACKETPAYLOAD:
			logMessage(LOG_DEBUG, "AQ_Serial.c | _get_packet | ST_RECEIVE_PACKETPAYLOAD\n");
			{
				// Get the next byte....note that this is non-blocking and will fall through.
				bytesRead = read(fd, &byte, 1);

				if (bytesRead < 0 && EAGAIN == errno)
				{
					// We were receiving a packet and something's happened...let's retry.
					logMessage(LOG_DEBUG, "AQ_Serial.c | _get_packet | Transition: ST_RECEIVE_PACKETPAYLOAD --> ST_RETRY_RECEIVEPAYLOAD\n");
					state = ST_RETRY_RECEIVEPAYLOAD;
				}
				else if ((1 == bytesRead) && (DLE == byte))
				{
					// This might be the last-but-one byte of the current packet...wait and check for the ETX byte.
					prevByte = byte;
				}
				else if ((1 == bytesRead) && (ETX == byte) && (DLE == prevByte))
				{
					// Compute the total number of bytes in the "packet".
					totalPacketBytesRead = packetPayloadBytesRead + 4;

					// Okay...everything has finished...lets store the "packet".
					rawPacketBytes[0] = DLE;
					rawPacketBytes[1] = STX;
					// <-- this is the "as-read" payload and checksum bytes -->
					rawPacketBytes[packetPayloadBytesRead + 2 + 1] = DLE;
					rawPacketBytes[packetPayloadBytesRead + 2 + 2] = ETX;

					logMessage(LOG_DEBUG, "AQ_Serial.c | _get_packet | Transition: ST_RECEIVE_PACKETPAYLOAD --> ST_VALIDATE_PACKETPAYLOAD\n");
					state = ST_VALIDATE_PACKETPAYLOAD;
				}
				else if ((1 == bytesRead) && (MAXIMUM_PAYLOAD_LENGTH > packetPayloadBytesRead))
				{
					// A payload byte was received - store it away (in the payload section).
					rawPacketBytes[packetPayloadBytesRead + 2] = byte;
					++packetPayloadBytesRead;
				}
				else if (MAXIMUM_PAYLOAD_LENGTH <= packetPayloadBytesRead)
				{
					// The payload is too long.  Transition to error state and throw it all away.
					logMessage(LOG_WARNING, "Serial read too long\n");
					logPacketError(rawPacketBytes, AQ_MAXPKTLEN);
					logMessage(LOG_DEBUG, "AQ_Serial.c | _get_packet | Transition: ST_RECEIVE_PACKETPAYLOAD --> ST_ERROR_OCCURRED\n");
					state = ST_ERROR_OCCURRED;
				}
				else
				{
					// Something unexpected/unplanned has happened....log and transition to error.
					logMessage(LOG_WARNING, "Unknown/unexpected error occured in ST_RECEIVE_PACKETPAYLOAD\n");
					logPacketError(rawPacketBytes, AQ_MAXPKTLEN);
					logMessage(LOG_DEBUG, "AQ_Serial.c | _get_packet | Transition: ST_RECEIVE_PACKETPAYLOAD --> ST_ERROR_OCCURRED\n");
					state = ST_ERROR_OCCURRED;
				}
			}
			break;

		case ST_RETRY_RECEIVEPAYLOAD:
			logMessage(LOG_DEBUG, "AQ_Serial.c | _get_packet | ST_RETRY_RECEIVEPAYLOAD\n");
			{
				++retryCounter;

				if (MAXIMUM_RETRY_COUNT <= retryCounter)
				{
					logMessage(LOG_WARNING, "Serial read too short\n");
					logPacketError(rawPacketBytes, AQ_MAXPKTLEN);
					logMessage(LOG_DEBUG, "AQ_Serial.c | _get_packet | Transition: ST_RETRY_RECEIVEPAYLOAD --> ST_ERROR_OCCURRED\n");
					state = ST_ERROR_OCCURRED;
				}
				else
				{
					logMessage(LOG_DEBUG, "Serial read too short - retrying....\n");
					logMessage(LOG_DEBUG, "AQ_Serial.c | _get_packet | Transition: ST_RETRY_RECEIVEPAYLOAD --> ST_RECEIVE_PACKETPAYLOAD\n");
					state = ST_RECEIVE_PACKETPAYLOAD;

					// Delay for 0.1s
				}
			}
			break;

		case ST_VALIDATE_PACKETPAYLOAD:
			logMessage(LOG_DEBUG, "AQ_Serial.c | _get_packet | ST_VALIDATE_PACKETPAYLOAD\n");
			{
				// Valid packet end...validate the packet (semantically) and transition as appropriate.
				const bool isPentairPacket = (4 <= packetPayloadBytesRead) && (PP1 == rawPacketBytes[2]) && (PP2 == rawPacketBytes[3]) && (PP3 == rawPacketBytes[4]) && (PP4 == rawPacketBytes[5]);
				const bool checksumIsValid = (isPentairPacket) ? check_pentair_checksum(rawPacketBytes, totalPacketBytesRead) : check_jandy_checksum(rawPacketBytes, totalPacketBytesRead);

				if (MINIMUM_PAYLOAD_LENGTH > packetPayloadBytesRead)
				{
					// The packet is too small...something has gone wrong so reject it.
					logMessage(LOG_WARNING, (isPentairPacket) ? "Serial read bad Pentair checksum, ignoring\n" : "Serial read bad Jandy checksum, ignoring\n");
					logPacketError(rawPacketBytes, AQ_MAXPKTLEN);
					logMessage(LOG_DEBUG, "AQ_Serial.c | _get_packet | Transition: ST_VALIDATE_PACKETPAYLOAD --> ST_ERROR_OCCURRED\n");
					state = ST_ERROR_OCCURRED;
				}
				else if (!checksumIsValid)
				{
					// The checksum for the packet is incorrect...something has gone wrong so reject it.
					logMessage(LOG_WARNING, "Serial read too short\n");
					logPacketError(rawPacketBytes, AQ_MAXPKTLEN);
					logMessage(LOG_DEBUG, "AQ_Serial.c | _get_packet | Transition: ST_VALIDATE_PACKETPAYLOAD --> ST_ERROR_OCCURRED\n");
					state = ST_ERROR_OCCURRED;
				}
				else
				{
					// Looks like we have a good, valid packet so return it back to the caller...
					memcpy(packet, rawPacketBytes, totalPacketBytesRead);
					returnCode = totalPacketBytesRead;
					logMessage(LOG_DEBUG, "AQ_Serial.c | _get_packet | Transition: ST_VALIDATE_PACKETPAYLOAD --> ST_TERMINATE\n");
					state = ST_TERMINATE;
				}
			}
			break;

		case ST_ERROR_OCCURRED:
			logMessage(LOG_ERR, "AQ_Serial.c | _get_packet | ST_ERROR_OCCURRED\n");
			logMessage(LOG_ERR, "Something has happened and the AQ_Serial packet process has entered an error state!\n");
			break;

		case ST_TERMINATE:
			logMessage(LOG_DEBUG, "AQ_Serial.c | _get_packet | ST_TERMINATE\n");
			break;

		default:
			logMessage(LOG_ERR, "AQ_Serial.c | _get_packet | UNKNOWN STATE\n");
			logMessage(LOG_ERR, "Something has happened and the AQ_Serial packet process has entered an unknown state!\n");
			logMessage(LOG_DEBUG, "AQ_Serial.c | _get_packet | Transition: UNKNOWN STATE --> ST_ERROR_OCCURRED\n");
			state = ST_ERROR_OCCURRED;
			break;
		}
	} 
	while (ST_TERMINATE != state || ST_ERROR_OCCURRED != state);

	// Return the number of bytes read.  Note this is set to 0 or -1 for error states.
	return returnCode;
}

/*
int _get_packet(int fd, unsigned char* packet, bool rawlog)
{
	unsigned char byte;
	int bytesRead;
	int index = 0;
	bool endOfPacket = false;
	bool lastByteDLE = false;
	int retry = 0;
	bool jandyPacketStarted = false;
	bool pentairPacketStarted = false;
	int PentairPreCnt = 0;
	int PentairDataCnt = -1;

	// Read packet in byte order below
	// DLE STX ........ ETX DLE
	// sometimes we get ETX DLE and no start, so for now just ignoring that.  Seem to be more applicable when busy RS485 traffic

	while (!endOfPacket) 
	{
		bytesRead = read(fd, &byte, 1);
		if (bytesRead < 0 && errno == EAGAIN && jandyPacketStarted == false && pentairPacketStarted == false && lastByteDLE == false) 
		{
			// We just have nothing to read
			return 0;
		}
		else if (bytesRead < 0 && errno == EAGAIN) 
		{
			// If we are in the middle of reading a packet, keep going
			if (retry > 20) {
				logMessage(LOG_WARNING, "Serial read timeout\n");
				logPacketError(packet, index);
				return 0;
			}
			retry++;
			delay(10);

#ifdef TESTING
		}
		else if (bytesRead == 0 && jandyPacketStarted == false && pentairPacketStarted == false) {
			// Probably set port to /dev/null for testing.
			//printf("Read loop return\n");
			return 0;
#endif

		}
		else if (bytesRead == 1) 
		{
			if (rawlog) {
				logPacketByte(&byte);
			}

			if (lastByteDLE == true && byte == NUL)
			{
				// Check for DLE | NULL (that's escape DLE so delete the NULL)
				lastByteDLE = false;
			}
			else if (lastByteDLE == true)
			{
				if (index == 0) {
					index++;
				}

				packet[index] = byte;
				index++;

				if (byte == STX && jandyPacketStarted == false)
				{
					jandyPacketStarted = true;
					pentairPacketStarted = false;
				}
				else if (byte == ETX && jandyPacketStarted == true)
				{
					endOfPacket = true;
				}
			}
			else if (jandyPacketStarted || pentairPacketStarted)
			{
				packet[index] = byte;
				index++;
				if (pentairPacketStarted == true && index == 9)
				{
					PentairDataCnt = byte;
				}
				if (PentairDataCnt >= 0 && index - 11 >= PentairDataCnt && pentairPacketStarted == true)
				{
					endOfPacket = true;
					PentairPreCnt = -1;
				}
			}
			else if (byte == DLE && jandyPacketStarted == false)
			{
				packet[index] = byte;
			}

			// // reset index incase we have EOP before start
			if (jandyPacketStarted == false && pentairPacketStarted == false)
			{
				index = 0;
			}

			if (byte == DLE && pentairPacketStarted == false)
			{
				lastByteDLE = true;
				PentairPreCnt = -1;
			}
			else
			{
				lastByteDLE = false;
				if (byte == PP1 && PentairPreCnt == 0)
					PentairPreCnt = 1;
				else if (byte == PP2 && PentairPreCnt == 1)
					PentairPreCnt = 2;
				else if (byte == PP3 && PentairPreCnt == 2)
					PentairPreCnt = 3;
				else if (byte == PP4 && PentairPreCnt == 3)
				{
					pentairPacketStarted = true;
					jandyPacketStarted = false;
					PentairDataCnt = -1;
					packet[0] = PP1;
					packet[1] = PP2;
					packet[2] = PP3;
					packet[3] = byte;
					index = 4;
				}
				else if (byte != PP1) // Don't reset counter if multiple PP1's
					PentairPreCnt = 0;
			}
		}
		else if (bytesRead < 0) {
			// Got a read error. Wait one millisecond for the next byte to
			// arrive.
			logMessage(LOG_WARNING, "Read error: %d - %s\n", errno, strerror(errno));
			if (errno == 9) {
				// Bad file descriptor. Port has been disconnected for some reason.
				// Return a -1.
				return -1;
			}
			delay(100);
		}

		// Break out of the loop if we exceed maximum packet
		// length.
		if (index >= AQ_MAXPKTLEN) {
			logPacketError(packet, index);
			logMessage(LOG_WARNING, "Serial packet too large\n");
			return 0;
			break;
		}
	}

	if (jandyPacketStarted) {
		if (check_jandy_checksum(packet, index) != true) {
			logPacketError(packet, index);
			logMessage(LOG_WARNING, "Serial read bad Jandy checksum, ignoring\n");
			return 0;
		}
	}
	else if (pentairPacketStarted) {
		if (check_pentair_checksum(packet, index) != true) {
			logPacketError(packet, index);
			logMessage(LOG_WARNING, "Serial read bad Pentair checksum, ignoring\n");
			return 0;
		}
	}

	if (index < AQ_MINPKTLEN && (jandyPacketStarted || pentairPacketStarted)) { //NSF. Sometimes we get END sequence only, so just ignore.
		logPacketError(packet, index);
		logMessage(LOG_WARNING, "Serial read too small\n");
		return 0;
	}

	if (getLogLevel() >= LOG_DEBUG_SERIAL) {
		logMessage(LOG_DEBUG_SERIAL, "Serial read %d bytes\n", index);
		logPacket(packet, index);
	}
	// Return the packet length.
	return index;
}

*/

#else // PLAYBACKMODE

#endif
