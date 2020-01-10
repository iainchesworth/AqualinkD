#ifndef AQ_SERIAL_WRITER_QUEUE_H_
#define AQ_SERIAL_WRITER_QUEUE_H_

#include <stdbool.h>

#include <cross-platform/threads.h>
#include "serial/serializers/aq_serial_message_ack_serializer.h"

typedef struct tagSerialWriter_SendQueueEntry
{
	bool SlotIsReserved;
	unsigned char* RawBytes;
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
int serial_writer_send_queue_empty_entries();

// Slot functions
bool serial_writer_enqueue_ack_message(AQ_Ack_Packet* ackPacket);


/*

int spsc_queue_push_many(SerialWriter_SendQueue* queue, void* ptrs[], size_t cnt);
int spsc_queue_pull_many(SerialWriter_SendQueue* queue, void** ptrs[], size_t cnt);
int spsc_queue_get_many(SerialWriter_SendQueue* queue, void** ptrs[], size_t cnt);

int spsc_queue_destroy(SerialWriter_SendQueue* queue);

static inline int serial_writer_send_queue_push(SerialWriter_SendQueue* queue, void* ptr)
{
	return serial_writer_send_queue_push_many(queue, &ptr, 1);
}

static inline int serial_writer_send_queue_pull(SerialWriter_SendQueue* queue, void** ptr)
{
	return serial_writer_send_queue_pull_many(queue, &ptr, 1);
}

static inline int serial_writer_send_queue_get(SerialWriter_SendQueue* queue, void** ptr)
{
	return serial_writer_send_queue_get_many(queue, &ptr, 1);
}
*/

#endif // AQ_SERIAL_WRITER_QUEUE_H_
