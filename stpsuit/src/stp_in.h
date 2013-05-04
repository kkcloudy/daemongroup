 /* This file contains prototypes for API from an operation
    system to the RSTP */
#ifndef _STP_API_H__
#define _STP_API_H__

/************************
 * Common base constants
 ************************/

#ifndef INOUT
#  define IN      /* consider as comments near 'input' parameters */
#  define OUT     /* consider as comments near 'output' parameters */
#  define INOUT   /* consider as comments near 'input/output' parameters */
#endif

#ifndef Zero
#  define Zero        0
#  define One         1
#endif

#ifndef Bool
#  define Bool        int
#  define False       0
#  define True        1
#endif

/********************************************
 * constants: default values and linitations
 *********************************************/
 
/* bridge configuration */

#define DEF_BR_PRIO 32768
#define MIN_BR_PRIO 0
#define MAX_BR_PRIO 61440

#define DEF_BR_HELLOT   2
#define MIN_BR_HELLOT   1
#define MAX_BR_HELLOT   10

#define DEF_BR_MAXAGE   20
#define MIN_BR_MAXAGE   6
#define MAX_BR_MAXAGE   40

#define DEF_BR_FWDELAY  15
#define MIN_BR_FWDELAY  4
#define MAX_BR_FWDELAY  30
/*mstp*/
#define DEF_REMAINING_HOPS 20
#define MIN_REMAINING_HOPS 6
#define MAX_REMAINING_HOPS 40
/*end mstp*/
#define DEF_FORCE_VERS  2 /* NORMAL_RSTP */

/* port configuration */

#define DEF_PORT_PRIO   128
#define MIN_PORT_PRIO   0
#define MAX_PORT_PRIO   240 /* in steps of 16 */

#ifndef DEF_ADMIN_NON_STP 
#define DEF_ADMIN_NON_STP   False
#endif
#define DEF_ADMIN_EDGE      True 
#define DEF_LINK_DELAY      3 /* see edge.c */
#define DEF_P2P         P2P_AUTO_E
#if 0
#define PORT_INDEX_MAX  /* 259*/327
#else
#define PORT_INDEX_MAX  640  /* for distributed system, 640 = 64 * 10 */
#endif

#define MAX_MST_ID 64


#define STP_DBUS_DEBUG(x) stp_syslog_dbg x

struct port_speed_node{
    unsigned int port_index;
	unsigned int port_speed;
};

struct stp_admin_infos {
	unsigned int 			stpEnable;
	unsigned int 	mstid[MAX_MST_ID];
	unsigned int 	pathcost[MAX_MST_ID];
	unsigned int 	prio[MAX_MST_ID];
	unsigned int 	p2p;
	unsigned int 	edge;
	unsigned int	nonstp;
};


/* Section 1: Create/Delete/Start/Stop the RSTP instance */

void /* init the engine */
stp_in_init (int max_port_index);

/* Section 2. "Get" management */

Bool
STP_IN_get_is_stpm_enabled (int vlan_id);

int
STP_IN_stpm_get_vlan_id_by_name (char* name, int* vlan_id);

int
STP_IN_stpm_get_name_by_vlan_id (int vlan_id, char* name, size_t buffsize);

const char*
stp_in_get_error_explanation (int rstp_err_no);

#ifdef _UID_STP_H__
int
stp_in_stpm_get_cfg (struct stpm_t* this, UID_STP_CFG_T* uid_cfg);
/*stp_in_stpm_get_cfg (int vlan_id, UID_STP_CFG_T* uid_cfg);*/

int
STP_IN_port_get_state (int vlan_id, UID_STP_PORT_STATE_T* entry);

int stp_in_enable_port_on_stpm 
(
	int port_index, Bool enable,
	unsigned int link_state,
	unsigned int speed, 
	unsigned int isWAN,
    unsigned int slot_no,
    unsigned int port_no
);
#endif

/* Section 3. "Set" management */

int
stp_in_stpm_set_cfg (struct stpm_t*  this,
                     UID_STP_CFG_T* uid_cfg);
/*stp_in_stpm_set_cfg (int vlan_id,
                     BITMAP_T* port_bmp,
                     UID_STP_CFG_T* uid_cfg);*/

int
stp_in_set_port_cfg (struct port_t*port,
                     UID_STP_PORT_CFG_T* uid_cfg);

int 
stp_in_reinit_port_cfg ( int vlan_id, 
	 int port_index,
	 UID_STP_PORT_CFG_T* uid_cfg);

#ifdef STP_DBG
int stp_in_dbg_set_port_trace (char* mach_name, int enadis,
                               int vlan_id, BITMAP_T* ports, short mstid,
                               int is_print_err);
#endif

/* Section 4. RSTP functionality events */

int 
stp_in_one_second (void);


int /* call it, when port speed has been changed, speed in Kb/s  */
stp_in_change_port_speed (int port_index, long speed);

int /* call it, when current port duplex mode has been changed  */
stp_in_change_port_duplex (int port_index);

#ifdef _STP_BPDU_H__
int
stp_in_check_bpdu_header (PORT_T* port, MSTP_BPDU_T* bpdu, size_t len, unsigned char  *bpdu_type);
/*stp_in_check_bpdu_header (BPDU_T* bpdu, size_t len);*/

int
stp_in_rx_bpdu 
(
	unsigned int vlan_id, 
	unsigned int port_index, 
	unsigned int slot_no, 
	unsigned int port_no, 
	BPDU_T* bpdu, 
	size_t len
);
#endif

#ifdef _STP_MACHINE_H__
/* Inner usage definitions & functions */

extern int max_port;

#ifdef __LINUX__
#  define RSTP_INIT_CRITICAL_PATH_PROTECTIO
#  define RSTP_CRITICAL_PATH_START
#  define RSTP_CRITICAL_PATH_END
#else
#  define RSTP_INIT_CRITICAL_PATH_PROTECTIO STP_OUT_psos_init_semaphore ()
#  define RSTP_CRITICAL_PATH_START          STP_OUT_psos_close_semaphore ()
#  define RSTP_CRITICAL_PATH_END            STP_OUT_psos_open_semaphore ()
   extern void STP_OUT_psos_init_semaphore (void);
   extern void STP_OUT_psos_close_semaphore (void);
   extern void STP_OUT_psos_open_semaphore (void);
#endif
int stp_in_global_enable();

int stp_in_global_disable();


STPM_T* stp_in_stpm_find (int vlan_id);

int  stp_in_add_vlan_to_cist (int vid, BITMAP_T port_bmp);

int stp_in_del_vlan_from_cist(int vid);

int stp_in_del_port_from_mstp(int vid,unsigned int port_index);

int stp_in_add_port_to_mstp(int vid,unsigned int port_index,unsigned int isWan);

int stp_in_set_vlan_2_instance (unsigned short vlan_id, short mstid);

int stp_mgmt_build_configid_digest();

#endif /* _STP_MACHINE_H__ */

void stp_mgmt_init_vlanpbmp(  unsigned short VID, BITMAP_T ports);

int stp_mgmt_stpm_create_cist ();

int stp_mgmt_stpm_update_instance ( unsigned short MSTID, unsigned short VID);

unsigned short stp_mgmt_get_mstid(int vid);

void stp_mgmt_del_all_mst();

#endif /* _STP_API_H__ */

