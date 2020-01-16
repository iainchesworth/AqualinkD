#ifndef AQ_LOGGING_SINK_TERMINAL_H_
#define AQ_LOGGING_SINK_TERMINAL_H_

#ifdef __cplusplus
extern "C" {
#endif // __cplusplus

const char* terminal_black(void);
const char* terminal_red(void);
const char* terminal_green(void);
const char* terminal_yellow(void);
const char* terminal_blue(void);
const char* terminal_purple(void);
const char* terminal_cyan(void);
const char* terminal_light_gray(void);
const char* terminal_white(void);
const char* terminal_light_red(void);

const char* terminal_dim(void);

const char* terminal_bold(void);
const char* terminal_underline(void);

const char* terminal_reset(void);

#ifdef __cplusplus
}
#endif // __cplusplus

#endif // AQ_LOGGING_SINK_TERMINAL_H_

