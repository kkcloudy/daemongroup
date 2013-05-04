#ifndef _PPPOE_NETLINK_H
#define _PPPOE_NETLINK_H

int netlink_init(void);
int netlink_recv_message(int sk, struct pppoe_buf *pbuf);
int netlink_recv_message_wait(int sk, struct pppoe_buf *pbuf, uint32 msec);

int netlink_register(int sk, struct pppoe_buf *pbuf, 
					uint32 local_id, uint32 instance_id);
int netlink_unregister(int sk, struct pppoe_buf *pbuf, 
					uint32 local_id, uint32 instance_id);
int netlink_create_interface(int sk, struct pppoe_buf *pbuf, char *ifname);
int netlink_destroy_interface(int sk, struct pppoe_buf *pbuf, int ifindex);
int netlink_base_interface(int sk, struct pppoe_buf *pbuf, int ifindex, char *ifname);
int netlink_unbase_interface(int sk, struct pppoe_buf *pbuf, int ifindex);

int netlink_channel_register(int sk, struct pppoe_buf *pbuf, int ifindex, 
							uint32 sid, uint8 *mac, uint8 *serverMac, uint8 *magic);
int netlink_channel_unregister(int sk, struct pppoe_buf *pbuf, 
						int ifindex, uint32 sid, uint8 *mac);
int netlink_channel_authorize(int sk, struct pppoe_buf *pbuf,
						int ifindex, uint32 sid, uint32 ip);
int netlink_channel_unauthorize(int sk, struct pppoe_buf *pbuf, 
						int ifindex, uint32 sid, uint32 ip);
int netlink_channel_clear(int sk, struct pppoe_buf *pbuf, int ifindex);

#endif
