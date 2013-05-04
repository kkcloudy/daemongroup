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
* Privsep_commands.h
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

#ifndef PRIVSEP_COMMANDS_H
#define PRIVSEP_COMMANDS_H

enum privsep_cmd {
	PRIVSEP_CMD_REGISTER,
	PRIVSEP_CMD_UNREGISTER,
	PRIVSEP_CMD_SET_WPA,
	PRIVSEP_CMD_SCAN,
	PRIVSEP_CMD_GET_SCAN_RESULTS,
	PRIVSEP_CMD_ASSOCIATE,
	PRIVSEP_CMD_GET_BSSID,
	PRIVSEP_CMD_GET_SSID,
	PRIVSEP_CMD_SET_KEY,
	PRIVSEP_CMD_GET_CAPA,
	PRIVSEP_CMD_L2_REGISTER,
	PRIVSEP_CMD_L2_UNREGISTER,
	PRIVSEP_CMD_L2_NOTIFY_AUTH_START,
	PRIVSEP_CMD_L2_SEND,
};

struct privsep_cmd_associate
{
	u8 bssid[ETH_ALEN];
	u8 ssid[32];
	size_t ssid_len;
	int freq;
	int pairwise_suite;
	int group_suite;
	int key_mgmt_suite;
	int auth_alg;
	int mode;
	size_t wpa_ie_len;
	/* followed by wpa_ie_len bytes of wpa_ie */
};

struct privsep_cmd_set_key
{
	int alg;
	u8 addr[ETH_ALEN];
	int key_idx;
	int set_tx;
	u8 seq[8];
	size_t seq_len;
	u8 key[32];
	size_t key_len;
};

enum privsep_event {
	PRIVSEP_EVENT_SCAN_RESULTS,
	PRIVSEP_EVENT_ASSOC,
	PRIVSEP_EVENT_DISASSOC,
	PRIVSEP_EVENT_ASSOCINFO,
	PRIVSEP_EVENT_MICHAEL_MIC_FAILURE,
	PRIVSEP_EVENT_INTERFACE_STATUS,
	PRIVSEP_EVENT_PMKID_CANDIDATE,
	PRIVSEP_EVENT_STKSTART,
	PRIVSEP_EVENT_FT_RESPONSE,
	PRIVSEP_EVENT_RX_EAPOL,
};

#endif /* PRIVSEP_COMMANDS_H */
