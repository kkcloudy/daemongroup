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
* eag_conf.h
*
*
* CREATOR:
* autelan.software.xxx. team
*
* DESCRIPTION:
* xxx module main routine
*
*
*******************************************************************************/
#ifndef _EAG_CONF_H
#define _EAG_CONF_H

#include <stdint.h>

/*default config*/
#define DEFAULT_PORTAL_PORT				2000
#define DEFAULT_PORTAL_RETRY_TIMES		0
#define DEFAULT_PORTAL_RETRY_INTERVAL	2

#define DEFAULT_ACCT_INTERVAL			900
#define DEFAULT_RADIUS_RETRY_INTERVAL	4
#define DEFAULT_RADIUS_RETRY_TIMES		1
#define DEFAULT_VICE_RADIUS_RETRY_TIMES	1

#define DEFAULT_REDIR_CHECK_INTERVAL	5
#define DEFAULT_MAX_REDIR_TIMES			35

#define DEFAULT_IDLE_TIMEOUT			900
#define DEFAULT_IDLE_FLOW				10240
#define DEFAULT_FLUX_INTERVAL			60	/* 20 */

#define DEFAULT_MACAUTH_FLUX_INTERVAL		2
#define DEFAULT_MACAUTH_FLUX_THRESHOLD		10240
#define DEFAULT_MACAUTH_CHECK_INTERVAL		300

#define MAX_RADIUS_SRV_NUM				16
#define MAX_RADIUS_DOMAIN_LEN			64
#define RADIUS_SECRETSIZE               128	/* No secrets that long */
#define RADIUS_CLASS_ATTR_SIZE          256

struct radius_srv_t {
	char domain[MAX_RADIUS_DOMAIN_LEN];
	int remove_domain_name;
	int class_to_bandwidth;

	uint32_t auth_ip;
	uint16_t auth_port;
	char auth_secret[RADIUS_SECRETSIZE];
	uint32_t auth_secretlen;

	uint32_t acct_ip;
	uint16_t acct_port;
	char acct_secret[RADIUS_SECRETSIZE];
	uint32_t acct_secretlen;

	uint32_t backup_auth_ip;
	uint16_t backup_auth_port;
	char backup_auth_secret[RADIUS_SECRETSIZE];
	uint32_t backup_auth_secretlen;

	uint32_t backup_acct_ip;
	uint16_t backup_acct_port;
	char backup_acct_secret[RADIUS_SECRETSIZE];
	uint32_t backup_acct_secretlen;
};

struct radius_conf{
	int current_num;
	struct radius_srv_t radius_srv[MAX_RADIUS_SRV_NUM];
};

struct radius_srv_t *
radius_conf_and_domain(struct radius_conf *radiusconf,
								char *domain);
int
radius_conf_del_domain(struct radius_conf *radiusconf,
								char *domain);
int 
radius_srv_set_auth(struct radius_srv_t *radius_srv,
			       uint32_t auth_ip,
					uint16_t auth_port,
			       char *auth_secret, size_t auth_secretlen);

int 
radius_srv_set_acct(struct radius_srv_t *radius_srv,
			       uint32_t acct_ip,
			       uint16_t acct_port,
			       char *acct_secret, size_t acct_secretlen);

int 
radius_srv_set_backauth(struct radius_srv_t *radius_srv,
				   uint32_t backup_auth_ip,
				   uint16_t backup_auth_port,
				   char *backup_auth_secret,
				   size_t backup_auth_secretlen);

int
radius_srv_set_backacct(struct radius_srv_t *radius_srv,
				   uint32_t backup_acct_ip,
				   uint16_t backup_acct_port,
				   char *backup_acct_secret,
				   size_t backup_acct_secretlen);

struct radius_srv_t *
radius_srv_get_by_domain(struct radius_conf *radiusconf,
						     char *domain);


/*portal config!!!!!*/
#define MAX_PORTAL_NUM			128
#define MAX_PORTAL_URL_LEN		256
#define MAX_PORTAL_KEY_BUFF_LEN	64
#define MAX_MULTPORTAL_ACNAME_LEN	32
#define MAX_PORTAL_URL_SUFFIX_LEN	64
#define MAX_DES_KEY_LEN			8

#define PORTAL_SECRETSIZE       128

typedef enum {
	PORTAL_KEYTYPE_ESSID,
	PORTAL_KEYTYPE_WLANID,
	PORTAL_KEYTYPE_VLANID,
	PORTAL_KEYTYPE_WTPID,
	PORTAL_KEYTYPE_INTF
} PORTAL_KEY_TYPE;


enum{
	WISPR_URL_NO,
	WISPR_URL_HTTP,
	WISPR_URL_HTTPS
};


struct portal_srv_t {
	PORTAL_KEY_TYPE key_type;
	union {
		char essid[MAX_PORTAL_KEY_BUFF_LEN];
		uint32_t wlanid;
		uint32_t vlanid;
		uint32_t wtpid;
		char intf[MAX_PORTAL_KEY_BUFF_LEN];
	} key;
	char portal_url[MAX_PORTAL_URL_LEN];
	uint16_t ntf_port;
	char domain[MAX_RADIUS_DOMAIN_LEN];
	char acname[MAX_MULTPORTAL_ACNAME_LEN];	
	int acip_to_url;
	int usermac_to_url;
	int clientmac_to_url;
	int apmac_to_url;
	int wlan_to_url;
	int redirect_to_url;
	int nasid_to_url;
	char url_suffix[MAX_PORTAL_URL_SUFFIX_LEN];	

	int	wlanparameter;//url param  usermac  des encrypt.
	char deskey[MAX_DES_KEY_LEN+2];

	int wlanuserfirsturl;//url param  user requrest url.
	char secret[PORTAL_SECRETSIZE];
	uint32_t secretlen;
	int wlanapmac;
	int wlanusermac;
	char wlanusermac_deskey[MAX_DES_KEY_LEN+2];

	int wisprlogin;//url param as wispr login 
	char wisprloginurl[MAX_PORTAL_URL_LEN];
	/*get by config! */
	uint32_t ip;
	uint32_t mac_server_ip;
	uint16_t mac_server_port;
};

struct portal_conf{
	int current_num;
	struct portal_srv_t portal_srv[MAX_PORTAL_NUM];	
};


int
portal_conf_add_srv( struct portal_conf *portalconf,
					PORTAL_KEY_TYPE key_type,
					void *key,
					char *portal_url, 
					uint16_t ntf_port,
					char *domain,
					uint32_t mac_server_ip,
					uint16_t mac_server_port);
int
portal_srv_del( struct portal_conf *portalconf,
					PORTAL_KEY_TYPE key_type,
					void *key );

struct portal_srv_t *
portal_srv_get_by_key( struct portal_conf *portalconf,
					   PORTAL_KEY_TYPE key_type, void *key);

int
portal_conf_del_srv(struct portal_conf *portalconf,
					PORTAL_KEY_TYPE key_type, void *key);

int
portal_conf_modify_srv( struct portal_conf *portalconf,
					PORTAL_KEY_TYPE key_type,
					void *key,
					char *portal_url, 
					uint16_t ntf_port,
					char *domain,
					uint32_t mac_server_ip,
					uint16_t mac_server_port );


/*nasid*/


typedef enum {
	NASID_KEYTYPE_WLANID,
	NASID_KEYTYPE_VLANID,
	NASID_KEYTYPE_WTPID,
	NASID_KEYTYPE_IPRANGE,
	NASID_KEYTYPE_INTF
} NASID_KEY_TYPE;

#define MAX_NASID_NUM	512
#define MAX_NASID_KEY_BUFF_LEN	64
#define MAX_NASID_LEN			64
#define MAX_CON_ID_LEN			3

struct iprange_t{
	uint32_t ip_begin;
	uint32_t ip_end;
};

struct idrange_t{
	unsigned long id_begin;
	unsigned long id_end;
};
struct nasid_map_t {
	NASID_KEY_TYPE key_type;
	union {
		#if 0
		uint32_t wlanid;	/* keep for compatibility, not used now */
		uint32_t vlanid;	/* keep for compatibility, not used now */
		uint32_t wtpid;		/* keep for compatibility, not used now */
		#endif
		struct idrange_t wlanidrange;
		struct idrange_t vlanidrange;
		struct idrange_t wtpidrange;
		struct iprange_t iprange;
		char intf[MAX_NASID_KEY_BUFF_LEN];
	} key;
	char nasid[MAX_NASID_LEN];
	uint32_t conid;
};

struct nasid_conf{
	int current_num;
	struct nasid_map_t nasid_map[MAX_NASID_NUM];
};

int
nasid_conf_add_map( struct nasid_conf *nasidconf,
				    NASID_KEY_TYPE key_type,
				    void *key, char *nasid, uint32_t conid);

int
nasid_conf_modify_map(struct nasid_conf *nasidconf,
			NASID_KEY_TYPE key_type,
			void *key, char *nasid, uint32_t conid);

int
nasid_conf_del_map( struct nasid_conf *nasidconf,
				    NASID_KEY_TYPE key_type,
				    void *key );

int
nasid_conf_get_map_by_key(struct nasid_conf *nasidconf,
					   NASID_KEY_TYPE key_type, void *key);


/*nas port id*/
#define MAX_NASPORTID_NUM	2048
#define MAX_MAPED_VLANID	99999999
#define MAX_WLANID_INPUT	128
#define MAX_WTPID_INPUT		8192
#define MAX_VLANID_INPUT	4096

/*for dcli*/
#define DEFUN_MAX_NASPORTID_NUM		"2048"
#define DEFUN_MAX_MAPED_VLANID		"99999999"
#define DEFUN_MAX_MAPED_NASPORTID 	"99999999"
#define DEFUN_MAX_VLANID_INPUT		"4096"
#define DEFUN_MAX_WLANID_INPUT		"128"
#define DEFUN_MAX_WTPID_INPUT		"8192"

typedef enum {
	NASPORTID_KEYTYPE_WLAN_WTP,
	NASPORTID_KEYTYPE_VLAN
} NASPORTID_KEY_TYPE;

struct wlan_wtp_range_t {
	uint32_t wlanid_begin;
	uint32_t wlanid_end;
	uint32_t wtpid_begin;
	uint32_t wtpid_end;
};

struct vlan_range_t {
	uint32_t vlanid_begin;
	uint32_t vlanid_end;
};

struct nasportid_map_t {
	NASPORTID_KEY_TYPE key_type;
	union {
		struct wlan_wtp_range_t wlan_wtp;
		struct vlan_range_t vlan;
	} key;
	uint32_t nasportid;
};

struct nasportid_conf{
	int current_num;
	struct nasportid_map_t nasportid_map[MAX_NASPORTID_NUM];
};

int
nasportid_conf_add_map_by_wlan( struct nasportid_conf *nasportidconf,
					uint32_t wlanid_begin, uint32_t wlanid_end,
					uint32_t wtpid_begin, uint32_t wtpid_end,
					uint32_t nasportid);


int
nasportid_conf_del_map_by_wlan( struct nasportid_conf *nasportidconf,
					uint32_t wlanid_begin, uint32_t wlanid_end,
					uint32_t wtpid_begin, uint32_t wtpid_end,
					uint32_t nasportid);
int
nasportid_conf_add_map_by_vlan(struct nasportid_conf *nasportidconf,
					uint32_t vlanid_begin, uint32_t vlanid_end,
					uint32_t nasportid);

int
nasportid_conf_del_map_by_vlan(struct nasportid_conf *nasportidconf,
					uint32_t vlanid_begin, uint32_t vlanid_end,
					uint32_t nasportid);

#if 0
uint32_t
nasportid_conf_get_vlan_maped( struct nasportid_conf *nasportidconf,
					uint32_t wlanid, uint32_t wtpid );

#endif

#endif

