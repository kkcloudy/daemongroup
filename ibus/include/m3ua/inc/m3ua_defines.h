/****************************************************************************
** Description:
** Some common defines used in the code.
*****************************************************************************
** Copyright(C) 2009 Shabd Communications Pvt. Ltd. http://www.shabdcom.org
*****************************************************************************
** Contact:
** vkgupta@shabdcom.org
*****************************************************************************
** License :
**
** This library is free software; you can redistribute it and/or
** modify it under the terms of the GNU Lesser General Public
** License as published by the Free Software Foundation; either
** version 2.1 of the License, or (at your option) any later version.
**
** This library is distributed in the hope that it will be useful,
** but WITHOUT ANY WARRANTY; without even the implied warranty of
** MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
** Lesser General Public License for more details.
**
** You should have received a copy of the GNU Lesser General Public
** License along with this library; if not, write to the Free Software
** Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
*****************************************************************************/

#ifndef __M3UA_DEFINES_H__
#define __M3UA_DEFINES_H__

#include <netinet/in.h>
#include <string.h>


#define M3_MAX_U32                             0xFFFFFFFF
#define M3_MAX_U16                             0xFFFF
#define M3_MAX_U8                              0xFF

#define M3_TRUE                                1
#define M3_FALSE                               0

#define M3_NULL                                0

/**************************** MO operations ***************************/

#define M3_ADD                                 1
#define M3_DELETE                              2
#define M3_GET                                 3
#define M3_MODIFY                              4

#define M3_REGISTER                            1
#define M3_DEREGISTER                          2
#define M3_STATUS                              3

/**************************** Memory Pools ****************************/

#define M3_ASPM_POOL                           0
#define M3_TIMER_POOL                          1
#define M3_SSNM_POOL                           2
#define M3_TXR_POOL                            3
#define M3_MGMT_POOL                           4
#define M3_PD_BUF_POOL                         5

#define M3_NUM_ASPM_POOL_BUFF                  32
#define M3_MAX_ASPM_BUFF_SIZE                  512

#define M3_NUM_TIMER_POOL_BUFF                 16
#define M3_MAX_TIMER_BUFF_SIZE                 128

#define M3_NUM_SSNM_POOL_BUFF                  8
#define M3_MAX_SSNM_BUFF_SIZE                  1024

/******** Presently only transfer pool management is supported ********/
#define M3_NUM_TXR_POOL_BUFF                   16
#define M3_MAX_TXR_BUFF_SIZE                   1024

#define M3_NUM_MGMT_POOL_BUFF                  16
#define M3_MAX_MGMT_BUFF_SIZE                  512

#define M3_NUM_PD_POOL_BUFF                    2048
#define M3_MAX_PD_BUFF_SIZE                    1024

/**************************** Message Class ****************************/
#define M3_MSG_CLASS_MGMT                      0x00
#define M3_MSG_CLASS_TXR                       0x01
#define M3_MSG_CLASS_SSNM                      0x02
#define M3_MSG_CLASS_ASPSM                     0x03
#define M3_MSG_CLASS_ASPTM                     0x04
#define M3_MSG_CLASS_RKM                       0x09

/************************ Message Type ***************************/

#define M3_MSG_TYPE_ERR                        0x00
#define M3_MSG_TYPE_NTFY                       0x01

#define M3_MSG_TYPE_DATA                       0x01

#define M3_MSG_TYPE_DUNA                       0x01
#define M3_MSG_TYPE_DAVA                       0x02
#define M3_MSG_TYPE_DAUD                       0x03
#define M3_MSG_TYPE_SCON                       0x04
#define M3_MSG_TYPE_DUPU                       0x05
#define M3_MSG_TYPE_DRST                       0x06

#define M3_MSG_TYPE_ASPUP                      0x01
#define M3_MSG_TYPE_ASPDN                      0x02
#define M3_MSG_TYPE_BEAT                       0x03
#define M3_MSG_TYPE_ASPUP_ACK                  0x04
#define M3_MSG_TYPE_ASPDN_ACK                  0x05
#define M3_MSG_TYPE_BEAT_ACK                   0x06

#define M3_MSG_TYPE_ASPAC                      0x01
#define M3_MSG_TYPE_ASPIA                      0x02
#define M3_MSG_TYPE_ASPAC_ACK                  0x03
#define M3_MSG_TYPE_ASPIA_ACK                  0x04

#define M3_MSG_TYPE_REG_REQ                    0x01
#define M3_MSG_TYPE_REG_RSP                    0x02
#define M3_MSG_TYPE_DEREG_REQ                  0x03
#define M3_MSG_TYPE_DEREG_RSP                  0x04

/********************** Message Tags ***********************/

#ifndef __LITTLE_ENDIAN__

#define M3_TAG_INFO_STR                        0x0004
#define M3_TAG_RT_CTX                          0x0006
#define M3_TAG_DIAG_INFO                       0x0007
#define M3_TAG_HBEAT_DATA                      0x0009
#define M3_TAG_TRFMODE                         0x000b
#define M3_TAG_ERR_CODE                        0x000c
#define M3_TAG_STATUS                          0x000d
#define M3_TAG_ASP_ID                          0x0011
#define M3_TAG_AFF_PC                          0x0012
#define M3_TAG_CRN_ID                          0x0013
#define M3_TAG_NW_APP                          0x0200
#define M3_TAG_USR_CAUSE                       0x0204
#define M3_TAG_CONG_IND                        0x0205
#define M3_TAG_CRND_DPC                        0x0206
#define M3_TAG_RKEY                            0x0207
#define M3_TAG_REG_RESULT                      0x0208
#define M3_TAG_DEREG_RESULT                    0x0209
#define M3_TAG_LRK_ID                          0x020a
#define M3_TAG_DPC                             0x020b
#define M3_TAG_SI                              0x020c
#define M3_TAG_OPC_LIST                        0x020e
#define M3_TAG_CKT_RANGE                       0x020f
#define M3_TAG_PROT_DATA                       0x0210
#define M3_TAG_REG_STATUS                      0x0211
#define M3_TAG_DEREG_STATUS                    0x0212

#else

#define M3_TAG_INFO_STR                        0x0400
#define M3_TAG_RT_CTX                          0x0600
#define M3_TAG_DIAG_INFO                       0x0700
#define M3_TAG_HBEAT_DATA                      0x0900
#define M3_TAG_TRFMODE                         0x0b00
#define M3_TAG_ERR_CODE                        0x0c00
#define M3_TAG_STATUS                          0x0d00
#define M3_TAG_ASP_ID                          0x1100
#define M3_TAG_AFF_PC                          0x1200
#define M3_TAG_CRN_ID                          0x1300
#define M3_TAG_NW_APP                          0x0002
#define M3_TAG_USR_CAUSE                       0x0402
#define M3_TAG_CONG_IND                        0x0502
#define M3_TAG_CRND_DPC                        0x0602
#define M3_TAG_RKEY                            0x0702
#define M3_TAG_REG_RESULT                      0x0802
#define M3_TAG_DEREG_RESULT                    0x0902
#define M3_TAG_LRK_ID                          0x0a02
#define M3_TAG_DPC                             0x0b02
#define M3_TAG_SI                              0x0c02
#define M3_TAG_OPC_LIST                        0x0e02
#define M3_TAG_CKT_RANGE                       0x0f02
#define M3_TAG_PROT_DATA                       0x1002
#define M3_TAG_REG_STATUS                      0x1202
#define M3_TAG_DEREG_STATUS                    0x1302

#endif


#define M3_MIN_REG_RESULT_LEN                  24
#define M3_MIN_DEREG_RESULT_LEN                16
#define M3_MIN_RKEY_LEN                        16
#define M3_RT_LBL_LEN                          12
#define M3_MSG_HEADER_SIZE                     8
#define M3_MSG_VERSION                         1


#define M3_STATUS_TYP_AS_STATE_CHG             1
#define M3_STATUS_TYP_OTHER                    2

#define M3_STATUS_INF_AS_INACTIVE              2
#define M3_STATUS_INF_AS_ACTIVE                3
#define M3_STATUS_INF_AS_PENDING               4
#define M3_STATUS_INF_INSUFF_ASP_ACT           1
#define M3_STATUS_INF_ALT_ASP_ACT              2
#define M3_STATUS_INF_ASP_FAILURE              3

/************* Error codes **************/
#define EM3_INV_VERSION                        0x01
#define EM3_UNSUPP_MSG_CLASS                   0x03
#define EM3_UNSUPP_MSG_TYPE                    0x04
#define EM3_INV_TRFMODE                        0x05
#define EM3_UNEXP_MSG                          0x06
#define EM3_PROT_ERR                           0x07
#define EM3_INV_STRMID                         0x09
#define EM3_REF_MGMT_BLOCK                     0x0d
#define EM3_ASPID_REQD                         0x0e
#define EM3_INV_M3ASPID                        0x0f
#define EM3_INV_PARAM_VAL                      0x11
#define EM3_PARAM_FIELD                        0x12
#define EM3_UNEXP_PARAM                        0x13
#define EM3_DST_STATUS_UNKNOWN                 0x14
#define EM3_INV_NA                             0x15
#define EM3_MISSING_PARAM                      0x16
#define EM3_INV_RC                             0x19
#define EM3_NO_AS_CONF_FOR_ASP                 0x1a

/************** cause *****************/
#define M3_CAUSE_UNKNOWN                       0
#define M3_CAUSE_UNEQUPPD_RMT_USR              1
#define M3_CAUSE_INACCESS_RMT_USR              2
#define M3_CAUSE_CONG                          3
#define M3_CAUSE_DRST                          4

/************** Service Information Octet *************/
#define M3_SIO_ISUP                            5
#define M3_SIO_SCCP                            3

/************** Default Timer durations ****************/
#ifndef M3_TICKS_PER_SEC
#define M3_TICKS_PER_SEC			1
#endif

#define M3_PD_TIMER_INT                        m3_timer_table.pdtimer.dur
#define M3_ASPM_TIMER_INT_LOW                  m3_timer_table.aspmtimer.l_dur
#define M3_ASPM_TIMER_INT_HIGH                 m3_timer_table.aspmtimer.h_dur
#define M3_HBEAT_TIMER_INT                     m3_timer_table.hbeattimer.dur
#define M3_RKM_TIMER_INT                       m3_timer_table.rkmtimer.dur

/************** Default Retry counts *****************/
#define M3_ASPM_RETRY_LOW                      m3_timer_table.aspmtimer.sw_try
#define M3_ASPM_MAX_RETRY                      m3_timer_table.aspmtimer.retry
#define M3_HBEAT_MAX_RETRY                     m3_timer_table.hbeattimer.retry
#define M3_RKM_MAX_RETRY                       m3_timer_table.rkmtimer.retry

/************** SG modes ****************/
#define M3_SGMODE_LOADSHARE                    0x01
#define M3_SGMODE_BROADCAST                    0x02

/************** AS modes ****************/
#define M3_TRFMODE_OVERRIDE        1
#define M3_TRFMODE_LOAD_SHARE      2
#define M3_TRFMODE_BROADCAST       3

#define M3_TRFMODE_VALID(mode)	((M3_TRFMODE_OVERRIDE == mode || M3_TRFMODE_LOAD_SHARE == mode || M3_TRFMODE_BROADCAST == mode) ? M3_TRUE:M3_FALSE)

/************** ASP states ***************/

#define M3_ASP_DOWN				0
#define M3_ASP_INACTIVE				1
#define M3_ASP_ACTIVE				2

#define M3_ASP_DNSENT				3 
#define M3_ASP_UPSENT                           4

#define M3_ASP_ACSENT                           5
#define M3_ASP_IASENT                           6

#define M3_MAX_ASP_STATE			5

/*************** Remote ASP states ***************/

#define M3_ASP_DOWN                             0
#define M3_ASP_INACTIVE                         1
#define M3_ASP_ACTIVE                           2

#define M3_ASP_DNRECV                           3
#define M3_ASP_UPRECV                           4

#define M3_MAX_ASP_STATE			5

/*************** AS states ****************/

#define M3_AS_DOWN				1
#define M3_AS_INACTIVE				2
#define M3_AS_ACTIVE				3
#define M3_AS_PENDING				4

#define M3_MAX_AS_STATE				4

/************** RK states ****************/

#define M3_RK_STATIC				0
#define M3_RK_REG_IN_PROG			1
#define M3_RK_REGD				2
#define M3_RK_DEREG_IN_PROG			3

/** Internal Registration/Deregistration status ****************/

#define M3_REG_STATUS_TIMEOUT			51
#define M3_DEREG_STATUS_TIMEOUT			101

/************** Types of management notifications **************/
#define M3_MGMT_NTFY_ASP_STATE                 1
#define M3_MGMT_NTFY_AS_STATE                  2
#define M3_MGMT_NTFY_R_ASP_STATE               3
#define M3_MGMT_NTFY_R_AS_STATE                4
#define M3_MGMT_NTFY_CONN_STATE                5
#define M3_MGMT_NTFY_NOTIFY                    6
#define M3_MGMT_NTFY_ERR                       7
#define M3_MGMT_NTFY_REGISTER                  8
#define M3_MGMT_NTFY_DEREGISTER                9
#define M3_MGMT_NTFY_REG_STATUS                10
#define M3_MGMT_NTFY_DEREG_STATUS              11

/************** Types of user notifications ****************/
#define M3_USER_NTFY_PAUSE                     1
#define M3_USER_NTFY_RESUME                    2
#define M3_USER_NTFY_STATUS                    3
#define M3_USER_NTFY_TRANSFER                  4
#define M3_USER_NTFY_AUDIT                     5

/*************** few imp. constants ****************/
#define M3_MAX_LOCAL_ROUTE			256
#define M3_MAX_ROUTE                            256
#define M3_MAX_INFO_STR_LEN                     256
#define M3_MSG_HDR_LEN                          8

/************** User configurable defines ***************/

#ifndef M3_MAX_RTCTX
#define M3_MAX_RTCTX				16
#endif

#ifndef M3_MAX_DPC_PER_RK
#define M3_MAX_DPC_PER_RK                       1
#endif

#ifndef M3_MAX_SI_PER_RK
#define M3_MAX_SI_PER_RK			4
#endif

#ifndef M3_MAX_OPC_PER_RK
#define M3_MAX_OPC_PER_RK			64
#endif

#ifndef M3_MAX_CKT_RANGE_PER_RK
#define M3_MAX_CKT_RANGE_PER_RK			16
#endif

#ifndef M3_MAX_TLV_VAL_SIZE
#define M3_MAX_TLV_VAL_SIZE			256
#endif

#ifndef M3_MAX_PROT_DATA_LEN
#define	M3_MAX_PROT_DATA_LEN			512
#endif

#ifndef M3_MAX_PC_SSNM
#define M3_MAX_PC_SSNM				64
#endif

#ifndef M3_MAX_RK_PER_RKM
#define M3_MAX_RK_PER_RKM			16
#endif

#ifndef M3_MAX_ASPM_SESS_PER_CONN
#define M3_MAX_ASPM_SESS_PER_CONN		8
#endif

#ifndef M3_MAX_RKM_SESS_PER_CONN
#define M3_MAX_RKM_SESS_PER_CONN                4
#endif

#ifndef M3_MAX_RK
#define M3_MAX_RK				M3_MAX_RK_PER_RKM
#endif

#ifndef M3_MAX_AS
#define M3_MAX_AS				8
#endif

#ifndef M3_MAX_ASP
#define M3_MAX_ASP				64
#endif

#ifndef M3_MAX_R_ASP
#define M3_MAX_R_ASP				128
#endif

#ifndef M3_MAX_R_SGP
#define M3_MAX_R_SGP				32
#endif

#ifndef M3_MAX_SGP
#define M3_MAX_SGP				4
#endif

#ifndef M3_MAX_R_AS
#define M3_MAX_R_AS				64
#endif

#ifndef M3_MAX_SG
#define M3_MAX_SG				8
#endif

#ifndef M3_MAX_CONN
#define M3_MAX_CONN				128
#endif

#ifndef M3_MAX_ASSOCID
#define M3_MAX_ASSOCID				512
#endif

#ifndef M3_MAX_NWAPP
#define M3_MAX_NWAPP				8
#endif

#ifndef M3_MAX_USR
#define M3_MAX_USR				32
#endif

#ifndef M3_MAX_SGP_PER_SG
#define M3_MAX_SGP_PER_SG			4
#endif

/* XXX - DEC2009 */
#ifndef M3_MAX_ROUTES_PER_SG
#define M3_MAX_ROUTES_PER_SG		64
#endif

#ifndef M3_MAX_MGMT_NTFY
#define M3_MAX_MGMT_NTFY                        8
#endif

#ifndef M3_MAX_USER_NTFY
#define M3_MAX_USER_NTFY			32
#endif

#ifndef M3_MAX_USER_DATA_SIZE
#define M3_MAX_USER_DATA_SIZE                   272
#endif

#ifndef M3_MAX_HBEAT_DATA_SIZE
#define M3_MAX_HBEAT_DATA_SIZE			256
#endif

#ifndef M3_MAX_TIMERS
#define M3_MAX_TIMERS				256
#endif

#ifndef M3_DEF_R_AS_TRFMODE
#define M3_DEF_R_AS_TRFMODE			M3_TRFMODE_BROADCAST
#endif

/************** Number of memory buffers available to M3UA **********/
/************** Scale these as per the application requirement **********/

#ifndef M3_NUM_32BYTE_BUFS
#define M3_NUM_32BYTE_BUFS	64
#endif

#ifndef M3_NUM_64BYTE_BUFS
#define M3_NUM_64BYTE_BUFS	256
#endif

#ifndef M3_NUM_128BYTE_BUFS
#define M3_NUM_128BYTE_BUFS	256
#endif

#ifndef M3_NUM_256BYTE_BUFS
#define M3_NUM_256BYTE_BUFS	256
#endif

#ifndef M3_NUM_512BYTE_BUFS
#define M3_NUM_512BYTE_BUFS	128
#endif

#ifndef M3_NUM_1024BYTE_BUFS
#define M3_NUM_1024BYTE_BUFS	64
#endif

#ifndef M3_NUM_2048BYTE_BUFS
#define M3_NUM_2048BYTE_BUFS	32
#endif

#ifndef M3_NUM_4096BYTE_BUFS
#define M3_NUM_4096BYTE_BUFS	16
#endif

#ifndef M3_NUM_8192BYTE_BUFS
#define M3_NUM_8192BYTE_BUFS	16
#endif


/************** maximum message sizes ********************/
#define M3_MAX_NTFY_SIZE                        292 + (4 * M3_MAX_RTCTX)
#define M3_MAX_ASPDNACK_SIZE                    268
#define M3_MAX_ASPUPACK_SIZE                    268
#define M3_MAX_ASPUP_SIZE                       276
#define M3_MAX_ASPDN_SIZE                       268
#define M3_MAX_ASPIAACK_SIZE                    272 + (4 * M3_MAX_RTCTX)
#define M3_MAX_ASPAC_SIZE                       280 + (4 * M3_MAX_RTCTX)
#define M3_MAX_ASPIA_SIZE                       272 + (4 * M3_MAX_RTCTX)
#define M3_MAX_ASPACACK_SIZE                    280 + (4 * M3_MAX_RTCTX)
#define M3_MAX_DRST_SIZE                        284 + (4 * M3_MAX_RTCTX) + \
                                                (4 * M3_MAX_PC_SSNM)
#define M3_MAX_SCON_SIZE                        300 + (4 * M3_MAX_RTCTX) + \
                                                (4 * M3_MAX_PC_SSNM)
#define M3_MAX_DUPU_SIZE                        296 + (4 * M3_MAX_RTCTX)
#define M3_MAX_DAVA_SIZE                        284 + (4 * M3_MAX_RTCTX) + \
                                                (4 * M3_MAX_PC_SSNM)
#define M3_MAX_DUNA_SIZE                        284 + (4 * M3_MAX_RTCTX) + \
                                                (4 * M3_MAX_PC_SSNM)
#define M3_MAX_DAUD_SIZE                        284 + (4 * M3_MAX_RTCTX) + \
                                                (4 * M3_MAX_PC_SSNM)
#define M3_MAX_DATA_SIZE                        36 + M3_MAX_USER_DATA_SIZE
#define M3_MAX_HBEAT_SIZE                       12 + M3_MAX_HBEAT_DATA_SIZE
#define M3_MAX_HBEAT_ACK_SIZE                   12 + M3_MAX_HBEAT_DATA_SIZE
#define M3_MAX_ERROR_SIZE                       1024
#define M3_MAX_REGRSP_SIZE                      8 + (28 * M3_MAX_RK)
#define M3_MAX_REGREQ_SIZE                      2048
#define M3_MAX_DEREGREQ_SIZE                    12 + (4 * M3_MAX_RK)
#define M3_MAX_DEREGRSP_SIZE                    8 + (8 * M3_MAX_RK)
/************** API Identifier ******************/

#define M3UA_NWAPP				1
#define M3UA_AS					2
#define M3UA_ASP				3
#define M3UA_SGP				4
#define M3UA_R_ASP				5
#define M3UA_R_AS				6
#define M3UA_R_SGP				7
#define M3UA_SG					8
#define M3UA_CONN				9
#define M3UA_CONN_STATE				10
#define M3UA_ASP_STATE				11
#define M3UA_ROUTE                              12
#define M3UA_R_ASPLOCK				13
#define M3UA_USER                               14
#define M3UA_TRANSFER                           15
#define M3UA_PAUSE                              16
#define M3UA_RESUME                             17
#define M3UA_STATUS                             18
#define M3UA_AUDIT                              19
#define M3UA_TIMER                              20
#define M3UA_HEARTBEAT				21
#define M3UA_R_ASP_STATE			22
#define M3UA_R_AS_STATE				23
#define M3UA_RKEY				24


/**************** Diagnostics *************/
#define M3_MEM_DIAG						(0x00000001)
#define M3_TIMER_DIAG					(0x00000002)
#define M3_ALL_DIAG						(0x000000FF)

/************** Macros ******************/
#ifndef __LITTLE_ENDIAN__
#define M3_IS_TAG_VALID(tag)      (((0x14 < (m3_u8)(tag)) || \
    (0x00 != (m3_u8)(tag >> 8) && 0x02 != (m3_u8)(tag >> 8)))?M3_FALSE:M3_TRUE)
#else
#define M3_IS_TAG_VALID(tag)      (((0x14 < (m3_u8)(tag >> 8)) || \
    (0x00 != (m3_u8)tag && 0x02 != (m3_u8)tag))?M3_FALSE:M3_TRUE)
#endif

#define M3_ALIGN_4(val)		val = ((val + 3)/4) * 4;

#ifdef __LITTLE_ENDIAN__
#define M3_HTONL(val)           htonl(val)
#else
#define M3_HTONL(val)           val
#endif

#ifdef __LITTLE_ENDIAN__
#define M3_HTONS(val)           htons(val)
#else
#define M3_HTONS(val)           val
#endif

#ifdef __LITTLE_ENDIAN__
#define M3_NTOHL(val)           ntohl(val)
#else
#define M3_NTOHL(val)           val
#endif

#ifdef __LITTLE_ENDIAN__
#define M3_NTOHS(val)           ntohs(val)
#else
#define M3_NTOHS(val)           val
#endif

#define M3_IS_SP_ASP(id)        ((0x00000000 == (0xFFFF0000 & id)) ? M3_TRUE:M3_FALSE)
#define M3_IS_SP_SGP(id)        ((0x00010000 == (0xFFFF0000 & id)) ? M3_TRUE:M3_FALSE)
#define M3_IS_LE_AS(id)         ((0x00000000 == (0xFFFF0000 & id)) ? M3_TRUE:M3_FALSE)
#define M3_IS_LE_SG(id)         ((0x00010000 == (0xFFFF0000 & id)) ? M3_TRUE:M3_FALSE)

#define M3_ASPID_VALID(id)	((M3_MAX_ASP > (id & 0x0000FFFF)) ? M3_TRUE:M3_FALSE)
#define M3_SGPID_VALID(id)	((M3_MAX_SGP > (id & 0x0000FFFF)) ? M3_TRUE:M3_FALSE)
#define M3_R_ASPID_VALID(id)	((M3_MAX_R_ASP > (id & 0x0000FFFF)) ? M3_TRUE:M3_FALSE)
#define M3_R_SGPID_VALID(id)	((M3_MAX_R_SGP > (id & 0x0000FFFF)) ? M3_TRUE:M3_FALSE)
#define M3_ASID_VALID(id)	((M3_MAX_AS > (id & 0x0000FFFF)) ? M3_TRUE:M3_FALSE)
#define M3_R_ASID_VALID(id)	((M3_MAX_R_AS > (id & 0x0000FFFF)) ? M3_TRUE:M3_FALSE)
#define M3_SGID_VALID(id)	((M3_MAX_SG > (id & 0x0000FFFF)) ? M3_TRUE:M3_FALSE)
#define M3_USRID_VALID(id)	((M3_MAX_USR > (id & 0x0000FFFF)) ? M3_TRUE:M3_FALSE)


#endif /* __M3_DEFS_H__ */

