#ifndef _DCLI_BSD_H_
#define _DCLI_BSD_H_


/* book add, 2012-2-08 */
enum dcli_bsd_debug{
	DCLI_BSD_DEFAULT = 0x1,
	DCLI_BSD_DBUS = 0x2,
	DCLI_BSD_BSDINFO = 0x4,
	DCLI_BSD_MB = 0x8,/*master and bak*/
	DCLI_BSD_ALL = 0xf
};


void dcli_bsd_init(void);
//int dcli_bsd_copy_files_to_boards(DBusConnection *connection,const char *src_path, const char *des_path, const int op);

#endif

