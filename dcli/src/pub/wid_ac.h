#ifndef _WID_AC_H
#define _WID_AC_H

#define DCLIAC_MAC_LEN	6
#define DCLIAC_RADIO_NUM_LEN	4
#define DCLIAC_OUI_LEN	3
#define DCLIAC_BUF_LEN	16

#define WIDS_TYPE_LEN 16
#define CHECK_COMMA 	','


typedef struct
{
    unsigned char       oui[DCLIAC_OUI_LEN];
}OUI_S;

typedef struct
{
    unsigned int min;
	unsigned int max;
	unsigned int num;
}batch_arg;

void CheckWIDSType(char *pattacktype, char* pframetype, unsigned char attacktype,unsigned char frametype);


typedef struct
{
    unsigned char       macaddr[DCLIAC_MAC_LEN];
}WIDMACADDR;

struct radio_detail_info{
	char radio_type;
	char radio_id;
	char bss_count;
	char reserved;
	
};

struct model_detail_info
{
	char *str_ap_model; 
	char *str_ap_version_name;
	char *str_ap_version_path;
	unsigned char radio_num;
	unsigned char bss_num;
	char ischanged;
	char ismodelchanged;
	struct radio_detail_info radio_info[DCLIAC_RADIO_NUM_LEN];
	char *str_ap_code;
};



int wid_mac_format_check(char* str,int len) ;


 int wid_parse_mac_addr(char* input,WIDMACADDR* macAddr) ;
  

int parse_country_code(char *input);
 

void CheckWIDSType(char *pattacktype, char* pframetype, unsigned char attacktype,unsigned char frametype);


int Check_Batch_Command_Format(char* str, batch_arg *arg);

int parse_str_by_character(char* str, char *arg, char ch);

int parse_daemonlog_level(int Loglevel, char *arg);


int dcli_add_attack_ap_mac(unsigned char mac[],ATTACK_MAC_LIST_S	**attacklist);
int dcli_add_legal_essid(char *essid,ESSID_LIST_S **dcli_essid_list);


int dcli_add_manufacturer_oui(unsigned char oui[],OUI_LIST_S **oui_list);

int dcli_add_ap_statics_node(Ap_statics_INFOS **list,wlan_stats_info *ele);

int dcli_add_bak_sock_node(Bak_Sock_INFO **b_list,struct bak_sock *b_sock);

int dcli_add_ap_conf_info_node(Config_Ver_Info **list,CWConfigVersionInfo_dcli *ele);

int dcli_add_rogue_ap_ele_fun(Neighbor_AP_INFOS **paplist,struct Neighbor_AP_ELE *elem);


int dcli_add_wids_ap_ele_fun(wid_wids_device **paplist,struct tag_wids_device_ele *elem);

void* dcli_ac_show_api_group_one(
	int index,
	unsigned int id1,
	unsigned int id2,
	unsigned int id3,
	unsigned int* ret,
	unsigned int* num2,
	unsigned int* num3,
	//DCLI_AC_API_GROUP_ONE **LIST,
	DBusConnection *dcli_dbus_connection,
	char *DBUS_METHOD
	);

//fengwenchao add 20101220
void* show_neighbor_ap_list_cmd_allap(int localid,int index,DBusConnection *dbus_connection,int* wtp_num,int* ret,int* ret1);

void* dcli_ac_show_api_group_two(
	int index,
	unsigned int id,
	unsigned int id1,
	unsigned int id2,
	unsigned int id3,
	unsigned int* num,
	unsigned int* num2,
	unsigned int* num3,
	//DCLI_AC_API_GROUP_TWO *LIST,
	DBusConnection *dcli_dbus_connection,
	char *DBUS_METHOD
	);


void* dcli_ac_show_api_group_three(
	int index,
	unsigned int id,
	unsigned int id1,
	unsigned int id2,
	unsigned int id3,
	unsigned int* num,
	unsigned int* num2,
	unsigned int* num3,
	//DCLI_AC_API_GROUP_THREE *LIST,	
	DBusConnection *dcli_dbus_connection,
	char *DBUS_METHOD
	);


void* dcli_ac_show_api_group_four(
	int index,
	unsigned int id,
	unsigned int id1,
	unsigned int id2,
	unsigned int *ret,
	char *char1,
	char *char2,
	int localid,
	//DCLI_AC_API_GROUP_FOUR*LIST,	
	DBusConnection *dcli_dbus_connection,
	char *DBUS_METHOD
	);


void* dcli_ac_show_api_group_five(
	int index,
	unsigned int dcli_sn,
	unsigned int id1,
	unsigned int id2,
	unsigned int id3,
	unsigned int *ret,
	char *char1,
	char *char2,
	int localid,
	//DCLI_AC_API_GROUP_FIVE *LIST,	
	DBusConnection *dcli_dbus_connection,
	char *DBUS_METHOD
	);
void dcli_ac_free_fun(char *DBUS_METHOD,DCLI_AC_API_GROUP_ONE *LIST);
void dcli_ac_free_fun_two(char *DBUS_METHOD,DCLI_AC_API_GROUP_TWO*LIST);
void dcli_ac_free_fun_three(char *DBUS_METHOD,DCLI_AC_API_GROUP_THREE*LIST);
void dcli_ac_free_fun_four(char *DBUS_METHOD,DCLI_AC_API_GROUP_FOUR*LIST);
void dcli_ac_free_fun_five(char *DBUS_METHOD,DCLI_AC_API_GROUP_FIVE*LIST);
int dcli_ac_set_dynamic_channel_selection_range(
	int localid,
	int index,
	unsigned int rangeNum,
	unsigned char *channelRange,
	DBusConnection *dcli_dbus_connection
	);
int dcli_ac_add_listen_if_node(Listen_IF *listen_if,char *ifname,unsigned int addr,LISTEN_FLAG flag);
int dcli_ac_free_listen_if_node(Listen_IF *listen_if);
void* dcli_ac_show_wid_listen_if(
	int localid,
	int index,
	int *ret,
	DBusConnection *dcli_dbus_connection
	);
typedef enum{
	c_first_id=0,
	c_sub,
	c_fail,
	c_end,
	c_success,
	c_second_id
}id_state;
#define PARSE_ID_IFNAME_SUB '-'

#endif

