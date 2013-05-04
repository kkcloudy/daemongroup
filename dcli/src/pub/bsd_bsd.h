#ifndef _WID_BSD_H
#define _WID_BSD_H

#define CMD_FAILURE -1
#define CMD_SUCCESS 0
#define DCLI_BSD_COMMAND_BUF_LEN 128

char * dcli_bsd_get_return_string(int iReturnValue, char pReturnStr[DCLI_BSD_COMMAND_BUF_LEN]);
int dcli_bsd_copy_files_to_boards(DBusConnection *connection,const char *src_path, const char *des_path, const int op);
int dcli_bsd_get_slot_ids(DBusConnection *connection, int *ID, const int op);
int dcli_bsd_copy_file_to_board(DBusConnection *connection,const int slot_id, const char *src_path, const char *des_path, const int flag, const int op);
int dcli_set_bsd_daemonlog(int index, unsigned int daemonlogtype, unsigned int daemonloglevel, DBusConnection *dbus_connection, char *DBUS_METHOD);
int dcli_bsd_close_tcp_socket(DBusConnection *connection, const int iSocketId);
int dcli_bsd_check_destination_board_information(DBusConnection *connection, const int iDesAddr, const char *src_path, const char *des_path, const int flag, const int iOption);
int dcli_bsd_check_destination_device_information(DBusConnection *connection, const int iDesAddr, const char *src_path, const char *des_path, const int flag, const int iOption, int *piSocketId);
int dcli_bsd_copy_file_to_device(DBusConnection *connection, const int iSocketId, const char *src_path, const char *des_path, const int flag, const int op);

#endif