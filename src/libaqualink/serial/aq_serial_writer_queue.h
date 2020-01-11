#ifndef AQ_SERIAL_WRITER_QUEUE_H_
#define AQ_SERIAL_WRITER_QUEUE_H_

#include <stdbool.h>

#include <cross-platform/threads.h>
#include "serial/serializers/aq_serial_message_ack_serializer.h"

typedef struct tagSerialWriter_SendQueueEntry
{
	bool SlotIsReserved;
	unsigned char* RawBytes;

	unsigned int DataLength;
	unsigned int BytesDequeued;
}
SerialWriter_SendQueueEntry;

typedef struct tagSerialWriter_SendQueue
{
	int Capacity;

	int Tail; // Consumer part
	int Head; // Producer Part

	bool IsFull;

	mtx_t SlotMutex;

	SerialWriter_SendQueueEntry* SendSlots;
}
SerialWriter_SendQueue;

bool initialise_serial_writer_send_queue(const unsigned int maximum_size);

// Utility functions
int serial_writer_send_queue_used_entries();
int serial_writer_send_queue_empty_entries();
int serial_writer_send_queue_total_entries();

// Slot functions - dequeue
unsigned char serial_writer_dequeue_message_get_next_byte();

// Slot functions - enqueue
bool serial_writer_enqueue_ack_message(AQ_Ack_Packet* ackPacket);

#endif // AQ_SERIAL_WRITER_QUEUE_H_
