#ifndef _WS_LOCAL_HANSI_H_
#define _WS_LOCAL_HANSI_H_

void free_broad_instance_info(struct Hmd_Board_Info_show **instanceHead);
int show_broad_instance_info(DBusConnection *connection, struct Hmd_Board_Info_show **instanceHead);

#endif
