#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <signal.h>
#include <syslog.h>
#include <ctype.h>
#include <math.h>
#include <sys/stat.h>
#include <time.h>
#include <dbus/dbus.h>

#include "m_log.h"

void m_openlog(void)
{
	openlog("trap-helper", LOG_PID, LOG_DAEMON);
}

void m_syslog(int priority,const char * message,...) 
{
	va_list args;

	va_start(args, message);

	m_vsyslog(priority, message, args);

	va_end (args);
}

void m_vsyslog(int priority,const char * message,va_list args)
{
	vsyslog(priority, message, args);

	if (LOG_ERR == priority)
		exit(1);
}

