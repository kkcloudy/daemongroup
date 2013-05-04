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
* ms_funcs.h
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

#ifndef MS_FUNCS_H
#define MS_FUNCS_H

void generate_nt_response(const u8 *auth_challenge, const u8 *peer_challenge,
			  const u8 *username, size_t username_len,
			  const u8 *password, size_t password_len,
			  u8 *response);
void generate_nt_response_pwhash(const u8 *auth_challenge,
				 const u8 *peer_challenge,
				 const u8 *username, size_t username_len,
				 const u8 *password_hash,
				 u8 *response);
void generate_authenticator_response(const u8 *password, size_t password_len,
				     const u8 *peer_challenge,
				     const u8 *auth_challenge,
				     const u8 *username, size_t username_len,
				     const u8 *nt_response, u8 *response);
void generate_authenticator_response_pwhash(
	const u8 *password_hash,
	const u8 *peer_challenge, const u8 *auth_challenge,
	const u8 *username, size_t username_len,
	const u8 *nt_response, u8 *response);
void nt_challenge_response(const u8 *challenge, const u8 *password,
			   size_t password_len, u8 *response);

void challenge_response(const u8 *challenge, const u8 *password_hash,
			u8 *response);
void nt_password_hash(const u8 *password, size_t password_len,
		      u8 *password_hash);
void hash_nt_password_hash(const u8 *password_hash, u8 *password_hash_hash);
void get_master_key(const u8 *password_hash_hash, const u8 *nt_response,
		    u8 *master_key);
void get_asymetric_start_key(const u8 *master_key, u8 *session_key,
			     size_t session_key_len, int is_send,
			     int is_server);
int __must_check encrypt_pw_block_with_password_hash(
	const u8 *password, size_t password_len,
	const u8 *password_hash, u8 *pw_block);
int __must_check new_password_encrypted_with_old_nt_password_hash(
	const u8 *new_password, size_t new_password_len,
	const u8 *old_password, size_t old_password_len,
	u8 *encrypted_pw_block);
void nt_password_hash_encrypted_with_block(const u8 *password_hash,
					   const u8 *block, u8 *cypher);
void old_nt_password_hash_encrypted_with_new_nt_password_hash(
	const u8 *new_password, size_t new_password_len,
	const u8 *old_password, size_t old_password_len,
	u8 *encrypted_password_hash);

#endif /* MS_FUNCS_H */
