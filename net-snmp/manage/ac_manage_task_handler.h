#ifndef _AC_MANAGE_TASK_HANDLER_H_
#define _AC_MANAGE_TASK_HANDLER_H_


int manage_task_test(void *para, size_t para_len,void **data, size_t *data_len);

int manage_task_interface_info(void *para, size_t para_len,void **data, size_t *data_len);
int
manage_task_I_O_board(void *data, size_t data_len,void **data_addr, size_t *data_addr_len);
int
manage_task_register_slot();


#endif
