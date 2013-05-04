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
* ASDBeaconOp.h
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

#ifndef BEACON_H
#define BEACON_H

#define CLEAR_PROB_INTERVAL		90
#define	PROB_MAX_COUNTER		15
#define	PROB_MIN_COUNTER		5


void handle_probe_req(struct asd_data *wasd, struct ieee80211_mgmt *mgmt, size_t len);
void ieee802_11_set_beacon(struct asd_data *wasd);
void ieee802_11_set_beacons(struct asd_iface *iface);
int free_balance_info_in_wlan(unsigned char mac[],unsigned char wlanid);/*xm0804*/ 
int free_balance_info_all_in_wlan(unsigned char wlanid);
void clear_prob_log_timer(void *circle_ctx, void *timeout_ctx);




#endif /* BEACON_H */
