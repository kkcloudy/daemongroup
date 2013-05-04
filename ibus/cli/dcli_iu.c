#include <string.h>
#include <dbus/dbus.h>
#include <sys/stat.h>
#include <sys/wait.h>
#include <fcntl.h>
#include <unistd.h>
#include <zebra.h>
#include "command.h"
#include <dirent.h>
#include "../../dcli/src/lib/dcli_main.h"
#include "iu/dcli_mapi_iu.h"
#include "mapi_iu.h"
#include "dcli_iu.h"
#include "dbus/iu/IuDBusPath.h"
#include "../../accapi/dbus/hmd/HmdDbusDef.h"

#define MAX_STR_LEN			(1024*1024)
#define POINT_CODE_LEN		(16)

typedef enum{
	IU_24BIT_POINT_CODE,
	IU_14BIT_POINT_CODE,
	IU_INTEGER_POINT_CODE
}PC_FORMAT;

struct cmd_node iu_node = 
{
	IU_NODE,
	"%s(config-iu)# ",
	1
};
struct cmd_node hansi_iu_node =
{
	HANSI_IU_NODE,
	"%s(hansi-iu %d-%d)# ",
};
struct cmd_node local_hansi_iu_node =
{
	LOCAL_HANSI_IU_NODE,
	"%s(local_hansi-iu %d-%d)# ",
};

/**********************************************************************************
 *  iu_ip2ulong
 *
 *	DESCRIPTION:
 * 		convert IP (A.B.C.D) to IP (ABCD) pattern
 *
 *	INPUT:
 *		str - (A.B.C.D)
 *	
 *	OUTPUT:
 *		null
 *
 * 	RETURN:
 *		
 *		IP	-  ip (ABCD)
 *		
 **********************************************************************************/

static unsigned long iu_ip2ulong(char *str)
{
	char *sep=".";
	char *token = NULL;
	unsigned long ip_long[4]; 
	unsigned long ip = 0;
	int i = 1;
	
	token=strtok(str,sep);
	if(NULL != token){
	    ip_long[0] = strtoul(token,NULL,10);
	}
	while((token!=NULL)&&(i<4))
	{
		token=strtok(NULL,sep);
		if(NULL != token){
		    ip_long[i] = strtoul(token,NULL,10);
		}
		i++;
	}

	ip=(ip_long[0]<<24)+(ip_long[1]<<16)+(ip_long[2]<<8)+ip_long[3];

	return ip;
}


/*****************************************************
** DISCRIPTION:
**          transform string format point code to integer
** INPUT:
**          string format point code
** OUTPUT:
**          integer format point code
** RETURN:
**          void
** DATE:	
**			2011-09-13
** AUTHOR:
**			<zhangshu@autelan.com>
*****************************************************/
void iu_parse_point_code(char *point_code, int *ptc, int type)
{
	char *ptc_str1 = NULL;
	char *ptc_str2 = NULL;
	char *ptc_str3 = NULL;
	int ptc_int1 = 0;
	int ptc_int2 = 0;
	int ptc_int3 = 0;
	    
	if(point_code == NULL)
	    return;

	ptc_str1 = strtok(point_code, "-");
	if(ptc_str1 == NULL)
		return;
	else
		ptc_int1 = atoi(ptc_str1);
	
	ptc_str2 = strtok(NULL,"-");
	if(ptc_str2 == NULL) 
		return;
	else
		ptc_int2 = atoi(ptc_str2);
	
	ptc_str3 = strtok(NULL,"-");
	if(ptc_str3 == NULL) 
		return;
	else
		ptc_int3 = atoi(ptc_str3);

	if(type == IU_24BIT_POINT_CODE){
		*ptc = ptc_int1 * 256 * 256 + ptc_int2 * 256 + ptc_int3;
	}
	else if(type == IU_14BIT_POINT_CODE){
		*ptc = ptc_int1 * 256 * 8 + ptc_int2 * 8 + ptc_int3;
	}

	return;
}


DEFUN(config_iu_mode_cmd_func,
		config_iu_mode_cmd,
		"config iu",
		CONFIG_STR
		"iu config node\n"
     )
{
	int ret = 0;
	int islocal = 1;
	int slot_id = HostSlotId;
	int insid = 0;
	if(vty->node == CONFIG_NODE){
		insid = 0;
	}else if(vty->node == HANSI_NODE){
		insid = vty->index;
		islocal = vty->local;
		slot_id = vty->slotindex;
	}else if(vty->node == LOCAL_HANSI_NODE){
		insid = vty->index;
		islocal = vty->local;
		slot_id = vty->slotindex;
	}
	DBusConnection *dbus_connection = NULL;
	ReInitFemtoDbusConnection(&dbus_connection, slot_id, distributFag);
	ret = mapi_femto_service_state_check(insid, islocal, slot_id, 1, dbus_connection, HMD_DBUS_CONF_METHOD_FEMTO_SERVICE_CHECK);
	if(ret == HMD_DBUS_SUCCESS)
	{
		if(vty->node == CONFIG_NODE)
		{
			vty->node = IU_NODE;
		}
		else if(vty->node == HANSI_NODE)
		{
			vty->node = HANSI_IU_NODE;
		}
		else if(vty->node == LOCAL_HANSI_NODE)
		{
			vty->node = LOCAL_HANSI_IU_NODE;
		}
	}
	else
		vty_out(vty, "<error> you should enable iu first!!\n");

	return CMD_SUCCESS;
}


/*****************************************************
** DISCRIPTION:
**          set  point-code by str
** INPUT:
**          
** OUTPUT:
**          
** RETURN:
**          void
** DATE:	
**			2011-12-28
** AUTHOR:
**			<zhangshu@autelan.com>
*****************************************************/
DEFUN(set_iu_point_code_str_func,
	  set_iu_point_code_str_cmd,
	  "set (local|remote) (msc|sgsn) point-code POINTCODE format (8-8-8|3-8-3)",
	  "set point code string format\n"
	  "local is HNB-GW, remote is CN\n"
	  "msc is IU-CS, sgsn is IU-PS\n"
	  "set point code X-X-X\n"
	  "set point code X-X-X\n"
	 )
{
	int ret = 0;
	unsigned char is_ps = 0;
	unsigned char is_local = 0;
	char point_code[POINT_CODE_LEN] = {0};
	unsigned int self_pd = 0;
	int type = 0;
	if(!strcmp((char*)argv[2],"0-0-0"))
	{
		vty_out(vty, "illegal parameter 0-0-0!\n");
	}
    if(strcmp((char*)argv[0],"local")==0)
    {
	    is_local = 1;
    }
	else if(strcmp((char*)argv[0],"remote")==0)
	{
	    is_local = 0;
    }
	else
	{
	    vty_out(vty, "error input of 1st parameter\n");
	    return CMD_IU_SUCCESS;
	}
	
	if(strcmp((char*)argv[1],"msc")==0)
	{
	    is_ps = 0;
	}
	else if(strcmp((char*)argv[1],"sgsn")==0)
	{
	    is_ps = 1;
	}
	else
	{
	    vty_out(vty, "error input of 2nd parameter\n");
	    return CMD_IU_SUCCESS;
	}

    if(strcmp(argv[3], "8-8-8") == 0)
		type = IU_24BIT_POINT_CODE;
	else if(strcmp(argv[3], "3-8-3") == 0)
		type = IU_14BIT_POINT_CODE;

    memcpy(point_code, argv[2], strnlen(argv[2],POINT_CODE_LEN));
	iu_parse_point_code(point_code, &self_pd, type);
	
	int localid = 1;
	int slot_id = HostSlotId;
	int index = 0;
	if(vty->node == IU_NODE){
		index = 0;
	}else if(vty->node == HANSI_IU_NODE){
		index = vty->index;
		localid = vty->local;
		slot_id = vty->slotindex;
	}else if(vty->node == LOCAL_HANSI_IU_NODE){
		index = vty->index;
		localid = vty->local;
		slot_id = vty->slotindex;
	}
	DBusConnection *dbus_connection = NULL;
	ReInitFemtoDbusConnection(&dbus_connection, slot_id, distributFag);
   	ret = iu_set_point_code(index, localid, self_pd, is_local, is_ps, dbus_connection, IU_SET_POINT_CODE);
    if(ret)
	{
		vty_out(vty,"set local ip address fail\n");
	}
	
	return CMD_IU_SUCCESS;
}


/*****************************************************
** DISCRIPTION:
**          set  point-code integer
** INPUT:
**          
** OUTPUT:
**          
** RETURN:
**          void
** DATE:	
**			2011-12-28
** AUTHOR:
**			<zhangshu@autelan.com>
*****************************************************/
DEFUN(set_iu_point_code_int_func,
	  set_iu_point_code_int_cmd,
	  "set (local|remote) (msc|sgsn) point-code <1-4294967295> format integer",
	  "set point code\n"
	  "local is HNB-GW, remote is CN\n"
	  "msc is IU-CS, sgsn is IU-PS\n"
	  "point value 1-4294967295"
	  "point value 1-4294967295"
	 )
{
	int ret = 0;
	unsigned char is_ps = 0;
	unsigned char is_local = 0;
	unsigned int self_pd = 0;
    if(strcmp((char*)argv[0],"local")==0)
    {
	    is_local = 1;
    }
	else if(strcmp((char*)argv[0],"remote")==0)
	{
	    is_local = 0;
    }
	else
	{
	    vty_out(vty, "error input of 1st parameter\n");
	    return CMD_IU_SUCCESS;
	}
	
	if(strcmp((char*)argv[1],"msc")==0)
	{
	    is_ps = 0;
	}
	else if(strcmp((char*)argv[1],"sgsn")==0)
	{
	    is_ps = 1;
	}
	else
	{
	    vty_out(vty, "error input of 2nd parameter\n");
	    return CMD_IU_SUCCESS;
	}
    self_pd = atoi(argv[2]);
	
	int localid = 1;
	int slot_id = HostSlotId;
	int index = 0;
	if(vty->node == IU_NODE){
		index = 0;
	}else if(vty->node == HANSI_IU_NODE){
		index = vty->index;
		localid = vty->local;
		slot_id = vty->slotindex;
	}else if(vty->node == LOCAL_HANSI_IU_NODE){
		index = vty->index;
		localid = vty->local;
		slot_id = vty->slotindex;
	}
	DBusConnection *dbus_connection = NULL;
	ReInitFemtoDbusConnection(&dbus_connection, slot_id, distributFag);
   	ret = iu_set_point_code(index, localid, self_pd, is_local, is_ps, dbus_connection, IU_SET_POINT_CODE);
    if(ret)
	{
		vty_out(vty,"set point code fail\n");
	}
	
	return CMD_IU_SUCCESS;
}



/*****************************************************
** DISCRIPTION:
**          set connection-mode
** INPUT:
**          
** OUTPUT:
**          
** RETURN:
**          void
** DATE:	
**			2011-12-28
** AUTHOR:
**			<zhangshu@autelan.com>
*****************************************************/
DEFUN(set_iu_connection_mode_func,
	  set_iu_connection_mode_cmd,
	  "set local (msc|sgsn) connection-mode (client|server)",
	  "set connection-mode of sctp\n"
	  "msc is IU-CS, sgsn is IU-PS\n"
	  "connection-mode client or server"
	  "client or server"
	 )
{
	int ret = 0;
	unsigned char is_ps = 0;
	unsigned char conn_mode = 0;
	if(strcmp((char*)argv[0],"msc")==0)
	{
	    is_ps = 0;
	}
	else if(strcmp((char*)argv[0],"sgsn")==0)
	{
	    is_ps = 1;
	}
	else
	{
	    vty_out(vty, "error input of 1st parameter\n");
	    return CMD_IU_SUCCESS;
	}

	if(strcmp((char*)argv[1],"client")==0)
	{
	    conn_mode = 0;
	}
	else if(strcmp((char*)argv[1],"server")==0)
	{
	    conn_mode = 1;
	}
	else
	{
	    vty_out(vty, "error input of 2nd parameter\n");
	    return CMD_IU_SUCCESS;
	}

	int localid = 1;
	int slot_id = HostSlotId;
	int index = 0;
	if(vty->node == IU_NODE){
		index = 0;
	}else if(vty->node == HANSI_IU_NODE){
		index = vty->index;
		localid = vty->local;
		slot_id = vty->slotindex;
	}else if(vty->node == LOCAL_HANSI_IU_NODE){
		index = vty->index;
		localid = vty->local;
		slot_id = vty->slotindex;
	}
	DBusConnection *dbus_connection = NULL;
	ReInitFemtoDbusConnection(&dbus_connection, slot_id, distributFag);
   	ret = iu_set_connection_mode(index, localid, conn_mode, is_ps, dbus_connection, IU_SET_CONNECTION_MODE);
    if(ret)
	{
		vty_out(vty,"set sctp connection mode failed\n");
	}
	
	return CMD_IU_SUCCESS;
}



/*****************************************************
** DISCRIPTION:
**          set  multi-homing switch
** INPUT:
**          
** OUTPUT:
**          
** RETURN:
**          void
** DATE:	
**			2011-12-28
** AUTHOR:
**			<zhangshu@autelan.com>
*****************************************************/
DEFUN(set_iu_multi_switch_func,
	  set_iu_multi_switch_cmd,
	  "set (local|remote) (msc|sgsn) multi-homing switch (open|closed)",
	  "set multi-homing switch\n"
	  "local is HNB-GW, remote is CN\n"
	  "msc is IU-CS, sgsn is IU-PS\n"
	  "set sctp multi-homing mode switch"
	  "set sctp multi-homing mode switch"
	  "open or closed"
	 )
{
	int ret = 0;
	unsigned char is_ps = 0;
	unsigned char is_local = 0;
	unsigned char multi_switch = 0;
    if(strcmp((char*)argv[0],"local")==0)
    {
	    is_local = 1;
    }
	else if(strcmp((char*)argv[0],"remote")==0)
	{
	    is_local = 0;
    }
	else
	{
	    vty_out(vty, "error input of 1st parameter\n");
	    return CMD_IU_SUCCESS;
	}
	
	if(strcmp((char*)argv[1],"msc")==0)
	{
	    is_ps = 0;
	}
	else if(strcmp((char*)argv[1],"sgsn")==0)
	{
	    is_ps = 1;
	}
	else
	{
	    vty_out(vty, "error input of 2nd parameter\n");
	    return CMD_IU_SUCCESS;
	}

    if(strcmp((char*)argv[2],"closed")==0)
	{
	    multi_switch = 0;
	}
	else if(strcmp((char*)argv[2],"open")==0)
	{
	    multi_switch = 1;
	}
	else
	{
	    vty_out(vty, "error input of 2nd parameter\n");
	    return CMD_IU_SUCCESS;
	}
	
	int localid = 1;
	int slot_id = HostSlotId;
	int index = 0;
	if(vty->node == IU_NODE){
		index = 0;
	}else if(vty->node == HANSI_IU_NODE){
		index = vty->index;
		localid = vty->local;
		slot_id = vty->slotindex;
	}else if(vty->node == LOCAL_HANSI_IU_NODE){
		index = vty->index;
		localid = vty->local;
		slot_id = vty->slotindex;
	}
	DBusConnection *dbus_connection = NULL;
	ReInitFemtoDbusConnection(&dbus_connection, slot_id, distributFag);
   	ret = iu_set_multi_switch(index, localid, multi_switch, is_local, is_ps, dbus_connection, IU_SET_MULTI_SWITCH);
    if(ret)
	{
		vty_out(vty,"set multi-homing switch failed\n");
	}
	
	return CMD_IU_SUCCESS;
}




/*****************************************************
** DISCRIPTION:
**          set  address
** INPUT:
**          
** OUTPUT:
**          
** RETURN:
**          void
** DATE:	
**			2011-12-28
** AUTHOR:
**			<zhangshu@autelan.com>
*****************************************************/
DEFUN(set_iu_address_func,
	  set_iu_address_cmd,
	  "set (local|remote) (msc|sgsn) (primary|secondary) address A.B.C.D port <1-65535>",
	  "set address\n"
	  "set address\n"
	  "set address\n"
	 )
{
	int ret = 0;
	unsigned my_ip = 0;
	unsigned short my_port = 0;
	unsigned char is_primary = 0;
	unsigned char is_ps = 0;
	unsigned char is_local;
    if(strcmp((char*)argv[0],"local")==0)
	{
	    is_local = 1;
	}
	else if(strcmp((char*)argv[0],"remote")==0)
	{
	    is_local = 0;
	}
	else
	{
	    vty_out(vty, "error input of 1st parameter\n");
	    return CMD_IU_SUCCESS;
	}
	
	if(strcmp((char*)argv[1],"msc")==0)
	{
	    is_ps = 0;
	}
	else if(strcmp((char*)argv[1],"sgsn")==0)
	{
	    is_ps = 1;
	}
	else
	{
	    vty_out(vty, "error input of 2nd parameter\n");
	    return CMD_IU_SUCCESS;
	}

	if(strcmp((char*)argv[2],"primary")==0)
	{
	    is_primary = 1;
	}
	else if(strcmp((char*)argv[2],"secondary")==0)
	{
	    is_primary = 0;
	}
	else
	{
	    vty_out(vty, "error input of 3rd parameter\n");
	    return CMD_IU_SUCCESS;
	}
	
	ret = inet_pton (AF_INET, (char*)argv[3], &my_ip);
	if (ret != 1) {
		vty_out (vty, "malformed ip address : %s\n", argv[3]);
		return CMD_IU_SUCCESS;
	 }
	
	my_port = atoi(argv[4]);
	
	int localid = 1;
	int slot_id = HostSlotId;
	int index = 0;
	if(vty->node == IU_NODE){
		index = 0;
	}else if(vty->node == HANSI_IU_NODE){
		index = vty->index;
		localid = vty->local;
		slot_id = vty->slotindex;
	}else if(vty->node == LOCAL_HANSI_IU_NODE){
		index = vty->index;
		localid = vty->local;
		slot_id = vty->slotindex;
	}
	DBusConnection *dbus_connection = NULL;
	ReInitFemtoDbusConnection(&dbus_connection, slot_id, distributFag);

   	ret = iu_set_address(index, localid, my_ip, my_port, is_local, is_primary, is_ps, dbus_connection, IU_SET_ADDRESS);
    if(ret)
	{
		vty_out(vty,"set local ip address fail\n");
	}	
	
	return CMD_IU_SUCCESS;
}



/*****************************************************
** DISCRIPTION:
**          set traffic mode
** INPUT:
**          
** OUTPUT:
**          
** RETURN:
**          void
** DATE:	
**			2011-11-08
** AUTHOR:
**			<zhangshu@autelan.com>
*****************************************************/

DEFUN(set_iu_traffic_mode_func,
	  set_iu_traffic_mode_cmd,
	  "set (msc|sgsn) traffic mode (over-ride|load-share|broadcast)",
	  "set traffic mode\n"
	  "set traffic mode\n"
	  "set traffic mode\n"
	 )
{
	int ret = 0;
	unsigned int trfmode = 0;
	int isps = 0;	
	if(strcmp((char*)argv[0],"msc")==0)
	{
	    isps = 0;
	}
	else if(strcmp((char*)argv[0],"sgsn")==0)
	{
	    isps = 1;
	}
	else
	{
	    vty_out(vty, "error input of 1st parameter\n");
	    return CMD_IU_FAILURE;
	}

	if(strcmp((char*)argv[1],"over-ride")==0)
	{
	    trfmode = 1;
	}
	else if(strcmp((char*)argv[1],"load-share")==0)
	{
	    trfmode = 2;
	}
	else if(strcmp((char*)argv[1],"broadcast")==0)
	{
	    trfmode = 3;
	}
	
	int localid = 1;
	int slot_id = HostSlotId;
	int index = 0;
	if(vty->node == IU_NODE){
		index = 0;
	}else if(vty->node == HANSI_IU_NODE){
		index = vty->index;
		localid = vty->local;
		slot_id = vty->slotindex;
	}else if(vty->node == LOCAL_HANSI_IU_NODE){
		index = vty->index;
		localid = vty->local;
		slot_id = vty->slotindex;
	}
	DBusConnection *dbus_connection = NULL;
	ReInitFemtoDbusConnection(&dbus_connection, slot_id, distributFag);

   	ret = iu_set_traffic_mode(index, localid, trfmode, isps, dbus_connection, IU_SET_TRAFFIC_MODE);
    if(ret) {
		vty_out(vty,"set traffic mode fail\n");
		return CMD_IU_FAILURE;
	}	
	else {
	}
	
	return CMD_IU_SUCCESS;
}



/*****************************************************
** DISCRIPTION:
**          set ni
** INPUT:
**          
** OUTPUT:
**          
** RETURN:
**          void
** DATE:	
**			2011-12-02
** AUTHOR:
**			<zhangshu@autelan.com>
*****************************************************/

DEFUN(set_iu_ni_func,
	  set_iu_ni_cmd,
	  "set network indicator <1-255>",
	  "set net indicator\n"
	  "set net indicator\n"
	  "1-255\n"
	 )
{
	int ret = 0;
	int ni = 0;
	ni = atoi(argv[0]);
	
	int localid = 1;
	int slot_id = HostSlotId;
	int index = 0;
	if(vty->node == IU_NODE){
		index = 0;
	}else if(vty->node == HANSI_IU_NODE){
		index = vty->index;
		localid = vty->local;
		slot_id = vty->slotindex;
	}else if(vty->node == LOCAL_HANSI_IU_NODE){
		index = vty->index;
		localid = vty->local;
		slot_id = vty->slotindex;
	}
	DBusConnection *dbus_connection = NULL;
	ReInitFemtoDbusConnection(&dbus_connection, slot_id, distributFag);
   	ret = iu_set_network_indicator(index, localid, ni, dbus_connection, IU_SET_NETWORK_INDICATOR);
    if(ret) {
		vty_out(vty,"set network indicator failed\n");
		return CMD_IU_FAILURE;
	}	
	else {
	}
	
	return CMD_IU_SUCCESS;
}



/*****************************************************
** DISCRIPTION:
**          set network apperance
** INPUT:
**          
** OUTPUT:
**          
** RETURN:
**          void
** DATE:	
**			2011-12-27
** AUTHOR:
**			<zhangshu@autelan.com>
*****************************************************/
DEFUN(set_iu_nwapp_func,
	  set_iu_nwapp_cmd,
	  "set (msc|sgsn) network apperance <0-65535>",
	  "set network apperance\n"
	  "set network apperance\n"
	  "0-65535\n"
	 )
{
	int ret = 0;
	int nwapp = 0;
	unsigned char isps = 0;
    if(strcmp((char*)argv[0],"msc")==0)
	{
	    isps = 0;
	}
	else if(strcmp((char*)argv[0],"sgsn")==0)
	{
	    isps = 1;
	}
	else
	{
	    vty_out(vty, "error input of 1st parameter\n");
	    return CMD_IU_FAILURE;
	}
	
	nwapp = atoi(argv[1]);
	
	int localid = 1;
	int slot_id = HostSlotId;
	int index = 0;
	if(vty->node == IU_NODE){
		index = 0;
	}else if(vty->node == HANSI_IU_NODE){
		index = vty->index;
		localid = vty->local;
		slot_id = vty->slotindex;
	}else if(vty->node == LOCAL_HANSI_IU_NODE){
		index = vty->index;
		localid = vty->local;
		slot_id = vty->slotindex;
	}
	DBusConnection *dbus_connection = NULL;
	ReInitFemtoDbusConnection(&dbus_connection, slot_id, distributFag);
   	ret = iu_set_network_apperance(index, localid, nwapp, isps, dbus_connection, IU_SET_NETWORK_APPERANCE);
    if(ret) {
		vty_out(vty,"set network apperance failed\n");
		return CMD_IU_FAILURE;
	}	
	else {
	}
	
	return CMD_IU_SUCCESS;
}



/*****************************************************
** DISCRIPTION:
**          set routing context
** INPUT:
**          
** OUTPUT:
**          
** RETURN:
**          void
** DATE:	
**			2011-11-08
** AUTHOR:
**			<zhangshu@autelan.com>
*****************************************************/
DEFUN(set_iu_routing_context_func,
	  set_iu_routing_context_cmd,
	  "set (msc|sgsn) routing context <0-65535>",
	  "set routing context\n"
	  "set routing context\n"
	  "set routing context\n"
	 )
{
	int ret = 0;
	unsigned int rtctx = 0;
	unsigned char isps = 0;
	if(strcmp((char*)argv[0],"msc")==0)
	{
	    isps = 0;
	}
	else if(strcmp((char*)argv[0],"sgsn")==0)
	{
	    isps = 1;
	}
	else
	{
	    vty_out(vty, "error input of 1st parameter\n");
	    return CMD_IU_FAILURE;
	}

	rtctx = atoi(argv[1]);

	int localid = 1;
	int slot_id = HostSlotId;
	int index = 0;
	if(vty->node == IU_NODE){
		index = 0;
	}else if(vty->node == HANSI_IU_NODE){
		index = vty->index;
		localid = vty->local;
		slot_id = vty->slotindex;
	}else if(vty->node == LOCAL_HANSI_IU_NODE){
		index = vty->index;
		localid = vty->local;
		slot_id = vty->slotindex;
	}
	DBusConnection *dbus_connection = NULL;
	ReInitFemtoDbusConnection(&dbus_connection, slot_id, distributFag);
	ret = iu_set_routing_context(index, localid, rtctx, isps, dbus_connection, IU_SET_ROUTING_CONTEXT);
    if(ret) {
		vty_out(vty,"set routing context fail\n");
		return CMD_IU_FAILURE;
	}	
	else {
	}
	
	return CMD_IU_SUCCESS;
}



DEFUN(iu_debug_enable_cmd_func,
	iu_debug_enable_cmd,
	"debug iu (all|info|error|debug)",
	"Add debug iu Information\n"
	"Dhcp server\n"	
	"Open iu debug level all\n"	
	"Open iu debug level info\n"
	"Open iu debug level error\n"
	"Open iu debug level debug\n"
)
{
	unsigned int ret = 0, debug_type = 0, debug_enable = 1;
	unsigned int maxParaLen = 5;

	if(strncmp("all",argv[0],strnlen(argv[0],maxParaLen))==0) {
		debug_type = DEBUG_TYPE_ALL;
	}
	else if (strncmp("info",argv[0],strnlen(argv[0],maxParaLen))==0) {
		debug_type = DEBUG_TYPE_INFO;
	}
	else if (strncmp("error",argv[0],strnlen(argv[0],maxParaLen))==0) {
		debug_type = DEBUG_TYPE_ERROR;
	}
	else if (strncmp("debug",argv[0],strnlen(argv[0],maxParaLen))==0) {
		debug_type = DEBUG_TYPE_DEBUG;
	}
	else {
		vty_out(vty,"bad command parameter %s\n", argv[0]);
		return CMD_IU_FAILURE;
	}
	int localid = 1;
	int slot_id = HostSlotId;
	int index = 0;
	if(vty->node == IU_NODE){
		index = 0;
	}else if(vty->node == HANSI_IU_NODE){
		index = vty->index;
		localid = vty->local;
		slot_id = vty->slotindex;
	}else if(vty->node == LOCAL_HANSI_IU_NODE){
		index = vty->index;
		localid = vty->local;
		slot_id = vty->slotindex;
	}
	DBusConnection *dbus_connection = NULL;
	ReInitFemtoDbusConnection(&dbus_connection, slot_id, distributFag);
	ret = iu_set_debug_state(index, localid, debug_type, debug_enable, dbus_connection, IU_DBUS_METHOD_SET_DEBUG_STATE);
	if (!ret) {
		return CMD_IU_SUCCESS;
	}
	else {
		return CMD_IU_FAILURE;
	}
}

DEFUN(iu_debug_disable_cmd_func,
	iu_debug_disable_cmd,
	"no debug iu (all|info|error|debug)",
	"Delete old Configuration\n"
	"Config iu debugging close\n"
	"Dhcp server\n"
	"Close iu debug level all\n"	
	"Close iu debug level info\n"
	"Close iu debug level error\n"
	"Close iu debug level debug\n"

)
{
	unsigned int ret = 0, debug_type = 0, isEnable = 0;
	unsigned int maxParaLen = 5;

    if(strncmp("all",argv[0],strnlen(argv[0],maxParaLen))==0) {
		debug_type = DEBUG_TYPE_ALL;
	}
	else if (strncmp("info",argv[0],strnlen(argv[0],maxParaLen))==0) {
		debug_type = DEBUG_TYPE_INFO;
	}
	else if (strncmp("error",argv[0],strnlen(argv[0],maxParaLen))==0) {
		debug_type = DEBUG_TYPE_ERROR;
	}
	else if (strncmp("debug",argv[0],strnlen(argv[0],maxParaLen))==0) {
		debug_type = DEBUG_TYPE_DEBUG;
	}
	else {
		vty_out(vty,"bad command parameter %s\n", argv[0]);
		return CMD_IU_FAILURE;
	}
	int localid = 1;
	int slot_id = HostSlotId;
	int index = 0;
	if(vty->node == IU_NODE){
		index = 0;
	}else if(vty->node == HANSI_IU_NODE){
		index = vty->index;
		localid = vty->local;
		slot_id = vty->slotindex;
	}else if(vty->node == LOCAL_HANSI_IU_NODE){
		index = vty->index;
		localid = vty->local;
		slot_id = vty->slotindex;
	}
	DBusConnection *dbus_connection = NULL;
	ReInitFemtoDbusConnection(&dbus_connection, slot_id, distributFag);
	ret = iu_set_debug_state(index, localid, debug_type, isEnable, dbus_connection, IU_DBUS_METHOD_SET_DEBUG_STATE);
	if (!ret) {
		return CMD_IU_SUCCESS;
	}
	else {
		return CMD_IU_FAILURE;
	}
}

DEFUN(iu_enable_cmd_func,
	iu_enable_cmd,
	"(msc|sgsn) service (enable|disable)",
	CONFIG_STR
	"Ip dhcp server (enable|disable) entity\n"
	"Specify pool name begins with char, and name length no more than 20 characters\n"
)
{
	unsigned int ret = 0, op_ret = 0, isEnable = 0;
	unsigned char isps = 0;
	if(strncmp((char*)argv[0],"msc",strnlen(argv[0],4))==0)
	{
	    isps = 0;
	}
	else if(strncmp((char*)argv[0],"sgsn",strnlen(argv[0],4))==0)
	{
	    isps = 1;
	}
	else
	{
	    vty_out(vty, "error input of 1st parameter\n");
	    return CMD_IU_FAILURE;
	}

	if(strncmp("enable",argv[1],strnlen(argv[1],7))==0) {
		isEnable = 1;
	}
	else if (strncmp("disable",argv[1],strnlen(argv[1],7))==0) {
		isEnable = 0;
	}
	else {
		vty_out(vty,"bad command parameter!\n");
		return CMD_IU_FAILURE;
	}	
	int localid = 1;
	int slot_id = HostSlotId;
	int index = 0;
	if(vty->node == IU_NODE){
		index = 0;
	}else if(vty->node == HANSI_IU_NODE){
		index = vty->index;
		localid = vty->local;
		slot_id = vty->slotindex;
	}else if(vty->node == LOCAL_HANSI_IU_NODE){
		index = vty->index;
		localid = vty->local;
		slot_id = vty->slotindex;
	}
	DBusConnection *dbus_connection = NULL;
	ReInitFemtoDbusConnection(&dbus_connection, slot_id, distributFag);
	ret = set_iu_enable(index, localid, isEnable, isps, dbus_connection, IU_DBUS_METHOD_SET_IU_ENABLE);
	if (ret == 0) {
		vty_out(vty,"set %s enable successful!\n",(char*)argv[0]);
	}
	else if(ret == -1) {
		vty_out(vty,"failed to get reply.\n");
	}
	else if(ret == 1) {
		vty_out(vty,"failed, parameters of %s are not correctly set yet!\n",(char*)argv[0]);
	}
	else if(ret == 2) {
		vty_out(vty,"failed, connection over time!\n");
	}

	return CMD_IU_SUCCESS;
}


DEFUN(iu2sig_enable_cmd_func,
	iu2sig_enable_cmd,
	"iu to sigtran (enable|disable)",
	CONFIG_STR
	"iu to sigtran (enable|disable) entity\n"
	"create m3ua connection to sigtran\n"
)
{
	unsigned int ret = 0, op_ret = 0, isEnable = 0;
	if(strncmp("enable",argv[0],strnlen(argv[0],7))==0) {
		isEnable = 1;
	}
	else if (strncmp("disable",argv[0],strnlen(argv[0],7))==0) {
		isEnable = 0;
	}
	else {
		vty_out(vty,"bad command parameter!\n");
		return CMD_IU_FAILURE;
	}	
	
	int localid = 1;
	int slot_id = HostSlotId;
	int index = 0;
	if(vty->node == IU_NODE){
		index = 0;
	}else if(vty->node == HANSI_IU_NODE){
		index = vty->index;
		localid = vty->local;
		slot_id = vty->slotindex;
	}else if(vty->node == LOCAL_HANSI_IU_NODE){
		index = vty->index;
		localid = vty->local;
		slot_id = vty->slotindex;
	}
	DBusConnection *dbus_connection = NULL;
	ReInitFemtoDbusConnection(&dbus_connection, slot_id, distributFag);
	ret = set_iu2sig_enable(index, localid, isEnable, dbus_connection, IU_DBUS_METHOD_SET_IU_TO_SIGTRAN_ENABLE);
	if (!ret) {
		return CMD_IU_SUCCESS;
	}
	else {
		return CMD_IU_FAILURE;
	}
}

/* book add, 2012-1-4 */
DEFUN(iu_get_link_status_func,
    iu_get_link_status_cmd,
    "show (msc|sgsn) link status",
    CONFIG_STR
    "show iu interface link status\n"
    "show iu interface link status\n"
    "show iu interface link status\n"
)
{
    unsigned char isps = 0;
    unsigned char m3_asp_state = 0;
    unsigned char m3_conn_state = 0;
    int sctp_state = 0;
    int ret = 0;
    if(strncmp("msc",argv[0],strnlen(argv[0],3))==0) {
        isps = 0;
    }
    else if (strncmp("sgsn",argv[0],strnlen(argv[0],4))==0) {
        isps = 1;
    }
    else {
        vty_out(vty,"bad command parameter!\n");
        return CMD_IU_FAILURE;
    } 
	
	int localid = 1;
	int slot_id = HostSlotId;
	int index = 0;
	if(vty->node == IU_NODE){
		index = 0;
	}else if(vty->node == HANSI_IU_NODE){
		index = vty->index;
		localid = vty->local;
		slot_id = vty->slotindex;
	}else if(vty->node == LOCAL_HANSI_IU_NODE){
		index = vty->index;
		localid = vty->local;
		slot_id = vty->slotindex;
	}
	DBusConnection *dbus_connection = NULL;
	ReInitFemtoDbusConnection(&dbus_connection, slot_id, distributFag);
    ret = get_iu_link_status(index, localid, isps, &m3_asp_state, &m3_conn_state, &sctp_state, dbus_connection, IU_GET_LINK_STATUS);
    if (!ret) {
        vty_out(vty, "IU-%s Interface Status:\n",isps?"PS":"CS");
        vty_out(vty, "-----------------------------\n");
        if(m3_asp_state < MAX_M3_ASP_STATE)
        {
            vty_out(vty, "M3UA ASP STATE       : %s\n", M3_ASP_STATE[m3_asp_state]);
        }
        else
        {
            vty_out(vty, "M3UA ASP STATE       : %s\n", M3_ASP_STATE[0]);
        }
        if(m3_conn_state < MAX_M3_CONN_STATE)
        {
            vty_out(vty, "M3UA CONNECTION STATE: %s\n", M3_CONN_STATE[m3_conn_state]);
        }
        else
        {
            vty_out(vty, "M3UA CONNECTION STATE: %s\n", M3_CONN_STATE[0]);
        }
        if(sctp_state < MAX_SCTP_CONN_STATE)
        {
            vty_out(vty, "SCTP CONNECTION STATE: %s\n", SCTP_CONN_STATE[sctp_state]);
        }
        else
        {
            vty_out(vty, "SCTP CONNECTION STATE: %s\n", SCTP_CONN_STATE[0]);
        }
        return CMD_IU_SUCCESS;
    }
    else {
        vty_out(vty, "Error: get iu link status failed.\n");
        return CMD_IU_FAILURE;
    }
}
#if 0
/*xdw add for show running*/
int iu_show_running_cfg_lib(int index)
{	
	char *tmp_str = NULL;
	DBusMessage *query = NULL, *reply = NULL;
	DBusError err;
    int ret = 1;

	int localid = 1;
	index = 0;
	char BUSNAME[PATH_LEN];
	char OBJPATH[PATH_LEN];
	char INTERFACE[PATH_LEN];
	ReInitFemtoDbusPath(localid,index,IU_DBUS_BUSNAME,BUSNAME);
	ReInitFemtoDbusPath(localid,index,IU_DBUS_OBJPATH,OBJPATH);
	ReInitFemtoDbusPath(localid,index,IU_DBUS_INTERFACE,INTERFACE);

	//query = dbus_message_new_method_call(BUSNAME,OBJPATH,INTERFACE,DBUS_METHOD);
	
	query = dbus_message_new_method_call(BUSNAME, 
									OBJPATH, 
									INTERFACE, 
									IU_DBUS_METHOD_SHOW_RUNNING_CFG);

	dbus_error_init(&err);

	reply = dbus_connection_send_with_reply_and_block (dcli_dbus_connection,query,-1, &err);

	dbus_message_unref(query);
	if (NULL == reply) {
		printf("failed get reply.\n");
		if (dbus_error_is_set(&err)) {
			printf("%s raised: %s",err.name,err.message);
			dbus_error_free(&err);
		}
		return -1;
	}
	
	if (dbus_message_get_args ( reply, &err,
					DBUS_TYPE_STRING, &tmp_str,
					DBUS_TYPE_INVALID)) {
		char _tmpstr[64];
		memset(_tmpstr, 0, 64);
		sprintf(_tmpstr, BUILDING_MOUDLE, "IU");
		vtysh_add_show_string(_tmpstr);
		vtysh_add_show_string(tmp_str);
		dbus_message_unref(reply);
        return 0;
	} 
	else {
		
		if (dbus_error_is_set(&err)) {
			dbus_error_free(&err);
		}
		dbus_message_unref(reply);
		return 1;
	}
	
	return 0;	
}
#endif
void dcli_iu_init(void) {

	install_node (&hansi_iu_node,NULL,"HANSI_IU_NODE");
	install_default(HANSI_IU_NODE);
	install_node (&local_hansi_iu_node,NULL,"LOCAL_HANSI_IU_NODE");
	install_default(LOCAL_HANSI_IU_NODE);
	install_element(HANSI_NODE,&config_iu_mode_cmd);
	install_element(LOCAL_HANSI_NODE,&config_iu_mode_cmd);
	/****************HANSI IU NODE**********************************/
	install_element(HANSI_IU_NODE,&set_iu_ni_cmd);
	install_element(HANSI_IU_NODE,&set_iu_nwapp_cmd);
	install_element(HANSI_IU_NODE,&iu_debug_enable_cmd);
	install_element(HANSI_IU_NODE,&iu_debug_disable_cmd);
	install_element(HANSI_IU_NODE, &iu_enable_cmd);
	install_element(HANSI_IU_NODE, &set_iu_traffic_mode_cmd);
	install_element(HANSI_IU_NODE, &set_iu_routing_context_cmd);
	install_element(HANSI_IU_NODE, &set_iu_address_cmd);
	install_element(HANSI_IU_NODE, &set_iu_point_code_str_cmd);
	install_element(HANSI_IU_NODE, &set_iu_point_code_int_cmd);
	install_element(HANSI_IU_NODE, &set_iu_connection_mode_cmd);
	install_element(HANSI_IU_NODE, &set_iu_multi_switch_cmd);
	install_element(HANSI_IU_NODE, &iu_get_link_status_cmd);

	/****************LOCAL HANSI IU NODE****************************/
	install_element(LOCAL_HANSI_IU_NODE,&set_iu_ni_cmd);
	install_element(LOCAL_HANSI_IU_NODE,&set_iu_nwapp_cmd);
	install_element(LOCAL_HANSI_IU_NODE,&iu_debug_enable_cmd);
	install_element(LOCAL_HANSI_IU_NODE,&iu_debug_disable_cmd);
	install_element(LOCAL_HANSI_IU_NODE, &iu_enable_cmd);
	install_element(LOCAL_HANSI_IU_NODE, &set_iu_traffic_mode_cmd);
	install_element(LOCAL_HANSI_IU_NODE, &set_iu_routing_context_cmd);
	install_element(LOCAL_HANSI_IU_NODE, &set_iu_address_cmd);
	install_element(LOCAL_HANSI_IU_NODE, &set_iu_point_code_str_cmd);
	install_element(LOCAL_HANSI_IU_NODE, &set_iu_point_code_int_cmd);
	install_element(LOCAL_HANSI_IU_NODE, &set_iu_connection_mode_cmd);
	install_element(LOCAL_HANSI_IU_NODE, &set_iu_multi_switch_cmd);
	install_element(LOCAL_HANSI_IU_NODE, &iu_get_link_status_cmd);
	
	return;
}

