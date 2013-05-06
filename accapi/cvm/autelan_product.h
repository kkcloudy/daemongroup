/*
 *	Filename : autelan_product.h
 *	Auther : caojia@autelan.com
 *	Date : 12/17/2010
 *
 *	Define product type for all the autelan product.
 */

#ifndef __AUTELAN_PRODUCT_H__
#define	__AUTELAN_PRODUCT_H__

#define CPLD_BASE_ADDR 0x1d010000
#define CPLD_PRODUCT_SERIES_REG 0x1
#define CPLD_PRODUCT_TYPE_REG 0x2
#define CPLD_BOARD_TYPE_REG	0x3
#define CPLD_BOARD_SLOT_ID_REG 0x28

#define CPLD_PRODUCT_TYPE_MASK 0x7

#define CPLD_SLOT_ID 0x28

#define	PRODUCT_AX7605I	0x1
#define PRODUCT_AX8610	0x4
#define PRODUCT_AX8606 0x5

/*
 * Autelan product series enum-type
 * Not used now
 */
typedef enum product_series_s
{
	AUTELAN_PRODUCT_SERIES_AX7000,
	AUTELAN_PRODUCT_SERIES_AX8600,
	AUTELAN_NONDISTRIBUTED_PRODUCT_SERIES,
	AUTELAN_PRODUCT_SERIES_MAX
}product_series_t;

/*
 * Autelan product type enum-type
 */
typedef enum product_type_s
{
	AUTELAN_PRODUCT_AX7605,
	AUTELAN_PRODUCT_AX7605I,
	AUTELAN_PRODUCT_AX8610,
	AUTELAN_PRODUCT_AX8606,
	AUTELAN_PRODUCT_AX8603,
	AUTELAN_PRODUCT_AX8800,
	AUTELAN_UNKNOWN_PRODUCT
	/* 
	 * if you want to add some new autelan product types
	 * please add it upon the AUTELAN_UNKNOWN_PRODUCT in order
	 * and add it in src/kernel2.6.32.27cn/include/linux/autelan_product.h synchronously
	 */
}product_type_t;

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
	AUTELAN_BOARD_AX71_1X12G12S,
	AUTELAN_UNKNOWN_BOARD
	/* 
	 * if you want to add some new autelan product types
	 * please add it upon the AUTELAN_UNKNOWN_BOARD in order
	 * and add it in src/kernel2.6.32.27cn/include/linux/autelan_product.h synchronously
	 */
}board_type_t;

#define MAC_ADDRESS_LEN	6

typedef struct product_info_s
{
 	product_type_t product_type;
	board_type_t board_type;
	unsigned int product_series_num;
	unsigned int product_type_num;
	unsigned int board_type_num;
	unsigned int distributed_product;
	unsigned int board_slot_id;
	unsigned char mac[MAC_ADDRESS_LEN];
	unsigned int coremask;
	/*extend other necessary messages here*/
}product_info_t;

#endif
