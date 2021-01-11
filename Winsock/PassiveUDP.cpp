#include "stdafx.h"
#include <winsock2.h>

SOCKET passivesock(const char *, const char *, int);
/*-------------------------------------------------------------------------------------
* passiveUDP - create a passive socket for use in a UDP server
*-------------------------------------------------------------------------------------
*/
SOCKET passiveUDP(const char *service)
{
	return passivesock(service, "udp", 0);
}
