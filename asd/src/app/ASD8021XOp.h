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
* ASD8021XOp.h
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

#ifndef IEEE802_1X_H
#define IEEE802_1X_H

struct asd_data;
struct sta_info;
struct eapol_state_machine;
struct asd_config;
struct asd_bss_config;

/* RFC 3580, 4. RC4 EAPOL-Key Frame */

struct ieee802_1x_eapol_key {
	u8 type;
	u16 key_length;
	u8 replay_counter[8]; /* does not repeat within the life of the keying
			       * material used to encrypt the Key field;
			       * 64-bit NTP timestamp MAY be used here */
	u8 key_iv[16]; /* cryptographically random number */
	u8 key_index; /* key flag in the most significant bit:
		       * 0 = broadcast (default key),
		       * 1 = unicast (key mapping key); key index is in the
		       * 7 least significant bits */
	u8 key_signature[16]; /* HMAC-MD5 message integrity check computed with
			       * MS-MPPE-Send-Key as the key */

	/* followed by key: if packet body length = 44 + key length, then the
	 * key field (of key_length bytes) contains the key in encrypted form;
	 * if packet body length = 44, key field is absent and key_length
	 * represents the number of least significant octets from
	 * MS-MPPE-Send-Key attribute to be used as the keying material;
	 * RC4 key used in encryption = Key-IV + MS-MPPE-Recv-Key */
} __attribute__ ((packed));


void ieee802_1x_receive(struct asd_data *wasd, const u8 *sa, const u8 *buf,
			size_t len);
void ieee802_1x_new_station(struct asd_data *wasd, struct sta_info *sta);
void ieee802_1x_free_station(struct sta_info *sta);

void ieee802_1x_tx_key(struct asd_data *wasd, struct sta_info *sta);
void ieee802_1x_abort_auth(struct asd_data *wasd, struct sta_info *sta);
void ieee802_1x_set_sta_authorized(struct asd_data *wasd,
				   struct sta_info *sta, int authorized);
void ieee802_1x_dump_state(FILE *f, const char *prefix, struct sta_info *sta);
int ieee802_1x_init(struct asd_data *wasd);
void ieee802_1x_deinit(struct asd_data *wasd);
int ieee802_1x_reconfig(struct asd_data *wasd,
			struct asd_config *oldconf,
			struct asd_bss_config *oldbss);
int ieee802_1x_tx_status(struct asd_data *wasd, struct sta_info *sta,
			 u8 *buf, size_t len, int ack);
u8 * ieee802_1x_get_identity(struct eapol_state_machine *sm, size_t *len);
u8 * ieee802_1x_get_radius_class(struct eapol_state_machine *sm, size_t *len,
				 int idx);
const u8 * ieee802_1x_get_key(struct eapol_state_machine *sm, size_t *len);
void ieee802_1x_notify_port_enabled(struct eapol_state_machine *sm,
				    int enabled);
void ieee802_1x_notify_port_valid(struct eapol_state_machine *sm,
				  int valid);
void ieee802_1x_notify_pre_auth(struct eapol_state_machine *sm, int pre_auth);
int ieee802_1x_get_mib(struct asd_data *wasd, char *buf, size_t buflen);
int ieee802_1x_get_mib_sta(struct asd_data *wasd, struct sta_info *sta,
			   char *buf, size_t buflen);
void asd_get_ntp_timestamp(u8 *buf);
char *eap_type_text(u8 type);

struct radius_class_data;
struct radius_coa_info;
void ieee802_1x_free_radius_class(struct radius_class_data *class);
int ieee802_1x_copy_radius_class(struct radius_class_data *dst,
				 struct radius_class_data *src);
//mahz 2010.11.30
void wapi_radius_auth_send(void *ctx, void *sta_ctx);

//mahz add 2011.3.7
void sta_ip_addr_check(void *ctx, void *sta_ctx);
void eapol_keep_alive_timer(struct eapol_state_machine *sm,int *alive_time);
int asd_wlan_radius_init(unsigned int wlanid);
int send_dsgd_finish(int identity_id);
void  asd_wlan_radius_free(unsigned int wlanid);
//qiuchen
void radius_encapsulate_heart_test_radius(void *circle_ctx, void *timeout_ctx);
void RADIUS_PACKAGESEND_ACCT(struct heart_test_radius_data *wasd);
void RADIUS_PACKAGESEND_AUTH(struct heart_test_radius_data *wasd);
void ieee802_1x_encapsulate_dm_radius(struct radius_coa_info *coa_info);
void asd_radius_coa_request(struct radius_msg *msg,unsigned int radius_ip,int radius_port);

void eap_sm_run_cancel_timer(unsigned char securityid);

#endif /* IEEE802_1X_H */
