#ifndef AQ_CONFIG_PARSERS_H_
#define AQ_CONFIG_PARSERS_H_

#include <confuse.h>

int parse_log_level(cfg_t* cfg, cfg_opt_t* opt, const char* value, void* result);

#endif // AQ_CONFIG_PARSERS_H_
