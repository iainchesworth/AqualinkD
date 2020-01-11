#ifndef AQ_SIMULATOR_H_
#define AQ_SIMULATOR_H_

#include <stdbool.h>

#include "serial/serializers/aq_serial_message_ack_serializer.h"
#include "serial/serializers/aq_serial_message_msg_long_serializer.h"
#include "serial/serializers/aq_serial_message_probe_serializer.h"
#include "serial/serializers/aq_serial_message_status_serializer.h"
#include "serial/serializers/aq_serial_message_unknown_serializer.h"

typedef bool (*Simulator_Initialise)();
typedef bool (*Simulator_AckMessageHandler)(AQ_Ack_Packet* packet);
typedef bool (*Simulator_MsgLongMessageHandler)(AQ_Msg_Long_Packet* packet);
typedef bool (*Simulator_ProbeMessageHandler)(AQ_Probe_Packet* packet);
typedef bool (*Simulator_StatusMessageHandler)(AQ_Status_Packet* packet);
typedef bool (*Simulator_UnknownMessageHandler)(AQ_Unknown_Packet* packet);

#endif // AQ_SIMULATOR_H_
