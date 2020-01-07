#ifndef AQ_DAEMON_UTILS_H_
#define AQ_DAEMON_UTILS_H_

#include <stdbool.h>

typedef bool(*main_function_t)(void);

bool daemonize(main_function_t main_function);

#endif // AQ_DAEMON_UTILS_H_
