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
#include"mapi_sigtran2udp.h"
//#include "dcli_main.h"
#include "dbus/iu/SigtranDBusPath.h"


#define IU_RETURN_SUCCESS	0
#define IU_RETURN_ERROR		1


/*****************************************************************
 *  iu_set_self_point_code
 *
 *	DESCRIPTION:
 * 		set home gateway source point code
 *
 *	INPUT:		
 *		self_ptcode
 *	OUTPUT:
 *		void
 *
 * 	RETURN:
 *		
 *		IU_RETURN_SUCCESS 		->	success
 *		IU_RETURN_ERROR		->	fail
 *		
 ******************************************************************/

int sigtran_set_self_point_code(int index, int localid, unsigned int my_ip , unsigned short port, unsigned int self_ptcode, DBusConnection *dbus_connection, char *DBUS_METHOD)
{
	DBusMessage *query = NULL, *reply = NULL;
	DBusError err;
	int op_ret = 0;

	char BUSNAME[PATH_LEN];
	char OBJPATH[PATH_LEN];
	char INTERFACE[PATH_LEN];
	ReInitFemtoDbusPath(index,localid,UDP_IU_DBUS_BUSNAME,BUSNAME);
	ReInitFemtoDbusPath(index,localid,UDP_IU_DBUS_OBJPATH,OBJPATH);
	ReInitFemtoDbusPath(index,localid,UDP_IU_DBUS_INTERFACE,INTERFACE);

	query = dbus_message_new_method_call(BUSNAME,OBJPATH,INTERFACE,DBUS_METHOD);

	dbus_error_init(&err);

	dbus_message_append_args(query,
		 					 DBUS_TYPE_UINT32, &my_ip,
		 					 DBUS_TYPE_UINT16, &port,
							 DBUS_TYPE_UINT32, &self_ptcode,							 				 
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
 *  show_dhcp_lease_lib
 *
 *	DESCRIPTION:
 * 		show dhcp lease
 *
 *	INPUT:
 *		msc_ip -> msc ip address
 *		port -> use of socket
 *		d_point_code -> Destination point code
 * 		cnmode -> 1 is server , 0 is client
 *
 *	OUTPUT:
 *		
 * 	RETURN:
 *		
 *		0 		->	success
 *		other	->	fail
 *		
 **********************************************************************************/

int sigtran_set_msc(int index, int localid, unsigned int msc_ip, unsigned short port, unsigned int d_point_code, unsigned int cn_mode, DBusConnection *dbus_connection, char *DBUS_METHOD)
{
	DBusMessage *query = NULL, *reply = NULL;
	DBusError err;
	unsigned int op_ret = 0;

	char BUSNAME[PATH_LEN];
	char OBJPATH[PATH_LEN];
	char INTERFACE[PATH_LEN];
	ReInitFemtoDbusPath(index,localid,UDP_IU_DBUS_BUSNAME,BUSNAME);
	ReInitFemtoDbusPath(index,localid,UDP_IU_DBUS_OBJPATH,OBJPATH);
	ReInitFemtoDbusPath(index,localid,UDP_IU_DBUS_INTERFACE,INTERFACE);

	query = dbus_message_new_method_call(BUSNAME,OBJPATH,INTERFACE,DBUS_METHOD);

	dbus_error_init(&err);

	dbus_message_append_args(query,
							 DBUS_TYPE_UINT32, &msc_ip,
							 DBUS_TYPE_UINT16, &port,	
							 DBUS_TYPE_UINT32, &d_point_code,
							 DBUS_TYPE_UINT32, &cn_mode,
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
 *  iu_set_sgsn
 *
 *	DESCRIPTION:
 * 		show dhcp lease
 *
 *	INPUT:
 *		msc_ip -> msc ip address
 *		port -> use of socket
 *		d_point_code -> Destination point code
 * 		cnmode -> 1 is server , 0 is client
 *
 *	OUTPUT:
 *		
 * 	RETURN:
 *		
 *		0 		->	success
 *		other	->	fail
 *		
 **********************************************************************************/

int sigtran_set_cn(int index, int localid, unsigned int cn_ip, unsigned short port, DBusConnection *dbus_connection, char *DBUS_METHOD)
{
	DBusMessage *query = NULL, *reply = NULL;
	DBusError err;
	unsigned int op_ret = 0;
	const char *s_ip = NULL;
	char BUSNAME[PATH_LEN];
	char OBJPATH[PATH_LEN];
	char INTERFACE[PATH_LEN];
	ReInitFemtoDbusPath(index,localid,UDP_IU_DBUS_BUSNAME,BUSNAME);
	ReInitFemtoDbusPath(index,localid,UDP_IU_DBUS_OBJPATH,OBJPATH);
	ReInitFemtoDbusPath(index,localid,UDP_IU_DBUS_INTERFACE,INTERFACE);

	query = dbus_message_new_method_call(BUSNAME,OBJPATH,INTERFACE,DBUS_METHOD);

	dbus_error_init(&err);

	dbus_message_append_args(query,
							 DBUS_TYPE_UINT32, &cn_ip,
							 DBUS_TYPE_UINT16, &port,							
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

unsigned int sigtran_set_debug_state(int index, int localid, unsigned int debug_type, unsigned int debug_enable, DBusConnection *dbus_connection, char *DBUS_METHOD)
{
	DBusMessage *query = NULL, *reply = NULL;
	DBusError err;
	unsigned int op_ret = 0;
	char BUSNAME[PATH_LEN];
	char OBJPATH[PATH_LEN];
	char INTERFACE[PATH_LEN];
	ReInitFemtoDbusPath(index,localid,UDP_IU_DBUS_BUSNAME,BUSNAME);
	ReInitFemtoDbusPath(index,localid,UDP_IU_DBUS_OBJPATH,OBJPATH);
	ReInitFemtoDbusPath(index,localid,UDP_IU_DBUS_INTERFACE,INTERFACE);

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

int sigtran_show_running_cfg_lib(int index, int localid, char *showStr, DBusConnection *dbus_connection, char *DBUS_METHOD)
{	
	char *tmp_str = NULL;
	DBusMessage *query = NULL, *reply = NULL;
	DBusError err;
    int ret = 1;

	if(NULL == showStr){
		return ret;
	}
	char BUSNAME[PATH_LEN];
	char OBJPATH[PATH_LEN];
	char INTERFACE[PATH_LEN];
	ReInitFemtoDbusPath(index,localid,UDP_IU_DBUS_BUSNAME,BUSNAME);
	ReInitFemtoDbusPath(index,localid,UDP_IU_DBUS_OBJPATH,OBJPATH);
	ReInitFemtoDbusPath(index,localid,UDP_IU_DBUS_INTERFACE,INTERFACE);

	query = dbus_message_new_method_call(BUSNAME,OBJPATH,INTERFACE,DBUS_METHOD);
	
	dbus_error_init(&err);

	reply = dbus_connection_send_with_reply_and_block (dbus_connection,query,-1, &err);

	dbus_message_unref(query);
	if (NULL == reply) {
		printf("failed get reply.\n");
		if (dbus_error_is_set(&err)) {
			printf("%s raised: %s",err.name,err.message);
			dbus_error_free(&err);
		}
		return ret;
	}

	if (dbus_message_get_args ( reply, &err,
					DBUS_TYPE_STRING, &tmp_str,
					DBUS_TYPE_INVALID)) {			
        ret = 0;
	} 
	else {
		printf("Failed get args.\n");
		if (dbus_error_is_set(&err)) {
			printf("%s raised: %s",err.name,err.message);
			dbus_error_free(&err);
		}
	}

	dbus_message_unref(reply);

	if(!ret){
		strncpy(showStr, tmp_str, strlen(tmp_str));
	}
	
	return ret;	
}


unsigned int set_sigtran2udp_enable(int index, int localid, unsigned int enable, DBusConnection *dbus_connection, char *DBUS_METHOD)
{
	DBusMessage *query = NULL, *reply = NULL;
	DBusError err;
	unsigned int op_ret = 0;
	char BUSNAME[PATH_LEN];
	char OBJPATH[PATH_LEN];
	char INTERFACE[PATH_LEN];
	ReInitFemtoDbusPath(index,localid,UDP_IU_DBUS_BUSNAME,BUSNAME);
	ReInitFemtoDbusPath(index,localid,UDP_IU_DBUS_OBJPATH,OBJPATH);
	ReInitFemtoDbusPath(index,localid,UDP_IU_DBUS_INTERFACE,INTERFACE);

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


