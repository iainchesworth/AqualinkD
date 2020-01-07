#include "../socket.h"

#if defined (WIN32)

#include <winsock2.h>
#include <iphlpapi.h>
#include <assert.h>
#include <stdio.h>

#include "logging/logging.h"

#pragma comment(lib, "IPHLPAPI.lib")

#define MALLOC(x) HeapAlloc(GetProcessHeap(), 0, (x))
#define FREE(x) HeapFree(GetProcessHeap(), 0, (x))

// Find the first network interface with valid MAC and put mac address into buffer upto length
bool mac(char* buf, int len)
{
	assert(0 != buf);
	assert(13 <= len); // Due to MAC length (i.e. 6x 2 chars plus NUL terminator)

	PIP_ADAPTER_INFO pAdapterInfo, pAdapter = NULL;
	bool bRetValue = false;
	DWORD dwRetVal;

	ULONG ulOutBufLen = sizeof(IP_ADAPTER_INFO);

	if (NULL == (pAdapterInfo = (IP_ADAPTER_INFO*)MALLOC(sizeof(IP_ADAPTER_INFO))))
	{
		ERROR("Error allocating memory needed to call GetAdaptersinfo");
	} 
	else if ((ERROR_BUFFER_OVERFLOW == GetAdaptersInfo(pAdapterInfo, &ulOutBufLen)) && ((0 == FREE(pAdapterInfo)) || (NULL == (pAdapterInfo = (IP_ADAPTER_INFO*)MALLOC(ulOutBufLen)))))
	{
		ERROR("Error allocating memory needed to call GetAdaptersInfo");
	}
	else if (NO_ERROR != (dwRetVal = GetAdaptersInfo(pAdapterInfo, &ulOutBufLen)))
	{
		ERROR("GetAdaptersInfo failed with error: %d", dwRetVal);
	}
	else if (0 == (pAdapter = pAdapterInfo))
	{
		ERROR("Failed to detect a network adapter, cannot retrieve MAC");
	}
	else
	{
		sprintf(buf,
			"%02x%02x%02x%02x%02x%02x",
			pAdapter->Address[0],
			pAdapter->Address[1],
			pAdapter->Address[2],
			pAdapter->Address[3],
			pAdapter->Address[4],
			pAdapter->Address[5]);

		bRetValue = true;
	}

	if (NULL != pAdapterInfo)
	{
		FREE(pAdapterInfo);
	}

	return bRetValue;
}

#endif // defined (WIN32)
