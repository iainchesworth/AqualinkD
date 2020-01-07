#include "../socket.h"

#if !defined (WIN32)

#include <sys/ioctl.h>
#include <netinet/ip.h>
#include <net/if.h>
#include <net/if_arp.h>
#include <assert.h>
#include <stdio.h>
#include <string.h>

// Find the first network interface with valid MAC and put mac address into buffer upto length
bool mac(char* buf, int len)
{
	assert(0 != buf);
	assert(13 <= len); // Due to MAC length (i.e. 6x 2 chars plus NUL terminator)

	struct ifreq s;
	int fd = socket(PF_INET, SOCK_DGRAM, IPPROTO_IP);
	struct if_nameindex* if_nidxs, * intf;

	if_nidxs = if_nameindex();
	if (if_nidxs != 0)
	{
		for (intf = if_nidxs; intf->if_index != 0 || intf->if_name != 0; intf++)
		{
			strcpy(s.ifr_name, intf->if_name);
			if (0 == ioctl(fd, SIOCGIFHWADDR, &s))
			{
				int i;
				if (s.ifr_addr.sa_data[0] == 0 &&
					s.ifr_addr.sa_data[1] == 0 &&
					s.ifr_addr.sa_data[2] == 0 &&
					s.ifr_addr.sa_data[3] == 0 &&
					s.ifr_addr.sa_data[4] == 0 &&
					s.ifr_addr.sa_data[5] == 0) {
					continue;
				}
				for (i = 0; i < 6 && i * 2 < len; ++i)
				{
					sprintf(&buf[i * 2], "%02x", (unsigned char)s.ifr_addr.sa_data[i]);
				}
				return true;
			}
		}
	}

	return false;
}

#endif // !defined (WIN32)
