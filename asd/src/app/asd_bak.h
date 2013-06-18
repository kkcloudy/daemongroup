#ifndef ASD_BAK_H
#define ASD_BAK_H


typedef enum{
	B_ADD = 0,
	B_DEL = 1,
	B_MODIFY = 2,
	B_UPDATE = 3	//mahz add 2011.8.2
}BakOperate;

typedef enum{
	B_INFO_TYPE = 0,
	B_STA_TYPE = 1,
	B_END = 2,	
	B_CHECK_REQ = 3,
	B_CHECK_RES = 4,
	B_CHECK_ReREQ = 5,	
	B_UPDATE_REQ = 6,
	B_BATCH_STA_TYPE = 7,
	B_BSS_TYPE = 8,
	B_BATCH_ROAM_STA = 9,
	B_CHECK_STA_REQ = 10,
	B_BSS_STATIS = 11,//qiuchen 
	B_BSS_RADIUS = 12
}BakMsgType;


typedef struct {
	unsigned char STAMAC[6];
	unsigned char B_identity[128];
	time_t sta_add;
	size_t B_identity_len;
	unsigned int WTPID;	
	unsigned int BSSIndex;	
	unsigned int B_acct_session_id_hi;
	unsigned int B_acct_session_id_lo;
	time_t B_acct_session_start;
	int B_acct_session_started;
	int B_acct_terminate_cause; /* Acct-Terminate-Cause */
	int B_acct_interim_interval; /* Acct-Interim-Interval */	
	unsigned int B_acct_input_gigawords; /* Acct-Input-Gigawords */
	unsigned int B_acct_output_gigawords; /* Acct-Output-Gigawords */
	unsigned int ipaddr;
	unsigned int gifindex;
	unsigned int total_num;
	time_t sta_online_time;//qiuchen add it
	unsigned int PreBSSIndex;
	unsigned char PreBSSID[MAC_LEN];
	unsigned char rflag;
}B_UPDATE_STA;

typedef struct {
	char info[1024];
}B_CMD_INFO;

typedef struct {
	int sta_count;
	int bss_count;
}B_STA_COUNT;

typedef struct {
	unsigned int bss_index;
}B_BSS_STA_UPDATE;

//mahz add 2011.8.2
typedef struct{
	unsigned int bssindex;
	unsigned int sta_num;
}bssindex_struct;
typedef struct{
	unsigned int count;
	bssindex_struct bssindex_array[4096];
}B_BSS_UPDATE;
//qiuchen
typedef struct{
	unsigned char SID;
	unsigned char auth_server;
	unsigned char acct_server;
}sec_radius_array;
typedef struct{
	unsigned char sid_num;
	sec_radius_array sArray[128];
}B_BSS_RADIUS_STATE;
typedef struct {
	BakMsgType Type;
	BakOperate Op;
	union{
		B_UPDATE_STA	U_STA;
		//B_CMD_INFO		INFO;
		B_BSS_STA_UPDATE	U_BSS_STA;
		B_STA_COUNT	CHK;
		B_BSS_UPDATE	U_ALL_BSS;
		B_BSS_RADIUS_STATE  U_BSS_RADIUS;
	}Bu;
}B_Msg;

typedef struct {
	BakMsgType Type;
	BakOperate Op;
	unsigned int num;
	B_UPDATE_STA	U_STA[50];
}_B_Msg;
typedef struct{
		unsigned char addr[MAC_LEN];
		unsigned int BssIndex;
		unsigned int PreBssIndex;
		unsigned char PreBSSID[MAC_LEN];		
		unsigned char BSSID[MAC_LEN];
}B_ROAM_STA;
typedef struct{
	BakMsgType Type;
	BakOperate Op;
	unsigned int num;
	B_ROAM_STA U_STA[50];
}B_ROAM_Msg;


int send_info(const char * buf);
int update_end();
#ifdef ASD_MULTI_THREAD_MODE
void *asd_bak_thread();
void *asd_update_thread();
#else
void asd_bak_select_mode(int sock, void *circle_ctx, void *sock_ctx);
void asd_bak_select_mode2(int sock, void *circle_ctx, void *sock_ctx);
int asd_update_select_mode(void *circle_ctx, void *timeout_ctx);
void asd_update_batch_sta();
void B_UPDATE_REQUEST(int sockfd, int bssindex);
void asd_master_select_mode(int sock, void *eloop_ctx, void *sock_ctx);
void bak_update_bss_req(void *circle_ctx, void *timeout_ctx);
int init_asd_bak_socket();
int bak_add_sta(struct asd_data *wasd, struct sta_info *sta);
int bak_del_sta(struct asd_data *wasd, struct sta_info *sta);
int bak_update_sta_ip_info(struct asd_data *wasd, struct sta_info *sta);
void bak_check_sta_req(int sockfd, unsigned int BSSIndex,u8* mac);
//qiuchen add it for master_Bak radius server 2012.12.17
int update_radius_state_to_bakAC(unsigned char SID,unsigned char auth,unsigned char acct);
void asd_update_radius_state();
//end
#endif
extern char is_secondary;
extern int is_notice;
extern char bak_unreach;
extern struct sockaddr B_addr;
extern struct sockaddr M_addr;
extern pthread_t ASD_BAK;
extern int new_sock;
extern int asd_sock;
extern int asd_master_sock;
extern char update_pass;
#endif
