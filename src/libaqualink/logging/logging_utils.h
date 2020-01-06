#ifndef AQ_LOGGING_UTILS_H_
#define AQ_LOGGING_UTILS_H_

#include "logging_levels.h"

const char* logging_level_to_string(LoggingLevels level);
LoggingLevels string_to_logging_level(const char* level);

#endif // AQ_LOGGING_UTILS_H_
