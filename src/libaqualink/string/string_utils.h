#ifndef AQ_STRING_UTILS_H_
#define AQ_STRING_UTILS_H_

#include <string.h>

int aq_stricmp(const char* first, const char* second);
int aq_strnicmp(const char* first, const char* second, size_t max);
char* aq_stristr(const char* haystack, const char* needle);

#endif // AQ_STRING_UTILS_H_
