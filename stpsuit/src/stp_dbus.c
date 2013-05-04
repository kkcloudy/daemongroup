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
* stp_dbus.c
*
* CREATOR:
*       zhubo@autelan.com
*
* DESCRIPTION:
*       APIs for dbus message handling in stp module
*
* DATE:
*       04/18/2008
*
*  FILE REVISION NUMBER:
*       $Revision: 1.2 $
*******************************************************************************/
#ifdef __cplusplus
extern "C"
{
#endif
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <dbus/dbus.h>
#include <syslog.h>

#include "stp_dbus.h"
#include "stp_base.h"
#include "stp_uid.h"
#include "stp_statmch.h"
#include "stp_port.h"

#include "stp_stpm.h"
#include "stp_bpdu.h"
#include "stp_times.h"

#include "stp_param.h"
#include "stp_to.h"
#include "stp_in.h"

static DBusConnection *stp_dbus_connection = NULL;
extern unsigned int productId;
extern unsigned int stp_log_close;
extern struct stp_admin_infos **stp_ports;

DBusMessage * stp_dbus_get_stpm_state(DBusConnection *conn, DBusMessage *msg, void *user_data){

	DBusMessage* reply;
	DBusMessageIter iter = {0};
	DBusError		err;
	int ret = 0;
	//stp mode
	int stpmode = 0;

	ret = stp_param_check_stp_state();
	stpmode = stp_param_get_running_mode();
	
	
	STP_DBUS_DEBUG(("STP>>stp_dbus_get_stpm_state: state[%d]\n",ret));
	reply = dbus_message_new_method_return(msg);
    dbus_message_append_args(reply,
							 DBUS_TYPE_UINT32,&ret,
							 DBUS_TYPE_UINT32,&stpmode,
							 DBUS_TYPE_INVALID);
	
	return reply;
}

DBusMessage * stp_dbus_stpm_enable(DBusConnection *conn, DBusMessage *msg, void *user_data){

	DBusMessage* reply;
	DBusMessageIter iter = {0};
	DBusError		err;

	unsigned int	ret = STP_OK;
	int	enable  = 0;
	RUNNING_MODE mode;
	int count = 0;

	stp_syslog_dbg("STP>>stp dbus message handler: bstp enable\n");

	dbus_error_init( &err );

	if( !(dbus_message_get_args( msg ,&err, \
					DBUS_TYPE_UINT32,&enable, \
					DBUS_TYPE_UINT32,&mode,
					DBUS_TYPE_INVALID)))
	{
		stp_syslog_dbg("STP>>Unable to get input args\n");
		if(dbus_error_is_set( &err ))
		{
			STP_DBUS_ERR(("STP>>%s raised:%s\n",err.name ,err.message));
			dbus_error_free( &err );
		}
		return NULL;
	}

	/* here call rstp api */
	 if (0 == enable) {
	    stp_syslog_dbg("STP>>disable stpm\n");
		ret = stp_param_set_stpm_disable();
		if(0 == ret) {
			stp_param_set_running_mode(NOT_MODE);
			stp_npd_wan_mstp_enable(0);
		}
	  }
	  else{
		stp_syslog_dbg("STP>>enable stpm\n");
		ret = stp_param_set_stpm_enable();
		if(0 == ret) {
			stp_param_set_running_mode(mode);
			stp_npd_wan_mstp_enable(1);
		}
	  }
	
	
	reply = dbus_message_new_method_return(msg);
	if(NULL==reply)
	{
		STP_DBUS_DEBUG(("STP>>dbus bridge priority no reply resource error!\n"));
		return reply;
	}
	dbus_message_iter_init_append (reply, &iter);
	dbus_message_iter_append_basic (&iter,DBUS_TYPE_UINT32,&ret);
	return reply;

	
}

DBusMessage * stp_dbus_mstp_init(DBusConnection *conn, DBusMessage *msg, void *user_data){

	DBusMessage* reply;
	DBusMessageIter iter = {0};
	DBusError		err;

	unsigned int i,ret = STP_OK;
	unsigned short vid = 0;
	unsigned int untagbmp = 0, tagbmp = 0;
	VLAN_PORTS_T vlanPorts = {{0}};

	STP_DBUS_DEBUG(("STP>>stp dbus message handler: mstp init\n"));

	dbus_error_init( &err );

	if( !(dbus_message_get_args( msg ,&err, \
					DBUS_TYPE_UINT16, &vid, \
					DBUS_TYPE_UINT32, &untagbmp, \
					DBUS_TYPE_UINT32, &tagbmp, \
					DBUS_TYPE_UINT32, &productId, \
					DBUS_TYPE_INVALID)))
	{
		STP_DBUS_ERR(("STP>>Unable to get input args\n"));
		if(dbus_error_is_set( &err ))
		{
			STP_DBUS_ERR(("STP>>%s raised:%s\n",err.name ,err.message));
			dbus_error_free( &err );
		}
		return NULL;
	}
	/* here call rstp api */
	
	tagbmp |= untagbmp;
	vlanPorts.vid = vid;
	STP_DBUS_DEBUG(("MSTP>>116 :: %02x \n",tagbmp));	
	for(i = 1 ; i < 5; i++) {
		vlanPorts.ports.part[i] = ((tagbmp >> ((i-1)*8)) & 0xff);
		STP_DBUS_DEBUG(("MSTP>>116 ::vlanPorts.ports.part[i]: %02x \n",vlanPorts.ports.part[i]));
	}
	STP_DBUS_DEBUG(("MSTP>>stp_dbus117 :: vid %d \n",vid));
	if(0 == vid) {
		ret |= stp_param_add_vlan_to_mstp(vid, vlanPorts.ports);
		STP_DBUS_DEBUG(("MSTP>>120 :: init vlan [%d]\n",vid));
	}
	else {
		stp_param_mstp_init_vlan_info(vid,vlanPorts.ports);
		STP_DBUS_DEBUG(("MSTP>>124 :: init vlan [%d]\n",vid));
	}
	STP_DBUS_DEBUG(("MSTP>>stp_dbus118 :: dbus mstp init !\n"));
	
	reply = dbus_message_new_method_return(msg);
	if(NULL==reply)
	{
		STP_DBUS_DEBUG(("STP>>dbus bridge priority no reply resource error!\n"));
		return reply;
	}
	dbus_message_iter_init_append (reply, &iter);
	dbus_message_iter_append_basic (&iter,DBUS_TYPE_UINT32,&ret);
	return reply;

}

DBusMessage * stp_dbus_mstp_init_v1(DBusConnection *conn, DBusMessage *msg, void *user_data){

	DBusMessage* reply;
	DBusMessageIter iter = {0};
	DBusError		err;

	unsigned int i,ret = STP_OK;
	unsigned short vid = 0;
	//unsigned int untagbmp = 0, tagbmp = 0;
	VLAN_PORTS_T vlanPorts;
	PORT_MEMBER_BMP untagbmp,tagbmp;
	
	memset(&vlanPorts,0,sizeof(VLAN_PORTS_T));
	memset(&untagbmp,0,sizeof(PORT_MEMBER_BMP));
	memset(&tagbmp,0,sizeof(PORT_MEMBER_BMP));
	stp_syslog_dbg("STP>>stp dbus message handler: mstp init\n");

	dbus_error_init( &err );

	if( !(dbus_message_get_args( msg ,&err, \
					DBUS_TYPE_UINT16, &vid, \
					DBUS_TYPE_UINT32, &untagbmp.portMbr[0], \
					DBUS_TYPE_UINT32, &untagbmp.portMbr[1], \
					DBUS_TYPE_UINT32, &tagbmp.portMbr[0], \
					DBUS_TYPE_UINT32, &tagbmp.portMbr[1], \
					DBUS_TYPE_UINT32, &productId, \
					DBUS_TYPE_INVALID)))
	{
		stp_syslog_dbg("STP>>Unable to get input args\n");
		if(dbus_error_is_set( &err ))
		{
			STP_DBUS_ERR(("STP>>%s raised:%s\n",err.name ,err.message));
			dbus_error_free( &err );
		}
		return NULL;
	}
	/* here call rstp api */
	
	tagbmp.portMbr[0] |= untagbmp.portMbr[0];
	tagbmp.portMbr[1] |= untagbmp.portMbr[1];
	vlanPorts.vid = vid;
	stp_syslog_dbg("Mstp init bitmap[0] %#x,[1]%#x \n",tagbmp.portMbr[0],tagbmp.portMbr[1]);	
	for(i = 1 ; i < 9; i++) {
		if(i<5){
		   vlanPorts.ports.part[i] = ((tagbmp.portMbr[0]>> ((i-1)*8)) & 0xff);
		}
		else{
		   vlanPorts.ports.part[i] = ((tagbmp.portMbr[1]>> ((i-5)*8)) & 0xff);

		}
		stp_syslog_dbg("vlanPorts.ports.part[%d]: %02x \n", i, vlanPorts.ports.part[i]);
	}
	if(0 == vid) {
		ret = stp_param_add_vlan_to_mstp(vid, vlanPorts.ports);
		if(0 == ret){
		   stp_syslog_dbg("init vid %d portbmp success!\n",vid);
		}
	}
	else {
		 stp_syslog_dbg("init vid %d portbmp\n",vid);
		 stp_param_mstp_init_vlan_info(vid,vlanPorts.ports);
	}

	reply = dbus_message_new_method_return(msg);
	if(NULL==reply)
	{
		stp_syslog_dbg("STP>>dbus bridge priority no reply resource error!\n");
		return reply;
	}
	dbus_message_iter_init_append (reply, &iter);
	dbus_message_iter_append_basic (&iter,DBUS_TYPE_UINT32,&ret);
	return reply;

}

DBusMessage * stp_dbus_stpm_port_enable(DBusConnection *conn, DBusMessage *msg, void *user_data){

	DBusMessage* reply;
	DBusMessageIter iter = {0};
	DBusError		err;

	int	ret = STP_OK;
	int	enable  = 0;
	int port_link = 0,link_state = 0;
	unsigned int port_speed = 0;
	unsigned int port_index = 0;
	unsigned int port_isWAN = 0; 
	unsigned int slot_no = 0;
	unsigned int port_no = 0;
	
	STP_DBUS_DEBUG(("STP>>stp dbus message handler: stp port enable\n"));

	dbus_error_init( &err );

	if( !(dbus_message_get_args( msg ,&err, \
					DBUS_TYPE_UINT32,&port_index, \
					DBUS_TYPE_UINT32,&enable,  \
					DBUS_TYPE_UINT32,&port_link,  \
					DBUS_TYPE_UINT32,&port_speed,  \					
					DBUS_TYPE_UINT32,&port_isWAN,  \				
					DBUS_TYPE_UINT32,&slot_no,  \
					DBUS_TYPE_UINT32,&port_no,  \
					DBUS_TYPE_INVALID)))
	{
		STP_DBUS_ERR(("STP>>Unable to get input args\n"));
		if(dbus_error_is_set( &err ))
		{
			STP_DBUS_ERR(("STP>>%s raised:%s\n",err.name ,err.message));
			dbus_error_free( &err );
		}
		return NULL;
	}
	/**************************
	*change lk value to rstp link state
	*
	**************************/
	if(port_link){
       link_state = STP_LINK_STATE_UP_E;
	}
	else {
       link_state = STP_LINK_STATE_DOWN_E;
	}
	STP_DBUS_DEBUG (("port_speed %d\n",port_speed));

	/* here call rstp api */
	if (0 == enable) {
		STP_DBUS_DEBUG (("STP>>disable stpm\n"));
		ret = stp_param_set_stpm_port_disable(port_index,link_state,port_speed, port_isWAN, slot_no, port_no);
	}
	else{
		STP_DBUS_DEBUG (("STP>>enable stpm\n"));
		ret = stp_param_set_stpm_port_enable(port_index,link_state,port_speed, port_isWAN, slot_no, port_no);
	}


	reply = dbus_message_new_method_return(msg);
	if(NULL==reply)
	{
		STP_DBUS_DEBUG(("STP>>dbus bridge priority no reply resource error!\n"));
		return reply;
	}
	dbus_message_iter_init_append (reply, &iter);
	dbus_message_iter_append_basic (&iter,DBUS_TYPE_INT32,&ret);
	return reply;

	
}

DBusMessage * stp_dbus_mstp_bridge_reg_name(DBusConnection *conn, DBusMessage *msg, void *user_data){

	DBusMessage* reply;
	DBusMessageIter iter = {0};
	DBusError		err;
	
	unsigned int	ret = STP_OK;
	char*	pname = NULL;

	STP_DBUS_DEBUG(("STP>>stp dbus message handler: bridge region name \n"));

	dbus_error_init( &err );

	if( !(dbus_message_get_args( msg ,&err, \
					DBUS_TYPE_STRING, &pname, \
					DBUS_TYPE_INVALID)))
	{
		STP_DBUS_ERR(("STP>>Unable to get input args\n"));
		if(dbus_error_is_set( &err ))
		{
			STP_DBUS_ERR(("STP>>%s raised:%s\n",err.name ,err.message));
			dbus_error_free( &err );
		}
		return NULL;
	}
	STP_DBUS_DEBUG(("STP>>config bridge-reg-name %s\n",pname));

	/* here call rstp api */
	if(stp_param_check_stp_state()) {
		ret = stp_param_set_bridge_region_name (pname);
	}
	else {
		ret = STP_NOT_ENABLED;
	}
	
	
	reply = dbus_message_new_method_return(msg);
	if(NULL==reply)
	{
		STP_DBUS_DEBUG(("STP>>dbus bridge priority no reply resource error!\n"));
		return reply;
	}
	dbus_message_iter_init_append (reply, &iter);
	dbus_message_iter_append_basic (&iter,DBUS_TYPE_UINT32,&ret);
	return reply;
}

DBusMessage * stp_dbus_mstp_bridge_revision(DBusConnection *conn, DBusMessage *msg, void *user_data){

	DBusMessage* reply;
	DBusMessageIter iter = {0};
	DBusError		err;
	
	unsigned int	ret = STP_OK;
	unsigned short revision = 0;

	STP_DBUS_DEBUG(("STP>>stp dbus message handler: bridge revision\n"));

	dbus_error_init( &err );

	if( !(dbus_message_get_args( msg ,&err, \
					DBUS_TYPE_UINT16, &revision, \
					DBUS_TYPE_INVALID)))
	{
		STP_DBUS_ERR(("STP>>Unable to get input args\n"));
		if(dbus_error_is_set( &err ))
		{
			STP_DBUS_ERR(("STP>>%s raised:%s\n",err.name ,err.message));
			dbus_error_free( &err );
		}
		return NULL;
	}
	STP_DBUS_DEBUG(("STP>>config bridge-priority %d\n",revision));

	/* here call rstp api */
	if(stp_param_check_stp_state()) {
		ret = stp_param_set_bridge_revision (revision);
	}
	else {
		ret = STP_NOT_ENABLED;
	}
	
	reply = dbus_message_new_method_return(msg);
	if(NULL==reply)
	{
		STP_DBUS_DEBUG(("STP>>dbus bridge priority no reply resource error!\n"));
		return reply;
	}
	dbus_message_iter_init_append (reply, &iter);
	dbus_message_iter_append_basic (&iter,DBUS_TYPE_UINT32,&ret);
	return reply;
}

/*
  * cmd used to test this method
  * 
  * dbus-send --system --dest=aw.stp --type=method_call --print-reply=literal /aw/stp aw.stp.max_age
  *
  * arg lists for method STP_DBUS_METHOD_BRG_MAXAGE
  * in arg list:
  *  	NONE
  *	
  * out arg list:  // in the order as they are appended in the dbus message. 
  *
  *
  */
DBusMessage * stp_dbus_bridge_priority(DBusConnection *conn, DBusMessage *msg, void *user_data) {
	
	DBusMessage* reply;
	DBusMessageIter iter = {0};
	DBusError		err;
	
	unsigned int	ret = STP_OK;
	unsigned int mstid = 0;
	unsigned int	priority = 0;

	STP_DBUS_DEBUG(("STP>>stp dbus message handler: bridge priority\n"));

	dbus_error_init( &err );

	if( !(dbus_message_get_args( msg ,&err, \
					DBUS_TYPE_UINT32, &mstid, \
					DBUS_TYPE_UINT32, &priority, \
					DBUS_TYPE_INVALID)))
	{
		STP_DBUS_ERR(("STP>>Unable to get input args\n"));
		if(dbus_error_is_set( &err ))
		{
			STP_DBUS_ERR(("STP>>%s raised:%s\n",err.name ,err.message));
			dbus_error_free( &err );
		}
		return NULL;
	}
	STP_DBUS_DEBUG(("STP>>config mstid %d ,bridge-priority %d\n",mstid,priority));	
	if(stp_param_check_stp_state()) {
		ret = stp_param_set_bridge_priority (mstid,priority);
	}
	else
	{
		ret = STP_NOT_ENABLED;
	}
	
	reply = dbus_message_new_method_return(msg);
	dbus_message_iter_init_append (reply, &iter);
	dbus_message_iter_append_basic (&iter,DBUS_TYPE_UINT32,&ret);
	return reply;
}

/*
  * cmd used to test this method
  * 
  * dbus-send --system --dest=aw.stp --type=method_call --print-reply=literal /aw/stp aw.stp.brg_prio
  *
  * arg lists for method STP_DBUS_METHOD_BRG_PRIORITY
  * in arg list:
  * 	NONE
  *
  * out arg list:  // in the order as they are appended in the dbus message. 
  *
  *
  */
DBusMessage * stp_dbus_bridge_maxage(DBusConnection *conn, DBusMessage *msg, void *user_data) {
	DBusMessage* reply;
	DBusMessageIter iter = {0};
	DBusError		err;
	unsigned int	ret = STP_OK;
	unsigned int	max_age = 0,mstid =0;

	STP_DBUS_DEBUG(("STP>>stp dbus message handler: bridge max age\n"));

	dbus_error_init( &err );

	if( !(dbus_message_get_args( msg ,&err, \
					DBUS_TYPE_UINT32, &max_age, \
					DBUS_TYPE_INVALID)))
	{
		STP_DBUS_ERR(("STP>>Unable to get input args\n"));
		if(dbus_error_is_set( &err ))
		{
			STP_DBUS_ERR(("STP>>%s raised:%s\n",err.name ,err.message));
			dbus_error_free( &err );
		}
		return NULL;
	}
	STP_DBUS_DEBUG(("STP>>config mstid %d ,bridge max-age %d\n",mstid,max_age));	
	/* here call rstp api */

	if( stp_param_check_stp_state()) {
		ret = stp_param_set_bridge_maxage(max_age);
	}
	else 
	{
		ret = STP_NOT_ENABLED;
	}

	reply = dbus_message_new_method_return(msg);
	dbus_message_iter_init_append (reply, &iter);
	
	dbus_message_iter_append_basic (&iter,DBUS_TYPE_UINT32,&ret);
	return reply;
}

/*
  * cmd used to test this method
  * 
  * dbus-send --system --dest=aw.stp --type=method_call --print-reply=literal /aw/stp aw.stp.heltime
  *
  * arg lists for method STP_DBUS_METHOD_BRG_HELTIME
  * in arg list:
  *  	NONE
  *	
  * out arg list:  // in the order as they are appended in the dbus message. 
  *
  *
  */
DBusMessage * stp_dbus_bridge_heltime(DBusConnection *conn, DBusMessage *msg, void *user_data) {
	DBusMessage* reply;
	DBusMessageIter iter = {0};
	DBusError err;
	unsigned int heltime = 0;
	unsigned int ret = STP_OK,mstid = 0;
	
	STP_DBUS_DEBUG(("STP>>Entering inspect hello-time!\n"));
	
	dbus_error_init( &err );

	if( !(dbus_message_get_args( msg ,&err, \
				DBUS_TYPE_UINT32, &heltime,  \
				DBUS_TYPE_INVALID)))
	{
		STP_DBUS_ERR(("STP>>Unable to get input args\n"));
		if(dbus_error_is_set( &err ))
		{
			STP_DBUS_ERR(("STP>>%s raised:%s\n",err.name ,err.message));
			dbus_error_free( &err );
		}
		return NULL;
	}
	
	STP_DBUS_DEBUG(("STP>>config mstid %d ,bridge heltime %d\n",mstid,heltime));	
	/* here call rstp api */
	if(stp_param_check_stp_state()) {
		ret = stp_param_set_bridge_hello_time (heltime);
	}
	else
		ret = STP_NOT_ENABLED;

	
	reply = dbus_message_new_method_return(msg);
	dbus_message_iter_init_append (reply, &iter);
	dbus_message_iter_append_basic (&iter,DBUS_TYPE_UINT32,&ret);
		
	return reply;
}

DBusMessage * stp_dbus_bridge_maxhops(DBusConnection *conn, DBusMessage *msg, void *user_data) {
	DBusMessage* reply;
	DBusMessageIter iter = {0};
	DBusError err;
	unsigned int maxhops = 0;
	unsigned int ret = STP_OK,mstid = 0;
	
	STP_DBUS_DEBUG(("STP>>Entering inspect hello-time!\n"));
	
	dbus_error_init( &err );

	if( !(dbus_message_get_args( msg ,&err, \
				DBUS_TYPE_UINT32, &maxhops,  \
				DBUS_TYPE_INVALID)))
	{
		STP_DBUS_ERR(("STP>>Unable to get input args\n"));
		if(dbus_error_is_set( &err ))
		{
			STP_DBUS_ERR(("STP>>%s raised:%s\n",err.name ,err.message));
			dbus_error_free( &err );
		}
		return NULL;
	}
	
	STP_DBUS_DEBUG(("STP>>config mstid %d ,bridge maxHops %d\n",mstid,maxhops));	
	/* here call rstp api */
	if( stp_param_check_stp_state()) {
		ret = stp_param_set_bridge_max_hops (maxhops);
	}
	else
		ret = STP_NOT_ENABLED;

	
	reply = dbus_message_new_method_return(msg);
	dbus_message_iter_init_append (reply, &iter);
	dbus_message_iter_append_basic (&iter,DBUS_TYPE_UINT32,&ret);
		
	return reply;
}

  /*
  * cmd used to test this method
  * 
  * dbus-send --system --dest=aw.stp --type=method_call --print-reply=literal /aw/stp aw.stp.brg_fordelay
  *
  * arg lists for method STP_DBUS_METHOD_BRG_FORDELAY
  * in arg list:
  *  	NONE
  *	
  * out arg list:  // in the order as they are appended in the dbus message. 
  *
  *
  */
DBusMessage * stp_dbus_bridge_fordelay(DBusConnection *conn, DBusMessage *msg, void *user_data) {
		
	DBusMessage* reply;
	DBusError err;
	unsigned int forward_delay;
	unsigned int ret = STP_OK,mstid = 0;
	DBusMessageIter iter = {0};
	STP_DBUS_DEBUG(("STP>>Entering inspect forward-delay!\n"));
	dbus_error_init( &err );
	if(!(dbus_message_get_args(msg,&err,
				DBUS_TYPE_UINT32, &forward_delay, 
				DBUS_TYPE_INVALID)))
	{
		STP_DBUS_ERR(("STP>>Unable to get input args\n"));
		if(dbus_error_is_set( &err ))
		{
			STP_DBUS_ERR(("STP>>%s raised:%s\n",err.name ,err.message));
			dbus_error_free( &err );
		}
		return NULL;
	}

	STP_DBUS_DEBUG(("STP>>config mstid %d,bridge forward-delay %d\n",mstid,forward_delay));
	/* here call rstp api */
	if( stp_param_check_stp_state()) {
		ret = stp_param_set_bridge_fdelay (forward_delay);
	}
	else
		ret = STP_NOT_ENABLED;
	
	reply = dbus_message_new_method_return(msg);
	dbus_message_iter_init_append (reply, &iter);
	dbus_message_iter_append_basic (&iter,DBUS_TYPE_UINT32,&ret);	
	return reply;
	}

/*
  * cmd used to test this method
  * 
  * dbus-send --system --dest=aw.stp --type=method_call --print-reply=literal /aw/stp aw.stp.max_age
  *
  * arg lists for method STP_DBUS_METHOD_BRG_FORVERSION
  * in arg list:
  *  	NONE
  *	
  * out arg list:  // in the order as they are appended in the dbus message. 
  *
  *
  */
DBusMessage * stp_dbus_bridge_forversion(DBusConnection *conn, DBusMessage *msg, void *user_data) {
		
	DBusMessage* reply;
	DBusError err;
	unsigned int ret = STP_OK;
	unsigned int force_version;
	DBusMessageIter iter = {0};
	STP_DBUS_DEBUG(("STP>>Entering inspect forward-delay!\n"));
	dbus_error_init( &err );
	if(!(dbus_message_get_args(msg,&err,
								DBUS_TYPE_UINT32,&force_version,
								DBUS_TYPE_INVALID)))
	{
		STP_DBUS_ERR(("STP>>Unable to get input args\n"));
		if(dbus_error_is_set( &err ))
		{
			STP_DBUS_ERR(("STP>>%s raised:%s\n",err.name ,err.message));
			dbus_error_free( &err );
		}
		ret = STP_ERR;	
	}
	STP_DBUS_DEBUG(("STP>>config bridge force-version %d\n",force_version));
	/* here call rstp api */
	if( stp_param_check_stp_state()) {
		ret = stp_param_set_bridge_fvers(force_version);
	}
	else {
		ret = STP_NOT_ENABLED;
	}  
	reply = dbus_message_new_method_return(msg);
	
	dbus_message_iter_init_append (reply, &iter);
	dbus_message_iter_append_basic (&iter,DBUS_TYPE_UINT32,&ret);		
	return reply;
	}

/*
  * cmd used to test this method
  * 
  * dbus-send --system --dest=aw.stp --type=method_call --print-reply=literal /aw/stp aw.stp.max_age
  *
  * arg lists for method STP_DBUS_METHOD_BRG_NOCONFIG
  * in arg list:
  *  	NONE
  *	
  * out arg list:  // in the order as they are appended in the dbus message. 
  *
  *
  */
DBusMessage * stp_dbus_bridge_noconfig(DBusConnection *conn, DBusMessage *msg, void *user_data) {
		
	DBusMessage* reply;
	DBusError err;
	int mstid = 0;
	unsigned char isStp = 0;
	unsigned int ret= STP_OK;
	STP_DBUS_DEBUG(("STP>>get default bridge value !\n"));

	dbus_error_init( &err );
	if(!(dbus_message_get_args(msg,&err,
								DBUS_TYPE_UINT32, &mstid,
								DBUS_TYPE_INVALID)))
	{
		STP_DBUS_ERR(("STP>>Unable to get input args\n"));
		if(dbus_error_is_set( &err ))
		{
			STP_DBUS_ERR(("STP>>%s raised:%s\n",err.name ,err.message));
			dbus_error_free( &err );
		}
		ret = STP_ERR;	
	}
	STP_DBUS_DEBUG(("STP>>config mstid %d ,bridge default value,isStp %d\n",mstid,isStp));
	if( stp_param_check_stp_state()) 
		ret = stp_param_set_bridge_nocfg (mstid);
	else
		ret = STP_NOT_ENABLED;
	
	reply = dbus_message_new_method_return(msg);
	dbus_message_append_args(reply,
								 DBUS_TYPE_UINT32, &ret,								 
								 DBUS_TYPE_INVALID);
		
		
	return reply;
	}

/*
  * cmd used to test this method
  * 
  * dbus-send --system --dest=aw.stp --type=method_call --print-reply=literal /aw/stp aw.stp.max_age
  *
  * arg lists for method STP_DBUS_METHOD_BRG_PORTPATHCOST
  * in arg list:
  *  	NONE
  *	
  * out arg list:  // in the order as they are appended in the dbus message. 
  *
  *
  */
DBusMessage * stp_dbus_bridge_port_pathcost(DBusConnection *conn, DBusMessage *msg, void *user_data) {
		
	DBusMessage* reply;
	DBusError err;
	
	unsigned int ret = STP_OK;
	unsigned int mstid = 0;
	unsigned int  port_index;
	unsigned int  path_cost;
	
	STP_DBUS_DEBUG(("STP>>Entering inspect port-pathcost!\n"));
	
	dbus_error_init( &err );
	if(!(dbus_message_get_args(msg,&err,
								DBUS_TYPE_UINT32, &mstid, \
								DBUS_TYPE_UINT32,&port_index,	\
								DBUS_TYPE_UINT32,&path_cost,	\
								DBUS_TYPE_INVALID)))
	{
		STP_DBUS_ERR(("STP>>Unable to get input args\n"));
		if(dbus_error_is_set( &err ))
		{
			STP_DBUS_ERR(("STP>>%s raised:%s\n",err.name ,err.message));
			dbus_error_free( &err );
		}
		ret= STP_ERR;	
	}
	STP_DBUS_DEBUG(("STP>>config mstid %d,port path-cost %d\n",mstid,path_cost));
	/* here call rstp api */
	if( stp_param_check_stp_state()) {
		STP_DBUS_DEBUG(("port index %d\n", port_index));
		ret = stp_param_set_port_past_cost(mstid,port_index,path_cost);
		}
	else
		ret = STP_NOT_ENABLED;
	
	reply = dbus_message_new_method_return(msg);
	dbus_message_append_args(reply,
								 DBUS_TYPE_UINT32,&ret,
								 DBUS_TYPE_INVALID);
		
		
	return reply;
	}
/*
  * cmd used to test this method
  * 
  * dbus-send --system --dest=aw.stp --type=method_call --print-reply=literal /aw/stp aw.stp.max_age
  *
  * arg lists for method STP_DBUS_METHOD_BRG_PORTPRIORITY
  * in arg list:
  *  	NONE
  *	
  * out arg list:  // in the order as they are appended in the dbus message. 
  *
  *
  */
DBusMessage * stp_dbus_bridge_port_prio(DBusConnection *conn, DBusMessage *msg, void *user_data) {
		
	DBusMessage* reply;
	DBusError err;
	unsigned int ret = STP_OK,mstid = 0;
	unsigned int  port, port_prio;
	
	STP_DBUS_DEBUG(("STP>>Entering inspect port-priority!\n"));
	dbus_error_init( &err );
	if(!(dbus_message_get_args(msg,&err,
								DBUS_TYPE_UINT32, &mstid, \
								DBUS_TYPE_UINT32,&port,
								DBUS_TYPE_UINT32,&port_prio,
								DBUS_TYPE_INVALID)))
	{
		STP_DBUS_ERR(("STP>>Unable to get input args\n"));
		if(dbus_error_is_set( &err ))
		{
			STP_DBUS_ERR(("STP>>%s raised:%s\n",err.name ,err.message));
			dbus_error_free( &err );
		}
		ret = STP_ERR;	
	}
	if (!(port_prio%16 == 0)){
		STP_DBUS_ERR (("STP>>port-priority value must be 16 time !\n"));
		ret= STP_ERR;
		}
	STP_DBUS_DEBUG(("STP>>config mstid %d , port port-priority %d\n",mstid,port_prio));
	/* here call rstp api */
	if( stp_param_check_stp_state())
		ret  = stp_param_set_port_priority(mstid,port, port_prio);
	else
		ret = STP_NOT_ENABLED;
	
	reply = dbus_message_new_method_return(msg);
	dbus_message_append_args(reply,
								 DBUS_TYPE_UINT32,&ret,
								 DBUS_TYPE_INVALID);
		
		
	return reply;
	}

/*
  * cmd used to test this method
  * 
  * dbus-send --system --dest=aw.stp --type=method_call --print-reply=literal /aw/stp aw.stp.max_age
  *
  * arg lists for method STP_DBUS_METHOD_BRG_NONSTP
  * in arg list:
  *  	NONE
  *	
  * out arg list:  // in the order as they are appended in the dbus message. 
  *
  *
  */
DBusMessage * stp_dbus_bridge_port_non_stp(DBusConnection *conn, DBusMessage *msg, void *user_data) {
		
	DBusMessage * reply;
	DBusError err;
	unsigned int ret = STP_OK,mstid = 0;
	unsigned int port;
	unsigned int value;
	STP_DBUS_DEBUG(("STP>>Entering inspect non-stp!\n"));
	dbus_error_init( &err );
	if(!(dbus_message_get_args(msg,&err,
								DBUS_TYPE_UINT32,&mstid, \
								DBUS_TYPE_UINT32,&port,
								DBUS_TYPE_UINT32,&value,
								DBUS_TYPE_INVALID)))
	{
		STP_DBUS_ERR(("STP>>Unable to get input args\n"));
		if(dbus_error_is_set( &err ))
		{
			STP_DBUS_ERR(("STP>>%s raised:%s\n",err.name ,err.message));
			dbus_error_free( &err );
		}
		ret = STP_ERR;	
	}
	
	
	if(!((value == 1)||(value == 0))){
		STP_DBUS_ERR(("STP>> input value not in law! \n"));
		return NULL;
	}
	
	if( stp_param_check_stp_state())
   		ret = stp_param_set_port_non_stp(mstid,port, value);
	else
		ret = STP_NOT_ENABLED;

	reply  = dbus_message_new_method_return(msg);
	dbus_message_append_args(reply,
								 DBUS_TYPE_UINT32,&ret,
								 DBUS_TYPE_INVALID);
		
		
	return reply;
	}

/*
  * cmd used to test this method
  * 
  * dbus-send --system --dest=aw.stp --type=method_call --print-reply=literal /aw/stp aw.stp.max_age
  *
  * arg lists for method STP_DBUS_METHOD_BRG_PORT_P2P
  * in arg list:
  *  	NONE
  *	
  * out arg list:  // in the order as they are appended in the dbus message. 
  *
  *
  */
DBusMessage * stp_dbus_bridge_port_p2p(DBusConnection *conn, DBusMessage *msg, void *user_data) {
		
	DBusMessage* reply;
	DBusError err;
	unsigned int ret = STP_OK,mstid = 0;
	unsigned int port_index;
	unsigned int p2p ;

	STP_DBUS_DEBUG(("STP>>Entering inspect p2p!\n"));
	dbus_error_init( &err );
	if(!(dbus_message_get_args(msg,&err,
								DBUS_TYPE_UINT32, &mstid, \
								DBUS_TYPE_UINT32,&port_index,
								DBUS_TYPE_UINT32,&p2p,
								DBUS_TYPE_INVALID)))
	{
		STP_DBUS_ERR(("STP>>Unable to get input args\n"));
		if(dbus_error_is_set( &err ))
		{
			STP_DBUS_ERR(("STP>>%s raised:%s\n",err.name ,err.message));
			dbus_error_free( &err );
		}
		ret = STP_ERR;	
	}
	
	STP_DBUS_DEBUG(("STP>>config mstid %d , port p2p %c \n",mstid,p2p));
	if( stp_param_check_stp_state()) 
		ret = stp_param_set_port_p2p (mstid,port_index,p2p);
	else 
		ret =  STP_NOT_ENABLED;
		
	reply = dbus_message_new_method_return(msg);
	dbus_message_append_args(reply,
								 DBUS_TYPE_UINT32,&ret,
								 DBUS_TYPE_INVALID);
		
		
	return reply;
	}

/*
  * cmd used to test this method
  * 
  * dbus-send --system --dest=aw.stp --type=method_call --print-reply=literal /aw/stp aw.stp.max_age
  *
  * arg lists for method STP_DBUS_METHOD_BRG_EDGE_PORT_EDGE
  * in arg list:
  *  	NONE
  *	
  * out arg list:  // in the order as they are appended in the dbus message. 
  *
  *
  */
DBusMessage * stp_dbus_bridge_port_edge(DBusConnection *conn, DBusMessage *msg, void *user_data) {
		
	DBusMessage* reply;
	DBusError err;
	unsigned int  port_index;
	unsigned int ret=STP_OK, mstid = 0;
	unsigned int value ;
	
	STP_DBUS_DEBUG(("STP>>Entering inspect edge!\n"));
	dbus_error_init( &err );
	if(!(dbus_message_get_args(msg,&err,
								DBUS_TYPE_UINT32,&mstid,
								DBUS_TYPE_UINT32,&port_index,
								DBUS_TYPE_UINT32,&value,
								DBUS_TYPE_INVALID)))
	{
		STP_DBUS_ERR(("STP>>Unable to get input args\n"));
		if(dbus_error_is_set( &err ))
		{
			STP_DBUS_ERR(("STP>>%s raised:%s\n",err.name ,err.message));
			dbus_error_free( &err );
		}
		return NULL;	
	}

	if(!((value == 0)||(value== 1))){
		return NULL;
		}
	
	STP_DBUS_DEBUG(("STP>>config mstid %d ,port edge %d\n",mstid,value));

	if( stp_param_check_stp_state())
		ret = stp_param_set_port_edge (mstid,port_index, value);
	else
		ret = STP_NOT_ENABLED;
	
		
	reply = dbus_message_new_method_return(msg);
	dbus_message_append_args(reply,
								 DBUS_TYPE_UINT32,&ret,
								 DBUS_TYPE_INVALID);
		
		
	return reply;
	}

/*
  * cmd used to test this method
  * 
  * dbus-send --system --dest=aw.stp --type=method_call --print-reply=literal /aw/stp aw.stp.mcheck
  *
  * arg lists for method STP_DBUS_METHOD_BRG_EDGE_PORT_MCHECK
  * in arg list:
  *  	NONE
  *	
  * out arg list:  // in the order as they are appended in the dbus message. 
  *
  *
  */
DBusMessage * stp_dbus_bridge_port_mcheck(DBusConnection *conn, DBusMessage *msg, void *user_data) {
		
	DBusMessage* reply;
	DBusError err;
	unsigned int  port_index,check,mstid = 0;
	
	unsigned int ret = STP_OK;
	
	STP_DBUS_DEBUG(("STP>>Entering inspect non-stp!\n"));
	dbus_error_init( &err );
	if(!(dbus_message_get_args(msg,&err,
								DBUS_TYPE_UINT32,&mstid,
								DBUS_TYPE_UINT32,&port_index,
								DBUS_TYPE_UINT32,&check,
								DBUS_TYPE_INVALID)))

		{
			if(dbus_error_is_set( &err ))
			{
			STP_DBUS_ERR(("STP>>%s raised:%s\n",err.name ,err.message));
			dbus_error_free( &err );
			}
		ret = STP_ERR;		
		}
	
	/* here call rstp api */
	if( stp_param_check_stp_state())
	 	ret = stp_param_set_port_mcheck (mstid,port_index,1);
	else
		ret = STP_NOT_ENABLED;
	
	
	reply = dbus_message_new_method_return(msg);
	dbus_message_append_args(reply,
								 DBUS_TYPE_UINT32,&ret,
								 DBUS_TYPE_INVALID);
		
		
	return reply;
	}

/*
  * cmd used to test this method
  * 
  * dbus-send --system --dest=aw.stp --type=method_call --print-reply=literal /aw/stp aw.stp.portnoconfig
  *
  * arg lists for method STP_DBUS_METHOD_BRG_EDGE_PORT_NOCONFIG
  * in arg list:
  *  	NONE
  *	
  * out arg list:  // in the order as they are appended in the dbus message. 
  *
  *
  */
DBusMessage * stp_dbus_bridge_port_noconfig(DBusConnection *conn, DBusMessage *msg, void *user_data) {
		
	DBusMessage* reply;
	DBusError err;
	unsigned int  port_index,non_stp,mstid = 0;
	unsigned int ret=STP_OK;
	STP_DBUS_DEBUG(("STP>>Entering inspect non-stp!\n"));
	dbus_error_init( &err );
	if(!(dbus_message_get_args(msg,&err,
								DBUS_TYPE_UINT32,&mstid,
								DBUS_TYPE_UINT32,&port_index,
								DBUS_TYPE_INVALID)))
		{
			if(dbus_error_is_set( &err ))
			{
			STP_DBUS_ERR(("STP>>%s raised:%s\n",err.name ,err.message));
			dbus_error_free( &err );
		}
		return NULL;	
	}

	STP_DBUS_DEBUG(("STP>>config port default-value null ,mstid %d,port %d\n",mstid,port_index));

	/* here call rstp api */
	if( stp_param_check_stp_state())
		ret =  stp_param_set_port_cfg_defaultvalue(mstid ,port_index);
	else
		ret = STP_NOT_ENABLED;

	reply = dbus_message_new_method_return(msg);
	dbus_message_append_args(reply,
								 DBUS_TYPE_UINT32,&ret,
								 DBUS_TYPE_INVALID);
		
		
	return reply;
}

/*
*
set the port duplex mode to jion the p2p compute
*
*/
DBusMessage * stp_dbus_mstp_port_duplex_mode_set(DBusConnection *conn, DBusMessage *msg, void *user_data){

	DBusMessage* reply;
	DBusMessageIter iter = {0};
	DBusError		err;
	
	unsigned int	ret = STP_OK;
	unsigned int    port_index = 0;
	unsigned int    duplex_mode = 0;

	dbus_error_init( &err );

	if( !(dbus_message_get_args( msg ,&err, \
		            DBUS_TYPE_UINT32,&port_index,
					DBUS_TYPE_UINT32, &duplex_mode, \
					DBUS_TYPE_INVALID)))
	{
		STP_DBUS_ERR(("STP>>Unable to get input args\n"));
		if(dbus_error_is_set( &err ))
		{
			STP_DBUS_ERR(("STP>>%s raised:%s\n",err.name ,err.message));
			dbus_error_free( &err );
		}
		return NULL;
	}
	printf("stp_dbus.c::stp_dbus_mstp_port_duplex_mode_set get the duplex mode value is %d\n",duplex_mode);

	/* here call rstp api */
	if(stp_param_check_stp_state()) {
		ret = stp_param_set_port_duplex_mode(port_index,duplex_mode);
	}
	else {
		ret = STP_NOT_ENABLED;
	}
	
	
	reply = dbus_message_new_method_return(msg);
	if(NULL==reply)
	{
		STP_DBUS_DEBUG(("STP>>dbus bridge duplex mode no reply resource error!\n"));
		return reply;
	}
	dbus_message_iter_init_append (reply, &iter);
	dbus_message_iter_append_basic (&iter,DBUS_TYPE_UINT32,&ret);
	return reply;
}


/*
  * cmd used to test this method
  * 
  * dbus-send --system --dest=aw.stp --type=method_call --print-reply=literal /aw/stp aw.stp.showbrginfo
  *
  * arg lists for method STP_DBUS_METHOD_BRG_SHOWSPANTREE
  * in arg list:
  *  	NONE
  *	
  * out arg list:  // in the order as they are appended in the dbus message. 
  *
  *
  */
DBusMessage *stp_dbus_show_spanning_tree_br_info(DBusConnection *conn, DBusMessage *msg, void *user_data) {

	DBusMessage* reply;

	DBusMessageIter iter;
	DBusMessageIter iter_array;
	DBusMessageIter iter_struct;
	
	int j = 0;
	int ret = 0,reval = 0;

	STPM_T* stpm = stp_stpm_get_the_cist();

	/*unsigned char  port_prio = 128;
	unsigned int     port_path_cost = 2000000;
	int     				port_role = DisabledPort;
	Bool                 port_adminEnable = False;
	Bool 				port_p2p = False;
	Bool                 port_edge = False;
	int   					port_state = 0;
	unsigned char mac[6] ={'\0','\0','\0','\0','\0','\0'};
	unsigned int 	br_32_def = 0;
	unsigned short br_16_def = 0;*/
		

	reply = dbus_message_new_method_return(msg);
	
	dbus_message_iter_init_append (reply, &iter);

	if( stp_param_check_stp_state())
	{
		dbus_message_iter_append_basic (&iter,
										 DBUS_TYPE_UINT32,
										 &ret);
		STP_DBUS_DEBUG(("RSTP>> into ....... ret %d\n",ret));
		dbus_message_iter_open_container (&iter,
											DBUS_TYPE_ARRAY,
											DBUS_STRUCT_BEGIN_CHAR_AS_STRING 
											DBUS_TYPE_BYTE_AS_STRING
											DBUS_TYPE_BYTE_AS_STRING
											DBUS_TYPE_BYTE_AS_STRING
											DBUS_TYPE_BYTE_AS_STRING
											DBUS_TYPE_BYTE_AS_STRING
											DBUS_TYPE_BYTE_AS_STRING
											DBUS_TYPE_UINT16_AS_STRING
											DBUS_TYPE_UINT32_AS_STRING
											DBUS_TYPE_UINT16_AS_STRING
											DBUS_TYPE_UINT16_AS_STRING
											DBUS_TYPE_UINT16_AS_STRING
											DBUS_TYPE_UINT16_AS_STRING
											DBUS_TYPE_BYTE_AS_STRING
											DBUS_TYPE_BYTE_AS_STRING
											DBUS_TYPE_BYTE_AS_STRING
											DBUS_TYPE_BYTE_AS_STRING
											DBUS_TYPE_BYTE_AS_STRING
											DBUS_TYPE_BYTE_AS_STRING
											DBUS_TYPE_UINT16_AS_STRING
											DBUS_TYPE_UINT32_AS_STRING
											DBUS_TYPE_UINT16_AS_STRING
											DBUS_TYPE_UINT16_AS_STRING
											DBUS_TYPE_UINT16_AS_STRING
											DBUS_STRUCT_END_CHAR_AS_STRING,
			  						  	    &iter_array);

		STP_DBUS_DEBUG(("RSTP>> into ....... *****ret %d\n",ret));
		dbus_message_iter_open_container (&iter_array,
								   DBUS_TYPE_STRUCT,
								   NULL,
								   &iter_struct);
		/*return root bridge info*/
		for(j = 0; j < 6; j ++)
		{
			dbus_message_iter_append_basic
					(&iter_struct,
		              DBUS_TYPE_BYTE,
					  &(stpm->rootPrio.root_bridge.addr[j]));
		}

		dbus_message_iter_append_basic
					(&iter_struct,
		              DBUS_TYPE_UINT16,
					  &(stpm->rootPrio.root_bridge.prio));
		
		dbus_message_iter_append_basic
					(&iter_struct,
		              DBUS_TYPE_UINT32,
					  &(stpm->rootPrio.root_path_cost));

		dbus_message_iter_append_basic
					(&iter_struct,
		              DBUS_TYPE_UINT16,
					  &(stpm->rootPrio.bridge_port));

		dbus_message_iter_append_basic
					(&iter_struct,
		              DBUS_TYPE_UINT16,
					  &(stpm->rootTimes.MaxAge));

		dbus_message_iter_append_basic
					(&iter_struct,
		              DBUS_TYPE_UINT16,
					  &(stpm->rootTimes.HelloTime));
		
		dbus_message_iter_append_basic
					(&iter_struct,
		              DBUS_TYPE_UINT16,
					  &(stpm->rootTimes.ForwardDelay));


		/*return self-bridge info*/		
		for(j = 0; j < 6; j++)
		{
			dbus_message_iter_append_basic
					(&iter_struct,
		              DBUS_TYPE_BYTE,
					  &(stpm->BrId.addr[j]));
		}

		dbus_message_iter_append_basic
					(&iter_struct,
		              DBUS_TYPE_UINT16,
					  &(stpm->BrId.prio));
		
		dbus_message_iter_append_basic
					(&iter_struct,
		              DBUS_TYPE_UINT32,
					  &(stpm->ForceVersion));
		STP_DBUS_DEBUG(("forceversion %d\n",stpm->ForceVersion));

		dbus_message_iter_append_basic
					(&iter_struct,
		              DBUS_TYPE_UINT16,
					  &(stpm->BrTimes.MaxAge));

		dbus_message_iter_append_basic
					(&iter_struct,
		              DBUS_TYPE_UINT16,
					  &(stpm->BrTimes.HelloTime));
		
		dbus_message_iter_append_basic
					(&iter_struct,
		              DBUS_TYPE_UINT16,
					  &(stpm->BrTimes.ForwardDelay));
		
		dbus_message_iter_close_container (&iter_array,&iter_struct);

		dbus_message_iter_close_container (&iter,&iter_array);
	}
	else
	{
		ret = STP_NOT_ENABLED;
		dbus_message_iter_append_basic (&iter,
										 DBUS_TYPE_UINT32,
 										 &ret);
	}		
	return reply;

}


/*
  * cmd used to test this method
  * 
  * dbus-send --system --dest=aw.stp --type=method_call --print-reply=literal /aw/stp aw.stp.showspanntree
  *
  * arg lists for method STP_DBUS_METHOD_BRG_SHOWSPANTREE
  * in arg list:
  *  	NONE
  *	
  * out arg list:  // in the order as they are appended in the dbus message. 
  *
  *
  */
DBusMessage * stp_dbus_show_spanning_tree_port_admin_state(DBusConnection *conn, DBusMessage *msg, void *user_data) {
	
	DBusMessage* reply;
	DBusError err;

	DBusMessageIter iter;
	DBusMessageIter iter_sub_sub_struct;
	DBusMessageIter iter_sub_struct;
	int i,ret = 0;
	Bool res = False;
	unsigned int port_index = 0;

	STPM_T* stpm = NULL;
	PORT_T* port = NULL;
    unsigned int enable = 0;
    struct stp_admin_infos *stp_info = NULL;



	dbus_error_init( &err );

	if(!(dbus_message_get_args(msg,&err,
								DBUS_TYPE_UINT32,&port_index,
								DBUS_TYPE_INVALID)))
		{
			if(dbus_error_is_set( &err ))
			{
				STP_DBUS_ERR(("STP>>%s raised:%s\n",err.name ,err.message));
				dbus_error_free( &err );
			}
		return NULL;	
	}
	
	reply = dbus_message_new_method_return(msg);	
	dbus_message_iter_init_append (reply, &iter);
	if(stp_param_check_stp_state())
	{
		stpm = stp_stpm_get_the_cist(); 
		if(NULL != stpm){
			for(port = stpm->ports; port; port = port->next){	
				if(port_index == port->port_index){
					if(NULL != stp_ports){
					  if(NULL != stp_ports[port_index]){
						 stp_info = stp_ports[port_index];
						 enable = stp_info->stpEnable;
					  }
				    }
			    }
		    }
		}
		dbus_message_iter_append_basic (&iter,
										 DBUS_TYPE_UINT32,
										 &ret);
		dbus_message_iter_append_basic (&iter,
										 DBUS_TYPE_UINT32,
										 &enable);		
	}
	else
	{
		ret = STP_NOT_ENABLED;
		dbus_message_iter_append_basic (&iter,
										 DBUS_TYPE_UINT32,
										 &ret);
	}	
	return reply;	
}


/*
  * cmd used to test this method
  * 
  * dbus-send --system --dest=aw.stp --type=method_call --print-reply=literal /aw/stp aw.stp.showspanntree
  *
  * arg lists for method STP_DBUS_METHOD_BRG_SHOWSPANTREE
  * in arg list:
  *  	NONE
  *	
  * out arg list:  // in the order as they are appended in the dbus message. 
  *
  *
  */
DBusMessage * stp_dbus_show_spanning_tree_port_info(DBusConnection *conn, DBusMessage *msg, void *user_data) {
	
	DBusMessage* reply;
	DBusError err;

	DBusMessageIter iter;
	DBusMessageIter iter_array;
	DBusMessageIter iter_sub_sub_struct;
	DBusMessageIter iter_sub_struct;
	int i,ret = 0;
	Bool res = False;
	unsigned int port_index = 0;

	STPM_T* stpm = NULL;
	PORT_T* port = NULL;

	unsigned char  port_prio = 128;
	unsigned int	 port_path_cost = 200000000;
	int 				port_role = DisabledPort;
	Bool				 port_adminEnable = False;
	Bool				port_p2p =	P2P_AUTO_E;
	Bool				 port_edge = True;
	int 					port_state = 0,state = 0;
	unsigned char mac[6] ={'\0','\0','\0','\0','\0','\0'};
	unsigned int	br_32_def = 0;
	unsigned short br_16_def = 0;



	dbus_error_init( &err );

	if(!(dbus_message_get_args(msg,&err,
								DBUS_TYPE_UINT32,&port_index,
								DBUS_TYPE_INVALID)))
		{
			if(dbus_error_is_set( &err ))
			{
				STP_DBUS_ERR(("STP>>%s raised:%s\n",err.name ,err.message));
				dbus_error_free( &err );
			}
		return NULL;	
	}
	
	reply = dbus_message_new_method_return(msg);	
	dbus_message_iter_init_append (reply, &iter);
	if(stp_param_check_stp_state())
	{

		printf("RSTP>> into ......\n");
		// Total slot count
		/*		
		Array of Port Infos.
				port no
				port prio
				port role
				port State
				port link
				port p2p
				port edge
				port Desi bridge
				port Dcost
				port D-port
		*/
		dbus_message_iter_append_basic (&iter,
										 DBUS_TYPE_UINT32,
										 &ret);
		dbus_message_iter_open_container (&iter,
													DBUS_TYPE_ARRAY,
													DBUS_STRUCT_BEGIN_CHAR_AS_STRING
													DBUS_TYPE_BYTE_AS_STRING 
													DBUS_TYPE_UINT32_AS_STRING
													DBUS_TYPE_INT32_AS_STRING
													DBUS_TYPE_INT32_AS_STRING
													DBUS_TYPE_INT32_AS_STRING
													DBUS_TYPE_INT32_AS_STRING
													DBUS_TYPE_INT32_AS_STRING
													DBUS_TYPE_UINT32_AS_STRING
													DBUS_TYPE_UINT16_AS_STRING
													DBUS_STRUCT_BEGIN_CHAR_AS_STRING
													DBUS_TYPE_UINT16_AS_STRING 
													DBUS_TYPE_BYTE_AS_STRING
													DBUS_TYPE_BYTE_AS_STRING
													DBUS_TYPE_BYTE_AS_STRING
													DBUS_TYPE_BYTE_AS_STRING
													DBUS_TYPE_BYTE_AS_STRING
													DBUS_TYPE_BYTE_AS_STRING
													DBUS_STRUCT_END_CHAR_AS_STRING
													DBUS_STRUCT_END_CHAR_AS_STRING,
													&iter_array);
		dbus_message_iter_open_container (&iter_array,
								   DBUS_TYPE_STRUCT,
								   NULL,
								   &iter_sub_struct);

		stpm = stp_stpm_get_the_cist(); 
		if(NULL != stpm){
			for(port = stpm->ports; port; port = port->next)
			{	
				if(port_index == port->port_index)
				{
				   if((NonStpPort == port->role )&&(0 == port->adminEnable)){
					   res = False;
					   break;
					}
				   else if ((port_index == port->port_index)&&(DisabledPort != port->role)){
					   res = True;
					   break;
					}
				}
			}
		}
	  
		if(res)
		{
			if (DisabledPort == port->role) {
				stp_syslog_dbg("port %d/%d role dis\n", port->slot_no, port->port_no);
				state = UID_PORT_DISABLED;
			} else if (! port->forward && ! port->learn) {
				stp_syslog_dbg("port %d/%d role disca\n", port->slot_no, port->port_no);
				state = UID_PORT_DISCARDING;
			} else if (! port->forward && port->learn) {
				stp_syslog_dbg("port %d/%d role learn\n", port->slot_no, port->port_no);
				state = UID_PORT_LEARNING;
			} else {
				stp_syslog_dbg("port %d/%d role forwoard\n", port->slot_no, port->port_no);
				state = UID_PORT_FORWARDING;
			}

			port_prio = (unsigned char)((port->port_id&0xff00)>>8);
			dbus_message_iter_append_basic
						(&iter_sub_struct,
						  DBUS_TYPE_BYTE,
						  &(port_prio));

			dbus_message_iter_append_basic
						(&iter_sub_struct,
						  DBUS_TYPE_UINT32,
						 /* &(port->adminPCost)*/&(port->operPCost));
			dbus_message_iter_append_basic
						(&iter_sub_struct,
						  DBUS_TYPE_INT32,
						  &(port->role));
			dbus_message_iter_append_basic
						(&iter_sub_struct,
						  DBUS_TYPE_INT32,
						  &(state));
			dbus_message_iter_append_basic
						(&iter_sub_struct,
						  DBUS_TYPE_INT32,
						  &(port->adminEnable));
			dbus_message_iter_append_basic
						(&iter_sub_struct,
						  DBUS_TYPE_INT32,
						  &(port->operPointToPointMac));
			dbus_message_iter_append_basic
						(&iter_sub_struct,
						  DBUS_TYPE_INT32,
						  &(port->operEdge));

			dbus_message_iter_append_basic
						(&iter_sub_struct,
						  DBUS_TYPE_UINT32,
						  &(port->portPrio.root_path_cost));

			dbus_message_iter_append_basic
						(&iter_sub_struct,
						  DBUS_TYPE_UINT16,
						  &(port->portPrio.design_port));


			dbus_message_iter_open_container (&iter_sub_struct,
											   DBUS_TYPE_STRUCT,
												NULL,
											   &iter_sub_sub_struct);

			dbus_message_iter_append_basic
						(&iter_sub_sub_struct,
						  DBUS_TYPE_UINT16,
						  &(port->portPrio.design_bridge.prio));


			for(i= 0; i < 6; i++)
			{
				dbus_message_iter_append_basic
						(&iter_sub_sub_struct,
						  DBUS_TYPE_BYTE,
						  &(port->portPrio.design_bridge.addr[i]));
			}
		}
		else
		{

			dbus_message_iter_append_basic
						(&iter_sub_struct,
						  DBUS_TYPE_BYTE,
						  &(port_prio));

			dbus_message_iter_append_basic
						(&iter_sub_struct,
						  DBUS_TYPE_UINT32,
						  &(port_path_cost));
			dbus_message_iter_append_basic
						(&iter_sub_struct,
						  DBUS_TYPE_INT32,
						  &(port_role));
			dbus_message_iter_append_basic
						(&iter_sub_struct,
						  DBUS_TYPE_INT32,
						  &(port_state));
			dbus_message_iter_append_basic
						(&iter_sub_struct,
						  DBUS_TYPE_INT32,
						  &(port_adminEnable));
			dbus_message_iter_append_basic
						(&iter_sub_struct,
						  DBUS_TYPE_INT32,
						  &(port_p2p));
			dbus_message_iter_append_basic
						(&iter_sub_struct,
						  DBUS_TYPE_INT32,
						  &(port_edge));

			dbus_message_iter_append_basic
						(&iter_sub_struct,
						  DBUS_TYPE_UINT32,
						  &(br_32_def));
			
			dbus_message_iter_append_basic
						(&iter_sub_struct,
						  DBUS_TYPE_UINT16,
						  &(br_16_def));


			dbus_message_iter_open_container (&iter_sub_struct,
											   DBUS_TYPE_STRUCT,
												NULL,
											   &iter_sub_sub_struct);

			dbus_message_iter_append_basic
						(&iter_sub_sub_struct,
						  DBUS_TYPE_UINT16,
						  &(br_16_def));


			for(i= 0; i < 6; i++)
			{
				dbus_message_iter_append_basic
						(&iter_sub_sub_struct,
						  DBUS_TYPE_BYTE,
						  &(mac[i]));
			}

			
		}	

		dbus_message_iter_close_container(&iter_sub_struct,&iter_sub_sub_struct);

		/*dbus_message_iter_close_container (&iter_struct_array, );*/

		dbus_message_iter_close_container (&iter_array,&iter_sub_struct);
		dbus_message_iter_close_container (&iter,&iter_array);
	}
	else
	{
		ret = STP_NOT_ENABLED;
		dbus_message_iter_append_basic (&iter,
										 DBUS_TYPE_UINT32,
										 &ret);
	}

	
	return reply;	
}


/*
  * cmd used to test this method
  * 
  * dbus-send --system --dest=aw.stp --type=method_call --print-reply=literal /aw/stp aw.stp.showspanntree
  *
  * arg lists for method STP_DBUS_METHOD_CHECK_AND_SAVE_PORT_INFO_CONFIG
  * in arg list:
  *  	NONE
  *	
  * out arg list:  // in the order as they are appended in the dbus message. 
  *
  *
  */
DBusMessage * stp_dbus_save_port_stp_config(DBusConnection *conn, DBusMessage *msg, void *user_data) {
	
	DBusMessage* reply;
	DBusError err;

	DBusMessageIter iter;
	DBusMessageIter iter_sub_sub_struct;
	DBusMessageIter iter_sub_struct;
	int i,ret = 0;
	Bool res = False;
	unsigned int slot_no = 0,port_no = 0,port_index = 0;
	unsigned char *showStr = NULL,tmpPtr = NULL;

	STPM_T* stpm = NULL;
	UID_STP_PORT_CFG_T  cfg = {0};
	unsigned length = 0;
	unsigned short mstid = 0;
    PORT_T* port = NULL, *cist_port = NULL;
	unsigned int flag= 0,enable = 1;

    showStr = (unsigned char*)malloc(STP_RUNNING_PORT_CFG_MEM);
	if(NULL == showStr){
		printf("Memory malloc for port config failed!\n");
		return NULL;

	}
    tmpPtr = showStr;
	dbus_error_init( &err );

	if(!(dbus_message_get_args(msg,&err,
		                        DBUS_TYPE_UINT32,&slot_no,
		                        DBUS_TYPE_UINT32,&port_no,
								DBUS_TYPE_UINT32,&port_index,
								DBUS_TYPE_INVALID)))
		{
			if(dbus_error_is_set( &err ))
			{
				STP_DBUS_ERR(("STP>>%s raised:%s\n",err.name ,err.message));
				dbus_error_free( &err );
			}
		free(showStr);
		showStr = NULL;
		return NULL;	
	}


	reply = dbus_message_new_method_return(msg);	
	dbus_message_iter_init_append (reply, &iter);
	if(stp_param_check_stp_state())
	{

		stpm = stp_stpm_get_the_cist();
	    if(stpm) {
	         port =cist_port = stp_port_mst_findport(STP_CIST_ID, port_index);
	 
	         if (! port) {
			 	ret = STP_PORT_NOT_EXIST;
				dbus_message_iter_append_basic (&iter,
											 DBUS_TYPE_UINT32,
											 &ret);
				return reply;
	         }
  
             for( ; port; port = port->nextMst){
				    if (port->portEnabled == enable) {
						flag = 1;
						break;
				    }
             }
			 if(0==flag){
                ret = STP_PORT_NOT_ENABLE;
				dbus_message_iter_append_basic (&iter,
											 DBUS_TYPE_UINT32,
											 &ret);
				return reply;
			 }
			
            stp_to_get_init_port_cfg (stpm->vlan_id,port_index,&cfg);
		    port =cist_port = stp_port_mst_findport(STP_CIST_ID, port_index);
		    printf("start>>\n");
		
		    length += sprintf(tmpPtr,"config spanning-tree eth-port %d/%d enable\n",slot_no,port_no);
		    tmpPtr = showStr + length;
		
			
		    if(port->port_id != (((cfg.port_priority & 0xf0)<< 8) + port->port_index)){
			   length += sprintf(tmpPtr,"config spanning-tree eth-port %d/%d priority %d\n",slot_no,port_no,(port->port_id&0xff00)>>8);
			   tmpPtr = showStr + length;
		    }

			if(port->adminPCost != cfg.admin_port_path_cost) {
				length += sprintf(tmpPtr,"config spanning-tree eth-port %d/%d path-cost %d\n",slot_no,port_no,port->adminPCost);
				tmpPtr = showStr + length;
			}

			if(port->admin_non_stp != cfg.admin_non_stp) {
				length += sprintf(tmpPtr,"config spanning-tree eth-port %d/%d non-stp %s\n", slot_no,port_no,( 1 == port->admin_non_stp)?"yes":"no");
				tmpPtr = showStr + length;
			}

			if(port->adminPointToPointMac != cfg.admin_point2point) {
			
				length += sprintf(tmpPtr,"config spanning-tree eth-port %d/%d p2p %s\n",slot_no,port_no,port->adminPointToPointMac);
				tmpPtr = showStr + length;
			}

			if(port->adminEdge != cfg.admin_edge) {
				length += sprintf(tmpPtr,"config spanning-tree eth-port %d/%d edge %d\n", slot_no,port_no,port->adminEdge);
				tmpPtr = showStr + length;
			}
		}
	else
	{
		ret = STP_NOT_ENABLED;
		dbus_message_iter_append_basic (&iter,
										 DBUS_TYPE_UINT32,
										 &ret);
	}

	
	return reply;	
}

}

/*
  * cmd used to test this method
  * 
  * dbus-send --system --dest=aw.stp --type=method_call --print-reply=literal /aw/stp aw.stp.debug
  *
  * arg lists for method STP_DBUS_METHOD_BRG_DBUG_SPANNTREE
  * in arg list:
  *  	NONE
  *	
  * out arg list:  // in the order as they are appended in the dbus message. 
  *
  *
  */
DBusMessage * stp_dbus_set_debug_value(DBusConnection *conn, DBusMessage *msg, void *user_data){
	DBusMessage* reply;
	DBusError err;
	unsigned int ret =STP_OK ;
	unsigned int flag = 0;
	char *ident = "STP";
	
	dbus_error_init( &err );
	if(!(dbus_message_get_args(msg,&err,
								DBUS_TYPE_UINT32,&flag,
								DBUS_TYPE_INVALID)))
	{
		STP_DBUS_ERR(("STP>>Unable to get input args\n"));
		if(dbus_error_is_set( &err ))
		{
			STP_DBUS_ERR(("STP>>%s raised:%s\n",err.name ,err.message));
			dbus_error_free( &err );
		}
		return NULL;	
	}
	/* here call rstp api */
	if( !(stp_param_set_debug_value(flag))){
		openlog(ident, 0, LOG_DAEMON); 
		stp_log_close = 0;
		ret = STP_ERR;
	} 
	STP_DBUS_DEBUG(("STP>>Entering set stp debug!\n"));
	reply = dbus_message_new_method_return(msg);
    dbus_message_append_args(reply,
							 DBUS_TYPE_UINT32,&ret,
							 DBUS_TYPE_INVALID);
	
	return reply;
}

/*
  * cmd used to test this method
  * 
  * dbus-send --system --dest=aw.stp --type=method_call --print-reply=literal /aw/stp aw.stp.nodebug
  *
  * arg lists for method STP_DBUS_METHOD_BRG_NO_DBUG_SPANNTREE
  * in arg list:
  *  	NONE
  *	
  * out arg list:  // in the order as they are appended in the dbus message. 
  *
  *
  */
DBusMessage * stp_dbus_set_no_debug_value(DBusConnection *conn, DBusMessage *msg, void *user_data){
	DBusMessage* reply;
	DBusError err;
	unsigned int ret =STP_OK ;
	unsigned int flag;
	dbus_error_init( &err );
	if(!(dbus_message_get_args(msg,&err,
								DBUS_TYPE_UINT32,&flag,
								DBUS_TYPE_INVALID)))
	{
		STP_DBUS_ERR(("STP>>Unable to get input args\n"));
		if(dbus_error_is_set( &err ))
		{
			STP_DBUS_ERR(("STP>>%s raised:%s\n",err.name ,err.message));
			dbus_error_free( &err );
		}
		return NULL;	
	}
	/* here call rstp api */
	if( !(stp_param_set_no_debug_value(flag))){
		stp_log_close = 1;
		closelog();
		ret = STP_ERR;
	}

	STP_DBUS_DEBUG(("STP>>Entering no debug info!\n"));
	reply = dbus_message_new_method_return(msg);
    dbus_message_append_args(reply,
							 DBUS_TYPE_UINT32,&ret,
							 
							 DBUS_TYPE_INVALID);
	
	return reply;

}


DBusMessage * stp_dbus_mstp_set_valn_on_mst(DBusConnection *conn, DBusMessage *msg, void *user_data){

	DBusMessage* reply;
	DBusError err;
	unsigned int  mstid = 0;
	unsigned short vid = 0;
	unsigned int ret=STP_OK;
	STP_DBUS_DEBUG(("STP>>Entering set vlan on mst!\n"));
	
	dbus_error_init( &err );
	if(!(dbus_message_get_args(msg,&err,
								DBUS_TYPE_UINT16,&vid,
								DBUS_TYPE_UINT32,&mstid,
								DBUS_TYPE_INVALID)))
		{
			if(dbus_error_is_set( &err ))
			{
			STP_DBUS_ERR(("STP>>%s raised:%s\n",err.name ,err.message));
			dbus_error_free( &err );
		}
		return NULL;	
	}

	STP_DBUS_DEBUG(("STP>>config port default-value null \n"));

	/* here call rstp api */
	if(stp_param_check_stp_state()) 
		ret =  stp_param_set_vlan_to_instance(vid,mstid);
	else
		ret = STP_NOT_ENABLED;
	

	reply = dbus_message_new_method_return(msg);
	dbus_message_append_args(reply,
								 DBUS_TYPE_UINT32,&ret,
								 DBUS_TYPE_INVALID);
		
		
	return reply;

}

DBusMessage *stp_dbus_mstp_show_cist_info(DBusConnection *conn, DBusMessage *msg, void *user_data){

	DBusMessage* reply;
	DBusError err;
	DBusMessageIter iter,iter_struct;
	STPM_T* cist_stpm = stp_stpm_get_the_cist();
	int j,ret = 0;

	STP_DBUS_DEBUG(("STP>>Enteringset show cist info!\n"));

	dbus_error_init( &err );

	reply = dbus_message_new_method_return(msg);	
	dbus_message_iter_init_append (reply, &iter);
	if(stp_param_check_stp_state())
	{
		if(cist_stpm) {
			
			dbus_message_iter_append_basic (&iter,
											 DBUS_TYPE_UINT32,
											 &ret);

			dbus_message_iter_open_container (&iter,
											   DBUS_TYPE_STRUCT,
											   DBUS_STRUCT_BEGIN_CHAR_AS_STRING 
										    		DBUS_TYPE_BYTE_AS_STRING
										    		DBUS_TYPE_BYTE_AS_STRING
										    		DBUS_TYPE_BYTE_AS_STRING
										    		DBUS_TYPE_BYTE_AS_STRING
										    		DBUS_TYPE_BYTE_AS_STRING
										    		DBUS_TYPE_BYTE_AS_STRING
													DBUS_TYPE_UINT16_AS_STRING
													DBUS_TYPE_UINT32_AS_STRING
													DBUS_TYPE_UINT16_AS_STRING
											DBUS_STRUCT_END_CHAR_AS_STRING,
				  						   &iter_struct);

			/*RSTP root*/
			for(j = 0; j < 6; j++)
			{
				dbus_message_iter_append_basic
						(&iter_struct,
			              DBUS_TYPE_BYTE,
						  &(cist_stpm->rootPrio.root_bridge.addr[j]));
			}

			dbus_message_iter_append_basic
						(&iter_struct,
			              DBUS_TYPE_UINT16,
						  &(cist_stpm->rootPrio.root_bridge.prio));
			
			dbus_message_iter_append_basic
						(&iter_struct,
			              DBUS_TYPE_UINT32,
						  &(cist_stpm->rootPrio.root_path_cost));

			dbus_message_iter_append_basic
						(&iter_struct,
			              DBUS_TYPE_UINT16,
						  &(cist_stpm->rootPortId));
				
			dbus_message_iter_close_container (&iter,&iter_struct);

			
		}
		else
		{
			printf("stp_dbus 1554 :: no cist\n");
			ret =  STP_DBUS_NO_SUCH_INST;
			dbus_message_iter_append_basic (&iter,
											 DBUS_TYPE_UINT32,
											 &ret);
		}
	}
	else
	{
		ret =  STP_NOT_ENABLED;
		dbus_message_iter_append_basic (&iter,
										 DBUS_TYPE_UINT32,
										 &ret);
	}

	return reply;
}


DBusMessage *stp_dbus_mstp_show_msti_info(DBusConnection *conn, DBusMessage *msg, void *user_data){

	DBusMessage* reply;
	DBusError err;
	DBusMessageIter iter,iter_struct;
	unsigned int mstid = 0;
	STPM_T* this = NULL,*cist_stpm = stp_stpm_get_the_cist();
	int j,ret =0;

	STP_DBUS_DEBUG(("STP>>Enteringset show msti info!\n"));

	dbus_error_init( &err );
	if(!(dbus_message_get_args(msg,&err,
								DBUS_TYPE_UINT32,&mstid,
								DBUS_TYPE_INVALID)))
		{
			if(dbus_error_is_set( &err ))
			{
				STP_DBUS_ERR(("STP>>%s raised:%s\n",err.name ,err.message));
				dbus_error_free( &err );
			}
		return NULL;	
	}
	
	reply = dbus_message_new_method_return(msg);	
	dbus_message_iter_init_append (reply, &iter);
	if(stp_param_check_stp_state())
	{
		this = stp_stpm_get_instance(mstid);
		if(!this || !cist_stpm) {
			printf("stp_dbus 1593 :: no msti\n");
			ret = STP_DBUS_NO_SUCH_INST;
			dbus_message_iter_append_basic (&iter,
											 DBUS_TYPE_UINT32,
											 &ret);
		}
		else
		{
			printf("stp_dbus 1601 :: into get msti\n");
			dbus_message_iter_append_basic (&iter,
											 DBUS_TYPE_UINT32,
											 &ret);
			dbus_message_iter_open_container (&iter,
											   DBUS_TYPE_STRUCT,
											   DBUS_STRUCT_BEGIN_CHAR_AS_STRING 
										    		DBUS_TYPE_BYTE_AS_STRING
										    		DBUS_TYPE_BYTE_AS_STRING
										    		DBUS_TYPE_BYTE_AS_STRING
										    		DBUS_TYPE_BYTE_AS_STRING
										    		DBUS_TYPE_BYTE_AS_STRING
										    		DBUS_TYPE_BYTE_AS_STRING
													DBUS_TYPE_UINT16_AS_STRING
													DBUS_TYPE_UINT32_AS_STRING
													DBUS_TYPE_UINT16_AS_STRING
													DBUS_TYPE_UINT16_AS_STRING
													DBUS_TYPE_UINT16_AS_STRING
													DBUS_TYPE_UINT16_AS_STRING
													DBUS_TYPE_BYTE_AS_STRING
											DBUS_STRUCT_END_CHAR_AS_STRING,
				  						   &iter_struct);


		/*return mstp root bridge info*/
			for(j = 0; j < 6; j++)
			{
				dbus_message_iter_append_basic
						(&iter_struct,
			              DBUS_TYPE_BYTE,
						  &(this->rootPrio.region_root_bridge.addr[j]));
			}

			dbus_message_iter_append_basic
						(&iter_struct,
			              DBUS_TYPE_UINT16,
						  &(this->rootPrio.region_root_bridge.prio));
			
			dbus_message_iter_append_basic
						(&iter_struct,
			              DBUS_TYPE_UINT32,
						  &(this->rootPrio.region_root_path_cost));

			dbus_message_iter_append_basic
						(&iter_struct,
			              DBUS_TYPE_UINT16,
						  &(this->rootPortId));

			dbus_message_iter_append_basic
						(&iter_struct,
			              DBUS_TYPE_UINT16,
						  &(this->rootTimes.MaxAge));

			dbus_message_iter_append_basic
						(&iter_struct,
			              DBUS_TYPE_UINT16,
						  &(this->rootTimes.HelloTime));
			
			dbus_message_iter_append_basic
						(&iter_struct,
			              DBUS_TYPE_UINT16,
						  &(this->rootTimes.ForwardDelay));

			dbus_message_iter_append_basic
						(&iter_struct,
			              DBUS_TYPE_BYTE,
						  &(this->rootTimes.RemainingHops));

				
			dbus_message_iter_close_container (&iter,&iter_struct);

		}
	}
	else
	{
		ret =  STP_NOT_ENABLED;
		dbus_message_iter_append_basic (&iter,
											 DBUS_TYPE_UINT32,
											 &ret);
	}
	return reply;
}

DBusMessage *stp_dbus_mstp_show_self_info(DBusConnection *conn, DBusMessage *msg, void *user_data){

	DBusMessage* reply;
	DBusError err;
	DBusMessageIter iter,iter_struct,iter_sub_array,iter_sub_struct;
	unsigned int mstid = 0;
	STPM_T* this = NULL,*cist_stpm = stp_stpm_get_the_cist();
	char* pname = NULL;
	unsigned short revision,vid[4095] = {0};
	
	int i,j,cnt =0,ret = 0;

	STP_DBUS_DEBUG(("STP>>Enteringset show self info!\n"));
	dbus_error_init( &err );
	if(!(dbus_message_get_args(msg,&err,
								DBUS_TYPE_UINT32,&mstid,
								DBUS_TYPE_INVALID)))
		{
			if(dbus_error_is_set( &err ))
			{
				STP_DBUS_ERR(("STP>>%s raised:%s\n",err.name ,err.message));
				dbus_error_free( &err );
			}
		return NULL;	
	}

	reply = dbus_message_new_method_return(msg);	
	dbus_message_iter_init_append (reply, &iter);
	printf("STP>> show self info>> mstid: %u\n", mstid);
	if(stp_param_check_stp_state())
	{
		this = stp_stpm_get_instance(mstid);
		if(!this || !cist_stpm) 
		{
			printf("stp_dbus1706::MSTP>> no the instance \n");
			ret = STP_DBUS_NO_SUCH_INST;
			dbus_message_iter_append_basic (&iter,
											 DBUS_TYPE_UINT32,
											 &ret);
		}
		else
		{
			pname = cist_stpm->MstConfigId.ConfigurationName;
			revision = *((short *)cist_stpm->MstConfigId.RevisionLevel);
			stp_param_get_mstp_vlan_map_info(mstid,vid,&cnt);
			printf("stp_dbus1804:: vid[5] %d, count %d\n ",vid[5],cnt);

			printf("stp_dbus1721::MSTP>> get self info vid num %d\n",cnt);
			dbus_message_iter_append_basic (&iter,
											 DBUS_TYPE_UINT32,
											 &ret);
			dbus_message_iter_open_container (&iter,
											   DBUS_TYPE_STRUCT,
											   DBUS_STRUCT_BEGIN_CHAR_AS_STRING 
											   		DBUS_TYPE_STRING_AS_STRING
											   		DBUS_TYPE_UINT16_AS_STRING
											   		DBUS_TYPE_UINT32_AS_STRING
											   		DBUS_TYPE_ARRAY_AS_STRING
											   		DBUS_STRUCT_BEGIN_CHAR_AS_STRING
											   			DBUS_TYPE_UINT16_AS_STRING
											   		DBUS_STRUCT_END_CHAR_AS_STRING
													DBUS_TYPE_BYTE_AS_STRING
										    		DBUS_TYPE_BYTE_AS_STRING
										    		DBUS_TYPE_BYTE_AS_STRING
										    		DBUS_TYPE_BYTE_AS_STRING
										    		DBUS_TYPE_BYTE_AS_STRING
										    		DBUS_TYPE_BYTE_AS_STRING
													DBUS_TYPE_UINT16_AS_STRING
													DBUS_TYPE_UINT32_AS_STRING
													DBUS_TYPE_UINT16_AS_STRING
													DBUS_TYPE_UINT16_AS_STRING
													DBUS_TYPE_UINT16_AS_STRING
													DBUS_TYPE_BYTE_AS_STRING
											DBUS_STRUCT_END_CHAR_AS_STRING,
				  						   &iter_struct);


		/*return self info*/

			dbus_message_iter_append_basic
						(&iter_struct,
			              DBUS_TYPE_STRING,
						  &pname);
			dbus_message_iter_append_basic
						(&iter_struct,
			              DBUS_TYPE_UINT16,
						  &revision);
			
			dbus_message_iter_append_basic
						(&iter_struct,
			              DBUS_TYPE_UINT32,
						  &cnt);

			dbus_message_iter_open_container (&iter_struct,
												   DBUS_TYPE_ARRAY,
												   DBUS_STRUCT_BEGIN_CHAR_AS_STRING
														   DBUS_TYPE_UINT16_AS_STRING
													DBUS_STRUCT_END_CHAR_AS_STRING,
												   &iter_sub_array);
			for(j = 0; j < cnt; j++) 
			{
				dbus_message_iter_open_container
							(&iter_sub_array,
							 DBUS_TYPE_STRUCT,
							NULL,
							&iter_sub_struct);
				dbus_message_iter_append_basic
							(&iter_sub_struct,
							 DBUS_TYPE_UINT16,
							 &(vid[j]));
								
				dbus_message_iter_close_container (&iter_sub_array, &iter_sub_struct);

			}
			dbus_message_iter_close_container (&iter_struct, &iter_sub_array);
			
			for(j = 0; j < 6; j++)
			{
				dbus_message_iter_append_basic
						(&iter_struct,
			              DBUS_TYPE_BYTE,
						  &(this->BrId.addr[j]));
			}

			dbus_message_iter_append_basic
						(&iter_struct,
			              DBUS_TYPE_UINT16,
						  &(this->BrId.prio));
			
			dbus_message_iter_append_basic
						(&iter_struct,
			              DBUS_TYPE_UINT32,
						  &(this->ForceVersion));

			dbus_message_iter_append_basic
						(&iter_struct,
			              DBUS_TYPE_UINT16,
						  &(this->BrTimes.MaxAge));

			dbus_message_iter_append_basic
						(&iter_struct,
			              DBUS_TYPE_UINT16,
						  &(this->BrTimes.HelloTime));
			
			dbus_message_iter_append_basic
						(&iter_struct,
			              DBUS_TYPE_UINT16,
						  &(this->BrTimes.ForwardDelay));
			dbus_message_iter_append_basic
						(&iter_struct,
			              DBUS_TYPE_UINT16,
						  &(this->BrTimes.RemainingHops));

			dbus_message_iter_close_container (&iter,&iter_struct);
		}
	}
	else
	{
		ret =  STP_NOT_ENABLED;
		dbus_message_iter_append_basic (&iter,
										 DBUS_TYPE_UINT32,
										 &ret);
	}

	return reply;


}


DBusMessage * stp_dbus_mstp_get_vid_by_mstid(DBusConnection *conn, DBusMessage *msg, void *user_data) {
		
	DBusMessage* reply;
	DBusError err;
	unsigned int ret = STP_OK,mstid = 0;
	unsigned int  port, port_prio;
	
	STP_DBUS_DEBUG(("STP>>Entering inspect port-priority!\n"));
	dbus_error_init( &err );
	if(!(dbus_message_get_args(msg,&err,
								DBUS_TYPE_UINT32, &mstid, \
								DBUS_TYPE_INVALID)))
	{
		STP_DBUS_ERR(("STP>>Unable to get input args\n"));
		if(dbus_error_is_set( &err ))
		{
			STP_DBUS_ERR(("STP>>%s raised:%s\n",err.name ,err.message));
			dbus_error_free( &err );
		}
		ret = STP_ERR;	
	}

	STP_DBUS_DEBUG(("STP>>config mstid %d , port port-priority %d\n",mstid,port_prio));
	/* here call rstp api */
	//ret  = stp_param_set_port_priority(mstid,port, port_prio);
	
	reply = dbus_message_new_method_return(msg);
	dbus_message_append_args(reply,
								 DBUS_TYPE_UINT32,&port,
								 DBUS_TYPE_UINT32,&port_prio,
								 DBUS_TYPE_UINT32,&ret,
								 DBUS_TYPE_INVALID);
		
		
	return reply;
}


DBusMessage *stp_dbus_mstp_show_instance_port_info(DBusConnection *conn, DBusMessage *msg, void *user_data){

	DBusMessage* reply;
	DBusError err;

	DBusMessageIter iter;
	DBusMessageIter iter_sub_sub_struct;
	DBusMessageIter iter_sub_struct;
	int i,ret = 0;
	unsigned int port_index,mstid = 0;
	UID_STP_PORT_STATE_T portInfo = {{0}};
	unsigned char port_prio;
	
	dbus_error_init( &err );
	if(!(dbus_message_get_args(msg,&err,
								DBUS_TYPE_UINT32,&mstid,
								DBUS_TYPE_UINT32,&port_index,
								DBUS_TYPE_INVALID)))
		{
			if(dbus_error_is_set( &err ))
			{
				STP_DBUS_ERR(("STP>>%s raised:%s\n",err.name ,err.message));
				dbus_error_free( &err );
			}
		return NULL;	
	}

	reply = dbus_message_new_method_return(msg);	
	dbus_message_iter_init_append (reply, &iter);
	if(stp_param_check_stp_state())
	{

		dbus_message_iter_append_basic (&iter,
											 DBUS_TYPE_UINT32,
											 &ret);
		// Total slot count
		/*		
		Array of Port Infos.
				port no
				port prio
				port role
				port State
				port link
				port p2p
				port edge
				port Desi bridge
				port Dcost
				port D-port
		*/
		
		dbus_message_iter_open_container (&iter,
										   DBUS_TYPE_STRUCT,
										   DBUS_STRUCT_BEGIN_CHAR_AS_STRING
												 DBUS_TYPE_BYTE_AS_STRING 
												 DBUS_TYPE_UINT32_AS_STRING
												 DBUS_TYPE_INT32_AS_STRING
												 DBUS_TYPE_INT32_AS_STRING
												 DBUS_TYPE_INT32_AS_STRING
												 DBUS_TYPE_INT32_AS_STRING
												 DBUS_TYPE_INT32_AS_STRING
											   	 DBUS_TYPE_UINT32_AS_STRING
											   	 DBUS_TYPE_UINT16_AS_STRING
											   		DBUS_STRUCT_BEGIN_CHAR_AS_STRING
											    		DBUS_TYPE_UINT16_AS_STRING 
											    		DBUS_TYPE_BYTE_AS_STRING
											    		DBUS_TYPE_BYTE_AS_STRING
											    		DBUS_TYPE_BYTE_AS_STRING
											    		DBUS_TYPE_BYTE_AS_STRING
											    		DBUS_TYPE_BYTE_AS_STRING
											    		DBUS_TYPE_BYTE_AS_STRING
											    	DBUS_STRUCT_END_CHAR_AS_STRING
										DBUS_STRUCT_END_CHAR_AS_STRING,
			  						   &iter_sub_struct);

		stp_param_get_port_state(mstid,port_index,&portInfo);
		/*dbus_message_iter_open_container (&iter_struct_array,
				   DBUS_TYPE_STRUCT,
				   NULL,
				   &iter_sub_struct);*/

		port_prio = (unsigned char)((portInfo.port_id&0xff00)>>8);
		dbus_message_iter_append_basic
					(&iter_sub_struct,
		              DBUS_TYPE_BYTE,
					  &(port_prio));
		if(portInfo.linkState) {
			dbus_message_iter_append_basic
						(&iter_sub_struct,
					  	  DBUS_TYPE_UINT32,
					  	  &(portInfo.oper_port_path_cost));
		}
		else {
			dbus_message_iter_append_basic
							(&iter_sub_struct,
						  	  DBUS_TYPE_UINT32,
						  	  &(portInfo.path_cost));	
		}
		dbus_message_iter_append_basic
					(&iter_sub_struct,
				  	  DBUS_TYPE_INT32,
				  	  &(portInfo.role));
		dbus_message_iter_append_basic
					(&iter_sub_struct,
				  	  DBUS_TYPE_INT32,
				  	  &(portInfo.state));
		dbus_message_iter_append_basic
					(&iter_sub_struct,
				 	  DBUS_TYPE_INT32,
				  	  &(portInfo.linkState));
		dbus_message_iter_append_basic
					(&iter_sub_struct,
				   	  DBUS_TYPE_INT32,
				  	  &(portInfo.oper_point2point));
		dbus_message_iter_append_basic
					(&iter_sub_struct,
				  	  DBUS_TYPE_INT32,
				  	  &(portInfo.oper_edge));

		dbus_message_iter_append_basic
					(&iter_sub_struct,
				  	  DBUS_TYPE_UINT32,
				  	  &(portInfo.designated_cost));

		dbus_message_iter_append_basic
					(&iter_sub_struct,
				   	  DBUS_TYPE_UINT16,
				  	  &(portInfo.designated_port));


		dbus_message_iter_open_container (&iter_sub_struct,
										   DBUS_TYPE_STRUCT,
											NULL,
										   &iter_sub_sub_struct);

		dbus_message_iter_append_basic
					(&iter_sub_sub_struct,
					  DBUS_TYPE_UINT16,
					  &(portInfo.designated_bridge.prio));


		for(i= 0; i < 6; i++)
		{
			dbus_message_iter_append_basic
					(&iter_sub_sub_struct,
					  DBUS_TYPE_BYTE,
					  &(portInfo.designated_bridge.addr[i]));
		}


		dbus_message_iter_close_container(&iter_sub_struct,&iter_sub_sub_struct);

		/*dbus_message_iter_close_container (&iter_struct_array, );*/

		dbus_message_iter_close_container (&iter,&iter_sub_struct);

	}
	else
	{
		ret =  STP_NOT_ENABLED;
		dbus_message_iter_append_basic (&iter,
											 DBUS_TYPE_UINT32,
											 &ret);
	}
	return reply;	
	
}

// Show the current stp configuration to save it
#if 0
DBusMessage* stp_dbus_stp_show_running_cfg(DBusConnection *conn, DBusMessage *msg, void *user_data){


	DBusMessage* reply;
	DBusMessageIter iter;
	//DBusError		err;

	
	STP_DBUS_DEBUG(("STP>> stp dbus message handler: show running configure\n"));
	char* showStr = NULL;
	char tempPtr[BUF_LEN] = "0";
	showStr = tempPtr;
	memset(tempPtr,0,BUF_LEN);
	int ret = 0;

	STP_DBUS_DEBUG(("STP>> stp dbus message handler: show running configure\n"));

	//dbus_error_init(&err);

	/*if( !(dbus_message_get_args(msg, &err,\
					DBUS_TYPE_UINT32, &ret,\
					DUBS_TYPE_STIRNG, &tempPtr, \
					DBUS_TPYE_INVLAID)))
		{
			STP_DBUS_DEBUG("STP >> Unable to get input args\n");
			if(dbus_error_is_set(&err))
				{
					STP_DBUS_DEBUG("STP >> %s, raised :%s\n", err.name, err.message);
				}
			
			return NULL;
		}*/
		
	ret = stp_param_save_running_cfg(showStr,BUF_LEN);

	reply = dbus_message_new_method_return(msg);
	if(NULL == reply)
		{
			STP_DBUS_DEBUG(("STP >> dbus show stp cfg no reply resource error!\n"));
			return reply;
		}
	dbus_message_iter_init_append(reply, &iter);
	dbus_message_iter_append_basic (&iter,
										 DBUS_TYPE_UINT32,
										 &ret);
	dbus_message_iter_append_basic (&iter,
									DBUS_TYPE_STRING,
									&showStr);

	return reply;

}
#endif

#if 1
DBusMessage *stp_dbus_bridge_port_configDigestSnp
(
	DBusConnection *conn,
	DBusMessage *msg,
	void *user_data
)
{
	DBusMessage* reply = NULL;
	DBusError err;
	unsigned int ret = STP_PORT_NOTFOUND;
	unsigned int portindex = 0;
	unsigned int configDigestSnpEnDis = 0;

	int i = 0;
	STPM_T* stpm = NULL;
	PORT_T* port = NULL;
	register STPM_T *this = NULL;
	
	STP_DBUS_DEBUG(("STP>>Entering inspect port-digest snooping!\n"));
	dbus_error_init( &err );
	if(!(dbus_message_get_args(msg, &err,
								DBUS_TYPE_UINT32, &portindex,
								DBUS_TYPE_UINT32, &configDigestSnpEnDis,
								DBUS_TYPE_INVALID)))
	{
		STP_DBUS_ERR(("STP>>Unable to get input args\n"));
		if(dbus_error_is_set( &err ))
		{
			STP_DBUS_ERR(("STP>>%s raised:%s\n", err.name ,err.message));
			dbus_error_free( &err );
		}
		ret = STP_ERR;	
	}

	STP_DBUS_DEBUG(("STP>>config port %x, configDigestSnp %s\n",
					portindex,
					configDigestSnpEnDis == 1 ? "enable" : "disable"));
	
	/* here call rstp api */
	if (stp_param_check_stp_state()) {
		for (i = 0; i < STP_MSTI_MAX; i++)
		{
			this = stp_stpm_get_instance(i);
			if (NULL != this)
			{
				for (port = this->ports; port; port = port->next)
				{
					if(portindex == port->port_index)
					{
						port->configDigestSnp = (configDigestSnpEnDis == 1)? 1 : 0;
						ret = STP_OK;
						STP_DBUS_DEBUG(("set instance %d portindex %#x configDigestSnp %d\n",
										i, portindex, configDigestSnpEnDis));
						//break;
					}
				}
			}
		}
	}
	else {
		ret = STP_NOT_ENABLED;
	}
		
	reply = dbus_message_new_method_return(msg);
	dbus_message_append_args(reply,
								 DBUS_TYPE_UINT32, &ret,
								 DBUS_TYPE_INVALID);

	return reply;
}

/**********************************************************************************
 * stp_dbus_str2hex
 * 		transfor string from Ascii format to Hex format
 *
 *	INPUT:
 *		srcString	- source string, Ascii format
 *	
 *	OUTPUT:
 *		desString	- string will be returned back, Hex format
 *
 * 	RETURN:
 *		-1 - if desStrint buffer size too small.
 *		-2 - illegal character found.
 *		-3 - parameter is null
 *		0 - if no error occur
 *
 *	NOTATION:
 *		srcString: such as "001122334455", size = 12
 *		desString: {0x00, 0x11, 0x22, 0x33, 0x44, 0x55}, size = 6
 *		
 **********************************************************************************/
int stp_dbus_str2hex
(
    unsigned char *srcString,
    unsigned char *desString
)
{
    unsigned char *tmpString = NULL;
	unsigned char cur = 0;
	unsigned char value = 0;
	unsigned int i = 0;

	if (!srcString || !desString) {
		return -3;
	}
	
	tmpString = srcString;
    for (i = 0; i < strlen(srcString); i++ ) {
        cur = tmpString[i];
		if((cur >= '0') &&(cur <='9')) {
			value = cur - '0';
		}
		else if((cur >= 'A') &&(cur <='F')) {
			value = cur - 'A';
			value += 0xa;
		}
		else if((cur >= 'a') &&(cur <='f')) {
			value = cur - 'a';
			value += 0xa;
		}
		else { /* illegal character found*/
			return -2;
		}

		if(0 == i % 2) {
			desString[i / 2] = value;
		}
		else {
			desString[i / 2] <<= 4;
			desString[i / 2] |= value;
		}
	}

	return 0;
} 

DBusMessage *stp_dbus_bridge_digest_content
(
	DBusConnection *conn,
	DBusMessage *msg,
	void *user_data
)
{
	DBusMessage* reply = NULL;
	DBusError err;
	unsigned int ret = STP_PORT_NOTFOUND;
	unsigned int mstid = 0;	/* save the config in mstid = 0, global config */
	unsigned char *digest_str = NULL;

	register STPM_T *this = NULL;
	
	STP_DBUS_DEBUG(("STP>>Entering bridge digest!\n"));
	dbus_error_init( &err );
	if(!(dbus_message_get_args(msg, &err,
								DBUS_TYPE_STRING, &digest_str,
								DBUS_TYPE_INVALID)))
	{
		STP_DBUS_ERR(("STP>>Unable to get input args\n"));
		if(dbus_error_is_set( &err ))
		{
			STP_DBUS_ERR(("STP>>%s raised:%s\n", err.name ,err.message));
			dbus_error_free( &err );
		}
		ret = STP_ERR;	
	}

	STP_DBUS_DEBUG(("STP>>config instance %d digest %s\n",
					mstid, digest_str));
	
	/* here call rstp api */
	if (stp_param_check_stp_state()) {
		this = stp_stpm_get_instance(mstid);
		if (NULL != this)
		{
			/* clear the old config */
			memset(this->digest, 0, 16 + 1);
			/* 32(ascii) -> 16(hex) */
			ret = stp_dbus_str2hex(digest_str, this->digest);
			ret = STP_OK;
			STP_DBUS_DEBUG(("set instance %d digest:\n",
							mstid));
			int i = 0;
			for (i = 0; i < strlen(this->digest); i++)
			{
				STP_DBUS_DEBUG(("%02x ",
								this->digest[i]));
			}
			STP_DBUS_DEBUG(("\n"));
		}
	}
	else {
		ret = STP_NOT_ENABLED;
	}
		
	reply = dbus_message_new_method_return(msg);
	dbus_message_append_args(reply,
								 DBUS_TYPE_UINT32, &ret,
								 DBUS_TYPE_INVALID);

	return reply;
}
#endif

DBusMessage * stp_dbus_stp_show_running_cfg(DBusConnection *conn, DBusMessage *msg, void *user_data){
	DBusMessage* reply;
	DBusMessageIter	 iter;
	char *showStr = NULL;
	int ret = 0;

	showStr = (char*)malloc(STP_RUNNING_CFG_MEM);
	if(NULL == showStr) {
		printf("memory malloc error\n");
		return;
	}
	memset(showStr,0,STP_RUNNING_CFG_MEM);
	printf("stp cfg \n");
	/*save vlan cfg*/
	ret = stp_param_save_running_cfg(showStr,STP_RUNNING_CFG_MEM);
	
	printf("%s",showStr);
	reply = dbus_message_new_method_return(msg);
	
	dbus_message_iter_init_append (reply, &iter);

	dbus_message_iter_append_basic(&iter,
									DBUS_TYPE_UINT32,
									&ret);
	
	dbus_message_iter_append_basic (&iter,
									 DBUS_TYPE_STRING,
									 &showStr);	
	free(showStr);
	showStr = NULL;
	return reply;
}

static DBusHandlerResult stp_dbus_message_handler 
(
	DBusConnection *connection, 
	DBusMessage *message, 
	void *user_data
)
{
		DBusMessage 	*reply = NULL;
		
		if (strcmp(dbus_message_get_path(message),STP_DBUS_OBJPATH) == 0) 
		{
			if(dbus_message_is_method_call(message,STP_DBUS_INTERFACE,RSTP_DBUS_METHOD_GET_PROTOCOL_STATE))
			{
				reply = stp_dbus_get_stpm_state(connection,message,user_data);
			}
			else if(dbus_message_is_method_call(message,STP_DBUS_INTERFACE,STP_DBUS_METHOD_STPM_ENABLE))
			{
				reply = stp_dbus_stpm_enable(connection,message,user_data);
			}
			else if(dbus_message_is_method_call(message,STP_DBUS_INTERFACE,STP_DBUS_METHOD_INIT_MSTP))
			{
				reply = stp_dbus_mstp_init(connection,message,user_data);
			}
			else if(dbus_message_is_method_call(message,STP_DBUS_INTERFACE,STP_DBUS_METHOD_INIT_MSTP_V1))
			{
				reply = stp_dbus_mstp_init_v1(connection,message,user_data);
			}
			else if(dbus_message_is_method_call(message,STP_DBUS_INTERFACE,RSTP_DBUS_METHOD_PORT_STP_ENABLE))
			{
				reply = stp_dbus_stpm_port_enable(connection,message,user_data);
			}
			else if(dbus_message_is_method_call(message,STP_DBUS_INTERFACE, MSTP_DBUS_METHOD_SET_STP_DUPLEX_MODE))
			{
				reply = stp_dbus_mstp_port_duplex_mode_set(connection,message,user_data);
			}
			else if(dbus_message_is_method_call(message,STP_DBUS_INTERFACE,MSTP_DBUS_METHOD_CFG_VLAN_ON_MST))
			{
				reply = stp_dbus_mstp_set_valn_on_mst(connection,message,user_data);
			}
			else if(dbus_message_is_method_call(message,STP_DBUS_INTERFACE,MSTP_DBUS_METHOD_CONFIG_REG_NAME))
			{
				reply = stp_dbus_mstp_bridge_reg_name(connection,message,user_data);
			}
			else if(dbus_message_is_method_call(message,STP_DBUS_INTERFACE,MSTP_DBUS_METHOD_CONFIG_BR_REVISION))
			{
				reply = stp_dbus_mstp_bridge_revision(connection,message,user_data);
			}
			else if (dbus_message_is_method_call(message,STP_DBUS_INTERFACE,STP_DBUS_METHOD_BRG_PRIORITY)) 
			{
				reply = stp_dbus_bridge_priority(connection,message,user_data);
			}
			else if (dbus_message_is_method_call(message,STP_DBUS_INTERFACE,STP_DBUS_METHOD_BRG_MAXAGE)) 
			{
				reply = stp_dbus_bridge_maxage(connection,message,user_data);
			}
			else if (dbus_message_is_method_call(message,STP_DBUS_INTERFACE,STP_DBUS_METHOD_BRG_HELTIME))
			{
				reply = stp_dbus_bridge_heltime(connection,message,user_data);
			}
			else if (dbus_message_is_method_call(message,STP_DBUS_INTERFACE,MSTP_DBUS_METHOD_CONFIG_MAXHOPS))
			{
				reply = stp_dbus_bridge_maxhops(connection,message,user_data);
			}
			else if (dbus_message_is_method_call(message,STP_DBUS_INTERFACE,STP_DBUS_METHOD_BRG_FORDELAY))
			{
				reply = stp_dbus_bridge_fordelay(connection,message,user_data);
			}
			else if (dbus_message_is_method_call(message,STP_DBUS_INTERFACE,STP_DBUS_METHOD_BRG_FORVERSION))
			{
				reply = stp_dbus_bridge_forversion(connection,message,user_data);
			}
			else if (dbus_message_is_method_call(message,STP_DBUS_INTERFACE,STP_DBUS_METHOD_BRG_NOCONFIG))
			{
				reply = stp_dbus_bridge_noconfig(connection,message,user_data);
			}
			else if (dbus_message_is_method_call(message,STP_DBUS_INTERFACE,STP_DBUS_METHOD_BRG_PORT_PATHCOST))
			{
				reply = stp_dbus_bridge_port_pathcost(connection,message,user_data);
			}
			else if (dbus_message_is_method_call(message,STP_DBUS_INTERFACE,STP_DBUS_METHOD_BRG_PORTPRIO))
			{
				reply = stp_dbus_bridge_port_prio(connection,message,user_data);
			}
			else if (dbus_message_is_method_call(message,STP_DBUS_INTERFACE,STP_DBUS_METHOD_BRG_NONSTP))
			{
				reply = stp_dbus_bridge_port_non_stp(connection,message,user_data);
			}
			else if (dbus_message_is_method_call(message,STP_DBUS_INTERFACE,STP_DBUS_METHOD_BRG_P2P))
			{
				reply = stp_dbus_bridge_port_p2p(connection,message,user_data);
			}
			else if (dbus_message_is_method_call(message,STP_DBUS_INTERFACE,STP_DBUS_METHOD_BRG_EDGE))
			{
				reply = stp_dbus_bridge_port_edge(connection,message,user_data);
			}
			else if (dbus_message_is_method_call(message,STP_DBUS_INTERFACE,STP_DBUS_METHOD_BRG_MCHECK))
			{
				reply = stp_dbus_bridge_port_mcheck(connection,message,user_data);
			}
			else if (dbus_message_is_method_call(message,STP_DBUS_INTERFACE,STP_DBUS_METHOD_BRG_PORTNOCONFIG))
			{
				reply = stp_dbus_bridge_port_noconfig(connection,message,user_data);
			}			
			else if (dbus_message_is_method_call(message,STP_DBUS_INTERFACE,STP_DBUS_METHOD_BRG_SHOWSPANTREE))
			{
				reply = stp_dbus_show_spanning_tree_port_info(connection,message,user_data);
			}
			else if (dbus_message_is_method_call(message,STP_DBUS_INTERFACE,STP_DBUS_METHOD_BRG_SHOWSPANTREE_ADMIN_STATE))
			{
				reply = stp_dbus_show_spanning_tree_port_admin_state(connection,message,user_data);
			}			
			else if (dbus_message_is_method_call(message,STP_DBUS_INTERFACE,RSTP_DBUS_METHOD_CHECK_AND_SAVE_PORT_CONFIG))
			{
				reply = stp_dbus_save_port_stp_config(connection,message,user_data);
			}
			else if (dbus_message_is_method_call(message,STP_DBUS_INTERFACE,STP_DBUS_METHOD_SHOW_BRIDGE_INFO))
			{
				reply = stp_dbus_show_spanning_tree_br_info(connection,message,user_data);
			}
			else if (dbus_message_is_method_call(message,STP_DBUS_INTERFACE,STP_DBUS_METHOD_BRG_DBUG_SPANNTREE))
			{
				reply = stp_dbus_set_debug_value(connection,message,user_data);
			}
			else if (dbus_message_is_method_call(message,STP_DBUS_INTERFACE,STP_DBUS_METHOD_BRG_NO_DBUG_SPANNTREE))
			{
				reply = stp_dbus_set_no_debug_value(connection,message,user_data);
			}
			else if(dbus_message_is_method_call(message,STP_DBUS_INTERFACE,MSTP_DBUS_METHOD_SHOW_CIST_INFO))
			{
				reply = stp_dbus_mstp_show_cist_info(connection,message,user_data);
			}
			else if(dbus_message_is_method_call(message,STP_DBUS_INTERFACE,MSTP_DBUS_METHOD_SHOW_MSTI_INFO))
			{
				reply = stp_dbus_mstp_show_msti_info(connection,message,user_data);
			}
			else if(dbus_message_is_method_call(message,STP_DBUS_INTERFACE,MSTP_DBUS_METHOD_SHOW_SELF_INFO))
			{
				reply = stp_dbus_mstp_show_self_info(connection,message,user_data);
			}
			else if(dbus_message_is_method_call(message,STP_DBUS_INTERFACE,MSTP_DBUS_METHOD_GET_VID_BY_MSTID))
			{
				reply = stp_dbus_mstp_get_vid_by_mstid(connection,message,user_data);
			}
			else if(dbus_message_is_method_call(message,STP_DBUS_INTERFACE,MSTP_DBUS_METHOD_GET_PORT_INFO))
			{
				reply = stp_dbus_mstp_show_instance_port_info(connection,message,user_data);
			}
			else if(dbus_message_is_method_call(message,STP_DBUS_INTERFACE,RSTP_DBUS_METHOD_GET_STP_RUNNING_CFG))
			{
				STP_DBUS_DEBUG(("STP>> stp dbus message handler: show running configure\n"));
				reply = stp_dbus_stp_show_running_cfg(connection,message,user_data);
			}
			else if(dbus_message_is_method_call(message,STP_DBUS_INTERFACE,MSTP_DBUS_METHOD_PORT_ENDIS_CFG_DIGEST_SNP))
			{
				STP_DBUS_DEBUG(("STP>> stp dbus message handler: endis configDigestSnp\n"));
				reply = stp_dbus_bridge_port_configDigestSnp(connection,message,user_data);
			}
			else if(dbus_message_is_method_call(message,STP_DBUS_INTERFACE,MSTP_DBUS_METHOD_CONFIG_BRIDGE_DIGEST_CONTENT))
			{
				STP_DBUS_DEBUG(("STP>> stp dbus message handler: config digest content\n"));
				reply = stp_dbus_bridge_digest_content(connection,message,user_data);
			}
			else
			{
				STP_DBUS_DEBUG(("STP>>dbus message handler::no method match!\n"));
			}
		} 
		
		if (reply)
		{
			dbus_connection_send (connection, reply, NULL);
			dbus_connection_flush(connection); // TODO	  Maybe we should let main loop process the flush
			dbus_message_unref (reply);
		}
	
	//	dbus_message_unref(message); //TODO who should unref the incoming message? 
	return DBUS_HANDLER_RESULT_HANDLED ;
}


/** Message handler for Signals
 *  or normally there should be no signals except dbus-daemon related.
 *
 *  @param  connection          D-BUS connection
 *  @param  message             Message
 *  @param  user_data           User data
 *  @return                     What to do with the message
 */
DBusHandlerResult
stp_dbus_filter_function 
(
	DBusConnection * connection,
	DBusMessage * message, 
	void *user_data
)
{
	if (dbus_message_is_signal (message, DBUS_INTERFACE_LOCAL, "Disconnected") &&
		   strcmp (dbus_message_get_path (message), DBUS_PATH_LOCAL) == 0)
	{
		/* this is a local message; e.g. from libdbus in this process */
		STP_DBUS_DEBUG (("STP>>Got disconnected from the system message bus; "
							"STP>>retrying to reconnect every 3000 ms"));
		
		dbus_connection_unref (stp_dbus_connection);
		stp_dbus_connection = NULL;
	} 
	else if (dbus_message_is_signal (message,DBUS_INTERFACE_DBUS,"STP>>NameOwnerChanged")) 
	{

		//if (services_with_locks != NULL)  service_deleted (message);
	} else
	{
		STP_DBUS_DEBUG(("STP>>no matched message filtered!\n"));
		return TRUE;
	}
		//return hald_dbus_filter_handle_methods (connection, message, user_data, FALSE);

	return DBUS_HANDLER_RESULT_HANDLED;
}

int stp_dbus_init(void)
{
	DBusError 	dbus_error;
	int			ret = STP_OK;
	DBusObjectPathVTable	stp_vtable = {NULL, &stp_dbus_message_handler, NULL, NULL, NULL, NULL};

	STP_DBUS_DEBUG (("STP>>DBUS init...\n"));

	dbus_connection_set_change_sigpipe (TRUE);

	dbus_error_init (&dbus_error);
	stp_dbus_connection = dbus_bus_get (DBUS_BUS_SYSTEM, &dbus_error);
	if (stp_dbus_connection == NULL) {
		STP_DBUS_ERR (("STP>>dbus_bus_get(): %s", dbus_error.message));
		return STP_ERR;
	}
	STP_DBUS_DEBUG(("STP>>stp_dbus_connection %p\n",stp_dbus_connection));

	// Use npd to handle subsection of STP_DBUS_OBJPATH including slots
	if (!dbus_connection_register_fallback (stp_dbus_connection, STP_DBUS_OBJPATH, &stp_vtable, NULL)) {
		STP_DBUS_ERR(("STP>>can't register D-BUS handlers (fallback NPD). cannot continue."));
		return STP_OK;		
	}
		
	ret = dbus_bus_request_name (stp_dbus_connection, STP_DBUS_BUSNAME,0, &dbus_error);
	if(-1 == ret)
	{
		STP_DBUS_ERR(("STP>>dbus request name err %d\n",ret));
		
		ret = STP_ERR;
	}
	else
		STP_DBUS_DEBUG(("STP>>dbus request name ok\n"));
	
	if (dbus_error_is_set (&dbus_error)) {
		STP_DBUS_ERR (("STP>>dbus_bus_request_name(): %s", dbus_error.message));
		
		return STP_ERR;
	}
	return STP_OK;
}

void * stp_dbus_thread_main(void *arg)
{
	int loop_count = 0;
	
	/*
	  * For all OAM method call, synchronous is necessary.
	  * Only signal/event could be asynchronous, it could be sent in other thread.
	  */	
	while (dbus_connection_read_write_dispatch(stp_dbus_connection,-1)) {
             ;
	}
	return NULL;
}

#ifdef __cplusplus
}
#endif
