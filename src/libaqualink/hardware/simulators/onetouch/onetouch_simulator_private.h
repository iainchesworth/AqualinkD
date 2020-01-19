#ifndef AQ_ONETOUCH_SIMULATOR_PRIVATE_H_
#define AQ_ONETOUCH_SIMULATOR_PRIVATE_H_

#include <stdbool.h>

#include "onetouch_simulator_types.h"
#include "cross-platform/threads.h"
#include "messages/message-bus/aq_serial_message_bus.h"

bool onetouch_simulator_initmutex(void);

extern Aqualink_OneTouchSimulator aqualink_onetouch_simulator;
extern MessageBus_Topic onetouch_simulator_messagetopic;
extern unsigned char onetouch_simulator_messagetopic_buffer[];
extern thrd_t onetouch_simulator_thread;

#endif // AQ_ONETOUCH_SIMULATOR_PRIVATE_H_
