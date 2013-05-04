
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <syslog.h>

#include "ac_manage_def.h"

#include "ac_manage_extend.h"


static int
manage_command_parse(char *command) {

    if(NULL == command) {
        return AC_MANAGE_INPUT_TYPE_ERROR;
    }

    syslog(LOG_DEBUG, "manage_command_parse: command before parse %s\n", command);
    
    int i = 0;
    unsigned long len = strlen(command);
    
    for(i = 0; i < len; i++) {
        if('$' == command[i]) {
            command[i] = '\n';
        }
    }
    
    syslog(LOG_DEBUG, "manage_command_parse: command after parse %s\n", command);

    return AC_MANAGE_SUCCESS;
}

int
manage_command_exec(unsigned int command_type, char *command, FILE **fp) {

    if(NULL == command || NULL == fp) {
        return AC_MANAGE_INPUT_TYPE_ERROR;
    }
    *fp = NULL;
    
    
    if(AC_MANAGE_SUCCESS != manage_command_parse(command)) {
        syslog(LOG_WARNING, "manage_command_exec: manage_command_parse %s fail\n", command);
        return AC_MANAGE_INPUT_TYPE_ERROR;
    }

    switch(command_type) {
        case AC_MANAGE_EXTEND_COMMAND_DCLI:
            {
                unsigned int len = strlen(command) + 64;
                char *temp_comm = (char *)malloc(len);
                if(NULL == temp_comm) {
                    syslog(LOG_WARNING, "manage_command_exec: malloc temp_comm fail\n");
                    return AC_MANAGE_MALLOC_ERROR;
                }

                memset(temp_comm, 0, len);

                snprintf(temp_comm, len - 1,"/opt/bin/vtysh -c 'configure terminal %c %s'", '\n', command);
                
                syslog(LOG_DEBUG, "manage_command_exec: exec dcli command %s\n", temp_comm);

                FILE *temp_fp = NULL;
                if(NULL == (temp_fp = popen(temp_comm, "r"))) {
                    MANAGE_FREE(temp_comm);
                    syslog(LOG_WARNING, "manage_command_exec: popen %s fail\n", temp_comm);
                    return AC_MANAGE_FILE_OPEN_FAIL;
                }

                MANAGE_FREE(temp_comm);                
                *fp = temp_fp;
            }
            break;
            
        case AC_MANAGE_EXTEND_COMMAND_SYSTEM:
            {
                FILE *temp_fp = NULL;
                
                syslog(LOG_DEBUG, "manage_command_exec: exec system command %s\n", command);
                
                if(NULL == (temp_fp = popen(command, "r"))) {
                    syslog(LOG_WARNING, "manage_command_exec: popen %s fail\n", command);
                    return AC_MANAGE_FILE_OPEN_FAIL;
                }

                *fp = temp_fp;
            }
            
            break;
            
        default :
            syslog(LOG_INFO, "manage_command_exec: unknow command type (%d) \n", command_type);
            break;
    }
    
    return AC_MANAGE_SUCCESS;
}





