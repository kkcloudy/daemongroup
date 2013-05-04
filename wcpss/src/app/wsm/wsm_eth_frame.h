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
* wsm_eth_frame.h
*
*
* DESCRIPTION:
*  Mainly defined for the ethernet frame format.
*  There are 4 types frame type in the ethernet:
*           Ethernet II
*           802.3 raw frame
*           802.3 SAP
*           802.3 SNAP
*  But now, the mainly used frame type is Ethernet II. 
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

#ifndef _WSM_ETH_FRAME_H
#define _WSM_ETH_FRAME_H

#include "wcpss/waw.h"
#include "wsm_types.h"

struct eth_v2 {
	unsigned char dest_mac[MAC_LEN];
	unsigned char src_mac[MAC_LEN];
	unsigned char eth_type[2];
	unsigned char *data;
	unsigned int len;
};

#endif
