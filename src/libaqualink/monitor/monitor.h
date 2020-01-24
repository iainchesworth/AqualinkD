#ifndef AQ_MONITOR_H_
#define AQ_MONITOR_H_

#include <stdbool.h>

#include "cross-platform/threads.h"
#include "messages/message-bus/aq_serial_message_bus.h"

extern const char monitor_messagetopic_name[];
extern unsigned char monitor_messagetopic_buffer[];
extern thrd_t monitor_thread;

bool monitor_initialise(void);
bool monitor_enable(void);
bool monitor_disable(void);

MessageBus* monitor_get_monitor_message_bus(void);
MessageBus_Topic* monitor_get_monitor_message_topic(void);

int monitor_processing_thread(void* argument);

#endif // AQ_MONITOR_H_
