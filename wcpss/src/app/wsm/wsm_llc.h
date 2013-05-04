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
* wsm_llc.h
*
*
* DESCRIPTION:
*  Because the 802.11 frame is encapsulation by 802.2 LLC, we should add this 
*  when convertion the frame from 802.3 to 802.11. And there have two types 
*  of the LLC: AppleTalk and IPX use 802.1H, Others use  RFC 1042.
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
 
#ifndef _WSM_LLC_H
#define _WSM_LLC_H

#include "wsm_types.h"

/*
 * RFC1042: 
 *          DSAP=0xAA, SSAP= 0xAA, Command=0x03(UI)
 *          Org Code=0x00 00 00, Protocol type=0x0800(IP), or 0x0806(ARP)
 *
 * 802.1H: 
 *          DSAP=0xAA, SSAP= 0xAA, Command=0x03(UI)
 *          Org Code=0x00 00 F8, Protocol type=0x0800(IP), or 0x0806(ARP)
 *
 */
 
struct ieee80211_llc {
	unsigned char llc_dsap;
	unsigned char llc_ssap;
	unsigned char llc_cmd;
	unsigned char llc_org_code[3];
	unsigned char llc_ether_type[2];
} __packed;

#endif
