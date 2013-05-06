
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <syslog.h>
#include <dbus/dbus.h>

#include "ac_manage_def.h"
#include "ac_manage_extend_interface.h"

void
free_ac_manage_command_return(struct command_return_s **command_return) {
    if(NULL == command_return)
        return ;

    while(*command_return) {
        struct command_return_s *next = (*command_return)->next;
        MANAGE_FREE(*command_return);
        *command_return = next;
    }

    return ;
}


int
ac_manage_exec_extend_command(DBusConnection *connection, 
                                            unsigned int command_type, 
                                            char *command, 
                                            struct command_return_s **command_return) {

    if(NULL == connection || NULL == command || NULL == command_return)
        return AC_MANAGE_INPUT_TYPE_ERROR;

    char *comm = command;  
  
    *command_return = NULL;    
    struct command_return_s *end_return = NULL;
    
	DBusMessage *query, *reply;
	DBusError err;
	DBusMessageIter	 iter;

	int ret = AC_MANAGE_DBUS_ERROR;
    int moreReturn = 0;
    char *returnStr = NULL;

	query = dbus_message_new_method_call(AC_MANAGE_DBUS_DBUSNAME, 
										AC_MANAGE_DBUS_OBJPATH,
										AC_MANAGE_DBUS_INTERFACE,
										AC_MANAGE_DBUS_EXTEND_COMMAND_EXEC);

    dbus_error_init(&err);

    dbus_message_append_args(query,
                            DBUS_TYPE_UINT32, &command_type,
                            DBUS_TYPE_STRING, &comm,
                            DBUS_TYPE_INVALID);

    reply = dbus_connection_send_with_reply_and_block(connection, query, -1, &err);

    dbus_message_unref(query);

    if(NULL == reply) {
        if(dbus_error_is_set(&err)) {
            dbus_error_free(&err);
        }
        return AC_MANAGE_DBUS_ERROR;
    }
    
	dbus_message_iter_init(reply, &iter);
	dbus_message_iter_get_basic(&iter, &ret);
	
    dbus_message_iter_next(&iter);  
	dbus_message_iter_get_basic(&iter, &moreReturn);

    while(AC_MANAGE_SUCCESS == ret && moreReturn) {
    
        dbus_message_iter_next(&iter);  
        dbus_message_iter_get_basic(&iter, &returnStr);

        dbus_message_iter_next(&iter);  
        dbus_message_iter_get_basic(&iter, &moreReturn);

        struct command_return_s *temp_return = (struct command_return_s *)malloc(sizeof(struct command_return_s));
        if(NULL == temp_return) {
            dbus_message_unref(reply);
            syslog(LOG_WARNING, "ac_manage_exec_extend_command: malloc temp_return fail!\n");
            return AC_MANAGE_MALLOC_ERROR;
        }
        memset(temp_return, 0, sizeof(struct command_return_s));
        
        strncpy(temp_return->returnString, returnStr, sizeof(temp_return->returnString) - 1);

        if(NULL == *command_return) {
            *command_return = temp_return;
        }
        else {
            end_return->next = temp_return;
        }
        
        end_return = temp_return;
    }
    
    dbus_message_unref(reply);
    
    return ret;
}

