#ifndef HMD_LOCAL_H
#define HMD_LOCAL_H
void dcli_free_HmdBoardInfo(struct Hmd_Board_Info_show *hmd_board_node);
struct Hmd_Board_Info_show* show_hmd_info_show(DBusConnection *dcli_dbus_connection, int *board_num, int *ret);



#endif
