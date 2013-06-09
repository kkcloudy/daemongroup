#include <zebra.h>
#include "zebra/rib.h"
#include "zebra/zserv.h"

#include "zebra/redistribute.h"

void zebra_redistribute_add (int a, struct zserv *b, int c)
{ return; }
#pragma weak zebra_redistribute_delete = zebra_redistribute_add
#pragma weak zebra_redistribute_default_add = zebra_redistribute_add
#pragma weak zebra_redistribute_default_delete = zebra_redistribute_add

void redistribute_add (struct prefix *a, struct rib *b)
{ return; }
#pragma weak redistribute_delete = redistribute_add

/**add by gjd : for tipc**/
void tipc_redistribute_add(struct prefix *a, struct rib *b)
{ return; }
#pragma weak tipc_redistribute_delete = tipc_redistribute_add
/**2011-01--02**/
void vice_redistribute_interface_delete(struct interface * ifp)
{return;}

void redistribute_interface_linkdetection(struct interface * ifp,int done)
{return;}
void
zebra_interface_uplink_state(struct interface *ifp, int done)
{return;}
#if 0
void zebra_interface_packets_statistics_request(struct interface *ifp)
{return;}
#endif
void zebra_interface_up_update (struct interface *a)
{ return; }
#pragma weak zebra_interface_down_update = zebra_interface_up_update
#pragma weak zebra_interface_add_update = zebra_interface_up_update
#pragma weak zebra_interface_delete_update = zebra_interface_up_update

void zebra_interface_description_update (int command,struct interface *ifp)
{ return; }

void zebra_interface_address_add_update (struct interface *a,
					 	struct connected *b)
{ return; }
#pragma weak zebra_interface_address_delete_update = zebra_interface_address_add_update
