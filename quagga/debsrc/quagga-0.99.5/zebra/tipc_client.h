#ifndef _TIPC_CLIENT_H_
#define _TIPC_CLIENT_H_

/**add by gjd **/
//#define BUFSIZ 1024		


 /**used for master board, like struct zserver**/
typedef struct tipc_client
{
  int sock;
  int board_id;/******add*******/

  /* Input/output buffer to the client. */
  struct stream *ibuf;
  struct stream *obuf;

  /* Buffer of data waiting to be written to client. */
  struct buffer *wb;

  /* Threads for read/write. */
  struct thread *t_read;
  struct thread *t_write;

  /* Thread for delayed close. */
 // struct thread *t_suicide;
 
  struct thread *t_connect;

  /* Connection failure count. */
  int fail;

  /* default routing table this client munges */
  int rtm_table;

  /* Interface information. */
  u_char ifinfo;

  /**hooks**/  
  int (*router_id_update) (int, struct tipc_client*, uint16_t);
  int (*interface_add) (int,struct tipc_client *, uint16_t);
  int (*interface_delete) (int, struct tipc_client *, uint16_t);
  int (*interface_up) (int, struct tipc_client *, uint16_t);
  int (*interface_down) (int,  struct tipc_client *, uint16_t);
  int (*interface_address_add) (int, struct tipc_client *, uint16_t);
  int (*interface_address_delete) (int, struct tipc_client *, uint16_t);
  int (*ipv4_route_add) (int,struct tipc_client *, uint16_t);
  int (*ipv4_route_delete) (int, struct tipc_client *, uint16_t);
  int (*ipv6_route_add) (int, struct tipc_client *, uint16_t);
  int (*ipv6_route_delete) (int, struct tipc_client *, uint16_t);

  /**add by gjd**/
  int (*redistribute_interface_all)(int,struct tipc_client *, uint16_t);/**all interface when system start**/
  int (*redistribute_route_all)(int, struct tipc_client *, uint16_t);/**all route when system start**/

} tipc_client;

enum tipc_client_events { TIPC_CLIENT_SCHEDULE, TIPC_CLIENT_CONNECT, TIPC_CLIENT_READ, TIPC_PACKETS_STATISTICS_TIMER};

#define TIPC_CONNECT_TIMES        120 /*every 3s , 120 times so in all 6 minutes.*/

extern void rib_delnode(struct route_node * rn,struct rib * rib);
extern void rib_addnode(struct route_node * rn,struct rib * rib);

extern int netlink_route_multipath(int cmd,struct prefix * p,struct rib * rib,int family);
/**2011-01-16:am 10:30**/

/*gujd: 2012-02-09: am10:00 . In order to decrease the warning when make img . For declaration  func .*/
extern void master_board_event(enum tipc_client_events event,tipc_client * master_board);

#endif
