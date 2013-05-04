#ifndef WBMD_WB_H
#define WBMD_WB_H
void *dcli_show_wbridge_list(
	int index,
	unsigned int localid,
	unsigned int* ret,
	int wbid,
	DBusConnection *dcli_dbus_connection
	);
#endif
