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
* Asd80211Auth.c
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

#include "includes.h"


#include "asd.h"
#include "ASD80211Op.h"
#include "ASD80211AuthOp.h"
#include "ASDRadius/radius.h"
#include "ASDRadius/radius_client.h"
#include "wcpss/waw.h"
#include "wcpss/wid/WID.h"
#include "wcpss/asd/asd.h"
#include "circle.h"
#include "asd_bak.h"

#define RADIUS_ACL_TIMEOUT 30
extern struct acl_config ac_acl_conf;


struct asd_cached_radius_acl {
	time_t timestamp;
	macaddr addr;
	int accepted; /* asd_ACL_* */
	struct asd_cached_radius_acl *next;
	u32 session_timeout;
	u32 acct_interim_interval;
	int vlan_id;
};


struct asd_acl_query_data {
	time_t timestamp;
	u8 radius_id;
	macaddr addr;
	u8 *auth_msg; /* IEEE 802.11 authentication frame from station */
	size_t auth_msg_len;
	struct asd_acl_query_data *next;
};


static void asd_acl_cache_free(struct asd_cached_radius_acl *acl_cache)
{
	struct asd_cached_radius_acl *prev;

	while (acl_cache) {
		prev = acl_cache;
		acl_cache = acl_cache->next;
		os_free(prev);
	}
}

#if 0
static int asd_acl_cache_get(struct asd_data *wasd, const u8 *addr,
				 u32 *session_timeout,
				 u32 *acct_interim_interval, int *vlan_id)
{
	struct asd_cached_radius_acl *entry;
	time_t now;

	time(&now);
	entry = wasd->acl_cache;

	while (entry) {
		if (os_memcmp(entry->addr, addr, ETH_ALEN) == 0) {
			if (now - entry->timestamp > RADIUS_ACL_TIMEOUT)
				return -1; /* entry has expired */
			if (entry->accepted == asd_ACL_ACCEPT_TIMEOUT)
				*session_timeout = entry->session_timeout;
			*acct_interim_interval = entry->acct_interim_interval;
			if (vlan_id)
				*vlan_id = entry->vlan_id;
			return entry->accepted;
		}

		entry = entry->next;
	}

	return -1;
}
#endif

static void asd_acl_query_free(struct asd_acl_query_data *query)
{
	if (query == NULL)
		return;
	os_free(query->auth_msg);
	os_free(query);
}

#if 0
static int asd_radius_acl_query(struct asd_data *wasd, const u8 *addr,
				    struct asd_acl_query_data *query)
{
	struct radius_msg *msg;
	char buf[128];
	unsigned char SID = wasd->SecurityID;
	query->radius_id = radius_client_get_id(wasd->radius);
	msg = radius_msg_new(RADIUS_CODE_ACCESS_REQUEST, query->radius_id);
	if (msg == NULL)
		return -1;

	radius_msg_make_authenticator(msg, addr, ETH_ALEN);

	os_snprintf(buf, sizeof(buf), RADIUS_ADDR_FORMAT, MAC2STR(addr));
	if (!radius_msg_add_attr(msg, RADIUS_ATTR_USER_NAME, (u8 *) buf,
				 os_strlen(buf))) {
		asd_printf(ASD_DEFAULT,MSG_DEBUG, "Could not add User-Name");
		goto fail;
	}

	if (!radius_msg_add_attr_user_password(
		    msg, (u8 *) buf, os_strlen(buf),
		    wasd->conf->radius->auth_server->shared_secret,
		    wasd->conf->radius->auth_server->shared_secret_len)) {
		asd_printf(ASD_DEFAULT,MSG_DEBUG, "Could not add User-Password");
		goto fail;
	}

	if (wasd->conf->own_ip_addr.af == AF_INET &&
	    !radius_msg_add_attr(msg, RADIUS_ATTR_NAS_IP_ADDRESS,
				 (u8 *) &wasd->conf->own_ip_addr.u.v4, 4)) {
		asd_printf(ASD_DEFAULT,MSG_DEBUG, "Could not add NAS-IP-Address");
		goto fail;
	}

#ifdef ASD_IPV6
	if (wasd->conf->own_ip_addr.af == AF_INET6 &&
	    !radius_msg_add_attr(msg, RADIUS_ATTR_NAS_IPV6_ADDRESS,
				 (u8 *) &wasd->conf->own_ip_addr.u.v6, 16)) {
		asd_printf(ASD_DEFAULT,MSG_DEBUG, "Could not add NAS-IPv6-Address");
		goto fail;
	}
#endif /* ASD_IPV6 */

	if (wasd->conf->nas_identifier &&
	    !radius_msg_add_attr(msg, RADIUS_ATTR_NAS_IDENTIFIER,
				 (u8 *) wasd->conf->nas_identifier,
				 os_strlen(wasd->conf->nas_identifier))) {
		asd_printf(ASD_DEFAULT,MSG_DEBUG, "Could not add NAS-Identifier");
		goto fail;
	}

	os_snprintf(buf, sizeof(buf), RADIUS_802_1X_ADDR_FORMAT ":%s",
		    MAC2STR(wasd->own_addr), wasd->conf->ssid.ssid);
	if (!radius_msg_add_attr(msg, RADIUS_ATTR_CALLED_STATION_ID,
				 (u8 *) buf, os_strlen(buf))) {
		asd_printf(ASD_DEFAULT,MSG_DEBUG, "Could not add Called-Station-Id");
		goto fail;
	}

	os_snprintf(buf, sizeof(buf), RADIUS_802_1X_ADDR_FORMAT,
		    MAC2STR(addr));
	if (!radius_msg_add_attr(msg, RADIUS_ATTR_CALLING_STATION_ID,
				 (u8 *) buf, os_strlen(buf))) {
		asd_printf(ASD_DEFAULT,MSG_DEBUG, "Could not add Calling-Station-Id");
		goto fail;
	}
	if((ASD_SECURITY[SID] != NULL)&&(ASD_SECURITY[SID]->wired_radius == 1)){
		if (!radius_msg_add_attr_int32(msg, RADIUS_ATTR_NAS_PORT_TYPE,
						   RADIUS_NAS_PORT_TYPE_IEEE_802_3)) {
			asd_printf(ASD_DEFAULT,MSG_DEBUG, "Could not add NAS-Port-Type");
			goto fail;
		}
	}else{	
		if (!radius_msg_add_attr_int32(msg, RADIUS_ATTR_NAS_PORT_TYPE,
					       RADIUS_NAS_PORT_TYPE_IEEE_802_11)) {
			asd_printf(ASD_DEFAULT,MSG_DEBUG, "Could not add NAS-Port-Type");
			goto fail;
		}
	}

	os_snprintf(buf, sizeof(buf), "CONNECT 11Mbps 802.11b");
	if (!radius_msg_add_attr(msg, RADIUS_ATTR_CONNECT_INFO,
				 (u8 *) buf, os_strlen(buf))) {
		asd_printf(ASD_DEFAULT,MSG_DEBUG, "Could not add Connect-Info");
		goto fail;
	}
	if(is_secondary != 1)
	radius_client_send(wasd->radius, msg, RADIUS_AUTH, addr);
	return 0;

 fail:
	radius_msg_free(msg);
	os_free(msg);
	return -1;
}
#endif

int asd_allowed_address(struct asd_data *wasd, const u8 *addr,
			    const u8 *msg, size_t len, u32 *session_timeout,
			    u32 *acct_interim_interval, int *vlan_id)
{
	unsigned char wlanid = wasd->WlanID;
	unsigned int wtpid = wasd->Radio_G_ID/4;
	unsigned int bssid = wasd->BSSIndex;
	
	*session_timeout = 0;
	*acct_interim_interval = 0;
	if (vlan_id)
		*vlan_id = 0;

	asd_printf(ASD_DEFAULT,MSG_DEBUG,"%s begin\n",__func__);

	/*for ac black list chech 20100830 by nl*/	
	if (ac_acl_conf.macaddr_acl == ACCEPT_UNLESS_DENIED){
			if (asd_maclist_found(ac_acl_conf.deny_mac, addr)){
				asd_printf(ASD_80211,MSG_DEBUG,"ac black list deny \n");
				return asd_ACL_REJECT;
			}
			else{
				//asd_printf(ASD_80211,MSG_DEBUG,"ac black list accept \n");
				return asd_ACL_ACCEPT;
			}
	}else if (ac_acl_conf.macaddr_acl == DENY_UNLESS_ACCEPTED){
		if (asd_maclist_found(ac_acl_conf.accept_mac, addr)){
			asd_printf(ASD_80211,MSG_DEBUG,"ac white list deny \n");
			return asd_ACL_ACCEPT;
		}else{
			asd_printf(ASD_80211,MSG_DEBUG,"ac white list accept \n");
			return asd_ACL_REJECT;
		}
	}

	if ((ASD_BSS[bssid]==NULL) || (ASD_WTP_AP[wtpid]==NULL) || (ASD_WLAN[wlanid]==NULL))
		return asd_ACL_REJECT;

	if (ASD_BSS[bssid]->acl_conf->macaddr_acl == ACCEPT_UNLESS_DENIED){
		if (asd_maclist_found(ASD_BSS[bssid]->acl_conf->deny_mac, addr))
			return asd_ACL_REJECT;
		else
			return asd_ACL_ACCEPT;
	}else if(ASD_BSS[bssid]->acl_conf->macaddr_acl == DENY_UNLESS_ACCEPTED){
		if (asd_maclist_found(ASD_BSS[bssid]->acl_conf->accept_mac, addr))
			  return asd_ACL_ACCEPT;
		else 
			return asd_ACL_REJECT;
	}else {
		if (ASD_WTP_AP[wtpid]->acl_conf->macaddr_acl == ACCEPT_UNLESS_DENIED){
			if (asd_maclist_found(ASD_WTP_AP[wtpid]->acl_conf->deny_mac, addr))
				return asd_ACL_REJECT;
			else
				return asd_ACL_ACCEPT;
		}else if(ASD_WTP_AP[wtpid]->acl_conf->macaddr_acl == DENY_UNLESS_ACCEPTED){
			if (asd_maclist_found(ASD_WTP_AP[wtpid]->acl_conf->accept_mac, addr))
			  	return asd_ACL_ACCEPT;
			else 
				return asd_ACL_REJECT;
		}else{
			if (ASD_WLAN[wlanid]->acl_conf->macaddr_acl == ACCEPT_UNLESS_DENIED){
				if (asd_maclist_found(ASD_WLAN[wlanid]->acl_conf->deny_mac, addr))
					return asd_ACL_REJECT;
				else
					return asd_ACL_ACCEPT;
			}else if(ASD_WLAN[wlanid]->acl_conf->macaddr_acl == DENY_UNLESS_ACCEPTED){
				if (asd_maclist_found(ASD_WLAN[wlanid]->acl_conf->accept_mac, addr))
					  return asd_ACL_ACCEPT;
				else 
					return asd_ACL_REJECT;
			}else
				return asd_ACL_ACCEPT;
		}
	}
	return asd_ACL_REJECT;
	
}	
/*
	//=======================================================================
	asd_printf(ASD_DEFAULT,MSG_DEBUG,"\n\n\n\n++++++++++++++++++++++++++++++++++++++++++\n");
	asd_printf(ASD_DEFAULT,MSG_DEBUG,"accept_mac_list:%d\n",wasd->conf->num_accept_mac);
	asd_printf(ASD_DEFAULT,MSG_DEBUG,"deny_mac_list:%d\n",wasd->conf->num_deny_mac);
	{
		int i=0;
		asd_printf(ASD_DEFAULT,MSG_DEBUG,"accept_mac_list:\n");
		for(i=0;i<wasd->conf->num_accept_mac;i++)
			asd_printf(ASD_DEFAULT,MSG_DEBUG,"%2d:\t"MACSTR"\n",i+1,MAC2STR((u8*)wasd->conf->accept_mac[i]));

		asd_printf(ASD_DEFAULT,MSG_DEBUG,"deny_mac_list:\n");
		for(i=0;i<wasd->conf->num_deny_mac;i++)
			asd_printf(ASD_DEFAULT,MSG_DEBUG,"%2d:\t"MACSTR"\n",i+1,MAC2STR((u8*)wasd->conf->deny_mac[i]));

		asd_printf(ASD_DEFAULT,MSG_DEBUG,"++++++++++++++++++++++++++++++++++++++++++\n\n\n\n");
	}
	//=======================================================================
	if (asd_maclist_found(wasd->conf->accept_mac,
				  wasd->conf->num_accept_mac, addr))
		return asd_ACL_ACCEPT;

	if (asd_maclist_found(wasd->conf->deny_mac,
				  wasd->conf->num_deny_mac, addr))
		return asd_ACL_REJECT;

	if (wasd->conf->macaddr_acl == ACCEPT_UNLESS_DENIED)
		return asd_ACL_ACCEPT;
	if (wasd->conf->macaddr_acl == DENY_UNLESS_ACCEPTED)
		return asd_ACL_REJECT;

	if (wasd->conf->macaddr_acl == USE_EXTERNAL_RADIUS_AUTH) {
		struct asd_acl_query_data *query;

		int res = asd_acl_cache_get(wasd, addr, session_timeout,
						acct_interim_interval,
						vlan_id);
		if (res == asd_ACL_ACCEPT ||
		    res == asd_ACL_ACCEPT_TIMEOUT)
			return res;
		if (res == asd_ACL_REJECT)
			return asd_ACL_REJECT;

		query = wasd->acl_queries;
		while (query) {
			if (os_memcmp(query->addr, addr, ETH_ALEN) == 0) {
				return asd_ACL_PENDING;
			}
			query = query->next;
		}

		if (!wasd->conf->radius->auth_server)
			return asd_ACL_REJECT;

		query = os_zalloc(sizeof(*query));
		if (query == NULL) {
			asd_printf(ASD_DEFAULT,MSG_ERROR, "malloc for query data failed");
			return asd_ACL_REJECT;
		}
		time(&query->timestamp);
		os_memcpy(query->addr, addr, ETH_ALEN);
		if (asd_radius_acl_query(wasd, addr, query)) {
			asd_printf(ASD_DEFAULT,MSG_DEBUG, "Failed to send Access-Request "
				   "for ACL query.");
			asd_acl_query_free(query);
			return asd_ACL_REJECT;
		}

		query->auth_msg = os_malloc(len);
		if (query->auth_msg == NULL) {
			asd_printf(ASD_DEFAULT,MSG_ERROR, "Failed to allocate memory for "
				   "auth frame.");
			asd_acl_query_free(query);
			return asd_ACL_REJECT;
		}
		os_memcpy(query->auth_msg, msg, len);
		query->auth_msg_len = len;
		query->next = wasd->acl_queries;
		wasd->acl_queries = query;

		return asd_ACL_PENDING;
	}

	return asd_ACL_REJECT;
*/	

static void asd_acl_expire_cache(struct asd_data *wasd, time_t now)
{
	struct asd_cached_radius_acl *prev, *entry, *tmp;

	prev = NULL;
	entry = wasd->acl_cache;

	while (entry) {
		if (now - entry->timestamp > RADIUS_ACL_TIMEOUT) {
			asd_printf(ASD_DEFAULT,MSG_DEBUG, "Cached ACL entry for " MACSTR
				   " has expired.", MAC2STR(entry->addr));
			if (prev)
				prev->next = entry->next;
			else
				wasd->acl_cache = entry->next;

			tmp = entry;
			entry = entry->next;
			os_free(tmp);
			continue;
		}

		prev = entry;
		entry = entry->next;
	}
}


static void asd_acl_expire_queries(struct asd_data *wasd, time_t now)
{
	struct asd_acl_query_data *prev, *entry, *tmp;

	prev = NULL;
	entry = wasd->acl_queries;

	while (entry) {
		if (now - entry->timestamp > RADIUS_ACL_TIMEOUT) {
			asd_printf(ASD_DEFAULT,MSG_DEBUG, "ACL query for " MACSTR
				   " has expired.", MAC2STR(entry->addr));
			if (prev)
				prev->next = entry->next;
			else
				wasd->acl_queries = entry->next;

			tmp = entry;
			entry = entry->next;
			asd_acl_query_free(tmp);
			continue;
		}

		prev = entry;
		entry = entry->next;
	}
}


static void asd_acl_expire(void *circle_ctx, void *timeout_ctx)
{
	struct asd_data *wasd = circle_ctx;
	time_t now;

	time(&now);
	asd_acl_expire_cache(wasd, now);
	asd_acl_expire_queries(wasd, now);

	circle_register_timeout(10, 0, asd_acl_expire, wasd, NULL);
}


/* Return 0 if RADIUS message was a reply to ACL query (and was processed here)
 * or -1 if not. */
static RadiusRxResult
asd_acl_recv_radius(struct radius_msg *msg, struct radius_msg *req,
			u8 *shared_secret, size_t shared_secret_len,
			void *data)
{
	struct asd_data *wasd = data;
	struct asd_acl_query_data *query, *prev;
	struct asd_cached_radius_acl *cache;

	query = wasd->acl_queries;
	prev = NULL;
	while (query) {
		if (query->radius_id == msg->hdr->identifier)
			break;
		prev = query;
		query = query->next;
	}
	if (query == NULL)
		return RADIUS_RX_UNKNOWN;

	asd_printf(ASD_DEFAULT,MSG_DEBUG, "Found matching Access-Request for RADIUS "
		   "message (id=%d)", query->radius_id);

	if (radius_msg_verify(msg, shared_secret, shared_secret_len, req, 0)) {
		asd_printf(ASD_DEFAULT,MSG_INFO, "Incoming RADIUS packet did not have "
			   "correct authenticator - dropped\n");
		return RADIUS_RX_INVALID_AUTHENTICATOR;
	}

	if (msg->hdr->code != RADIUS_CODE_ACCESS_ACCEPT &&
	    msg->hdr->code != RADIUS_CODE_ACCESS_REJECT) {
		asd_printf(ASD_DEFAULT,MSG_DEBUG, "Unknown RADIUS message code %d to ACL "
			   "query", msg->hdr->code);
		return RADIUS_RX_UNKNOWN;
	}

	/* Insert Accept/Reject info into ACL cache */
	cache = os_zalloc(sizeof(*cache));
	if (cache == NULL) {
		asd_printf(ASD_DEFAULT,MSG_DEBUG, "Failed to add ACL cache entry");
		goto done;
	}
	time(&cache->timestamp);
	os_memcpy(cache->addr, query->addr, sizeof(cache->addr));
	if (msg->hdr->code == RADIUS_CODE_ACCESS_ACCEPT) {
		if (radius_msg_get_attr_int32(msg, RADIUS_ATTR_SESSION_TIMEOUT,
					      &cache->session_timeout) == 0)
			cache->accepted = asd_ACL_ACCEPT_TIMEOUT;
		else
			cache->accepted = asd_ACL_ACCEPT;

		if (radius_msg_get_attr_int32(
			    msg, RADIUS_ATTR_ACCT_INTERIM_INTERVAL,
			    &cache->acct_interim_interval) == 0 &&
		    cache->acct_interim_interval < 60) {
			asd_printf(ASD_DEFAULT,MSG_DEBUG, "Ignored too small "
				   "Acct-Interim-Interval %d for STA " MACSTR,
				   cache->acct_interim_interval,
				   MAC2STR(query->addr));
			cache->acct_interim_interval = 0;
		}

		cache->vlan_id = radius_msg_get_vlanid(msg);
	} else
		cache->accepted = asd_ACL_REJECT;
	cache->next = wasd->acl_cache;
	wasd->acl_cache = cache;

	/* Re-send original authentication frame for 802.11 processing */
	asd_printf(ASD_DEFAULT,MSG_DEBUG, "Re-sending authentication frame after "
		   "successful RADIUS ACL query");
	ieee802_11_mgmt(wasd, query->auth_msg, query->auth_msg_len,
			WLAN_FC_STYPE_AUTH, NULL);

 done:
	if (prev == NULL)
		wasd->acl_queries = query->next;
	else
		prev->next = query->next;

	asd_acl_query_free(query);

	return RADIUS_RX_PROCESSED;
}


int asd_acl_init(struct asd_data *wasd)
{
	if (radius_client_register(wasd->radius->auth,
				   asd_acl_recv_radius, wasd))
		return -1;

	circle_register_timeout(10, 0, asd_acl_expire, wasd, NULL);

	return 0;
}


void asd_acl_deinit(struct asd_data *wasd)
{
	struct asd_acl_query_data *query, *prev;

	circle_cancel_timeout(asd_acl_expire, wasd, NULL);

	asd_acl_cache_free(wasd->acl_cache);

	query = wasd->acl_queries;
	while (query) {
		prev = query;
		query = query->next;
		asd_acl_query_free(prev);
	}
}


int asd_acl_reconfig(struct asd_data *wasd,
			 struct asd_config *oldconf)
{
	if (!wasd->radius_client_reconfigured)
		return 0;

	asd_acl_deinit(wasd);
	return asd_acl_init(wasd);
}

