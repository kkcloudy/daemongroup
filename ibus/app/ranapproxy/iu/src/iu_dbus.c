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
* iu_dbus.c
*
* MODIFY:
*		by <sunjc@autelan.com> on 12/03/2010 revision <0.1>
*
* CREATOR:
*		sunjc@autelan.com
*
* DESCRIPTION:
*		CLI definition for iu module.
*
* DATE:
*		12/03/2010
*
*  FILE REVISION NUMBER:
*  		$Revision: 1.6 $	
*******************************************************************************/
#ifdef __cplusplus
extern "C"
{
#endif

#include <string.h>
#include <dbus/dbus.h>
#include <stdbool.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <fcntl.h>
#include <syslog.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <bits/sockaddr.h>
#include "confasp1.h"
#include "sccp.h"
#include "dbus/iu/IuDbusDef.h"
//#include "dbus/iu/IuDBusPath.h"
#include "iu_dbus.h"
#include "iu_log.h"

#define IU_SAVE_CFG_MEM (10*1024)

#define ASP_PORCESS_MSC 	(0)
#define ASP_PORCESS_SGSN	(1)

pthread_t		*dbus_thread, *netlink_thread;
pthread_attr_t	dbus_thread_attr, netlink_thread_attr;
DBusConnection *iu_dbus_connection = NULL;


/*local msc paras*/
struct cn_config home_gateway_msc ;
/*local sgsn paras*/
struct cn_config home_gateway_sgsn ;
/*remote msc paras*/
struct cn_config global_msc_parameter;
/*remote sgsn paras*/
struct cn_config global_sgsn_parameter;
/*debug level*/
extern unsigned int iu_log_level;

/* iu state */
int iu_enable;
/* msc server state */
int msc_enable;
/* sgsn server state */
int sgsn_enable;


/***************************************************************
 * iu_dbus_get_link_status 
 *          
 * INPUT:
 *      proc
 * OUTPUT:
 *      sctp conn state
 *      m3ua asp state
 *      m3ua conn state
 * RETURN:
 *      NULL - get args failed
 *      reply - set success
 *  book add, 2012-1-4
 ***************************************************************/
DBusMessage*iu_dbus_get_link_status
(
    DBusConnection *conn, 
    DBusMessage *msg, 
    void *user_data
)
{
    DBusMessage* reply = NULL;
    DBusMessageIter  iter;
    DBusError err;
    unsigned char isps = 0;
    unsigned char m3_asp_state = 0;
    unsigned char m3_conn_state = 0;
    int sctp_state = 0;
    
    dbus_error_init(&err);

    if (!(dbus_message_get_args ( msg, &err,     
        DBUS_TYPE_BYTE, &isps,
        DBUS_TYPE_INVALID))) {
         iu_log_error("get link status unable to get input args ");
        if (dbus_error_is_set(&err)) {
             iu_log_error("%s raised: %s",err.name,err.message);
            dbus_error_free(&err);
        }
        return NULL;
    }
    if(isps)
    {
        um3_get_iu_interface_status(ASP_PORCESS_SGSN, &m3_asp_state, &m3_conn_state, &sctp_state);
    }
    else
    {
        um3_get_iu_interface_status(ASP_PORCESS_MSC, &m3_asp_state, &m3_conn_state, &sctp_state);
    }
    iu_log_debug("m3_asp_state = %d\n",m3_asp_state);
    iu_log_debug("m3_conn_state = %d\n",m3_conn_state);
    iu_log_debug("sctp_state = %d\n",sctp_state);
    
    reply = dbus_message_new_method_return(msg);
    dbus_message_iter_init_append(reply, &iter);
    dbus_message_iter_append_basic(&iter,DBUS_TYPE_BYTE,&m3_asp_state);
    dbus_message_iter_append_basic(&iter,DBUS_TYPE_BYTE,&m3_conn_state);
    dbus_message_iter_append_basic(&iter,DBUS_TYPE_UINT32,&sctp_state);
    
    return reply;
}



/***************************************************************
 * iu_dbus_set_address  
 *          
 * INPUT:
 *      conn
 *      msg
 *      user_data
 * OUTPUT:
 *      void
 * RETURN:
 *      NULL - get args failed
 *      reply - set success
 *  book add, 2011-12-28
 ***************************************************************/
DBusMessage*iu_dbus_set_address
(
    DBusConnection *conn, 
    DBusMessage *msg, 
    void *user_data
)
{
    DBusMessage* reply = NULL;
    DBusMessageIter  iter;
    DBusError err;
    unsigned int my_ip = 0; 
    unsigned short my_port = 0;
    unsigned char is_local = 0;
    unsigned char is_primary = 0;
    unsigned char isps = 0;
    
    dbus_error_init(&err);

    if (!(dbus_message_get_args ( msg, &err,
        DBUS_TYPE_UINT32, &my_ip,
        DBUS_TYPE_UINT16, &my_port,
        DBUS_TYPE_BYTE, &is_local,
        DBUS_TYPE_BYTE, &is_primary,     
        DBUS_TYPE_BYTE, &isps,
        DBUS_TYPE_INVALID))) {
         iu_log_error("set self point code unable to get input args ");
        if (dbus_error_is_set(&err)) {
             iu_log_error("%s raised: %s",err.name,err.message);
            dbus_error_free(&err);
        }
        return NULL;
    }
    if(is_local)
    {
        if(isps)
        {
            if(is_primary){
                home_gateway_sgsn.primary_ip = my_ip;
                home_gateway_sgsn.primary_port = my_port;
                //asp_set_global_opc(home_gateway_sgsn.point_code,isps);
            }
            else{
                home_gateway_sgsn.secondary_ip = my_ip;
                home_gateway_sgsn.secondary_port = my_port;
            }
        }
        else
        {
            if(is_primary){
                home_gateway_msc.primary_ip = my_ip;
                home_gateway_msc.primary_port = my_port;
                //asp_set_global_opc(home_gateway_msc.point_code,isps);
            }
            else{
                home_gateway_msc.secondary_ip = my_ip;
                home_gateway_msc.secondary_port = my_port;
            }
        }
    }
    else
    {
        if(isps)
        {
            if(is_primary){
                global_sgsn_parameter.primary_ip = my_ip;
                global_sgsn_parameter.primary_port = my_port;
                //asp_set_global_opc(home_gateway_sgsn.point_code,isps);
            }
            else{
                global_sgsn_parameter.secondary_ip = my_ip;
                global_sgsn_parameter.secondary_port = my_port;
            }
        }
        else
        {
            if(is_primary){
                global_msc_parameter.primary_ip = my_ip;
                global_msc_parameter.primary_port = my_port;
                //asp_set_global_opc(home_gateway_msc.point_code,isps);
            }
            else{
                global_msc_parameter.secondary_ip = my_ip;
                global_msc_parameter.secondary_port = my_port;
            }
        }
    }
    reply = dbus_message_new_method_return(msg);
    dbus_message_iter_init_append (reply, &iter);
    return reply;
}



/***************************************************************
 * iu_dbus_set_point_code 
 *          
 * INPUT:
 *      conn
 *      msg
 *      user_data
 * OUTPUT:
 *      void
 * RETURN:
 *      NULL - get args failed
 *      reply - set success
 *  book add, 2011-12-28
 ***************************************************************/
DBusMessage*iu_dbus_set_point_code
(
    DBusConnection *conn, 
    DBusMessage *msg, 
    void *user_data
)
{
    DBusMessage* reply = NULL;
    DBusMessageIter  iter;
    DBusError err;
    unsigned int my_pc = 0; 
    unsigned char is_local = 0;
    unsigned char isps = 0;
    
    dbus_error_init(&err);

    if (!(dbus_message_get_args ( msg, &err,
        DBUS_TYPE_UINT32, &my_pc,
        DBUS_TYPE_BYTE, &is_local,     
        DBUS_TYPE_BYTE, &isps,
        DBUS_TYPE_INVALID))) {
         iu_log_error("set point code unable to get input args ");
        if (dbus_error_is_set(&err)) {
             iu_log_error("%s raised: %s",err.name,err.message);
            dbus_error_free(&err);
        }
        return NULL;
    }
    if(is_local)
    {
        if(isps)
            home_gateway_sgsn.point_code= my_pc;
        else
            home_gateway_msc.point_code= my_pc;
    }
    else
    {
        if(isps)
            global_sgsn_parameter.point_code = my_pc;
        else
            global_msc_parameter.point_code = my_pc;
    }
    
    reply = dbus_message_new_method_return(msg);
    dbus_message_iter_init_append (reply, &iter);
    return reply;
}



/***************************************************************
 * iu_dbus_set_connection_mode
 *          
 * INPUT:
 *      conn
 *      msg
 *      user_data
 * OUTPUT:
 *      void
 * RETURN:
 *      NULL - get args failed
 *      reply - set success
 *  book add, 2011-12-28
 ***************************************************************/
DBusMessage*iu_dbus_set_connection_mode
(
    DBusConnection *conn, 
    DBusMessage *msg, 
    void *user_data
)
{
    DBusMessage* reply = NULL;
    DBusMessageIter  iter;
    DBusError err;
    unsigned char conn_mode = 0; 
    unsigned char isps = 0;
    
    dbus_error_init(&err);

    if (!(dbus_message_get_args ( msg, &err,
        DBUS_TYPE_BYTE, &conn_mode,    
        DBUS_TYPE_BYTE, &isps,
        DBUS_TYPE_INVALID))) {
         iu_log_error("set self point code unable to get input args ");
        if (dbus_error_is_set(&err)) {
             iu_log_error("%s raised: %s",err.name,err.message);
            dbus_error_free(&err);
        }
        return NULL;
    }
    
    if(isps)
        home_gateway_sgsn.connect_mode = conn_mode;
    else
        home_gateway_msc.connect_mode= conn_mode;
    
    asp_set_sctp_cn_mode(conn_mode, isps);
    
    reply = dbus_message_new_method_return(msg);
    dbus_message_iter_init_append (reply, &iter);
    return reply;
}



/***************************************************************
 * iu_dbus_set_multi_switch
 *          
 * INPUT:
 *      conn
 *      msg
 *      user_data
 * OUTPUT:
 *      void
 * RETURN:
 *      NULL - get args failed
 *      reply - set success
 *  book add, 2011-12-28
 ***************************************************************/
DBusMessage*iu_dbus_set_multi_switch
(
    DBusConnection *conn, 
    DBusMessage *msg, 
    void *user_data
)
{
    DBusMessage* reply = NULL;
    DBusMessageIter  iter;
    DBusError err;
    unsigned char multi_switch = 0; 
    unsigned char is_local = 0;
    unsigned char isps = 0;
    
    dbus_error_init(&err);

    if (!(dbus_message_get_args ( msg, &err,
        DBUS_TYPE_BYTE, &multi_switch,
        DBUS_TYPE_BYTE, &is_local,
        DBUS_TYPE_BYTE, &isps,
        DBUS_TYPE_INVALID))) {
         iu_log_error("set self point code unable to get input args ");
        if (dbus_error_is_set(&err)) {
             iu_log_error("%s raised: %s",err.name,err.message);
            dbus_error_free(&err);
        }
        return NULL;
    }

    if(is_local)
    {
        if(isps)
            home_gateway_sgsn.multi_switch = multi_switch;
        else
            home_gateway_msc.multi_switch= multi_switch;
    }
    else{
        if(isps)
            global_sgsn_parameter.multi_switch = multi_switch;
        else
            global_msc_parameter.multi_switch= multi_switch;
    }
   
    reply = dbus_message_new_method_return(msg);
    dbus_message_iter_init_append (reply, &iter);
    return reply;
}


/*
 * iu set routing context
 * add by book, 2011-3-15
*/
DBusMessage*iu_dbus_set_routing_context
(
	DBusConnection *conn, 
	DBusMessage *msg, 
	void *user_data
)
{
	DBusMessage* reply;
	DBusMessageIter	 iter;	
	DBusError err;	
	unsigned int rtctx = 0;
	unsigned char isps = 0;
	
	dbus_error_init(&err);

	if (!(dbus_message_get_args ( msg, &err,
		DBUS_TYPE_UINT32, &rtctx,	
		DBUS_TYPE_BYTE, &isps,	
		DBUS_TYPE_INVALID))) {
		 iu_log_error("msc unable to get input args ");
		if (dbus_error_is_set(&err)) {
			 iu_log_error("%s raised: %s",err.name,err.message);
			dbus_error_free(&err);
		}
		return NULL;
	}

    if(((isps==1)&&(sgsn_enable==0)) || ((isps == 0) && (msc_enable== 0)))
    {
        as_set_routing_context(rtctx,isps);
    }
	
    reply = dbus_message_new_method_return(msg);
	dbus_message_iter_init_append (reply, &iter);
	return reply;
}



/*
 * iu set ni
 * add by book, 2011-12-2
*/
DBusMessage*iu_dbus_set_network_indicator
(
	DBusConnection *conn, 
	DBusMessage *msg, 
	void *user_data
)
{
	DBusMessage* reply;
	DBusMessageIter	 iter;	
	DBusError err;	
	unsigned int ni = 0;
	
	dbus_error_init(&err);

	if (!(dbus_message_get_args ( msg, &err,
		DBUS_TYPE_UINT32, &ni,	
		DBUS_TYPE_INVALID))) {
		 iu_log_error("msc unable to get input args ");
		if (dbus_error_is_set(&err)) {
			 iu_log_error("%s raised: %s",err.name,err.message);
			dbus_error_free(&err);
		}
		return NULL;
	}

    if((iu_enable == 0)){
        m3ua_set_gNi(ni);
    }
	
    reply = dbus_message_new_method_return(msg);
	dbus_message_iter_init_append (reply, &iter);
	return reply;
}




/*
 * iu set traffic mode
 * add by book, 2011-3-15
*/
DBusMessage*iu_dbus_set_traffic_mode
(
	DBusConnection *conn, 
	DBusMessage *msg, 
	void *user_data
)
{
	DBusMessage* reply;
	DBusMessageIter	 iter;	
	DBusError err;	
	unsigned int trfmode = 0;
	unsigned int isps = 0;
	
	dbus_error_init(&err);

	if (!(dbus_message_get_args ( msg, &err,
		DBUS_TYPE_UINT32, &trfmode,	
		DBUS_TYPE_UINT32, &isps,	
		DBUS_TYPE_INVALID))) {
		 iu_log_error("msc unable to get input args ");
		if (dbus_error_is_set(&err)) {
			 iu_log_error("%s raised: %s",err.name,err.message);
			dbus_error_free(&err);
		}
		return NULL;
	}
    /* OVERRIDE 1,   LOAD_SHARE 2,   BROADCAST 3 */  
    if(((isps==1)&&(sgsn_enable==0)) || ((isps == 0) && (msc_enable== 0)))
	{
		if((trfmode >= 1) && (trfmode <= 3))
        	as_set_traffic_mode(trfmode, isps);
    }
	
    reply = dbus_message_new_method_return(msg);
	dbus_message_iter_init_append (reply, &iter);
	return reply;
}


/*
 * iu set network apperance
 * add by book, 2011-12-27
*/
DBusMessage*iu_dbus_set_network_apperance
(
	DBusConnection *conn, 
	DBusMessage *msg, 
	void *user_data
)
{
	DBusMessage* reply;
	DBusMessageIter	 iter;	
	DBusError err;	
	unsigned int nwapp = 0;
	unsigned char isps = 0;
	
	dbus_error_init(&err);

	if (!(dbus_message_get_args ( msg, &err,
		DBUS_TYPE_UINT32, &nwapp,	
		DBUS_TYPE_BYTE, &isps,	
		DBUS_TYPE_INVALID))) {
		 iu_log_error("msc unable to get input args ");
		if (dbus_error_is_set(&err)) {
			 iu_log_error("%s raised: %s",err.name,err.message);
			dbus_error_free(&err);
		}
		return NULL;
	}
	
    if(((isps==1)&&(sgsn_enable==0)) || ((isps == 0) && (msc_enable== 0)))
	{
		as_set_network_apperance(nwapp, isps);
    }
	
    reply = dbus_message_new_method_return(msg);
	dbus_message_iter_init_append (reply, &iter);
	return reply;
}



/***************************************************************
 * iu_dbus_set_debug_state
 *			 
 *			
 * INPUT:
 *		uint32 - profilee
 *		uint32 - detect
 * OUTPUT:
 *		uint32 - return code
 *				DHCP_SERVER_RETURN_CODE_SUCCESS  -set success
 * RETURN:
 *		NULL - get args failed
 *		reply - set success
 *		
 ***************************************************************/
DBusMessage*iu_dbus_set_debug_state
(
	DBusConnection *conn, 
	DBusMessage *msg, 
	void *user_data
)
{
	DBusMessage* reply;
	DBusMessageIter	 iter;
	unsigned int debug_type = 0;
	unsigned int enable = 0;
	unsigned int	op_ret = 0;
	DBusError err;

	dbus_error_init(&err);

	if (!(dbus_message_get_args ( msg, &err,
		DBUS_TYPE_UINT32, &debug_type,
		DBUS_TYPE_UINT32, &enable,
		DBUS_TYPE_INVALID))) {
		 iu_log_error("while set_debug_state,unable to get input args ");
		if (dbus_error_is_set(&err)) {
			 iu_log_error("%s raised: %s",err.name,err.message);
			dbus_error_free(&err);
		}
		return NULL;
	}	
		
	if(debug_type == DEBUG_TYPE_ALL){
		iu_log_info("iu debug_type is %s \n", "all");
	}
	else if(debug_type == DEBUG_TYPE_INFO){
		iu_log_info("iu debug_type is %s \n", "info");
	}
	else if(debug_type == DEBUG_TYPE_ERROR){
		iu_log_info("iu debug_type is %s \n", "error");
	}
	else if(debug_type == DEBUG_TYPE_DEBUG){
		iu_log_info("iu debug_type is %s \n", "debug");
	}

	if(enable){
		iu_log_level |= debug_type;
	}else{
		iu_log_level &= ~debug_type;
	}
	
	iu_log_info("globle iu_log_level is %d \n", iu_log_level);
	
	
	reply = dbus_message_new_method_return(msg);

	dbus_message_iter_init_append (reply, &iter);

	dbus_message_iter_append_basic (&iter,
									 DBUS_TYPE_UINT32,
									 &op_ret);

	return reply;
}


/*****************************************************
 * iu_dbus_profile_config_save
 *		
 *
 * INPUT:
 *				
 *		
 * OUTPUT:
 *		void
 *				
 * RETURN:		
 *		void
 *		
 *****************************************************/
void iu_dbus_profile_config_save(char* showStr)
{	
	char *cursor = NULL;	
	int totalLen = 0;
	unsigned char showip[4] = {0};

	if(NULL == showStr){
		return ;
	}
	
	cursor = showStr;
	/*xiaodawei add*/
	totalLen += sprintf(cursor, "config iu\n");
	cursor = showStr + totalLen;
	if(iu_log_level == DEBUG_TYPE_ALL){
		totalLen += sprintf(cursor, "debug iu all\n");
		cursor = showStr + totalLen;
	}
	else{
		if(iu_log_level == DEBUG_TYPE_INFO){
			totalLen += sprintf(cursor, "debug iu info\n");
			cursor = showStr + totalLen;
		}
		if(iu_log_level == DEBUG_TYPE_ERROR){
			totalLen += sprintf(cursor, "debug iu error\n");
			cursor = showStr + totalLen;
		}
		if(iu_log_level == DEBUG_TYPE_DEBUG){
			totalLen += sprintf(cursor, "debug iu debug\n");
			cursor = showStr + totalLen;
		}
	}
	if(home_gateway_msc.point_code){
    	totalLen += sprintf(cursor, "set local msc point-code %u format integer\n", home_gateway_msc.point_code);
    	cursor = showStr + totalLen;
    }
	if(home_gateway_sgsn.point_code){
		totalLen += sprintf(cursor, "set local sgsn point-code %u format integer\n", home_gateway_sgsn.point_code);
		cursor = showStr + totalLen;
	}
	if(global_msc_parameter.point_code){
    	totalLen += sprintf(cursor, "set remote msc point-code %u format integer\n", global_msc_parameter.point_code);
    	cursor = showStr + totalLen;
    }
	if(global_sgsn_parameter.point_code){
		totalLen += sprintf(cursor, "set remote sgsn point-code %u format integer\n", global_sgsn_parameter.point_code);
		cursor = showStr + totalLen;
	}
	if(home_gateway_msc.connect_mode >= 0){
		totalLen += sprintf(cursor, "set local msc connection-mode %s\n",home_gateway_msc.connect_mode?"server":"client");
		cursor = showStr + totalLen;
	}
	if(home_gateway_sgsn.connect_mode >= 0){
		totalLen += sprintf(cursor, "set local sgsn connection-mode %s\n",home_gateway_sgsn.connect_mode?"server":"client");
		cursor = showStr + totalLen;
	}
	if(home_gateway_msc.multi_switch){
		totalLen += sprintf(cursor, "set local msc multi-homing switch open\n");
		cursor = showStr + totalLen;
	}
	if(home_gateway_sgsn.multi_switch){
		totalLen += sprintf(cursor, "set local sgsn multi-homing switch open\n");
		cursor = showStr + totalLen;
	}
	if(global_msc_parameter.multi_switch){
		totalLen += sprintf(cursor, "set remote msc multi-homing switch open\n");
		cursor = showStr + totalLen;
	}
	if(global_sgsn_parameter.multi_switch){
		totalLen += sprintf(cursor, "set remote sgsn multi-homing switch open\n");
		cursor = showStr + totalLen;
	}
	if(home_gateway_msc.primary_ip && home_gateway_msc.primary_port){
		memcpy(showip , &home_gateway_msc.primary_ip, 4);
		totalLen += sprintf(cursor, "set local msc primary address %u.%u.%u.%u port %u\n",
							showip[0], showip[1], showip[2], showip[3],
							home_gateway_msc.primary_port);
		cursor = showStr + totalLen;
	}
	if(home_gateway_msc.secondary_ip && home_gateway_msc.secondary_port){
		memcpy(showip , &home_gateway_msc.secondary_ip, 4);
		totalLen += sprintf(cursor, "set local msc secondary address %u.%u.%u.%u port %u\n",
							showip[0], showip[1], showip[2], showip[3],
							home_gateway_msc.secondary_port);
		cursor = showStr + totalLen;
	}
	if(home_gateway_sgsn.primary_ip && home_gateway_sgsn.primary_port){
		memcpy(showip , &home_gateway_sgsn.primary_ip, 4);
		totalLen += sprintf(cursor, "set local sgsn primary address %u.%u.%u.%u port %u\n",
							showip[0], showip[1], showip[2], showip[3],
							home_gateway_sgsn.primary_port);
		cursor = showStr + totalLen;
	}
	if(home_gateway_sgsn.secondary_ip && home_gateway_sgsn.secondary_port){
		memcpy(showip , &home_gateway_sgsn.secondary_ip, 4);
		totalLen += sprintf(cursor, "set local sgsn secondary address %u.%u.%u.%u port %u\n",
							showip[0], showip[1], showip[2], showip[3],
							home_gateway_sgsn.secondary_port);
		cursor = showStr + totalLen;
	}
	if(global_msc_parameter.primary_ip && global_msc_parameter.primary_port){
		memcpy(showip , &global_msc_parameter.primary_ip, 4);
		totalLen += sprintf(cursor, "set remote msc primary address %u.%u.%u.%u port %u\n",
							showip[0], showip[1], showip[2], showip[3],
							global_msc_parameter.primary_port);
		cursor = showStr + totalLen;
	}
	if(global_msc_parameter.secondary_ip && global_msc_parameter.secondary_port){
		memcpy(showip , &global_msc_parameter.secondary_ip, 4);
		totalLen += sprintf(cursor, "set remote msc secondary address %u.%u.%u.%u port %u\n",
							showip[0], showip[1], showip[2], showip[3],
							global_msc_parameter.secondary_port);
		cursor = showStr + totalLen;
	}
	if(global_sgsn_parameter.primary_ip && global_sgsn_parameter.primary_port){
		memcpy(showip , &global_sgsn_parameter.primary_ip, 4);
		totalLen += sprintf(cursor, "set remote sgsn primary address %u.%u.%u.%u port %u\n",
							showip[0], showip[1], showip[2], showip[3],
							global_sgsn_parameter.primary_port);
		cursor = showStr + totalLen;
	}
	if(global_sgsn_parameter.secondary_ip && global_sgsn_parameter.secondary_port){
		memcpy(showip , &global_sgsn_parameter.secondary_ip, 4);
		totalLen += sprintf(cursor, "set remote sgsn secondary address %u.%u.%u.%u port %u\n",
							showip[0], showip[1], showip[2], showip[3],
							global_sgsn_parameter.secondary_port);
		cursor = showStr + totalLen;
	}	
	int msc_tfcmode = as_get_traffic_mode(ASP_PORCESS_MSC);
	int sgsn_tfcmode = as_get_traffic_mode(ASP_PORCESS_SGSN);
	unsigned char TrafficMode[3][11] = {"over-ride","load-share","broadcast"};
	if(msc_tfcmode && msc_tfcmode <= 4){
	    totalLen += sprintf(cursor, "set msc traffic mode %s\n", TrafficMode[msc_tfcmode-1]);
		cursor = showStr + totalLen;
	}
	if(sgsn_tfcmode && sgsn_tfcmode <= 4){
	    totalLen += sprintf(cursor, "set sgsn traffic mode %s\n", TrafficMode[sgsn_tfcmode-1]);
		cursor = showStr + totalLen;
	}
	if(gNi){
		totalLen += sprintf(cursor, "set network indicator %d\n", gNi);
		cursor = showStr + totalLen;
	}
	if(msc_as_conf.networkApperance != M3_MAX_U32){
		totalLen += sprintf(cursor, "set msc network apperance %u\n", msc_as_conf.networkApperance);
		cursor = showStr + totalLen;
	}
	if(sgsn_as_conf.networkApperance != M3_MAX_U32){
		totalLen += sprintf(cursor, "set sgsn network apperance %u\n", sgsn_as_conf.networkApperance);
		cursor = showStr + totalLen;
	}
	if(msc_as_conf.routingContext >= 0){
	    totalLen += sprintf(cursor, "set msc routing context %u\n", msc_as_conf.routingContext);
		cursor = showStr + totalLen;
	}
	if(sgsn_as_conf.routingContext >= 0){
	    totalLen += sprintf(cursor, "set sgsn routing context %u\n", sgsn_as_conf.routingContext);
		cursor = showStr + totalLen;
	}
	
	if(msc_enable){
		totalLen += sprintf(cursor, "msc service enable\n");
		cursor = showStr + totalLen;
	}
	if(sgsn_enable){
		totalLen += sprintf(cursor, "sgsn service enable\n");
		cursor = showStr + totalLen;
	}
	totalLen += sprintf(cursor, "exit\n");
		cursor = showStr + totalLen;
	return ;
}


/*****************************************************
 * iu_dbus_show_running_cfg
 *		
 *
 * INPUT:
 *				
 *		
 * OUTPUT:
 *		void
 *				
 * RETURN:
 *		NULL - get args failed
 *		reply - set success
 *		
 *****************************************************/
DBusMessage* 
iu_dbus_show_running_cfg
(
	DBusConnection *conn, 
	DBusMessage *msg, 
	void *user_data
)
{
	DBusMessage*		reply;	  
	DBusMessageIter 	iter= {0};
	DBusError			err;   		
	char *strShow = NULL;
	strShow = (char*)malloc(IU_SAVE_CFG_MEM);	
	if(!strShow) {
		iu_log_debug("alloc memory fail when mirror show running-config\n");
		return NULL;
	}
	memset(strShow, 0, IU_SAVE_CFG_MEM);

	dbus_error_init(&err);

	iu_dbus_profile_config_save(strShow);

	reply = dbus_message_new_method_return(msg);
	dbus_message_iter_init_append (reply, &iter);	
	dbus_message_iter_append_basic (&iter,
								   DBUS_TYPE_STRING,
								   &strShow);
	if(strShow){
		free(strShow);
		strShow = NULL;
	}	
	
	return reply;
}


/*
** set iu service enable|disable
*/
DBusMessage * 
set_iu_enable
(	
	DBusConnection *conn, 
	DBusMessage *msg, 
	void *user_data
)
{
	DBusMessage* reply;
	DBusMessageIter	 iter;
	unsigned int enable = 0;
	//unsigned int debug_type = 0;
	unsigned int	ret = 0;
	DBusError err;
	unsigned char isps = 0;

	dbus_error_init(&err);
	printf("iu service enable\n");
	if (!(dbus_message_get_args ( msg, &err,
		DBUS_TYPE_UINT32, &enable,
		DBUS_TYPE_BYTE, &isps,
		DBUS_TYPE_INVALID))) {
		 iu_log_error("Unable to get input args ");
		if (dbus_error_is_set(&err)) {
			 iu_log_error("%s raised: %s",err.name,err.message);
			dbus_error_free(&err);
		}
		return NULL;
	}

	if(enable)
	{
	    if(isps == 0)
	    {
	        if((home_gateway_msc.primary_ip==0)||(home_gateway_msc.primary_port==0)||(global_msc_parameter.primary_ip==0)||(global_msc_parameter.primary_port==0))
	        {
	            ret = 2;
	        }
	        else
	        {
	            iu_log_debug("***************iu-ps parameters**************");
	            iu_log_debug("home_gateway_msc.primary_ip = %d\n",home_gateway_msc.primary_ip);
	            iu_log_debug("home_gateway_msc.primary_port = %d\n",home_gateway_msc.primary_port);
	            iu_log_debug("home_gateway_msc.secondary_ip = %d\n",home_gateway_msc.secondary_ip);
	            iu_log_debug("home_gateway_msc.secondary_port = %d\n",home_gateway_msc.secondary_port);
	            iu_log_debug("home_gateway_msc.point_code = %d\n",home_gateway_msc.point_code);
	            iu_log_debug("home_gateway_msc.connect_mode = %d\n",home_gateway_msc.connect_mode);
	            iu_log_debug("home_gateway_msc.multi_switch = %d\n",home_gateway_msc.multi_switch);
	            iu_log_debug("global_msc_parameter.primary_ip = %d\n",global_msc_parameter.primary_ip);
	            iu_log_debug("global_msc_parameter.primary_port = %d\n",global_msc_parameter.primary_port);
	            iu_log_debug("global_msc_parameter.secondary_ip = %d\n",global_msc_parameter.secondary_ip);
	            iu_log_debug("global_msc_parameter.secondary_port = %d\n",global_msc_parameter.secondary_port);
	            iu_log_debug("global_msc_parameter.point_code = %d\n",global_msc_parameter.point_code);
	            iu_log_debug("global_msc_parameter.connect_mode = %d\n",global_msc_parameter.connect_mode);
	            iu_log_debug("global_msc_parameter.multi_switch = %d\n",global_msc_parameter.multi_switch);
	            iu_log_debug("*********************************************");
	            msc_enable = 1;
	            ret = um3_server_start(ASP_PORCESS_MSC);
				if(ret == -1)
				{
					msc_enable = 0;
				}
	        }
	    }
	    else if(isps == 1)
	    {
	        if((home_gateway_sgsn.primary_ip==0)||(home_gateway_sgsn.primary_port==0)||(global_sgsn_parameter.primary_ip==0)||(global_sgsn_parameter.primary_port==0))
	        {
	            ret = 2;
	        }
	        else
	        {
	            iu_log_debug("***************iu-ps parameters**************");
	            iu_log_debug("home_gateway_sgsn.primary_ip = %d\n",home_gateway_sgsn.primary_ip);
	            iu_log_debug("home_gateway_sgsn.primary_port = %d\n",home_gateway_sgsn.primary_port);
	            iu_log_debug("home_gateway_sgsn.secondary_ip = %d\n",home_gateway_sgsn.secondary_ip);
	            iu_log_debug("home_gateway_sgsn.secondary_port = %d\n",home_gateway_sgsn.secondary_port);
	            iu_log_debug("home_gateway_sgsn.point_code = %d\n",home_gateway_sgsn.point_code);
	            iu_log_debug("home_gateway_sgsn.connect_mode = %d\n",home_gateway_sgsn.connect_mode);
	            iu_log_debug("home_gateway_sgsn.multi_switch = %d\n",home_gateway_sgsn.multi_switch);
	            iu_log_debug("global_sgsn_parameter.primary_ip = %d\n",global_sgsn_parameter.primary_ip);
	            iu_log_debug("global_sgsn_parameter.primary_port = %d\n",global_sgsn_parameter.primary_port);
	            iu_log_debug("global_sgsn_parameter.secondary_ip = %d\n",global_sgsn_parameter.secondary_ip);
	            iu_log_debug("global_sgsn_parameter.secondary_port = %d\n",global_sgsn_parameter.secondary_port);
	            iu_log_debug("global_sgsn_parameter.point_code = %d\n",global_sgsn_parameter.point_code);
	            iu_log_debug("global_sgsn_parameter.connect_mode = %d\n",global_sgsn_parameter.connect_mode);
	            iu_log_debug("global_sgsn_parameter.multi_switch = %d\n",global_sgsn_parameter.multi_switch);
	            iu_log_debug("*********************************************");
	            sgsn_enable = 1;
	            ret = um3_server_start(ASP_PORCESS_SGSN);
				if(ret == -1)
				{
					sgsn_enable = 0;
				}
	        }
	    }
		else
		{
		    iu_log_info("Error: iu service enable Parameter error.\n");
		}
		
		if(ret == 0)
		{
			iu_enable = enable;
		}
	}
	else /* book add 2011-12-27 */
	{
		//need release the resource of iu interfaces
		if(isps == 0)
	    {
	        um3_server_stop(ASP_PORCESS_MSC);
	        msc_enable = 0;
	    }
	    else if(isps == 1)
	    {
	        um3_server_stop(ASP_PORCESS_SGSN);
	        sgsn_enable = 0;
	    }
	    else
	    {
	        iu_log_error("Error: iu service enable Parameter error.\n");
	    }
	    if((0==msc_enable) && (0==sgsn_enable))
	    {
	        iu_enable = 0;
	    }
	}

	reply = dbus_message_new_method_return(msg);
	
	dbus_message_iter_init_append (reply, &iter);
	
	dbus_message_iter_append_basic (&iter,
					 DBUS_TYPE_UINT32, 
					 &ret);
	
	
	return reply;	
}


/*
** func:set sigtrans enable
** time:2011-11-08
** <zhangshu@autelan.com>
*/
DBusMessage * 
set_iu_to_sigtran_enable
(   
    DBusConnection *conn, 
    DBusMessage *msg, 
    void *user_data
)
{
    DBusMessage* reply;
    DBusMessageIter  iter;
    unsigned int enable = 0;
    unsigned int    op_ret = 0;
    DBusError err;

    dbus_error_init(&err);
    printf("iu service enable\n");
    if (!(dbus_message_get_args ( msg, &err,
        DBUS_TYPE_UINT32, &enable,
        DBUS_TYPE_INVALID))) {
         iu_log_error("Unable to get input args ");
        if (dbus_error_is_set(&err)) {
             iu_log_error("%s raised: %s",err.name,err.message);
            dbus_error_free(&err);
        }
        return NULL;
    }
    
    iu_enable = enable;
    msc_enable = enable;
    
    reply = dbus_message_new_method_return(msg);
    
    dbus_message_iter_init_append (reply, &iter);
    
    dbus_message_iter_append_basic (&iter,
                     DBUS_TYPE_UINT32, 
                     &op_ret);

    if(iu_enable)
    {
        //um3_start_iu2sig(home_gateway_msc.ip, home_gateway_msc.port, global_msc_parameter.ip, global_msc_parameter.port);
    }
    
    return reply;   
}



/* init dhcp dbus */
static DBusHandlerResult 
iu_dbus_message_handler 
(
	DBusConnection *connection, 
	DBusMessage *message, 
	void *user_data
)
{
	DBusMessage		*reply = NULL;	

	if (dbus_message_is_method_call(message, IU_DBUS_INTERFACE, IU_SET_ROUTING_CONTEXT)) {
		reply = iu_dbus_set_routing_context(connection, message, user_data);
	}
	else if (dbus_message_is_method_call(message, IU_DBUS_INTERFACE, IU_SET_TRAFFIC_MODE)) {
		reply = iu_dbus_set_traffic_mode(connection, message, user_data);
	}
	else if (dbus_message_is_method_call(message, IU_DBUS_INTERFACE, IU_DBUS_METHOD_SET_IU_ENABLE)) {
		reply = set_iu_enable(connection, message, user_data);
	}	
    else if (dbus_message_is_method_call(message, IU_DBUS_INTERFACE, IU_DBUS_METHOD_SET_IU_TO_SIGTRAN_ENABLE)) {
		reply = set_iu_to_sigtran_enable(connection, message, user_data);
	}
	else if (dbus_message_is_method_call(message, IU_DBUS_INTERFACE, IU_DBUS_METHOD_SHOW_RUNNING_CFG)) {
		reply = iu_dbus_show_running_cfg(connection, message, user_data);
	}
	else if (dbus_message_is_method_call(message, IU_DBUS_INTERFACE, IU_DBUS_METHOD_SET_DEBUG_STATE)) {
		reply = iu_dbus_set_debug_state(connection, message, user_data);
	}
	else if (dbus_message_is_method_call(message, IU_DBUS_INTERFACE, IU_SET_NETWORK_INDICATOR)) {
		reply = iu_dbus_set_network_indicator(connection, message, user_data);
	}
	else if (dbus_message_is_method_call(message, IU_DBUS_INTERFACE, IU_SET_NETWORK_APPERANCE)) {
		reply = iu_dbus_set_network_apperance(connection, message, user_data);
	}
	else if (dbus_message_is_method_call(message, IU_DBUS_INTERFACE, IU_SET_ADDRESS)) {
		reply = iu_dbus_set_address(connection, message, user_data);
	}
	else if (dbus_message_is_method_call(message, IU_DBUS_INTERFACE, IU_SET_POINT_CODE)) {
		reply = iu_dbus_set_point_code(connection, message, user_data);
	}
	else if (dbus_message_is_method_call(message, IU_DBUS_INTERFACE, IU_SET_CONNECTION_MODE)) {
		reply = iu_dbus_set_connection_mode(connection, message, user_data);
	}
	else if (dbus_message_is_method_call(message, IU_DBUS_INTERFACE, IU_SET_MULTI_SWITCH)) {
		reply = iu_dbus_set_multi_switch(connection, message, user_data);
	}
	else if (dbus_message_is_method_call(message, IU_DBUS_INTERFACE, IU_GET_LINK_STATUS)) {
		reply = iu_dbus_get_link_status(connection, message, user_data);
	}

	if (reply) {
		dbus_connection_send (connection, reply, NULL);
		dbus_connection_flush(connection); /* TODO Maybe we should let main loop process the flush*/
		dbus_message_unref (reply);
	}

	return DBUS_HANDLER_RESULT_HANDLED;
}



DBusHandlerResult 
iu_dbus_filter_function 
(
	DBusConnection * connection,
	DBusMessage * message, 
	void *user_data
)
{
	if (dbus_message_is_signal (message, DBUS_INTERFACE_LOCAL, "Disconnected") &&
		   strcmp (dbus_message_get_path (message), DBUS_PATH_LOCAL) == 0) {

		/* this is a local message; e.g. from libdbus in this process */
		dbus_connection_unref (iu_dbus_connection);
		iu_dbus_connection = NULL;		

	} 
	else if (dbus_message_is_signal (message,
			      DBUS_INTERFACE_DBUS,
			      "NameOwnerChanged")) {		
	}
	else {
		return 1;
	}	

	return DBUS_HANDLER_RESULT_HANDLED;
}

int iu_dbus_init(void)
{
	DBusError dbus_error;
	DBusObjectPathVTable	iu_vtable = {NULL, &iu_dbus_message_handler, NULL, NULL, NULL, NULL};	
	dbus_connection_set_change_sigpipe (1);
    
	dbus_error_init (&dbus_error);
	iu_dbus_connection = dbus_bus_get (DBUS_BUS_SYSTEM, &dbus_error);
	if (iu_dbus_connection == NULL) {
		iu_log_error ("dbus_bus_get(): %s", dbus_error.message);
		return 1;
	}
    
	/* Use dhcp to handle subsection of IU_DBUS_OBJPATH including slots*/
	if (!dbus_connection_register_fallback (iu_dbus_connection, IU_DBUS_OBJPATH, &iu_vtable, NULL)) {
		iu_log_error("can't register D-BUS handlers (fallback DHCP). cannot continue.");
		return 1;	
	}
	
	dbus_bus_request_name (iu_dbus_connection, IU_DBUS_BUSNAME,
			       0, &dbus_error);
		
	if (dbus_error_is_set (&dbus_error)) {
		iu_log_error ("dbus_bus_request_name(): %s",
			    dbus_error.message);
		return 1;
	}

	dbus_connection_add_filter (iu_dbus_connection, iu_dbus_filter_function, NULL, NULL);

	dbus_bus_add_match (iu_dbus_connection,
			    		"type='signal'"
					    ",interface='"DBUS_INTERFACE_DBUS"'"
					    ",sender='"DBUS_SERVICE_DBUS"'"
					    ",member='NameOwnerChanged'",
			    NULL);

	return 0;
}

void * iu_dbus_thread_main(void *arg)
{	
    if(!iu_dbus_init()){
    	/*
        	For all OAM method call, synchronous is necessary.
        	Only signal/event could be asynchronous, it could be sent in other thread.
        	*/	
    	while (dbus_connection_read_write_dispatch(iu_dbus_connection,-1)) {
    		;
    	}
    }	
	return NULL;
}

void iu_dbus_start(void)
{
	/*int ret = 0;	
	iu_dbus_init();
	
	dbus_thread = (pthread_t *)malloc(sizeof(pthread_t));
	pthread_attr_init(&dbus_thread_attr);
	ret = pthread_create(dbus_thread, &dbus_thread_attr, iu_dbus_thread_main, NULL);
    if (0 != ret) {
	   iu_log_error ("start iu dbus pthread fail\n");
	}

	pthread_join(dbus_thread, NULL);

	return 0;
	*/
}
#ifdef __cplusplus
}
#endif

