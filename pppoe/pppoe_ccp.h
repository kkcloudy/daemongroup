#ifndef _PPPOE_CCP_H
#define _PPPOE_CCP_H

int ccpConfigReq_packet(session_struct_t *sess, struct pppoe_buf *pbuf);

int ccp_proto_init(void);
int ccp_proto_exit(void);


#endif
