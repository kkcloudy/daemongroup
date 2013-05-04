#ifndef _PPPOE_BACKUP_H
#define _PPPOE_BACKUP_H

typedef int (*backupProtoFunc) (struct pppoe_buf *, void *);
typedef struct backup_task backup_task_t;
typedef struct backup_struct backup_struct_t;

#define BACKUP_HEADER_LEN	4

enum {
	BACKUP_ECHO_REQUEST,
	BACKUP_ECHO_REPLY,
	BACKUP_INSTANCE_SYNC,
};

enum {
	CHANNEL_NONE,
	CHANNEL_INIT,
	CHANNEL_CONNECT,
	CHANNEL_DISCONNECT,
	CHANNEL_CLOSE,
};

enum {
	BACKUP_NONE,
	BACKUP_ACTIVE,
	BACKUP_STANDBY,
	BACKUP_DISABLE,
	BACKUP_STATUSNUMS,
};

backup_task_t *backup_task_create(backup_struct_t *backup, 
								struct pppoe_buf *pbuf, uint16 proto);
int backup_task_add(backup_struct_t *backup, backup_task_t *task);
void backup_task_destroy(backup_struct_t *backup, backup_task_t **task);

int backup_proto_register(backup_struct_t *backup, uint16 proto, 
						backupProtoFunc process, void *proto_data);

#define backup_proto_register(backup, proto, process, data)		\
				backup_proto_register_with_funcname(backup, proto, process, data, #process)
int backup_proto_register_with_funcname(backup_struct_t *backup, 
										uint16 proto, backupProtoFunc process, 
										void *proto_data, const char *funcname);
int backup_proto_unregister(backup_struct_t *backup, uint16 proto);

int backup_notifier_unregister(backup_struct_t *backup, struct notifier_struct *notifier);
int backup_notifier_register(backup_struct_t *backup, struct notifier_struct *notifier);

uint32 backup_status(backup_struct_t *backup);
uint32 backup_prevstatus(backup_struct_t *backup);
uint32 backup_channel_state(backup_struct_t *backup);
int backup_status_setup(backup_struct_t *backup, uint32 status,
					uint32 s_ip, uint16 s_port, uint32 d_ip, uint16 d_port);

backup_struct_t *backup_init(thread_master_t *master);
void backup_exit(backup_struct_t **backup);

#endif
