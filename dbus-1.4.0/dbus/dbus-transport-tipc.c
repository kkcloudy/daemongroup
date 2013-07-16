

#include <config.h>
#include <stdio.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <string.h>
#include <errno.h>
#include <unistd.h>
#include <netinet/in.h>
#include <linux/tipc.h>
#include "dbus-internals.h"
#include "dbus-connection-internal.h"
#include "dbus-transport-tipc.h"
#include "dbus-transport-socket.h"
#include "dbus-transport-protected.h"
#include "dbus-watch.h"
#include "dbus-sysdeps-unix.h"

int
_dbus_connect_tipc_socket (const char     *server_inst,
                           DBusError      *error);


/**
 * @defgroup DBusTransportUnix DBusTransport implementations for UNIX
 * @ingroup  DBusInternals
 * @brief Implementation details of DBusTransport on UNIX
 *
 * @{
 */

/**
 * Creates a new transport for the given Unix domain socket
 * path. This creates a client-side of a transport.
 *
 * @todo once we add a way to escape paths in a dbus
 * address, this function needs to do escaping.
 *
 * @param path the path to the domain socket.
 * @param abstract #TRUE to use abstract socket namespace
 * @param error address where an error can be returned.
 * @returns a new transport, or #NULL on failure.
 */
DBusTransport*
_dbus_transport_new_for_tipc_socket (const char     *server_inst,
                                       DBusError      *error)
{
  int fd;
  DBusTransport *transport;
  DBusString address;
  
  _DBUS_ASSERT_ERROR_IS_CLEAR (error);

  if (!_dbus_string_init (&address))
    {
      dbus_set_error (error, DBUS_ERROR_NO_MEMORY, NULL);
      return NULL;
    }

  fd = -1;

  if ((server_inst &&
       !_dbus_string_append (&address, "tipc:inst=")) ||
      !_dbus_string_append (&address, server_inst))
    {
      dbus_set_error (error, DBUS_ERROR_NO_MEMORY, NULL);
      goto failed_0;
    }
  fd = _dbus_connect_tipc_socket (server_inst,error);
  if (fd < 0)
    {
      _DBUS_ASSERT_ERROR_IS_SET (error);
      goto failed_0;
    }

  _dbus_verbose ("Successfully connected to tipc socket %s\n",
                 server_inst);

  transport = _dbus_transport_new_for_socket (fd, NULL, &address);
  if (transport == NULL)
    {
      dbus_set_error (error, DBUS_ERROR_NO_MEMORY, NULL);
      goto failed_1;
    }
  
  _dbus_string_free (&address);
  
  return transport;

 failed_1:
  _dbus_close_socket (fd, NULL);
 failed_0:
  _dbus_string_free (&address);
  return NULL;
}

/**
 * Opens tipc transport types.
 * 
 * @param entry the address entry to try opening
 * @param transport_p return location for the opened transport
 * @param error error to be set
 * @returns result of the attempt
 */
DBusTransportOpenResult
_dbus_transport_open_tipc (DBusAddressEntry  *entry,
                                        DBusTransport    **transport_p,
                                        DBusError         *error)
{
  const char *method;
  
  method = dbus_address_entry_get_method (entry);
  _dbus_assert (method != NULL);
  if (strcmp (method, "tipc") == 0)
    {
      const char *server_inst = dbus_address_entry_get_value (entry, "inst");
          
      if (server_inst)
        *transport_p = _dbus_transport_new_for_tipc_socket (server_inst,
                                                           error);
      if (*transport_p == NULL)
        {
          _DBUS_ASSERT_ERROR_IS_SET (error);
          return DBUS_TRANSPORT_OPEN_DID_NOT_CONNECT;
        }
      else
        {
          _DBUS_ASSERT_ERROR_IS_CLEAR (error);
          return DBUS_TRANSPORT_OPEN_OK;
        }      
    }
  else
    {
      _DBUS_ASSERT_ERROR_IS_CLEAR (error);
      return DBUS_TRANSPORT_OPEN_NOT_HANDLED;
    }
}

/** @} */
int
_dbus_connect_tipc_socket (const char     *server_inst,
                           DBusError      *error)
{
  int fd;
  unsigned int inst;
  size_t path_len;
  struct sockaddr_tipc addr;
sscanf(server_inst,"%d",&inst);
  _DBUS_ZERO (addr);
  addr.family = AF_TIPC;
  addr.addrtype = TIPC_ADDR_NAME;
  addr.addr.name.name.type = SERVER_TYPE;
  addr.addr.name.name.instance = inst;
  addr.addr.name.domain = 0;
  path_len = strlen (server_inst);
  
  _DBUS_ASSERT_ERROR_IS_CLEAR (error);

  _dbus_verbose ("connecting to tipc socket %s \n",
                 server_inst);
  


//  if (!_dbus_open_tipc_socket (&fd, &addr.addr.name.name, error))
	if (!_dbus_open_tipc_socket (&fd, error))

    {
      _DBUS_ASSERT_ERROR_IS_SET(error);
      return -1;
    }
  _DBUS_ASSERT_ERROR_IS_CLEAR(error);

  


  if (connect (fd, (struct sockaddr*) &addr, sizeof(addr)) < 0)
    {
      dbus_set_error (error,
                      _dbus_error_from_errno (errno),
                      "Failed to connect to tipc socket %s: %s",
                      server_inst, _dbus_strerror (errno));

      _dbus_close (fd, NULL);
      fd = -1;

      return -1;
    }
  if (!_dbus_set_fd_nonblocking (fd, error))
    {
      _DBUS_ASSERT_ERROR_IS_SET (error);

      _dbus_close (fd, NULL);
      fd = -1;

      return -1;
    }
  

  return fd;
}
/*
dbus_bool_t
_dbus_open_tipc_socket (int              *fd,
						struct tipc_name *name_tmp,
                        DBusError        *error)
*/
dbus_bool_t
_dbus_open_tipc_socket (int              *fd,
                        DBusError        *error)

{
	//wait_for_server(name_tmp,2);
  return __dbus_open_socket(fd, AF_TIPC, SOCK_STREAM, 0, error);
}

void wait_for_server(struct tipc_name* name,int wait)
{
	struct sockaddr_tipc topsrv;
	struct tipc_subscr subscr = {{name->type,name->instance,name->instance},
		wait,TIPC_SUB_SERVICE,{}};
	struct tipc_event event;
	int sd = socket (AF_TIPC, SOCK_SEQPACKET,0);
	if(sd == -1)/*coverity modify for CID 15134 */
	{
		exit(1);
	}
	memset(&topsrv,0,sizeof(topsrv));
	topsrv.family = AF_TIPC;
	topsrv.addrtype = TIPC_ADDR_NAME;
	topsrv.addr.name.name.type = TIPC_TOP_SRV;
	topsrv.addr.name.name.instance = TIPC_TOP_SRV;

	/* Connect to topology server: */

	if (0 > connect(sd,(struct sockaddr*)&topsrv,sizeof(topsrv))) {
		perror("failed to connect to topology server");
		exit(1);
	}
	if (send(sd,&subscr,sizeof(subscr),0) != sizeof(subscr)) {
		perror("failed to send subscription");
		exit(1);
	}
	/* Now wait for the subscription to fire: */
	if (recv(sd,&event,sizeof(event),0) != sizeof(event)) {
		perror("Failed to receive event");
		exit(1);
	}
	if (event.event != TIPC_PUBLISHED) {
		printf("Server %u,%u not published within %u [s] ---\n",
		       name->type,name->instance,wait/1000);
		exit(1);
	}
	close(sd);
}

static dbus_bool_t
__dbus_open_socket (int              *fd_p,
                   int               domain,
                   int               type,
                   int               protocol,
                   DBusError        *error)
{
#ifdef SOCK_CLOEXEC
  dbus_bool_t cloexec_done;

  *fd_p = socket (domain, type | SOCK_CLOEXEC, protocol);
  cloexec_done = *fd_p >= 0;

  /* Check if kernel seems to be too old to know SOCK_CLOEXEC */
  if (*fd_p < 0 && errno == EINVAL)
#endif
    {
      *fd_p = socket (domain, type, protocol);
    }

  if (*fd_p >= 0)
    {
#ifdef SOCK_CLOEXEC
      if (!cloexec_done)
#endif
        {
          _dbus_fd_set_close_on_exec(*fd_p);
        }

      _dbus_verbose ("socket fd %d opened\n", *fd_p);
	  
      return TRUE;
    }
  else
    {
      dbus_set_error(error,
                     _dbus_error_from_errno (errno),
                     "Failed to open socket: %s",
                     _dbus_strerror (errno));
      return FALSE;
    }
}



