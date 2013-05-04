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
* stp_receive.c
*
* CREATOR:
*       wangxiangfeng@autelan.com
*
* DESCRIPTION:
*       APIs for packet receive op in stp module
*
* DATE:
*       04/18/2008
*
*  FILE REVISION NUMBER:
*       $Revision: 1.1.1.1 $
*******************************************************************************/
#ifdef __cplusplus
extern "C" {
#endif
#include <string.h>
#include <ctype.h>
#include "stp_base.h"
#include "stp_port.h"
#include "stp_stpm.h"
#include "stp_in.h"

#define STATES { \
  CHOOSE(INIT),             \
  CHOOSE(DISCARD),         \
  CHOOSE(RECEIVE),         \
}

#define GET_STATE_NAME STP_receive_get_state_name
#include "stp_choose.h"
#include "stp_vector.h"
        
Bool stp_receive_mstpVendorCisco
(
	char *mac
) 
{
	int result = 0;
    BPDU_T  *bpdu ;
    int            bpdu_type = -1;
	unsigned char digestSnp = 0;
	#ifdef VENDOR_CISCO_CHECK_MAC /* currently not check mac to identify peer vendor Cisco */
	static unsigned char cisco_vendor[] = {0x00, 0x0d, 0xbd};

	if(!mac) {
		return False;
	}
	result = memcmp(tolower(mac), cisco_vendor, 3);
	return (result ? False : True);
	#else
    if (!stp_in_get_cur_bpdu(&bpdu, &bpdu_type, &digestSnp)){
        return False ;
    }

	return (digestSnp ? True : False);
	#endif
}

Bool stp_receive_fromSameRegion(STATE_MACH_T* this )   /* 13.26.5 */
{
    register PORT_T*      port = this->owner.port;
    BPDU_T  *bpdu ;
    int            bpdu_type;
	unsigned char digestSnp = 0;

    if (!stp_in_get_cur_bpdu(&bpdu, &bpdu_type, &digestSnp))
    {
        return False ;
    }
    
    if ( port->rcvdRSTP == True && 
			bpdu_type == BPDU_MSTP  &&
         		port->owner->ForceVersion >= 3 ) 
    {
		
        #ifdef STP_DBG
          if ( this->debug )        
              stp_trace( "Receive : stp_receive_fromSameRegion True");
        #endif
		#if 0
		if(stp_receive_mstpVendorCisco(bpdu->mac.src_mac)) {
		#endif
		#if 1
		if (port->configDigestSnp & 0x1) {
		#endif
			stp_trace("mstp packet from vendor Cisco, view as same region partner!\n");
			return True;
		}
		else if(0 == memcmp(&((MSTP_BPDU_T *)bpdu)->MSTCID , &port->owner->MstConfigId , sizeof(MST_CFG_ID_S))){
        	return True ;
		}
    }

    #ifdef STP_DBG
    if ( this->debug )
    {
          stp_trace( "Receive : stp_receive_fromSameRegion False ");
    }
    #endif
        
    return False ;
}


Bool stp_receive_rcvdAnyMsg(STATE_MACH_T* this)  /* 13.25.7 */
{
   register PORT_T*      port = this->owner.port;
  
   for( ; port; port  = port->nextMst )
   {
       if ( port->rcvdMsg == True)
       {
           return True;
       }    
   }

   return False ;
}

void stp_receive_clearAllRcvdMsgs(void)  /* 13.26.3 */
{
    register PORT_T*  port;
    register STPM_T*  stpm;

    for(stpm = stp_stpm_get_the_list (); stpm; stpm = stpm->next)
    {
        for(port = stpm->ports; port; port = port->next)
            port->rcvdMsg = False ;
    }
    
    return ;
}

static void stp_receive_setRcvdMsgs_Cisco
(
	STATE_MACH_T *this
)  /* 13.26.15 */
{
	register PORT_T *port = NULL;
	register PORT_T *mstport = NULL; 
	register int index = 0;
	unsigned short mstid = 0;
	MSTP_Cisco_BPDU_T *bpdu = NULL ;
	int bpdu_type = 0;
	unsigned char digestSnp = 0;

	port = this->owner.port;

    if (!stp_in_get_cur_bpdu((BPDU_T *)&bpdu, &bpdu_type, &digestSnp))
    {
        return ;
    }
   
    port->rcvdMsg = True ; 

	/* save digest to port */
	if (bpdu_type == BPDU_MSTP)	{
		memcpy(port->digest, bpdu->ciscoMSTID.ConfigurationDigest, 16);
		stp_transmit_dump_string(port->digest, 16);
		stp_trace ("get digest form packet.");
	}

    /* 13.24.6 cistMsgPriority */
    stp_vector_get_bridge_id ((unsigned char*) bpdu->body.root_id , &port->msgPrio.root_bridge);
    port->msgPrio.root_path_cost = ntohl (*((long*) bpdu->body.root_path_cost));

    stp_vector_get_bridge_id ((unsigned char*) bpdu->cist_region_root_id, &port->msgPrio.region_root_bridge );
    port->msgPrio.design_port = ntohs (*(unsigned short *)bpdu->body.port_id);
    port->msgPrio.bridge_port = port->port_id;

    if ( bpdu_type == BPDU_MSTP )
    {
		stp_vector_get_bridge_id ((unsigned char*) bpdu->body.bridge_id , &port->msgPrio.design_bridge);
        if ( port->rcvdInternal )
        {
           port->msgPrio.region_root_path_cost = ntohl (*((long*) &bpdu->cist_inter_path_cost));
        }
        else
        {
            port->msgPrio.region_root_path_cost = 0;
        }
    }
    else
    {
        stp_vector_get_bridge_id ((unsigned char*) bpdu->body.bridge_id , &port->msgPrio.design_bridge );
        port->msgPrio.region_root_path_cost = 0 ;
    }

    #ifdef STP_DBG
      if ( this->debug )
      {
          stp_trace ("(%s-%s) setRcvdMsgs msgPrio :\n",
            port->owner->name, port->port_name);
          
          stp_vector_print("",&port->msgPrio);    
      }
    #endif
    
    /* 13.24.7 cistMsgTimes */
    stp_times_get ((BPDU_BODY_T *)&bpdu->body, &port->msgTimes);
    if ( bpdu_type == BPDU_MSTP )
    {
        port->msgTimes.RemainingHops = bpdu->cist_remaining_hops ;
    }
    else
    {
        port->msgTimes.RemainingHops = 0 ;
    }
    
    if ( port->rcvdInternal   ) /* Msti */
    {
        unsigned short version_3_length = 0 ;
        unsigned short MstCnt;

        version_3_length = ntohs (*(unsigned short*)(bpdu->version_3_length)) ;

        MstCnt = (version_3_length - 64 ) / 26;			/* 26 bytes in Cisco packet */
        for ( index = 0 ; index < MstCnt ; index ++ )
        {
            unsigned short * pPrio = NULL ;
            pPrio = (unsigned short*)bpdu->MCfgMsg[index].msti_region_root_id;
            mstid =  ntohs(*pPrio) & 0x0fff ;
            mstport = stp_port_mst_findport(mstid, port->port_index);
            if (mstport != NULL )
            {
                mstport->rcvdMsg = True ;
                mstport->msgBpduType  = port->msgBpduType ;

                stp_vector_get_mst_vector(bpdu, index, 1,&mstport->msgPrio); 
                mstport->msgFlags = bpdu->MCfgMsg[index].msti_flags;
                mstport->msgPrio.bridge_port = mstport->port_id ;
                mstport->msgPrio.design_bridge.prio += ( 0x0fff & mstid ) ;
                
                stp_vector_get_mst_timers(bpdu, index, &mstport->msgTimes );
                
               #ifdef STP_DBG
                  if ( this->debug )
                  {
	               stp_trace ("RX msti %d config message " , mstid );
                	  stp_vector_br_id_print ("            region root br id", &mstport->msgPrio.region_root_bridge, False);
                	  stp_trace ("            region root path cost = %ld ", (long) mstport->msgPrio.region_root_path_cost);
                  }
               #endif

            }
        }
    }
    
    return  ;
}

static void stp_receive_setRcvdMsgs(STATE_MACH_T* this)  /* 13.26.15 */
{
    register PORT_T*      port = this->owner.port;
    register PORT_T*      mstport;
    register int index = 0;
    unsigned short mstid = 0;
    MSTP_BPDU_T  *bpdu ;
    int            bpdu_type;
	unsigned char digestSnp = 0;

    if (!stp_in_get_cur_bpdu((BPDU_T *)&bpdu, &bpdu_type, &digestSnp))
    {
        return ;
    }
   
    port->rcvdMsg = True ; 

	if (port->configDigestSnp & 0x1) {
		stp_receive_setRcvdMsgs_Cisco(this);
		return;
	}

    /* 13.24.6 cistMsgPriority */
    stp_vector_get_bridge_id ((unsigned char*) bpdu->body.root_id , &port->msgPrio.root_bridge);
    port->msgPrio.root_path_cost = ntohl (*((long*) bpdu->body.root_path_cost));
    
    stp_vector_get_bridge_id ((unsigned char*) bpdu->body.bridge_id , &port->msgPrio.region_root_bridge );
    port->msgPrio.design_port = ntohs (*(unsigned short *)bpdu->body.port_id);
    port->msgPrio.bridge_port = port->port_id;

    if ( bpdu_type == BPDU_MSTP )
    {
        stp_vector_get_bridge_id ((unsigned char*) bpdu->cist_bridge_id , &port->msgPrio.design_bridge);
        if ( port->rcvdInternal )
        {
           port->msgPrio.region_root_path_cost = ntohl (*((long*) &bpdu->cist_inter_path_cost));
        }
        else
        {
            port->msgPrio.region_root_path_cost = 0;
        }
    }
    else
    {
        stp_vector_get_bridge_id ((unsigned char*) bpdu->body.bridge_id , &port->msgPrio.design_bridge );
        port->msgPrio.region_root_path_cost = 0 ;
    }

    #ifdef STP_DBG
      if ( this->debug )
      {
          stp_trace ("(%s-%s) setRcvdMsgs msgPrio :\n",
            port->owner->name, port->port_name);
          
          stp_vector_print("",&port->msgPrio);    
      }
    #endif
    
    /* 13.24.7 cistMsgTimes */
    stp_times_get ((BPDU_BODY_T *)&bpdu->body, &port->msgTimes);
    if ( bpdu_type == BPDU_MSTP )
    {
        port->msgTimes.RemainingHops = bpdu->cist_remaining_hops ;
    }
    else
    {
        port->msgTimes.RemainingHops = 0 ;
    }
    
    if ( port->rcvdInternal   ) /* Msti */
    {
        unsigned short version_3_length = 0 ;
        unsigned short MstCnt;

        version_3_length = ntohs (*(unsigned short*)(bpdu->version_3_length)) ;

        MstCnt = (version_3_length - 64 ) / 16;
        for ( index = 0 ; index < MstCnt ; index ++ )
        {
            unsigned short * pPrio = NULL ;
            pPrio = (unsigned short*)bpdu->MCfgMsg[index].msti_region_root_id;
            mstid =  ntohs(*pPrio) & 0x0fff ;
            mstport = stp_port_mst_findport(mstid, port->port_index);
            if (mstport != NULL )
            {
                mstport->rcvdMsg = True ;
                mstport->msgBpduType  = port->msgBpduType ;

                stp_vector_get_mst_vector(bpdu, index, 0, &mstport->msgPrio); 
                mstport->msgFlags = bpdu->MCfgMsg[index].msti_flags;
                mstport->msgPrio.bridge_port = mstport->port_id ;
                mstport->msgPrio.design_bridge.prio += ( 0x0fff & mstid ) ;
                
                stp_vector_get_mst_timers(bpdu, index, &mstport->msgTimes );
                
               #ifdef STP_DBG
                  if ( this->debug )
                  {
	               stp_trace ("RX msti %d config message " , mstid );
                	  stp_vector_br_id_print ("            region root br id", &mstport->msgPrio.region_root_bridge, False);
                	  stp_trace ("            region root path cost = %ld ", (long) mstport->msgPrio.region_root_path_cost);
                  }
               #endif

            }
        }
    }
    
    return  ;
}


static Bool stp_receive_setTcFlags(STATE_MACH_T* this) /* 13.26.19 */
{
    register PORT_T* port = this->owner.port;
    register PORT_T* mstport = port->nextMst ;

    if (BPDU_TOPO_CHANGE_TYPE == port->msgBpduType) 
    {
#ifdef STP_DBG
      if (this->debug) {
        stp_trace ("port %s rx rcvdTcn", port->port_name);
      }
#endif    

        port->rcvdTcn = True;
        for(; mstport; mstport = mstport->nextMst) /* for each and every MSTI. */
        {
            mstport->rcvdTc = True;            
        }
    }
    
    else if ( BPDU_CONFIG_TYPE == port->msgBpduType ||
                BPDU_RSTP == port->msgBpduType )
    {
        if (TOLPLOGY_CHANGE_ACK_BIT & port->msgFlags)
        {
            #ifdef STP_DBG
              if (this->debug) {
                stp_trace ("(%s-%s) rx rcvdTc 0X%lx",
                    port->owner->name, port->port_name,
                    (unsigned long) port->msgFlags);
              }
            #endif        
            
            port->rcvdTcAck = True;
        }

        if ( port->rcvdInternal == False)
        {
            if ( TOLPLOGY_CHANGE_BIT & port->msgFlags) 
            {
                for( ; mstport; mstport = mstport->nextMst)
                {
                    mstport->rcvdTc = True ;
                }
            }
        }
	 else if ( port->rcvdInternal == True )
        {
            for( ; mstport; mstport = mstport->nextMst)
            {
                if ( (mstport->rcvdMsg) && (TOLPLOGY_CHANGE_BIT & mstport->msgFlags))
                {
                     mstport->rcvdTc = True ; 
                }
            }
        }
      
    }
  
   return True;
}

Bool stp_receive_updtBPDUVersion (STATE_MACH_T* this)
{/* 17.19.18 */
  register PORT_T* port = this->owner.port;

  if (BPDU_TOPO_CHANGE_TYPE == port->msgBpduType) {
    port->rcvdSTP = True;
  }

  if (port->msgBpduVersion < 2) {
    port->rcvdSTP = True;
  }
  
  if (BPDU_RSTP == port->msgBpduType) {
    port->rcvdRSTP = True;
  }

  return True;
}

void MSTP_receive_enter_state (STATE_MACH_T* this)
{
  register PORT_T*      port = this->owner.port;

  switch (this->State) {
    case BEGIN:
        break;
    case DISCARD:
        port->rcvdBpdu = port->rcvdRSTP = port->rcvdSTP = False ;
        stp_receive_clearAllRcvdMsgs();
        break;
    case RECEIVE:
        stp_receive_updtBPDUVersion(this);
        port->rcvdInternal = stp_receive_fromSameRegion(this );
        stp_receive_setRcvdMsgs(this);
        stp_receive_setTcFlags(this);
        port->rcvdBpdu = False ;
        break; 
    default:
       break;
  };
}

Bool MSTP_receive_check_conditions (STATE_MACH_T* this)
{
  register PORT_T*      port = this->owner.port;

  switch (this->State) 
  {
      case BEGIN:
        return stp_statmch_hop_2_state (this, DISCARD);
      case DISCARD:
        if (port->rcvdBpdu && !port->portEnabled )
        {
            return stp_statmch_hop_2_state (this, DISCARD);
        }

        if ( port->rcvdBpdu && port->portEnabled )
        {
            return stp_statmch_hop_2_state (this, RECEIVE);
        }
        break;/*pclint*/
        
      case RECEIVE:
        if (port->rcvdBpdu && !port->portEnabled )
        {
            return stp_statmch_hop_2_state (this, DISCARD);
        }

        if ( port->rcvdBpdu && port->portEnabled && !(stp_receive_rcvdAnyMsg(this)) )
        {
            return stp_statmch_hop_2_state (this, RECEIVE);
        }
        break;

      default:
        break;
  }
  
  return False;
}


#ifdef __cplusplus
}
#endif


