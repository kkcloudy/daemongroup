#include "includes.h"
#include "asd.h"
#include "wcpss/asd/asd.h"
#include "ASDDbus_handler.h"
#include "ASDStaInfo.h"
#include "wcpss/wid/WID.h"




#define ASD_STA_SERVER_PATH  "/var/run/wcpss/asd_sta"

struct mac_info
{
	unsigned char mac[MAC_LEN];
	struct mac_info *hnext;
	struct mac_info *next;
};


struct manage_stainfo{
	struct mac_info *mac_list;
	struct mac_info *sta_hash[STA_HASH_SIZE];
	unsigned int num;
};
void *asd_sta_manage();
int asd_init_socket_server(char *path);
int asd_init_send_socket();
int notice_asd_del_sta(int bssindex,int num,struct manage_stainfo *maclist);
int maclist_match(struct manage_stainfo *sta_manage_list,struct manage_stainfo *sta_report_list);
int notice_ap_del_sta(int bssindex,unsigned char wlanid ,int wtpid ,struct manage_stainfo *maclist);
int maclist_add_mac(struct manage_stainfo *maclist,const unsigned char *mac);
struct mac_info * maclist_get_mac(struct manage_stainfo *maclist,const unsigned char *mac);
int maclist_del_mac(struct manage_stainfo *maclist,const unsigned char *mac);
int mac_hash_add_mac(struct manage_stainfo *maclist,struct mac_info *sta);
int mac_hash_del_mac(struct manage_stainfo *maclist,const unsigned char *mac);
int clean_maclist(struct manage_stainfo *maclist);
struct manage_stainfo *malloc_init_mac_list();

