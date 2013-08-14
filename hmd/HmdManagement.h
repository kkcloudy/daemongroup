#ifndef _HMD_MANAGEMENT_H
#define _HMD_MANAGEMENT_H 
void * HMDManagementC();
void * HMDManagementS();
int hmd_hansi_synchronize_request(int slotid);
extern  struct Hmd_For_Dhcp_restart *DHCP_MONITOR;
#endif
