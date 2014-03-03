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
* nm_dbus.c
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

#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <dbus/dbus.h>
#include "nm_list.h"
#include "nm_errcode.h"
#include "nm_mem.h"
#include "nm_log.h"
#include "nm_dbus.h"

struct nm_dbus {
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
	nm_dbus_method_func method_call_cb;
};

static DBusHandlerResult 
nm_dbus_message_handler( 
		DBusConnection *connection, 
		DBusMessage *message,     
		void *handler_param )
{
	nm_dbus_t *nm_dbus = (nm_dbus_t *)handler_param;
	DBusMessage *reply = NULL;
	struct method *pos, *n;
    
	if (strcmp(dbus_message_get_path(message), nm_dbus->obj_path) == 0) {
    		list_for_each_entry_safe(pos, n, &(nm_dbus->methods), node) {
			if(dbus_message_is_method_call(message,
						pos->method_interface,
						pos->method_name) ) {
				reply = pos->method_call_cb(connection, message, pos->method_param);
                		break;
			}
		}
	}
    
	if (reply) {
		dbus_connection_send (connection, reply, NULL);
		dbus_connection_flush(connection); /* TODO : Maybe we should let main loop process the flush*/
		dbus_message_unref (reply);
	}
	
	return DBUS_HANDLER_RESULT_HANDLED ;
}

nm_dbus_t *
nm_dbus_new( const char *bus_name, 
		const char *obj_path )
{
	int ret;
	DBusError dbus_error;
	nm_dbus_t *nm_dbus = NULL;
	DBusObjectPathVTable vtable = {NULL, 
				nm_dbus_message_handler, NULL, NULL, NULL, NULL};
	

	if( NULL == bus_name || NULL == obj_path 
		|| strlen(bus_name)>sizeof(nm_dbus->bus_name)-1
		|| strlen(obj_path)>sizeof(nm_dbus->obj_path)-1 ){
		nm_log_err("nm_dbus_new param err "\
					"bus_name=%s(len:%d) obj_path=%s(len%d)",
					(NULL!=bus_name)?bus_name:"NULL",
					(NULL!=bus_name)?strlen(bus_name):0,
					(NULL!=obj_path)?obj_path:"NULL",
					(NULL!=obj_path)?strlen(obj_path):0 );
		return NULL;
	}

	nm_dbus = (nm_dbus_t *)nm_malloc(sizeof(nm_dbus_t));
	if( NULL != nm_dbus ){
		memset( nm_dbus, 0, sizeof(nm_dbus_t));
		INIT_LIST_HEAD( &(nm_dbus->methods) );
		
		strncpy( nm_dbus->bus_name, bus_name, 
				sizeof(nm_dbus->bus_name)-1 );
		strncpy( nm_dbus->obj_path, obj_path,
				sizeof(nm_dbus->obj_path)-1 );

		dbus_connection_set_change_sigpipe (TRUE);
		dbus_error_init (&dbus_error);
		nm_dbus->dbus_conn = dbus_bus_get (DBUS_BUS_SYSTEM, 
						&dbus_error);
		if( NULL == nm_dbus->dbus_conn ){
			nm_log_err("nm_dbus get dbus connect failed!");
			goto error1;
		}

	    if (!dbus_connection_register_fallback (nm_dbus->dbus_conn, 
					obj_path, &vtable, nm_dbus)){
	    	nm_log_err("nm_dbus dbus_connection_register_fallback failed!");
	        goto error1;	
	    }

	    ret = dbus_bus_request_name( nm_dbus->dbus_conn, bus_name, 0, &dbus_error );
	    if( -1 == ret ){	
	    	nm_log_err("nm_dbus_new dbus_bus_request_name failed!");
			if ( dbus_error_is_set (&dbus_error) ){
				nm_log_err("nm_dbus_new request bus name %s with error %s", 
								bus_name, dbus_error.message);
	        }
			goto error1;
	    }
	    
	    nm_dbus->pre_state = 1;
		return nm_dbus;
	}	

error1:
	nm_free(nm_dbus);
	return NULL;
}

int 
nm_dbus_reinit(nm_dbus_t *nm_dbus)
{
	DBusConnection *conn = NULL;
	DBusError 	dbus_error;
	
	dbus_connection_set_change_sigpipe (TRUE);
	dbus_error_init (&dbus_error);
	
	conn = dbus_bus_get(DBUS_BUS_SYSTEM, &dbus_error);
	if( NULL == conn ){
		nm_log_err("nm_dbus_conn_reinit get dbus connect failed!");
		return -1;
	}
	dbus_connection_close(nm_dbus->dbus_conn);

	dbus_bus_request_name(conn, nm_dbus->bus_name, 0, &dbus_error);
	if (dbus_error_is_set(&dbus_error)) {
		nm_log_err("nm_dbus_conn_reinit request bus name %s with error %s", 
						nm_dbus->bus_name, dbus_error.message);
		return -1;
	}
	nm_dbus->dbus_conn = conn;
	return 0;
	
}

int
nm_dbus_free( nm_dbus_t *nm_dbus )
{
	struct method *pos,*n;

	list_for_each_entry_safe(pos,n,&(nm_dbus->methods),node){
		list_del(&(pos->node));
		nm_free(pos);
	}
	dbus_connection_close(nm_dbus->dbus_conn);
	nm_free(nm_dbus);
	
	return NM_RETURN_OK;
}

DBusConnection *
nm_dbus_get_dbus_conn( const nm_dbus_t *nm_dbus )
{
	return (NULL!=nm_dbus)?nm_dbus->dbus_conn:NULL;
}

int
nm_dbus_register_method_name( nm_dbus_t *nm_dbus, 
			const char *method_intf, 
			nm_dbus_method_func method_func,
			const char *method_name,
			void *method_param )
{
	struct method *method=NULL;

	if( NULL == nm_dbus 
		|| NULL == method_intf 
		|| strlen(method_intf)>=sizeof(method->method_interface)
		|| NULL == method_func
		|| NULL == method_name
		|| strlen(method_name)>=sizeof(method->method_name) )
	{
		nm_log_err("nm_dbus_register_method_name input err!");
		nm_log_err("  nm_dbus=%p; method_intf=%s(len:%d)",nm_dbus,
						(NULL!=method_intf)?method_intf:"NULL",
						(NULL!=method_intf)?strlen(method_intf):0);
		nm_log_err("  method_func=%p;method_name=%s(len:%d)",method_func,
						(NULL!=method_name)?method_name:"NULL",
						(NULL!=method_name)?strlen(method_name):0);
		return NM_ERR_INPUT_PARAM_ERR;
	}
	method = (struct method *)nm_malloc(sizeof(struct method));
	if( NULL == method )
	{
		nm_log_err("nm_dbus_register_method_name malloc falied!"\
					" method_name=%s", method_name);
		return NM_ERR_MALLOC_FAILED;
	}
	memset( method, 0, sizeof(struct method));
	strncpy( method->method_interface, method_intf,
				sizeof(method->method_interface)-1 );
	strncpy( method->method_name, method_name,
				 sizeof(method->method_name)-1);

	method->method_param = method_param;
	method->method_call_cb = method_func;

	list_add(&(method->node), &(nm_dbus->methods));
	
	return NM_ERR_UNKNOWN;
}
		

void
nm_dbus_dispach( nm_dbus_t *nm_dbus, 
						unsigned int block_usecond )
{
    
    if (nm_dbus && nm_dbus->pre_state) {
    	nm_dbus->pre_state = dbus_connection_read_write_dispatch( 
					nm_dbus->dbus_conn, block_usecond );
		if (!nm_dbus->pre_state ) {/*should it re init dbus connection!?*/
			nm_log_err("nm_dbus_dispach error! nm_dbus->pre_state=%d", 
					nm_dbus->pre_state);
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
	int iret = NM_RETURN_OK;
	DBusMessage *msg=NULL;
	unsigned int serial = 0;

	msg = dbus_message_new_signal(obj_path, 			/* object name of the signal */
								  interface_name,			/* interface name of the signal */
								  signal_name); 		/* name of the signal */
	if (NULL == msg) 
	{
		nm_log_err("dbus_send_signale_v dbus_message_new_signal failed!");
		return NM_ERR_UNKNOWN;
	}

	
	dbus_message_append_args_valist ( msg,
									  first_arg_type,
									  var_args );

	if (!dbus_connection_send(conn, msg, &serial)) 
	{
		dbus_message_unref(msg);
		nm_log_err("dbus_send_signale_v dbus_connection_send failed!");
		return NM_ERR_UNKNOWN;
	}
	
	dbus_connection_flush(conn);
	dbus_message_unref(msg);


	return iret;		
}

int
nm_dbus_send_signal( nm_dbus_t *nm_dbus,
								const char *obj_path,
								const char *interface_name,
								const char *signal_name, 
								int first_arg_type,
								va_list var_args  )																
{
	DBusConnection *conn = NULL;
	int iret = NM_RETURN_OK;
	
	if( NULL == nm_dbus || NULL == obj_path 
		|| NULL == interface_name 
		|| NULL == signal_name )
		
	{
		nm_log_err("nm_dbus_send_signal param err!"\
				" nm_dbus=%p obj_path=%s "\
				"interface_name=%s signal_name = %s",
				nm_dbus,
				(NULL!=obj_path)?obj_path:"NULL",
				(NULL!=interface_name)?interface_name:"NULL",
				(NULL!=signal_name)?signal_name:"NULL");
		return NM_ERR_INPUT_PARAM_ERR;
	}
	conn = nm_dbus_get_dbus_conn(nm_dbus);

	iret = dbus_send_signale_v( conn, 
								obj_path,
								interface_name,
								signal_name,
								first_arg_type,
								var_args );
 
	return iret;
}

#if 0
int
nm_dbus_register_method_name_ex( 
			nm_dbus_t *nm_dbus, 
/*			const char *method_intf, */
			nm_dbus_method_func method_func,
			const char *method_name,
			void *method_param )
{
	struct method *method=NULL;

	if( NULL == nm_dbus 
		|| NULL == method_func
		|| NULL == method_name
		|| strlen(method_name)>=sizeof(method->method_name) )
	{
		nm_log_err("nm_dbus_register_method_name input err!");
		nm_log_err("  nm_dbus=%p;",nm_dbus);
		nm_log_err("  method_func=%p;method_name=%s(len:%d)",method_func,
						(NULL!=method_name)?method_name:"NULL",
						(NULL!=method_name)?strlen(method_name):0);
		return NM_ERR_INPUT_PARAM_ERR;
	}
	method = (struct method *)nm_malloc(sizeof(struct method));
	if( NULL == method )
	{
		nm_log_err("nm_dbus_register_method_name malloc falied!"\
					" method_name=%s", method_name);
		return NM_ERR_MALLOC_FAILED;
	}
	memset( method, 0, sizeof(struct method));
	strncpy( method->method_interface, NM_DBUS_INTERFACE,
				sizeof(method->method_interface)-1 );
	strncpy( method->method_name, method_name,
				 sizeof(method->method_name)-1);

	method->method_param = method_param;
	method->method_call_cb = method_func;

	list_add( &(method->node), &(nm_dbus->methods) );
	
	return NM_ERR_UNKNOWN;
}

#endif

#ifdef nm_debus_test
#include <stdio.h>
#include "nm_mem.c"
#include "nm_log.c"

#define NM_DBUS_TEST_DBUS_NAME		"aw.nmdbustest"
#define NM_DBUS_TEST_DBUS_OBJPATH	"/aw/nmdbustest"
#define NM_DBUS_TEST_DBUS_INTERFACE    "aw.nmdbustest"


DBusMessage *nm_dbus_method_functest1(    
			DBusConnection *conn,     
			DBusMessage *msg,     
			void *method_param)
{
	return NULL;
}


DBusMessage *nm_dbus_method_functest2(    
			DBusConnection *conn,     
			DBusMessage *msg,     
			void *method_param)
{
	return NULL;
}

DBusMessage *nm_dbus_method_functest3(    
			DBusConnection *conn,     
			DBusMessage *msg,     
			void *method_param)
{
	return NULL;
}

int
main()
{
	nm_dbus_t *nm_dbus;
	nm_dbus = nm_dbus_new( NM_DBUS_TEST_DBUS_NAME, 
				NM_DBUS_TEST_DBUS_INTERFACE );
	if( NULL == nm_dbus )
	{
		printf("create nm dbus failed!");
		return -1;
	}

	while(1)
	{
		nm_dbus_dispach(nm_dbus,5000);
	}
	
	return 0;
}

#endif

