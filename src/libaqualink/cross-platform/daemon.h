#ifndef AQ_DAEMON_H_
#define AQ_DAEMON_H_

#include <stdbool.h>

typedef bool(*main_function_t)();

bool daemonize(main_function_t main_function);

#endif // AQ_DAEMONS_H_
