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
* ASDPMKCache.h
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

#ifndef PMKSA_CACHE_H
#define PMKSA_CACHE_H

/**
 * struct rsn_pmksa_cache_entry - PMKSA cache entry
 */
struct rsn_pmksa_cache_entry {
	struct rsn_pmksa_cache_entry *next, *hnext;
	u8 pmkid[PMKID_LEN];
	u8 pmk[PMK_LEN];
	size_t pmk_len;
	os_time_t expiration;
	int akmp; /* WPA_KEY_MGMT_* */
	u8 spa[ETH_ALEN];

	u8 *identity;
	size_t identity_len;
	struct radius_class_data radius_class;
	u8 eap_type_authsrv;
	int vlan_id;
	u8 *cui_identity;
	size_t cui_len;
};

struct rsn_pmksa_cache;

struct rsn_pmksa_cache *
pmksa_cache_init(void (*free_cb)(struct rsn_pmksa_cache_entry *entry,
				 void *ctx), void *ctx);
void pmksa_cache_deinit(struct rsn_pmksa_cache *pmksa);
struct rsn_pmksa_cache_entry * pmksa_cache_get(struct rsn_pmksa_cache *pmksa,
					       const u8 *spa, const u8 *pmkid);
struct rsn_pmksa_cache_entry *
pmksa_cache_add(struct rsn_pmksa_cache *pmksa, const u8 *pmk, size_t pmk_len,
		const u8 *aa, const u8 *spa, int session_timeout,
		struct eapol_state_machine *eapol);
void pmksa_cache_to_eapol_data(struct rsn_pmksa_cache_entry *entry,
			       struct eapol_state_machine *eapol);
void rsn_pmkid(const u8 *pmk, size_t pmk_len, const u8 *aa, const u8 *spa,
	       u8 *pmkid);


/*****************************************************************************
 *	pmksa_cache_delete_from_hash
 * 
 *	remove from pmkid hash
 *
 *  INPUT:
 *		pmksa - RSN pmksa cache (hash buctets)
 *		entry - RSN pmksa cache entry (hash node)
 *  
 *  OUTPUT:
 * 	 NULL
 *
 *  RETURN:
 * 	  0 - find entry and remove
 * 	 -1 - no such pmk entry
 *
 ****************************************************************************/
int pmksa_cache_delete_from_hash
(
	struct rsn_pmksa_cache *pmksa,
	struct rsn_pmksa_cache_entry *entry
);

/*****************************************************************************
 *	pmksa_cache_insert_to_hash
 * 
 *	insert entry to pmkid hash
 *
 *  INPUT:
 *		pmksa - RSN pmksa cache (hash buctets)
 *		entry - RSN pmksa cache entry (hash node)
 *  
 *  OUTPUT:
 * 	 NULL
 *
 *  RETURN:
 * 	  0 - success
 * 	 -1 - failed
 *
 ****************************************************************************/
int pmksa_cache_insert_to_hash
(
	struct rsn_pmksa_cache *pmksa,
	struct rsn_pmksa_cache_entry *entry
);

#endif /* PMKSA_CACHE_H */
