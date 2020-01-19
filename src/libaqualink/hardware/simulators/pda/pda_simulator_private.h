#ifndef AQ_PDA_SIMULATOR_PRIVATE_H_
#define AQ_PDA_SIMULATOR_PRIVATE_H_

#include <stdbool.h>

#include "pda_simulator_types.h"
#include "cross-platform/threads.h"
#include "messages/message-bus/aq_serial_message_bus.h"

bool pda_simulator_initmutex(void);

extern Aqualink_PDASimulator aqualink_pda_simulator;
extern MessageBus_Topic pda_simulator_messagetopic;
extern unsigned char pda_simulator_messagetopic_buffer[];
extern thrd_t pda_simulator_thread;

#endif // AQ_PDA_SIMULATOR_PRIVATE_H_
