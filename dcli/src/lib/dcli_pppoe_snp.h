#ifndef __DCLI_PPPOE_SNP_H__
#define __DCLI_PPPOE_SNP_H__

#define IFNAMESIZE 16

#define SET_FALG			1
#define CLR_FALG			0

#define ENABLE			1
#define DISABLE			0

#define PPPOE_SNP_LOG_DEBUG			0x01
#define PPPOE_SNP_LOG_INFO			0x02
#define PPPOE_SNP_LOG_ERROR			0x04	
#define PPPOE_SNP_LOG_DEFAULT		0x00
#define PPPOE_SNP_LOG_ALL			(PPPOE_SNP_LOG_DEBUG | PPPOE_SNP_LOG_INFO | PPPOE_SNP_LOG_ERROR)

#define MRU_MIN			60
#define MRU_MAX			1492

#define DISTRIBUTED_BOARD_SLOT_ID_FILE	"/dbm/local_board/slot_id"


#endif
