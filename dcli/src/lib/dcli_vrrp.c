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
* dcli_vrrp.c
*
*
* CREATOR:
*		zhengcs@autelan.com
*
* DESCRIPTION:
*		CLI definition for VRRP module.
*
* DATE:
*		06/16/2009	
*
*  FILE REVISION NUMBER:
*  		$Revision: 1.60 $	
*******************************************************************************/
#ifdef __cplusplus
extern "C"
{
#endif

#include <zebra.h>
#include <lib/vty.h>
#include "vtysh/vtysh.h"
#include <dbus/dbus.h>
#include <stdlib.h>
#include <sysdef/npd_sysdef.h>
#include <dbus/npd/npd_dbus_def.h>
#include <util/npd_list.h>

#include "sysdef/returncode.h"

#include "dcli_main.h"
#include "dcli_vrrp.h"
#include "dcli_dhcp.h"
#include "dcli_acl.h"
#include "dcli_pppoe.h"
#include "command.h"
#include "memory.h"
#include "dbus/dhcp/dhcp_dbus_def.h"
//#include "dbus/hmd/HmdDbusPath.h"
#include "dbus/hmd/HmdDbusDef.h"
#include "dbus/wcpss/ACDbusDef1.h"
#include "hmd/hmdpub.h"
#define DCLI_IPMASK_STRING_MAXLEN	(strlen("255.255.255.255/32"))
#define DCLI_IPMASK_STRING_MINLEN	(strlen("0.0.0.0/0"))
#define DCLI_IP_STRING_MAXLEN	(strlen("255.255.255.255"))
#define DCLI_IP_STRING_MINLEN   (strlen("0.0.0.0"))
#define PATH_LEN 64

extern DBusConnection *dcli_dbus_connection;

struct cmd_node hansi_node = 
{
	HANSI_NODE,
	"%s(config-hansi %d-%d)# "
};

unsigned char *dcli_vrrp_err_msg[] = {	\
/*   0 */	"%% Error none\n",
/*   1 */	"%% General failure\n",
/*   2 */	"%% Profile out of range\n",
/*   3 */	"%% Profile has already exist\n",
/*   4 */	"%% Profile not exist\n",
/*   5 */	"%% Memory malloc failed\n",
/*   6 */	"%% Bad parameter input\n",
/*   7 */	"%% Heartbeatlink or uplink,downlink,vgateway not configured\n",
/*	 8 */	"%% Service should be disabled first\n",
/*   9 */	"%% Virtual gateway have not setted\n",
/*	 A */	"%% Did not choose the appropriate (link+priority) mode\n",
/*	 B */	"%% Interface name error\n",
/*	 C */	"%% Virtual IP address has setted\n",
/*	 D */	"%% Virtual IP address or interface has not setted\n",
/*	 E */	"%% Virtual IP address is last one, not allow to delete\n",
/*	 F */	"%% Interface has exist\n",
/*	 10 */	"%% Interface not exist\n",
/*    11 */  "%% No more items can be configed for this command!\n",
/*	  12 */  "%% Virtual mac should be enable first\n",
/*    13 */  "%% The value is out of range!\n",

};


unsigned int dcli_show_dhcp_failover
(
   struct vty *vty,
	unsigned int profile,
	struct dhcp_failover_state *state,
	unsigned int slot_id
)
{
   	DBusMessage *query = NULL, *reply = NULL;
	DBusError err;
	DBusMessageIter iter;
	unsigned int op_ret = 0;
	unsigned int peer_state = 0;
	unsigned int local_state = 0;

	if (!vty || !state) {
		return DCLI_VRRP_RETURN_CODE_ERR;
	}
    
	DBusConnection *dcli_dbus_connection = NULL;
	ReInitDbusConnection(&dcli_dbus_connection,slot_id,distributFag);

	query = dbus_message_new_method_call(DHCP_DBUS_BUSNAME,
										 DHCP_DBUS_OBJPATH,
										 DHCP_DBUS_INTERFACE,
										 DHCP_DBUS_METHOD_GET_FAILOVER_STATE);
	dbus_error_init(&err);
	dbus_message_append_args(query,
	                     DBUS_TYPE_UINT32, &profile,				 
						 DBUS_TYPE_INVALID);

	reply = dbus_connection_send_with_reply_and_block(dcli_dbus_connection, query, -1, &err);
	dbus_message_unref(query);
	if (NULL == reply) {
		vty_out(vty,"failed get reply.\n");
		if (dbus_error_is_set(&err)) {
			printf("%s raised: %s", err.name, err.message);
			dbus_error_free(&err);
		}
		//dbus_message_unref(reply);		
		return DCLI_VRRP_RETURN_CODE_ERR;
	}
	dbus_message_iter_init(reply, &iter);
	dbus_message_iter_get_basic(&iter, &op_ret);
	if((DHCP_FAILOVER_NOT_ENABLE == op_ret)) {
	   dbus_message_unref(reply);	
	   return op_ret;
	}

	dbus_message_iter_next(&iter);
	dbus_message_iter_get_basic(&iter, &local_state);

	dbus_message_iter_next(&iter);
	dbus_message_iter_get_basic(&iter, &peer_state);

	state->local = local_state;
	state->peer = peer_state;

	dbus_message_unref(reply);

	return DCLI_VRRP_RETURN_CODE_OK;
}


#ifndef DISTRIBUT
int dcli_show_hansi_profile_detail
(
   struct vty* vty,
   int profile
)
{
   	DBusMessage *query = NULL, *reply = NULL;
	DBusError err = {0};
	DBusMessageIter	 iter;
	unsigned int op_ret = 0;
	unsigned int advert = 0,preempt = 0,priority = 0,state = 0,vir_mac = 0;
	char *uplink_ifname = NULL,*downlink_ifname =NULL,*heartbeatlink_ifname = NULL,*vgateway_ifname = NULL;
    char *action = NULL,*step = NULL;
	int uplink_ip = 0,downlink_ip = 0,heartbeatlink_ip = 0,ip = 0;
	int wid_transfer = 0,portal_enable = 0,portal_transfer = 0;
    int vgateway_ip = 0,vgateway_mask = 0;
    unsigned char link = 0;
	unsigned int  heartbeat_link = 0,vgateway_link = 0;
	unsigned int  log = 0;
	unsigned int uplink_flag = 0, downlink_flag = 0, heartbeat_flag = 0, vgateway_flag = 0;
	int i = 0;
	int uplink_naddr = 0, downlink_naddr = 0, vgateway_naddr = 0;
	char vrrp_obj_path[DCLI_VRRP_OBJPATH_LEN] = {0};
	char vrrp_dbus_name[DCLI_VRRP_DBUSNAME_LEN] = {0};
	int instRun = 0;
	unsigned int failover_peer = 0, failover_local = 0;
	unsigned int l2_uplink_flag = 0;
	char *l2_uplink_ifname = NULL;
	int l2_uplink_naddr = 0;

	/* [1] check if had process has started before or not */
	instRun = dcli_vrrp_hansi_is_running(vty, profile);
	if (DCLI_VRRP_INSTANCE_NO_CREATED == instRun) {
		//vty_out(vty, "had instance %d not created!\n", profile);
		return DCLI_VRRP_RETURN_CODE_PROFILE_NOTEXIST;
	}
	else if (DCLI_VRRP_INSTANCE_CHECK_FAILED == instRun) {
		//vty_out(vty, "check had instance %d whether created was failed!\n",
		//			profile);
		return DCLI_VRRP_RETURN_CODE_ERR;
	}

	dcli_vrrp_splice_objpath_string(VRRP_DBUS_OBJPATH, profile, vrrp_obj_path);
	dcli_vrrp_splice_dbusname(VRRP_DBUS_BUSNAME, profile, vrrp_dbus_name);
	
	query = dbus_message_new_method_call(vrrp_dbus_name,
										 vrrp_obj_path,
										 VRRP_DBUS_INTERFACE,
										 VRRP_DBUS_METHOD_SHOW_DETAIL);
	dbus_error_init(&err);

	dbus_message_append_args(query,
		                     DBUS_TYPE_UINT32,&profile,				 
							 DBUS_TYPE_INVALID);
	
	reply = dbus_connection_send_with_reply_and_block (dcli_dbus_connection,query,-1, &err);
	dbus_message_unref(query);
	if (NULL == reply) {
		vty_out(vty,"failed get reply.\n");
		if (dbus_error_is_set(&err)) {
			printf("%s raised: %s",err.name,err.message);
			dbus_error_free_for_dcli(&err);
		}
		//dbus_message_unref(reply);		
		return DCLI_VRRP_RETURN_CODE_ERR;
	}
	dbus_message_iter_init(reply,&iter);
	dbus_message_iter_get_basic(&iter,&op_ret);
	if(DCLI_VRRP_RETURN_CODE_PROFILE_NOTEXIST == op_ret){
	   dbus_message_unref(reply);
	   return DCLI_VRRP_RETURN_CODE_PROFILE_NOTEXIST;
	}
	vty_out(vty,"HANSI %d detail info:\n",profile);
	dbus_message_iter_next(&iter);
	dbus_message_iter_get_basic(&iter,&state);

	dbus_message_iter_next(&iter);
	dbus_message_iter_get_basic(&iter,&log);
	vty_out(vty,"%-12s:%-s\n","STATE",(state==3)?"MASTER":(state == 2)?"BACK" : (state == 99)?"DISABLE" : (state == 4)?"LEARNING" :(state == 6)? "TRANSFER" :"INIT");
    if(log){
		dbus_message_iter_next(&iter);
		dbus_message_iter_get_basic(&iter,&action);
		dbus_message_iter_next(&iter);
		dbus_message_iter_get_basic(&iter,&step);
        vty_out(vty,"    action: %s\n",action);
		vty_out(vty,"    detal : %s\n",step);
	}
	dbus_message_iter_next(&iter);
	dbus_message_iter_get_basic(&iter,&priority);
	vty_out(vty,"%-12s:%d\n","PRIORITY",priority);
	dbus_message_iter_next(&iter);
	dbus_message_iter_get_basic(&iter,&advert);
	vty_out(vty,"%-12s:%d(s)\n","ADVERTISEMENT",advert);
	dbus_message_iter_next(&iter);
	dbus_message_iter_get_basic(&iter,&preempt);
	vty_out(vty,"%-12s:%-s\n","PREEMPT",preempt ? "Yes" : "No");
	dbus_message_iter_next(&iter);
	dbus_message_iter_get_basic(&iter,&vir_mac);
	vty_out(vty,"%-12s:%s\n","VIRTUAL MAC",vir_mac ? "No" : "Yes");	

	/* uplink interface */
	dbus_message_iter_next(&iter);
	dbus_message_iter_get_basic(&iter,&uplink_flag);
	vty_out(vty, "-UPLINK-\n");
	if(1 == uplink_flag){
		dbus_message_iter_next(&iter);
		dbus_message_iter_get_basic(&iter, &uplink_naddr);
		for (i = 0; i < uplink_naddr; i++)
		{
			dbus_message_iter_next(&iter);
			dbus_message_iter_get_basic(&iter,&uplink_ifname);	
			dbus_message_iter_next(&iter);
			dbus_message_iter_get_basic(&iter,&link);
			dbus_message_iter_next(&iter);
			dbus_message_iter_get_basic(&iter,&ip);
			dbus_message_iter_next(&iter);
			dbus_message_iter_get_basic(&iter,&uplink_ip);
			vty_out(vty, "%-7s%-3d%-s[%-s] ip real %d.%d.%d.%d virtual %d.%d.%d.%d\n", \
							"", i+1, uplink_ifname ? uplink_ifname : "nil", link ? "U":"D", \
							((ip & 0xff000000) >> 24),((ip & 0xff0000) >> 16),	\
							((ip& 0xff00) >> 8),(ip & 0xff), \
							((uplink_ip & 0xff000000) >> 24),((uplink_ip & 0xff0000) >> 16),	\
						((uplink_ip& 0xff00) >> 8),(uplink_ip & 0xff));
		}
	}
	else {
		vty_out(vty, "%-7snot configured\n", "");
	}


	/* l2-uplink interface */
	dbus_message_iter_next(&iter);
	dbus_message_iter_get_basic(&iter,&l2_uplink_flag);
	vty_out(vty, "-L2-UPLINK-\n");
	if(1 == l2_uplink_flag){
		dbus_message_iter_next(&iter);
		dbus_message_iter_get_basic(&iter, &l2_uplink_naddr);
		for (i = 0; i < l2_uplink_naddr; i++)
		{
			dbus_message_iter_next(&iter);
			dbus_message_iter_get_basic(&iter,&l2_uplink_ifname);	
			dbus_message_iter_next(&iter);
			dbus_message_iter_get_basic(&iter,&link);
			vty_out(vty, "%-7s%-3d%-s[%-s]\n", \
							"", i+1, l2_uplink_ifname ? l2_uplink_ifname : "nil", link ? "U":"D");
		}
	}
	else {
		vty_out(vty, "%-7snot configured\n", "");
	}

	/* downlink interface */
	dbus_message_iter_next(&iter);
	dbus_message_iter_get_basic(&iter,&downlink_flag);
	vty_out(vty, "-DOWNLINK-\n");
	if (1 == downlink_flag) {		
		dbus_message_iter_next(&iter);
		dbus_message_iter_get_basic(&iter, &downlink_naddr);

		for (i = 0; i < downlink_naddr; i++)
		{
			dbus_message_iter_next(&iter);
			dbus_message_iter_get_basic(&iter,&downlink_ifname);	
			dbus_message_iter_next(&iter);
			dbus_message_iter_get_basic(&iter,&link);
			dbus_message_iter_next(&iter);
			dbus_message_iter_get_basic(&iter,&ip);
			dbus_message_iter_next(&iter);
			dbus_message_iter_get_basic(&iter,&downlink_ip);
			vty_out(vty, "%-7s%-3d%-s[%-s] ip real %d.%d.%d.%d virtual %d.%d.%d.%d\n", \
							"", i+1, downlink_ifname ? downlink_ifname:"nil", link ? "U":"D", \
							((ip & 0xff000000) >> 24),((ip & 0xff0000) >> 16),	\
							((ip& 0xff00) >> 8),(ip & 0xff), \
							((downlink_ip & 0xff000000) >> 24),((downlink_ip & 0xff0000) >> 16),	\
						((downlink_ip & 0xff00) >> 8),(downlink_ip & 0xff));
		}
	}
	else {
		vty_out(vty, "%-7snot configured\n","");
	}

	/* wireless and portal state */
	dbus_message_iter_next(&iter);
	dbus_message_iter_get_basic(&iter,&wid_transfer);
	dbus_message_iter_next(&iter);
	dbus_message_iter_get_basic(&iter,&portal_enable);	
	dbus_message_iter_next(&iter);
	dbus_message_iter_get_basic(&iter,&portal_transfer);
    if(state == 6){
	   vty_out(vty,"Batch-sync state\n");
       vty_out(vty,"%-2s%-10s:%-s\n", "", "wireless",wid_transfer ? "TRANSFERING" : "RUN");
	   vty_out(vty,"%-2s%-10s:%-s\n", "", "portal", portal_enable ?  \
	   					(portal_transfer ? "TRANSFERING" : "RUN") : "NOT ELECTION");
	}	

	/* heartbeat interface */
	dbus_message_iter_next(&iter);
	dbus_message_iter_get_basic(&iter,&heartbeatlink_ip);
	dbus_message_iter_next(&iter);
	dbus_message_iter_get_basic(&iter,&heartbeat_link);
	dbus_message_iter_next(&iter);
	dbus_message_iter_get_basic(&iter,&heartbeat_flag);
	vty_out(vty, "-HEARTBEAT-\n");
	if (1 == heartbeat_flag)
	{
		dbus_message_iter_next(&iter);	
		dbus_message_iter_get_basic(&iter,&heartbeatlink_ifname);
		vty_out(vty, "%-7s%-s[%-s] ip ", \
						"", heartbeatlink_ifname ? heartbeatlink_ifname:"nil", heartbeat_link ? "U":"D");
		if(heartbeatlink_ip) {
			vty_out(vty,"%d.%d.%d.%d\n",((heartbeatlink_ip & 0xff000000) >> 24),((heartbeatlink_ip & 0xff0000) >> 16),	\
							((heartbeatlink_ip& 0xff00) >> 8),(heartbeatlink_ip & 0xff));
		}
		else {
			vty_out(vty, "0\n");		
		}
	}
	else {
		vty_out(vty, "%-7snot configured\n", "");
	}

	/* vgateway interface */
	dbus_message_iter_next(&iter);
	dbus_message_iter_get_basic(&iter,&vgateway_flag);
	vty_out(vty, "-VGATEWAY-\n");
	if(0 != vgateway_flag){
		dbus_message_iter_next(&iter);
		dbus_message_iter_get_basic(&iter, &vgateway_naddr);
		for(i = 0; i < vgateway_naddr; i++) {
			dbus_message_iter_next(&iter);
			dbus_message_iter_get_basic(&iter,&vgateway_ifname);
			dbus_message_iter_next(&iter);
			dbus_message_iter_get_basic(&iter,&vgateway_ip);
			dbus_message_iter_next(&iter);
			dbus_message_iter_get_basic(&iter,&vgateway_mask);
			dbus_message_iter_next(&iter);
			dbus_message_iter_get_basic(&iter,&vgateway_link);		
			vty_out(vty, "%-7s%-3d%-s[%-s] ip %d.%d.%d.%d", \
							"", i+1, vgateway_ifname ? vgateway_ifname : "nil", vgateway_link ? "U":"D", \
							((vgateway_ip & 0xff000000) >> 24),((vgateway_ip & 0xff0000) >> 16),	\
							((vgateway_ip& 0xff00) >> 8),(vgateway_ip & 0xff), vgateway_mask);
			if(vgateway_mask) {
				vty_out(vty, "/%d", vgateway_mask);
			}
			vty_out(vty, "\n");
		}
	}
	else{
        vty_out(vty, "%-7snot configured\n","");
	}

	/* dhcp failover config */
	dbus_message_iter_next(&iter);
	dbus_message_iter_get_basic(&iter,&failover_peer);
	dbus_message_iter_next(&iter);
	dbus_message_iter_get_basic(&iter,&failover_local);
	vty_out(vty, "-DHCP FAILOVER-\n");
	if(~0UI != failover_peer) {
		struct dhcp_failover_state failover_state;
		memset(&failover_state, 0, sizeof(failover_state));
		if (DCLI_VRRP_RETURN_CODE_OK != dcli_show_dhcp_failover(vty, profile, &failover_state, slot_id)){
		    vty_out(vty, "cannot get failobver state\n");
		}
		vty_out(vty, "%-7speer [%s] %d.%d.%d.%d","", dcli_dhcp_failover_state_print(failover_state.peer),(failover_peer >> 24) & 0xFF, \
			(failover_peer >> 16) & 0xFF, (failover_peer >> 8) & 0xFF, failover_peer & 0xFF);
		if(~0UI != failover_local) {
			vty_out(vty, " local [%s] %d.%d.%d.%d",dcli_dhcp_failover_state_print(failover_state.local), (failover_local >> 24) & 0xFF, \
				(failover_local >> 16) & 0xFF, (failover_local >> 8) & 0xFF, failover_local & 0xFF);
		}
		vty_out(vty, "\n");
	}
	else {
		vty_out(vty, "%-7snot configured\n","");
	}
	dbus_message_unref(reply);
	return DCLI_VRRP_RETURN_CODE_OK;
}
#else
int dcli_show_hansi_profile_detail
(
   struct vty* vty,
   int profile,
   int slot_id
)
{
	DBusMessage *query = NULL, *reply = NULL;
	DBusError err = {0};
	DBusMessageIter  iter;
	unsigned int op_ret = 0;
	unsigned int advert = 0,preempt = 0,priority = 0,state = 0,vir_mac = 0;
	char *uplink_ifname = NULL,*downlink_ifname =NULL,*heartbeatlink_ifname = NULL,*vgateway_ifname = NULL;
	char *action = NULL,*step = NULL;
	int uplink_ip = 0,downlink_ip = 0,heartbeatlink_ip = 0,ip = 0;
	int wid_transfer = 0,portal_enable = 0,portal_transfer = 0;
	int vgateway_ip = 0,vgateway_mask = 0;
	unsigned char link = 0;
	unsigned int  heartbeat_link = 0,vgateway_link = 0;
	unsigned int  log = 0;
	unsigned int uplink_flag = 0, downlink_flag = 0, heartbeat_flag = 0, vgateway_flag = 0;
	int i = 0;
	int uplink_naddr = 0, downlink_naddr = 0, vgateway_naddr = 0;
	char vrrp_obj_path[DCLI_VRRP_OBJPATH_LEN] = {0};
	char vrrp_dbus_name[DCLI_VRRP_DBUSNAME_LEN] = {0};
	int instRun = 0;
	unsigned int failover_peer = 0, failover_local = 0;
	unsigned int l2_uplink_flag = 0;
	char *l2_uplink_ifname = NULL;
	int l2_uplink_naddr = 0;

	/* [1] check if had process has started before or not */
	instRun = dcli_hmd_hansi_is_running(vty,slot_id, 0,profile);
	if (INSTANCE_NO_CREATED == instRun) {
		//vty_out(vty, "had instance %d not created!\n", profile);
		return DCLI_VRRP_RETURN_CODE_PROFILE_NOTEXIST;
	}
#ifdef DISTRIBUT
	DBusConnection *dcli_dbus_connection = NULL;
	ReInitDbusConnection(&dcli_dbus_connection,slot_id,distributFag);
#endif
	dcli_vrrp_splice_objpath_string(VRRP_DBUS_OBJPATH, profile, vrrp_obj_path);
	dcli_vrrp_splice_dbusname(VRRP_DBUS_BUSNAME, profile, vrrp_dbus_name);
	
	query = dbus_message_new_method_call(vrrp_dbus_name,
										 vrrp_obj_path,
										 VRRP_DBUS_INTERFACE,
										 VRRP_DBUS_METHOD_SHOW_DETAIL);
	dbus_error_init(&err);

	dbus_message_append_args(query,
							 DBUS_TYPE_UINT32,&profile, 			 
							 DBUS_TYPE_INVALID);
	
	reply = dbus_connection_send_with_reply_and_block (dcli_dbus_connection,query,-1, &err);
	dbus_message_unref(query);
	if (NULL == reply) {
		vty_out(vty,"failed get reply.\n");
		if (dbus_error_is_set(&err)) {
			printf("%s raised: %s",err.name,err.message);
			dbus_error_free_for_dcli(&err);
		}
		//dbus_message_unref(reply);		
		return DCLI_VRRP_RETURN_CODE_ERR;
	}
	dbus_message_iter_init(reply,&iter);
	dbus_message_iter_get_basic(&iter,&op_ret);
	if(DCLI_VRRP_RETURN_CODE_PROFILE_NOTEXIST == op_ret){
	   dbus_message_unref(reply);
	   return DCLI_VRRP_RETURN_CODE_PROFILE_NOTEXIST;
	}
	vty_out(vty,"HANSI %d detail info:\n",profile);
	dbus_message_iter_next(&iter);
	dbus_message_iter_get_basic(&iter,&state);

	dbus_message_iter_next(&iter);
	dbus_message_iter_get_basic(&iter,&log);
	vty_out(vty,"%-12s:%-s\n","STATE",(state==3)?"MASTER":(state == 2)?"BACK" : (state == 99)?"DISABLE" : (state == 4)?"LEARNING" :(state == 6)? "TRANSFER" :"INIT");
	if(log){
		dbus_message_iter_next(&iter);
		dbus_message_iter_get_basic(&iter,&action);
		dbus_message_iter_next(&iter);
		dbus_message_iter_get_basic(&iter,&step);
		vty_out(vty,"	 action: %s\n",action);
		vty_out(vty,"	 detal : %s\n",step);
	}
	dbus_message_iter_next(&iter);
	dbus_message_iter_get_basic(&iter,&priority);
	vty_out(vty,"%-12s:%d\n","PRIORITY",priority);
	dbus_message_iter_next(&iter);
	dbus_message_iter_get_basic(&iter,&advert);
	vty_out(vty,"%-12s:%d(s)\n","ADVERTISEMENT",advert);
	dbus_message_iter_next(&iter);
	dbus_message_iter_get_basic(&iter,&preempt);
	vty_out(vty,"%-12s:%-s\n","PREEMPT",preempt ? "Yes" : "No");
	dbus_message_iter_next(&iter);
	dbus_message_iter_get_basic(&iter,&vir_mac);
	vty_out(vty,"%-12s:%s\n","VIRTUAL MAC",vir_mac ? "No" : "Yes"); 

	/* uplink interface */
	dbus_message_iter_next(&iter);
	dbus_message_iter_get_basic(&iter,&uplink_flag);
	vty_out(vty, "-UPLINK-\n");
	if(1 == uplink_flag){
		dbus_message_iter_next(&iter);
		dbus_message_iter_get_basic(&iter, &uplink_naddr);
		for (i = 0; i < uplink_naddr; i++)
		{
			dbus_message_iter_next(&iter);
			dbus_message_iter_get_basic(&iter,&uplink_ifname);	
			dbus_message_iter_next(&iter);
			dbus_message_iter_get_basic(&iter,&link);
			dbus_message_iter_next(&iter);
			dbus_message_iter_get_basic(&iter,&ip);
			dbus_message_iter_next(&iter);
			dbus_message_iter_get_basic(&iter,&uplink_ip);
			vty_out(vty, "%-7s%-3d%-s[%-s] ip real %d.%d.%d.%d virtual %d.%d.%d.%d\n", \
							"", i+1, uplink_ifname ? uplink_ifname : "nil", link ? "U":"D", \
							((ip & 0xff000000) >> 24),((ip & 0xff0000) >> 16),	\
							((ip& 0xff00) >> 8),(ip & 0xff), \
							((uplink_ip & 0xff000000) >> 24),((uplink_ip & 0xff0000) >> 16),	\
						((uplink_ip& 0xff00) >> 8),(uplink_ip & 0xff));
		}
	}
	else {
		vty_out(vty, "%-7snot configured\n", "");
	}


	/* l2-uplink interface */
	dbus_message_iter_next(&iter);
	dbus_message_iter_get_basic(&iter,&l2_uplink_flag);
	vty_out(vty, "-L2-UPLINK-\n");
	if(1 == l2_uplink_flag){
		dbus_message_iter_next(&iter);
		dbus_message_iter_get_basic(&iter, &l2_uplink_naddr);
		for (i = 0; i < l2_uplink_naddr; i++)
		{
			dbus_message_iter_next(&iter);
			dbus_message_iter_get_basic(&iter,&l2_uplink_ifname);	
			dbus_message_iter_next(&iter);
			dbus_message_iter_get_basic(&iter,&link);
			vty_out(vty, "%-7s%-3d%-s[%-s]\n", \
							"", i+1, l2_uplink_ifname ? l2_uplink_ifname : "nil", link ? "U":"D");
		}
	}
	else {
		vty_out(vty, "%-7snot configured\n", "");
	}

	/* downlink interface */
	dbus_message_iter_next(&iter);
	dbus_message_iter_get_basic(&iter,&downlink_flag);
	vty_out(vty, "-DOWNLINK-\n");
	if (1 == downlink_flag) {		
		dbus_message_iter_next(&iter);
		dbus_message_iter_get_basic(&iter, &downlink_naddr);

		for (i = 0; i < downlink_naddr; i++)
		{
			dbus_message_iter_next(&iter);
			dbus_message_iter_get_basic(&iter,&downlink_ifname);	
			dbus_message_iter_next(&iter);
			dbus_message_iter_get_basic(&iter,&link);
			dbus_message_iter_next(&iter);
			dbus_message_iter_get_basic(&iter,&ip);
			dbus_message_iter_next(&iter);
			dbus_message_iter_get_basic(&iter,&downlink_ip);
			vty_out(vty, "%-7s%-3d%-s[%-s] ip real %d.%d.%d.%d virtual %d.%d.%d.%d\n", \
							"", i+1, downlink_ifname ? downlink_ifname:"nil", link ? "U":"D", \
							((ip & 0xff000000) >> 24),((ip & 0xff0000) >> 16),	\
							((ip& 0xff00) >> 8),(ip & 0xff), \
							((downlink_ip & 0xff000000) >> 24),((downlink_ip & 0xff0000) >> 16),	\
						((downlink_ip & 0xff00) >> 8),(downlink_ip & 0xff));
		}
	}
	else {
		vty_out(vty, "%-7snot configured\n","");
	}

	/* wireless and portal state */
	dbus_message_iter_next(&iter);
	dbus_message_iter_get_basic(&iter,&wid_transfer);
	dbus_message_iter_next(&iter);
	dbus_message_iter_get_basic(&iter,&portal_enable);	
	dbus_message_iter_next(&iter);
	dbus_message_iter_get_basic(&iter,&portal_transfer);
	if(state == 6){
	   vty_out(vty,"Batch-sync state\n");
	   vty_out(vty,"%-2s%-10s:%-s\n", "", "wireless",wid_transfer ? "TRANSFERING" : "RUN");
	   vty_out(vty,"%-2s%-10s:%-s\n", "", "portal", portal_enable ?  \
						(portal_transfer ? "TRANSFERING" : "RUN") : "NOT ELECTION");
	}	

	/* heartbeat interface */
	dbus_message_iter_next(&iter);
	dbus_message_iter_get_basic(&iter,&heartbeatlink_ip);
	dbus_message_iter_next(&iter);
	dbus_message_iter_get_basic(&iter,&heartbeat_link);
	dbus_message_iter_next(&iter);
	dbus_message_iter_get_basic(&iter,&heartbeat_flag);
	vty_out(vty, "-HEARTBEAT-\n");
	if (1 == heartbeat_flag)
	{
		dbus_message_iter_next(&iter);	
		dbus_message_iter_get_basic(&iter,&heartbeatlink_ifname);
		vty_out(vty, "%-7s%-s[%-s] ip ", \
						"", heartbeatlink_ifname ? heartbeatlink_ifname:"nil", heartbeat_link ? "U":"D");
		if(heartbeatlink_ip) {
			vty_out(vty,"%d.%d.%d.%d\n",((heartbeatlink_ip & 0xff000000) >> 24),((heartbeatlink_ip & 0xff0000) >> 16),	\
							((heartbeatlink_ip& 0xff00) >> 8),(heartbeatlink_ip & 0xff));
		}
		else {
			vty_out(vty, "0\n");		
		}
	}
	else {
		vty_out(vty, "%-7snot configured\n", "");
	}

	/* vgateway interface */
	dbus_message_iter_next(&iter);
	dbus_message_iter_get_basic(&iter,&vgateway_flag);
	vty_out(vty, "-VGATEWAY-\n");
	if(0 != vgateway_flag){
		dbus_message_iter_next(&iter);
		dbus_message_iter_get_basic(&iter, &vgateway_naddr);
		for(i = 0; i < vgateway_naddr; i++) {
			dbus_message_iter_next(&iter);
			dbus_message_iter_get_basic(&iter,&vgateway_ifname);
			dbus_message_iter_next(&iter);
			dbus_message_iter_get_basic(&iter,&ip);
			/*dbus_message_iter_next(&iter);
			dbus_message_iter_get_basic(&iter,&vgateway_mask);*/
			dbus_message_iter_next(&iter);
			dbus_message_iter_get_basic(&iter,&vgateway_ip);
			
			dbus_message_iter_next(&iter);
			dbus_message_iter_get_basic(&iter,&vgateway_link);		
			vty_out(vty, "%-7s%-3d%-s[%-s] ip real %d.%d.%d.%d virtual %d.%d.%d.%d\n", \
							"", i+1, vgateway_ifname ? vgateway_ifname : "nil", vgateway_link ? "U":"D", \
							((ip & 0xff000000) >> 24),((ip & 0xff0000) >> 16),((ip& 0xff00) >> 8),\
							(ip & 0xff),((vgateway_ip & 0xff000000) >> 24),\
							((vgateway_ip & 0xff0000) >> 16),\
							((vgateway_ip& 0xff00) >> 8),(vgateway_ip & 0xff));
			if(vgateway_mask) {
				vty_out(vty, "/%d", vgateway_mask);
			}
			vty_out(vty, "\n");
		}
	}
	else{
		vty_out(vty, "%-7snot configured\n","");
	}

	/* dhcp failover config */
	dbus_message_iter_next(&iter);
	dbus_message_iter_get_basic(&iter,&failover_peer);
	dbus_message_iter_next(&iter);
	dbus_message_iter_get_basic(&iter,&failover_local);
	vty_out(vty, "-DHCP FAILOVER-\n");
	if(~0UI != failover_peer) {
		struct dhcp_failover_state failover_state;
		memset(&failover_state, 0, sizeof(failover_state));
		if (DCLI_VRRP_RETURN_CODE_OK != dcli_show_dhcp_failover(vty, profile, &failover_state, slot_id)){
		    vty_out(vty, "cannot get failobver state\n");
		}
		vty_out(vty, "%-7speer [%s] %d.%d.%d.%d","",dcli_dhcp_failover_state_print(failover_state.peer), (failover_peer >> 24) & 0xFF, \
			(failover_peer >> 16) & 0xFF, (failover_peer >> 8) & 0xFF, failover_peer & 0xFF);
		if(~0UI != failover_local) {
			vty_out(vty, " local [%s] %d.%d.%d.%d", dcli_dhcp_failover_state_print(failover_state.local),(failover_local >> 24) & 0xFF, \
				(failover_local >> 16) & 0xFF, (failover_local >> 8) & 0xFF, failover_local & 0xFF);
		}
		vty_out(vty, "\n");
	}
	else {
		vty_out(vty, "%-7snot configured\n","");
	}
	dbus_message_unref(reply);
	return DCLI_VRRP_RETURN_CODE_OK;
}
#endif



#ifdef DISTRIBUT
int dcli_show_hansi_profile
(
   struct vty* vty,
   int profile,
   int slot_id
)
{
   	DBusMessage *query = NULL, *reply = NULL;
	DBusError err = {0};
	DBusMessageIter	 iter;
	unsigned int op_ret = 0;
	unsigned int advert = 0,preempt = 0,priority = 0,state = 0;
	char *uplink_ifname = NULL,*downlink_ifname =NULL,*heartbeatlink_ifname = NULL,*vgateway_ifname = NULL;
	char * l2_uplink_ifname = NULL;
	int uplink_ip = 0,downlink_ip = 0,heartbeatlink_ip = 0;
	int wid_transfer = 0,portal_enable = 0,portal_transfer = 0;
    int vgateway_ip = 0,vgateway_mask = 0;
	char vrrp_obj_path[DCLI_VRRP_OBJPATH_LEN] = {0};
	char vrrp_dbus_name[DCLI_VRRP_DBUSNAME_LEN] = {0};	
	int instRun = 0, i = 0;
	/* 0: not setted, 1: setted */
	unsigned int uplink_set_flg = 0, downlink_set_flg = 0, heartbeat_set_flg = 0, vgateway_set_flg;
	unsigned int failover_peer = 0, failover_local = 0;
	unsigned int l2_uplink_set_flg = 0;

	/* [1] check if had process has started before or not*/
	instRun =  dcli_hmd_hansi_is_running(vty,slot_id, 0,profile);
	if (INSTANCE_NO_CREATED == instRun) {
		/*vty_out(vty, "had instance %d not created!\n", profile);*/
		return DCLI_VRRP_RETURN_CODE_PROFILE_NOTEXIST;
	}
    DBusConnection *dcli_dbus_connection = NULL;
    ReInitDbusConnection(&dcli_dbus_connection,slot_id,distributFag);
	dcli_vrrp_splice_objpath_string(VRRP_DBUS_OBJPATH, profile, vrrp_obj_path);
	dcli_vrrp_splice_dbusname(VRRP_DBUS_BUSNAME, profile, vrrp_dbus_name);

	query = dbus_message_new_method_call(vrrp_dbus_name,
										 vrrp_obj_path,
										 VRRP_DBUS_INTERFACE,
										 VRRP_DBUS_METHOD_SHOW);
	dbus_error_init(&err);

	dbus_message_append_args(query,
		                     DBUS_TYPE_UINT32,&profile,				 
							 DBUS_TYPE_INVALID);
	
	reply = dbus_connection_send_with_reply_and_block (dcli_dbus_connection,query,-1, &err);
	dbus_message_unref(query);
	if (NULL == reply) {
		vty_out(vty,"failed get reply.\n");
		if (dbus_error_is_set(&err)) {
			printf("%s raised: %s",err.name,err.message);
			dbus_error_free_for_dcli(&err);
		}
		//dbus_message_unref(reply);		
		return DCLI_VRRP_RETURN_CODE_ERR;
	}
	dbus_message_iter_init(reply,&iter);
	dbus_message_iter_get_basic(&iter,&op_ret);
	if((DCLI_VRRP_RETURN_CODE_PROFILE_NOTEXIST == op_ret)||\
		(DCLI_VRRP_RETURN_CODE_NO_CONFIG == op_ret)){
	   dbus_message_unref(reply);	
	   return op_ret;
	}
	vty_out(vty,"------------HANSI %-2d------------\n",profile);
	vty_out(vty,"--STATE---PRI--ADVERT--PREEMPT--\n");
	dbus_message_iter_next(&iter);
	dbus_message_iter_get_basic(&iter,&state);
	vty_out(vty," %-9s",(state==3)?"MASTER":(state == 2)?"BACK" : (state == 99)?"DISABLE" : (state == 4)?"LEARNING" :(state == 6)? "TRANSFER" :"INIT");
	dbus_message_iter_next(&iter);
	dbus_message_iter_get_basic(&iter,&priority);
	vty_out(vty,"%-5d",priority);
	dbus_message_iter_next(&iter);
	dbus_message_iter_get_basic(&iter,&advert);
	vty_out(vty,"%-8d",advert);
	dbus_message_iter_next(&iter);
	dbus_message_iter_get_basic(&iter,&preempt);
	vty_out(vty,"  %-7s\n",preempt ? "Y" : "N");
	dbus_message_iter_next(&iter);
	dbus_message_iter_get_basic(&iter,&uplink_set_flg);
	vty_out(vty, "-UPLINK-\n");
	if (uplink_set_flg) {	
		for(i = 0; i < uplink_set_flg; i++) {
			vty_out(vty, "%-7s%-3d", "", i+1);
		dbus_message_iter_next(&iter);
		dbus_message_iter_get_basic(&iter,&uplink_ifname);
			vty_out(vty, "%-s ",uplink_ifname ? uplink_ifname : "nil");
		dbus_message_iter_next(&iter);
		dbus_message_iter_get_basic(&iter,&uplink_ip);
		if(0 == uplink_ip){
		        vty_out(vty,"0\n");
		}
		else {
				vty_out(vty,"%d.%d.%d.%d\n",((uplink_ip & 0xff000000) >> 24),((uplink_ip & 0xff0000) >> 16),	\
						((uplink_ip& 0xff00) >> 8),(uplink_ip & 0xff));
		}
		}
	} 
	else {
		vty_out(vty,"%-7snot configured\n","");
	}

	dbus_message_iter_next(&iter);
	dbus_message_iter_get_basic(&iter,&l2_uplink_set_flg);
	vty_out(vty, "-L2-UPLINK-\n");
	if (l2_uplink_set_flg) {	
		for(i = 0; i < l2_uplink_set_flg; i++) {
			vty_out(vty, "%-7s%-3d", "", i+1);
			dbus_message_iter_next(&iter);
			dbus_message_iter_get_basic(&iter,&l2_uplink_ifname);
			vty_out(vty, "%-s \n",l2_uplink_ifname ? l2_uplink_ifname : "nil");
		}
	} 
	else {
		vty_out(vty,"%-7snot configured\n","");
	}

	dbus_message_iter_next(&iter);
	dbus_message_iter_get_basic(&iter,&downlink_set_flg);
	vty_out(vty, "-DOWNLINK-\n");
	if (downlink_set_flg) {
		for(i = 0; i < downlink_set_flg; i++) {
			vty_out(vty, "%-7s%-3d", "", i+1);
		dbus_message_iter_next(&iter);
		dbus_message_iter_get_basic(&iter,&downlink_ifname);
			vty_out(vty,"%-s ",downlink_ifname ? downlink_ifname : "nil");
		dbus_message_iter_next(&iter);
		dbus_message_iter_get_basic(&iter,&downlink_ip);
			if(0 == downlink_ip) {
				vty_out(vty, "0\n");
			}
			else {
				vty_out(vty,"%d.%d.%d.%d\n",((downlink_ip & 0xff000000) >> 24),((downlink_ip & 0xff0000) >> 16),	\
						((downlink_ip& 0xff00) >> 8),(downlink_ip & 0xff));
			}
		}
	}
	else {
		vty_out(vty,"%-7snot configured\n","");
	}

	dbus_message_iter_next(&iter);
	dbus_message_iter_get_basic(&iter,&wid_transfer);
	dbus_message_iter_next(&iter);
	dbus_message_iter_get_basic(&iter,&portal_enable);	
	dbus_message_iter_next(&iter);
	dbus_message_iter_get_basic(&iter,&portal_transfer);
    if(state == 6){
       vty_out(vty,"WID STATE %s\n",wid_transfer ? "transfering" : "run");
	   if(portal_enable){
          vty_out(vty,"PORTAL STATE %s\n",portal_transfer ? "transfering" : "run");
	   }
	}		
	dbus_message_iter_next(&iter);
	dbus_message_iter_get_basic(&iter,&heartbeat_set_flg);
	vty_out(vty, "-HEARTBEAT-\n");
	if (1 == heartbeat_set_flg) {
		dbus_message_iter_next(&iter);
		dbus_message_iter_get_basic(&iter,&heartbeatlink_ip);
		dbus_message_iter_next(&iter);
		dbus_message_iter_get_basic(&iter,&heartbeatlink_ifname);
		if(0 != heartbeatlink_ip){
			vty_out(vty,"%-7s%-s %d.%d.%d.%d\n", "",
					heartbeatlink_ifname ? heartbeatlink_ifname :"nil",
					((heartbeatlink_ip & 0xff000000) >> 24),
					((heartbeatlink_ip & 0xff0000) >> 16),
					((heartbeatlink_ip& 0xff00) >> 8),
					heartbeatlink_ip & 0xff);
		}
	}
	else {
		vty_out(vty, "%-7snot configured\n","");
	}
	
	dbus_message_iter_next(&iter);
	dbus_message_iter_get_basic(&iter,&vgateway_set_flg);
	vty_out(vty, "-VGATEWAY-\n");
	if(0 != vgateway_set_flg){
		for(i = 0; i < vgateway_set_flg; i++) {
		dbus_message_iter_next(&iter);
		dbus_message_iter_get_basic(&iter,&vgateway_ifname);
		dbus_message_iter_next(&iter);
		dbus_message_iter_get_basic(&iter,&vgateway_ip);
		dbus_message_iter_next(&iter);
		dbus_message_iter_get_basic(&iter,&vgateway_mask);
			if(0 == vgateway_ip) {
				vty_out(vty, "%-7s%-3d%-s 0", "", i+1, vgateway_ifname);
			}
			else {
				vty_out(vty, "%-7s%-3d%-s %d.%d.%d.%d",  "", i+1, vgateway_ifname ? vgateway_ifname :"nil", \
							((vgateway_ip & 0xff000000) >> 24),((vgateway_ip & 0xff0000) >> 16), \
							((vgateway_ip & 0xff00) >> 8),vgateway_ip & 0xff);
				if(vgateway_mask) {
					vty_out(vty, "/%d",vgateway_mask);
				}
				vty_out(vty, "\n");
			}
		}
	}
	else {
		vty_out(vty, "%-7snot configured\n", "");
	}

	dbus_message_iter_next(&iter);
	dbus_message_iter_get_basic(&iter,&failover_peer);
	dbus_message_iter_next(&iter);
	dbus_message_iter_get_basic(&iter,&failover_local);
	vty_out(vty, "-DHCP FAILOVER-\n");
	if(~0UI != failover_peer) {
		struct dhcp_failover_state failover_state;
		memset(&failover_state, 0, sizeof(failover_state));
		if (DCLI_VRRP_RETURN_CODE_OK != dcli_show_dhcp_failover(vty, profile, &failover_state, slot_id)){
		vty_out(vty, "cannot get failobver state\n");
		}
		vty_out(vty, "%-7speer [%s] %d.%d.%d.%d", "",dcli_dhcp_failover_state_print(failover_state.peer), (failover_peer >> 24) & 0xFF, \
			(failover_peer >> 16) & 0xFF, (failover_peer >> 8) & 0xFF, failover_peer & 0xFF);
		if(~0UI != failover_local) {
			vty_out(vty, " local [%s] %d.%d.%d.%d", dcli_dhcp_failover_state_print(failover_state.local),(failover_local >> 24) & 0xFF, \
				(failover_local >> 16) & 0xFF, (failover_local >> 8) & 0xFF, failover_local & 0xFF);
		}
		vty_out(vty, "\n");
	}
	else {
		vty_out(vty, "%-7snot configured\n", "");
	}
	dbus_message_unref(reply);
	return DCLI_VRRP_RETURN_CODE_OK;
}
#else
int dcli_show_hansi_profile
(
   struct vty* vty,
   int profile
)
{
   	DBusMessage *query = NULL, *reply = NULL;
	DBusError err = {0};
	DBusMessageIter	 iter;
	unsigned int op_ret = 0;
	unsigned int advert = 0,preempt = 0,priority = 0,state = 0;
	char *uplink_ifname = NULL,*downlink_ifname =NULL,*heartbeatlink_ifname = NULL,*vgateway_ifname = NULL;
	char * l2_uplink_ifname = NULL;
	int uplink_ip = 0,downlink_ip = 0,heartbeatlink_ip = 0;
	int wid_transfer = 0,portal_enable = 0,portal_transfer = 0;
    int vgateway_ip = 0,vgateway_mask = 0;
	char vrrp_obj_path[DCLI_VRRP_OBJPATH_LEN] = {0};
	char vrrp_dbus_name[DCLI_VRRP_DBUSNAME_LEN] = {0};	
	int instRun = 0, i = 0;
	/* 0: not setted, 1: setted */
	unsigned int uplink_set_flg = 0, downlink_set_flg = 0, heartbeat_set_flg = 0, vgateway_set_flg;
	unsigned int failover_peer = 0, failover_local = 0;
	unsigned int l2_uplink_set_flg = 0;

	/* [1] check if had process has started before or not */
	instRun = dcli_vrrp_hansi_is_running(vty, profile);
	if (DCLI_VRRP_INSTANCE_NO_CREATED == instRun) {
		//vty_out(vty, "had instance %d not created!\n", profile);
		return DCLI_VRRP_RETURN_CODE_PROCESS_NOTEXIST;
	}
	else if (DCLI_VRRP_INSTANCE_CHECK_FAILED == instRun) {
		//vty_out(vty, "check had instance %d whether created was failed!\n",
		//			profile);
		return DCLI_VRRP_RETURN_CODE_ERR;
	} 
#ifdef DISTRIBUT
	DBusConnection *dcli_dbus_connection = NULL;
	ReInitDbusConnection(&dcli_dbus_connection,slot_id,distributFag);
#endif	
	dcli_vrrp_splice_objpath_string(VRRP_DBUS_OBJPATH, profile, vrrp_obj_path);
	dcli_vrrp_splice_dbusname(VRRP_DBUS_BUSNAME, profile, vrrp_dbus_name);

	query = dbus_message_new_method_call(vrrp_dbus_name,
										 vrrp_obj_path,
										 VRRP_DBUS_INTERFACE,
										 VRRP_DBUS_METHOD_SHOW);
	dbus_error_init(&err);

	dbus_message_append_args(query,
		                     DBUS_TYPE_UINT32,&profile,				 
							 DBUS_TYPE_INVALID);
	
	reply = dbus_connection_send_with_reply_and_block (dcli_dbus_connection,query,-1, &err);
	dbus_message_unref(query);
	if (NULL == reply) {
		vty_out(vty,"failed get reply.\n");
		if (dbus_error_is_set(&err)) {
			printf("%s raised: %s",err.name,err.message);
			dbus_error_free_for_dcli(&err);
		}
		//dbus_message_unref(reply);		
		return DCLI_VRRP_RETURN_CODE_ERR;
	}
	dbus_message_iter_init(reply,&iter);
	dbus_message_iter_get_basic(&iter,&op_ret);
	if((DCLI_VRRP_RETURN_CODE_PROFILE_NOTEXIST == op_ret)||\
		(DCLI_VRRP_RETURN_CODE_NO_CONFIG == op_ret)){
	   dbus_message_unref(reply);	
	   return op_ret;
	}
	vty_out(vty,"------------HANSI %-2d------------\n",profile);
	vty_out(vty,"--STATE---PRI--ADVERT--PREEMPT--\n");
	dbus_message_iter_next(&iter);
	dbus_message_iter_get_basic(&iter,&state);
	vty_out(vty," %-9s",(state==3)?"MASTER":(state == 2)?"BACK" : (state == 99)?"DISABLE" : (state == 4)?"LEARNING" :(state == 6)? "TRANSFER" :"INIT");
	dbus_message_iter_next(&iter);
	dbus_message_iter_get_basic(&iter,&priority);
	vty_out(vty,"%-5d",priority);
	dbus_message_iter_next(&iter);
	dbus_message_iter_get_basic(&iter,&advert);
	vty_out(vty,"%-8d",advert);
	dbus_message_iter_next(&iter);
	dbus_message_iter_get_basic(&iter,&preempt);
	vty_out(vty,"  %-7s\n",preempt ? "Y" : "N");
	dbus_message_iter_next(&iter);
	dbus_message_iter_get_basic(&iter,&uplink_set_flg);
	vty_out(vty, "-UPLINK-\n");
	if (uplink_set_flg) {	
		for(i = 0; i < uplink_set_flg; i++) {
			vty_out(vty, "%-7s%-3d", "", i+1);
		dbus_message_iter_next(&iter);
		dbus_message_iter_get_basic(&iter,&uplink_ifname);
			vty_out(vty, "%-s ",uplink_ifname ? uplink_ifname : "nil");
		dbus_message_iter_next(&iter);
		dbus_message_iter_get_basic(&iter,&uplink_ip);
		if(0 == uplink_ip){
		        vty_out(vty,"0\n");
		}
		else {
				vty_out(vty,"%d.%d.%d.%d\n",((uplink_ip & 0xff000000) >> 24),((uplink_ip & 0xff0000) >> 16),	\
						((uplink_ip& 0xff00) >> 8),(uplink_ip & 0xff));
		}
		}
	} 
	else {
		vty_out(vty,"%-7snot configured\n","");
	}

	dbus_message_iter_next(&iter);
	dbus_message_iter_get_basic(&iter,&l2_uplink_set_flg);
	vty_out(vty, "-L2-UPLINK-\n");
	if (l2_uplink_set_flg) {	
		for(i = 0; i < l2_uplink_set_flg; i++) {
			vty_out(vty, "%-7s%-3d", "", i+1);
			dbus_message_iter_next(&iter);
			dbus_message_iter_get_basic(&iter,&l2_uplink_ifname);
			vty_out(vty, "%-s \n",l2_uplink_ifname ? l2_uplink_ifname : "nil");
		}
	} 
	else {
		vty_out(vty,"%-7snot configured\n","");
	}

	dbus_message_iter_next(&iter);
	dbus_message_iter_get_basic(&iter,&downlink_set_flg);
	vty_out(vty, "-DOWNLINK-\n");
	if (downlink_set_flg) {
		for(i = 0; i < downlink_set_flg; i++) {
			vty_out(vty, "%-7s%-3d", "", i+1);
		dbus_message_iter_next(&iter);
		dbus_message_iter_get_basic(&iter,&downlink_ifname);
			vty_out(vty,"%-s ",downlink_ifname ? downlink_ifname : "nil");
		dbus_message_iter_next(&iter);
		dbus_message_iter_get_basic(&iter,&downlink_ip);
			if(0 == downlink_ip) {
				vty_out(vty, "0\n");
			}
			else {
				vty_out(vty,"%d.%d.%d.%d\n",((downlink_ip & 0xff000000) >> 24),((downlink_ip & 0xff0000) >> 16),	\
						((downlink_ip& 0xff00) >> 8),(downlink_ip & 0xff));
			}
		}
	}
	else {
		vty_out(vty,"%-7snot configured\n","");
	}

	dbus_message_iter_next(&iter);
	dbus_message_iter_get_basic(&iter,&wid_transfer);
	dbus_message_iter_next(&iter);
	dbus_message_iter_get_basic(&iter,&portal_enable);	
	dbus_message_iter_next(&iter);
	dbus_message_iter_get_basic(&iter,&portal_transfer);
    if(state == 6){
       vty_out(vty,"WID STATE %s\n",wid_transfer ? "transfering" : "run");
	   if(portal_enable){
          vty_out(vty,"PORTAL STATE %s\n",portal_transfer ? "transfering" : "run");
	   }
	}		
	dbus_message_iter_next(&iter);
	dbus_message_iter_get_basic(&iter,&heartbeat_set_flg);
	vty_out(vty, "-HEARTBEAT-\n");
	if (1 == heartbeat_set_flg) {
		dbus_message_iter_next(&iter);
		dbus_message_iter_get_basic(&iter,&heartbeatlink_ip);
		dbus_message_iter_next(&iter);
		dbus_message_iter_get_basic(&iter,&heartbeatlink_ifname);
		if(0 != heartbeatlink_ip){
			vty_out(vty,"%-7s%-s %d.%d.%d.%d\n", "",
					heartbeatlink_ifname ? heartbeatlink_ifname :"nil",
					((heartbeatlink_ip & 0xff000000) >> 24),
					((heartbeatlink_ip & 0xff0000) >> 16),
					((heartbeatlink_ip& 0xff00) >> 8),
					heartbeatlink_ip & 0xff);
		}
	}
	else {
		vty_out(vty, "%-7snot configured\n","");
	}
	
	dbus_message_iter_next(&iter);
	dbus_message_iter_get_basic(&iter,&vgateway_set_flg);
	vty_out(vty, "-VGATEWAY-\n");
	if(0 != vgateway_set_flg){
		for(i = 0; i < vgateway_set_flg; i++) {
		dbus_message_iter_next(&iter);
		dbus_message_iter_get_basic(&iter,&vgateway_ifname);
		dbus_message_iter_next(&iter);
		dbus_message_iter_get_basic(&iter,&vgateway_ip);
		dbus_message_iter_next(&iter);
		dbus_message_iter_get_basic(&iter,&vgateway_mask);
			if(0 == vgateway_ip) {
				vty_out(vty, "%-7s%-3d%-s 0", "", i+1, vgateway_ifname);
			}
			else {
				vty_out(vty, "%-7s%-3d%-s %d.%d.%d.%d",  "", i+1, vgateway_ifname ? vgateway_ifname :"nil", \
							((vgateway_ip & 0xff000000) >> 24),((vgateway_ip & 0xff0000) >> 16), \
							((vgateway_ip & 0xff00) >> 8),vgateway_ip & 0xff);
				if(vgateway_mask) {
					vty_out(vty, "/%d",vgateway_mask);
				}
				vty_out(vty, "\n");
			}
		}
	}
	else {
		vty_out(vty, "%-7snot configured\n", "");
	}

	dbus_message_iter_next(&iter);
	dbus_message_iter_get_basic(&iter,&failover_peer);
	dbus_message_iter_next(&iter);
	dbus_message_iter_get_basic(&iter,&failover_local);
	vty_out(vty, "-DHCP FAILOVER-\n");
	if(~0UI != failover_peer) {
		struct dhcp_failover_state failover_state;
		memset(&failover_state, 0, sizeof(failover_state));
		if (DCLI_VRRP_RETURN_CODE_OK != dcli_show_dhcp_failover(vty, profile, &failover_state, slot_id)){
		vty_out(vty, "cannot get failobver state\n");
		}
		vty_out(vty, "%-7speer [%s] %d.%d.%d.%d", "",dcli_dhcp_failover_state_print(failover_state.peer), (failover_peer >> 24) & 0xFF, \
			(failover_peer >> 16) & 0xFF, (failover_peer >> 8) & 0xFF, failover_peer & 0xFF);
		if(~0UI != failover_local) {
			vty_out(vty, " local [%s] %d.%d.%d.%d", dcli_dhcp_failover_state_print(failover_state.local),(failover_local >> 24) & 0xFF, \
				(failover_local >> 16) & 0xFF, (failover_local >> 8) & 0xFF, failover_local & 0xFF);
		}
		vty_out(vty, "\n");
	}
	else {
		vty_out(vty, "%-7snot configured\n", "");
	}
	dbus_message_unref(reply);
	return DCLI_VRRP_RETURN_CODE_OK;
}
#endif


int dcli_vrrp_check_ip_format
(
   char* buf,
   int* split_count
)
{
    char *split = "/";
	char *str = NULL;
	int length = 0,i = 0,splitCount = 0;
	if(NULL == buf){
        return CMD_WARNING;
	}	
	str = buf;
	length = strlen(str);
	if( length > DCLI_IPMASK_STRING_MAXLEN ||  \
		length < DCLI_IP_STRING_MINLEN ){
		return CMD_WARNING;
	}
	if((str[0] > '9')||(str[0] < '1')){
		return CMD_WARNING;
	}
	for(i = 0; i < length; i++){
		if('/' == str[i]){
            splitCount++;
			if((i == length - 1)||('0' > str[i+1])||(str[i+1] > '9')){
                return CMD_WARNING;
			}
		}
		if((str[i] > '9'||str[i]<'0') &&  \
			str[i] != '.' &&  \
			str[i] != '/' &&  \
			str[i] != '\0'
		){
			return CMD_WARNING;
		}
	}  
    *split_count = splitCount;
	return CMD_SUCCESS;
}


/*
 *******************************************************************************
 *dcli_vrrp_splice_objpath_string()
 *
 *  DESCRIPTION:
 *		use of common path and vrrp instance profile,
 *		splicing obj-path string of special vrrp instance.
 *  INPUTS:
 * 	 	char *common_objpath,	- vrrp obj common path
 *		unsigned int profile,	- profile of vrrp instance
 *
 *  OUTPUTS:
 * 	 	char *splice_objpath	- special obj-path string 
 *
 *  RETURN VALUE:
 *		void
 *
 *******************************************************************************
 */
void dcli_vrrp_splice_objpath_string
(
	char *common_objpath,
	unsigned int profile,
	char *splice_objpath
)
{
	sprintf(splice_objpath,
			"%s%d",
			common_objpath, profile);
	return;
}

/*
 *******************************************************************************
 *dcli_vrrp_splice_dbusname()
 *
 *  DESCRIPTION:
 *		use of common dbus name and vrrp instance profile,
 *		splicing dbus name of special vrrp instance.
 *  INPUTS:
 * 	 	char *common_dbusname,	- vrrp common dbus name
 *		unsigned int profile,	- profile of vrrp instance
 *
 *  OUTPUTS:
 * 	 	char *splice_dbusname	- special dbus name 
 *
 *  RETURN VALUE:
 *		void
 *
 *******************************************************************************
 */
void dcli_vrrp_splice_dbusname
(
	char *common_dbusname,
	unsigned int profile,
	char *splice_dbusname
)
{
	sprintf(splice_dbusname,
			"%s%d",
			common_dbusname, profile);
	return;
}

/**
 * check if had process has started before or not
 * now we use 'ps -ef | grep had #' format to indicate 
 * the corresponding process is running or not
 */
int dcli_vrrp_hansi_is_running
(
	struct vty* vty,
	unsigned int profileId
)
{
	int isRunning = 0, ret = 0, fd = -1, iCnt = 0;	
	char commandBuf[DCLI_VRRP_SYS_COMMAND_LEN] = {0}, readBuf[4] = {0};
	FILE *pidmap_file = NULL, *hadpid_file = NULL, *pathpid_file = NULL;
	char msg[DCLI_VRRP_SYS_COMMAND_LEN] = {0}, str_pid[DCLI_VRRP_SYS_COMMAND_LEN] = {0}, pathpid[DCLI_VRRP_SYS_COMMAND_LEN] = {0};
	char *sep=" ", *token = NULL;
	int hadpid = 0;

	sprintf(commandBuf, "/var/run/had%d.pid", profileId);
	hadpid_file = fopen(commandBuf, "r");
	if (!hadpid_file) {
		return DCLI_VRRP_INSTANCE_NO_CREATED;
	}
	fclose(hadpid_file);
	
	memset(commandBuf, 0, DCLI_VRRP_SYS_COMMAND_LEN);
	sprintf(commandBuf, "/var/run/had%d.pidmap", profileId);
	pidmap_file = fopen(commandBuf, "r");
	if (!pidmap_file) {
		return DCLI_VRRP_INSTANCE_NO_CREATED;
	}
	while (fgets(msg, DCLI_VRRP_SYS_COMMAND_LEN, pidmap_file)) {
		token = strtok(msg, sep);
		while (token) {
			strcpy(str_pid, token);
			token = strtok(NULL, sep);
		}
		hadpid = strtoul(str_pid, NULL, 10);
		sprintf(pathpid, "/proc/%d", hadpid);
		pathpid_file = fopen(pathpid, "r");
		if (!pathpid_file) {
			fclose(pidmap_file);
			return DCLI_VRRP_INSTANCE_NO_CREATED;
		}
		fclose(pathpid_file);
		memset(msg, 0, DCLI_VRRP_SYS_COMMAND_LEN);
		memset(pathpid, 0, DCLI_VRRP_SYS_COMMAND_LEN);
	}
	
	fclose(pidmap_file);
	
	if (!hadpid) {
		return DCLI_VRRP_INSTANCE_NO_CREATED;
	}
#if 0
	else {
		/* check if hansi is running or not 
		 * with following shell command:
		 *	'ps -ef | grep had # | wc -l'
		 *	if count result gt 1, running, else not running.
		 */
		memset(commandBuf, 0, DCLI_VRRP_SYS_COMMAND_LEN);
		sprintf(commandBuf, "sudo ps auxww | grep \"had %d$\" | wc -l > /var/run/had%d.num", profileId, profileId);
		ret = system(commandBuf);
		if(ret) {
			vty_out(vty, "%% Check hansi instance %d failed!\n", profileId);
			return DCLI_VRRP_INSTANCE_CHECK_FAILED;
		}

		/* get the process # */
		memset(commandBuf, 0, DCLI_VRRP_SYS_COMMAND_LEN);
		sprintf(commandBuf,"/var/run/had%d.num", profileId);
		if((fd = open(commandBuf, O_RDONLY))< 0) {
			vty_out(vty, "%% Check hansi instance %d count failed!\n", profileId);
			return DCLI_VRRP_INSTANCE_CHECK_FAILED;
		}

		memset(commandBuf, 0, DCLI_VRRP_SYS_COMMAND_LEN);
		read(fd, commandBuf, 4);
		iCnt = strtoul(commandBuf, NULL, 10);
		//vty_out(vty, "had %d thread count %d\n", profileId, iCnt);

		/*
		 * why number 2?
		 *	one is 'sh -c ps -ef | grep "had #"'
		 *  the other is 'grep had #'
		 */
		if(iCnt > 2) {
			ret = DCLI_VRRP_INSTANCE_CREATED;
		}
		else {
			ret = DCLI_VRRP_INSTANCE_NO_CREATED;
		}

		/* release file resources */
		close(fd);

		memset(commandBuf, 0, DCLI_VRRP_SYS_COMMAND_LEN);
		sprintf(commandBuf, "sudo rm /var/run/had%d.num", profileId);
		system(commandBuf);
	}
#endif
	return DCLI_VRRP_INSTANCE_CREATED;
}

/**
 * check if had/wid/asd/wsm process has started completely.
 * now we use 'ps -ef | grep \"%s %d$\"' format to indicate 
 * the corresponding process is running or not
 */
int dcli_vrrp_check_service_started
(
	struct vty* vty,
	unsigned char *service_name,
	unsigned int profileId
)
{
	int ret = 0;
	int fd = -1;
	int iCnt = 0;	
	char commandBuf[DCLI_VRRP_SYS_COMMAND_LEN] = {0};
	char readBuf[4] = {0};

	if (!vty || !service_name) {
		return DCLI_VRRP_INSTANCE_CHECK_FAILED;
	}

	/* check if hansi is running or not 
	 * with following shell command:
	 *	'ps -ef | grep \"%s %d$\" | wc -l'
	 *	if count result gt 1, running, else not running.
	 */
	sprintf(commandBuf, "sudo ps auxww | grep \"%s %d$\" | wc -l > /var/run/%s%d.boot",
						service_name, profileId, service_name, profileId);
	ret = system(commandBuf);
	if (ret) {
		vty_out(vty, "%% Check %s instance %d failed!\n", service_name, profileId);
		return DCLI_VRRP_INSTANCE_CHECK_FAILED;
	}

	/* get the process # */
	memset(commandBuf, 0, DCLI_VRRP_SYS_COMMAND_LEN);
	sprintf(commandBuf, "/var/run/%s%d.boot", service_name, profileId);
	if ((fd = open(commandBuf, O_RDONLY))< 0) {
		vty_out(vty, "%% Check %s instance %d count failed!\n", service_name, profileId);
		return DCLI_VRRP_INSTANCE_CHECK_FAILED;
	}

	memset(readBuf, 0, 4);
	read(fd, readBuf, 4);
	iCnt = strtoul(readBuf, NULL, 10);
	//vty_out(vty, "%s %d thread count %d\n", service_name, profileId, iCnt);

	if (!strncmp(service_name, "had", 3)) {
		if (DCLI_VRRP_THREADS_CNT == iCnt) {
			ret = DCLI_VRRP_INSTANCE_CREATED;
		}
		else {
			ret = DCLI_VRRP_INSTANCE_NO_CREATED;
		}
	}else {
		/* for wcpss, include wid/asd/wsm process */
		if (3 <= iCnt) {
			ret = DCLI_VRRP_INSTANCE_CREATED;
		}
		else {
			ret = DCLI_VRRP_INSTANCE_NO_CREATED;
		}
	}	
	
	/* release file resources */
	close(fd);

	memset(commandBuf, 0, DCLI_VRRP_SYS_COMMAND_LEN);
	sprintf(commandBuf, "sudo rm /var/run/%s%d.boot", service_name, profileId);
	system(commandBuf);

	return ret;
}


int dcli_vrrp_check_portal_service_started
(
	struct vty* vty,
	char *service_name,
	unsigned int profileId,
	int is_local
)
{
	int ret = 0;
	int fd = -1;
	int iCnt = 0;	
	char commandBuf[DCLI_VRRP_SYS_COMMAND_LEN] = {0};
	char readBuf[4] = {0};

	if (!vty || !service_name) {
		return DCLI_VRRP_INSTANCE_CHECK_FAILED;
	}

	/* check if hansi is running or not 
	 * with following shell command:
	 *	'ps -ef | grep \"%s %d$\" | wc -l'
	 *	if count result gt 1, running, else not running.
	 */
	sprintf(commandBuf, "sudo ps auxww | grep \"%s %d %d$\" | grep -v grep | wc -l > /var/run/%s%d_%d.boot",
						service_name, is_local, profileId, service_name,is_local, profileId);
	ret = system(commandBuf);
	if (ret) {
		vty_out(vty, "%% Check %s instance %d failed!\n", service_name, profileId);
		return DCLI_VRRP_INSTANCE_CHECK_FAILED;
	}

	/* get the process # */
	memset(commandBuf, 0, DCLI_VRRP_SYS_COMMAND_LEN);
	sprintf(commandBuf, "/var/run/%s%d_%d.boot", service_name, is_local, profileId);
	if ((fd = open(commandBuf, O_RDONLY))< 0) {
		vty_out(vty, "%% Check %s instance %d count failed!\n", service_name, profileId);
		return DCLI_VRRP_INSTANCE_CHECK_FAILED;
	}

	memset(readBuf, 0, 4);
	read(fd, readBuf, 4);
	iCnt = strtoul(readBuf, NULL, 10);
	//vty_out(vty, "%s %d thread count %d\n", service_name, profileId, iCnt);

	if (!strncmp(service_name, "had", 3)) {
		if (DCLI_VRRP_THREADS_CNT == iCnt) {
			ret = DCLI_VRRP_INSTANCE_CREATED;
		}
		else {
			ret = DCLI_VRRP_INSTANCE_NO_CREATED;
		}
	}else {
		/* for wcpss, include wid/asd/wsm process */
		if (1 <= iCnt) {
			ret = DCLI_VRRP_INSTANCE_CREATED;
		}
		else {
			ret = DCLI_VRRP_INSTANCE_NO_CREATED;
		}
	}	
	
	/* release file resources */
	close(fd);

	memset(commandBuf, 0, DCLI_VRRP_SYS_COMMAND_LEN);
	sprintf(commandBuf, "sudo rm /var/run/%s%d_%d.boot", service_name,is_local, profileId);
	system(commandBuf);

	return ret;
}


/*
 *******************************************************************************
 *dcli_vrrp_config_service_disable()
 *
 *  DESCRIPTION:
 *		disable special had instance service.
 *  INPUTS:
 *		struct vty* vty,
 *		unsigned int profile
 *
 *  OUTPUTS:
 *		NULL
 *
 *  RETURN VALUE:
 *		0		- success
 *		1		- fail
 *
 *******************************************************************************
 */
#ifdef DISTRIBUT
unsigned int dcli_vrrp_config_service_disable
(
	struct vty* vty,
	unsigned int profile,
	unsigned int slot_id
)
{
	DBusMessage *query = NULL;
	DBusMessage *reply = NULL;
	DBusError err = {0};
	unsigned int op_ret = DCLI_VRRP_RETURN_CODE_OK;
	unsigned int enable = 0;	/* 0: disable */
	char vrrp_obj_path[DCLI_VRRP_OBJPATH_LEN] = {0};
	char vrrp_dbus_name[DCLI_VRRP_DBUSNAME_LEN] = {0};

	if (!vty) {
		return 1;
	}
    DBusConnection *dcli_dbus_connection = NULL;
    ReInitDbusConnection(&dcli_dbus_connection,slot_id,distributFag);
	dcli_vrrp_splice_objpath_string(VRRP_DBUS_OBJPATH, profile, vrrp_obj_path);
	dcli_vrrp_splice_dbusname(VRRP_DBUS_BUSNAME, profile, vrrp_dbus_name);

	query = dbus_message_new_method_call(vrrp_dbus_name,
										vrrp_obj_path,
										VRRP_DBUS_INTERFACE,
										VRRP_DBUS_METHOD_VRRP_SERVICE_ENABLE);
	dbus_error_init(&err);

	dbus_message_append_args(query,
							DBUS_TYPE_UINT32, &profile,
							DBUS_TYPE_UINT32, &enable,
							DBUS_TYPE_INVALID);

	reply = dbus_connection_send_with_reply_and_block(dcli_dbus_connection, query, -1, &err);
	dbus_message_unref(query);
	if (NULL == reply) {
		vty_out(vty, "failed get reply.\n");
		if (dbus_error_is_set(&err)) {
			printf("%s raised: %s", err.name, err.message);
			dbus_error_free_for_dcli(&err);
		}
		return CMD_SUCCESS;
	}
	else if (dbus_message_get_args ( reply, &err,
									DBUS_TYPE_UINT32,&op_ret,
									DBUS_TYPE_INVALID))
	{
		if( DCLI_VRRP_RETURN_CODE_OK != op_ret){
			vty_out(vty,dcli_vrrp_err_msg[op_ret - DCLI_VRRP_RETURN_CODE_OK]);
			/* return 0 for delete hansi instance when service not enable. */
			return 0;
		}
	}else { 	
		if (dbus_error_is_set(&err)) {
			printf("%s raised: %s", err.name, err.message);
			dbus_error_free_for_dcli(&err);
		}
	}
	
	dbus_message_unref(reply);
	
	return 0;
}
#else
unsigned int dcli_vrrp_config_service_disable
(
	struct vty* vty,
	unsigned int profile
)
{
	DBusMessage *query = NULL;
	DBusMessage *reply = NULL;
	DBusError err = {0};
	unsigned int op_ret = DCLI_VRRP_RETURN_CODE_OK;
	unsigned int enable = 0;	/* 0: disable */
	char vrrp_obj_path[DCLI_VRRP_OBJPATH_LEN] = {0};
	char vrrp_dbus_name[DCLI_VRRP_DBUSNAME_LEN] = {0};

	if (!vty) {
		return 1;
	}
#ifdef DISTRIBUT	
	DBusConnection *dcli_dbus_connection = NULL;
	ReInitDbusConnection(&dcli_dbus_connection,slot_id,distributFag);
#endif	
	dcli_vrrp_splice_objpath_string(VRRP_DBUS_OBJPATH, profile, vrrp_obj_path);
	dcli_vrrp_splice_dbusname(VRRP_DBUS_BUSNAME, profile, vrrp_dbus_name);

	query = dbus_message_new_method_call(vrrp_dbus_name,
										vrrp_obj_path,
										VRRP_DBUS_INTERFACE,
										VRRP_DBUS_METHOD_VRRP_SERVICE_ENABLE);
	dbus_error_init(&err);

	dbus_message_append_args(query,
							DBUS_TYPE_UINT32, &profile,
							DBUS_TYPE_UINT32, &enable,
							DBUS_TYPE_INVALID);

	reply = dbus_connection_send_with_reply_and_block(dcli_dbus_connection, query, -1, &err);
	dbus_message_unref(query);
	if (NULL == reply) {
		vty_out(vty, "failed get reply.\n");
		if (dbus_error_is_set(&err)) {
			printf("%s raised: %s", err.name, err.message);
			dbus_error_free_for_dcli(&err);
		}
		return CMD_SUCCESS;
	}
	else if (dbus_message_get_args ( reply, &err,
									DBUS_TYPE_UINT32,&op_ret,
									DBUS_TYPE_INVALID))
	{
		if( DCLI_VRRP_RETURN_CODE_OK != op_ret){
			vty_out(vty,dcli_vrrp_err_msg[op_ret - DCLI_VRRP_RETURN_CODE_OK]);
			/* return 0 for delete hansi instance when service not enable. */
			return 0;
		}
	}else {		
		if (dbus_error_is_set(&err)) {
			printf("%s raised: %s", err.name, err.message);
			dbus_error_free_for_dcli(&err);
		}
	}
	
	dbus_message_unref(reply);
	
	return 0;
}

#endif
extern int boot_flag;
#ifndef DISTRIBUT
DEFUN(config_hansi_cmd_func,
	  config_hansi_cmd,
	  "config hansi-profile <0-16>",
	  CONFIG_STR
	  HANSI_STR
	  "Profile ID for configuration"
)

{
	DBusMessage *query = NULL, *reply = NULL;
	DBusError err = {0};
	unsigned int op_ret = 0;
	unsigned int profile = 0;
	int instRun = 0;
	char vrrp_obj_path[DCLI_VRRP_OBJPATH_LEN] = {0};
	char vrrp_dbus_name[DCLI_VRRP_DBUSNAME_LEN] = {0};
	char cmd[DCLI_VRRP_DBUSNAME_LEN] = {0};
	int cr_timeout = 0;
	int check_result = DCLI_VRRP_INSTANCE_CREATED;
	unsigned char service_name[4][4] = {"had","wid", "asd", "wsm"};
	unsigned int service_index = 0;
	
    if(argc == 1){
		profile = strtoul((char *)argv[0],NULL,0);
		if((profile < 0)||profile > 16){
	        vty_out(vty,"%% Bad parameter : %s !",argv[0]);
			return CMD_WARNING;
		}
    }

	/* system to fork had process */
	instRun = dcli_vrrp_hansi_is_running(vty, profile);
	if(DCLI_VRRP_INSTANCE_NO_CREATED == instRun) {
		/* user verification */
		if (!boot_flag)	{
			printf("Are you sure you want to start new hansi(yes/no)?");
			fscanf(stdin, "%s", cmd);
			if (!strncasecmp("no", cmd, strlen(cmd))){
				printf("\n%% User cancelled command.\n");
				return CMD_WARNING;
			}
			if (strncasecmp("yes", cmd, strlen(cmd))){
				printf("\nPlease type 'yes' or 'no':");	
				fscanf(stdin, "%s", cmd);
				if (!strncasecmp("no", cmd, strlen(cmd))){
					printf("\n%% User cancelled command.\n");
					return CMD_WARNING;
				}
				else if (strncasecmp("yes", cmd, strlen(cmd))){
					printf("\n%% Bad choice given.\n");
					return CMD_WARNING;
				}
			}
			printf("\n");
		}
		
		sprintf(cmd, "sudo /etc/init.d/had start %d &", profile);
		if (system(cmd)) {
			vty_out(vty, "create hansi %d faild.\n", profile);
			return CMD_WARNING;
		}

		/* check wheather had instance started completely. */
		while (1) {
			cr_timeout++;
			check_result = dcli_vrrp_check_service_started(vty, service_name[0], profile);
			if (DCLI_VRRP_INSTANCE_NO_CREATED == check_result) {
				//vty_out(vty, "create %s instrance %d time %d s.\n", service_name[0], profile, cr_timeout);
				/* 3524 1s */
				if (4 == cr_timeout) {
					vty_out(vty, "create %s instrance %d timeout.\n", service_name[0], profile);
					return CMD_WARNING;
				}
				sleep(1);
				continue;
			}else if (DCLI_VRRP_INSTANCE_CHECK_FAILED == check_result) {
				return CMD_WARNING;
			}else if (DCLI_VRRP_INSTANCE_CREATED == check_result) {
				//vty_out(vty, "create %s instrance %d used %d s.\n", service_name[0], profile, cr_timeout);
				cr_timeout = 0;
				break;
			}
		}		

		/* profile = 0, it's not necessary create wcpss process */
		if (0 != profile) {
			/* add for create wcpss process */
			memset(cmd,0,DCLI_VRRP_DBUSNAME_LEN);
			sprintf(cmd, "sudo /etc/init.d/wcpss start %d &", profile);
			//printf("wcpss %s\n",cmd);
			if (system(cmd)) {
				vty_out(vty, "create wcpss %d faild.\n", profile);
				return CMD_WARNING;
			}

			/* check wheather wcpss instance started completely.
			 * wsm not support multi-process.
			 */
			for (service_index = 1; service_index < 3; service_index++) {
				cr_timeout = 0;
				while (1) {
					cr_timeout++;
					check_result = dcli_vrrp_check_service_started(vty, service_name[service_index], profile);
					if (DCLI_VRRP_INSTANCE_NO_CREATED == check_result) {
						//vty_out(vty, "create %s instrance %d time %d s.\n", service_name[service_index], profile, cr_timeout);
						if (100 == cr_timeout) {
							vty_out(vty, "create %s instrance %d timeout.\n", service_name[service_index], profile);
							return CMD_WARNING;
						}
						sleep(1);
						continue;
					}else if (DCLI_VRRP_INSTANCE_CHECK_FAILED == check_result) {
						return CMD_WARNING;
					}else if (DCLI_VRRP_INSTANCE_CREATED == check_result) {
						//vty_out(vty, "create %s instrance %d used %d s.\n", service_name[service_index], profile, cr_timeout);
						cr_timeout = 0;
						break;
					}
				}
			}
			sleep(3);	/* for wait asd dbus thread create ok */
		}
	}
	else if(DCLI_VRRP_INSTANCE_CHECK_FAILED == instRun) {
		return CMD_WARNING;
	}

	dcli_vrrp_splice_objpath_string(VRRP_DBUS_OBJPATH, profile, vrrp_obj_path);
	dcli_vrrp_splice_dbusname(VRRP_DBUS_BUSNAME, profile, vrrp_dbus_name);

	//vty_out(vty, "path[%s] name[%s]\n", vrrp_obj_path, vrrp_dbus_name);
	
	query = dbus_message_new_method_call(vrrp_dbus_name,
										 vrrp_obj_path,
										 VRRP_DBUS_INTERFACE,
										 VRRP_DBUS_METHOD_CONFIG_HANSI_PROFILE);
	dbus_error_init(&err);

	dbus_message_append_args(query,
							 DBUS_TYPE_UINT32,&profile,
							 DBUS_TYPE_INVALID);
	
	reply = dbus_connection_send_with_reply_and_block (dcli_dbus_connection,query,-1, &err);
	dbus_message_unref(query);
	if (NULL == reply) {
		vty_out(vty,"failed get reply.\n");
		if (dbus_error_is_set(&err)) {
			printf("%s raised: %s",err.name,err.message);
			dbus_error_free_for_dcli(&err);
		}
		
		dbus_message_unref(reply);
		return CMD_WARNING;
		
	}

	if (dbus_message_get_args ( reply, &err,
		DBUS_TYPE_UINT32,&op_ret,
		DBUS_TYPE_UINT32,&profile,
		DBUS_TYPE_INVALID)){	

		if(DCLI_VRRP_RETURN_CODE_OK ==op_ret){
			if(CONFIG_NODE==(vty->node)){
					vty->node= HANSI_NODE;	
					vty->index = (void *)profile;
			}
		}
		else{
            vty_out(vty,dcli_vrrp_err_msg[op_ret - DCLI_VRRP_RETURN_CODE_OK]);
		}
	} 
	else 
	{		
		if (dbus_error_is_set(&err)) 
		{
			printf("%s raised: %s",err.name,err.message);
			dbus_error_free_for_dcli(&err);
			dbus_message_unref(reply);
			return CMD_WARNING;
		}
	}
	dbus_message_unref(reply);
	return CMD_SUCCESS;
}
#else
DEFUN(config_hansi_cmd_func,
	  config_hansi_cmd,
	  "config hansi-profile PARAMETER",
	  CONFIG_STR
	  HANSI_STR
	  "Profile ID for configuration"
)
{
	//vty_out(vty,"1\n");
	DBusMessage *query = NULL, *reply = NULL;
	DBusError err = {0};
	unsigned int hmd_ret = 0;
	unsigned int profile = 0;
	unsigned int slot_no = 0;
	unsigned int slot_no1 = 0;
	unsigned int slot_id = HostSlotId;
	
	DBusMessage *query2 = NULL, *reply2 = NULL;
	DBusError err2 = {0};
	unsigned int op_ret = 0;
	int instRun = 0;
	char cmd[DCLI_VRRP_DBUSNAME_LEN] = {0}; /*wcl add*/
	unsigned char insID = 0;
	int flag = 0;
	char vrrp_obj_path[DCLI_VRRP_OBJPATH_LEN] = {0};
	char vrrp_dbus_name[DCLI_VRRP_DBUSNAME_LEN] = {0};
	
	int local_id = 0;
	int ret = 0;
	//vty_out(vty,"2\n");
	ret = parse_slot_hansi_id((char*)argv[0],&slot_id,&profile);
	if(ret != WID_DBUS_SUCCESS){
		slot_id = HostSlotId;
		flag = 1;
		ret = parse_char_ID((char*)argv[0], &insID);
		if(ret != WID_DBUS_SUCCESS){
	            if(ret == WID_ILLEGAL_INPUT){
	            	vty_out(vty,"<error> illegal input:Input exceeds the maximum value of the parameter type \n");
	            }
				else{
					vty_out(vty,"<error> unknown id format\n");
				}
			return CMD_WARNING;
		}	
		profile = insID;
	}
	if(distributFag == 0){
		if(slot_id != 0){
			vty_out(vty,"<error> slot id should be 0\n");
			return CMD_WARNING;
		}	
	}else if(flag == 1){
		slot_id = HostSlotId;
	}
	
	//vty_out(vty,"3\n");
	//vty_out(vty,"4,slot_id=%d,profile=%d.\n",slot_id,profile);
	if((ret != 0)||(profile < 1)||(profile > 16)){
		vty_out(vty,"%% Bad parameter : %s !",argv[0]);
		return CMD_WARNING;
	}
	/*wcl add*/	
	instRun = dcli_hmd_hansi_is_running(vty,slot_id,0,profile);
		if(INSTANCE_NO_CREATED == instRun) {
			/* user verification */
			if (!boot_flag) {
				printf("Are you sure you want to start new hansi(yes/no)?");
				fscanf(stdin, "%s", cmd);
				if (!strncasecmp("no", cmd, strlen(cmd))){
					printf("\n%% User cancelled command.\n");
					return CMD_WARNING;
				}
				if (strncasecmp("yes", cmd, strlen(cmd))){
					printf("\nPlease type 'yes' or 'no':"); 
					fscanf(stdin, "%s", cmd);
					if (!strncasecmp("no", cmd, strlen(cmd))){
						printf("\n%% User cancelled command.\n");
						return CMD_WARNING;
					}
					else if (strncasecmp("yes", cmd, strlen(cmd))){
						printf("\n%% Bad choice given.\n");
						return CMD_WARNING;
					}
				}
				printf("\n");
			}
			}
	/*end*/
	/* system to fork had process */
//	instRun = dcli_hmd_hansi_is_running(vty,slot_id, 0, profile);

	query = dbus_message_new_method_call(HMD_DBUS_BUSNAME,HMD_DBUS_OBJPATH,HMD_DBUS_INTERFACE,HMD_DBUS_CONF_METHOD_REMOTE_HANSI);

	//vty_out(vty,"9\n");
	//query = dbus_message_new_method_call(HMD_DBUS_BUSNAME,HMD_DBUS_OBJPATH,HMD_DBUS_INTERFACE,HMD_DBUS_CONF_METHOD_REMOTE_HANSI);

	dbus_error_init(&err);
	//vty_out(vty,"10\n");
	dbus_message_append_args(query,
							 DBUS_TYPE_UINT32,&profile,
							 DBUS_TYPE_UINT32,&slot_id,
							 DBUS_TYPE_INVALID);
	//vty_out(vty,"11\n");
	reply = dbus_connection_send_with_reply_and_block (dcli_dbus_connection,query,150000, &err);

	//vty_out(vty,"12\n");
	dbus_message_unref(query);
	if (NULL == reply) {
		vty_out(vty,"hmd failed get reply.\n");
		if (dbus_error_is_set(&err)) {
			printf("%s raised: %s",err.name,err.message);
			dbus_error_free_for_dcli(&err);
		}
		return CMD_WARNING;
	}
	//vty_out(vty,"13\n");

	if (dbus_message_get_args ( reply, &err,
				DBUS_TYPE_UINT32,&hmd_ret,
				//DBUS_TYPE_UINT32,&profile,
				//DBUS_TYPE_UINT32,&slot_no,
				DBUS_TYPE_INVALID))
	{	
	
	//vty_out(vty,"14\n");
		if(hmd_ret == 0){
			if(vty->node == CONFIG_NODE){
				vty->node = HANSI_NODE;
				vty->index = (void *)profile;
				vty->slotindex = slot_id;
				//vty->slotindex1 = slot_no1;
				vty->local = 0;
			}
		}
		else if(hmd_ret == HMD_DBUS_PERMISSION_DENIAL){
			vty_out(vty,"<error> The Slot is not active master board, permission denial\n");			
			return CMD_WARNING;
		}else if(hmd_ret == HMD_DBUS_SLOT_ID_NOT_EXIST){
			vty_out(vty,"<error> The Slot is not EXIST\n");
			return CMD_WARNING;
		}else if (hmd_ret == HMD_DBUS_ID_NO_EXIST){
			vty_out(vty,"<error> remote hansi not exist\n");
			return CMD_WARNING;
		}
		else if(hmd_ret == HMD_DBUS_DELETING_HANSI){
			vty_out(vty,"<error>deleting hansi not completed,please wait several minutes\n");
			return CMD_WARNING;
		}
		else{
			vty_out(vty,"<error>  %d\n",hmd_ret);
			return CMD_WARNING;
		}

	} 
	else 
	{	
	
	//vty_out(vty,"15\n");
		if (dbus_error_is_set(&err)) 
		{
			printf("%s raised: %s",err.name,err.message);
			dbus_error_free_for_dcli(&err);
		}		
		return CMD_WARNING;
	}
	
	//vty_out(vty,"16\n");
	dbus_message_unref(reply);
	reply = NULL;
	/*fengwenchao comment for onlinebug-939*/
	/*HMD已经在启动HAD脚本时，执行了注释掉的动作*/
	#if 0
    /* send msg to had */
    DBusConnection *dcli_dbus_connection2 = NULL;
    ReInitDbusConnection(&dcli_dbus_connection2,slot_id,distributFag);
	dcli_vrrp_splice_objpath_string(VRRP_DBUS_OBJPATH, profile, vrrp_obj_path);
	dcli_vrrp_splice_dbusname(VRRP_DBUS_BUSNAME, profile, vrrp_dbus_name);

	
	query2 = dbus_message_new_method_call(vrrp_dbus_name,
										 vrrp_obj_path,
										 VRRP_DBUS_INTERFACE,
										 VRRP_DBUS_METHOD_CONFIG_HANSI_PROFILE);
	dbus_error_init(&err2);
	dbus_message_append_args(query2,
							 DBUS_TYPE_UINT32,&profile,
							 DBUS_TYPE_INVALID);
	
	reply2 = dbus_connection_send_with_reply_and_block (dcli_dbus_connection2,query2,-1, &err2);
	dbus_message_unref(query2);
	if (NULL == reply2) {
		if (dbus_error_is_set(&err2)) {
			dbus_error_free_for_dcli(&err2);
		}
		return CMD_WARNING;
	}

	if (dbus_message_get_args ( reply2, &err2,
		DBUS_TYPE_UINT32,&op_ret,
		DBUS_TYPE_UINT32,&profile,
		DBUS_TYPE_INVALID)){	

		if(DCLI_VRRP_RETURN_CODE_OK != op_ret){
			
            vty_out(vty,dcli_vrrp_err_msg[op_ret - DCLI_VRRP_RETURN_CODE_OK]);
		}
	} 
	else 
	{		
		if (dbus_error_is_set(&err2)) 
		{
			dbus_error_free_for_dcli(&err2);
			dbus_message_unref(reply2);
	
			return CMD_WARNING;
		}
	}
	dbus_message_unref(reply2);
	#endif
	return CMD_SUCCESS;
}
#endif



DEFUN(config_vrrp_global_switch_add_del_hansi_cmd_func,
	  config_vrrp_golbal_swtich_add_del_hansi_cmd,
	  "config vrrp global switch (add|delete) hansi-profile PARAMETER",
	  "config \n"
	  "config vrrp\n"
	  "config vrrp global\n"
	  "config vrrp global switch\n"
	  "add hansi profile\n "
	  "delete hansi profile\n"
	  "hansi-profile\n"
	  "Profile ID for configuration\n"
)
{
	//vty_out(vty,"1\n");
	DBusMessage *query = NULL, *reply = NULL;
	DBusError err = {0};
	unsigned int hmd_ret = 0;
	unsigned int profile = 0;
	unsigned int slot_id = HostSlotId;
	int instRun = 0;
	int is_added = 0;
	int ret = 0;
	//vty_out(vty,"2\n");

	if (!strcmp(argv[0], "add"))
	{
		is_added = 1;
	}
	else if (!strcmp(argv[0], "delete")) 
	{
		is_added = 0;
	}
	else 
	{
        vty_out(vty, "input value should be add or delete!\n");
        return CMD_WARNING;
	}	
	ret = parse_slot_hansi_id((char*)argv[1],&slot_id,&profile);
	if(ret != WID_DBUS_SUCCESS)
	{
		return CMD_WARNING;		
	}

	if((profile < 1)||(profile > 16) || (slot_id < 1) || (slot_id > 16))
	{
		vty_out(vty,"%% Bad parameter : %s !",argv[1]);
		return CMD_WARNING;
	}
	instRun = dcli_hmd_hansi_is_running(vty,slot_id,0,profile);
	if(INSTANCE_NO_CREATED == instRun) 
	{
		/* user verification */
		vty_out(vty,"%%hansi%d-%d not exists\n",slot_id,profile);
		return CMD_WARNING;
	}	
	query = dbus_message_new_method_call(HMD_DBUS_BUSNAME,    \
										HMD_DBUS_OBJPATH,	  \
										HMD_DBUS_INTERFACE,	  \
										HMD_DBUS_CONF_METHOD_VRRP_GLOBAL_SWITCH_ADD_DEL_HANSI);


	dbus_error_init(&err);
	//vty_out(vty,"10\n");
	dbus_message_append_args(query,
							 DBUS_TYPE_UINT32,&is_added,
							 DBUS_TYPE_UINT32,&profile,
							 DBUS_TYPE_UINT32,&slot_id,
							 DBUS_TYPE_INVALID);
	//vty_out(vty,"11\n");
	reply = dbus_connection_send_with_reply_and_block (dcli_dbus_connection,query,-1, &err);

	//vty_out(vty,"12\n");
	dbus_message_unref(query);
	if (NULL == reply) 
	{
		vty_out(vty,"hmd failed get reply.\n");
		if (dbus_error_is_set(&err)) 
		{
			vty_out(vty,"%s raised: %s",err.name,err.message);
			dbus_error_free_for_dcli(&err);
		}
		return CMD_WARNING;
	}
	//vty_out(vty,"13\n");

	if (dbus_message_get_args ( reply, &err,
				DBUS_TYPE_UINT32,&hmd_ret,
				DBUS_TYPE_INVALID))
	{	

		if(hmd_ret == HMD_DBUS_SUCCESS)
		{
			return CMD_SUCCESS;
		}
		if(hmd_ret == HMD_DBUS_PERMISSION_DENIAL)
		{
			vty_out(vty,"<error> The Slot is not active master board, permission denial\n");			
			return CMD_WARNING;
		}
		else if(hmd_ret == HMD_DBUS_SLOT_ID_NOT_EXIST)
		{
			vty_out(vty,"<error> The Slot is not EXIST\n");
			return CMD_WARNING;
		}
		else if (hmd_ret == HMD_DBUS_ID_NO_EXIST)
		{
			vty_out(vty,"<error> remote hansi not exist\n");
			return CMD_WARNING;
		}
		else if (hmd_ret == HMD_DBUS_HANSI_ID_EXIST)
		{
			vty_out(vty,"hansi %d-%d is already added\n",slot_id,profile);
			return CMD_WARNING;
		}
		else if (hmd_ret == HMD_DBUS_HANSI_ID_NOT_EXIST)
		{
			vty_out(vty,"hansi %d-%d is already deleted\n",slot_id,profile);
			return CMD_WARNING;
		}
		else if(hmd_ret == HMD_DBUS_WARNNING)
		{
			vty_out(vty,"Please enable vrrp global switch first !\n");
			return CMD_WARNING;
		}
		else
		{
			vty_out(vty,"<error>  %d\n",hmd_ret);
			return CMD_WARNING;
		}

	} 
	else 
	{	
	
		if (dbus_error_is_set(&err)) 
		{
			printf("%s raised: %s",err.name,err.message);
			dbus_error_free_for_dcli(&err);
		}		
		return CMD_WARNING;
	}
	
	dbus_message_unref(reply);
	
	return CMD_SUCCESS;
}


DEFUN(show_vrrp_global_switch_hansi_list_cmd_func,
	  show_vrrp_golbal_swtich_hansi_list_cmd,
	  "show vrrp global switch hansi-profile list",
	  "show \n"
	  "show vrrp\n"
	  "show vrrp global\n"
	  "show vrrp global switch\n"
	  "show vrrp global switch hansi-profile\n "
	  "show vrrp global switch hansi-profile list\n"
)
{
	//vty_out(vty,"1\n");
	unsigned int profile = 0;
	unsigned int slot_id = HostSlotId;
	int ret = 0;
	FILE *fp;
	int vrrp_global_hansi[16] = {0};
	int i = 0;

	fp=fopen("/var/run/vrrp_global_switch_hansi","r");

	if(fp != NULL)
	{
		for(i = 0;i<16;i++)
		{
			fscanf(fp,"%x",&vrrp_global_hansi[i]);
		}

		fclose(fp);
	}
	i = 0;
	vty_out(vty,"========================================================================\n");
	vty_out(vty,"                           hansi-profile list                           \n");
	vty_out(vty,"========================================================================\n");

	for(slot_id=0;slot_id<16;slot_id++)
	{
		for(profile=0;profile<16;profile++)
		{
			if(vrrp_global_hansi[slot_id]&(1<<profile))
			{
				if(i && (0 == i%15))
				{
					vty_out(vty,"\n%");
				}
				vty_out(vty,"%s%d-%d",(i%15)?",":"",slot_id+1,profile+1);
				i++;

				
			}
		}
		
	}
	vty_out(vty,"\n");
	if(i == 0)
	{
		vty_out(vty,"No hansi-profile\n");
	}

	vty_out(vty,"========================================================================\n");
	
	
	return CMD_SUCCESS;
}

#ifndef DISTRIBUT
/*******************************************************************************
 *config_delete_hansi_cmd
 *  DESCRIPTION:
 *		delete special hansi instance in version 1.3.3
 *		no support this command before version 1.3.3
 *
 *  INPUTS:
 *		<0-16>	- hansi instance ID
 *
 *  OUTPUTS:
 *		NULL
 *
 *  RETURN VALUE:
 *
 ******************************************************************************/
DEFUN(config_delete_hansi_cmd_func,
	config_delete_hansi_cmd,
	"delete hansi-profile <0-16>",
	"Delete hansi instance"
	HANSI_STR
	"Profile ID for configuration"
)
{
	unsigned int ret = 0;
	unsigned int profile = 0;
	int instRun = 0;
	char cmd[DCLI_VRRP_DBUSNAME_LEN * 2] = {0};

	if (argc == 1) {
		profile = strtoul((char *)argv[0], NULL, 0);
		if ((profile < 0) ||
			(profile > 16)) {
			vty_out(vty, "%% Bad parameter : %s !", argv[0]);
			return CMD_WARNING;
		}
	}

	/* system to fork had process */
	instRun = dcli_vrrp_hansi_is_running(vty, profile);
	if (DCLI_VRRP_INSTANCE_CREATED == instRun) {
		/* [1] disable the had instance service,
		 *		for it will tell other service(eag/wcpss) its state,
		 *		 when STATE MACHINE transfer to other state.
		 */
		vty_out(vty, "%% Disable hansi %d service ...\n", profile);
		ret = dcli_vrrp_config_service_disable(vty, profile);
		if (0 != ret) {
			vty_out(vty, "config hansi %d service disable faild.\n", profile);
			return CMD_WARNING;
		}

		/* [2] kill the special had instance process which instance no is profile. */
		vty_out(vty, "%% Delete hansi instance %d ...\n", profile);
		#if 0
		sprintf(cmd,
				"PIDS=`ps -ef | grep \"had %d$\" | grep -v grep | awk '{print $2}'`; father=`echo $PIDS | awk '{print $1}'`; sudo kill $father",
				profile);
		#else
		sprintf(cmd, "/etc/init.d/had stop %d &", profile);
		#endif
		if (system(cmd)) {
			vty_out(vty, "delete hansi %d faild.\n", profile);
			return CMD_WARNING;
		}

		/* [3] clear tmp file of had instance */
		memset(cmd, 0, DCLI_VRRP_DBUSNAME_LEN * 2);
		sprintf(cmd,
				"sudo rm /var/run/had%d.pidmap",
				profile);
		if (system(cmd)) {
			vty_out(vty, "delete hansi %d faild.\n", profile);
			return CMD_WARNING;
		}
		usleep(1000000);

		/* profile = 0, it's not necessary delete wcpss process */
		if (0 != profile) {
			/* [4] kill wid(...) process */
			vty_out(vty, "%% Delete wcpss instance %d ...\n", profile);
			memset(cmd, 0, DCLI_VRRP_DBUSNAME_LEN * 2);
			sprintf(cmd, "sudo /etc/init.d/wcpss stop %d &", profile);
			if (system(cmd)) {
				vty_out(vty, "create hansi %d faild.\n", profile);
				return CMD_WARNING;
			}
		}
	}
	else if (DCLI_VRRP_INSTANCE_CHECK_FAILED == instRun) {
		return CMD_WARNING;
	}else if (DCLI_VRRP_INSTANCE_NO_CREATED == instRun) {
		vty_out(vty, "hansi %d no exist.\n", profile);
		return CMD_SUCCESS;
	}

	return CMD_SUCCESS;
}
#else
DEFUN(config_delete_hansi_cmd_func,
	config_delete_hansi_cmd,
	"delete hansi-profile PARAMETER",
	"Delete hansi instance"
	HANSI_STR
	"Profile ID for configuration"
)

{
	DBusMessage *query = NULL, *reply = NULL;
	DBusError err = {0};
	unsigned int hmd_ret = 0;
	unsigned int profile = 0;
	unsigned int slot_no = 0;
	unsigned int slot_no1 = 0;
	unsigned int slot_id = HostSlotId;
	
	char cmd[DCLI_VRRP_DBUSNAME_LEN] = {0};
	int local_id = 0;
	int instRun = 0;
	int ret = 0;
	unsigned char insID = 0;
	int flag = 0;
	ret = parse_slot_hansi_id((char*)argv[0],&slot_id,&profile);

    insID = (unsigned char)profile;
	if(ret != WID_DBUS_SUCCESS){
		slot_id = HostSlotId;
		flag = 1;
		ret = parse_char_ID((char*)argv[0], &insID);
		if(ret != WID_DBUS_SUCCESS){
	            if(ret == WID_ILLEGAL_INPUT){
	            	vty_out(vty,"<error> illegal input:Input exceeds the maximum value of the parameter type \n");
	            }
				else{
					vty_out(vty,"<error> unknown id format\n");
				}
			return CMD_SUCCESS;
		}	
	}
	profile = insID;
	if(distributFag == 0){
		if(slot_id != 0){
			vty_out(vty,"<error> slot id should be 0\n");
			return CMD_SUCCESS;
		}	
	}else if(flag == 1){
		slot_id = HostSlotId;
	}
	
	if((ret != 0)||(profile < 0)||(profile > 16)){
		vty_out(vty,"%% Bad parameter : %s !",argv[0]);
		return CMD_WARNING;
	}
	
	/*wcl add*/
	instRun = dcli_hmd_hansi_is_running(vty,slot_id,0,profile);
	if (INSTANCE_NO_CREATED == instRun) {
		/*vty_out(vty, "had instance %d not created!\n", profile);*/
		printf("This hansi-profile %d had not created!\n",profile);
		return DCLI_VRRP_RETURN_CODE_PROFILE_NOTEXIST;
	}
	/*end*/
			 /*wcl add*/
			if (!boot_flag) {
				printf("Are you sure you want to delete this hansi(yes/no)?");
				fscanf(stdin, "%s", cmd);
				if (!strncasecmp("no", cmd, strlen(cmd))){
					printf("\n%% User cancelled command.\n");
					return CMD_WARNING;
				}
				if (strncasecmp("yes", cmd, strlen(cmd))){
					printf("\nPlease type 'yes' or 'no':"); 
					fscanf(stdin, "%s", cmd);
					if (!strncasecmp("no", cmd, strlen(cmd))){
						printf("\n%% User cancelled command.\n");
						return CMD_WARNING;
					}
					else if (strncasecmp("yes", cmd, strlen(cmd))){
						printf("\n%% Bad choice given.\n");
						return CMD_WARNING;
					}
				}
				printf("\n");
			}
		/*end*/
//	instRun = dcli_vrrp_hansi_is_running(vty, profile);
	//vty_out(vty,"instRun = %d\n",instRun);
	{
		/* notify hmd deamon to release resource, book add 2011-5-23 */
        query = dbus_message_new_method_call(HMD_DBUS_BUSNAME,HMD_DBUS_OBJPATH,HMD_DBUS_INTERFACE,HMD_DBUS_METHOD_DELETE_REMOTE_HANSI);

    	dbus_error_init(&err);
    	dbus_message_append_args(query,
    							 DBUS_TYPE_UINT32,&profile,
    							 DBUS_TYPE_UINT32,&slot_id,
    							 DBUS_TYPE_INVALID);
    	reply = dbus_connection_send_with_reply_and_block (dcli_dbus_connection,query,-1, &err);
        //vty_out(vty,"444\n");
    	dbus_message_unref(query);
    	if (NULL == reply) {
    		vty_out(vty,"failed get reply.\n");
    		if (dbus_error_is_set(&err)) {
    			printf("%s raised: %s",err.name,err.message);
    			dbus_error_free_for_dcli(&err);
    		}
    		return CMD_SUCCESS;
    	}
        //vty_out(vty,"555\n");
    	if (dbus_message_get_args ( reply, &err,
    				DBUS_TYPE_UINT32,&hmd_ret,
    				//DBUS_TYPE_UINT32,&profile,
    				//DBUS_TYPE_UINT32,&slot_no,
    				DBUS_TYPE_INVALID))
    	{	
            if(hmd_ret == 0)
                vty_out(vty,"delete hansi successful\n");
            else if(hmd_ret == 2)
            	vty_out(vty,"hansi not exist\n");
            else
                vty_out(vty,"<error>  %d\n",hmd_ret);
    	} 
    	else 
    	{	
    	    //vty_out(vty,"15\n");
    		if (dbus_error_is_set(&err)) 
    		{
    			printf("%s raised: %s",err.name,err.message);
    			dbus_error_free_for_dcli(&err);
    		}
    	}
    	//vty_out(vty,"666\n");
    	dbus_message_unref(reply);
    	if(reply == NULL){
    	}else{
    		reply = NULL;
    	}
	}
	
	return CMD_SUCCESS;
}

#endif


DEFUN(config_vrrp_heartbeat_cmd_func,
	  config_vrrp_heartbeat_cmd,
	  "config heartbeatlink IFNAME IP",
	  CONFIG_STR
	  "Heart beat link\n"
	  "L3 interface name\n"
	  "L3 interface ip addr\n"
)

{
	DBusMessage *query = NULL, *reply = NULL;
	DBusError err = {0};
	unsigned int op_ret = 0;
	unsigned int profile = 0,vrid = 0,priority = 0;
	int add = 1;
	unsigned int slot_id = HostSlotId;
	char* heartbeat_ifname = NULL;
    char* heartbeat_ip = NULL;
	char vrrp_obj_path[DCLI_VRRP_OBJPATH_LEN] = {0};
	char vrrp_dbus_name[DCLI_VRRP_DBUSNAME_LEN] = {0};	
	
	heartbeat_ifname = (char *)malloc(MAX_IFNAME_LEN);
    if(NULL == heartbeat_ifname){
       return CMD_WARNING;
	}	
	memset(heartbeat_ifname,0,MAX_IFNAME_LEN);
	memcpy(heartbeat_ifname,argv[0],strlen(argv[0]));

	
	heartbeat_ip = (char *)malloc(MAX_IPADDR_LEN);
    if(NULL == heartbeat_ip){
       return CMD_WARNING;
	}	
	memset(heartbeat_ip,0,MAX_IPADDR_LEN);
	memcpy(heartbeat_ip,argv[1],strlen(argv[1]));

	if(HANSI_NODE==vty->node){
		profile = (unsigned int)(vty->index);
		slot_id = vty->slotindex;
	}

#ifdef DISTRIBUT
    DBusConnection *dcli_dbus_connection = NULL;
    ReInitDbusConnection(&dcli_dbus_connection,slot_id,distributFag);
#endif
	dcli_vrrp_splice_objpath_string(VRRP_DBUS_OBJPATH, profile, vrrp_obj_path);
	dcli_vrrp_splice_dbusname(VRRP_DBUS_BUSNAME, profile, vrrp_dbus_name);

	query = dbus_message_new_method_call(vrrp_dbus_name,
										 vrrp_obj_path,
										 VRRP_DBUS_INTERFACE,
										 VRRP_DBUS_METHOD_VRRP_HEARTBEAT_LINK);
	dbus_error_init(&err);

	dbus_message_append_args(query,
		                     DBUS_TYPE_UINT32,&profile,
		                     DBUS_TYPE_STRING,&heartbeat_ifname,
							 DBUS_TYPE_STRING,&heartbeat_ip,						 
							 DBUS_TYPE_INVALID);
	
	reply = dbus_connection_send_with_reply_and_block (dcli_dbus_connection,query,-1, &err);
	dbus_message_unref(query);
	if (NULL == reply) {
		vty_out(vty,"failed get reply.\n");
		if (dbus_error_is_set(&err)) {
			printf("%s raised: %s",err.name,err.message);
			dbus_error_free_for_dcli(&err);
		}
		return CMD_SUCCESS;
	}
	else if (dbus_message_get_args ( reply, &err,
		DBUS_TYPE_UINT32,&op_ret,
		DBUS_TYPE_INVALID)){	
        if( DCLI_VRRP_RETURN_CODE_OK != op_ret){
            vty_out(vty,dcli_vrrp_err_msg[op_ret - DCLI_VRRP_RETURN_CODE_OK]);
		}
		/*dcli_vrrp_notify_to_npd(vty,uplink_ifname,downlink_ifname,add);*/
	} 
	else 
	{		
		if (dbus_error_is_set(&err)) 
		{
			printf("%s raised: %s",err.name,err.message);
			dbus_error_free_for_dcli(&err);
		}
	}
	free(heartbeat_ip);
	free(heartbeat_ifname);
	dbus_message_unref(reply);
	return CMD_SUCCESS;
}


DEFUN(config_vrrp_real_ip_cmd_func,
	  config_vrrp_real_ip_cmd,
	  "appoint realip uplink IFNAME IP downlink IFNAME IP",
	  "Appoint infomation used in hansi\n"
	  "Appoint real ip\n"
	  "Config uplink\n"
	  "L3 interface name\n"
	  "L3 interface real ip\n"
	  "Config downlink\n"
	  "L3 interface name\n"
	  "L3 interface real ip\n"
)

{
	DBusMessage *query = NULL, *reply = NULL;
	DBusError err = {0};
	unsigned int op_ret = 0;
	unsigned int slot_id = HostSlotId;
	unsigned int profile = 0,vrid = 0,priority = 0;
	int add = 1;
	char* uplink_ifname = NULL,*downlink_ifname = NULL;
    char* uplink_ip = NULL,*downlink_ip = NULL;
	uplink_ifname = (char *)malloc(MAX_IFNAME_LEN);
    if(NULL == uplink_ifname){
       return CMD_WARNING;
	}	
	memset(uplink_ifname,0,MAX_IFNAME_LEN);
	memcpy(uplink_ifname,argv[0],strlen(argv[0]));

	downlink_ifname = (char *)malloc(MAX_IFNAME_LEN);	
	if(NULL == downlink_ifname){
       return CMD_WARNING;
	}
	memset(downlink_ifname,0,MAX_IFNAME_LEN);
	memcpy(downlink_ifname,argv[2],strlen(argv[2]));
	
	uplink_ip = (char *)malloc(MAX_IPADDR_LEN);
    if(NULL == uplink_ip){
       return CMD_WARNING;
	}	
	memset(uplink_ip,0,MAX_IPADDR_LEN);
	memcpy(uplink_ip,argv[1],strlen(argv[1]));

	downlink_ip = (char *)malloc(MAX_IPADDR_LEN);	
	if(NULL == downlink_ip){
       return CMD_WARNING;
	}
	memset(downlink_ip,0,MAX_IPADDR_LEN);
	memcpy(downlink_ip,argv[3],strlen(argv[3]));


	if(HANSI_NODE==vty->node){
		profile = (unsigned int)(vty->index);
		slot_id = vty->slotindex;
	}

	char vrrp_obj_path[DCLI_VRRP_OBJPATH_LEN] = {0};
	char vrrp_dbus_name[DCLI_VRRP_DBUSNAME_LEN] = {0};	

#ifdef DISTRIBUT
    DBusConnection *dcli_dbus_connection = NULL;
    ReInitDbusConnection(&dcli_dbus_connection,slot_id,distributFag);
#endif
	dcli_vrrp_splice_objpath_string(VRRP_DBUS_OBJPATH, profile, vrrp_obj_path);
	dcli_vrrp_splice_dbusname(VRRP_DBUS_BUSNAME, profile, vrrp_dbus_name);
	
	query = dbus_message_new_method_call(vrrp_dbus_name,
										 vrrp_obj_path,
										 VRRP_DBUS_INTERFACE,
										 VRRP_DBUS_METHOD_VRRP_REAL_IP);
	dbus_error_init(&err);

	dbus_message_append_args(query,
		                     DBUS_TYPE_UINT32,&profile,
		                     DBUS_TYPE_STRING,&uplink_ifname,
							 DBUS_TYPE_STRING,&uplink_ip,
							 DBUS_TYPE_STRING,&downlink_ifname,
							 DBUS_TYPE_STRING,&downlink_ip,							 
							 DBUS_TYPE_INVALID);
	
	reply = dbus_connection_send_with_reply_and_block (dcli_dbus_connection,query,-1, &err);
	dbus_message_unref(query);
	if (NULL == reply) {
		vty_out(vty,"failed get reply.\n");
		if (dbus_error_is_set(&err)) {
			printf("%s raised: %s",err.name,err.message);
			dbus_error_free_for_dcli(&err);
		}
		return CMD_SUCCESS;
	}
	else if (dbus_message_get_args ( reply, &err,
		DBUS_TYPE_UINT32,&op_ret,
		DBUS_TYPE_INVALID)){	
        if( DCLI_VRRP_RETURN_CODE_OK != op_ret){
            vty_out(vty,dcli_vrrp_err_msg[op_ret - DCLI_VRRP_RETURN_CODE_OK]);
		}
	} 
	else 
	{		
		if (dbus_error_is_set(&err)) 
		{
			printf("%s raised: %s",err.name,err.message);
			dbus_error_free_for_dcli(&err);
		}
	}
	free(uplink_ip);
	free(downlink_ip);
	dbus_message_unref(reply);
	return CMD_SUCCESS;
}

DEFUN(config_vrrp_real_ip_downlink_cmd_func,
	  config_vrrp_real_ip_downlink_cmd,
	  "appoint realip downlink IFNAME IP",
	  "Appoint infomation used in hansi\n"
	  "Appoint real ip\n"
	  "Config downlink\n"
	  "L3 interface name\n"
	  "L3 interface real ip\n"
)

{
	DBusMessage *query = NULL, *reply = NULL;
	DBusError err = {0};
	unsigned int op_ret = 0;
	unsigned int slot_id = HostSlotId;
	unsigned int profile = 0,vrid = 0,priority = 0;
	int add = 1;
	char* downlink_ifname = NULL;
    char* downlink_ip = NULL;


	 downlink_ifname = (char*)malloc(sizeof(char)*MAX_IFNAME_LEN);
	if(NULL == downlink_ifname){
          vty_out(vty,"%% Downlink ifname malloc failed\n");
		  return CMD_WARNING;
	}
	downlink_ip = (char*)malloc(sizeof(char)*MAX_IPADDR_LEN);
	if(NULL == downlink_ip){
          vty_out(vty,"%% Downlink ip malloc failed\n");
		  free(downlink_ifname);
		  return CMD_WARNING;
	}
	
	memset(downlink_ifname, 0, sizeof(char)*MAX_IPADDR_LEN);
	if(strlen(argv[0])>sizeof(char)*MAX_IPADDR_LEN){
        memcpy(downlink_ifname, argv[0], sizeof(char)*MAX_IPADDR_LEN);
	}
	else{
		memcpy(downlink_ifname, argv[0], strlen(argv[0])); 
	}
	
	memset(downlink_ip, 0, sizeof(char)*MAX_IPADDR_LEN);
	if(strlen(argv[1])>sizeof(char)*MAX_IPADDR_LEN){
        memcpy(downlink_ip, argv[1], sizeof(char)*MAX_IPADDR_LEN);
	}
	else{
		memcpy(downlink_ip, argv[1], strlen(argv[1])); 
	}	


	if(HANSI_NODE==vty->node){
		profile = (unsigned int)(vty->index);
		slot_id = vty->slotindex;
	}

	char vrrp_obj_path[DCLI_VRRP_OBJPATH_LEN] = {0};
	char vrrp_dbus_name[DCLI_VRRP_DBUSNAME_LEN] = {0};	

#ifdef DISTRIBUT
    DBusConnection *dcli_dbus_connection = NULL;
    ReInitDbusConnection(&dcli_dbus_connection,slot_id,distributFag);
#endif	
	dcli_vrrp_splice_objpath_string(VRRP_DBUS_OBJPATH, profile, vrrp_obj_path);
	dcli_vrrp_splice_dbusname(VRRP_DBUS_BUSNAME, profile, vrrp_dbus_name);

	query = dbus_message_new_method_call(vrrp_dbus_name,
										 vrrp_obj_path,
										 VRRP_DBUS_INTERFACE,
										 VRRP_DBUS_METHOD_VRRP_REAL_IP_DOWNLINK);
	dbus_error_init(&err);

	dbus_message_append_args(query,
		                     DBUS_TYPE_UINT32,&profile,
							 DBUS_TYPE_STRING,&downlink_ifname,
							 DBUS_TYPE_STRING,&downlink_ip,							 
							 DBUS_TYPE_INVALID);
	
	reply = dbus_connection_send_with_reply_and_block (dcli_dbus_connection,query,-1, &err);
	dbus_message_unref(query);
	if (NULL == reply) {
		vty_out(vty,"failed get reply.\n");
		if (dbus_error_is_set(&err)) {
			printf("%s raised: %s",err.name,err.message);
			dbus_error_free_for_dcli(&err);
		}
		return CMD_SUCCESS;
	}
	else if (dbus_message_get_args ( reply, &err,
		DBUS_TYPE_UINT32,&op_ret,
		DBUS_TYPE_INVALID)){	
        if( DCLI_VRRP_RETURN_CODE_OK != op_ret){
            vty_out(vty,dcli_vrrp_err_msg[op_ret - DCLI_VRRP_RETURN_CODE_OK]);
		}
		/*dcli_vrrp_notify_to_npd(vty,uplink_ifname,downlink_ifname,add);*/
	} 
	else 
	{		
		if (dbus_error_is_set(&err)) 
		{
			printf("%s raised: %s",err.name,err.message);
			dbus_error_free_for_dcli(&err);
		}
	}
	free(downlink_ip);
	dbus_message_unref(reply);
	return CMD_SUCCESS;
}

DEFUN(config_vrrp_real_ip_uplink_cmd_func,
	config_vrrp_real_ip_uplink_cmd,
	"appoint realip uplink IFNAME IP",
	"Appoint infomation used in hansi\n"
	"Appoint real ip\n"
	"Config uplink\n"
	"L3 interface name\n"
	"L3 interface real ip\n"
)
{
	DBusMessage *query = NULL;
	DBusMessage *reply = NULL;
	DBusError err = {0};

	unsigned int op_ret = 0;
	unsigned int slot_id = HostSlotId;
	unsigned int profile = 0;
	char *uplink_ifname = NULL;
	char *uplink_ip = NULL;
	char vrrp_obj_path[DCLI_VRRP_OBJPATH_LEN] = {0};
	char vrrp_dbus_name[DCLI_VRRP_DBUSNAME_LEN] = {0};

	uplink_ifname = (char*)malloc(sizeof(char)*MAX_IFNAME_LEN);
	if(NULL == uplink_ifname){
		  vty_out(vty,"%% Uplink ifname malloc failed\n");
		  return CMD_WARNING;
	}
	uplink_ip = (char*)malloc(sizeof(char)*MAX_IPADDR_LEN);
	if(NULL == uplink_ip){
		  vty_out(vty,"%% Uplink ip malloc failed\n");
		  free(uplink_ifname);
		  return CMD_WARNING;
	}

	memset(uplink_ifname, 0, sizeof(char)*MAX_IPADDR_LEN);
	if(strlen(argv[0])>sizeof(char)*MAX_IPADDR_LEN){
        memcpy(uplink_ifname, argv[0], sizeof(char)*MAX_IPADDR_LEN);
	}
	else{
		memcpy(uplink_ifname, argv[0], strlen(argv[0])); 
	}
	
	memset(uplink_ip, 0, sizeof(char)*MAX_IPADDR_LEN);
	if(strlen(argv[1])>sizeof(char)*MAX_IPADDR_LEN){
        memcpy(uplink_ip, argv[1], sizeof(char)*MAX_IPADDR_LEN);
	}
	else{
		memcpy(uplink_ip, argv[1], strlen(argv[1])); 
	}	

	if (HANSI_NODE == vty->node) {
		profile = (unsigned int)(vty->index);
		slot_id = vty->slotindex;
	}

#ifdef DISTRIBUT
    DBusConnection *dcli_dbus_connection = NULL;
    ReInitDbusConnection(&dcli_dbus_connection,slot_id,distributFag);
#endif
	dcli_vrrp_splice_objpath_string(VRRP_DBUS_OBJPATH, profile, vrrp_obj_path);
	dcli_vrrp_splice_dbusname(VRRP_DBUS_BUSNAME, profile, vrrp_dbus_name);

	query = dbus_message_new_method_call(vrrp_dbus_name,
										 vrrp_obj_path,
										 VRRP_DBUS_INTERFACE,
										 VRRP_DBUS_METHOD_VRRP_REAL_IP_UPLINK);
	dbus_error_init(&err);
	dbus_message_append_args(query,
							DBUS_TYPE_UINT32, &profile,
							DBUS_TYPE_STRING, &uplink_ifname,
							DBUS_TYPE_STRING, &uplink_ip,
							DBUS_TYPE_INVALID);
	
	reply = dbus_connection_send_with_reply_and_block(dcli_dbus_connection, query, -1, &err);
	dbus_message_unref(query);
	if (NULL == reply) {
		vty_out(vty, "failed get reply.\n");
		if (dbus_error_is_set(&err)) {
			printf("%s raised: %s",err.name,err.message);
			dbus_error_free_for_dcli(&err);
		}
		return CMD_SUCCESS;
	}
	else if (dbus_message_get_args(reply, &err,
									DBUS_TYPE_UINT32, &op_ret,
									DBUS_TYPE_INVALID))
	{
        if (DCLI_VRRP_RETURN_CODE_OK != op_ret) {
            vty_out(vty, dcli_vrrp_err_msg[op_ret - DCLI_VRRP_RETURN_CODE_OK]);
		}
	}
	else
	{
		if (dbus_error_is_set(&err))
		{
			printf("%s raised: %s",err.name,err.message);
			dbus_error_free_for_dcli(&err);
		}
	}

	dbus_message_unref(reply);

	free(uplink_ifname);
	free(uplink_ip);
	return CMD_SUCCESS;
}

#if 1
DEFUN(config_vrrp_no_real_ip_downlink_cmd_func,
	config_vrrp_no_real_ip_downlink_cmd,
	"no appoint realip downlink IFNAME IP",
	"Cancel order\n"
	"Appoint infomation used in hansi\n"
	"Appoint real ip\n"
	"Config downlink\n"
	"L3 interface name\n"
	"L3 interface real ip\n"
)
{
	DBusMessage *query = NULL;
	DBusMessage *reply = NULL;
	DBusError err = {0};

	unsigned int op_ret = 0;
	unsigned int slot_id = HostSlotId;
	unsigned int profile = 0;
	char *downlink_ifname = NULL;
	char *downlink_ip = NULL;
	char vrrp_obj_path[DCLI_VRRP_OBJPATH_LEN] = {0};
	char vrrp_dbus_name[DCLI_VRRP_DBUSNAME_LEN] = {0};

    downlink_ifname = (char*)malloc(sizeof(char)*MAX_IFNAME_LEN);
	if(NULL == downlink_ifname){
          vty_out(vty,"%% Downlink ifname malloc failed\n");
		  return CMD_WARNING;
	}
	downlink_ip = (char*)malloc(sizeof(char)*MAX_IPADDR_LEN);
	if(NULL == downlink_ip){
          vty_out(vty,"%% Downlink ip malloc failed\n");
		  free(downlink_ifname);
		  return CMD_WARNING;
	}
	
	memset(downlink_ifname, 0, sizeof(char)*MAX_IPADDR_LEN);
	if(strlen(argv[0])>sizeof(char)*MAX_IPADDR_LEN){
        memcpy(downlink_ifname, argv[0], sizeof(char)*MAX_IPADDR_LEN);
	}
	else{
		memcpy(downlink_ifname, argv[0], strlen(argv[0])); 
	}
	
	memset(downlink_ip, 0, sizeof(char)*MAX_IPADDR_LEN);
	if(strlen(argv[1])>sizeof(char)*MAX_IPADDR_LEN){
        memcpy(downlink_ip, argv[1], sizeof(char)*MAX_IPADDR_LEN);
	}
	else{
		memcpy(downlink_ip, argv[1], strlen(argv[1])); 
	}	

	if (HANSI_NODE==vty->node) {
		profile = (unsigned int)(vty->index);
		slot_id = vty->slotindex;
	}

#ifdef DISTRIBUT
    DBusConnection *dcli_dbus_connection = NULL;
    ReInitDbusConnection(&dcli_dbus_connection,slot_id,distributFag);
#endif
	dcli_vrrp_splice_objpath_string(VRRP_DBUS_OBJPATH, profile, vrrp_obj_path);
	dcli_vrrp_splice_dbusname(VRRP_DBUS_BUSNAME, profile, vrrp_dbus_name);

	query = dbus_message_new_method_call(vrrp_dbus_name,
										 vrrp_obj_path,
										 VRRP_DBUS_INTERFACE,
										 VRRP_DBUS_METHOD_VRRP_NO_REAL_IP_DOWNLINK);
	dbus_error_init(&err);

	dbus_message_append_args(query,
		                     DBUS_TYPE_UINT32, &profile,
							 DBUS_TYPE_STRING, &downlink_ifname,
							 DBUS_TYPE_STRING, &downlink_ip,
							 DBUS_TYPE_INVALID);
	
	reply = dbus_connection_send_with_reply_and_block(dcli_dbus_connection, query, -1, &err);
	dbus_message_unref(query);
	if (NULL == reply) {
		vty_out(vty, "failed get reply.\n");
		if (dbus_error_is_set(&err)) {
			printf("%s raised: %s", err.name, err.message);
			dbus_error_free_for_dcli(&err);
		}
		if(downlink_ifname){
	        free(downlink_ifname);
			downlink_ifname = NULL;
		}
		if(downlink_ip){
	        free(downlink_ip);
			downlink_ip = NULL;
		}
	
		return CMD_SUCCESS;
	}
	else if (dbus_message_get_args(reply, &err,
										DBUS_TYPE_UINT32, &op_ret,
										DBUS_TYPE_INVALID))
	{
		if (DCLI_VRRP_RETURN_CODE_OK != op_ret) {
			vty_out(vty,dcli_vrrp_err_msg[op_ret - DCLI_VRRP_RETURN_CODE_OK]);
		}
	}
	else
	{
		if (dbus_error_is_set(&err))
		{
			printf("%s raised: %s", err.name, err.message);
			dbus_error_free_for_dcli(&err);
		}
	}
    if(downlink_ifname){
        free(downlink_ifname);
		downlink_ifname = NULL;
	}
	if(downlink_ip){
        free(downlink_ip);
		downlink_ip = NULL;
	}
	dbus_message_unref(reply);
	return CMD_SUCCESS;
}

DEFUN(config_vrrp_no_real_ip_uplink_cmd_func,
	config_vrrp_no_real_ip_uplink_cmd,
	"no appoint realip uplink IFNAME IP",
	"Cancel order\n"
	"Appoint infomation used in hansi\n"
	"Appoint real ip\n"
	"Config uplink\n"
	"L3 interface name\n"
	"L3 interface real ip\n"
)
{
	DBusMessage *query = NULL;
	DBusMessage *reply = NULL;
	DBusError err = {0};

	unsigned int op_ret = 0;
	unsigned int slot_id = HostSlotId;
	unsigned int profile = 0;
	char *uplink_ifname = NULL;
	char *uplink_ip = NULL;
	char vrrp_obj_path[DCLI_VRRP_OBJPATH_LEN] = {0};
	char vrrp_dbus_name[DCLI_VRRP_DBUSNAME_LEN] = {0};

	uplink_ifname = (char*)malloc(sizeof(char)*MAX_IFNAME_LEN);
	if(NULL == uplink_ifname){
		  vty_out(vty,"%% Uplink ifname malloc failed\n");
		  return CMD_WARNING;
	}
	uplink_ip = (char*)malloc(sizeof(char)*MAX_IPADDR_LEN);
	if(NULL == uplink_ip){
		  vty_out(vty,"%% Uplink ip malloc failed\n");
		  free(uplink_ifname);
		  return CMD_WARNING;
	}

	memset(uplink_ifname, 0, sizeof(char)*MAX_IPADDR_LEN);
	if(strlen(argv[0])>sizeof(char)*MAX_IPADDR_LEN){
        memcpy(uplink_ifname, argv[0], sizeof(char)*MAX_IPADDR_LEN);
	}
	else{
		memcpy(uplink_ifname, argv[0], strlen(argv[0])); 
	}
	
	memset(uplink_ip, 0, sizeof(char)*MAX_IPADDR_LEN);
	if(strlen(argv[1])>sizeof(char)*MAX_IPADDR_LEN){
        memcpy(uplink_ip, argv[1], sizeof(char)*MAX_IPADDR_LEN);
	}
	else{
		memcpy(uplink_ip, argv[1], strlen(argv[1])); 
	}	

	if (HANSI_NODE == vty->node) {
		profile = (unsigned int)(vty->index);
		slot_id = vty->slotindex;
	}

#ifdef DISTRIBUT
    DBusConnection *dcli_dbus_connection = NULL;
    ReInitDbusConnection(&dcli_dbus_connection,slot_id,distributFag);
#endif
	dcli_vrrp_splice_objpath_string(VRRP_DBUS_OBJPATH, profile, vrrp_obj_path);
	dcli_vrrp_splice_dbusname(VRRP_DBUS_BUSNAME, profile, vrrp_dbus_name);

	query = dbus_message_new_method_call(vrrp_dbus_name,
										 vrrp_obj_path,
										 VRRP_DBUS_INTERFACE,
										 VRRP_DBUS_METHOD_VRRP_NO_REAL_IP_UPLINK);
	dbus_error_init(&err);
	dbus_message_append_args(query,
							DBUS_TYPE_UINT32, &profile,
							DBUS_TYPE_STRING, &uplink_ifname,
							DBUS_TYPE_STRING, &uplink_ip,
							DBUS_TYPE_INVALID);

	reply = dbus_connection_send_with_reply_and_block(dcli_dbus_connection, query, -1, &err);
	dbus_message_unref(query);
	if (NULL == reply) {
		vty_out(vty, "failed get reply.\n");
		if (dbus_error_is_set(&err)) {
			printf("%s raised: %s",err.name,err.message);
			dbus_error_free_for_dcli(&err);
		}
		if(uplink_ifname){
	        free(uplink_ifname);
			uplink_ifname = NULL;
		}
		if(uplink_ip){
	        free(uplink_ip);
			uplink_ip = NULL;
		}
		return CMD_SUCCESS;
	}
	else if (dbus_message_get_args(reply, &err,
									DBUS_TYPE_UINT32, &op_ret,
									DBUS_TYPE_INVALID))
	{
        if (DCLI_VRRP_RETURN_CODE_OK != op_ret) {
            vty_out(vty, dcli_vrrp_err_msg[op_ret - DCLI_VRRP_RETURN_CODE_OK]);
		}
	}
	else
	{
		if (dbus_error_is_set(&err))
		{
			printf("%s raised: %s",err.name,err.message);
			dbus_error_free_for_dcli(&err);
		}
	}
	
	 if(uplink_ifname){
        free(uplink_ifname);
		uplink_ifname = NULL;
	}
	if(uplink_ip){
        free(uplink_ip);
		uplink_ip = NULL;
	}
	dbus_message_unref(reply);
	return CMD_SUCCESS;
}
#endif

DEFUN(set_hansi_vrid_cmd_func,
	  set_hansi_vrid_cmd,
	  "set vrid <1-255>",
	  "Set value\n"
	  "Vrrp vrid used in hansi profile\n"
	  "Vrrp vrid value\n"
)

{
	DBusMessage *query = NULL, *reply = NULL;
	DBusError err = {0};
	unsigned int op_ret = 0;
	unsigned int slot_id = HostSlotId;
	unsigned int profile = 0,vrid = 0;
	vrid = strtoul((char *)argv[0],NULL,10);
	if((vrid < 1)||vrid > 255){
        vty_out(vty,"%% Bad parameter : %s !",argv[0]);
		return CMD_WARNING;
	}

	if(HANSI_NODE==vty->node){
		profile = (unsigned int)(vty->index);
		slot_id = vty->slotindex;
	}

	char vrrp_obj_path[DCLI_VRRP_OBJPATH_LEN] = {0};
	char vrrp_dbus_name[DCLI_VRRP_DBUSNAME_LEN] = {0};	

#ifdef DISTRIBUT
	DBusConnection *dcli_dbus_connection = NULL;
	ReInitDbusConnection(&dcli_dbus_connection,slot_id,distributFag);
#endif
	dcli_vrrp_splice_objpath_string(VRRP_DBUS_OBJPATH, profile, vrrp_obj_path);
	dcli_vrrp_splice_dbusname(VRRP_DBUS_BUSNAME, profile, vrrp_dbus_name);
	
	query = dbus_message_new_method_call(vrrp_dbus_name,
										 vrrp_obj_path,
										 VRRP_DBUS_INTERFACE,
										 VRRP_DBUS_METHOD_SET_VRRPID);
	dbus_error_init(&err);

	dbus_message_append_args(query,
		                     DBUS_TYPE_UINT32,&profile,
							 DBUS_TYPE_UINT32,&vrid,						 
							 DBUS_TYPE_INVALID);
	
	reply = dbus_connection_send_with_reply_and_block (dcli_dbus_connection,query,-1, &err);
	dbus_message_unref(query);
	if (NULL == reply) {
		vty_out(vty,"failed get reply.\n");
		if (dbus_error_is_set(&err)) {
			printf("%s raised: %s",err.name,err.message);
			dbus_error_free_for_dcli(&err);
		}
		return CMD_SUCCESS;
	}
	else if (dbus_message_get_args ( reply, &err,
		DBUS_TYPE_UINT32,&op_ret,
		DBUS_TYPE_INVALID)){	
        if( DCLI_VRRP_RETURN_CODE_OK != op_ret){
            vty_out(vty,dcli_vrrp_err_msg[op_ret - DCLI_VRRP_RETURN_CODE_OK]);
		}
		/*dcli_vrrp_notify_to_npd(vty,uplink_ifname,downlink_ifname,add);*/
	} 
	else 
	{		
		if (dbus_error_is_set(&err)) 
		{
			printf("%s raised: %s",err.name,err.message);
			dbus_error_free_for_dcli(&err);
		}
	}
	dbus_message_unref(reply);
	return CMD_SUCCESS;
}


DEFUN(config_vrrp_cmd_func,
	  config_vrrp_cmd,
	  "config uplink IFNAME IP downlink IFNAME IP priority <1-255>",
	  CONFIG_STR
	  "Config uplink\n"
	  "L3 interface name\n"
	  "L3 interface virtual ip\n"
	  "Config downlink\n"
	  "L3 interface name\n"
	  "L3 interface virtual ip\n"
	  "Hansi priority\n"
	  "Hansi priority value\n"
)

{
	DBusMessage *query = NULL, *reply = NULL;
	DBusError err = {0};
	unsigned int op_ret = 0;
	unsigned int slot_id = HostSlotId;
	unsigned int profile = 0,vrid = 0,priority = 0;
	int add = 1;
	char* uplink_ifname = NULL,*downlink_ifname = NULL;
    /*char* uplink_ip = NULL,*downlink_ip = NULL;*/
	unsigned long uplink_ip = 0,downlink_ip = 0;
	unsigned int  uplink_mask = 32,downlink_mask = 32;
	uplink_ifname = (char *)malloc(MAX_IFNAME_LEN);
    if(NULL == uplink_ifname){
       return CMD_WARNING;
	}	
	memset(uplink_ifname,0,MAX_IFNAME_LEN);
	memcpy(uplink_ifname,argv[0],strlen(argv[0]));

	downlink_ifname = (char *)malloc(MAX_IFNAME_LEN);	
	if(NULL == downlink_ifname){
       return CMD_WARNING;
	}
	memset(downlink_ifname,0,MAX_IFNAME_LEN);
	memcpy(downlink_ifname,argv[2],strlen(argv[2]));
#if 0	
	uplink_ip = (char *)malloc(MAX_IPADDR_LEN);
    if(NULL == uplink_ip){
       return CMD_WARNING;
	}	
	memset(uplink_ip,0,MAX_IPADDR_LEN);
	memcpy(uplink_ip,argv[1],strlen(argv[1]));

	downlink_ip = (char *)malloc(MAX_IPADDR_LEN);	
	if(NULL == downlink_ip){
       return CMD_WARNING;
	}
	memset(downlink_ip,0,MAX_IPADDR_LEN);
	memcpy(downlink_ip,argv[3],strlen(argv[3]));
#endif
    uplink_ip = inet_addr((unsigned char*)argv[1]);
    downlink_ip = inet_addr((unsigned char*)argv[3]);
	
	priority = strtoul((char *)argv[4],NULL,10);
	if((priority < 1)||priority > 255){
        vty_out(vty,"%% Bad parameter : %s !",argv[4]);
		return CMD_WARNING;
	}

	if(HANSI_NODE==vty->node){
		profile = (unsigned int)(vty->index);
		slot_id = vty->slotindex;
	}

	char vrrp_obj_path[DCLI_VRRP_OBJPATH_LEN] = {0};
	char vrrp_dbus_name[DCLI_VRRP_DBUSNAME_LEN] = {0};	

#ifdef DISTRIBUT
	DBusConnection *dcli_dbus_connection = NULL;
	ReInitDbusConnection(&dcli_dbus_connection,slot_id,distributFag);
#endif
	dcli_vrrp_splice_objpath_string(VRRP_DBUS_OBJPATH, profile, vrrp_obj_path);
	dcli_vrrp_splice_dbusname(VRRP_DBUS_BUSNAME, profile, vrrp_dbus_name);
	
	query = dbus_message_new_method_call(vrrp_dbus_name,
										 vrrp_obj_path,
										 VRRP_DBUS_INTERFACE,
										 VRRP_DBUS_METHOD_START_VRRP);
	dbus_error_init(&err);

	dbus_message_append_args(query,
		                     DBUS_TYPE_UINT32,&profile,
							 DBUS_TYPE_UINT32,&priority,
							 DBUS_TYPE_STRING,&uplink_ifname,
							 DBUS_TYPE_UINT32,&uplink_ip,
							 DBUS_TYPE_UINT32,&uplink_mask,
							 DBUS_TYPE_STRING,&downlink_ifname,
							 DBUS_TYPE_UINT32,&downlink_ip,	
							 DBUS_TYPE_UINT32,&downlink_mask,	
							 DBUS_TYPE_INVALID);
	
	reply = dbus_connection_send_with_reply_and_block (dcli_dbus_connection,query,-1, &err);
	dbus_message_unref(query);
	if (NULL == reply) {
		vty_out(vty,"failed get reply.\n");
		if (dbus_error_is_set(&err)) {
			printf("%s raised: %s",err.name,err.message);
			dbus_error_free_for_dcli(&err);
		}
		return CMD_SUCCESS;
	}
	else if (dbus_message_get_args ( reply, &err,
		DBUS_TYPE_UINT32,&op_ret,
		DBUS_TYPE_INVALID)){	
        if( DCLI_VRRP_RETURN_CODE_OK != op_ret){
            vty_out(vty,dcli_vrrp_err_msg[op_ret - DCLI_VRRP_RETURN_CODE_OK]);
		}
		/*dcli_vrrp_notify_to_npd(vty,uplink_ifname,downlink_ifname,add);*/
	} 
	else 
	{		
		if (dbus_error_is_set(&err)) 
		{
			printf("%s raised: %s",err.name,err.message);
			dbus_error_free_for_dcli(&err);
		}
	}
	/*release malloc mem*/
	free(uplink_ifname);
	free(downlink_ifname);
	dbus_message_unref(reply);
	return CMD_SUCCESS;
}


DEFUN(config_vrrp_with_mask_cmd_func,
	  config_vrrp_with_mask_cmd,
	  "config uplink IFNAME (A.B.C.D|A.B.C.D/M) downlink IFNAME (A.B.C.D|A.B.C.D/M) priority <1-255>",
	  CONFIG_STR
	  "Config uplink\n"
	  "L3 interface name\n"
	  "L3 interface virtual ip with mask 32\n"
	  "L3 interface virtual ip\n"
	  "Config downlink\n"
	  "L3 interface name\n"
	  "L3 interface virtual ip with mask 32\n"
	  "L3 interface virtual ip\n"
	  "Hansi priority\n"
	  "Hansi priority value\n"
)

{
	DBusMessage *query = NULL, *reply = NULL;
	DBusError err = {0};
	unsigned int op_ret = 0;
	unsigned int slot_id = HostSlotId;
	unsigned int profile = 0,vrid = 0,priority = 0;
	int split = 0;
	char* uplink_ifname = NULL,*downlink_ifname = NULL;
    /*char* uplink_ip = NULL,*downlink_ip = NULL;*/
	unsigned long uplink_ip = 0,downlink_ip = 0;
	unsigned int  uplink_mask = 0,downlink_mask = 0;
	uplink_ifname = (char *)malloc(MAX_IFNAME_LEN);
    if(NULL == uplink_ifname){
       return CMD_WARNING;
	}	
	memset(uplink_ifname,0,MAX_IFNAME_LEN);
	memcpy(uplink_ifname,argv[0],strlen(argv[0]));

	downlink_ifname = (char *)malloc(MAX_IFNAME_LEN);	
	if(NULL == downlink_ifname){
       return CMD_WARNING;
	}
	memset(downlink_ifname,0,MAX_IFNAME_LEN);
	memcpy(downlink_ifname,argv[2],strlen(argv[2]));
#if 0	
	uplink_ip = (char *)malloc(MAX_IPADDR_LEN);
    if(NULL == uplink_ip){
       return CMD_WARNING;
	}	
	memset(uplink_ip,0,MAX_IPADDR_LEN);
	memcpy(uplink_ip,argv[1],strlen(argv[1]));

	downlink_ip = (char *)malloc(MAX_IPADDR_LEN);	
	if(NULL == downlink_ip){
       return CMD_WARNING;
	}
	memset(downlink_ip,0,MAX_IPADDR_LEN);
	memcpy(downlink_ip,argv[3],strlen(argv[3]));
#endif
    op_ret = dcli_vrrp_check_ip_format((char*)argv[1],&split);
    if(CMD_SUCCESS == op_ret){
	   if(0 == split){
		   uplink_ip = (unsigned long)inet_addr((char*)argv[1]);
		   uplink_mask = 32;
	   }
	   else if(1 == split){
		   op_ret = ip_address_format2ulong((char**)&argv[1],&uplink_ip,&uplink_mask); 
		   if(op_ret==CMD_FAILURE){
			   free(uplink_ifname);
			   free(downlink_ifname);
			   return CMD_FAILURE;
		   }
	   }
	}
	else{
	   free(uplink_ifname);
	   free(downlink_ifname);
	   return CMD_FAILURE;		
	}
    op_ret = dcli_vrrp_check_ip_format((char*)argv[3],&split);
    if(CMD_SUCCESS == op_ret){
	   if(0 == split){
		   downlink_ip = (unsigned long)inet_addr((char*)argv[3]);
		   downlink_mask = 32;
	   }
	   else if(1 == split){
		   op_ret = ip_address_format2ulong((char**)&argv[3],&downlink_ip,&downlink_mask); 
		   if(op_ret==CMD_FAILURE){
			   free(uplink_ifname);
			   free(downlink_ifname);
			   return CMD_FAILURE;
		   }
	   }
	}
	else{
	   free(uplink_ifname);
	   free(downlink_ifname);
	   return CMD_FAILURE;		
	}
	priority = strtoul((char *)argv[4],NULL,10);
	if((priority < 1)||priority > 255){
        vty_out(vty,"%% Bad parameter : %s !",argv[4]);
		return CMD_WARNING;
	}

	if(HANSI_NODE==vty->node){
		profile = (unsigned int)(vty->index);
		slot_id = vty->slotindex;
	}

	char vrrp_obj_path[DCLI_VRRP_OBJPATH_LEN] = {0};
	char vrrp_dbus_name[DCLI_VRRP_DBUSNAME_LEN] = {0};	

#ifdef DISTRIBUT
	DBusConnection *dcli_dbus_connection = NULL;
	ReInitDbusConnection(&dcli_dbus_connection,slot_id,distributFag);
#endif
	dcli_vrrp_splice_objpath_string(VRRP_DBUS_OBJPATH, profile, vrrp_obj_path);
	dcli_vrrp_splice_dbusname(VRRP_DBUS_BUSNAME, profile, vrrp_dbus_name);
	
	query = dbus_message_new_method_call(vrrp_dbus_name,
										 vrrp_obj_path,
										 VRRP_DBUS_INTERFACE,
										 VRRP_DBUS_METHOD_START_VRRP);
	dbus_error_init(&err);

	dbus_message_append_args(query,
		                     DBUS_TYPE_UINT32,&profile,
							 DBUS_TYPE_UINT32,&priority,
							 DBUS_TYPE_STRING,&uplink_ifname,
							 DBUS_TYPE_UINT32,&uplink_ip,
							 DBUS_TYPE_UINT32,&uplink_mask,
							 DBUS_TYPE_STRING,&downlink_ifname,
							 DBUS_TYPE_UINT32,&downlink_ip,
							 DBUS_TYPE_UINT32,&downlink_mask,						 
							 DBUS_TYPE_INVALID);
	
	reply = dbus_connection_send_with_reply_and_block (dcli_dbus_connection,query,-1, &err);
	dbus_message_unref(query);
	if (NULL == reply) {
		vty_out(vty,"failed get reply.\n");
		if (dbus_error_is_set(&err)) {
			printf("%s raised: %s",err.name,err.message);
			dbus_error_free_for_dcli(&err);
		}
		return CMD_SUCCESS;
	}
	else if (dbus_message_get_args ( reply, &err,
		DBUS_TYPE_UINT32,&op_ret,
		DBUS_TYPE_INVALID)){	
        if( DCLI_VRRP_RETURN_CODE_OK != op_ret){
            vty_out(vty,dcli_vrrp_err_msg[op_ret - DCLI_VRRP_RETURN_CODE_OK]);
		}
		/*dcli_vrrp_notify_to_npd(vty,uplink_ifname,downlink_ifname,add);*/
	} 
	else 
	{		
		if (dbus_error_is_set(&err)) 
		{
			printf("%s raised: %s",err.name,err.message);
			dbus_error_free_for_dcli(&err);
		}
	}
	/*release malloc mem*/
	free(uplink_ifname);
	free(downlink_ifname);
	/*
	free(uplink_ip);
	free(downlink_ip);
      */
	dbus_message_unref(reply);
	return CMD_SUCCESS;
}

#if 1
/* add by jinpengcheng,
 * for support mode "uplink + heartbeatlink"
 */
/*only uplink*/
DEFUN(config_vrrp_uplink_cmd_func,
	config_vrrp_uplink_cmd,
	"config uplink IFNAME (A.B.C.D|A.B.C.D/M) priority <1-255>",
	CONFIG_STR
	"Config uplink\n"
	"L3 interface name\n"
	"L3 interface virtual ip, use default mask 32 bits\n"
	"L3 interface virtual ip\n"
	"Hansi priority\n"
	"Hansi priority value, valid range [1-255]\n"
)
{
	DBusMessage *query = NULL, *reply = NULL;
	DBusError err = {0};

	unsigned int op_ret = 0;
	unsigned int slot_id = HostSlotId;
	unsigned int profile = 0;
	unsigned int vrid = 0;
	unsigned int priority = 0;
	int add = 1;
	int split = 0;
	char *uplink_ifname = NULL;
	unsigned long uplink_ip = 0;
	unsigned int uplink_mask = 0;
	char vrrp_obj_path[DCLI_VRRP_OBJPATH_LEN] = {0};
	char vrrp_dbus_name[DCLI_VRRP_DBUSNAME_LEN] = {0};	

	uplink_ifname = (char *)malloc(MAX_IFNAME_LEN);	
	if (NULL == uplink_ifname) {
		return CMD_WARNING;
	}
	memset(uplink_ifname, 0, MAX_IFNAME_LEN);
	memcpy(uplink_ifname, argv[0], strlen(argv[0]));
	
	op_ret = dcli_vrrp_check_ip_format((char*)argv[1], &split);
	if (CMD_SUCCESS == op_ret) {
		if (0 == split) {
			/* mask default is 32 */
			uplink_ip = (unsigned long)inet_addr((char*)argv[1]);
			uplink_mask = 32;
		}
		else if (1 == split) {
			op_ret = ip_address_format2ulong((char**)&argv[1], &uplink_ip, &uplink_mask); 
			if (CMD_SUCCESS != op_ret) {
				free(uplink_ifname);
				vty_out(vty, "%% Bad parameter: %s !", argv[1]);
				return CMD_WARNING;
			}
		}
	}
	else {
		free(uplink_ifname);
		vty_out(vty, "%%Illegal IP address %s!", argv[1]);
		return CMD_WARNING;		
	}

	priority = strtoul((char *)argv[2],NULL,10);
	if (priority < 1 ||
		priority > 255) {
		vty_out(vty, "%%error priority %s, valid range [1-255]!", argv[2]);
		return CMD_WARNING;
	}

	if (HANSI_NODE == vty->node) {
		profile = (unsigned int)(vty->index);
		slot_id = vty->slotindex;
	}

#ifdef DISTRIBUT
	DBusConnection *dcli_dbus_connection = NULL;
	ReInitDbusConnection(&dcli_dbus_connection,slot_id,distributFag);
#endif
	dcli_vrrp_splice_objpath_string(VRRP_DBUS_OBJPATH, profile, vrrp_obj_path);
	dcli_vrrp_splice_dbusname(VRRP_DBUS_BUSNAME, profile, vrrp_dbus_name);

	query = dbus_message_new_method_call(vrrp_dbus_name,
										 vrrp_obj_path,
										 VRRP_DBUS_INTERFACE,
										 VRRP_DBUS_METHOD_START_VRRP_UPLINK);
	dbus_error_init(&err);

	dbus_message_append_args(query,
		                     DBUS_TYPE_UINT32, &profile,
							 DBUS_TYPE_UINT32, &priority,
							 DBUS_TYPE_STRING, &uplink_ifname,
							 DBUS_TYPE_UINT32, &uplink_ip,
							 DBUS_TYPE_UINT32, &uplink_mask,
							 DBUS_TYPE_INVALID);
	
	reply = dbus_connection_send_with_reply_and_block(dcli_dbus_connection, query, -1, &err);
	dbus_message_unref(query);
	if (NULL == reply) {
		vty_out(vty, "failed get reply.\n");
		if (dbus_error_is_set(&err)) {
			printf("%s raised: %s", err.name, err.message);
			dbus_error_free_for_dcli(&err);
		}
		return CMD_SUCCESS;
	}
	else if (dbus_message_get_args(reply, &err,
									DBUS_TYPE_UINT32, &op_ret,
									DBUS_TYPE_INVALID))
	{	
        if (DCLI_VRRP_RETURN_CODE_OK != op_ret) {
            vty_out(vty, dcli_vrrp_err_msg[op_ret - DCLI_VRRP_RETURN_CODE_OK]);
		}
	} 
	else {		
		if (dbus_error_is_set(&err)) 
		{
			printf("%s raised: %s",err.name,err.message);
			dbus_error_free_for_dcli(&err);
		}
	}
	/*release malloc mem*/
	free(uplink_ifname);
	dbus_message_unref(reply);
	return CMD_SUCCESS;
}

#endif

/*only downlink*/
DEFUN(config_vrrp_downlink_cmd_func,
	  config_vrrp_downlink_cmd,
	  "config downlink IFNAME (A.B.C.D|A.B.C.D/M) priority <1-255>",
	  CONFIG_STR
	  "Config downlink\n"
	  "L3 interface name\n"
	  "L3 interface virtual ip, use default mask 32 bits\n"
	  "L3 interface virtual ip\n"
	  "Hansi priority\n"
	  "Hansi priority value\n"
)

{
	DBusMessage *query = NULL, *reply = NULL;
	DBusError err = {0};
	unsigned int op_ret = 0;
	unsigned int slot_id = HostSlotId;
	unsigned int profile = 0,vrid = 0,priority = 0;
	int add = 1;
	int split = 0;
	char* downlink_ifname = NULL;
    unsigned  long downlink_ip = 0;
	unsigned int downlink_mask = 0;

	downlink_ifname = (char *)malloc(MAX_IFNAME_LEN);	
	if(NULL == downlink_ifname){
       return CMD_WARNING;
	}
	memset(downlink_ifname,0,MAX_IFNAME_LEN);
	memcpy(downlink_ifname,argv[0],strlen(argv[0]));
	
    op_ret = dcli_vrrp_check_ip_format((char*)argv[1],&split);
    if(CMD_SUCCESS == op_ret){
	   if(0 == split){
		   downlink_ip = (unsigned long)inet_addr((char*)argv[1]);
		   downlink_mask = 32;
	   }
	   else if(1 == split){
		   op_ret = ip_address_format2ulong((char**)&argv[1],&downlink_ip,&downlink_mask); 
		   if(op_ret==CMD_FAILURE){
			   free(downlink_ifname);
			   return CMD_FAILURE;
		   }
	   }
	}
	else{
	   free(downlink_ifname);
	   return CMD_FAILURE;		
	}


	priority = strtoul((char *)argv[2],NULL,10);
	if((priority < 1)||priority > 255){
        vty_out(vty,"%% Bad parameter : %s !",argv[2]);
		return CMD_WARNING;
	}

	if(HANSI_NODE==vty->node){
		profile = (unsigned int)(vty->index);
		slot_id = vty->slotindex;
	}

	char vrrp_obj_path[DCLI_VRRP_OBJPATH_LEN] = {0};
	char vrrp_dbus_name[DCLI_VRRP_DBUSNAME_LEN] = {0};	

#ifdef DISTRIBUT
	DBusConnection *dcli_dbus_connection = NULL;
	ReInitDbusConnection(&dcli_dbus_connection,slot_id,distributFag);
#endif
	dcli_vrrp_splice_objpath_string(VRRP_DBUS_OBJPATH, profile, vrrp_obj_path);
	dcli_vrrp_splice_dbusname(VRRP_DBUS_BUSNAME, profile, vrrp_dbus_name);

	query = dbus_message_new_method_call(vrrp_dbus_name,
										 vrrp_obj_path,
										 VRRP_DBUS_INTERFACE,
										 VRRP_DBUS_METHOD_START_VRRP_DOWNLINK);
	dbus_error_init(&err);

	dbus_message_append_args(query,
		                     DBUS_TYPE_UINT32,&profile,
							 DBUS_TYPE_UINT32,&priority,
							 DBUS_TYPE_STRING,&downlink_ifname,
							 DBUS_TYPE_UINT32,&downlink_ip,
							 DBUS_TYPE_UINT32,&downlink_mask,
							 DBUS_TYPE_INVALID);
	
	reply = dbus_connection_send_with_reply_and_block (dcli_dbus_connection,query,-1, &err);
	dbus_message_unref(query);
	if (NULL == reply) {
		vty_out(vty,"failed get reply.\n");
		if (dbus_error_is_set(&err)) {
			printf("%s raised: %s",err.name,err.message);
			dbus_error_free_for_dcli(&err);
		}
		return CMD_SUCCESS;
	}
	else if (dbus_message_get_args ( reply, &err,
		DBUS_TYPE_UINT32,&op_ret,
		DBUS_TYPE_INVALID)){	
        if( DCLI_VRRP_RETURN_CODE_OK != op_ret){
            vty_out(vty,dcli_vrrp_err_msg[op_ret - DCLI_VRRP_RETURN_CODE_OK]);
		}
		
		/*dcli_vrrp_notify_to_npd(vty,NULL,downlink_ifname,add);*/
	} 
	else 
	{		
		if (dbus_error_is_set(&err)) 
		{
			printf("%s raised: %s",err.name,err.message);
			dbus_error_free_for_dcli(&err);
		}
	}
	/*release malloc mem*/
	free(downlink_ifname);
	dbus_message_unref(reply);
	return CMD_SUCCESS;
}

/*
 *******************************************************************************
 *config_vrrp_link_add_vip_cmd_func()
 *
 *  DESCRIPTION:
 *		add uplink|downlink virtual ip
 *
 *******************************************************************************
 */
DEFUN(config_vrrp_link_add_vip_cmd_func,
	config_vrrp_link_add_vip_cmd,
	"add (uplink|downlink|vgateway) IFNAME (A.B.C.D|A.B.C.D/M)",
	"Add l3 interface virtual ip\n"
	"Config uplink interface\n"
	"Config downlink interface\n"
	"Config vageway interface\n"
	"L3 interface name\n"
	"L3 interface virtual ip, use default mask 32 bits\n"
	"L3 interface virtual ip\n"
)
{
	DBusMessage *query = NULL;
	DBusMessage *reply = NULL;
	DBusError err = {0};

	unsigned int op_ret = 0;
	unsigned int slot_id = HostSlotId;
	unsigned int profile = 0;
	int add = 1;
	int split = 0;
	unsigned opt_type = DCLI_VRRP_VIP_OPT_TYPE_ADD;
	unsigned link_type = DCLI_VRRP_LINK_TYPE_INVALID;
	char *ifname = NULL;
	unsigned long virtual_ip = 0;
	unsigned int mask = 0;
	char vrrp_obj_path[DCLI_VRRP_OBJPATH_LEN] = {0};
	char vrrp_dbus_name[DCLI_VRRP_DBUSNAME_LEN] = {0};	

	if (!strncmp(argv[0], "uplink", strlen(argv[0]))) {
		link_type = DCLI_VRRP_LINK_TYPE_UPLINK;
	}else if (!strncmp(argv[0], "downlink", strlen(argv[0]))) {
		link_type = DCLI_VRRP_LINK_TYPE_DOWNLINK;
	}
	else if(!strncmp(argv[0], "vgateway", strlen(argv[0]))) {
		link_type = DCLI_VRRP_LINK_TYPE_VGATEWAY;
	}else {
		vty_out(vty, "%% Unknown link type %s!\n", argv[0]);
		return CMD_WARNING;
	}

	ifname = (char *)malloc(MAX_IFNAME_LEN);	
	if (NULL == ifname) {
		return CMD_WARNING;
	}
	memset(ifname, 0, MAX_IFNAME_LEN);
	memcpy(ifname, argv[1], strlen(argv[1]));
	
	op_ret = dcli_vrrp_check_ip_format((char*)argv[2], &split);
	if (CMD_SUCCESS == op_ret)
	{
		if (0 == split)
		{
			/* mask default is 32 */
			virtual_ip = (unsigned long)inet_addr((char*)argv[2]);
			mask = 32;
		}
		else if (1 == split)
		{
			op_ret = ip_address_format2ulong((char**)&argv[2], &virtual_ip, &mask); 
			if (CMD_SUCCESS != op_ret) {
				free(ifname);
				vty_out(vty, "%% Bad parameter: %s !", argv[2]);
				return CMD_WARNING;
			}
		}
	}
	else {
		free(ifname);
		vty_out(vty, "%%Illegal IP address %s!", argv[2]);
		return CMD_WARNING;		
	}


	if (HANSI_NODE == vty->node) {
		profile = (unsigned int)(vty->index);
		slot_id = vty->slotindex;
	}

#ifdef DISTRIBUT
	DBusConnection *dcli_dbus_connection = NULL;
	ReInitDbusConnection(&dcli_dbus_connection,slot_id,distributFag);
#endif
	dcli_vrrp_splice_objpath_string(VRRP_DBUS_OBJPATH, profile, vrrp_obj_path);
	dcli_vrrp_splice_dbusname(VRRP_DBUS_BUSNAME, profile, vrrp_dbus_name);

	query = dbus_message_new_method_call(vrrp_dbus_name,
										 vrrp_obj_path,
										 VRRP_DBUS_INTERFACE,
										 VRRP_DBUS_METHOD_VRRP_LINK_ADD_DEL_VIP);
	dbus_error_init(&err);

	dbus_message_append_args(query,
		                     DBUS_TYPE_UINT32, &profile,
							 DBUS_TYPE_UINT32, &opt_type,		                     
							 DBUS_TYPE_UINT32, &link_type,
							 DBUS_TYPE_STRING, &ifname,
							 DBUS_TYPE_UINT32, &virtual_ip,
							 DBUS_TYPE_UINT32, &mask,
							 DBUS_TYPE_INVALID);
	
	reply = dbus_connection_send_with_reply_and_block(dcli_dbus_connection, query, -1, &err);
	dbus_message_unref(query);
	if (NULL == reply) {
		vty_out(vty, "failed get reply.\n");
		if (dbus_error_is_set(&err)) {
			printf("%s raised: %s", err.name, err.message);
			dbus_error_free_for_dcli(&err);
		}
		return CMD_SUCCESS;
	}
	else if (dbus_message_get_args(reply, &err,
									DBUS_TYPE_UINT32, &op_ret,
									DBUS_TYPE_INVALID))
	{
        if (DCLI_VRRP_RETURN_CODE_OK != op_ret) {
            vty_out(vty, dcli_vrrp_err_msg[op_ret - DCLI_VRRP_RETURN_CODE_OK]);
		}
	}
	else
	{
		if (dbus_error_is_set(&err))
		{
			printf("%s raised: %s", err.name, err.message);
			dbus_error_free_for_dcli(&err);
		}
	}
	
	/*release malloc mem*/
	free(ifname);
	dbus_message_unref(reply);
	return CMD_SUCCESS;
}

/*
 *******************************************************************************
 *config_vrrp_link_del_vip_cmd_func()
 *
 *  DESCRIPTION:
 *		del uplink|downlink virtual ip
 *
 *******************************************************************************
 */
DEFUN(config_vrrp_link_del_vip_cmd_func,
	config_vrrp_link_del_vip_cmd,
	"delete (uplink|downlink|vgateway) IFNAME",
	"Delete l3 interface virtual ip\n"
	"Config uplink interface\n"
	"Config downlink interface\n"
	"Config vageway interface\n"
	"L3 interface name\n"
	"L3 interface virtual ip, use default mask 32 bits\n"
	"L3 interface virtual ip\n"
)
{
	DBusMessage *query = NULL;
	DBusMessage *reply = NULL;
	DBusError err = {0};

	unsigned int op_ret = 0;
	unsigned int slot_id = HostSlotId;
	unsigned int profile = 0;
	int add = 1;
	int split = 0;
	unsigned opt_type = DCLI_VRRP_VIP_OPT_TYPE_DEL;
	unsigned link_type = DCLI_VRRP_LINK_TYPE_INVALID;
	char *ifname = NULL;
	unsigned long virtual_ip = 0;
	unsigned int mask = 0;
	char vrrp_obj_path[DCLI_VRRP_OBJPATH_LEN] = {0};
	char vrrp_dbus_name[DCLI_VRRP_DBUSNAME_LEN] = {0};	

	if (!strncmp(argv[0], "uplink", strlen(argv[0]))) {
		link_type = DCLI_VRRP_LINK_TYPE_UPLINK;
	}else if (!strncmp(argv[0], "downlink", strlen(argv[0]))) {
		link_type = DCLI_VRRP_LINK_TYPE_DOWNLINK;
	}
	else if(!strncmp(argv[0], "vgateway", strlen(argv[0]))) {
		link_type = DCLI_VRRP_LINK_TYPE_VGATEWAY;
	}else {
		vty_out(vty, "%% Unknown link type %s!\n", argv[0]);
		return CMD_WARNING;
	}

	ifname = (char *)malloc(MAX_IFNAME_LEN);	
	if (NULL == ifname) {
		return CMD_WARNING;
	}
	memset(ifname, 0, MAX_IFNAME_LEN);
	memcpy(ifname, argv[1], strlen(argv[1]));
	
	if (HANSI_NODE == vty->node) {
		profile = (unsigned int)(vty->index);
		slot_id = vty->slotindex;
	}

#ifdef DISTRIBUT
	DBusConnection *dcli_dbus_connection = NULL;
	ReInitDbusConnection(&dcli_dbus_connection,slot_id,distributFag);
#endif
	dcli_vrrp_splice_objpath_string(VRRP_DBUS_OBJPATH, profile, vrrp_obj_path);
	dcli_vrrp_splice_dbusname(VRRP_DBUS_BUSNAME, profile, vrrp_dbus_name);

	query = dbus_message_new_method_call(vrrp_dbus_name,
										 vrrp_obj_path,
										 VRRP_DBUS_INTERFACE,
										 VRRP_DBUS_METHOD_VRRP_LINK_DEL_VIP);
	dbus_error_init(&err);

	dbus_message_append_args(query,
		                     DBUS_TYPE_UINT32, &profile,
							 DBUS_TYPE_UINT32, &opt_type,		                     
							 DBUS_TYPE_UINT32, &link_type,
							 DBUS_TYPE_STRING, &ifname,
							 DBUS_TYPE_UINT32, &virtual_ip,
							 DBUS_TYPE_UINT32, &mask,
							 DBUS_TYPE_INVALID);
	
	reply = dbus_connection_send_with_reply_and_block(dcli_dbus_connection, query, -1, &err);
	dbus_message_unref(query);
	if (NULL == reply) {
		vty_out(vty, "failed get reply.\n");
		if (dbus_error_is_set(&err)) {
			printf("%s raised: %s", err.name, err.message);
			dbus_error_free_for_dcli(&err);
		}
		return CMD_SUCCESS;
	}
	else if (dbus_message_get_args(reply, &err,
									DBUS_TYPE_UINT32, &op_ret,
									DBUS_TYPE_INVALID))
	{
        if (DCLI_VRRP_RETURN_CODE_OK != op_ret) {
            vty_out(vty, dcli_vrrp_err_msg[op_ret - DCLI_VRRP_RETURN_CODE_OK]);
		}
	}
	else
	{
		if (dbus_error_is_set(&err))
		{
			printf("%s raised: %s", err.name, err.message);
			dbus_error_free_for_dcli(&err);
		}
	}
	
	/*release malloc mem*/
	free(ifname);
	dbus_message_unref(reply);
	return CMD_SUCCESS;
}

DEFUN(config_vrrp_start_state_cmd_func,
	  config_vrrp_start_state_cmd,
	  "config start-state master (yes|no)",
	  CONFIG_STR
	  "Config hansi instance state when start\n"
	  "Config state to master\n"
	  "Config state setting\n"
	  "Cancel state setting\n"
)

{
	DBusMessage *query = NULL, *reply = NULL;
	DBusError err = {0};
	unsigned int op_ret = 0,enable = 0;
	unsigned int profile = 0;
	unsigned int slot_id = HostSlotId;

	if (!strncmp(argv[0], "yes", strlen(argv[0]))) {
        enable = 1;
	}
	else if (!strncmp(argv[0], "no", strlen(argv[0]))) {
        enable = 0;
	}
	else {
		vty_out(vty, "%% Bad parameter %s!\n", argv[0]);
		return CMD_WARNING;
	}

	if(HANSI_NODE==vty->node){
		profile = (unsigned int)(vty->index);
		slot_id = vty->slotindex;
	}

	char vrrp_obj_path[DCLI_VRRP_OBJPATH_LEN] = {0};
	char vrrp_dbus_name[DCLI_VRRP_DBUSNAME_LEN] = {0};	

#ifdef DISTRIBUT
	DBusConnection *dcli_dbus_connection = NULL;
	ReInitDbusConnection(&dcli_dbus_connection,slot_id,distributFag);
#endif
	dcli_vrrp_splice_objpath_string(VRRP_DBUS_OBJPATH, profile, vrrp_obj_path);
	dcli_vrrp_splice_dbusname(VRRP_DBUS_BUSNAME, profile, vrrp_dbus_name);

	query = dbus_message_new_method_call(vrrp_dbus_name,
										 vrrp_obj_path,
										 VRRP_DBUS_INTERFACE,
										 VRRP_DBUS_METHOD_VRRP_WANT_STATE);
	dbus_error_init(&err);

	dbus_message_append_args(query,
		                     DBUS_TYPE_UINT32,&profile,	
		                     DBUS_TYPE_UINT32,&enable,
							 DBUS_TYPE_INVALID);
	
	reply = dbus_connection_send_with_reply_and_block (dcli_dbus_connection,query,-1, &err);
	dbus_message_unref(query);
	if (NULL == reply) {
		vty_out(vty,"failed get reply.\n");
		if (dbus_error_is_set(&err)) {
			printf("%s raised: %s",err.name,err.message);
			dbus_error_free_for_dcli(&err);
		}
		return CMD_SUCCESS;
	}
	else if (dbus_message_get_args ( reply, &err,
		DBUS_TYPE_UINT32,&op_ret,
		DBUS_TYPE_INVALID)){	
        if( DCLI_VRRP_RETURN_CODE_OK != op_ret){
            vty_out(vty,dcli_vrrp_err_msg[op_ret - DCLI_VRRP_RETURN_CODE_OK]);
		}
		
	} 
	else 
	{		
		if (dbus_error_is_set(&err)) 
		{
			printf("%s raised: %s",err.name,err.message);
			dbus_error_free_for_dcli(&err);
		}
	}
	dbus_message_unref(reply);
	return CMD_SUCCESS;
}


DEFUN(config_vrrp_service_cmd_func,
	  config_vrrp_service_cmd,
	  "config service (enable|disable)",
	  CONFIG_STR
	  "Config vrrp service\n"
	  "Enable service\n"
	  "Disable service\n"
)

{
	DBusMessage *query = NULL, *reply = NULL;
	DBusError err = {0};
	unsigned int op_ret = 0;
	unsigned int slot_id = HostSlotId;
	unsigned int profile = 0,vrid = 0,priority = 0;
	int enable = -1;

	if (!strncmp(argv[0], "enable", strlen(argv[0]))) {
        enable = 1;
	}
	else if (!strncmp(argv[0], "disable", strlen(argv[0]))) {
        enable = 0;
	}
	else {
		vty_out(vty, "%% Bad parameter %s!\n", argv[0]);
		return CMD_WARNING;
	}

	if(HANSI_NODE==vty->node){
		profile = (unsigned int)(vty->index);
		slot_id = vty->slotindex;
	}
	
	char vrrp_obj_path[DCLI_VRRP_OBJPATH_LEN] = {0};
	char vrrp_dbus_name[DCLI_VRRP_DBUSNAME_LEN] = {0};	

#ifdef DISTRIBUT
	DBusConnection *dcli_dbus_connection = NULL;
	ReInitDbusConnection(&dcli_dbus_connection,slot_id,distributFag);
#endif
	dcli_vrrp_splice_objpath_string(VRRP_DBUS_OBJPATH, profile, vrrp_obj_path);
	dcli_vrrp_splice_dbusname(VRRP_DBUS_BUSNAME, profile, vrrp_dbus_name);

	query = dbus_message_new_method_call(vrrp_dbus_name,
										 vrrp_obj_path,
										 VRRP_DBUS_INTERFACE,
										 VRRP_DBUS_METHOD_VRRP_SERVICE_ENABLE);
	dbus_error_init(&err);

	dbus_message_append_args(query,
		                     DBUS_TYPE_UINT32,&profile,
							 DBUS_TYPE_UINT32,&enable,							 
							 DBUS_TYPE_INVALID);
	                                                            /*time out value 150 seconds*/
	reply = dbus_connection_send_with_reply_and_block (dcli_dbus_connection,query,150000, &err);
	dbus_message_unref(query);
	if (NULL == reply) {
		vty_out(vty,"failed get reply.\n");
		if (dbus_error_is_set(&err)) {
			printf("%s raised: %s",err.name,err.message);
			dbus_error_free_for_dcli(&err);
		}
		return CMD_SUCCESS;
	}
	else if (dbus_message_get_args ( reply, &err,
		DBUS_TYPE_UINT32,&op_ret,
		DBUS_TYPE_INVALID)){	
        if( DCLI_VRRP_RETURN_CODE_OK != op_ret){
            vty_out(vty,dcli_vrrp_err_msg[op_ret - DCLI_VRRP_RETURN_CODE_OK]);
		}
		
	} 
	else 
	{		
		if (dbus_error_is_set(&err)) 
		{
			printf("%s raised: %s",err.name,err.message);
			dbus_error_free_for_dcli(&err);
		}
	}
	dbus_message_unref(reply);
	return CMD_SUCCESS;
}


DEFUN(config_vrrp_gateway_cmd_func,
	  config_vrrp_gateway_cmd,
	  "config vgateway IFNAME A.B.C.D/M",
	  CONFIG_STR
	  "Virtual gateway\n"
	  "L3 interface name\n"
	  "L3 interface virtual ip\n"
)

{
	DBusMessage *query = NULL, *reply = NULL;
	DBusError err = {0};
	unsigned int op_ret = 0;
	unsigned int slot_id = HostSlotId;
	unsigned int profile = 0,vrid = 0,priority = 0;
	int add = 1;
	char* vgateway_ifname = NULL;
    char* vgateway_ip = NULL;
    unsigned long dipno = 0;
	unsigned int dipmaskLen = 0;
	vgateway_ifname = (char *)malloc(MAX_IFNAME_LEN);	
	if(NULL == vgateway_ifname){
       return CMD_WARNING;
	}
	memset(vgateway_ifname,0,MAX_IFNAME_LEN);
	memcpy(vgateway_ifname,argv[0],strlen(argv[0]));
#if 0	
	vgateway_ip = (char *)malloc(MAX_IPADDR_LEN);	
	if(NULL == vgateway_ip){
       return CMD_WARNING;
	}
	memset(vgateway_ip,0,MAX_IPADDR_LEN);
	memcpy(vgateway_ip,argv[1],strlen(argv[1]));
#endif
	op_ret = ip_address_format2ulong((char**)&argv[1],&dipno,&dipmaskLen);	
	if(op_ret==CMD_FAILURE)  return CMD_FAILURE;

	if(HANSI_NODE==vty->node){
		profile = (unsigned int)(vty->index);
		slot_id = vty->slotindex;
	}

	char vrrp_obj_path[DCLI_VRRP_OBJPATH_LEN] = {0};
	char vrrp_dbus_name[DCLI_VRRP_DBUSNAME_LEN] = {0};	

#ifdef DISTRIBUT
	DBusConnection *dcli_dbus_connection = NULL;
	ReInitDbusConnection(&dcli_dbus_connection,slot_id,distributFag);
#endif
	dcli_vrrp_splice_objpath_string(VRRP_DBUS_OBJPATH, profile, vrrp_obj_path);
	dcli_vrrp_splice_dbusname(VRRP_DBUS_BUSNAME, profile, vrrp_dbus_name);

	query = dbus_message_new_method_call(vrrp_dbus_name,
										 vrrp_obj_path,
										 VRRP_DBUS_INTERFACE,
										 VRRP_DBUS_METHOD_V_GATEWAY);
	dbus_error_init(&err);

	dbus_message_append_args(query,
		                     DBUS_TYPE_UINT32,&profile,
							 DBUS_TYPE_STRING,&vgateway_ifname,
							 DBUS_TYPE_UINT32,&dipno,	
							 DBUS_TYPE_UINT32,&dipmaskLen,
							 DBUS_TYPE_INVALID);
	
	reply = dbus_connection_send_with_reply_and_block (dcli_dbus_connection,query,-1, &err);
	dbus_message_unref(query);
	if (NULL == reply) {
		vty_out(vty,"failed get reply.\n");
		if (dbus_error_is_set(&err)) {
			printf("%s raised: %s",err.name,err.message);
			dbus_error_free_for_dcli(&err);
		}
		return CMD_SUCCESS;
	}
	else if (dbus_message_get_args ( reply, &err,
		DBUS_TYPE_UINT32,&op_ret,
		DBUS_TYPE_INVALID)){	
        if( DCLI_VRRP_RETURN_CODE_OK != op_ret){
			   vty_out(vty,dcli_vrrp_err_msg[op_ret - DCLI_VRRP_RETURN_CODE_OK]);
		}
	} 
	else 
	{		
		if (dbus_error_is_set(&err)) 
		{
			printf("%s raised: %s",err.name,err.message);
			dbus_error_free_for_dcli(&err);
		}
	}
	/*release malloc mem*/
	free(vgateway_ifname);
	dbus_message_unref(reply);
	return CMD_SUCCESS;
}


DEFUN(cancel_vrrp_gateway_cmd_func,
	  cancel_vrrp_gateway_cmd,
	  "no vgateway IFNAME A.B.C.D/M",
	  "Cancel order\n"
	  "Virtual gateway\n"
	  "L3 interface name\n"
	  "L3 interface virtual ip\n"
)

{
	DBusMessage *query = NULL, *reply = NULL;
	DBusError err = {0};
	unsigned int op_ret = 0;
	unsigned int slot_id = HostSlotId;
	unsigned int profile = 0,vrid = 0,priority = 0;
	int add = 1;
	char* vgateway_ifname = NULL;
    char* vgateway_ip = NULL;
    unsigned long dipno = 0;
	unsigned int dipmaskLen = 0;
	vgateway_ifname = (char *)malloc(MAX_IFNAME_LEN);	
	if(NULL == vgateway_ifname){
       return CMD_WARNING;
	}
	memset(vgateway_ifname,0,MAX_IFNAME_LEN);
	memcpy(vgateway_ifname,argv[0],strlen(argv[0]));
#if 0	
	vgateway_ip = (char *)malloc(MAX_IPADDR_LEN);	
	if(NULL == vgateway_ip){
       return CMD_WARNING;
	}
	memset(vgateway_ip,0,MAX_IPADDR_LEN);
	memcpy(vgateway_ip,argv[1],strlen(argv[1]));
#endif
	op_ret = ip_address_format2ulong((char**)&argv[1],&dipno,&dipmaskLen);	
	if(op_ret==CMD_FAILURE)  return CMD_FAILURE;

	if(HANSI_NODE==vty->node){
		profile = (unsigned int)(vty->index);
		slot_id = vty->slotindex;
	}
	vty_out(vty,"vgateway ifname %s,ip %d.%d.%d.%d,masklen %d\n",vgateway_ifname,\
		(dipno & 0xff000000)>>24,(dipno & 0xff0000)>>16,(dipno & 0xff00)>>8,dipno & 0xff,dipmaskLen);

	char vrrp_obj_path[DCLI_VRRP_OBJPATH_LEN] = {0};
	char vrrp_dbus_name[DCLI_VRRP_DBUSNAME_LEN] = {0};	

#ifdef DISTRIBUT
	DBusConnection *dcli_dbus_connection = NULL;
	ReInitDbusConnection(&dcli_dbus_connection,slot_id,distributFag);
#endif
	dcli_vrrp_splice_objpath_string(VRRP_DBUS_OBJPATH, profile, vrrp_obj_path);
	dcli_vrrp_splice_dbusname(VRRP_DBUS_BUSNAME, profile, vrrp_dbus_name);
	
	query = dbus_message_new_method_call(vrrp_dbus_name,
										 vrrp_obj_path,
										 VRRP_DBUS_INTERFACE,
										 VRRP_DBUS_METHOD_NO_V_GATEWAY);
	dbus_error_init(&err);

	dbus_message_append_args(query,
		                     DBUS_TYPE_UINT32,&profile,
							 DBUS_TYPE_STRING,&vgateway_ifname,
							 DBUS_TYPE_UINT32,&dipno,	
							 DBUS_TYPE_UINT32,&dipmaskLen,
							 DBUS_TYPE_INVALID);
	
	reply = dbus_connection_send_with_reply_and_block (dcli_dbus_connection,query,-1, &err);
	dbus_message_unref(query);
	if (NULL == reply) {
		vty_out(vty,"failed get reply.\n");
		if (dbus_error_is_set(&err)) {
			printf("%s raised: %s",err.name,err.message);
			dbus_error_free_for_dcli(&err);
		}
		return CMD_SUCCESS;
	}
	else if (dbus_message_get_args ( reply, &err,
		DBUS_TYPE_UINT32,&op_ret,
		DBUS_TYPE_INVALID)){	
        if( DCLI_VRRP_RETURN_CODE_OK != op_ret){
            vty_out(vty,dcli_vrrp_err_msg[op_ret - DCLI_VRRP_RETURN_CODE_OK]);
		}
		
		/*dcli_vrrp_notify_to_npd(vty,NULL,downlink_ifname,add);*/
	} 
	else 
	{		
		if (dbus_error_is_set(&err)) 
		{
			printf("%s raised: %s",err.name,err.message);
			dbus_error_free_for_dcli(&err);
		}
	}
	/*release malloc mem*/
	free(vgateway_ifname);
	dbus_message_unref(reply);
	return CMD_SUCCESS;
}


DEFUN(config_vrrp_end_cmd_func,
	  config_vrrp_end_cmd,
	  "no hansi",	  
	  "Cancel hansi\n"
	  "Config hansi\n"
)

{
	DBusMessage *query = NULL, *reply = NULL;
	DBusError err = {0};
	unsigned int op_ret = 0;
	unsigned int slot_id = HostSlotId;
	unsigned int profile = 0,vrid = 0;
	char *uplink_ifname = NULL,*downlink_ifname = NULL;
	int add = 0;


	if(HANSI_NODE==vty->node){
		profile = (unsigned int)(vty->index);
		slot_id = vty->slotindex;
	}
#if 0
	uplink_ifname = (char*)malloc(MAX_IFNAME_LEN);
	if(NULL == uplink_ifname){
       return CMD_WARNING;
	}
	downlink_ifname = (char*)malloc(MAX_IFNAME_LEN);
	if(NULL == downlink_ifname){
		free(uplink_ifname);
		return CMD_WARNING;
	}
    op_ret = dcli_vrrp_get_if_by_profile(vty,profile,uplink_ifname,downlink_ifname);
	if(DCLI_VRRP_RETURN_CODE_OK == op_ret){
		if(0 == strcmp(uplink_ifname,downlink_ifname)){
			dcli_vrrp_notify_to_npd(vty,NULL,downlink_ifname,add);           
		}
		else {
		    dcli_vrrp_notify_to_npd(vty,uplink_ifname,downlink_ifname,add);
		}
    } 
	free(uplink_ifname);
	free(downlink_ifname);
#endif

	char vrrp_obj_path[DCLI_VRRP_OBJPATH_LEN] = {0};
	char vrrp_dbus_name[DCLI_VRRP_DBUSNAME_LEN] = {0};	

#ifdef DISTRIBUT
	DBusConnection *dcli_dbus_connection = NULL;
	ReInitDbusConnection(&dcli_dbus_connection,slot_id,distributFag);
#endif
	dcli_vrrp_splice_objpath_string(VRRP_DBUS_OBJPATH, profile, vrrp_obj_path);
	dcli_vrrp_splice_dbusname(VRRP_DBUS_BUSNAME, profile, vrrp_dbus_name);

	query = dbus_message_new_method_call(vrrp_dbus_name,
										 vrrp_obj_path,
										 VRRP_DBUS_INTERFACE,
										 VRRP_DBUS_METHOD_END_VRRP);
	dbus_error_init(&err);

	dbus_message_append_args(query,
		                     DBUS_TYPE_UINT32,&profile,							 
							 DBUS_TYPE_INVALID);
	
	reply = dbus_connection_send_with_reply_and_block (dcli_dbus_connection,query,-1, &err);
	dbus_message_unref(query);
	if (NULL == reply) {
		vty_out(vty,"failed get reply.\n");
		if (dbus_error_is_set(&err)) {
			printf("%s raised: %s",err.name,err.message);
			dbus_error_free_for_dcli(&err);
		}
		return CMD_SUCCESS;
	}
	if (dbus_message_get_args ( reply, &err,
		DBUS_TYPE_UINT32,&op_ret,
		DBUS_TYPE_INVALID)){	
        if( DCLI_VRRP_RETURN_CODE_OK != op_ret){
            vty_out(vty,dcli_vrrp_err_msg[op_ret - DCLI_VRRP_RETURN_CODE_OK]);
		}
	}
	else 
	{		
		if (dbus_error_is_set(&err)) 
		{
			printf("%s raised: %s",err.name,err.message);
			dbus_error_free_for_dcli(&err);
		}
	}
	dbus_message_unref(reply);
	return CMD_SUCCESS;
}




DEFUN(cancel_vrrp_transfer_cmd_func,
	  cancel_vrrp_transfer_cmd,
	  "no transfer state",
	  "Cancel order\n"
	  "Transfer state\n"
	  "Vrrp state\n"
)

{
	DBusMessage *query = NULL, *reply = NULL;
	DBusError err = {0};
	unsigned int op_ret = 0;
	unsigned int slot_id = HostSlotId;
	unsigned int profile = 0,vrid = 0,priority = 0;

	if(HANSI_NODE==vty->node){
		profile = (unsigned int)(vty->index);
		slot_id = vty->slotindex;
	}

	char vrrp_obj_path[DCLI_VRRP_OBJPATH_LEN] = {0};
	char vrrp_dbus_name[DCLI_VRRP_DBUSNAME_LEN] = {0};	

#ifdef DISTRIBUT
	DBusConnection *dcli_dbus_connection = NULL;
	ReInitDbusConnection(&dcli_dbus_connection,slot_id,distributFag);
#endif
	dcli_vrrp_splice_objpath_string(VRRP_DBUS_OBJPATH, profile, vrrp_obj_path);
	dcli_vrrp_splice_dbusname(VRRP_DBUS_BUSNAME, profile, vrrp_dbus_name);

	query = dbus_message_new_method_call(vrrp_dbus_name,
										 vrrp_obj_path,
										 VRRP_DBUS_INTERFACE,
										 VRRP_DBUS_METHOD_NO_TRANSFER);
	dbus_error_init(&err);

	dbus_message_append_args(query,
		                     DBUS_TYPE_UINT32,&profile,
							 DBUS_TYPE_INVALID);
	
	reply = dbus_connection_send_with_reply_and_block (dcli_dbus_connection,query,-1, &err);
	dbus_message_unref(query);
	if (NULL == reply) {
		vty_out(vty,"failed get reply.\n");
		if (dbus_error_is_set(&err)) {
			printf("%s raised: %s",err.name,err.message);
			dbus_error_free_for_dcli(&err);
		}
		return CMD_SUCCESS;
	}
	else if (dbus_message_get_args ( reply, &err,
		DBUS_TYPE_UINT32,&op_ret,
		DBUS_TYPE_INVALID)){	
           ;		
	} 
	else 
	{		
		if (dbus_error_is_set(&err)) 
		{
			printf("%s raised: %s",err.name,err.message);
			dbus_error_free_for_dcli(&err);
		}
	}
	dbus_message_unref(reply);
	return CMD_SUCCESS;
}

DEFUN(config_vrrp_priority_cmd_func,
	  config_vrrp_priority_cmd,
	  "config hansi priority <1-255>",
	  CONFIG_STR
	  "Config hansi\n"
	  "Config hansi priority\n"
	  "Priority value range\n"
)

{
	DBusMessage *query = NULL, *reply = NULL;
	DBusError err = {0};
	unsigned int op_ret = 0;
	unsigned int slot_id = HostSlotId;
	unsigned int profile = 0,vrid = 0,priority = 0,preempt = 0;

	priority = strtoul((char *)argv[0],NULL,10);
	if((priority < 1)||priority > 255){
        vty_out(vty,"%% Bad parameter : %s !",argv[0]);
		return CMD_WARNING;
	}
	if(HANSI_NODE==vty->node){
		profile = (unsigned int)(vty->index);
		slot_id = vty->slotindex;
	}

	char vrrp_obj_path[DCLI_VRRP_OBJPATH_LEN] = {0};
	char vrrp_dbus_name[DCLI_VRRP_DBUSNAME_LEN] = {0};	

#ifdef DISTRIBUT
	DBusConnection *dcli_dbus_connection = NULL;
	ReInitDbusConnection(&dcli_dbus_connection,slot_id,distributFag);
#endif
	dcli_vrrp_splice_objpath_string(VRRP_DBUS_OBJPATH, profile, vrrp_obj_path);
	dcli_vrrp_splice_dbusname(VRRP_DBUS_BUSNAME, profile, vrrp_dbus_name);
	
	query = dbus_message_new_method_call(vrrp_dbus_name,
										 vrrp_obj_path,
										 VRRP_DBUS_INTERFACE,
										 VRRP_DBUS_METHOD_PROFILE_VALUE);
	dbus_error_init(&err);

	dbus_message_append_args(query,
		                     DBUS_TYPE_UINT32,&profile,
							 DBUS_TYPE_UINT32,&priority,					 
							 DBUS_TYPE_INVALID);
	
	reply = dbus_connection_send_with_reply_and_block (dcli_dbus_connection,query,-1, &err);
	dbus_message_unref(query);
	if (NULL == reply) {
		vty_out(vty,"failed get reply.\n");
		if (dbus_error_is_set(&err)) {
			printf("%s raised: %s",err.name,err.message);
			dbus_error_free_for_dcli(&err);
		}
		return CMD_SUCCESS;
	}
	if (dbus_message_get_args ( reply, &err,
		DBUS_TYPE_UINT32,&op_ret,
		DBUS_TYPE_INVALID)){
        if( DCLI_VRRP_RETURN_CODE_OK != op_ret){
            vty_out(vty,dcli_vrrp_err_msg[op_ret - DCLI_VRRP_RETURN_CODE_OK]);
		}
	} 
	else 
	{		
		if (dbus_error_is_set(&err)) 
		{
			printf("%s raised: %s",err.name,err.message);
			dbus_error_free_for_dcli(&err);
		}
	}
	dbus_message_unref(reply);
	return CMD_SUCCESS;
}


DEFUN(config_vrrp_global_vmac_cmd_func,
	  config_vrrp_global_vmac_cmd,
	  "config hansi smart vmac (enable|disable)",
	  CONFIG_STR
	  "Config hansi\n"
	  "Config hansi virtual mac attribute\n"
	  "Config hansi global virtual mac \n"
	  "Config hansi global virtual mac enable\n"
	  "Config hansi global virtual mac disable\n"
)

{
	DBusMessage *query = NULL, *reply = NULL;
	DBusError err = {0};
	unsigned int op_ret = 0;
	unsigned int slot_id = HostSlotId;
	unsigned int profile = 0,vrid = 0,state = 0,g_vmac= 0;

	if (!strncmp(argv[0], "enable", strlen(argv[0]))) {
        g_vmac = 1;
	}
	else if (!strncmp(argv[0], "disable", strlen(argv[0]))) {
        g_vmac = 0;
	}
	else {
		vty_out(vty, "%% Bad parameter %s!\n", argv[0]);
		return CMD_WARNING;
	}
	
	if(HANSI_NODE==vty->node){
		profile = (unsigned int)(vty->index);
		slot_id = vty->slotindex;
	}
        
        char vrrp_obj_path[DCLI_VRRP_OBJPATH_LEN] = {0};
	char vrrp_dbus_name[DCLI_VRRP_DBUSNAME_LEN] = {0};	

#ifdef DISTRIBUT
	DBusConnection *dcli_dbus_connection = NULL;
	ReInitDbusConnection(&dcli_dbus_connection,slot_id,distributFag);
#endif
	dcli_vrrp_splice_objpath_string(VRRP_DBUS_OBJPATH, profile, vrrp_obj_path);
	dcli_vrrp_splice_dbusname(VRRP_DBUS_BUSNAME, profile, vrrp_dbus_name);

	query = dbus_message_new_method_call(vrrp_dbus_name,
										 vrrp_obj_path,
										 VRRP_DBUS_INTERFACE,
										 VRRP_DBUS_METHOD_GLOBAL_VMAC_ENABLE);
	dbus_error_init(&err);

	dbus_message_append_args(query,
		                     DBUS_TYPE_UINT32,&profile,	
							 DBUS_TYPE_UINT32,&g_vmac,
							 DBUS_TYPE_INVALID);
	
	reply = dbus_connection_send_with_reply_and_block (dcli_dbus_connection,query,-1, &err);
	dbus_message_unref(query);
	if (NULL == reply) {
		vty_out(vty,"failed get reply.\n");
		if (dbus_error_is_set(&err)) {
			printf("%s raised: %s",err.name,err.message);
			dbus_error_free_for_dcli(&err);
		}
		return CMD_SUCCESS;
	}
	if (dbus_message_get_args ( reply, &err,
		DBUS_TYPE_UINT32,&op_ret,
		DBUS_TYPE_INVALID)){
        if( DCLI_VRRP_RETURN_CODE_OK != op_ret){
            vty_out(vty,dcli_vrrp_err_msg[op_ret - DCLI_VRRP_RETURN_CODE_OK]);
		}
	} 
	else 
	{		
		if (dbus_error_is_set(&err)) 
		{
			printf("%s raised: %s",err.name,err.message);
			dbus_error_free_for_dcli(&err);
		}
	}
	dbus_message_unref(reply);
	return CMD_SUCCESS;
}

DEFUN(config_vrrp_preempt_cmd_func,
	  config_vrrp_preempt_cmd,
	  "config hansi preempt (yes|no)",
	  CONFIG_STR
	  "Config hansi\n"
	  "Config preempt attribute\n"
	  "Config master can be preempted by higher preempt\n"
	  "Config master can not be preempted by higher preempt\n"

)
{
	DBusMessage *query = NULL, *reply = NULL;
	DBusError err = {0};
	unsigned int op_ret = 0;
	unsigned int slot_id = HostSlotId;
	unsigned int profile = 0,vrid = 0,state = 0,preempt = 0;

	if (!strncmp(argv[0], "yes", strlen(argv[0]))) {
        preempt = 1;
	}
	else if (!strncmp(argv[0], "no", strlen(argv[0]))) {
        preempt = 0;
	}
	else {
		vty_out(vty, "%% Bad parameter %s!\n", argv[0]);
		return CMD_WARNING;
	}
	
	if(HANSI_NODE==vty->node){
		profile = (unsigned int)(vty->index);
		slot_id = vty->slotindex;
	}

	char vrrp_obj_path[DCLI_VRRP_OBJPATH_LEN] = {0};
	char vrrp_dbus_name[DCLI_VRRP_DBUSNAME_LEN] = {0};	
    
#ifdef DISTRIBUT
	DBusConnection *dcli_dbus_connection = NULL;
	ReInitDbusConnection(&dcli_dbus_connection,slot_id,distributFag);
#endif
	dcli_vrrp_splice_objpath_string(VRRP_DBUS_OBJPATH, profile, vrrp_obj_path);
	dcli_vrrp_splice_dbusname(VRRP_DBUS_BUSNAME, profile, vrrp_dbus_name);

	query = dbus_message_new_method_call(vrrp_dbus_name,
										 vrrp_obj_path,
										 VRRP_DBUS_INTERFACE,
										 VRRP_DBUS_METHOD_PREEMPT_VALUE);
	dbus_error_init(&err);

	dbus_message_append_args(query,
		                     DBUS_TYPE_UINT32,&profile,	
							 DBUS_TYPE_UINT32,&preempt,
							 DBUS_TYPE_INVALID);
	
	reply = dbus_connection_send_with_reply_and_block (dcli_dbus_connection,query,-1, &err);
	dbus_message_unref(query);
	if (NULL == reply) {
		vty_out(vty,"failed get reply.\n");
		if (dbus_error_is_set(&err)) {
			printf("%s raised: %s",err.name,err.message);
			dbus_error_free_for_dcli(&err);
		}
		return CMD_SUCCESS;
	}
	if (dbus_message_get_args ( reply, &err,
		DBUS_TYPE_UINT32,&op_ret,
		DBUS_TYPE_INVALID)){
        if( DCLI_VRRP_RETURN_CODE_OK != op_ret){
            vty_out(vty,dcli_vrrp_err_msg[op_ret - DCLI_VRRP_RETURN_CODE_OK]);
		}
	} 
	else 
	{		
		if (dbus_error_is_set(&err)) 
		{
			printf("%s raised: %s",err.name,err.message);
			dbus_error_free_for_dcli(&err);
		}
	}
	dbus_message_unref(reply);
	return CMD_SUCCESS;
}


DEFUN(config_vrrp_advert_cmd_func,
	  config_vrrp_advert_cmd,
	  "config hansi advertime TIME",
	  CONFIG_STR
	  "Config hansi\n"
	  "Config hansi advertisement time\n"
	  "Config hansi advertime value in range <1-255>\n"
)

{
	DBusMessage *query = NULL, *reply = NULL;
	DBusError err = {0};
	unsigned int op_ret = 0;
	unsigned int slot_id = HostSlotId;
	unsigned int profile = 0,vrid = 0,advert = 0;
	
	advert = strtoul((char *)argv[0],NULL,10);
	if((advert < 1)||advert > 255){
        vty_out(vty,"%% Bad parameter : %s !",argv[0]);
		return CMD_WARNING;
	}


	if(HANSI_NODE==vty->node){
		profile = (unsigned int)(vty->index);
		slot_id = vty->slotindex;
	}

	char vrrp_obj_path[DCLI_VRRP_OBJPATH_LEN] = {0};
	char vrrp_dbus_name[DCLI_VRRP_DBUSNAME_LEN] = {0};	

#ifdef DISTRIBUT
	DBusConnection *dcli_dbus_connection = NULL;
	ReInitDbusConnection(&dcli_dbus_connection,slot_id,distributFag);
#endif
	dcli_vrrp_splice_objpath_string(VRRP_DBUS_OBJPATH, profile, vrrp_obj_path);
	dcli_vrrp_splice_dbusname(VRRP_DBUS_BUSNAME, profile, vrrp_dbus_name);

	query = dbus_message_new_method_call(vrrp_dbus_name,
										 vrrp_obj_path,
										 VRRP_DBUS_INTERFACE,
										 VRRP_DBUS_METHOD_ADVERT_VALUE);
	dbus_error_init(&err);

	dbus_message_append_args(query,
		                     DBUS_TYPE_UINT32,&profile,
							 DBUS_TYPE_UINT32,&advert,					 
							 DBUS_TYPE_INVALID);
	
	reply = dbus_connection_send_with_reply_and_block (dcli_dbus_connection,query,-1, &err);
	dbus_message_unref(query);
	if (NULL == reply) {
		vty_out(vty,"failed get reply.\n");
		if (dbus_error_is_set(&err)) {
			printf("%s raised: %s",err.name,err.message);
			dbus_error_free_for_dcli(&err);
		}
		return CMD_SUCCESS;
	}
	if (dbus_message_get_args ( reply, &err,
		DBUS_TYPE_UINT32,&op_ret,
		DBUS_TYPE_INVALID)){
        if( DCLI_VRRP_RETURN_CODE_OK != op_ret){
            vty_out(vty,dcli_vrrp_err_msg[op_ret - DCLI_VRRP_RETURN_CODE_OK]);
		}
	} 
	else 
	{		
		if (dbus_error_is_set(&err)) 
		{
			printf("%s raised: %s",err.name,err.message);
			dbus_error_free_for_dcli(&err);
		}
	}
	dbus_message_unref(reply);
	return CMD_SUCCESS;
}

DEFUN(config_vrrp_mac_cmd_func,
	  config_vrrp_mac_cmd,
	  "config hansi virtual mac (yes|no)",
	  CONFIG_STR
	  "Config hansi\n"
	  "Set virtul mac\n"
	  "Config hansi virtul mac(00-00-5e-00-01-id)\n"
	  "set the virtual mac as interface mac\n"
	  "use the interface real mac\n"
)

{
	DBusMessage *query = NULL, *reply = NULL;
	DBusError err = {0};
	unsigned int op_ret = 0;
	unsigned int slot_id = HostSlotId;
	unsigned int profile = 0,vrid = 0,mac = 0;
	
	if (!strncmp(argv[0], "yes", strlen(argv[0]))) {
       mac = 0;
	}
	else if (!strncmp(argv[0], "no", strlen(argv[0]))) {
        mac = 1;
	}
	else {
		vty_out(vty, "%% Bad parameter %s!\n", argv[0]);
		return CMD_WARNING;
	}

	if(HANSI_NODE==vty->node){
		profile = (unsigned int)(vty->index);
		slot_id = vty->slotindex;
	}
	
	char vrrp_obj_path[DCLI_VRRP_OBJPATH_LEN] = {0};
	char vrrp_dbus_name[DCLI_VRRP_DBUSNAME_LEN] = {0};	

#ifdef DISTRIBUT
	DBusConnection *dcli_dbus_connection = NULL;
	ReInitDbusConnection(&dcli_dbus_connection,slot_id,distributFag);
#endif
	dcli_vrrp_splice_objpath_string(VRRP_DBUS_OBJPATH, profile, vrrp_obj_path);
	dcli_vrrp_splice_dbusname(VRRP_DBUS_BUSNAME, profile, vrrp_dbus_name);

	query = dbus_message_new_method_call(vrrp_dbus_name,
										 vrrp_obj_path,
										 VRRP_DBUS_INTERFACE,
										 VRRP_DBUS_METHOD_VIRTUAL_MAC_VALUE);
	dbus_error_init(&err);

	dbus_message_append_args(query,
		                     DBUS_TYPE_UINT32,&profile,
							 DBUS_TYPE_UINT32,&mac,					 
							 DBUS_TYPE_INVALID);
	
	reply = dbus_connection_send_with_reply_and_block (dcli_dbus_connection,query,-1, &err);
	dbus_message_unref(query);
	if (NULL == reply) {
		vty_out(vty,"failed get reply.\n");
		if (dbus_error_is_set(&err)) {
			printf("%s raised: %s",err.name,err.message);
			dbus_error_free_for_dcli(&err);
		}
		return CMD_SUCCESS;
	}
	if (dbus_message_get_args ( reply, &err,
		DBUS_TYPE_UINT32,&op_ret,
		DBUS_TYPE_INVALID)){
        if( DCLI_VRRP_RETURN_CODE_OK != op_ret){
            vty_out(vty,dcli_vrrp_err_msg[op_ret - DCLI_VRRP_RETURN_CODE_OK]);
		}
	} 
	else 
	{		
		if (dbus_error_is_set(&err)) 
		{
			printf("%s raised: %s",err.name,err.message);
			dbus_error_free_for_dcli(&err);
		}
	}
	dbus_message_unref(reply);
	return CMD_SUCCESS;
}


DEFUN(config_vrrp_max_down_count_cmd_func,
	  config_vrrp_max_down_count_cmd,
	  "config hansi master down-count <1-255>",
	  CONFIG_STR
	  "Config hansi\n"
	  "Config state master device\n"
	  "Config hansi master down packet count\n"
	  "Set master down by packet count\n"
)

{
	DBusMessage *query = NULL, *reply = NULL;
	DBusError err = {0};
	unsigned int op_ret = 0;
	unsigned int slot_id = HostSlotId;
	unsigned int profile = 0,vrid = 0,mac = 0;
	int count = 0;
	char vrrp_obj_path[DCLI_VRRP_OBJPATH_LEN] = {0};
	char vrrp_dbus_name[DCLI_VRRP_DBUSNAME_LEN] = {0};	

	count = strtoul((char *)argv[0],NULL,10);
	if((count < 1)||count > 255){
        vty_out(vty,"%% Bad parameter : %s !",argv[0]);
		return CMD_WARNING;
	}

	if(HANSI_NODE==vty->node){
		profile = (unsigned int)(vty->index);
		slot_id = vty->slotindex;
	}

#ifdef DISTRIBUT
	DBusConnection *dcli_dbus_connection = NULL;
	ReInitDbusConnection(&dcli_dbus_connection,slot_id,distributFag);
#endif
	dcli_vrrp_splice_objpath_string(VRRP_DBUS_OBJPATH, profile, vrrp_obj_path);
	dcli_vrrp_splice_dbusname(VRRP_DBUS_BUSNAME, profile, vrrp_dbus_name);

	query = dbus_message_new_method_call(vrrp_dbus_name,
										 vrrp_obj_path,
										 VRRP_DBUS_INTERFACE,
										 VRRP_DBUS_METHOD_MS_DOWN_PACKT_COUNT);
	dbus_error_init(&err);

	dbus_message_append_args(query,
							 DBUS_TYPE_UINT32,&count,					 
							 DBUS_TYPE_INVALID);
	
	reply = dbus_connection_send_with_reply_and_block (dcli_dbus_connection,query,-1, &err);
	dbus_message_unref(query);
	if (NULL == reply) {
		vty_out(vty,"failed get reply.\n");
		if (dbus_error_is_set(&err)) {
			printf("%s raised: %s",err.name,err.message);
			dbus_error_free_for_dcli(&err);
		}
		return CMD_SUCCESS;
	}
	if (dbus_message_get_args ( reply, &err,
		DBUS_TYPE_UINT32,&op_ret,
		DBUS_TYPE_INVALID)){
        if( DCLI_VRRP_RETURN_CODE_OK != op_ret){
            vty_out(vty,dcli_vrrp_err_msg[op_ret - DCLI_VRRP_RETURN_CODE_OK]);
		}
	} 
	else 
	{		
		if (dbus_error_is_set(&err)) 
		{
			printf("%s raised: %s",err.name,err.message);
			dbus_error_free_for_dcli(&err);
		}
	}
	dbus_message_unref(reply);
	return CMD_SUCCESS;
}



DEFUN(config_vrrp_multi_link_detect_cmd_func,
	  config_vrrp_multi_link_detect_cmd,
	  "config hansi multi-link-detect (on|off)",
	  CONFIG_STR
	  "Config hansi\n"
	  "Config hansi uplink and(or) multi-link detect\n"
	  "Config hansi multi-link detect on\n"
	  "Config hansi multi-link detect off\n"
)

{
	DBusMessage *query = NULL, *reply = NULL;
	DBusError err = {0};
	unsigned int op_ret = 0;
	unsigned int profile = 0;
	unsigned int slot_id = HostSlotId;
	int detect = 0;
	
	if (!strncmp(argv[0], "on", strlen(argv[0]))) {
        detect = 1;
	}
	else if (!strncmp(argv[0], "off", strlen(argv[0]))) {
        detect = 0;
	}
	else {
		vty_out(vty, "%% Bad parameter %s!\n", argv[0]);
		return CMD_WARNING;
	}
	
	if(HANSI_NODE==vty->node){
		profile = (unsigned int)(vty->index);
		slot_id = vty->slotindex;
	}

	char vrrp_obj_path[DCLI_VRRP_OBJPATH_LEN] = {0};
	char vrrp_dbus_name[DCLI_VRRP_DBUSNAME_LEN] = {0};	

#ifdef DISTRIBUT
	DBusConnection *dcli_dbus_connection = NULL;
	ReInitDbusConnection(&dcli_dbus_connection,slot_id,distributFag);
#endif
	dcli_vrrp_splice_objpath_string(VRRP_DBUS_OBJPATH, profile, vrrp_obj_path);
	dcli_vrrp_splice_dbusname(VRRP_DBUS_BUSNAME, profile, vrrp_dbus_name);

	query = dbus_message_new_method_call(vrrp_dbus_name,
										 vrrp_obj_path,
										 VRRP_DBUS_INTERFACE,
										 VRRP_DBUS_METHOD_MULTI_LINK_DETECT);
	dbus_error_init(&err);

	dbus_message_append_args(query,
							 DBUS_TYPE_UINT32,&profile,	
							 DBUS_TYPE_UINT32,&detect,					 
							 DBUS_TYPE_INVALID);
	
	reply = dbus_connection_send_with_reply_and_block (dcli_dbus_connection,query,-1, &err);
	dbus_message_unref(query);
	if (NULL == reply) {
		vty_out(vty,"failed get reply.\n");
		if (dbus_error_is_set(&err)) {
			printf("%s raised: %s",err.name,err.message);
			dbus_error_free_for_dcli(&err);
		}
		return CMD_SUCCESS;
	}
	if (dbus_message_get_args ( reply, &err,
		DBUS_TYPE_UINT32,&op_ret,
		DBUS_TYPE_INVALID)){
        if( DCLI_VRRP_RETURN_CODE_OK != op_ret){
            vty_out(vty,dcli_vrrp_err_msg[op_ret - DCLI_VRRP_RETURN_CODE_OK]);
		}
	} 
	else 
	{		
		if (dbus_error_is_set(&err)) 
		{
			printf("%s raised: %s",err.name,err.message);
			dbus_error_free_for_dcli(&err);
		}
	}
	dbus_message_unref(reply);
	return CMD_SUCCESS;
}


DEFUN(show_vrrp_cmd_func,
	  show_vrrp_cmd,
	  "show hansi [PARAMETER] [detail]",
	  SHOW_STR
	  "Show hansi\n"
	  "Hansi ID\n"
	  "Detail information\n"
)

{

	unsigned int op_ret = 0;
	int profile = 0;
	unsigned int slot_id = HostSlotId;
	int ret = 0;
	unsigned char insID = 0;
	int flag = 0;
	if(HANSI_NODE==vty->node){
		profile = (unsigned int)(vty->index);
		slot_id = vty->slotindex;
	}
	if((1 == argc)||(2 == argc)){
		ret = parse_slot_hansi_id((char*)argv[0],&slot_id,&profile);
        insID = (unsigned char)profile;
    	if(ret != WID_DBUS_SUCCESS){
    		slot_id = HostSlotId;
    		flag = 1;
    		ret = parse_char_ID((char*)argv[0], &insID);
    		if(ret != WID_DBUS_SUCCESS){
    	            if(ret == WID_ILLEGAL_INPUT){
    	            	vty_out(vty,"<error> illegal input:Input exceeds the maximum value of the parameter type \n");
    	            }
    				else{
    					vty_out(vty,"<error> unknown id format\n");
    				}
    			return CMD_SUCCESS;
    		}	
    	}
    	profile = insID;
    	if(distributFag == 0){
    		if(slot_id != 0){
    			vty_out(vty,"<error> slot id should be 0\n");
    			return CMD_SUCCESS;
    		}	
    	}else if(flag == 1){
    		slot_id = HostSlotId;
    	}
    	if((ret != 0)||(profile < 0)||(profile > 16)){
    		vty_out(vty,"%% Bad parameter : %s !",argv[0]);
    		return CMD_WARNING;
    	}
	}
	if((0 == argc)&&(HANSI_NODE != vty->node)){
        for(profile = 0;profile < 17;profile++){
		   op_ret = 0;
#ifdef DISTRIBUT
           op_ret = dcli_show_hansi_profile(vty,profile,slot_id);
#else
			op_ret = dcli_show_hansi_profile(vty,profile);
#endif
		   if(DCLI_VRRP_RETURN_CODE_PROFILE_NOTEXIST == op_ret ){
			   vty_out(vty, "hansi profile %d not exist!\n", profile);
			   /* return CMD_WARNING; */
			}
			else if(DCLI_VRRP_RETURN_CODE_PROCESS_NOTEXIST == op_ret){
				vty_out(vty, "%% Hansi %d service not start!\n", profile);
			}
			else if(DCLI_VRRP_RETURN_CODE_NO_CONFIG == op_ret){
				vty_out(vty, "%% Hansi %d has no configuration!\n", profile);
		   }
		}

		return CMD_SUCCESS;
	}
	if((1 == argc)||(0 == argc)){
#ifdef DISTRIBUT
		op_ret = dcli_show_hansi_profile(vty,profile,slot_id);
#else
		op_ret = dcli_show_hansi_profile(vty,profile);
#endif
	}
	if(2 == argc){
		if((!strcmp(argv[1],"detail"))||(!strcmp(argv[1],"detai"))||(!strcmp(argv[1],"deta"))||\
			(!strcmp(argv[1],"det"))||(!strcmp(argv[1],"de"))||(!strcmp(argv[1],"d"))){
#ifdef DISTRIBUT
		op_ret = dcli_show_hansi_profile_detail(vty,profile,slot_id);
#else
		op_ret = dcli_show_hansi_profile_detail(vty,profile);
#endif
		}
		else{
			vty_out(vty,"you should input as:config hansi 1-1 d de det deta detai or detail");
			return CMD_SUCCESS;
		}
	}
	if(DCLI_VRRP_RETURN_CODE_PROFILE_NOTEXIST == op_ret){
	   vty_out(vty,"%% Hansi profile %d not exist!\n",profile);
	   return CMD_WARNING;
	}
	else if(DCLI_VRRP_RETURN_CODE_PROCESS_NOTEXIST == op_ret){
		vty_out(vty, "%% Hansi %d service not start!\n", profile);
	   	return CMD_WARNING;
	}
	else if(DCLI_VRRP_RETURN_CODE_NO_CONFIG == op_ret){
		vty_out(vty, "%% Hansi %d has no configuration!\n", profile);
	   	return CMD_WARNING;
	}
	return CMD_SUCCESS;
}

/******************************************************************************************
*	common command
******************************************************************************************/
DEFUN(debug_vrrp_cmd_func,
		debug_vrrp_cmd,
		"debug vrrp (all|error|warning|debug|event|protocol|packet_send|packet_receive|packet_all)",
		DEBUG_STR
		"debus vrrp parameter \n"
		"debug vrrp all \n"
		"debug vrrp error \n"
		"debug vrrp warning \n"
		"debug vrrp debug \n"
		"debug vrrp event \n"
		"debug vrrp protocol \n"
		"debug vrrp packet send \n"
		"debug vrrp packet received \n"
		"debug vrrp packet send and receive \n"       
)
{	DBusMessage *query, *reply;
	DBusError err;
	
	unsigned int ret;
	unsigned int flag;
	unsigned int profile = 0;
	unsigned int slot_id = HostSlotId;
		
	if(argc > 1) {
		vty_out(vty,"command parameters error!\n");
		return CMD_WARNING;
	}
	
	
	if(0 == strcmp(argv[0],"all")) {
		flag = VRRP_DEBUG_FLAG_ALL;
	}
	else if(0 == strcmp(argv[0],"error")) {
		flag = VRRP_DEBUG_FLAG_ERR;
	}
	else if(0 == strcmp(argv[0],"warning")) {
		flag = VRRP_DEBUG_FLAG_WAR;
	}
	else if(0 == strcmp(argv[0],"debug")) {
		flag = VRRP_DEBUG_FLAG_DBG;
	}
	else if(0 == strcmp(argv[0],"event")) {
		flag = VRRP_DEBUG_FLAG_EVT;
	}
	else if(0 == strcmp(argv[0],"packet_send")) {
		flag = VRRP_DEBUG_FLAG_PKT_SED;
	}
	else if(0 == strcmp(argv[0],"packet_receive")) {
		flag = VRRP_DEBUG_FLAG_PKT_REV;
	}
	else if(0 == strcmp(argv[0],"packet_all")) {
		flag = VRRP_DEBUG_FLAG_PKT_ALL;
	}
	else if(0 == strcmp(argv[0],"protocol")) {
		flag = VRRP_DEBUG_FLAG_PROTOCOL;
	}
	else {
		vty_out(vty,"command parameter error!\n");
		return CMD_WARNING;
	}

	if(HANSI_NODE==vty->node){
		profile = (unsigned int)(vty->index);
		slot_id = vty->slotindex;
	}

	char vrrp_obj_path[DCLI_VRRP_OBJPATH_LEN] = {0};
	char vrrp_dbus_name[DCLI_VRRP_DBUSNAME_LEN] = {0};	

#ifdef DISTRIBUT
	DBusConnection *dcli_dbus_connection = NULL;
	ReInitDbusConnection(&dcli_dbus_connection,slot_id,distributFag);
#endif
	dcli_vrrp_splice_objpath_string(VRRP_DBUS_OBJPATH, profile, vrrp_obj_path);
	dcli_vrrp_splice_dbusname(VRRP_DBUS_BUSNAME, profile, vrrp_dbus_name);


	query = dbus_message_new_method_call(vrrp_dbus_name,
										vrrp_obj_path,
										VRRP_DBUS_INTERFACE,
										VRRP_DBUS_METHOD_BRG_DBUG_VRRP);
    dbus_error_init(&err);
	dbus_message_append_args(query,
									DBUS_TYPE_UINT32,&flag,
									DBUS_TYPE_INVALID);
 	
	reply = dbus_connection_send_with_reply_and_block (dcli_dbus_connection,query,-1, &err);
	dbus_message_unref(query);
	if (NULL == reply) {		
		vty_out(vty,"failed get reply.\n");	
		if (dbus_error_is_set(&err)) {
			vty_out(vty,"%s raised: %s",err.name,err.message);
			dbus_error_free_for_dcli(&err);
		}
		return CMD_SUCCESS;
	}
	if (dbus_message_get_args ( reply, &err,
									DBUS_TYPE_UINT32,&ret,
									DBUS_TYPE_INVALID)) {
         ;
	}
	else {		
		vty_out(vty,"Failed get args.\n");		
		if (dbus_error_is_set(&err)) {
				vty_out(vty,"%s raised: %s",err.name,err.message);
				dbus_error_free_for_dcli(&err);
		}
	}
		
	dbus_message_unref(reply);
	return CMD_SUCCESS;
}

DEFUN(no_debug_vrrp_cmd_func,
		no_debug_vrrp_cmd,
		"no debug vrrp (all|error|warning|debug|event|protocol|packet_send|packet_receive|packet_all)",
        "Cancel the debug\n"
        DEBUG_STR
		"debus vrrp parameter \n"
		"debug vrrp all \n"
		"debug vrrp error \n"
		"debug vrrp warning \n"
		"debug vrrp debug \n"
		"debug vrrp event \n"
		"debug vrrp protocol \n"
		"debug vrrp packet send \n"
		"debug vrrp packet received \n"
		"debug vrrp packet send and receive \n"       
)
{	
    DBusMessage *query, *reply;
	DBusError err;
	
	unsigned int ret;
	unsigned int flag;
	unsigned int profile = 0;
	unsigned int slot_id = HostSlotId;
	
	if(argc > 1) {
		vty_out(vty,"command parameters error!\n");
		return CMD_WARNING;
	}
	
	
	if(0 == strcmp(argv[0],"all")) {
		flag = VRRP_DEBUG_FLAG_ALL;
	}
	else if(0 == strcmp(argv[0],"error")) {
		flag = VRRP_DEBUG_FLAG_ERR;
	}
	else if(0 == strcmp(argv[0],"warning")) {
		flag = VRRP_DEBUG_FLAG_WAR;
	}
	else if(0 == strcmp(argv[0],"debug")) {
		flag = VRRP_DEBUG_FLAG_DBG;
	}
	else if(0 == strcmp(argv[0],"event")) {
		flag = VRRP_DEBUG_FLAG_EVT;
	}
	else if(0 == strcmp(argv[0],"packet_send")) {
		flag = VRRP_DEBUG_FLAG_PKT_SED;
	}
	else if(0 == strcmp(argv[0],"packet_receive")) {
		flag = VRRP_DEBUG_FLAG_PKT_REV;
	}
	else if(0 == strcmp(argv[0],"packet_all")) {
		flag = VRRP_DEBUG_FLAG_PKT_ALL;
	}
	else if(0 == strcmp(argv[0],"protocol")) {
		flag = VRRP_DEBUG_FLAG_PROTOCOL;
	}
	else {
		vty_out(vty,"command parameter error!\n");
		return CMD_WARNING;
	}

	if(HANSI_NODE==vty->node){
		profile = (unsigned int)(vty->index);
		slot_id = vty->slotindex;
	}

	char vrrp_obj_path[DCLI_VRRP_OBJPATH_LEN] = {0};
	char vrrp_dbus_name[DCLI_VRRP_DBUSNAME_LEN] = {0};	

#ifdef DISTRIBUT
	DBusConnection *dcli_dbus_connection = NULL;
	ReInitDbusConnection(&dcli_dbus_connection,slot_id,distributFag);
#endif
	dcli_vrrp_splice_objpath_string(VRRP_DBUS_OBJPATH, profile, vrrp_obj_path);
	dcli_vrrp_splice_dbusname(VRRP_DBUS_BUSNAME, profile, vrrp_dbus_name);
	
	query = dbus_message_new_method_call(vrrp_dbus_name,
										vrrp_obj_path,
										VRRP_DBUS_INTERFACE,
										VRRP_DBUS_METHOD_BRG_NO_DBUG_VRRP);
    dbus_error_init(&err);
	dbus_message_append_args(query,
									DBUS_TYPE_UINT32,&flag,
									DBUS_TYPE_INVALID);
 	
	reply = dbus_connection_send_with_reply_and_block (dcli_dbus_connection,query,-1, &err);
	dbus_message_unref(query);
	if (NULL == reply) {		
		vty_out(vty,"failed get reply.\n");	
		if (dbus_error_is_set(&err)) {
			vty_out(vty,"%s raised: %s",err.name,err.message);
			dbus_error_free_for_dcli(&err);
		}
		return CMD_SUCCESS;
	}
	if (dbus_message_get_args ( reply, &err,
									DBUS_TYPE_UINT32,&ret,
									DBUS_TYPE_INVALID)) {
         ;
	}
	else {		
		vty_out(vty,"Failed get args.\n");		
		if (dbus_error_is_set(&err)) {
				vty_out(vty,"%s raised: %s",err.name,err.message);
				dbus_error_free_for_dcli(&err);
		}
	}
		
	dbus_message_unref(reply);
	return CMD_SUCCESS;
}

/******************************************************************************************
*	common command
******************************************************************************************/
DEFUN(vrrp_trap_sw_cmd_func,
		vrrp_trap_sw_cmd,
		"config vrrp trap switch (enable|disable)",
		DEBUG_STR
		"set vrrp trap parameter \n"
		"set vrrp enable \n"
		"set vrrp disable \n"
)
{	DBusMessage *query, *reply;
	DBusError err;
	
	unsigned int ret;
	unsigned int flag;
	unsigned int profile = 0;
	unsigned int slot_id = HostSlotId;
		
	if(argc > 1) {
		vty_out(vty,"command parameters error!\n");
		return CMD_WARNING;
	}
	
	
	if(0 == strcmp(argv[0],"enable")) {
		flag = 1;
	}
	else if(0 == strcmp(argv[0],"disable")) {
		flag = 0;
	}
	else {
		vty_out(vty,"command parameter error!\n");
		return CMD_WARNING;
	}

	if(HANSI_NODE==vty->node){
		profile = (unsigned int)(vty->index);
		slot_id = vty->slotindex;
	}

	char vrrp_obj_path[DCLI_VRRP_OBJPATH_LEN] = {0};
	char vrrp_dbus_name[DCLI_VRRP_DBUSNAME_LEN] = {0};	

#ifdef DISTRIBUT
	DBusConnection *dcli_dbus_connection = NULL;
	ReInitDbusConnection(&dcli_dbus_connection,slot_id,distributFag);
#endif
	dcli_vrrp_splice_objpath_string(VRRP_DBUS_OBJPATH, profile, vrrp_obj_path);
	dcli_vrrp_splice_dbusname(VRRP_DBUS_BUSNAME, profile, vrrp_dbus_name);


	query = dbus_message_new_method_call(vrrp_dbus_name,
										vrrp_obj_path,
										VRRP_DBUS_INTERFACE,
										VRRP_DBUS_METHOD_HAD_TRAP_SWITCH);
    dbus_error_init(&err);
	dbus_message_append_args(query,
									DBUS_TYPE_UINT32,&profile,    
									DBUS_TYPE_UINT32,&flag,
									DBUS_TYPE_INVALID);
 	
	reply = dbus_connection_send_with_reply_and_block (dcli_dbus_connection,query,-1, &err);
	dbus_message_unref(query);
	if (NULL == reply) {		
		vty_out(vty,"failed get reply.\n");	
		if (dbus_error_is_set(&err)) {
			vty_out(vty,"%s raised: %s",err.name,err.message);
			dbus_error_free_for_dcli(&err);
		}
		return CMD_SUCCESS;
	}
	if (dbus_message_get_args ( reply, &err,
									DBUS_TYPE_UINT32,&ret,
									DBUS_TYPE_INVALID)) {
         ;
	}
	else {		
		vty_out(vty,"Failed get args.\n");		
		if (dbus_error_is_set(&err)) {
				vty_out(vty,"%s raised: %s",err.name,err.message);
				dbus_error_free_for_dcli(&err);
		}
	}
		
	dbus_message_unref(reply);
	return CMD_SUCCESS;
}

/*only downlink*/
DEFUN(send_arp_cmd_func,
	  send_arp_cmd,
	  "send arp interface IFNAME ip IP mac MAC",
	  "Send packets\n"
	  "Arp packets\n"
	  "L3 interface name\n"
	  "Name string\n"
	  "L3 interface ip\n"
	  "Ip string\n"
	  "L3 interface mac\n"
	  "Mac addr\n"
)

{
	DBusMessage *query = NULL, *reply = NULL;
	DBusError err = {0};
	unsigned int op_ret = 0;
	unsigned int profile = 0,vrid = 0,priority = 0;
	int add = 1;
	char* ifname = NULL;
    char* ip = NULL;
    unsigned int slot_id = HostSlotId;

	ifname = (char *)malloc(MAX_IFNAME_LEN);	
	if(NULL == ifname){
       return CMD_WARNING;
	}
	memset(ifname,0,MAX_IFNAME_LEN);
	memcpy(ifname,argv[0],strlen(argv[0]));
	
	ip = (char *)malloc(MAX_IPADDR_LEN);	
	if(NULL == ip){
       return CMD_WARNING;
	}
	memset(ip,0,MAX_IPADDR_LEN);
	memcpy(ip,argv[1],strlen(argv[1]));

	ETHERADDR		macAddr;
	memset(&macAddr,0,sizeof(ETHERADDR));	
	op_ret = parse_mac_addr((char *)argv[2],&macAddr);
	if (NPD_FAIL == op_ret) {
    	vty_out(vty,"% Bad Parameter,Unknow mac addr format!\n");
		return CMD_SUCCESS;
	}

	if(HANSI_NODE==vty->node){
		profile = (unsigned int)(vty->index);
		slot_id = vty->slotindex;
	}

	char vrrp_obj_path[DCLI_VRRP_OBJPATH_LEN] = {0};
	char vrrp_dbus_name[DCLI_VRRP_DBUSNAME_LEN] = {0};	

#ifdef DISTRIBUT
	DBusConnection *dcli_dbus_connection = NULL;
	ReInitDbusConnection(&dcli_dbus_connection,slot_id,distributFag);
#endif
	dcli_vrrp_splice_objpath_string(VRRP_DBUS_OBJPATH, profile, vrrp_obj_path);
	dcli_vrrp_splice_dbusname(VRRP_DBUS_BUSNAME, profile, vrrp_dbus_name);

	query = dbus_message_new_method_call(vrrp_dbus_name,
										 vrrp_obj_path,
										 VRRP_DBUS_INTERFACE,
										 VRRP_DBUS_METHOD_START_SEND_ARP);
	dbus_error_init(&err);

	dbus_message_append_args(query,
                             DBUS_TYPE_UINT32,&profile,
							 DBUS_TYPE_STRING,&ifname,
							 DBUS_TYPE_STRING,&ip,
						 	 DBUS_TYPE_BYTE,&macAddr.arEther[0],
						 	 DBUS_TYPE_BYTE,&macAddr.arEther[1],
						 	 DBUS_TYPE_BYTE,&macAddr.arEther[2],
						 	 DBUS_TYPE_BYTE,&macAddr.arEther[3],
						 	 DBUS_TYPE_BYTE,&macAddr.arEther[4],
						 	 DBUS_TYPE_BYTE,&macAddr.arEther[5],							 
							 DBUS_TYPE_INVALID);
	
	reply = dbus_connection_send_with_reply_and_block (dcli_dbus_connection,query,-1, &err);
	dbus_message_unref(query);
	if (NULL == reply) {
		vty_out(vty,"failed get reply.\n");
		if (dbus_error_is_set(&err)) {
			printf("%s raised: %s",err.name,err.message);
			dbus_error_free_for_dcli(&err);
		}
		return CMD_SUCCESS;
	}
	else if (dbus_message_get_args ( reply, &err,
		DBUS_TYPE_UINT32,&op_ret,
		DBUS_TYPE_INVALID)){	
        if(DCLI_VRRP_RETURN_CODE_OK != op_ret){
            if((op_ret > DCLI_VRRP_RETURN_CODE_OK)&&\
                ((op_ret - DCLI_VRRP_RETURN_CODE_OK) < sizeof(dcli_vrrp_err_msg)/sizeof(char *))){
                vty_out(vty,"%s",dcli_vrrp_err_msg[op_ret - DCLI_VRRP_RETURN_CODE_OK]);
            }
            else{
                vty_out(vty,"%% Unknown error,ret %#x \n",op_ret);
            }
        }
		/*dcli_vrrp_notify_to_npd(vty,NULL,downlink_ifname,add);*/
	} 
	else 
	{		
		if (dbus_error_is_set(&err)) 
		{
			printf("%s raised: %s",err.name,err.message);
			dbus_error_free_for_dcli(&err);
		}
	}

	dbus_message_unref(reply);
	return CMD_SUCCESS;
}


/*
 *******************************************************************************
 *config_vrrp_notify_cmd()
 *
 *  DESCRIPTION:
 *		Config hansi notify attribute
 *
 *******************************************************************************
 */
DEFUN(config_vrrp_notify_cmd_func,
	config_vrrp_notify_cmd,
	"config hansi notify (wireless-control|easy-access-gateway|dhcp-server|pppoe) (on|off)",/*add pppoe*/
	CONFIG_STR
	"Config hansi\n"
	"Config hansi notify attribute\n"
	"Config hansi notification object to wireless-control\n"
	"Config hansi notification object to easy-access-gateway\n"
	"Config hansi notification object to dhcp server\n"
	"Open notify\n"
	"Close notify\n"
)
{
	DBusMessage *query = NULL, *reply = NULL;
	DBusError err = {0};

	unsigned int op_ret = 0;
	unsigned int profile = 0;
	unsigned int slot_id = HostSlotId;
	unsigned char notify_obj = DCLI_VRRP_NOTIFY_OBJ_TYPE_INVALID;
	unsigned char notify_flg = DCLI_VRRP_NOTIFY_OFF;
	char vrrp_obj_path[DCLI_VRRP_OBJPATH_LEN] = {0};
	char vrrp_dbus_name[DCLI_VRRP_DBUSNAME_LEN] = {0};	

	if (!strncmp(argv[0], "wireless-control", strlen(argv[0]))) {
		notify_obj = DCLI_VRRP_NOTIFY_OBJ_TYPE_WID;
	}else if (!strncmp(argv[0], "easy-access-gateway", strlen(argv[0]))) {
		notify_obj = DCLI_VRRP_NOTIFY_OBJ_TYPE_PORTAL;
	}
	else if(!strncmp(argv[0], "dhcp-server", strlen(argv[0]))) {
		notify_obj = DCLI_VRRP_NOTIFY_OBJ_TYPE_DHCP;
	}
	/*add pppoe*/
	else if(!strncmp(argv[0], "pppoe", strlen(argv[0]))) {
	#ifdef _VERSION_18SP7_
		vty_out(vty, "This version not support pppoe!\n");
		return CMD_WARNING;
	#else /* !_VERSION_18SP7_ */
		notify_obj = DCLI_VRRP_NOTIFY_OBJ_TYPE_PPPOE;
	#endif	
	}
	else {
		vty_out(vty, "%% Unknown notification object!\n");
		return CMD_WARNING;
	}

	if (!strncmp(argv[1], "on", strlen(argv[1]))) {
		notify_flg = DCLI_VRRP_NOTIFY_ON;
	}else if (!strncmp(argv[1], "off", strlen(argv[1]))) {
		notify_flg = DCLI_VRRP_NOTIFY_OFF;
	}else {
		vty_out(vty, "%% Unknown command format!\n");
		return CMD_WARNING;
	}

	if (HANSI_NODE == vty->node) {
		profile = (unsigned int)(vty->index);
		slot_id = vty->slotindex;
	}

#ifdef DISTRIBUT
	DBusConnection *dcli_dbus_connection = NULL;
	ReInitDbusConnection(&dcli_dbus_connection,slot_id,distributFag);
#endif
	dcli_vrrp_splice_objpath_string(VRRP_DBUS_OBJPATH, profile, vrrp_obj_path);
	dcli_vrrp_splice_dbusname(VRRP_DBUS_BUSNAME, profile, vrrp_dbus_name);

	query = dbus_message_new_method_call(vrrp_dbus_name,
										 vrrp_obj_path,
										 VRRP_DBUS_INTERFACE,
										 VRRP_DBUS_METHOD_SET_NOTIFY_OBJ_AND_FLG);
	dbus_error_init(&err);

	dbus_message_append_args(query,
		                     DBUS_TYPE_UINT32, &profile,	
							 DBUS_TYPE_BYTE, &notify_obj,
							 DBUS_TYPE_BYTE, &notify_flg,
							 DBUS_TYPE_INVALID);
	
	reply = dbus_connection_send_with_reply_and_block(dcli_dbus_connection, query, -1, &err);
	dbus_message_unref(query);
	if (NULL == reply) {
		vty_out(vty, "failed get reply.\n");
		if (dbus_error_is_set(&err)) {
			printf("%s raised: %s", err.name, err.message);
			dbus_error_free_for_dcli(&err);
		}
		return CMD_SUCCESS;
	}
	
	if (dbus_message_get_args(reply, &err,
							DBUS_TYPE_UINT32, &op_ret,
							DBUS_TYPE_INVALID))
	{
        if (DCLI_VRRP_RETURN_CODE_OK != op_ret) {
            vty_out(vty, dcli_vrrp_err_msg[op_ret - DCLI_VRRP_RETURN_CODE_OK]);
		}
	}
	else {
		if (dbus_error_is_set(&err)) 
		{
			printf("%s raised: %s",err.name,err.message);
			dbus_error_free_for_dcli(&err);
		}
	}

	dbus_message_unref(reply);
	return CMD_SUCCESS;
}

/*
 *******************************************************************************
 *config_vrrp_set_vgateway_transform_cmd()
 *
 *  DESCRIPTION:
 *		Config hansi instance vgateway transform attribute
 *
 *******************************************************************************
 */
DEFUN(config_vrrp_set_vgateway_transform_cmd_func,
	config_vrrp_set_vgateway_transform_cmd,
	"config vgateway anomaly-check (on|off)",
	CONFIG_STR
	"Virtual gateway\n"
	"Config virtual gateway anomaly-check attribute\n"
	"Open\n"
	"Close\n"
)
{
	DBusMessage *query = NULL, *reply = NULL;
	DBusError err = {0};

	unsigned int op_ret = 0;
	unsigned int profile = 0;
	unsigned int slot_id = HostSlotId;
	unsigned int vgateway_tf_flg = DCLI_VRRP_VGATEWAY_TF_FLG_OFF;
	char vrrp_obj_path[DCLI_VRRP_OBJPATH_LEN] = {0};
	char vrrp_dbus_name[DCLI_VRRP_DBUSNAME_LEN] = {0};	

	if (!strncmp(argv[0], "on", strlen(argv[0]))) {
		vgateway_tf_flg = DCLI_VRRP_VGATEWAY_TF_FLG_ON;
	}else if (!strncmp(argv[0], "off", strlen(argv[0]))) {
		vgateway_tf_flg = DCLI_VRRP_VGATEWAY_TF_FLG_OFF;
	}else {
		vty_out(vty, "%% Unknown command format!\n");
		return CMD_WARNING;
	}

	if (HANSI_NODE == vty->node) {
		profile = (unsigned int)(vty->index);
		slot_id = vty->slotindex;
	}

#ifdef DISTRIBUT
	DBusConnection *dcli_dbus_connection = NULL;
	ReInitDbusConnection(&dcli_dbus_connection,slot_id,distributFag);
#endif
	dcli_vrrp_splice_objpath_string(VRRP_DBUS_OBJPATH, profile, vrrp_obj_path);
	dcli_vrrp_splice_dbusname(VRRP_DBUS_BUSNAME, profile, vrrp_dbus_name);

	query = dbus_message_new_method_call(vrrp_dbus_name,
										 vrrp_obj_path,
										 VRRP_DBUS_INTERFACE,
										 VRRP_DBUS_METHOD_SET_VGATEWAY_TRANSFORM_FLG);
	dbus_error_init(&err);

	dbus_message_append_args(query,
		                     DBUS_TYPE_UINT32, &profile,	
							 DBUS_TYPE_UINT32, &vgateway_tf_flg,
							 DBUS_TYPE_INVALID);
	
	reply = dbus_connection_send_with_reply_and_block(dcli_dbus_connection, query, -1, &err);
	dbus_message_unref(query);
	if (NULL == reply) {
		vty_out(vty, "failed get reply.\n");
		if (dbus_error_is_set(&err)) {
			printf("%s raised: %s", err.name, err.message);
			dbus_error_free_for_dcli(&err);
		}
		return CMD_SUCCESS;
	}
	
	if (dbus_message_get_args(reply, &err,
							DBUS_TYPE_UINT32, &op_ret,
							DBUS_TYPE_INVALID))
	{
        if (DCLI_VRRP_RETURN_CODE_OK != op_ret) {
            vty_out(vty, dcli_vrrp_err_msg[op_ret - DCLI_VRRP_RETURN_CODE_OK]);
		}
	}
	else {
		if (dbus_error_is_set(&err)) 
		{
			printf("%s raised: %s",err.name,err.message);
			dbus_error_free_for_dcli(&err);
		}
	}

	dbus_message_unref(reply);
	return CMD_SUCCESS;
}

#ifndef DISTRIBUT
DEFUN(show_vrrp_runconfig_cmd_func,
	show_vrrp_runconfig_cmd,
	"show hansi running-config [<0-16>]",
	SHOW_STR
	"Show hansi\n"
	"Running configuration"
	"hansi id\n"
)
{
	unsigned int op_ret = 0;
	int profile = 0;

	if (HANSI_NODE == vty->node) {
		profile = (unsigned int)(vty->index);
	}

	if (1 == argc) {
		profile = strtoul((char *)argv[0], NULL, 10);
		if ((profile < 0) ||
			(profile > 16))
		{
			vty_out(vty,"%% Invalid instance IO %s !", argv[0]);
			return CMD_WARNING;
		}
	}

	/* in config node, show all instances */
	if ((0 == argc) &&
		(HANSI_NODE != vty->node))
	{
        for (profile = 0; profile < 17; profile++)
		{
		   op_ret = 0;
           op_ret = dcli_vrrp_show_hansi_running_cfg(vty, profile);
		}
		return CMD_SUCCESS;
	}
	/* in config node or hansi node, show special instance */
	if ((1 == argc) ||
		(0 == argc))
	{
		op_ret = dcli_vrrp_show_hansi_running_cfg(vty, profile);
	}

	return CMD_SUCCESS;
}
#else
/* book modify, 2011-5-24 */
DEFUN(show_vrrp_runconfig_cmd_func,
	show_vrrp_runconfig_cmd,
	"show hansi running-config [PARAMETER]",
	SHOW_STR
	"Show hansi\n"
	"Running configuration"
	"hansi id\n"
)
{
	unsigned int op_ret = 0;
	int profile = 0;
	int ret = 0;
	unsigned int slot_id = HostSlotId;
	unsigned int slot_no = 0;
	unsigned int slotid[MAX_SLOT_NUM] = {0};
	int i = 0;
	unsigned char insID = 0;
	int flag = 0,in_hansi_node = 0;;
    for(i = 0; i < MAX_SLOT_NUM; i++){
        slotid[i] = -1;
    }
	if (HANSI_NODE == vty->node) {
		profile = (unsigned int)(vty->index);
		slot_id = vty->slotindex;
		in_hansi_node= 1;
	}
    
	if (1 == argc) {
		ret = parse_slot_hansi_id((char*)argv[0],&slot_id,&profile);
        insID = (unsigned char)profile;
    	if(ret != WID_DBUS_SUCCESS){
    		slot_id = HostSlotId;
    		flag = 1;
    		ret = parse_char_ID((char*)argv[0], &insID);
    		if(ret != WID_DBUS_SUCCESS){
    	            if(ret == WID_ILLEGAL_INPUT){
    	            	vty_out(vty,"<error> illegal input:Input exceeds the maximum value of the parameter type \n");
    	            }
    				else{
    					vty_out(vty,"<error> unknown id format\n");
    				}
    			return CMD_SUCCESS;
    		}	
    	}
    	profile = insID;
    	if(distributFag == 0){
    		if(slot_id != 0){
    			vty_out(vty,"<error> slot id should be 0\n");
    			return CMD_SUCCESS;
    		}	
    	}else if(flag == 1){
    		slot_id = HostSlotId;
    	}
		
    	if((ret != 0)||(profile < 0)||(profile > 16)){
    		vty_out(vty,"%% Bad parameter : %s !",argv[0]);
    		return CMD_WARNING;
    	}
	}

	/* in config node, show all instances */
	if ((0 == argc) &&
		(HANSI_NODE != vty->node))
	{
	    /* get all alive slot numbers from master hmd daemon, 
	       ** and show all running configs in loops
	       ** add by book, 2011-5-12
	        */ 
    	DBusMessage *hmdquery = NULL, *hmdreply = NULL;
    	DBusError hmderr = {0};
    	DBusMessageIter	 iter;
    	unsigned int slot_num = 0;
    	
    	hmdquery = dbus_message_new_method_call(HMD_DBUS_BUSNAME,HMD_DBUS_OBJPATH,HMD_DBUS_INTERFACE,HMD_DBUS_HAD_GET_ALIVE_SLOT_NO);

    	dbus_error_init(&hmderr);
    	
    	hmdreply = dbus_connection_send_with_reply_and_block (dcli_dbus_connection,hmdquery,-1, &hmderr);

    	dbus_message_unref(hmdquery);
    	if (NULL == hmdreply) {
    		vty_out(vty,"failed get reply.\n");
    		if (dbus_error_is_set(&hmderr)) {
    			printf("%s raised: %s",hmderr.name,hmderr.message);
    			dbus_error_free_for_dcli(&hmderr);
    		}
    		return CMD_SUCCESS;
    	}

        dbus_message_iter_init(hmdreply,&iter);
	    dbus_message_iter_get_basic(&iter,&slot_num);
    	
	    /* loop */
	    if((slot_num > 0) && (slot_num <= MAX_SLOT_NUM)){
	        for (i = 0; i < slot_num; i++){
	            dbus_message_iter_next(&iter);	
		        dbus_message_iter_get_basic(&iter,&slot_no);
		        slotid[i] = slot_no;
    		}       
	    }
    	
		dbus_message_unref(hmdreply);

		for(i = 0; i < MAX_SLOT_NUM; i++){
		    if(slotid[i] != -1){
		        for(profile = 1; profile <= MAX_SLOT_NUM; profile++){
    		        dcli_vrrp_show_hansi_running_cfg(vty, profile, slotid[i]);
		        }
		    }
		}
		return CMD_SUCCESS;
	}
	
	/* in config node or hansi node, show special instance */
	if ((1 == argc) || (in_hansi_node == 1))
	{
	    //vty_out(vty,"bbbb\n");
		op_ret = dcli_vrrp_show_hansi_running_cfg(vty, profile, slot_id);
	}

	return CMD_SUCCESS;
}
#endif
DEFUN(config_vrrp_set_vip_back_down_cmd_func,
	config_vrrp_set_vip_back_down_cmd,
	"config smart (uplink|downlink|vgateway|l2-uplink) (enable|disable)",
	CONFIG_STR	
	"Config hansi smart link\n"
	"Config hansi smart uplink\n"
	"Config hansi smart downlink\n"
	"Config hansi smart vgateway\n"
	"Config hansi smart l2-uplink\n"
	"Enable hansi smart link\n"
	"Disable hansi smart link\n"
)
{
	DBusMessage *query = NULL, *reply = NULL;
	DBusError err ;
	unsigned int slot_id = HostSlotId;
	unsigned int op_ret = 0;
	unsigned int profile = 0;
	unsigned int set_flg = DCLI_VRRP_OFF;
	unsigned int link_type_flg = DCLI_VRRP_LINK_TYPE_INVALID;
	char vrrp_obj_path[DCLI_VRRP_OBJPATH_LEN] = {0};
	char vrrp_dbus_name[DCLI_VRRP_DBUSNAME_LEN] = {0};	

	if(argc < 2){
		vty_out(vty,"%% Bad parameter number!\n");
		return CMD_WARNING;
	}
	if (!strncmp(argv[0], "uplink", strlen(argv[0]))) {
		link_type_flg = DCLI_VRRP_LINK_TYPE_UPLINK;
	}else if (!strncmp(argv[0], "downlink", strlen(argv[0]))) {
		link_type_flg = DCLI_VRRP_LINK_TYPE_DOWNLINK;
	}else if (!strncmp(argv[0], "vgateway", strlen(argv[0]))) {
		link_type_flg = DCLI_VRRP_LINK_TYPE_VGATEWAY;
	}else if (!strncmp(argv[0], "l2-uplink", strlen(argv[0]))) {
		link_type_flg = DCLI_VRRP_LINK_TYPE_L2_UPLINK;
	}else {
		vty_out(vty, "%% Unknown command format!\n");
		return CMD_WARNING;
	}
	if (!strncmp(argv[1], "enable", strlen(argv[0]))) {
		set_flg = DCLI_VRRP_ON;
	}else if (!strncmp(argv[1], "disable", strlen(argv[0]))) {
		set_flg = DCLI_VRRP_OFF;
	}else {
		vty_out(vty, "%% Unknown command format!\n");
		return CMD_WARNING;
	}

	if (HANSI_NODE == vty->node) {
		profile = (unsigned int)(vty->index);
		slot_id = vty->slotindex;
	}
#ifdef DISTRIBUT
		DBusConnection *dcli_dbus_connection = NULL;
		ReInitDbusConnection(&dcli_dbus_connection,slot_id,distributFag);
#endif

	dcli_vrrp_splice_objpath_string(VRRP_DBUS_OBJPATH, profile, vrrp_obj_path);
	dcli_vrrp_splice_dbusname(VRRP_DBUS_BUSNAME, profile, vrrp_dbus_name);

	query = dbus_message_new_method_call(vrrp_dbus_name,
										 vrrp_obj_path,
										 VRRP_DBUS_INTERFACE,
										 VRRP_DBUS_METHOD_VIP_BACK_DOWN_FLG_SET);
	dbus_error_init(&err);

	dbus_message_append_args(query,
		                     DBUS_TYPE_UINT32, &profile,	
							 DBUS_TYPE_UINT32, &link_type_flg,
							 DBUS_TYPE_UINT32, &set_flg,
							 DBUS_TYPE_INVALID);
	
	reply = dbus_connection_send_with_reply_and_block(dcli_dbus_connection, query, -1, &err);
	dbus_message_unref(query);
	if (NULL == reply) {
		vty_out(vty, "failed get reply.\n");
		if (dbus_error_is_set(&err)) {
			printf("%s raised: %s", err.name, err.message);
			dbus_error_free_for_dcli(&err);
		}
		return CMD_SUCCESS;
	}
	
	if (dbus_message_get_args(reply, &err,
							DBUS_TYPE_UINT32, &op_ret,
							DBUS_TYPE_INVALID))
	{
        if (DCLI_VRRP_RETURN_CODE_OK != op_ret) {
			if(DCLI_VRRP_RETURN_CODE_VIP_NOT_EXIST == op_ret){
            	vty_out(vty, dcli_vrrp_err_msg[op_ret - DCLI_VRRP_RETURN_CODE_OK]);
			}
		}
	}
	else {
		if (dbus_error_is_set(&err)) 
		{
			printf("%s raised: %s",err.name,err.message);
			dbus_error_free_for_dcli(&err);
		}
	}

	dbus_message_unref(reply);
	return CMD_SUCCESS;
}

DEFUN(vrrp_conf_failover_ip_func,
	  vrrp_conf_failover_ip_cmd,
	  "config hansi dhcp-failover peer A.B.C.D local A.B.C.D",
	  CONFIG_STR
	  "Config hansi\n"
	  "Config hansi DHCP failover settings\n"
	  "Config hansi DHCP failover peer interface\n"
	  "Specify IP address of peer interface\n"
	  "Config hansi DHCP failover local interface\n"
	  "Specify IP address of local interface\n"
)
{
	DBusMessage *query = NULL;
	DBusMessage *reply = NULL;
	DBusError err;
	unsigned int profile = 0, localip = 0, peerip = 0, op_ret = 0;
	char vrrp_obj_path[DCLI_VRRP_OBJPATH_LEN] = {0};
	char vrrp_dbus_name[DCLI_VRRP_DBUSNAME_LEN] = {0};
	unsigned int slot_id = HostSlotId;

	if (HANSI_NODE == vty->node) {
		profile = (unsigned int)(vty->index);
		slot_id = vty->slotindex;
	}
	else {
		return CMD_WARNING;
	}

	peerip = dcli_ip2ulong((char*)argv[0]);
	if(2 == argc) {
		localip = dcli_ip2ulong((char*)argv[1]);
	}
	
	memset(vrrp_obj_path, 0, DCLI_VRRP_OBJPATH_LEN);
	memset(vrrp_dbus_name, 0, DCLI_VRRP_DBUSNAME_LEN);
	
#ifdef DISTRIBUT
	DBusConnection *dcli_dbus_connection = NULL;
	ReInitDbusConnection(&dcli_dbus_connection,slot_id,distributFag);
#endif
	dcli_vrrp_splice_objpath_string(VRRP_DBUS_OBJPATH, profile, vrrp_obj_path);
	dcli_vrrp_splice_dbusname(VRRP_DBUS_BUSNAME, profile, vrrp_dbus_name);
	
	query = dbus_message_new_method_call(vrrp_dbus_name,
										 vrrp_obj_path,
										 VRRP_DBUS_INTERFACE,
										 VRRP_DBUS_METHOD_SET_DHCP_FAILOVER_IPADDR);
	dbus_error_init(&err);
	dbus_message_append_args(query,
		                     DBUS_TYPE_UINT32,&profile,	
		                     DBUS_TYPE_UINT32,&peerip,
		                     DBUS_TYPE_UINT32,&localip,
							 DBUS_TYPE_INVALID);

	reply = dbus_connection_send_with_reply_and_block (dcli_dbus_connection, query, -1, &err);

	dbus_message_unref(query);
	if (NULL == reply) {
		printf("hansi%d set dhcp failover failed get reply.\n", profile);
		if (dbus_error_is_set(&err)) {
			printf("%s raised: %s",err.name,err.message);
			dbus_error_free_for_dcli(&err);
		}
		return CMD_WARNING;
	}
	else if (dbus_message_get_args ( reply, &err,
				DBUS_TYPE_UINT32,&op_ret,
				DBUS_TYPE_INVALID)){	
		if(op_ret) {
			vty_out(vty,dcli_vrrp_err_msg[op_ret - DCLI_VRRP_RETURN_CODE_OK]);
			//not pretty
        //	vty_out(vty, "%% Config failover failed %d!\n", op_ret);
			return CMD_WARNING;
		}
	} 
	else 
	{		
		if (dbus_error_is_set(&err)) 
		{
			printf("%s raised: %s",err.name,err.message);
			dbus_error_free_for_dcli(&err);
		}
	}
	dbus_message_unref(reply);
	
	return CMD_SUCCESS;
}

ALIAS(vrrp_conf_failover_ip_func,
	  vrrp_conf_failover_ip_peer_cmd,
	  "config hansi dhcp-failover peer A.B.C.D",
	  CONFIG_STR
	  "Config hansi\n"
	  "Config hansi DHCP failover settings\n"
	  "Config hansi DHCP failover peer interface\n"
	  "Specify IP address of peer interface\n"
);
DEFUN(vrrp_no_failover_ip_func,
	  vrrp_no_failover_ip_peer_cmd,
	  "no hansi dhcp-failover",
	  "Cancel hansi\n"
	  "Cancel hansi settings\n"
	  "Cancel hansi dhcp-failover configuration\n"
)
{
	DBusMessage *query = NULL;
	DBusMessage *reply = NULL;
	DBusError err;
	unsigned int profile = 0, op_ret = 0;
	char vrrp_obj_path[DCLI_VRRP_OBJPATH_LEN] = {0};
	char vrrp_dbus_name[DCLI_VRRP_DBUSNAME_LEN] = {0};
	unsigned int slot_id = HostSlotId;
	if (HANSI_NODE == vty->node) {
		profile = (unsigned int)(vty->index);
		slot_id = vty->slotindex;
	}
	else {
		return CMD_WARNING;
	}
	memset(vrrp_obj_path, 0, DCLI_VRRP_OBJPATH_LEN);
	memset(vrrp_dbus_name, 0, DCLI_VRRP_DBUSNAME_LEN);
	
#ifdef DISTRIBUT
	DBusConnection *dcli_dbus_connection = NULL;
	ReInitDbusConnection(&dcli_dbus_connection,slot_id,distributFag);
#endif
	dcli_vrrp_splice_objpath_string(VRRP_DBUS_OBJPATH, profile, vrrp_obj_path);
	dcli_vrrp_splice_dbusname(VRRP_DBUS_BUSNAME, profile, vrrp_dbus_name);
	query = dbus_message_new_method_call(vrrp_dbus_name,
										 vrrp_obj_path,
										 VRRP_DBUS_INTERFACE,
										 VRRP_DBUS_METHOD_CLEAR_DHCP_FAILOVER_IPADDR);
	dbus_error_init(&err);
	dbus_message_append_args(query,
							 DBUS_TYPE_UINT32,&profile, 
							 DBUS_TYPE_INVALID);
	reply = dbus_connection_send_with_reply_and_block (dcli_dbus_connection, query, -1, &err);
	dbus_message_unref(query);
	if (NULL == reply) {
		printf("hansi%d clear dhcp failover failed get reply.\n", profile);
		if (dbus_error_is_set(&err)) {
			printf("%s raised: %s",err.name,err.message);
			dbus_error_free_for_dcli(&err);
		}
		return CMD_WARNING;
	}
	else if (dbus_message_get_args ( reply, &err,
				DBUS_TYPE_UINT32,&op_ret,
				DBUS_TYPE_INVALID)){	
		if(op_ret) {
			vty_out(vty,dcli_vrrp_err_msg[op_ret - DCLI_VRRP_RETURN_CODE_OK]);
			//not pretty
			//vty_out(vty, "%% Clear failover setting failed %d!\n", op_ret);
			return CMD_WARNING;
		}
	} 
	else 
	{		
		if (dbus_error_is_set(&err)) 
		{
			printf("%s raised: %s",err.name,err.message);
			dbus_error_free_for_dcli(&err);
		}
	}
	dbus_message_unref(reply);
	return CMD_SUCCESS;
}

DEFUN(vrrp_config_gratuitous_arp_continous_value_func,
	  vrrp_config_gratuitous_arp_continous_value_cmd,
	  "config gratuitous-arp continous scale <1-300>",
	  CONFIG_STR
	  "Config hansi gratuitous arp send mode\n"
	  "Config hansi gratuitous arp continous send\n"
	  "Config hansi gratuitous arp continous send scale\n"
	  "Config hansi gratuitous arp continous send scale value\n"
)
{
	unsigned int profile = 0;
	unsigned int mode = HAD_GRATUITOUS_ARP_CONTINOUS;
	unsigned int value = 0;
	if (HANSI_NODE == vty->node) {
		profile = (unsigned int)(vty->index);
	}
	else {
		vty_out(vty,"%% Unexpected error : the node %d is not HANSI_NODE(%d)\n",vty->node,HANSI_NODE);
		return CMD_WARNING;
	}
	if(1 == argc){
		value = strtoul((char *)argv[0],NULL,0);
	}
	else{
		vty_out(vty,"%% Bad parameter number!\n");
	}
	return dcli_vrrp_config_continous_discrete_value(profile,mode,value);
}

DEFUN(vrrp_config_gratuitous_arp_discrete_value_func,
	  vrrp_config_gratuitous_arp_discrete_value_cmd,
	  "config gratuitous-arp discrete scale <1-50>",
	  CONFIG_STR
	  "Config hansi gratuitous arp send mode\n"
	  "Config hansi gratuitous arp continous send\n"
	  "Config hansi gratuitous arp continous send scale\n"
	  "Config hansi gratuitous arp continous send scale value\n"
)
{
	unsigned int profile = 0;
	unsigned int mode = HAD_GRATUITOUS_ARP_DISCRETE;
	unsigned int value = 0;
	if (HANSI_NODE == vty->node) {
		profile = (unsigned int)(vty->index);
	}
	else {
		vty_out(vty,"%% Unexpected error : the node %d is not HANSI_NODE(%d)\n",vty->node,HANSI_NODE);
		return CMD_WARNING;
	}
	if(1 == argc){
		value = strtoul((char *)argv[0],NULL,0);
	}
	else{
		vty_out(vty,"%% Bad parameter number!\n");
	}
	return dcli_vrrp_config_continous_discrete_value(profile,mode,value);
}

/***********************************************************************
 *
 * dcli_vrrp_config_continous_discrete_value()
 * DESCRIPTION:
 *		config gratuitous arp send mode for hansi
 * INPUTS:
 * 		unsigned int profile
 *		unsigned int mode
 *		unsigned int value
 * OUTPUTS:
 *		unsigned int op_ret
 * RETURN:
 *		CMD_WARNING
 *		CMD_SUCCESS
 * NOTE:
 *
 *************************************************************************/

int dcli_vrrp_config_continous_discrete_value
(
	unsigned int profile,
	unsigned int mode,
	unsigned value
)
	{
		DBusMessage *query = NULL;
		DBusMessage *reply = NULL;
		DBusError err;
		unsigned int op_ret = 0;
		char vrrp_obj_path[DCLI_VRRP_OBJPATH_LEN] = {0};
		char vrrp_dbus_name[DCLI_VRRP_DBUSNAME_LEN] = {0};
		
		memset(vrrp_obj_path, 0, DCLI_VRRP_OBJPATH_LEN);
		memset(vrrp_dbus_name, 0, DCLI_VRRP_DBUSNAME_LEN);
		dcli_vrrp_splice_objpath_string(VRRP_DBUS_OBJPATH, profile, vrrp_obj_path);
		dcli_vrrp_splice_dbusname(VRRP_DBUS_BUSNAME, profile, vrrp_dbus_name);
		query = dbus_message_new_method_call(vrrp_dbus_name,
											 vrrp_obj_path,
											 VRRP_DBUS_INTERFACE,
											 VRRP_DBUS_METHOD_CONFIG_CONTINOUS_DISCRETE_VALUE);
		dbus_error_init(&err);
		dbus_message_append_args(query,
								 DBUS_TYPE_UINT32,&profile, 
								 DBUS_TYPE_UINT32,&mode, 
								 DBUS_TYPE_UINT32,&value,							 
								 DBUS_TYPE_INVALID);
		reply = dbus_connection_send_with_reply_and_block (dcli_dbus_connection, query, -1, &err);
		dbus_message_unref(query);
		if (NULL == reply) {		
			if (dbus_error_is_set(&err)) {
				printf("%s raised: %s",err.name,err.message);
				dbus_error_free_for_dcli(&err);
			}
			return CMD_WARNING;
		}
		else if (dbus_message_get_args ( reply, &err,
					DBUS_TYPE_UINT32,&op_ret,
					DBUS_TYPE_INVALID)){	
			if(DCLI_VRRP_RETURN_CODE_OK != op_ret) {
				if((op_ret > DCLI_VRRP_RETURN_CODE_OK)&&\
					((op_ret - DCLI_VRRP_RETURN_CODE_OK) < (sizeof(dcli_vrrp_err_msg)/sizeof(dcli_vrrp_err_msg[0])))){
					vty_out(vty, "%s", dcli_vrrp_err_msg[op_ret - DCLI_VRRP_RETURN_CODE_OK]);
				}
				else{
					vty_out(vty,"%% Unknown error,ret %#x\n",op_ret);
				}
				
				dbus_message_unref(reply);
				return CMD_WARNING;
			}
		} 
		else 
		{		
			if (dbus_error_is_set(&err)) 
			{
				printf("%s raised: %s",err.name,err.message);
				dbus_error_free_for_dcli(&err);
			}
			
			dbus_message_unref(reply);
			return CMD_WARNING;
		}
		dbus_message_unref(reply);
		return CMD_SUCCESS;
	}
DEFUN(show_hansi_backup_switch_times_cmd_func,
	show_hansi_backup_switch_times_cmd,
	"show hansi switch-times",
	SHOW_STR	
	"Show hansi\n"
	"Switch times of MAST and none MAST state\n"
)
{
	DBusMessage *query = NULL, *reply = NULL;
	DBusError err ;
	DBusMessageIter	 iter;
	int instRun =0;

	unsigned int op_ret = 0;
	unsigned int profile = 0;
	char vrrp_obj_path[DCLI_VRRP_OBJPATH_LEN] = {0};
	char vrrp_dbus_name[DCLI_VRRP_DBUSNAME_LEN] = {0};	
	unsigned int backup_switch_times = 0;
   
   /*  get current profile id */
	if(HANSI_NODE==vty->node){
		profile = (unsigned int)(vty->index);
	}
	
	/* [1] check if had process has started before or not */
	instRun = dcli_vrrp_hansi_is_running(vty, profile);
	if (DCLI_VRRP_INSTANCE_NO_CREATED == instRun) {
		vty_out(vty,"%% Hansi profile %d not exist!\n",profile);
		return DCLI_VRRP_RETURN_CODE_PROFILE_NOTEXIST;
	}
	else if (DCLI_VRRP_INSTANCE_CHECK_FAILED == instRun) {
		vty_out(vty, "check had instance %d whether created was failed!\n",
					profile);
		return DCLI_VRRP_RETURN_CODE_ERR;
	}
	
	dcli_vrrp_splice_objpath_string(VRRP_DBUS_OBJPATH, profile, vrrp_obj_path);
	dcli_vrrp_splice_dbusname(VRRP_DBUS_BUSNAME, profile, vrrp_dbus_name);

	query = dbus_message_new_method_call(vrrp_dbus_name,
										 vrrp_obj_path,
										 VRRP_DBUS_INTERFACE,
										 VRRP_DBUS_METHOD_SHOW_SWITCH_TIMES);
	dbus_error_init(&err);

	dbus_message_append_args(query,
		                     DBUS_TYPE_UINT32,&profile,				 
							 DBUS_TYPE_INVALID);
	
	reply = dbus_connection_send_with_reply_and_block(dcli_dbus_connection, query, -1, &err);
	dbus_message_unref(query);
	if (NULL == reply) {
		vty_out(vty, "failed get reply.\n");
		if (dbus_error_is_set(&err)) {
			printf("%s raised: %s", err.name, err.message);
			dbus_error_free(&err);
		}
		return CMD_SUCCESS;
	}
	
	/* handle with abnormal return value */
	dbus_message_iter_init(reply,&iter);
	dbus_message_iter_get_basic(&iter,&op_ret);
	
	if((DCLI_VRRP_RETURN_CODE_PROFILE_NOTEXIST == op_ret)||\
		(DCLI_VRRP_RETURN_CODE_NO_CONFIG == op_ret)){
			if(DCLI_VRRP_RETURN_CODE_PROFILE_NOTEXIST == op_ret){
				 vty_out(vty,"%% Hansi profile %d not exist!\n",profile);
				 
			}else if(DCLI_VRRP_RETURN_CODE_NO_CONFIG == op_ret){
				 vty_out(vty, "%% Hansi %d has no configuration!\n", profile);
				 
			}	 
	   dbus_message_unref(reply);	
	   return op_ret;
	}
	
	/* get  backup_switch_times */
	dbus_message_iter_next(&iter);
	dbus_message_iter_get_basic(&iter,&backup_switch_times);
	vty_out(vty,"backup_switch_times = %d\n",backup_switch_times);
	
	dbus_message_unref(reply);
	
	return CMD_SUCCESS;
}
int dcli_vrrp_get_if_by_profile
(
    struct vty* vty,
    int profile,
    char* ifname1,
    char* ifname2
)
{
    DBusMessage *query, *reply;
	DBusError err;
	
	unsigned int ret;
	unsigned int flag;
	char* uplink_ifname = NULL;
	char* downlink_ifname = NULL;
	char vrrp_obj_path[DCLI_VRRP_OBJPATH_LEN] = {0};
	char vrrp_dbus_name[DCLI_VRRP_DBUSNAME_LEN] = {0};	

	dcli_vrrp_splice_objpath_string(VRRP_DBUS_OBJPATH, profile, vrrp_obj_path);
	dcli_vrrp_splice_dbusname(VRRP_DBUS_BUSNAME, profile, vrrp_dbus_name);

	query = dbus_message_new_method_call(vrrp_dbus_name,
										vrrp_obj_path,
										VRRP_DBUS_INTERFACE,
										VRRP_DBUS_METHOD_GET_IFNAME);
    dbus_error_init(&err);
	dbus_message_append_args(query,
									DBUS_TYPE_UINT32,&profile,
									DBUS_TYPE_INVALID);
 	
	reply = dbus_connection_send_with_reply_and_block (dcli_dbus_connection,query,-1, &err);
	dbus_message_unref(query);
	if (NULL == reply) {		
		vty_out(vty,"failed get reply.\n");	
		if (dbus_error_is_set(&err)) {
			vty_out(vty,"%s raised: %s",err.name,err.message);
			dbus_error_free_for_dcli(&err);
		}
		return CMD_SUCCESS;
	}
	if (dbus_message_get_args ( reply, &err,
		                            DBUS_TYPE_UINT32,&ret,
									DBUS_TYPE_STRING,&uplink_ifname,
									DBUS_TYPE_STRING,&downlink_ifname,
									DBUS_TYPE_INVALID)) {
		 memcpy(ifname1,uplink_ifname,strlen(uplink_ifname));
		 memcpy(ifname2,downlink_ifname,strlen(downlink_ifname));
	}
	else {		
		vty_out(vty,"Failed get args.\n");		
		if (dbus_error_is_set(&err)) {
				vty_out(vty,"%s raised: %s",err.name,err.message);
				dbus_error_free_for_dcli(&err);
		}
	}
		
	dbus_message_unref(reply);
	return ret;  
}

void dcli_vrrp_notify_to_npd
(
    struct vty* vty,
    char* ifname1,
    char* ifname2,
    int   add
)
{
	DBusMessage *query, *reply;
	DBusError err;
    unsigned int op_ret;
	if(NULL == ifname1){
       ifname1 = ifname2;
	}
	query = dbus_message_new_method_call(
								NPD_DBUS_BUSNAME,			\
								NPD_DBUS_OBJPATH,			\
								NPD_DBUS_INTERFACE,					\
								NPD_DBUS_FDB_METHOD_CREATE_VRRP_BY_IFNAME);
	
	dbus_error_init(&err);

	dbus_message_append_args(query,
							 DBUS_TYPE_STRING,&ifname1,
							 DBUS_TYPE_STRING,&ifname2,
							 DBUS_TYPE_UINT32,&add,
							 DBUS_TYPE_INVALID);
    
	reply = dbus_connection_send_with_reply_and_block (dcli_dbus_connection,query,-1, &err);
	
	dbus_message_unref(query);
	if (NULL == reply) {
		vty_out(vty,"Failed to get reply!\n");
		if (dbus_error_is_set(&err)) {
			dbus_error_free_for_dcli(&err);
			vty_out(vty,"Failed to get reply!\n");
		}
		return CMD_SUCCESS;
	}
	else{
			if (dbus_message_get_args ( reply, &err,
				DBUS_TYPE_UINT32,&op_ret,
				DBUS_TYPE_INVALID)) {
                vty_out(vty,"get return value %d\n",op_ret);
			} 
			else {
				if (dbus_error_is_set(&err)) {
					dbus_error_free_for_dcli(&err);
					vty_out(vty,"failed to get return value!\n");
				}
			}
	}
	dbus_message_unref(reply);
	return;

}
extern struct vtysh_client vtysh_client[];
int g_bridge_mcast_config_save()
{
	FILE* fp = NULL;
	char cmd[64];
	char buf[512]={0};
	sprintf(cmd,"sudo chmod 777 %s	>/dev/NULL","/var/run/hmd/mcast_config_save");

	fp = fopen("/var/run/hmd/mcast_config_save", "r");
	if(NULL == fp) {
		return -1;
	}
	else 
	{
		while(fgets(buf,512,fp))
		{
			vtysh_add_show_string(buf);
		}
		fclose(fp);
	}
	return 0;
}

#ifdef DISTRIBUT 
#if 1
int dcli_vrrp_show_running_cfg(struct vty *vty)
{	
	char *showStr = NULL, ch = 0,tmpBuf[SHOWRUN_PERLINE_SIZE] = {0};
	DBusMessage *query, *reply;
	DBusError err;

	int profile = 0;
	int instRun = 0, ret = 0;
	char vrrp_obj_path[DCLI_VRRP_OBJPATH_LEN] = {0};
	char vrrp_dbus_name[DCLI_VRRP_DBUSNAME_LEN] = {0};
	/* flag for one time insert string "VRRP section" into show running-config  */
	int bulid_vrrp_moudle_flg = 0;
	char *showRunningCfg_str = NULL;
	char *cursor = NULL;
	int totalLen = 0;
	char *tmp = NULL;
	char wireless_cmd[] = "show interface wireless_config remote";	
	char ebr_cmd[] = "show interface ebr_config remote";
	char wireless_cmd1[128];
	showRunningCfg_str = (char *)malloc(DCLI_VRRP_SHOW_RUNNING_CFG_LEN);
	int check_result = DCLI_VRRP_INSTANCE_CREATED;
	unsigned char service_name[4][4] = {"had","wid", "asd", "wsm"};
	unsigned int service_index = 0;
	int localid = 0;
	int slot_id = 0;
	int old_slot_id = -1;
	int dhcp_flag = 0;
	int create_cfg_flag = 0;
	if (NULL == showRunningCfg_str) {
		//syslog_ax_vlan_err("memory malloc error\n");
		return 1;
	}
	memset(showRunningCfg_str, 0, DCLI_VRRP_SHOW_RUNNING_CFG_LEN);

	if (0 == bulid_vrrp_moudle_flg) {
		char _tmpstr[64];
		memset(_tmpstr,0,64);
		sprintf(_tmpstr, BUILDING_MOUDLE, "VRRP");
		vtysh_add_show_string(_tmpstr);
		bulid_vrrp_moudle_flg = 1;
	}

	/* loop to get configures on the had instance */
	for (slot_id = 0; slot_id <= MAX_SLOT_NUM; slot_id++) {
		if(active_master_slot == slot_id)
		{dhcp_flag = 1;
			tmp = dcli_dhcp_show_running_cfg2(slot_id);
			if(tmp != NULL){
				if (0 != strlen(tmp)){
					dcli_config_writev2(tmp,slot_id,0,"dhcp_poll",0); 
					if(tmp){
						free(tmp);
						tmp = NULL;
					}
				}else{
					free(showRunningCfg_str);
					return 1;
				}
			}
		}
		else
			dhcp_flag = 0;
	for (profile = 0; profile <= DCLI_VRRP_INSTANCE_CNT; profile++) {
	    create_cfg_flag = 1;
		/* [1] check if had process has started before or not */
		instRun = dcli_hmd_hansi_is_running(vty,slot_id, 0, profile);
		if (INSTANCE_NO_CREATED == instRun) {
			//vty_out(vty, "had instance %d not created!\n", profile);
			continue;
		}
		else if (DCLI_VRRP_INSTANCE_CHECK_FAILED == instRun) {
			//vty_out(vty, "check had instance %d whether created was failed!\n",
			//			profile);
			continue;
		}
#if 0
		/* profile = 0, it's not necessary create wcpss process */
		if (0 != profile) {
			/* check wheather wcpss instance started completely.
			 * wsm not support multi-process.
			 */
			for (service_index = 1; service_index < 3; service_index++) {
				check_result = dcli_vrrp_check_service_started(vty, service_name[service_index], profile);
				if (DCLI_VRRP_INSTANCE_CREATED != check_result) {
					break;					
				}
			}
		}
		if (DCLI_VRRP_INSTANCE_CREATED != check_result) {
				continue;					
		}
#endif
		/* [2] init string showRunningCfg_str
		 *     fill vrrp start string in showRunningCfg_str
		 */
		memset(showRunningCfg_str, 0, DCLI_VRRP_SHOW_RUNNING_CFG_LEN);
		cursor = showRunningCfg_str;
		totalLen = 0;
		

		/* [3] fill wid running config string in showRunningCfg_str */
#ifdef _D_WCPSS_
		totalLen += sprintf(cursor, "config hansi-profile %d-%d\n", slot_id,profile);
				cursor = showRunningCfg_str + totalLen;

		if (0 != profile) {
		    dcli_config_write("",localid,slot_id,profile,create_cfg_flag,0);
		    if(create_cfg_flag)
		        create_cfg_flag = 0;
		    
			if(dhcp_flag == 0){
				dhcp_flag = 1;
				tmp = dcli_dhcp_show_running_cfg2(slot_id); 		
				if(tmp != NULL){
					if (0 != strlen(tmp)) { 				
						totalLen += sprintf(cursor, " \n");
						cursor = showRunningCfg_str + totalLen;
		
						totalLen += sprintf(cursor, "%s\n",tmp);
						cursor = showRunningCfg_str + totalLen;
					}
					free(tmp);
					tmp = NULL;
				}else{
					free(showRunningCfg_str);
					return 1;
				}
				totalLen += sprintf(cursor, " exit\n", profile);
						cursor = showRunningCfg_str + totalLen;				
				dcli_config_write(showRunningCfg_str,localid,slot_id,profile,0,0);
				dcli_config_writev2(showRunningCfg_str,slot_id,0,"dhcp_poll",0); 	
				vtysh_add_show_string(showRunningCfg_str);		
				memset(showRunningCfg_str, 0, DCLI_VRRP_SHOW_RUNNING_CFG_LEN);
				cursor = showRunningCfg_str;
				totalLen = 0;
				totalLen += sprintf(cursor, "config hansi-profile %d-%d\n", slot_id,profile);
						cursor = showRunningCfg_str + totalLen;
				/*dhcp ipv6 section*/
				tmp = dcli_dhcp_ipv6_show_running_cfg2(slot_id); 		
				if(tmp != NULL){
					if (0 != strlen(tmp)) { 				
						totalLen += sprintf(cursor, " \n");
						cursor = showRunningCfg_str + totalLen;
		
						totalLen += sprintf(cursor, "%s\n",tmp);
						cursor = showRunningCfg_str + totalLen;
					}
					free(tmp);
					tmp = NULL;
				}else{
					free(showRunningCfg_str);
					return 1;
				}
				totalLen += sprintf(cursor, " exit\n", profile);
						cursor = showRunningCfg_str + totalLen;
				dcli_config_write(showRunningCfg_str,localid,slot_id,profile,0,0);		
				dcli_config_writev2(showRunningCfg_str,slot_id,0,"dhcp_ipv6",0);
				vtysh_add_show_string(showRunningCfg_str);	
				memset(showRunningCfg_str, 0, DCLI_VRRP_SHOW_RUNNING_CFG_LEN);
				cursor = showRunningCfg_str;
				totalLen = 0;
				totalLen += sprintf(cursor, "config hansi-profile %d-%d\n", slot_id,profile);
						cursor = showRunningCfg_str + totalLen;
				tmp = dcli_dhcrelay_show_running_cfg2(vty,slot_id); 		
				if(tmp != NULL){
					if (0 != strlen(tmp)) { 				
						totalLen += sprintf(cursor, " \n");
						cursor = showRunningCfg_str + totalLen;
		
						totalLen += sprintf(cursor, "%s\n",tmp);
						cursor = showRunningCfg_str + totalLen;
					}
					free(tmp);
					tmp = NULL;
				}else{
					free(showRunningCfg_str);
					return 1;
				}
				totalLen += sprintf(cursor, " exit\n", profile);
						cursor = showRunningCfg_str + totalLen;
				dcli_config_write(showRunningCfg_str,localid,slot_id,profile,0,0);		
				dcli_config_writev2(showRunningCfg_str,slot_id,0,"dhcp_relay",0); 	
				vtysh_add_show_string(showRunningCfg_str);		
				memset(showRunningCfg_str, 0, DCLI_VRRP_SHOW_RUNNING_CFG_LEN);
				cursor = showRunningCfg_str;
				totalLen = 0;
				totalLen += sprintf(cursor, "config hansi-profile %d-%d\n", slot_id,profile);
						cursor = showRunningCfg_str + totalLen;

				tmp = dcli_dhcp_snp_show_running_config2(vty, slot_id); 		
				if(tmp != NULL){
					if (0 != strlen(tmp)) { 				
						totalLen += sprintf(cursor, " \n");
						cursor = showRunningCfg_str + totalLen;
		
						totalLen += sprintf(cursor, "%s\n",tmp);
						cursor = showRunningCfg_str + totalLen;
					}
					free(tmp);
					tmp = NULL;
				}else{
					free(showRunningCfg_str);
					return 1;
				}				
				totalLen += sprintf(cursor, " exit\n", profile);
						cursor = showRunningCfg_str + totalLen;
				dcli_config_write(showRunningCfg_str,localid,slot_id,profile,0,0);		
				dcli_config_writev2(showRunningCfg_str,slot_id,0,"dhcp_snp",0); 	
				vtysh_add_show_string(showRunningCfg_str);		
				memset(showRunningCfg_str, 0, DCLI_VRRP_SHOW_RUNNING_CFG_LEN);
				cursor = showRunningCfg_str;
				totalLen = 0;
				totalLen += sprintf(cursor, "config hansi-profile %d-%d\n", slot_id,profile);
						cursor = showRunningCfg_str + totalLen;
			}
			tmp = dcli_hansi_ebr_show_running_config_start(localid, slot_id,profile);
			if(tmp != NULL){
				if (0 != strlen(tmp)) {
					totalLen += sprintf(cursor, "%s\n",tmp);
					cursor = showRunningCfg_str + totalLen;
				}
				free(tmp);
				tmp = NULL;
			}else{
				free(showRunningCfg_str);
				return 1;
			}
			memset(wireless_cmd1, 0, 128);
			sprintf(wireless_cmd1, "%s %d %d",ebr_cmd,slot_id,profile);
			tmp = vtysh_client_config_wireless_interface(&vtysh_client[0], wireless_cmd1); 	
			if(tmp != NULL){
				if (0 != strlen(tmp)) {
					totalLen += sprintf(cursor, "%s\n",tmp);
					cursor = showRunningCfg_str + totalLen;
				}
				XFREE(MTYPE_TMP,tmp);
				tmp = NULL;
			}
			
			if(active_master_slot == slot_id)
				dcli_config_write(showRunningCfg_str,localid,slot_id,profile,1,0);		
			else
				dcli_config_write(showRunningCfg_str,localid,slot_id,profile,0,0);				
			dcli_config_writev2(showRunningCfg_str,slot_id,profile,"hansi_wcpss",0);		
			vtysh_add_show_string(showRunningCfg_str);		
			memset(showRunningCfg_str, 0, DCLI_VRRP_SHOW_RUNNING_CFG_LEN);
			cursor = showRunningCfg_str;
			totalLen = 0;
			tmp = dcli_hansi_security_show_running_config(localid, slot_id,profile);
			if(tmp != NULL){
				if (0 != strlen(tmp)) {
					totalLen += sprintf(cursor, "%s\n",tmp);
					cursor = showRunningCfg_str + totalLen;
				}
				free(tmp);
				tmp = NULL;
			}else{
				free(showRunningCfg_str);
				return 1;
			}
			tmp = dcli_hansi_ac_ip_list_show_running_config(localid, slot_id,profile);
			if(tmp != NULL){
				if (0 != strlen(tmp)) {
					totalLen += sprintf(cursor, "%s\n",tmp);
					cursor = showRunningCfg_str + totalLen;
				}
				free(tmp);
				tmp = NULL;
			}else{
				free(showRunningCfg_str);
				return 1;
			}
			tmp = dcli_hansi_wlan_show_running_config_start(localid, slot_id,profile);
			if(tmp != NULL){
				if (0 != strlen(tmp)) {
					totalLen += sprintf(cursor, "%s\n",tmp);
					cursor = showRunningCfg_str + totalLen;
				}
				free(tmp);
				tmp = NULL;
			}else{
				free(showRunningCfg_str);
				return 1;
			}

			/*yjl copy from aw3.1.2 for local forwarding.2014-2-28*/
			tmp = dcli_hansi_vir_dhcp_show_running_config(localid, slot_id,profile);
			if(tmp != NULL){
				if (0 != strlen(tmp)) {
					totalLen += sprintf(cursor, "%s\n",tmp);
					cursor = showRunningCfg_str + totalLen;
				}
				free(tmp);
				tmp = NULL;
			}else{
				free(showRunningCfg_str);
				return 1;
			}
			/*end**************************************************/
			
			tmp = dcli_hansi_wtp_show_running_config_start(localid, slot_id,profile);
			if(tmp != NULL){
				if (0 != strlen(tmp)) {
					totalLen += sprintf(cursor, "%s\n",tmp);
					cursor = showRunningCfg_str + totalLen;
				}
				free(tmp);
				tmp = NULL;
			}else{
				free(showRunningCfg_str);
				return 1;
			}
			tmp = dcli_hansi_wlan_list_show_running_config(localid, slot_id,profile);
			if(tmp != NULL){
				if (0 != strlen(tmp)) {
					totalLen += sprintf(cursor, "%s\n",tmp);
					cursor = showRunningCfg_str + totalLen;
				}
				free(tmp);
				tmp = NULL;
			}else{
				free(showRunningCfg_str);
				return 1;
			}
			tmp = dcli_hansi_wtp_list_show_running_config(localid, slot_id,profile);
			if(tmp != NULL){
				if (0 != strlen(tmp)) {
					totalLen += sprintf(cursor, "%s\n",tmp);
					cursor = showRunningCfg_str + totalLen;
				}
				free(tmp);
				tmp = NULL;
			}else{
				free(showRunningCfg_str);
				return 1;
			}
			tmp = dcli_hansi_bss_list_show_running_config(localid, slot_id,profile);
			if(tmp != NULL){
				if (0 != strlen(tmp)) {
					totalLen += sprintf(cursor, "%s\n",tmp);
					cursor = showRunningCfg_str + totalLen;
				}
				free(tmp);
				tmp = NULL;
			}else{
				free(showRunningCfg_str);
				return 1;
			}
			memset(wireless_cmd1, 0, 128);
			sprintf(wireless_cmd1, "%s %d %d",wireless_cmd,slot_id,profile);
			tmp = vtysh_client_config_wireless_interface(&vtysh_client[0], wireless_cmd1); 	
			if(tmp != NULL){
				if (0 != strlen(tmp)) {
					totalLen += sprintf(cursor, "%s\n",tmp);					
//					dcli_config_write(tmp,0,slot_id,profile,0,0);
					cursor = showRunningCfg_str + totalLen;
				}
				XFREE(MTYPE_TMP,tmp);
				tmp = NULL;
			}		
			dcli_config_write(showRunningCfg_str,localid,slot_id,profile,0,0);		
			dcli_config_writev2(showRunningCfg_str,slot_id,profile,"hansi_wcpss",1);		
			vtysh_add_show_string(showRunningCfg_str);		
			memset(showRunningCfg_str, 0, DCLI_VRRP_SHOW_RUNNING_CFG_LEN);
			cursor = showRunningCfg_str;
			totalLen = 0;
			
			memset(wireless_cmd1, 0, 128);
			sprintf(wireless_cmd1, "%s %d %d",ebr_cmd,slot_id,profile);
			tmp = vtysh_client_config_wireless_interface(&vtysh_client[0], wireless_cmd1); 	
			if(tmp != NULL){
				dcli_config_write(tmp,localid,slot_id,profile,0,0);				
				dcli_config_writev2(tmp,slot_id,profile,"hansi_wcpss",1);		
				XFREE(MTYPE_TMP,tmp);
				tmp = NULL;
			}
			tmp = dcli_hansi_wlan_show_running_config_end(localid, slot_id,profile);
			if(tmp != NULL){
				if (0 != strlen(tmp)) {
					totalLen += sprintf(cursor, "%s\n",tmp);
					cursor = showRunningCfg_str + totalLen;
				}
				free(tmp);
				tmp = NULL;
			}else{
				free(showRunningCfg_str);
				return 1;
			}
			tmp = dcli_hansi_wtp_show_running_config_end(localid, slot_id,profile);
			if(tmp != NULL){
				if (0 != strlen(tmp)) {
					totalLen += sprintf(cursor, "%s\n",tmp);
					cursor = showRunningCfg_str + totalLen;
				}
				free(tmp);
				tmp = NULL;
			}else{
				free(showRunningCfg_str);
				return 1;
			}
			tmp = dcli_hansi_ebr_show_running_config_end(localid, slot_id,profile);
			if(tmp != NULL){
				if (0 != strlen(tmp)) {
					totalLen += sprintf(cursor, "%s\n",tmp);
					cursor = showRunningCfg_str + totalLen;
				}
				free(tmp);
				tmp = NULL;
			}else{
				free(showRunningCfg_str);
				return 1;
			}			
			totalLen += sprintf(cursor, " exit\n", profile);
					cursor = showRunningCfg_str + totalLen;
					dcli_config_write(showRunningCfg_str,localid,slot_id,profile,0,0);		
			dcli_config_writev2(showRunningCfg_str,slot_id,profile,"hansi_wcpss",1);		
			vtysh_add_show_string(showRunningCfg_str);	

			/*hmd hansi config start*/			
			memset(showRunningCfg_str, 0, DCLI_VRRP_SHOW_RUNNING_CFG_LEN);
			cursor = showRunningCfg_str;
			totalLen = 0;
			totalLen += sprintf(cursor, "config hansi-profile %d-%d\n", slot_id,profile);
					cursor = showRunningCfg_str + totalLen;
			tmp = dcli_hmd_show_running_config(localid, slot_id,profile);
			if(tmp != NULL){
				if (0 != strlen(tmp)) {
					totalLen += sprintf(cursor, "%s\n",tmp);
					cursor = showRunningCfg_str + totalLen;
				}
				free(tmp);
				tmp = NULL;
			}else{
				free(showRunningCfg_str);
				return 1;
			}
			totalLen += sprintf(cursor, " exit\n", profile);
					cursor = showRunningCfg_str + totalLen;
			dcli_config_write(showRunningCfg_str,localid,slot_id,profile,0,0);		
			dcli_config_writev2(showRunningCfg_str,slot_id,profile,"hmd_hansi_config",0);		
			vtysh_add_show_string(showRunningCfg_str);		
			/*hmd hansi config end*/			
			memset(showRunningCfg_str, 0, DCLI_VRRP_SHOW_RUNNING_CFG_LEN);
			cursor = showRunningCfg_str;
			totalLen = 0;
			totalLen += sprintf(cursor, "config hansi-profile %d-%d\n", slot_id,profile);
					cursor = showRunningCfg_str + totalLen;
			/*eag*/
			//memset(showRunningCfg_str, 0, DCLI_VRRP_SHOW_RUNNING_CFG_LEN);
			//cursor = showRunningCfg_str;
			//totalLen = 0;
			tmp = dcli_eag_show_running_config_2(localid, slot_id,profile);
			if(tmp != NULL){
				if (0 != strlen(tmp)) {
					totalLen += sprintf(cursor, "%s\n",tmp);
					cursor = showRunningCfg_str + totalLen;
				}
				free(tmp);
				tmp = NULL;
			}else{
				free(showRunningCfg_str);
				return 1;
			}
			totalLen += sprintf(cursor, " exit\n", profile);
					cursor = showRunningCfg_str + totalLen;
			dcli_config_write(showRunningCfg_str,localid,slot_id,profile,0,0);		
			dcli_config_writev2(showRunningCfg_str,slot_id,profile,"hansi_eag",0);		
			vtysh_add_show_string(showRunningCfg_str);		
			memset(showRunningCfg_str, 0, DCLI_VRRP_SHOW_RUNNING_CFG_LEN);
			cursor = showRunningCfg_str;
			totalLen = 0;
			totalLen += sprintf(cursor, "config hansi-profile %d-%d\n", slot_id,profile);
					cursor = showRunningCfg_str + totalLen;
			//vtysh_add_show_string(showRunningCfg_str);
			/*end*/

			/* fast forward distribute config */
			if (slot_id != old_slot_id  && 0 != slot_id)
			{
				old_slot_id = slot_id;
				tmp = se_agent_show_running_cfg_2(localid, slot_id,profile);
				if(tmp != NULL){
					if (0 != strlen(tmp)) {
						totalLen += sprintf(cursor, "%s\n",tmp);
						cursor = showRunningCfg_str + totalLen;
					}
					free(tmp);
					tmp = NULL;
				}else{
					free(showRunningCfg_str);
					return 1;
				}
				totalLen += sprintf(cursor, " exit\n", profile);
						cursor = showRunningCfg_str + totalLen;
				dcli_config_write(showRunningCfg_str,localid,slot_id,profile,0,0);		
				dcli_config_writev2(showRunningCfg_str,slot_id,profile,"hansi-fastfwd",0);		
				vtysh_add_show_string(showRunningCfg_str);		
				memset(showRunningCfg_str, 0, DCLI_VRRP_SHOW_RUNNING_CFG_LEN);
				cursor = showRunningCfg_str;
				totalLen = 0;
				totalLen += sprintf(cursor, "config hansi-profile %d-%d\n", slot_id,profile);
						cursor = showRunningCfg_str + totalLen;
			}
			/*end*/
			
			/* pdc */
			tmp = dcli_pdc_show_running_config_2(localid, slot_id,profile);
			if(tmp != NULL){
				if (0 != strlen(tmp)) {
					totalLen += sprintf(cursor, "%s\n",tmp);
					cursor = showRunningCfg_str + totalLen;
				}
				free(tmp);
				tmp = NULL;
			}else{
				free(showRunningCfg_str);
				return 1;
			}			
			totalLen += sprintf(cursor, " exit\n", profile);
					cursor = showRunningCfg_str + totalLen;
					dcli_config_write(showRunningCfg_str,localid,slot_id,profile,0,0);		
			dcli_config_writev2(showRunningCfg_str,slot_id,profile,"hansi_pdc",0);		
			vtysh_add_show_string(showRunningCfg_str);		
			memset(showRunningCfg_str, 0, DCLI_VRRP_SHOW_RUNNING_CFG_LEN);
			cursor = showRunningCfg_str;
			totalLen = 0;
			totalLen += sprintf(cursor, "config hansi-profile %d-%d\n", slot_id,profile);
					cursor = showRunningCfg_str + totalLen;
			/*end*/
			/* rdc */
			tmp = dcli_rdc_show_running_config_2(localid, slot_id,profile);
			if(tmp != NULL){
				if (0 != strlen(tmp)) {
					totalLen += sprintf(cursor, "%s\n",tmp);
					cursor = showRunningCfg_str + totalLen;
				}
				free(tmp);
				tmp = NULL;
			}else{
				free(showRunningCfg_str);
				return 1;
			}			
			totalLen += sprintf(cursor, " exit\n", profile);
					cursor = showRunningCfg_str + totalLen;
			dcli_config_write(showRunningCfg_str,localid,slot_id,profile,0,0);		
			dcli_config_writev2(showRunningCfg_str,slot_id,profile,"hansi_rdc",0);		
			vtysh_add_show_string(showRunningCfg_str);		

			/* for air monitor service begin */
			memset(showRunningCfg_str, 0, DCLI_VRRP_SHOW_RUNNING_CFG_LEN);
			cursor = showRunningCfg_str;
			totalLen = 0;
			totalLen += sprintf(cursor, "config hansi-profile %d-%d\n",slot_id, profile);
			cursor = showRunningCfg_str + totalLen;
			tmp = dcli_hansi_scanlocate_show_running_config_start(localid, slot_id, profile); 
			if(tmp != NULL){
				if (0 != strlen(tmp)) {
					totalLen += sprintf(cursor, "%s\n",tmp);
					cursor = showRunningCfg_str + totalLen;
				}
				free(tmp);
				tmp = NULL;
			}else{
				free(showRunningCfg_str);
				return 1;
			}

			totalLen += sprintf(cursor, " exit\n", profile);
			cursor = showRunningCfg_str + totalLen;
			dcli_config_write(showRunningCfg_str,localid,slot_id,profile,0,0);

			dcli_config_writev2(showRunningCfg_str,slot_id,profile, "AIR-MONITOR", 0);

			vtysh_add_show_string(showRunningCfg_str);
			//dcli_vrrp_update_configv2(0, 0, profile, "AIR-MONITOR");

			/* for air monitor service end */
	
			/* pppoe config*/
		#ifndef _VERSION_18SP7_
			tmp = dcli_pppoe_show_running_config(localid, slot_id, profile);
			if (tmp){
				if (strlen(tmp)) {
					totalLen += sprintf(cursor, "config hansi-profile %d-%d\n", slot_id, profile);
					cursor = showRunningCfg_str + totalLen;			
					
					totalLen += sprintf(cursor, "%s\n",tmp);
					cursor = showRunningCfg_str + totalLen;
					
					totalLen += sprintf(cursor, " exit\n", profile);
					cursor = showRunningCfg_str + totalLen;

					dcli_config_write(showRunningCfg_str, localid, slot_id, profile, 0, 0);		
					dcli_config_writev2(showRunningCfg_str, slot_id, profile, "hansi_pppoe", 0);		
					vtysh_add_show_string(showRunningCfg_str);		
					memset(showRunningCfg_str, 0, DCLI_VRRP_SHOW_RUNNING_CFG_LEN);
					cursor = showRunningCfg_str;
					totalLen = 0;
				}
				free(tmp);
				tmp = NULL;
			}
		#endif /* !_VERSION_18SP7_ */	
			/*end*/
		}
		
		/* dhcp config section */
		memset(showRunningCfg_str, 0, DCLI_VRRP_SHOW_RUNNING_CFG_LEN);
		cursor = showRunningCfg_str;
		totalLen = 0;
		tmp = dcli_dhcp_show_running_hansi_cfg(slot_id,profile,localid);
		if(tmp != NULL){
			if (0 != strlen(tmp)) {
				totalLen += sprintf(cursor, "%s\n",tmp);
				cursor = showRunningCfg_str + totalLen;
			}
			free(tmp);
			tmp = NULL;
		}else{
			free(showRunningCfg_str);
			return 1;
		}
		dcli_config_write(showRunningCfg_str,localid,slot_id,profile,create_cfg_flag,0);
		dcli_config_writev2(showRunningCfg_str,slot_id,profile,"hansi_dhcp",0);		
		vtysh_add_show_string(showRunningCfg_str);		
		memset(showRunningCfg_str, 0, DCLI_VRRP_SHOW_RUNNING_CFG_LEN);
		cursor = showRunningCfg_str;
		totalLen = 0;
		/*dhcp ipv6 section*/
		tmp = dcli_dhcp6_show_running_hansi_cfg(slot_id,profile,localid);
		if(tmp != NULL){
			if (0 != strlen(tmp)) {
				totalLen += sprintf(cursor, "%s\n",tmp);
				cursor = showRunningCfg_str + totalLen;
			}
			free(tmp);
			tmp = NULL;
		}else{
			free(showRunningCfg_str);
			return 1;
		}
		dcli_config_write(showRunningCfg_str,localid,slot_id,profile,create_cfg_flag,0);
		dcli_config_writev2(showRunningCfg_str,slot_id,profile,"hansi_dhcp_ipv6",0);
		vtysh_add_show_string(showRunningCfg_str);
		memset(showRunningCfg_str, 0, DCLI_VRRP_SHOW_RUNNING_CFG_LEN);
		cursor = showRunningCfg_str;
		totalLen = 0;
		tmp = dcli_dhcrelay_show_running_hansi_cfg(slot_id,profile,localid);
		if(tmp != NULL){
			if (0 != strlen(tmp)) {
				totalLen += sprintf(cursor, "%s\n",tmp);
				cursor = showRunningCfg_str + totalLen;
			}
			free(tmp);
			tmp = NULL;
		}else{
			free(showRunningCfg_str);
			return 1;
		}
		dcli_config_write(showRunningCfg_str,localid,slot_id,profile,0,0);		
		dcli_config_writev2(showRunningCfg_str,slot_id,profile,"hansi_dhcprelay",0);		
		vtysh_add_show_string(showRunningCfg_str);		
		memset(showRunningCfg_str, 0, DCLI_VRRP_SHOW_RUNNING_CFG_LEN);
		cursor = showRunningCfg_str;
		totalLen = 0;
		tmp = dcli_dhcp_snp_show_running_hansi_cfg(slot_id,profile,localid);
		if(tmp != NULL){
			if (0 != strlen(tmp)) {
				totalLen += sprintf(cursor, "%s\n",tmp);
				cursor = showRunningCfg_str + totalLen;
			}
			free(tmp);
			tmp = NULL;
		}else{
			free(showRunningCfg_str);
			return 1;
		}
		dcli_config_write(showRunningCfg_str,localid,slot_id,profile,0,0);		
		dcli_config_writev2(showRunningCfg_str,slot_id,profile,"hansi_dhcpsnp",0);		
		vtysh_add_show_string(showRunningCfg_str);		
		memset(showRunningCfg_str, 0, DCLI_VRRP_SHOW_RUNNING_CFG_LEN);
		cursor = showRunningCfg_str;
		totalLen = 0;				
		//dcli_config_write(showRunningCfg_str,localid,slot_id,profile,0,0);		
	//	vtysh_add_show_string(showRunningCfg_str);
#endif		
		}
}	
	/* loop to get configures on the had instance */
	for (slot_id = 1; slot_id <= MAX_SLOT_NUM; slot_id++) 	
	for (profile = 1; profile <= DCLI_VRRP_INSTANCE_CNT; profile++) {
		/* [1] check if had process has started before or not */
		instRun = dcli_hmd_hansi_is_running(vty,slot_id,0,profile);
		if (INSTANCE_NO_CREATED == instRun) {
			//vty_out(vty, "had instance %d not created!\n", profile);
			continue;
		}
		else if (DCLI_VRRP_INSTANCE_CHECK_FAILED == instRun) {
			//vty_out(vty, "check had instance %d whether created was failed!\n",
			//			profile);
			continue;
		}

		/* [2] init string showRunningCfg_str
		 *     fill vrrp start string in showRunningCfg_str
		 */
		memset(showRunningCfg_str, 0, DCLI_VRRP_SHOW_RUNNING_CFG_LEN);
		cursor = showRunningCfg_str;
		totalLen = 0;
		/*hansi config real start here */
		totalLen += sprintf(cursor, "config hansi-profile %d-%d\n", slot_id,profile);
				cursor = showRunningCfg_str + totalLen;

		/* [4] fill vrrp running config string in showRunningCfg_str */
#ifdef DISTRIBUT
		DBusConnection *dcli_dbus_connection = NULL;
		ReInitDbusConnection(&dcli_dbus_connection,slot_id,distributFag);
#endif
		memset(vrrp_obj_path, 0, DCLI_VRRP_OBJPATH_LEN);
		memset(vrrp_dbus_name, 0, DCLI_VRRP_DBUSNAME_LEN);
		memset(tmpBuf, 0, SHOWRUN_PERLINE_SIZE);
		dcli_vrrp_splice_objpath_string(VRRP_DBUS_OBJPATH, profile, vrrp_obj_path);
		dcli_vrrp_splice_dbusname(VRRP_DBUS_BUSNAME, profile, vrrp_dbus_name);

		query = dbus_message_new_method_call(vrrp_dbus_name,
											 vrrp_obj_path,
											 VRRP_DBUS_INTERFACE,
											 VRRP_DBUS_METHOD_SHOW_RUNNING);

		dbus_error_init(&err);

		reply = dbus_connection_send_with_reply_and_block (dcli_dbus_connection,query,-1, &err);

		dbus_message_unref(query);
		if (NULL == reply) {
		printf("show vrrp running config failed get reply.\n");
			if (dbus_error_is_set(&err)) {
				printf("%s raised: %s",err.name,err.message);
				dbus_error_free_for_dcli(&err);
			}
			free(showRunningCfg_str);
			return 1;
		}

		if (dbus_message_get_args ( reply, &err,
						DBUS_TYPE_STRING, &showStr,
						DBUS_TYPE_INVALID)) 
		{
			/* [5] add showRunningCfg_str to vtysh process */
			totalLen += sprintf(cursor, "%s", showStr);
			cursor = showRunningCfg_str + totalLen;
			/* add string "exit" */
			totalLen += sprintf(cursor, " exit\n");
			cursor = showRunningCfg_str + totalLen;
//			dcli_config_write(showRunningCfg_str,localid,slot_id,profile,0,0);		
			dcli_config_write(showRunningCfg_str,localid,slot_id,profile,0,0);		
			dcli_config_writev2(showRunningCfg_str,slot_id,profile,"hansi_had");		
			vtysh_add_show_string(showRunningCfg_str);
		} 
		else
		{
			printf("Failed get args.\n");
			if (dbus_error_is_set(&err)) 
			{
				printf("%s raised: %s",err.name,err.message);
				dbus_error_free_for_dcli(&err);
			}
			free(showRunningCfg_str);
			return 1;
		}

		dbus_message_unref(reply);
	}

	free(showRunningCfg_str);
	
	/**gjd :add pfm show running func**/
			vtysh_pfm_config_write(vty);
	/* houxx: add for bridge_mcast show running */
	ret = g_bridge_mcast_config_save();
	return 0;	
}
#endif
#else
int dcli_vrrp_show_running_cfg(struct vty *vty)
{	
	char *showStr = NULL, ch = 0,tmpBuf[SHOWRUN_PERLINE_SIZE] = {0};
	DBusMessage *query, *reply;
	DBusError err;

	int profile = 0;
	int instRun = 0, ret = 0;
	char vrrp_obj_path[DCLI_VRRP_OBJPATH_LEN] = {0};
	char vrrp_dbus_name[DCLI_VRRP_DBUSNAME_LEN] = {0};
	/* flag for one time insert string "VRRP section" into show running-config  */
	int bulid_vrrp_moudle_flg = 0;
	char *showRunningCfg_str = NULL;
	char *cursor = NULL;
	int totalLen = 0;
	char *tmp = NULL;
#ifdef DISTRIBUT
	char wireless_cmd[] = "show interface wireless_config remote";	
	char ebr_cmd[] = "show interface ebr_config remote";
#else
	char wireless_cmd[] = "show interface wireless_config";	
	char ebr_cmd[] = "show interface ebr_config";
#endif
	char wireless_cmd1[128];
	showRunningCfg_str = (char *)malloc(DCLI_VRRP_SHOW_RUNNING_CFG_LEN);
	int check_result = DCLI_VRRP_INSTANCE_CREATED;
	unsigned char service_name[4][4] = {"had","wid", "asd", "wsm"};
	unsigned int service_index = 0;
	int localid = 0;
	int slot_id = HostSlotId;
	if (NULL == showRunningCfg_str) {
		//syslog_ax_vlan_err("memory malloc error\n");
		return 1;
	}
	memset(showRunningCfg_str, 0, DCLI_VRRP_SHOW_RUNNING_CFG_LEN);

	if (0 == bulid_vrrp_moudle_flg) {
		char _tmpstr[64];
		memset(_tmpstr,0,64);
		sprintf(_tmpstr, BUILDING_MOUDLE, "VRRP");
		vtysh_add_show_string(_tmpstr);
		bulid_vrrp_moudle_flg = 1;
	}

	/* loop to get configures on the had instance */
	for (profile = 0; profile <= DCLI_VRRP_INSTANCE_CNT; profile++) {
		/* [1] check if had process has started before or not */
		instRun = dcli_vrrp_hansi_is_running(vty, profile);
		if (DCLI_VRRP_INSTANCE_NO_CREATED == instRun) {
			//vty_out(vty, "had instance %d not created!\n", profile);
			continue;
		}
		else if (DCLI_VRRP_INSTANCE_CHECK_FAILED == instRun) {
			//vty_out(vty, "check had instance %d whether created was failed!\n",
			//			profile);
			continue;
		}

		/* profile = 0, it's not necessary create wcpss process */
		if (0 != profile) {
			/* check wheather wcpss instance started completely.
			 * wsm not support multi-process.
			 */
			for (service_index = 1; service_index < 3; service_index++) {
				check_result = dcli_vrrp_check_service_started(vty, service_name[service_index], profile);
				if (DCLI_VRRP_INSTANCE_CREATED != check_result) {
					break;					
				}
			}
		}
		if (DCLI_VRRP_INSTANCE_CREATED != check_result) {
				continue;					
		}

		/* [2] init string showRunningCfg_str
		 *     fill vrrp start string in showRunningCfg_str
		 */
		memset(showRunningCfg_str, 0, DCLI_VRRP_SHOW_RUNNING_CFG_LEN);
		cursor = showRunningCfg_str;
		totalLen = 0;
		

		/* [3] fill wid running config string in showRunningCfg_str */
#ifdef _D_WCPSS_
		totalLen += sprintf(cursor, "config hansi-profile %d\n", profile);
				cursor = showRunningCfg_str + totalLen;

		if (0 != profile) {
#ifdef DISTRIBUT
			tmp = dcli_hansi_ebr_show_running_config_start(localid, slot_id,profile);
#else
			tmp = dcli_hansi_ebr_show_running_config_start(profile);
#endif
			if(tmp != NULL){
				if (0 != strlen(tmp)) {
					totalLen += sprintf(cursor, "%s\n",tmp);
					cursor = showRunningCfg_str + totalLen;
				}
				free(tmp);
				tmp = NULL;
			}else{
				free(showRunningCfg_str);
				return 1;
			}
			memset(wireless_cmd1, 0, 128);
#ifdef DISTRIBUT
			sprintf(wireless_cmd1, "%s %d %d",ebr_cmd,slot_id,profile);
#else
			sprintf(wireless_cmd1, "%s %d",ebr_cmd,profile);
#endif
			tmp = vtysh_client_config_wireless_interface(&vtysh_client[0], wireless_cmd1); 	
			if(tmp != NULL){
				if (0 != strlen(tmp)) {
					totalLen += sprintf(cursor, "%s\n",tmp);
					cursor = showRunningCfg_str + totalLen;
				}
				XFREE(MTYPE_TMP,tmp);
				tmp = NULL;
			}		
			vtysh_add_show_string(showRunningCfg_str);		
			memset(showRunningCfg_str, 0, DCLI_VRRP_SHOW_RUNNING_CFG_LEN);
			cursor = showRunningCfg_str;
			totalLen = 0;
#ifdef DISTRIBUT
			tmp = dcli_hansi_security_show_running_config(localid, slot_id,profile);
#else
			tmp = dcli_hansi_security_show_running_config(profile);
#endif			
			if(tmp != NULL){
				if (0 != strlen(tmp)) {
					totalLen += sprintf(cursor, "%s\n",tmp);
					cursor = showRunningCfg_str + totalLen;
				}
				free(tmp);
				tmp = NULL;
			}else{
				free(showRunningCfg_str);
				return 1;
			}
#ifdef DISTRIBUT
			tmp = dcli_hansi_ac_ip_list_show_running_config(localid, slot_id,profile);
#else
			tmp = dcli_hansi_ac_ip_list_show_running_config(profile);
#endif			
			if(tmp != NULL){
				if (0 != strlen(tmp)) {
					totalLen += sprintf(cursor, "%s\n",tmp);
					cursor = showRunningCfg_str + totalLen;
				}
				free(tmp);
				tmp = NULL;
			}else{
				free(showRunningCfg_str);
				return 1;
			}
#ifdef DISTRIBUT
			tmp = dcli_hansi_wlan_show_running_config_start(localid, slot_id,profile);
#else
			tmp = dcli_hansi_wlan_show_running_config_start(profile);
#endif			
			if(tmp != NULL){
				if (0 != strlen(tmp)) {
					totalLen += sprintf(cursor, "%s\n",tmp);
					cursor = showRunningCfg_str + totalLen;
				}
				free(tmp);
				tmp = NULL;
			}else{
				free(showRunningCfg_str);
				return 1;
			}
			
/*yjl copy from aw3.1.2 for local forwarding.2014-2-28*/
#ifdef DISTRIBUT
			tmp = dcli_hansi_vir_dhcp_show_running_config(localid, slot_id,profile);
#else
			tmp = dcli_hansi_vir_dhcp_show_running_config(profile);
#endif			
			if(tmp != NULL){
				if (0 != strlen(tmp)) {
					totalLen += sprintf(cursor, "%s\n",tmp);
					cursor = showRunningCfg_str + totalLen;
				}
				free(tmp);
				tmp = NULL;
			}else{
				free(showRunningCfg_str);
				return 1;
			}
/*end**************************************************/
#ifdef DISTRIBUT
			tmp = dcli_hansi_wtp_show_running_config_start(localid, slot_id,profile);
#else
			tmp = dcli_hansi_wtp_show_running_config_start(profile);
#endif			
			if(tmp != NULL){
				if (0 != strlen(tmp)) {
					totalLen += sprintf(cursor, "%s\n",tmp);
					cursor = showRunningCfg_str + totalLen;
				}
				free(tmp);
				tmp = NULL;
			}else{
				free(showRunningCfg_str);
				return 1;
			}
#ifdef DISTRIBUT
			tmp = dcli_hansi_wlan_list_show_running_config(localid, slot_id,profile);
#else
			tmp = dcli_hansi_wlan_list_show_running_config(profile);
#endif			
			if(tmp != NULL){
				if (0 != strlen(tmp)) {
					totalLen += sprintf(cursor, "%s\n",tmp);
					cursor = showRunningCfg_str + totalLen;
				}
				free(tmp);
				tmp = NULL;
			}else{
				free(showRunningCfg_str);
				return 1;
			}
#ifdef DISTRIBUT
			tmp = dcli_hansi_wtp_list_show_running_config(localid, slot_id,profile);
#else
			tmp = dcli_hansi_wtp_list_show_running_config(profile);
#endif			
			if(tmp != NULL){
				if (0 != strlen(tmp)) {
					totalLen += sprintf(cursor, "%s\n",tmp);
					cursor = showRunningCfg_str + totalLen;
				}
				free(tmp);
				tmp = NULL;
			}else{
				free(showRunningCfg_str);
				return 1;
			}
#ifdef DISTRIBUT
			tmp = dcli_hansi_bss_list_show_running_config(localid, slot_id,profile);
#else
			tmp = dcli_hansi_bss_list_show_running_config(profile);
#endif			
			if(tmp != NULL){
				if (0 != strlen(tmp)) {
					totalLen += sprintf(cursor, "%s\n",tmp);
					cursor = showRunningCfg_str + totalLen;
				}
				free(tmp);
				tmp = NULL;
			}else{
				free(showRunningCfg_str);
				return 1;
			}
			memset(wireless_cmd1, 0, 128);
#ifdef DISTRIBUT
			sprintf(wireless_cmd1, "%s %d %d",wireless_cmd,slot_id,profile);
#else			
			sprintf(wireless_cmd1, "%s %d",wireless_cmd,profile);
#endif
			tmp = vtysh_client_config_wireless_interface(&vtysh_client[0], wireless_cmd1); 	
			if(tmp != NULL){
				if (0 != strlen(tmp)) {
					totalLen += sprintf(cursor, "%s\n",tmp);					
					dcli_config_write(showStr,0,slot_id,profile,0,0);
					cursor = showRunningCfg_str + totalLen;
				}
				XFREE(MTYPE_TMP,tmp);
				tmp = NULL;
			}		
			vtysh_add_show_string(showRunningCfg_str);		
			memset(showRunningCfg_str, 0, DCLI_VRRP_SHOW_RUNNING_CFG_LEN);
			cursor = showRunningCfg_str;
			totalLen = 0;
#ifdef DISTRIBUT
			tmp = dcli_hansi_wlan_show_running_config_end(localid, slot_id,profile);
#else
			tmp = dcli_hansi_wlan_show_running_config_end(profile);
#endif			
			if(tmp != NULL){
				if (0 != strlen(tmp)) {
					totalLen += sprintf(cursor, "%s\n",tmp);
					cursor = showRunningCfg_str + totalLen;
				}
				free(tmp);
				tmp = NULL;
			}else{
				free(showRunningCfg_str);
				return 1;
			}
#ifdef DISTRIBUT
			tmp = dcli_hansi_wtp_show_running_config_end(localid, slot_id,profile);
#else
			tmp = dcli_hansi_wtp_show_running_config_end(profile);
#endif			
			if(tmp != NULL){
				if (0 != strlen(tmp)) {
					totalLen += sprintf(cursor, "%s\n",tmp);
					cursor = showRunningCfg_str + totalLen;
				}
				free(tmp);
				tmp = NULL;
			}else{
				free(showRunningCfg_str);
				return 1;
			}
#ifdef DISTRIBUT
			tmp = dcli_hansi_ebr_show_running_config_end(localid, slot_id,profile);
#else
			tmp = dcli_hansi_ebr_show_running_config_end(profile);
#endif			
			if(tmp != NULL){
				if (0 != strlen(tmp)) {
					totalLen += sprintf(cursor, "%s\n",tmp);
					cursor = showRunningCfg_str + totalLen;
				}
				free(tmp);
				tmp = NULL;
			}else{
				free(showRunningCfg_str);
				return 1;
			}
		}
		
		totalLen += sprintf(cursor, " exit\n", profile);
				cursor = showRunningCfg_str + totalLen;
				
		vtysh_add_show_string(showRunningCfg_str);
#endif		
		}
	
		/* dhcp config section */
		ret = dcli_dhcp_show_running_cfg(NULL);
		if (ret) {
			return 1;
		}
		
	/* loop to get configures on the had instance */
	for (profile = 0; profile <= DCLI_VRRP_INSTANCE_CNT; profile++) {
		/* [1] check if had process has started before or not */
		instRun = dcli_vrrp_hansi_is_running(vty, profile);
		if (DCLI_VRRP_INSTANCE_NO_CREATED == instRun) {
			//vty_out(vty, "had instance %d not created!\n", profile);
			continue;
		}
		else if (DCLI_VRRP_INSTANCE_CHECK_FAILED == instRun) {
			//vty_out(vty, "check had instance %d whether created was failed!\n",
			//			profile);
			continue;
		}

		/* [2] init string showRunningCfg_str
		 *     fill vrrp start string in showRunningCfg_str
		 */
		memset(showRunningCfg_str, 0, DCLI_VRRP_SHOW_RUNNING_CFG_LEN);
		cursor = showRunningCfg_str;
		totalLen = 0;
		/*hansi config real start here */
		totalLen += sprintf(cursor, "config hansi-profile %d\n", profile);
				cursor = showRunningCfg_str + totalLen;

		/* [4] fill vrrp running config string in showRunningCfg_str */
		memset(vrrp_obj_path, 0, DCLI_VRRP_OBJPATH_LEN);
		memset(vrrp_dbus_name, 0, DCLI_VRRP_DBUSNAME_LEN);
		memset(tmpBuf, 0, SHOWRUN_PERLINE_SIZE);
		dcli_vrrp_splice_objpath_string(VRRP_DBUS_OBJPATH, profile, vrrp_obj_path);
		dcli_vrrp_splice_dbusname(VRRP_DBUS_BUSNAME, profile, vrrp_dbus_name);

		query = dbus_message_new_method_call(vrrp_dbus_name,
											 vrrp_obj_path,
											 VRRP_DBUS_INTERFACE,
											 VRRP_DBUS_METHOD_SHOW_RUNNING);

		dbus_error_init(&err);

		reply = dbus_connection_send_with_reply_and_block (dcli_dbus_connection,query,-1, &err);

		dbus_message_unref(query);
		if (NULL == reply) {
		printf("show vrrp running config failed get reply.\n");
			if (dbus_error_is_set(&err)) {
				printf("%s raised: %s",err.name,err.message);
				dbus_error_free_for_dcli(&err);
			}
			free(showRunningCfg_str);
			return 1;
		}

		if (dbus_message_get_args ( reply, &err,
						DBUS_TYPE_STRING, &showStr,
						DBUS_TYPE_INVALID)) 
		{
			/* [5] add showRunningCfg_str to vtysh process */
			totalLen += sprintf(cursor, "%s", showStr);
			cursor = showRunningCfg_str + totalLen;
			/* add string "exit" */
			totalLen += sprintf(cursor, " exit\n");
			cursor = showRunningCfg_str + totalLen;
			vtysh_add_show_string(showRunningCfg_str);
		} 
		else
		{
			printf("Failed get args.\n");
			if (dbus_error_is_set(&err)) 
			{
				printf("%s raised: %s",err.name,err.message);
				dbus_error_free_for_dcli(&err);
			}
			free(showRunningCfg_str);
			return 1;
		}

		dbus_message_unref(reply);
	}

	free(showRunningCfg_str);
	
	/**gjd :add pfm show running func**/
			vtysh_pfm_config_write(vty);
	return 0;	
}

#endif
#ifdef DISTRIBUT 
int dcli_vrrp_show_hansi_running_cfg
(
	struct vty *vty,
	unsigned int profile,
	unsigned int slot_id
)
{	
	DBusMessage *query = NULL;
	DBusMessage *reply = NULL;
	DBusError err;

	char *showStr = NULL;
	char tmpBuf[SHOWRUN_PERLINE_SIZE] = {0};

	int instRun = 0;
	char vrrp_obj_path[DCLI_VRRP_OBJPATH_LEN] = {0};
	char vrrp_dbus_name[DCLI_VRRP_DBUSNAME_LEN] = {0};

	char *showRunningCfg_str = NULL;
	char *cursor = NULL;
	int totalLen = 0;

	showRunningCfg_str = (char *)malloc(DCLI_VRRP_SHOW_RUNNING_CFG_LEN);
	if (NULL == showRunningCfg_str) {
		return 1;
	}
	memset(showRunningCfg_str, 0, DCLI_VRRP_SHOW_RUNNING_CFG_LEN);
#if 1
	/* [1] check if had process has started before or not */
	instRun = dcli_hmd_hansi_is_running(vty,slot_id,0,profile);
	if (INSTANCE_NO_CREATED == instRun) {
		vty_out(vty, "%% Hansi instance %d not created!\n", profile);
		free(showRunningCfg_str);
		return 1;
	}
#endif
	/* [2] fill vrrp start string in showRunningCfg_str
	 */
	cursor = showRunningCfg_str;
	totalLen = 0;
	totalLen += sprintf(cursor, "==================================================\n");
	cursor = showRunningCfg_str + totalLen;
	/* [4] fill vrrp running config string in showRunningCfg_str */
	memset(vrrp_obj_path, 0, DCLI_VRRP_OBJPATH_LEN);
	memset(vrrp_dbus_name, 0, DCLI_VRRP_DBUSNAME_LEN);
	memset(tmpBuf, 0, SHOWRUN_PERLINE_SIZE);

	/* book modify, 2011-5-20 */
#ifdef DISTRIBUT
	DBusConnection *dcli_dbus_connection = NULL;
	ReInitDbusConnection(&dcli_dbus_connection,slot_id,distributFag);
#endif
	dcli_vrrp_splice_objpath_string(VRRP_DBUS_OBJPATH, profile, vrrp_obj_path);
	dcli_vrrp_splice_dbusname(VRRP_DBUS_BUSNAME, profile, vrrp_dbus_name);

	query = dbus_message_new_method_call(vrrp_dbus_name,
										 vrrp_obj_path,
										 VRRP_DBUS_INTERFACE,
										 VRRP_DBUS_METHOD_SHOW_RUNNING);
	dbus_error_init(&err);

	reply = dbus_connection_send_with_reply_and_block (dcli_dbus_connection, query, -1, &err);

	dbus_message_unref(query);
	if (NULL == reply) {
#if 0
		printf("failed get reply.\n");
		if (dbus_error_is_set(&err)) {
			printf("%s raised: %s",err.name,err.message);
			dbus_error_free_for_dcli(&err);
		}
#endif
		free(showRunningCfg_str);
		return 1;
	}

	if (dbus_message_get_args ( reply, &err,
					DBUS_TYPE_STRING, &showStr,
					DBUS_TYPE_INVALID)) 
	{
		
		totalLen += sprintf(cursor, "config hansi-profile %d-%d\n", slot_id,profile);
		cursor = showRunningCfg_str + totalLen;
		
		totalLen += sprintf(cursor, "%s", showStr);
		cursor = showRunningCfg_str + totalLen;
		/* add string "exit" */
		totalLen += sprintf(cursor, "==================================================\n");
		cursor = showRunningCfg_str + totalLen;
		vty_out(vty, "%s", showRunningCfg_str);
	}
	else {
		printf("Failed get args.\n");
		if (dbus_error_is_set(&err)) 
		{
			printf("%s raised: %s", err.name, err.message);
			dbus_error_free_for_dcli(&err);
		}
		free(showRunningCfg_str);
		return 1;
	}

	dbus_message_unref(reply);
	free(showRunningCfg_str);
	return 0;	
}
#else
int dcli_vrrp_show_hansi_running_cfg
(
	struct vty *vty,
	unsigned int profile
)
{	
	DBusMessage *query = NULL;
	DBusMessage *reply = NULL;
	DBusError err;

	char *showStr = NULL;
	char tmpBuf[SHOWRUN_PERLINE_SIZE] = {0};

	int instRun = 0;
	char vrrp_obj_path[DCLI_VRRP_OBJPATH_LEN] = {0};
	char vrrp_dbus_name[DCLI_VRRP_DBUSNAME_LEN] = {0};

	char *showRunningCfg_str = NULL;
	char *cursor = NULL;
	int totalLen = 0;

	showRunningCfg_str = (char *)malloc(DCLI_VRRP_SHOW_RUNNING_CFG_LEN);
	if (NULL == showRunningCfg_str) {
		return 1;
	}
	memset(showRunningCfg_str, 0, DCLI_VRRP_SHOW_RUNNING_CFG_LEN);

	/* [1] check if had process has started before or not */
	instRun = dcli_vrrp_hansi_is_running(vty, profile);
	if (DCLI_VRRP_INSTANCE_NO_CREATED == instRun) {
		vty_out(vty, "%% Hansi instance %d not created!\n", profile);
		free(showRunningCfg_str);
		return 1;
	}
	else if (DCLI_VRRP_INSTANCE_CHECK_FAILED == instRun) {
		vty_out(vty, "%% Check had instance %d whether created was failed!\n",
					profile);
		free(showRunningCfg_str);
		return 1;
	}

	/* [2] fill vrrp start string in showRunningCfg_str
	 */
	cursor = showRunningCfg_str;
	totalLen = 0;
	totalLen += sprintf(cursor, "==================================================\n");
	cursor = showRunningCfg_str + totalLen;

	totalLen += sprintf(cursor, "config hansi-profile %d\n", profile);
	cursor = showRunningCfg_str + totalLen;

	/* [4] fill vrrp running config string in showRunningCfg_str */
	memset(vrrp_obj_path, 0, DCLI_VRRP_OBJPATH_LEN);
	memset(vrrp_dbus_name, 0, DCLI_VRRP_DBUSNAME_LEN);
	memset(tmpBuf, 0, SHOWRUN_PERLINE_SIZE);

	/* book modify, 2011-5-20 */
#ifdef DISTRIBUT
	DBusConnection *dcli_dbus_connection = NULL;
	ReInitDbusConnection(&dcli_dbus_connection,slot_id,distributFag);
#endif
	dcli_vrrp_splice_objpath_string(VRRP_DBUS_OBJPATH, profile, vrrp_obj_path);
	dcli_vrrp_splice_dbusname(VRRP_DBUS_BUSNAME, profile, vrrp_dbus_name);

	query = dbus_message_new_method_call(vrrp_dbus_name,
										 vrrp_obj_path,
										 VRRP_DBUS_INTERFACE,
										 VRRP_DBUS_METHOD_SHOW_RUNNING);
	dbus_error_init(&err);

	reply = dbus_connection_send_with_reply_and_block (dcli_dbus_connection, query, -1, &err);

	dbus_message_unref(query);
	if (NULL == reply) {
		printf("failed get reply.\n");
		if (dbus_error_is_set(&err)) {
			printf("%s raised: %s",err.name,err.message);
			dbus_error_free_for_dcli(&err);
		}
		free(showRunningCfg_str);
		return 1;
	}

	if (dbus_message_get_args ( reply, &err,
					DBUS_TYPE_STRING, &showStr,
					DBUS_TYPE_INVALID)) 
	{
		totalLen += sprintf(cursor, "%s", showStr);
		cursor = showRunningCfg_str + totalLen;
		/* add string "exit" */
		totalLen += sprintf(cursor, "==================================================\n");
		cursor = showRunningCfg_str + totalLen;
		vty_out(vty, "%s", showRunningCfg_str);
	}
	else {
		printf("Failed get args.\n");
		if (dbus_error_is_set(&err)) 
		{
			printf("%s raised: %s", err.name, err.message);
			dbus_error_free_for_dcli(&err);
		}
		free(showRunningCfg_str);
		return 1;
	}

	dbus_message_unref(reply);
	free(showRunningCfg_str);
	return 0;	
}
#endif
DEFUN(vrrp_l2_uplink_ifname_add_delete_func,
	  vrrp_l2_uplink_ifname_add_delete_cmd,
	  "(add|delete) l2-uplink IFNAME",
	  "Add l2-uplink operation\n"
	  "Delete l2-uplink operation\n"
	  "Add/Delete l2-uplink interface\n"
	  "Add/Delete this interface as l2-uplink\n"
)
{
	DBusMessage *query = NULL;
	DBusMessage *reply = NULL;
	DBusError err;
	unsigned int profile = 0, op_ret = 0;
	char vrrp_obj_path[DCLI_VRRP_OBJPATH_LEN] = {0};
	char vrrp_dbus_name[DCLI_VRRP_DBUSNAME_LEN] = {0};
	unsigned char * ifname = NULL;
	unsigned int isAdd = 0;
	unsigned int slot_id = HostSlotId;
	
	if (HANSI_NODE == vty->node) {
		profile = (unsigned int)(vty->index);
		slot_id = vty->slotindex;
	}
	else {
		return CMD_WARNING;
	}

	if(argc != 2){return CMD_WARNING;}
	if(!strncmp(argv[0], "add", strlen(argv[0]))){
		isAdd = 1;
	}
	else if(!strncmp(argv[0], "delete", strlen(argv[0]))){
		isAdd = 0;
	}
	else{
		vty_out(vty, "%% Bad parameter input: %s !\n",argv[0]);
		return CMD_WARNING;
	}
	ifname = (unsigned char *)argv[1];
	memset(vrrp_obj_path, 0, DCLI_VRRP_OBJPATH_LEN);
	memset(vrrp_dbus_name, 0, DCLI_VRRP_DBUSNAME_LEN);
#ifdef DISTRIBUT
		DBusConnection *dcli_dbus_connection = NULL;
		ReInitDbusConnection(&dcli_dbus_connection,slot_id,distributFag);
#endif
	dcli_vrrp_splice_objpath_string(VRRP_DBUS_OBJPATH, profile, vrrp_obj_path);
	dcli_vrrp_splice_dbusname(VRRP_DBUS_BUSNAME, profile, vrrp_dbus_name);
	query = dbus_message_new_method_call(vrrp_dbus_name,
										 vrrp_obj_path,
										 VRRP_DBUS_INTERFACE,
										 VRRP_DBUS_METHOD_L2_UPLINK_ADD_DELETE);
	dbus_error_init(&err);
	dbus_message_append_args(query,
							 DBUS_TYPE_UINT32,&profile, 
							 DBUS_TYPE_STRING,&ifname, 
							 DBUS_TYPE_UINT32,&isAdd, 
							 DBUS_TYPE_INVALID);
	reply = dbus_connection_send_with_reply_and_block (dcli_dbus_connection, query, -1, &err);
	dbus_message_unref(query);
	if (NULL == reply) {
		printf("Hansi%d %s l2-uplink failed get reply.\n", profile, isAdd ? "add":"delete");
		if (dbus_error_is_set(&err)) {
			printf("%s raised: %s",err.name,err.message);
			dbus_error_free_for_dcli(&err);
		}
		return CMD_WARNING;
	}
	else if (dbus_message_get_args ( reply, &err,
				DBUS_TYPE_UINT32,&op_ret,
				DBUS_TYPE_INVALID)){	
		if(op_ret) {
			if(DCLI_VRRP_RETURN_CODE_SERVICE_NOT_PREPARE == op_ret){
				vty_out(vty, "%% Service disable first!\n");
			}
			else if(DCLI_VRRP_RETURN_CODE_IF_NOT_EXIST == op_ret){
				vty_out(vty, "%% Interface %s not exists!\n",ifname);
			}
			else if(DCLI_VRRP_RETURN_CODE_PROFILE_NOTEXIST == op_ret){
				vty_out(vty, "%% Hansi %d is not configured\n",profile);
			}
			else if(DCLI_VRRP_RETURN_CODE_UP_LIMIT == op_ret){
				vty_out(vty, "%% L2-uplink configure full\n");
			}
			else if(DCLI_VRRP_RETURN_CODE_IF_EXIST == op_ret){
				vty_out(vty, "%% %s l2-uplink  failed,Interface %s already exised!\n", isAdd ? "Add":"Delete",ifname);
			}
			else{
				vty_out(vty, "%% %s l2-uplink  failed,ret %#x!\n", isAdd ? "Add":"Delete", op_ret);
			}
			return CMD_WARNING;
		}
	} 
	else 
	{		
		if (dbus_error_is_set(&err)) 
		{
			printf("%s raised: %s",err.name,err.message);
			dbus_error_free_for_dcli(&err);
		}
	}
	dbus_message_unref(reply);
	return CMD_SUCCESS;
}
#if 1
extern struct cmd_element vtysh_ping_cmd;
extern struct cmd_element vtysh_ping_count_cmd;
extern struct cmd_element vtysh_ping_time_cmd;
extern struct cmd_element vtysh_ping_ip_cmd;
extern struct cmd_element vtysh_telnet_cmd;
extern struct cmd_element vtysh_telnet_port_cmd;
#endif

DEFUN(set_global_bridge_mcast_cmd_func,
	  set_global_bridge_mcast_cmd,
	  "set global_bridge_mcast IFNAME (enable|disable)",
	  CONFIG_STR
	  "Global Bridge mcast\n"
	  "Ebr/Wlan ifname\n"
	  "ebr mcast_solicit enable|disable\n"
)
{
	//vty_out(vty,"1\n");
	DBusMessage *query = NULL, *reply = NULL;
	DBusError err = {0};
	unsigned int hmd_ret = 0;
	unsigned int profile = 0;
	unsigned int slot_no = 0;
	unsigned int slot_no1 = 0;
	unsigned int slot_id = HostSlotId;
	
	DBusMessage *query2 = NULL, *reply2 = NULL;
	DBusError err2 = {0};
	unsigned int op_ret = 0;
	int instRun = 0;
	char cmd[DCLI_VRRP_DBUSNAME_LEN] = {0}; /*wcl add*/
	unsigned char insID = 0;
	int flag = 0;
	int local_id = 0;
	int ret = 0;
	char command[64] = {0};
#if 1	
	unsigned char state=0;
	unsigned char * ifname = NULL;

	int local_slot_id = get_product_info("/dbm/local_board/slot_id");
	int slot_count = get_product_info("/dbm/product/slotcount");
#endif
	if (!strcmp(argv[1],"enable")){
		state=1;
	}
	else if (!strcmp(argv[1],"disable")){
		state=0;
	}
	else
	{
		vty_out(vty,"<error> input parameter should only be 'enable' or 'disable'\n");
		return CMD_SUCCESS;
	}
	ifname = (unsigned char *)argv[0];
	for(slot_id=1;slot_id<=slot_count;slot_id++)
	{

		query = dbus_message_new_method_call(HMD_DBUS_BUSNAME,HMD_DBUS_OBJPATH,HMD_DBUS_INTERFACE,HMD_DBUS_SET_GLOBAL_BRIDGE_MCAST);


		dbus_error_init(&err);
		dbus_message_append_args(query,
								 DBUS_TYPE_STRING,&ifname,
								 DBUS_TYPE_BYTE,&state,
								 DBUS_TYPE_INVALID);
		
		if(NULL == dbus_connection_dcli[slot_id]->dcli_dbus_connection) 				
		{
			if(slot_id == local_slot_id)
			{
				reply = dbus_connection_send_with_reply_and_block (dcli_dbus_connection,query,-1, &err);
			}
			else 
			{	
				vty_out(vty,"Can not connect to slot:%d \n",slot_id);	
				continue;
			}
		}
		else
		{
			reply = dbus_connection_send_with_reply_and_block (dbus_connection_dcli[slot_id]->dcli_dbus_connection,query,-1, &err); 			
		}

		dbus_message_unref(query);
		if (NULL == reply) {
			vty_out(vty,"hmd failed get reply.\n");
			if (dbus_error_is_set(&err)) {
				printf("%s raised: %s",err.name,err.message);
				dbus_error_free_for_dcli(&err);
			}
			return CMD_WARNING;
		}

		if (dbus_message_get_args ( reply, &err,
					DBUS_TYPE_UINT32,&hmd_ret,
					DBUS_TYPE_INVALID))
		{	
		
			if(hmd_ret != 0){
				vty_out(vty,"hmd_ret is %d\n",hmd_ret);
			}
		} 
		else 
		{	
			if (dbus_error_is_set(&err)) 
			{
				printf("%s raised: %s",err.name,err.message);
				dbus_error_free_for_dcli(&err);
			}		
			return CMD_WARNING;
		}
		
		dbus_message_unref(reply);
	}
	
	sprintf(command,"mcast_config_save.sh %s %d",ifname,state);
	ret = system(command);
	ret = WEXITSTATUS(ret);
	if(ret != 0)
	{
		vty_out(vty,"save config file! \n");
		return CMD_WARNING;
	}

	return CMD_SUCCESS;
}

void dcli_vrrp_element_init(void)  
{
	install_node (&hansi_node, dcli_vrrp_show_running_cfg, "HANSI_NODE");
//	install_node (&hansi_node, NULL, "HANSI_NODE");
	install_default(HANSI_NODE);

    install_element(CONFIG_NODE,&config_hansi_cmd);
	install_element(CONFIG_NODE,&config_vrrp_golbal_swtich_add_del_hansi_cmd);
	install_element(CONFIG_NODE, &config_delete_hansi_cmd);
	install_element(CONFIG_NODE, &show_vrrp_runconfig_cmd);
	/*install_element(HANSI_NODE,&config_vrrp_cmd);*/
	install_element(HANSI_NODE,&config_vrrp_with_mask_cmd);
	install_element(HANSI_NODE,&send_arp_cmd);
	install_element(HANSI_NODE,&config_vrrp_heartbeat_cmd);
	install_element(HANSI_NODE,&set_hansi_vrid_cmd);
	install_element(HANSI_NODE,&vrrp_trap_sw_cmd);
	install_element(HANSI_NODE,&config_vrrp_start_state_cmd);
	install_element(HANSI_NODE,&config_vrrp_service_cmd);
	install_element(HANSI_NODE,&config_vrrp_gateway_cmd);
	install_element(HANSI_NODE,&cancel_vrrp_gateway_cmd);
	install_element(HANSI_NODE,&cancel_vrrp_transfer_cmd);
	install_element(HANSI_NODE,&config_vrrp_real_ip_cmd);
	install_element(HANSI_NODE,&config_vrrp_real_ip_downlink_cmd);
	install_element(HANSI_NODE,&config_vrrp_real_ip_uplink_cmd);
	install_element(HANSI_NODE,&config_vrrp_no_real_ip_downlink_cmd);
	install_element(HANSI_NODE,&config_vrrp_no_real_ip_uplink_cmd);
	install_element(HANSI_NODE,&config_vrrp_downlink_cmd);
	install_element(HANSI_NODE,&config_vrrp_uplink_cmd);
	install_element(HANSI_NODE,&config_vrrp_link_add_vip_cmd);
	install_element(HANSI_NODE,&config_vrrp_link_del_vip_cmd);
	install_element(HANSI_NODE,&config_vrrp_end_cmd);
	install_element(HANSI_NODE,&config_vrrp_priority_cmd);
	install_element(HANSI_NODE,&config_vrrp_advert_cmd);
	install_element(HANSI_NODE,&config_vrrp_mac_cmd);
	install_element(HANSI_NODE,&config_vrrp_global_vmac_cmd);
	install_element(HANSI_NODE,&config_vrrp_preempt_cmd);
	install_element(HANSI_NODE,&config_vrrp_max_down_count_cmd);
	install_element(HANSI_NODE,&config_vrrp_multi_link_detect_cmd);
	install_element(HANSI_NODE,&show_vrrp_cmd);	
	install_element(CONFIG_NODE,&show_vrrp_cmd);
	install_element(ENABLE_NODE,&show_vrrp_cmd);
	install_element(VIEW_NODE,&show_vrrp_cmd);	
	install_element(HANSI_NODE,&config_vrrp_notify_cmd);
	install_element(HANSI_NODE,&config_vrrp_set_vgateway_transform_cmd);
	install_element(HANSI_NODE, &show_vrrp_runconfig_cmd);
	install_element(HANSI_NODE,&debug_vrrp_cmd);
	install_element(HANSI_NODE,&no_debug_vrrp_cmd);
	install_element(HANSI_NODE,&vrrp_conf_failover_ip_cmd);
	install_element(HANSI_NODE,&vrrp_conf_failover_ip_peer_cmd);
	install_element(HANSI_NODE,&vrrp_no_failover_ip_peer_cmd);
	#if 0
	install_element(HANSI_NODE,&vrrp_config_gratuitous_arp_discrete_value_cmd);
	install_element(HANSI_NODE,&vrrp_config_gratuitous_arp_continous_value_cmd);
	#endif
#if 1
		  install_element (HANSI_NODE, &vtysh_ping_cmd);
		  install_element(HANSI_NODE,&vtysh_ping_count_cmd);
		  install_element(HANSI_NODE,&vtysh_ping_time_cmd);
		  install_element (HANSI_NODE, &vtysh_ping_ip_cmd);
		  install_element (HANSI_NODE, &vtysh_telnet_cmd);
		  install_element (HANSI_NODE, &vtysh_telnet_port_cmd);
#endif
	install_element(HANSI_NODE,&vrrp_l2_uplink_ifname_add_delete_cmd);	
	install_element(HANSI_NODE,&config_vrrp_set_vip_back_down_cmd);	
	install_element(HANSI_NODE,&show_hansi_backup_switch_times_cmd);
	install_element(CONFIG_NODE,&show_vrrp_golbal_swtich_hansi_list_cmd);
	install_element(CONFIG_NODE,&set_global_bridge_mcast_cmd);
}
#ifdef __cplusplus
}
#endif
