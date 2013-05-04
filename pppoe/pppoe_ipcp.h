#ifndef _PPPOE_IPCP_H
#define _PPPOE_IPCP_H

int ipcpConfigReq_packet(session_struct_t *sess, struct pppoe_buf *pbuf);

int ipcp_proto_init(void);
int ipcp_proto_exit(void);

#endif
