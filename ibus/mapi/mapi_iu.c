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
* mapi_iu.c
*
*
* CREATOR:
*		sunjc@autelan.com
*
* DESCRIPTION:
*		CLI definition for dhcp public function.
*
* DATE:
*		11/29/2010
*
*  FILE REVISION NUMBER:
*  		$Revision: 
*******************************************************************************/
#ifdef __cplusplus
	extern "C"
	{
#endif

#include <dbus/dbus.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <net/if.h>
#include <net/if_arp.h>
#include <sys/ioctl.h>
#include <netinet/in.h>
#include <errno.h>

#include"mapi_iu.h"
#include"dcli_main.h"
#include"dbus/iu/IuDbusDef.h"
#include "dbus/iu/IuDBusPath.h"


#define IU_RETURN_SUCCESS	0
#define IU_RETURN_ERROR		1




/*****************************************************************
 *  iu_get_link_status
 *
 *	DESCRIPTION:
 * 		get iu link status
 *	INPUT:		
 *		proc
 *	OUTPUT:
 *	       m3_asp_state
 *           m3_conn_state
 *           sctp_state
 * 	RETURN:
 *		IU_RETURN_SUCCESS 		->	success
 *		IU_RETURN_ERROR		->	fail
 *	<zhangshu@autelan.com>        2012-1-4	
 ******************************************************************/
int get_iu_link_status
(
    int index, 
    int localid,
    unsigned char isps, 
    unsigned char *m3_asp_state, 
    unsigned char *m3_conn_state, 
    int *sctp_state, 
    DBusConnection *dbus_connection, 
    char *DBUS_METHOD
)
{
	DBusMessage *query = NULL, *reply = NULL;
	DBusMessageIter	 iter;
	DBusError err;
	unsigned int op_ret = 0;

	char BUSNAME[PATH_LEN];
	char OBJPATH[PATH_LEN];
	char INTERFACE[PATH_LEN];
	ReInitFemtoDbusPath(localid,index,IU_DBUS_BUSNAME,BUSNAME);
	ReInitFemtoDbusPath(localid,index,IU_DBUS_OBJPATH,OBJPATH);
	ReInitFemtoDbusPath(localid,index,IU_DBUS_INTERFACE,INTERFACE);

	query = dbus_message_new_method_call(BUSNAME,OBJPATH,INTERFACE,DBUS_METHOD);
	
	dbus_error_init(&err);
	dbus_message_append_args(query, 
							 DBUS_TYPE_BYTE, &isps,
							 DBUS_TYPE_INVALID);	

	reply = dbus_connection_send_with_reply_and_block (dbus_connection,query,-1, &err);
				
	dbus_message_unref(query);
	
	if (NULL == reply) {
		printf("failed get reply.\n");
		if (dbus_error_is_set(&err)) {
			printf("%s raised: %s",err.name,err.message);
			dbus_error_free(&err);
		}
		return IU_RETURN_SUCCESS;
	}

	dbus_message_iter_init(reply,&iter);
	dbus_message_iter_get_basic(&iter,m3_asp_state);
	dbus_message_iter_next(&iter);
	dbus_message_iter_get_basic(&iter,m3_conn_state);
	dbus_message_iter_next(&iter);
	dbus_message_iter_get_basic(&iter,sctp_state);

	dbus_message_unref(reply);
			
	return IU_RETURN_SUCCESS;	
}



/*****************************************************************
 *  iu_set_address
 *
 *	DESCRIPTION:
 * 		set iu interface paras
 *	INPUT:		
 *		address
 *	OUTPUT:
 *		void
 * 	RETURN:
 *		IU_RETURN_SUCCESS 		->	success
 *		IU_RETURN_ERROR		->	fail
 *	<zhangshu@autelan.com>        2011-12-28	
 ******************************************************************/
int iu_set_address(int index, int localid, unsigned int my_ip , unsigned short port, unsigned char is_local, unsigned char is_primary, unsigned char is_ps, DBusConnection *dbus_connection, char *DBUS_METHOD)
{
	DBusMessage *query = NULL, *reply = NULL;
	DBusError err;
	int op_ret = 0;
	
	char BUSNAME[PATH_LEN];
	char OBJPATH[PATH_LEN];
	char INTERFACE[PATH_LEN];
	ReInitFemtoDbusPath(localid,index,IU_DBUS_BUSNAME,BUSNAME);
	ReInitFemtoDbusPath(localid,index,IU_DBUS_OBJPATH,OBJPATH);
	ReInitFemtoDbusPath(localid,index,IU_DBUS_INTERFACE,INTERFACE);

	query = dbus_message_new_method_call(BUSNAME,OBJPATH,INTERFACE,DBUS_METHOD);
									
	dbus_error_init(&err);

	dbus_message_append_args(query,
		 					 DBUS_TYPE_UINT32, &my_ip,
		 					 DBUS_TYPE_UINT16, &port,
		 					 DBUS_TYPE_BYTE, &is_local,
							 DBUS_TYPE_BYTE, &is_primary,
							 DBUS_TYPE_BYTE, &is_ps,
							 DBUS_TYPE_INVALID);	
	
	reply = dbus_connection_send_with_reply_and_block(dbus_connection,query,-1, &err);
	
	dbus_message_unref(query);
	if (NULL == reply) {
		printf("failed get reply.\n");
		if (dbus_error_is_set(&err)) {
			printf("%s raised: %s",err.name,err.message);
			dbus_error_free(&err);
		}
		op_ret = 1;
	}
	
	dbus_message_unref(reply);

	return op_ret;
	
}


/*****************************************************************
 *  iu_set_point_code
 *
 *	DESCRIPTION:
 * 		set iu interface paras
 *	INPUT:		
 *		point_code
 *	OUTPUT:
 *		void
 * 	RETURN:
 *		IU_RETURN_SUCCESS 		->	success
 *		IU_RETURN_ERROR		->	fail
 *	<zhangshu@autelan.com>        2011-12-28	
 ******************************************************************/
int iu_set_point_code(int index, int localid, unsigned int my_pc, unsigned char is_local, unsigned char is_ps, DBusConnection *dbus_connection, char *DBUS_METHOD)
{
	DBusMessage *query = NULL, *reply = NULL;
	DBusError err;
	int op_ret = 0;
	
	char BUSNAME[PATH_LEN];
	char OBJPATH[PATH_LEN];
	char INTERFACE[PATH_LEN];
	ReInitFemtoDbusPath(localid,index,IU_DBUS_BUSNAME,BUSNAME);
	ReInitFemtoDbusPath(localid,index,IU_DBUS_OBJPATH,OBJPATH);
	ReInitFemtoDbusPath(localid,index,IU_DBUS_INTERFACE,INTERFACE);

	query = dbus_message_new_method_call(BUSNAME,OBJPATH,INTERFACE,DBUS_METHOD);
									
	dbus_error_init(&err);

	dbus_message_append_args(query,
		 					 DBUS_TYPE_UINT32, &my_pc,
		 					 DBUS_TYPE_BYTE, &is_local,
							 DBUS_TYPE_BYTE, &is_ps,
							 DBUS_TYPE_INVALID);	
	
	reply = dbus_connection_send_with_reply_and_block(dbus_connection,query,-1, &err);
	
	dbus_message_unref(query);
	if (NULL == reply) {
		printf("failed get reply.\n");
		if (dbus_error_is_set(&err)) {
			printf("%s raised: %s",err.name,err.message);
			dbus_error_free(&err);
		}
		op_ret = 1;
	}
	
	dbus_message_unref(reply);

	return op_ret;
	
}


/*****************************************************************
 *  iu_set_connection_mode
 *
 *	DESCRIPTION:
 * 		set iu interface paras
 *	INPUT:		
 *		conn_mode
 *	OUTPUT:
 *		void
 * 	RETURN:
 *		IU_RETURN_SUCCESS 		->	success
 *		IU_RETURN_ERROR		->	fail
 *	<zhangshu@autelan.com>        2011-12-28	
 ******************************************************************/
int iu_set_connection_mode(int index, int localid, unsigned char conn_mode, unsigned char is_ps, DBusConnection *dbus_connection, char *DBUS_METHOD)
{
	DBusMessage *query = NULL, *reply = NULL;
	DBusError err;
	int op_ret = 0;
	
	char BUSNAME[PATH_LEN];
	char OBJPATH[PATH_LEN];
	char INTERFACE[PATH_LEN];
	ReInitFemtoDbusPath(localid,index,IU_DBUS_BUSNAME,BUSNAME);
	ReInitFemtoDbusPath(localid,index,IU_DBUS_OBJPATH,OBJPATH);
	ReInitFemtoDbusPath(localid,index,IU_DBUS_INTERFACE,INTERFACE);

	query = dbus_message_new_method_call(BUSNAME,OBJPATH,INTERFACE,DBUS_METHOD);
									
	dbus_error_init(&err);

	dbus_message_append_args(query,
		 					 DBUS_TYPE_BYTE, &conn_mode,
							 DBUS_TYPE_BYTE, &is_ps,
							 DBUS_TYPE_INVALID);	
	
	reply = dbus_connection_send_with_reply_and_block(dbus_connection,query,-1, &err);
	
	dbus_message_unref(query);
	if (NULL == reply) {
		printf("failed get reply.\n");
		if (dbus_error_is_set(&err)) {
			printf("%s raised: %s",err.name,err.message);
			dbus_error_free(&err);
		}
		op_ret = 1;
	}
	
	dbus_message_unref(reply);

	return op_ret;
	
}


/*****************************************************************
 *  iu_set_multi_homing_switch
 *
 *	DESCRIPTION:
 * 		set iu interface paras
 *	INPUT:		
 *		multi-homing switch
 *	OUTPUT:
 *		void
 * 	RETURN:
 *		IU_RETURN_SUCCESS 		->	success
 *		IU_RETURN_ERROR		->	fail
 *	<zhangshu@autelan.com>        2011-12-28	
 ******************************************************************/
int iu_set_multi_switch(int index, int localid, unsigned char multi_switch, unsigned char is_local, unsigned char is_ps, DBusConnection *dbus_connection, char *DBUS_METHOD)
{
	DBusMessage *query = NULL, *reply = NULL;
	DBusError err;
	int op_ret = 0;
	
	char BUSNAME[PATH_LEN];
	char OBJPATH[PATH_LEN];
	char INTERFACE[PATH_LEN];
	ReInitFemtoDbusPath(localid,index,IU_DBUS_BUSNAME,BUSNAME);
	ReInitFemtoDbusPath(localid,index,IU_DBUS_OBJPATH,OBJPATH);
	ReInitFemtoDbusPath(localid,index,IU_DBUS_INTERFACE,INTERFACE);

	query = dbus_message_new_method_call(BUSNAME,OBJPATH,INTERFACE,DBUS_METHOD);
									
	dbus_error_init(&err);

	dbus_message_append_args(query,
		 					 DBUS_TYPE_BYTE, &multi_switch,
		 					 DBUS_TYPE_BYTE, &is_local,
							 DBUS_TYPE_BYTE, &is_ps,
							 DBUS_TYPE_INVALID);	
	
	reply = dbus_connection_send_with_reply_and_block(dbus_connection,query,-1, &err);
	
	dbus_message_unref(query);
	if (NULL == reply) {
		printf("failed get reply.\n");
		if (dbus_error_is_set(&err)) {
			printf("%s raised: %s",err.name,err.message);
			dbus_error_free(&err);
		}
		op_ret = 1;
	}
	
	dbus_message_unref(reply);

	return op_ret;
	
}

/**********************************************************************************
 *  iu_set_routing_context
 *
 *	DESCRIPTION:
 * 		set routing context of as or remote as
 *
 *	INPUT:
 *		rtctx
 *
 *	OUTPUT:
 *		
 * 	RETURN:
 *		
 *		0 		->	success
 *		other	->	fail
 *	auther:    book, 2011-3-15
 **********************************************************************************/

int iu_set_routing_context(int index, int localid, unsigned int rtctx, unsigned char as_flag, DBusConnection *dbus_connection, char *DBUS_METHOD)
{
	DBusMessage *query = NULL, *reply = NULL;
	DBusError err;
	unsigned int op_ret = 0;

	char BUSNAME[PATH_LEN];
	char OBJPATH[PATH_LEN];
	char INTERFACE[PATH_LEN];
	ReInitFemtoDbusPath(localid,index,IU_DBUS_BUSNAME,BUSNAME);
	ReInitFemtoDbusPath(localid,index,IU_DBUS_OBJPATH,OBJPATH);
	ReInitFemtoDbusPath(localid,index,IU_DBUS_INTERFACE,INTERFACE);

	query = dbus_message_new_method_call(BUSNAME,OBJPATH,INTERFACE,DBUS_METHOD);
	dbus_error_init(&err);

	dbus_message_append_args(query,
							 DBUS_TYPE_UINT32, &rtctx,
							 DBUS_TYPE_BYTE, &as_flag,	
							 DBUS_TYPE_INVALID);
	
	reply = dbus_connection_send_with_reply_and_block(dbus_connection,query,-1, &err);
	
	dbus_message_unref(query);
	if (NULL == reply) {
		printf("failed get reply.\n");
		if (dbus_error_is_set(&err)) {
			printf("%s raised: %s",err.name,err.message);
			dbus_error_free(&err);
		}
		op_ret = 1;
	}

	
	dbus_message_unref(reply);

	return op_ret;
}


/**********************************************************************************
 *  iu_set_traffic_mode
 *
 *	DESCRIPTION:
 * 		set traffic mode
 *
 *	INPUT:
 *		rtctx
 *
 *	OUTPUT:
 *		
 * 	RETURN:
 *		
 *		0 		->	success
 *		other	->	fail
 *	auther:    book, 2011-3-15
 **********************************************************************************/

int iu_set_traffic_mode(int index, int localid, unsigned int trfmode, unsigned int as_flag, DBusConnection *dbus_connection, char *DBUS_METHOD)
{
	DBusMessage *query = NULL, *reply = NULL;
	DBusError err;
	unsigned int op_ret = 0;

	char BUSNAME[PATH_LEN];
	char OBJPATH[PATH_LEN];
	char INTERFACE[PATH_LEN];
	ReInitFemtoDbusPath(localid,index,IU_DBUS_BUSNAME,BUSNAME);
	ReInitFemtoDbusPath(localid,index,IU_DBUS_OBJPATH,OBJPATH);
	ReInitFemtoDbusPath(localid,index,IU_DBUS_INTERFACE,INTERFACE);

	query = dbus_message_new_method_call(BUSNAME,OBJPATH,INTERFACE,DBUS_METHOD);
	dbus_error_init(&err);

	dbus_message_append_args(query,
							 DBUS_TYPE_UINT32, &trfmode,
							 DBUS_TYPE_UINT32, &as_flag,	
							 DBUS_TYPE_INVALID);
	
	reply = dbus_connection_send_with_reply_and_block(dbus_connection,query,-1, &err);
	
	dbus_message_unref(query);
	if (NULL == reply) {
		printf("failed get reply.\n");
		if (dbus_error_is_set(&err)) {
			printf("%s raised: %s",err.name,err.message);
			dbus_error_free(&err);
		}
		op_ret = 1;
	}

	
	dbus_message_unref(reply);

	return op_ret;
}


/**********************************************************************************
 *  iu_set_ni
 *
 *	DESCRIPTION:
 * 		set net indicator
 *
 *	INPUT:
 *		ni
 *
 *	OUTPUT:
 *		
 * 	RETURN:
 *		
 *		0 		->	success
 *		other	->	fail
 *	auther:    book, 2011-12-2
 **********************************************************************************/

int iu_set_network_indicator(int index, int localid, int ni, DBusConnection *dbus_connection, char *DBUS_METHOD)
{
	DBusMessage *query = NULL, *reply = NULL;
	DBusError err;
	unsigned int op_ret = 0;

	char BUSNAME[PATH_LEN];
	char OBJPATH[PATH_LEN];
	char INTERFACE[PATH_LEN];
	ReInitFemtoDbusPath(localid,index,IU_DBUS_BUSNAME,BUSNAME);
	ReInitFemtoDbusPath(localid,index,IU_DBUS_OBJPATH,OBJPATH);
	ReInitFemtoDbusPath(localid,index,IU_DBUS_INTERFACE,INTERFACE);

	query = dbus_message_new_method_call(BUSNAME,OBJPATH,INTERFACE,DBUS_METHOD);
	dbus_error_init(&err);

	dbus_message_append_args(query,
							 DBUS_TYPE_UINT32, &ni,
							 DBUS_TYPE_INVALID);
	
	reply = dbus_connection_send_with_reply_and_block(dbus_connection,query,-1, &err);
	
	dbus_message_unref(query);
	if (NULL == reply) {
		printf("failed get reply.\n");
		if (dbus_error_is_set(&err)) {
			printf("%s raised: %s",err.name,err.message);
			dbus_error_free(&err);
		}
		op_ret = 1;
	}

	
	dbus_message_unref(reply);

	return op_ret;
}



/**********************************************************************************
 *  iu_set_nwapp
 *
 *	DESCRIPTION:
 * 		set network apperance
 *	INPUT:
 *		nwapp
 *	OUTPUT:
 *		
 * 	RETURN:
 *		0 		->	success
 *		other	->	fail
 *	auther:    book, 2011-12-27
 **********************************************************************************/
int iu_set_network_apperance(int index, int localid, int nwapp, unsigned char isps, DBusConnection *dbus_connection, char *DBUS_METHOD)
{
	DBusMessage *query = NULL, *reply = NULL;
	DBusError err;
	unsigned int op_ret = 0;

	char BUSNAME[PATH_LEN];
	char OBJPATH[PATH_LEN];
	char INTERFACE[PATH_LEN];
	ReInitFemtoDbusPath(localid,index,IU_DBUS_BUSNAME,BUSNAME);
	ReInitFemtoDbusPath(localid,index,IU_DBUS_OBJPATH,OBJPATH);
	ReInitFemtoDbusPath(localid,index,IU_DBUS_INTERFACE,INTERFACE);

	query = dbus_message_new_method_call(BUSNAME,OBJPATH,INTERFACE,DBUS_METHOD);
	
	dbus_error_init(&err);

	dbus_message_append_args(query,
							 DBUS_TYPE_UINT32, &nwapp,
							 DBUS_TYPE_BYTE, &isps,
							 DBUS_TYPE_INVALID);
	
	reply = dbus_connection_send_with_reply_and_block(dbus_connection,query,-1, &err);
	
	dbus_message_unref(query);
	if (NULL == reply) {
		printf("failed get reply.\n");
		if (dbus_error_is_set(&err)) {
			printf("%s raised: %s",err.name,err.message);
			dbus_error_free(&err);
		}
		op_ret = 1;
	}
	dbus_message_unref(reply);

	return op_ret;
}



/**********************************************************************************
 *  iu_set_debug_state
 *
 *	DESCRIPTION:
 * 		set debug_state
 *
 *	INPUT:
 *		
 *		debug_type
 *		debug_enable
 * 		
 *
 *	OUTPUT:
 *		
 * 	RETURN:
 *		
 *		0 		->	success
 *		other	->	fail
 *		
 **********************************************************************************/

unsigned int iu_set_debug_state(int index, int localid, unsigned int debug_type, unsigned int debug_enable, DBusConnection *dbus_connection, char *DBUS_METHOD)
{
	DBusMessage *query = NULL, *reply = NULL;
	DBusError err;
	unsigned int op_ret = 0;

	char BUSNAME[PATH_LEN];
	char OBJPATH[PATH_LEN];
	char INTERFACE[PATH_LEN];
	ReInitFemtoDbusPath(localid,index,IU_DBUS_BUSNAME,BUSNAME);
	ReInitFemtoDbusPath(localid,index,IU_DBUS_OBJPATH,OBJPATH);
	ReInitFemtoDbusPath(localid,index,IU_DBUS_INTERFACE,INTERFACE);

	query = dbus_message_new_method_call(BUSNAME,OBJPATH,INTERFACE,DBUS_METHOD);

	dbus_error_init(&err);
	dbus_message_append_args(query,	
							DBUS_TYPE_UINT32, &debug_type,
							 DBUS_TYPE_UINT32, &debug_enable,
							 DBUS_TYPE_INVALID);

	reply = dbus_connection_send_with_reply_and_block (dbus_connection,query,-1, &err);
				
	dbus_message_unref(query);
	
	if (NULL == reply) {
		if (dbus_error_is_set(&err)) {
			dbus_error_free(&err);
		}
		return IU_RETURN_SUCCESS;
	}

	if (dbus_message_get_args ( reply, &err,
					DBUS_TYPE_UINT32, &op_ret,
					DBUS_TYPE_INVALID)) {
		if(!op_ret) {
	
			dbus_message_unref(reply);
			return IU_RETURN_SUCCESS;
		}
	} 
	else {
		if (dbus_error_is_set(&err)) {
			dbus_error_free(&err);
		}
		dbus_message_unref(reply);
	
		return IU_RETURN_ERROR;
	}
}



/**********************************************************************************
 *  iu_show_running_cfg_lib
 *
 *	DESCRIPTION:
 * 		show and save iu configration
 *
 *	INPUT:		
 *		
 * 		
 *
 *	OUTPUT:
 *		
 * 	RETURN:
 *		
 *		0 		->	success
 *		other	->	fail
 *		
 **********************************************************************************/

unsigned int set_iu_enable(int index, int localid, unsigned int enable, unsigned char isps, DBusConnection *dbus_connection, char *DBUS_METHOD)
{
	DBusMessage *query = NULL, *reply = NULL;
	DBusError err;
	unsigned int op_ret = 0;

	char BUSNAME[PATH_LEN];
	char OBJPATH[PATH_LEN];
	char INTERFACE[PATH_LEN];
	ReInitFemtoDbusPath(localid,index,IU_DBUS_BUSNAME,BUSNAME);
	ReInitFemtoDbusPath(localid,index,IU_DBUS_OBJPATH,OBJPATH);
	ReInitFemtoDbusPath(localid,index,IU_DBUS_INTERFACE,INTERFACE);

	query = dbus_message_new_method_call(BUSNAME,OBJPATH,INTERFACE,DBUS_METHOD);
	
	dbus_error_init(&err);
	dbus_message_append_args(query,
							 DBUS_TYPE_UINT32, &enable, 
							 DBUS_TYPE_BYTE, &isps,
							 DBUS_TYPE_INVALID);	

	reply = dbus_connection_send_with_reply_and_block (dbus_connection,query,-1, &err);
				
	dbus_message_unref(query);
	
	if (NULL == reply) {
		if (dbus_error_is_set(&err)) {
			dbus_error_free(&err);
		}
		return IU_RETURN_SUCCESS;
	}

	if (dbus_message_get_args ( reply, &err,
					DBUS_TYPE_UINT32, &op_ret,
					DBUS_TYPE_INVALID)) {
		if(!op_ret) {
	
			dbus_message_unref(reply);
			return IU_RETURN_SUCCESS;
		}
	} 
	else {
		if (dbus_error_is_set(&err)) {
			dbus_error_free(&err);
		}
		dbus_message_unref(reply);
	
		return IU_RETURN_ERROR;
	}
}



/**********************************************************************************
 *  iu_set_sigtran_service_enable
 *
 *	DESCRIPTION:
 * 		set iu to sigtran enable
 *
 *	INPUT:
 *		enable|disable
 *
 *	OUTPUT:
 *		
 * 	RETURN:
 *		
 *		0 		->	success
 *		other	->	fail
 *	auther:    book, 2011-11-08
 **********************************************************************************/

unsigned int set_iu2sig_enable(int index, int localid, unsigned int enable, DBusConnection *dbus_connection, char *DBUS_METHOD)
{
	DBusMessage *query = NULL, *reply = NULL;
	DBusError err;
	unsigned int op_ret = 0;

	char BUSNAME[PATH_LEN];
	char OBJPATH[PATH_LEN];
	char INTERFACE[PATH_LEN];
	ReInitFemtoDbusPath(localid,index,IU_DBUS_BUSNAME,BUSNAME);
	ReInitFemtoDbusPath(localid,index,IU_DBUS_OBJPATH,OBJPATH);
	ReInitFemtoDbusPath(localid,index,IU_DBUS_INTERFACE,INTERFACE);

	query = dbus_message_new_method_call(BUSNAME,OBJPATH,INTERFACE,DBUS_METHOD);
	
	dbus_error_init(&err);
	dbus_message_append_args(query,
							 DBUS_TYPE_UINT32, &enable,
							 DBUS_TYPE_INVALID);	

	reply = dbus_connection_send_with_reply_and_block (dbus_connection,query,-1, &err);
				
	dbus_message_unref(query);
	
	if (NULL == reply) {
		if (dbus_error_is_set(&err)) {
			dbus_error_free(&err);
		}
		return IU_RETURN_SUCCESS;
	}

	if (dbus_message_get_args ( reply, &err,
					DBUS_TYPE_UINT32, &op_ret,
					DBUS_TYPE_INVALID)) {
		if(!op_ret) {
	
			dbus_message_unref(reply);
			return IU_RETURN_SUCCESS;
		}
	} 
	else {
		if (dbus_error_is_set(&err)) {
			dbus_error_free(&err);
		}
		dbus_message_unref(reply);
	
		return IU_RETURN_ERROR;
	}
}


#ifdef __cplusplus
}
#endif


