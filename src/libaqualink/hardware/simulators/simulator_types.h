#ifndef AQ_SIMULATOR_H_
#define AQ_SIMULATOR_H_

#include <stdbool.h>

typedef bool (*Simulator_Initialise)();
typedef bool (*Simulator_ProbeMessageHandler)();
typedef bool (*Simulator_MsgLongMessageHandler)();

#endif // AQ_SIMULATOR_H_
