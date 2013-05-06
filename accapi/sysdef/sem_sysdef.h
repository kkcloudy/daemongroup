#ifndef _SEM_SYSDEF_H_
#define _SEM_SYSDEF_H_

#define ETH_ATTR_LINKUP 1
#define ETH_ATTR_LINKDOWN 0

#define SEM_SUCCESS       0
#define SEM_FAIL          -1

/* for mode convertion between fiber and copper */
typedef enum {
	PHY_MEDIA_MODE_NONE = 0,
	PHY_MEDIA_MODE_FIBER,
	PHY_MEDIA_MODE_COPPER
}PHY_MEDIA;  


/* Bits 30~31 reserved */

typedef enum {
	COMBO_PHY_MEDIA_PREFER_NONE = 0,
	COMBO_PHY_MEDIA_PREFER_FIBER,
	COMBO_PHY_MEDIA_PREFER_COPPER
}COMBO_PHY_MEDIA;

#define SEM_TRUE 	1
#define SEM_FALSE	0
#define ETH_ATTR_BP_ENABLE 1
#define ETH_ATTR_BP_DISABLE 0
#define ETH_ATTR_MEDIA_EXIST_PRIO  1
#define ETH_ATTR_MEDIA_NOT_EXIST_PRIO 0

/* Bits 0~11 to have 12 kinds of binary attributes */
#define ETH_ATTR_ADMIN_STATUS 	0x1	/* bit0 */
#define ETH_ATTR_LINK_STATUS 	0x2	/* bit1 */
#define ETH_ATTR_AUTONEG 	0x4	/* bit2: port auto-negotiation status */
#define ETH_ATTR_DUPLEX 	0x8	/* bit3 */
#define ETH_ATTR_FLOWCTRL 	0x10	/* bit4 */
#define ETH_ATTR_BACKPRESSURE 	0x20	/* bit5 */
#define ETH_ATTR_AUTONEG_SPEED 0x40	/* bit6 */
#define ETH_ATTR_AUTONEG_DUPLEX 0x80	/* bit7 */
#define ETH_ATTR_AUTONEG_FLOWCTRL 0x100	/* bit8 */
#define ETH_ATTR_AUTONEG_CTRL	0x200 	/* bit9 */


#define ETH_ATTR_ON 1
#define ETH_ATTR_OFF 0
#define ETH_ATTR_ENABLE 1
#define ETH_ATTR_DISABLE 0
#define ETH_ATTR_LINKUP 1
#define ETH_ATTR_LINKDOWN 0
#define ETH_ATTR_DUPLEX_FULL 0
#define ETH_ATTR_DUPLEX_HALF 1
#define ETH_ATTR_AUTONEG_DONE 1
#define ETH_ATTR_AUTONEG_NOT_DONE 0
#define ETH_ATTR_FC_ENABLE 1
#define ETH_ATTR_FC_DISABLE 0
#define ETH_ATTR_AUTONEG_SPEEDON 1 
#define ETH_ATTR_AUTONEG_SPEEDDOWN 0
#define ETH_ATTR_AUTONEG_FCON  1  
#define ETH_ATTR_AUTONEG_FCDOWN  0
#define ETH_ATTR_AUTONEG_DUPLEX_ENABLE 1 
#define ETH_ATTR_AUTONEG_DUPLEX_DISABLE 0 


#define ETH_ADMIN_STATUS_BIT	0x0
#define ETH_LINK_STATUS_BIT	0x1
#define ETH_AUTONEG_BIT		0x2
#define ETH_DUPLEX_BIT		0x3
#define ETH_FLOWCTRL_BIT	0x4
#define ETH_BACKPRESSURE_BIT	0x5
#define ETH_AUTONEG_SPEED_BIT 	0x6
#define ETH_AUTONEG_DUPLEX_BIT 	0x7
#define ETH_AUTONEG_FLOWCTRL_BIT 0x8
#define ETH_AUTONEG_CTRL_BIT	0x9
#define ETH_PREFERRED_COPPER_MEDIA_BIT  0x1c
#define ETH_PREFERRED_FIBER_MEDIA_BIT 0x1d



/* Bits 12~15 to represent 4bits 16 kinds of speed */
#define ETH_ATTR_SPEED_MASK 		0xF000
#define ETH_ATTR_SPEED_10M 		0x0
#define ETH_ATTR_SPEED_100M 		0x1
#define ETH_ATTR_SPEED_1000M 		0x2
#define ETH_ATTR_SPEED_10G 		0x3
#define ETH_ATTR_SPEED_12G 		0x4
#define ETH_ATTR_SPEED_2500M 		0x5
#define ETH_ATTR_SPEED_5G			0x6
#define ETH_ATTR_SPEED_MAX 		0xF

#define ETH_SPEED_BIT				0xC

/* Bits 16~19 to represent 4bits of 16 kinds of pluggable port status, e.g. sfp/gbic */
#define ETH_ATTR_PLUGGABLE_PHY_TYPE_MASK 0xF0000
#define ETH_ATTR_PLUGGABLE_PHY_LX 0x10

/* Bits 20~21 reserved */


/* Bits 24~27 to represent 4bits(kinds) of basic functions with two state and no extend data structure */
#define ETH_ATTR_BASICFUNC_L2BRDGE 0x1000000 

/* preferred media 28~29 */
#define ETH_ATTR_PREFERRED_COPPER_MEDIA		0x10000000
#define ETH_ATTR_PREFERRED_FIBER_MEDIA 0x20000000

#endif
