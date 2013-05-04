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
* ASDPreauth.h
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

#ifndef PREAUTH_H
#define PREAUTH_H

#ifndef ETH_P_PREAUTH
#define ETH_P_PREAUTH 0x88C7 /* IEEE 802.11i pre-authentication */
#endif /* ETH_P_PREAUTH */


#ifdef ASD_RSN_PREAUTH

int rsn_preauth_iface_init(struct asd_data *wasd);
void rsn_preauth_iface_deinit(struct asd_data *wasd);
void rsn_preauth_finished(struct asd_data *wasd, struct sta_info *sta,
			  int success);
void rsn_preauth_send(struct asd_data *wasd, struct sta_info *sta,
		      u8 *buf, size_t len);
void rsn_preauth_free_station(struct asd_data *wasd, struct sta_info *sta);

#else /* ASD_RSN_PREAUTH */

static inline int rsn_preauth_iface_init(struct asd_data *wasd)
{
	return 0;
}

static inline void rsn_preauth_iface_deinit(struct asd_data *wasd)
{
}

static inline void rsn_preauth_finished(struct asd_data *wasd,
					struct sta_info *sta,
					int success)
{
}

static inline void rsn_preauth_send(struct asd_data *wasd,
				    struct sta_info *sta,
				    u8 *buf, size_t len)
{
}

static inline void rsn_preauth_free_station(struct asd_data *wasd,
					    struct sta_info *sta)
{
}

#endif /* ASD_RSN_PREAUTH */

#endif /* PREAUTH_H */
