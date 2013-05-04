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
 *
 *
 * CREATOR:
 * autelan.software.xxx. team
 *
 * DESCRIPTION:
 * xxx module main routine
 *
 *
 *******************************************************************************/

/* drp_dbus.c */

#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <dbus/dbus.h>
#include "nm_list.h"

#include "nm_list.h"
#include "drp_def.h"
#include "drp_log.h"
#include "drp_mem.h"
#include "drp_dbus.h"
#include "drp_opxml.h"
#include "drp_handle.h"


struct drp_dbus{
	int pre_state;
	DBusConnection *dbus_conn;
	char bus_name[MAX_DBUS_BUSNAME_LEN];
	char obj_path[MAX_DBUS_OBJPATH_LEN];
	struct list_head methods;
};

struct method{
	struct list_head node;
	char method_interface[MAX_DBUS_INTERFACE_LEN];
	char method_name[MAX_DBUS_METHOD_LEN];
	void *method_param;
	drp_dbus_method_func method_call_cb;
};


static DBusHandlerResult 
drp_dbus_message_handler( 
		DBusConnection *connection, 
		DBusMessage *message,     
		void *handler_param )
{
	drp_dbus_t *drp_dbus = (drp_dbus_t *)handler_param;
	DBusMessage 	*reply = NULL;
	struct method *pos,*n;

	if (strcmp(dbus_message_get_path(message),drp_dbus->obj_path) == 0){
		list_for_each_entry_safe(pos,n,&(drp_dbus->methods),node){
			if( dbus_message_is_method_call(message,
						pos->method_interface,
						pos->method_name) ){
				reply = pos->method_call_cb(connection, 
						message, pos->method_param);
				break;
			}
		}
	}

	if (reply){
		dbus_connection_send (connection, reply, NULL);
		dbus_connection_flush(connection); /* TODO	  Maybe we should let main loop process the flush*/
		dbus_message_unref (reply);
	}

	return DBUS_HANDLER_RESULT_HANDLED ;
}

	drp_dbus_t *
drp_dbus_new( const char *bus_name, 
		const char *obj_path )
{
	int ret;
	DBusError 	dbus_error;
	drp_dbus_t *drp_dbus = NULL;
	DBusObjectPathVTable	vtable = {NULL, 
		drp_dbus_message_handler, NULL, NULL, NULL, NULL};


	if( NULL == bus_name || NULL == obj_path 
			|| strlen(bus_name)>sizeof(drp_dbus->bus_name)-1
			|| strlen(obj_path)>sizeof(drp_dbus->obj_path)-1 ){
		drp_log_err("drp_dbus_new param err "\
				"bus_name=%s(len:%d) obj_path=%s(len%d)",
				(NULL!=bus_name)?bus_name:"NULL",
				(NULL!=bus_name)?strlen(bus_name):0,
				(NULL!=obj_path)?obj_path:"NULL",
				(NULL!=obj_path)?strlen(obj_path):0 );
		return NULL;
	}

	drp_dbus = (drp_dbus_t *)drp_malloc(sizeof(drp_dbus_t));
	if( NULL != drp_dbus ){
		memset( drp_dbus, 0, sizeof(drp_dbus_t));
		INIT_LIST_HEAD( &(drp_dbus->methods) );

		strncpy( drp_dbus->bus_name, bus_name, 
				sizeof(drp_dbus->bus_name)-1 );
		strncpy( drp_dbus->obj_path, obj_path,
				sizeof(drp_dbus->obj_path)-1 );

		dbus_connection_set_change_sigpipe (TRUE);
		dbus_error_init (&dbus_error);
		drp_dbus->dbus_conn = dbus_bus_get (DBUS_BUS_SYSTEM, 
				&dbus_error);
		if( NULL == drp_dbus->dbus_conn ){
			drp_log_err("drp_dbus get dbus connect failed!");
			goto error1;
		}

		if (!dbus_connection_register_fallback (drp_dbus->dbus_conn, 
					obj_path, &vtable, drp_dbus)){
			drp_log_err("drp_dbus dbus_connection_register_fallback failed!");
			goto error1;	
		}

		ret = dbus_bus_request_name( drp_dbus->dbus_conn, bus_name, 0, &dbus_error );
		if( -1 == ret ){	
			drp_log_err("drp_dbus_new dbus_bus_request_name failed!");
			if ( dbus_error_is_set (&dbus_error) ){
				drp_log_err("drp_dbus_new request bus name %s with error %s", 
						bus_name, dbus_error.message);
			}
			goto error1;
		}
	}

	drp_dbus->pre_state = 1;
	return drp_dbus;

error1:
	drp_free(drp_dbus);
	return NULL;
}

	int
drp_dbus_free( drp_dbus_t *drp_dbus )
{
	struct method *pos,*n;

	list_for_each_entry_safe(pos,n,&(drp_dbus->methods),node){
		list_del(&(pos->node));
		drp_free(pos);
	}

	drp_free(drp_dbus);

	return DRP_RETURN_OK;
}

	DBusConnection *
drp_dbus_get_dbus_conn( const drp_dbus_t *drp_dbus )
{
	return (NULL!=drp_dbus)?drp_dbus->dbus_conn:NULL;
}

	int
drp_dbus_register_method_name( drp_dbus_t *drp_dbus, 
		const char *method_intf, 
		drp_dbus_method_func method_func,
		const char *method_name,
		void *method_param )
{
	struct method *method=NULL;

	if( NULL == drp_dbus 
			|| NULL == method_intf 
			|| strlen(method_intf)>=sizeof(method->method_interface)
			|| NULL == method_func
			|| NULL == method_name
			|| strlen(method_name)>=sizeof(method->method_name) )
	{
		drp_log_err("drp_dbus_register_method_name input err!");
		drp_log_err("  drp_dbus=%p; method_intf=%s(len:%d)",drp_dbus,
				(NULL!=method_intf)?method_intf:"NULL",
				(NULL!=method_intf)?strlen(method_intf):0);
		drp_log_err("  method_func=%p;method_name=%s(len:%d)",method_func,
				(NULL!=method_name)?method_name:"NULL",
				(NULL!=method_name)?strlen(method_name):0);
		return DRP_ERR_INPUT_PARAM_ERR;
	}
	method = (struct method *)drp_malloc(sizeof(struct method));
	if( NULL == method )
	{
		drp_log_err("drp_dbus_register_method_name malloc falied!"\
				" method_name=%s", method_name);
		return DRP_ERR_MALLOC_FAILED;
	}
	memset( method, 0, sizeof(struct method));
	strncpy( method->method_interface, method_intf,
			sizeof(method->method_interface)-1 );
	strncpy( method->method_name, method_name,
			sizeof(method->method_name)-1);

	method->method_param = method_param;
	method->method_call_cb = method_func;

	list_add( &(method->node), &(drp_dbus->methods) );

	return DRP_ERR_UNKNOWN;
}

	void
drp_dbus_dispach( drp_dbus_t *drp_dbus, 
		unsigned int block_usecond )
{
	if( drp_dbus && drp_dbus->pre_state )
	{
		drp_dbus->pre_state = 
			dbus_connection_read_write_dispatch( 
					drp_dbus->dbus_conn, block_usecond );
		if( !drp_dbus->pre_state )/*should it re init dbus connection!?*/
		{
			drp_log_err("drp_dbus_dispach error! drp_dbus->pre_state=%d", 
					drp_dbus->pre_state);
		}
	}	 
	return;
}

	static int
dbus_send_signale_v( DBusConnection *conn,
		const char *obj_path,
		const char *interface_name,
		const char *signal_name, 
		int first_arg_type,
		va_list var_args  )
{
	int iret = DRP_RETURN_OK;
	DBusMessage *msg=NULL;
	unsigned int serial = 0;

	msg = dbus_message_new_signal(obj_path, 			/* object name of the signal */
			interface_name,			/* interface name of the signal */
			signal_name); 		/* name of the signal */
	if (NULL == msg) 
	{
		drp_log_err("dbus_send_signale_v dbus_message_new_signal failed!");
		return DRP_ERR_UNKNOWN;
	}


	dbus_message_append_args_valist ( msg,
			first_arg_type,
			var_args );

	if (!dbus_connection_send(conn, msg, &serial)) 
	{
		dbus_message_unref(msg);
		drp_log_err("dbus_send_signale_v dbus_connection_send failed!");
		return DRP_ERR_UNKNOWN;
	}

	dbus_connection_flush(conn);
	dbus_message_unref(msg);

	return iret;		
}

	int
drp_dbus_send_signal( const drp_dbus_t *drp_dbus,
		const char *obj_path,
		const char *interface_name,
		const char *signal_name, 
		int first_arg_type,...)

{
	DBusConnection *conn = NULL;
	va_list var_args;
	int iret = DRP_RETURN_OK;

	if( NULL == drp_dbus || NULL == obj_path 
			|| NULL == interface_name 
			|| NULL == signal_name )

	{
		drp_log_err("drp_dbus_send_signal param err!"\
				" drp_dbus=%p obj_path=%s "\
				"interface_name=%s signal_name = %s",
				drp_dbus,
				(NULL!=obj_path)?obj_path:"NULL",
				(NULL!=interface_name)?interface_name:"NULL",
				(NULL!=signal_name)?signal_name:"NULL");
		return DRP_ERR_INPUT_PARAM_ERR;
	}
	conn = drp_dbus->dbus_conn;

	va_start ( var_args, first_arg_type );
	iret = dbus_send_signale_v( drp_dbus->dbus_conn, 
			obj_path,
			interface_name,
			signal_name,
			first_arg_type,
			var_args );
	va_end (var_args);

	return iret;
}





