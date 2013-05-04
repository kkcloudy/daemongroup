#ifndef _PPPOE_LCP_H
#define _PPPOE_LCP_H


int lcpConfigReq_packet(session_struct_t *sess, struct pppoe_buf *pbuf);
int lcpTerm_packet(session_struct_t *sess, struct pppoe_buf *pbuf, uint32 termType);
int lcpEchoReq_packet(session_struct_t *sess, struct pppoe_buf *pbuf);

int lcp_proto_init(void);
int lcp_proto_exit(void);

#endif

