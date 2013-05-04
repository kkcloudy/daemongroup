/* iu_log.c  */

#include <string.h>
#include <errno.h>
#include <syslog.h>
#include <stdarg.h>
#include <stdio.h>
#include <sys/types.h>
#include <sys/uio.h>
#include <unistd.h>

#include "sigtran2udp_log.h"

#define IU_STDERR_FILENO 2

#ifdef DEBUG
int log_perror = -1;
#else
int log_perror = 1;
#endif
int log_priority;
void (*log_cleanup) (void);

#define CVT_BUF_MAX 1023
static char mbuf [CVT_BUF_MAX + 1];
static char fbuf [CVT_BUF_MAX + 1];

unsigned int iu_log_level;


#if !defined(__GNUC__) || (__GNUC__ < 4) || \
	((__GNUC__ == 4) && (__GNUC_MINOR__ < 3))
#define IGNORE_RET(x) (void) x
#else
#define IGNORE_RET(x)			\
	do {				\
		int ignore_return;	\
		ignore_return = x;	\
	} while (0)
#endif

/* Find %m in the input string and substitute an error message string. */

void do_percentm (obuf, ibuf)
     char *obuf;
     const char *ibuf;
{
	const char *s = ibuf;
	char *p = obuf;
	int infmt = 0;
	const char *m;
	int len = 0;

	while (*s) {
		if (infmt) {
			if (*s == 'm') {
#ifndef __CYGWIN32__
				m = strerror (errno);
#else
				m = pWSAError ();
#endif
				if (!m)
					m = "<unknown error>";
				len += strlen (m);
				if (len > CVT_BUF_MAX)
					goto out;
				strcpy (p - 1, m);
				p += strlen (p);
				++s;
			} else {
				if (++len > CVT_BUF_MAX)
					goto out;
				*p++ = *s++;
			}
			infmt = 0;
		} else {
			if (*s == '%')
				infmt = 1;
			if (++len > CVT_BUF_MAX)
				goto out;
			*p++ = *s++;
		}
	}
      out:
	*p = 0;
}



/* Log an error message... */

int iu_log_error (const char * fmt, ...)
{
  va_list list;

  do_percentm (fbuf, fmt);

  /* %Audit% This is log output. %2004.06.17,Safe%
   * If we truncate we hope the user can get a hint from the log.
   */

  va_start (list, fmt);
  vsnprintf (mbuf, sizeof mbuf, fbuf, list);
  va_end (list);

#ifndef DEBUG
	if(iu_log_level & DEBUG_TYPE_ERROR){
  		syslog (log_priority | LOG_ERR, "%s", mbuf);
	}
#endif
 
  IGNORE_RET (write (IU_STDERR_FILENO, mbuf, strlen (mbuf)));
  IGNORE_RET (write (IU_STDERR_FILENO, "\n", 1));

 
  return 0;
}


/* Log a note... */

int iu_log_info (const char *fmt, ...)
{
  va_list list;
	  do_percentm (fbuf, fmt);

	  /* %Audit% This is log output. %2004.06.17,Safe%
	   * If we truncate we hope the user can get a hint from the log.
	   */
	  va_start (list, fmt);
	  vsnprintf (mbuf, sizeof mbuf, fbuf, list);
	  va_end (list);

#ifndef DEBUG
	if(iu_log_level & DEBUG_TYPE_INFO){
	  	syslog (log_priority | LOG_INFO, "%s", mbuf);
	}
#endif

	  IGNORE_RET (write (IU_STDERR_FILENO, mbuf, strlen (mbuf)));
	  IGNORE_RET (write (IU_STDERR_FILENO, "\n", 1));


  return 0;
}

/* Log a debug message... */

int iu_log_debug (const char *fmt, ...)
{
  va_list list;
  do_percentm (fbuf, fmt);

  /* %Audit% This is log output. %2004.06.17,Safe%
   * If we truncate we hope the user can get a hint from the log.
   */
  va_start (list, fmt);
  vsnprintf (mbuf, sizeof mbuf, fbuf, list);
  va_end (list);

#ifndef DEBUG
	  if(iu_log_level & DEBUG_TYPE_DEBUG){
	  	syslog (log_priority | LOG_DEBUG, "%s", mbuf);
	  }
#endif

	  IGNORE_RET (write (IU_STDERR_FILENO, mbuf, strlen (mbuf)));
	  IGNORE_RET (write (IU_STDERR_FILENO, "\n", 1));


  return 0;
}




#ifdef NO_STRERROR
char *strerror (err)
	int err;
{
	extern char *sys_errlist [];
	extern int sys_nerr;
	static char errbuf [128];

	if (err < 0 || err >= sys_nerr) {
		sprintf (errbuf, "Error %d", err);
		return errbuf;
	}
	return sys_errlist [err];
}
#endif /* NO_STRERROR */

#ifdef _WIN32
char *pWSAError ()
{
  int err = WSAGetLastError ();

  switch (err)
    {
    case WSAEACCES:
      return "Permission denied";
    case WSAEADDRINUSE:
      return "Address already in use";
    case WSAEADDRNOTAVAIL:
      return "Cannot assign requested address";
    case WSAEAFNOSUPPORT:
      return "Address family not supported by protocol family";
    case WSAEALREADY:
      return "Operation already in progress";
    case WSAECONNABORTED:
      return "Software caused connection abort";
    case WSAECONNREFUSED:
      return "Connection refused";
    case WSAECONNRESET:
      return "Connection reset by peer";
    case WSAEDESTADDRREQ:
      return "Destination address required";
    case WSAEFAULT:
      return "Bad address";
    case WSAEHOSTDOWN:
      return "Host is down";
    case WSAEHOSTUNREACH:
      return "No route to host";
    case WSAEINPROGRESS:
      return "Operation now in progress";
    case WSAEINTR:
      return "Interrupted function call";
    case WSAEINVAL:
      return "Invalid argument";
    case WSAEISCONN:
      return "Socket is already connected";
    case WSAEMFILE:
      return "Too many open files";
    case WSAEMSGSIZE:
      return "Message too long";
    case WSAENETDOWN:
      return "Network is down";
    case WSAENETRESET:
      return "Network dropped connection on reset";
    case WSAENETUNREACH:
      return "Network is unreachable";
    case WSAENOBUFS:
      return "No buffer space available";
    case WSAENOPROTOOPT:
      return "Bad protocol option";
    case WSAENOTCONN:
      return "Socket is not connected";
    case WSAENOTSOCK:
      return "Socket operation on non-socket";
    case WSAEOPNOTSUPP:
      return "Operation not supported";
    case WSAEPFNOSUPPORT:
      return "Protocol family not supported";
    case WSAEPROCLIM:
      return "Too many processes";
    case WSAEPROTONOSUPPORT:
      return "Protocol not supported";
    case WSAEPROTOTYPE:
      return "Protocol wrong type for socket";
    case WSAESHUTDOWN:
      return "Cannot send after socket shutdown";
    case WSAESOCKTNOSUPPORT:
      return "Socket type not supported";
    case WSAETIMEDOUT:
      return "Connection timed out";
    case WSAEWOULDBLOCK:
      return "Resource temporarily unavailable";
    case WSAHOST_NOT_FOUND:
      return "Host not found";
#if 0
    case WSA_INVALID_HANDLE:
      return "Specified event object handle is invalid";
    case WSA_INVALID_PARAMETER:
      return "One or more parameters are invalid";
    case WSAINVALIDPROCTABLE:
      return "Invalid procedure table from service provider";
    case WSAINVALIDPROVIDER:
      return "Invalid service provider version number";
    case WSA_IO_PENDING:
      return "Overlapped operations will complete later";
    case WSA_IO_INCOMPLETE:
      return "Overlapped I/O event object not in signaled state";
    case WSA_NOT_ENOUGH_MEMORY:
      return "Insufficient memory available";
#endif
    case WSANOTINITIALISED:
      return "Successful WSAStartup not yet performer";
    case WSANO_DATA:
      return "Valid name, no data record of requested type";
    case WSANO_RECOVERY:
      return "This is a non-recoverable error";
#if 0
    case WSAPROVIDERFAILEDINIT:
      return "Unable to initialize a service provider";
    case WSASYSCALLFAILURE:
      return "System call failure";
#endif
    case WSASYSNOTREADY:
      return "Network subsystem is unavailable";
    case WSATRY_AGAIN:
      return "Non-authoritative host not found";
    case WSAVERNOTSUPPORTED:
      return "WINSOCK.DLL version out of range";
    case WSAEDISCON:
      return "Graceful shutdown in progress";
#if 0
    case WSA_OPERATION_ABORTED:
      return "Overlapped operation aborted";
#endif
    }
  return "Unknown WinSock error";
}




#endif /* _WIN32 */
