#ifndef AQ_LOGGING_SINK_TERMINAL_H_
#define AQ_LOGGING_SINK_TERMINAL_H_

#ifdef __cplusplus
extern "C" {
#endif // __cplusplus

const char* terminal_black();
const char* terminal_red();
const char* terminal_green();
const char* terminal_yellow();
const char* terminal_blue();
const char* terminal_purple();
const char* terminal_cyan();
const char* terminal_light_gray();
const char* terminal_white();
const char* terminal_light_red();

const char* terminal_dim();

const char* terminal_bold();
const char* terminal_underline();

const char* terminal_reset();

#ifdef __cplusplus
}
#endif // __cplusplus

#endif // AQ_LOGGING_SINK_TERMINAL_H_

