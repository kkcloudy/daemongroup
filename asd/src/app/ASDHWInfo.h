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
* AsdHWInfo.h
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

#ifndef HW_FEATURES_H
#define HW_FEATURES_H

#define asd_CHAN_W_SCAN 0x00000001
#define asd_CHAN_W_ACTIVE_SCAN 0x00000002
#define asd_CHAN_W_IBSS 0x00000004

struct asd_channel_data {
	short chan; /* channel number (IEEE 802.11) */
	short freq; /* frequency in MHz */
	int flag; /* flag for asd use (asd_CHAN_*) */
};

#define asd_RATE_ERP 0x00000001
#define asd_RATE_BASIC 0x00000002
#define asd_RATE_PREAMBLE2 0x00000004
#define asd_RATE_SUPPORTED 0x00000010
#define asd_RATE_OFDM 0x00000020
#define asd_RATE_CCK 0x00000040
#define asd_RATE_MANDATORY 0x00000100

struct asd_rate_data {
	int rate; /* rate in 100 kbps */
	int flags; /* asd_RATE_ flags */
};

struct asd_hw_modes {
	int mode;
	int num_channels;
	struct asd_channel_data *channels;
	int num_rates;
	struct asd_rate_data *rates;
};


void asd_free_hw_features(struct asd_hw_modes *hw_features,
			      size_t num_hw_features);
int asd_get_hw_features(struct asd_iface *iface);
int asd_select_hw_mode_start(struct asd_iface *iface,
				 asd_iface_cb cb);
int asd_select_hw_mode_stop(struct asd_iface *iface);
const char * asd_hw_mode_txt(int mode);
int asd_hw_get_freq(struct asd_data *wasd, int chan);
int asd_hw_get_channel(struct asd_data *wasd, int freq);

#endif /* HW_FEATURES_H */
