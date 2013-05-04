#ifndef __DCLI_PVLAN_H__
#define __DCLI_PVLAN_H__

#define MAX_VLANID 4094
#define MIN_VLANID 1
#define MIN_SLOT   1
#define MAX_SLOT   4
#define MIN_PORT   1
#define MAX_PORT   6
#define ENABLE     1
#define DISABLE    2
#define PVE_ERR   -1
#define MAX_TRUNK 127
#define MIN_TRUNK 1
#define MAX_PVLAN 31
#define MIN_PVLAN 1
#define PVE_MAX_PORT    64
#define NPD_DBUS_NOT_SUPPORT 3

typedef enum{
	PVE_FAIL=0,
	PVE_TRUE=1,
}PVE_BOOL;

int is_enable_or_disable(char * str);

void dcli_pve_init(void);

#endif

