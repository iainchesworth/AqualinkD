#include "logging_sink_registry.h"

#include <assert.h>
#include <stdlib.h>

static bool add_sink_to_tail_of_list(LoggingSinkRegistry* registry, LoggingSink* sink)
{
	assert(0 != registry);
	assert(0 == registry->tail->next);
	assert(0 != sink);

	if (0 == (registry->tail->next = (LoggingSink_ListNode*)malloc(sizeof(LoggingSink_ListNode))))
	{
		// Failed to allocate a new node.
		return false;
	}

	// Add the sink to the tail (by adding a new node).
	registry->tail->next->sink = sink;
	registry->tail->next->next = 0;
	registry->sink_count++;

	// Move the tail to point at the last node.
	registry->tail = registry->tail->next;

	return true;
}

bool register_logging_sink(LoggingSinkRegistry* registry, LoggingSink* sink)
{
	assert(0 != registry);
	assert(0 != sink);

	bool retSuccess;
	
	if (0 != registry->head)
	{
		// The list already exists, add the sink to the end of the list
		retSuccess = add_sink_to_tail_of_list(registry, sink);
	}
	else if (0 == (registry->head = (LoggingSink_ListNode*)malloc(sizeof(LoggingSink_ListNode))))
	{
		// Failed to allocate a new node to create a new list.
		retSuccess = false;
	}
	else
	{
		// Trigger the sink's initialisation process (if it has one).
		if (0 != sink->Initialise)
		{
			sink->Initialise(sink);
		}

		// Add the sink as the first node of the list.
		registry->head->sink = sink;
		registry->head->next = 0;
		registry->sink_count++;
		
		// Make the tail point to the "last" node of the list.
		registry->tail = registry->head;

		retSuccess = true;
	}

	return retSuccess;
}
