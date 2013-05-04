/*******************************************************************************
Copyright (C) Autelan Technology

This software file is owned and distributed by Autelan Technology
********************************************************************************

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND
ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE LIABLE FOR
ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
(INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON
ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
(INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
********************************************************************************
* stp_uid_sock.c
*
* CREATOR:
*       zhubo@autelan.com
*
* DESCRIPTION:
*       APIs for socket op in stp module
*
* DATE:
*       04/18/2008
*
*  FILE REVISION NUMBER:
*       $Revision: 1.1.1.1 $
*******************************************************************************/
#ifdef __cplusplus
    extern "C" {
#endif

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <strings.h>
#include <errno.h>
#include <sys/time.h>
#include <unistd.h>

#include "stp_uid_sock.h"

static int stp_uid_sock_init(int family, int type, int protocol)
{
  int sock_fd;

  sock_fd = socket(family, type, protocol);

  if (sock_fd < 0) {
    return -1;
  }

  return sock_fd;
}

#define UID_SET_SOCKET_ID(SPTR, X)	\
	strncpy((SPTR)->socket_id, (X),SOCKET_NAME_LENGTH); \
        (SPTR)->socket_id[SOCKET_NAME_LENGTH - 1] = '\0';

#define UID_SET_SOCKET_SERVER_ID(SPTR, X)	\
	(SPTR)->serverAddr.sun_path[0] = '\0';				\
	strncpy((SPTR)->serverAddr.sun_path + 1, (X),SOCKET_NAME_LENGTH - 1);	\
        (SPTR)->serverAddr.sun_path[SOCKET_NAME_LENGTH - 1] = '\0';

#define UID_SET_SOCKET_CLIENT_ID(SPTR, X)       \
	(SPTR)->clientAddr.sun_path[0] = '\0';				\
	strncpy((SPTR)->clientAddr.sun_path + 1, (X),SOCKET_NAME_LENGTH - 1); \
        (SPTR)->clientAddr.sun_path[SOCKET_NAME_LENGTH - 1] = '\0';

int
stp_uid_sock_compare (UID_SOCKET_T* s, UID_SOCKET_T* t)
{
  register char* ps;
  register char* pt;

  ps = s->clientAddr.sun_path;
  if (! *ps) ps++;
  if (! *ps) return 1;

  pt = t->clientAddr.sun_path;
  if (! *pt) pt++;
  if (! *pt) return 2;

  if (strcmp (pt, ps))
    return 3;
  else {
    return 0;
  }
}

int stp_uid_sock_SocketInit(UID_SOCKET_T* s,
                      UID_SOCK_ID socket_id,
                      TYPE_OF_BINDING binding)
{
  bzero(s, sizeof (UID_SOCKET_T));

  s->sock_fd = stp_uid_sock_init(AF_LOCAL, SOCK_DGRAM, 0);

  s->clientAddr.sun_family = AF_LOCAL;

  s->serverAddr.sun_family = AF_LOCAL;
  

  switch (binding) {
    case UID_BIND_AS_CLIENT:
      strncpy (s->socket_id, tmpnam(NULL),SOCKET_NAME_LENGTH );
      UID_SET_SOCKET_CLIENT_ID(s,s->socket_id );      
      if (bind(s->sock_fd, (SA *)&(s->clientAddr), SIZE_OF_ADDRESS) < 0) {
	return -2;
      }
      bzero(&s->serverAddr, SIZE_OF_ADDRESS);
      s->serverAddr.sun_family = AF_LOCAL;
      UID_SET_SOCKET_SERVER_ID(s, socket_id);

      break;

    case UID_BIND_AS_SERVER:
      unlink(socket_id);
      strncpy (s->socket_id, socket_id, SOCKET_NAME_LENGTH);
      UID_SET_SOCKET_SERVER_ID(s,s->socket_id);
      
      if (bind(s->sock_fd, (SA *)&(s->serverAddr), SIZE_OF_ADDRESS) < 0) {
         perror ("Error:");
         fflush (stdout);
         return -4;
      }

      break;

    default:
      return -5;
  }

  s->binding = binding;

  return 0;
}

int stp_uid_sock_SocketRecvfrom (UID_SOCKET_T* sock,
                        void* msg, int buffer_size,
                        UID_SOCKET_T* sock_4_reply)
{
  int		size;
  socklen_t	len = SIZE_OF_ADDRESS;

  while (1) {
      size = recvfrom(sock->sock_fd, msg, buffer_size, 0,
                  (struct sockaddr *)(((UID_BIND_AS_CLIENT == sock->binding) || !sock_4_reply) ?
                     NULL : &(sock_4_reply->clientAddr)),
                  (UID_BIND_AS_CLIENT == (sock->binding)) ?
                     NULL : &len);
      if (size < 0 && errno == EINTR) continue;
      break;
  }

  if ((UID_BIND_AS_CLIENT != sock->binding) && sock_4_reply) {
    sock_4_reply->sock_fd = sock->sock_fd;
    sock_4_reply->binding = UID_BIND_AS_SERVER;
  }

  return size;
}

int stp_uid_sock_SocketSendto (UID_SOCKET_T* sock, void* msg, int msg_size)
{
  int	rc, size = 0;

  while (size != msg_size) {
     rc  = sendto (sock->sock_fd, (msg+size), (msg_size-size), 0,
                  (struct sockaddr *)((UID_BIND_AS_CLIENT == sock->binding) ? &sock->serverAddr : &sock->clientAddr),
                  SIZE_OF_ADDRESS);
     
     if (rc < 0) {
        if (errno == EINTR) {
          continue;
        } else {
          return -1; 
        }
     }
     size += rc;
  }

  return 0;
}

int stp_uid_sock_SocketClose(UID_SOCKET_T* s)
{
  close (s->sock_fd);

  return 0;
}

int stp_uid_sock_SocketSetReadTimeout (UID_SOCKET_T* s, int timeout)
{
  int			retval = -1;
  struct timeval	wait;
  fd_set		read_mask;

  wait.tv_sec = timeout;
  wait.tv_usec = 0;

  FD_ZERO(&read_mask);
  FD_SET(s->sock_fd, &read_mask);

  retval = select (s->sock_fd + 1, &read_mask, NULL, NULL, &wait);

  if (retval < 0) {		// Error
    fprintf (stderr, "stp_uid_sock_SocketSetReadTimeout failed: %s\n", strerror(errno));
    return 0;
  } else if (! retval) {	// Timeout expired
    return 0;
  } else {			// 0
    ;
  }

  return retval;
}
#ifdef __cplusplus
}
#endif


