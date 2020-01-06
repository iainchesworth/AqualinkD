#ifndef AQ_LOGGING_MESSAGE_H_
#define AQ_LOGGING_MESSAGE_H_

#ifdef __cplusplus
extern "C" {
#endif // __cplusplus

#include "logging_levels.h"

typedef struct tagLoggingMessage
{
	const char* LoggerName;
	LoggingLevels Level;

	struct tagPayload
	{
		const char* File;
		const char* Function;
		unsigned int Line;
		const char* Format;
	}
	Payload;

	const char* Message;
}
LoggingMessage;

#ifdef __cplusplus
}
#endif // __cplusplus

#endif // AQ_LOGGING_MESSAGE_H_
