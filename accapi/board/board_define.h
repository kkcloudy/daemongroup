#ifndef __BOARD_DEFINE_H_
#define __BOARD_DEFINE_H_

#define MAX_BOARD_NAME_LEN			32	
#define MAX_OBC_PORT_NUM			10
#define	MAX_CSCD_PORT_NUM			10
#define MAX_PORT_NUM_PER_BOARD		48

#define SERIAL_NUMBER_LEN 10

#define IS_MASTER	1
#define NOT_MASTER	!IS_MASTER

#define UNKONWN_SLOT_ID -1


#define ACTIVE_MASTER	1
#define NOT_ACTIVE_MASTER	0

#define MAX_BOARD_PORT_NAME_LEN		10
#define OBC_PORT	1
#define	NOT_OBC_PORT	!OBC_PORT

#define CSCD_PORT	1
#define NOT_CSCD_PORT	!CSCD_PORT

#define VALID_DBM_FLAG   1	/*data under /dbm is ready for use*/
#define INVALID_DBM_FLAG 0	/*data under /dbm is not ready*/

enum board_id
{
	BOARD_SOFT_ID_AX7605I_CRSMU,
	BOARD_SOFT_ID_AX7605I_2X12G12S,
	BOARD_SOFT_ID_AX7605I_1X12G12S,
	BOARD_SOFT_ID_AX81_SMU,
	BOARD_SOFT_ID_AX81_AC12C,
	BOARD_SOFT_ID_AX81_AC8C,
	BOARD_SOFT_ID_AX81_AC4X,
	BOARD_SOFT_ID_AX81_12X,
	BOARD_SOFT_ID_AX81_1X12G12S,
	BOARD_SOFT_ID_AX81_2X12G12S
};



enum board_state
{
	BOARD_INSERTED_AND_REMOVED, /*to describe the slot were insert a board,and the board is removed for some reasons.*/
	BOARD_EMPTY,
	BOARD_REMOVED,
	BOARD_INSERTED,
	BOARD_INITIALIZING,
	BOARD_READY,
	BOARD_RUNNING
};

enum board_function_type{
	UNKNOWN_BOARD = 0,
	MASTER_BOARD = 0x1,		/*master board*/
	AC_BOARD = 0x2,			/*wireless service board*/
	SWITCH_BOARD = 0x4,		/*switch board*/
	BASE_BOARD = 0x8,		/*Bas board*/
	NAT_BOARD = 0x10		/*NAT board*/
};

enum cpu_num
{
	SINGLE_CPU = 1,
	DUAL_CPU   = 2
};

/*product serial*/
enum
{
	PRODUCT_SERIAL_7 = 7,
	PRODUCT_SERIAL_8,
	PRODUCT_SERIAL_MAX
};

typedef struct env_state_s
{
    unsigned int fan_state;
	unsigned int fan_rpm;
	unsigned int power_state;
	int master_remote_temp;
	int master_inter_temp;
	int slave_remote_temp;
	int slave_inter_temp;
}env_state_t;

typedef struct sys_power_state
{
	unsigned int power_no;
	unsigned int power_supply;
}sys_power_state_t;

#endif
