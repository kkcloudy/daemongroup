#ifndef __DCLI_MLD_SNP_H__
#define __DCLI_MLD_SNP_H__

#define MLD_SNOOPING_STR "Config MLD Snooping Protocol\n"
#define IGMP_STR_CMP_LEN	4
#define SIZE_OF_IPV6_ADDR 8

#define ISMLD 1
#define ISIGMP 2

__inline__ int parse_slotport_no(char * str, unsigned char * slotno, unsigned char * portno);

void dcli_mld_snp_element_init(void);
int dcli_mld_snp_show_running_config(struct vty *vty);
int dcli_mld_snp_time_show_running_config(unsigned char Ismld);

void dcli_show_mld_snp_mcgroup_list(struct vty *vty,unsigned short vlanId);
void dcli_show_mld_snp_mcgroup_count(struct vty *vty,unsigned short vlanId);

extern int dcli_igmp_snp_vlan_show_running_config(unsigned char Ismld);
extern int dcli_igmp_snp_time_show_running_config(unsigned char Ismld);
extern int dcli_igmp_snp_show_running_routeport(struct vty	*vty,unsigned short	vlanId,PORT_MEMBER_BMP	*routeBmp);
extern int parse_timeout(char* str,unsigned int* timeout);
extern int show_igmp_snp_timer(struct vty *vty,unsigned int vlanlife,unsigned int grouplife,unsigned int robust,unsigned int queryinterval,unsigned int respinterval,unsigned int hosttime);
extern int show_igmpsnp_vlan_member_info(struct vty *vty,unsigned int product_id,PORT_MEMBER_BMP untagBmp,PORT_MEMBER_BMP tagBmp);

#endif

