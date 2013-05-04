#ifndef _STP_UID_SOCKET_H__ 
#define _STP_UID_SOCKET_H__

/* Socket API */

#include        <sys/socket.h>  /* basic socket definitions */
#include        <netinet/in.h>
#include        <linux/un.h>              /* for Unix domain sockets */

#define UID_REPL_PATH   "/tmp/UidSocketFile"

typedef struct sockaddr SA;

#define SOCKET_NAME_LENGTH 108
#define SIZE_OF_ADDRESS sizeof(struct sockaddr_un)

typedef enum {
  UID_BIND_AS_CLIENT,
  UID_BIND_AS_SERVER
} TYPE_OF_BINDING;


typedef char        UID_SOCK_ID[SOCKET_NAME_LENGTH];

typedef struct{
  int           sock_fd;
  struct sockaddr_un    clientAddr;
  struct sockaddr_un    serverAddr; // Only for socket of UID_BIND_AS_CLIENT
  UID_SOCK_ID       socket_id;
  TYPE_OF_BINDING   binding;
} UID_SOCKET_T;

#define MESSAGE_SIZE        2048

int stp_uid_sock_SocketInit(UID_SOCKET_T* sock,
            UID_SOCK_ID id,
            TYPE_OF_BINDING binding);

int stp_uid_sock_SocketRecvfrom (UID_SOCKET_T* sock,
            void* msg_buffer,
            int buffer_size,
            UID_SOCKET_T* sock_4_reply);

int stp_uid_sock_SocketSendto (UID_SOCKET_T* sock,
            void* msg_buffer,
            int buffer_size);

int stp_uid_sock_SocketClose(UID_SOCKET_T* sock);

int stp_uid_sock_SocketSetReadTimeout (UID_SOCKET_T* s, int timeout);

int
stp_uid_sock_compare (UID_SOCKET_T* s, UID_SOCKET_T* t);

#define GET_FILE_DESCRIPTOR(sock) (((UID_SOCKET_T*)sock)->sock_fd)

#endif /* _UID_SOCKET_H__ */


