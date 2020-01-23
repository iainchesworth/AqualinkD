#ifndef AQ_RS_KEYPAD_SIMULATOR_H_
#define AQ_RS_KEYPAD_SIMULATOR_H_

#include <stdbool.h>

#include "messages/message-bus/aq_serial_message_bus.h"

bool rs_keypad_simulator_initialise(MessageBus* simulator_message_bus);
bool rs_keypad_simulator_enable(void);
bool rs_keypad_simulator_disable(void);

#endif // AQ_RS_KEYPADSIMULATOR_H_
