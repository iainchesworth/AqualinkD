#ifndef AQ_THREAD_UTILS_H_
#define AQ_THREAD_UTILS_H_

#include <stdbool.h>

bool initialise_termination_handler();
void trigger_application_termination();
void termination_handler(int signum);
bool wait_for_termination();
void cleanup_termination_handler();

#endif // AQ_THREAD_UTILS_H_
