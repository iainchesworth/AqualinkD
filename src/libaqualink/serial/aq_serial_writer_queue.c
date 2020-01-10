#include "aq_serial_writer_queue.h"

#include <assert.h>
#include <stdlib.h>

#include "logging/logging.h"
#include "serial/serializers/aq_serial_message_ack_serializer.h"

SerialWriter_SendQueue serial_device_send_queue;

bool initialise_serial_writer_send_queue(const unsigned int maximum_size)
{
	assert(0 == serial_device_send_queue.SendSlots);
	assert(0 < maximum_size);

	bool send_queue_initialised_successfully = false;

	if (thrd_error == mtx_init(&serial_device_send_queue.SlotMutex, mtx_recursive | mtx_plain))
	{
		ERROR("Failed to create serial device send queue slot mutex");
	}
	else if (0 == (serial_device_send_queue.SendSlots = (SerialWriter_SendQueueEntry*)malloc(maximum_size * sizeof(SerialWriter_SendQueueEntry))))
	{
		ERROR("Failed to allocate memory for the serial device send queue");
	}
	else if (thrd_error == mtx_lock(&serial_device_send_queue.SlotMutex))
	{
		ERROR("Failed to lock the serial writer send queue slot mutex...cannot continue initiailing queue");
	}
	else
	{
		int slot_index;

		for (slot_index = 0; slot_index < maximum_size; ++slot_index)
		{
			serial_device_send_queue.SendSlots[slot_index].RawBytes = 0;
			serial_device_send_queue.SendSlots[slot_index].SlotIsReserved = false;
		}

		if (thrd_error == mtx_unlock(&serial_device_send_queue.SlotMutex))
		{
			ERROR("Failed to unlock serial writer send queue slot mutex");
		}

		serial_device_send_queue.Capacity = maximum_size;
		serial_device_send_queue.Head = 0;
		serial_device_send_queue.Tail = 0;
		serial_device_send_queue.IsFull = false;


		TRACE("Successfully initialised serial device send queue");
		send_queue_initialised_successfully = true;
	}

	return send_queue_initialised_successfully;
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

static int serial_writer_send_queue_allocate_entry(unsigned int required_length)
{
	assert(0 < required_length);

	// Initialise the allocated slot to the error code.
	int slot_id = -1;

	if (serial_device_send_queue.IsFull)
	{
		DEBUG("Serial device send queue is at capacity but additional packets were sent.  Is buffer too small?");
	}
	else if (thrd_error == mtx_lock(&serial_device_send_queue.SlotMutex))
	{
		ERROR("Failed to lock the serial writer send queue slot mutex...cannot continue allocating entry");
	}
	else if ((serial_device_send_queue.SendSlots[serial_device_send_queue.Head].SlotIsReserved) || (0 != serial_device_send_queue.SendSlots[serial_device_send_queue.Head].RawBytes))
	{
		DEBUG("Collision while allocating a slot...the proposed slot is marked as reserved and/or has a buffer allocated");
	}
	else if (0 == (serial_device_send_queue.SendSlots[serial_device_send_queue.Head].RawBytes = (unsigned char*)malloc(required_length)))
	{
		ERROR("Failed to allocate memory while allocated slot in serial device send queue");
	}
	else
	{
		serial_device_send_queue.SendSlots[serial_device_send_queue.Head].SlotIsReserved = true;

		slot_id = serial_device_send_queue.Head;

		// Advance the head pointer to include this packet!
		serial_device_send_queue.Head = (serial_device_send_queue.Head + 1) % serial_device_send_queue.Capacity;
		serial_device_send_queue.IsFull = (serial_device_send_queue.Head == serial_device_send_queue.Tail);

		if (thrd_error == mtx_unlock(&serial_device_send_queue.SlotMutex))
		{
			ERROR("Failed to unlock serial writer send queue slot mutex");
		}

		TRACE("Successfully allocated serial device send slot");
	}

	return slot_id;
}

bool serial_writer_enqueue_ack_message(AQ_Ack_Packet* ackPacket)
{
	assert(0 != ackPacket);

	bool packet_enqueued_successfully = false;
	int slot_id;

	if (serial_device_send_queue.IsFull)
	{
		DEBUG("Serial device send queue is at capacity but additional packets were sent.  Is buffer too small?");
	}
	else if (thrd_error == mtx_lock(&serial_device_send_queue.SlotMutex))
	{
		ERROR("Failed to lock the serial writer send queue slot mutex...cannot continue allocating entry");
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
		
		if (thrd_error == mtx_unlock(&serial_device_send_queue.SlotMutex))
		{
			ERROR("Failed to unlock serial writer send queue slot mutex");
		}
	}

	return packet_enqueued_successfully;
}
