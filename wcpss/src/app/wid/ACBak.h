#ifndef AC_BAK_H
#define AC_BAK_H

#define AC_LICENSE_COUNT 16

typedef enum{
	B_ADD = 0,
	B_DEL = 1,
	B_MODIFY = 2
}BakOperate;

typedef enum{
	B_INFO_TYPE = 0,
	B_WTP_TYPE = 1,
	B_BSS_TYPE = 2,
	B_AC_REQUEST = 3,
	B_CHECK_REQ = 4,
	B_CHECK_RES = 5,
	B_END = 6,
	B_CHECK_ReREQ = 7,
	B_CHECK_LICENSE = 8,
	B_WIFILOCATE_TYPE = 9,
	B_LICENSE_REQUEST
}BakMsgType;

typedef struct {
	unsigned int WTPID;
	unsigned int WTPIP;
	unsigned char WTP_MAC[MAC_LEN];
	unsigned char WTP_IP[DEFAULT_LEN];
	unsigned char SN[DEFAULT_LEN];
	struct sockaddr addr;
	unsigned int oemoption;
	
	time_t	add_time;
	time_t	imagedata_time;
	time_t	config_update_time;
	unsigned int ElectrifyRegisterCircle;
	unsigned char WTP_VER[NAME_LEN];
	unsigned char WTP_CODEVER[NAME_LEN];
	unsigned char WTP_SYSVER[NAME_LEN];

}B_UPDATE_WTP;
typedef struct{
	unsigned int WTPID;
	unsigned int BSSIndex;
	unsigned char BSSID[MAC_LEN];
	unsigned char WLANID;
}B_UPDATE_BSS;

typedef struct {
	char info[DEFAULT_LEN];
}B_CMD_INFO;

typedef struct {
	int wtp_count;
	char wtp_check[4097];
}B_CHECK;

typedef struct {
	BakMsgType Type;
	BakOperate Op;
	union{
		B_CHECK WTP_CHECK;
	}Bu;
}B_Check_Msg;

typedef struct {
	unsigned int count;
	unsigned int license[AC_LICENSE_COUNT];
}B_UPDATE_LICENSE;

typedef struct {
	u_int32_t groupid;	
	unsigned short report_interval;
	unsigned short report_pattern;
	unsigned char scan_type;
	unsigned long long channel;
	unsigned short channel_scan_interval;
	unsigned short channel_scan_dwell;
	unsigned char rssi;
	unsigned int server_ip;
	unsigned short server_port;
	unsigned int radio_count;
	unsigned char bit_radioarray[WIFILOCATE_TO_BACK_SIZE];  //radio count 512*8
	unsigned char bit_radioarray_isenable[WIFILOCATE_TO_BACK_SIZE];

	unsigned int isenable;
	unsigned int radioid;

	/*for wifi-locate-0 use begin*/
	unsigned short report_interval_5_8G;
	unsigned short report_pattern_5_8G;
	unsigned char scan_type_5_8G;
	unsigned long long channel_5_8G;
	unsigned short channel_scan_interval_5_8G;
	unsigned short channel_scan_dwell_5_8G;
	unsigned char rssi_5_8G;
	unsigned int server_ip_5_8G;
	unsigned short server_port_5_8G;
	/*for wifi-locate-0 use end*/
}B_UPDATE_WIFILOCATE;

typedef struct {
	BakMsgType Type;
	BakOperate Op;
	union{
		B_UPDATE_WTP	WTP;
		B_UPDATE_BSS	BSS;
		B_UPDATE_LICENSE	LICENSE;
		B_UPDATE_WIFILOCATE	WIFILOCATE;
	}Bu;
}B_Msg;

typedef struct{
	char state;
	int vrrid;
	int master_uplinkip;
	int master_downlinkip;
	int bak_uplinkip;
	int bak_downlinkip;
	int vir_uplinkip;
	int vir_downlinkip;
	int global_ht_ip;
	int global_ht_opposite_ip;
	char * vir_uplinkname;
	char * vir_downlinkname;
	char * global_ht_ifname;
}vrrp_info;

extern pthread_t WID_BAK;
extern pthread_t WID_MASTER;
extern struct sockaddr M_addr;
extern struct sockaddr B_addr;
extern struct sockaddr Lic_Active_addr;
extern struct sockaddr Lic_bak_addr;
extern struct lic_ip_info Lic_ip;
extern struct bak_sock *bak_list;
extern CWACThreadArg_clone BakArgPtr;
extern int wid_sock;
extern int set_vmac;
extern char v_mac[MAC_LEN];
extern unsigned int vrrid;
extern vrrp_info vinfo;
extern CWThreadMutex MasterBak;
extern int set_vmac_state;
extern unsigned int neighbor_slotid;
void *wid_bak_thread();
int GetBakIFInfo(char *name, unsigned int ip);
void *wid_master_thread();
void bak_add_del_bss(int sockfd,BakOperate Op,unsigned int BSSIndex);
void bak_add_del_wtp(int sockfd,BakOperate Op,unsigned int WTPID);
int send_info(const char * buf);
int hwaddr_set( char *ifname, char *addr, int addrlen );
int set_wid_mac(char * addr, int is_vmac);
int send_all_tunnel_interface_arp();
int init_wid_bak_socket();
int UpdateWifiHansiState();
int AC_SYNCHRONIZE_WSM_TABLE_INFO();
int AC_UPDATE_BAK_AC_WIRELESS_INFO();
int AC_SYNCHRONIZE_ASD_TABLE_INFO();
int set_wid_src_mac();
int update_license_req(int sockfd,struct sockaddr_in *addr);
int update_license(int sockfd,struct sockaddr_in *addr);
int check_license(B_Msg *tmp);
int compare_license(B_Msg *tmp);
extern int notice_hmd_update_state_change(unsigned int vrrid,unsigned int state);
void bak_add_del_wifilocate
(
	int sockfd,
	BakOperate op, 
	unsigned int groupid, 
	unsigned char isenable,
	unsigned int radioid
);
#endif
