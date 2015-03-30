/*******************************************************************************
Copyright (C) Autelan Technology


This software file is owned and distributed by Autelan Technology 
********************************************************************************


THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND 
ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED 
WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE 
DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE LIABLE FOR 
ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES 
(INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; 
LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON 
ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT 
(INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS 
SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
********************************************************************************
* ASDStaInfo.h
*
*
* CREATOR:
* autelan.software.WirelessControl. team
*
* DESCRIPTION:
* asd module
*
*
*******************************************************************************/
 


#ifndef STA_INFO_H
#define STA_INFO_H

#define ASSOC_UPDATE_TIME (60*10)
#define BRCTL_ADD_SFDB 20
#define BRCTL_DEL_SFDB 21

#define STA_ROAM_TYPE_NONE		(0)
#define STA_ROAM_TYPE_L2		(1)
#define STA_ROAM_TYPE_L3		(2)

int kick_sta_mac(unsigned char mac[]);

int ap_for_each_sta(struct asd_data *wasd,
		    int (*cb)(struct asd_data *wasd, struct sta_info *sta,
			      void *ctx),
		    void *ctx);

void get_flow_timer_handler(void *circle_ctx, void *timeout_ctx);	//	xm0715

struct sta_info * ap_get_sta(struct asd_data *wasd, const u8 *sta);
void ap_sta_hash_add(struct asd_data *wasd, struct sta_info *sta);
void ap_free_sta(struct asd_data *wasd, struct sta_info *sta, unsigned int state);
void ap_free_sta_without_wsm(struct asd_data *wasd, struct sta_info *sta, unsigned int state);
//void ap_free_sta(struct asd_data *wasd, struct sta_info *sta);
void asd_free_stas(struct asd_data *wasd);
/*超时重发处理*/
void wapi_retry_timer(void *circle_ctx, void *timeout_ctx);
void del_wids_mac_timer(void *circle_ctx, void *timeout_ctx);
void assoc_update_timer(void *circle_ctx, void *timeout_ctx);
void sta_info_to_wsm_timer(void *circle_ctx, void *timeout_ctx);
void sta_auth_timer(void *circle_ctx, void *timeout_ctx);
void ap_handle_timer(void *circle_ctx, void *timeout_ctx);
void ap_sta_session_timeout(struct asd_data *wasd, struct sta_info *sta,
			    u32 session_timeout);
void ap_sta_no_session_timeout(struct asd_data *wasd,
			       struct sta_info *sta);
struct sta_info * ap_sta_add(struct asd_data *wasd, const u8 *addr, int both);
void ap_sta_disassociate(struct asd_data *wasd, struct sta_info *sta,
			 u16 reason);
void ap_sta_deauthenticate(struct asd_data *wasd, struct sta_info *sta,
			   u16 reason);
int ap_sta_bind_vlan(struct asd_data *wasd, struct sta_info *sta,
		     int old_vlanid);
void ap_free_sta_for_pmk(struct asd_data *wasd, struct sta_info *sta);

struct sta_static_info *asd_get_static_sta(const u8 *sta);
void asd_static_sta_hash_add(struct sta_static_info *sta);
int asd_static_sta_hash_del(struct sta_static_info *sta,unsigned char wlanid);
int add_and_del_static_br_fdb(const char *brname,const char *ifname, const unsigned char * addr,int isadd)	;
int asd_get_ip_v2(struct sta_info *sta);
int UpdateStaInfoToCHILL(unsigned int BSSIndex, struct sta_info *sta, Operate op);
int UpdateStaInfoToWSM(struct asd_data *wasd, const u8 *addr, Operate op);
void wlan_free_stas(unsigned char wlanid);
int UpdateStaInfoToEAG(unsigned int BSSIndex, struct FLASHDISCONN_STAINFO *sta, Operate op);
void ap_sta_idle_timeout(void *circle_ctx,void *timeout_ctx);
void ap_sta_eap_auth_timer(void *circle_ctx,void *timeout_ctx);
int check_sta_authorized(struct asd_data *wasd,struct sta_info *sta);
struct sta_info *ap_get_sta_acct_id(const unsigned char *acct_id);
//int radius_dm_kick_sta(const unsigned char *acct_id);
int ap_kick_eap_sta(struct sta_info *sta);
struct asd_data *get_wasd_by_radius_ip(const unsigned int ip,const int port);
unsigned int sta_acct_get_key(u8 *acct_id);
struct sta_acct_info *sta_acct_info_get(u8 *acct_id);
void sta_acct_info_add(struct sta_info *sta);
void sta_acct_info_del(u8 *acct_id);
struct sta_info * asd_sta_hash_get(const u8 *sta);
void asd_sta_hash_add(struct sta_info *sta);
int AsdStaInfoToEAG(struct asd_data *wasd, struct sta_info *sta, Operate op);
int asd_notify_to_protal(uint32_t userip, uint8_t *usermac); /* yjl add for mac_auth in tl */
void asd_sta_roaming_management(struct sta_info *new_sta);
/*****xk add for asd sta check*****/
void asd_sta_check(void *circle_ctx,void *timeout_ctx);
void asd_sta_check_all(void);
void asd_sta_check_wtp(unsigned wtpid);
/**********************************/
#ifdef __ASD_STA_ACL
/* caojia add for sta acl function */
int AsdStaInfo2Wifi(struct asd_data *wasd, struct sta_info *sta, Operate op);
#endif

/*yjl copy from aw3.1.2 for local forwarding.2014-2-28*/
#if 0
typedef struct sta_hash_info
{
	struct hlist_node hw_hlist;			/* haddr hlist */
#if 0
	struct hlist_node ip_hlist;			/* ip addr hlist */
	struct hlist_node id_hlist;			/* name hlist */
#endif

	unsigned int bssindex, first_bssindex;
	unsigned int wlanid, first_wlanid;
	unsigned int wtpid, first_wtpid;
	unsigned int g_radioid, first_g_radioid;
	unsigned int l_radioid, first_l_radioid;

	unsigned char BSSID[MAC_LEN];			/* cur BSSID sta access */
	unsigned char first_BSSID[MAC_LEN];

	/* if sta roam, save roam out bssindex, then free roam sta info at right time and clean "last_bssindex" */
	unsigned int last_bssindex;
	
	unsigned char haddr[MAC_LEN];			/* STA haddr */
	unsigned flag;
	unsigned short vlanid;
}sta_hash_t;
#endif

int b_virdhcp_handle(struct asd_data *wasd, struct sta_info *sta, unsigned int viripaddr, unsigned int addflag);
int asd_virdhcp_handle(struct asd_data *wasd, struct sta_info *sta, unsigned int addflag);
extern struct ip_info* dhcp_assign_ip(struct vir_dhcp * vdhcp, unsigned char *mac);
extern struct ip_info* dhcp_release_ip(struct vir_dhcp * vdhcp, unsigned int ip, unsigned char *mac);
/*end**************************************************/
#endif /* STA_INFO_H */
