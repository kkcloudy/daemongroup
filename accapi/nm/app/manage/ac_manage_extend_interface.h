#ifndef _AC_MANAGE_EXTEND_INTERFACE_H_
#define _AC_MANAGE_EXTEND_INTERFACE_H_


struct command_return_s {
    char returnString[AC_MANAGE_LINE_SIZE];
    struct command_return_s *next;
};

void 
free_ac_manage_command_return(struct command_return_s **command_return);


int
ac_manage_exec_extend_command(DBusConnection *connection, 
                                            unsigned int command_type, 
                                            char *command, 
                                            struct command_return_s **command_return);


#endif
