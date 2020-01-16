#ifndef AQ_CONFIG_PRIVATE_DEFAULTS_H_
#define AQ_CONFIG_PRIVATE_DEFAULTS_H_

#include <confuse.h>

#include "logging/logging_levels.h"

// Some defaults need to be defines because of how C interprets
// the constant expression (i.e. char* is fine, int is not).
//
// Specifically, in C2011 it is the subject of section 6.6.

#define DEFAULT_LOG_LEVEL		Notice
#define DEFAULT_WEBPORT			80
#define DEFAULT_SSL_WEBPORT		443
#define DEFAULT_DEVICEID		0xA0
#define TEMP_UNKNOWN			-999

#define DEFAULT_INSECURE		cfg_false
#define DEFAULT_PLAYBACK_MODE	cfg_false
#define DEFAULT_RECORD_MODE		cfg_false

extern const char DEFAULT_LOG_FILE[];
extern const char DEFAULT_WEBROOT[];
extern const char DEFAULT_CONFIG_FILE[];
extern const char DEFAULT_PID_FILE[];
extern const char DEFAULT_SERIALPORT[];
extern const char DEFAULT_MQTT_DZ_IN[];
extern const char DEFAULT_MQTT_DZ_OUT[];
extern const char DEFAULT_MQTT_AQ_TP[];
extern const char DEFAULT_MQTT_SERVER[];
extern const char DEFAULT_MQTT_USER[];
extern const char DEFAULT_MQTT_PASSWD[];
extern const char DEFAULT_RAWSERIAL_LOGFILE[];

#endif // AQ_CONFIG_PRIVATE_DEFAULTS_H_
