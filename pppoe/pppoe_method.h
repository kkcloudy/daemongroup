#ifndef _PPPOE_METHOD_H
#define _PPPOE_METHOD_H

int pppoe_method_perform(uint32 method_id, void *data, void *para);

int pppoe_method_register(uint32 method_id, methodFunc method);
int pppoe_method_unregister(uint32 method_id);

void pppoe_method_init(void);
void pppoe_method_exit(void);

#endif
