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
* AsdMlme.c
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

#include "includes.h"

#include "asd.h"
#include "ASD80211Op.h"
#include "ASDWPAOp.h"
#include "ASDMlme.h"


static const char * mlme_auth_alg_str(int alg)
{
	switch (alg) {
	case WLAN_AUTH_OPEN:
		return "OPEN_SYSTEM";
	case WLAN_AUTH_SHARED_KEY:
		return "SHARED_KEY";
	case WLAN_AUTH_FT:
		return "FT";
	}

	return "unknown";
}


/**
 * mlme_authenticate_indication - Report the establishment of an authentication
 * relationship with a specific peer MAC entity
 * @wasd: BSS data
 * @sta: peer STA data
 *
 * MLME calls this function as a result of the establishment of an
 * authentication relationship with a specific peer MAC entity that
 * resulted from an authentication procedure that was initiated by
 * that specific peer MAC entity.
 *
 * PeerSTAAddress = sta->addr
 * AuthenticationType = sta->auth_alg (WLAN_AUTH_OPEN / WLAN_AUTH_SHARED_KEY)
 */
void mlme_authenticate_indication(struct asd_data *wasd,
				  struct sta_info *sta)
{
	asd_logger(wasd, sta->addr, asd_MODULE_MLME,
		       asd_LEVEL_DEBUG,
		       "MLME-AUTHENTICATE.indication(" MACSTR ", %s)",
		       MAC2STR(sta->addr), mlme_auth_alg_str(sta->auth_alg));
	if (sta->auth_alg != WLAN_AUTH_FT)
		mlme_deletekeys_request(wasd, sta);
}


/**
 * mlme_deauthenticate_indication - Report the invalidation of an
 * authentication relationship with a specific peer MAC entity
 * @wasd: BSS data
 * @sta: Peer STA data
 * @reason_code: ReasonCode from Deauthentication frame
 *
 * MLME calls this function as a result of the invalidation of an
 * authentication relationship with a specific peer MAC entity.
 *
 * PeerSTAAddress = sta->addr
 */
void mlme_deauthenticate_indication(struct asd_data *wasd,
				    struct sta_info *sta, u16 reason_code)
{
	asd_logger(wasd, sta->addr, asd_MODULE_MLME,
		       asd_LEVEL_DEBUG,
		       "MLME-DEAUTHENTICATE.indication(" MACSTR ", %d)",
		       MAC2STR(sta->addr), reason_code);
	mlme_deletekeys_request(wasd, sta);
}


/**
 * mlme_associate_indication - Report the establishment of an association with
 * a specific peer MAC entity
 * @wasd: BSS data
 * @sta: peer STA data
 *
 * MLME calls this function as a result of the establishment of an
 * association with a specific peer MAC entity that resulted from an
 * association procedure that was initiated by that specific peer MAC entity.
 *
 * PeerSTAAddress = sta->addr
 */
void mlme_associate_indication(struct asd_data *wasd, struct sta_info *sta)
{
	asd_logger(wasd, sta->addr, asd_MODULE_MLME,
		       asd_LEVEL_DEBUG,
		       "MLME-ASSOCIATE.indication(" MACSTR ")",
		       MAC2STR(sta->addr));
	if (sta->auth_alg != WLAN_AUTH_FT)
		mlme_deletekeys_request(wasd, sta);
}


/**
 * mlme_reassociate_indication - Report the establishment of an reassociation
 * with a specific peer MAC entity
 * @wasd: BSS data
 * @sta: peer STA data
 *
 * MLME calls this function as a result of the establishment of an
 * reassociation with a specific peer MAC entity that resulted from a
 * reassociation procedure that was initiated by that specific peer MAC entity.
 *
 * PeerSTAAddress = sta->addr
 *
 * sta->previous_ap contains the "Current AP" information from ReassocReq.
 */
void mlme_reassociate_indication(struct asd_data *wasd,
				 struct sta_info *sta)
{
	asd_logger(wasd, sta->addr, asd_MODULE_MLME,
		       asd_LEVEL_DEBUG,
		       "MLME-REASSOCIATE.indication(" MACSTR ")",
		       MAC2STR(sta->addr));
	if (sta->auth_alg != WLAN_AUTH_FT)
		mlme_deletekeys_request(wasd, sta);
}


/**
 * mlme_disassociate_indication - Report disassociation with a specific peer
 * MAC entity
 * @wasd: BSS data
 * @sta: Peer STA data
 * @reason_code: ReasonCode from Disassociation frame
 *
 * MLME calls this function as a result of the invalidation of an association
 * relationship with a specific peer MAC entity.
 *
 * PeerSTAAddress = sta->addr
 */
void mlme_disassociate_indication(struct asd_data *wasd,
				  struct sta_info *sta, u16 reason_code)
{
	asd_logger(wasd, sta->addr, asd_MODULE_MLME,
		       asd_LEVEL_DEBUG,
		       "MLME-DISASSOCIATE.indication(" MACSTR ", %d)",
		       MAC2STR(sta->addr), reason_code);
	mlme_deletekeys_request(wasd, sta);
}


void mlme_michaelmicfailure_indication(struct asd_data *wasd,
				       const u8 *addr)
{
	asd_logger(wasd, addr, asd_MODULE_MLME,
		       asd_LEVEL_DEBUG,
		       "MLME-MichaelMICFailure.indication(" MACSTR ")",
		       MAC2STR(addr));
}


void mlme_deletekeys_request(struct asd_data *wasd, struct sta_info *sta)
{
	asd_logger(wasd, sta->addr, asd_MODULE_MLME,
		       asd_LEVEL_DEBUG,
		       "MLME-DELETEKEYS.request(" MACSTR ")",
		       MAC2STR(sta->addr));

	if (sta->wpa_sm)
		wpa_remove_ptk(sta->wpa_sm);
}
