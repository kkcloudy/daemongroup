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
* wsm_frame_transform.c
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
 
#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <arpa/inet.h>
#include <syslog.h>

#include "CWCommon.h"
#include "CWProtocol.h"
#include "CWNetwork.h"
#include "CWLog.h"

#include "wcpss/waw.h"

#include "wsm_main_tbl.h"
#include "wsm_frame_transform.h"
#include "wsm_types.h"


static unsigned short SeqNumber = 0;
static unsigned short cwFragmentID = 0;
extern WTP_Element WTPtable[HASHTABLE_SIZE];
extern struct WSM_WLANID_BSSID_TABLE WLANID_BSSID_Table[WIFI_BIND_MAX_NUM]; 
extern __inline__ void WSMLog(unsigned char type, char *format, ...);


/**
* Description:
*  Judge LLC format , and now just support RFC1042 defined LLC
* 
* Parameter:
*  llc:Check out validity of LLC
*
* Return:
*  0 : OK ;  -1 : Error
*
*/
int ieee80211_judge_llc(struct ieee80211_llc llc)
{
	if (llc.llc_dsap != 0xaa ||
		llc.llc_ssap != 0xaa ||
		llc.llc_cmd != 0x03    )
		return -1;
	
	if (llc.llc_org_code[0] != '\0' ||
		llc.llc_org_code[1] != '\0' ||
		llc.llc_org_code[2] != '\0'   )
		return -1;
	
	return 0;
}

/**
* Description:
*   Get the Lay3 packet type from ethernet frame
*
* Parameter:
*  eth_raw_type: buffer
*  type: ARP or IP packet, type of packet
*
* Return:
*  0: ok ;  -1: Data error
*
*/
int ieee80211_get_eth_type(unsigned char* eth_raw_type, ETHER_TYPE_T *type)
{
	if (eth_raw_type[0] == 0x08 && eth_raw_type[1] == 0x0)
	{
		*type = ETHER_IP;
		return 0;
	}
	
	if (eth_raw_type[0] == 0x08 && eth_raw_type[1] == 0x06)
	{
		*type = ETHER_ARP;
		return 0;
	}
	if (eth_raw_type[0] == 0x86 && eth_raw_type[1] == 0xdd)
	{
		*type = ETHER_IPV6;
		return 0;
	}
	
	WSMLog(L_DEBUG, "%s: unknown frame type[%2x%2x]\n",
			__func__, eth_raw_type[0], eth_raw_type[1]);
	return 0;
}

/**
* Description:
*  Parse 802.11 Mac Header Frame Control Field
*
* Parameter:
*  fc: buffer
*  parsed_fc: parsed FC
*
* Return:
*  0: OK, -1: Data error
*
*/
int ieee80211_fc_parse(unsigned char *fc, struct ieee80211_fc_field *parsed_fc)
{
	if (fc == NULL || parsed_fc == NULL)
	{
		return -1;
	}

	if ((parsed_fc->fc_proto_ver = fc[0] & IEEE80211_FC0_VERSION_MASK) != 
		IEEE80211_FC0_VERSION_0)
	{
		return -2;
	}
		
	if ((parsed_fc->fc_type = fc[0] & IEEE80211_FC0_TYPE_MASK) != 
		IEEE80211_FC0_TYPE_DATA)
	{
		return -2;
	}
	
	parsed_fc->fc_subtype = fc[0] & IEEE80211_FC0_SUBTYPE_MASK;
	parsed_fc->fc_ds = fc[1] & IEEE80211_FC1_DIR_MASK;
	parsed_fc->fc_frag = fc[1] & IEEE80211_FC1_MORE_FRAG;
	parsed_fc->fc_retry = fc[1] & IEEE80211_FC1_RETRY;
	parsed_fc->fc_pwr = fc[1] & IEEE80211_FC1_PWR_MGT;
	parsed_fc->fc_moredata = fc[1] & IEEE80211_FC1_MORE_DATA;
	parsed_fc->fc_protect = fc[1] & IEEE80211_FC1_WEP;
	parsed_fc->fc_order = fc[1] & IEEE80211_FC1_ORDER;

	return 0;
}

/**
* Description:
*  Fill in frame control field with common option
*
* Parameter:
*  fs: fc header.
*
* Return:
*  void.
*
*/
void fill_fc_field(struct ieee80211_fc_field *fc)
{
	fc->fc_proto_ver = IEEE80211_FC0_VERSION_0;
	fc->fc_type = IEEE80211_FC0_TYPE_DATA;
	fc->fc_subtype = IEEE80211_FC0_SUBTYPE_DATA;
	fc->fc_ds = IEEE80211_FC1_DIR_FROMDS;
	fc->fc_frag = 0;
	fc->fc_moredata = 0;
	fc->fc_order = 0;
	fc->fc_protect = 0;
	fc->fc_pwr = 0;
	fc->fc_retry = 0;
}

/**
* Description:
*  Assemble capwap packet with 802.11 native frame 
*
* Parameter:
*  pdata: buffer
*  offset: offset
*  len: data length
*  wsmdata: Necessary data for assemble 802.11 frame
*
* Return:
*  0: ok, -1:error
*
*/
int assemble_capwap(unsigned char *pdata, unsigned int offset, unsigned int len, WSMData_T *wsmdata)
{
	unsigned int val = 0;
	unsigned char *buff = pdata + offset;
	CWProtocolTransportHeaderValues cwhead;
	CWProtocolTransportHeaderValues *valuesPtr = &cwhead;
	int sendSize;
	unsigned int *wtpid;
	
	cwhead.bindingValuesPtr = NULL;
	cwhead.fragmentID = getCWSeqNum();
	cwhead.fragmentOffset = 0;
	cwhead.isFragment = 0;
	cwhead.keepAlive = 0;
	cwhead.last = 0;
	cwhead.payloadType =0;
	cwhead.type = 0;
	
	CWSetField32(val, 
		     CW_TRANSPORT_HEADER_VERSION_START,
		     CW_TRANSPORT_HEADER_VERSION_LEN,
		     CW_PROTOCOL_VERSION); /* CAPWAP VERSION */

	CWSetField32(val,
		     CW_TRANSPORT_HEADER_TYPE_START,
		     CW_TRANSPORT_HEADER_TYPE_LEN,
		     0);
	
	CWSetField32(val,
		     CW_TRANSPORT_HEADER_HLEN_START,
		     CW_TRANSPORT_HEADER_HLEN_LEN,
		     12);

	CWSetField32(val,
		     CW_TRANSPORT_HEADER_RID_START,
		     CW_TRANSPORT_HEADER_RID_LEN,
		     wsmdata->Radio_L_ID); /* Radio local id */
	
	CWSetField32(val,
		     CW_TRANSPORT_HEADER_WBID_START,
		     CW_TRANSPORT_HEADER_WBID_LEN,
		     1); /* Wireless Binding ID */
	
	CWSetField32(val,
		     CW_TRANSPORT_HEADER_T_START,
		     CW_TRANSPORT_HEADER_T_LEN,
		     1);

	CWSetField32(val,
		     CW_TRANSPORT_HEADER_F_START,
		     CW_TRANSPORT_HEADER_F_LEN,
		     valuesPtr->isFragment); /* is fragment */

	CWSetField32(val,
		     CW_TRANSPORT_HEADER_L_START,
		     CW_TRANSPORT_HEADER_L_LEN,
		     valuesPtr->last); /* last fragment */
	
	CWSetField32(val,
		     CW_TRANSPORT_HEADER_W_START,
		     CW_TRANSPORT_HEADER_W_LEN,
		     1); /* have wireless option header */
	

	CWSetField32(val,
		     CW_TRANSPORT_HEADER_M_START,
		     CW_TRANSPORT_HEADER_M_LEN,
		     0); /* no radio MAC address */

	CWSetField32(val,
		     CW_TRANSPORT_HEADER_K_START,
		     CW_TRANSPORT_HEADER_K_LEN,
		     0); /* Keep alive flag */

	CWSetField32(val,
		     CW_TRANSPORT_HEADER_FLAGS_START,
		     CW_TRANSPORT_HEADER_FLAGS_LEN,
		     0); /* required */
		     
	val = htonl(val);
	WSMProtocolStore32(buff, val);
	buff += 4;
	
	val = 0;
	CWSetField32(val,
		     CW_TRANSPORT_HEADER_FRAGMENT_ID_START,
		     CW_TRANSPORT_HEADER_FRAGMENT_ID_LEN,
		     valuesPtr->fragmentID); /* fragment ID */
	
	CWSetField32(val,
		     CW_TRANSPORT_HEADER_FRAGMENT_OFFSET_START,
		     CW_TRANSPORT_HEADER_FRAGMENT_OFFSET_LEN,
		     valuesPtr->fragmentOffset); /* fragment offset */

	CWSetField32(val,
		     CW_TRANSPORT_HEADER_RESERVED_START,
		     CW_TRANSPORT_HEADER_RESERVED_LEN,
		     0); /* required */
		     
	val = htonl(val);	     
	WSMProtocolStore32(buff, val);
	buff += 4;
	val = 0;
	CWSetField32(val, 0, 8, wsmdata->WlanID);
	val = htonl(val);
	WSMProtocolStore32(buff, val);

	pthread_mutex_lock(&split_send_mutex);
	if (WTPtable[wsmdata->WTPID].ac_fd <= 0)
	{
		WSMLog(L_WARNING, "%s: ac file descriptor is :%d, error.\n",
				__func__, WTPtable[wsmdata->WTPID].ac_fd);
		pthread_mutex_unlock(&split_send_mutex);
		return -1;
	}
	
	if ((sendSize = sendto(WTPtable[wsmdata->WTPID].ac_fd, (pdata+offset), (len+54-offset), 0, 
		(struct sockaddr*)&(WTPtable[wsmdata->WTPID].WTPSock), sizeof(struct sockaddr_in6))) < 0)
	{
		WSMLog(L_INFO, "%s: sendto failed errno:%d.\n", __func__, sendSize);
		pthread_mutex_unlock(&split_send_mutex);
		return -1;
	}

	pthread_mutex_unlock(&split_send_mutex);
	
	return 0;
}

/**
* Description:
*  Main function, change 802.3 frame to 802.11 frame
*
* Parameter:
*  pdata: buffer
*  dataLen: data length
*  offset: data offset
*  seqNum: 802.11 Sequence Number
*  BSSID: BSSID...
*
* Return:
*  0:OK, -1, Error
*
*/
int ieee8023_to_ieee80211(unsigned char *pdata, int dataLen, int offset, unsigned short  seqNum, 
							unsigned char *BSSID,unsigned int protect_type)
{
	unsigned char IEEE8023_destMac[MAC_LEN];
	unsigned char IEEE8023_srcMac[MAC_LEN];
	unsigned char IEEE80211_addr1[MAC_LEN];
	unsigned char IEEE80211_addr2[MAC_LEN];
	unsigned char IEEE80211_addr3[MAC_LEN];
	
	unsigned char IEEE8023_type[2];
	struct ieee80211_frame *macHead;
	struct ieee80211_llc *llcHead;
	unsigned char  fcField[2];
	unsigned char *buff;
	/* No QoS and WDS */
	unsigned int macHeadLen = 24;

	buff = pdata + 54;
	/* It's just for test, every type of frames , the 802.11 mac header's length is different */
	macHead = (struct ieee80211_frame*)(pdata + 36);
	IEEE80211_RETRIEVE(IEEE8023_destMac, buff, IEEE8023_DEST_MAC_START, IEEE8023_MAC_LEN);
	IEEE80211_RETRIEVE(IEEE8023_srcMac, buff , IEEE8023_SRC_MAC_START, IEEE8023_MAC_LEN);
	IEEE80211_RETRIEVE(IEEE8023_type, buff, IEEE8023_TYPE_START, IEEE8023_TYPE_LEN);

	llcHead = (struct ieee80211_llc*)(pdata + 60);

	/* WEP type */
	if (protect_type)
	{
		macHead->i_fc[0] = 0x08;
		macHead->i_fc[1] = 0x42;
	}
	else
	{
		macHead->i_fc[0] = 0x08;
		macHead->i_fc[1] = 0x02;
	}

	macHead->i_dur[0] = 0;
	macHead->i_dur[1] = 0;
	
	memcpy(macHead->i_addr1, IEEE8023_destMac, MAC_LEN);
	memcpy(macHead->i_addr3, IEEE8023_srcMac, MAC_LEN);
	memcpy(macHead->i_addr2, BSSID, MAC_LEN);
	macHead->i_seq[0] = (seqNum<<4)&0xf0;
	macHead->i_seq[1] = (seqNum>>4)&0xff;

	llcHead->llc_dsap = 0xaa;
	llcHead->llc_ssap = 0xaa;
	llcHead->llc_cmd = 0x03;
	llcHead->llc_org_code[0] = 0;
	llcHead->llc_org_code[1] = 0;
	llcHead->llc_org_code[2] = 0;
	
	return 0;
}


/**
* Description:
*  Main function, assemble 802.11 frame to 802.3 frame
*
* Parameter:
*  index: Number of Queue,in the wsm there has 16 Queues, and every 
*			   queue has self-governed thread to deal with its. the index is
*			   queue number, but also is thread index.
* 
* Return:
*  0:OK, -1:Error
*
*/

int ieee80211_to_ieee8023(int index, unsigned char *retry_flag, unsigned char *stamac)
{
	unsigned char IEEE80211_fc[2];
	unsigned char IEEE80211_dur[2];
	unsigned char IEEE80211_addr1[MAC_LEN];
	unsigned char IEEE80211_addr2[MAC_LEN];
	unsigned char IEEE80211_addr3[MAC_LEN];
	unsigned char IEEE80211_addr4[MAC_LEN];
	unsigned char IEEE80211_seqfield[2];
	unsigned short IEEE80211_seq;
	unsigned char IEEE80211_frag;
	unsigned char IEEE80211_qos[2];
	unsigned int has_QoS = 0;
	unsigned int has_addr4 = 0;
	unsigned int macHead_len;
	ETHER_TYPE_T eth_type;
	unsigned char	eth_raw_type[2];
	unsigned char  BSSID[MAC_LEN];
	struct ieee80211_fc_field fc;
	struct ieee80211_llc llc;
	CWProtocolMessage *tmpMsg = NULL;
	unsigned char *pdata;
	
	tmpMsg = toWifiBuff + index;
	pdata = tmpMsg->msg + tmpMsg->offset;
	
	/* Get 802.11 Mac Header Frame Control Field */
	IEEE80211_RETRIEVE(IEEE80211_fc, pdata, IEEE80211_FC_START, IEEE80211_FC_LEN);
	if (ieee80211_fc_parse(IEEE80211_fc, &fc) != 0)
	{
		WSMLog(L_DEBUG, "%s: ieee80211_fc_parse failed.\n", __func__);
		return -1;
	}
	/* Get 802.11 Mac Header Duration/ID field */
	IEEE80211_RETRIEVE(IEEE80211_dur, pdata, IEEE80211_DUR_START, IEEE80211_DUR_LEN);
	/*Get 802.11 Mac header Addr1 to Addr3 */
	IEEE80211_RETRIEVE(IEEE80211_addr1, pdata, IEEE80211_ADDR1_START, IEEE80211_ADDR_LEN);
	IEEE80211_RETRIEVE(IEEE80211_addr2, pdata, IEEE80211_ADDR2_START, IEEE80211_ADDR_LEN);
	IEEE80211_RETRIEVE(IEEE80211_addr3, pdata, IEEE80211_ADDR3_START, IEEE80211_ADDR_LEN);

	/* Sequence field and fragment field */
	IEEE80211_RETRIEVE(IEEE80211_seqfield, pdata, IEEE80211_SEQ_START, IEEE80211_SEQ_LEN);

	/* For example, seq[0] = 0xab seq[1] = 0xcd, so frag=0xb, seq = 0xcda */
	IEEE80211_frag = IEEE80211_seqfield[0] & IEEE80211_SEQ_MASK;
	IEEE80211_seq = IEEE80211_seqfield[1] * 16 + IEEE80211_frag;
	
	macHead_len = 24;

	/* Option field Addr4 */
	if (fc.fc_ds == IEEE80211_FC1_DIR_DSTODS)
	{
		has_addr4 = 1;
		macHead_len += IEEE80211_ADDR_LEN;
		IEEE80211_RETRIEVE(IEEE80211_addr4, pdata, IEEE80211_ADDR4_START, IEEE80211_ADDR_LEN);
	}
	/* Option field QoS */
	if (fc.fc_subtype & IEEE80211_FC0_SUBTYPE_QOS)
	{	
		has_QoS = 1;
		/* Have padding.... */
		macHead_len = roundup(macHead_len + IEEE80211_QOS_LEN, sizeof(int));
		
		if (has_addr4)
			IEEE80211_RETRIEVE(IEEE80211_qos, pdata, IEEE80211_HAS_ADDR4_QOS, IEEE80211_QOS_LEN);
		else
			IEEE80211_RETRIEVE(IEEE80211_qos, pdata, IEEE80211_NO_ADDR4_QOS, IEEE80211_QOS_LEN);		
	}

	/* Judge 802.2 LLC */
	IEEE80211_RETRIEVE(&llc, pdata, macHead_len, IEEE80211_LLC_LEN);
	if (ieee80211_judge_llc(llc) != 0)
	{
		WSMLog(L_DEBUG, "%s: ieee80211_judge_llc failed.\n", __func__);
		return -3;
	}

	/* Get the Ethernet frame type, IP or ARP or other */
	IEEE80211_RETRIEVE(eth_raw_type, pdata, macHead_len + 6, 2);
	if (ieee80211_get_eth_type(eth_raw_type, &eth_type) != 0)
	{
		WSMLog(L_DEBUG, "%s: ieee80211_get_eth_type failed.\n", __func__);
		return -1;
	}
	
	/* From LLC get ethernet frame's payload type */
	macHead_len += (IEEE80211_LLC_LEN - 2);
	/* Prepare to reassemble ethernet frame header */
	memset(tmpMsg->msg, 0, macHead_len + tmpMsg->offset);
	/* DMACLen + SMACLen  == 12*/
	tmpMsg->offset = tmpMsg->offset + macHead_len - 12;
	tmpMsg->msgLen = tmpMsg->msgLen - (macHead_len - 12);
	pdata = tmpMsg->msg + tmpMsg->offset;

	/* Is it a retry packet? */
	if (fc.fc_retry)
	{
		*retry_flag = 1;
	}
	
	/* Fill in Ethernet frame address fields */
	switch(fc.fc_ds)
	{
		case IEEE80211_FC1_DIR_TODS:  /* STA  ---> AP */
			memcpy(pdata, IEEE80211_addr3, MAC_LEN);
			memcpy(pdata + MAC_LEN, IEEE80211_addr2, MAC_LEN);	
			memcpy(BSSID, IEEE80211_addr1, MAC_LEN);
			/* For STA packet statistics */
			memcpy(stamac, IEEE80211_addr2, MAC_LEN);
			break;
		/* This type of packet will be transfer to AP? */
		case IEEE80211_FC1_DIR_FROMDS:  /* STA <--- AP */
			memcpy(pdata, IEEE80211_addr1, MAC_LEN);
			memcpy(pdata + MAC_LEN, IEEE80211_addr3, MAC_LEN);
			memcpy(BSSID, IEEE80211_addr2, MAC_LEN);
			break;
		case IEEE80211_FC1_DIR_DSTODS:  /* AP <--> AP */
			memcpy(pdata, IEEE80211_addr3, MAC_LEN);
			memcpy(pdata + MAC_LEN, IEEE80211_addr4, MAC_LEN);
			/* When the WDS Mode, BSSID == addr2? Is it right? */
			memcpy(BSSID, IEEE80211_addr2, MAC_LEN);
			break;
		/* This type of packet will be transfer to AP?*/
		case IEEE80211_FC1_DIR_NODS:  /* STA <-->STA */
			memcpy(pdata, IEEE80211_addr1, MAC_LEN);
			memcpy(pdata + MAC_LEN, IEEE80211_addr2, MAC_LEN);
			memcpy(BSSID, IEEE80211_addr3, MAC_LEN);
			break;
		default:
			return -1;
	}
	
	return 0;
}

/**
* Description:
*  For the Sequence Number field, range of Sequence Number: [0, 4095]
*  
* Parameter:
*  void.
*
* Return:
*  Sequence Number
*
*/
unsigned short getSeqNum(void)
{
	unsigned short flag = SeqNumber;

	if (++SeqNumber == 4096)
		SeqNumber = 0;

	return flag;
}

/**
* Description:
*  For CAPWAP Fragment ID field, range [0, 65535]
* 
* Parameter:
*  void.
* 
* Return:
*  CAPWAP Sequence Number
*
*/
unsigned short getCWSeqNum(void)
{
	unsigned short flag = cwFragmentID;

	if (++cwFragmentID == 65535)
		cwFragmentID = 0;
	return flag;
}

/**
* Description:
*  Find BSSID by BSSIndex for broadcast
*
* Parameter:
*  BSSIndex: Use BSSIndex for searching necessary data
*  data: Necessary data for assembling 802.11 frame
*
* Return:
*  BSSID data.
*
*/
__inline__ WSMData_T  *wsm_find_bssid(int BSSIndex, WSMData_T *data)
{
	if ( WTPtable[BSSIndex/BSS_ARRAY_SIZE].flag == 0 ||
		WTPtable[BSSIndex/BSS_ARRAY_SIZE].next == NULL ||
	     (WTPtable[BSSIndex/BSS_ARRAY_SIZE].next + (BSSIndex%BSS_ARRAY_SIZE)) == NULL ||
	     (WTPtable[BSSIndex/BSS_ARRAY_SIZE].next + (BSSIndex%BSS_ARRAY_SIZE))->flag == 0 ||
	     data == NULL)
		{
			return NULL;
		}
	data->Radio_L_ID = (WTPtable[BSSIndex/BSS_ARRAY_SIZE].next + (BSSIndex%BSS_ARRAY_SIZE))->BSS.Radio_L_ID;
	data->WlanID = (WTPtable[BSSIndex/BSS_ARRAY_SIZE].next + (BSSIndex%BSS_ARRAY_SIZE))->BSS.WlanID;
	data->WTPID = BSSIndex/BSS_ARRAY_SIZE;
	data->BSSID = (WTPtable[BSSIndex/BSS_ARRAY_SIZE].next + (BSSIndex%BSS_ARRAY_SIZE))->BSS.BSSID;
	
	return data;
}

/** 
* Description:
*  Get BSS data from the same Wlan
*
* Parameter:
*  wlanid: WlanID...
* 
* Return:
*  Null: Cannot find data, Other: BSS data pointer
*
*/
__inline__ struct wsm_wlanid_bssid *wsm_get_bss_of_wlan(unsigned int wlanid)
{
	return WLANID_BSSID_Table[wlanid - 1].bsscount > 0? WLANID_BSSID_Table[wlanid - 1].next : NULL;
}


