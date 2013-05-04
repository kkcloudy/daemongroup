#ifndef __IU_DBUS_H__
#define __IU_DBUS_H__

/*msc config informaion*/
struct cn_config{
	unsigned int ip;
	unsigned short port ;
	unsigned int point_code;
	unsigned int connect_mode;
};

void iu_dbus_start(void);


#endif

