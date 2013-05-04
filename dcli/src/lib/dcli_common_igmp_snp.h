#ifndef __DCLI_COMMON_IGMP_SNP_H__
#define __DCLI_COMMON_IGMP_SNP_H__

extern int dcli_debug_out;
#define DCLI_DEBUG(x) if(dcli_debug_out){printf x ;}


#define SIZE_OF_IPV6_ADDR	8

#define BRG_MC_FAIL 0xff

#define IGMP_SNOOPING_STR "IGMP snooping information\n"
#define DCLI_IGMP_MAX_CHASSIS_SLOT_COUNT 16

#define NIPQUAD(addr) \
	((unsigned char *)&addr)[0], \
	((unsigned char *)&addr)[1], \
	((unsigned char *)&addr)[2], \
	((unsigned char *)&addr)[3]

typedef struct{
    unsigned int   ports[2];
}CPSS_PORTS_BMP_STC;

int show_group_member_slot_port
(
	struct vty *vty,
	unsigned int product_id,
	PORT_MEMBER_BMP mbrBmp
);

unsigned int ltochararry
(
	unsigned int Num,
	unsigned char *c0,
	unsigned char *c1,
	unsigned char *c2,
	unsigned char *c3
);
int dcli_igmp_snp_check_status(unsigned char* stats,unsigned char ismldq, unsigned char* devchk);

int dcli_enable_disable_igmp_one_vlan
(
	struct vty*	 vty,
	unsigned short vlanId,
	unsigned int enable
);

int dcli_enable_disable_igmp_one_port
(
	struct vty*  vty,
	unsigned short vlanId,
	unsigned char slot_no,
	unsigned char port_no,
	unsigned char enable
);

int dcli_show_igmp_snp_one_mcgroup
(
	struct vty*		vty,
	unsigned short vid,
	unsigned short vidx
);

void dcli_show_igmp_snp_mcgroup_list
(
	struct vty*		vty,
	unsigned short vlanId
);

/**********************************************************************************
 *  dcli_get_product_igmp_function
 *
 *	DESCRIPTION:
 * 		get broad type
 *
 *	INPUT:
 *		NONE		
 *	
 * RETURN:
 *		0   -   disable
 *		1   -   enable
 *
 **********************************************************************************/

int dcli_get_product_igmp_function
(
	struct vty* vty,
	unsigned char *IsSupport
);

#endif

