#ifndef DCLI_PPPOE_H
#define DCLI_PPPOE_H

char *dcli_pppoe_show_running_config(int localid, int slot_id, int index);

void dcli_pppoe_init(void);

#endif
