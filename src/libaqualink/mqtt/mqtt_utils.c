#include "mqtt_utils.h"

#include <sys/types.h>
#include <libgen.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>

#include "utils.h"


char* generate_mqtt_id(char* buf, int len) 
{
	extern char* __progname; // glibc populates this

	int i;
	
	strncpy(buf, basename(__progname), len);
	
	i = strlen(buf);

	if (i < len)
	{

		buf[i++] = '_';

		// If we can't get MAC to pad mqtt id then use PID
		if (!mac(&buf[i], len - i))
		{
			sprintf(&buf[i], "%.*d", (len - i), getpid());
		}
	}

	buf[len] = '\0';

	return buf;
}
