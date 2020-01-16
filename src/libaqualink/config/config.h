#ifndef AQ_CONFIG_H_
#define AQ_CONFIG_H_

#include <stdbool.h>

void initialise_config_parameters(void);
void handle_configuration_file_options(void);
bool write_configuration_options_to_file(void);

#endif // AQ_CONFIG_H_
