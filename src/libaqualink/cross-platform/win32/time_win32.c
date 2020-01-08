#include "../time.h"

#if defined (WIN32)

#include <Windows.h>
#include <stdbool.h>
#include <time.h>

enum { BILLION = 1000 * 1000 * 1000 };

char* strptime(const char* s, const char* format, struct tm* tm)
{
	return 0;
}

int nanosleep(const struct timespec* requested_delay, struct timespec* remaining_delay)
{
	static bool initialized;
	static double ticks_per_nanosecond;

	if (requested_delay->tv_nsec < 0 || BILLION <= requested_delay->tv_nsec)
	{
		errno = EINVAL;
		return -1;
	}

	// For requested delays of one second or more, 15ms resolution is sufficient.
	if (requested_delay->tv_sec == 0)
	{
		if (!initialized)
		{
			LARGE_INTEGER ticks_per_second;

			if (QueryPerformanceFrequency(&ticks_per_second))
			{
				// QueryPerformanceFrequency worked....we can use QueryPerformanceCounter later.
				ticks_per_nanosecond = (double)ticks_per_second.QuadPart / 1000000000.0;
			}

			initialized = true;
		}

		if (ticks_per_nanosecond)
		{
			// Number of milliseconds to pass to the Sleep function. Since Sleep can take up to 8 ms less or 8 ms more than requested
			// (or maybe more if the system is loaded), we subtract 10 ms.
			int sleep_millis = (int)requested_delay->tv_nsec / 1000000 - 10;

			LONGLONG wait_ticks = requested_delay->tv_nsec * ticks_per_nanosecond;
			LARGE_INTEGER counter_before;

			if (QueryPerformanceCounter(&counter_before))
			{
				// Wait until the performance counter has reached this value.  We don't need to worry about overflow, because the performance
				// counter is reset at reboot, and with a frequency of 3.6E6 ticks per second 63 bits suffice for over 80000 years.
				LONGLONG wait_until = counter_before.QuadPart + wait_ticks;

				if (sleep_millis > 0)
				{
					Sleep(sleep_millis);
				}

				for (;;)
				{
					LARGE_INTEGER counter_after;
					if (!QueryPerformanceCounter(&counter_after)) { break; }
					if (counter_after.QuadPart >= wait_until) { break; }
				}

				goto done;
			}
		}
	}

	// Implementation for long delays and as fallback.
	Sleep(requested_delay->tv_sec * 1000 + requested_delay->tv_nsec / 1000000);

done:
	if (NULL != remaining_delay)
	{
		remaining_delay->tv_sec = 0;
		remaining_delay->tv_nsec = 0;
	}

	return 0;
}

#endif // defined (WIN32)
