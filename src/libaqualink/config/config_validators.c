#include "config_validators.h"

#include <confuse.h>

#include "logging/logging.h"
#include "logging/logging_levels.h"

int validate_loglevel(cfg_t* cfg, cfg_opt_t* opt)
{
	LoggingLevels proposed_log_level = (LoggingLevels) cfg_opt_getnint(opt, cfg_opt_size(opt) - 1);
	int return_value = -1;

	if ((Off > proposed_log_level) || (Trace < proposed_log_level))
	{
		cfg_error(cfg, "Invalid logging level was provided: %d", proposed_log_level);

		WARN("Invalid logging level was provided: %d", proposed_log_level);
	}
	else
	{
		return_value = 0;
	}

	return return_value;
}
