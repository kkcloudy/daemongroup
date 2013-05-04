#ifndef __HAD_IPADDR_H__
#define __HAD_IPADDR_H__
/* system include */
/* local include */
#ifndef u_char
#define u_char unsigned char
#endif

/*@$#[ipaddr.c] global proto. AutoProtoSigV1.1. date: 00/06/02 23:47:40 */
#include "had_proto.h"
int had_ipaddr_list PROTO((int ifindex, uint32_t *array, int max_elem));
int had_ipaddr_op PROTO((int ifindex, uint32_t addr,uint32_t mask, int addF));
/*@$% end of AutoProtoSigV1.1 (Dont remove this line) []*/
 

/****************************************************************
 NAME	: had_ipaddr_list				00/06/02 20:02:23
 AIM	: 
 REMARK	:
****************************************************************/
int had_ipaddr_list
(
	int ifindex,
	uint32_t *array,
	int max_elem
);

/****************************************************************
 NAME	: had_ipaddr_op				00/06/02 23:00:58
 AIM	: add or remove 
 REMARK	:
****************************************************************/
int had_ipaddr_op
(
	int ifindex,
	uint32_t addr,
	uint32_t mask,
	int addF
);

/****************************************************************
 NAME	: had_ipaddr_op_withmask				00/06/02 23:00:58
 AIM	: add or remove 
 REMARK	:
****************************************************************/
int had_ipaddr_op_withmask
(
	int ifindex,
	uint32_t addr,
	int mask,int addF
);


#endif	/* __IPADDR_H__ */

