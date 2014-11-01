#ifndef _HMD_MONITOR_H
#define _HMD_MONITOR_H
void * HMDHansiMonitor(void *arg);
unsigned int notice_wid_local_hansi_service_change_state(unsigned int InstID, unsigned int neighbor_slotid);
int send_tunnel_interface_arp(char *MAC, int addr, char* ifname );
unsigned int notice_eag_local_hansi_service_change_state(unsigned int InstID, unsigned int neighbor_slotid);
void HMDReInitHadDbusPath(int index, char * path, char * newpath, int islocaled);
unsigned int notice_had_to_change_vrrp_state(unsigned int InstID, int op);
int hmd_wcpss_reload(int vrrid, int islocal);
int hmd_load_config(char *ConfigPath);
int hmd_eag_reload(unsigned slotid, unsigned int islocal, unsigned int vrrid);
int hmd_pdc_reload(unsigned slotid, unsigned int islocal, unsigned int vrrid);
int hmd_rdc_reload(unsigned slotid, unsigned int islocal, unsigned int vrrid);

#if 1
struct io_info
{
	unsigned char stamac[MAC_LEN];
	unsigned char acmac[MAC_LEN];
	unsigned char ifname[MAX_IFNAME_LEN];	
	unsigned char in_ifname[MAX_IFNAME_LEN];	
	unsigned int staip;
	unsigned int vrrid;
	unsigned int seq_num;
};

#define AAT_IOC_MAGIC 249
#define AAT_IOC_MAXNR 0x16

#define AAT_IOC_ADD_STA		_IOWR(AAT_IOC_MAGIC, 1, struct io_info) // read values
#define AAT_IOC_DEL_STA		_IOWR(AAT_IOC_MAGIC, 2, struct io_info) // read values
#define AAT_IOC_GET_STA_IP	_IOWR(AAT_IOC_MAGIC, 3, struct io_info)
/* use vrrid clean stas for HMD */
#define AAT_IOC_CLEAN_STAS	_IOWR(AAT_IOC_MAGIC, 4, struct io_info) 
#endif

#endif

