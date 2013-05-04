#ifndef _STP_NPD_CMD_H__
#define _STP_NPD_CMD_H__

typedef enum
{
	CPSS_RSTP_PORT_STATE_DISABLE_E = 0,
	CPSS_RSTP_PORT_STATE_DISCARD_E,
	CPSS_RSTP_PORT_STATE_LEARN_E,
	CPSS_RSTP_PORT_STATE_FORWARD_E
}RSTP_PORT_STATE_E;

typedef enum
{
   NAM_STP_PORT_STATE_DISABLE_E = 0,
   NAM_STP_PORT_STATE_DISCARD_E,
   NAM_STP_PORT_STATE_LEARN_E,
   NAM_STP_PORT_STATE_FORWARD_E

}NAM_RSTP_PORT_STATE_E;

typedef enum 
{
		PORT_ENABLE_E = 0,
		PORT_DISABLE_E,
		INTERFACE_ADD_E,
		INTERFACE_DEL_E,
		VLAN_ADD_ON_MST_E,
		VLAN_DEL_ON_MST_E,
		FDB_ENTRY_ADD_E,
		FDB_ENTRY_DEL_E,
		FDB_ENTRY_CLEAR_E,
		STP_STATE_UPDATE_E,
		STP_RECEIVE_TCN_E,
		LINK_CHANGE_EVENT_E,
		STP_GET_SYS_MAC_E,
		RSTP_OP_TYPE_MAX
}CMD_TYPE_E;

typedef enum _rstp_link_event 
{
    LINK_PORT_UP_E = 0, 
    LINK_PORT_DOWN_E,
	LINK_PORT_MAX
}RSTP_LINK_ENT;

typedef struct __CMD_INTF_STC_
{
	unsigned short vid;
	unsigned int		portIdx;	
	unsigned int isWan;
}CMD_INTF_STC;

typedef struct __CMD_VLAN_STC_
{
	unsigned short vid;
	unsigned int 	untagbmp[2];
	unsigned int	 	tagbmp[2];
}CMD_VLAN_STC;

typedef struct __CMD_FDB_STC_
{
	unsigned int		portIdx;
	unsigned char	MacDa[6];
}CMD_FDB_STC;

typedef struct __CMD_STP_STC_
{
	unsigned int 				mstid;
	unsigned short 			vid;
	unsigned int					port_index;
	NAM_RSTP_PORT_STATE_E	portState;
}CMD_STP_STC;

typedef struct __CMD_LINK_STC_
{
	unsigned int			portIdx;
	RSTP_LINK_ENT	portState;
	unsigned int    speed;
	unsigned int    duplex_mode;
	unsigned int    isWAN;	
	unsigned int    slot_no;
	unsigned int	port_no;
}CMD_LINK_STC;


typedef union 
{
	CMD_INTF_STC	cmdIntf;
	CMD_VLAN_STC  cmdVlan;
	CMD_FDB_STC		cmdFdb;
	CMD_STP_STC		cmdStp;
	CMD_LINK_STC	cmdLink;
	
}CMD_VALUE_UNION;

typedef struct __NPD_CMD_STC_
{
	CMD_TYPE_E				cmdType;
	unsigned int					cmdLen;
	CMD_VALUE_UNION	cmdData;
}NPD_CMD_STC;

/*
typedef struct
{
	int ret;
	NPD_CMD_STC cmd;
}NPD_SEND_INFO;
*/

int stp_npd_socket_init(void);

int stp_npd_read_socket(void);

int stp_npd_cmd_send_stp_info
(
	unsigned int mstid,
	unsigned short vid,
	unsigned int port_index,
	NAM_RSTP_PORT_STATE_E state
);
int stp_npd_cmd_send_stp_mac();

#endif


