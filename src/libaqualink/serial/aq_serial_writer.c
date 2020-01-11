#include "aq_serial_writer.h"

#include <assert.h>

#include "cross-platform/serial.h"
#include "logging/logging.h"
#include "serial/aq_serial_writer_queue.h"
#include "aq_serial_data_logger.h"
#include "aq_serial_types.h"

int serial_sendnextpacket(SerialDevice serial_device)
{
	assert((SERIALDEVICE_INVALID != serial_device));	// Look for a "valid" file descriptor

	static const int ERROR_WHILE_SENDING_PACKET = 0;
	static const unsigned char NUL_BYTE_TO_SEND = NUL;

	int bytesSent, returnCodeOrBytesSent = ERROR_WHILE_SENDING_PACKET;

	SerialThread_WriteStates state = ST_SEND_PACKETPAYLOAD;

	unsigned char prevByte = 0;

	set_verbosity(&aqualink_default_logger, Trace);

	do
	{
		switch (state)
		{
		case ST_SEND_PACKETPAYLOAD:
			TRACE("ST_SEND_PACKETPAYLOAD");
			{
				if (serial_writer_send_queue_total_entries() == serial_writer_send_queue_empty_entries())
				{
					TRACE("Was asked to write data but there isn't any to send...weird");

					// There are no more packets (or bytes) to send so finish writing data.
					TRACE("Transition: ST_SEND_PACKETPAYLOAD --> ST_TERMINATE_WRITEPACKET");
					state = ST_TERMINATE_WRITEPACKET;
				}
				else
				{
					// Get the next byte that is available to be sent.
					unsigned char byte = serial_writer_dequeue_message_get_next_byte();

					///FIXME what if there's no bytes to send?

					if ((STX == byte) && (DLE == prevByte))
					{
						// This is the packet start sequence...DLE+STX - just send as is.

						// Clear the prevByte buffer to prevent data carry-over issues.
						prevByte = 0;
					}
					else if ((ETX == byte) && (DLE == prevByte))
					{
						// This is the packet end sequence bytes...DLE+ETX - just send as is.

						// Clear the prevByte buffer to prevent data carry-over issues.
						prevByte = 0;
					}
					else if (DLE == prevByte)
					{
						// NOTE: All non-start and non-end bytes with value DLE (0x10) need to be
						// escaped with a following NUL byte.

						// Get the next byte....note that this is blocking and wait for data to send.
						bytesSent = write_to_serial_device(serial_device, &NUL_BYTE_TO_SEND, 1);

						///FIXME what if the write failed?

						// Record the number of bytes sent (incrementally).
						returnCodeOrBytesSent += (bytesSent < 0) ? 0 : bytesSent;

						// Log the raw byte (which will go out to file if initialised)...
						TRACE_TO(&aq_serial_data_logger, "%d", NUL_BYTE_TO_SEND);

						TRACE("ST_SEND_PACKETPAYLOAD - NUL - 0x%02x", NUL_BYTE_TO_SEND);

						// Clear the prevByte buffer to prevent data carry-over issues.
						prevByte = 0;
					}
					else
					{
						// Just a "regular" byte.  No special pre-processing is required.
					}

					// Get the next byte....note that this is blocking and wait for data to send.
					bytesSent = write_to_serial_device(serial_device, &byte, 1);

					if ((bytesSent < 0) && (EBADF == errno))
					{
						// The file descriptor seems to have closed...that's bad but nothing can be done.
						TRACE("Transition: ST_SEND_PACKETPAYLOAD --> ST_WRITEERROR_OCCURRED");
						state = ST_WRITEERROR_OCCURRED;
					}
					else if ((1 == bytesSent) && (DLE == byte))
					{
						// Log the raw byte (which will go out to file if initialised)...
						TRACE_TO(&aq_serial_data_logger, "%d", byte);

						TRACE("ST_SEND_PACKETPAYLOAD - DLE - 0x%02x", byte);

						// This might be the first/last-but-one byte of the current packet...wait and check for the STX/ETX byte.
						prevByte = byte;
					}
					else if (1 == bytesSent)
					{
						// Log the raw byte (which will go out to file if initialised)...
						TRACE_TO(&aq_serial_data_logger, "%d", byte);

						TRACE("ST_SEND_PACKETPAYLOAD - process0x%02x", byte);

						if (serial_writer_send_queue_total_entries() == serial_writer_send_queue_empty_entries())
						{
							// There are no more packets (or bytes) to send so finish writing data.
							TRACE("Transition: ST_SEND_PACKETPAYLOAD --> ST_TERMINATE_WRITEPACKET");
							state = ST_TERMINATE_WRITEPACKET;
						}

						// Record the number of bytes sent (incrementally).
						returnCodeOrBytesSent += bytesSent;
					}
					else
					{
						///FIXME log_serial_packet(rawPacketBytes, AQ_MAXPKTLEN, true);

						// Something unexpected/unplanned has happened....log and transition to error.
						WARN("Unknown/unexpected error occured in ST_SEND_PACKETPAYLOAD");
						WARN("Write error: %d - %s", errno, strerror(errno));

						DEBUG("             %d bytes written in last write", bytesSent);
						DEBUG("             0x%02x written in last write", byte);

						TRACE("Transition: ST_SEND_PACKETPAYLOAD --> ST_WRITEERROR_OCCURRED");
						state = ST_WRITEERROR_OCCURRED;
					}
				}
			}
			break;

		case ST_RETRY_SENDPAYLOAD:
			TRACE("ST_RETRY_SENDPAYLOAD");
			{
			}
			break;

		case ST_WRITEERROR_OCCURRED:
			TRACE("ST_WRITEERROR_OCCURRED");
			ERROR("Something has happened and the AQ_Serial packet process has entered an error state!");
			break;

		case ST_TERMINATE_WRITEPACKET:
			TRACE("ST_TERMINATE_WRITEPACKET");
			break;

		default:
			TRACE("UNKNOWN STATE");
			ERROR("Something has happened and the AQ_Serial packet process has entered an unknown state!");
			TRACE("Transition: UNKNOWN STATE --> ST_WRITEERROR_OCCURRED");
			state = ST_WRITEERROR_OCCURRED;
			break;
		}
	} 
	while ((ST_TERMINATE_WRITEPACKET != state) && (ST_WRITEERROR_OCCURRED != state));

	set_verbosity(&aqualink_default_logger, Debug);

	// Return the number of bytes read.  Note this is set to 0 or -1 for error states.
	return returnCodeOrBytesSent;
}
