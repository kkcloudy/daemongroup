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
* stp_portinfo.c
*
* CREATOR:
*       zhubo@autelan.com
*
* DESCRIPTION:
*       APIs for Port info in stp module
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

#include "stp_base.h"
#include "stp_port.h"
#include "stp_stpm.h"
#include "stp_in.h"
#include "stp_log.h"

/* The Port Information State Machine : 17.21 */

#define STATES { \
  CHOOSE(DISABLED), \
  CHOOSE(ENABLED),  \
  CHOOSE(AGED),     \
  CHOOSE(UPDATE),   \
  CHOOSE(CURRENT),  \
  CHOOSE(RECEIVE),  \
  CHOOSE(SUPERIOR_DESIGNATED),  /*mstp*/\
  CHOOSE(REPEATED_DESIGNATED),   /*mstp*/\
  CHOOSE(ROOT),     /*mstp*/ \
  CHOOSE(OTHER) 	/*mstp*/   \
}
/* 
  CHOOSE(REPEAT),
  CHOOSE(AGREEMENT),*/

#define GET_STATE_NAME STP_info_get_state_name
#include "stp_choose.h"

#if 0 /* for debug */
void
_stp_dump (char* title, unsigned char* buff, int len)
{
  register int iii;

  printf ("\n%s:", title);
  for (iii = 0; iii < len; iii++) {
    if (! (iii % 24)) Print ("\n%6d:", iii);
    if (! (iii % 8)) Print (" ");
    Print ("%02lx", (unsigned long) buff[iii]);
  }
  Print ("\n");
}

static void _stp_check_infoIs_Received(PORT_T * port )
{
}

#endif

/*char*
STP_info_get_state_name (int state)
{
	//return GET_STATE_NAME (state);
}*/
/*mstp*/

static void stp_portinfo_updt_recvmsg_timer( PORT_T * port , TIMEVALUES_T * v )
{
    register int eff_age;

    memcpy(v, &port->msgTimes, sizeof(TIMEVALUES_T));
	
    if (  port->vlan_id == STP_CIST_ID)   /* CIST */
    {
        if ( port->rcvdInternal == False )
        {
            eff_age = ( 8 + port->portTimes.MaxAge ) / 16;

            if ( eff_age < 1 )
                eff_age = 1;
            eff_age += port->msgTimes.MessageAge;

            v->MessageAge = eff_age ; 
            v->RemainingHops = port->owner->BrTimes.RemainingHops ;

        }
        else 
        {
            v->MessageAge = port->msgTimes.MessageAge ;
            v->RemainingHops = port->msgTimes.RemainingHops - 1 ;
        }
    }
    else /* MSTI */
    {
        v->RemainingHops = port->msgTimes.RemainingHops - 1 ;
    }

    return ;
}
/*mstp*/
#if 0
static RCVD_MSG_T
rcvBpdu (STATE_MACH_T* this)
{/* 17.19.8 */
  int   bridcmp;
  register PORT_T* port = this->owner.port;

  if (port->msgBpduType == BPDU_TOPO_CHANGE_TYPE) {
#ifdef STP_DBG
    if (this->debug) {
        stp_trace ("%s", "OtherMsg:BPDU_TOPO_CHANGE_TYPE");
    }
#endif
    return OtherMsg;
  }

  port->msgPortRole = RSTP_PORT_ROLE_UNKN;

  if (BPDU_RSTP == port->msgBpduType) {

#ifdef STP_DBG
    if (this->debug) {
        stp_trace ("port->msgBpduType = %s", "BPDU_RSTP");
    }
#endif
    port->msgPortRole = (port->msgFlags & PORT_ROLE_MASK) >> PORT_ROLE_OFFS;
  }

#ifdef STP_DBG
 	STP_LOG(STP_LOG_DEBUG,"\n");
	stp_vector_br_id_print ("ch:msgPrio.root_bridge",
		&port->msgPrio.root_bridge, True);
	stp_vector_br_id_print ("ch:msgPrio.design_bridge",
		&port->msgPrio.design_bridge, True);
	STP_LOG(STP_LOG_DEBUG,"RSTP>>port->msgPrio.root_path_cost = %d \n  \
		port->msgPrio.design_port = %d \n \
		port->msgPrio.bridge_port = %d \n",  \
		port->msgPrio.root_path_cost,port->msgPrio.design_port,port->msgPrio.bridge_port);

	STP_LOG(STP_LOG_DEBUG,"\n");
	stp_vector_br_id_print ("ch:portPrio.root_bridge",
		&port->portPrio.root_bridge, True);	
	stp_vector_br_id_print ("ch:portPrio.design_bridge",
		&port->portPrio.design_bridge, True);
	STP_LOG(STP_LOG_DEBUG,"RSTP>>port->portPrio.root_path_cost = %d \n  \
		port->portPrio.design_port = %d \n \
		port->portPrio.bridge_port = %d \n",  \
		port->portPrio.root_path_cost,port->portPrio.design_port,port->portPrio.bridge_port);
#endif

  if (RSTP_PORT_ROLE_DESGN == port->msgPortRole ||
      BPDU_CONFIG_TYPE == port->msgBpduType) {
			
    bridcmp = stp_vector_compare (&port->msgPrio, &port->portPrio);

    STP_LOG (STP_LOG_DEBUG,"RSTP>>INTO Correct;bridcmp = %d \n ", \
					bridcmp);
	
    if (bridcmp < 0 ||
        (! stp_vector_compare_bridge_id (&port->msgPrio.design_bridge,
                                       &port->portPrio.design_bridge) &&
         port->msgPrio.design_port == port->portPrio.design_port      &&
         stp_times_compare (&port->msgTimes, &port->portTimes))) {
#ifdef STP_DBG
         if (this->debug) {
           stp_trace ("SuperiorDesignateMsg:bridcmp=%d", (int) bridcmp);
         }
#endif
      return SuperiorDesignateMsg;
    }
  }

  if (BPDU_CONFIG_TYPE == port->msgBpduType ||
      RSTP_PORT_ROLE_DESGN == port->msgPortRole) {
    if (! stp_vector_compare (&port->msgPrio,
                                   &port->portPrio) &&
        ! stp_times_compare (&port->msgTimes, &port->portTimes)) {
#ifdef STP_DBG
        if (this->debug) {
          stp_trace ("%s", "RepeatedDesignateMsg");
        }
#endif
        return RepeatedDesignateMsg;
    }
  }

  if (RSTP_PORT_ROLE_ROOT == port->msgBpduType                    &&
      port->operPointToPointMac                                   &&
      ! stp_vector_compare_bridge_id (&port->msgPrio.design_bridge,
                                    &port->portPrio.design_bridge) &&
      AGREEMENT_BIT & port->msgFlags) {
#ifdef STP_DBG
    if (this->debug) {
      stp_trace ("%s", "ConfirmedRootMsg");
    }
#endif
    return ConfirmedRootMsg;
  }
  
#ifdef STP_DBG
    if (this->debug) {
      stp_trace ("%s", "OtherMsg");
    }
#endif
  return OtherMsg;
}
#endif
/*13.26.7*/
inline static RCVD_MSG_T stp_portinfo_cist_rcvinfo ( STATE_MACH_T * this )  
{
  int   bridcmp, timecmp;
  register PORT_T* port = this->owner.port;
  TIMEVALUES_T msgtimer ;
  
  if (port->msgBpduType == BPDU_TOPO_CHANGE_TYPE) {
#ifdef STP_DBG
    if (this->debug) {
        stp_trace ("%s", "OtherMsg:BPDU_TOPO_CHANGE_TYPE");
    }
#endif
    return OtherMsg;
  }

  port->msgPortRole = RSTP_PORT_ROLE_UNKN;

  if (BPDU_RSTP == port->msgBpduType) {
    port->msgPortRole = (port->msgFlags & PORT_ROLE_MASK) >> PORT_ROLE_OFFS;
  }

  
  bridcmp = stp_vector_compare (&port->msgPrio, &port->portPrio);
  stp_portinfo_updt_recvmsg_timer(port, &msgtimer);
  timecmp = stp_times_compare (&msgtimer, &port->portTimes);
  if (RSTP_PORT_ROLE_DESGN == port->msgPortRole ||
      BPDU_CONFIG_TYPE == port->msgBpduType) {

    if (bridcmp < 0 ||
        (! stp_vector_compare_bridge_id (&port->msgPrio.design_bridge,
                                       &port->portPrio.design_bridge) &&
         port->msgPrio.design_port == port->portPrio.design_port &&
         timecmp)) {
#ifdef STP_DBG
         if (this->debug) {
           stp_trace ("SuperiorDesignateMsg:bridcmp=%d timecmp=%d",  bridcmp, timecmp);
         }
#endif
      return SuperiorDesignateMsg;
    }
  }

  if (BPDU_CONFIG_TYPE == port->msgBpduType ||
      RSTP_PORT_ROLE_DESGN == port->msgPortRole) {
    if (! bridcmp &&  ! timecmp) {
#ifdef STP_DBG
        if (this->debug) {
          stp_trace ("%s", "RepeatedDesignateMsg");
        }
#endif
        return RepeatedDesignateMsg;
    }
  }

  if (RSTP_PORT_ROLE_ROOT == port->msgPortRole  &&
          bridcmp >= 0) {
#ifdef STP_DBG
    if (this->debug) {
      stp_trace ("%s", "ConfirmedRootMsg");
    }
#endif
    return ConfirmedRootMsg;
  }
  
#ifdef STP_DBG
    if (this->debug) {
      stp_trace ("%s", "OtherMsg");
    }
#endif
  return OtherMsg;
}

/*13.26.8*/
inline static RCVD_MSG_T stp_portinfo_msti_rcvinfo( STATE_MACH_T * this )  
{
  int   bridcmp, timecmp;
  register PORT_T* port = this->owner.port;
  TIMEVALUES_T msgtimer ;

  if (port->msgBpduType == BPDU_TOPO_CHANGE_TYPE) {
#ifdef STP_DBG
    if (this->debug) {
        stp_trace ("%s", "OtherMsg:BPDU_TOPO_CHANGE_TYPE");
    }
#endif
    return OtherMsg;
  }

  port->msgPortRole = RSTP_PORT_ROLE_UNKN;

  if (BPDU_RSTP == port->msgBpduType) {
    port->msgPortRole = (port->msgFlags & PORT_ROLE_MASK) >> PORT_ROLE_OFFS;
  }

  bridcmp = stp_vector_compare (&port->msgPrio, &port->portPrio);
  stp_portinfo_updt_recvmsg_timer(port, &msgtimer);
  timecmp = stp_times_compare (&msgtimer, &port->portTimes);
  if (RSTP_PORT_ROLE_DESGN == port->msgPortRole ) {

    if (bridcmp < 0 ||
        (! stp_vector_compare_bridge_id (&port->msgPrio.design_bridge,
                                       &port->portPrio.design_bridge) &&
         port->msgPrio.design_port == port->portPrio.design_port &&
         timecmp)) {
#ifdef STP_DBG
         if (this->debug) {
           stp_trace ("SuperiorDesignateMsg:bridcmp=%d", (int) bridcmp);
         }
#endif
      return SuperiorDesignateMsg;
    }
  }

  if (RSTP_PORT_ROLE_DESGN == port->msgPortRole) {
    if (! bridcmp &&  ! timecmp) {
#ifdef STP_DBG
        if (this->debug) {
          stp_trace ("%s", "RepeatedDesignateMsg");
        }
#endif
        return RepeatedDesignateMsg;
    }
  }

  if (RSTP_PORT_ROLE_ROOT == port->msgPortRole  &&
          bridcmp >= 0) {
#ifdef STP_DBG
    if (this->debug) {
      stp_trace ("%s", "ConfirmedRootMsg");
    }
#endif
    return ConfirmedRootMsg;
  }
  
#ifdef STP_DBG
    if (this->debug) {
      stp_trace ("%s", "OtherMsg");
    }
#endif
  return OtherMsg;}

static RCVD_MSG_T stp_portinfo_xst_rcvinfo ( STATE_MACH_T * this )  
{
    PORT_T* port = this->owner.port;

    if(port->vlan_id == STP_CIST_ID)
	return stp_portinfo_cist_rcvinfo( this);
  
    return stp_portinfo_msti_rcvinfo( this);
}

inline static void stp_portinfo_record_masteredcist( STATE_MACH_T * this )  /* 13.26.11*/
{
    register PORT_T * port = this->owner.port;
	
    if ( !port->rcvdInternal )
    {
        port = port->nextMst;
        for(  ; port; port = port->nextMst )  /* for all MSTIs. */
        {
            port->mstiMastered = False ;
        }
    }
    return ;
}


inline static void stp_portinfo_record_masteredmsti( STATE_MACH_T * this )  /*13.26.12 */
{
    register PORT_T * port = this->owner.port;
	
    if ( port->operPointToPointMac &&
                ( port->msgFlags & MASTER_BIT ) )
    {
        port->mstiMastered = True ;
    }
    else
    {
        port->mstiMastered = False ;
    }
    return ;
}

static void stp_portinfo_record_masteredxst( STATE_MACH_T * this )  
{
    PORT_T* port = this->owner.port;

    if(port->vlan_id == STP_CIST_ID)
	return stp_portinfo_record_masteredcist( this);
  
    return stp_portinfo_record_masteredmsti( this);
}

inline static void stp_portinfo_record_proposalcist( STATE_MACH_T * this )  /* 13.26.13*/
{
    register PORT_T * port = this->owner.port;
    register PORT_T* mstport = NULL ;


    if ( RSTP_PORT_ROLE_DESGN == port->msgPortRole &&
	    ( PROPOSAL_BIT & port->msgFlags ) && port->operPointToPointMac )
    {
        port->proposed = True ;
    }
    else
    {
        port->proposed = False ;
    }

    if ( !port->rcvdInternal )
    {
        mstport = port->nextMst;
        for(  ; mstport; mstport = mstport->nextMst )  /* for all MSTIs. */
        {
            mstport->proposed = port->proposed ;
        }
    }

    return ;
}



inline static void stp_portinfo_record_proposalmsti( STATE_MACH_T * this)  /* 13.26.13*/
{
    register PORT_T * port = this->owner.port;


    if ( ( PROPOSAL_BIT & port->msgFlags ) && port->operPointToPointMac )
    {
        port->proposed = True ;
    }
    else
    {
        port->proposed = False ;
    }

    return ;
}

static void stp_portinfo_record_proposalxst( STATE_MACH_T * this)  /* 13.26.13*/
{
    PORT_T* port = this->owner.port;

    if(port->vlan_id == STP_CIST_ID)
	return stp_portinfo_record_proposalcist( this);
  
    return stp_portinfo_record_proposalmsti( this);
}


static Bool stp_portinfo_xstinfo_check( STATE_MACH_T * this)  /* 13.16.1 , 13.26.2 */
{
    register PORT_T * port = this->owner.port;

    return ( stp_vector_compare( &port->msgPrio, &port->portPrio ) <= 0 ? True : False ) ;
}

inline static void stp_portinfo_record_agreementcist( STATE_MACH_T * this )  /* 13.26.9 */
{
    register PORT_T * port = this->owner.port;
    register int bridcmp = 0;

    if ( AGREEMENT_BIT & port->msgFlags && port->operPointToPointMac )
    {
        bridcmp = ( stp_vector_compare( &port->msgPrio, &port->portPrio ) <= 0 ) ;
        if ( ( ( ( RSTP_PORT_ROLE_ROOT << PORT_ROLE_OFFS ) & port->msgFlags ) && bridcmp <= 0 ) ||
                ( ( ( RSTP_PORT_ROLE_DESGN << PORT_ROLE_OFFS ) & port->msgFlags ) && bridcmp >= 0 )
           )
        {
            port->agreed = True;
        }
        else
        {
            port->agreed = False ;
        }
    }
    else
    {
        port->agreed = False ;
    }

    if ( port->rcvdInternal == False )
    {
        PORT_T* mstport = NULL ;
        mstport = port->nextMst;
        for(  ; mstport; mstport = mstport->nextMst ) 
        {
            mstport->agreed = port->agreed ;
        }
    }

    return ;
}


static void stp_portinfo_record_agreementmsti( STATE_MACH_T * this)  /*13.26.10*/
{
    register PORT_T * port = this->owner.port;
    register int bridcmp = 0;
    register PORT_T *cist_port = stp_port_mst_findport(STP_CIST_ID, port->port_index);;
	
    if(!cist_port)
    {
      stp_trace("recordAgreementMsti: failed to find the port %s' cist port", port->port_name);
      return;
    }
    bridcmp = stp_vector_compare( &cist_port->msgPrio, &cist_port->portPrio ) ;
    if ( bridcmp == 0  || bridcmp < -4 || bridcmp > 4)  /* cist root ,external cost and  region root  are not the same*/ 
    {
        if ( AGREEMENT_BIT & port->msgFlags && port->operPointToPointMac )
        {
            bridcmp = ( stp_vector_compare( &port->msgPrio, &port->portPrio ) <= 0 ) ;
            if ( ( ( ( RSTP_PORT_ROLE_ROOT << PORT_ROLE_OFFS ) & port->msgFlags ) && bridcmp <= 0 ) ||
                    ( ( ( RSTP_PORT_ROLE_DESGN << PORT_ROLE_OFFS ) & port->msgFlags ) && bridcmp >= 0 )
               )
            {
                port->agreed = True;
                return ;
            }
        }
    }
	
    port->agreed = False ;

    return ;
}

#if 0
static Bool
recordProposed (STATE_MACH_T* this, char* reason)
{/* 17.19.9 */
  register PORT_T* port = this->owner.port;

  if (RSTP_PORT_ROLE_DESGN == port->msgPortRole &&
      (PROPOSAL_BIT & port->msgFlags)           &&
      port->operPointToPointMac) {
    return True;
  }
  return False;
}

static Bool
stp_receive_setTcFlags (STATE_MACH_T* this)
{/* 17.19.13 */
  register PORT_T* port = this->owner.port;

  if (BPDU_TOPO_CHANGE_TYPE == port->msgBpduType) {
#ifdef STP_DBG
      if (this->debug) {
        stp_trace ("port %s rx rcvdTcn", port->port_name);
      }
#endif
    port->rcvdTcn = True;
  } else {
    if (TOLPLOGY_CHANGE_BIT & port->msgFlags) {
#ifdef STP_DBG
      if (this->debug) {
        stp_trace ("(%s-%s) rx rcvdTc 0X%lx",
            port->owner->name, port->port_name,
            (unsigned long) port->msgFlags);
      }
#endif
      port->rcvdTc = True;
    }
    if (TOLPLOGY_CHANGE_ACK_BIT & port->msgFlags) {
#ifdef STP_DBG
      if (this->debug) {
        stp_trace ("port %s rx rcvdTcAck 0X%lx",
            port->port_name,
            (unsigned long) port->msgFlags);
      }
#endif
      port->rcvdTcAck = True;
    }
  }

  return True;
}

static Bool
updtBPDUVersion (STATE_MACH_T* this)
{/* 17.19.18 */
  register PORT_T* port = this->owner.port;

  if (BPDU_TOPO_CHANGE_TYPE == port->msgBpduType) {
    port->rcvdSTP = True;
  }

  if (port->msgBpduVersion < 2) {
    port->rcvdSTP = True;
  }
  
  if (BPDU_RSTP == port->msgBpduType) {
    /* port->port->owner->ForceVersion >= NORMAL_RSTP
       we have checked in stp_portinfo_rx_bpdu */
    port->rcvdRSTP = True;
  }

  return True;
}

static Bool
updtRcvdInfoWhile (STATE_MACH_T* this)
{/* 17.19.19 */
  register int eff_age, dm, dt;
  register int hello3;
  register PORT_T* port = this->owner.port;
  
  eff_age = ( + port->portTimes.MaxAge) / 16;
  if (eff_age < 1) eff_age = 1;
  eff_age += port->portTimes.MessageAge;

  if (eff_age <= port->portTimes.MaxAge) {
    hello3 = 3 *  port->portTimes.HelloTime;
    dm = port->portTimes.MaxAge - eff_age;
    if (dm > hello3)
      dt = hello3;
    else
      dt = dm;
    port->rcvdInfoWhile = dt;
/****
    stp_trace ("ma=%d eff_age=%d dm=%d dt=%d p=%s",
               (int) port->portTimes.MessageAge,
               (int) eff_age, (int) dm, (int) dt, port->port_name);
****/
  } else {
    port->rcvdInfoWhile = 0;
/****/
#ifdef STP_DBG
    /*if (this->debug) */
    {
      stp_trace ("port %s: MaxAge=%d MessageAge=%d HelloTime=%d rcvdInfoWhile=null !",
            port->port_name,
                (int) port->portTimes.MaxAge,
                (int) port->portTimes.MessageAge,
                (int) port->portTimes.HelloTime);
    }
#endif
/****/
  }

  return True;
}
 
#endif

static void stp_portinfo_send_tcn2npd(PORT_T *port)
{
	register STPM_T  *stpm = port->owner;
	unsigned int mstid = port->vlan_id;
	unsigned int port_index = port->port_index;
	unsigned short vid = 0;
    stp_syslog_dbg("stp_portinfo_send_tcn2npd::port index %d,mstid %d\n",port->port_index,mstid);
	stp_npd_cmd_notify_tcn(mstid,vid,port_index);

	return ;
}

static void stp_portinfo_record_agreementxst( STATE_MACH_T * this )  /* 13.26.9 */
{
    PORT_T* port = this->owner.port;

    if(port->vlan_id == STP_CIST_ID)
	return stp_portinfo_record_agreementcist( this);
  
    return stp_portinfo_record_agreementmsti( this);
}
inline static Bool stp_portinfo_rcvd_cistinfo( PORT_T * this )   /* 13.25.8  */
{

    if ( this == NULL )
        return False ;

    if ( this->rcvdMsg == True )
        return True ;

    return False ;
}

inline static Bool stp_portinfo_rcvd_mstinfo( PORT_T * this )   /*  13.25.9 */
{
    PORT_T * port = NULL ;

    if ( this == NULL )
        return False ;

    port = stp_port_mst_findport( STP_CIST_ID, this->port_index );
    if ( port != NULL )
    {
         if ( !port->rcvdMsg && this->rcvdMsg )
             return True ;
    }

    return False ;
}
static Bool stp_portinfo_rcvd_xstinfo( PORT_T * this )   /* 13.25.8  */
{

    if(this->vlan_id == STP_CIST_ID)
	return stp_portinfo_rcvd_cistinfo( this);
  
    return stp_portinfo_rcvd_mstinfo( this);
}


inline static Bool stp_portinfo_updt_cistinfo( PORT_T * this )   /* 13.25.11  */
{

    if ( this == NULL )
        return False ;

    if ( this->updtInfo )
        return True ;

    return False ;
}


inline static Bool stp_portinfo_updt_mstinfo( PORT_T * this )   /*  13.25.12 */
{
    PORT_T *port;
    if ( this == NULL )
        return False ;

    if ( this->updtInfo )
        return True ;
    port = stp_port_mst_findport( STP_CIST_ID, this->port_index );
    if ( port != NULL )
    {
        if ( port->updtInfo || port->reselect ) return True;
    }
    return False ;
}

static Bool stp_portinfo_updt_xstinfo( PORT_T * this )   /*  13.25.12 */
{

    if(this->vlan_id == STP_CIST_ID)
	return stp_portinfo_updt_cistinfo( this);
  
    return stp_portinfo_updt_mstinfo( this);
}
inline static Bool stp_portinfo_updtRcvdInfoWhileCist ( STATE_MACH_T * this )  /* 13.26.23 */
{
    register int eff_age, dm, dt;
    register int hello3;
    register PORT_T* port = this->owner.port;

    if ( port->rcvdInternal == False )
    {
        eff_age = ( 8 + port->portTimes.MaxAge ) / 16;  

        if ( eff_age < 1 )
            eff_age = 1;
        eff_age += port->msgTimes.MessageAge;

        if ( eff_age <= port->portTimes.MaxAge )
        {
            hello3 = 3 * port->portTimes.HelloTime;
            dm = port->portTimes.MaxAge - eff_age;
            if ( dm > hello3 )
                dt = hello3;
            else
                dt = dm;
            port->rcvdInfoWhile = dt;
            
        }
        else
        {
            port->rcvdInfoWhile = 0;
        }

        port->portTimes.MessageAge = eff_age ; 
        port->portTimes.RemainingHops = port->owner->BrTimes.RemainingHops ;

    }
    else 
    {
        port->portTimes.MessageAge = port->msgTimes.MessageAge ;
        port->portTimes.RemainingHops = port->msgTimes.RemainingHops - 1 ;
        if ( port->portTimes.MessageAge > port->portTimes.MaxAge ||
                port->portTimes.RemainingHops <= 0 )
        {
            port->rcvdInfoWhile = 0;
        }
        else
        {
            port->rcvdInfoWhile = 3 * port->portTimes.HelloTime;
        }
    }

    return True;
}

inline static Bool stp_portinfo_updtRcvdInfoWhileMsti( STATE_MACH_T * this )  /*13.26.24 */
{
    register PORT_T* port = this->owner.port;
    register PORT_T* cistport = NULL ;

    port->portTimes.RemainingHops = port->msgTimes.RemainingHops - 1 ;
    if ( port->portTimes.RemainingHops <= 0 )
    {
        port->rcvdInfoWhile = 0;
    }
    else
    {
        cistport = stp_port_mst_findport( STP_CIST_ID, port->port_index ) ;
        if ( cistport )
        {
            port->rcvdInfoWhile = 3 * cistport->portTimes.HelloTime;
        }
        else
        {
            port->rcvdInfoWhile = 3 * DEF_BR_HELLOT ;
        }
    }

    return True;
}

inline static Bool stp_portinfo_updtRcvdInfoWhileXst ( STATE_MACH_T * this ) 
{
    PORT_T* port = this->owner.port;

    if(port->vlan_id == STP_CIST_ID)
	return stp_portinfo_updtRcvdInfoWhileCist( this);
  
    return stp_portinfo_updtRcvdInfoWhileMsti( this);
}

int
stp_portinfo_rx_bpdu (PORT_T* port, struct stp_bpdu_t* bpdu, size_t len)
{  
  unsigned char bpdu_type;	/*mstp*/
  int ret = 10;
  struct stpm_t*    owner = NULL;
  struct port_t*    ports = NULL;
  

/*mstp*/
  if(stp_in_check_bpdu_header(port, (MSTP_BPDU_T*)bpdu, len, &bpdu_type) < 0)
    return -1;
  #if 0
  if(port->receive->debug)
  {
    int iii =0;
    unsigned char *temp = (unsigned char *)bpdu;
    stp_trace("stp_portinfo_rx_bpdu receive bpdu(len %d type %d) on %s", len, bpdu_type, port->port_name);
    for(iii = 0; iii < len; iii++)
    {
      if((iii % 16) == 0)
        printf("\n");
      printf("%.2x  ", temp[iii]);
    }
    printf("\n");
  }
  #endif
/*mstp*/
  /* check bpdu type */
  switch (bpdu_type) {
    case BPDU_CONFIG_TYPE:
      port->rx_cfg_bpdu_cnt++;
#if 0 /* def STP_DBG */
      if (port->info->debug) 
        stp_trace ("CfgBpdu on port %s", port->port_name);
#endif
      if (port->admin_non_stp) return -1;
      port->rcvdBpdu = True;
      break;
    case BPDU_TOPO_CHANGE_TYPE:
      port->rx_tcn_bpdu_cnt++;
#if 0 /* def STP_DBG */
      if (port->info->debug)
        stp_trace ("TcnBpdu on port %s", port->port_name);
#endif
      if (port->admin_non_stp) return -1;
      port->rcvdBpdu = True;
      port->msgBpduVersion = bpdu->hdr.version;
      port->msgBpduType = bpdu->hdr.bpdu_type;
      break;
    case BPDU_RSTP:
    case BPDU_MSTP:
      port->rx_rstp_bpdu_cnt++;
      if (port->admin_non_stp) return -1;
      if (port->owner->ForceVersion >= NORMAL_RSTP) {
        port->rcvdBpdu = True;
      } else {          
        return -1;
      }
#if 0 /* def STP_DBG */
      if (port->info->debug)
        stp_trace ("BPDU_RSTP on port %s", port->port_name);
#endif
      break;
    default:
      stp_trace ("RX undef bpdu type=%d", (int) bpdu->hdr.bpdu_type);
      return-1;
  }

  port->msgBpduVersion = bpdu->hdr.version;
  port->msgBpduType =    bpdu->hdr.bpdu_type;
  port->msgFlags =       bpdu->body.flags;

  /*when topo changed, notify npd*/

  if(( BPDU_TOPO_CHANGE_TYPE == bpdu_type)||(bpdu->body.flags & TOLPLOGY_CHANGE_BIT)||(bpdu->body.flags & TOLPLOGY_CHANGE_ACK_BIT)){
	  /*should notify to npd to syn the arp & fdb*/
	 owner = port->owner;
	 for(ports = owner->ports;ports;ports = ports->next){
	 	if(ports->portEnabled){
			stp_syslog_dbg("stp_portinfo_rx_bpdu:: port %d\n",ports->port_index);
			stp_portinfo_send_tcn2npd(ports);
	 	}
	 }	
  }

  /* 17.18.25, 17.18.26 : see stp_receive_setTcFlags() */
/*mstp*/
 stp_in_set_cur_bpdu(bpdu, bpdu_type, port->configDigestSnp);
  stp_trace ("configDigestSnp RX set bpdu %p and type=%d", bpdu, bpdu_type);
 return 0;

}

void MSTP_info_enter_state (STATE_MACH_T* this)
{
  register PORT_T* port = this->owner.port;

  switch (this->State) {
    case BEGIN:
      port->rcvdMsg = False;
      /*port->rcvdMsg = OtherMsg;*/
      port->msgBpduType = -1;
      port->msgPortRole = RSTP_PORT_ROLE_UNKN;
      port->msgFlags = 0;

      /* clear port statistics */
      port->rx_cfg_bpdu_cnt =
      port->rx_rstp_bpdu_cnt =
      port->rx_tcn_bpdu_cnt = 0;
      
    case DISABLED:
      /*port->rcvdBpdu = port->rcvdRSTP = port->rcvdSTP = False;
		port->updtInfo = port->proposing = False;  In DISABLED 
		port->agreed = port->proposed = False;*/
      port->rcvdMsg = False;  /*mstp*/
      port->proposing = port->proposed = port->agree = port->agreed = False; /* In DISABLED */
      port->rcvdInfoWhile = 0;
      port->infoIs = Disabled;
      port->updtInfo = False;  /*mstp*/
      port->reselect = True;
      port->selected = False;
	  /*portTimes,portPrio*/	  
      break;
    case ENABLED: /* IEEE 802.1y, 17.21, Z.14 */
      stp_vector_copy (&port->portPrio, &port->designPrio);
      stp_times_copy (&port->portTimes, &port->designTimes);
      break;
    case AGED:
      port->infoIs = Aged;
      port->reselect = True;
      port->selected = False;
      break;
    case UPDATE:
/*mstp*/
      port->proposing = port->proposed = False; /* in UPDATE */
      port->synced = False; 
      port->sync = port->changedMaster;
      port->agreed = port->agreed && stp_portinfo_xstinfo_check( this ) && !port->changedMaster ;
/*mstp*/
      stp_vector_copy (&port->portPrio, &port->designPrio);
      stp_times_copy (&port->portTimes, &port->designTimes);
      /*port->updtInfo = False;*/
      port->changedMaster = port->updtInfo = False;
      /*port->agreed = port->synced = False;  In UPDATE */
      /*port->proposed = port->proposing = False; *//* in UPDATE */ 
      port->infoIs = Mine;
      port->newInfo = True;
      stp_roletrns_setnewinfoXst( port);       /*mstp*/
#ifdef STP_DBG
      if (this->debug) {
        stp_vector_br_id_print ("updated: portPrio.design_bridge",
                            &port->portPrio.design_bridge, True);
      }
#endif
      break;
    case CURRENT:
      break;
    case RECEIVE:
      /*port->rcvdMsg = rcvBpdu (this);
      updtBPDUVersion (this);
      stp_receive_setTcFlags (this);
      port->rcvdBpdu = False;*/
            port->rcvdInfo = stp_portinfo_xst_rcvinfo(this); /*mstp*/
            stp_portinfo_record_masteredxst( this );
      break;
    /*case SUPERIOR:
      stp_vector_copy (&port->portPrio, &port->msgPrio);
      stp_times_copy (&port->portTimes, &port->msgTimes);
      updtRcvdInfoWhile (this);*/
#if 0 /* due 802.1y, Z.7 */
      port->agreed = False; /* deleted due 802.y in SUPERIOR */
      port->synced = False; /* due 802.y deleted in SUPERIOR */

      port->proposing = False; /* in SUPERIOR */
      port->proposed = recordProposed (this, "SUPERIOR");
      port->infoIs = Received;
      port->reselect = True;
      port->selected = False;
#endif
/*mstp*/
    case SUPERIOR_DESIGNATED:
            port->infoInternal = port->rcvdInternal;
            port->proposing = False;
            stp_portinfo_record_proposalxst( this );
            port->agree = port->agree && stp_portinfo_xstinfo_check( this );
            stp_portinfo_record_agreementxst( this  ) ;
            port->synced = port->synced && port->agreed ;

            stp_vector_copy ( &port->portPrio, &port->msgPrio );
            stp_times_copy ( &port->portTimes, &port->msgTimes );
            stp_portinfo_updtRcvdInfoWhileXst( this );
            port->infoIs = Received ;
            port->reselect = True;
            port->selected = False ;
            port->rcvdMsg = False ;
         /*   break;*//*pc-lint*/
#ifdef STP_DBG

            if ( this->debug )
            {
                stp_vector_br_id_print ( "stored: portPrio.design_bridge",
                                       &port->portPrio.design_bridge, True );
                stp_trace ( "proposed=%d on port %s",
                            ( int ) port->proposed, port->port_name );
            }
#endif
           break; 
#if 0
    case REPEAT:
      port->proposed = recordProposed (this, "REPEAT");
      updtRcvdInfoWhile (this);
      break;
  case AGREEMENT:
#ifdef STP_DBG
      if (port->roletrns->debug) {
        stp_trace ("(%s-%s) rx AGREEMENT flag !",
            port->owner->name, port->port_name);
      }
#endif
      
      port->agreed = True;
      port->proposing = False; /* In AGREEMENT */
      break;
  }
#endif
/*mstp*/
        case REPEATED_DESIGNATED:
            port->infoInternal = port->rcvdInternal;
            stp_portinfo_record_proposalxst( this );
            stp_portinfo_record_agreementxst( this );
            stp_portinfo_updtRcvdInfoWhileXst( this );
            port->rcvdMsg = False ;
            break;

        case ROOT:
            stp_portinfo_record_agreementxst( this  );
            port->rcvdMsg = False ;
            
            #ifdef STP_DBG
            
            if ( port->agreed && port->roletrns->debug )
            {
                stp_trace ( "(%s-%s) rx AGREEMENT flag !",
                            port->owner->name, port->port_name );
            }
            #endif
            
            break;

        case OTHER:
            port->rcvdMsg = False ;
            break;
        default :
            break;
    }
}

Bool MSTP_info_check_conditions (STATE_MACH_T* this)
{
  register PORT_T* port = this->owner.port;

  if ((! port->portEnabled && port->infoIs != Disabled) || BEGIN == this->State) {
    return stp_statmch_hop_2_state (this, DISABLED);
  }

  switch (this->State) {
    case BEGIN:
      return stp_statmch_hop_2_state ( this, DISABLED );
    case DISABLED:
# if 0
      if (port->updtInfo) {
        return stp_statmch_hop_2_state (this, DISABLED);
      }
      if (port->portEnabled && port->selected) {
        return stp_statmch_hop_2_state (this, ENABLED);
      }
      if (port->rcvdBpdu) {
        return stp_statmch_hop_2_state (this, DISABLED);
      }
#endif
      if ( port->portEnabled )
      {
          return stp_statmch_hop_2_state ( this, AGED );
      }
      if ( port->rcvdMsg )
      {
          return stp_statmch_hop_2_state ( this, DISABLED );
      }
      if (port->updtInfo) 
      {
          return stp_statmch_hop_2_state (this, DISABLED);
      }
      break; 
    case ENABLED: /* IEEE 802.1y, 17.21, Z.14 */
      return stp_statmch_hop_2_state (this, AGED);
      break; 
    case AGED:
      if (port->selected && port->updtInfo) {
        return stp_statmch_hop_2_state (this, UPDATE);
      }
      break;
    case UPDATE:
      return stp_statmch_hop_2_state (this, CURRENT);
      break;
    case CURRENT:
      if (port->selected && port->updtInfo) {
        return stp_statmch_hop_2_state (this, UPDATE);
      }

      if (Received == port->infoIs       &&
          ! port->rcvdInfoWhile &&
          ! port->updtInfo               &&
          !stp_portinfo_rcvd_xstinfo( port ) )	/*mstp*/
      {
        return stp_statmch_hop_2_state (this, AGED);
      }
      if ( port->rcvdMsg && !stp_portinfo_updt_xstinfo( port ) )
        return stp_statmch_hop_2_state (this, RECEIVE);
      break;
    case RECEIVE:
      switch ( port->rcvdInfo )  /*mstp*/
      {
        case SuperiorDesignateMsg:
              return stp_statmch_hop_2_state ( this, SUPERIOR_DESIGNATED );
          case RepeatedDesignateMsg:
              return stp_statmch_hop_2_state ( this, REPEATED_DESIGNATED );
          case ConfirmedRootMsg:
              return stp_statmch_hop_2_state ( this, ROOT );
          case OtherMsg :
              return stp_statmch_hop_2_state ( this, OTHER );
          default:
              break;
      }
      break;
    case SUPERIOR_DESIGNATED:
      return stp_statmch_hop_2_state (this, CURRENT);
      break;
    case REPEATED_DESIGNATED:
      return stp_statmch_hop_2_state ( this, CURRENT );
      break;
    case ROOT:
      return stp_statmch_hop_2_state (this, CURRENT);
      break;
    case OTHER:
      return stp_statmch_hop_2_state (this, CURRENT);
      break;
    default :
      break;
  }

  return False;
}

#ifdef __cplusplus
}
#endif

