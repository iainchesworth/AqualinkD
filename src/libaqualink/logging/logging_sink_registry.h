#ifndef AQ_LOGGING_SINK_REGISTRY_H_
#define AQ_LOGGING_SINK_REGISTRY_H_

#ifdef __cplusplus
extern "C" {
#endif // __cplusplus

#include "logging_sink.h"

#include <stdbool.h>

struct LoggingSink_ListNode
{
	LoggingSink* sink;
	struct LoggingSink_ListNode* next;
};
typedef struct LoggingSink_ListNode LoggingSink_ListNode;

struct LoggingSinkRegistry
{
	LoggingSink_ListNode* head;
	LoggingSink_ListNode* tail;
	unsigned int sink_count;
};
typedef struct LoggingSinkRegistry LoggingSinkRegistry;

bool register_logging_sink(LoggingSinkRegistry* registry, LoggingSink* sink);

#ifdef __cplusplus
}
#endif // __cplusplus

#endif // AQ_LOGGING_SINK_REGISTRY_H_
