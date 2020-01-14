#ifndef AQ_ONETOUCH_SIMULATOR_PROBE_HANDLER_H_
#define AQ_ONETOUCH_SIMULATOR_PROBE_HANDLER_H_

#include <stdbool.h>

#include "messages/aq_serial_message_probe.h"

bool onetouch_simulator_probemessagehandler(AQ_Probe_Packet* packet);

#endif // AQ_ONETOUCH_SIMULATOR_PROBE_HANDLER_H_
