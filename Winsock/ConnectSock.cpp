/* consock.cpp - connectsock */

#include "stdafx.h"
#include <winsock2.h>
#include <ws2tcpip.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>


#ifndef INADDR_NONE
#define INADDR_NONE 0xffffffff
#endif /* INADDR_NONE */

void errexit(const char *, ...);
/*-------------------------------------------------------
* connectsock - allocate & connect a socket using TCP or UDP
*------------------------------------------------------
*/
SOCKET connectsock(const char *host, const char *service, const char
	*transport)
{
	struct hostent *phe; /* pointer to host information entry */
	struct servent *pse; /* pointer to service information entry */
	struct protoent *ppe; /* pointer to protocol information entry */
	struct sockaddr_in sin;/* an Internet endpoint address */
	int s, type; /* socket descriptor and socket type */
	memset(&sin, 0, sizeof(sin));
	sin.sin_family = AF_INET;
	/* Map service name to port number */
	if (pse = getservbyname(service, transport))
		sin.sin_port = pse->s_port;
	else if ((sin.sin_port = htons((u_short)atoi(service))) == 0)
		errexit("can't get \"%s\" service entry\n", service);
	/* Map host name to IP address, allowing for dotted decimal */

//	int status;
//	struct addrinfo hints;
//	struct addrinfo *servinfo; 
//
//	memset(&hints, 0, sizeof hints); 
//	hints.ai_family = AF_UNSPEC; 
//	hints.ai_socktype = SOCK_STREAM; 
//	hints.ai_flags = AI_PASSIVE; 
//
//	if ((status = getaddrinfo(host, "3490", &hints, &servinfo)) != 0) {
//		fprintf(stderr, "getaddrinfo error: %s\n", gai_strerror(status));
//		exit(1);
//	}

	if (phe = gethostbyname(host))
		memcpy(&sin.sin_addr, phe->h_addr, phe->h_length);
	else if ((sin.sin_addr.s_addr = inet_addr(host)) == INADDR_NONE)
		errexit("can't get \"%s\" host entry\n", host);


	/* Map protocol name to protocol number */
	if ((ppe = getprotobyname(transport)) == 0)
		errexit("can't get \"%s\" protocol entry\n", transport);
	/* Use protocol to choose a socket type */
	if (strcmp(transport, "udp") == 0)
		type = SOCK_DGRAM;
	else
		type = SOCK_STREAM;
	/* Allocate a socket */
	s = socket(PF_INET, type, ppe->p_proto);
	if (s == INVALID_SOCKET)
		errexit("can't create socket: %d\n", GetLastError());
	/* Connect the socket */
	if (connect(s, (struct sockaddr *)&sin, sizeof(sin)) == SOCKET_ERROR)
		errexit("can't connect to %s.%s: %d\n", host, service,
			GetLastError());
	return s;
}