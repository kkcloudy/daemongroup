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
* igmp_snp_dbus.c
*
*
* CREATOR:
* 		chenbin@autelan.com
*
* DESCRIPTION:
* 		igmp dbus source, comunicate with dcli.
*
* DATE:
*		6/19/2008
*
* FILE REVISION NUMBER:
*  		$Revision: 1.20 $
*
*******************************************************************************/

#ifdef __cplusplus
extern "C"
{
#endif
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <syslog.h>
#include <dbus/dbus.h>

#include "igmp_snp_dbus.h"
#include "igmp_snoop_inter.h"
#include "igmp_snoop.h"
#include "igmp_snp_log.h"
#include "sysdef/returncode.h"

extern unsigned int igmp_snp_daemon_log_open;
extern ULONG igmp_vlanlife;
extern ULONG igmp_grouplife;
extern USHORT igmp_robust_variable;
extern ULONG igmp_query_interval;
extern ULONG igmp_resp_interval;
extern LONG igmp_snoop_enable;
extern LONG mld_snoop_enable;
extern int igmp_slot;
static DBusConnection *igmp_snp_dbus_connection = NULL;
/*******************************************************************************
* igmp_snp_dbus_igmp_snp_debug_on
*
* DESCRIPTION:
*		config IGMP snoop debug on
*
* INPUTS:
*	   conn - dbusconnection
*	   msg - dbusmessage
*	   user_data - dbus data   
*
* OUTPUTS:
*	   null
*
* RETURNS:
*	   reply -
*
* COMMENTS:
*	   
**
********************************************************************************/
DBusMessage *igmp_snp_dbus_igmp_snp_debug_on
(
	DBusConnection *conn, 
	DBusMessage *msg, 
	void *user_data
)
{
	DBusMessage* reply;
	DBusMessageIter iter = {0};
	DBusError		err;

    char 			*ident = "IGMP_SNP";
	unsigned int	ret = IGMPSNP_RETURN_CODE_OK;
	int				debug  = 0;

	dbus_error_init( &err );

	if( !(dbus_message_get_args( msg ,&err, \
					DBUS_TYPE_UINT32, &debug, \
					DBUS_TYPE_INVALID))) {
		igmp_snp_syslog_err("unable to get input args\n");
		if(dbus_error_is_set( &err ))
		{
			igmp_snp_syslog_err("%s raised:%s\n",err.name ,err.message);
			dbus_error_free( &err );
		}
		return NULL;
	}

	if(IGMPSNP_RETURN_CODE_OK == igmp_snoop_log_set_debug_value(debug)){
		openlog(ident, 0, LOG_DAEMON); 
		igmp_snp_daemon_log_open = 1;
		ret = IGMPSNP_RETURN_CODE_OK;
	} 

	reply = dbus_message_new_method_return(msg);
	if(NULL==reply)
	{
		igmp_snp_syslog_err("DBUS new reply message error when set debug!\n");
		return reply;
	}
	
	dbus_message_iter_init_append (reply, &iter);
	dbus_message_iter_append_basic (&iter,DBUS_TYPE_UINT32,&ret);
	
	return reply;
}
/*******************************************************************************
* igmp_snp_dbus_igmp_snp_debug_off
*
* DESCRIPTION:
*		config IGMP snoop debug off
*
* INPUTS:
*	   conn - dbusconnection
*	   msg - dbusmessage
*	   user_data - dbus data   
*
* OUTPUTS:
*	   null
*
* RETURNS:
*	   reply -
*
* COMMENTS:
*	   
**
********************************************************************************/
DBusMessage *igmp_snp_dbus_igmp_snp_debug_off
(
	DBusConnection *conn, 
	DBusMessage *msg, 
	void *user_data
)
{
	DBusMessage* reply;
	DBusMessageIter iter = {0};
	DBusError		err;

    char 			*ident = "IGMP_SNP";
	unsigned int	ret = IGMPSNP_RETURN_CODE_OK;
	int				debug  = 0;

	dbus_error_init( &err );

	if( !(dbus_message_get_args( msg ,&err, \
					DBUS_TYPE_UINT32, &debug, \
					DBUS_TYPE_INVALID))) {
		igmp_snp_syslog_err("unable to get input args\n");
		if(dbus_error_is_set( &err ))
		{
			igmp_snp_syslog_err("%s raised:%s\n",err.name ,err.message);
			dbus_error_free( &err );
		}
		return NULL;
	}

	if(IGMPSNP_RETURN_CODE_OK == igmp_snoop_log_set_no_debug_value(debug)){
		openlog(ident, 0, LOG_DAEMON); 
		igmp_snp_daemon_log_open = 0;
		ret = IGMPSNP_RETURN_CODE_OK;
	} 

	reply = dbus_message_new_method_return(msg);
	if(NULL==reply)
	{
		igmp_snp_syslog_err("DBUS new reply message error when set debug!\n");
		return reply;
	}
	
	dbus_message_iter_init_append (reply, &iter);
	dbus_message_iter_append_basic (&iter,DBUS_TYPE_UINT32,&ret);

	return reply;
}
/*******************************************************************************
* igmp_snp_dbus_igmp_snp_enable
*
* DESCRIPTION:
*		config IGMP snoop enable
*
* INPUTS:
*	   conn - dbusconnection
*	   msg - dbusmessage
*	   user_data - dbus data   
*
* OUTPUTS:
*	   null
*
* RETURNS:
*	   reply -
*
* COMMENTS:
*	   
**
********************************************************************************/
DBusMessage * igmp_snp_dbus_igmp_snp_enable
(
	DBusConnection *conn, 
	DBusMessage *msg, 
	void *user_data
)
{

	DBusMessage* reply;
	DBusMessageIter iter = {0};
	DBusError		err;

	unsigned int	ret = IGMPSNP_RETURN_CODE_OK;
	unsigned char	enable  = 0,ismld =0;

	dbus_error_init( &err );

	if( !(dbus_message_get_args( msg ,&err, \
					DBUS_TYPE_BYTE, &enable, \
					DBUS_TYPE_BYTE, &ismld,\
					DBUS_TYPE_INVALID)))
	{
		igmp_snp_syslog_err("Unable to get input args\n");
		if(dbus_error_is_set( &err ))
		{
			igmp_snp_syslog_err("%s raised:%s\n",err.name ,err.message);
			dbus_error_free( &err );
		}
		return NULL;
	}

	/* here call igmp_snp or mld_snp api */
	if(1 == ismld){/*mld enable or disable*/
		if (0 == enable) {
			ret = mld_snp_set_disable_dbus();
		}
		else if(1 == enable){
			ret = mld_snp_set_enable_dbus();
		}
		else {
			ret = IGMPSNP_RETURN_CODE_ERROR;
		}
	}
	else{/*igmp enable or disable*/
		if (0 == enable) {
			ret = igmp_snp_set_disable_dbus();
		}
		else if(1 == enable){
			ret = igmp_snp_set_enable_dbus();
		}
		else {
			ret = IGMPSNP_RETURN_CODE_ERROR;
		}
	}
	
	reply = dbus_message_new_method_return(msg);
	if(NULL==reply)
	{
		igmp_snp_syslog_dbg("dbus add vlan no reply resource error!\n");
		return reply;
	}
	dbus_message_iter_init_append (reply, &iter);
	dbus_message_iter_append_basic (&iter,DBUS_TYPE_UINT32,&ret);
	return reply;

	
}

/*******************************************************************************
* igmp_snp_dbus_igmp_snp_get_state
*
* DESCRIPTION:
*		get IGMP snoop state
*
* INPUTS:
*	   conn - dbusconnection
*	   msg - dbusmessage
*	   user_data - dbus data   
*
* OUTPUTS:
*	   null
*
* RETURNS:
*	   reply - igmp/mld state
*
* COMMENTS:
*	   
**
********************************************************************************/
DBusMessage * igmp_snp_dbus_get_config_state
(
	DBusConnection *conn, 
	DBusMessage *msg, 
	void *user_data
)
{
	DBusMessage* reply;
	DBusMessageIter iter = {0};
	DBusError		err;

	unsigned int ret = IGMPSNP_RETURN_CODE_OK;
	unsigned int state = 0,ismld =0;

	dbus_error_init( &err );

	if( !(dbus_message_get_args( msg ,&err, \
					DBUS_TYPE_UINT32, &ismld,\
					DBUS_TYPE_INVALID)))
	{
		igmp_snp_syslog_err("Unable to get input args\n");
		if(dbus_error_is_set( &err ))
		{
			igmp_snp_syslog_err("%s raised:%s\n",err.name ,err.message);
			dbus_error_free( &err );
		}
		return NULL;
	}

	/* here call igmp_snp or mld_snp api */
	if(1 == ismld)
	{/*mld enable or disable*/
		state = mld_snoop_enable;
	}
	else
	{/*igmp enable or disable*/
		state = igmp_snoop_enable;
	}
	
	reply = dbus_message_new_method_return(msg);
	if(NULL==reply)
	{
		igmp_snp_syslog_dbg("dbus add vlan no reply resource error!\n");
		return reply;
	}
	dbus_message_iter_init_append (reply, &iter);
	dbus_message_iter_append_basic (&iter,DBUS_TYPE_UINT32,&state);
	return reply;	
}

/*******************************************************************************
* igmp_snp_dbus_config_igmp_snp_timer
*
* DESCRIPTION:
*		config IGMP snooping Timer params
*
* INPUTS:
*	   conn - dbusconnection
*	   msg - dbusmessage
*	   user_data - dbus data   
*
* OUTPUTS:
*	   null
*
* RETURNS:
*	   reply -
*
* COMMENTS:
*	   
**
********************************************************************************/
DBusMessage * igmp_snp_dbus_config_igmp_snp_timer
(
	DBusConnection *conn, 
	DBusMessage *msg, 
	void *user_data
)
{

	DBusMessage* reply;
	DBusMessageIter iter = {0};
	DBusError		err;

	unsigned int	ret = IGMPSNP_RETURN_CODE_OK;
	int	type,timeout = 0;

	//IGMP_DBUS_DEBUG(("IGMP>>igmp_snp dbus message handler: igmp_snp config timer\n"));

	dbus_error_init( &err );

	if( !(dbus_message_get_args( msg ,&err, \
							DBUS_TYPE_UINT32,&type,
							DBUS_TYPE_UINT32,&timeout,
							DBUS_TYPE_INVALID))){
		igmp_snp_syslog_err("Unable to get input args\n");
		if(dbus_error_is_set( &err ))
		{
			igmp_snp_syslog_err("%s raised:%s\n",err.name ,err.message);
			dbus_error_free( &err );
		}
		return NULL;
	}
	//printf("timer type: %d; timeout value: %d\n",type,timeout);

	/* here call rigmp_snp api */
	ret = igmp_snp_config_timer_dbus(type,timeout);

	#if 0
	if (0 == ret ) {
		ret = IGMPSNP_RETURN_CODE_OK;
	}/*according to timer type,error code*/
	else if(IGMP_SNOOP_ERR_NOT_ENABLE_GLB == ret) {
		IGMP_DBUS_DEBUG(("IGMP not enabled global."));
		ret = IGMPSNP_RETURN_CODE_NOT_ENABLE_GBL;
	}
	#endif
	
	reply = dbus_message_new_method_return(msg);
	if(NULL==reply)
	{
		igmp_snp_syslog_dbg("dbus add vlan no reply resource error!\n");
		return reply;
	}
	dbus_message_iter_init_append (reply, &iter);
	dbus_message_iter_append_basic (&iter,DBUS_TYPE_UINT32,&ret);
	return reply;

	
}

/*******************************************************************************
* igmp_snp_dbus_show_igmp_snp_time
*
* DESCRIPTION:
*		show IGMP snooping Timer params
*
* INPUTS:
*	   conn - dbusconnection
*	   msg - dbusmessage
*	   user_data - dbus data   
*
* OUTPUTS:
*	   null
*
* RETURNS:
*	   reply -
*
* COMMENTS:
*	   
**
********************************************************************************/
DBusMessage * igmp_snp_dbus_show_igmp_snp_time
(
	DBusConnection *conn, 
	DBusMessage *msg, 
	void *user_data
)
{

	DBusMessage* reply;
	DBusMessageIter iter = {0};
	DBusError		err;

	unsigned int	ret = IGMPSNP_RETURN_CODE_OK;
	unsigned long	mcgvlanlife=0,grouplife =0,robust=0,queryinterval=0,respinterval=0,hosttime = 0;

	igmp_snp_syslog_dbg("igmp_snp dbus message handler: igmp_snp show timer\n");

	dbus_error_init( &err );

	/* here call igmp_snp api */
	if(IGMPSNP_RETURN_CODE_OK != (ret =igmp_show_vlanlife_timerinterval_dbus(&mcgvlanlife))){
			ret = IGMPSNP_RETURN_CODE_GET_VLANLIFE_E;
	}
	else if(IGMPSNP_RETURN_CODE_OK != (ret =igmp_show_grouplife_timerinterval_dbus(&grouplife))){
			ret = IGMPSNP_RETURN_CODE_GET_GROUPLIFE_E;
	}
	else if(IGMPSNP_RETURN_CODE_OK != (ret =igmp_show_host_timerinterval_dbus(&hosttime))){
			ret = IGMPSNP_RETURN_CODE_GET_HOSTLIFE_E;
	}
	else if(IGMPSNP_RETURN_CODE_OK != (ret =igmp_show_robust_timerinterval_dbus(&robust))){
			ret = IGMPSNP_RETURN_CODE_GET_ROBUST_E;
	}
	else if(IGMPSNP_RETURN_CODE_OK != (ret =igmp_show_query_timerinterval_dbus(&queryinterval))){
			ret= IGMPSNP_RETURN_CODE_GET_QUREY_INTERVAL_E;
	}
	else if(IGMPSNP_RETURN_CODE_OK != (ret =igmp_show_resp_timerinterval_dbus(&respinterval))){
			ret= IGMPSNP_RETURN_CODE_GET_RESP_INTERVAL_E;
	}
	//printf("return value = %d\n",ret);

	
	reply = dbus_message_new_method_return(msg);
	if(NULL==reply)
	{
		igmp_snp_syslog_dbg("dbus add vlan no reply resource error!\n");
		return reply;
	}
	dbus_message_iter_init_append (reply, &iter);
	dbus_message_iter_append_basic (&iter,DBUS_TYPE_UINT32,&ret);
	dbus_message_iter_append_basic (&iter,DBUS_TYPE_UINT32,&mcgvlanlife);
	dbus_message_iter_append_basic (&iter,DBUS_TYPE_UINT32,&grouplife);
	dbus_message_iter_append_basic (&iter,DBUS_TYPE_UINT32,&robust);
	dbus_message_iter_append_basic (&iter,DBUS_TYPE_UINT32,&queryinterval);
	dbus_message_iter_append_basic (&iter,DBUS_TYPE_UINT32,&respinterval);
	dbus_message_iter_append_basic (&iter,DBUS_TYPE_UINT32,&hosttime);
	return reply;

	
}

/*******************************************************************************
* igmp_snp_dbus_igmp_snp_time_get
*
* DESCRIPTION:
*		get IGMP snooping Timer parammeters and each default values
*
* INPUTS:
*	   conn - dbusconnection
*	   msg - dbusmessage
*	   user_data - dbus data   
*
* OUTPUTS:
*	   null
*
* RETURNS:
*	   reply -
*
* COMMENTS:
*	   
**
********************************************************************************/
DBusMessage * igmp_snp_dbus_igmp_snp_time_get
(
	DBusConnection *conn, 
	DBusMessage *msg, 
	void *user_data
)
{
	DBusMessage* 	reply;
	DBusMessageIter iter = {0};
	DBusError		err;

	int length = 0;
	int state = 0;
	int rc = 0;
	int ret = 0;
	char *showStr = NULL;
	char *current = NULL;
	unsigned char Ismld = 0;
    int sw_config_flag = 0;

	dbus_error_init(&err);

	if( !(dbus_message_get_args( msg ,&err, \
					DBUS_TYPE_BYTE, &Ismld, \
					DBUS_TYPE_INVALID))) {
		igmp_snp_syslog_err("unable to get input args\n");
		if(dbus_error_is_set( &err ))
		{
			igmp_snp_syslog_err("%s raised:%s\n",err.name ,err.message);
			dbus_error_free( &err );
		}
		return NULL;
	}
	
	reply = dbus_message_new_method_return(msg);
	if(NULL == reply)
	{
		igmp_snp_syslog_dbg("dbus add igmp or mld snoop time no reply resource error!\n");
		return reply;
	}

	/**************************************** 
	  * save config igmp snooping times information
	  ***************************************/
	showStr = (char*)malloc(IGMP_DBUS_RUNNING_CFG_MEM);
	if(NULL == showStr) {
		igmp_snp_syslog_dbg("memory malloc error.\n");
		return NULL;
	}
	memset(showStr, 0, IGMP_DBUS_RUNNING_CFG_MEM);
	
	if ((igmp_snoop_enable == IGMP_SNOOP_YES) || (mld_snoop_enable == IGMP_SNOOP_YES))
	{
		ret = 0;
		current = showStr;

    	/* into igmp/mld sw-board config node */
		if((igmp_vlanlife != IGMP_VLAN_LIFE_TIME)||(igmp_grouplife != IGMP_GROUP_LIFETIME) \
			||(igmp_robust_variable != IGMP_ROBUST_VARIABLE)||(igmp_query_interval != IGMP_V2_UNFORCED_QUERY_INTERVAL) \
			||(igmp_resp_interval != IGMP_V2_QUERY_RESP_INTERVAL))
		{
            if ((length + 35) < IGMP_DBUS_RUNNING_CFG_MEM)
			{
				if((IGMP_SNOOP_YES == igmp_snoop_enable) && (ISIGMP == Ismld)){
					length += sprintf(current, "config igmp-snooping sw-board %d\n", igmp_slot);
					sw_config_flag = 1;

				}
				else if((IGMP_SNOOP_YES == mld_snoop_enable) && (ISMLD == Ismld)){
					length += sprintf(current, "config mld-snooping sw-board %d\n", igmp_slot);
					sw_config_flag = 1;
				}				
    			igmp_snp_syslog_dbg("%s\n", current);
    			current = showStr + length;
            }
		}
		
		if (igmp_vlanlife != IGMP_VLAN_LIFE_TIME)
		{
			if ((length + 40) < IGMP_DBUS_RUNNING_CFG_MEM)
			{
				if((IGMP_SNOOP_YES == igmp_snoop_enable) && (ISIGMP == Ismld)){
					length += sprintf(current, " config igmp-snooping vlan-lifetime %d\n", igmp_vlanlife);
				}
				if((IGMP_SNOOP_YES == mld_snoop_enable) && (ISMLD == Ismld)){
					length += sprintf(current, " config mld-snooping vlan-lifetime %d\n", igmp_vlanlife);
				}
				igmp_snp_syslog_dbg("%s\n", current);
				current = showStr + length;
			}
		}
		if (igmp_grouplife != IGMP_GROUP_LIFETIME)
		{
			if ((length + 40) < IGMP_DBUS_RUNNING_CFG_MEM)
			{		
				if((IGMP_SNOOP_YES == igmp_snoop_enable) && (ISIGMP == Ismld)){
					length += sprintf(current, " config igmp-snooping group-lifetime %d\n", igmp_grouplife);
				}
				if((IGMP_SNOOP_YES == mld_snoop_enable) && (ISMLD == Ismld)){
					length += sprintf(current, " config mld-snooping group-lifetime %d\n", igmp_grouplife);
				}
				igmp_snp_syslog_dbg("%s\n", current);
				current = showStr + length;
			}	
		}
		if (igmp_robust_variable != IGMP_ROBUST_VARIABLE)
		{
			if ((length + 40) < IGMP_DBUS_RUNNING_CFG_MEM)
			{
				if((IGMP_SNOOP_YES == igmp_snoop_enable) && (ISIGMP == Ismld)){
					length += sprintf(current, " config igmp-snooping robust %d\n", igmp_robust_variable);
				}
				if((IGMP_SNOOP_YES == mld_snoop_enable) && (ISMLD == Ismld)){
					length += sprintf(current, " config mld-snooping robust %d\n", igmp_robust_variable);
				}
				igmp_snp_syslog_dbg("%s\n", current);
				current = showStr + length;
			}
		}
		if (igmp_query_interval != IGMP_V2_UNFORCED_QUERY_INTERVAL)
		{
			if ((length + 40) < IGMP_DBUS_RUNNING_CFG_MEM)
			{
				if((IGMP_SNOOP_YES == igmp_snoop_enable) && (ISIGMP == Ismld)){
					length += sprintf(current, " config igmp-snooping query-interval %d\n", igmp_query_interval);
				}
				if((IGMP_SNOOP_YES == mld_snoop_enable) && (ISMLD == Ismld)){
					length += sprintf(current, " config mld-snooping query-interval %d\n", igmp_query_interval);
				}
				igmp_snp_syslog_dbg("%s\n", current);
				current = showStr + length;
			}
		}
		if (igmp_resp_interval != IGMP_V2_QUERY_RESP_INTERVAL)
		{
			if ((length + 40) < IGMP_DBUS_RUNNING_CFG_MEM)
			{		
				if((IGMP_SNOOP_YES == igmp_snoop_enable) && (ISIGMP == Ismld)){
					length += sprintf(current, " config igmp-snooping response-interval %d\n", igmp_resp_interval);
				}
				if((IGMP_SNOOP_YES == mld_snoop_enable) && (ISMLD == Ismld)){
					length += sprintf(current, " config mld-snooping response-interval %d\n", igmp_resp_interval);
				}
				igmp_snp_syslog_dbg("%s\n", current);
				current = showStr + length;
			}
		}
		/* exit conifg sw-board node */
        if(sw_config_flag ==1)
        {
            /* exit distributed igmp sw-baord config node */
        	length += sprintf(current,"exit\n");
			current = showStr + length;
        }			
	}
	else
	{
		ret = -1;
	}

	dbus_message_iter_init_append(reply, &iter);
	dbus_message_iter_append_basic(&iter,
									DBUS_TYPE_STRING, &showStr);

	free(showStr);
	
	return reply;
}


 /*******************************************************************************
 * igmp_snp_dbus_del_one_mcgroup_vlan
 *
 * DESCRIPTION:
 *   		 delete mcgroup one Or All
 *
 * INPUTS:
 * 		conn - dbusconnection
 *		msg - dbusmessage
 *		user_data - dbus data 	
 *
 * OUTPUTS:
 *    	null
 *
 * RETURNS:
 *		reply -
 *
 * COMMENTS:
 *      
 **
 ********************************************************************************/
DBusMessage * igmp_snp_dbus_mcgroup_del
(
	DBusConnection *conn, 
	DBusMessage *msg, 
	void *user_data
)
{
	
	DBusMessage* reply;
	DBusMessageIter iter = {0};
	DBusError		err;
	
	unsigned int	ret = IGMPSNP_RETURN_CODE_OK;
	unsigned int	vid = 0;
	unsigned int	ipAddr = 0;
	igmp_snp_syslog_dbg("igmp_snp dbus message handler: delete group\n");

	dbus_error_init( &err );

	if( !(dbus_message_get_args( msg ,&err, \
							DBUS_TYPE_UINT16,&vid,\
							DBUS_TYPE_UINT32,&ipAddr,\
					DBUS_TYPE_INVALID)))
	{
		igmp_snp_syslog_err("Unable to get input args\n");
		if(dbus_error_is_set( &err ))
		{
			igmp_snp_syslog_err("%s raised:%s\n",err.name ,err.message);
			dbus_error_free( &err );
		}
		return NULL;
	}

	/* here call igmp_snp api */
	ret = igmp_del_spec_mcgroup_dbus(vid,ipAddr);
	
	reply = dbus_message_new_method_return(msg);
	if(NULL==reply)
	{
		igmp_snp_syslog_dbg("dbus add vlan no reply resource error!\n");
		return reply;
	}
	dbus_message_iter_init_append (reply, &iter);
	dbus_message_iter_append_basic (&iter,DBUS_TYPE_UINT32,&ret);
	return reply;
}


 /*******************************************************************************
 * igmp_snp_dbus_del_one_mcgroup_vlan
 *
 * DESCRIPTION:
 *   		 delete mcgroup vlan one
 *
 * INPUTS:
 * 		conn - dbusconnection
 *		msg - dbusmessage
 *		user_data - dbus data 	
 *
 * OUTPUTS:
 *    	null
 *
 * RETURNS:
 *		reply -
 *
 * COMMENTS:
 *      
 **
 ********************************************************************************/
DBusMessage * igmp_snp_dbus_del_one_mcgroup_vlan
(
	DBusConnection *conn, 
	DBusMessage *msg, 
	void *user_data
)
{
	
	DBusMessage* reply;
	DBusMessageIter iter = {0};
	DBusError		err;
	
	unsigned int	ret = IGMPSNP_RETURN_CODE_OK;
	unsigned int	vid = 0;
	igmp_snp_syslog_dbg("igmp_snp dbus message handler: add vlan\n");

	dbus_error_init( &err );

	if( !(dbus_message_get_args( msg ,&err, \
					DBUS_TYPE_UINT16,&vid,\
					DBUS_TYPE_INVALID)))
	{
		igmp_snp_syslog_err("Unable to get input args\n");
		if(dbus_error_is_set( &err ))
		{
			igmp_snp_syslog_err("%s raised:%s\n",err.name ,err.message);
			dbus_error_free( &err );
		}
		return NULL;
	}

	/* here call igmp_snp api */
	ret = igmp_snp_del_mcgroupvlan(vid);
	
	reply = dbus_message_new_method_return(msg);
	if(NULL==reply)
	{
		igmp_snp_syslog_dbg("dbus add vlan no reply resource error!\n");
		return reply;
	}
	dbus_message_iter_init_append (reply, &iter);
	dbus_message_iter_append_basic (&iter,DBUS_TYPE_UINT32,&ret);
	return reply;
}


 /*******************************************************************************
 * igmp_snp_dbus_mcgroup_vlan_del_all
 *
 * DESCRIPTION:
 *   		 delete mcgroup vlan all.
 *
 * INPUTS:
 * 		conn - dbusconnection
 *		msg - dbusmessage
 *		user_data - dbus data 	
 *
 * OUTPUTS:
 *    	null
 *
 * RETURNS:
 *		reply -
 *
 * COMMENTS:
 *      
 **
 ********************************************************************************/
DBusMessage * igmp_snp_dbus_mcgroup_vlan_del_all
(
	DBusConnection *conn, 
	DBusMessage *msg, 
	void *user_data
)
{
	
	DBusMessage* reply;
	DBusMessageIter iter = {0};
	DBusError		err;
	
	unsigned int	ret = IGMPSNP_RETURN_CODE_OK;
	igmp_snp_syslog_dbg("igmp_snp dbus message handler: add vlan\n");

	dbus_error_init( &err );
	/*No params*/
	/* here call igmp_snp api */
	ret = igmp_del_all_mcgvlan_dbus();
	
	reply = dbus_message_new_method_return(msg);
	if(NULL==reply)
	{
		igmp_snp_syslog_dbg("dbus add vlan no reply resource error!\n");
		return reply;
	}
	dbus_message_iter_init_append (reply, &iter);
	dbus_message_iter_append_basic (&iter,DBUS_TYPE_UINT32,&ret);
	return reply;
}

/*******************************************************************************
 * igmp_snp_dbus_show_igmp_vlan_count
 *
 * DESCRIPTION:
 *   		show IGMP snooping Timer params.
 *
 * INPUTS:
 * 		conn - dbusconnection
 *		msg - dbusmessage
 *		user_data - dbus data 	
 *
 * OUTPUTS:
 *    	null
 *
 * RETURNS:
 *		reply -
 *
 * COMMENTS:
 *      
 **
 ********************************************************************************/
DBusMessage * igmp_snp_dbus_show_igmp_vlan_count
(
	DBusConnection *conn, 
	DBusMessage *msg, 
	void *user_data
)
{

	DBusMessage* reply;
	DBusMessageIter iter = {0};
	DBusError		err;

	unsigned int	ret = IGMPSNP_RETURN_CODE_OK;
	unsigned int	vlancount = 0;

	igmp_snp_syslog_dbg("igmp_snp dbus message handler: igmp_snp show timer\n");

	dbus_error_init( &err );

	/* here call rigmp_snp api */
	ret = igmp_show_igmpvlan_cnt_dbus(&vlancount);
	
	reply = dbus_message_new_method_return(msg);
	if(NULL==reply)
	{
		igmp_snp_syslog_dbg("IGMP>>dbus add vlan no reply resource error!\n");
		return reply;
	}
	dbus_message_iter_init_append (reply, &iter);
	dbus_message_iter_append_basic (&iter,
									 DBUS_TYPE_UINT32,
									 &ret);
	// Total active vlan count
	dbus_message_iter_append_basic (&iter,
									 DBUS_TYPE_UINT32,
									 &vlancount);
	return reply;

	
}

/*******************************************************************************
 * igmp_snp_dbus_show_igmp_mcgroup_total_count
 *
 * DESCRIPTION:
 *   		show IGMP snooping mcgroup count.
 *
 * INPUTS:
 * 		conn - dbusconnection
 *		msg - dbusmessage
 *		user_data - dbus data 	
 *
 * OUTPUTS:
 *    	null
 *
 * RETURNS:
 *		reply -
 *
 * COMMENTS:
 *      
 **
 ********************************************************************************/
DBusMessage * igmp_snp_dbus_show_igmp_mcgroup_total_count
(
	DBusConnection *conn, 
	DBusMessage *msg, 
	void *user_data
)
{

	DBusMessage* reply;
	DBusMessageIter iter = {0};
	DBusError		err;

	unsigned int	ret = IGMPSNP_RETURN_CODE_OK;
	unsigned short  vid = 0;
	int	groupcount = 0;

	dbus_error_init( &err );

	/* here call igmp_snp api */
	ret = igmp_show_group_total_cnt_dbus(&groupcount);
	igmp_snp_syslog_dbg("exists %d mcgroup,ret %d.\n",groupcount,ret);
	reply = dbus_message_new_method_return(msg);
	if(NULL==reply)
	{
		igmp_snp_syslog_dbg("IGMP>>dbus add vlan no reply resource error!\n");
		return reply;
	}
	dbus_message_iter_init_append (reply, &iter);
	dbus_message_iter_append_basic (&iter,
									 DBUS_TYPE_UINT32,
									 &ret);
	dbus_message_iter_append_basic (&iter,
									 DBUS_TYPE_UINT32,
									 &groupcount);
	return reply;
}
/*******************************************************************************
 * igmp_snp_dbus_show_route_port
 *
 * DESCRIPTION:
 *   		show multicast route-port.
 *
 * INPUTS:
 * 		conn - dbusconnection
 *		msg - dbusmessage
 *		user_data - dbus data 	
 *
 * OUTPUTS:
 *    	null
 *
 * RETURNS:
 *		reply -
 *
 * COMMENTS:
 *      
 **
 ********************************************************************************/
DBusMessage *igmp_snp_dbus_show_route_port
(
	DBusConnection *conn, 
	DBusMessage *msg, 
	void *user_data
)
{
	DBusMessage		*reply;
	DBusMessageIter	iter;
	DBusError		err;
	DBusMessageIter	iter_array;


	unsigned int	i = 0;
	unsigned int	count = 0;
	unsigned int	ret = IGMPSNP_RETURN_CODE_OK;
	unsigned short	vlanId = 0;
	long			eth_g_index_array[IGMP_DBUS_MAX_ETHPORT_PER_BOARD * IGMP_DBUS_MAX_CHASSIS_SLOT_COUNT];

	/* init routeArray */
	for (i = 0; i < (IGMP_DBUS_MAX_ETHPORT_PER_BOARD * IGMP_DBUS_MAX_CHASSIS_SLOT_COUNT); i++)
	{
		eth_g_index_array[i] = -1;
	}


	dbus_error_init(&err);

	if (!(dbus_message_get_args(msg, &err,
								DBUS_TYPE_UINT16, &vlanId,
								DBUS_TYPE_INVALID)))
	{
		igmp_snp_syslog_err("Unable to get input args ");
		if (dbus_error_is_set(&err)) {
			igmp_snp_syslog_err("%s raised: %s", err.name, err.message);
			dbus_error_free(&err);
		}
		return NULL;
	}

	ret = igmp_snp_route_port_show(vlanId, &count, eth_g_index_array);

	for (i = 0; i < count; i++)
	{
		if (eth_g_index_array[i] != -1) {
			igmp_snp_syslog_dbg("eth_g_index_array[%d]=[%d]\n", i, eth_g_index_array[i]);
		}
	}

	reply = dbus_message_new_method_return(msg);

	dbus_message_iter_init_append(reply, &iter);
	dbus_message_iter_append_basic(&iter,
									DBUS_TYPE_UINT32, &ret);
	dbus_message_iter_append_basic(&iter,
									DBUS_TYPE_UINT32, &count);

	dbus_message_iter_open_container(&iter,
									DBUS_TYPE_ARRAY,
										DBUS_STRUCT_BEGIN_CHAR_AS_STRING
											DBUS_TYPE_UINT32_AS_STRING
										DBUS_STRUCT_END_CHAR_AS_STRING,
									&iter_array);
	for (i = 0; i < count; i++)
	{
		DBusMessageIter iter_struct;
		dbus_message_iter_open_container(&iter_array,
										 DBUS_TYPE_STRUCT,
										 NULL,
										 &iter_struct);

		dbus_message_iter_append_basic(&iter_struct,
									   DBUS_TYPE_UINT32,
									   &(eth_g_index_array[i]));

		dbus_message_iter_close_container(&iter_array, &iter_struct);
	}

	dbus_message_iter_close_container (&iter, &iter_array);

	return reply;
}

#if 0

/******************************************
*	show IGMP snooping mcgroup count 
*
*	igmp_snp_dbus_show_igmp_mcgroup_count
******************************************/
DBusMessage * igmp_snp_dbus_show_igmp_mcgroup_count
(
	DBusConnection *conn, 
	DBusMessage *msg, 
	void *user_data
)
{

	DBusMessage* reply;
	DBusMessageIter iter = {0};
	DBusError		err;

	unsigned int	ret = IGMP_SNP_OK;
	unsigned short  vid = 0;
	int	groupcount = 0;

	//IGMP_DBUS_DEBUG(("IGMP>>igmp_snp dbus message handler: igmp_snp show mcgroup counts.\n"));

	dbus_error_init( &err );

	if( !(dbus_message_get_args( msg ,&err, \
					DBUS_TYPE_UINT16, &vid, \
					DBUS_TYPE_INVALID)))
	{
		IGMP_DBUS_ERR(("IGMP>>Unable to get input args\n"));
		if(dbus_error_is_set( &err ))
		{
			IGMP_DBUS_ERR(("IGMP>>%s raised:%s\n",err.name ,err.message));
			dbus_error_free( &err );
		}
		return NULL;
	}
	IGMP_DBUS_DEBUG(("vlanId = %d\n",vid));
	/* here call igmp_snp api */
	ret = igmp_show_group_cnt_dbus((unsigned int)vid,&groupcount);
	IGMP_DBUS_DEBUG(("vlan id = %d exists %d mcgroup(s).\n",vid,groupcount));
	reply = dbus_message_new_method_return(msg);
	if(NULL==reply)
	{
		IGMP_DBUS_DEBUG(("IGMP>>dbus add vlan no reply resource error!\n"));
		return reply;
	}
	dbus_message_iter_init_append (reply, &iter);
	dbus_message_iter_append_basic (&iter,
									 DBUS_TYPE_UINT32,
									 &ret);
	// Total active vlan count
	dbus_message_iter_append_basic (&iter,
									 DBUS_TYPE_UINT32,
									 &groupcount);
	return reply;

	
}

/******************************************
*
*	add mcroute port: Proccess by NPD MngSock
*				As-->DEV_EVENT_MCROUTER_PORT_UP
*										/DOWN
*
******************************************/
DBusMessage * igmp_snp_dbus_igmp_snp_add_mcroute_port
(
	DBusConnection *conn, 
	DBusMessage *msg, 
	void *user_data
)
{

	DBusMessage* reply;
	DBusMessageIter iter = {0};
	DBusError		err;

	unsigned int	ret = IGMP_SNP_OK;
	int	isAdd  = 0;

	IGMP_DBUS_DEBUG(("IGMP>>igmp_snp dbus message handler: igmp_snp enable\n"));

	dbus_error_init( &err );

	if( !(dbus_message_get_args( msg ,&err, \
					DBUS_TYPE_UINT32, &isAdd, \
					DBUS_TYPE_INVALID)))
	{
		IGMP_DBUS_ERR(("IGMP>>Unable to get input args\n"));
		if(dbus_error_is_set( &err ))
		{
			IGMP_DBUS_ERR(("IGMP>>%s raised:%s\n",err.name ,err.message));
			dbus_error_free( &err );
		}
		return NULL;
	}

	/* here call igmp_snp api */
	if (0 == isAdd) {
	    IGMP_DBUS_DEBUG (("IGMP>>delete igmp snoop mcgroup router port.\n"));
		//igmp_snp_set_disable_dbus();
	  }
	  else{
		IGMP_DBUS_DEBUG (("IGMP>>add igmp snoop mcgroup router port.\n"));
		//igmp_snp_set_enable_dbus();
	  }
	
	
	reply = dbus_message_new_method_return(msg);
	if(NULL==reply)
	{
		IGMP_DBUS_DEBUG(("IGMP>>dbus add vlan no reply resource error!\n"));
		return reply;
	}
	dbus_message_iter_init_append (reply, &iter);
	dbus_message_iter_append_basic (&iter,DBUS_TYPE_UINT32,&ret);
	return reply;

	
}

/***************************************
  * cmd used to test this method
  * 
  * dbus-send --system --dest=aw.igmp_snp --type=method_call --print-reply=literal /aw/igmp_snp aw.igmp_snp.max_age
  *
  * arg lists for method IGMP_DBUS_METHOD_BRG_MAXAGE
  * in arg list:
  *  	NONE
  *	
  * out arg list:  // in the order as they are appended in the dbus message. 
  *
  *
  */
DBusMessage * igmp_snp_dbus_vlan_add_del_one
(
	DBusConnection *conn, 
	DBusMessage *msg, 
	void *user_data
)
{
	
	DBusMessage* reply;
	DBusMessageIter iter = {0};
	DBusError		err;
	IGMP_DBUS_DEBUG(("IGMP>>igmp_snp dbus message handler: vlan Add|delete.\n"));
	unsigned int	ret = IGMP_SNP_OK;
	unsigned short	vid = 0;
	unsigned char 	add = FALSE;

	dbus_error_init( &err );

	if( !(dbus_message_get_args( msg ,&err, \
					DBUS_TYPE_BYTE,	&add,
					DBUS_TYPE_UINT16, &vid, \
					DBUS_TYPE_INVALID)))
	{
		IGMP_DBUS_ERR(("IGMP>>Unable to get input args\n"));
		if(dbus_error_is_set( &err ))
		{
			IGMP_DBUS_ERR(("IGMP>>%s raised:%s\n",err.name ,err.message));
			dbus_error_free( &err );
		}
		return NULL;
	}
	IGMP_DBUS_DEBUG(("isEnable = %d\n",add));
	IGMP_DBUS_DEBUG(("vlan id = %d\n",vid));
	/* here call igmp_snp api */
	if (add) {
		ret = igmp_snp_vlan_add_dbus(vid);
  	}
	else {
		ret = igmp_snp_vlan_del_dbus(vid);
	}
	
	reply = dbus_message_new_method_return(msg);
	if(NULL==reply)
	{
		IGMP_DBUS_DEBUG(("IGMP>>dbus add vlan no reply resource error!\n"));
		return reply;
	}
	dbus_message_iter_init_append (reply, &iter);
	dbus_message_iter_append_basic (&iter,DBUS_TYPE_UINT32,&ret);
	return reply;
}
/*
  * cmd used to test this method
  * 
  * dbus-send --system --dest=aw.igmp_snp --type=method_call --print-reply=literal /aw/igmp_snp aw.igmp_snp.max_age
  *
  * arg lists for method IGMP_DBUS_METHOD_BRG_MAXAGE
  * in arg list:
  *  	NONE
  *	
  * out arg list:  // in the order as they are appended in the dbus message. 
  *
  *
  */
DBusMessage * igmp_snp_dbus_port_add_del_one(DBusConnection *conn, DBusMessage *msg, void *user_data) {
	
	DBusMessage* reply;
	DBusMessageIter iter = {0};
	DBusError		err;
	
	unsigned int	ret = IGMP_SNP_OK;
	unsigned int	vid = 0,eth_g_index = 0;
	unsigned char 	add = FALSE;
	IGMP_DBUS_DEBUG(("IGMP>>igmp_snp dbus message handler: add vlan\n"));

	dbus_error_init( &err );

	if( !(dbus_message_get_args( msg ,&err, \
					DBUS_TYPE_BYTE,	&add,
					DBUS_TYPE_UINT16, &vid, \
					DBUS_TYPE_UINT32, &eth_g_index, \
					DBUS_TYPE_INVALID)))
	{
		IGMP_DBUS_ERR(("IGMP>>Unable to get input args\n"));
		if(dbus_error_is_set( &err ))
		{
			IGMP_DBUS_ERR(("IGMP>>%s raised:%s\n",err.name ,err.message));
			dbus_error_free( &err );
		}
		return NULL;
	}

	/* here call igmp_snp api */
	if (!add) {
		ret = igmp_snp_add_port_dbus(vid,eth_g_index);
  	}
	else {
		ret = igmp_snp_del_port_dbus(vid,eth_g_index);
	}
	
	reply = dbus_message_new_method_return(msg);
	if(NULL==reply)
	{
		IGMP_DBUS_DEBUG(("IGMP>>dbus add vlan no reply resource error!\n"));
		return reply;
	}
	dbus_message_iter_init_append (reply, &iter);
	dbus_message_iter_append_basic (&iter,DBUS_TYPE_UINT32,&ret);
	return reply;
}
#endif
/*******************************************************************************
 * igmp_snp_dbus_message_handler
 *
 * DESCRIPTION:
 *   		handle the dbus mesage.
 *
 * INPUTS:
 * 		connection - dbusconnection
 *		message - dbusmessage
 *		user_data - dbus data
 *
 * OUTPUTS:
 *    	null
 *
 * RETURNS:
 *		null
 *
 * COMMENTS:
 *      
 **
 ********************************************************************************/
static DBusHandlerResult igmp_snp_dbus_message_handler 
(
	DBusConnection *connection, 
	DBusMessage *message, 
	void *user_data
)
{
	DBusMessage 	*reply = NULL;
	igmp_snp_syslog_dbg ("IGMP>>entering igmp dbus message handler.\n");
	
 	if(strcmp(dbus_message_get_path(message),IGMP_DBUS_OBJPATH) == 0) {

		igmp_snp_syslog_dbg("IGMP>>igmp obj path:"IGMP_DBUS_OBJPATH"\n");

		if (dbus_message_is_method_call(message,IGMP_DBUS_INTERFACE, \
									IGMP_SNP_DBUS_METHOD_IGMP_SNP_DEBUG_ON)) {
			reply = igmp_snp_dbus_igmp_snp_debug_on(connection,message,user_data);
		}
		else if (dbus_message_is_method_call(message,IGMP_DBUS_INTERFACE, \
									IGMP_SNP_DBUS_METHOD_IGMP_SNP_DEBUG_OFF)) {
			reply = igmp_snp_dbus_igmp_snp_debug_off(connection,message,user_data);
		}
		else if(dbus_message_is_method_call(message,IGMP_DBUS_INTERFACE,	\
									IGMP_SNP_DBUS_METHOD_IGMP_SNP_ENABLE)) {
			reply = igmp_snp_dbus_igmp_snp_enable(connection,message,user_data);
		}
		else if (dbus_message_is_method_call(message,IGMP_DBUS_INTERFACE, \
									IGMP_SNP_DBUS_METHOD_IGMP_SNP_SET_TIMER)) {
			reply = igmp_snp_dbus_config_igmp_snp_timer(connection, message, user_data);
		}
		else if (dbus_message_is_method_call(message,IGMP_DBUS_INTERFACE, \
									IGMP_SNP_DBUS_METHOD_IGMP_SNP_SHOW_TIMER)) {
			reply = igmp_snp_dbus_show_igmp_snp_time(connection,message,user_data);
		}
		else if (dbus_message_is_method_call(message,IGMP_DBUS_INTERFACE, \
									IGMP_SNP_DBUS_METHOD_IGMP_SNP_TIME_PARAMETER_GET)) {
			reply = igmp_snp_dbus_igmp_snp_time_get(connection,message,user_data);
		}		
		else if(dbus_message_is_method_call(message, IGMP_DBUS_INTERFACE,	\
									IGMP_SNP_DBUS_METHOD_IGMP_SNP_DEL_MCGROUP)) {
			reply = igmp_snp_dbus_mcgroup_del(connection,message,user_data);
		}
		else if (dbus_message_is_method_call(message,IGMP_DBUS_INTERFACE, \
									IGMP_SNP_DBUS_METHOD_IGMP_SNP_DEL_MCGROUP_VLAN_ONE)) {
			reply = igmp_snp_dbus_del_one_mcgroup_vlan(connection, message, user_data);
		}
		else if (dbus_message_is_method_call(message,IGMP_DBUS_INTERFACE, \
									IGMP_SNP_DBUS_METHOD_IGMP_SNP_DEL_MCGROUP_VLAN_ALL)) {
			reply = igmp_snp_dbus_mcgroup_vlan_del_all(connection,message,user_data);
		}
		/*
		else if (dbus_message_is_method_call(message,IGMP_DBUS_INTERFACE, \
			IGMP_SNP_DBUS_METHOD_IGMP_SNP_ADD_DEL_MCROUTE_PORT)) {
			reply = igmp_snp_dbus_igmp_snp_add_mcroute_port(connection, message, user_data);
		}
		else if (dbus_message_is_method_call(message,IGMP_DBUS_INTERFACE, \
			IGMP_SNP_DBUS_METHOD_IGMP_SNP_VLAN_EN_DIS)) {
			reply = igmp_snp_dbus_vlan_add_del_one(connection, message, user_data);
		}
		else if (dbus_message_is_method_call(message,IGMP_DBUS_INTERFACE, \
			IGMP_SNP_DBUS_METHOD_IGMP_SNP_ETH_PORT_EN_DIS)) {
			reply = igmp_snp_dbus_port_add_del_one(connection, message, user_data);
		}
		else if (dbus_message_is_method_call(message,IGMP_DBUS_INTERFACE, \
									IGMP_SNP_DBUS_METHOD_IGMP_SNP_VLAN_COUNT_SHOW)) {
			reply = igmp_snp_dbus_show_igmp_vlan_count(connection, message, user_data);
		}
		else if (dbus_message_is_method_call(message,IGMP_DBUS_INTERFACE, \
									IGMP_SNP_DBUS_METHOD_IGMP_SNP_GROUP_COUNT_SHOW)) {
			reply = igmp_snp_dbus_show_igmp_mcgroup_count(connection, message, user_data);
		}
		*/
		else if (dbus_message_is_method_call(message,IGMP_DBUS_INTERFACE, \
									IGMP_SNP_DBUS_METHOD_IGMP_SNP_TOTAL_GROUP_COUNT_SHOW)) {
			reply = igmp_snp_dbus_show_igmp_mcgroup_total_count(connection, message, user_data);
		}
		else if (dbus_message_is_method_call(message,IGMP_DBUS_INTERFACE, \
									IGMP_SNP_DBUS_METHOD_IGMP_SNP_SHOW_ROUTE_PORT)) {
			reply = igmp_snp_dbus_show_route_port(connection, message, user_data);
		}
		else if (dbus_message_is_method_call(message,IGMP_DBUS_INTERFACE, \
									IGMP_SNP_DBUS_METHOD_IGMP_SNP_SHOW_STATE)) {
			reply = igmp_snp_dbus_get_config_state(connection, message, user_data);
		}		
	}
	if (reply)
	{
		//IGMP_DBUS_DEBUG(("here send reply to dcli.\n"));
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
igmp_snp_dbus_filter_function 
(
	DBusConnection * connection,
	DBusMessage * message, 
	void *user_data
)
{
	igmp_snp_syslog_dbg("IGMP>>dbus entering filter...\n");
	if (dbus_message_is_signal (message, DBUS_INTERFACE_LOCAL, "Disconnected") &&
		   strcmp (dbus_message_get_path (message), DBUS_PATH_LOCAL) == 0)
	{
		/* this is a local message; e.g. from libdbus in this process */
		igmp_snp_syslog_dbg("IGMP>>Got disconnected from the system message bus; "
							"IGMP>>retrying to reconnect every 3000 ms");
		
		dbus_connection_unref (igmp_snp_dbus_connection);
		igmp_snp_dbus_connection = NULL;
	} 
	else if (dbus_message_is_signal (message,DBUS_INTERFACE_DBUS,"IGMP>>NameOwnerChanged")) 
	{

		//if (services_with_locks != NULL)  service_deleted (message);
	} else
	{
		igmp_snp_syslog_dbg("IGMP>>no matched message filtered!\n");
		return TRUE;
	}
		//return hald_dbus_filter_handle_methods (connection, message, user_data, FALSE);

	return DBUS_HANDLER_RESULT_HANDLED;
}
/*******************************************************************************
 * igmp_snp_dbus_init
 *
 * DESCRIPTION:
 *   		init the dbus ,include bus get and connection etc.
 *
 * INPUTS:
 * 		null
 *
 * OUTPUTS:
 *    	null
 *
 * RETURNS:
 *		IGMPSNP_RETURN_CODE_OK - init ok or can't register D-BUS handlers
 *		IGMPSNP_RETURN_CODE_DBUS_CONNECTION_E - bus get or request name or error set fail
 *
 * COMMENTS:
 *    
 **
 ********************************************************************************/
int igmp_snp_dbus_init(void)
{
	DBusError 	dbus_error;
	int			ret = IGMPSNP_RETURN_CODE_OK;
	DBusObjectPathVTable	igmp_snp_vtable = {NULL, &igmp_snp_dbus_message_handler, NULL, NULL, NULL, NULL};

	igmp_snp_syslog_dbg ("IGMP>>DBUS init...\n");

	dbus_connection_set_change_sigpipe (TRUE);

	dbus_error_init (&dbus_error);
	igmp_snp_dbus_connection = dbus_bus_get (DBUS_BUS_SYSTEM, &dbus_error);
	if (igmp_snp_dbus_connection == NULL) {
		igmp_snp_syslog_err ("IGMP>>dbus_bus_get(): %s", dbus_error.message);
		return IGMPSNP_RETURN_CODE_DBUS_CONNECTION_E;
	}
	igmp_snp_syslog_dbg("IGMP>>igmp_snp_dbus_connection %p\n",igmp_snp_dbus_connection);

	// Use npd to handle subsection of IGMP_DBUS_OBJPATH including slots
	if (!dbus_connection_register_fallback (igmp_snp_dbus_connection, IGMP_DBUS_OBJPATH, &igmp_snp_vtable, NULL)) {
		igmp_snp_syslog_err("IGMP>>can't register D-BUS handlers (fallback NPD). cannot continue.");
		return IGMPSNP_RETURN_CODE_OK;		
	}
		
	ret = dbus_bus_request_name (igmp_snp_dbus_connection, IGMP_DBUS_BUSNAME,0, &dbus_error);
	if(-1 == ret)
	{
		igmp_snp_syslog_err("IGMP>>dbus request name err %d\n",ret);
		
		ret = IGMPSNP_RETURN_CODE_DBUS_CONNECTION_E;
	}
	else
		igmp_snp_syslog_dbg("IGMP>>dbus request name ok\n");
	
	if (dbus_error_is_set (&dbus_error)) {
		igmp_snp_syslog_err ("IGMP>>dbus_bus_request_name(): %s", dbus_error.message);
		
		return IGMPSNP_RETURN_CODE_DBUS_CONNECTION_E;
	}
	return IGMPSNP_RETURN_CODE_OK;
}
/*******************************************************************************
 * igmp_snp_dbus_thread_main
 *
 * DESCRIPTION:
 *   		start the dbus thread.
 *
 * INPUTS:	
 *		null
 * OUTPUTS:   	
 *		null
 * RETURNS:
 *		null
 * COMMENTS:
 *    
 **
 ********************************************************************************/

void * igmp_snp_dbus_thread_main(void *arg)
{
	int loop_count = 0;
	
	/*
	  * For all OAM method call, synchronous is necessary.
	  * Only signal/event could be asynchronous, it could be sent in other thread.
	  */	
	while (dbus_connection_read_write_dispatch(igmp_snp_dbus_connection,-1)) {
		igmp_snp_syslog_dbg("igmp snooping dbus thread running.\n");
	}
	return NULL;
}

#ifdef __cplusplus
}
#endif
