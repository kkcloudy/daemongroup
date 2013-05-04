/******************************************************************************
Copyright (C) Autelan Technology

This software file is owned and distributed by Autelan Technology 
*******************************************************************************

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
*******************************************************************************
* wsm_80211_frame.h
*
*
* DESCRIPTION:
*  Define the format of frames for IEEE802.11 Data type frame.
*
* DATE:
*  2008-07-11
*
* CREATOR:
*  guoxb@autelan.com
*
* CHANGE LOG:
*  2008-07-11 <guoxb> Create file.
*  2010-02-09 <guoxb> Modified files name, copyright etc.
*
******************************************************************************/

#ifndef _WSM_80211_FRAME_H
#define _WSM_80211_FRAME_H

#include "wsm_types.h"


/*
 * generic definitions for IEEE 802.11 frames
 */
struct ieee80211_frame {
	unsigned char i_fc[2];
	unsigned char i_dur[2];
	unsigned char i_addr1[IEEE80211_ADDR_LEN];
	unsigned char i_addr2[IEEE80211_ADDR_LEN];
	unsigned char i_addr3[IEEE80211_ADDR_LEN];
	unsigned char i_seq[2];
}__packed;
 
struct ieee80211_qosframe {
	unsigned char i_fc[2];
	unsigned char i_dur[2];
	unsigned char i_addr1[IEEE80211_ADDR_LEN];
	unsigned char i_addr2[IEEE80211_ADDR_LEN];
	unsigned char i_addr3[IEEE80211_ADDR_LEN];
	unsigned char i_seq[2];
	unsigned char i_qos[2];
}__packed;
 
struct ieee80211_frame_addr4 {
	unsigned char i_fc[2];
	unsigned char i_dur[2];
	unsigned char i_addr1[IEEE80211_ADDR_LEN];
	unsigned char i_addr2[IEEE80211_ADDR_LEN];
	unsigned char i_addr3[IEEE80211_ADDR_LEN];
	unsigned char i_seq[2];
	unsigned char i_addr4[IEEE80211_ADDR_LEN];
}__packed;

struct ieee80211_qosframe_addr4 {
	unsigned char i_fc[2];
	unsigned char i_dur[2];
	unsigned char i_addr1[IEEE80211_ADDR_LEN];
	unsigned char i_addr2[IEEE80211_ADDR_LEN];
	unsigned char i_addr3[IEEE80211_ADDR_LEN];
	unsigned char i_seq[2];
	unsigned char i_addr4[IEEE80211_ADDR_LEN];
	unsigned char i_qos[2];
}__packed;

struct ieee80211_fc_field{
	unsigned char fc_proto_ver;
	unsigned char fc_type;
	unsigned char fc_subtype;
	unsigned char fc_ds;
	unsigned char fc_frag;
	unsigned char fc_retry;
	unsigned char fc_pwr;
	unsigned char fc_moredata;
	unsigned char fc_protect;
	unsigned char fc_order;
};

#endif

