
#ifndef AQ_PROGRAMMER_H_
#define AQ_PROGRAMMER_H_

#include <threads.h>

// need to get the C values from aqualink manual and add those just incase
// someone has the controller set to C.
#define HEATER_MAX_F 104
#define HEATER_MIN_F 36
#define FREEZE_PT_MAX_F 42
#define FREEZE_PT_MIN_F 34

#define HEATER_MAX_C 40
#define HEATER_MIN_C 0
#define FREEZE_PT_MAX_C 5
#define FREEZE_PT_MIN_C 1

#define SWG_PERCENT_MAX 101
#define SWG_PERCENT_MIN 0

#define THREAD_ARG 25
#define LIGHT_MODE_BUFER THREAD_ARG

typedef enum {
  AQP_NULL = -1,
  AQ_GET_POOL_SPA_HEATER_TEMPS,
  AQ_GET_FREEZE_PROTECT_TEMP,
  AQ_SET_TIME,
  AQ_SET_POOL_HEATER_TEMP,
  AQ_SET_SPA_HEATER_TEMP,
  AQ_SET_FRZ_PROTECTION_TEMP,
  AQ_GET_DIAGNOSTICS_MODEL,
  //AQ_SEND_CMD,
  AQ_GET_PROGRAMS,
  AQ_SET_COLORMODE,
  AQ_PDA_INIT,
  AQ_SET_SWG_PERCENT,
  AQ_PDA_DEVICE_STATUS,
  AQ_PDA_DEVICE_ON_OFF,
  AQ_GET_AUX_LABELS,
  AQ_PDA_WAKE_INIT,
  AQ_SET_BOOST
} program_type;

struct programmingThreadCtrl {
  thrd_t thread_id;
  //void *thread_args;
  char thread_args[THREAD_ARG];
  struct aqualinkdata *aq_data;
};



//void aq_programmer(program_type type, void *args, struct aqualinkdata *aq_data);
void aq_programmer(program_type type, char *args, struct aqualinkdata *aq_data);
void kick_aq_program_thread(struct aqualinkdata *aq_data);

void aq_send_cmd(unsigned char cmd);

unsigned char pop_aq_cmd(struct aqualinkdata *aq_data);
//bool push_aq_cmd(unsigned char cmd);

//void send_cmd(unsigned char cmd, struct aqualinkdata *aq_data);
//void cancel_menu(struct aqualinkdata *aq_data);

//void *set_aqualink_time( void *ptr );
//void *get_aqualink_pool_spa_heater_temps( void *ptr );

int get_aq_cmd_length();
int setpoint_check(int type, int value, struct aqualinkdata *aqdata);
const char *ptypeName(program_type type);

// These shouldn't be here, but just for the PDA AQ PROGRAMMER
void send_cmd(unsigned char cmd);
bool push_aq_cmd(unsigned char cmd);
void waitForSingleThreadOrTerminate(struct programmingThreadCtrl *threadCtrl, program_type type);
void cleanAndTerminateThread(struct programmingThreadCtrl *threadCtrl);
bool waitForMessage(struct aqualinkdata *aq_data, char* message, int numMessageReceived);

#endif
