#ifndef _PPPOE_PPP_H
#define _PPPOE_PPP_H

typedef int (*protoFunc) (session_struct_t *, struct pppoe_buf *);

int ppp_proto_process(uint32 proto, session_struct_t *sess, struct pppoe_buf *pbuf);

int ppp_proto_register(uint32 proto, protoFunc process);
int ppp_proto_unregister(uint32 proto);

void ppp_proto_init(void);
void ppp_proto_exit(void);

int ppp_recv_packet(int sk, struct sockaddr_pppoe *addr, struct pppoe_buf *pbuf);
int ppp_send_packet(int sk, struct sockaddr_pppoe *addr, struct pppoe_buf *pbuf);


#endif
