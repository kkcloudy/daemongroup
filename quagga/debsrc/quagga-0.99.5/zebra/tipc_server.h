#ifndef _TIPC_SERVER_H_
#define _TIPC_SERVER_H_

#include "tipc_zclient_lib.h"


#define TIPC_MAX_PACKET_SIZ          4096

 /**used for vice board, like zclient**/
typedef struct tipc_server
{
  /* Socket to zebra daemon. */
  int sock;
  int accept_slot;

  /* Connection failure count. */
  int fail;

  /* Input buffer for zebra message. */
  struct stream *ibuf;

  /* Output buffer for zebra message. */
  struct stream *obuf;

  /* Buffer of data waiting to be written to zebra. */
  struct buffer *wb;

  /* Read and connect thread. */
  struct thread *t_read;
//  struct thread *t_connect;
  struct thread *t_suicide;

  /* Thread to write buffered data to zebra. */
  struct thread *t_write;

  /* Interface information. */ 
  u_char ifinfo;

  /* Pointer to the callback functions. */
  int (*router_id_update) (int,struct tipc_server *, uint16_t);
  int (*interface_add) (int, struct tipc_server *, uint16_t);
  int (*interface_delete) (int,struct tipc_server *, uint16_t);
  int (*interface_up) (int,struct tipc_server *, uint16_t);
  int (*interface_down) (int,struct tipc_server *, uint16_t);
  int (*interface_address_add) (int,struct tipc_server *, uint16_t);
  int (*interface_address_delete) (int,struct tipc_server *, uint16_t);
  int (*ipv4_route_add) (int,struct tipc_server *, uint16_t);
  int (*ipv4_route_delete) (int,struct tipc_server *, uint16_t);
  int (*ipv6_route_add) (int, struct tipc_server *, uint16_t);
  int (*ipv6_route_delete) (int,struct tipc_server *, uint16_t);
  /**add by gjd**/
  int (*redistribute_interface_all)(int,struct tipc_server *, uint16_t);/**all interface when system start**/
  int (*redistribute_route_all)(int, struct tipc_server *, uint16_t);/**all route when system start**/
}tipc_server;

extern	struct zebra_t zebrad ;

/**add by gjd for tipc server redistribute to client**/
void tipc_redistribute_add (struct prefix *p, struct rib *rib);
void tipc_redistribute_delete (struct prefix *p, struct rib *rib);
extern int zebra_server_send_message(struct zserv * client);
extern void zserv_create_header(struct stream * s,uint16_t cmd);
/**2011-01--02**/

extern int zserv_delayed_close(struct thread * thread);
extern int zserv_flush_data(struct thread * thread);

/*gujd: 2012-02-09: am10:00 . In order to decrease the warning when make img . For declaration  func .*/
extern int vice_board_close(tipc_server * vice_board);

#endif

