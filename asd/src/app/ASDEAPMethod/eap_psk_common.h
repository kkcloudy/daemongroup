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
* eap_psk_common.h
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

#ifndef EAP_PSK_COMMON_H
#define EAP_PSK_COMMON_H


#define EAP_PSK_RAND_LEN 16
#define EAP_PSK_MAC_LEN 16
#define EAP_PSK_TEK_LEN 16
#define EAP_PSK_PSK_LEN 16
#define EAP_PSK_AK_LEN 16
#define EAP_PSK_KDK_LEN 16

#define EAP_PSK_R_FLAG_CONT 1
#define EAP_PSK_R_FLAG_DONE_SUCCESS 2
#define EAP_PSK_R_FLAG_DONE_FAILURE 3
#define EAP_PSK_E_FLAG 0x20

#define EAP_PSK_FLAGS_GET_T(flags) (((flags) & 0xc0) >> 6)
#define EAP_PSK_FLAGS_SET_T(t) ((u8) (t) << 6)

#ifdef _MSC_VER
#pragma pack(push, 1)
#endif /* _MSC_VER */

/* EAP-PSK First Message (AS -> Supplicant) */
struct eap_psk_hdr_1 {
	u8 flags;
	u8 rand_s[EAP_PSK_RAND_LEN];
	/* Followed by variable length ID_S */
} STRUCT_PACKED;

/* EAP-PSK Second Message (Supplicant -> AS) */
struct eap_psk_hdr_2 {
	u8 flags;
	u8 rand_s[EAP_PSK_RAND_LEN];
	u8 rand_p[EAP_PSK_RAND_LEN];
	u8 mac_p[EAP_PSK_MAC_LEN];
	/* Followed by variable length ID_P */
} STRUCT_PACKED;

/* EAP-PSK Third Message (AS -> Supplicant) */
struct eap_psk_hdr_3 {
	u8 flags;
	u8 rand_s[EAP_PSK_RAND_LEN];
	u8 mac_s[EAP_PSK_MAC_LEN];
	/* Followed by variable length PCHANNEL */
} STRUCT_PACKED;

/* EAP-PSK Fourth Message (Supplicant -> AS) */
struct eap_psk_hdr_4 {
	u8 flags;
	u8 rand_s[EAP_PSK_RAND_LEN];
	/* Followed by variable length PCHANNEL */
} STRUCT_PACKED;

#ifdef _MSC_VER
#pragma pack(pop)
#endif /* _MSC_VER */


int __must_check eap_psk_key_setup(const u8 *psk, u8 *ak, u8 *kdk);
int __must_check eap_psk_derive_keys(const u8 *kdk, const u8 *rand_p, u8 *tek,
				     u8 *msk, u8 *emsk);

#endif /* EAP_PSK_COMMON_H */
