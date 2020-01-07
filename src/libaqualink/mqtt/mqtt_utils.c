#include "mqtt_utils.h"

#include <stdio.h>
#include <string.h>

#include "cross-platform/process.h"
#include "cross-platform/socket.h"
#include "version/version.h"
#include "utils.h"


char* generate_mqtt_id(char* buf, int len) 
{
	strncpy(buf, AQUALINKD_NAME, len);
	
	int i = strlen(buf);

	if (i < len)
	{
		buf[i++] = '_';

		// If we can't get MAC to pad mqtt id then use PID
		if (!mac(&buf[i], len - i))
		{
			sprintf(&buf[i], "%.*d", (len - i), GetPid());
		}
	}

	buf[len] = '\0';

	return buf;
}
