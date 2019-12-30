#ifndef AQ_JSON_STRING_UTILS_H_
#define AQ_JSON_STRING_UTILS_H_

#ifdef __cplusplus
extern "C" {
#endif // __cplusplus

#include "serial/aq_serial_types.h"

#define JSON_LABEL_SIZE			300
#define JSON_STATUS_SIZE		800
#define JSON_MQTT_MSG_SIZE		100

#define JSON_ON					"on"
#define JSON_OFF				"off"
#define JSON_FLASH				"flash"
#define JSON_ENABLED			"enabled"

#define JSON_FAHRENHEIT			"f"
#define JSON_CELSIUS			"c"
#define JSON_UNKNOWN			"u"

#define JSON_OK					"ok"
#define JSON_LOW				"low"

#define JSON_PROGRAMMING		"Programming"
#define JSON_SERVICE			"Service Mode"
#define JSON_TIMEOUT			"Timeout Mode"
#define JSON_READY				"Ready"

const char* LED2text(AQ_LED_States state);
int LED2int(AQ_LED_States state);

#ifdef __cplusplus
}
#endif // __cplusplus

#endif // AQ_JSON_STRING_UTILS_H_
