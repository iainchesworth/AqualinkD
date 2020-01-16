#ifndef AQ_THREAD_UTILS_H_
#define AQ_THREAD_UTILS_H_

#include <stdbool.h>

bool initialise_termination_handler(void);
void trigger_application_termination(void);
void termination_handler(int signum);
bool test_for_termination(void);
bool wait_for_termination(void);
void cleanup_termination_handler(void);

#endif // AQ_THREAD_UTILS_H_
