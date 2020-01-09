
#ifndef AQUALINK_H_
#define AQUALINK_H_

#include <stdbool.h>

#include "cross-platform/threads.h"
#include "hardware/buttons/rs_buttons.h"
#include "serial/aq_serial.h"
#include "aq_programmer.h"

#define TIME_CHECK_INTERVAL  3600
#define ACCEPTABLE_TIME_DIFF 120

#define MAX_ZERO_READ_BEFORE_RECONNECT 500

#define TEMP_UNKNOWN    -999
#define DATE_STRING_LEN   30

#define MAX_PUMPS 4

// Message payload length definitions
// TODO Should these be in AQ_SERIAL.H?
#define BOOST_MSGLEN 10

enum {
 FAHRENHEIT,
 CELSIUS,
 UNKNOWN
};

typedef struct aqualinkkey
{
  aqled *led;
  char *label;
  char *name;
  char *pda_label;
  unsigned char code;
  int dz_idx;
} aqkey;

struct programmingthread {
  thrd_t *thread_id;
  mtx_t thread_mutex;
  cnd_t thread_cond;
  program_type ptype;
};

typedef enum action_type {
  NO_ACTION = -1,
  POOL_HTR_SETOINT,
  SPA_HTR_SETOINT,
  FREEZE_SETPOINT,
  SWG_SETPOINT,
  SWG_BOOST
} action_type;

struct action {
  action_type type;
  time_t requested;
  int value;
};

typedef struct pumpd
{
  int rpm;
  int gph;
  int watts;
  unsigned char pumpID;
  protocolType ptype;
  aqkey *button;
} pump_detail;

struct aqualinkdata
{
  char version[AQ_MSGLEN*2];
  char date[AQ_MSGLEN];
  char time[AQ_MSGLEN];
  char last_message[AQ_MSGLONGLEN+1]; // NSF just temp for PDA crap
  char last_display_message[AQ_MSGLONGLEN+1];
  unsigned char raw_status[AQ_PSTLEN];
  aqled aqualinkleds[TOTAL_LEDS];
  aqkey aqbuttons[ButtonTypeCount];
  int air_temp;
  int pool_temp;
  int spa_temp;
  int temp_units;
  bool single_device; // Pool or Spa only, not Pool & Spa (Thermostat setpoints are different)
  int battery;
  int frz_protect_set_point;
  int pool_htr_set_point;
  int spa_htr_set_point;
  struct programmingthread active_thread;
  struct action unactioned;
  int swg_percent;
  int swg_ppm;
  unsigned char ar_swg_status;
  int swg_delayed_percent;
  bool simulate_panel;
  AQ_LED_States service_mode_state;
  AQ_LED_States frz_protect_state;
  unsigned char last_packet_type;
  pump_detail pumps[MAX_PUMPS];
  int open_websockets;
  bool boost;
  char boost_msg[BOOST_MSGLEN];
  #ifdef AQ_DEBUG
  struct timespec last_active_time;
  struct timespec start_active_time;
  #endif
};

#endif 
