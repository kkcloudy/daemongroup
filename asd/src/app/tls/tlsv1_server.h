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
* Tlsv1_server.h
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

#ifndef TLSV1_SERVER_H
#define TLSV1_SERVER_H

#include "tlsv1_cred.h"

struct tlsv1_server;

int tlsv1_server_global_init(void);
void tlsv1_server_global_deinit(void);
struct tlsv1_server * tlsv1_server_init(struct tlsv1_credentials *cred);
void tlsv1_server_deinit(struct tlsv1_server *conn);
int tlsv1_server_established(struct tlsv1_server *conn);
int tlsv1_server_prf(struct tlsv1_server *conn, const char *label,
		     int server_random_first, u8 *out, size_t out_len);
u8 * tlsv1_server_handshake(struct tlsv1_server *conn,
			    const u8 *in_data, size_t in_len, size_t *out_len);
int tlsv1_server_encrypt(struct tlsv1_server *conn,
			 const u8 *in_data, size_t in_len,
			 u8 *out_data, size_t out_len);
int tlsv1_server_decrypt(struct tlsv1_server *conn,
			 const u8 *in_data, size_t in_len,
			 u8 *out_data, size_t out_len);
int tlsv1_server_get_cipher(struct tlsv1_server *conn, char *buf,
			    size_t buflen);
int tlsv1_server_shutdown(struct tlsv1_server *conn);
int tlsv1_server_resumed(struct tlsv1_server *conn);
int tlsv1_server_get_keys(struct tlsv1_server *conn, struct tls_keys *keys);
int tlsv1_server_get_keyblock_size(struct tlsv1_server *conn);
int tlsv1_server_set_cipher_list(struct tlsv1_server *conn, u8 *ciphers);
int tlsv1_server_set_verify(struct tlsv1_server *conn, int verify_peer);

typedef int (*tlsv1_server_session_ticket_cb)
(void *ctx, const u8 *ticket, size_t len, const u8 *client_random,
 const u8 *server_random, u8 *master_secret);

void tlsv1_server_set_session_ticket_cb(struct tlsv1_server *conn,
					tlsv1_server_session_ticket_cb cb,
					void *ctx);

#endif /* TLSV1_SERVER_H */
