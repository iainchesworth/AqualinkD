#include "mqtt_utils.h"

#include <stdio.h>
#include <string.h>

#include "cross-platform/process.h"
#include "cross-platform/socket.h"
#include "utility/utils.h"
#include "version/version.h"

char* generate_mqtt_id(char* buf, int len) 
{
	strncpy(buf, AQUALINKD_NAME, len);
	
	size_t i = strlen(buf);

	if (i < len)
	{
		buf[i++] = '_';

		// If we can't get MAC to pad mqtt id then use PID
		if (!mac(&buf[i], (int)(len - i)))
		{
			sprintf(&buf[i], "%.*d", (int)(len - i), GetPid());
		}
	}

	buf[len] = '\0';

	return buf;
}
