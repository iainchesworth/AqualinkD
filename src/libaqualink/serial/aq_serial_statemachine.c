#include "aq_serial_statemachine.h"

#include <assert.h>
#include <errno.h>
#include <stdbool.h>
#include <string.h>
#include <unistd.h>
#include "aq_serial.h"
#include "packetLogger.h"
#include "utils.h"

int serial_getnextpacket(int fd, unsigned char* packet)
{
	assert((0 <= fd));			// Look for a "valid" file descriptor
	assert((0 != packet));	// Ensure the destination packet buffer is not NULL

	static const int MAXIMUM_PAYLOAD_LENGTH = AQ_MAXPKTLEN - 4;  // 64 bytes minus DLE+STX / DLE+ETX
	static const int MINIMUM_PAYLOAD_LENGTH = 3;				 // Ignore DLE+STX | DEST + CMD + CSUM | Ignore DLE+ETX
	static const int ERROR_WHILE_RECEIVING_PACKET = 0;
	static const int ERROR_INVALID_PARAMETERS = 0;
	static const int MAXIMUM_RETRY_COUNT = 20;

	SerialThread_ReadStates state = ST_WAITFOR_PACKETSTART;

	unsigned char rawPacketBytes[AQ_MAXPKTLEN];
	unsigned char prevByte;

	int bytesRead, returnCodeOrBytesRead = ERROR_WHILE_RECEIVING_PACKET;
	int packetPayloadBytesRead = 0, retryCounter = 0, totalPacketBytesRead = 0;

	memset(rawPacketBytes, 0, AQ_MAXPKTLEN);

	do
	{
		switch (state)
		{
		case ST_WAITFOR_PACKETSTART:
			logMessage(LOG_DEBUG_SERIAL, "AQ_Serial.c | serial_getnextpacket | ST_WAITFOR_PACKETSTART\n");
			{
				// Clear the byte buffer to prevent any unintended data being copied.
				unsigned char byte = 0;

				// Get the next byte....note that this is non-blocking and will fall through.
				bytesRead = read(fd, &byte, 1);

				if ((bytesRead < 0) && ((EAGAIN == errno) || (EWOULDBLOCK == errno)))
				{
					// Portability Note: In many older Unix systems, this condition was indicated by EWOULDBLOCK, which was a distinct error
					// code different from EAGAIN.  To make a program portable, one should check for both codes and treat them the same way.

					// Nothing to do...so stop looking for serial data.
					logMessage(LOG_DEBUG_SERIAL, "AQ_Serial.c | serial_getnextpacket | Transition: ST_WAITFOR_PACKETSTART --> ST_TERMINATE_READPACKET\n");
					state = ST_TERMINATE_READPACKET;
				}
				else if ((bytesRead < 0) && (EBADF == errno))
				{
					// The file descriptor seems to have closed...that's bad but nothing can be done.
					logMessage(LOG_DEBUG_SERIAL, "AQ_Serial.c | serial_getnextpacket | Transition: ST_WAITFOR_PACKETSTART --> ST_ERROR_OCCURRED\n");
					state = ST_ERROR_OCCURRED;
				}
				else if ((1 == bytesRead) && (DLE == byte))
				{
					// This might be the first byte of a new packet...wait and check for the STX byte.
					prevByte = byte;
				}
				else if ((1 == bytesRead) && (STX == byte) && (DLE == prevByte))
				{
					// Valid packet start...transition and receive the packet payload and terminators.
					logMessage(LOG_DEBUG_SERIAL, "AQ_Serial.c | serial_getnextpacket | Transition: ST_WAITFOR_PACKETSTART --> ST_RECEIVE_PACKETPAYLOAD\n");
					state = ST_RECEIVE_PACKETPAYLOAD;

					// Clear the prevByte buffer to prevent data carry-over issues.
					prevByte = 0;
				}
				else if (1 == bytesRead)
				{
					// It was a valid byte but since we are waiting for a packet start...do nothing.
				}
				else
				{
					// Something unexpected/unplanned has happened....log and transition to error.
					logMessage(LOG_WARNING, "Unknown/unexpected error occured in ST_WAITFOR_PACKETSTART\n");
					logMessage(LOG_WARNING, "Read error: %d - %s\n", errno, strerror(errno));

					logMessage(LOG_DEBUG, "            %d bytes read in last read\n", bytesRead);
					logMessage(LOG_DEBUG, "            0x%02x read in last read\n", byte);

					logPacketError(rawPacketBytes, AQ_MAXPKTLEN);
					logMessage(LOG_DEBUG_SERIAL, "AQ_Serial.c | serial_getnextpacket | Transition: ST_WAITFOR_PACKETSTART --> ST_ERROR_OCCURRED\n");
					state = ST_ERROR_OCCURRED;
				}
			}
			break;

		case ST_RECEIVE_PACKETPAYLOAD:
			logMessage(LOG_DEBUG_SERIAL, "AQ_Serial.c | serial_getnextpacket | ST_RECEIVE_PACKETPAYLOAD\n");
			{
				// Clear the byte buffer to prevent any unintended data being copied.
				unsigned char byte = 0;

				// Get the next byte....note that this is non-blocking and will fall through.
				bytesRead = read(fd, &byte, 1);

				if ((bytesRead < 0) && ((EAGAIN == errno) || (EWOULDBLOCK == errno)))
				{
					// Portability Note: In many older Unix systems, this condition was indicated by EWOULDBLOCK, which was a distinct error
					// code different from EAGAIN.  To make a program portable, one should check for both codes and treat them the same way.

					// We were receiving a packet and something's happened...let's retry.
					logMessage(LOG_DEBUG_SERIAL, "AQ_Serial.c | serial_getnextpacket | Transition: ST_RECEIVE_PACKETPAYLOAD --> ST_RETRY_RECEIVEPAYLOAD\n");
					state = ST_RETRY_RECEIVEPAYLOAD;
				}
				else if ((bytesRead < 0) && (EBADF == errno))
				{
					// The file descriptor seems to have closed...that's bad but nothing can be done.
					logMessage(LOG_DEBUG_SERIAL, "AQ_Serial.c | serial_getnextpacket | Transition: ST_RETRY_RECEIVEPAYLOAD --> ST_ERROR_OCCURRED\n");
					state = ST_ERROR_OCCURRED;
				}
				else if ((1 == bytesRead) && (DLE == byte))
				{
					logMessage(LOG_DEBUG_SERIAL, "AQ_Serial.c | serial_getnextpacket | ST_RECEIVE_PACKETPAYLOAD - A - 0x%02x\n", byte);

					// This might be the last-but-one byte of the current packet...wait and check for the ETX byte.
					prevByte = byte;
				}
				else if ((1 == bytesRead) && (NUL == byte) && (DLE == prevByte))
				{
					logMessage(LOG_DEBUG_SERIAL, "AQ_Serial.c | serial_getnextpacket | ST_RECEIVE_PACKETPAYLOAD - B - 0x%02x\n", byte);

					// Okay, this is an escaped DLE that is merely part of the packet so keep processing bytes.

					rawPacketBytes[packetPayloadBytesRead + 2] = prevByte;  // Store the 0x10 byte value (i.e. same as the DTE)
					++packetPayloadBytesRead;

					rawPacketBytes[packetPayloadBytesRead + 2] = byte;		// Store the 0x00 byte value (i.e. same as the NUL)
					++packetPayloadBytesRead;

					prevByte = byte;
				}
				else if ((1 == bytesRead) && (ETX == byte) && (DLE == prevByte))
				{
					logMessage(LOG_DEBUG_SERIAL, "AQ_Serial.c | serial_getnextpacket | ST_RECEIVE_PACKETPAYLOAD - C - 0x%02x\n", byte);

					// Compute the total number of bytes in the "packet".
					totalPacketBytesRead = packetPayloadBytesRead + 4;

					// Okay...everything has finished...lets store the "packet".
					rawPacketBytes[0] = DLE;
					rawPacketBytes[1] = STX;
					// <-- this is the "as-read" payload and checksum bytes -->
					rawPacketBytes[packetPayloadBytesRead + 2 + 0] = DLE;
					rawPacketBytes[packetPayloadBytesRead + 2 + 1] = ETX;

					logMessage(LOG_DEBUG_SERIAL, "AQ_Serial.c | serial_getnextpacket | Transition: ST_RECEIVE_PACKETPAYLOAD --> ST_VALIDATE_PACKETPAYLOAD\n");
					state = ST_VALIDATE_PACKETPAYLOAD;

					// Clear the prevByte buffer to prevent data carry-over issues.
					prevByte = 0;
				}
				else if ((1 == bytesRead) && (MAXIMUM_PAYLOAD_LENGTH > packetPayloadBytesRead))
				{
					logMessage(LOG_DEBUG_SERIAL, "AQ_Serial.c | serial_getnextpacket | ST_RECEIVE_PACKETPAYLOAD - D - 0x%02x\n", byte);

					// A payload byte was received - store it away (in the payload section).
					rawPacketBytes[packetPayloadBytesRead + 2] = byte;
					++packetPayloadBytesRead;
				}
				else if (MAXIMUM_PAYLOAD_LENGTH <= packetPayloadBytesRead)
				{
					// The payload is too long.  Transition to error state and throw it all away.
					logMessage(LOG_WARNING, "Serial read too long\n");
					logPacketError(rawPacketBytes, AQ_MAXPKTLEN);
					logMessage(LOG_DEBUG_SERIAL, "AQ_Serial.c | serial_getnextpacket | Transition: ST_RECEIVE_PACKETPAYLOAD --> ST_ERROR_OCCURRED\n");
					state = ST_ERROR_OCCURRED;
				}
				else
				{
					// Something unexpected/unplanned has happened....log and transition to error.
					logMessage(LOG_WARNING, "Unknown/unexpected error occured in ST_RECEIVE_PACKETPAYLOAD\n");
					logPacketError(rawPacketBytes, AQ_MAXPKTLEN);

					logMessage(LOG_WARNING, "Read error: %d - %s\n", errno, strerror(errno));
					logMessage(LOG_DEBUG, "            %d bytes read in last read\n", bytesRead);
					logMessage(LOG_DEBUG, "            0x%02x read in last read\n", byte);

					logMessage(LOG_DEBUG_SERIAL, "AQ_Serial.c | serial_getnextpacket | Transition: ST_RECEIVE_PACKETPAYLOAD --> ST_ERROR_OCCURRED\n");
					state = ST_ERROR_OCCURRED;
				}
			}
			break;

		case ST_RETRY_RECEIVEPAYLOAD:
			logMessage(LOG_DEBUG_SERIAL, "AQ_Serial.c | serial_getnextpacket | ST_RETRY_RECEIVEPAYLOAD\n");
			{
				++retryCounter;

				if (MAXIMUM_RETRY_COUNT <= retryCounter)
				{
					logMessage(LOG_WARNING, "Serial read too short\n");
					logPacketError(rawPacketBytes, AQ_MAXPKTLEN);
					logMessage(LOG_DEBUG_SERIAL, "AQ_Serial.c | serial_getnextpacket | Transition: ST_RETRY_RECEIVEPAYLOAD --> ST_ERROR_OCCURRED\n");
					state = ST_ERROR_OCCURRED;
				}
				else
				{
					logMessage(LOG_DEBUG, "Serial read too short - retrying....\n");
					logMessage(LOG_DEBUG_SERIAL, "AQ_Serial.c | serial_getnextpacket | Transition: ST_RETRY_RECEIVEPAYLOAD --> ST_RECEIVE_PACKETPAYLOAD\n");
					state = ST_RECEIVE_PACKETPAYLOAD;

					// At 9600 baud, each character takes approx. 1.04 milliseconds.  Delay for a sensible amount.
					//delayMilliseconds(1);
				}
			}
			break;

		case ST_VALIDATE_PACKETPAYLOAD:
			logMessage(LOG_DEBUG_SERIAL, "AQ_Serial.c | serial_getnextpacket | ST_VALIDATE_PACKETPAYLOAD\n");
			{
				// Valid packet end...validate the packet (semantically) and transition as appropriate.
				const bool isPentairPacket = (4 <= packetPayloadBytesRead) && (PP1 == rawPacketBytes[2]) && (PP2 == rawPacketBytes[3]) && (PP3 == rawPacketBytes[4]) && (PP4 == rawPacketBytes[5]);
				const bool checksumIsValid = (isPentairPacket) ? check_pentair_checksum(rawPacketBytes, totalPacketBytesRead) : check_jandy_checksum(rawPacketBytes, totalPacketBytesRead);

				if (MINIMUM_PAYLOAD_LENGTH > packetPayloadBytesRead)
				{
					// The packet is too small...something has gone wrong so reject it.
					logMessage(LOG_WARNING, "Serial read too short\n");
					logPacketError(rawPacketBytes, AQ_MAXPKTLEN);
					logMessage(LOG_DEBUG_SERIAL, "AQ_Serial.c | serial_getnextpacket | Transition: ST_VALIDATE_PACKETPAYLOAD --> ST_ERROR_OCCURRED\n");
					state = ST_ERROR_OCCURRED;
				}
				else if (!checksumIsValid)
				{
					// The checksum for the packet is incorrect...something has gone wrong so reject it.
					logMessage(LOG_WARNING, (isPentairPacket) ? "Serial read bad Pentair checksum, ignoring\n" : "Serial read bad Jandy checksum, ignoring\n");
					logPacketError(rawPacketBytes, AQ_MAXPKTLEN);
					logMessage(LOG_DEBUG_SERIAL, "AQ_Serial.c | serial_getnextpacket | Transition: ST_VALIDATE_PACKETPAYLOAD --> ST_ERROR_OCCURRED\n");
					state = ST_ERROR_OCCURRED;
				}
				else
				{
					// Looks like we have a good, valid packet so return it back to the caller...
					memcpy(packet, rawPacketBytes, totalPacketBytesRead);
					returnCodeOrBytesRead = totalPacketBytesRead;
					logMessage(LOG_DEBUG_SERIAL, "AQ_Serial.c | serial_getnextpacket | Transition: ST_VALIDATE_PACKETPAYLOAD --> ST_TERMINATE_READPACKET\n");
					state = ST_TERMINATE_READPACKET;
				}
			}
			break;

		case ST_ERROR_OCCURRED:
			logMessage(LOG_DEBUG_SERIAL, "AQ_Serial.c | serial_getnextpacket | ST_ERROR_OCCURRED\n");
			logMessage(LOG_ERR, "Something has happened and the AQ_Serial packet process has entered an error state!\n");
			break;

		case ST_TERMINATE_READPACKET:
			logMessage(LOG_DEBUG_SERIAL, "AQ_Serial.c | serial_getnextpacket | ST_TERMINATE_READPACKET\n");
			break;

		default:
			logMessage(LOG_DEBUG_SERIAL, "AQ_Serial.c | serial_getnextpacket | UNKNOWN STATE\n");
			logMessage(LOG_ERR, "Something has happened and the AQ_Serial packet process has entered an unknown state!\n");
			logMessage(LOG_DEBUG_SERIAL, "AQ_Serial.c | serial_getnextpacket | Transition: UNKNOWN STATE --> ST_ERROR_OCCURRED\n");
			state = ST_ERROR_OCCURRED;
			break;
		}

		///TODO Make this loop act on blocking reads from the serial bus (i.e. no polling).

		// The loop will poll and chew as much CPU as it can.  Delay by a small, but prime, amount.
		delayMicroseconds(103);
	} while ((ST_TERMINATE_READPACKET != state) && (ST_ERROR_OCCURRED != state));

	// Return the number of bytes read.  Note this is set to 0 or -1 for error states.
	return returnCodeOrBytesRead;
}
