#ifndef PDA_SIMULATOR_PROBE_HANDLER_H_
#define PDA_SIMULATOR_PROBE_HANDLER_H_

#include <stdbool.h>

#include "messages/aq_serial_message_probe.h"

bool pda_simulator_probemessagehandler(AQ_Probe_Packet* packet);

#endif // PDA_SIMULATOR_PROBE_HANDLER_H_
