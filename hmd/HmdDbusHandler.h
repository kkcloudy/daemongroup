#ifndef _HMD_DBUS_HANDLER_H_
#define	_HMD_DBUS_HANDLER_H_
void notice_hmd_server_hansi_info_update(int InstID,struct Hmd_Inst_Mgmt * Inst);
int vrrp_hansi_is_running
(
	unsigned int profileId
);
int check_hansi_service_started
(
	char *service_name,
	unsigned int profileId
);
int femto_service_switch_check(unsigned int type, unsigned int slotid, unsigned insid, unsigned int islocal, unsigned int femto_switch);
int femto_service_state_check(unsigned int type, unsigned int slotid, unsigned insid, unsigned int islocal);
int femto_service_switch(unsigned int type, unsigned int slotid, unsigned int insid, int islocal, unsigned int service_switch);
int parse_int_ve(char* str, unsigned int* slotid, unsigned int *vlanid, unsigned int *vlanid2,char *cid, unsigned int *port);//fengwenchao add "vlanid2" 20130325 for axxzfi-1506
int check_ve_interface(char *ifname, char *name);
#endif
