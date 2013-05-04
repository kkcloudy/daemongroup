#ifndef _DCLI_EAG_H_
#define _DCLI_EAG_H_

#include "vty.h"
#include "ws_eag_conf.h"

#define EAG_SUCCESS		0
#define EAG_FAILURE		1

/* nas policy */
#define MAX_NAS_BEGIN_POINT_LEN		32
#define MAX_NAS_END_POINT_LEN		32
//#define MAX_NASID_LEN				32

/* radius policy */
//#define MAX_RADIUS_DOMAIN_LEN		128
#define MAX_RADIUS_KEY_LEN		128

/* portal policy */
#define MAX_PORTAL_SSID_LEN		32
#define MAX_PORTAL_WEBURL_LEN  256
#define MAX_PORTAL_ACNAME_LEN 32
#define MAX_PORTAL_DOMAIN_LEN   MAX_RADIUS_DOMAIN_LEN

#define FALSE_EAG_VTY		0
#define TRUE_EAG_VTY		!FALSE_EAG_VTY

/*eag_log :  syslog level define*/
#define LOG_EMERG_MASK			(LOG_MASK(LOG_EMERG))
#define LOG_ALERT_MASK			(LOG_MASK(LOG_ALERT))
#define LOG_CRIT_MASK			(LOG_MASK(LOG_CRIT))
#define LOG_ERR_MASK			(LOG_MASK(LOG_ERR))
#define LOG_WARNING_MASK		(LOG_MASK(LOG_WARNING))
#define LOG_NOTICE_MASK			(LOG_MASK(LOG_NOTICE))
#define LOG_INFO_MASK			(LOG_MASK(LOG_INFO))
#define LOG_DEBUG_MASK			(LOG_MASK(LOG_DEBUG))
#define LOG_USER_MASK			(LOG_MASK(LOG_USER))

#define LOG_AT_LEAST_ALERT		LOG_EMERG_MASK|LOG_ALERT_MASK
#define LOG_AT_LEAST_CRIT		LOG_AT_LEAST_ALERT|LOG_CRIT_MASK
#define LOG_AT_LEAST_ERR		LOG_AT_LEAST_CRIT|LOG_ERR_MASK
#define LOG_AT_LEAST_WARNING	LOG_AT_LEAST_ERR|LOG_WARNING_MASK
#define LOG_AT_LEAST_NOTICE		LOG_AT_LEAST_WARNING|LOG_NOTICE_MASK
#define LOG_AT_LEAST_INFO		LOG_AT_LEAST_NOTICE|LOG_INFO_MASK
#define LOG_AT_LEAST_DEBUG		LOG_AT_LEAST_INFO|LOG_DEBUG_MASK
#define LOG_AT_LEAST_USER		LOG_AT_LEAST_DEBUG|LOG_USER_MASK

/* nas policy */
#define IS_PRINT(c) (c >= 32 && c <= 126)   /* note : better to use isprint or isgraph function */
int eag_nas_policy_is_legal_nasid(const char *strnasid);
int eag_nas_policy_has_item(int policy_id, const char *attr);
int eag_nas_policy_add_item(int policy_id, const char *attr, const char *nas_type, const char *begin_point, const char *end_point, const char *nasid, const char *syntaxis_point);
int eag_nas_policy_del_item(int policy_id, const char *nas_type);
int eag_nas_policy_get_items(int policy_id, struct st_nasz *chead, int *num);
#define eag_nas_policy_is_legal_strid(strid)	(strlen(strid) == 1 && strid[0] >= '1' && strid[0] <= '5')
void show_nas_policy(struct vty *vty, int policy_id);
int eag_nas_policy_is_ready(int policy_id, char *err, int err_size);
int eag_nas_policy_being_used(int policy_id, int *idset);

/* radius policy */
int eag_check_ip_format(const char *str);
int eag_check_port_format(const char *str);
int eag_radius_policy_has_item(int policy_id, const char *domain_name);
int eag_radius_policy_modify_param(int policy_id, const char *domain_name, const char *key, const char *value);
int eag_radius_policy_del_item(int policy_id, const char *domain_name);
int eag_radius_policy_get_items(int policy_id, struct st_radiusz *chead, int *num);
#define eag_radius_policy_is_legal_strid(strid)	(strlen(strid) == 1 && strid[0] >= '1' && strid[0] <= '5')
int eag_radius_policy_get_item_by_domain(int policy_id, const char *domain_name, struct st_radiusz *cq);
#define eag_radius_policy_is_legal_strplcid_with_all(strplcid)	(strcmp(strplcid, "all") == 0 \
				|| (strlen(strplcid) == 1 && strplcid[0] >= '1' && strplcid[0] <= '5'))
void show_radius_policy(struct vty *vty, int policy_id, const char *domain_name, const char *view);
int eag_radius_policy_is_ready(int policy_id, char *err, int err_size);
int eag_radius_policy_being_used(int policy_id, int *idset);

/* portal policy */
int eag_portal_policy_has_item(int portal_id, const char *ssid);
int eag_portal_policy_modify_param(int portal_id, const char *ssid, const char *key, const char *value);
int eag_portal_policy_del_item(int portal_id, const char *ssid);
int eag_portal_policy_get_items(int policy_id, struct st_portalz *chead, int *num);
#define eag_portal_policy_is_legal_strid(strid)	(strlen(strid) == 1 && strid[0] >= '1' && strid[0] <= '5')
int eag_portal_policy_is_legal_ssid(const char *strssid);
int eag_portal_policy_get_item_by_ssid(int policy_id, const char *ssid, struct st_portalz *cq);
void show_portal_policy(struct vty *vty, int policy_id);
int eag_portal_policy_is_ready(int policy_id, char *err, int err_size);
int eag_portal_policy_being_used(int policy_id, int *idset);

/* vlanmap policy */
int eag_vlanmap_policy_add_item(int policy_id, const char *wlanid_begin, const char *wlanid_end, 
							const char *wtpid_begin, const char *wtpid_end, const char *vlanid);
int eag_vlanmap_policy_has_item(int policy_id, const char *wlanid_begin, const char *wlanid_end, const char *wtpid_begin, const char *wtpid_end);
int eag_vlanmap_policy_del_item(int policy_id, const char *wlanid_begin, const char *wlanid_end, const char *wtpid_begin, const char *wtpid_end);
int eag_vlanmap_policy_get_items(int policy_id, struct st_wwvz *chead, int *num);
#define eag_vlanmap_policy_is_legal_strid(strid)	(strlen(strid) == 1 && strid[0] >= '1' && strid[0] <= '5')
void show_vlanmap_policy(struct vty *vty, int policy_id);
int eag_vlanmap_policy_being_used(int policy_id, int *idset);

/* eag instance */
int eag_ins_set_param(int ins_id, const char *conf_key, const char *conf_value);
int eag_ins_exist(int ins_id);
int eag_ins_del(int ins_id);
int eag_ins_get(int ins_id, struct st_eagz *cq);
#define eag_ins_is_legal_strid(strid)	(strlen(strid) == 1 && strid[0] >= '1' && strid[0] <= '5')
void show_eag_instance(struct vty *vty, int ins_id);
int eag_ins_is_ready(int ins_id, char *err, int err_size);
int eag_ins_is_running(int ins_id);

/* eag 2.0 */

void 
dcli_eag_init(void);

#endif

