/*
 *	finename: aute_product_info.h
 *	Auther	: lutao
 *	define product type for all the autelan product.
 * */

#ifndef __AUTE_PRODUCT_INFO_H__
#define	__AUTE_PRODUCT_INFO_H__

#define CPLD1_BASE_ADDR 	0x1d010000
#define CPLD_PRODUCT_CTL 	0x1
#define CPLD_PRODUCT_TYPE   0x2
#define PRODUCT_TYPE_MASK   0x7

#define CPLD_MODULE_CTL	0x3

#define AUTELAN_BOARD_AX71_CRSMU_MODULE_NUM 	0x80

/* product id*/
typedef enum product_id_enum
{
	/*serial 7000*/
	AX_7605 = 0,
	/*serial 5000*/
	AX_5612I,
	AX_5612,
	AX_5612E,
	AX_5608,
	/*serail 4000*/
	AU_4624,
	AU_4524,
	AU_4626_P,
	AU_4524_P,
	/*serial 3000*/
	AU_3524,
	AU_3052,
	AU_3052E,
	AU_3028,
	AU_3028E,
	AU_3052_P,
	AU_3028_P,
	AU_3524_P,
	/*seraial 86*/
	AX_86xx,
	UNKNOWN_PRODUCT_ID,
}product_id;

/*
 * Autelan product type enum-type
 */
typedef enum board_type_s
{
	AUTELAN_BOARD_AX71_CRSMU,
	AUTELAN_BOARD_AX71_2X12G12S,
	AUTELAN_BOARD_AX81_SMU,
	AUTELAN_BOARD_AX81_AC8C,
	AUTELAN_BOARD_AX81_AC12C,
	AUTELAN_BOARD_AX81_2X12G12S,
	AUTELAN_BOARD_AX81_1X12G12S,
	AUTELAN_BOARD_AX81_AC_12X,
	AUTELAN_BOARD_AX81_AC_4X,
	AUTELAN_UNKNOWN_BOARD
	/* 
	 * if you want to add some new autelan product types
	 * please add it upon the AUTELAN_UNKNOWN_BOARD in order
	 * and add it in src/kernel2.6.32.27cn/include/linux/autelan_product.h synchronously
	 */
}board_type_t;


typedef enum se_mode_enum
{
	SE_MODE_STANDALONE,
	SE_MODE_COEXIST,		
}se_mode_t;

typedef struct product_info_t
{
 	product_id	product_type;
	se_mode_t  	se_mode;
	uint8_t 		board_type;
	uint8_t 		to_linux_group;/*Coexist: the group ID that Linux Core join; Standalone: useless */
	uint8_t 		from_linux_group; /*the group ID that  SE Core join, receive the packets come from POW interface (Coexist)or from PCI interface(Standalone)*/
	uint8_t 		panel_interface_group;	
	uint8_t 		back_interface_group;
	uint8_t          to_linux_fccp_group;
	uint8_t         rule_mem_size;

}product_info_t;


#define DEFAULT_PACKET_SSO_QUEUE 1
#define DEFAULT_PACKET_GRP 0

#define TO_LINUX_GROUP 15
#define TO_LINUX_FCCP_GROUP 14

#define FROM_LINUX_GROUP 2
#define PANEL_PORT_GROUP 0
#define BACK_PORT_GROUP 1

#define PORT_TYPE_PANEL 	1  /*To the panel */
#define PORT_TYPE_BACK   	2  /*To the switch chip, connect the backbone*/
#define PORT_TYPE_CONNECT 	3  /*Connect the other OCTEON directly*/
#define PORT_TYPE_POW 	4
#define PORT_TYPE_PCI 	5

#define PORT_USED 	0x1  
#define PORT_NOT_USED   	0x0

#define RULE_MEM_128M       1
#define RULE_MEM_256M       2
#define RULE_MEM_512M       3
#define RULE_MEM_1792M     4

typedef union
{
    uint64_t u64;
    struct
    {
        uint64_t    reserved_20_63  : 35;
        uint64_t    link_up         : 1;    /**< Is the physical link up? */
        uint64_t    full_duplex     : 1;    /**< 1 if the link is full duplex */
        uint64_t    speed           : 18;   /**< Speed of the link in Mbps */
        uint64_t    port_type         : 8;  	
        uint64_t    if_used         : 1; 
    } s;
} port_info_t;

void cvmx_oct_set_product_id(product_info_t *product_info);
	
#endif
