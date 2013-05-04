/******************************************************************************
Copyright (C) Autelan Technology

This software file is owned and distributed by Autelan Technology 
*******************************************************************************

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
*******************************************************************************
* dcli_wsm.c
*
*
* DESCRIPTION:
*  WSM module Dbus implement.
*
* DATE:
*  2008-10-13
*
* CREATOR:
*  guoxb@autelan.com
*
*
* CHANGE LOG:
*  2009-10-13 <guoxb> Create file.
*  2009-11-04 <guoxb> Add wsm watchdog dbus switch.
*  2009-11-12 <guoxb> Add display wtp ipv6 address
*  2009-11-25 <guoxb> Add function dbus_msg_new_method_call for Hansi.
*  2009-12-10 <guoxb> Add enable pmtu dbus command.
*  2010-02-09 <guoxb> Modified files name, copyright etc.
*  2010-02-09 <guoxb> Modified "show flow-based-forwarding state", because 
*                               module ipfwd changed to ip-fast-forwarding.ko.
*  2010-03-23 <guoxb> Add command set ipfrag_ignoredf (enable|disable).
*  2010-03-25 <guoxb> Modify command set ipfrag_ignoredf
*
******************************************************************************/

#ifdef _D_WCPSS_

#include <string.h>
#include <sys/socket.h>
#include <zebra.h>
#include <dbus/dbus.h>
#include <dbus/wcpss/wsm_dbus_def.h>
#include <sys/wait.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include "command.h"
#include "vtysh/vtysh.h"
#include "memory.h"

#include "../dcli_main.h"
#include "dcli_wsm.h"
#include "hmd/hmdpub.h"
#include "dbus/hmd/HmdDbusDef.h"

DBusMessage *dbus_msg_new_method_call(int local,unsigned char *dbus_name, unsigned char *obj_name,
			unsigned char *if_name, unsigned char *cmd_name, int idx);
			

struct cmd_node cwtunnel_node =
{
	CWTUNNEL_NODE,
	"%s(config-capwap-tunnel)# "
};

struct cmd_node hansi_cwtunnel_node =
{
	HANSI_CWTUNNEL_NODE,
	"%s(hansi-capwap-tunnel %d-%d)# ",
	1
};
struct cmd_node local_hansi_cwtunnel_node =
{
	LOCAL_HANSI_CWTUNNEL_NODE,
	"%s(local-hansi-capwap-tunnel %d-%d)# ",
	1
};


DEFUN(config_tunnel_cmd_func,
	  config_tunnel_cmd,
	  "config capwap-tunnel",
	  CONFIG_STR
	  "CAPWAP tunnel information\n"
	  "config CAPWAP tunnel\n"
	 )
{
	if (vty->node == CONFIG_NODE)
		vty->node = CWTUNNEL_NODE;
	else if (vty->node == HANSI_NODE)
		vty->node = HANSI_CWTUNNEL_NODE;
	else if (vty->node == LOCAL_HANSI_NODE)
		vty->node = LOCAL_HANSI_CWTUNNEL_NODE;
	else
		return CMD_FAILURE;
		
	return CMD_SUCCESS;
}

DEFUN(show_tunnel_flow_based_forwarding_state_func,
	  show_tunnel_flow_based_forwarding_state_cmd,
	  "show flow-based-forwarding state",
	  CONFIG_STR
	  "Display ipfwd state\n"
	  "config CAPWAP tunnel\n"
	 )
{
	int rval = 0, stat = 0;

	rval = system("test -d /sys/module/ip_fast_forwarding");
	stat = WEXITSTATUS(rval);

	if (stat)
		vty_out(vty, "flow-based-forwarding is Disable\n");
	else
		vty_out(vty, "flow-based-forwarding is Enable\n");

	
	return CMD_SUCCESS;
}

DEFUN(show_tunnel_log_wsm_state_func,
	  show_tunnel_log_wsm_state_cmd,
	  "show log wsm state",
	  CONFIG_STR
	  "Display wsm log level\n"
	  "config CAPWAP tunnel\n"
)
{
	DBusMessage *query, *reply;
	DBusError err;
	DBusMessageIter iter, iter_array;
	int ret = 0;
	int local = 1;
	char emerg_stat = 0, alert_stat = 0, crit_stat = 0, err_stat = 0;
	char warn_stat = 0, notice_stat = 0, info_stat = 0, dbg_stat = 0;
	int idx = -1;
	int slot_id = HostSlotId;
	if (vty->node == CWTUNNEL_NODE)
		idx = 0;
	else if(vty->node == HANSI_CWTUNNEL_NODE){
		idx = (int)vty->index;
		local = vty->local;
		slot_id = vty->slotindex;
	}
	else if(vty->node == LOCAL_HANSI_CWTUNNEL_NODE){
		idx = vty->index;
		local = vty->local;
		slot_id = vty->slotindex;
	}
	DBusConnection *dcli_dbus_connection = NULL;
	ReInitDbusConnection(&dcli_dbus_connection,slot_id,distributFag);
	query = dbus_msg_new_method_call(local,WSM_DBUS_BUSNAME,WSM_DBUS_OBJPATH,
			WSM_DBUS_INTERFACE,WSM_DBUS_CONF_SHOW_LOG_WSM_STATE, idx);
	dbus_error_init(&err);
	reply = dbus_connection_send_with_reply_and_block (dcli_dbus_connection, query, -1, &err);	
	dbus_message_unref(query);
	
	if (reply == NULL) 
	{
		cli_syslog_info("<error> failed get reply.\n");
		if (dbus_error_is_set(&err))
		{
			cli_syslog_info("%s raised: %s", err.name, err.message);
			dbus_error_free_for_dcli(&err);
		}
		
		return CMD_SUCCESS;
	}

	dbus_message_iter_init(reply, &iter);
	dbus_message_iter_get_basic(&iter, &ret);
	
	if (ret != 0)
	{
		vty_out(vty, "Get WSM log state failed\n");
		goto out;
	}
		
	dbus_message_iter_next(&iter);	
	dbus_message_iter_get_basic(&iter, &emerg_stat);
	dbus_message_iter_next(&iter);
	dbus_message_iter_get_basic(&iter, &alert_stat);
	dbus_message_iter_next(&iter);
	dbus_message_iter_get_basic(&iter, &crit_stat);
	dbus_message_iter_next(&iter);
	dbus_message_iter_get_basic(&iter, &err_stat);
	dbus_message_iter_next(&iter);
	dbus_message_iter_get_basic(&iter, &warn_stat);
	dbus_message_iter_next(&iter);
	dbus_message_iter_get_basic(&iter, &notice_stat);
	dbus_message_iter_next(&iter);
	dbus_message_iter_get_basic(&iter, &info_stat);
	dbus_message_iter_next(&iter);
	dbus_message_iter_get_basic(&iter, &dbg_stat);
	dbus_message_iter_next(&iter);

	if (emerg_stat)
		vty_out (vty, "%-13s: ON\n", "EMERG Level");
	else
		vty_out(vty, "%-13s: OFF\n", "EMERG Level");
	
	if (alert_stat)
		vty_out (vty, "%-13s: ON\n", "ALERT Level");
	else
		vty_out(vty, "%-13s: OFF\n", "ALERT Level");

	if (crit_stat)
		vty_out (vty, "%-13s: ON\n", "CRIT Level");
	else
		vty_out(vty, "%-13s: OFF\n", "CRIT Level");

	if (err_stat)
		vty_out (vty, "%-13s: ON\n", "ERR Level");
	else
		vty_out(vty, "%-13s: OFF\n", "ERR Level");

	if (warn_stat)
		vty_out (vty, "%-13s: ON\n", "WARN Level");
	else
		vty_out(vty, "%-13s: OFF\n", "WARN Level");

	if (notice_stat)
		vty_out (vty, "%-13s: ON\n", "NOTICE Level");
	else
		vty_out(vty, "%-13s: OFF\n", "NOTICE Level");

	if (info_stat)
		vty_out (vty, "%-13s: ON\n", "INFO Level");
	else
		vty_out(vty, "%-13s: OFF\n", "INFO Level");

	if (dbg_stat)
		vty_out (vty, "%-13s: ON\n", "DEBUG Level");
	else
		vty_out(vty, "%-13s: OFF\n", "DEBUG Level");
		
out:	
	dbus_message_unref(reply);
	return CMD_SUCCESS;
}

DEFUN(set_tunnel_log_wsm_level_func,
	  set_tunnel_log_wsm_level_cmd,
	  "set log wsm level (emerg | alert | crit | err | warn | notice | info | debug)",
	  CONFIG_STR
	  "Set wsm log level\n"
	  "Set wsm log level\n"
)
{
	DBusMessage *query, *reply;	
	DBusMessageIter	 iter;
	DBusError err;
	int ret = 0, level = 0;
	int idx = -1;
	int local = 1;
	int slot_id = HostSlotId;
	if (!strcmp(argv[0], "emerg"))
	{
		level = 0;
	}
	else if (!strcmp(argv[0], "alert"))
	{
		level = 1;
	}
	else if (!strcmp(argv[0], "crit"))
	{
		level = 2;
	}
	else if (!strcmp(argv[0], "err"))
	{
		level = 3;
	}
	else if (!strcmp(argv[0], "warn"))
	{
		level = 4;
	}
	else if (!strcmp(argv[0], "notice"))
	{
		level = 5;
	}
	else if (!strcmp(argv[0], "info"))
	{
		level = 6;
	}
	else if (!strcmp(argv[0], "debug"))
	{
		level = 7;
	}
	else
	{
		vty_out(vty, "Log level error.\n");
		return CMD_SUCCESS;
	}

	if (vty->node == CWTUNNEL_NODE)
		idx = 0;
	else if(vty->node == HANSI_CWTUNNEL_NODE){
		idx = (int)vty->index;
		local = vty->local;
		slot_id = vty->slotindex;
	}
	else if(vty->node == LOCAL_HANSI_CWTUNNEL_NODE){
		idx = vty->index;
		local = vty->local;
		slot_id = vty->slotindex;
	}
	DBusConnection *dcli_dbus_connection = NULL;
	ReInitDbusConnection(&dcli_dbus_connection,slot_id,distributFag);
		
	query = dbus_msg_new_method_call(local,WSM_DBUS_BUSNAME,WSM_DBUS_OBJPATH,
						WSM_DBUS_INTERFACE,WSM_DBUS_CONF_SET_LOG_WSM_LEVEL, idx);
	dbus_error_init(&err);
	dbus_message_append_args(query,
							 DBUS_TYPE_UINT32, &level,
							 DBUS_TYPE_INVALID);
	reply = dbus_connection_send_with_reply_and_block (dcli_dbus_connection,query,-1, &err);
	dbus_message_unref(query);
	
	if (NULL == reply)
	{
		cli_syslog_info("<error> failed get reply.\n");
		if (dbus_error_is_set(&err))
		{
			cli_syslog_info("%s raised: %s",err.name,err.message);
			dbus_error_free_for_dcli(&err);
		}
		
		return CMD_SUCCESS;
	}
		
	dbus_message_iter_init(reply,&iter);
	dbus_message_iter_get_basic(&iter,&ret);
	
	if(ret == 0)
	{
		vty_out(vty,"WSM Log level set successed\n");
	}				
	else
	{
		vty_out(vty,"WSM Log level set failed\n");
	}
			
	dbus_message_unref(reply);
	
	return CMD_SUCCESS;
}


DEFUN(set_tunnel_flow_based_forwarding_func,
	set_tunnel_flow_based_forwarding_cmd,
	"set flow-based-forwarding (enable|disable)",
	"Enable or Disable ipfwd"
	"wireless-control config\n"
	"Enable or disable ipfwd module\n"
	"Enable ipfwd module\n"
	"Disable ipfwd module\n"
	)
{
	DBusMessage *query, *reply;	
	DBusMessageIter	 iter;
	DBusError err;
	int ret;
	int policy = 0;
	int idx = 0;
	int local = 1;
	int slot_id = HostSlotId;
	if (!strcmp(argv[0],"enable"))
	{
		policy = 1;	
	}
	else if (!strcmp(argv[0],"disable"))
	{
		policy = 0;	
	}
	else
	{
		vty_out(vty,"<error> input patameter only with 'enable' or 'disable'\n");
		return CMD_SUCCESS;
	}
	int i = 0;
	DBusConnection *dcli_dbus_connection = NULL;
	for(i = 1; i < MAX_SLOT_NUM; i++){
		if((NULL != dbus_connection_dcli[i])&&(NULL != dbus_connection_dcli[i]->dcli_dbus_connection)){
			dcli_dbus_connection = dbus_connection_dcli[i]->dcli_dbus_connection;
		}else
			continue;
		query = dbus_message_new_method_call(HMD_DBUS_BUSNAME,HMD_DBUS_OBJPATH,HMD_DBUS_INTERFACE, HMD_DBUS_CONF_METHOD_SET_FAST_FORWARDING);
		dbus_error_init(&err);

		dbus_message_append_args(query,
								 DBUS_TYPE_UINT32,&policy,
								 DBUS_TYPE_INVALID);

		reply = dbus_connection_send_with_reply_and_block (dcli_dbus_connection,query,-1, &err);
		
		dbus_message_unref(query);
		
		if (NULL == reply)
		{
			cli_syslog_info("<error> failed get reply.\n");
			if (dbus_error_is_set(&err))
			{
				cli_syslog_info("%s raised: %s",err.name,err.message);
				dbus_error_free_for_dcli(&err);
			}
			
			return CMD_SUCCESS;
		}
		
		dbus_message_iter_init(reply,&iter);
		dbus_message_iter_get_basic(&iter,&ret);

		if(ret == 0)
		{
			vty_out(vty,"slot %d set ipfwd %s successfully\n",i,argv[0]);
		}				
		else
		{
			vty_out(vty,"<error>  %d\n",ret);
		}
			
		dbus_message_unref(reply);
	}
	return CMD_SUCCESS;			
}
#if 0
DEFUN(set_tunnel_qos_map_func,
	set_tunnel_qos_map_cmd,
	"set (wifi_qos|fast_forwarding_qos) (enable|disable)",
	"Enable or Disable qos_map"
	"wireless-control config\n"
	"Enable or Disable wifi_qos\n"
	"Enable or disable fast_forwarding_qos\n"
	"Enable qos_map\n"
	"Disable qos_map\n"
	)
{
	int module = 0;
	int policy = 0;
	int rval = 0, stat = 0, stat1 = 0;
	if (!strcmp(argv[0],"wifi_qos"))
	{
		module = 0;	
	}
	else if (!strcmp(argv[0],"fast_forwarding_qos"))
	{
		module = 1;	
	}
	else
	{
		vty_out(vty,"<error> input patameter only with 'wifi_qos' or 'fast_forwarding_qos'\n");
		return CMD_SUCCESS;
	}
	
	if (!strcmp(argv[1],"enable"))
	{
		policy = 1;	
	}
	else if (!strcmp(argv[1],"disable"))
	{
		policy = 0;	
	}
	else
	{
		vty_out(vty,"<error> input patameter only with 'enable' or 'disable'\n");
		return CMD_SUCCESS;
	}
	if(module == 0){
		rval = system("test -d /sys/module/wifi_ethernet");
		stat = WEXITSTATUS(rval);

		if (stat){
			vty_out(vty, "wifi.ko load fail.\n");
			return CMD_SUCCESS;
		}
		if (policy){
			system("sudo echo 1 > /sys/module/wifi_ethernet/parameters/wifi_QoS_open");
		}
		else{
			system("sudo echo 0 > /sys/module/wifi_ethernet/parameters/wifi_QoS_open");
		}

	}else if(module == 1){

		rval = system("test -d /sys/module/ip_fast_forwarding");
		stat1 = WEXITSTATUS(rval);

		if (stat1){
			vty_out(vty, "please set flow-based-forwarding enable first.\n");
			return CMD_SUCCESS;
		}
		if (policy){
			system("sudo echo 1 > /sys/module/ip_fast_forwarding/parameters/qos_enable");
		}
		else{
			system("sudo echo 0 > /sys/module/ip_fast_forwarding/parameters/qos_enable");
		}

	}

	return CMD_SUCCESS;			
}
#endif
DEFUN(set_tunnel_qos_map_func,
	set_tunnel_qos_map_cmd,
	"set (wifi_qos|fast_forwarding_qos | wifi_ipv6) (enable|disable)",
	"Enable or Disable qos_map"
	"wireless-control config\n"
	"Enable or Disable wifi_qos\n"
	"Enable or disable fast_forwarding_qos\n"
	"Enable qos_map\n"
	"Disable qos_map\n"
	)
{
	DBusMessage *query, *reply;	
	DBusMessageIter	 iter;
	DBusError err;
	int ret;
	int type = 0;
	int policy = 0;
	int idx = 0;
	int local = 1;
	int slot_id = HostSlotId;
	int rval = 0, stat = 0, stat1 = 0;
	if (!strcmp(argv[0],"wifi_qos"))
	{
		type = 0;	
	}
	else if (!strcmp(argv[0],"fast_forwarding_qos"))
	{
		type = 1;	
	}
	else if (!strcmp(argv[0], "wifi_ipv6"))
	{
		type = 2;
	}
	else
	{
		vty_out(vty,"<error> input patameter only with 'enable' or 'disable'\n");
		return CMD_SUCCESS;
	}

	if (!strcmp(argv[1],"enable"))
	{
		policy = 1;	
	}
	else if (!strcmp(argv[1],"disable"))
	{
		policy = 0;	
	}
	else
	{
		vty_out(vty,"<error> input patameter only with 'enable' or 'disable'\n");
		return CMD_SUCCESS;
	}

	if(type == 0){
		rval = system("test -d /sys/module/wifi_ethernet");
		stat = WEXITSTATUS(rval);

		if (stat){
			vty_out(vty, "wifi.ko load fail.\n");
			return CMD_SUCCESS;
		}
	}else if(type == 1){
		rval = system("test -d /sys/module/ip_fast_forwarding");
		stat1 = WEXITSTATUS(rval);

		if (stat1){
			vty_out(vty, "please set flow-based-forwarding enable first.\n");
			return CMD_SUCCESS;
		}
	}
	else if (type == 2)
	{
		if (policy)
			policy = 0;
		else 
			policy = 1;
		rval = system("test -d /sys/module/wifi_ethernet");
		stat1 = WEXITSTATUS(rval);
		if (stat1)
		{
			vty_out(vty, "wifi.ko load fail.\n");
			return CMD_SUCCESS;
		}
	}
	else
	{
		vty_out(vty, "some other unknow error happend: type = %d \n", type);
		return CMD_FAILURE;
	}

	
	int i = 0;
	DBusConnection *dcli_dbus_connection = NULL;
	for(i = 1; i < MAX_SLOT_NUM; i++){
		if((NULL != dbus_connection_dcli[i])&&(NULL != dbus_connection_dcli[i]->dcli_dbus_connection)){
			dcli_dbus_connection = dbus_connection_dcli[i]->dcli_dbus_connection;
		}else
			continue;
		query = dbus_message_new_method_call(HMD_DBUS_BUSNAME,HMD_DBUS_OBJPATH,HMD_DBUS_INTERFACE, HMD_DBUS_CONF_METHOD_SET_TUNNEL_QOS_MAP);
		dbus_error_init(&err);

		dbus_message_append_args(query,
								 DBUS_TYPE_UINT32,&type,
								 DBUS_TYPE_UINT32,&policy,
								 DBUS_TYPE_INVALID);

		reply = dbus_connection_send_with_reply_and_block (dcli_dbus_connection,query,-1, &err);
		
		dbus_message_unref(query);
		
		if (NULL == reply)
		{
			cli_syslog_info("<error> failed get reply.\n");
			if (dbus_error_is_set(&err))
			{
				cli_syslog_info("%s raised: %s",err.name,err.message);
				dbus_error_free_for_dcli(&err);
			}
			
			return CMD_SUCCESS;
		}
		
		dbus_message_iter_init(reply,&iter);
		dbus_message_iter_get_basic(&iter,&ret);

		if(ret == 0)
		{
			vty_out(vty," set %s %s successfully\n",argv[0],argv[1]);
		}				
		else
		{
			vty_out(vty,"<error>  %d\n",ret);
		}
			
		dbus_message_unref(reply);
	}
	return CMD_SUCCESS;			
}


DEFUN(show_tunnel_wtp_list_func,
		show_tunnel_wtp_list_cmd,
		"show wtp list",
		"Display tables information in CAPWAP tunnel.\n"
		"List wtp summary\n"
		)
{
	DBusMessage *query, *reply;
	DBusError err;
	DBusMessageIter iter, iter_array;
	DBusMessageIter iter_struct, iter_sub_struct;
	unsigned int total_wtp_cnt = 0, total_sta_cnt = 0, i = 0, j = 0, loop = 0;
	int ret = 0;
	unsigned char wtpmac[WSM_MAC_LEN];
	unsigned int wtpid = 0, sta_cnt = 0;
	unsigned char bss_cnt = 0;
	unsigned char ip_str[21] = {0};
	unsigned char mac_str[18] = {0};
	unsigned char wtpip[8] = {0};
	unsigned short addr_family = 0;
	int idx = -1;
	int local = 1;
	int slot_id = HostSlotId;
	if (vty->node == CWTUNNEL_NODE)
		idx = 0;
	else if(vty->node == HANSI_CWTUNNEL_NODE){
		idx = (int)vty->index;
		local = vty->local;
		slot_id = vty->slotindex;
	}
	else if(vty->node == LOCAL_HANSI_CWTUNNEL_NODE){
		idx = vty->index;
		local = vty->local;
		slot_id = vty->slotindex;
	}
	DBusConnection *dcli_dbus_connection = NULL;
	ReInitDbusConnection(&dcli_dbus_connection,slot_id,distributFag);
		
	query = dbus_msg_new_method_call(local,WSM_DBUS_BUSNAME, WSM_DBUS_OBJPATH,
			WSM_DBUS_INTERFACE, WSM_DBUS_CONF_SHOW_WTP_LIST, idx);
	dbus_error_init(&err);
	reply = dbus_connection_send_with_reply_and_block (dcli_dbus_connection, query, -1, &err);	
	dbus_message_unref(query);
	
	if (reply == NULL) 
	{
		cli_syslog_info("<error> failed get reply.\n");
		if (dbus_error_is_set(&err))
		{
			cli_syslog_info("%s raised: %s", err.name, err.message);
			dbus_error_free_for_dcli(&err);
		}
		
		return CMD_SUCCESS;
	}

	dbus_message_iter_init(reply, &iter);
	dbus_message_iter_get_basic(&iter, &ret);
	
	if (ret != 0)
	{
		vty_out(vty, "Total Assoc WTP Number is : %d\n", total_wtp_cnt);
		goto out;
	}
		
	dbus_message_iter_next(&iter);	
	dbus_message_iter_get_basic(&iter, &total_wtp_cnt);
	dbus_message_iter_next(&iter);
	dbus_message_iter_get_basic(&iter, &total_sta_cnt);
	dbus_message_iter_next(&iter);
	dbus_message_iter_recurse(&iter, &iter_array);
	
	vty_out(vty, "Total Assoc WTP Number is : %d\n", total_wtp_cnt);
	vty_out(vty, "Total Assoc STA Number is : %d\n", total_sta_cnt);
	vty_out(vty, "================================================================================\n");
	vty_out(vty, "WTPID    WTPIP                   WTPMAC               Assoc BSS    Assoc STA\n");
	vty_out(vty, "--------------------------------------------------------------------------------\n");	

	for (i = 0; i < total_wtp_cnt; i++)
	{
		dbus_message_iter_recurse(&iter_array, &iter_struct);	
		dbus_message_iter_get_basic(&iter_struct, &wtpid);
		dbus_message_iter_next(&iter_struct);
		dbus_message_iter_get_basic(&iter_struct, &addr_family);
		dbus_message_iter_next(&iter_struct);
		for (loop = 0; loop < 8; loop++)
		{
			dbus_message_iter_get_basic(&iter_struct, &wtpip[loop]);
			dbus_message_iter_next(&iter_struct);
		}
		dbus_message_iter_get_basic(&iter_struct, &sta_cnt);
		dbus_message_iter_next(&iter_struct);
		dbus_message_iter_get_basic(&iter_struct, &bss_cnt);
		dbus_message_iter_next(&iter_struct);

		for (j = 0; j < WSM_MAC_LEN - 1; j++)
		{
			dbus_message_iter_get_basic(&iter_struct, &wtpmac[j]);
			dbus_message_iter_next(&iter_struct);
		}
		dbus_message_iter_get_basic(&iter_struct, &wtpmac[WSM_MAC_LEN - 1]);
		dbus_message_iter_next(&iter_array);

		bzero(mac_str, 18);
		bzero(ip_str, 21);
	
		sprintf(mac_str, "%02x:%02x:%02x:%02x:%02x:%02x", wtpmac[0], wtpmac[1], 
				wtpmac[2], wtpmac[3], wtpmac[4], wtpmac[5]);

		if (addr_family == AF_INET)
			sprintf(ip_str, "%d.%d.%d.%d", wtpip[0], wtpip[1], wtpip[2], wtpip[3]);
		else
			sprintf(ip_str, "%02x%02x:%02x%02x..%02x%02x:%02x%02x", wtpip[0], wtpip[1], 
					wtpip[2], wtpip[3], wtpip[4], wtpip[5], wtpip[6], wtpip[7]);
		
		vty_out(vty, "%-9d%-24s%-21s%-13u%u\n", wtpid, ip_str, mac_str, bss_cnt, sta_cnt);
	}
	vty_out(vty, "================================================================================\n");

out:
	dbus_message_unref(reply);
	
	return CMD_SUCCESS;
}


DEFUN(show_tunnel_wtp_wtpid_func,
		show_tunnel_wtp_wtpid_cmd,
		"show wtp WTPID",
		"Display tables information in CAPWAP tunnel.\n"
		"Show WTP WTPID\n"
		"Show WTP WTPID\n"
		)
{
	DBusMessage *query, *reply;	
	DBusMessageIter	 iter;
	DBusMessageIter	 iter_bss_array, iter_bss_struct;
	DBusError err;
	unsigned int  wtpid = 0;
	char *endptr = NULL;
	const unsigned char *p = argv[0];
	int ret = 0;
	unsigned int wtp_sta_cnt = 0, bss_sta_cnt = 0;
	unsigned char bss_cnt = 0;
	unsigned char mac[WSM_MAC_LEN] = {0};
	unsigned char mac_str[18] = {0};
	int i = 0, j = 0, loop = 0;
	unsigned char wtpip[16] = {0};
	unsigned short addr_family = 0;
	unsigned char s_addr[5] = {0};
	int idx = -1;
	int local = 1;
	int slot_id = HostSlotId;
	if (p[0] < '0' && p[0] > '9')
	{
		vty_out(vty, "Input WTPID Error\n");
		return CMD_SUCCESS;
	}
	
	wtpid = strtoul(p, &endptr, 10);
	if (wtpid > 4096 || wtpid <= 0)
	{
		vty_out(vty, "The range of WTPID is 1--4096\n");
		return CMD_SUCCESS;
	}

	if (vty->node == CWTUNNEL_NODE)
		idx = 0;
	else if(vty->node == HANSI_CWTUNNEL_NODE){
		idx = (int)vty->index;
		local = vty->local;
		slot_id = vty->slotindex;
	}
	else if(vty->node == LOCAL_HANSI_CWTUNNEL_NODE){
		idx = vty->index;
		local = vty->local;
		slot_id = vty->slotindex;
	}
	DBusConnection *dcli_dbus_connection = NULL;
	ReInitDbusConnection(&dcli_dbus_connection,slot_id,distributFag);

	query = dbus_msg_new_method_call(local,WSM_DBUS_BUSNAME, WSM_DBUS_OBJPATH,
			WSM_DBUS_INTERFACE, WSM_DBUS_CONF_SHOW_WTP_WTPID, idx);
	dbus_error_init(&err);
	dbus_message_append_args(query,
							 DBUS_TYPE_UINT32, &wtpid,
							 DBUS_TYPE_INVALID);
	reply = dbus_connection_send_with_reply_and_block (dcli_dbus_connection, query, -1, &err);	
	dbus_message_unref(query);
	
	if (reply == NULL) 
	{
		cli_syslog_info("<error> failed get reply.\n");
		if (dbus_error_is_set(&err))
		{
			cli_syslog_info("%s raised: %s", err.name, err.message);
			dbus_error_free_for_dcli(&err);
		}
		
		return CMD_SUCCESS;
	}

	dbus_message_iter_init(reply, &iter);
	dbus_message_iter_get_basic(&iter, &ret);	
	
	if (ret != 0)
	{
		vty_out(vty, "WTP <WTPID = %d> is not run.\n", wtpid);
		dbus_message_unref(reply);
		return CMD_SUCCESS;
	}
	
	dbus_message_iter_next(&iter);	
	dbus_message_iter_get_basic(&iter, &bss_cnt);
	dbus_message_iter_next(&iter);
	dbus_message_iter_get_basic(&iter, &wtp_sta_cnt);
	dbus_message_iter_next(&iter);
	dbus_message_iter_get_basic(&iter, &addr_family);
	dbus_message_iter_next(&iter);
	for (loop = 0; loop < 16; loop++)
	{
		dbus_message_iter_get_basic(&iter, &wtpip[loop]);
		dbus_message_iter_next(&iter);
	}
	
	for (i = 0; i < WSM_MAC_LEN; i++)
	{
		dbus_message_iter_get_basic(&iter, &mac[i]);
		dbus_message_iter_next(&iter);	
	}
	
	bzero(mac_str, 18);
	bzero(s_addr, 5);
	
	if (addr_family == AF_INET)
		sprintf(s_addr, "IPv4");
	else
		sprintf(s_addr, "IPv6");
		
	sprintf(mac_str, "%02x:%02x:%02x:%02x:%02x:%02x", mac[0], mac[1], mac[2], mac[3], mac[4], mac[5]);
	vty_out(vty, "================================================================================\n");
	vty_out(vty, "WTPID       Addr Family         WTPMAC                Assoc BSS     Assoc STA\n");
	vty_out(vty, "--------------------------------------------------------------------------------\n");	
	vty_out(vty, "%-12d%-20s%-22s%-14u%u\n\n", wtpid, s_addr, mac_str, bss_cnt, wtp_sta_cnt);
	if (addr_family == AF_INET)
		vty_out(vty, "WTPIP : %d.%d.%d.%d\n", wtpip[0], wtpip[1], wtpip[2], wtpip[3]);
	else
		vty_out(vty, "WTPIP : %02x%02x:%02x%02x:%02x%02x:%02x%02x:%02x%02x:%02x%02x:%02x%02x:%02x%02x\n",
			wtpip[0], wtpip[1], wtpip[2], wtpip[3], wtpip[4], wtpip[5], wtpip[6], wtpip[7], wtpip[8], wtpip[9], 
			wtpip[10], wtpip[11], wtpip[12], wtpip[13], wtpip[14], wtpip[15]);
	vty_out(vty, "--------------------------------------------------------------------------------\n");
	vty_out(vty, "\nAssoc BSS Information:\n");
	vty_out(vty, "--------------------------------------------------------------------------------\n");
	if (!bss_cnt)
	{
		dbus_message_unref(reply);
		return CMD_SUCCESS;
	}
	
	dbus_message_iter_recurse(&iter, &iter_bss_array);
	
	for (i = 0; i < bss_cnt; i++)
	{
		bzero(mac, WSM_MAC_LEN);
		dbus_message_iter_recurse(&iter_bss_array, &iter_bss_struct);
		dbus_message_iter_get_basic(&iter_bss_struct, &bss_sta_cnt);
		dbus_message_iter_next(&iter_bss_struct);
		bzero(mac, WSM_MAC_LEN);
		for (j = 0; j < WSM_MAC_LEN; j++)
		{
			dbus_message_iter_get_basic(&iter_bss_struct, &mac[j]);
			if (j != WSM_MAC_LEN - 1)
				dbus_message_iter_next(&iter_bss_struct);
		}
		dbus_message_iter_next(&iter_bss_array);
		bzero(mac_str, 18);
		sprintf(mac_str, "%02x:%02x:%02x:%02x:%02x:%02x", mac[0], mac[1], mac[2], mac[3], mac[4], mac[5]);
		vty_out(vty, "BSSID : [%s]        Assoc STA : [%d]\n", mac_str, bss_sta_cnt);
	}
	
	vty_out(vty, "================================================================================\n");

	dbus_message_unref(reply);
	return CMD_SUCCESS;
	
}

DEFUN(show_tunnel_bss_bssid_func,
		show_tunnel_bss_bssid_cmd,
		"show bss BSSID",
		"Display tables information in CAPWAP tunnel.\n"
		"Show bss BSSID\n"
		"Show bss BSSID\n"
		)
{
	DBusMessage *query, *reply;	
	DBusMessageIter	 iter;
	DBusMessageIter	 iter_sta_array, iter_sta_struct;
	DBusError err;
	unsigned char mac[WSM_MAC_LEN] = {0};
	unsigned char stamac[WSM_MAC_LEN] = {0};
	int ret = 0;
	unsigned int sta_cnt = 0, bss_idx = 0, rid_g = 0;
	unsigned char wlanid = 0;
	unsigned int wtpid = 0;
	unsigned char mac_str[18] = {0};
	int rval = 0, i = 0, j = 0;
	int idx = -1;
	int local = 1;
	int slot_id = HostSlotId;
	rval = sscanf((char *)argv[0], "%hhx:%hhx:%hhx:%hhx:%hhx:%hhx", 
			&mac[0], &mac[1], &mac[2], &mac[3], &mac[4], &mac[5]);
	
	if (rval != WSM_MAC_LEN)
	{
		vty_out(vty, "Input BSSID error. BSSID Format: aa:bb:cc:dd:ee:ff\n");
		return CMD_SUCCESS;
	}

	if (vty->node == CWTUNNEL_NODE)
		idx = 0;
	else if(vty->node == HANSI_CWTUNNEL_NODE){
		idx = (int)vty->index;
		local= vty->local;
		slot_id = vty->slotindex;
	}
	else if(vty->node == LOCAL_HANSI_CWTUNNEL_NODE){
		idx = vty->index;
		local = vty->local;
		slot_id = vty->slotindex;
	}
	DBusConnection *dcli_dbus_connection = NULL;
	ReInitDbusConnection(&dcli_dbus_connection,slot_id,distributFag);
		
	query = dbus_msg_new_method_call(local,WSM_DBUS_BUSNAME, WSM_DBUS_OBJPATH,
							WSM_DBUS_INTERFACE, WSM_DBUS_CONF_SHOW_BSS_BSSID, idx);
	dbus_error_init(&err);
	dbus_message_append_args(query,
								 DBUS_TYPE_BYTE, &mac[0],
								 DBUS_TYPE_BYTE, &mac[1],
								 DBUS_TYPE_BYTE, &mac[2],
								 DBUS_TYPE_BYTE, &mac[3],
								 DBUS_TYPE_BYTE, &mac[4],
								 DBUS_TYPE_BYTE, &mac[5],
								 DBUS_TYPE_INVALID);
	reply = dbus_connection_send_with_reply_and_block (dcli_dbus_connection,query,-1, &err);
	dbus_message_unref(query);
	if (NULL == reply)
	{
		cli_syslog_info("<error> failed get reply.\n");
		if (dbus_error_is_set(&err))
		{
			cli_syslog_info("%s raised: %s",err.name,err.message);
			dbus_error_free_for_dcli(&err);
		}
		return CMD_SUCCESS;
	}
	
	dbus_message_iter_init(reply,&iter);
	dbus_message_iter_get_basic(&iter,&ret);
	if (ret)
	{
		vty_out(vty, "Cannot find this BSSID in wsm tables\n");
		dbus_message_unref(reply);
		return CMD_SUCCESS;
	}

	dbus_message_iter_next(&iter);	
	dbus_message_iter_get_basic(&iter, &bss_idx);
	dbus_message_iter_next(&iter);
	dbus_message_iter_get_basic(&iter, &rid_g);
	dbus_message_iter_next(&iter);
	dbus_message_iter_get_basic(&iter, &wtpid);
	dbus_message_iter_next(&iter);
	dbus_message_iter_get_basic(&iter, &wlanid);
	dbus_message_iter_next(&iter);
	dbus_message_iter_get_basic(&iter, &sta_cnt);
	dbus_message_iter_next(&iter);

	bzero(mac_str, 18);
	sprintf(mac_str, "%02x:%02x:%02x:%02x:%02x:%02x", mac[0], mac[1], mac[2], mac[3], mac[4], mac[5]);
	vty_out(vty, "================================================================================\n");
	vty_out(vty, "BSSID                WTPID    WlanID   BSSIndex      Radio_G_ID       Assoc STA\n");
	vty_out(vty, "--------------------------------------------------------------------------------\n");	
	vty_out(vty, "%-21s%-9d%-9d%-14d%-17d%d\n", mac_str, wtpid, wlanid, bss_idx, rid_g, sta_cnt);
	vty_out(vty, "--------------------------------------------------------------------------------\n");
	
	if (!sta_cnt)
	{
		dbus_message_unref(reply);
		return CMD_SUCCESS;
	}
	
	dbus_message_iter_recurse(&iter, &iter_sta_array);
	
	vty_out(vty, "\nAssoc STA Information:\n");
	vty_out(vty, "--------------------------------------------------------------------------------\n");
	
	for (i = 0; i < sta_cnt; i++)
	{
		dbus_message_iter_recurse(&iter_sta_array, &iter_sta_struct);
		bzero(stamac, WSM_MAC_LEN);
		for (j = 0; j < WSM_MAC_LEN; j++)
		{
			dbus_message_iter_get_basic(&iter_sta_struct, &stamac[j]);
			if (j != WSM_MAC_LEN - 1)
				dbus_message_iter_next(&iter_sta_struct);
		}
		dbus_message_iter_next(&iter_sta_array);
		bzero(mac_str, 18);
		sprintf(mac_str, "%02x:%02x:%02x:%02x:%02x:%02x", stamac[0], stamac[1], stamac[2], stamac[3], stamac[4], stamac[5]);
		vty_out(vty, "STAMAC : [%s]\n", mac_str);
	}
	vty_out(vty, "================================================================================\n");
	dbus_message_unref(reply);
	return CMD_SUCCESS;

}


DEFUN(show_tunnel_sta_stamac_func,
		show_tunnel_sta_stamac_cmd,
		"show sta STAMAC",
		"Display tables information in CAPWAP tunnel.\n"
		"Show sta STAMAC\n"
		"Show sta STAMAC\n"
		)
{
	DBusMessage *query, *reply;	
	DBusMessageIter iter;
	DBusError err;
	unsigned char mac[WSM_MAC_LEN] = {0};
	unsigned char bssid[WSM_MAC_LEN] = {0};
	unsigned char mac_str[18] = {0};
	unsigned char bssid_str[18] = {0};
	unsigned char wlanid = 0;
	unsigned int wtpid = 0;
	int ret = 0, rval = 0, i = 0;
	int idx = -1;
	int local = 1;
	int slot_id = HostSlotId;
	rval = sscanf((char *)argv[0], "%hhx:%hhx:%hhx:%hhx:%hhx:%hhx", 
			&mac[0], &mac[1], &mac[2], &mac[3], &mac[4], &mac[5]);
	
	if (rval != WSM_MAC_LEN)
	{
		vty_out(vty, "Input STAMAC error. STAMAC Format: aa:bb:cc:dd:ee:ff\n");
		return CMD_SUCCESS;
	}

	if (vty->node == CWTUNNEL_NODE)
		idx = 0;
	else if(vty->node == HANSI_CWTUNNEL_NODE){
		idx = (int)vty->index;
		local = vty->local;
		slot_id = vty->slotindex;
	}
	else if(vty->node == LOCAL_HANSI_CWTUNNEL_NODE){
		idx = vty->index;
		local = vty->local;
		slot_id = vty->slotindex;
	}
	DBusConnection *dcli_dbus_connection = NULL;
	ReInitDbusConnection(&dcli_dbus_connection,slot_id,distributFag);
		
	query = dbus_msg_new_method_call(local,WSM_DBUS_BUSNAME, WSM_DBUS_OBJPATH,
							WSM_DBUS_INTERFACE, WSM_DBUS_CONF_SHOW_STA_STAMAC, idx);
	dbus_error_init(&err);
	dbus_message_append_args(query,
								 DBUS_TYPE_BYTE, &mac[0],
								 DBUS_TYPE_BYTE, &mac[1],
								 DBUS_TYPE_BYTE, &mac[2],
								 DBUS_TYPE_BYTE, &mac[3],
								 DBUS_TYPE_BYTE, &mac[4],
								 DBUS_TYPE_BYTE, &mac[5],
								 DBUS_TYPE_INVALID);
								 
	reply = dbus_connection_send_with_reply_and_block (dcli_dbus_connection,query,-1, &err);
	dbus_message_unref(query);
	if (NULL == reply)
	{
		cli_syslog_info("<error> failed get reply.\n");
		if (dbus_error_is_set(&err))
		{
			cli_syslog_info("%s raised: %s",err.name,err.message);
			dbus_error_free_for_dcli(&err);
		}
		return CMD_SUCCESS;
	}
	
	dbus_message_iter_init(reply, &iter);
	dbus_message_iter_get_basic(&iter, &ret);
	if (ret)
	{
		vty_out(vty, "Cannot find this STA in wsm tables\n");
		dbus_message_unref(reply);
		return CMD_SUCCESS;
	}

	for (i = 0; i < WSM_MAC_LEN; i++)
	{
		dbus_message_iter_next(&iter);
		dbus_message_iter_get_basic(&iter, &bssid[i]);	
	}
	
	dbus_message_iter_next(&iter);
	dbus_message_iter_get_basic(&iter, &wlanid);
	dbus_message_iter_next(&iter);
	dbus_message_iter_get_basic(&iter, &wtpid);
	dbus_message_iter_next(&iter);

	bzero(mac_str, 18);
	bzero(bssid_str, 18);
	sprintf(mac_str, "%02x:%02x:%02x:%02x:%02x:%02x", mac[0], mac[1], mac[2], mac[3], mac[4], mac[5]);
	sprintf(bssid_str, "%02x:%02x:%02x:%02x:%02x:%02x", bssid[0], bssid[1], bssid[2], bssid[3], bssid[4], bssid[5]);

	vty_out(vty, "================================================================================\n");
	vty_out(vty, "STAMAC                     BSSID                  WlanID     WTPID\n");
	vty_out(vty, "--------------------------------------------------------------------------------\n");
	vty_out(vty, "%-27s%-23s%-11d%d\n", mac_str, bssid_str, wlanid, wtpid);
	vty_out(vty, "================================================================================\n");
	
	dbus_message_unref(reply);
	
	return CMD_SUCCESS;
}

DEFUN(set_tunnel_wsm_watchdog_func,
	set_tunnel_wsm_watchdog_cmd,
	"set wsm watchdog (enable|disable)",
	"Enable or Disable watch dog"
	"capwap-tunnel config\n"
	"Enable or disable wsm watch dog\n"
	"Enable wsm watch dog\n"
	"Disable wsm watch dog\n"
	)
{
	DBusMessage *query, *reply;	
	DBusMessageIter	 iter;
	DBusError err;
	int ret;
	unsigned char policy = 0;
	int idx = -1;
	int local = 1;
	int slot_id = HostSlotId;
	if (!strcmp(argv[0],"enable"))
	{
		policy = 1;	
	}
	else if (!strcmp(argv[0],"disable"))
	{
		policy = 0;	
	}
	else
	{
		vty_out(vty,"<error> input patameter only with 'enable' or 'disable'\n");
		return CMD_SUCCESS;
	}
	
	if (vty->node == CWTUNNEL_NODE)
		idx = 0;
	else if(vty->node == HANSI_CWTUNNEL_NODE){
		idx = (int)vty->index;
		local = vty->local;
		slot_id = vty->slotindex;
	}
	else if(vty->node == LOCAL_HANSI_CWTUNNEL_NODE){
		idx = vty->index;
		local = vty->local;
		slot_id = vty->slotindex;
	}
	DBusConnection *dcli_dbus_connection = NULL;
	ReInitDbusConnection(&dcli_dbus_connection,slot_id,distributFag);

	query = dbus_msg_new_method_call(local,WSM_DBUS_BUSNAME, WSM_DBUS_OBJPATH,
						WSM_DBUS_INTERFACE, WSM_DBUS_CONF_SET_WSM_WATCHDOG_STATE, idx);
	dbus_error_init(&err);

	dbus_message_append_args(query,
							 DBUS_TYPE_BYTE, &policy,
							 DBUS_TYPE_INVALID);

	reply = dbus_connection_send_with_reply_and_block (dcli_dbus_connection, query, -1, &err);
	
	dbus_message_unref(query);
	
	if (NULL == reply)
	{
		cli_syslog_info("<error> failed get reply.\n");
		if (dbus_error_is_set(&err))
		{
			cli_syslog_info("%s raised: %s",err.name,err.message);
			dbus_error_free_for_dcli(&err);
		}
		
		return CMD_SUCCESS;
	}
	
	dbus_message_iter_init(reply,&iter);
	dbus_message_iter_get_basic(&iter,&ret);

	if(ret == 0)
	{
		vty_out(vty," set WSM watch dog %s successfully\n",argv[0]);
	}				
	else
	{
		vty_out(vty,"<error>  %d\n",ret);
	}
		
	dbus_message_unref(reply);

	return CMD_SUCCESS;			
}


DEFUN(show_tunnel_wsm_watchdog_state_func,
	  show_tunnel_wsm_watchdog_state_cmd,
	  "show wsm watchdog state",
	  "Display wsm watchdog state\n"
	  "capwap-tunnel config\n"
	  "Display wsm watchdog state\n"
	  "config CAPWAP tunnel\n"
)
{
	DBusMessage *query, *reply;
	DBusError err;
	DBusMessageIter iter, iter_array;
	int ret = 0;
	unsigned char tunnel_stat = 0, wd_stat = 0;
	int idx = -1;
	int local = 1;
	int slot_id = HostSlotId;
	if (vty->node == CWTUNNEL_NODE)
		idx = 0;
	else if(vty->node == HANSI_CWTUNNEL_NODE){
		idx = (int)vty->index;
		local = vty->local;
		slot_id = vty->slotindex;
	}
	else if(vty->node == LOCAL_HANSI_CWTUNNEL_NODE){
		idx = vty->index;
		local = vty->local;
		slot_id = vty->slotindex;
	}
	DBusConnection *dcli_dbus_connection = NULL;
	ReInitDbusConnection(&dcli_dbus_connection,slot_id,distributFag);

	query = dbus_msg_new_method_call(local,WSM_DBUS_BUSNAME, WSM_DBUS_OBJPATH,
			WSM_DBUS_INTERFACE, WSM_DBUS_CONF_SHOW_WSM_WATCH_DOG_STATE, idx);
	dbus_error_init(&err);
	reply = dbus_connection_send_with_reply_and_block (dcli_dbus_connection, query, -1, &err);	
	dbus_message_unref(query);
	
	if (reply == NULL) 
	{
		cli_syslog_info("<error> failed get reply.\n");
		if (dbus_error_is_set(&err))
		{
			cli_syslog_info("%s raised: %s", err.name, err.message);
			dbus_error_free_for_dcli(&err);
		}
		
		return CMD_SUCCESS;
	}

	dbus_message_iter_init(reply, &iter);
	dbus_message_iter_get_basic(&iter, &ret);
	
	if (ret != 0)
	{
		vty_out(vty, "Get WSM watchdog state failed\n");
		goto out;
	}
		
	dbus_message_iter_next(&iter);	
	dbus_message_iter_get_basic(&iter, &tunnel_stat);
	dbus_message_iter_next(&iter);
	dbus_message_iter_get_basic(&iter, &wd_stat);
	dbus_message_iter_next(&iter);
	
	if (tunnel_stat)
		vty_out (vty, "%-20s: Enable\n", "Tunnel state");
	else
		vty_out(vty, "%-20s: Disable\n", "Tunnel state");

	if (wd_stat)
		vty_out (vty, "%-20s: Enable\n", "Watchdog flag state");
	else
		vty_out(vty, "%-20s: Disable\n", "Watchdog flag state");	

	vty_out(vty, "------------------------------\n");
	if (wd_stat && tunnel_stat)
		vty_out (vty, "%-20s: Enable\n", "Watchdog state");
	else
		vty_out (vty, "%-20s: Disable\n", "Watchdog state");
		
out:	
	dbus_message_unref(reply);
	return CMD_SUCCESS;
}

#if 0
DEFUN(set_ipfrag_ingress_pmtu_state_func,
	  set_ipfrag_ingress_pmtu_state_func_cmd,
	  "set ipfrag_ingress_pmtu (enable|disable)",
	  CONFIG_STR
	  "Enable or disable ipfrag_ingress_pmtu\n"
	  "config CAPWAP tunnel\n"
	 )
{
	int policy = 0;
	unsigned char buff[128] = {0};
	
	if (!strcmp(argv[0],"enable"))
	{
		policy = 1;	
	}
	else if (!strcmp(argv[0],"disable"))
	{
		policy = 0;	
	}
	else
	{
		vty_out(vty, "Parameter error.\n");
		return CMD_SUCCESS;
	}
	
	sprintf(buff, "sudo sysctl -w net.ipv4.ipfrag_ingress_pmtu=%d", policy);
	system(buff);

	return CMD_SUCCESS;
}

DEFUN(set_ipfrag_inform_nhmtu_instead_state_func,
	  set_ipfrag_inform_nhmtu_instead_state_func_cmd,
	  "set ipfrag_inform_nhmtu_instead (enable|disable)",
	  CONFIG_STR
	  "Enable or disable ipfrag_inform_nhmtu_instead\n"
	  "config CAPWAP tunnel\n"
	 )
{
	int policy = 0;
	unsigned char buff[128] = {0};
	
	if (!strcmp(argv[0],"enable"))
	{
		policy = 1;	
	}
	else if (!strcmp(argv[0],"disable"))
	{
		policy = 0;	
	}
	else
	{
		vty_out(vty, "Parameter error.\n");
		return CMD_SUCCESS;
	}

	sprintf(buff, "sudo sysctl -w net.ipv4.ipfrag_inform_nhmtu_instead=%d", policy);
	system(buff);

	return CMD_SUCCESS;
}

DEFUN(set_ipfrag_ignoredf_state_func,
	  set_ipfrag_ignoredf_state_func_cmd,
	  "set ipfrag_ignoredf (enable|disable)",
	  CONFIG_STR
	  "Enable or disable ipfrag_ignoredf\n"
	  "config CAPWAP tunnel\n"
	 )
{
	int policy = 0;
	unsigned char buff[128] = {0};
	
	if (!strcmp(argv[0],"enable"))
	{
		policy = 0;	
	}
	else if (!strcmp(argv[0],"disable"))
	{
		policy = 1;	
	}
	else
	{
		vty_out(vty, "Parameter error.\n");
		return CMD_SUCCESS;
	}

	sprintf(buff, "sudo sysctl -w net.ipv4.ipfrag_ignoredf=%d", policy);
	system(buff);

	return CMD_SUCCESS;

}
#endif
DEFUN(set_ipfrag_ingress_pmtu_state_func,
	  set_ipfrag_ingress_pmtu_state_func_cmd,
	  "set ipfrag_ingress_pmtu (enable|disable)",
	  CONFIG_STR
	  "Enable or disable ipfrag_ingress_pmtu\n"
	  "config CAPWAP tunnel\n"
	 )
{
	DBusMessage *query, *reply;	
	DBusMessageIter	 iter;
	DBusError err;
	int ret;
	int policy = 0;
	int idx = 0;
	int local = 1;
	int slot_id = HostSlotId;
	if (!strcmp(argv[0],"enable"))
	{
		policy = 1;	
	}
	else if (!strcmp(argv[0],"disable"))
	{
		policy = 0;	
	}
	else
	{
		vty_out(vty,"<error> input patameter only with 'enable' or 'disable'\n");
		return CMD_SUCCESS;
	}
	int i = 0;
	DBusConnection *dcli_dbus_connection = NULL;
	for(i = 1; i < MAX_SLOT_NUM; i++){
		if((NULL != dbus_connection_dcli[i])&&(NULL != dbus_connection_dcli[i]->dcli_dbus_connection)){
			dcli_dbus_connection = dbus_connection_dcli[i]->dcli_dbus_connection;
		}else
			continue;
		query = dbus_message_new_method_call(HMD_DBUS_BUSNAME,HMD_DBUS_OBJPATH,HMD_DBUS_INTERFACE, HMD_DBUS_CONF_METHOD_SET_IPFRAG_INGRESS_PMTU);
		dbus_error_init(&err);

		dbus_message_append_args(query,
								 DBUS_TYPE_UINT32,&policy,
								 DBUS_TYPE_INVALID);

		reply = dbus_connection_send_with_reply_and_block (dcli_dbus_connection,query,-1, &err);
		
		dbus_message_unref(query);
		
		if (NULL == reply)
		{
			cli_syslog_info("<error> failed get reply.\n");
			if (dbus_error_is_set(&err))
			{
				cli_syslog_info("%s raised: %s",err.name,err.message);
				dbus_error_free_for_dcli(&err);
			}
			
			return CMD_SUCCESS;
		}
		
		dbus_message_iter_init(reply,&iter);
		dbus_message_iter_get_basic(&iter,&ret);

		if(ret == 0)
		{
			vty_out(vty,"slot %d set ipfrag_ingress_pmtu %s successfully\n",i,argv[0]);
		}				
		else
		{
			vty_out(vty,"<error>  %d\n",ret);
		}
			
		dbus_message_unref(reply);
	}
	return CMD_SUCCESS;			
}


DEFUN(set_ipfrag_inform_nhmtu_instead_state_func,
	  set_ipfrag_inform_nhmtu_instead_state_func_cmd,
	  "set ipfrag_inform_nhmtu_instead (enable|disable)",
	  CONFIG_STR
	  "Enable or disable ipfrag_inform_nhmtu_instead\n"
	  "config CAPWAP tunnel\n"
	 )
{
	DBusMessage *query, *reply; 
	DBusMessageIter  iter;
	DBusError err;
	int ret;
	int policy = 0;
	int idx = 0;
	int local = 1;
	int slot_id = HostSlotId;
	if (!strcmp(argv[0],"enable"))
	{
		policy = 1; 
	}
	else if (!strcmp(argv[0],"disable"))
	{
		policy = 0; 
	}
	else
	{
		vty_out(vty,"<error> input patameter only with 'enable' or 'disable'\n");
		return CMD_SUCCESS;
	}
	int i = 0;
	DBusConnection *dcli_dbus_connection = NULL;
	for(i = 1; i < MAX_SLOT_NUM; i++){
		if((NULL != dbus_connection_dcli[i])&&(NULL != dbus_connection_dcli[i]->dcli_dbus_connection)){
			dcli_dbus_connection = dbus_connection_dcli[i]->dcli_dbus_connection;
		}else
			continue;
		query = dbus_message_new_method_call(HMD_DBUS_BUSNAME,HMD_DBUS_OBJPATH,HMD_DBUS_INTERFACE, HMD_DBUS_CONF_METHOD_SET_IPFRAG_INFORM_NHMTU_INSTEAD);
		dbus_error_init(&err);

		dbus_message_append_args(query,
								 DBUS_TYPE_UINT32,&policy,
								 DBUS_TYPE_INVALID);

		reply = dbus_connection_send_with_reply_and_block (dcli_dbus_connection,query,-1, &err);
		
		dbus_message_unref(query);
		
		if (NULL == reply)
		{
			cli_syslog_info("<error> failed get reply.\n");
			if (dbus_error_is_set(&err))
			{
				cli_syslog_info("%s raised: %s",err.name,err.message);
				dbus_error_free_for_dcli(&err);
			}
			
			return CMD_SUCCESS;
		}
		
		dbus_message_iter_init(reply,&iter);
		dbus_message_iter_get_basic(&iter,&ret);

		if(ret == 0)
		{
			vty_out(vty,"slot %d set ipfrag_inform_nhmtu_instead %s successfully\n",i,argv[0]);
		}				
		else
		{
			vty_out(vty,"<error>  %d\n",ret);
		}
			
		dbus_message_unref(reply);
	}
	return CMD_SUCCESS; 		
}


DEFUN(set_ipfrag_ignoredf_state_func,
	  set_ipfrag_ignoredf_state_func_cmd,
	  "set ipfrag_ignoredf (enable|disable)",
	  CONFIG_STR
	  "Enable or disable ipfrag_ignoredf\n"
	  "config CAPWAP tunnel\n"
	 )
{
	DBusMessage *query, *reply; 
	DBusMessageIter  iter;
	DBusError err;
	int ret;
	int policy = 0;
	int idx = 0;
	int local = 1;
	int slot_id = HostSlotId;
	if (!strcmp(argv[0],"enable"))
	{
		policy = 0; 
	}
	else if (!strcmp(argv[0],"disable"))
	{
		policy = 1; 
	}
	else
	{
		vty_out(vty,"<error> input patameter only with 'enable' or 'disable'\n");
		return CMD_SUCCESS;
	}
	int i = 0;
	DBusConnection *dcli_dbus_connection = NULL;
	for(i = 1; i < MAX_SLOT_NUM; i++){
		if((NULL != dbus_connection_dcli[i])&&(NULL != dbus_connection_dcli[i]->dcli_dbus_connection)){
			dcli_dbus_connection = dbus_connection_dcli[i]->dcli_dbus_connection;
		}else
			continue;
		query = dbus_message_new_method_call(HMD_DBUS_BUSNAME,HMD_DBUS_OBJPATH,HMD_DBUS_INTERFACE, HMD_DBUS_CONF_METHOD_SET_IPFRAG_IGNOREDF);
		dbus_error_init(&err);

		dbus_message_append_args(query,
								 DBUS_TYPE_UINT32,&policy,
								 DBUS_TYPE_INVALID);

		reply = dbus_connection_send_with_reply_and_block (dcli_dbus_connection,query,-1, &err);
		
		dbus_message_unref(query);
		
		if (NULL == reply)
		{
			cli_syslog_info("<error> failed get reply.\n");
			if (dbus_error_is_set(&err))
			{
				cli_syslog_info("%s raised: %s",err.name,err.message);
				dbus_error_free_for_dcli(&err);
			}
			
			return CMD_SUCCESS;
		}
		
		dbus_message_iter_init(reply,&iter);
		dbus_message_iter_get_basic(&iter,&ret);

		if(ret == 0)
		{
			vty_out(vty,"slot %d set ipfrag_ignoredf %s successfully\n",i,argv[0]);
		}				
		else
		{
			vty_out(vty,"<error>  %d\n",ret);
		}
			
		dbus_message_unref(reply);
	}
	return CMD_SUCCESS; 		
}


DEFUN(set_tunnel_wsm_wifi_rx_func,
	set_tunnel_wsm_wifi_rx_cmd,
	"set wsm (receive|transmit) (enable|disable)",
	"Enable or Disable wsm receive"
	"wireless-control config\n"
	"Enable or disable wsm receive module\n"
	"Enable wsm receive module\n"
	"Disable wsm receive module\n"
	)
{
	DBusMessage *query, *reply;	
	DBusMessageIter	 iter;
	DBusError err;
	int ret;
	int type = 0;
	int policy = 0;
	int idx = 0;
	
	if (!strcmp(argv[0],"receive"))
	{
		type = 1;	
	}
	else if (!strcmp(argv[0],"transmit"))
	{
		type = 0;	
	}
	else
	{
		vty_out(vty,"<error> input patameter only with 'receive' or 'transmit'\n");
		return CMD_SUCCESS;
	}
	if (!strcmp(argv[1],"enable"))
	{
		policy = 1;	
	}
	else if (!strcmp(argv[1],"disable"))
	{
		policy = 0;	
	}
	else
	{
		vty_out(vty,"<error> input patameter only with 'enable' or 'disable'\n");
		return CMD_SUCCESS;
	}
	
	int local = 1;
	int slot_id = HostSlotId;
	if (vty->node == CWTUNNEL_NODE)
		idx = 0;
	else if(vty->node == HANSI_CWTUNNEL_NODE){
		idx = (int)vty->index;
		local = vty->local;
		slot_id = vty->slotindex;
	}
	else if(vty->node == LOCAL_HANSI_CWTUNNEL_NODE){
		idx = vty->index;
		local = vty->local;
		slot_id = vty->slotindex;
	}
	DBusConnection *dcli_dbus_connection = NULL;
	ReInitDbusConnection(&dcli_dbus_connection,slot_id,distributFag);

	query = dbus_msg_new_method_call(local,WSM_DBUS_BUSNAME,WSM_DBUS_OBJPATH,
						WSM_DBUS_INTERFACE,WSM_DBUS_CONF_SET_TUNNEL_WSM_RX_TX, idx);
	dbus_error_init(&err);

	dbus_message_append_args(query,
 							 DBUS_TYPE_UINT32,&type,
 							 DBUS_TYPE_UINT32,&policy,
							 DBUS_TYPE_INVALID);

	reply = dbus_connection_send_with_reply_and_block (dcli_dbus_connection,query,-1, &err);
	
	dbus_message_unref(query);
	
	if (NULL == reply)
	{
		cli_syslog_info("<error> failed get reply.\n");
		if (dbus_error_is_set(&err))
		{
			cli_syslog_info("%s raised: %s",err.name,err.message);
			dbus_error_free_for_dcli(&err);
		}
		
		return CMD_SUCCESS;
	}
	
	dbus_message_iter_init(reply,&iter);
	dbus_message_iter_get_basic(&iter,&ret);

	if(ret == 0)
	{
		vty_out(vty," set %s %s successfully\n",argv[0],argv[1]);
	}				
	else
	{
		vty_out(vty,"<error>  %d\n",ret);
	}
		
	dbus_message_unref(reply);

	return CMD_SUCCESS;			
}

int dcli_capwap_tunnel_show_running_config_start(struct vty *vty)
{
	DBusMessage *query, *reply;
	DBusError err;
	unsigned char *cmd = NULL;
	int idx = 0;
	int local = 1;
	int slot_id = HostSlotId;
	DBusConnection *dcli_dbus_connection = NULL;
	ReInitDbusConnection(&dcli_dbus_connection,slot_id,distributFag);
	query = dbus_msg_new_method_call(local,
									WSM_DBUS_BUSNAME,		
									WSM_DBUS_OBJPATH, 
									WSM_DBUS_INTERFACE,
									WSM_DBUS_CAPWAP_TUNNEL_SHOW_RUNNING, idx);

	dbus_error_init(&err);
	reply = dbus_connection_send_with_reply_and_block (dcli_dbus_connection, query, 300000, &err); //fengwenchao change "-1" to "300000" for TESTBED-7,20111213
	dbus_message_unref(query);
	
	if (NULL == reply) 
	{
		printf("show capwap-tunnel config failed get reply.\n");
		if (dbus_error_is_set(&err))
		{
			printf("%s raised: %s",err.name,err.message);
			dbus_error_free_for_dcli(&err);
		}
		return -1;
	}

	if (dbus_message_get_args (reply, &err, DBUS_TYPE_STRING, &cmd, DBUS_TYPE_INVALID)) 
	{
		vtysh_add_show_string(cmd);
	} 
	else 
	{
		printf("Failed get args.\n");
		if (dbus_error_is_set(&err)) 
		{
			printf("%s raised: %s",err.name,err.message);
			dbus_error_free_for_dcli(&err);
		}
	}
	
	dbus_message_unref(reply);

	return 0;	
}

/*
* Description:
*  Re-encapsulation function dbus_message_new_method_call for Hansi.
*
* Parameter:
*  idx: VRID
*  Other same to function dbus_message_new_method_call
*
* Return:
*  Same to function dbus_message_new_method_call.
*
*/
DBusMessage *dbus_msg_new_method_call(int local,unsigned char *dbus_name, unsigned char *obj_name,
			unsigned char *if_name, unsigned char *cmd_name, int idx)
{
	unsigned char dbus_path[PATH_MAX_LEN] = {0};
	unsigned char obj_path[PATH_MAX_LEN] = {0};
	unsigned char if_path[PATH_MAX_LEN] = {0};
	unsigned char cmd_path[PATH_MAX_LEN] = {0};

	if (dbus_name == NULL || obj_name == NULL || if_name == NULL || cmd_name == NULL || idx < 0)
		return NULL;
#ifndef _DISTRIBUTION_
	sprintf(dbus_path, "%s%d", dbus_name, idx);
	sprintf(obj_path, "%s%d", obj_name, idx);
	sprintf(if_path, "%s%d", if_name, idx);
	sprintf(cmd_path, "%s%d", cmd_name, idx);
	printf("11 dbus_name=%s,cmd_name=%s.\n",dbus_name,cmd_name);
#else
	sprintf(dbus_path, "%s%d_%d", dbus_name,local, idx);
	sprintf(obj_path, "%s%d_%d", obj_name,local,  idx);
	sprintf(if_path, "%s%d_%d", if_name,local,  idx);
	sprintf(cmd_path, "%s%d_%d", cmd_name,local,  idx);
//	printf("22 dbus_name=%s,cmd_name=%s.\n",dbus_name,cmd_name);
#endif

	return dbus_message_new_method_call(dbus_path, obj_path, if_path, cmd_path);
} 

void dcli_wsm_init(void)
{
	install_node(&cwtunnel_node, dcli_capwap_tunnel_show_running_config_start, "CWTUNNEL_NODE");
	install_default(CWTUNNEL_NODE);

	install_element(CONFIG_NODE, &config_tunnel_cmd);
	install_element(CWTUNNEL_NODE,&set_tunnel_flow_based_forwarding_cmd);
	install_element(CWTUNNEL_NODE,&show_tunnel_flow_based_forwarding_state_cmd);
//	install_element(CWTUNNEL_NODE, &show_tunnel_wtp_list_cmd);
//	install_element(CWTUNNEL_NODE, &show_tunnel_log_wsm_state_cmd);
//	install_element(CWTUNNEL_NODE, &set_tunnel_log_wsm_level_cmd);
//	install_element(CWTUNNEL_NODE, &show_tunnel_wtp_wtpid_cmd);
//	install_element(CWTUNNEL_NODE, &show_tunnel_bss_bssid_cmd);	
//	install_element(CWTUNNEL_NODE, &show_tunnel_sta_stamac_cmd);
//	install_element(CWTUNNEL_NODE, &set_tunnel_wsm_watchdog_cmd);
//	install_element(CWTUNNEL_NODE, &show_tunnel_wsm_watchdog_state_cmd);
	install_element(CWTUNNEL_NODE, &set_ipfrag_ingress_pmtu_state_func_cmd);
	install_element(CWTUNNEL_NODE, &set_ipfrag_inform_nhmtu_instead_state_func_cmd);
	install_element(CWTUNNEL_NODE, &set_ipfrag_ignoredf_state_func_cmd);
	install_element(CWTUNNEL_NODE, &set_tunnel_qos_map_cmd);
//	install_element(CWTUNNEL_NODE, &set_tunnel_wsm_wifi_rx_cmd);
	/* The function relate to flow-based module just exist in main instance */
	install_node(&hansi_cwtunnel_node, NULL, "HANSI_CWTUNNEL_NODE");
	install_default(HANSI_CWTUNNEL_NODE);

	install_element(HANSI_NODE, &config_tunnel_cmd);
	install_element(HANSI_CWTUNNEL_NODE, &show_tunnel_wtp_list_cmd);
	install_element(HANSI_CWTUNNEL_NODE, &show_tunnel_log_wsm_state_cmd);
	install_element(HANSI_CWTUNNEL_NODE, &set_tunnel_log_wsm_level_cmd);
	install_element(HANSI_CWTUNNEL_NODE, &show_tunnel_wtp_wtpid_cmd);
	install_element(HANSI_CWTUNNEL_NODE, &show_tunnel_bss_bssid_cmd);	
	install_element(HANSI_CWTUNNEL_NODE, &show_tunnel_sta_stamac_cmd);
	install_element(HANSI_CWTUNNEL_NODE, &set_tunnel_wsm_watchdog_cmd);
	install_element(HANSI_CWTUNNEL_NODE, &show_tunnel_wsm_watchdog_state_cmd);
	install_element(HANSI_CWTUNNEL_NODE, &set_tunnel_wsm_wifi_rx_cmd);

	install_node(&local_hansi_cwtunnel_node, NULL, "HANSI_CWTUNNEL_NODE");
	install_default(LOCAL_HANSI_CWTUNNEL_NODE);
	install_element(LOCAL_HANSI_NODE, &config_tunnel_cmd);
	install_element(LOCAL_HANSI_CWTUNNEL_NODE, &show_tunnel_wtp_list_cmd);
	install_element(LOCAL_HANSI_CWTUNNEL_NODE, &show_tunnel_log_wsm_state_cmd);
	install_element(LOCAL_HANSI_CWTUNNEL_NODE, &set_tunnel_log_wsm_level_cmd);
	install_element(LOCAL_HANSI_CWTUNNEL_NODE, &show_tunnel_wtp_wtpid_cmd);
	install_element(LOCAL_HANSI_CWTUNNEL_NODE, &show_tunnel_bss_bssid_cmd);	
	install_element(LOCAL_HANSI_CWTUNNEL_NODE, &show_tunnel_sta_stamac_cmd);
	install_element(LOCAL_HANSI_CWTUNNEL_NODE, &set_tunnel_wsm_watchdog_cmd);
	install_element(LOCAL_HANSI_CWTUNNEL_NODE, &show_tunnel_wsm_watchdog_state_cmd);
	install_element(LOCAL_HANSI_CWTUNNEL_NODE, &set_tunnel_wsm_wifi_rx_cmd);

	//install_element(LOCAL_HANSI_CWTUNNEL_NODE, &set_tunnel_wsm_wifi_rx_cmd);




}

#endif
