/*********************************************************************************************
*			Copyright(c), 2008, Autelan Technology Co.,Ltd.
*						All Rights Reserved
*
**********************************************************************************************
$RCSfile:mld_snoop_inter.h
$Author: Rock
$Revision: 1.00
$Date:2008-3-8 10:07
***********************************************************************************************/
/****************************This file is inter H file*************************************************/
#ifndef __MLD_SNOOP_INTER_H__
#define __MLD_SNOOP_INTER_H__

#ifdef __cplusplus
extern "C"
{
#endif


#ifndef ULONG
#define ULONG unsigned long
#endif

#ifndef LONG
#define LONG long 
#endif

#ifndef USHORT
#define USHORT unsigned short
#endif

#ifndef UCHAR
#define UCHAR unsigned char
#endif

#ifndef CHAR
#define CHAR char
#endif

#ifndef  SHORT
#define SHORT short
#endif

#ifndef	UINT
#define	UINT	unsigned int
#endif

#ifndef	INT
#define INT		int
#endif

#ifndef	VOID
#define	VOID	void
#endif



/*big end*/
#ifndef __BIG_ENDIAN_BITFIELD
#define __BIG_ENDIAN_BITFIELD
#endif

#define SIZE_OF_IPV6_ADDR	8

typedef struct mld_snoop_pkt
{
	USHORT	type;		/*msg type*/
	ULONG	ifindex;		/*port index*/
	LONG	vlan_id;		/*vlan index*/
	LONG	group_id;	/*group vlan index*/ /**vidx*/
	USHORT	saddr[SIZE_OF_IPV6_ADDR];		/*source ip*/
	USHORT	groupadd[SIZE_OF_IPV6_ADDR];	/*group ip*/
	USHORT	len;			/*skbuff len*/
	USHORT	retranscnt;	/*retransmit cnt*/
	ULONG	action;
}mld_snoop_pkt;

#define MLD_IPV6_OPTION_LEN 8

#define MLD_VERSION 6
#define MLD_PRIORITY 0
#define MLD_HOP_LIMIT 1
#define MLD_FLOW_LABEL 0
#define MLD_TRAFFIC_CLASS 0
#define IPPROTO_MLD 	0x3A		/*MLD protocol type(actually is ICMPV6 protocol type)*/



/****************MLD Packet Type*****************/
#define MLD_MEMSHIP_QUERY	0x82	/*query*/
#define MLD_V1_MEMSHIP_REPORT	0x83	/*V1 report*/
#define MLD_V1_LEAVE_GROUP		0x84	/*V1 leave*/
#define MLD_V2_MEMSHIP_REPORT	0x8F	/*V2 report*/

/****************MLD Group State*****************/
#define MLD_SNP_GROUP_NOMEMBER		0x00	/*no member*/
#define MLD_SNP_CHECK_MEMBER		0x01	/*checking member*/
#define MLD_SNP_HAS_MEMBER			0x02	/*has member*/
#define MLD_SNP_TEMP_MEMBER		0x08	/*send query temporary state*/


/******************** Packet Type ***********************/
enum mldpkttype
{
	MLD_TYPE_PKT_RECV,			/*receive */
	MLD_TIMEOUT_TICK,			/*定时器超时：1sec*/
	MLD_TIMEOUT_GROUP_LIFE,	/*组存活定时器*/
	MLD_TIMEOUT_GEN_QUERY,		/*query定时器*/
	MLD_TIMEOUT_GROUP_MEMBERSHIP,	/*组成员定时器*/
	MLD_TIMEOUT_V1_HOST,		/*MLD V1主机定时器*/
	MLD_TIMEOUT_RXMT,			/*MLD 转发定时器*/
	MLD_TIMEOUT_RESP,			/*Switch响应多播路由器的query*/
	MLD_TIMEOUT_ROUTER,		/*路由定时器*/
	MLD_MSG_GEN_QUERY,			/*发送MLD v1 普通query*/
	MLD_MSG_GS_QUERY,			/*发送MLD v1 特定query*/
	MLD_MSG_REPORT,			/*发送MLD v1 report*/
	MLD_MSG_LEAVE,				/*发送MLD v1 leave*/
	MLD_MSG_V2_REPORT,			/*发送MLD v2 report*/

	MLD_VLAN_ADD_PORT,			/*VLAN添加端口*/
	MLD_VLAN_DEL_PORT,			/*VLAN删除端口*/
	MLD_PORT_DOWN,				/*端口down*/
	MLD_VLAN_DEL_VLAN,
	MLD_INVALID				/*Invalid*/
};



#define MLD_SNOOP_NO	0
#define MLD_SNOOP_YES	1



#ifdef __cplusplus
}
#endif

#endif

