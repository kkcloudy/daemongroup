/* -*- mode: C; c-file-style: "gnu"; indent-tabs-mode: nil; -*- */
/* dbus-server-unix.c Server implementation for Unix network protocols.
 *
 * Copyright (C) 2002, 2003, 2004  Red Hat Inc.
 *
 * Licensed under the Academic Free License version 2.1
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA
 *
 */

#include <config.h>
#include <sys/types.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <netinet/in.h>
#include <linux/tipc.h>
#include <stdio.h>
#ifdef HAVE_ERRNO_H
#include <errno.h>
#endif

#include "dbus-internals.h"
#include "dbus-server-tipc.h"
#include "dbus-server-unix.h"
#include "dbus-server-socket.h"
#include "dbus-transport-unix.h"
#include "dbus-connection-internal.h"
#include "dbus-sysdeps-unix.h"
#include "dbus-string.h"

#define SERVER_TYPE 0x3000
/**
 * @defgroup DBusServerUnix DBusServer implementations for UNIX
 * @ingroup  DBusInternals
 * @brief Implementation details of DBusServer on UNIX
 *
 * @{
 */

/**
 * Tries to interpret the address entry in a platform-specific
 * way, creating a platform-specific server type if appropriate.
 * Sets error if the result is not OK.
 *
 * @param entry an address entry
 * @param server_p location to store a new DBusServer, or #NULL on failure.
 * @param error location to store rationale for failure on bad address
 * @returns the outcome
 *
 */
	DBusServerListenResult
	_dbus_server_listen_tipc_socket (DBusAddressEntry *entry,
										   DBusServer	   **server_p,
										   DBusError		*error)
	{
	  const char *method;
	
	  *server_p = NULL;
	
	  method = dbus_address_entry_get_method (entry);
	
	  if (strcmp (method, "tipc") == 0)
		{
		  const char *server_inst = dbus_address_entry_get_value (entry, "inst");
/*		  fprintf(stderr,"dbus-daemon server : got inst mothed ,and listen list is %s\n",server_inst); */
	
		  if (server_inst == NULL)
			{
			  _dbus_set_bad_address(error, "tipc",
									"server_inst",
									NULL);
			  return DBUS_SERVER_LISTEN_BAD_ADDRESS;
			}
	
			  *server_p =
				_dbus_server_new_for_tipc_domain_socket (server_inst,error);
	
	
		  if (*server_p != NULL)
			{
			  _DBUS_ASSERT_ERROR_IS_CLEAR(error);
			  return DBUS_SERVER_LISTEN_OK;
			}
		  else
			{
			  _DBUS_ASSERT_ERROR_IS_SET(error);
			  return DBUS_SERVER_LISTEN_DID_NOT_CONNECT;
			}
		}
	  else
		{
		  /* If we don't handle the method, we return NULL with the
		   * error unset
		   */
		  _DBUS_ASSERT_ERROR_IS_CLEAR(error);
		  return DBUS_SERVER_LISTEN_NOT_HANDLED;
		}
	}
/**
 * Creates a new server listening on the given Unix domain socket.
 *
 * @param path the path for the domain socket.
 * @param abstract #TRUE to use abstract socket namespace
 * @param error location to store reason for failure.
 * @returns the new server, or #NULL on failure.
 */
DBusServer*
_dbus_server_new_for_tipc_domain_socket (const char     *server_inst,
                                    DBusError      *error)
{
  DBusServer *server;
  int listen_fd;
  DBusString address;
  char *path_copy;
  DBusString path_str;

  _DBUS_ASSERT_ERROR_IS_CLEAR (error);

  if (!_dbus_string_init (&address))
    {
      dbus_set_error (error, DBUS_ERROR_NO_MEMORY, NULL);
      return NULL;
    }

  _dbus_string_init_const (&path_str, server_inst);
  if ((server_inst &&
       !_dbus_string_append (&address, "tipc:inst="))||
      !_dbus_address_append_escaped (&address, &path_str))
    {
      dbus_set_error (error, DBUS_ERROR_NO_MEMORY, NULL);
      goto failed_0;
    }

  path_copy = _dbus_strdup (server_inst);
  if (path_copy == NULL)
    {
      dbus_set_error (error, DBUS_ERROR_NO_MEMORY, NULL);
      goto failed_0;
    }

  listen_fd = _dbus_listen_tipc_socket (server_inst, error);

  if (listen_fd < 0)
    {
      _DBUS_ASSERT_ERROR_IS_SET (error);
      goto failed_1;
    }

  server = _dbus_server_new_for_socket (&listen_fd, 1, &address, 0);
  if (server == NULL)
    {
      dbus_set_error (error, DBUS_ERROR_NO_MEMORY, NULL);
      goto failed_2;
    }

  _dbus_server_socket_own_filename(server, path_copy);

  _dbus_string_free (&address);

  return server;

 failed_2:
  _dbus_close_socket (listen_fd, NULL);
 failed_1:
  dbus_free (path_copy);
 failed_0:
  _dbus_string_free (&address);

  return NULL;
}

/** @} */



int
_dbus_listen_tipc_socket (const char     *server_inst,
                          DBusError      *error)
{
  int listen_fd;
  struct sockaddr_tipc server_addr;
  size_t path_len;
  unsigned int inst;
  unsigned int reuseaddr;

  _DBUS_ASSERT_ERROR_IS_CLEAR (error);

  _dbus_verbose ("listening on tipc socket inst %s\n",
                 server_inst);

  if (!__dbus_open_tipc_socket (&listen_fd, error))
    {
      _DBUS_ASSERT_ERROR_IS_SET(error);
      return -1;
    }
  _DBUS_ASSERT_ERROR_IS_CLEAR(error);

  _DBUS_ZERO (server_addr);
  sscanf(server_inst,"%d",&inst);
  /*fprintf(stderr,"listening on tipc socket inst %d\n",inst);*/
  server_addr.family = AF_TIPC;
  server_addr.addrtype = TIPC_ADDR_NAMESEQ;
  server_addr.addr.nameseq.type = SERVER_TYPE;
  server_addr.addr.nameseq.lower = inst;
  server_addr.addr.nameseq.upper = inst;
  server_addr.scope = TIPC_CLUSTER_SCOPE;
  
  path_len = strlen (server_inst);

  if (bind (listen_fd, (struct sockaddr*) &server_addr, sizeof(server_addr)))
    {
      dbus_set_error (error, _dbus_error_from_errno (errno),
                      "Failed to bind socket \"%s\": %s",
                      server_inst, _dbus_strerror (errno));
      _dbus_close (listen_fd, NULL);
      return -1;
    }

  if (listen (listen_fd, 30 /* backlog */) < 0)
    {
      dbus_set_error (error, _dbus_error_from_errno (errno),
                      "Failed to listen on socket \"%s\": %s",
                      server_inst, _dbus_strerror (errno));
      _dbus_close (listen_fd, NULL);
      return -1;
    }

  if (!_dbus_set_local_creds (listen_fd, TRUE))
    {
      dbus_set_error (error, _dbus_error_from_errno (errno),
                      "Failed to enable LOCAL_CREDS on socket \"%s\": %s",
                      server_inst, _dbus_strerror (errno));
	  shutdown(listen_fd,SHUT_RDWR);
      close (listen_fd);
      return -1;
    }

  if (!_dbus_set_fd_nonblocking (listen_fd, error))
    {
      _DBUS_ASSERT_ERROR_IS_SET (error);
      _dbus_close (listen_fd, NULL);
      return -1;
    }


  return listen_fd;
}

dbus_bool_t
__dbus_open_tipc_socket (int              *fd,
                        DBusError        *error)
{
#ifdef SOCK_CLOEXEC
	dbus_bool_t cloexec_done;
  
	*fd = socket (AF_TIPC, SOCK_STREAM | SOCK_CLOEXEC, 0);
	cloexec_done = *fd >= 0;
  
	/* Check if kernel seems to be too old to know SOCK_CLOEXEC */
	if (*fd < 0 && errno == EINVAL)
#endif
	  {
		*fd = socket (AF_TIPC, SOCK_STREAM, 0);
	  }
  
	if (*fd >= 0)
	  {
#ifdef SOCK_CLOEXEC
		if (!cloexec_done)
#endif
		  {
			_dbus_fd_set_close_on_exec(*fd);
		  }
  
		_dbus_verbose ("socket fd %d opened\n", *fd);
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
static dbus_bool_t
_dbus_set_local_creds (int fd, dbus_bool_t on)
{
  dbus_bool_t retval = TRUE;

#if defined(HAVE_CMSGCRED)
  /* NOOP just to make sure only one codepath is used
   *      and to prefer CMSGCRED
   */
#elif defined(LOCAL_CREDS)
  int val = on ? 1 : 0;
  if (setsockopt (fd, 0, LOCAL_CREDS, &val, sizeof (val)) < 0)
    {
      _dbus_verbose ("Unable to set LOCAL_CREDS socket option on fd %d\n", fd);
      retval = FALSE;
    }
  else
    _dbus_verbose ("LOCAL_CREDS %s for further messages on fd %d\n",
                   on ? "enabled" : "disabled", fd);
#endif

  return retval;
}


