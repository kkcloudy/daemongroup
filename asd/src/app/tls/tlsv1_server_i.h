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
* Tlsv1_server_i.h
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

#ifndef TLSV1_SERVER_I_H
#define TLSV1_SERVER_I_H

struct tlsv1_server {
	enum {
		CLIENT_HELLO, SERVER_HELLO, SERVER_CERTIFICATE,
		SERVER_KEY_EXCHANGE, SERVER_CERTIFICATE_REQUEST,
		SERVER_HELLO_DONE, CLIENT_CERTIFICATE, CLIENT_KEY_EXCHANGE,
		CERTIFICATE_VERIFY, CHANGE_CIPHER_SPEC, CLIENT_FINISHED,
		SERVER_CHANGE_CIPHER_SPEC, SERVER_FINISHED,
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

	struct crypto_public_key *client_rsa_key;

	struct tls_verify_hash verify;

#define MAX_CIPHER_COUNT 30
	u16 cipher_suites[MAX_CIPHER_COUNT];
	size_t num_cipher_suites;

	u16 cipher_suite;

	struct tlsv1_credentials *cred;

	int verify_peer;
	u16 client_version;

	u8 *session_ticket;
	size_t session_ticket_len;

	tlsv1_server_session_ticket_cb session_ticket_cb;
	void *session_ticket_cb_ctx;

	int use_session_ticket;

	u8 *dh_secret;
	size_t dh_secret_len;
};


void tlsv1_server_alert(struct tlsv1_server *conn, u8 level, u8 description);
int tlsv1_server_derive_keys(struct tlsv1_server *conn,
			     const u8 *pre_master_secret,
			     size_t pre_master_secret_len);
u8 * tlsv1_server_handshake_write(struct tlsv1_server *conn, size_t *out_len);
u8 * tlsv1_server_send_alert(struct tlsv1_server *conn, u8 level,
			     u8 description, size_t *out_len);
int tlsv1_server_process_handshake(struct tlsv1_server *conn, u8 ct,
				   const u8 *buf, size_t *len);

#endif /* TLSV1_SERVER_I_H */
