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
* dcli_pppoe_snp.c
*
*
* CREATOR:
*		wangxc@autelan.com
*
* DESCRIPTION:
*		CLI definition for PPPoE snooping module.
*
* DATE:
*		2012-03-09
*
*  FILE REVISION NUMBER:
*  		$Revision: 1.6 $	
*******************************************************************************/
#ifdef __cplusplus
extern "C"
{
#endif
#include <zebra.h>
#include <dbus/dbus.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <net/if.h>
#include <net/if_arp.h>
#include <sys/ioctl.h>
#include <netinet/in.h>
#include <errno.h>
#include <dbus/npd/npd_dbus_def.h>
#include "dbus/dhcp/dhcp_dbus_def.h"
#include "dcli_main.h"
#include "vty.h"
#include "command.h"
#include "if.h"
#include "sysdef/returncode.h"
#include "dcli_dhcp.h"
#include "dcli_pppoe_snp.h"

extern int ve_ifname_transfer_conversion(char *new_ifname, const char *original_ifname);

/*
DBusConnection *dcli_dbus_connection = NULL;
dhcp_snp_reinitDbusConnection(&dcli_dbus_connection, vty);
*/
void dcli_pppoe_snp_reinitDbusConnection(DBusConnection **dcli_dbus_connection, struct vty *vty)
{
	int localid = 1, slotid = HostSlotId, indextmp = 0;
	dhcp_snp_get_slotid_index(vty, &indextmp, &slotid, &localid);

	ReInitDbusConnection(dcli_dbus_connection, slotid, distributFag);
}


/**********************************************************************************
 *  dcli_pppoe_get_board_slot_id
 *
 *	DESCRIPTION:
 * 		get board slot id
 *
 *	INPUT:
 *		NULL
 *	
 *	OUTPUT:
 *		NULL
 *
 * 	RETURN:
 *		-1 -> failed
 *		slot_id -> slot id
 *		
 **********************************************************************************/ 
int dcli_pppoe_get_board_slot_id(void)
{
	int fd = -1;
	int length = -1;
	int slot_id = -1;
	int ret = -1;
	char buf[8];

	memset(buf, 0, sizeof(buf));

	if ((fd = open(DISTRIBUTED_BOARD_SLOT_ID_FILE, O_RDONLY)) < 0) {
		return ret;
	}

	if ((length = read(fd, buf, sizeof(buf))) < 0) {
		close(fd);
		return ret;
	}

	if (buf[length-1] == '\n') {
		buf[length-1] = '\0';
		length--;
	}

	slot_id = atoi(buf);
	close(fd);
	
	return slot_id;
}


/**********************************************************************************
 *  dcli_pppoe_snp_convert_ifname
 * 
 * 	convert name
 *
 *	INPUT:
 *		ifname	- input interface name
 *		
 *	
 *	OUTPUT:
 *		name - ouput interface name
 *
 * 	RETURN:
 *		0  - 
 *		
 **********************************************************************************/
int dcli_pppoe_snp_convert_ifname(struct vty *vty, char *ifname)
{
	int slotid = 0, vrrpid = 0, num = 0, local_flag = 0;

	if ((!ifname) || (!vty)) {
		return -1;
	}

	if (!distributFag) {
		return 0;
	}
	
	if (CONFIG_NODE == vty->node) {
		if ((slotid = dcli_dhcp_get_board_slot_id()) < 0) {
			return -1;
		}
		vrrpid = 0;
		local_flag = 1;

	} else if ((HANSI_NODE == vty->node) || (LOCAL_HANSI_NODE == vty->node)) {
		slotid = vty->slotindex;
		vrrpid = (int)vty->index;
		local_flag = (LOCAL_HANSI_NODE == vty->node);
	}

	/* hansi node or config node */
	if (!local_flag) {
		if (0 == strncmp(ifname, "wlan", 4)) {
			sscanf(ifname, "wlan%d", &num);	
			memset(ifname, 0, strlen(ifname));
			sprintf(ifname, "wlan%d-%d-%d", slotid, vrrpid, num);
		} else if (0 == strncmp(ifname, "ebr", 3)) {
			sscanf(ifname, "ebr%d", &num);	
			memset(ifname, 0, strlen(ifname));	
			sprintf(ifname, "ebr%d-%d-%d", slotid, vrrpid, num);
		} else {
			sprintf(ifname, "%s", ifname);
		}

	/* local-hansi node */
	} else {
		if (!strncmp(ifname, "wlan", 4)) {
			sscanf(ifname, "wlan%d", &num);	
			memset(ifname, 0, strlen(ifname));
			sprintf(ifname, "wlanl%d-%d-%d", slotid, vrrpid, num);
		} else if (!strncmp(ifname, "ebr", 3)) {
			sscanf(ifname, "ebr%d", &num);			
			memset(ifname, 0, strlen(ifname));
			sprintf(ifname, "ebrl%d-%d-%d", slotid, vrrpid, num);
		} else {
			sprintf(ifname, "%s", ifname);
		}
	}
	return 0;
}


char *dcli_pppoe_snp_opcode_to_string(unsigned int opcode)
{
	switch (opcode) {
		case PPPoE_SNP_RETURN_CODE_SUCCESS:
			return "";
		case PPPoE_SNP_RETURN_CODE_FAIL:
			return "%% failed";
		case PPPoE_SNP_RETURN_CODE_NO_SUCH_INTERFACE:
			return "%% No such interface";
		case PPPoE_SNP_RETURN_CODE_INVALID_MRU:
			return "%% Invalid MRU, range 60 - 1492 bytes.";			
		default :
			return "%% failed";

	}
	return;
}


unsigned int dcli_direct_broadcast_enable(struct vty *vty, unsigned int enable_flag)
{
	DBusMessage *query = NULL, *reply = NULL;
	DBusError err;
	unsigned int op_ret = 0;

	if (!vty) {
		return CMD_WARNING;
	}

	DBusConnection *dcli_dbus_connection = NULL;
	dcli_pppoe_snp_reinitDbusConnection(&dcli_dbus_connection, vty);
	
	query = dbus_message_new_method_call(DHCP_DBUS_BUSNAME, 
									DHCP_DBUS_OBJPATH, 
									DHCP_DBUS_INTERFACE, 
									DHCP_DBUS_METHOD_DIRECT_BROADCAST_ENABLE);
	
	dbus_error_init(&err);
	dbus_message_append_args(query,
							 DBUS_TYPE_UINT32, &enable_flag,
							 DBUS_TYPE_INVALID);

	reply = dbus_connection_send_with_reply_and_block (dcli_dbus_connection,query,-1, &err);
				
	dbus_message_unref(query);
	
	if (NULL == reply) {
		if (dbus_error_is_set(&err)) {
			dbus_error_free_for_dcli(&err);
		}
		return CMD_SUCCESS;
	}

	if (dbus_message_get_args ( reply, &err,
					DBUS_TYPE_UINT32, &op_ret,
					DBUS_TYPE_INVALID)) {
		if(!op_ret) {
			dbus_message_unref(reply);
			return CMD_SUCCESS;
		}
	} 
	else {
		if (dbus_error_is_set(&err)) {
			dbus_error_free_for_dcli(&err);
		}
		dbus_message_unref(reply);
	
		return op_ret;
	}
}


DEFUN(config_direct_broadcast_cmd_func,
	config_direct_broadcast_cmd,
	"config direct-broadcast (enable|disable)",
	CONFIG_STR	
	"Direct Broadcast Agent\n"
	"Direct Broadcast Agent enable\n"
	"Direct Broadcast Agent disable\n"
)
{
	unsigned int ret = 0, isEnable = 0;

	if(strncmp("enable",argv[0],strlen(argv[0]))==0) {
		isEnable = 1;
	} else if (strncmp("disable",argv[0],strlen(argv[0]))==0) {
		isEnable = 0;
	} else {
		vty_out(vty,"%% bad command parameter!\n");
		return CMD_WARNING;
	}
	
	ret = dcli_direct_broadcast_enable(vty, isEnable);
	if (!ret) {	
		return CMD_SUCCESS;
	} else {
		vty_out(vty, "%s\n", dcli_pppoe_snp_opcode_to_string(ret));	
		return CMD_WARNING;
	}
}


unsigned int dcli_pppoe_snp_enable(struct vty *vty, unsigned int enable_flag)
{
	DBusMessage *query = NULL, *reply = NULL;
	DBusError err;
	unsigned int op_ret = 0;

	if (!vty) {
		return CMD_WARNING;
	}
	
	DBusConnection *dcli_dbus_connection = NULL;
	dcli_pppoe_snp_reinitDbusConnection(&dcli_dbus_connection, vty);
	query = dbus_message_new_method_call(DHCP_DBUS_BUSNAME, 
									DHCP_DBUS_OBJPATH, 
									DHCP_DBUS_INTERFACE, 
									DHCP_DBUS_METHOD_PPPOE_SNP_ENABLE);
	
	dbus_error_init(&err);
	dbus_message_append_args(query,
							 DBUS_TYPE_UINT32, &enable_flag,
							 DBUS_TYPE_INVALID);

	reply = dbus_connection_send_with_reply_and_block (dcli_dbus_connection,query,-1, &err);
				
	dbus_message_unref(query);
	
	if (NULL == reply) {
		if (dbus_error_is_set(&err)) {
			dbus_error_free_for_dcli(&err);
		}
		return CMD_SUCCESS;
	}

	if (dbus_message_get_args ( reply, &err,
					DBUS_TYPE_UINT32, &op_ret,
					DBUS_TYPE_INVALID)) {
		if(!op_ret) {
			dbus_message_unref(reply);
			return CMD_SUCCESS;
		}
	} 
	else {
		if (dbus_error_is_set(&err)) {
			dbus_error_free_for_dcli(&err);
		}
		dbus_message_unref(reply);
	
		return op_ret;
	}
}


DEFUN(config_pppoe_snp_server_cmd_func,
	config_pppoe_snp_server_cmd,
	"config pppoe-snooping (enable|disable)",
	CONFIG_STR	
	"PPPoE snooping\n"
	"Enable PPPoE snooping\n"
	"Disable PPPoE snooping\n"
)
{
	unsigned int ret = 0, isEnable = 0;

	if(!strncmp("enable",argv[0],strlen(argv[0]))) {
		isEnable = 1;
	} else if (!strncmp("disable",argv[0],strlen(argv[0]))) {
		isEnable = 0;
	} else {
		vty_out(vty,"%% bad command parameter!\n");
		return CMD_WARNING;
	}
	
	ret = dcli_pppoe_snp_enable(vty, isEnable);
	if (!ret) {		
		return CMD_SUCCESS;
	} else {
		vty_out(vty, "%s\n", dcli_pppoe_snp_opcode_to_string(ret));	
		return CMD_WARNING;
	}
}


unsigned int dcli_pppoe_snp_iface_enable
(
	struct vty *vty,
	char *ifname,
	unsigned int enable,
	unsigned short mru
)
{
	DBusMessage *query = NULL, *reply = NULL;
	DBusError err;
	unsigned int op_ret = 0;

	if (!vty) {
		return CMD_WARNING;
	}	

	DBusConnection *dcli_dbus_connection = NULL;
	dcli_pppoe_snp_reinitDbusConnection(&dcli_dbus_connection, vty);
	query = dbus_message_new_method_call(DHCP_DBUS_BUSNAME, 
									DHCP_DBUS_OBJPATH, 
									DHCP_DBUS_INTERFACE, 
									DHCP_DBUS_METHOD_PPPOE_SNP_IFACE_ENABLE);
	
	dbus_error_init(&err);
	dbus_message_append_args(query,
							DBUS_TYPE_STRING, &ifname,
							DBUS_TYPE_UINT32, &enable,
							DBUS_TYPE_UINT16, &mru,
							DBUS_TYPE_INVALID);

	reply = dbus_connection_send_with_reply_and_block (dcli_dbus_connection, query, -1, &err);
				
	dbus_message_unref(query);
	
	if (NULL == reply) {
		if (dbus_error_is_set(&err)) {
			dbus_error_free_for_dcli(&err);
		}
		return CMD_SUCCESS;
	}

	if (dbus_message_get_args (reply, &err,
					DBUS_TYPE_UINT32, &op_ret,
					DBUS_TYPE_INVALID)) {
		dbus_message_unref(reply);
		
		return op_ret;
	} 
	else {
		if (dbus_error_is_set(&err)) {
			dbus_error_free_for_dcli(&err);
		}
		dbus_message_unref(reply);
	
		return op_ret;
	}
}

DEFUN(config_pppoe_snooping_enable_cmd_func,
	config_pppoe_snooping_enable_cmd,
	"config pppoe-snooping IFNAME (enable|disable) [MRU]",
	CONFIG_STR	
	"PPPoE snooping\n"
	"Interface name\n"
	"Enable PPPoE snooping\n"
	"Disable PPPoE snooping\n"
	"Maximum Receive Unit(60 - 1492 bytes)\n"	
)
{
	int ret = 0;
	unsigned short mru = 0;
	unsigned int enable_flag = 0;
	char ifname[IFNAMESIZE];
	char tmp_ifname[IFNAMESIZE];
	char *ptr = NULL;

	if(!strncmp("enable", argv[1], strlen(argv[1]))) {
		enable_flag = 1;
	} else if (!strncmp("disable", argv[1], strlen(argv[1]))) {
		enable_flag = 0;
	} else {
		vty_out(vty,"%% bad command parameter!\n");
		return CMD_WARNING;
	}

	if(strlen(argv[0]) > IFNAMESIZE){
		vty_out (vty, "%% Interface name is too long\n");
		return CMD_WARNING;
	}

	memset(ifname, 0, sizeof(ifname));
	memcpy(ifname, argv[0], strlen(argv[0]));

	memset(tmp_ifname, 0, sizeof(tmp_ifname));
	ve_ifname_transfer_conversion(tmp_ifname, ifname);
	memcpy(ifname, tmp_ifname, strlen(tmp_ifname));

	dcli_pppoe_snp_convert_ifname(vty, ifname);

	if (argc == 3) {
		mru = (unsigned short)strtol((char*)argv[2], &ptr, 10); 
		if (*ptr) {
			vty_out (vty, "%% Invalid MRU %s\n", (char*)argv[2]);
			return CMD_WARNING;
		}

		if ((mru < MRU_MIN) || (mru > MRU_MAX)) {
			vty_out (vty, "%% MRU range: %d - %d\n", MRU_MIN, MRU_MAX);
			return CMD_WARNING;
		}
	}
	
	ret = dcli_pppoe_snp_iface_enable(vty, ifname, enable_flag, mru);
	if (!ret) {
		return CMD_SUCCESS;
	} else {	
		vty_out(vty, "%s\n", dcli_pppoe_snp_opcode_to_string(ret));	
		return CMD_WARNING;
	}
}



unsigned int
dcli_pppoe_set_debug
(
	struct vty *vty,
    unsigned int set_flag,
    unsigned int debug_type
)
{
	DBusMessage *query = NULL, *reply = NULL;
	DBusError err;
	unsigned int op_ret = 0;

	if (!vty) {
		return CMD_WARNING;
	}

	DBusConnection *dcli_dbus_connection = NULL;
	dcli_pppoe_snp_reinitDbusConnection(&dcli_dbus_connection, vty);
	
	query = dbus_message_new_method_call(DHCP_DBUS_BUSNAME, 
									DHCP_DBUS_OBJPATH, 
									DHCP_DBUS_INTERFACE, 
									DHCP_DBUS_METHOD_PPPOE_DEBUG);
	
	dbus_error_init(&err);
	dbus_message_append_args(query,	
							DBUS_TYPE_UINT32, &set_flag,
							DBUS_TYPE_UINT32, &debug_type,
							DBUS_TYPE_INVALID);

	reply = dbus_connection_send_with_reply_and_block (dcli_dbus_connection, query,-1, &err);
				
	dbus_message_unref(query);
	
	if (NULL == reply) {
		if (dbus_error_is_set(&err)) {
			dbus_error_free_for_dcli(&err);
		}
		return CMD_SUCCESS;
	}

	if (dbus_message_get_args(reply, &err,
					DBUS_TYPE_UINT32, &op_ret,
					DBUS_TYPE_INVALID)) {
		if(!op_ret) {
			dbus_message_unref(reply);
			return CMD_SUCCESS;
		}
	} 
	else {
		if (dbus_error_is_set(&err)) {
			dbus_error_free_for_dcli(&err);
		}
		dbus_message_unref(reply);
	
		return op_ret;
	}
}

DEFUN(pppoe_snp_debug_enable_cmd_func,
	pppoe_snp_debug_enable_cmd,
	"debug pppoe-snooping (all|info|error|debug)",
	"Add debug pppoe-snooping Information\n"
	"PPPoE snooping\n"	
	"Open pppoe-snooping debug level all\n"	
	"Open pppoe-snooping debug level info\n"
	"Open pppoe-snooping debug level error\n"
	"Open pppoe-snooping debug level debug\n"
)
{
	unsigned int ret = 0, debug_type = 0;

	if(!strncmp("all", argv[0], strlen(argv[0]))) {
		debug_type = PPPOE_SNP_LOG_ALL;
	} else if (!strncmp("info", argv[0], strlen(argv[0]))) {
		debug_type = PPPOE_SNP_LOG_INFO;
	}
	else if (!strncmp("error", argv[0], strlen(argv[0]))) {
		debug_type = PPPOE_SNP_LOG_ERROR;
	}
	else if (!strncmp("debug",argv[0],strlen(argv[0]))) {
		debug_type = PPPOE_SNP_LOG_DEBUG;
	} else {
		vty_out(vty,"%% bad command parameter %s\n", argv[0]);
		return CMD_WARNING;
	}
	
	ret = dcli_pppoe_set_debug(vty, SET_FALG, debug_type);
	
	if (!ret) {
		return CMD_SUCCESS;
	} else {
		vty_out(vty, "%s\n", dcli_pppoe_snp_opcode_to_string(ret));	
		return CMD_WARNING;
	}
}


DEFUN(pppoe_snp_debug_disable_cmd_func,
	pppoe_snp_debug_disable_cmd,
	"no debug pppoe-snooping (all|info|error|debug)",
	"Close debug pppoe-snooping Information\n"
	"Cancel system debugging\n"
	"PPPoE snooping\n"	
	"Close pppoe-snooping debug level all\n"	
	"Close pppoe-snooping debug level info\n"
	"Close pppoe-snooping debug level error\n"
	"Close pppoe-snooping debug level debug\n"
)
{
	unsigned int ret = 0, debug_type = 0;

	if(!strncmp("all", argv[0], strlen(argv[0]))) {
		debug_type = PPPOE_SNP_LOG_ALL;
	} else if (!strncmp("info", argv[0], strlen(argv[0]))) {
		debug_type = PPPOE_SNP_LOG_INFO;
	}
	else if (!strncmp("error", argv[0], strlen(argv[0]))) {
		debug_type = PPPOE_SNP_LOG_ERROR;
	}
	else if (!strncmp("debug",argv[0],strlen(argv[0]))) {
		debug_type = PPPOE_SNP_LOG_DEBUG;
	} else {
		vty_out(vty,"%% bad command parameter %s\n", argv[0]);
		return CMD_WARNING;
	}
	
	ret = dcli_pppoe_set_debug(vty, CLR_FALG, debug_type);
	if (!ret) {
		return CMD_SUCCESS;
	} else {
		vty_out(vty, "%s\n", dcli_pppoe_snp_opcode_to_string(ret));	
		return CMD_WARNING;
	}
}



void 
dcli_pppoe_snp_init
(
	void
)  
{
	/* DBA */
	install_element(CONFIG_NODE, &config_direct_broadcast_cmd);
	install_element(HANSI_NODE, &config_direct_broadcast_cmd);
	install_element(LOCAL_HANSI_NODE, &config_direct_broadcast_cmd);

	/* pppoe snooping */
	install_element(CONFIG_NODE, &config_pppoe_snp_server_cmd);
	install_element(HANSI_NODE, &config_pppoe_snp_server_cmd);	
	install_element(LOCAL_HANSI_NODE, &config_pppoe_snp_server_cmd);
	
	install_element(CONFIG_NODE, &config_pppoe_snooping_enable_cmd);
	install_element(HANSI_NODE, &config_pppoe_snooping_enable_cmd);
	install_element(LOCAL_HANSI_NODE, &config_pppoe_snooping_enable_cmd);
	
	install_element(CONFIG_NODE, &pppoe_snp_debug_enable_cmd);
	install_element(HANSI_NODE, &pppoe_snp_debug_enable_cmd);
	install_element(LOCAL_HANSI_NODE, &pppoe_snp_debug_enable_cmd);
	
	install_element(CONFIG_NODE, &pppoe_snp_debug_disable_cmd);
	install_element(HANSI_NODE, &pppoe_snp_debug_disable_cmd);
	install_element(LOCAL_HANSI_NODE, &pppoe_snp_debug_disable_cmd);	
}


#ifdef __cplusplus
}
#endif
