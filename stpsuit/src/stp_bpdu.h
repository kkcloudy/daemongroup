/* BPDU formats: 9.1 - 9.3, 17.28 */
#ifndef _STP_BPDU_H__
#define _STP_BPDU_H__

#define MIN_BPDU                7
#define BPDU_L_SAP              0x42
#define LLC_UI                  0x03
#define BPDU_PROTOCOL_ID        0x0000
#define BPDU_VERSION_ID         0x00
#define BPDU_VERSION_RAPID_ID   0x02
#define BPDU_VERSION_MULTI_ID    0x03   /*mstp*/

#define BPDU_TOPO_CHANGE_TYPE   0x80
#define BPDU_CONFIG_TYPE        0x00
#define BPDU_RSTP               0x02
#define BPDU_MSTP               0x03   /*mstp*/

#define TOLPLOGY_CHANGE_BIT     0x01
#define PROPOSAL_BIT            0x02
#define PORT_ROLE_OFFS          2   /* 0x04 & 0x08 */
#define PORT_ROLE_MASK          (0x03 << PORT_ROLE_OFFS)
#define LEARN_BIT               0x10
#define FORWARD_BIT             0x20
#define AGREEMENT_BIT           0x40
#define TOLPLOGY_CHANGE_ACK_BIT 0x80
#define MASTER_BIT              0x80    /*mstp*/

#define RSTP_PORT_ROLE_UNKN     0x00
#define MSTP_PORT_ROLE_MASTER 0x00    /*mstp*/
#define RSTP_PORT_ROLE_ALTBACK  0x01
#define RSTP_PORT_ROLE_ROOT     0x02
#define RSTP_PORT_ROLE_DESGN    0x03

typedef struct mac_header_t {
  unsigned char dst_mac[6];
  unsigned char src_mac[6];
} MAC_HEADER_T;

typedef struct eth_header_t {
  unsigned char len8023[2];
  unsigned char dsap;
  unsigned char ssap;
  unsigned char llc;
} ETH_HEADER_T;

typedef struct bpdu_header_t {
  unsigned char protocol[2];
  unsigned char version;
  unsigned char bpdu_type;
} BPDU_HEADER_T;

typedef struct bpdu_body_t {
  unsigned char flags;
  unsigned char root_id[8];
  unsigned char root_path_cost[4];
  unsigned char bridge_id[8];
  unsigned char port_id[2];
  unsigned char message_age[2];
  unsigned char max_age[2];
  unsigned char hello_time[2];
  unsigned char forward_delay[2];
} BPDU_BODY_T;
/*mstp*/
typedef struct msti_cfg_msg_t 
{
    unsigned char    msti_flags;
    unsigned char    msti_region_root_id[8];
    unsigned char    msti_internal_root_path_cost[4];
    unsigned char    msti_bridge_priority;
    unsigned char    msti_port_priority ;
    unsigned char    msti_remaining_hops;
}MSTI_CFG_MSG_T;

typedef struct msti_cisco_cfg_msg_t 
{
	unsigned char 	 msti_id;
    unsigned char    msti_flags;
    unsigned char    msti_region_root_id[8];
    unsigned char    msti_internal_root_path_cost[4];
    unsigned char    msti_bridge_priority[8];
    unsigned char    msti_port_priority[2];
    unsigned char    msti_remaining_hops;
	unsigned char 	 rsvd; /* reserved */
}MSTI_Cisco_CFG_MST_T;

typedef struct Mst_Configuration_Identifier /* 13.7 */
{
    unsigned char    FormatSelector ;
    unsigned char    ConfigurationName[32] ;
    unsigned char    RevisionLevel[2] ;
    unsigned char    ConfigurationDigest[16] ;
}MST_CFG_ID_S;

typedef struct Mst_Cisco_Configuration_Identifier /* 13.7 for cisco */
{
    unsigned char    ConfigurationName[32] ;
    unsigned char    RevisionLevel[2] ;
    unsigned char    ConfigurationDigest[16] ;
}MST_Cisco_CFG_ID_S;

/*mstp*/
typedef struct stp_bpdu_t {
  MAC_HEADER_T  mac;   /*12*/
  ETH_HEADER_T  eth;     /*5*/
  BPDU_HEADER_T hdr;    /*4*/
  BPDU_BODY_T   body;	/*31*/
  unsigned char ver_1_len[2]; 
} BPDU_T;
/*mstp*/
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
  unsigned char ver_1_length;
} RSTP_BPDU_T;  /*53B*/

typedef struct tx_mstp_bpdu_t{
  MAC_HEADER_T  mac;
  ETH_HEADER_T  eth;
  BPDU_HEADER_T hdr;
  BPDU_BODY_T   body;
  unsigned char ver_1_len;
  /*cist*/
  unsigned char version_3_length[2];
  MST_CFG_ID_S MSTCID;  /*51B*/
  unsigned char cist_inter_path_cost[4];
  unsigned char cist_bridge_id[8];
  unsigned char cist_remaining_hops;
  /*msti*/
  MSTI_CFG_MSG_T MCfgMsg[64];  /*16*64*/
 }MSTP_BPDU_T ; /*1143B*/

typedef struct tx_mstp_cisco_bpdu_t{
	MAC_HEADER_T  mac; /* 12B */
	ETH_HEADER_T  eth; /* 5B */
	BPDU_HEADER_T hdr; /* 4B */
	BPDU_BODY_T   body;/* 31B */
	unsigned char ver_1_len;
	unsigned char rsvd1;/* reserved */
	unsigned char version_3_length[2];
	MST_Cisco_CFG_ID_S ciscoMSTID; /* 50B */
	unsigned char cist_region_root_id[8];
	unsigned char cist_inter_path_cost[4];
	unsigned char cist_remaining_hops;
	unsigned char rsvd2;/* reserved */
	
	/* msti configuration message */
	MSTI_Cisco_CFG_MST_T MCfgMsg[64]; /* 26B*64 */
}MSTP_Cisco_BPDU_T; /* 1784B */

typedef struct stp_bpdu_record_t {
  BPDU_T *bpdu;
  int    bpdu_type;
  unsigned char  digestSnp;/* current packet from digest snooping port */  
} BPDU_RECORD_T;
/*mstp*/
#endif /* _STP_BPDU_H__ */

