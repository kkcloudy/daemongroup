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
* stp_transmit.c
*
* CREATOR:
*       zhubo@autelan.com
*
* DESCRIPTION:
*       APIs for Port Transmit state machine in stp module
*
* DATE:
*       04/18/2008
*
*  FILE REVISION NUMBER:
*       $Revision: 1.2 $
*******************************************************************************/
#ifdef __cplusplus
extern "C" {
#endif

/* Port Transmit state machine : 17.27 */
  
#include "stp_base.h"
#include "stp_stpm.h"
#include "stp_to.h" /* for stp_to_get_port_mac & stp_to_tx_bpdu */
#include "stp_transmit.h"
#define BPDU_LEN8023_OFF    12

#define STATES {        \
  CHOOSE(TRANSMIT_INIT),    \
  CHOOSE(TRANSMIT_PERIODIC),    \
  CHOOSE(IDLE),         \
  CHOOSE(TRANSMIT_CONFIG),  \
  CHOOSE(TRANSMIT_TCN),     \
  CHOOSE(TRANSMIT_RSTP),    \
}

#define GET_STATE_NAME STP_transmit_get_state_name
#include "stp_choose.h"

#define MIN_FRAME_LENGTH    64
#if 0
typedef struct tx_tcn_bpdu_t {
  MAC_HEADER_T  mac;
  ETH_HEADER_T  eth;
  BPDU_HEADER_T hdr;
} TCN_BPDU_T;

typedef struct tx_stp_bpdu_t {
  MAC_HEADER_T  mac;
  ETH_HEADER_T  eth;
  BPDU_HEADER_T hdr;
  BPDU_BODY_T   body;
} CONFIG_BPDU_T;

typedef struct tx_rstp_bpdu_t {
  MAC_HEADER_T  mac;
  ETH_HEADER_T  eth;
  BPDU_HEADER_T hdr;
  BPDU_BODY_T   body;
  unsigned char ver_1_length[2];
} RSTP_BPDU_T;
#endif
void sendStpTcnbittoNpd(PORT_T *port)
{
	register STPM_T  *stpm = port->owner;
	unsigned int mstid = port->vlan_id;
	unsigned int port_index = port->port_index;
	unsigned short vid = 0;
        stp_syslog_dbg("sendStpTcnbittoNpd::port index %d,mstid %d\n",port->port_index,mstid);
				stp_npd_cmd_notify_tcn(mstid,vid,port_index);
	return ;
}
static RSTP_BPDU_T g_bpdu_packet  = {
  {/* MAC_HEADER_T */
    {0x01, 0x80, 0xc2, 0x00, 0x00, 0x00},   /* dst_mac */
    {0x00, 0x00, 0x00, 0x00, 0x00, 0x00}    /* src_mac */
  },
  { /* ETH_HEADER_T */
    {0x00, 0x00},               /* len8023 */
    BPDU_L_SAP, BPDU_L_SAP, LLC_UI      /* dsap, ssap, llc */
  },
  {/* BPDU_HEADER_T */
    {0x00, 0x00},               /* protocol */
    BPDU_VERSION_ID, 0x00           /* version, bpdu_type */
  },
  {
    0x00,                   /*  flags; */
    {0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00},  /*  root_id[8]; */
    {0x00,0x00,0x00,0x00},          /*  root_path_cost[4]; */
    {0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00},  /*  bridge_id[8]; */
    {0x00,0x00},                /*  port_id[2]; */
    {0x00,0x00},                /*  message_age[2]; */
    {0x00,0x00},                /*  max_age[2]; */
    {0x00,0x00},                /*  hello_time[2]; */
    {0x00,0x00}                /*  forward_delay[2]; */
  },
  0x00               /*  ver_1_length[1]; */
};

static size_t
stp_transmit_build_bpdu_header (RSTP_BPDU_T * bpdu_packet, 
                   int port_index,
                   unsigned char bpdu_type,
                   unsigned short pkt_len)
{
  unsigned short len8023;

  stp_to_get_port_mac (port_index, bpdu_packet->mac.src_mac);

  bpdu_packet->hdr.bpdu_type = bpdu_type;
  switch( bpdu_type )
  {
      case BPDU_RSTP:
          bpdu_packet->hdr.version = BPDU_VERSION_RAPID_ID ;
          break;
      case BPDU_MSTP:
          bpdu_packet->hdr.bpdu_type = BPDU_RSTP ;
          bpdu_packet->hdr.version = BPDU_VERSION_MULTI_ID ;
          break;
      default:
          bpdu_packet->hdr.version = BPDU_VERSION_ID ;
  }
  
  /* NOTE: I suppose, that sizeof(unsigned short)=2 ! */
  len8023 = htons ((unsigned short) (pkt_len + 3));
  memcpy (&bpdu_packet->eth.len8023, &len8023, 2); 
  
  if (pkt_len < MIN_FRAME_LENGTH) pkt_len = MIN_FRAME_LENGTH;
  return pkt_len;
}

static int
stp_transmit_txTcn (STATE_MACH_T* this)
{ /* 17.19.17 (page 68) & 9.3.2 (page 25) */
  register size_t       pkt_len;
  register int          port_index, vlan_id;
  RSTP_BPDU_T	* bpdu_packet = NULL;
  register PORT_T *ports = NULL;
  struct stpm_t*   owner = NULL;
  int iret = STP_OK;
#ifdef STP_DBG
  if (this->owner.port->skip_tx > 0) {
    if (1 == this->owner.port->skip_tx)
      stp_trace ("port %s stop tx skipping",
                 this->owner.port->port_name);
    this->owner.port->skip_tx--;
    return STP_Nothing_To_Do;
  }
#endif

  if (this->owner.port->admin_non_stp) return 1;
  port_index = this->owner.port->port_index;
  vlan_id = this->owner.port->owner->vlan_id;

  bpdu_packet = (RSTP_BPDU_T *)malloc(sizeof(RSTP_BPDU_T));
  if (NULL == bpdu_packet)
  {
    return STP_ERROR;
  }
  memcpy(bpdu_packet, &g_bpdu_packet, sizeof(RSTP_BPDU_T));
  pkt_len = stp_transmit_build_bpdu_header (bpdu_packet, port_index,
                               BPDU_TOPO_CHANGE_TYPE,
                               sizeof (BPDU_HEADER_T));

#ifdef STP_DBG
  if (this->debug)
    stp_trace ("port %s stp_transmit_txTcn", this->owner.port->port_name);
#endif
  iret = stp_to_tx_bpdu (port_index, vlan_id,
                          (unsigned char *) bpdu_packet,
                          pkt_len + sizeof(MAC_HEADER_T) + sizeof(ETH_HEADER_T), 
                          this->owner.port->isWAN, this->owner.port->slot_no, this->owner.port->port_no);
 
  /*should notify to npd to syn the arp & fdb*/
 owner = this->owner.port->owner;
 for(ports = owner->ports;ports;ports = ports->next){
 	if(ports->portEnabled){
		stp_syslog_dbg("stp_transmit_txTcn:: port %d flush fdb & arp!\n",ports->port_index);
		sendStpTcnbittoNpd(ports);
 	}
 }
  free(bpdu_packet);
  return iret;
}

static void
stp_transmit_build_config_bpdu (RSTP_BPDU_T * bpdu_packet, PORT_T* port, Bool set_topo_ack_flag)
{
  bpdu_packet->body.flags = 0;
  if (port->tcWhile) {
#ifdef STP_DBG
    if (port->topoch->debug)
      stp_trace ("tcWhile=%d =>tx TOLPLOGY_CHANGE_BIT to port %s",
                 (int) port->tcWhile, port->port_name);
#endif
    bpdu_packet->body.flags |= TOLPLOGY_CHANGE_BIT;
  }

  if (set_topo_ack_flag && port->tcAck) {
    bpdu_packet->body.flags |= TOLPLOGY_CHANGE_ACK_BIT;
  }

  stp_vector_set_vector (&port->portPrio, &(bpdu_packet->body));
  stp_times_set (&port->portTimes, &(bpdu_packet->body));
}

static int
stp_transmit_txConfig (STATE_MACH_T* this)
{/* 17.19.15 (page 67) & 9.3.1 (page 23) */
  register size_t   pkt_len;
  register PORT_T*  port = NULL;
  register int      port_index, vlan_id;
  RSTP_BPDU_T	* bpdu_packet = NULL;
  int iret =STP_OK;

#ifdef STP_DBG
  if (this->owner.port->skip_tx > 0) {
    if (1 == this->owner.port->skip_tx)
      stp_trace ("port %s stop tx skipping",
                 this->owner.port->port_name);
    this->owner.port->skip_tx--;
    return STP_Nothing_To_Do;
  }
#endif

  port = this->owner.port;
  if (port->admin_non_stp) return 1;
  port_index = port->port_index;
  vlan_id = port->owner->vlan_id;
  
  bpdu_packet = (RSTP_BPDU_T *)malloc(sizeof(RSTP_BPDU_T));
  if (NULL == bpdu_packet)
  {
    return STP_ERROR;
  }
  memcpy(bpdu_packet, &g_bpdu_packet, sizeof(RSTP_BPDU_T));
  
  pkt_len = stp_transmit_build_bpdu_header (bpdu_packet, port->port_index,
                               BPDU_CONFIG_TYPE,
                               sizeof (BPDU_HEADER_T) + sizeof (BPDU_BODY_T));
  stp_transmit_build_config_bpdu (bpdu_packet, port, True);
 
#ifdef STP_DBG
  if (this->debug)
    stp_trace ("port %s stp_transmit_txConfig flags=0X%lx",
        port->port_name,
        (unsigned long) bpdu_packet->body.flags);
#endif
  iret = stp_to_tx_bpdu (port_index, vlan_id,
                          (unsigned char *) bpdu_packet,
                          pkt_len + sizeof(MAC_HEADER_T) + sizeof(ETH_HEADER_T), 
                          this->owner.port->isWAN, this->owner.port->slot_no, this->owner.port->port_no);
  free(bpdu_packet);
  return iret;
}

static int
stp_transmit_txRstp (STATE_MACH_T* this)
{/* 17.19.16 (page 68) & 9.3.3 (page 25) */
  register size_t       pkt_len;
  register PORT_T*      port = NULL;
  register int          port_index, vlan_id;
  unsigned char         role;
  RSTP_BPDU_T	* bpdu_packet = NULL;
  int iret = STP_OK;

#ifdef STP_DBG
  if (this->owner.port->skip_tx > 0) {
    if (1 == this->owner.port->skip_tx)
      stp_trace ("port %s stop tx skipping",
                 this->owner.port->port_name);
    else
      stp_trace ("port %s skip tx %d",
                 this->owner.port->port_name, this->owner.port->skip_tx);

    this->owner.port->skip_tx--;
    return STP_Nothing_To_Do;
  }
#endif

  port = this->owner.port;
  if (port->admin_non_stp) return 1;
  port_index = port->port_index;
  vlan_id = port->owner->vlan_id;
  bpdu_packet = (RSTP_BPDU_T *)malloc(sizeof(RSTP_BPDU_T));
  if (NULL == bpdu_packet)
  {
    return STP_ERROR;
  }
  memcpy(bpdu_packet, &g_bpdu_packet, sizeof(RSTP_BPDU_T));
  

  pkt_len = stp_transmit_build_bpdu_header (bpdu_packet, port->port_index,
                               BPDU_RSTP,
                               sizeof (BPDU_HEADER_T) + sizeof (BPDU_BODY_T) + 1);
  stp_transmit_build_config_bpdu (bpdu_packet, port, False);

  switch (port->selectedRole) {
    default:
    case DisabledPort:
#ifdef STP_DBG
	stp_trace("port %s port_index[%d],port->selectedRole = %s\n",
			this->owner.port->port_name, this->owner.port->port_index,"DisabledPort");
#endif
      role = RSTP_PORT_ROLE_UNKN;
      break;
    case AlternatePort:
      role = RSTP_PORT_ROLE_ALTBACK;
      break;
    case BackupPort:
      role = RSTP_PORT_ROLE_ALTBACK;
      break;
    case RootPort:
#ifdef STP_DBG
	stp_trace("port %s port_index[%d],port->selectedRole = %s\n",
			this->owner.port->port_name, this->owner.port->port_index,"RootPort");
#endif
      role = RSTP_PORT_ROLE_ROOT;
      break;
    case DesignatedPort:
#ifdef STP_DBG
	stp_trace("port %s port_index[%d],port->selectedRole = %s\n",
			this->owner.port->port_name, this->owner.port->port_index,"DesignatedPort");
#endif
      role = RSTP_PORT_ROLE_DESGN;
      break;
  }

  bpdu_packet->body.flags |= (role << PORT_ROLE_OFFS);

  if (port->synced) {
#if 0 /* def STP_DBG */
    if (port->roletrns->debug)
      stp_trace ("tx AGREEMENT_BIT to port %s", port->port_name);
#endif
    bpdu_packet->body.flags |= AGREEMENT_BIT;
  }

  if (port->proposing) {
#if 0 /* def STP_DBG */
    if (port->roletrns->debug)
      stp_trace ("tx PROPOSAL_BIT to port %s", port->port_name);
#endif
    bpdu_packet->body.flags |= PROPOSAL_BIT;
  }
#if 0
	if(port->learning)
	{
		bpdu_packet.body.flags |=LEARN_BIT;
	}

	if(port->forwarding)
	{
		bpdu_packet.body.flags |=FORWARD_BIT;
	}
#endif
  if (port->forwarding) {
#if 0/*def STP_DBG*/
	if (port->transmit->debug)
	  stp_trace ("tx FORWARD_BIT to port %s", port->port_name);
#endif
    bpdu_packet->body.flags |= FORWARD_BIT;
  }
  
  if (port->learning) {
  #if 0 /* def STP_DBG */
    if (port->transmit->debug)
      stp_trace ("Ttx LEARN_BIT to port %s", port->port_name);
  #endif
    bpdu_packet->body.flags |= LEARN_BIT;
  }
  
#ifdef STP_DBG
  if (this->debug)
    stp_trace ("port %s stp_transmit_txRstp flags=0X%lx",
        port->port_name,
        (unsigned long) bpdu_packet->body.flags);
#endif
   
  iret = stp_to_tx_bpdu (port_index, vlan_id,
                          (unsigned char *) bpdu_packet,
                          pkt_len + sizeof(MAC_HEADER_T) + sizeof(ETH_HEADER_T), 
                          this->owner.port->isWAN, this->owner.port->slot_no, this->owner.port->port_no);
  free(bpdu_packet);
  return iret;
}

static void stp_transmit_build_cisco_config_bpdu_mstp
(
	MSTP_Cisco_BPDU_T *bpdu_packet,
	PORT_T* port
)
{
	unsigned int root_path_cost = 0;

	bpdu_packet->body.flags = 0;
	if (port->tcWhile) {
		bpdu_packet->body.flags |= TOLPLOGY_CHANGE_BIT;
	}
	if (port->proposing) {
		bpdu_packet->body.flags |= PROPOSAL_BIT;
	}
	if (port->learning) {
		bpdu_packet->body.flags |= LEARN_BIT;
	}
	if (port->forwarding) {
		bpdu_packet->body.flags |= FORWARD_BIT;
	}
	if (port->agree) {
		bpdu_packet->body.flags |= AGREEMENT_BIT;
	}

	stp_vector_set_bridge_id(&port->portPrio.root_bridge, bpdu_packet->body.root_id);
	root_path_cost = htonl(port->portPrio.root_path_cost);
	memcpy (bpdu_packet->body.root_path_cost, &root_path_cost, 4);

	/* CIST Bridge Identifier */
	stp_vector_set_bridge_id(&(port->owner->BrId), bpdu_packet->body.bridge_id);
	(*(unsigned short*)bpdu_packet->body.port_id) = htons(port->port_id);
  
	stp_times_set (&port->portTimes, (BPDU_BODY_T *)&bpdu_packet->body);

	return;    
}

static void stp_transmit_build_config_bpdu_mstp (MSTP_BPDU_T * bpdu_packet, PORT_T* port)
{
  unsigned int            root_path_cost = 0 ;
  
  bpdu_packet->body.flags = 0;
  if (port->tcWhile) bpdu_packet->body.flags |= TOLPLOGY_CHANGE_BIT;
  if ( port->proposing ) bpdu_packet->body.flags |= PROPOSAL_BIT;   
  if ( port->learning ) bpdu_packet->body.flags |= LEARN_BIT;
  if ( port->forwarding ) bpdu_packet->body.flags |= FORWARD_BIT;
  if (port->agree )  bpdu_packet->body.flags |= AGREEMENT_BIT;
  
  stp_vector_set_bridge_id ( &port->portPrio.root_bridge , bpdu_packet->body.root_id );
  root_path_cost = htonl( port->portPrio.root_path_cost);
  memcpy (bpdu_packet->body.root_path_cost, &root_path_cost, 4);	
  stp_vector_set_bridge_id ( &port->portPrio.region_root_bridge , bpdu_packet->body.bridge_id);
  (*(unsigned short*)bpdu_packet->body.port_id ) = htons( port->port_id );
  
  stp_times_set (&port->portTimes, (BPDU_BODY_T *)&bpdu_packet->body);
  
  return;    
}

void stp_transmit_dump_string
(
	unsigned char *string,
	unsigned int size
)
{
	unsigned int i;
	unsigned char lineBuffer[64] = {0}, *bufPtr = NULL;
	unsigned int curLen = 0;

	if (!string) {
		return;
	}
	stp_trace("dump string:");

	bufPtr = lineBuffer;
	curLen = 0;
	for (i = 0; i < size; i++)
	{
		curLen += sprintf(bufPtr,"%02x ", string[i]);
		bufPtr = lineBuffer + curLen;
		
		if (0==(i+1)%16)
		{
			stp_trace("%s", lineBuffer);
			memset(lineBuffer, 0, sizeof(lineBuffer));
			curLen = 0;
			bufPtr = lineBuffer;
		}
	}

	if ((size % 16) != 0)
	{
		stp_trace("%s", lineBuffer);
	}
	stp_trace("dump string:over");

	return;
}

int stp_transmit_txMstp_to_Cisco
(
	STATE_MACH_T *this
)
{
	register PORT_T *port = NULL;
	register PORT_T *ports = NULL;
	register PORT_T *mstport = NULL;
	MSTP_Cisco_BPDU_T *bpdu_packet = NULL;
	struct stpm_t *owner = NULL;
	unsigned char bpdu_type = 0;
	unsigned short pkt_len = 0;
	unsigned short version_3_length = 0;
	unsigned short temp = 0;
	unsigned short index = 0;
	int iret = 0;
	unsigned char role = RSTP_PORT_ROLE_UNKN;

	port = this->owner.port;

	/* 1 malloc memory for packet of Cisco MSTP BPDU */  
	bpdu_packet = (MSTP_Cisco_BPDU_T *)malloc(sizeof(MSTP_Cisco_BPDU_T));
	if (NULL == bpdu_packet)
	{
		return STP_ERROR;
	}
	memset(bpdu_packet, 0, sizeof(MSTP_Cisco_BPDU_T));

	/* 2 copy eth header */
	memcpy(bpdu_packet, &g_bpdu_packet, sizeof(g_bpdu_packet));
  
	stp_transmit_build_cisco_config_bpdu_mstp(bpdu_packet, port);

	switch (port->role) 
	{ 
		case MasterPort :
			role = MSTP_PORT_ROLE_MASTER;
			break;
		case AlternatePort:
			role = RSTP_PORT_ROLE_ALTBACK;
			break;
		case BackupPort:
			role = RSTP_PORT_ROLE_ALTBACK;
			break;
		case RootPort:
			role = RSTP_PORT_ROLE_ROOT;
			break;
		case DesignatedPort:
			role = RSTP_PORT_ROLE_DESGN;
			break;
		default :
			role = RSTP_PORT_ROLE_UNKN;
	}

	bpdu_packet->body.flags |= (role << PORT_ROLE_OFFS);	
  
	if (port->owner->ForceVersion >= 3)
	{

		MSTP_BPDU_T *inBpduPkt = NULL ;
		MSTP_Cisco_BPDU_T *ciscoBpdu = NULL;
		int inBpduType = 0;
		int ret_val = False;
		unsigned char digestSnp = 0;

		/* CISTBridgeIdentifier */
		memcpy(bpdu_packet->ciscoMSTID.ConfigurationName,
				port->owner->MstConfigId.ConfigurationName, 32);
		memcpy(bpdu_packet->ciscoMSTID.RevisionLevel,
				port->owner->MstConfigId.RevisionLevel, 2);
		memcpy(bpdu_packet->ciscoMSTID.ConfigurationDigest,
				port->owner->MstConfigId.ConfigurationDigest, 16);
		stp_trace("owner configuration Digest:");
		stp_transmit_dump_string(bpdu_packet->ciscoMSTID.ConfigurationDigest, 16);

		/* patch up: support inter-connect with Cisco for configuration digest differ */
		ret_val = stp_in_get_cur_bpdu((BPDU_T *)&inBpduPkt, &inBpduType, &digestSnp);
		stp_trace("ret_val %d inBpduPkt %p inBpduType %x",
					ret_val, inBpduPkt, inBpduType);
		stp_trace("portindex %#x, configDigestSnp %d",
					port->port_index, port->configDigestSnp);

		/*
		 * set digest from port which get from receive packet
		 */
		if (1 == (port->configDigestSnp & 0x1))
		{
			memcpy(&bpdu_packet->ciscoMSTID.ConfigurationDigest, port->digest, 16);
			stp_trace("port receive Digest Snooping:");
			stp_transmit_dump_string(bpdu_packet->ciscoMSTID.ConfigurationDigest, 16);
		}

		#if 1
		/* if have user confif digest,
		 * set digest with user config
		 */
		unsigned char tmp_bpdu_type = 0;
		struct stpm_t *cist = NULL;
		cist = stp_stpm_get_instance(STP_CIST_ID);
		if (!cist) {
			free(bpdu_packet);
			return STP_ERROR;
		}
		tmp_bpdu_type = (char)(port->owner->ForceVersion < 3 ) ? BPDU_RSTP : BPDU_MSTP;
		if ((BPDU_MSTP == tmp_bpdu_type) &&
			(1 == (port->configDigestSnp & 0x1)) &&
			0 != strlen(cist->digest))
		{
			memcpy(&bpdu_packet->ciscoMSTID.ConfigurationDigest, cist->digest, 16);
			stp_trace("User configuration Digest:");
			stp_transmit_dump_string(bpdu_packet->ciscoMSTID.ConfigurationDigest, 16);
		}	
		#endif

		/* CIST Region Root Identifier */
		stp_vector_set_bridge_id(&port->portPrio.region_root_bridge, bpdu_packet->cist_region_root_id);
		
		(*(long*) bpdu_packet->cist_inter_path_cost) = htonl(port->portPrio.region_root_path_cost );
    
		bpdu_packet->cist_remaining_hops = port->portTimes.RemainingHops;
  
		for( mstport = port->nextMst; mstport; mstport = mstport->nextMst)
		{
			stp_vector_cisco_set_mst_vector(bpdu_packet, index, mstport);
			index ++ ;
		}
  
		version_3_length = 64 + 26 * index ;	
		temp = htons(version_3_length);
		memcpy(bpdu_packet->version_3_length, &temp, 2) ;
	}
  
	bpdu_type = (char ) ( ( port->owner->ForceVersion < 3 ) ? BPDU_RSTP : BPDU_MSTP ) ;
	/* special point at Cisco MSTP:
	 * 1 + 1 + 2 :
	 * ver_1_len(1) + rsvd1(1) + version_3_length(2)
	 */
	pkt_len = ( bpdu_type == BPDU_MSTP ) ? (sizeof (BPDU_HEADER_T) + sizeof (BPDU_BODY_T)  + 1 + 1 + 2 + version_3_length ) : (sizeof (BPDU_HEADER_T) + sizeof (BPDU_BODY_T)	+ 1 );
	stp_transmit_build_bpdu_header ((RSTP_BPDU_T *)bpdu_packet, port->port_index, bpdu_type, pkt_len );
  
#ifdef STP_DBG
	if (this->debug)
	{
		PRIO_VECTOR_T v ;
		memcpy(&v, &port->portPrio, sizeof(PRIO_VECTOR_T));  
		memcpy(&(v.design_bridge), &port->owner->BrId, sizeof(BRIDGE_ID));
		stp_trace ("port %s stp_transmit_txMstp_to_Cisco flags=0X%lx bpdu_len %d role %x",
					port->port_name,
					(unsigned long) bpdu_packet->body.flags, pkt_len, port->role);
		stp_vector_print ("txMtxMstp_to_Ciscostp", &v);
	}
#endif

	iret = stp_to_tx_bpdu (port->port_index, 1,
							(unsigned char *) bpdu_packet,
							pkt_len + sizeof(MAC_HEADER_T) + sizeof(ETH_HEADER_T),
							this->owner.port->isWAN, this->owner.port->slot_no, this->owner.port->port_no);
	/*should notify to npd to syn the arp & fdb*/
	if ((bpdu_packet->body.flags & TOLPLOGY_CHANGE_BIT) ||
		(bpdu_packet->body.flags & TOLPLOGY_CHANGE_ACK_BIT))
	{
		owner = this->owner.port->owner;
		for (ports = owner->ports; ports; ports = ports->next){
			if (ports->portEnabled) {
				stp_syslog_dbg("stp_transmit_txMstp:: port %d flush fdb & arp!\n", ports->port_index);
				sendStpTcnbittoNpd(ports);
			}
		}
	}
	free(bpdu_packet);
	return iret;
}

int	stp_transmit_txMstp (STATE_MACH_T* this)  /* 13.26.22 */
{
  register PORT_T*      port = NULL,*ports = NULL;
  register PORT_T*      mstport = NULL;
  MSTP_BPDU_T	* bpdu_packet = NULL;
  struct stpm_t *owner = NULL;
  unsigned char  bpdu_type = 0;
  unsigned short pkt_len = 0;
  unsigned short version_3_length = 0, temp = 0;
  unsigned short  index = 0;
  int iret = 0;
  unsigned char         role;
  
  
  port = this->owner.port;
  /*1 端口是否参加STP*/
  if (port->admin_non_stp) return 1;
  
  if (port->owner->ForceVersion <= 1)
  {
    return STP_Nothing_To_Do ;
  }	

  /*2 建立并初始化MSTP BPDU包*/
  /* packet is to Cisco? */
  if (port->configDigestSnp & 0x1) {
		iret = stp_transmit_txMstp_to_Cisco(this);
		stp_trace("stp_transmit_txMstp_to_Cisco ret %d", iret);
		return iret;
  }
  
  bpdu_packet = (MSTP_BPDU_T *)malloc(sizeof(MSTP_BPDU_T));
  if (NULL == bpdu_packet)
  {
    return STP_ERROR;
  }
  memset( bpdu_packet , 0, sizeof(MSTP_BPDU_T)) ;
  memcpy( bpdu_packet , &g_bpdu_packet , sizeof (g_bpdu_packet) ) ;
  
  stp_transmit_build_config_bpdu_mstp ( bpdu_packet, port);
  
  switch (port->role) 
  { 
     case MasterPort :
       role = MSTP_PORT_ROLE_MASTER;
       break;
     case AlternatePort:
       role = RSTP_PORT_ROLE_ALTBACK;
       break;
     case BackupPort:
       role = RSTP_PORT_ROLE_ALTBACK;
       break;
     case RootPort:
       role = RSTP_PORT_ROLE_ROOT;
       break;
     case DesignatedPort:
       role = RSTP_PORT_ROLE_DESGN;
       break;
     default :
        role = RSTP_PORT_ROLE_UNKN;
  }
  
  bpdu_packet->body.flags |= (role << PORT_ROLE_OFFS);	
  
  if ( port->owner->ForceVersion >= 3 )
  {
    memcpy( &bpdu_packet->MSTCID , &port->owner->MstConfigId ,sizeof (MST_CFG_ID_S) );
    stp_vector_set_bridge_id( &port->owner->BrId , bpdu_packet->cist_bridge_id);
    (*(long*) bpdu_packet->cist_inter_path_cost) = htonl(port->portPrio.region_root_path_cost );
  
  
    bpdu_packet->cist_remaining_hops = port->portTimes.RemainingHops ;
  
    for( mstport = port->nextMst; mstport; mstport = mstport->nextMst)
    {
      stp_vector_set_mst_vector(bpdu_packet , index, mstport );
      index ++ ;
    }
  
    version_3_length =  64  + 16 * index ;  
    temp = htons(version_3_length);
     memcpy( bpdu_packet->version_3_length, &temp, 2) ;
  }
  
  bpdu_type = (char ) ( ( port->owner->ForceVersion < 3 ) ? BPDU_RSTP : BPDU_MSTP ) ;
  pkt_len = ( bpdu_type == BPDU_MSTP ) ? (sizeof (BPDU_HEADER_T) + sizeof (BPDU_BODY_T)  + 1 + 2 + version_3_length ) : (sizeof (BPDU_HEADER_T) + sizeof (BPDU_BODY_T)  + 1 );
  stp_transmit_build_bpdu_header ((RSTP_BPDU_T *)bpdu_packet, port->port_index, bpdu_type, pkt_len );
  
  #ifdef STP_DBG
  if (this->debug)
  {
    PRIO_VECTOR_T v ;
    memcpy(&v, &port->portPrio, sizeof(PRIO_VECTOR_T));  
    memcpy(&(v.design_bridge), &port->owner->BrId, sizeof(BRIDGE_ID));
    stp_trace ("port %s stp_transmit_txMstp flags=0X%lx bpdu_len %d role %x", port->port_name,
    (unsigned long) bpdu_packet->body.flags, pkt_len, port->role);
    stp_vector_print ("stp_transmit_txMstp", &v);

  }
  #endif
  #if 0
  if(port->transmit->debug)
  {
    int iii =0;
    unsigned char *temp = (unsigned char *)bpdu_packet;
    stp_trace("stp_transmit_txMstp bpdu(len %d ) on %s", pkt_len + sizeof(MAC_HEADER_T) + sizeof(ETH_HEADER_T),  port->port_name);
    for(iii = 0; iii < pkt_len + sizeof(MAC_HEADER_T) + sizeof(ETH_HEADER_T); iii++)
    {
      if((iii % 16) == 0)
        printf("\n");
      printf("%.2x  ", temp[iii]);
    }
    printf("\n");
  }
  #endif 
  iret = stp_to_tx_bpdu ( port->port_index, 1,
                          (unsigned char *) bpdu_packet,
                          pkt_len + sizeof(MAC_HEADER_T) + sizeof(ETH_HEADER_T),
                          this->owner.port->isWAN, this->owner.port->slot_no, this->owner.port->port_no);
  /*should notify to npd to syn the arp & fdb*/
  if((bpdu_packet->body.flags & TOLPLOGY_CHANGE_BIT)||(bpdu_packet->body.flags & TOLPLOGY_CHANGE_ACK_BIT)){
	  owner = this->owner.port->owner;
	  for(ports = owner->ports;ports;ports = ports->next){
	 	if(ports->portEnabled){
			stp_syslog_dbg("stp_transmit_txMstp:: port %d flush fdb & arp!\n",ports->port_index);
			sendStpTcnbittoNpd(ports);
	 	}
     }
  }
  free(bpdu_packet);
  return iret;
}



/* 13.25.3 stp_transmit_cistRootPort */
Bool stp_transmit_cistRootPort( unsigned long port_index )
{
    register PORT_T *cistport = stp_port_mst_findport(STP_CIST_ID, port_index);

    if ( cistport != NULL  )
    {
        if (cistport->role == RootPort)
        {
            return True;
        }
    }
    
    return False;
}



/* 13.25.5 stp_transmit_mstiRootPort */
Bool stp_transmit_mstiRootPort( unsigned long port_index )
{
    PORT_T *port = NULL ;

    port = stp_port_mst_findport(STP_CIST_ID, port_index);
    if(!port)
      return False;
    for ( port = port->nextMst; port; port = port->nextMst) /*可以遍历桥的链表*/
    {
        if (port->role == RootPort)
        {
            return True;
        }
    }
    return False;
}

/* 13.25.4 stp_transmit_cistDesignatedPort */
Bool stp_transmit_cistDesignatedPort( unsigned long port_index )
{
    register PORT_T *cistport = stp_port_mst_findport(STP_CIST_ID, port_index);

    if ( cistport != NULL  )
    {
        if (cistport->role == DesignatedPort)
        {
            return True;
        }
    }
    
    return False;
}
 
/* 13.25.6 stp_transmit_mstiDesignatedPort */
Bool stp_transmit_mstiDesignatedPort( unsigned long port_index )
{
    PORT_T *port = NULL ;

    port = stp_port_mst_findport(STP_CIST_ID, port_index);
    if(!port)
      return False;
    for ( port = port->nextMst ; port; port = port->nextMst) /*可以遍历桥的链表*/
    {
        if (port->role == DesignatedPort )
        {
            return True;
        }
    }
    return False;
}
Bool stp_transmit_update_newInfoCist( unsigned long port_index )
{
    register PORT_T *cistport = stp_port_mst_findport(STP_CIST_ID, port_index); ;

    if ( cistport != NULL  )
    {
        return cistport->newInfoCist ||
                ((cistport->role == DesignatedPort)||
                ( cistport->role  == RootPort && cistport->tcWhile != 0));
    }
    
    return False;
}

Bool stp_transmit_update_newInfoMsti( unsigned long port_index )
{
    PORT_T *port = NULL ;

    port = stp_port_mst_findport(STP_CIST_ID, port_index);
    if(!port)
      return False;
    for ( port = port->nextMst; port; port = port->nextMst) /*可以遍历桥的链表*/
    {
        if (port->newInfoMsti ||
              ((port->role  == DesignatedPort)||
                ( port->role  == RootPort && port->tcWhile != 0)))
        {
            return True;
        }
    }
    return False;
}

static Bool  stp_transmit_ifselectedAndnotupdtInfoInAnyMstPort( unsigned long port_index )
{
    PORT_T *port = NULL ;

    port = stp_port_mst_findport(STP_CIST_ID, port_index);

    for ( ; port; port = port->nextMst) /*可以遍历桥的链表*/
    {
        if (port->selected && !port->updtInfo)
        {
            return True;
        }
    }

    return False;
}
void
MSTP_transmit_enter_state (STATE_MACH_T* this)
{
  register PORT_T*     port = this->owner.port;

  switch (this->State) {
    case BEGIN:
    case TRANSMIT_INIT:
      port->newInfoCist = port->newInfoMsti = False;
      /*port->newInfo = False;*/
      port->helloWhen = 0;
      port->txCount = 0;
      break;
    case TRANSMIT_PERIODIC:
      port->newInfoCist = stp_transmit_update_newInfoCist(port->port_index);
      port->newInfoMsti = stp_transmit_update_newInfoMsti(port->port_index);
      /*port->newInfo = port->newInfo ||
                            ((port->role == DesignatedPort) ||
                             ((port->role == RootPort) && port->tcWhile));*/
      port->helloWhen = port->owner->rootTimes.HelloTime;
      break;
    case IDLE:
      break;
    case TRANSMIT_CONFIG:
      port->newInfoCist = port->newInfoMsti = False;
      /*port->newInfo = False;*/
      stp_transmit_txConfig (this);
      port->txCount++;
      port->tcAck = False;
      break;
    case TRANSMIT_TCN:
      port->newInfoCist = port->newInfoMsti = False;
      /*port->newInfo = False;*/
      stp_transmit_txTcn (this);
      port->txCount++;
      break;
    case TRANSMIT_RSTP:
      port->newInfoCist = port->newInfoMsti = False;
      /*port->newInfo = False;*/
      stp_transmit_txMstp (this);
      port->txCount++;
      port->tcAck = False;
      break;
  };
}
  
Bool
MSTP_transmit_check_conditions (STATE_MACH_T* this)
{
  register PORT_T*     port = this->owner.port;

  if (BEGIN == this->State) return stp_statmch_hop_2_state (this, TRANSMIT_INIT);

  switch (this->State) {
    case TRANSMIT_INIT:
      return stp_statmch_hop_2_state (this, IDLE);
    case TRANSMIT_PERIODIC:
      return stp_statmch_hop_2_state (this, IDLE);
    case IDLE:
#if 0
      if (!port->helloWhen) return stp_statmch_hop_2_state (this, TRANSMIT_PERIODIC);
      if (!port->sendRSTP && port->newInfo &&
          (port->txCount < TxHoldCount) &&
          (port->role == DesignatedPort) &&
          port->helloWhen)
        return stp_statmch_hop_2_state (this, TRANSMIT_CONFIG);
      if (!port->sendRSTP && port->newInfo &&
          (port->txCount < TxHoldCount) &&
          (port->role == RootPort) &&
          port->helloWhen)
        return stp_statmch_hop_2_state (this, TRANSMIT_TCN);
      if (port->sendRSTP && port->newInfo &&
          (port->txCount < TxHoldCount) &&
          ((port->role == RootPort) ||
           (port->role == DesignatedPort)))
        return stp_statmch_hop_2_state (this, TRANSMIT_RSTP);
#endif
        if (!stp_transmit_ifselectedAndnotupdtInfoInAnyMstPort(port->port_index))
          break;

      	 if ( port->adminEnable== STP_DISABLED || port->admin_non_stp )
      	 {
      	    break;
      	 }

        if ( !port->helloWhen )
      	   return stp_statmch_hop_2_state (this, TRANSMIT_PERIODIC);
      	
        if (!port->sendRSTP && port->newInfoCist &&
              (port->txCount < TxHoldCount)&&
              (stp_transmit_cistDesignatedPort(port->port_index)) &&
              port->helloWhen != 0 )
            return stp_statmch_hop_2_state (this, TRANSMIT_CONFIG);

         if (!port->sendRSTP && port->newInfoCist &&
              (port->txCount < TxHoldCount)  &&
              (stp_transmit_cistRootPort(port->port_index)) && port->tcWhile &&
              port->helloWhen != 0 )
            return stp_statmch_hop_2_state (this, TRANSMIT_TCN);
              
         if (  port->sendRSTP && (port->txCount < TxHoldCount)&& (port->helloWhen !=0)  &&
                   ((port->newInfoCist && ( stp_transmit_cistRootPort(port->port_index) || stp_transmit_cistDesignatedPort(port->port_index) ))
                     ||(port->newInfoMsti && ( stp_transmit_mstiRootPort(port->port_index) || stp_transmit_mstiDesignatedPort(port->port_index) ))
                    ) 
             ) 
            {
                return stp_statmch_hop_2_state (this, TRANSMIT_RSTP);
            }        
      break;
    case TRANSMIT_CONFIG:
      return stp_statmch_hop_2_state (this, IDLE);
    case TRANSMIT_TCN:
      return stp_statmch_hop_2_state (this, IDLE);
    case TRANSMIT_RSTP:
      return stp_statmch_hop_2_state (this, IDLE);
  };
  return False;
}
#ifdef __cplusplus
}
#endif

