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
* radius_server.h
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
#ifndef RADIUS_SERVER_H
#define RADIUS_SERVER_H

struct radius_server_data;
struct eap_user;

struct radius_server_conf {
	int auth_port;
	char *client_file;
	void *conf_ctx;
	void *eap_sim_db_priv;
	void *ssl_ctx;
	u8 *pac_opaque_encr_key;
	char *eap_fast_a_id;
	int eap_sim_aka_result_ind;
	int ipv6;
	int (*get_eap_user)(void *ctx, const u8 *identity, size_t identity_len,
			    int phase2, struct eap_user *user);
};


#ifdef RADIUS_SERVER

struct radius_server_data *
radius_server_init(struct radius_server_conf *conf);

void radius_server_deinit(struct radius_server_data *data);

int radius_server_get_mib(struct radius_server_data *data, char *buf,
			  size_t buflen);

void radius_server_eap_pending_cb(struct radius_server_data *data, void *ctx);

#else /* RADIUS_SERVER */

static inline struct radius_server_data *
radius_server_init(struct radius_server_conf *conf)
{
	return NULL;
}

static inline void radius_server_deinit(struct radius_server_data *data)
{
}

static inline int radius_server_get_mib(struct radius_server_data *data,
					char *buf, size_t buflen)
{
	return 0;
}

static inline void
radius_server_eap_pending_cb(struct radius_server_data *data, void *ctx)
{
}

#endif /* RADIUS_SERVER */

#endif /* RADIUS_SERVER_H */
