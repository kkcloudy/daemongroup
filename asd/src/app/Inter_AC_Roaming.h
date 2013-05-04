#ifndef INTER_AC_ROAMING_H
#define INTER_AC_ROAMING_H

typedef enum{
	G_BSS_UPDATE,
	G_STA_UPDATE,
	G_DATA_SYNCH,
	G_SYN_END,
	G_GROUP_DEL
}GMsgType;

typedef enum{
	G_ADD,
	G_DEL,
	G_MODIFY
}GOperate;

typedef struct{
	unsigned char group_id;
	unsigned char ACID;
	int sock;
}gThreadArg;

typedef struct{
	unsigned char STAMAC[MAC_LEN];	
	unsigned int BssIndex;
	unsigned int PreBssIndex;
	unsigned char PreBSSID[MAC_LEN];
	unsigned int PreACIP;
	unsigned int O_PreACIP;
	unsigned int O_PreBssIndex;
}G_UPDATE_STA;

typedef struct{
	unsigned int BSSIndex;
	unsigned int Radio_G_ID;
	unsigned int protect_type;
	unsigned char Radio_L_ID;
	unsigned char WLANID;
	unsigned char BSSID[MAC_LEN];
	wAW_IF_Type	bss_ifaces_type;
	wAW_IF_Type	wlan_ifaces_type;
}G_UPDATE_BSS;

typedef struct {
	GMsgType Type;
	GOperate Op;
	unsigned char group_id;
	unsigned int acip;
	union{
		G_UPDATE_STA	U_STA;
		G_UPDATE_BSS	U_BSS;
	}Gu;
}G_Msg;

struct AC_ROAMING_STAINFO{
	struct AC_ROAMING_STAINFO *next;
	struct AC_ROAMING_STAINFO *hnext;
	unsigned char addr[MAC_LEN];
	unsigned int BssIndex;
	unsigned int PreBssIndex;
	unsigned char PreBSSID[MAC_LEN];
	unsigned int PreACIP;
};

typedef struct {
	unsigned char ACID;
	unsigned int L_BSSIndex;
	unsigned int vBSSIndex;
	unsigned int BSSIndex;
	unsigned int Radio_G_ID;
	unsigned int protect_type;
	unsigned char Radio_L_ID;
	unsigned char WLANID;
	unsigned char BSSID[MAC_LEN];
	wAW_IF_Type	bss_ifaces_type;
	wAW_IF_Type	wlan_ifaces_type;
}Mobility_BSS_Info;

typedef struct {
	unsigned int L_BSS_INDEX[L_BSS_NUM];
	unsigned int ACIP;
	unsigned char ACID;
	unsigned int vWTPID;
	unsigned int GroupID;
	struct sockaddr_in ac_addr;
	Mobility_BSS_Info **BSS;
	unsigned int r_num_sta;
	unsigned int is_conn;
	int sock;
	struct AC_ROAMING_STAINFO *r_sta_list;
	struct AC_ROAMING_STAINFO *r_sta_hash[256];
}Mobility_AC_Info;

typedef struct {
	unsigned char GroupID;
	unsigned char WLANID;
	unsigned char *ESSID;
	unsigned char *name;
	unsigned int host_ip;
	struct sockaddr_in host_addr;
	Mobility_AC_Info *Mobility_AC[G_AC_NUM];
}Inter_AC_R_Group;

int G_Wsm_WTPOp(unsigned char group_id,unsigned char ACID,Operate op);
int G_Wsm_BSSOp(unsigned char group_id,unsigned char ACID,unsigned int BSSIndex,Operate op);
int init_asd_sctp_socket(unsigned int port);
struct AC_ROAMING_STAINFO * G_roaming_get_sta(Mobility_AC_Info * Mobility_AC, const u8 *sta);
void G_roaming_sta_list_del(Mobility_AC_Info * Mobility_AC, struct AC_ROAMING_STAINFO * sta);
void G_roaming_free_sta(Mobility_AC_Info * Mobility_AC, struct AC_ROAMING_STAINFO *sta);
struct AC_ROAMING_STAINFO * G_roaming_sta_add(Mobility_AC_Info * Mobility_AC, G_UPDATE_STA	*U_STA);
void G_roaming_sta_del(Mobility_AC_Info * Mobility_AC, unsigned char *STAMAC);
void G_roaming_del_all_sta(Mobility_AC_Info * Mobility_AC);
int ac_group_add_del_bss_info(int sockfd, unsigned char group_id, struct asd_data *wasd, GOperate Op);
int ac_group_add_del_sta_info(int sockfd, unsigned char group_id, struct sta_info *sta, struct asd_data *wasd, GOperate Op);
int ac_group_modify_sta_info(int sockfd, unsigned char group_id, struct sta_info *sta,  struct asd_data *wasd, unsigned int O_IP, unsigned int O_BSSIndex);
#if 0
void *asd_ac_group_thread();
void *asd_synch_thread(void *arg);
#endif
int init_ac_group_info(unsigned char group_id, char *name, char *ESSID);
int del_ac_group_member(unsigned char group_id,unsigned char ac_id, int flag);

int del_ac_group_info(unsigned char group_id);

void asd_connect_select(void *circle_ctx, void *timeout_ctx);
void asd_synch_recv_select(int sock, void *circle_ctx, void *sock_ctx);
void asd_synch_select(void *circle_ctx, void *timeout_ctx);
void asd_ac_group_recv_select(int sock, void *circle_ctx, void *sock_ctx);
void asd_ac_group_accept_select(int sock, void *circle_ctx, void *sock_ctx);
int ACGroupRoamingCheck(unsigned char WLANID, struct asd_data *wasd, const unsigned char *addr);

extern Inter_AC_R_Group *AC_GROUP[GROUP_NUM];
extern unsigned int AC_G_FIRST;
extern unsigned int roaming_notice;
extern int gsock;
extern unsigned int inter_ac_roaming_count;
extern unsigned int inter_ac_roaming_in_count;
extern unsigned int inter_ac_roaming_out_count;

#endif
