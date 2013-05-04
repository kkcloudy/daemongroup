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
* Ikev2.h
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

#ifndef IKEV2_H
#define IKEV2_H

#include "ASDEAPMethod/ikev2_common.h"

struct ikev2_proposal_data {
	u8 proposal_num;
	int integ;
	int prf;
	int encr;
	int dh;
};


struct ikev2_initiator_data {
	enum { SA_INIT, SA_AUTH, CHILD_SA, IKEV2_DONE } state;
	u8 i_spi[IKEV2_SPI_LEN];
	u8 r_spi[IKEV2_SPI_LEN];
	u8 i_nonce[IKEV2_NONCE_MAX_LEN];
	size_t i_nonce_len;
	u8 r_nonce[IKEV2_NONCE_MAX_LEN];
	size_t r_nonce_len;
	struct wpabuf *r_dh_public;
	struct wpabuf *i_dh_private;
	struct ikev2_proposal_data proposal;
	const struct dh_group *dh;
	struct ikev2_keys keys;
	u8 *IDi;
	size_t IDi_len;
	u8 *IDr;
	size_t IDr_len;
	u8 IDr_type;
	struct wpabuf *r_sign_msg;
	struct wpabuf *i_sign_msg;
	u8 *shared_secret;
	size_t shared_secret_len;
	enum { PEER_AUTH_CERT, PEER_AUTH_SECRET } peer_auth;
	u8 *key_pad;
	size_t key_pad_len;

	const u8 * (*get_shared_secret)(void *ctx, const u8 *IDr,
					size_t IDr_len, size_t *secret_len);
	void *cb_ctx;
	int unknown_user;
};


void ikev2_initiator_deinit(struct ikev2_initiator_data *data);
int ikev2_initiator_process(struct ikev2_initiator_data *data,
			    const struct wpabuf *buf);
struct wpabuf * ikev2_initiator_build(struct ikev2_initiator_data *data);

#endif /* IKEV2_H */
