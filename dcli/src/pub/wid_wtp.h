#ifndef _WID_WTP_H
#define _WID_WTP_H


#define WTP_ARRAY_NAME_LEN 20
#define WTP_COMMAND_LEN 256
#define WTP_IP_BUFF_LEN 16
#define WTP_WLAN_VLAN_LEN 4
#define WTP_WTP_IP_LEN 21

struct wtp_wlan_vlan_info{
	unsigned char	wlanid;
	unsigned int	vlanid;
	unsigned char	priority;
};
typedef enum{
	dcli_wtp_check_wtpid=0,
	dcli_wtp_check_comma,
	dcli_wtp_check_fail,
	dcli_wtp_check_end,
	dcli_wtp_check_success,
	dcli_wtp_check_bar
}wtp_list_state;
#define WTP_LIST_SPLIT_COMMA 	','	
#define WTP_LIST_SPLIT_BAR 	'-'	

void str2higher(char **str) ;

unsigned long wid_ip2ulong(char *str);

void CheckWTPState(char *state, unsigned char WTPstate);

void CheckWTPQuitReason(char *quitreason,unsigned char quitstate);

int parse_int_ID(char* str,unsigned int* ID);
int wtp_check_sta_num_asd(int index,int localid,DBusConnection *dbus_connection,int wtpid);

int read_ac_info(char *FILENAME,char *buff);

void DcliWReInit();

void DcliWInit();

char * WID_parse_ap_extension_command(const char **argv, int argc);

int WID_Check_IP_Format(char* str);

int WID_Check_Mask_Format(char* str);

char* WID_parse_CMD_str(const char **argv, int argc, const char *buf, int preflag);

int wid_wtp_parse_char_ID(char* str,unsigned char* ID);


int wtp_check_wtp_ip_addr(char *ipaddr, char *WTPIP);

int check_ip_with_mask(unsigned long ipaddr,unsigned long mask,char * WTPIP);

int check_mac_with_mask(WIDMACADDR *macaddr,WIDMACADDR *macmask,char *WTPMAC);

int dcli_wtp_remove_list_repeat(int list[],int num);

//int dcli_wtp_parse_wtp_list(char* ptr,int* count,int wtpId[]);

//int parse_wtpid_list(char* ptr,update_wtp_list **wtplist);

void destroy_input_wtp_list(update_wtp_list *pwtplist);

void delsame(update_wtp_list *pwtplist);

int dcli_wtp_add_wlanid_node(WID_WTP **list,struct wlanid *ele);

int dcli_wtp_method_parse_fist(char *DBUS_METHOD);
int dcli_wtp_method_parse_two(char *DBUS_METHOD);
int dcli_wtp_method_parse_three(char *DBUS_METHOD);

int dcli_wtp_add_wtp_node(WID_WTP_INFO**list,WID_WTP *ele);
void* dcli_wtp_show_api_group_one(
	int index,
	unsigned int id,
	unsigned int id1,
	unsigned int id2,
	unsigned int id3,
	unsigned int* num,
	unsigned int* ret,
	unsigned int* num3,
	unsigned char *num4,
	unsigned char *num5,
	int *num6,
	//DCLI_WTP_API_GROUP_ONE *LIST,	
	DBusConnection *dcli_dbus_connection,
	char *DBUS_METHOD
	);


void* dcli_wtp_show_api_group_two(
	int index,
	unsigned int id,
	unsigned int id1,
	unsigned int id2,
	unsigned int id3,
	unsigned int* num,
	unsigned int* num2,
	unsigned int* num3,
	unsigned char *num4,
	unsigned char *num5,
	int *num6,
//	DCLI_WTP_API_GROUP_TWO *LIST,	
	DBusConnection *dcli_dbus_connection,
	char *DBUS_METHOD
	);


void* dcli_wtp_show_api_group_three(
	int index,
	unsigned int id,
	unsigned int id1,
	unsigned int id2,
	unsigned int id3,
	unsigned int* num,
	unsigned int* num2,
	unsigned int* num3,
	unsigned char *num4,
	unsigned char *num5,
	int *num6,
//	DCLI_WTP_API_GROUP_THREE *LIST,	
	DBusConnection *dcli_dbus_connection,
	char *DBUS_METHOD
	);
void dcli_wtp_free_fun(char *DBUS_METHOD,DCLI_WTP_API_GROUP_ONE*WTP);

#endif


