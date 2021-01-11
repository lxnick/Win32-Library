#include "stdafx.h"

#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <winsock.h>

#define ERROR_MESSAGE	(256)

static char buffer[ERROR_MESSAGE];

void
errexit(const char *format, ...)
{
	va_list args;
	va_start(args, format);
	vfprintf(stderr, format, args);
	va_end(args);
	WSACleanup();
	exit(1); 
}

char* 
errmessage(int errnum)
{
	strerror_s(buffer,sizeof(buffer), errnum);
	return buffer;
}
