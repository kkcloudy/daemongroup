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
* Tlsv1_client_i.h
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

#ifndef TLSV1_CLIENT_I_H
#define TLSV1_CLIENT_I_H

struct tlsv1_client {
	enum {
		CLIENT_HELLO, SERVER_HELLO, SERVER_CERTIFICATE,
		SERVER_KEY_EXCHANGE, SERVER_CERTIFICATE_REQUEST,
		SERVER_HELLO_DONE, CLIENT_KEY_EXCHANGE, CHANGE_CIPHER_SPEC,
		SERVER_CHANGE_CIPHER_SPEC, SERVER_FINISHED, ACK_FINISHED,
		ESTABLISHED, FAILED
	} state;

	struct tlsv1_record_layer rl;

	u8 session_id[TLS_SESSION_ID_MAX_LEN];
	size_t session_id_len;
	u8 client_random[TLS_RANDOM_LEN];
	u8 server_random[TLS_RANDOM_LEN];
	u8 master_secret[TLS_MASTER_SECRET_LEN];

	u8 alert_level;
	u8 alert_description;

	unsigned int certificate_requested:1;
	unsigned int session_resumed:1;
	unsigned int session_ticket_included:1;
	unsigned int use_session_ticket:1;

	struct crypto_public_key *server_rsa_key;

	struct tls_verify_hash verify;

#define MAX_CIPHER_COUNT 30
	u16 cipher_suites[MAX_CIPHER_COUNT];
	size_t num_cipher_suites;

	u16 prev_cipher_suite;

	u8 *client_hello_ext;
	size_t client_hello_ext_len;

	/* The prime modulus used for Diffie-Hellman */
	u8 *dh_p;
	size_t dh_p_len;
	/* The generator used for Diffie-Hellman */
	u8 *dh_g;
	size_t dh_g_len;
	/* The server's Diffie-Hellman public value */
	u8 *dh_ys;
	size_t dh_ys_len;

	struct tlsv1_credentials *cred;

	tlsv1_client_session_ticket_cb session_ticket_cb;
	void *session_ticket_cb_ctx;
};


void tls_alert(struct tlsv1_client *conn, u8 level, u8 description);
void tlsv1_client_free_dh(struct tlsv1_client *conn);
int tls_derive_pre_master_secret(u8 *pre_master_secret);
int tls_derive_keys(struct tlsv1_client *conn,
		    const u8 *pre_master_secret, size_t pre_master_secret_len);
u8 * tls_send_client_hello(struct tlsv1_client *conn, size_t *out_len);
u8 * tlsv1_client_send_alert(struct tlsv1_client *conn, u8 level,
			     u8 description, size_t *out_len);
u8 * tlsv1_client_handshake_write(struct tlsv1_client *conn, size_t *out_len,
				  int no_appl_data);
int tlsv1_client_process_handshake(struct tlsv1_client *conn, u8 ct,
				   const u8 *buf, size_t *len,
				   u8 **out_data, size_t *out_len);

#endif /* TLSV1_CLIENT_I_H */
