#ifndef UTILS_H_
#define UTILS_H_

#include <stdbool.h>

#define MAXLEN 256

#define round(a) (int) (a+0.5)				// 0 decimal places
#define roundf(a) (float) ((a*100)/100)		// 2 decimal places

#define UNREFERENCED_PARAMETER(P) (P)		// Prevent unused parameter warnings

typedef void (*worker_termination_handler_t)(void);	

void setLoggingPrms(int level, bool deamonized, const char* log_file, const char* error_messages);
int getLogLevel(void);
int count_characters(const char* str, char character);
char* cleanwhitespace(char* str);
char* chopwhitespace(char* str);
char* trimwhitespace(char* str);
char* stripwhitespace(char* str);
int cleanint(char* str);
bool text2bool(const char* str);
bool request2bool(const char* str);
const char* bool2text(bool val);
void delaySeconds(unsigned int howLong);
void delayMilliseconds(unsigned int howLong);
void delayMicroseconds(unsigned int howLong);
float degFtoC(float degF);
float degCtoF(float degC);
int ascii(char* destination, const char* source);
char* prettyString(char* str);
void startInlineDebug(void);
void stopInlineDebug(void);
void cleanInlineDebug(void);
char* getInlineLogFName(void);
bool islogFileReady(void);

#endif /* UTILS_H_ */
