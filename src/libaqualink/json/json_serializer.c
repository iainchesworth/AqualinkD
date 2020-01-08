#include "json_serializer.h"

#include <json-c/json_object.h>
// #include <json-c/json_object_private.h> // Needed for the definition of the "struct json_object" type
#include <json-c/printbuf.h>
#include <stdio.h>
#include <string.h>

int double_to_json_string(json_object* jso, struct printbuf* pb, int level, int flags)
{
	static const char* DEFAULT_FORMAT_STRING = "%.01f";

	// const char* format = ((NULL == jso) || (NULL == jso->_userdata)) ? DEFAULT_FORMAT_STRING : (const char*) jso->_userdata;

	// return sprintbuf(pb, format, jso->o.c_double);
	return 0;
}
