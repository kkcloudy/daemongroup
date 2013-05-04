#ifndef _PPPOE_CHAP_H
#define _PPPOE_CHAP_H

int chapChallenge_packet(session_struct_t *sess, struct pppoe_buf *pbuf);
int chapResult_packet(session_struct_t *sess, struct pppoe_buf *pbuf, uint32 result);

int chap_proto_init(void);
int  chap_proto_exit(void);


#endif
