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
* wsm_frame_transform.h
*
*
* DESCRIPTION:
*  WSM wireless DATA type frame transmit mainly source code file. In this file,
*  there have all of the operation about change wireless frame to ethernet 
*  frame or from ethernet frame to wireless frame.
*
*
* DATE:
*  2009-07-11
*
* CREATOR:
*  guoxb@autelan.com
*
* CHANGE LOG:
*  2008-07-11 <guoxb> Create file.
*  2010-02-09 <guoxb> Modified files name, copyright etc.
*
******************************************************************************/

#ifndef _WSM_FRAME_TRANSFORM_H
#define _WSM_FRAME_TRANSFORM_H

#include <stdio.h>
#include <string.h>
#include <strings.h>

#include "CWCommon.h"
#include "CWProtocol.h"
#include "CWNetwork.h"
#include "CWLog.h"

#include "wcpss/waw.h"

#include "wsm_80211_frame.h"
#include "wsm_eth_frame.h"
#include "wsm_shared_mem.h"
#include "wsm_llc.h"
#include "wsm_types.h"


/* to any y */
#define	roundup(x, y)	((((x)+((y)-1))/(y))*(y)) 
#define WSMProtocolStore32(msg, val) bcopy(&(val), (msg), 4)
#define IEEE80211_RETRIEVE(dest, src, offset, len) memcpy((unsigned char*)(dest),((unsigned char*)(src)+offset), (len))

extern CWProtocolMessage WTPmsg;
extern pthread_mutex_t split_send_mutex;
extern CWProtocolMessage toWifiBuff[WSM_QUEUE_NUM];
extern CWProtocolMessage fromWifiBuff[WSM_QUEUE_NUM];

typedef struct {
	unsigned char Radio_L_ID;
	unsigned char WlanID;
	unsigned char *BSSID;
	unsigned int WTPID;
	unsigned int protect_type;
} WSMData_T;


int ieee80211_judge_llc(struct ieee80211_llc llc);
int ieee80211_get_eth_type(unsigned char* eth_raw_type, ETHER_TYPE_T *type);
int ieee80211_fc_parse(unsigned char *fc, struct ieee80211_fc_field *parsed_fc);
void fill_fc_field(struct ieee80211_fc_field *fc);
int assemble_capwap(unsigned char *pdata, unsigned int offset, unsigned int len, WSMData_T *wsmdata);
int ieee8023_to_ieee80211(unsigned char *pdata, int dataLen, int offset, unsigned short seqNum, 
	unsigned char *BSSID, unsigned int protect_type);
int ieee80211_to_ieee8023(int index, unsigned char *retry_flag, unsigned char *stamac);
unsigned short getSeqNum(void);
unsigned short getCWSeqNum(void);
__inline__ WSMData_T *wsm_find_bssid(int BSSIndex, WSMData_T *bData);
__inline__ struct wsm_wlanid_bssid *wsm_get_bss_of_wlan(unsigned int wlanid);

#endif

