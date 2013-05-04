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
* dcli_pim.c
*
* MODIFY:
*
* CREATOR:
*		shancx@autelan.com
*
* DESCRIPTION:
*		CLI definition for pim module.
*
* DATE:
*		11/22/2009	
*
*  FILE REVISION NUMBER:
*  		$Revision: 1.14 $	
*******************************************************************************/


#include <stdio.h>
#include <string.h> 
#include <ctype.h>
#include <zebra.h>
#include <sys/types.h>
#include <dbus/dbus.h>

#include "command.h"
#include "dcli_main.h"
#include "dcli_acl.h"

#include "../../../accapi/dbus/pimd/mrt_dbus_def.h"
#include "dcli_pim.h"

DEFUN ( mrt_multicast_routing_func,
	mrt_multicast_routing_cmd,
	"ip multicast-routing",
	IP_STR
	MULTICAST_STR )
{
	DBusMessage *query = NULL, *reply = NULL;
	DBusError err = {0};
	int op_ret = -1;
	unsigned int enable = 1;
	int detect = 0;
	
	query = dbus_message_new_method_call(PIMD_DBUS_BUSNAME, 
									PIMD_DBUS_OBJPATH, 
									PIMD_DBUS_INTERFACE, 
									PIMD_DBUS_INTERFACE_METHOD_IPMRT_ENABLE);
	dbus_error_init(&err);
	dbus_message_append_args(query,
							 DBUS_TYPE_UINT32, &enable, 
							 DBUS_TYPE_INVALID);
	reply = dbus_connection_send_with_reply_and_block(dcli_dbus_connection,query,-1, &err);
	dbus_message_unref(query);
	
	if (NULL == reply) {
		
		vty_out(vty,"failed get reply.\n");
		if (dbus_error_is_set(&err)) {
			vty_out(vty,"%s raised: %s",err.name,err.message);
			dbus_error_free_for_dcli(&err);
		}
		
		return CMD_WARNING;
	}
	
	if (dbus_message_get_args(reply, &err,
		DBUS_TYPE_UINT32, &enable,
		DBUS_TYPE_INT32, &op_ret,
		DBUS_TYPE_INVALID)) 
		{
			dbus_message_unref(reply);
			if(op_ret == MRT_CONFIG_REPEAT )
			{
				vty_out(vty,"The system has enabled ip mroute already!\n");
				return CMD_WARNING;
			}
			else if(op_ret<0)
			{
				vty_out(vty,"Enable ip multicast-routing failure\n");
				return CMD_WARNING;

			}
				
		} 
	else {		
		if (dbus_error_is_set(&err)) {
			
			vty_out(vty,"%s raised: %s",err.name,err.message);
			dbus_error_free_for_dcli(&err);
		}
		
		dbus_message_unref(reply);
		return CMD_WARNING;
	}
	
	return CMD_SUCCESS;
}

DEFUN ( no_mrt_multicast_routing_func,
	no_mrt_multicast_routing_cmd,
	"no ip multicast-routing",
	NO_STR
	IP_STR
	MULTICAST_STR )
{
	DBusMessage *query = NULL, *reply = NULL;
	DBusError err = {0};
	int op_ret = -1;
	unsigned int enable = 0;
	int detect = 0;
	
	query = dbus_message_new_method_call(PIMD_DBUS_BUSNAME, 
									PIMD_DBUS_OBJPATH, 
									PIMD_DBUS_INTERFACE, 
									PIMD_DBUS_INTERFACE_METHOD_IPMRT_ENABLE);
	dbus_error_init(&err);
	dbus_message_append_args(query,
							 DBUS_TYPE_UINT32, &enable, 
							 DBUS_TYPE_INVALID);
	reply = dbus_connection_send_with_reply_and_block(dcli_dbus_connection,query,-1, &err);
	dbus_message_unref(query);
	if (NULL == reply) {
		vty_out(vty,"failed get reply.\n");
		if (dbus_error_is_set(&err)) {
			vty_out(vty,"%s raised: %s",err.name,err.message);
			dbus_error_free_for_dcli(&err);
		}
		return CMD_WARNING;
	}
	if (dbus_message_get_args(reply, &err,
		DBUS_TYPE_UINT32, &enable,
		DBUS_TYPE_INT32, &op_ret,
		DBUS_TYPE_INVALID)) 
	{
		dbus_message_unref(reply);
		
		if(op_ret == MRT_CONFIG_REPEAT )
		{
			vty_out(vty,"The system has been disabled ip mroute already!\n");
				return CMD_WARNING;
		}
		else if(op_ret<0)
		{
			vty_out(vty,"Disable ip multicast-routing failure\n");
			return CMD_WARNING;

		}
	} 
	else {		
		if (dbus_error_is_set(&err)) {
			vty_out(vty,"%s raised: %s",err.name,err.message);
			dbus_error_free_for_dcli(&err);
		}
		
		dbus_message_unref(reply);
		return CMD_WARNING;
	}
	return CMD_SUCCESS;
}


DEFUN ( ip_mroute_static_eth_eth,
	ip_mroute_static_eth_eth_cmd,
	"ip mroute-static <A.B.C.D> <A.B.C.D> INIFNAME .OUTIFNAME",
	IP_STR
	"Config static IP multicast route\n"
	"Multicast source address\n"
	"Multicast group address\n"
	"Incoming interface name\n"
	"Outgoing interface name\n"
 )
{

}
DEFUN ( no_ip_mroute_static_eth_eth,
	no_ip_mroute_static_eth_eth_cmd,
	"no ip mroute-static <A.B.C.D> <A.B.C.D>",
	NO_STR
	IP_STR	
	"Config static IP multicast route\n"
	"Multicast source address\n"
	"Multicast group address\n"
	"Interface name\n"
 )
{

}

DEFUN ( pim_start_func,
  pim_start_cmd,
  "ip pim sparse-mode",
  IP_STR
  PIM_STR
  "PIM sparse-mode operation\n" )
{
	DBusMessage *query = NULL, *reply = NULL;
	DBusError err = {0};
	int op_ret = -1;
	unsigned int enable = 1;
	int detect = 0;
	char *ifname = vty->index;
/*
	vty_out(vty,"interface %s\n",(char*)vty->index);
*/
	query = dbus_message_new_method_call(PIMD_DBUS_BUSNAME, 
									PIMD_DBUS_OBJPATH, 
									PIMD_DBUS_INTERFACE, 
									PIMD_DBUS_INTERFACE_METHOD_IPPIM_ENABLE);
	dbus_error_init(&err);
	dbus_message_append_args(query,
							 DBUS_TYPE_UINT32, &enable,	
							 DBUS_TYPE_STRING,&ifname,
							 DBUS_TYPE_INVALID);
	reply = dbus_connection_send_with_reply_and_block(dcli_dbus_connection,query,-1, &err);
	dbus_message_unref(query);
	
	if (NULL == reply) {
		
		vty_out(vty,"failed get reply.\n");
		if (dbus_error_is_set(&err)) {
			vty_out(vty,"%s raised: %s",err.name,err.message);
			dbus_error_free_for_dcli(&err);
		}
		return CMD_WARNING;
	}
	
	if (dbus_message_get_args(reply, &err,
		DBUS_TYPE_UINT32, &enable,
		DBUS_TYPE_INT32, &op_ret,
		DBUS_TYPE_INVALID)) 
	{
		
		if(op_ret < 0 )
		{
			vty_out(vty,"Set interface %s ip pim sparse-mode enable failure!\n",ifname);
			return CMD_WARNING;
		}
		else if (op_ret == MRT_DISABLE)
		{
			vty_out(vty,"The system isn't running multicast-routing, pls run is!\n");
			return CMD_WARNING;
		
		}
		dbus_message_unref(reply);
	} 
	else {		
		if (dbus_error_is_set(&err)) {

			
			vty_out(vty,"%s raised: %s",err.name,err.message);
			dbus_error_free_for_dcli(&err);
		}
		
		dbus_message_unref(reply);
		return CMD_WARNING;
	}
	
	return CMD_SUCCESS;
}
DEFUN ( no_pim_start_func,
  no_pim_start_cmd,
  "no ip pim sparse-mode",
  NO_STR
  IP_STR
  PIM_STR
  "PIM sparse-mode operation\n" )
{
	DBusMessage *query = NULL, *reply = NULL;
	DBusError err = {0};
	int op_ret = -1;
	unsigned int enable = 0;
	int detect = 0;
	
	query = dbus_message_new_method_call(PIMD_DBUS_BUSNAME, 
									PIMD_DBUS_OBJPATH, 
									PIMD_DBUS_INTERFACE, 
									PIMD_DBUS_INTERFACE_METHOD_IPPIM_ENABLE);
	dbus_error_init(&err);
	dbus_message_append_args(query,
							DBUS_TYPE_UINT32, &enable, 
							DBUS_TYPE_STRING,&vty->index,
							DBUS_TYPE_INVALID);
	reply = dbus_connection_send_with_reply_and_block(dcli_dbus_connection,query,-1, &err);
	dbus_message_unref(query);
	if (NULL == reply) {
		vty_out(vty,"failed get reply.\n");
		if (dbus_error_is_set(&err)) {
			vty_out(vty,"%s raised: %s",err.name,err.message);
			dbus_error_free_for_dcli(&err);
		}
		return CMD_WARNING;
	}
	if (dbus_message_get_args(reply, &err,
		DBUS_TYPE_UINT32, &enable,
		DBUS_TYPE_INT32, &op_ret,
		DBUS_TYPE_INVALID)) 
	{
		dbus_message_unref(reply);
		if(op_ret < 0 )
		{
			vty_out(vty,"Set the interface %s ip pim sparse-mode disable failure!\n",(char*)vty->index);
			return CMD_WARNING;
		}
		else if (op_ret == MRT_DISABLE)
		{
			vty_out(vty,"The system isn't running multicast-routing, pls run it!\n");
			return CMD_WARNING;
		
		}
	} 
	else {
		
		vty_out(vty,"failed get reply.\n");
		if (dbus_error_is_set(&err)) {
			vty_out(vty,"%s raised: %s",err.name,err.message);
			dbus_error_free_for_dcli(&err);
		}
		dbus_message_unref(reply);
		return CMD_WARNING;
	}
	
	return CMD_SUCCESS;
}



DEFUN ( pim_bsr_candidate_eth,
          pim_bsr_candidate_eth_cmd,
          "ip pim bsr-candidate IFNAME hash_mask-length <0-32> priority <0-255>",
          IP_STR
          PIM_STR
          "PIM BSR-candidate (Bootstrap Router)\n" 
          "Interface name\n"
          "Hash mask length of BSR candidate\n"
          "Hash mask length, default is 30\n"
          "Priority of BSR candidate\n"
          "Priority value, default is 0 (lowest)\n" )
{
	DBusMessage *query = NULL, *reply = NULL;
	DBusError err = {0};
	int op_ret = -1;
	unsigned int enable = 1;
	unsigned int hmasklen=atoi(argv[1]),priority=atoi(argv[2]);

	if(hmasklen>32 )
	{
		vty_out(vty,"hash_mask-length must be 0-32");
		return CMD_WARNING;
	}
	if( priority>255)
	{
		vty_out(vty,"priority must be 0-255");
		return CMD_WARNING;
	}
	if(strlen(argv[0])>IFNAMSIZ)
	{
		vty_out(vty,"Error ifname length,it should be < %d \n",IFNAMSIZ);
		return CMD_WARNING;
	}
	query = dbus_message_new_method_call(PIMD_DBUS_BUSNAME, 
									PIMD_DBUS_OBJPATH, 
									PIMD_DBUS_INTERFACE, 
									PIMD_DBUS_INTERFACE_METHOD_BSR_CANDIDATE);
	dbus_error_init(&err);
	dbus_message_append_args(query,
							DBUS_TYPE_UINT32, &enable, 
							DBUS_TYPE_STRING,&argv[0],							
							DBUS_TYPE_UINT32,&hmasklen,
							DBUS_TYPE_UINT32,&priority,							
							DBUS_TYPE_INVALID);
	reply = dbus_connection_send_with_reply_and_block(dcli_dbus_connection,query,-1, &err);
	dbus_message_unref(query);
	if (NULL == reply) {
		vty_out(vty,"failed get reply.\n");
		if (dbus_error_is_set(&err)) {
			vty_out(vty,"%s raised: %s",err.name,err.message);
			dbus_error_free_for_dcli(&err);
		}
		return CMD_WARNING;
	}
	if (dbus_message_get_args(reply, &err,
		DBUS_TYPE_INT32, &op_ret,
		DBUS_TYPE_INVALID)) 
		{
		dbus_message_unref(reply);
		if(op_ret == MRT_DISABLE )
		{
			vty_out(vty,"The system isn't run multicast-routing !\n");
			return CMD_WARNING;
		}
		else if(op_ret == PIM_INTERFACE_DISABLE )
		{
			vty_out(vty,"The interface %s isn't runing ip pim !\n",argv[0]);
			return CMD_WARNING;
		}
		else if(op_ret!=0)
		{
			vty_out(vty,"Set the interface ip pim bsr-candidate failure!\n");
			return CMD_WARNING;

		}
	} 
	else {		
		if (dbus_error_is_set(&err)) {
			vty_out(vty,"%s raised: %s",err.name,err.message);
			dbus_error_free_for_dcli(&err);
		}
		dbus_message_unref(reply);
		return CMD_WARNING;
	}
	
	return CMD_SUCCESS;
	
}


DEFUN ( no_pim_bsr_candidate_eth,
          no_pim_bsr_candidate_eth_cmd,
          "no ip pim bsr-candidate ",
          NO_STR
		  IP_STR
          PIM_STR
          "PIM BSR-candidate (Bootstrap Router)\n"
          IFNAME_STR)
{
	DBusMessage *query = NULL, *reply = NULL;
	DBusError err = {0};
	int op_ret = -1;
	unsigned int enable = 0;
	int detect = 0;
	query = dbus_message_new_method_call(PIMD_DBUS_BUSNAME, 
									PIMD_DBUS_OBJPATH, 
									PIMD_DBUS_INTERFACE, 
									PIMD_DBUS_INTERFACE_METHOD_NO_BSR_CANDIDATE);
	dbus_error_init(&err);
	dbus_message_append_args(query,
							DBUS_TYPE_UINT32, &enable, 
							DBUS_TYPE_INVALID);
	reply = dbus_connection_send_with_reply_and_block(dcli_dbus_connection,query,-1, &err);
	dbus_message_unref(query);
	if (NULL == reply) {
		vty_out(vty,"failed get reply.\n");
		if (dbus_error_is_set(&err)) {
			vty_out(vty,"%s raised: %s",err.name,err.message);
			dbus_error_free_for_dcli(&err);
		}
		return CMD_WARNING;
	}
	if (dbus_message_get_args(reply, &err,
		DBUS_TYPE_INT32, &op_ret,
		DBUS_TYPE_INVALID)) 
	{
		dbus_message_unref(reply);
		if(op_ret == MRT_DISABLE )
		{
			vty_out(vty,"The system isn't run multicast-routing !\n");
			return CMD_WARNING;
		}
		else if(op_ret!=0)
		{
			vty_out(vty,"Del ip pim bsr-candidate failure!\n");
			return CMD_WARNING;

		}
	} 
	else {		
		if (dbus_error_is_set(&err)) {
			vty_out(vty,"%s raised: %s",err.name,err.message);
			dbus_error_free_for_dcli(&err);
			dbus_message_unref(reply);
			return CMD_WARNING;
		}
	}
	
	return CMD_SUCCESS;
}


DEFUN ( show_ip_pim_bsr_router,
          show_ip_pim_bsr_router_cmd,
          "show ip pim bsr-router",
          SHOW_STR
          IP_STR
          PIM_STR
          "Bootstrap router\n" )
{
	DBusMessage *query = NULL, *reply = NULL;
	DBusError err = {0};
	int op_ret = -1;
	unsigned int enable = 1;
	int detect = 0;
	char* bsr_str;
	query = dbus_message_new_method_call(PIMD_DBUS_BUSNAME, 
									PIMD_DBUS_OBJPATH, 
									PIMD_DBUS_INTERFACE, 
									PIMD_DBUS_INTERFACE_METHOD_SHOW_BSR);
	dbus_error_init(&err);
	dbus_message_append_args(query,
							DBUS_TYPE_UINT32, &enable, 
							DBUS_TYPE_INVALID);
	reply = dbus_connection_send_with_reply_and_block(dcli_dbus_connection,query,-1, &err);
	dbus_message_unref(query);
	if (NULL == reply) {
		vty_out(vty,"failed get reply.\n");
		if (dbus_error_is_set(&err)) {
			vty_out(vty,"%s raised: %s",err.name,err.message);
			dbus_error_free_for_dcli(&err);
		}
		return CMD_WARNING;
	}
	if (dbus_message_get_args(reply, &err,
		DBUS_TYPE_INT32, &op_ret,
		DBUS_TYPE_STRING,&bsr_str,							
		DBUS_TYPE_INVALID)) 
	{
		dbus_message_unref(reply);
		if(op_ret == MRT_DISABLE )
		{
			vty_out(vty,"The system isn't run multicast-routing !\n");
			return CMD_WARNING;
		}
		else if(op_ret==0)
			vty_out(vty,"%s\n",bsr_str);
		else{
			vty_out(vty,"Can't get bsr infomation\n");
			return CMD_WARNING;
		}
	} 
	else 
	{		
		if (dbus_error_is_set(&err)) {
			vty_out(vty,"%s raised: %s",err.name,err.message);
			dbus_error_free_for_dcli(&err);
		}
		dbus_message_unref(reply);
		
		return CMD_WARNING;
	}
	return CMD_SUCCESS;
}

DEFUN ( set_pim_rp_candidate,
  set_pim_rp_candidate_cmd,
  "ip pim rp-candidate IFNAME priority <1-255> time <10-60>",
  IP_STR
  PIM_STR
  "PIM RP-candidate\n" 
  "Pim RP_candidate interface name \n"
  "Priority of rp-candidate\n"
  "Priority value, default is 0 (highest)\n"
  "Period of Cand-RP adv.\n"
  )
{
	DBusMessage *query = NULL, *reply = NULL;
	DBusError err = {0};
	int op_ret = -1;
	unsigned int enable = 1;
	int detect = 0;
	unsigned int priority = atoi(argv[1]) ;
	unsigned int time = atoi(argv[2]);


	if(strlen(argv[0])>IFNAMSIZ)
	{
		vty_out(vty,"Error ifname length,it should be < %d \n",IFNAMSIZ);
		return CMD_WARNING;
	}

	query = dbus_message_new_method_call(PIMD_DBUS_BUSNAME, 
									PIMD_DBUS_OBJPATH, 
									PIMD_DBUS_INTERFACE, 
									PIMD_DBUS_INTERFACE_METHOD_SET_RP_CANDIDATE);
	dbus_error_init(&err);
	dbus_message_append_args(query,
							DBUS_TYPE_STRING, &argv[0],
							DBUS_TYPE_UINT32,&enable,
							DBUS_TYPE_UINT32,&priority,
							DBUS_TYPE_UINT32,&time,
							DBUS_TYPE_INVALID);
	reply = dbus_connection_send_with_reply_and_block(dcli_dbus_connection,query,-1, &err);
	dbus_message_unref(query);
	if (NULL == reply) {
		vty_out(vty,"failed get reply.\n");
		if (dbus_error_is_set(&err)) {
			vty_out(vty,"%s raised: %s",err.name,err.message);
			dbus_error_free_for_dcli(&err);
		}
		return CMD_WARNING;
	}
	if (dbus_message_get_args(reply, &err,
		DBUS_TYPE_INT32, &op_ret,
		DBUS_TYPE_INVALID)) 
	{
		dbus_message_unref(reply);
		if(op_ret == MRT_DISABLE )
		{
			vty_out(vty,"The system isn't run multicast-routing !\n");
			return CMD_WARNING;
		}
		else if(op_ret == PIM_INTERFACE_DISABLE)
		{
			vty_out(vty,"The interface %s isn't runing  pim !\n",argv[0]);
			return CMD_WARNING;

		}
		else if(op_ret<0)
		{
			vty_out(vty,"Set RP-candidate failure\n");
			return CMD_WARNING;		
		}
	} 
	else 
	{		
		if (dbus_error_is_set(&err)) {
			vty_out(vty,"%s raised: %s",err.name,err.message);
			dbus_error_free_for_dcli(&err);
		}
		dbus_message_unref(reply);
		return CMD_WARNING;
	}
	return CMD_SUCCESS;
}

DEFUN ( del_set_pim_rp_candidate,
  del_set_pim_rp_candidate_cmd,
  "no ip pim rp-candidate",
  NO_STR
  IP_STR
  PIM_STR
  "PIM RP-candidate\n" 
  "PIM RP-candidate interface name\n"
  )
{
	DBusMessage *query = NULL, *reply = NULL;
	DBusError err = {0};
	int op_ret = -1;
	unsigned int enable = 0;
	int detect = 0;
	unsigned int priority = 0 ;
	unsigned int time = 0;
	char *ifname = "ifname";
	query = dbus_message_new_method_call(PIMD_DBUS_BUSNAME, 
									PIMD_DBUS_OBJPATH, 
									PIMD_DBUS_INTERFACE, 
									PIMD_DBUS_INTERFACE_METHOD_SET_RP_CANDIDATE);
	
	dbus_error_init(&err);
	dbus_message_append_args(query,
							DBUS_TYPE_STRING, &ifname,
							DBUS_TYPE_UINT32,&enable,
							DBUS_TYPE_UINT32,&priority,
							DBUS_TYPE_UINT32,&time,
							DBUS_TYPE_INVALID);
	reply = dbus_connection_send_with_reply_and_block(dcli_dbus_connection,query,-1, &err);
	dbus_message_unref(query);
	if (NULL == reply) {
		vty_out(vty,"failed get reply.\n");
		if (dbus_error_is_set(&err)) {
			vty_out(vty,"%s raised: %s",err.name,err.message);
			dbus_error_free_for_dcli(&err);
		}
		return CMD_WARNING;
	}
	if (dbus_message_get_args(reply, &err,
		DBUS_TYPE_INT32, &op_ret,
		DBUS_TYPE_INVALID)) 
	{
	
		dbus_message_unref(reply);
		if(op_ret == MRT_DISABLE )
		{
			vty_out(vty,"The system isn't run multicast-routing !\n");
			return CMD_WARNING;
		}
		else if(op_ret != 0)
		{
			vty_out(vty,"Del RP-candidate failure\n");
			return CMD_WARNING;		
		}
	} 
	else 
	{		
		if (dbus_error_is_set(&err)) {
			vty_out(vty,"%s raised: %s",err.name,err.message);
			dbus_error_free_for_dcli(&err);
		}
		dbus_message_unref(reply);
	}
	return CMD_SUCCESS;
}

DEFUN ( show_pim_rp_candidate,
  show_pim_rp_candidate_cmd,
  "show ip pim rp-candidate",
  SHOW_STR
  IP_STR
  PIM_STR
  "PIM RP-candidate\n" )
{
	DBusMessage *query = NULL, *reply = NULL;
	DBusError err = {0};
	int op_ret = -1;
	unsigned int enable = 1;
	int detect = 0;
	char* bsr_str;
	query = dbus_message_new_method_call(PIMD_DBUS_BUSNAME, 
									PIMD_DBUS_OBJPATH, 
									PIMD_DBUS_INTERFACE, 
									PIMD_DBUS_INTERFACE_METHOD_SHOW_RP_CANDIDATE);
	dbus_error_init(&err);
	dbus_message_append_args(query,
							DBUS_TYPE_UINT32, &enable, 
							DBUS_TYPE_INVALID);
	reply = dbus_connection_send_with_reply_and_block(dcli_dbus_connection,query,-1, &err);
	dbus_message_unref(query);
	if (NULL == reply) {
		vty_out(vty,"failed get reply.\n");
		if (dbus_error_is_set(&err)) {
			vty_out(vty,"%s raised: %s",err.name,err.message);
			dbus_error_free_for_dcli(&err);
		}
		return CMD_WARNING;
	}
	if (dbus_message_get_args(reply, &err,
		DBUS_TYPE_INT32, &op_ret,
		DBUS_TYPE_STRING,&bsr_str,							
		DBUS_TYPE_INVALID)) 
	{
		dbus_message_unref(reply);
		if(op_ret==0)
			vty_out(vty,"%s\n",bsr_str);
		else if(op_ret == MRT_DISABLE )
		{
			vty_out(vty,"The system isn't run multicast-routing !\n");
			return CMD_WARNING;
		}
		else
		{
			vty_out(vty,"Can't get RP-candidate infomation\n");
			return CMD_WARNING;
		}
	} 
	else 
	{		
		if (dbus_error_is_set(&err)) {
			vty_out(vty,"%s raised: %s",err.name,err.message);
			dbus_error_free_for_dcli(&err);
		}
		dbus_message_unref(reply);
		return CMD_WARNING;
	}
	return CMD_SUCCESS;
}

/* When string format is invalid return 0. */
static int
str2ip_ipv4 (const char *str, unsigned int *p,unsigned int *masklen)
{
  int ret;
  int plen;
  char *pnt;
  char *cp;
  int flag=1;

  /* Find slash inside string. */
  pnt = strchr (str, '/');

  /* String doesn't contail slash. */
  if (pnt == NULL) 
    {
      /* Convert string to prefix. */
      ret = inet_aton (str, p);
      if (ret == 0)
				return 0;

      masklen = 0;

      return ret;
    }
  else
    {
      cp = malloc ((pnt - str) + 1);
      strncpy (cp, str, pnt - str);
      *(cp + (pnt - str)) = '\0';
	if(!strcmp(cp,"0.0.0.0"))
		flag=0;
      ret = inet_aton (cp, p);
      free (cp);

      /* Get prefix length. */
	/*avoid "a.b.c.d/024"*/
      if(('0'==*(pnt+1))&&flag)
	  	return 0;
      *masklen =  atoi (++pnt);
      if (*masklen > 32)
				return 0;

    }

  return ret;
}

DEFUN ( pim_rp_address,
  pim_rp_address_cmd,
  "ip pim rp-address A.B.C.D group A.B.C.D/M",
  IP_STR
  PIM_STR
  "Static PIM RP address (Rendezvous Point)\n"
  "Rendezvous-point IP address\n"
  MULTICAST_GROUP_STR
  "Group prefix and length of mask\n")
{
	
	DBusMessage *query = NULL, *reply = NULL;
	DBusError err = {0};
	int op_ret = -1;
	unsigned int enable = 1;
	unsigned int rp_addr=0,grp_addr=0,grp_masken=0,mask=0;
	int ret;
	int detect = 0;
	
	ret = str2ip_ipv4(argv[0],&rp_addr,&grp_masken);

	//rp_addr=dcli_ip2ulong(argv[0]);
	if(0 == ret) {
		 vty_out(vty, "%% Bad parameter %s\n", argv[0]);
		 return CMD_WARNING;
	}
	if(argc==2){
		ret=str2ip_ipv4(argv[1],&grp_addr,&grp_masken);
		if(0 == ret) {
			 vty_out(vty, "%% Bad parameter %s\n", argv[1]);
			 return CMD_WARNING;
		}
	}
	else
	{
		grp_addr=0xe0000000;
		grp_masken=0x4;
	}
	MASKLEN_TO_MASK(grp_masken,mask);

	query = dbus_message_new_method_call(PIMD_DBUS_BUSNAME, 
									PIMD_DBUS_OBJPATH, 
									PIMD_DBUS_INTERFACE, 
									PIMD_DBUS_INTERFACE_METHOD_SET_RP_STATIC);
	dbus_error_init(&err);
	dbus_message_append_args(query,
							DBUS_TYPE_UINT32, &enable, 
							DBUS_TYPE_UINT32, &rp_addr, 
							DBUS_TYPE_UINT32, &grp_addr, 
							DBUS_TYPE_UINT32, &mask, 							
							DBUS_TYPE_INVALID);
	reply = dbus_connection_send_with_reply_and_block(dcli_dbus_connection,query,-1, &err);
	dbus_message_unref(query);
	if (NULL == reply) {
		vty_out(vty,"failed get reply.\n");
		if (dbus_error_is_set(&err)) {
			vty_out(vty,"%s raised: %s",err.name,err.message);
			dbus_error_free_for_dcli(&err);
		}
		return CMD_WARNING;
	}
	if (!dbus_message_get_args(reply, &err,
		DBUS_TYPE_INT32, &op_ret,
		DBUS_TYPE_INVALID)) 
	{		
		if (dbus_error_is_set(&err)) {
			vty_out(vty,"%s raised: %s",err.name,err.message);
			dbus_error_free_for_dcli(&err);
		}
		dbus_message_unref(reply);
	}
	else
	{
	
		dbus_message_unref(reply);
		if(op_ret == MRT_ERR_MIPADDR)
		{
			vty_out(vty,"Error multicast address 0x%x\n",grp_addr);
			return CMD_WARNING;
		}
		else if(op_ret == MRT_DISABLE )
		{
			vty_out(vty,"The system isn't run multicast-routing !\n");
			return CMD_WARNING;
		}
		else if(op_ret == MRT_ERR_IPADDR )
		{
			vty_out(vty,"Error rp address 0x%x\n",rp_addr);
			return CMD_WARNING;
		}
		else if(op_ret!=0)
		{
			vty_out(vty,"Set RP infomation failure\n");
			return CMD_WARNING;
		}	
		
	}
	return CMD_SUCCESS;



}

ALIAS ( pim_rp_address,
  pim_rp_address_cmd1,
  "ip pim rp-address A.B.C.D ",
  IP_STR
  PIM_STR
  "Static PIM RP address (Rendezvous Point)\n"
  "Rendezvous-point IP address\n")

DEFUN ( no_pim_rp_address,
  no_pim_rp_address_cmd,
  "no ip pim rp-address A.B.C.D group A.B.C.D/M",
  NO_STR
  IP_STR
  PIM_STR
  "PIM RP-address (Rendezvous Point)\n"
  "Rendezvous-point IP address for group\n"
  MULTICAST_GROUP_STR
  "Group prefix and length of mask\n" )
{
	
	DBusMessage *query = NULL, *reply = NULL;
	DBusError err = {0};
	int op_ret = 0;
	unsigned int enable = 0;
	unsigned int rp_addr=0,grp_addr=0,grp_masken=0,mask;
	int ret;
	int detect = 0;

	ret = str2ip_ipv4(argv[0],&rp_addr,&grp_masken);

	//rp_addr=dcli_ip2ulong(argv[0]);
	if(0 == ret) {
		 vty_out(vty, "%% Bad parameter %s\n", argv[0]);
		 return CMD_WARNING;
	}
	if(argc==2){
		ret=str2ip_ipv4(argv[1],&grp_addr,&grp_masken);
		if(0 == ret) {
			 vty_out(vty, "%% Bad parameter %s\n", argv[1]);
			 return CMD_WARNING;
		}
	}
	else
	{
		grp_addr=0xe0000000;
		grp_masken=0x4;
	}
	MASKLEN_TO_MASK(grp_masken,mask);

	query = dbus_message_new_method_call(PIMD_DBUS_BUSNAME, 
									PIMD_DBUS_OBJPATH, 
									PIMD_DBUS_INTERFACE, 
									PIMD_DBUS_INTERFACE_METHOD_SET_RP_STATIC);
	dbus_error_init(&err);
	dbus_message_append_args(query,
							DBUS_TYPE_UINT32, &enable, 
							DBUS_TYPE_UINT32, &rp_addr, 
							DBUS_TYPE_UINT32, &grp_addr, 
							DBUS_TYPE_UINT32, &mask,							
							DBUS_TYPE_INVALID);
	reply = dbus_connection_send_with_reply_and_block(dcli_dbus_connection,query,-1, &err);
	dbus_message_unref(query);
	if (NULL == reply) {
		vty_out(vty,"failed get reply.\n");
		if (dbus_error_is_set(&err)) {
			vty_out(vty,"%s raised: %s",err.name,err.message);
			dbus_error_free_for_dcli(&err);
		}
		return CMD_WARNING;
	}
	if (!dbus_message_get_args(reply, &err,
		DBUS_TYPE_INT32, &op_ret,
		DBUS_TYPE_INVALID)) 
	{		
		if (dbus_error_is_set(&err)) {
			vty_out(vty,"%s raised: %s",err.name,err.message);
			dbus_error_free_for_dcli(&err);
		}
		dbus_message_unref(reply);
		return CMD_WARNING;
	}
	else
	{
		dbus_message_unref(reply);
		if(op_ret == MRT_ERR_MIPADDR)
		{
			vty_out(vty,"Error multicast address 0x%x\n",grp_addr);
			return CMD_WARNING;
		}
		else if(op_ret == MRT_DISABLE )
		{
			vty_out(vty,"The system isn't run multicast-routing !\n");
			return CMD_WARNING;
		}
		else if(op_ret == MRT_ERR_IPADDR )
		{
			vty_out(vty,"Error rp address 0x%x\n",rp_addr);
			return CMD_WARNING;
		}
		else if(op_ret!=0)
		{
			vty_out(vty,"Set RP infomation failure\n");
			return CMD_WARNING;
		}	
		
	}
	
	return CMD_SUCCESS;



}

ALIAS ( no_pim_rp_address,
  no_pim_rp_address_cmd1,
"no ip pim rp-address A.B.C.D ",
NO_STR
IP_STR
PIM_STR
"PIM RP-address (Rendezvous Point)\n"
"Rendezvous-point IP address for group\n")





DEFUN ( show_ip_pim_rp,
  show_ip_pim_rp_cmd,
  "show ip pim rp",
  SHOW_STR
  IP_STR
  PIM_STR
  "PIM Rendezvous Point (RP) information\n" )
{
	DBusMessage *query = NULL, *reply = NULL;
	DBusError err = {0};
	int op_ret = -1;
	unsigned int enable = 1;
	int detect = 0;
	char* bsr_str;
	query = dbus_message_new_method_call(PIMD_DBUS_BUSNAME, 
									PIMD_DBUS_OBJPATH, 
									PIMD_DBUS_INTERFACE, 
									PIMD_DBUS_INTERFACE_METHOD_SHOW_RP);
	dbus_error_init(&err);
	dbus_message_append_args(query,
							DBUS_TYPE_UINT32, &enable, 
							DBUS_TYPE_INVALID);
	reply = dbus_connection_send_with_reply_and_block(dcli_dbus_connection,query,-1, &err);
	dbus_message_unref(query);
	if (NULL == reply) {
		vty_out(vty,"failed get reply.\n");
		if (dbus_error_is_set(&err)) {
			vty_out(vty,"%s raised: %s",err.name,err.message);
			dbus_error_free_for_dcli(&err);
		}
		return CMD_WARNING;
	}
	if (dbus_message_get_args(reply, &err,
		DBUS_TYPE_INT32, &op_ret,
		DBUS_TYPE_STRING,&bsr_str,							
		DBUS_TYPE_INVALID)) 
	{
		dbus_message_unref(reply);
		if(op_ret==0)
			vty_out(vty,"%s\n",bsr_str);
		else if(op_ret == MRT_DISABLE )
		{
			vty_out(vty,"The system isn't run multicast-routing !\n");
			return CMD_WARNING;
		}
		else
		{
			vty_out(vty,"Get RP infomation failure\n");
			return CMD_WARNING;
		}
	} 
	else 
	{		
		if (dbus_error_is_set(&err)) {
			vty_out(vty,"%s raised: %s",err.name,err.message);
			dbus_error_free_for_dcli(&err);
		}
		dbus_message_unref(reply);
		return CMD_WARNING;
	}
	return CMD_SUCCESS;
}


DEFUN ( show_ip_pim_interface,
  show_ip_pim_interface_cmd,
  "show ip pim interface",
  SHOW_STR
  IP_STR
  PIM_STR
  INTERFACE_STR )
{
	DBusMessage *query = NULL, *reply = NULL;
	DBusError err = {0};
	int op_ret = -1;
	unsigned int enable = 1;
	int detect = 0;
	char* bsr_str;
	query = dbus_message_new_method_call(PIMD_DBUS_BUSNAME, 
									PIMD_DBUS_OBJPATH, 
									PIMD_DBUS_INTERFACE, 
									PIMD_DBUS_INTERFACE_METHOD_SHOW_IF);
	dbus_error_init(&err);
	dbus_message_append_args(query,
							DBUS_TYPE_UINT32, &enable, 
							DBUS_TYPE_INVALID);
	reply = dbus_connection_send_with_reply_and_block(dcli_dbus_connection,query,-1, &err);
	dbus_message_unref(query);
	if (NULL == reply) {
		vty_out(vty,"failed get reply.\n");
		if (dbus_error_is_set(&err)) {
			vty_out(vty,"%s raised: %s",err.name,err.message);
			dbus_error_free_for_dcli(&err);
		}
		return CMD_WARNING;
	}
	if (dbus_message_get_args(reply, &err,
		DBUS_TYPE_INT32, &op_ret,
		DBUS_TYPE_STRING,&bsr_str,							
		DBUS_TYPE_INVALID)) 
	{
		dbus_message_unref(reply);
		if(op_ret==0)
			vty_out(vty,"%s\n",bsr_str);
		else if(op_ret == MRT_DISABLE )
		{
			vty_out(vty,"The system isn't run multicast-routing !\n");
			return CMD_WARNING;
		}
		else
		{
			vty_out(vty,"Get pim interface infomation failure\n");
			return CMD_WARNING;
		}
	
	} 
	else 
	{		
		if (dbus_error_is_set(&err)) {
			vty_out(vty,"%s raised: %s",err.name,err.message);
			dbus_error_free_for_dcli(&err);
		}
		dbus_message_unref(reply);
		return CMD_WARNING;
		
	}
	return CMD_SUCCESS;
}


DEFUN ( show_ip_pim_interface_name_eth,
  show_ip_pim_interface_cmd1,
  "show ip pim interface IFNAME",
  SHOW_STR
  IP_STR
  PIM_STR
  INTERFACE_STR
  "Interface name\n" )
{
	DBusMessage *query = NULL, *reply = NULL;
	DBusError err = {0};
	int op_ret = -1;
	unsigned int enable = 1;
	int detect = 0;
	char* bsr_str;

	
	
	query = dbus_message_new_method_call(PIMD_DBUS_BUSNAME, 
									PIMD_DBUS_OBJPATH, 
									PIMD_DBUS_INTERFACE, 
									PIMD_DBUS_INTERFACE_METHOD_SHOW_IF1);
	dbus_error_init(&err);
	dbus_message_append_args(query,
							DBUS_TYPE_UINT32, &enable, 
							DBUS_TYPE_STRING,&argv[0],							
							DBUS_TYPE_INVALID);
	reply = dbus_connection_send_with_reply_and_block(dcli_dbus_connection,query,-1, &err);
	dbus_message_unref(query);
	if (NULL == reply) {
		vty_out(vty,"failed get reply.\n");
		if (dbus_error_is_set(&err)) {
			vty_out(vty,"%s raised: %s",err.name,err.message);
			dbus_error_free_for_dcli(&err);
		}
		return CMD_WARNING;
	}
	if (dbus_message_get_args(reply, &err,
		DBUS_TYPE_INT32, &op_ret,
		DBUS_TYPE_STRING,&bsr_str,							
		DBUS_TYPE_INVALID)) 
	{
	
		dbus_message_unref(reply);
		if(op_ret==0)
			vty_out(vty,"%s\n",bsr_str);
		else if(op_ret == MRT_DISABLE )
		{
			vty_out(vty,"The system isn't run multicast-routing !\n");
			return CMD_WARNING;
		}
		else if(op_ret == PIM_INTERFACE_DISABLE )
		{
			vty_out(vty,"The interface %s isn't run ip pim !\n",argv[0]);
			return CMD_WARNING;
		}
		else
		{
			vty_out(vty,"Get pim interface infomation failure\n");
			return CMD_WARNING;
		}

	} 
	else 
	{		
		if (dbus_error_is_set(&err)) {
			vty_out(vty,"%s raised: %s",err.name,err.message);
			dbus_error_free_for_dcli(&err);
			vty_out(vty,"Can't get pim interface infomation\n");
		}
		dbus_message_unref(reply);
		return CMD_WARNING;
	}
	return CMD_SUCCESS;
}


DEFUN ( show_ip_pim_neighbor,
  show_ip_pim_neighbor_cmd,
  "show ip pim neighbor",
  SHOW_STR
  IP_STR
  PIM_STR
  "PIM neighbor information\n" )
{
	DBusMessage *query = NULL, *reply = NULL;
	DBusError err = {0};
	unsigned int op_ret = 0;
	unsigned int enable = 1;
	int detect = 0;
	char* bsr_str;

	
	
	query = dbus_message_new_method_call(PIMD_DBUS_BUSNAME, 
									PIMD_DBUS_OBJPATH, 
									PIMD_DBUS_INTERFACE, 
									PIMD_DBUS_INTERFACE_METHOD_SHOW_NBR);
	dbus_error_init(&err);
	dbus_message_append_args(query,
							DBUS_TYPE_UINT32, &enable, 
							DBUS_TYPE_INVALID);
	reply = dbus_connection_send_with_reply_and_block(dcli_dbus_connection,query,-1, &err);
	dbus_message_unref(query);
	if (NULL == reply) {
		vty_out(vty,"failed get reply.\n");
		if (dbus_error_is_set(&err)) {
			vty_out(vty,"%s raised: %s",err.name,err.message);
			dbus_error_free_for_dcli(&err);
		}
		return CMD_WARNING;
	}
	if (dbus_message_get_args(reply, &err,
		DBUS_TYPE_UINT32, &op_ret,
		DBUS_TYPE_STRING,&bsr_str,							
		DBUS_TYPE_INVALID)) 
	{
		if(op_ret==0)
			vty_out(vty,"%s\n",bsr_str);
		else
			vty_out(vty,"Can't get pim neighbor infomation\n");
		
		dbus_message_unref(reply);
	} 
	else 
	{		
		if (dbus_error_is_set(&err)) {
			vty_out(vty,"%s raised: %s",err.name,err.message);
			dbus_error_free_for_dcli(&err);
			vty_out(vty,"Can't get pim neighbor infomation\n");
			
		}
		
		dbus_message_unref(reply);
		return CMD_WARNING;
	}
	return CMD_SUCCESS;
}





DEFUN ( ip_pim_query_interval,
  ip_pim_query_interval_cmd,
  "ip pim query-interval <1-18724>",
  IP_STR
  PIM_STR
  "PIM router query interval\n"
  "Query interval in seconds, default is 30\n" )
{
	DBusMessage *query = NULL, *reply = NULL;
	DBusError err = {0};
	int op_ret = -1;
	unsigned int enable = 1;
	short query_interval=atoi(argv[0]);
	query = dbus_message_new_method_call(PIMD_DBUS_BUSNAME, 
									PIMD_DBUS_OBJPATH, 
									PIMD_DBUS_INTERFACE, 
									PIMD_DBUS_INTERFACE_METHOD_QUERY_INTERVAL);
	dbus_error_init(&err);
	dbus_message_append_args(query,
							DBUS_TYPE_UINT32, &enable, 
							DBUS_TYPE_INT16, &query_interval, 
							DBUS_TYPE_INVALID);
	reply = dbus_connection_send_with_reply_and_block(dcli_dbus_connection,query,-1, &err);
	dbus_message_unref(query);
	if (NULL == reply) {
		vty_out(vty,"failed get reply.\n");
		if (dbus_error_is_set(&err)) {
			vty_out(vty,"%s raised: %s",err.name,err.message);
			dbus_error_free_for_dcli(&err);
		}
		return CMD_WARNING;
	}
	if (dbus_message_get_args(reply, &err,
		DBUS_TYPE_INT32, &op_ret,
		DBUS_TYPE_INVALID)) 
		{
		dbus_message_unref(reply);
		if(op_ret == MRT_DISABLE )
		{
			vty_out(vty,"The system isn't run multicast-routing !\n");
			return CMD_WARNING;
		}
		else if(op_ret!=0)
		{
			vty_out(vty,"Set ip pim query-interval failure!\n");
			return CMD_WARNING;

		}
	} 
	else {		
		if (dbus_error_is_set(&err)) {
			vty_out(vty,"%s raised: %s",err.name,err.message);
			dbus_error_free_for_dcli(&err);
		}
		dbus_message_unref(reply);
		return CMD_WARNING;
	}
	
	return CMD_SUCCESS;
	
}


DEFUN ( no_ip_pim_query_interval,
  no_ip_pim_query_interval_cmd,
  "no ip pim query-interval",
  NO_STR
  IP_STR
  PIM_STR
  "PIM router query interval\n" )
{
	DBusMessage *query = NULL, *reply = NULL;
	DBusError err = {0};
	int op_ret = -1;
	unsigned int enable = 1;
	short query_interval=30;
	query = dbus_message_new_method_call(PIMD_DBUS_BUSNAME, 
									PIMD_DBUS_OBJPATH, 
									PIMD_DBUS_INTERFACE, 
									PIMD_DBUS_INTERFACE_METHOD_QUERY_INTERVAL);
	dbus_error_init(&err);
	dbus_message_append_args(query,
							DBUS_TYPE_UINT32, &enable, 
							DBUS_TYPE_INT16, &query_interval, 
							DBUS_TYPE_INVALID);
	reply = dbus_connection_send_with_reply_and_block(dcli_dbus_connection,query,-1, &err);
	dbus_message_unref(query);
	if (NULL == reply) {
		vty_out(vty,"failed get reply.\n");
		if (dbus_error_is_set(&err)) {
			vty_out(vty,"%s raised: %s",err.name,err.message);
			dbus_error_free_for_dcli(&err);
		}
		return CMD_WARNING;
	}
	if (dbus_message_get_args(reply, &err,
		DBUS_TYPE_INT32, &op_ret,
		DBUS_TYPE_INVALID)) 
		{
		dbus_message_unref(reply);
		if(op_ret == MRT_DISABLE )
		{
			vty_out(vty,"The system isn't run multicast-routing !\n");
			return CMD_WARNING;
		}
		else if(op_ret!=0)
		{
			vty_out(vty,"Set ip pim query-interval failure!\n");
			return CMD_WARNING;

		}
	} 
	else {		
		if (dbus_error_is_set(&err)) {
			vty_out(vty,"%s raised: %s",err.name,err.message);
			dbus_error_free_for_dcli(&err);
		}
		dbus_message_unref(reply);
		return CMD_WARNING;
	}
	
	return CMD_SUCCESS;
	
}



DEFUN ( ip_pim_message_interval,
	ip_pim_message_interval_cmd,
	"ip pim message-interval <1-65535>",
	IP_STR
	PIM_STR
	"PIM router Join/Prune message interval\n"
	"Message interval in seconds\n" )
{
	DBusMessage *query = NULL, *reply = NULL;
	DBusError err = {0};
	int op_ret = -1;
	unsigned int enable = 1;
	unsigned short msg_interval=(unsigned short)atoi(argv[0]);
	query = dbus_message_new_method_call(PIMD_DBUS_BUSNAME, 
									PIMD_DBUS_OBJPATH, 
									PIMD_DBUS_INTERFACE, 
									PIMD_DBUS_INTERFACE_METHOD_MSG_INTERVAL);
	dbus_error_init(&err);
	dbus_message_append_args(query,
							DBUS_TYPE_UINT32, &enable, 
							DBUS_TYPE_UINT16, &msg_interval, 
							DBUS_TYPE_INVALID);
	reply = dbus_connection_send_with_reply_and_block(dcli_dbus_connection,query,-1, &err);
	dbus_message_unref(query);
	if (NULL == reply) {
		vty_out(vty,"failed get reply.\n");
		if (dbus_error_is_set(&err)) {
			vty_out(vty,"%s raised: %s",err.name,err.message);
			dbus_error_free_for_dcli(&err);
		}
		return CMD_WARNING;
	}
	if (dbus_message_get_args(reply, &err,
		DBUS_TYPE_INT32, &op_ret,
		DBUS_TYPE_INVALID)) 
		{
		dbus_message_unref(reply);
		if(op_ret == MRT_DISABLE )
		{
			vty_out(vty,"The system isn't run multicast-routing !\n");
			return CMD_WARNING;
		}
		else if(op_ret!=0)
		{
			vty_out(vty,"Set ip pim message-interval failure!\n");
			return CMD_WARNING;

		}
	} 
	else {		
		if (dbus_error_is_set(&err)) {
			vty_out(vty,"%s raised: %s",err.name,err.message);
			dbus_error_free_for_dcli(&err);
		}
		dbus_message_unref(reply);
		return CMD_WARNING;
	}
	
	return CMD_SUCCESS;
	
}


DEFUN ( no_ip_pim_message_interval,
	no_ip_pim_message_interval_cmd,
	"no ip pim message-interval",
	NO_STR
	IP_STR
	PIM_STR
	"PIM router Join/Prune message interval\n" )
{
	DBusMessage *query = NULL, *reply = NULL;
	DBusError err = {0};
	int op_ret = -1;
	unsigned int enable = 1;
	unsigned short msg_interval=60;
	query = dbus_message_new_method_call(PIMD_DBUS_BUSNAME, 
									PIMD_DBUS_OBJPATH, 
									PIMD_DBUS_INTERFACE, 
									PIMD_DBUS_INTERFACE_METHOD_MSG_INTERVAL);
	dbus_error_init(&err);
	dbus_message_append_args(query,
							DBUS_TYPE_UINT32, &enable, 
							DBUS_TYPE_UINT16, &msg_interval, 
							DBUS_TYPE_INVALID);
	reply = dbus_connection_send_with_reply_and_block(dcli_dbus_connection,query,-1, &err);
	dbus_message_unref(query);
	if (NULL == reply) {
		vty_out(vty,"failed get reply.\n");
		if (dbus_error_is_set(&err)) {
			vty_out(vty,"%s raised: %s",err.name,err.message);
			dbus_error_free_for_dcli(&err);
		}
		return CMD_WARNING;
	}
	if (dbus_message_get_args(reply, &err,
		DBUS_TYPE_INT32, &op_ret,
		DBUS_TYPE_INVALID)) 
		{
		dbus_message_unref(reply);
		if(op_ret == MRT_DISABLE )
		{
			vty_out(vty,"The system isn't run multicast-routing !\n");
			return CMD_WARNING;
		}
		else if(op_ret!=0)
		{
			vty_out(vty,"Set ip pim message-interval failure!\n");
			return CMD_WARNING;

		}
	} 
	else {		
		if (dbus_error_is_set(&err)) {
			vty_out(vty,"%s raised: %s",err.name,err.message);
			dbus_error_free_for_dcli(&err);
		}
		dbus_message_unref(reply);
		return CMD_WARNING;
	}
	
	return CMD_SUCCESS;
	
}


DEFUN ( ip_pim_dr_priority,
  ip_pim_dr_priority_cmd,
  "ip pim dr-priority <0-4294967294>",
  IP_STR
  PIM_STR
  "PIM router DR priority\n"
  "DR priority, preference given to larger value\n" )
{
	DBusMessage *query = NULL, *reply = NULL;
	DBusError err = {0};
	int op_ret = -1;
	unsigned int enable = 1;
	unsigned int priority=atoi(argv[0]);
	char* ifname = (char*)vty->index;
	query = dbus_message_new_method_call(PIMD_DBUS_BUSNAME, 
									PIMD_DBUS_OBJPATH, 
									PIMD_DBUS_INTERFACE, 
									PIMD_DBUS_INTERFACE_METHOD_DR_PRIORITY);
	dbus_error_init(&err);
	dbus_message_append_args(query,
							DBUS_TYPE_UINT32, &enable, 
							DBUS_TYPE_UINT32, &priority, 
							DBUS_TYPE_STRING, &ifname,							
							DBUS_TYPE_INVALID);
	reply = dbus_connection_send_with_reply_and_block(dcli_dbus_connection,query,-1, &err);
	dbus_message_unref(query);
	if (NULL == reply) {
		vty_out(vty,"failed get reply.\n");
		if (dbus_error_is_set(&err)) {
			vty_out(vty,"%s raised: %s",err.name,err.message);
			dbus_error_free_for_dcli(&err);
		}
		return CMD_WARNING;
	}
	if (dbus_message_get_args(reply, &err,
		DBUS_TYPE_INT32, &op_ret,
		DBUS_TYPE_INVALID)) 
		{
		dbus_message_unref(reply);
		if(op_ret == MRT_DISABLE )
		{
			vty_out(vty,"The system isn't run multicast-routing !\n");
			return CMD_WARNING;
		}
		else if(op_ret == PIM_INTERFACE_DISABLE )
		{
			vty_out(vty,"The interface %s isn't runing ip pim !\n",argv[0]);
			return CMD_WARNING;
		}
		else if(op_ret!=0)
		{
			vty_out(vty,"Set the interface ip pim dr priority failure!\n");
			return CMD_WARNING;

		}
	} 
	else {		
		if (dbus_error_is_set(&err)) {
			vty_out(vty,"%s raised: %s",err.name,err.message);
			dbus_error_free_for_dcli(&err);
		}
		dbus_message_unref(reply);
		return CMD_WARNING;
	}
	
	return CMD_SUCCESS;
	
}


DEFUN ( no_ip_pim_dr_priority,
  no_ip_pim_dr_priority_cmd,
  "no ip pim dr-priority",
  NO_STR
  IP_STR
  PIM_STR
  "PIM router DR priority\n" )
{
	DBusMessage *query = NULL, *reply = NULL;
	DBusError err = {0};
	int op_ret = -1;
	unsigned int enable = 1;
	unsigned int priority=0;
	char* ifname = (char*)vty->index;
	query = dbus_message_new_method_call(PIMD_DBUS_BUSNAME, 
									PIMD_DBUS_OBJPATH, 
									PIMD_DBUS_INTERFACE, 
									PIMD_DBUS_INTERFACE_METHOD_DR_PRIORITY);
	dbus_error_init(&err);
	dbus_message_append_args(query,
							DBUS_TYPE_UINT32, &enable, 
							DBUS_TYPE_UINT32, &priority, 
							DBUS_TYPE_STRING, &ifname,							
							DBUS_TYPE_INVALID);
	reply = dbus_connection_send_with_reply_and_block(dcli_dbus_connection,query,-1, &err);
	dbus_message_unref(query);
	if (NULL == reply) {
		vty_out(vty,"failed get reply.\n");
		if (dbus_error_is_set(&err)) {
			vty_out(vty,"%s raised: %s",err.name,err.message);
			dbus_error_free_for_dcli(&err);
		}
		return CMD_WARNING;
	}
	if (dbus_message_get_args(reply, &err,
		DBUS_TYPE_INT32, &op_ret,
		DBUS_TYPE_INVALID)) 
		{
		dbus_message_unref(reply);
		if(op_ret == MRT_DISABLE )
		{
			vty_out(vty,"The system isn't run multicast-routing !\n");
			return CMD_WARNING;
		}
		else if(op_ret == PIM_INTERFACE_DISABLE )
		{
			vty_out(vty,"The interface %s isn't runing ip pim !\n",argv[0]);
			return CMD_WARNING;
		}
		else if(op_ret!=0)
		{
			vty_out(vty,"Set the interface ip pim dr priority failure!\n");
			return CMD_WARNING;

		}
	} 
	else {		
		if (dbus_error_is_set(&err)) {
			vty_out(vty,"%s raised: %s",err.name,err.message);
			dbus_error_free_for_dcli(&err);
		}
		dbus_message_unref(reply);
		return CMD_WARNING;
	}
	
	return CMD_SUCCESS;
	
}


DEFUN ( ip_pim_bsr_border,
  ip_pim_bsr_border_cmd,
  "ip pim bsr-border",
  IP_STR
  PIM_STR
  "PIM router bsr-border\n"
  )
{
	DBusMessage *query = NULL, *reply = NULL;
	DBusError err = {0};
	int op_ret = -1;
	unsigned int enable = 1;
	char* ifname = (char*)vty->index;
	query = dbus_message_new_method_call(PIMD_DBUS_BUSNAME, 
									PIMD_DBUS_OBJPATH, 
									PIMD_DBUS_INTERFACE, 
									PIMD_DBUS_INTERFACE_METHOD_BSR_BORDER);
	dbus_error_init(&err);
	dbus_message_append_args(query,
							DBUS_TYPE_UINT32, &enable, 
							DBUS_TYPE_STRING,&ifname,							
							DBUS_TYPE_INVALID);
	reply = dbus_connection_send_with_reply_and_block(dcli_dbus_connection,query,-1, &err);
	dbus_message_unref(query);
	if (NULL == reply) {
		vty_out(vty,"failed get reply.\n");
		if (dbus_error_is_set(&err)) {
			vty_out(vty,"%s raised: %s",err.name,err.message);
			dbus_error_free_for_dcli(&err);
		}
		return CMD_WARNING;
	}
	if (dbus_message_get_args(reply, &err,
		DBUS_TYPE_INT32, &op_ret,
		DBUS_TYPE_INVALID)) 
		{
		dbus_message_unref(reply);
		if(op_ret == MRT_DISABLE )
		{
			vty_out(vty,"The system isn't run multicast-routing !\n");
			return CMD_WARNING;
		}
		else if(op_ret == PIM_INTERFACE_DISABLE )
		{
			vty_out(vty,"The interface %s isn't runing ip pim !\n",argv[0]);
			return CMD_WARNING;
		}
		else if(op_ret!=0)
		{
			vty_out(vty,"Set the interface ip pim bsr-border failure!\n");
			return CMD_WARNING;

		}
	} 
	else {		
		if (dbus_error_is_set(&err)) {
			vty_out(vty,"%s raised: %s",err.name,err.message);
			dbus_error_free_for_dcli(&err);
		}
		dbus_message_unref(reply);
		return CMD_WARNING;
	}
	
	return CMD_SUCCESS;
	
}


DEFUN ( no_ip_pim_bsr_border,
  no_ip_pim_bsr_border_cmd,
  "no ip pim bsr-border",
  NO_STR
  IP_STR
  PIM_STR
  "PIM router bsr-border\n" )
{
	DBusMessage *query = NULL, *reply = NULL;
	DBusError err = {0};
	int op_ret = -1;
	unsigned int enable = 0;
	char* ifname = (char*)vty->index;
	query = dbus_message_new_method_call(PIMD_DBUS_BUSNAME, 
									PIMD_DBUS_OBJPATH, 
									PIMD_DBUS_INTERFACE, 
									PIMD_DBUS_INTERFACE_METHOD_BSR_BORDER);
	dbus_error_init(&err);
	dbus_message_append_args(query,
							DBUS_TYPE_UINT32, &enable, 
							DBUS_TYPE_STRING,&ifname,							
							DBUS_TYPE_INVALID);
	reply = dbus_connection_send_with_reply_and_block(dcli_dbus_connection,query,-1, &err);
	dbus_message_unref(query);
	if (NULL == reply) {
		vty_out(vty,"failed get reply.\n");
		if (dbus_error_is_set(&err)) {
			vty_out(vty,"%s raised: %s",err.name,err.message);
			dbus_error_free_for_dcli(&err);
		}
		return CMD_WARNING;
	}
	if (dbus_message_get_args(reply, &err,
		DBUS_TYPE_INT32, &op_ret,
		DBUS_TYPE_INVALID)) 
		{
		dbus_message_unref(reply);
		if(op_ret == MRT_DISABLE )
		{
			vty_out(vty,"The system isn't run multicast-routing !\n");
			return CMD_WARNING;
		}
		else if(op_ret == PIM_INTERFACE_DISABLE )
		{
			vty_out(vty,"The interface %s isn't runing ip pim !\n",argv[0]);
			return CMD_WARNING;
		}
		else if(op_ret!=0)
		{
			vty_out(vty,"Set the interface ip pim bsr-border failure!\n");
			return CMD_WARNING;

		}
	} 
	else {		
		if (dbus_error_is_set(&err)) {
			vty_out(vty,"%s raised: %s",err.name,err.message);
			dbus_error_free_for_dcli(&err);
		}
		dbus_message_unref(reply);
		return CMD_WARNING;
	}
	
	return CMD_SUCCESS;
	
}








DEFUN ( clear_ip_pim_mroute,
  clear_ip_pim_mroute_cmd,
  "clear ip mroute A.B.C.D",
  CLEAR_STR
  IP_STR
  MULTICAST_ROUTE_STR
  MULTICAST_GROUP_IP_STR )
{

}

DEFUN ( clear_ip_pim_mfib,
  clear_ip_pim_mfib_cmd,
  "clear ip pim mfib",
  CLEAR_STR
  IP_STR
  "PIM protocol\n"
  "Multicast FIB information\n" )
{

}





DEFUN ( show_vif_func,
	show_vif_cmd,
	"show ip multicast interface",
	SHOW_STR
	IP_STR
	"IP multicast routing information\n"
	"Interface list and status\n" )
{

}


			 
DEFUN( show_ip_mroute_static_summary,
	show_ip_mroute_static_summary_cmd,
	"show ip mroute-static summary",
	SHOW_STR
	IP_STR
	"Static multicast route\n"
	"Summary information\n"
	)
{
}


DEFUN( show_ip_mroute_static_source,
	show_ip_mroute_static_source_cmd,
	"show ip mroute-static source <A.B.C.D>",
	SHOW_STR
	IP_STR
	"Static multicast route\n"
	"Multicast source\n"
	MULTICAST_SOURCE_IP_STR )
{

}


DEFUN( show_ip_mroute_static,
	show_ip_mroute_static_cmd,
	"show ip mroute-static A.B.C.D/M",
	SHOW_STR
	IP_STR
	"Static multicast route\n"
	"Multicast group prefix and length of mask\n"
)
{

}



DEFUN( show_ip_mroute,
	show_ip_mroute_cmd,
	"show ip mroute",
	SHOW_STR
	IP_STR
	MULTICAST_ROUTE_STR )
{
	DBusMessage *query = NULL, *reply = NULL;
	DBusError err = {0};
	int op_ret = -1;
	unsigned int enable = 1;
	int detect = 0;
	char* ret_str;
	query = dbus_message_new_method_call(PIMD_DBUS_BUSNAME, 
									PIMD_DBUS_OBJPATH, 
									PIMD_DBUS_INTERFACE, 
									PIMD_DBUS_INTERFACE_METHOD_SHOW_MRT);
	dbus_error_init(&err);
	dbus_message_append_args(query,
							DBUS_TYPE_UINT32, &enable, 
							DBUS_TYPE_INVALID);
	reply = dbus_connection_send_with_reply_and_block(dcli_dbus_connection,query,-1, &err);
	dbus_message_unref(query);
	if (NULL == reply) {
		vty_out(vty,"failed get reply.\n");
		if (dbus_error_is_set(&err)) {
			vty_out(vty,"%s raised: %s",err.name,err.message);
			dbus_error_free_for_dcli(&err);
		}
		return CMD_WARNING;
	}
	if (dbus_message_get_args(reply, &err,
		DBUS_TYPE_INT32, &op_ret,
		DBUS_TYPE_STRING,&ret_str,							
		DBUS_TYPE_INVALID)) 
	{
		dbus_message_unref(reply);
		if(op_ret==0)
			vty_out(vty,"%s\n",ret_str);
		else if(op_ret == MRT_DISABLE )
		{
			vty_out(vty,"The system isn't run multicast-routing !\n");
			return CMD_WARNING;
		}
		else
		{
			vty_out(vty,"Get multicast route failure !\n");
			return CMD_WARNING;

		}
	} 
	else 
	{		
		if (dbus_error_is_set(&err)) {
			vty_out(vty,"%s raised: %s",err.name,err.message);
			dbus_error_free_for_dcli(&err);
		}
		dbus_message_unref(reply);
		return CMD_WARNING;
		
	}
	return CMD_SUCCESS;
}


DEFUN ( show_ip_mroute_group,
  show_ip_mroute_group_cmd,
  "show ip mroute [<A.B.C.D>|<A.B.C.D/M>]",
  SHOW_STR
  IP_STR
  MULTICAST_ROUTE_STR
  MULTICAST_GROUP_IP_STR
  "Multicast group prefix and length of mask\n" )
{

}

DEFUN ( show_ip_mroute_source,
  show_ip_mroute_source_cmd,
  "show ip mroute source <A.B.C.D>",
  SHOW_STR
  IP_STR
  MULTICAST_ROUTE_STR
  "Multicast source\n"
  MULTICAST_SOURCE_IP_STR )
{

}


DEFUN ( show_ip_mroute_count,
  show_ip_mroute_count_cmd,
  "show ip mroute summary",
  SHOW_STR
  IP_STR
  MULTICAST_ROUTE_STR
  "Summary information\n" )
{

}

static struct debugname {
    char *name;
    int	 level;
    int	 nchars;
} debugnames[] = {
    {   "igmp_proto",	    DEBUG_IGMP_PROTO,     6	    },
    {   "igmp_timers",	    DEBUG_IGMP_TIMER,     6	    },
    {   "igmp_members",	    DEBUG_IGMP_MEMBER,    6	    },
    {   "groups",	    DEBUG_MEMBER,         1	    },
    {   "igmp",	            DEBUG_IGMP, 	  1	    },
    {   "trace",	    DEBUG_TRACE,          2	    },
    {   "timeout",	    DEBUG_TIMEOUT,        2	    },
    {   "packets",	    DEBUG_PKT,  	  2	    },
    {   "interfaces",       DEBUG_IF,   	  2	    },
    {   "kernel",           DEBUG_KERN,           2	    },
    {   "cache",            DEBUG_MFC,   	  1	    },
    {   "pim_detail",       DEBUG_PIM_DETAIL,     5	    },
    {   "pim_hello",        DEBUG_PIM_HELLO,      5	    },
    {   "pim_register",     DEBUG_PIM_REGISTER,   5	    },
    {   "pim_join_prune",   DEBUG_PIM_JOIN_PRUNE, 5	    },
    {   "pim_bootstrap",    DEBUG_PIM_BOOTSTRAP,  5	    },
    {   "pim_asserts",      DEBUG_PIM_ASSERT,     5	    },
    {   "pim_cand_rp",      DEBUG_PIM_CAND_RP,    5	    },
    {   "pim_routing",      DEBUG_PIM_MRT,        6	    },
    {   "pim_timers",       DEBUG_PIM_TIMER,      5	    },
    {   "pim_rpf",          DEBUG_PIM_RPF,        6	    },
    {   "rpf",              DEBUG_RPF,            3	    },
    {   "pim",              DEBUG_PIM,  	  1	    },
    {   "routes",	    DEBUG_MRT,            1	    },
    {   "routers",          DEBUG_NEIGHBORS,      6	    },
    {   "timers",           DEBUG_TIMER,          1	    },
    {   "asserts",          DEBUG_ASSERT,         1	    },
    {   "all",              DEBUG_ALL,            2         },
};

DEFUN ( debug_pim_func,
  debug_pim_func_cmd,
  "debug mrout (igmp_proto|igmp_timers|igmp_members|groups|igmp|trace|timeout|packets|interfaces\
  |kernel|cache|pim_detail|pim_hello|pim_register|pim_join_prune|pim_bootstrap|pim_asserts|pim_cand_rp\
  |pim_routing|pim_timers|pim_rpf|rpf|pim|routes|routers|timers|asserts|all)",
  DEBUG_STR
  MULTICAST_ROUTE_STR
  "igmp_proto\n"
  "igmp_timers\n"
  "igmp_members\n"
  "groups\n"
  "igmp\n"
  "trace\n"
  "timeout\n"
  "packets\n"
  "interfaces\n"
  "kernel\n"
  "cache\n"
  "pim_detail\n"
  "pim_hello\n"
  "pim_register\n"
  "pim_join_prune\n"
  "pim_bootstrap\n"
  "pim_asserts\n"
  "pim_cand_rp\n"
  "pim_routing\n"
  "pim_timers\n"
  "pim_rpf\n"
  "rpf\n"
  "pim\n"
  "routes\n"
  "routers\n"
  "timers\n"
  "asserts\n"
  "all\n")
{
	DBusMessage *query = NULL, *reply = NULL;
	DBusError err = {0};
	int op_ret = -1;
	unsigned int enable = 1,debug=0;
	int detect = 0,i=0;
	struct debugname *d;

	
	for (i = 0, d = debugnames;
	 i < sizeof(debugnames) / sizeof(debugnames[0]);
	 i++, d++)
	 {
		if(strncmp(d->name,argv[0],strlen(argv[0]))==0)
			break;
	 }
	 if(i>=sizeof(debugnames) / sizeof(debugnames[0])){
	 	vty_out(vty,"Error debug name %s\n",argv[0]);
		return CMD_WARNING;
	 }
	 
	 debug |= d->level;
	 
	query = dbus_message_new_method_call(PIMD_DBUS_BUSNAME, 
									PIMD_DBUS_OBJPATH, 
									PIMD_DBUS_INTERFACE, 
									PIMD_DBUS_INTERFACE_METHOD_DEBUG_MRT);
	dbus_error_init(&err);
	dbus_message_append_args(query,
								DBUS_TYPE_UINT32, &enable, 
								DBUS_TYPE_UINT32, &debug, 
							 DBUS_TYPE_INVALID);
	reply = dbus_connection_send_with_reply_and_block(dcli_dbus_connection,query,-1, &err);
	dbus_message_unref(query);
	
	if (NULL == reply) {
		
		vty_out(vty,"failed get reply.\n");
		if (dbus_error_is_set(&err)) {
			vty_out(vty,"%s raised: %s",err.name,err.message);
			dbus_error_free_for_dcli(&err);
		}
		return CMD_WARNING;
	}
	
	if (dbus_message_get_args(reply, &err,
		DBUS_TYPE_INT32, &op_ret,
		DBUS_TYPE_INVALID)) 
	{
		dbus_message_unref(reply);
		if(op_ret != 0)
		{
			vty_out(vty,"Set debug failure\n");
			return CMD_WARNING;
		}
	} 
	else {		
		if (dbus_error_is_set(&err)) {
			vty_out(vty,"%s raised: %s",err.name,err.message);
			dbus_error_free_for_dcli(&err);
		}
		dbus_message_unref(reply);
		return CMD_WARNING;
	}
	
	return CMD_SUCCESS;
}


DEFUN ( no_debug_pim_func,
  no_debug_pim_func_cmd,
  "no debug mrout (igmp_proto|igmp_timers|igmp_members|groups|igmp|trace|timeout|packets|interfaces\
  |kernel|cache|pim_detail|pim_hello|pim_register|pim_join_prune|pim_bootstrap|pim_asserts|pim_cand_rp\
  |pim_routing|pim_timers|pim_rpf|rpf|pim|routes|routers|timers|asserts|all)",
	NO_STR
	DEBUG_STR
  MULTICAST_ROUTE_STR
  "igmp_proto"
  "igmp_timers"
  "igmp_members"
  "groups"
  "igmp"
  "trace"
  "timeout"
  "packets"
  "interfaces"
  "kernel"
  "cache"
  "pim_detail"
  "pim_hello"
  "pim_register"
  "pim_join_prune"
  "pim_bootstrap"
  "pim_asserts"
  "pim_cand_rp"
  "pim_routing"
  "pim_timers"
  "pim_rpf"
  "rpf"
  "pim"
  "routes"
  "routers"
  "timers"
  "asserts"
  "all")
{
	DBusMessage *query = NULL, *reply = NULL;
	DBusError err = {0};
	int op_ret = -1;
	unsigned int enable = 0,debug=0;
	int detect = 0,i=0;
	struct debugname *d;

	
	for (i = 0, d = debugnames;
	 i < sizeof(debugnames) / sizeof(debugnames[0]);
	 i++, d++)
	 {
		if(strncmp(d->name,argv[0],strlen(argv[0]))==0)
			break;
	 }
	 if(i>=sizeof(debugnames) / sizeof(debugnames[0])){
	 	vty_out(vty,"Error debug name %s\n",argv[0]);
		return CMD_WARNING;
	 }
	 
	 vty_out(vty,"debug = 0x%x;d->level = 0x%X ",debug,d->level);
	 debug |= d->level;
	 
	query = dbus_message_new_method_call(PIMD_DBUS_BUSNAME, 
									PIMD_DBUS_OBJPATH, 
									PIMD_DBUS_INTERFACE, 
									PIMD_DBUS_INTERFACE_METHOD_DEBUG_MRT);
	dbus_error_init(&err);
	dbus_message_append_args(query,
								DBUS_TYPE_UINT32, &enable, 
								DBUS_TYPE_UINT32, &debug, 
							 DBUS_TYPE_INVALID);
	reply = dbus_connection_send_with_reply_and_block(dcli_dbus_connection,query,-1, &err);
	dbus_message_unref(query);
	
	if (NULL == reply) {
		
		vty_out(vty,"failed get reply.\n");
		if (dbus_error_is_set(&err)) {
			vty_out(vty,"%s raised: %s",err.name,err.message);
			dbus_error_free_for_dcli(&err);
		}
		return CMD_WARNING;
	}
	
	if (dbus_message_get_args(reply, &err,
		DBUS_TYPE_INT32, &op_ret,
		DBUS_TYPE_INVALID)) 
	{
		dbus_message_unref(reply);
		if(op_ret != 0)
		{
			vty_out(vty,"Set debug failure\n");
			return CMD_WARNING;
		}
	} 
	else {		
		if (dbus_error_is_set(&err)) {
			vty_out(vty,"%s raised: %s",err.name,err.message);
			dbus_error_free_for_dcli(&err);
		}
		dbus_message_unref(reply);
		return CMD_WARNING;
	}
	
	return CMD_SUCCESS;
}
struct cmd_node pimd_node =
{
  PIMD_NODE,
  "	",
  1,
};

int dcli_pimd_write(struct vty*vty)
{
	DBusMessage *query = NULL, *reply = NULL;
	DBusError err = {0};
	int op_ret = -1;
	unsigned int enable = 1;
	int detect = 0;
	char* bsr_str;
	query = dbus_message_new_method_call(PIMD_DBUS_BUSNAME, 
									PIMD_DBUS_OBJPATH, 
									PIMD_DBUS_INTERFACE, 
									PIMD_DBUS_INTERFACE_METHOD_DEBUG_WRITE);
	dbus_error_init(&err);
	dbus_message_append_args(query,
							DBUS_TYPE_UINT32, &enable, 
							DBUS_TYPE_INVALID);
	reply = dbus_connection_send_with_reply_and_block(dcli_dbus_connection,query,-1, &err);
	dbus_message_unref(query);
	if (NULL == reply) {
		if (dbus_error_is_set(&err)) {
			dbus_error_free_for_dcli(&err);
		}
		return CMD_WARNING;
	}
	if (dbus_message_get_args(reply, &err,
		DBUS_TYPE_INT32, &op_ret,
		DBUS_TYPE_STRING,&bsr_str,							
		DBUS_TYPE_INVALID)) 
	{
		dbus_message_unref(reply);
		if(op_ret==0){
			
			char _tmpstr[64];
			memset(_tmpstr,0,64);
			sprintf(_tmpstr,BUILDING_MOUDLE,"IP PIM");
			vtysh_add_show_string(_tmpstr);
			vtysh_add_show_string(bsr_str);

		}
		else if(op_ret==1)
		{
			return CMD_SUCCESS;

		}
		
	} 
	else 
	{		
		if (dbus_error_is_set(&err)) {
			dbus_error_free_for_dcli(&err);
		}
		dbus_message_unref(reply);
		return CMD_WARNING;
	}
	return CMD_SUCCESS;
}

void dcli_pim_init() {
	install_node(&pimd_node,dcli_pimd_write,"PIMD_NODE");
	install_element(CONFIG_NODE,&mrt_multicast_routing_cmd);
	install_element(CONFIG_NODE,&no_mrt_multicast_routing_cmd);
	install_element(CONFIG_NODE,&pim_bsr_candidate_eth_cmd);
	install_element(CONFIG_NODE,&no_pim_bsr_candidate_eth_cmd);
	install_element(CONFIG_NODE,&show_ip_pim_bsr_router_cmd);
	install_element(ENABLE_NODE,&show_ip_pim_bsr_router_cmd);
	install_element(VIEW_NODE,&show_ip_pim_bsr_router_cmd);

	install_element(CONFIG_NODE,&show_ip_pim_rp_cmd);
	install_element(ENABLE_NODE,&show_ip_pim_rp_cmd);
	install_element(VIEW_NODE,&show_ip_pim_rp_cmd);
	install_element(CONFIG_NODE,&set_pim_rp_candidate_cmd);
	install_element(CONFIG_NODE,&del_set_pim_rp_candidate_cmd);
	install_element(CONFIG_NODE,&show_pim_rp_candidate_cmd);
	install_element(ENABLE_NODE,&show_pim_rp_candidate_cmd);
	install_element(VIEW_NODE,&show_pim_rp_candidate_cmd);
	install_element(CONFIG_NODE,&pim_rp_address_cmd);
	install_element(CONFIG_NODE,&pim_rp_address_cmd1);
	install_element(CONFIG_NODE,&no_pim_rp_address_cmd);
	install_element(CONFIG_NODE,&no_pim_rp_address_cmd1);

	install_element(CONFIG_NODE,&ip_pim_query_interval_cmd);
	install_element(CONFIG_NODE,&no_ip_pim_query_interval_cmd);
	
	install_element(CONFIG_NODE,&ip_pim_message_interval_cmd);
	install_element(CONFIG_NODE,&no_ip_pim_message_interval_cmd);

	install_element(CONFIG_NODE,&show_ip_pim_interface_cmd);
	install_element(ENABLE_NODE,&show_ip_pim_interface_cmd);
	install_element(VIEW_NODE,&show_ip_pim_interface_cmd);
	install_element(CONFIG_NODE,&show_ip_pim_interface_cmd1);
	install_element(ENABLE_NODE,&show_ip_pim_interface_cmd1);
	install_element(VIEW_NODE,&show_ip_pim_interface_cmd1);

	install_element(CONFIG_NODE,&show_ip_mroute_cmd);
	install_element(ENABLE_NODE,&show_ip_mroute_cmd);
	install_element(VIEW_NODE,&show_ip_mroute_cmd);

	install_element(CONFIG_NODE,&debug_pim_func_cmd);
	install_element(ENABLE_NODE,&debug_pim_func_cmd);
	install_element(CONFIG_NODE,&no_debug_pim_func_cmd);
	install_element(ENABLE_NODE,&no_debug_pim_func_cmd);
	/*
	install_element(CONFIG_NODE,&show_ip_pim_neighbor_cmd);
	install_element(ENABLE_NODE,&show_ip_pim_neighbor_cmd);
	
	install_element(CONFIG_NODE,&ip_mroute_static_eth_eth_cmd);
	install_element(CONFIG_NODE,&no_ip_mroute_static_eth_eth_cmd);
*/
	install_element(INTERFACE_NODE,&pim_start_cmd);
	install_element(INTERFACE_NODE,&no_pim_start_cmd);
	
	install_element(INTERFACE_NODE,&ip_pim_bsr_border_cmd);
	install_element(INTERFACE_NODE,&no_ip_pim_bsr_border_cmd);
	
	install_element(INTERFACE_NODE,&ip_pim_dr_priority_cmd);
	install_element(INTERFACE_NODE,&no_ip_pim_dr_priority_cmd);

}


