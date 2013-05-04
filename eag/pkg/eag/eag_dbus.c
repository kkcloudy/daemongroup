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
* portal_ha.c
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

/* eag_log.c */

#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <dbus/dbus.h>
#include "nm_list.h"
#include "eag_errcode.h"
#include "eag_mem.h"
#include "eag_log.h"
#include "eag_dbus.h"

#include "had_vrrpd.h"

struct eag_dbus{
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
	eag_dbus_method_func method_call_cb;
};

static DBusHandlerResult 
eag_dbus_message_handler( 
		DBusConnection *connection, 
		DBusMessage *message,     
		void *handler_param )
{
	eag_dbus_t *eag_dbus = (eag_dbus_t *)handler_param;
    DBusMessage 	*reply = NULL;
	struct method *pos,*n;
    
    if (strcmp(dbus_message_get_path(message),eag_dbus->obj_path) == 0){
        list_for_each_entry_safe(pos,n,&(eag_dbus->methods),node){
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






eag_dbus_t *
eag_dbus_new( const char *bus_name, 
		const char *obj_path )
{
	int ret;
    DBusError 	dbus_error;
	eag_dbus_t *eag_dbus = NULL;
    DBusObjectPathVTable	vtable = {NULL, 
				eag_dbus_message_handler, NULL, NULL, NULL, NULL};
	

	if( NULL == bus_name || NULL == obj_path 
		|| strlen(bus_name)>sizeof(eag_dbus->bus_name)-1
		|| strlen(obj_path)>sizeof(eag_dbus->obj_path)-1 ){
		eag_log_err("eag_dbus_new param err "\
					"bus_name=%s(len:%d) obj_path=%s(len%d)",
					(NULL!=bus_name)?bus_name:"NULL",
					(NULL!=bus_name)?strlen(bus_name):0,
					(NULL!=obj_path)?obj_path:"NULL",
					(NULL!=obj_path)?strlen(obj_path):0 );
		return NULL;
	}

	eag_dbus = (eag_dbus_t *)eag_malloc(sizeof(eag_dbus_t));
	if( NULL != eag_dbus ){
		memset( eag_dbus, 0, sizeof(eag_dbus_t));
		INIT_LIST_HEAD( &(eag_dbus->methods) );
		
		strncpy( eag_dbus->bus_name, bus_name, 
				sizeof(eag_dbus->bus_name)-1 );
		strncpy( eag_dbus->obj_path, obj_path,
				sizeof(eag_dbus->obj_path)-1 );

		dbus_connection_set_change_sigpipe (TRUE);
		dbus_error_init (&dbus_error);
		eag_dbus->dbus_conn = dbus_bus_get (DBUS_BUS_SYSTEM, 
						&dbus_error);
		if( NULL == eag_dbus->dbus_conn ){
			eag_log_err("eag_dbus get dbus connect failed!");
			goto error1;
		}

	    if (!dbus_connection_register_fallback (eag_dbus->dbus_conn, 
					obj_path, &vtable, eag_dbus)){
	    	eag_log_err("eag_dbus dbus_connection_register_fallback failed!");
	        goto error1;	
	    }

	    ret = dbus_bus_request_name( eag_dbus->dbus_conn, bus_name, 0, &dbus_error );
	    if( -1 == ret ){	
	    	eag_log_err("eag_dbus_new dbus_bus_request_name failed!");
			if ( dbus_error_is_set (&dbus_error) ){
				eag_log_err("eag_dbus_new request bus name %s with error %s", 
								bus_name, dbus_error.message);
	        }
			goto error1;
	    }
	    
	    eag_dbus->pre_state = 1;
		return eag_dbus;
	}	

error1:
	eag_free(eag_dbus);
	return NULL;
}

int 
eag_dbus_reinit(eag_dbus_t *eag_dbus)
{
	DBusConnection *conn = NULL;
	DBusError 	dbus_error;
	
	dbus_connection_set_change_sigpipe (TRUE);
	dbus_error_init (&dbus_error);
	
	conn = dbus_bus_get(DBUS_BUS_SYSTEM, &dbus_error);
	if( NULL == conn ){
		eag_log_err("eag_dbus_conn_reinit get dbus connect failed!");
		return -1;
	}
	
	dbus_bus_request_name(conn, eag_dbus->bus_name, 0, &dbus_error);
	if (dbus_error_is_set(&dbus_error)) {
		eag_log_err("eag_dbus_conn_reinit request bus name %s with error %s", 
						eag_dbus->bus_name, dbus_error.message);
		return -1;
	}

	eag_dbus->dbus_conn = conn;
	return 0;
	
}


int
eag_dbus_free( eag_dbus_t *eag_dbus )
{
	struct method *pos,*n;

	list_for_each_entry_safe(pos,n,&(eag_dbus->methods),node){
		list_del(&(pos->node));
		eag_free(pos);
	}

	eag_free(eag_dbus);
	
	return EAG_RETURN_OK;
}


DBusConnection *
eag_dbus_get_dbus_conn( const eag_dbus_t *eag_dbus )
{
	return (NULL!=eag_dbus)?eag_dbus->dbus_conn:NULL;
}


int
eag_dbus_register_method_name( eag_dbus_t *eag_dbus, 
			const char *method_intf, 
			eag_dbus_method_func method_func,
			const char *method_name,
			void *method_param )
{
	struct method *method=NULL;

	if( NULL == eag_dbus 
		|| NULL == method_intf 
		|| strlen(method_intf)>=sizeof(method->method_interface)
		|| NULL == method_func
		|| NULL == method_name
		|| strlen(method_name)>=sizeof(method->method_name) )
	{
		eag_log_err("eag_dbus_register_method_name input err!");
		eag_log_err("  eag_dbus=%p; method_intf=%s(len:%d)",eag_dbus,
						(NULL!=method_intf)?method_intf:"NULL",
						(NULL!=method_intf)?strlen(method_intf):0);
		eag_log_err("  method_func=%p;method_name=%s(len:%d)",method_func,
						(NULL!=method_name)?method_name:"NULL",
						(NULL!=method_name)?strlen(method_name):0);
		return EAG_ERR_INPUT_PARAM_ERR;
	}
	method = (struct method *)eag_malloc(sizeof(struct method));
	if( NULL == method )
	{
		eag_log_err("eag_dbus_register_method_name malloc falied!"\
					" method_name=%s", method_name);
		return EAG_ERR_MALLOC_FAILED;
	}
	memset( method, 0, sizeof(struct method));
	strncpy( method->method_interface, method_intf,
				sizeof(method->method_interface)-1 );
	strncpy( method->method_name, method_name,
				 sizeof(method->method_name)-1);

	method->method_param = method_param;
	method->method_call_cb = method_func;

	list_add( &(method->node), &(eag_dbus->methods) );
	
	return EAG_ERR_UNKNOWN;
}
		





void
eag_dbus_dispach( eag_dbus_t *eag_dbus, 
						unsigned int block_usecond )
{
    
    if( eag_dbus && eag_dbus->pre_state )
    {
#if 0    
		static unsigned long prevtime = 0;
		unsigned long curtime = time(NULL);
		if( curtime != prevtime){
			eag_log_err("eag_dbus_dispach heartbeat! eag_dbus->dbus_conn=%p",
						eag_dbus->dbus_conn);
			prevtime = curtime;
		}
#endif
		eag_dbus->pre_state = 
				dbus_connection_read_write_dispatch( 
					eag_dbus->dbus_conn, block_usecond );
		if( !eag_dbus->pre_state )/*should it re init dbus connection!?*/
		{
			eag_log_err("eag_dbus_dispach error! eag_dbus->pre_state=%d", 
					eag_dbus->pre_state);
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
	int iret = EAG_RETURN_OK;
	DBusMessage *msg=NULL;
	unsigned int serial = 0;

	msg = dbus_message_new_signal(obj_path, 			/* object name of the signal */
								  interface_name,			/* interface name of the signal */
								  signal_name); 		/* name of the signal */
	if (NULL == msg) 
	{
		eag_log_err("dbus_send_signale_v dbus_message_new_signal failed!");
		return EAG_ERR_UNKNOWN;
	}

	
	dbus_message_append_args_valist ( msg,
									  first_arg_type,
									  var_args );

	if (!dbus_connection_send(conn, msg, &serial)) 
	{
		dbus_message_unref(msg);
		eag_log_err("dbus_send_signale_v dbus_connection_send failed!");
		return EAG_ERR_UNKNOWN;
	}
	
	dbus_connection_flush(conn);
	dbus_message_unref(msg);


	return iret;		
}

int
eag_dbus_send_signal( eag_dbus_t *eag_dbus,
								const char *obj_path,
								const char *interface_name,
								const char *signal_name, 
								int first_arg_type,
								va_list var_args  )																
{
	DBusConnection *conn = NULL;
	int iret = EAG_RETURN_OK;
	
	if( NULL == eag_dbus || NULL == obj_path 
		|| NULL == interface_name 
		|| NULL == signal_name )
		
	{
		eag_log_err("eag_dbus_send_signal param err!"\
				" eag_dbus=%p obj_path=%s "\
				"interface_name=%s signal_name = %s",
				eag_dbus,
				(NULL!=obj_path)?obj_path:"NULL",
				(NULL!=interface_name)?interface_name:"NULL",
				(NULL!=signal_name)?signal_name:"NULL");
		return EAG_ERR_INPUT_PARAM_ERR;
	}
	conn = eag_dbus_get_dbus_conn(eag_dbus);

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
eag_dbus_register_method_name_ex( 
			eag_dbus_t *eag_dbus, 
/*			const char *method_intf, */
			eag_dbus_method_func method_func,
			const char *method_name,
			void *method_param )
{
	struct method *method=NULL;

	if( NULL == eag_dbus 
		|| NULL == method_func
		|| NULL == method_name
		|| strlen(method_name)>=sizeof(method->method_name) )
	{
		eag_log_err("eag_dbus_register_method_name input err!");
		eag_log_err("  eag_dbus=%p;",eag_dbus);
		eag_log_err("  method_func=%p;method_name=%s(len:%d)",method_func,
						(NULL!=method_name)?method_name:"NULL",
						(NULL!=method_name)?strlen(method_name):0);
		return EAG_ERR_INPUT_PARAM_ERR;
	}
	method = (struct method *)eag_malloc(sizeof(struct method));
	if( NULL == method )
	{
		eag_log_err("eag_dbus_register_method_name malloc falied!"\
					" method_name=%s", method_name);
		return EAG_ERR_MALLOC_FAILED;
	}
	memset( method, 0, sizeof(struct method));
	strncpy( method->method_interface, EAG_DBUS_INTERFACE,
				sizeof(method->method_interface)-1 );
	strncpy( method->method_name, method_name,
				 sizeof(method->method_name)-1);

	method->method_param = method_param;
	method->method_call_cb = method_func;

	list_add( &(method->node), &(eag_dbus->methods) );
	
	return EAG_ERR_UNKNOWN;
}

#endif

#ifdef eag_debus_test
#include <stdio.h>
#include "eag_mem.c"
#include "eag_log.c"

#define EAG_DBUS_TEST_DBUS_NAME		"aw.eagdbustest"
#define EAG_DBUS_TEST_DBUS_OBJPATH	"/aw/eagdbustest"
#define EAG_DBUS_TEST_DBUS_INTERFACE    "aw.eagdbustest"


DBusMessage *eag_dbus_method_functest1(    
			DBusConnection *conn,     
			DBusMessage *msg,     
			void *method_param)
{
	return NULL;
}


DBusMessage *eag_dbus_method_functest2(    
			DBusConnection *conn,     
			DBusMessage *msg,     
			void *method_param)
{
	return NULL;
}

DBusMessage *eag_dbus_method_functest3(    
			DBusConnection *conn,     
			DBusMessage *msg,     
			void *method_param)
{
	return NULL;
}



int
main()
{
	eag_dbus_t *eag_dbus;
	eag_dbus = eag_dbus_new( EAG_DBUS_TEST_DBUS_NAME, 
				EAG_DBUS_TEST_DBUS_INTERFACE );
	if( NULL == eag_dbus )
	{
		printf("create eag dbus failed!");
		return -1;
	}

	while(1)
	{
		eag_dbus_dispach(eag_dbus,5000);
	}
	
	return 0;
}

#endif

