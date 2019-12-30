#ifndef AQ_CONFIG_H_
#define AQ_CONFIG_H_

void initialise_config_parameters();
void handle_configuration_file_options();

#ifdef _OLD_CFG_REMOVE_ME

struct aqconfig
{
  char *config_file;
  char *serial_port;
  unsigned int log_level;
  char *socket_port;
  char *web_directory;
  unsigned char device_id;
  bool deamonize;
  char *log_file;
  char *mqtt_dz_sub_topic;
  char *mqtt_dz_pub_topic;
  char *mqtt_aq_topic;
  char *mqtt_server;
  char *mqtt_user;
  char *mqtt_passwd;
  char mqtt_ID[MQTT_ID_LEN];
  int dzidx_air_temp;
  int dzidx_pool_water_temp;
  int dzidx_spa_water_temp;
  int dzidx_swg_percent;
  int dzidx_swg_ppm;
  int dzidx_swg_status;
  float light_programming_mode;
  int light_programming_initial_on;
  int light_programming_initial_off;
  int light_programming_button_pool;
  int light_programming_button_spa;
  bool override_freeze_protect;
  bool pda_mode;
  bool pda_sleep_mode;
  bool convert_mqtt_temp;
  bool convert_dz_temp;
  //bool flash_mqtt_buttons;
  bool report_zero_spa_temp;
  bool report_zero_pool_temp;
  bool read_all_devices;
  bool use_panel_aux_labels;
  bool force_swg;
  int swg_zero_ignore;
  bool display_warnings_web;
  //bool swg_pool_and_spa;
  //bool use_PDA_auxiliary;
  bool read_pentair_packets;
  bool debug_RSProtocol_packets;
  bool log_raw_RS_bytes;
  //int dzidx_pool_thermostat; // Domoticz virtual thermostats are crap removed until better
  //int dzidx_spa_thermostat;  // Domoticz virtual thermostats are crap removed until better
  //char mqtt_pub_topic[250];
  //char *mqtt_pub_tp_ptr = mqtt_pub_topic[];
};




void initialise_parameters (struct aqconfig * parms);
//bool parse_config (struct aqconfig * parms, char *cfgfile);
//void readCfg (struct aqconfig *config_parameters, char *cfgFile);
void readCfg (struct aqconfig *config_parameters, struct aqualinkdata *aqdata, char *cfgFile);
bool writeCfg (struct aqconfig *config_parameters, struct aqualinkdata *aqdata);
bool setConfigValue(struct aqconfig *config_parameters, struct aqualinkdata *aqdata, char *param, char *value);

#endif // _OLD_CFG_REMOVE_ME

char *cleanalloc(char*str);

#endif // AQ_CONFIG_H_
