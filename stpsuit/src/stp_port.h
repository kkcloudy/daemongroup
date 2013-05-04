/* STP PORT instance : 17.18, 17.15 */
 
#ifndef _STP_PORT_H__
#define _STP_PORT_H__

#include "stp_statmch.h"


#define TIMERS_NUMBER   9
#define MAX_PORT_NUM    51
typedef unsigned int    PORT_TIMER_T;

typedef enum {
  Mine,
  Aged,
  Received,
  Disabled
} INFO_IS_T;

typedef enum {
  SuperiorDesignateMsg,
  RepeatedDesignateMsg,
  ConfirmedRootMsg,
  OtherMsg
} RCVD_MSG_T;

typedef enum {
  DisabledPort = 0,
  AlternatePort,
  BackupPort,
  RootPort,
  DesignatedPort,
  MasterPort,   /*mstp*/
  NonStpPort
} PORT_ROLE_T;

typedef struct vlan_map_t{    /*mstp*/
  unsigned char bmp[512];
  unsigned long ulcount;
}VLAN_MAP_T;

typedef struct port_t {
  struct port_t*     next;
  struct port_t*     nextMst;/*used to chain per port for cist and each mst, the header is always cist port */
  VLAN_MAP_T    vlan_map;


  /* per Port state machines 802.1w 2001*/
  STATE_MACH_T*     info;      /* 17.21  The Port Information state machine*/
  STATE_MACH_T*     roletrns;  /* 17.23 The Port Role Transitions state machine*/
  STATE_MACH_T*     sttrans;   /* 17.24 Port State Transition state machine*/
  STATE_MACH_T*     topoch;    /* 17.25 Topology Change state machine*/
  STATE_MACH_T*     migrate;   /* 17.26 Port Protocol Migration state machine*/
  STATE_MACH_T*     transmit;  /* 17.27 Port Transmit state machine*/
  STATE_MACH_T*     p2p;       /* 6.4.3, 6.5.1 */
  STATE_MACH_T*     edge;      /* edge port */
  STATE_MACH_T*     pcost;     /* path cost  */
  STATE_MACH_T*     receive ;  /* 13.28  mstp*/

  STATE_MACH_T*     machines; /* list of machines */

  struct stpm_t*    owner; /* Bridge, that this port belongs to */
  
  /* per port Timers  */
  PORT_TIMER_T      fdWhile;      /* 17.15.1 */
  PORT_TIMER_T      helloWhen;    /* 17.15.2 */
  PORT_TIMER_T      mdelayWhile;  /* 17.15.3 */
  PORT_TIMER_T      rbWhile;      /* 17.15.4 */
  PORT_TIMER_T      rcvdInfoWhile;/* 17.15.5 */
  PORT_TIMER_T      rrWhile;      /* 17.15.6 */
  PORT_TIMER_T      tcWhile;      /* 17.15.7 */
  PORT_TIMER_T      txCount;      /* 17.18.40 */
  PORT_TIMER_T      lnkWhile;

  PORT_TIMER_T*     timers[TIMERS_NUMBER]; /*list of timers */

  Bool              agreed;        /* 17.18.1 */
  PRIO_VECTOR_T     designPrio;    /* 17.18.2 */
  TIMEVALUES_T      designTimes;   /* 17.18.3 */
  Bool              forward;       /* 17.18.4 */
  Bool              forwarding;    /* 17.18.5 */
  INFO_IS_T         infoIs;        /* 17.18.6 */
  Bool              initPm;        /* 17.18.7  */
  Bool              learn;         /* 17.18.8 */
  Bool              learning;      /* 17.18.9 */
  Bool              mcheck;        /* 17.18.10 */
  PRIO_VECTOR_T     msgPrio;       /* 17.18.11 */
  TIMEVALUES_T      msgTimes;      /* 17.18.12 */
  Bool              newInfo;       /* 17.18.13 */
  Bool              operEdge;      /* 17.18.14 */
  Bool              adminEdge;     /* 17.18.14 */
  Bool              portEnabled;   /* 17.18.15 */
  PORT_ID           port_id;       /* 17.18.16 */
  PRIO_VECTOR_T     portPrio;      /* 17.18.17 */
  TIMEVALUES_T      portTimes;     /* 17.18.18 */
  Bool              proposed;      /* 17.18.19 */
  Bool              proposing;     /* 17.18.20 */
  Bool              rcvdBpdu;      /* 17.18.21 */
  RCVD_MSG_T        rcvdInfo;       /* 17.18.22, 13.24.23 *//* 1...1*/
 /*  RCVD_MSG_T        rcvdMsg;       17.18.22 */
  Bool              rcvdRSTP;      /* 17/18.23 */
  Bool              rcvdSTP;       /* 17.18.24 */
  Bool              rcvdTc;        /* 17.18.25 */
  Bool              rcvdTcAck;     /* 17.18.26 */
  Bool              rcvdTcn;       /* 17.18.27 */
  Bool              reRoot;        /* 17.18.28 */
  Bool              reselect;      /* 17.18.29 */
  PORT_ROLE_T       role;          /* 17.18.30 */
  Bool              selected;      /* 17.18.31 */
  PORT_ROLE_T       selectedRole;  /* 17.18.32 */
  Bool              sendRSTP;      /* 17.18.33 */
  Bool              sync;          /* 17.18.34 */
  Bool              synced;        /* 17.18.35 */
  Bool              tc;            /* 17.18.36 */
  Bool              tcAck;         /* 17.18.37 */
  Bool              tcProp;        /* 17.18.38 */

  Bool              updtInfo;      /* 17.18.41 *//* 1...1*/
  
  Bool              agree;      /*mstp 13.24.1*//* 1...1*/
  Bool              changedMaster;     /*13.24.3*//* 1...1*/
  Bool              infoInternal;      /*13.24.10*/
  Bool              mstiMaster;     /*13.24.13*/
  Bool              mstiMastered;     /*13.24.14*/
  Bool              newInfoCist;       /*13.24.19*/
  Bool              newInfoMsti;       /*13.24.20*/
  Bool              rcvdInternal;        /*13.24.22*/
  Bool              rcvdMsg;     /* mstp 13.24.24*//* 1...1*/

  /* message information */
  unsigned char     msgBpduVersion;
  unsigned char     msgBpduType;
  unsigned char     msgPortRole;
  unsigned char     msgFlags;

  unsigned long     adminPCost; /* may be ADMIN_PORT_PATH_COST_AUTO */
  unsigned long     operPCost;
  unsigned long     operSpeed;
  unsigned long     usedSpeed;
  int               LinkDelay;   /* TBD: LinkDelay may be managed ? */
  Bool              adminEnable; /* 'has LINK' ,modify link up or down*/
  /* add for master port mstp */
  Bool              isWAN;  /*port is Master Control Port*/
  unsigned int      slot_no;  /*port is Master Control Port use it*/
  unsigned int      port_no;  /*port is Master Control Port use it*/
  unsigned int      port_ifindex; /*add for kernel stp*/
  unsigned int      br_ifindex; /*add for kernel stp*/
  
  Bool              wasInitBpdu;  
  Bool              admin_non_stp;

  Bool              p2p_recompute;
  Bool              operPointToPointMac;
  ADMIN_P2P_T       adminPointToPointMac;

  /* statistics */
  unsigned long     rx_cfg_bpdu_cnt;
  unsigned long     rx_rstp_bpdu_cnt;
  unsigned long     rx_tcn_bpdu_cnt;

  unsigned long     uptime;       /* 14.8.2.1.3.a */

  int               port_index;
  /*char*             port_name;*/
  char             port_name[32]; /*mstp*/
  unsigned short vlan_id; /*mstp*/
#ifdef STP_DBG
  unsigned int	    skip_rx;
  unsigned int	    skip_tx;
#endif

  /* configuration digest snooping option 
   *   When option enabled,send mstp packet with configuration digest originate
   * by the sender 
   */
  unsigned char 	configDigestSnp:1, /* 0 - disable, 1 - enable */
  					rsvd:7;
  unsigned char		digest[16];
} PORT_T;

PORT_T*
STP_port_create (struct stpm_t* stpm, int port_index);

void
stp_port_delete (PORT_T* this);

int
stp_port_rx_bpdu (PORT_T* this, BPDU_T* bpdu, size_t len);

void
stp_port_init (PORT_T* this, struct stpm_t* stpm, Bool check_link);

#ifdef STP_DBG
int
stp_port_trace_state_machine (PORT_T* this, char* mach_name, int enadis, int vlan_id);

int 
stp_port_bridge_trace_state_machine(struct stpm_t* this, char* mach_name,int enadis,int vlan_id);

void
stp_port_trace_flags (char* title, PORT_T* this);
/*mstp*/
int stp_port_mst_addport( unsigned short MSTID, unsigned long port_index, unsigned short VID, Bool bIfLock, unsigned int isWan );

PORT_T * stp_port_mst_findport( unsigned short MSTID, unsigned long port_index );

int stp_port_mst_delport( unsigned short MSTID, unsigned long port_index, unsigned short VID, Bool bIfLock );

void stp_port_clear_gloabl_portarray();

#endif

#endif /*  _STP_PORT_H__ */

