#ifndef _STP_MNGR_H__
#define _STP_MNGR_H__
 
typedef enum {
  UID_CNTRL = 0,
  UID_BPDU
} UID_CMD_TYPE_T;

typedef enum {
  UID_PORT_CONNECT,
  UID_PORT_DISCONNECT,
  UID_BRIDGE_SHUTDOWN,
  UID_BRIDGE_HANDSHAKE,
  UID_LAST_DUMMY
} UID_CNTRL_CMD_T;

typedef struct uid_port_control_s {
  UID_CNTRL_CMD_T cmd;
  unsigned long  param1;  
  unsigned long  param2;  
} UID_CNTRL_BODY_T;

typedef struct uid_msg_header_s {
  UID_CMD_TYPE_T    cmd_type;
  long          sender_pid;
  int           destination_port;
  int           source_port;
  size_t        body_len;
} UID_MSG_HEADER_T;

typedef struct uid_msg_s {
	unsigned int port_index;
	unsigned int bodyLen;
	unsigned char *bpdu;
} UID_MSG_T;

#define MAX_UID_MSG_SIZE    1200
/*#define MAX_UID_MSG_SIZE    sizeof(UID_MSG_T)*/
#define STP_MallocMsg( size)  malloc(size + sizeof(UID_MSG_T))

#define STP_FreeMsg( Msg)  free((char *)Msg - sizeof(UID_MSG_T))


int stp_uid_bpduPktSocketInit(void);

char stp_uid_read_socket(void);

#endif
