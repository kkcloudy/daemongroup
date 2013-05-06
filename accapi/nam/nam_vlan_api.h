#ifndef __NAM_VLAN_API_H__
#define __NAM_VLAN_API_H__

/*
  APIs exported to be used by NPD module
*/
#if 0
extern unsigned int nam_asic_vlan_get_port_members
(
	unsigned short	vlanId,
	unsigned int	*npdMbrBmpWord,
	unsigned int	*npdTagBmpWord
);
extern unsigned int nam_asic_trunk_member_get
(
	unsigned short trunkId,
	unsigned int *memberCount,
	unsigned char memberArray[]
);
extern unsigned int nam_asic_vlan_get_port_members_bmp
(
	unsigned short	vlanId,
	unsigned int	*untagBmp,
	unsigned int	*tagBmp
);
#endif
#endif
