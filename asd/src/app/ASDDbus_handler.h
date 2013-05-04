#ifndef _ASDDBUS_HANDLER_H_
#define _ASDDBUS_HANDLER_H_

int update_sta_traffic_limit_info(struct asd_data *wasd,unsigned char include_vip);
/*xm0723*/
struct asd_data * AsdCheckBSSIndex(unsigned int BSSIndex);

unsigned char FindWapiWlan(WID_WLAN ** WLAN,unsigned char *wlan_num); /*nl add 20100607*/
unsigned int Asd_Find_Wtp(ASD_WTP_ST **WTP);/*nl add 20100607*/
unsigned int ASD_Find_Running_Wtp(ASD_WTP_ST **WTP);/*20100721 nl*/

/*wlan[] is designed for avoiding repeatly record the situation that different radio of the same wtp bind the same wlan by nl 20100617*/
unsigned int ASD_FIND_WLAN_BY_WTPID(unsigned int WTPID,unsigned int wlan[],WID_WLAN	**WLAN);
/*=====add for check whether the id is incorrect by nl 20100804======*/
/*===============================================*/
int ASD_CHECK_WTP_ID(u_int32_t Id);
int ASD_CHECK_WLAN_ID(u_int32_t Id);
int ASD_CHECK_G_RADIO_ID(u_int32_t Id);
int ASD_CHECK_SECURITY_ID(u_int32_t Id);
int ASD_CHECK_BSS_ID(u_int32_t Id);
int ASD_CHECK_ID(unsigned int TYPE,u_int32_t Id);
/*===============================================*/


/*xm0723*/
unsigned int ASD_STA_SUMMARY(unsigned int wtp[],unsigned wlan[]);	//	0610xm
struct asd_stainfo* ASD_SEARCH_STA(unsigned char *sa);
int ASD_SEARCH_ALL_STA(struct asd_data **bss);
int ASD_SEARCH_WLAN_STA(unsigned char WlanID, struct asd_data **bss);
int ASD_SEARCH_BSS_BY_RADIO_BSS(unsigned int radioid, unsigned char bssid, struct asd_data **bss);
int AsdCheckWTPID(unsigned int WTPID);
int ASD_SEARCH_WTP_STA(unsigned int WTPID, struct asd_data **bss);
int ASD_ADD_SECURITY_PROFILE(char *name, unsigned char ID);
int ASD_DELETE_SECURITY_PROFILE(unsigned char ID);
int ASD_SET_ACCT(unsigned char ID, unsigned int port, char *IP, char *secret);
int ASD_SET_AUTH(unsigned char ID, unsigned int port, char *IP, char *secret);
int ASD_SECONDARY_SET_ACCT(unsigned char ID, unsigned int port, char *IP, char *secret);
int ASD_SECONDARY_SET_AUTH(unsigned char ID, unsigned int port, char *IP, char *secret);
int ASD_SECURITY_PROFILE_CHECK(unsigned char ID);
int ASD_WLAN_INF_OP(unsigned char WLANID, unsigned int SecurityID, Operate op);
void SECURITY_ENCYPTION_MATCH(unsigned char SecurityID, unsigned int security_type);
int Clear_SECURITY(unsigned char SecurityID);
int bss_kick_sta(struct asd_data *bss, struct sta_info * sta);
int add_ASD_BSS_only_acl_conf(unsigned int bssindex);
int Clear_WLAN_APPLY(unsigned char SecurityID);

/* zhangshu copy from 1.2, 2010-09-13 */
//zhaoruijia,判断当前的radio是否绑定当前的wtp
int ASD_IS_WTP_BIND_RADIO(unsigned int WTPID,unsigned int RADIOID);
//zhaoruijia,判断当前wlanid是否与当前的radio绑定
int ASD_IS_RADIO_BIND_WLAN(unsigned int RADIOID,unsigned char WlanID,int bsscount);

//mahz add 2010.12.9 
int ASD_WAPI_RADIUS_AUTH_SET_USER_PASSWD(unsigned char ID, char *passwd);
int ASD_SEARCH_BSS_HAS_STA(struct asd_data **bss);
int ASD_SEARCH_BSS_BY_RADIO_BSS(unsigned int radioid, unsigned char bssid, struct asd_data **bss);
struct asd_data * AsdCheckBSSIndex(unsigned int BSSIndex);
int ASD_SEARCH_ALL_STA(struct asd_data **bss);
int AsdCheckRadioID(unsigned int radioid);
int ASD_SEARCH_WTP_HAS_STA(unsigned int WTPID, struct asd_data **bss);
int Create_WIRED_node(int VLANID, int PORTID, unsigned char securityID);
void SECURITY_WAPI_INIT(unsigned char SecurityID);
int ASD_DEL_WAPI_CERT(unsigned char ID, unsigned int path_type);
int ASD_SET_WAPI_CERT_PATH(unsigned char ID, unsigned int path_type,  char *path);
int ASD_SET_WAPI_AUTH(unsigned char ID, unsigned int type, char *IP/*, char *path*/);
int ASDReInit();
int ASD_SEARCH_ALL_WIRED_STA(struct asd_data **bss);
int ASD_SEARCH_BSS_BY_WLANID(unsigned int radioid, unsigned char WlanID, struct asd_data **bss);
int ASD_SEARCH_ALL_BSS_NUM(struct asd_data **bss);
int ASD_SEARCH_WTP_BSS(unsigned int WTPID, struct asd_data **bss);
int ASD_SEARCH_RADIO_STA(unsigned int RADIOID, struct asd_data **bss);
int ASD_SEARCH_WTP_STA_NUM(unsigned int WTPID);
struct asd_ip_secret *asd_ip_secret_get(u32 ip);
struct asd_ip_secret *asd_ip_secret_add(u32 ip);
void asd_ip_secret_del(u32 ip);
//qiuchen add it for master_bak radius server 2012.12.16
int ASD_SEC_RADIUS_CHECK(unsigned char ID);

//end
int ASD_GET_R_STA_BYWTP(struct r_sta_wlan_info *r_sta_wtp);
int ASD_GET_R_STA_BYWLAN(struct r_sta_wlan_info *r_sta_wlan);
int ASD_GET_R_STA_BYCLASS (struct r_sta_wlan_info *r_sta_class);
int ASD_GET_CNUM(struct r_sta_wlan_info *r_sta_wlan,unsigned char type);
int ASD_GET_BSS_BYRADIO(struct asd_bss_summary_info *bss_summary,int id,int *circlenum);
int ASD_GET_BSS_BYWLAN(struct asd_bss_summary_info *bss_summary,int id,int *circlenum);
int ASD_GET_BSS_BYWTP(struct asd_bss_summary_info *bss_summary,int id,int *circlenum);


#endif
