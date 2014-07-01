#ifndef _HMD_PUB_H
#define	_HMD_PUB_H
#include <linux/tipc.h>
#include <dbus/dbus.h>
#define MAX_IFNAME_LEN 16
#define MAX_IFNAME_NUM 16
#define MAX_INSTANCE (16+1)
#define MAX_SLOT_NUM 16
#define PATH_LEN 64
#define DEFAULT_LEN	256
#define MAC_LEN 6
#define LICREQ_LEN	40
#define HMD_CHECKING_TIMER	60
#define HMD_CHECKING_UPDATE_TIMER	300
#define INSTANCE_CHECK_FAILED	(-1)
#define INSTANCE_CREATED		(1)		/* instance have created.		*/
#define INSTANCE_NO_CREATED	(0)		/* instance have not created.	*/
#define SYS_COMMAND_LEN	(64*2)		/* shell command length */
#define DHCP_QID   255
#define MAX_GROUP_NUM  (32+1)
#define MAX_HANSI_NUM  16
#define MAX_STRLEN_NUM 20


#define SERVER_TYPE 10086
#define SERVER_BASE_INST 128
extern int HMDMsgqID;
extern char MSGQ_PATH[PATH_LEN];
#define SHOW_RUNNING_LEN	(1024 * 1024)
#define DEFAULT_LICENSE_NUM		2

#define DEFAULT_CONFIG_DIR	 	"/var/run/config"
#define DEFAULT_CONFIG_PATH 	"/var/run/config/Instconfig"

#define HMD_HOT_RESTART_FLAG_PATH 	"/var/run/waw.hotrestart"
#define HMD_HOT_RESTART_FLAG		'1'
#define HMD_SET_FLAG				(1)
#define HMD_CLR_FLAG				(0)



#define RETURN_CODE_BASE                 (0x150000)
#define RETURN_CODE_OK		              (RETURN_CODE_BASE + 1)
#define RETURN_CODE_ERR		          (RETURN_CODE_BASE + 2)
#define RETURN_CODE_PROFILE_OUT_OF_RANGE (RETURN_CODE_BASE + 3)
#define RETURN_CODE_PROFILE_EXIST        (RETURN_CODE_BASE + 4)
#define RETURN_CODE_PROFILE_NOTEXIST     (RETURN_CODE_BASE + 5)
#define VRETURN_CODE_MALLOC_FAILED        (RETURN_CODE_BASE + 6)
#define RETURN_CODE_BAD_PARAM            (RETURN_CODE_BASE + 7)  
#define RETURN_CODE_PROFILE_NOT_PREPARE  (RETURN_CODE_BASE + 8)  
#define RETURN_CODE_SERVICE_NOT_PREPARE  (RETURN_CODE_BASE + 9)
#define RETURN_CODE_VGATEWAY_NO_SETTED   (RETURN_CODE_BASE + 0xA)
#define RETURN_CODE_LINKMODE_NO_SETTED   (RETURN_CODE_BASE + 0xB)
#define RETURN_CODE_IFNAME_ERROR         (RETURN_CODE_BASE + 0xC)
#define RETURN_CODE_VIP_EXIST            (RETURN_CODE_BASE + 0xD)
#define RETURN_CODE_VIP_NOT_EXIST        (RETURN_CODE_BASE + 0xE)
#define RETURN_CODE_VIP_LAST_ONE         (RETURN_CODE_BASE + 0xF)
#define RETURN_CODE_IF_EXIST             (RETURN_CODE_BASE + 0x10)
#define RETURN_CODE_IF_NOT_EXIST         (RETURN_CODE_BASE + 0x11)
#define RETURN_CODE_IF_UP_LIMIT          (RETURN_CODE_BASE + 0x12)
#define RETURN_CODE_VMAC_NOT_PREPARE	  (RETURN_CODE_BASE + 0x13)
#define RETURN_CODE_NO_CONFIG			  (RETURN_CODE_BASE + 0x14)

/*
sush@autelan.com 2/Jul/2011
remove redefinitions.

enum board_func_type{
	MASTER_BOARD = 0x1,
	AC_BOARD = 0x2,
	SWITCH_BOARD = 0x4,
	BASE_BOARD = 0x8,
	NAT_BOARD = 0x10
};
enum board_state
{
	BOARD_REMOVED,
	BOARD_INSERTED,
	BOARD_INITIALIZING,
	BOARD_READY,
	BOARD_RUNNING
};

*/

typedef enum{
	INST_BACKUP = 2,
	INST_ACTIVE = 3,
	INST_DISABLE = 99
}InstState;
typedef enum{
	IUH_TYPE = 0,
	IU_TYPE = 1
}Femto_Type;
struct Inst_Interface{	
	char ifname[MAX_IFNAME_LEN];	/* interface name */
	int real_ip;					/* appoint real ip of the interface */
	int vir_ip;					/* appoint virtual ip of the interface */
	int remote_r_ip;			/* appoint remote real ip of the interface */
	int umask;
	int dmask;
	int mask;
	char mac[MAC_LEN];
};

struct hansi_depend{
	int Depend_Inst_ID;
	int depend_slot_no;
};

struct Hmd_Inst_Mgmt{
	int Inst_ID;
	int vrrid;
	int slot_no;
	int	r_slot_no;
	int InstState;
	int	HmdTimerID;/*for hansi checking*/
	int	HmdCheckUpdateTimerID;/*for hansi checking CheckUpdate*/
	int HmdWidUpOk;
	int HmdBakForever;
	int WidReadyTimes;
	int AsdReadyTimes;
	time_t WidLastReadyTime;
	time_t AsdLastReadyTime;
	pthread_t	threadID;
	DBusConnection * connection;
	int tipcfd;
	int Inst_UNum;
	struct Inst_Interface Inst_Uplink[MAX_IFNAME_NUM];
	int Inst_DNum;
	struct Inst_Interface Inst_Downlink[MAX_IFNAME_NUM];
	int Inst_GNum;
	struct Inst_Interface Inst_Gateway[MAX_IFNAME_NUM];	
	struct hansi_depend depend_hansi[MAX_INSTANCE];
	struct Inst_Interface Inst_Hb;
	int RestartTimes;
	int priority;
	int isActive;
	int wid_check;
	int asd_check;
	int wsm_check;
	int delete_flag;//init 0,is deleting 1
	int eag_check;
	int rdc_check;
	int pdc_check;
	unsigned int iuh_service_switch;
	unsigned int iu_service_switch;
	int wid_check_timeout;
	int asd_check_timeout;
	int wsm_check_timeout;
	char auto_sync_config_ip[20];
	int eag_check_timeout;
	int rdc_check_timeout;
	int pdc_check_timeout;
	int group_id;   //add for hansi reference group

};
struct Hmd_L_Inst_Mgmt_Summary{
	int Inst_ID;
	int slot_num;
	int	slot_no;
	int InstState;
	int isActive;
	int	slot_no1;
	int InstState1;
	int isActive1;
};
struct Hmd_L_Inst_Mgmt{
	int Inst_ID;
	int slot_num;
	int	slot_no;
	int InstState;	
	int	slot_no1;
	int InstState1;
	int	HmdTimerID;/*for hansi checking*/
	int	HmdCheckUpdateTimerID;/*for hansi checking CheckUpdate*/
	int HmdWidUpOk;
	int HmdBakForever;
	int WidReadyTimes;
	int AsdReadyTimes;
	time_t WidLastReadyTime;
	time_t AsdLastReadyTime;
	pthread_t	threadID;
	DBusConnection * connection;
	struct sockaddr_tipc tipcaddr;/*tipc addr for another one*/
	int tipcfd;
	int Inst_UNum;
	struct Inst_Interface Inst_Uplink[MAX_IFNAME_NUM];
	int Inst_DNum;
	struct Inst_Interface Inst_Downlink[MAX_IFNAME_NUM];
	int Inst_GNum;
	struct Inst_Interface Inst_Gateway[MAX_IFNAME_NUM];		
	int RestartTimes;	
	int RestartTimes1;
	int priority;
	int isActive;
	int isActive1;
	int wid_check;
	int asd_check;
	int wsm_check;
	int delete_flag;/*1 is deleting,0 other*/
	int eag_check;
	int rdc_check;
	int pdc_check;
	unsigned int iuh_service_switch;
	unsigned int iu_service_switch;
	int wid_check_timeout;
	int asd_check_timeout;
	int wsm_check_timeout;
	int eag_check_timeout;
	int rdc_check_timeout;
	int pdc_check_timeout;
};


struct group_node{
    int Inst_ID;
    int slot_id;
};

struct Hmd_Hansi_Group{
    int group_id;
    struct group_node hansi[MAX_HANSI_NUM];
};

struct Hmd_Board_Info{
	int board_func_type;
	int slot_no;
	int isMaster;
	int InstNum;	
	int LocalInstNum;
	int NeighborSlotID;
	int tipcfd;
	int HMDTimer_ConfigSave; //fengwenchao add 20130412 for hmd timer config save
	int sem_max_ap_num;  //fengwenchao add for read gMaxWTPs from  /dbm/local_board/board_ap_max_counter
	int *L_LicenseNum[MAX_INSTANCE];
	int *R_LicenseNum[MAX_INSTANCE];
	struct sockaddr_tipc tipcaddr;
	struct Hmd_Inst_Mgmt * Hmd_Inst[MAX_INSTANCE];	
	struct Hmd_L_Inst_Mgmt * Hmd_Local_Inst[MAX_INSTANCE];
};
struct Hmd_For_Dhcp_restart{
	DBusConnection * connection;
	int tipcfd;
	//pthread_t	dhcp_monitor[MAX_SLOT_NUM];
	pthread_t	dhcp_monitor;
	int	HmdTimerID;
	int dhcp_check;
	int RestartTimes;
	int dhcp_check_timeout;
};
/*fengwenchao add 20110616*/
struct Hmd_Board_Info_show
{
	int board_func_type;
	int slot_no;
	int isMaster;
	int InstNum;	
	int LocalInstNum;
	int NeighborSlotID;
	int tipcfd;
	struct sockaddr_tipc tipcaddr;

    	struct Hmd_Board_Info_show *next;
	struct Hmd_Board_Info_show *hmd_board_list;
	struct Hmd_Board_Info_show *hmd_board_last;	

	struct Hmd_Inst_Mgmt * Hmd_Inst[MAX_INSTANCE];	
	struct Hmd_L_Inst_Mgmt * Hmd_Local_Inst[MAX_INSTANCE];	
};
/*fengwenchao add end*/
typedef enum{
	HMD_CREATE = 1,
	HMD_DELETE = 2,
	HMD_STATE_SWITCH = 3,
	HMD_HANSI_INFO_NOTICE = 4,
	HMD_HANSI_CHECKING = 5,
	HMD_RESTART = 6,
	HMD_HANSI_UPDATE = 7,
	HMD_HANSI_ENABLE = 8,
	HMD_HANSI_DISABLE = 9,
	HMD_LICENSE_UPDATE = 10,
	HMD_LICENSE_SYNCHRONIZE = 11,
	HMD_HANSI_INFO_SYN_REQ = 12,
	HMD_HANSI_INFO_SYN_ADD = 13,
	HMD_HANSI_INFO_SYN_DEL = 14,
	HMD_HANSI_INFO_SYN_MODIFY = 15,
	HMD_HANSI_INFO_SYN_LICENSE = 16	,
	HMD_DELETE_SLOTID_AP_UPDATA_IMG ,    /*fengwenchao add 20120228 for AXSSZFI-680*/
	HMD_CLEAR_APPLY_IFNAME_FLAG,   /*fengwenchao copy from 1318 for AXSSZFI-839*/
	HMD_BAKUP_FOREVER_CONFIG,
	HMD_IS_DELETE_HANSI,/*hmd is deleting hansi set inst_state,config hansi is not permitted*/
	HMD_SERVER_DELETE_HANSI,/*delete hansi compeleted,notice server to free hansi*/
	HMD_CREATE_FOR_DHCP,
	HMD_CREATE_FOR_DHCP_DISABLE,
	HMD_CREATE_FOR_DHCP_ENABLE,
	HMD_DHCP_TO_START,
	HMD_DHCP_CHECKING,
	HMD_NOTICE_VRRP_STATE_CHANGE_MASTER,
	HMD_NOTICE_VRRP_STATE_CHANGE_BAK,
	HMD_RELOAD_CONFIG_FOR_EAG,
	HMD_RELOAD_CONFIG_FOR_PDC,
	HMD_RELOAD_CONFIG_FOR_RDC
}HmdOP;

typedef enum{
	HMD_HANSI = 0,
	HMD_LOCAL_HANSI = 1
}HmdType;

typedef struct{
	unsigned int slotid;
	unsigned int vrrid;
	unsigned int local;
	unsigned int boardType;
	unsigned int hmdforevesw;
}wVRRP_HANSI;
typedef struct{
	int prestate;
	int nowstate;
}StateInfo;
typedef struct{
	int state;
	int NeighborSlotID;	
	int NeighborState;
	int NeighborActive;	
	int isActive;
	int slot_num;
	int priority;
	struct Inst_Interface Inst_Hb;
	int Inst_UNum;
	struct Inst_Interface Inst_Uplink[MAX_IFNAME_NUM];
	int Inst_DNum;
	struct Inst_Interface Inst_Downlink[MAX_IFNAME_NUM];
	int Inst_GNum;
	struct Inst_Interface Inst_Gateway[MAX_IFNAME_NUM];
}HansiUpdateInfo;
typedef struct{
	int Inst_ID;
	int slot_num;
	int	slot_no;
	int InstState;	
	int	slot_no1;
	int InstState1;
	int isActive;
	int isActive1;
	struct Inst_Interface Inst_Hb;
	int Inst_UNum;
	struct Inst_Interface Inst_Uplink[MAX_IFNAME_NUM];
	int Inst_DNum;
	struct Inst_Interface Inst_Downlink[MAX_IFNAME_NUM];
	int Inst_GNum;
	struct Inst_Interface Inst_Gateway[MAX_IFNAME_NUM];
}HansiSynInfo;

typedef struct{
	int licenseType;
	int licenseNum;
	int licenseSlotID;
	char licreq[LICREQ_LEN];
}LicenseUpdate;

struct HmdMsg{
	HmdOP op;
	HmdType type;
	int S_SlotID;
	int D_SlotID;
	int InstID;
	int local;
	char clear_ifname[MAX_IFNAME_LEN]; /*fengwenchao copy from 1318 for AXSSZFI-839*/
	char slot_ap_updata_img[DEFAULT_LEN];/*fengwenchao add 20120228 for AXSSZFI-680*/
	union{
		StateInfo statechange;
		HansiUpdateInfo updateinfo;
		wVRRP_HANSI HANSI;
		LicenseUpdate LicenseInfo;
		HansiSynInfo HansiSyn;
	}u;
};

struct HmdMsgQ{
	long mqid;
	struct HmdMsg mqinfo;
};

struct LicenseMgmt{
	int total_num;
	int free_num;
	int l_assigned_num[MAX_SLOT_NUM][MAX_INSTANCE];
	int r_assigned_num[MAX_SLOT_NUM][MAX_INSTANCE];
	char licreq[LICREQ_LEN];
	char licreq2[LICREQ_LEN];
};


enum hmd_reload_type
{
	HMD_RELOAD_MOD_NONE = 0,
	HMD_RELOAD_MOD_WCPSS = 1,	/* wcpss need restart */
	HMD_RELOAD_MOD_EAG = 2,
	HMD_RELOAD_MOD_DHCP = 3,
	HMD_RELOAD_MOD_RDC = 4,
	HMD_RELOAD_MOD_PDC = 5,
};

#define SEM_PATHNAME_HMD	"/opt/bin"
#define SEM_PROJ_ID_HMD		0x02

/* semaphore = vrrid * HMD_SEM_LOCK_MAX + LOCK_TYPE */
enum hmd_lock_type
{
	HMD_SEM_LOCK_WCPSS = 0,
	HMD_SEM_LOCK_EAG = 1,
	HMD_SEM_LOCK_DHCP = 2,
	HMD_SEM_LOCK_RDC = 3,
	HMD_SEM_LOCK_MAX = 4,
};
extern struct Hmd_Board_Info *HOST_BOARD;
extern struct Hmd_Board_Info *HMD_BOARD[MAX_SLOT_NUM];
extern struct Hmd_L_Inst_Mgmt_Summary *HMD_L_HANSI[MAX_INSTANCE];
extern struct LicenseMgmt *LICENSE_MGMT;
extern struct Hmd_Hansi_Group *hmd_group;
extern int global_ht_ip;
extern int global_ht_state;
extern int global_ht_opposite_ip;

#define VRRP_THREADS_CNT			(8)

#endif
