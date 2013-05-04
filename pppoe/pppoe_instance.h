#ifndef _PPPOE_INSTANCE_H
#define _PPPOE_INSTANCE_H

typedef struct instance_struct instance_struct_t;

void pppoe_instance_dispatch(instance_struct_t *instance, uint32 msec);
instance_struct_t *pppoe_instance_init(uint32 slot_id, uint32 local_id, uint32 instance_id);
void pppoe_instance_exit(instance_struct_t **instance);


#endif
