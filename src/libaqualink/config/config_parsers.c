#include "config_parsers.h"

#include <assert.h>
#include <confuse.h>

#include "logging/logging_utils.h"

int parse_log_level(cfg_t* cfg, cfg_opt_t* opt, const char* value, void* result)
{
	assert(0 != cfg);
	assert(0 != opt);
	assert(0 != value);
	assert(0 != result);

	*(int*)result = string_to_logging_level(value);

	return 0;
}
