/*
 * Copyright (c) 2017 Shaun Feakes - All rights reserved
 *
 * You may use redistribute and/or modify this code under the terms of
 * the GNU General Public License version 2 as published by the 
 * Free Software Foundation. For the terms of this license, 
 * see <http://www.gnu.org/licenses/>.
 *
 * You are free to use this software under the terms of the GNU General
 * Public License, but WITHOUT ANY WARRANTY; without even the implied
 * warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
 * See the GNU General Public License for more details.
 *
 *  https://github.com/sfeakes/aqualinkd
 */

#include <stdio.h>
#include <stdarg.h>
#include <unistd.h>
#include <stdlib.h>
#include <errno.h>
#include <string.h>
#include <signal.h>
#include <syslog.h>
#include <sys/file.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <time.h>
#include <ctype.h>
#include <fcntl.h>

#ifdef AD_DEBUG
#include <sys/time.h>
#endif

#ifndef _UTILS_C_
#define _UTILS_C_
#endif

#include "utils.h"


#define DEFAULT_LOG_FILE "/tmp/aqualinkd-inline.log"
//#define MAXCFGLINE 265
#define TIMESTAMP_LENGTH 30


static bool _daemonise = false;
static bool _log2file = false;
static int _log_level = LOG_ERR;
static char *_log_filename = NULL;
static bool _cfg_log2file;
static int _cfg_log_level;

static char *_loq_display_message = NULL;
//static char _log_filename[256];

void setLoggingPrms(int level , bool deamonized, char* log_file, char *error_messages)
{
	_log_level = level;
  _daemonise = deamonized;
  _loq_display_message = error_messages;

  _cfg_log_level = _log_level;
  _cfg_log2file = _log2file;
  
  if (log_file == NULL || strlen(log_file) <= 0) {
    _log2file = false;
  } else {
    _log2file = true;
    _log_filename = log_file;
    //strcpy(_log_filename, log_file);
  }  
}

int getLogLevel()
{
  return _log_level;
}


void startInlineDebug()
{
  _log_level = LOG_DEBUG;
  _log2file = true;
  if (_log_filename == NULL)
    _log_filename = DEFAULT_LOG_FILE;
}

void stopInlineDebug()
{
  _log_level = _cfg_log_level;
  _log2file = _cfg_log2file;
}

char *getInlineLogFName()
{
  return _log_filename;
}



bool islogFileReady()
{
  if (_log_filename != NULL) {   
    struct stat st;
    stat(_log_filename, &st);
    if ( st.st_size > 0)
      return true;
  } 
  return false;
}

void cleanInlineDebug() {
  if (_log_filename != NULL) {
    fclose(fopen(_log_filename, "w"));
  }
}

/*
* This function reports the error and
* exits back to the shell:
*/
void displayLastSystemError (const char *on_what)
{
  fputs (strerror (errno), stderr);
  fputs (": ", stderr);
  fputs (on_what, stderr);
  fputc ('\n', stderr);

  if (_daemonise == TRUE)
  {
    logMessage (LOG_ERR, "%d : %s", errno, on_what);
    closelog ();
  }
}

/*
From -- syslog.h --
#define	LOG_EMERG	0	// system is unusable 
#define	LOG_ALERT	1	// action must be taken immediately 
#define	LOG_CRIT	2	// critical conditions 
#define	LOG_ERR		3	// error conditions 
#define	LOG_WARNING	4	// warning conditions 
#define	LOG_NOTICE	5	// normal but significant condition 
#define	LOG_INFO	6	// informational 
#define	LOG_DEBUG	7	// debug-level messages 
*/

char *elevel2text(int level) 
{
  switch(level) {
    case LOG_ERR:
      return "Error:";
      break;
    case LOG_WARNING:
      return "Warning:";
      break;
    case LOG_NOTICE:
      return "Notice:";
      break;
    case LOG_INFO:
      return "Info:";
      break;
    case LOG_DEBUG:
    default:
      return "Debug:";
      break;
  }
  
  return "";
}

int text2elevel(char* level)
{
  if (strcmp(level, "DEBUG_SERIAL") == 0) {
    return LOG_DEBUG_SERIAL;
  }
  if (strcmp(level, "DEBUG") == 0) {
    return LOG_DEBUG;
  }
  else if (strcmp(level, "INFO") == 0) {
    return LOG_INFO;
  }
  else if (strcmp(level, "WARNING") == 0) {
    return LOG_WARNING;
  }
  else if (strcmp(level, "NOTICE") == 0) {
    return LOG_NOTICE;
  }
  else if (strcmp(level, "INFO") == 0) {
    return LOG_INFO;
  }
  
 return  LOG_ERR; 
}

void timestamp(char* time_string)
{
    time_t now;
    struct tm *tmptr;

    time(&now);
    tmptr = localtime(&now);
    strftime(time_string, TIMESTAMP_LENGTH, "%b-%d-%y %H:%M:%S %p ", tmptr);
}

//Move existing pointer
char *cleanwhitespace(char *str)
{
  char *end;

  // Trim leading space
  while(isspace(*str)) str++;

  if(*str == 0)  // All spaces?
    return str;

  // Trim trailing space
  end = str + strlen(str) - 1;
  while(end > str && isspace(*end)) end--;

  // Write new null terminator
  *(end+1) = 0;

  return str;
}

// Return new pointer
char *stripwhitespace(char *str)
{
  // Should probably just call Trim and Chop functions.
  char *end;
  char *start = str;

  // Trim leading space
  while(isspace(*start)) start++;

  if(*start == 0)  // All spaces?
    return start;

  // Trim trailing space
  end = str + strlen(str) - 1;
  while(end > str && isspace(*end)) end--;

  // Write new null terminator
  *(end+1) = 0;

  return start;
}

// Trim whispace (return new pointer) leave trailing whitespace
char *trimwhitespace(char *str)
{
  char *start = str;

  // Trim leading space
  while(isspace(*start)) start++;

  if(*start == 0)  // All spaces?
    return start;

  return start;
}


char *chopwhitespace(char *str)
{
  char *end;
  end = str + strlen(str) - 1;
  while(end > str && isspace(*end)) end--;
  
  // Write new null terminator
  *(end+1) = 0;

  return str;
}


/*
char *cleanquotes(char *str)
{
  char *end;
  // Trim leading whitespace
  //while(isspace(*str)) str++;
  //if(*str == 0)  // All spaces?
  //  return str;
syslog(LOG_INFO, "String to clean %s\n", str);

  while(*str=='"' || *str== '\'' || *str==' ') str++;
  if(*str == 0)  // All spaces
    return str;

  end = str + strlen(str) - 1;
  while(end > str && (*end=='"' || *end== '\'' || *end==' ')) end--;

  // Write new null terminator
  *(end+1) = 0;

syslog(LOG_INFO, "String cleaned %s\n", str);

  return str;
}
*/
int cleanint(char*str)
{
  if (str == NULL)
    return 0;
    
  str = cleanwhitespace(str);
  return atoi(str);
}

void test(int msg_level, char *msg)
{
  char buffer[256];
  
  sprintf(buffer,"Level %d | MsgLvl %d | Dmn %d | LF %d | %s - %s",_log_level,msg_level,_daemonise,_log2file,_log_filename,msg);
  if ( buffer[strlen(buffer)-1] != '\n') {
    strcat(buffer, "\n");
  } 
  
  int fp = open("/var/log/aqualink.log", O_WRONLY | O_APPEND | O_CREAT, S_IRUSR | S_IWUSR | S_IRGRP | S_IWGRP | S_IROTH | S_IWOTH);
  if (fp != -1) {
    write(fp, buffer, strlen(buffer) );
    close(fp);  
  } else {
    syslog(LOG_ERR, "Can't open file /var/log/aqualink.log");
  }
}


void logMessage(int msg_level, char *format, ...)
{
  // Simply return ASAP.
  if (msg_level > _log_level) {
    return;
  }

  char buffer[1024];
  va_list args;
  va_start(args, format);
  strncpy(buffer, "         ", 8);
  vsprintf (&buffer[8], format, args);
  va_end(args);
  
  //test(msg_level, buffer);
  //fprintf (stderr, buffer);

  // Logging has not been setup yet, so STD error & syslog
  if (_log_level == -1) {
    fprintf (stderr, buffer);
    syslog (msg_level, "%s", &buffer[8]);
    closelog ();
  }
  
  if (_daemonise == TRUE)
  {
    if (msg_level > LOG_DEBUG)  // Let's not confuse syslog with custom levels
      syslog (LOG_DEBUG, "%s", &buffer[8]);
    else
      syslog (msg_level, "%s", &buffer[8]);
    closelog ();
    //return;
  }
  
  int len;
  char *strLevel = elevel2text(msg_level);
  strncpy(buffer, strLevel, strlen(strLevel));
  len = strlen(buffer); 
  if ( buffer[len-1] != '\n') {
    strcat(buffer, "\n");
  }

  // Sent the log to the UI if configured.
  if (msg_level <= LOG_WARNING && _loq_display_message != NULL) {
    snprintf(_loq_display_message, 127, buffer);
  } 

  if (_log2file == TRUE && _log_filename != NULL) {   
    char time[TIMESTAMP_LENGTH];
    int fp = open(_log_filename, O_WRONLY | O_APPEND | O_CREAT, S_IRUSR | S_IWUSR | S_IRGRP | S_IWGRP | S_IROTH | S_IWOTH);
    if (fp != -1) {
      timestamp(time);
      write(fp, time, strlen(time) );
      write(fp, buffer, strlen(buffer) );
      close(fp);
    } else {
      if (_daemonise == TRUE)
      syslog(LOG_ERR, "Can't open log file\n %s", buffer);
      else
      fprintf (stderr, "Can't open debug log\n %s", buffer);
    }
  }
  
  if (_daemonise == FALSE) {
    if (msg_level == LOG_ERR) {
      fprintf(stderr, "%s", buffer);
    } else {
#ifndef AD_DEBUG
      printf("%s", buffer);
#else
      struct timespec tspec;
      struct tm localtm;
      clock_gettime(CLOCK_REALTIME, &tspec);
      char timeStr[TIMESTAMP_LENGTH];
      strftime(timeStr, sizeof(timeStr), "%H:%M:%S", localtime_r(&tspec.tv_sec, &localtm));
      printf("%s.%03ld %s", timeStr, tspec.tv_nsec / 1000000L, buffer);
#endif
    }
  }
}

void daemonise (char *pidFile, void (*main_function) (void))
{
  FILE *fp = NULL;
  pid_t process_id = 0;
  pid_t sid = 0;

  _daemonise = true;
  
  /* Check we are root */
  if (getuid() != 0)
  {
    logMessage(LOG_ERR,"Can only be run as root\n");
    exit(EXIT_FAILURE);
  }

  int pid_file = open (pidFile, O_CREAT | O_RDWR, 0666);
  int rc = flock (pid_file, LOCK_EX | LOCK_NB);
  if (rc)
  {
    //if (EWOULDBLOCK == errno)
    //; // another instance is running
    //fputs ("\nAnother instance is already running\n", stderr);
    logMessage(LOG_ERR,"\nAnother instance is already running\n");
    exit (EXIT_FAILURE);
  }

  process_id = fork ();
  // Indication of fork() failure
  if (process_id < 0)
  {
    displayLastSystemError ("fork failed!");
    // Return failure in exit status
    exit (EXIT_FAILURE);
  }
  // PARENT PROCESS. Need to kill it.
  if (process_id > 0)
  {
    fp = fopen (pidFile, "w");

    if (fp == NULL)
    logMessage(LOG_ERR,"can't write to PID file %s",pidFile);
    else
    fprintf(fp, "%d", process_id);

    fclose (fp);
    logMessage (LOG_DEBUG, "process_id of child process %d \n", process_id);
    // return success in exit status
    exit (EXIT_SUCCESS);
  }
  //unmask the file mode
  umask (0);
  //set new session
  sid = setsid ();
  if (sid < 0)
  {
    // Return failure
    displayLastSystemError("Failed to fork process");
    exit (EXIT_FAILURE);
  }
  // Change the current working directory to root.
  chdir ("/");
  // Close stdin. stdout and stderr
  close (STDIN_FILENO);
  close (STDOUT_FILENO);
  close (STDERR_FILENO);

  // this is the first instance
  (*main_function) ();

  return;
}

int count_characters(const char *str, char character)
{
    const char *p = str;
    int count = 0;

    do {
        if (*p == character)
            count++;
    } while (*(p++));

    return count;
}

bool text2bool(char *str)
{
  str = cleanwhitespace(str);
  if (strcasecmp (str, "YES") == 0 || strcasecmp (str, "ON") == 0)
    return TRUE;
  else
    return FALSE;
}

bool request2bool(char *str)
{
  str = cleanwhitespace(str);
  if (strcasecmp (str, "YES") == 0 || strcasecmp (str, "ON") == 0 || atoi(str) == 1)
    return TRUE;
  else
    return FALSE;
}

char *bool2text(bool val)
{
  if(val == TRUE)
    return "YES";
  else
    return "NO";
}

// (50°F - 32) x .5556 = 10°C
float degFtoC(float degF)
{
  return ((degF-32) / 1.8);
}
// 30°C x 1.8 + 32 = 86°F 
float degCtoF(float degC)
{
  return (degC * 1.8 + 32);
}

#include <time.h>

void delay (unsigned int howLong) // Microseconds (1000000 = 1 second)
{
  struct timespec sleeper, dummy ;

  sleeper.tv_sec  = (time_t)(howLong / 1000) ;
  sleeper.tv_nsec = (long)(howLong % 1000) * 1000000 ;

  nanosleep (&sleeper, &dummy) ;
}

char* stristr(const char* haystack, const char* needle) {
  do {
    const char* h = haystack;
    const char* n = needle;
    while (tolower((unsigned char) *h) == tolower((unsigned char ) *n) && *n) {
      h++;
      n++;
    }
    if (*n == 0) {
      return (char *) haystack;
    }
  } while (*haystack++);
  return 0;
}

int ascii(char *destination, char *source) {
  unsigned int i;
  for(i = 0; i < strlen(source); i++) {
    if ( source[i] >= 0 && source[i] < 128 )
      destination[i] = source[i];
    else
      destination[i] = ' ';

    if ( source[i]==126 ) {
      destination[i-1] = '<';
      destination[i] = '>';
    } 

  }
  destination[i] = '\0';
  return i;
}

char *prittyString(char *str)
{
  char *ptr = str;
  char *end;
  bool lastspace=true;

  end = str + strlen(str) - 1;
  while(end >= ptr){
    //printf("%d %s ", *ptr, ptr);
    if (lastspace && *ptr > 96 && *ptr < 123) {
      *ptr = *ptr - 32;
      lastspace=false;
      //printf("to upper\n");
    } else if (lastspace == false && *ptr > 54 && *ptr < 91) {
      *ptr = *ptr + 32;
      lastspace=false;
      //printf("to lower\n");
    } else if (*ptr == 32) {
      lastspace=true;
      //printf("space\n");
    } else {
      lastspace=false;
      //printf("leave\n");
    }
    ptr++;
  }

  //printf("-- %s --\n", str);

  return str;
}

/*
static FILE *_packetLogFile = NULL;

void writePacketLog(char *buffer) {
  if (_packetLogFile == NULL)
    _packetLogFile = fopen("/tmp/RS485.log", "a");

  if (_packetLogFile != NULL) {
    fputs(buffer, _packetLogFile);
  } 
}
void closePacketLog() {
  fclose(_packetLogFile);
}
*/


