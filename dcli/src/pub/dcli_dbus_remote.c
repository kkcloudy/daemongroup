#include <string.h>
#include <stdbool.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <syslog.h>

#include <sys/stat.h>
#include <sys/wait.h>
#include <fcntl.h>

#include <dbus/dbus.h>


/*zhaocg add for supporting libpub.so */

#if 1
#define MAX_SLOT 					16
struct dbus_connection
{
	DBusConnection *dcli_dbus_connection;
	int 			slot_id;
	int 			board_type;
	int 			board_state;
	};
typedef struct dbus_connection dbus_connection;
#endif

int dcli_dbus_init_remote(void);
void dbus_error_free_for_dcli(DBusError *error);

dbus_connection *dbus_connection_dcli[MAX_SLOT];


int dbus_connection_remote_init(dbus_connection** connection)
{
	if(NULL != (*connection))
	{	
		if(NULL != (*connection) -> dcli_dbus_connection)
			dbus_connection_close((*connection) -> dcli_dbus_connection);
		free(*connection);
		(*connection) = NULL;
	}

	(*connection) = (dbus_connection*)malloc(sizeof(dbus_connection));
	
	if(NULL == (*connection))
	{
		syslog(LOG_INFO,"malloc error\n");
		return -1;
	}

	(*connection) -> dcli_dbus_connection	= NULL;
	(*connection) -> slot_id				= -1;
	(*connection) -> board_type				= -1;
	(*connection) -> board_state			= -1;

}

int dbus_connection_init_all(dbus_connection** dbus_connection)
{
	int i = 0;
	if(NULL == dbus_connection)
	{
		syslog(LOG_INFO,"ERROR:dbus_connection_init_all:dbus_connection = NULL\n");
		return -1;
	}

	for(i = 0;i < MAX_SLOT;i++)
	{
		if(0 == dbus_connection_remote_init(&dbus_connection[i]))
		{
			syslog(LOG_INFO,"ERROR:dbus_connection_init_all:init connection %d error\n",i);
			return -1;
		}
	}
	return 0;
}
int dbus_connection_register(int slot_id,dbus_connection** connection)
{
	
	DBusError dbus_error;	
	dbus_error_init (&dbus_error);
	
	if(slot_id > MAX_SLOT)
	{
		syslog(LOG_INFO,"ERROR:dbus_connection_register:error slot_id\n");
		return -1;
	}
	
	if((*connection) == NULL)
	{
		syslog(LOG_INFO,"ERROR:dbus_connection_register:connection is NULL\n");
		return -1;
	}

	(*connection) -> slot_id 			= slot_id;
	(*connection) -> board_type			= -1;
	(*connection) -> board_state			= -1;
	(*connection) -> dcli_dbus_connection 	= dbus_bus_get_remote(DBUS_BUS_SYSTEM,slot_id,&dbus_error);

	if((*connection) -> dcli_dbus_connection == NULL)
	{
		syslog(LOG_INFO,"dbus_bus_get(): %s", dbus_error.message);
		return -1;
		
	}
	
	return 0;
	
	
}
int dbus_connection_register_all(dbus_connection** dbus_connection)
{
	int i = 0;	
	int fd=0;	
	int product_serial = 0;	
	int max_number = 0;

	
	fd = fopen("/dbm/product/slotcount", "r");
	if(0==fd)
	{
		max_number=15;
	}else{
		fscanf(fd, "%d", &max_number);
		fclose(fd);
	}
#if 0
	if(8 == product_serial)
	{
		max_number = 15;
	}
	else if(7 == product_serial)
	{
		max_number = 3;
	}
#endif
	
	if(NULL == dbus_connection)
	{
		syslog(LOG_INFO,"ERROR:dbus_connection_register_all:dbus_connection = NULL\n");
		return -1;
	}
	for(i = 1;i <= max_number;i++)
	{
		
		syslog(LOG_INFO,"\n===============connect slot %d ===================\n",i);
		if(-1 == dbus_connection_register(i,&dbus_connection[i]))
		{
			syslog(LOG_INFO,"ERROR:dbus-connection_register_all:connect slot %d error\n",i);
			continue;
		}
	}
	return 0;
}
int dbus_connection_free(dbus_connection* connection)
{
	free(connection);
	connection = NULL;
	return 0;
}

int dbus_connection_free_all(dbus_connection** connection)
{
	int i = 0;
	for(i = 0;i < MAX_SLOT;i++)
	{
		dbus_connection_free(connection[i]);
	}

	return 0;
}

int dcli_dbus_init_remote(void) 
{
	if(dbus_connection_init_all(dbus_connection_dcli) == -1)
	{
		return FALSE;
	}

	if(dbus_connection_register_all(dbus_connection_dcli) == -1)
	{
		return FALSE;
	}
	return TRUE;
}

void dbus_error_free_for_dcli(DBusError *error)
{
	if (dbus_error_is_set(error)) {
		syslog(LOG_NOTICE,"dbus connection of dcli error ,reinit it\n");
		
		syslog(LOG_NOTICE,"%s raised: %s\n",error->name,error->message);
		dcli_dbus_init_remote();
		
	}
	dbus_error_free(error);
	
}

