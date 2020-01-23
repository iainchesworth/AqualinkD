#ifndef AQ_ONETOUCH_SIMULATOR_H_
#define AQ_ONETOUCH_SIMULATOR_H_

#include <stdbool.h>

#include "messages/message-bus/aq_serial_message_bus.h"

// Simulator functions

bool onetouch_simulator_initialise(MessageBus* simulator_message_bus);
bool onetouch_simulator_enable(void);
bool onetouch_simulator_disable(void);

#endif // AQ_ONETOUCH_SIMULATOR_H_
