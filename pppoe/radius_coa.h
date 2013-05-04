#ifndef _RADIUS_COA_H
#define _RADIUS_COA_H

typedef struct radius_coa radius_coa_t;
typedef int (*coaProcess) (void *, struct pppoe_buf *, uint32);
typedef int (*coaSendto) (void *, int, struct pppoe_buf *, struct sockaddr_in *, socklen_t);
typedef int (*coaRecvfrom) (void *, int, struct pppoe_buf *, struct sockaddr_in *, socklen_t *);

int coa_proto_register(radius_coa_t *coa, uint8 proto, 
					void *proto_ptr, coaProcess process);
int coa_proto_unregister(radius_coa_t *coa, uint8 proto);

radius_coa_t *radius_coa_init(thread_master_t *master, 
						void *sk_ptr, uint32 ip, uint16 *port,
						coaSendto sendto, coaRecvfrom recvfrom);
void radius_coa_exit(radius_coa_t **coa);

#endif
