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
* stp_vector.c
*
* CREATOR:
*       zhubo@autelan.com
*
* DESCRIPTION:
*       APIs for STP priority vectors in stp module
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

/* STP priority vectors API : 17.4.2 */
 
#include "stp_base.h"
#include "stp_bpdu.h"
#include "stp_vector.h"
#include "stp_port.h"
#include "stp_stpm.h"

unsigned int pkt_mstid = 0;

int
stp_vector_compare_bridge_id (BRIDGE_ID* b1, BRIDGE_ID* b2)
{
  if (b1->prio < b2->prio)
    return -1;

  if (b1->prio > b2->prio)
    return 1;
  return memcmp (b1->addr, b2->addr, 6);
}

void
stp_vector_copy (OUT PRIO_VECTOR_T* t, IN PRIO_VECTOR_T* f)
{
  memcpy (t, f, sizeof (PRIO_VECTOR_T));
}

void
stp_vector_create (OUT PRIO_VECTOR_T* t,
                 IN BRIDGE_ID* root_br,
                 IN unsigned long root_path_cost,
                  IN BRIDGE_ID* region_root_br,
                 IN unsigned long region_root_path_cost,
                IN BRIDGE_ID* design_bridge,
                 IN PORT_ID design_port,
                 IN PORT_ID bridge_port)
{
  memcpy (&t->root_bridge, root_br, sizeof (BRIDGE_ID));
  t->root_path_cost = root_path_cost;
  memcpy (&t->region_root_bridge, region_root_br, sizeof (BRIDGE_ID));
  t->region_root_path_cost = region_root_path_cost;
  memcpy (&t->design_bridge, design_bridge, sizeof (BRIDGE_ID));
  t->design_port = design_port;
  t->bridge_port = bridge_port;
}

int
stp_vector_compare (PRIO_VECTOR_T* v1, PRIO_VECTOR_T* v2)
{
  int bridcmp;

  bridcmp = stp_vector_compare_bridge_id (&v1->root_bridge, &v2->root_bridge);
  if (bridcmp < 0) return -1;
  if(bridcmp > 0) return 1;
  if (! bridcmp) {
    bridcmp = v1->root_path_cost - v2->root_path_cost;
    if (bridcmp < 0) return -2;
    if(bridcmp > 0) return 2;
    if (! bridcmp) {
      bridcmp = stp_vector_compare_bridge_id ( &v1->region_root_bridge, &v2->region_root_bridge );
      if ( bridcmp < 0 )  return -3;
      if(bridcmp > 0) return 3;
      if (! bridcmp) {
        bridcmp = v1->region_root_path_cost - v2->region_root_path_cost;  
        if ( bridcmp < 0 )  return -4;
        if(bridcmp > 0) return 4;
        if (! bridcmp) {
          bridcmp = stp_vector_compare_bridge_id (&v1->design_bridge, &v2->design_bridge);
          if (bridcmp < 0) return -5;
          if(bridcmp > 0) return 5;
          if (! bridcmp) {
             bridcmp = v1->design_port - v2->design_port;
             if (bridcmp < 0) return -6;
             if(bridcmp > 0) return 6;
             if (! bridcmp){
               bridcmp  = v1->bridge_port - v2->bridge_port;;
               if (bridcmp < 0) return -7;
               if(bridcmp > 0) return 7;
             }
           }
         }
       }
     }
  }

  return bridcmp;
}

static unsigned short
stp_vector_get_short (IN unsigned char* f)
{
  return ntohs (*(unsigned short *)f);
}

static void
stp_vector_set_short (IN unsigned short f, OUT unsigned char* t)
{
  *(unsigned short *)t = htons (f);
}

void
stp_vector_get_bridge_id (IN unsigned char* c_br, OUT BRIDGE_ID* bridge_id)
{
  bridge_id->prio = stp_vector_get_short (c_br);
  memcpy (bridge_id->addr, c_br + 2, 6);
}

void
stp_vector_set_bridge_id (IN BRIDGE_ID* bridge_id, OUT unsigned char* c_br)
{
  stp_vector_set_short (bridge_id->prio, c_br);
  memcpy (c_br + 2, bridge_id->addr, 6);
}

void
stp_vector_get_vector (IN BPDU_BODY_T* b, OUT PRIO_VECTOR_T* v)
{
  stp_vector_get_bridge_id (b->root_id, &v->root_bridge);

  v->root_path_cost = ntohl (*((long*) b->root_path_cost));

  stp_vector_get_bridge_id (b->bridge_id, &v->design_bridge);

  v->design_port = stp_vector_get_short (b->port_id);
}

void
stp_vector_set_vector (IN PRIO_VECTOR_T* v, OUT BPDU_BODY_T* b)
{
  unsigned long root_path_cost;

  stp_vector_set_bridge_id (&v->root_bridge, b->root_id);

  root_path_cost = htonl (v->root_path_cost);
  memcpy (b->root_path_cost, &root_path_cost, 4);

  stp_vector_set_bridge_id (&v->design_bridge, b->bridge_id);

  stp_vector_set_short (v->design_port, b->port_id);
}

void stp_vector_get_mst_vector( IN MSTP_BPDU_T * b , IN int index, IN Bool vendorCisco, OUT PRIO_VECTOR_T * v )
{
	MSTP_Cisco_BPDU_T *ciscoBpdu = NULL;
	MSTI_CFG_MSG_T *m = NULL;
	MSTI_Cisco_CFG_MST_T *mc = NULL;
	unsigned char msti_bridge_priority = 0, *msti_internal_root_path_cost = NULL;
	unsigned char *msti_region_root_id = NULL, *cist_bridge_id = NULL;
	
	if(vendorCisco) {
		ciscoBpdu = (MSTP_Cisco_BPDU_T*)b;
		cist_bridge_id = ciscoBpdu->cist_region_root_id;
		
		mc = &(ciscoBpdu->MCfgMsg[index]);
		msti_region_root_id = mc->msti_region_root_id;
		msti_internal_root_path_cost = mc->msti_internal_root_path_cost;
		//msti_bridge_priority = mc->msti_bridge_priority;
	}
	else {
		cist_bridge_id = b->cist_bridge_id;
		m = &(b->MCfgMsg[index]);
		msti_region_root_id = m->msti_region_root_id;
		msti_internal_root_path_cost = m->msti_internal_root_path_cost;
	}
	
    stp_vector_get_bridge_id(msti_region_root_id , &v->region_root_bridge );
    v->region_root_path_cost = ntohl( *( ( long* )msti_internal_root_path_cost ) );

    /* cist desginate bridge */
    stp_vector_get_bridge_id(cist_bridge_id, &v->design_bridge);
	if(vendorCisco) {
		memcpy(&v->design_bridge,&mc->msti_bridge_priority, 8);
	}
	else {
    	v->design_bridge.prio = 0 ;
    	v->design_bridge.prio += ( 0xf000 & ( ( m->msti_bridge_priority & 0xf0 ) << 8 ) ) ;
	}
	
    /* cist desginate bridge id */
	if(vendorCisco) {
		memcpy(&v->design_port,&mc->msti_port_priority, 2);
	}
	else {
	    v->design_port = ntohs (*(unsigned short *)b->body.port_id);
	    v->design_port &= 0x0fff;
	    v->design_port += ( 0xf000 & ( ( m->msti_port_priority & 0xf0 ) << 8 ) ) ;
	}

    return ;
}

void stp_vector_cisco_set_mst_vector
(
	OUT MSTP_Cisco_BPDU_T *b,
	IN int msti_id,
	IN PORT_T *p
)
{
    MSTI_Cisco_CFG_MST_T *m = NULL;
    STPM_T *stpm = NULL;
    char role = 0;
	PRIO_VECTOR_T *v = NULL;

	m = &(b->MCfgMsg[msti_id]);
	stpm = p->owner;
	v = &p->portPrio;

	m->msti_id = stpm->vlan_id;

	m->msti_flags = 0;
	if (p->tcWhile) {
		m->msti_flags |= TOLPLOGY_CHANGE_BIT;
	}
	if (p->proposing) {
		m->msti_flags |= PROPOSAL_BIT;
	}
	if (p->learning) {
		m->msti_flags |= LEARN_BIT;
	}
	if (p->forwarding) {
		m->msti_flags |= FORWARD_BIT;
	}
	if (p->agree) {
		m->msti_flags |= AGREEMENT_BIT;
	}
	if (p->role == MasterPort) {
		m->msti_flags |= MASTER_BIT;
	}
	
	switch (p->role)
	{
		case DisabledPort:
			role = RSTP_PORT_ROLE_UNKN;
			break;
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
		default:
			break;
	}

	m->msti_flags |= (role << PORT_ROLE_OFFS);  

	/* MSTI Region Root Identifier */
	stp_vector_set_bridge_id(&v->region_root_bridge, m->msti_region_root_id);
	/* MSTI Internal Root Path Cost */
	( *( long* )m->msti_internal_root_path_cost) = htonl(v->region_root_path_cost);
	/* MSTI Bridge Priority */
	#if 0
	m->msti_bridge_priority =(stpm->BrId.prio & 0xf000) >> 8;
	#else
	stp_vector_set_bridge_id(&stpm->BrId, m->msti_bridge_priority);
	#endif
	/* MSTI Port Priorit */
	(*(unsigned short *)m->msti_port_priority) = htons((p->port_id & 0xf000) >> 8);
	/* MSTI Remaining Hops */
	m->msti_remaining_hops = p->portTimes.RemainingHops ;
	return ;
}

void stp_vector_set_mst_vector( OUT MSTP_BPDU_T * b , IN int msti_id, IN PORT_T * p )
{
    MSTI_CFG_MSG_T *m = &(b->MCfgMsg[msti_id]);
    STPM_T *stpm = p->owner;
    char role = 0;
    PRIO_VECTOR_T *v = &p->portPrio ;
    m->msti_flags = 0;
    if ( p->tcWhile )
        m->msti_flags |= TOLPLOGY_CHANGE_BIT;
    if ( p->proposing )
        m->msti_flags |= PROPOSAL_BIT;
    if ( p->learning )
        m->msti_flags |= LEARN_BIT;
    if ( p->forwarding )
        m->msti_flags |= FORWARD_BIT;
    if ( p->agree)
        m->msti_flags |= AGREEMENT_BIT;
    if ( p->role == MasterPort )
        m->msti_flags |= MASTER_BIT;
    switch ( p->role )
    {
        case DisabledPort:
            role = RSTP_PORT_ROLE_UNKN;
            break;
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
        default:
            break;
    }

    m->msti_flags |= ( role << PORT_ROLE_OFFS );  

    stp_vector_set_bridge_id( &v->region_root_bridge , m->msti_region_root_id );
    *( unsigned short * ) m->msti_region_root_id = htons( v->region_root_bridge.prio ) ;

    ( *( long* ) m->msti_internal_root_path_cost ) = htonl( v->region_root_path_cost );

    m->msti_bridge_priority =( stpm->BrId.prio & 0xf000 ) >> 8 ;

    m->msti_port_priority = ( p->port_id & 0xf000 ) >> 8 ;

    m->msti_remaining_hops = p->portTimes.RemainingHops ;

	pkt_mstid = (int)(m->msti_region_root_id[1]);
    return ;
}
void stp_vector_get_mst_timers( OUT MSTP_BPDU_T * b , IN int msti_id, OUT TIMEVALUES_T * v )
{
    MSTI_CFG_MSG_T *m = &(b->MCfgMsg[msti_id]);

    v->RemainingHops = m->msti_remaining_hops ;
    v->ForwardDelay = v->HelloTime = v->MaxAge = v->MessageAge = 0;
}

#ifdef STP_DBG

void
stp_vector_br_id_print (IN char *title, IN BRIDGE_ID* br_id, IN Bool cr)
{
  Print ("%s=%04lX-%02x%02x%02x%02x%02x%02x",
            title,
          (unsigned long) br_id->prio,
          (unsigned char) br_id->addr[0],
          (unsigned char) br_id->addr[1],
          (unsigned char) br_id->addr[2],
          (unsigned char) br_id->addr[3],
          (unsigned char) br_id->addr[4],
          (unsigned char) br_id->addr[5]);
  Print (cr ? "\n" : " ");
}

void
stp_vector_print (IN char *title, IN PRIO_VECTOR_T *v)
{
  Print ("%s:", title);
  stp_vector_br_id_print ("rootBr", &v->root_bridge, False);
    
/****
  Print (" rpc=%ld ", (long) v->root_path_cost);
****/
  Print (" rpc=%ld ", (long) v->root_path_cost);

  Print ("%s:", title);
  stp_vector_br_id_print ("regionrootBr", &v->region_root_bridge, False);
    
  Print (" regionrpc=%ld ", (long) v->region_root_path_cost);
  stp_vector_br_id_print ("designBr", &v->design_bridge, False);

/****/
  Print (" dp=%lx bp=%lx ",
          (unsigned long) v->design_port,
          (unsigned long) v->bridge_port);
/***********/
  Print ("\n");
}
#endif
#ifdef __cplusplus
}
#endif

