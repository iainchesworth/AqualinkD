/*
 * Copyright (c) 2017 Shaun Feakes - All rights reserved
 *
 * You may use redistribute and/or modify this code under the terms of
 * the GNU General Public License version 2 as published by the
 * Free Software Foundation. For the terms of this license,
 * see <http://www.gnu.org/licenses/>.
 *
 * You are free to use this software under the terms of the GNU General
 * Public License, but WITHOUT ANY WARRANTY; without even the implied
 * warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
 * See the GNU General Public License for more details.
 *
 *  https://github.com/sfeakes/aqualinkd
 */

#include <assert.h>
#include <ctype.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "config/config_helpers.h"
#include "cross-platform/time.h"
#include "logging/logging.h"
#include "logging/logging_utils.h"
#include "string/string_utils.h"

#define TIMESTAMP_LENGTH 30

void timestamp(char* time_string)
{
	time_t now;
	struct tm* tmptr;

	time(&now);
	tmptr = localtime(&now);
	strftime(time_string, TIMESTAMP_LENGTH, "%b-%d-%y %H:%M:%S %p ", tmptr);
}

// Move existing pointer
char* cleanwhitespace(char* str)
{
	char* end;

	// Trim leading space
	while (isspace(*str)) 
	{
		str++;
	}

	if (*str == 0)
	{
		// All spaces?
		return str;
	}

	// Trim trailing space
	end = str + strlen(str) - 1;
	while (end > str&& isspace(*end))
	{
		end--;
	}

	// Write new null terminator
	*(end + 1) = 0;

	return str;
}

// Return new pointer
char* stripwhitespace(char* str)
{
	// Should probably just call Trim and Chop functions.
	char* end;
	char* start = str;

	// Trim leading space
	while (isspace(*start)) 
	{
		start++;
	}

	if (*start == 0)
	{
		// All spaces?
		return start;
	}

	// Trim trailing space
	end = str + strlen(str) - 1;
	while (end > str&& isspace(*end))
	{
		end--;
	}

	// Write new null terminator
	*(end + 1) = 0;

	return start;
}

// Trim whispace (return new pointer) leave trailing whitespace
char* trimwhitespace(char* str)
{
	char* start = str;

	// Trim leading space
	while (isspace(*start))
	{
		start++;
	}

	if (*start == 0)
	{
		// All spaces?
		return start;
	}

	return start;
}

char* chopwhitespace(char* str)
{
	char* end;
	end = str + strlen(str) - 1;
	while (end > str&& isspace(*end))
	{
		end--;
	}

	// Write new null terminator
	*(end + 1) = 0;

	return str;
}

int cleanint(char* str)
{
	if (str == NULL)
	{
		return 0;
	}

	str = cleanwhitespace(str);
	return atoi(str);
}

int count_characters(const char* str, char character)
{
	const char* p = str;
	int count = 0;

	do 
	{
		if (*p == character)
		{
			count++;
		}
	} 
	while (*(p++));

	return count;
}

static const char* YES_STRING = "YES";
static const char* NO_STRING = "NO";
static const char* ON_STRING = "ON";

bool text2bool(const char* str)
{
	assert(NULL != str);

	const size_t strLength = strlen(str);
	int firstCharPos = -1;
	size_t i;

	// Find the first non-whitespace char and "next" whitespace char after that.
	for (i = 0; i < strLength; ++i)
	{
		if ((-1 == firstCharPos) && (0 == isspace(str[i])))
		{
			// First non-whitespace char
			firstCharPos = i;

			// Done...exit the for loop.
			break;
		}
	}

	// Create a temporary string that starts where the non-whitespace characters start...
	const char* tmpStr = &str[firstCharPos];

	return ((0 == aq_strnicmp(tmpStr, YES_STRING, 3)) || (0 == aq_strnicmp(tmpStr, ON_STRING, 2))) ? true : false;
}

bool request2bool(const char* str)
{
	assert(NULL != str);

	const size_t strLength = strlen(str);
	int firstCharPos = -1;
	size_t i;

	// Find the first non-whitespace char and "next" whitespace char after that.
	for (i = 0; i < strLength; ++i)
	{
		if ((-1 == firstCharPos) && (0 == isspace(str[i])))
		{
			// First non-whitespace char
			firstCharPos = i;

			// Done...exit the for loop.
			break;
		}
	}

	// Create a temporary string that starts where the non-whitespace characters start...
	const char* tmpStr = &str[firstCharPos];

	return ((0 == aq_strnicmp(tmpStr, YES_STRING, 3)) || (0 == aq_strnicmp(tmpStr, ON_STRING, 2)) || (1 == atoi(str))) ? true : false;
}

const char* bool2text(bool val)
{
	return (true == val) ? YES_STRING : NO_STRING;
}

// (50°F - 32) x .5556 = 10°C
double degFtoC(double degF)
{
	return ((degF - 32.0f) / 1.8f);
}
// 30°C x 1.8 + 32 = 86°F 
double degCtoF(double degC)
{
	return ((degC * 1.8f) + 32.0f);
}

static const unsigned int MICROSECONDS_PER_SECOND = 1000000;
static const unsigned int MICROSECONDS_PER_MILLISECOND = 1000;

void delay(struct timespec* const sleeper)
{
    struct timespec dummy;
    nanosleep(sleeper, &dummy);
}

void delayMicroseconds(unsigned int howLong) // Microseconds (1000000 = 1 second)
{
    struct timespec sleepFor;

    sleepFor.tv_sec = (time_t)(howLong / 1000);
    sleepFor.tv_nsec = (long)(howLong % 1000) * 1000000;

    delay(&sleepFor);
}

void delayMilliseconds(unsigned int howLong)
{
    delayMicroseconds(howLong * MICROSECONDS_PER_MILLISECOND);
}

void delaySeconds(unsigned int howLong)
{
	delayMicroseconds(howLong * MICROSECONDS_PER_SECOND);
}

int ascii(char* destination, const char* source) 
{
	assert(NULL != destination);
	assert(NULL != source);

	unsigned int i;
	
	for (i = 0; i < strlen(source); i++) 
	{
		destination[i] = isascii(source[i]) ? source[i] : ' ';

		if (source[i] == 126) 
		{
			destination[i - 1] = '<';
			destination[i] = '>';
		}

	}

	destination[i] = '\0';
	return i;
}

char* prettyString(char* str)
{
	char* ptr = str;
	char* end;
	bool lastspace = true;

	end = str + strlen(str) - 1;
	while (end >= ptr) 
	{
		if (lastspace && *ptr > 96 && *ptr < 123) 
		{
			*ptr = *ptr - 32;
			lastspace = false;
		}
		else if (lastspace == false && *ptr > 54 && *ptr < 91) 
		{
			*ptr = *ptr + 32;
			lastspace = false;
		}
		else if (*ptr == 32) 
		{
			lastspace = true;
		}
		else 
		{
			lastspace = false;
		}

		ptr++;
	}

	return str;
}
