#include "aq_serial_writer_queue.h"

#include <assert.h>
#include <stdlib.h>

#include "logging/logging.h"
#include "serial/serializers/aq_serial_message_ack_serializer.h"

SerialWriter_SendQueue serial_device_send_queue =
{
	.Capacity = 0,

	.Tail = 0,
	.Head = 0,

	.IsFull = false,

	// .SlotMutex
	.SendSlots = 0
};

bool serial_writer_send_queue_initialise(const unsigned int maximum_size)
{
	assert(0 == serial_device_send_queue.SendSlots);
	assert(0 < maximum_size);

	bool send_queue_initialised_successfully = false;

	if (thrd_error == mtx_init(&serial_device_send_queue.SlotMutex, mtx_recursive | mtx_plain))
	{
		ERROR("Failed to create serial device send queue slot mutex");
	}
	else if (thrd_error == mtx_lock(&serial_device_send_queue.SlotMutex))
	{
		ERROR("Failed to lock the serial writer send queue slot mutex...cannot continue initiailing queue");
	}
	else if (0 == (serial_device_send_queue.SendSlots = (SerialWriter_SendQueueEntry*)malloc(maximum_size * sizeof(SerialWriter_SendQueueEntry))))
	{
		ERROR("Failed to allocate memory for the serial device send queue");
	}
	else
	{
		int slot_index;

		for (slot_index = 0; slot_index < maximum_size; ++slot_index)
		{
			serial_device_send_queue.SendSlots[slot_index].RawBytes = 0;
			serial_device_send_queue.SendSlots[slot_index].SlotIsReserved = false;
			serial_device_send_queue.SendSlots[slot_index].BytesDequeued = 0;
			serial_device_send_queue.SendSlots[slot_index].DataLength = 0;
		}

		serial_device_send_queue.Capacity = maximum_size;
		serial_device_send_queue.Head = 0;
		serial_device_send_queue.Tail = 0;
		serial_device_send_queue.IsFull = false;

		TRACE("Successfully initialised serial device send queue");
		send_queue_initialised_successfully = true;
	}

	if (thrd_error == mtx_unlock(&serial_device_send_queue.SlotMutex))
	{
		ERROR("Failed to unlock serial writer send queue slot mutex");
	}

	return send_queue_initialised_successfully;
}

void serial_writer_send_queue_destroy()
{
	if (thrd_error == mtx_lock(&serial_device_send_queue.SlotMutex))
	{
		ERROR("Failed to lock the serial writer send queue slot mutex...cannot destroy queue");
	}
	else
	{
		free(serial_device_send_queue.SendSlots);
		serial_device_send_queue.SendSlots = 0;
		
		TRACE("Successfully destroyed serial writer send queue slots");
	}

	if (thrd_error == mtx_unlock(&serial_device_send_queue.SlotMutex))
	{
		ERROR("Failed to unlock serial writer send queue slot mutex");
	}

}

int serial_writer_send_queue_used_entries()
{
	return serial_device_send_queue.Capacity - serial_writer_send_queue_empty_entries();
}

int serial_writer_send_queue_empty_entries()
{
	int numberOfFreeSlots = 0;

	if (serial_device_send_queue.IsFull)
	{
		// Queue is full so do nothing.
	}
	if (serial_device_send_queue.Tail == serial_device_send_queue.Head)
	{
		numberOfFreeSlots = serial_device_send_queue.Capacity;
	}
	else if (serial_device_send_queue.Head >= serial_device_send_queue.Tail)
	{
		numberOfFreeSlots = serial_device_send_queue.Head - serial_device_send_queue.Tail;
	}
	else
	{
		numberOfFreeSlots = serial_device_send_queue.Capacity + serial_device_send_queue.Head - serial_device_send_queue.Tail;
	}

	return numberOfFreeSlots;
}

int serial_writer_send_queue_total_entries()
{
	return serial_device_send_queue.Capacity;
}

static int serial_writer_send_queue_allocate_entry(unsigned int required_length)
{
	assert(0 < required_length);

	// Initialise the allocated slot to the error code.
	int slot_id = -1;

	if (thrd_error == mtx_lock(&serial_device_send_queue.SlotMutex))
	{
		ERROR("Failed to lock the serial writer send queue slot mutex...cannot continue allocating entry");
	}
	else if (serial_device_send_queue.IsFull)
	{
		WARN("Serial device send queue is at capacity but additional packets were sent.  Is %d slots too few?", serial_writer_send_queue_total_entries());
	}
	else if ((serial_device_send_queue.SendSlots[serial_device_send_queue.Head].SlotIsReserved) || (0 != serial_device_send_queue.SendSlots[serial_device_send_queue.Head].RawBytes))
	{
		ERROR("Collision while allocating a slot...the proposed slot is marked as reserved and/or has a buffer allocated");
	}
	else if (0 == (serial_device_send_queue.SendSlots[serial_device_send_queue.Head].RawBytes = (unsigned char*)malloc(required_length)))
	{
		ERROR("Failed to allocate memory while allocated slot in serial device send queue");
	}
	else
	{
		serial_device_send_queue.SendSlots[serial_device_send_queue.Head].SlotIsReserved = true;
		serial_device_send_queue.SendSlots[serial_device_send_queue.Head].DataLength = required_length;
		serial_device_send_queue.SendSlots[serial_device_send_queue.Head].BytesDequeued = 0;

		slot_id = serial_device_send_queue.Head;

		// Advance the head pointer to include this packet!
		serial_device_send_queue.Head = (serial_device_send_queue.Head + 1) % serial_device_send_queue.Capacity;
		serial_device_send_queue.IsFull = (serial_device_send_queue.Head == serial_device_send_queue.Tail);

		DEBUG("Successfully allocated serial device send slot %d", slot_id);
	}

	if (thrd_error == mtx_unlock(&serial_device_send_queue.SlotMutex))
	{
		ERROR("Failed to unlock serial writer send queue slot mutex");
	}

	return slot_id;
}

unsigned char serial_writer_dequeue_message_get_next_byte()
{
	unsigned char* byteArrayToSend;
	unsigned int byteIndexToSend;

	unsigned char byte_to_send = 0;

	if (thrd_error == mtx_lock(&serial_device_send_queue.SlotMutex))
	{
		ERROR("Failed to lock the serial writer send queue slot mutex...cannot continue dequeuing entry");
	}
	else if (0 == serial_device_send_queue.SendSlots[serial_device_send_queue.Tail].RawBytes)
	{
		ERROR("The serial writer send queue slot's raw byte pointer is not valid...cannot dequeue bytes");
	}
	else if (serial_device_send_queue.SendSlots[serial_device_send_queue.Tail].BytesDequeued >= serial_device_send_queue.SendSlots[serial_device_send_queue.Tail].DataLength)
	{
		ERROR("The serial writer send queue slot's entire byte buffer was dequeued however the slot is still indicated as valid for dequeuing...weird");
	}
	else
	{
		byteArrayToSend = serial_device_send_queue.SendSlots[serial_device_send_queue.Tail].RawBytes;
		byteIndexToSend = serial_device_send_queue.SendSlots[serial_device_send_queue.Tail].BytesDequeued;

		byte_to_send = byteArrayToSend[byteIndexToSend];

		// Move the "dequeue" pointer on by one.
		++serial_device_send_queue.SendSlots[serial_device_send_queue.Tail].BytesDequeued;

		if (serial_device_send_queue.SendSlots[serial_device_send_queue.Tail].BytesDequeued >= serial_device_send_queue.SendSlots[serial_device_send_queue.Tail].DataLength)
		{
			// Release the memory that was allocated for this slot.
			TRACE("The serial writer send queue slot's entire byte buffer was dequeued so deallocate the buffer");
			free(serial_device_send_queue.SendSlots[serial_device_send_queue.Tail].RawBytes);
			
			// All bytes from this message has been dequeued...remove it and move the tail on to the next slot.
			serial_device_send_queue.SendSlots[serial_device_send_queue.Tail].SlotIsReserved = false;
			serial_device_send_queue.SendSlots[serial_device_send_queue.Tail].DataLength = 0;
			serial_device_send_queue.SendSlots[serial_device_send_queue.Tail].BytesDequeued = 0;
			serial_device_send_queue.SendSlots[serial_device_send_queue.Tail].RawBytes = 0;

			DEBUG("Successfully deallocated serial device send slot %d", serial_device_send_queue.Tail);
			
			// Advance the tail pointer to past this packet!
			serial_device_send_queue.Tail = (serial_device_send_queue.Tail + 1) % serial_device_send_queue.Capacity;
			serial_device_send_queue.IsFull = false;
		}
	}

	if (thrd_error == mtx_unlock(&serial_device_send_queue.SlotMutex))
	{
		ERROR("Failed to unlock serial writer send queue slot mutex");
	}

	return byte_to_send;
}

bool serial_writer_enqueue_ack_message(AQ_Ack_Packet* ackPacket)
{
	assert(0 != ackPacket);

	bool packet_enqueued_successfully = false;
	int slot_id;

	if (thrd_error == mtx_lock(&serial_device_send_queue.SlotMutex))
	{
		ERROR("Failed to lock the serial writer send queue slot mutex...cannot continue allocating entry");
	}
	else if (serial_device_send_queue.IsFull)
	{
		WARN("Serial device send queue is at capacity but additional packets were sent.  Is %d slots too few?", serial_writer_send_queue_total_entries());
	}
	else if (-1 == (slot_id = serial_writer_send_queue_allocate_entry(AQ_ACK_PACKET_LENGTH)))
	{
		ERROR("Failed to allocate a send queue slot in the writer send queue for ACK packet");
	}
	else
	{
		// NOTE: A slot has been allocated in the send buffer...it will be sent in due course.
		//
		// All that is left is to serialse the bytes into the destination buffer (allocated 
		// specially during the original serial_writer_send_queue_allocate_entry() call.

		serialize_ack_packet(ackPacket, serial_device_send_queue.SendSlots[slot_id].RawBytes, AQ_ACK_PACKET_LENGTH);

		DEBUG("Enqueued RESPONSE_ACK packet");
	}

	if (thrd_error == mtx_unlock(&serial_device_send_queue.SlotMutex))
	{
		ERROR("Failed to unlock serial writer send queue slot mutex");
	}

	return packet_enqueued_successfully;
}
