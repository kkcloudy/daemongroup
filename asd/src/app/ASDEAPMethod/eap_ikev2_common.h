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
* eap_ikev2_common.h
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

#ifndef EAP_IKEV2_COMMON_H
#define EAP_IKEV2_COMMON_H

#ifdef CCNS_PL
/* incorrect bit order */
#define IKEV2_FLAGS_LENGTH_INCLUDED 0x01
#define IKEV2_FLAGS_MORE_FRAGMENTS 0x02
#define IKEV2_FLAGS_ICV_INCLUDED 0x04
#else /* CCNS_PL */
#define IKEV2_FLAGS_LENGTH_INCLUDED 0x80
#define IKEV2_FLAGS_MORE_FRAGMENTS 0x40
#define IKEV2_FLAGS_ICV_INCLUDED 0x20
#endif /* CCNS_PL */

#define IKEV2_FRAGMENT_SIZE 1400

struct ikev2_keys;

int eap_ikev2_derive_keymat(int prf, struct ikev2_keys *keys,
			    const u8 *i_nonce, size_t i_nonce_len,
			    const u8 *r_nonce, size_t r_nonce_len,
			    u8 *keymat);
struct wpabuf * eap_ikev2_build_frag_ack(u8 id, u8 code);
int eap_ikev2_validate_icv(int integ_alg, struct ikev2_keys *keys,
			   int initiator, const struct wpabuf *msg,
			   const u8 *pos, const u8 *end);

#endif /* EAP_IKEV2_COMMON_H */
