/*********************************************************************************************
*			Copyright(c), 2008, Autelan Technology Co.,Ltd.
*						All Rights Reserved
*
**********************************************************************************************
$RCSfile:igmp_snoop.h
$Author: Rock
$Revision: 1.00
$Date:2008-3-8 10:07
***********************************************************************************************/

#ifndef __MLD_SNOOP_H__
#define __MLD_SNOOP_H__

#ifdef __cplusplus
extern "C"
{
#endif

#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <fcntl.h>
#include <sys/socket.h>
#include <string.h>
#include <unistd.h>
#include <sys/un.h>
#include <sys/stat.h>
#include <linux/if_ether.h>
#include <netinet/ip.h>




#define SIZE_OF_IPV6_ADDR 8


typedef struct mld_notify_mod_pkt{
	ULONG	mod_type;
	LONG	vlan_id;
	ULONG	ifindex;
	USHORT	groupadd[SIZE_OF_IPV6_ADDR];
	ULONG	reserve;
}mld_notify_mod_pkt;

struct mld_mng{
		struct nlmsghdr nlh;
		mld_notify_mod_pkt mld_noti_npd;
};

extern INT npdmng_fd;
extern INT recv_fd;
extern struct sockaddr_un remote_addr;
extern igmp_vlan_list *p_vlanlist;


/*MLD Snoop kernel message type*/
#define	MLD_SNP_TYPE_MIN				0
#define	MLD_SNP_TYPE_NOTIFY_MSG		4	/*notify message*/
#define	MLD_SNP_TYPE_PACKET_MSG		5	/*Packet message*/
#define MLD_SNP_TYPE_DEVICE_EVENT		6	/*device message*/
#define	MLD_SNP_TYPE_MAX				9

/*MLD Snoop message flag*/
#define	MLD_SNP_FLAG_MIN				0
#define	MLD_SNP_FLAG_PKT_UNKNOWN	4	/*Unknown packet*/
#define	MLD_SNP_FLAG_PKT_MLD		5	/*MLD,PIM packet*/
#define	MLD_SNP_FLAG_ADDR_MOD		6	/*notify information for modify address*/
#define	MLD_SNP_FLAG_MAX				9


/*************function declaration*********************/
VOID mld_debug_print_notify(struct mld_mng *send);
INT mld_getvlan_addr(unsigned long vlan_id, unsigned short *vlan_addr);
LONG mld_L2mc_Entry_GetVidx(  unsigned long ulVlanId,unsigned short *uGroupAddr,unsigned long* ulVidx);
LONG mld_L2mc_Entry_Insert( mld_snoop_pkt *pPkt );
LONG mld_L2mc_Entry_Node_Delete( mld_snoop_pkt *pPkt );
LONG mld_L2mc_Entry_Group_Delete( mld_snoop_pkt *pPkt );
LONG mld_L2mc_Entry_Group_Delete_All( mld_snoop_pkt *pPkt);
LONG mld_send_msg_npd(VOID *data, INT datalen,unsigned long msg_type);
LONG mld_notify_msg(mld_snoop_pkt *pPkt,unsigned long ulArg);
LONG mld_snp_mod_addr( mld_snoop_pkt *pPkt, unsigned long ulArg);


#ifdef __cplusplus
}
#endif

#endif

