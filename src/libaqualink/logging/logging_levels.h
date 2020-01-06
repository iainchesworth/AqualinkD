#ifndef AQ_LOGGING_LEVELS_H_
#define AQ_LOGGING_LEVELS_H_

#ifdef __cplusplus
extern "C" {
#endif // __cplusplus

typedef enum tagLoggingLevels
{
	Trace = 7,
	Debug = 6,
	Info = 5,
	Notice = 4,
	Warning = 3,
	Error = 2,
	Critical = 1,
	Off = 0
}
LoggingLevels;

#ifdef __cplusplus
}
#endif // __cplusplus

#endif // AQ_LOGGING_LEVELS_H_
