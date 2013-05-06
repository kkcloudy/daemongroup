#ifndef __SEM_TIPC_H_
#define __SEM_TIPC_H_

#define TIPC_SEM_TYPE_LOW	0x1000
#define TIPC_SEM_TYPE_HIGH	0x1fff

#define MAX_SLOT_NUM			20
#define SEM_TIPC_INSTANCE_BASE_ACTIVE_TO_NON_ACTIVE	1121
#define SEM_TIPC_INSTANCE_BASE_NON_ACTIVE_TO_ACTIVE	SEM_TIPC_INSTANCE_BASE_ACTIVE_TO_NON_ACTIVE+MAX_SLOT_NUM

#define SME_CONN_LINK_TEST_COUNT 	3

#define SEM_TIPC_RECV_BUF_LEN		1024	
#define SEM_TIPC_SEND_BUF_LEN		SEM_TIPC_RECV_BUF_LEN


enum
{
	SEM_CMD_PDU_STBY_ACTIVE_LINK_TEST_REQUEST,			
	SEM_CMD_PDU_STBY_ACTIVE_LINK_TEST_RESPONSE,			
	SEM_FORCE_STBY,										
	SEM_NON_ACTIVE_BOARD_REGISTER,						
	SEM_ACTIVE_MASTER_BOARD_SYN_PRODUCT_INFO,			
	SEM_BOARD_INFO_SYN,									
	SEM_SOFTWARE_VERSION_SYN_REQUEST,					
	SEM_SOFTWARE_VERSION_SYN_RESPONSE,					
	SEM_SOFTWARE_VERSION_SYNING,						
	SEM_SOFTWARE_VERSION_SYN_FINISH,
	SEM_SOFTWARE_VERSION_SYNC_SUCCESS,
	SEM_FILE_SYNING,									
	SEM_FILE_SYN_FINISH,								
	SEM_ETHPORT_INFO_SYN,								
	SEM_NETLINK_MSG,									
	SEM_CONNECT_CONFIRM,								
	SEM_CONNECT_REQUEST,								
	SEM_HARDWARE_RESET,									
	SEM_HARDWARE_RESET_PREPARE,							
	SEM_RESET_READY,									
	SEM_MCB_ACTIVE_STANDBY_SWITCH_NOTIFICATION,
	SEM_SET_SYSTEM_IMG,
	SEM_SET_SYSTEM_IMG_REPLY,
	SEM_DISABLE_KEEP_ALIVE_TEMPORARILY,
	SEM_EXECUTE_SYSTEM_COMMAND,
	SEM_COMPATIBLE_SYNC_PRODUCT_INFO,		//25
	SEM_COMPATIBLE_BOARD_REGIST,			//26
	SEM_REQUEST_BOARD_REGISTER,
	SEM_CMD_FPGA_ABNORMAL,    //huangjing
	SEM_SEND_MD5_STR,
	SEM_SOFTWARE_VERSION_SYNC_FAILED,
	SEM_PEND_CVM,
	SEM_NETLINK_MSG_BROADCAST,
	SEM_CMD_PDU_MAX,
	SEM_CMD_SEND_STATE_TRAP,
	SEM_TIPC_TEST,
	SEM_SEND_FILE_TEST,
	SEM_CMD_SEND_STATE_TRAP_REBOOT
};




typedef struct sem_link_test_s
{
	unsigned short type;
	unsigned long length;
}sem_link_test_t;


#define MAX_SOFTWARE_VERSION_FILE_NAME_LEN	40
#define MAX_SOFTWARE_VERSION_FILE_PATHE_LEN	MAX_SOFTWARE_VERSION_FILE_NAME_LEN + 10
#define MAX_SOFTWARE_TEXT_LEN	50000


#define MAX_SOFTWARE_VERSION_SIZE  (100*1024*1024ull)
#define NEW_SOFTWARE_VERSION_PATH  "/blk/"
#define SOFTWARE_VERSION_NAME_LEN	20  
#define PATH_AND_NAME_LEN			((SOFTWARE_VERSION_NAME_LEN)+20)

#define MAX_SOFTWARE_VERSION_LEN 20
#define MAX_SOFTWARE_BUILDNO_LEN 20
#define SOFTWARE_VERSION_FILE	"/etc/version/version"
#define SOFTWARE_BUILDNO_FILE	"/etc/version/buildno"


typedef struct software_version_syn_text_s
{
	int len;
	char buf[MAX_SOFTWARE_TEXT_LEN];
}software_version_text_t;


typedef struct software_version_syn_response_s
{
	int is_need;
	char version[MAX_SOFTWARE_VERSION_LEN];
	char buildno[MAX_SOFTWARE_VERSION_LEN];
}software_version_response_t;

typedef struct software_version_syn_request_s
{
	int tem;
	char version[MAX_SOFTWARE_VERSION_LEN];
	char buildno[MAX_SOFTWARE_VERSION_LEN];
}software_version_request_t;

typedef struct tipc_buf
{
	int num;
	int total;
	char checkcode[9];
}tipc_buf_t;

typedef struct sem_tlv_s
{
    unsigned short type;
    unsigned short length;
    unsigned char body[0];
}sem_tlv_t;


typedef struct sem_pdu_head_s
{
    unsigned long  version;
    unsigned long  length;
	int slot_id;
    struct sem_tlv_s tlv[0];
}sem_pdu_head_t;

typedef struct temperature_info_s
{
	int core_temp;
	int remote_temp;
}sem_tmperature_info;
#if 0
extern int sem_tipc_init(board_fix_param_t *local_board);
extern int tipc_set_server_addr(struct sockaddr_tipc *sockaddr, __u32 lower, __u32 upper);
extern int tipc_set_client_addr(struct sockaddr_tipc *sockaddr,  __u32 instance);

extern int tipc_client_connect_to_server(int sd, struct sockaddr_tipc sockaddr);
//extern int tipc_server_wait_connect(int sd, struct sockaddr_tipc sockaddr);
extern int tipc_server_send_to_client(int sd, char *chBuf);
extern int tipc_server_recv(int sd, char *chBuf, int buf_len);
extern int tipc_send_to_server(int sd, char *chBuf);
extern int tipc_client_recv(int sd, char *chBuf);

#endif

#define MAX_PKT_TYPE_NUM 32

typedef struct packet_mark{
	unsigned char packet_type;
	unsigned char is_type_used;
	char chFileName[128];
}packet_mark_t;

typedef struct sync_file_head{
	unsigned int msg_len;
	unsigned char packet_type;
	char chFileName[64];
	int syn_to_blk;
}sync_file_head_t;

typedef struct sync_save_file_head{
	FILE *fp[MAX_SLOT_NUM];
	unsigned char packet_type;
}sync_save_file_head_t;

packet_mark_t          pkt_mark[MAX_PKT_TYPE_NUM];
sync_save_file_head_t  save_file_head[MAX_PKT_TYPE_NUM];

extern int sem_tipc_send(unsigned int slot_id, int type, char*msgBuf, int len);

#endif
