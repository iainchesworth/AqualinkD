#ifndef AQ_PDA_SIMULATOR_H_
#define AQ_PDA_SIMULATOR_H_

#include <stdbool.h>

#include "messages/message-bus/aq_serial_message_bus.h"

// Simulator functions

bool pda_simulator_initialise(MessageBus* simulator_message_bus);
bool pda_simulator_enable(void);
bool pda_simulator_disable(void);

#endif // AQ_PDA_SIMULATOR_H_
