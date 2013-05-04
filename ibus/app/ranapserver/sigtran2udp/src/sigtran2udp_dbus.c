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
*  		$Revision: 1.3 $	
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
#include "dbus/iu/IuDbusDef.h"

#include "sigtran2udp_dbus.h"
#include "sigtran2udp_log.h"

#include "confsgp1.h"

#define IU_SAVE_CFG_MEM (10*1024)

pthread_t		*dbus_thread, *netlink_thread;
pthread_attr_t	dbus_thread_attr, netlink_thread_attr;
DBusConnection *iu_dbus_connection = NULL;
int sigtran2udp_enable;
unsigned char connMode = 1;

/*self point code*/
struct cn_config home_gateway_cfg ;

/*msc configuration*/
struct cn_config global_msc_parameter;

/*sgsn configuration*/
struct cn_config global_cn_parameter;

/*debug level*/
extern unsigned int iu_log_level;



/***************************************************************
 * sigtran_dbus_set_self_point_code	
 *			 
 *			
 * INPUT:
 *		conn
 *		msg
 *		user_data
 *		
 * OUTPUT:
 *		void
 *				
 * RETURN:
 *		NULL - get args failed
 *		reply - set success
 *		
 ***************************************************************/

DBusMessage*sigtran_dbus_set_self_point_code
(
	DBusConnection *conn, 
	DBusMessage *msg, 
	void *user_data
)
{
	DBusMessage* reply = NULL;
	DBusMessageIter	 iter;
	DBusError err;
	unsigned int my_ip = 0;
	unsigned short my_port = 0;
	unsigned int self_ptcode = 0;
	
	dbus_error_init(&err);
	
	if (!(dbus_message_get_args ( msg, &err,
		DBUS_TYPE_UINT32, &my_ip,
		DBUS_TYPE_UINT16, &my_port,
		DBUS_TYPE_UINT32, &self_ptcode,		
		DBUS_TYPE_INVALID))) {
		 iu_log_error("while set_debug_state,unable to get input args ");
		if (dbus_error_is_set(&err)) {
			 iu_log_error("%s raised: %s",err.name,err.message);
			dbus_error_free(&err);
		}
		return NULL;
	}
	home_gateway_cfg.ip = my_ip;
	home_gateway_cfg.port = my_port;
	home_gateway_cfg.point_code= self_ptcode;	
	
	sgp_set_global_opc(home_gateway_cfg.point_code);
	
	iu_log_info("self point code is %u\n", self_ptcode);
	
    reply = dbus_message_new_method_return(msg);
	dbus_message_iter_init_append (reply, &iter);
	return reply;
}


/***************************************************************
 * sigtran_dbus_set_self_point_code	
 *			 
 *			
 * INPUT:
 *		conn
 *		msg
 *		user_data
 *		
 * OUTPUT:
 *		void
 *				
 * RETURN:
 *		NULL - get args failed
 *		reply - set success
 *		
 ***************************************************************/

DBusMessage*sigtran_dbus_set_msc
(
	DBusConnection *conn, 
	DBusMessage *msg, 
	void *user_data
)
{
	DBusMessage* reply;
	DBusMessageIter	 iter;	
	DBusError err;
	unsigned int msc_ip = 0;
	unsigned short port = 0; 
	unsigned int  msc_ptcode = 0;
	unsigned int msc_mode = 1; //default mode  is server
	
	dbus_error_init(&err);
    
	if (!(dbus_message_get_args ( msg, &err,
		DBUS_TYPE_UINT32, &msc_ip,
		DBUS_TYPE_UINT16, &port,	
		DBUS_TYPE_UINT32, &msc_ptcode,
		DBUS_TYPE_UINT32, &msc_mode,	
		DBUS_TYPE_INVALID))) {
		 iu_log_error("while set_debug_state,unable to get input args ");
		if (dbus_error_is_set(&err)) {
			 iu_log_error("%s raised: %s",err.name,err.message);
			dbus_error_free(&err);
		}
		return NULL;
	}
    
	global_msc_parameter.ip = msc_ip;
	global_msc_parameter.port = port;
	global_msc_parameter.point_code = msc_ptcode;
	global_msc_parameter.connect_mode = msc_mode;
    
	sgp_set_sctp_cn_mode(global_msc_parameter.connect_mode);
	sgp_set_global_msc_dpc(global_msc_parameter.point_code);
	
	printf("msc ip %u, port %u, point code %u msc connect mode %s\n", 
				global_msc_parameter.ip = msc_ip, 
				global_msc_parameter.port = port, 
				global_msc_parameter.point_code, 
				global_msc_parameter.connect_mode ? "server_mode":"client_mode");
    
	iu_log_info("msc ip %u, port %u, point code %u msc connect mode %s\n", 
				global_msc_parameter.ip = msc_ip, 
				global_msc_parameter.port = port, 
				global_msc_parameter.point_code, 
				global_msc_parameter.connect_mode ? "server_mode":"client_mode");
	
    reply = dbus_message_new_method_return(msg);
	dbus_message_iter_init_append (reply, &iter);
	
	return reply;
}


/***************************************************************
 * iu_dbus_set_sgsn	
 *			 
 *			
 * INPUT:
 *		conn
 *		msg
 *		user_data
 *		
 * OUTPUT:
 *		void
 *				
 * RETURN:
 *		NULL - get args failed
 *		reply - set success
 *		
 ***************************************************************/

DBusMessage*sigtran_dbus_set_cn
(
	DBusConnection *conn, 
	DBusMessage *msg, 
	void *user_data
)
{
	DBusMessage* reply;
	DBusMessageIter	 iter;	
	DBusError err;	
	unsigned int sgsn_ip = 0;
	unsigned short port = 0; 
	
	dbus_error_init(&err);

	if (!(dbus_message_get_args ( msg, &err,
		DBUS_TYPE_UINT32, &sgsn_ip,
		DBUS_TYPE_UINT16, &port,			
		DBUS_TYPE_INVALID))) {
		 iu_log_error("while set_debug_state,unable to get input args ");
		if (dbus_error_is_set(&err)) {
			 iu_log_error("%s raised: %s",err.name,err.message);
			dbus_error_free(&err);
		}
		return NULL;
	}

	global_cn_parameter.ip = sgsn_ip;
	global_cn_parameter.port = port;
	
	
    reply = dbus_message_new_method_return(msg);
	dbus_message_iter_init_append (reply, &iter);
	return reply;
}

/***************************************************************
 * sigtran_dbus_set_debug_state
 *			 
 *			
 *			
 * INPUT:
 *		uint32 - profilee
 *		uint32 - detect
 *		
 * OUTPUT:
 *		uint32 - return code
 *				DHCP_SERVER_RETURN_CODE_SUCCESS  -set success
 * RETURN:
 *		NULL - get args failed
 *		reply - set success
 *		
 ***************************************************************/

DBusMessage*sigtran_dbus_set_debug_state
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

void sigtran_dbus_profile_config_save(char* showStr)
{	
	char *cursor = NULL;	
	int totalLen = 0;
	unsigned char showip[4] = {0};

	if(NULL == showStr){
		return ;
	}
	
	cursor = showStr;

	memcpy(showip , &home_gateway_cfg.ip, 4);
	totalLen += sprintf(cursor, "set self address %u.%u.%u.%u port %u point_code %u\n",
						showip[0], showip[1], showip[2], showip[3],
						home_gateway_cfg.port,
						home_gateway_cfg.point_code);
	cursor = showStr + totalLen;

	if(global_msc_parameter.point_code){

		memcpy(showip , &global_msc_parameter.ip, 4);
		
		totalLen += sprintf(cursor, "msc address %u.%u.%u.%u port %u point-code %u Connection-mode %s\n",
							showip[0], showip[1], showip[2], showip[3],
							global_msc_parameter.port,
							global_msc_parameter.point_code,
							global_msc_parameter.connect_mode? "server" : "client");
		cursor = showStr + totalLen;
	}

	if(global_cn_parameter.point_code){

		memcpy(showip , &global_cn_parameter.ip, 4);
		totalLen += sprintf(cursor, "msc address %u.%u.%u.%u port %u point-code %u Connection-mode %s\n",
							showip[0], showip[1], showip[2], showip[3],
							global_cn_parameter.port,
							global_cn_parameter.point_code,
							global_cn_parameter.connect_mode? "server": "client");
		cursor = showStr + totalLen;
	}

	return ;
}


/*****************************************************
 * sigtran_dbus_show_running_cfg
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
sigtran_dbus_show_running_cfg
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

	sigtran_dbus_profile_config_save(strShow);

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

DBusMessage * 
set_sigtran2udp_enable
(	
	DBusConnection *conn, 
	DBusMessage *msg, 
	void *user_data
)
{
	DBusMessage* reply;
	DBusMessageIter	 iter;
	unsigned int enable = 0;
	unsigned int debug_type = 0;
	unsigned int	op_ret = 0, ret = 0;
	DBusError err;

	dbus_error_init(&err);
	
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

	sigtran2udp_enable = enable;

	
	reply = dbus_message_new_method_return(msg);
	
	dbus_message_iter_init_append (reply, &iter);
	
	dbus_message_iter_append_basic (&iter,
					 DBUS_TYPE_UINT32, 
					 &op_ret);	
	
	if(sigtran2udp_enable){	
	 printf("self ip %x , port %u, homegate ip %x port %u, cnip is %x, port %d\n", 
	 			home_gateway_cfg.ip,home_gateway_cfg.port , 
			global_msc_parameter.ip,global_msc_parameter.port, 
			global_cn_parameter.ip , global_cn_parameter.port);
	um3_server_start(home_gateway_cfg.ip,home_gateway_cfg.port , 
		global_msc_parameter.ip,global_msc_parameter.port , 
		global_cn_parameter.ip , global_cn_parameter.port);
	}
	
	return reply;	
}



/* init dhcp dbus */
static DBusHandlerResult 
sigtran_dbus_message_handler 
(
	DBusConnection *connection, 
	DBusMessage *message, 
	void *user_data
)
{
	DBusMessage		*reply = NULL;
	
	if (dbus_message_is_method_call(message, UDP_IU_DBUS_INTERFACE, UDP_IU_SET_SELF_POINT_CODE)) {
		reply = sigtran_dbus_set_self_point_code(connection, message, user_data);
	}
	if (dbus_message_is_method_call(message, UDP_IU_DBUS_INTERFACE, UDP_IU_SET_MSC)) {
		reply = sigtran_dbus_set_msc(connection, message, user_data);
	}
	if (dbus_message_is_method_call(message, UDP_IU_DBUS_INTERFACE, UDP_IU_SET_SGSN)) {
		reply = sigtran_dbus_set_cn(connection, message, user_data);
	}	

	if (dbus_message_is_method_call(message, UDP_IU_DBUS_INTERFACE, UDP_IU_DBUS_METHOD_SHOW_RUNNING_CFG)) {
		reply = sigtran_dbus_show_running_cfg(connection, message, user_data);
	}

	if (dbus_message_is_method_call(message, UDP_IU_DBUS_INTERFACE, UDP_IU_DBUS_METHOD_SET_SIGTRAN_ENABLE)) {
		reply = set_sigtran2udp_enable(connection, message, user_data);

	}

	if (dbus_message_is_method_call(message, UDP_IU_DBUS_INTERFACE, UDP_IU_DBUS_METHOD_SET_DEBUG_STATE)) {
		reply = sigtran_dbus_set_debug_state(connection, message, user_data);
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

static int sigtran_dbus_init(void)
{
	DBusError dbus_error;

	printf("iu_dbus init\n");
	DBusObjectPathVTable	iu_vtable = {NULL, &sigtran_dbus_message_handler, NULL, NULL, NULL, NULL};
	
	dbus_connection_set_change_sigpipe (1);

	dbus_error_init (&dbus_error);
	iu_dbus_connection = dbus_bus_get (DBUS_BUS_SYSTEM, &dbus_error);
	if (iu_dbus_connection == NULL) {
		iu_log_error ("dbus_bus_get(): %s", dbus_error.message);
		return 1;
	}

	/* Use dhcp to handle subsection of IU_DBUS_OBJPATH including slots*/
	if (!dbus_connection_register_fallback (iu_dbus_connection, UDP_IU_DBUS_OBJPATH, &iu_vtable, NULL)) {
		iu_log_error("can't register D-BUS handlers (fallback DHCP). cannot continue.");
		return 1;	
	}
		
	dbus_bus_request_name (iu_dbus_connection, UDP_IU_DBUS_BUSNAME,
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

void * sigtran_dbus_thread_main(void *arg)
{	
	/*
	For all OAM method call, synchronous is necessary.
	Only signal/event could be asynchronous, it could be sent in other thread.
	*/	
	while (dbus_connection_read_write_dispatch(iu_dbus_connection,-1)) {
		;
	}
	
	return NULL;
}

void sigtran_dbus_start(void)
{
	int ret = 0;	
	sigtran_dbus_init();
	
	dbus_thread = (pthread_t *)malloc(sizeof(pthread_t));
	pthread_attr_init(&dbus_thread_attr);
	ret = pthread_create(dbus_thread, &dbus_thread_attr, sigtran_dbus_thread_main, NULL);
    if (0 != ret) {
	   iu_log_error ("start iu dbus pthread fail\n");
	}
}
#ifdef __cplusplus
}
#endif

