#ifdef __cplusplus
extern "C"
{
#endif
#include <stdio.h>
#include <string.h>
#include "dbus/dbus.h"
#define DEMO_DBUS_BUSNAME				"aw.demo"
#define DEMO_DBUS_OBJPATH				"/aw/demo"
#define DEMO_DBUS_INTERFACE				"aw.demo"
#define DEMO_DBUS_METHOD_GET_SLOT_NUM 	"demo_get_slot"

DBusConnection *demo_dbus_connection = NULL;

/******************************************************************/

int get_local_listen_num()
{
	int fp;
	int tipc_connect_num=-1;
	fp=fopen("/mnt/slot_num_file","r");
	if(fp <=0 ){
		fprintf(stderr,"open slot_num_file error\n");
		return -1;
	}
	fscanf(fp,"%d",&tipc_connect_num);
	close(fp);
	return tipc_connect_num;
}
/***********************************************************************/
DBusMessage *dbus_get_slot_num(
		DBusConnection	*conn,
		DBusMessage			*msg,
		void						*user_data
		)
{
	DBusMessage			*reply = NULL;
	DBusMessageIter iter;
	DBusError				err;
	
	unsigned int slot_num;
	unsigned char *showstr = NULL;

	//showstr = (unsigned char*)malloc(1024);
	//memset(showstr,0,1024);

	slot_num = get_local_listen_num();
	if(-1 == slot_num)
	{
		fprintf(stderr,"get_local_listen_num() error\n");
		exit(-1);
	}
	dbus_error_init(&err);
	if(!(dbus_message_get_args(msg,&err,
														 DBUS_TYPE_STRING,&showstr,
														 DBUS_TYPE_INVALID)))
	{
		fprintf(stderr,"Unable to get input args\n");
		if (dbus_error_is_set(&err)){
			fprintf(stderr,"%s raised: %s",err.name,err.message);
			dbus_error_free(&err);
		}
	}
		fprintf(stderr,"[demo]:get dbus message is %s\n",showstr);
		reply = dbus_message_new_method_return(msg);
		dbus_message_iter_init_append(reply,&iter);
		dbus_message_iter_append_basic(&iter,
																		DBUS_TYPE_UINT32, &slot_num);
		return reply;
}	

static DBusHandlerResult tipc_dbus_demon_message_handler 
(
	DBusConnection *connection, 
	DBusMessage *message, 
	void *user_data
)
{
	DBusMessage 	*reply = NULL;

	if(strcmp(dbus_message_get_path(message), DEMO_DBUS_OBJPATH) == 0) {	
		if (dbus_message_is_method_call(message,
										DEMO_DBUS_INTERFACE,
										DEMO_DBUS_METHOD_GET_SLOT_NUM))
		{
			reply = dbus_get_slot_num(connection, message, user_data);
		}
	
	if (reply) {
		dbus_connection_send (connection, reply, NULL);
		dbus_connection_flush(connection); /* TODO	  Maybe we should let main loop process the flush*/
		dbus_message_unref (reply);
	}

/*	dbus_message_unref(message); //TODO who should unref the incoming message? */
	return DBUS_HANDLER_RESULT_HANDLED ;
}
}

DBusHandlerResult demo_dbus_filter_function 
(
	DBusConnection * connection,
	DBusMessage * message, 
	void *user_data
)
{
	if (dbus_message_is_signal (message, DBUS_INTERFACE_LOCAL, "Disconnected") &&
		   strcmp (dbus_message_get_path (message), DBUS_PATH_LOCAL) == 0) {

		/* this is a local message; e.g. from libdbus in this process */

		dbus_connection_unref (demo_dbus_connection);
		demo_dbus_connection = NULL;

		/*g_timeout_add (3000, reinit_dbus, NULL);*/

	} else if (dbus_message_is_signal (message,
			      DBUS_INTERFACE_DBUS,
			      "NameOwnerChanged")) {
		;
	} else
		return TRUE;

	return DBUS_HANDLER_RESULT_HANDLED;
}

 /*************************************************************************/
int demo_dbus_init(void)
{
	DBusError dbus_error;
	DBusObjectPathVTable	demo_vtable = {NULL, &tipc_dbus_demon_message_handler, NULL, NULL, NULL, NULL};
	
	dbus_error_init (&dbus_error);
	demo_dbus_connection = dbus_bus_get (DBUS_BUS_SYSTEM, &dbus_error);
	if (demo_dbus_connection == NULL) {
		fprintf (stderr,"init dbus dbus_bus_get(): %s", dbus_error.message);
		return FALSE;
	}

	if (!dbus_connection_register_fallback (demo_dbus_connection, DEMO_DBUS_OBJPATH, &demo_vtable, NULL)) {
		fprintf(stderr,"can't register D-BUS handlers (fallback dhcpsnp). cannot continue.\n");
		return FALSE;
		
	}
		
	dbus_bus_request_name (demo_dbus_connection, DEMO_DBUS_BUSNAME,
			       0, &dbus_error);
	
	
	if (dbus_error_is_set (&dbus_error)) {
		fprintf (stderr,"dbus request bus name error: %s\n",
			    dbus_error.message);
		return FALSE;
	}

	dbus_connection_add_filter (demo_dbus_connection, demo_dbus_filter_function, NULL, NULL);

	dbus_bus_add_match (demo_dbus_connection,
			    "type='signal'"
			    ",interface='"DBUS_INTERFACE_DBUS"'"
			    ",sender='"DBUS_SERVICE_DBUS"'"
			    ",member='NameOwnerChanged'",
			    NULL);
	return TRUE;
}

 /*************************************************************************/
int main(void)
{
	demo_dbus_init();
	while(dbus_connection_read_write_dispatch(demo_dbus_connection,-1)){
		;
	}
	return 0;
}

#ifdef __cplusplus
}
#endif
