#ifndef _MANAGE_API_H_
#define _MANAGE_API_H_


manage_session *manage_open(manage_session *session);

void manage_read(fd_set * fdset);

void manage_close(manage_session *session);

void manage_close_all(void);

int manage_select_info(int *numfds, fd_set * fdset,
							struct timeval *timeout, int *block);

void manage_method_regist(manage_method *array, u_long size);

void manage_task_process(void);

manage_message *manage_message_new(u_short method_id, void *data, size_t data_length);

int
manage_message_send(manage_session *session, manage_message *message,
									void *transport_data, size_t transport_data_length);


manage_message *
manage_message_send_with_reply_and_block(manage_session *session, manage_message *message, int timeout_milliseconds, 
															void *transport_data, size_t transport_data_length);

void
manage_register();


#endif
