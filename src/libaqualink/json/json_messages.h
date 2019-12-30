#ifndef JSON_MESSAGES_H_
#define JSON_MESSAGES_H_

#ifdef __cplusplus
extern "C" {
#endif // __cplusplus

#include <stdbool.h>
#include "aqualink.h"

#define AUX_BUFFER_SIZE			100

struct JSONkeyvalue{
  char *key;
  char *value;
};

struct JSONwebrequest {
  struct JSONkeyvalue first;
  struct JSONkeyvalue second;
  struct JSONkeyvalue third;
};

int build_mqtt_status_JSON(char* buffer, int buffer_length, int idx, int nvalue, double tvalue);
int build_mqtt_status_message_JSON(char* buffer, int buffer_length, int idx, int nvalue, char* svalue);

int build_aqualink_status_JSON(struct aqualinkdata *aqdata, char* buffer, int buffer_length);
int build_aqualink_error_status_JSON(char* buffer, int buffer_length, char* msg);

int build_device_JSON(struct aqualinkdata* aqdata, int programable_switch1, int programable_switch2, char* buffer, int size, bool homekit);
int build_aux_labels_JSON(struct aqualinkdata *aqdata, char* buffer, int size);

bool parseJSONwebrequest(char *buffer, struct JSONwebrequest *request);
bool parseJSONmqttrequest(const char *str, size_t len, int *idx, int *nvalue, char *svalue);

#ifdef __cplusplus
}
#endif // __cplusplus

#endif // JSON_MESSAGES_H_
