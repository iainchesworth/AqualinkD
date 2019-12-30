#ifndef AQ_JSON_SERIALIZER_H_
#define AQ_JSON_SERIALIZER_H_

#ifdef __cplusplus
extern "C" {
#endif // __cplusplus

#include <json-c/json_object.h>

int double_to_json_string(struct json_object* jso, struct printbuf* pb, int level, int flags);

#ifdef __cplusplus
}
#endif // __cplusplus

#endif // AQ_JSON_SERIALIZER_H_
