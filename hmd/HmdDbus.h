#ifndef _HMD_DBUS_H_
#define	_HMD_DBUS_H_
void *HMDDbus();
void *hmd_dbus_thread_restart();
int hmd_license_synchronize(int slotid,int licensetype, int licensenum);
int syn_hansi_info_to_backup(int slotid,int profile, int neighbor_slotid,int islocaled,HmdOP op,int licenseType);
int hmd_notice_vrrp_config_service_change_state(unsigned int InstID, unsigned int enable);
#endif
