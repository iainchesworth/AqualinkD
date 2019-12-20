#include <syslog.h>
#include <stdbool.h>

#ifndef UTILS_H_
#define UTILS_H_

#define LOG_DEBUG_SERIAL 8

#ifndef EXIT_SUCCESS
  #define EXIT_FAILURE 1
  #define EXIT_SUCCESS 0
#endif

#ifndef TRUE
  #define TRUE 1
  #define FALSE 0
#endif

#define MAXLEN 256

#define round(a) (int) (a+0.5)          // 0 decimal places
#define roundf(a) (float) ((a*100)/100) // 2 decimal places

#define UNREFERENCED_PARAMETER(P) (P)   // Prevent unused parameter warnings

/*
typedef enum
{
  false = FALSE, true = TRUE
} bool;
*/
//void setLoggingPrms(int level , bool deamonized, char* log_file);
void setLoggingPrms(int level , bool deamonized, char* log_file, char *error_messages);
int getLogLevel();
void daemonise ( char *pidFile, void (*main_function)(void) );
//void debugPrint (char *format, ...);
void displayLastSystemError (const char *on_what);
void logMessage(int level, char *format, ...);
int count_characters(const char *str, char character);
//void readCfg (char *cfgFile);
int text2elevel(char* level);
char *elevel2text(int level);
char *cleanwhitespace(char *str);
//char *cleanquotes(char *str);
char *chopwhitespace(char *str);
char *trimwhitespace(char *str);
char *stripwhitespace(char *str);
int cleanint(char*str);
bool text2bool(const char *str);
bool request2bool(const char *str);
const char *bool2text(bool val);
void delaySeconds(unsigned int howLong);
void delayMilliseconds(unsigned int howLong);
void delayMicroseconds (unsigned int howLong);
float degFtoC(float degF);
float degCtoF(float degC);
char* stristr(const char* haystack, const char* needle);
int ascii(char *destination, const char *source);
char *prettyString(char *str);
//void writePacketLog(char *buff);
//void closePacketLog();
void startInlineDebug();
void stopInlineDebug();
void cleanInlineDebug();
char *getInlineLogFName();
bool islogFileReady();

//#ifndef _UTILS_C_
  extern bool _daemon_;
  extern bool _debuglog_;
  extern bool _debug2file_;
//#endif

#endif /* UTILS_H_ */
