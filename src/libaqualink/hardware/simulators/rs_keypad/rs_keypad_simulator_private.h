#ifndef AQ_RS_KEYPAD_SIMULATOR_PRIVATE_H_
#define AQ_RS_KEYPAD_SIMULATOR_PRIVATE_H_

#include <stdbool.h>

#include "rs_keypad_simulator_types.h"
#include "cross-platform/threads.h"
#include "messages/message-bus/aq_serial_message_bus.h"

bool rs_keypad_simulator_initmutex(void);

extern Aqualink_RS_Keypad_Simulator aqualink_rs_keypad_simulator;
extern MessageBus_Topic rs_keypad_simulator_messagetopic;
extern unsigned char rs_keypad_simulator_messagetopic_buffer[];
extern thrd_t rs_keypad_simulator_thread;

#endif // AQ_RS6_KEYPAD_SIMULATOR_PRIVATE_H_
