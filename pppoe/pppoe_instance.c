
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <pthread.h>
#include <dbus/dbus.h>
#include <sys/time.h>
#include <sched.h>

#include <arpa/inet.h>
#include <sys/socket.h>
#include <linux/if.h>
#include <linux/if_ether.h>

#include "pppoe_def.h"
#include "pppoe_priv_def.h"
#include "pppoe_dbus_def.h"
#include "pppoe_interface_def.h"
#include "kernel/if_pppoe.h"
#include "pppoe_method_def.h"

#include "pppoe_log.h"
#include "pppoe_list.h"
#include "mem_cache.h"
#include "thread_bus.h"
#include "pppoe_util.h"
#include "pppoe_buf.h"
#include "pppoe_dbus.h"
#include "pppoe_method.h"
#include "pppoe_thread.h"
#include "notifier_chain.h"
#include "pppoe_backup.h"
#include "pppoe_netlink.h"
#include "pppoe_manage.h"
#include "pppoe_discover.h"
#include "pppoe_control.h"
#include "pppoe_radius.h"

#include "pppoe_instance.h"

#define CONFIG_NONE				0
#define CONFIG_DEV_BASE			0x1
#define CONFIG_DEV_IPADDR		0x2
#define CONFIG_SESS_IPADDR		0x4
#define CONFIG_RADIUS_SERVER	0x8
#define CONFIG_RADIUS_NASIP		0x10
#define CONFIG_RADIUS_RDC		0x20
#define CONFIG_SERVICE_NAME		0x40
#define CONFIG_READY			(CONFIG_DEV_BASE | CONFIG_DEV_IPADDR | \
								CONFIG_SESS_IPADDR | CONFIG_RADIUS_SERVER | \
								CONFIG_RADIUS_NASIP | CONFIG_SERVICE_NAME)


#define INSTANCE_CACHE_BLK_ITEMNUM	DEV_MAX_NUM
#define INSTANCE_CACHE_MAX_BLKNUM	0				/* blk num not limit */
#define INSTANCE_CACHE_EMPTY_BLKNUM	1
#define INSTANCE_CACHE_NAME			"pppoe instance cache"

#define THREAD_TIMEOUT				10000	
#define INSTANCE_BACKUP_PORT		2008

enum {
	THREAD_STOP,
	THREAD_RUNNING,
	THREAD_RESTART,
};

typedef enum {
	DEV_THR_INIT = 1,
	DEV_THR_START,
	DEV_THR_STOP,
	DEV_THR_DEAD,
} DevThrState;


typedef enum {
	DEVICE_CREATE = 1,	/* device wil be create*/
	DEVICE_DOWN,		/* device is create, not base interface */
	DEVICE_UP,			/* device is base interface, not create thread */
	DEVICE_RUN,			/* device is create thread */
	DEVICE_DESTROY,		/* device will be destroy */
} DeviceState;

enum {
	VRRP_STATE_INIT = 1,		/* rfc2338.6.4.1 */
	VRRP_STATE_BACK = 2,		/* rfc2338.6.4.2 */
	VRRP_STATE_MAST = 3,		/* rfc2338.6.4.3 */
	VRRP_STATE_LEARN = 4,		/* after init,to learning state,then to mast or back */
	VRRP_STATE_NONE = 5,  
	VRRP_STATE_TRANSFER = 6,	/* state that waiting synchronization data*/
	VRRP_STATE_DISABLE = 99,	/* internal */
};

struct device_thread {
	uint32 id;
	uint32 state, running;
	uint32 sync_switch;
	cpu_set_t coremask;

	backup_struct_t		*backup;
	tbus_connection_t 	*connection;	
	thread_master_t		*master;	
	pppoe_manage_t		*manage;
	
	discover_struct_t 	*discover;
	control_struct_t 	*control;
	radius_struct_t 	*radius;
};

struct device_config {
	uint32 dev_id;
	uint32 status;
	uint32 ipaddr;
	uint32 mask;

	char ifname[IFNAMSIZ];
	char base_ifname[IFNAMSIZ];
	char apply_ifname[IFNAMSIZ];
	char dev_desc[DEV_DESC_LEN];
	char sname[PPPOE_NAMELEN];	

	manage_config_t		*manage;
	discover_config_t	*discover;
	control_config_t	*control;
	radius_config_t		*radius;
};

struct device_struct {
	uint32 id;	
	int ifindex;
	uint32 state;
	uint32 configFlag;

	pthread_t thread_id;
	
	struct device_thread 	thread;
	struct device_config	config;
};

struct instance_struct {
	int sk;	
	uint32 state;

	uint32 count;
	uint32 coremask;

	uint32 slot_id;
	uint32 local_id;
	uint32 instance_id;
	
	uint32 heartlink_local_ip;
	uint32 heartlink_opposite_ip;

	struct pppoe_buf *pbuf;	

	mem_cache_t *cache;
	
	thread_bus_t *tbus;
	tbus_connection_t *connection;

	thread_master_t *master;
	thread_struct_t *thread;
	thread_struct_t *timer;
	
	backup_struct_t *backup;
	struct notifier_struct notifier;	/* backup channel notifier */

	pthread_attr_t attr;
	struct device_struct *dev[DEV_MAX_NUM + 1];	
};

static struct timeval dispatch_timeout = {
        .tv_sec = THREAD_TIMEOUT / 1000000,
        .tv_usec = THREAD_TIMEOUT - (THREAD_TIMEOUT / 1000000)
};


static inline void
thread_state_set(struct device_thread *thread, DevThrState type) {
	thread->state = type;
}

static inline void
thread_setup(instance_struct_t *instance, struct device_thread *thread) {
	CPU_ZERO(&thread->coremask);
	if (instance->coremask & ~0x1) {
		int i;
		for (i = 1; i < 16; i++) {
			if (instance->coremask & (0x1 << i)) {
				CPU_SET(i, &thread->coremask);
			} else {
				break;
			}
		}
	} else {
		CPU_SET(0, &thread->coremask);
	}
}


static inline void
thread_core_setup(struct device_thread *thread) {
	sched_setaffinity(0, sizeof(thread->coremask), &thread->coremask);

//#define COREMASK_DEBUG
#ifdef COREMASK_DEBUG
	cpu_set_t mask;
	int i;
	
	CPU_ZERO(&mask);
	sched_getaffinity(0, sizeof(mask), &mask);
	
	for (i = 0; i < 16; i++) {
		if (CPU_ISSET(i ,&mask)) {
			pppoe_log(LOG_DEBUG, "thread process cpu%d\n", i);
		}
	}
#endif	
}

static inline int
thread_sync_request(struct device_thread *thread) {
	struct instance_sync *sync;
	struct pppoe_buf *pbuf;	
	backup_task_t *task;
	int ret;
	
	pbuf = pbuf_alloc(sizeof(struct instance_sync) + BACKUP_HEADER_LEN);
	if (unlikely(!pbuf)) {
		pppoe_log(LOG_WARNING, "thread alloc pbuf failed\n");
		ret = PPPOEERR_ENOMEM;
		goto error;
	}
	
	pbuf_reserve(pbuf, BACKUP_HEADER_LEN);
	sync = (struct instance_sync *)pbuf_put(pbuf, sizeof(struct instance_sync));
	sync->code = htons(INSTANCE_SESSSYNC_REQUEST);
	sync->dev_id = htons(thread->id);	

	task = backup_task_create(thread->backup, pbuf, BACKUP_INSTANCE_SYNC);
	if (unlikely(!task)) {
		pppoe_log(LOG_WARNING, "backup task create failed\n");
		ret = PPPOEERR_ENOMEM;
		goto error1;
	}

	ret = backup_task_add(thread->backup, task);
	if (ret) {
		pppoe_log(LOG_WARNING, "backup task add failed, ret %d\n", ret);
		goto error2;
	}

	pppoe_log(LOG_INFO, "thread %u send session sync request\n", thread->id);
	return PPPOEERR_SUCCESS;

error2:
	backup_task_destroy(thread->backup, &task);
error1:
	PBUF_FREE(pbuf);
error:
	return ret;
}

static int
device_thread_init(instance_struct_t *instance, struct device_struct *dev) {
	struct device_thread *thread = &dev->thread;

	thread->id = dev->id;
	thread->backup = instance->backup;
	thread->connection = tbus_connection_create(instance->tbus, dev->id);
	if (!thread->connection)	{
		pppoe_log(LOG_ERR, "device thread create tbus connection %u fail\n", dev->id);
		goto error;
	}

	thread->master = thread_master_create();
	if (!thread->master) {
		pppoe_log(LOG_ERR, "device thread create master thread fail\n");
		goto error1;
	}

	thread->manage = pppoe_manage_init(thread->master, instance->backup, 
									thread->connection, dev->config.manage);
	if (!thread->manage) {
		pppoe_log(LOG_ERR, "pppoe device manage init fail\n");
		goto error2;
	}
	
	thread->discover = pppoe_discover_init(thread->master, thread->manage, dev->config.discover);
	if (!thread->discover) {
		pppoe_log(LOG_ERR, "pppoe device discover init fail\n");
		goto error3;
	}

	thread->control = pppoe_control_init(thread->master, thread->manage, dev->config.control);
	if (!thread->control) {
		pppoe_log(LOG_ERR, "pppoe device control init fail\n");
		goto error4;
	}

	thread->radius = pppoe_radius_init(thread->master, thread->manage, dev->config.radius);
	if (!thread->radius) {
		pppoe_log(LOG_ERR, "pppoe device radius init fail\n");
		goto error5;
	}

	/* thread setup */
	thread_setup(instance, thread);
	thread_state_set(thread, DEV_THR_INIT);
	pppoe_log(LOG_INFO, "thread %u init success\n", thread->id);
	return PPPOEERR_SUCCESS;


error5:
	pppoe_control_destroy(&thread->control);
error4:
	pppoe_discover_destroy(&thread->discover);
error3:
	pppoe_manage_destroy(&thread->manage);
error2:
	thread_master_destroy(thread->master);
error1:
	tbus_connection_destroy(&thread->connection);
error:
	thread->backup = NULL;
	return PPPOEERR_ENOMEM;
}

static inline void
device_thread_exit(struct device_struct *dev) {
	struct device_thread *thread = &dev->thread;

	tbus_connection_destroy(&thread->connection);
	pppoe_discover_destroy(&thread->discover);
	pppoe_control_destroy(&thread->control);
	pppoe_radius_destroy(&thread->radius);
	pppoe_manage_destroy(&thread->manage);	
	thread_master_destroy(thread->master);
	
	pppoe_log(LOG_INFO, "thread %u exit success\n", thread->id);
	memset(thread, 0, sizeof(struct device_thread));	
}


static inline int
device_thread_start(struct device_thread *thread) {
	int ret;

	switch (backup_status(thread->backup)) {
	case BACKUP_NONE:
	case BACKUP_ACTIVE:
		ret = pppoe_manage_start(thread->manage);
		if (unlikely(ret))
			goto error;

		ret = pppoe_discover_start(thread->discover);
		if (unlikely(ret))
			goto error1;

		ret = pppoe_control_start(thread->control);
		if (unlikely(ret))
			goto error2;

		ret = pppoe_radius_start(thread->radius);
		if (unlikely(ret))
			goto error3;

		break;

	case BACKUP_STANDBY:
		ret = pppoe_manage_start(thread->manage);
		if (unlikely(ret))
			goto error;

		thread->sync_switch = 1;
		if (backup_channel_state(thread->backup))
			thread_sync_request(thread);
		
		break;

	case BACKUP_DISABLE:
		ret = pppoe_manage_start(thread->manage);
		if (unlikely(ret))
			goto error;

		break;
	}

	thread_state_set(thread, DEV_THR_START);
	pppoe_log(LOG_INFO, "thread %u start success\n", thread->id);
	return PPPOEERR_SUCCESS;	

error3:
	pppoe_control_stop(thread->control);
error2:
	pppoe_discover_stop(thread->discover);
error1:
	pppoe_manage_stop(thread->manage);
error:
	return ret;
}	

static inline void 
device_thread_dispatch(struct device_thread *thread) {
	thread->running = THREAD_RUNNING;

	switch (backup_status(thread->backup)) {
	case BACKUP_NONE:
	case BACKUP_ACTIVE:
	case BACKUP_STANDBY:
		while (THREAD_RUNNING == thread->running) {
			thread_dispatch(thread->master, dispatch_timeout);
			tbus_connection_dispatch(thread->connection, 10);
		}	
		break;

	case BACKUP_DISABLE:
		while (THREAD_RUNNING == thread->running) {
			usleep(THREAD_TIMEOUT);
		}	
		break;
	}

	thread_state_set(thread, DEV_THR_STOP);	
}

static inline void
device_thread_stop(struct device_thread *thread) {
	switch (backup_status(thread->backup)) {
	case BACKUP_NONE:
	case BACKUP_ACTIVE:
		pppoe_manage_stop(thread->manage);
		pppoe_discover_stop(thread->discover);
		pppoe_control_stop(thread->control);
		pppoe_radius_stop(thread->radius);
		break;

	case BACKUP_STANDBY:
		thread->sync_switch = 0;
		pppoe_manage_stop(thread->manage);		
		break;
		
	case BACKUP_DISABLE:
		pppoe_manage_stop(thread->manage);		
		break;
	}
	
	thread_state_set(thread, DEV_THR_DEAD);
	pppoe_log(LOG_INFO, "thread %u stop success\n", thread->id);
}

static inline int
device_thread_restart(struct device_thread *thread) {
	int ret;

	switch (backup_prevstatus(thread->backup)) {
	case BACKUP_NONE:
	case BACKUP_ACTIVE:
		pppoe_discover_stop(thread->discover);
		pppoe_control_stop(thread->control);
		pppoe_radius_stop(thread->radius);
		break;

	case BACKUP_STANDBY:
		thread->sync_switch = 0;
		break;

	case BACKUP_DISABLE:
		break;
	}

	ret = pppoe_manage_restart(thread->manage);
	if (unlikely(ret))
		goto error;

	switch (backup_status(thread->backup)) {
	case BACKUP_NONE:
	case BACKUP_ACTIVE:
		ret = pppoe_discover_start(thread->discover);
		if (unlikely(ret))
			goto error1;

		ret = pppoe_control_start(thread->control);
		if (unlikely(ret))
			goto error2;

		ret = pppoe_radius_start(thread->radius);
		if (unlikely(ret))
			goto error3;
		
		break;

	case BACKUP_STANDBY:
		thread->sync_switch = 1;		
		if (backup_channel_state(thread->backup))
			thread_sync_request(thread);
		
		break;
		
	case BACKUP_DISABLE:
		break;
	}

	thread_state_set(thread, DEV_THR_START);
	pppoe_log(LOG_INFO, "thread %u restart success\n", thread->id);
	return PPPOEERR_SUCCESS;

error3:
	pppoe_control_stop(thread->control);
error2:
	pppoe_discover_stop(thread->discover);
error1:
	pppoe_manage_stop(thread->manage);
error:
	return ret;
}	


static void *
device_thread_run(void *arg) {
	struct device_thread *thread = (struct device_thread *)arg;
	int ret;

	thread_core_setup(thread);
	
	ret = device_thread_start(thread);
	if (unlikely(ret)) {
		pppoe_log(LOG_ERR, "thread %u start failed, ret %d\n",
							thread->id, ret);
		thread_state_set(thread, DEV_THR_DEAD);
		goto out;
	}
	
restart:
	device_thread_dispatch(thread);

	if (THREAD_RESTART == thread->running) {
		ret = device_thread_restart(thread);
		if (unlikely(ret)) {
			pppoe_log(LOG_ERR, "thread %u restart failed, ret %d\n", 
								thread->id, ret);
			thread_state_set(thread, DEV_THR_DEAD);
			goto out;
		}
		
		goto restart;
	}
	
	device_thread_stop(thread);

out:	
	return NULL;
}


static inline void
device_config_setup(instance_struct_t *instance, struct device_struct *dev) {
	struct device_config *config = &dev->config;

	pppoe_manage_config_setup(config->manage, 
					instance->slot_id, instance->local_id, instance->instance_id,
					dev->ifindex, dev->id, config->ipaddr, config->ifname);
	pppoe_discover_config_setup(config->discover, 
					config->base_ifname, config->sname);
	pppoe_control_config_setup(config->control, config->ifname);
	pppoe_radius_config_setup(config->radius, 
					instance->slot_id, instance->local_id, instance->instance_id);
}

static inline int
device_config_init(struct device_config *config) {
	int ret;

	if (unlikely(!config))
		return PPPOEERR_EINVAL;
	
	memset(config, 0, sizeof(struct device_config));

	ret = pppoe_manage_config_init(&config->manage);
	if (unlikely(ret)) {
		goto error;
	}

	ret = pppoe_discover_config_init(&config->discover);
	if (unlikely(ret)) {
		goto error1;
	}

	ret = pppoe_control_config_init(&config->control);
	if (unlikely(ret)) {
		goto error2;
	}

	ret = pppoe_radius_config_init(&config->radius);
	if (unlikely(ret)) {
		goto error3;
	}

	return PPPOEERR_SUCCESS;

error3:
	pppoe_control_config_exit(&config->control);
error2:
	pppoe_discover_config_exit(&config->discover);
error1:
	pppoe_manage_config_exit(&config->manage);
error:
	pppoe_log(LOG_ERR, "device config init fail, ret = %d\n", ret);
	return ret;
};


void
device_config_exit(struct device_config *config) {
	pppoe_manage_config_exit(&config->manage);
	pppoe_discover_config_exit(&config->discover);
	pppoe_control_config_exit(&config->control);
	pppoe_radius_config_exit(&config->radius);
}

static int
device_config_show(struct device_config *config, char **configCmd) {
	char *configStr, *cursor;
	
	configStr = (char *)malloc(CONFIGCMD_SIZE);
	if (!configStr)
		return PPPOEERR_ENOMEM;

	cursor = configStr;
	memset(configStr, 0, CONFIGCMD_SIZE);
	
	cursor += snprintf(cursor, CONFIGCMD_SIZE - (cursor - configStr), "create pppoe-device %u %s\n", 
					config->dev_id, config->dev_desc);

	cursor += snprintf(cursor, CONFIGCMD_SIZE - (cursor - configStr), "config pppoe-device %u\n", 
					config->dev_id);

	if (config->sname[0]) {
		cursor += snprintf(cursor, CONFIGCMD_SIZE - (cursor - configStr), " service name %s\n", 
						config->sname);
	}

	if (config->base_ifname[0]) {
		cursor += snprintf(cursor, CONFIGCMD_SIZE - (cursor - configStr), " base interface %s\n", 
						config->base_ifname);
	}

	if (config->apply_ifname[0]) {
		cursor += snprintf(cursor, CONFIGCMD_SIZE - (cursor - configStr), " apply interface %s\n", 
						config->apply_ifname);
	}
	
	if (config->ipaddr) {
		cursor += snprintf(cursor, CONFIGCMD_SIZE - (cursor - configStr), 
						" ip address %u.%u.%u.%u/%u\n", HIPQUAD(config->ipaddr), config->mask);
	}

	cursor += pppoe_manage_show_running_config(config->manage, cursor, CONFIGCMD_SIZE - (cursor - configStr));
	cursor += pppoe_discover_show_running_config(config->discover, cursor, CONFIGCMD_SIZE - (cursor - configStr));
	cursor += pppoe_control_show_running_config(config->control, cursor, CONFIGCMD_SIZE - (cursor - configStr));
	cursor += pppoe_radius_show_running_config(config->radius, cursor, CONFIGCMD_SIZE - (cursor - configStr));

	if (config->status) {
		cursor += snprintf(cursor, CONFIGCMD_SIZE - (cursor - configStr), " service enable\n");
	}

	cursor += snprintf(cursor, CONFIGCMD_SIZE - (cursor - configStr), " exit\n");

	*configCmd = configStr;
	return PPPOEERR_SUCCESS;
}

static inline struct device_struct *
device_find(instance_struct_t *instance, uint32 dev_id) {
	if (unlikely(!dev_id || dev_id > DEV_MAX_NUM))
		return NULL;
	
	return instance->dev[dev_id];
}

static inline int
device_destroy(instance_struct_t *instance, struct device_struct *dev) {
	if (unlikely(netlink_destroy_interface(instance->sk, 
					pbuf_init(instance->pbuf), dev->ifindex))) {
		pppoe_log(LOG_WARNING, "kernel destroy interface %s fail\n", 
							dev->config.ifname);
		return PPPOEERR_ESYSCALL;
	}

	instance->count--;
	instance->dev[dev->id] = NULL;
	mem_cache_free(instance->cache, dev);
	return PPPOEERR_SUCCESS;
}

static int
instance_device_config_ipaddr(instance_struct_t *instance, uint32 dev_id, 
								uint32 ipaddr, uint32 mask) {
	struct device_struct *dev;
	uint32 tmpMask;
	char cmd[128];
	int status, ret;

	if (unlikely(ipaddr && !mask)) {
		pppoe_log(LOG_WARNING, "input mask is zero\n");
		return PPPOEERR_EINVAL;
	}

	dev = device_find(instance, dev_id);
	if (!dev) {
		pppoe_log(LOG_WARNING, "dev(ID:%d) is not exist\n", dev_id);
		return PPPOEERR_ENOEXIST;
	}

	if (DEVICE_RUN == dev->state) {
		pppoe_log(LOG_WARNING, "dev(ID:%d) service enable, please disable it\n", dev_id);
		return PPPOEERR_ESERVICE;
	}

	memset(cmd, 0, sizeof(cmd));
	if (ipaddr) {		
		tmpMask = ~((1 << (32 - mask)) - 1);
		snprintf(cmd, sizeof(cmd), "ifconfig %s %u.%u.%u.%u netmask %u.%u.%u.%u", 
									dev->config.ifname, HIPQUAD(ipaddr), HIPQUAD(tmpMask));
	} else {
		snprintf(cmd, sizeof(cmd), "ifconfig %s 0.0.0.0", dev->config.ifname);
	}
	
	pppoe_log(LOG_DEBUG, "cmd is %s\n", cmd);

	status = system(cmd);
	ret = WEXITSTATUS(status);
	pppoe_log(LOG_DEBUG, "system status %d, ret %d\n", status, ret);

	if (ret) {
		pppoe_log(LOG_WARNING, "system call failed\n");
		return PPPOEERR_ESYSCALL;
	}

	dev->config.ipaddr = ipaddr;
	dev->config.mask = ipaddr ? mask : 0;
	if (ipaddr) {
		dev->configFlag |= CONFIG_DEV_IPADDR;
	} else {
		dev->configFlag &= ~CONFIG_DEV_IPADDR;
	}
	
	return PPPOEERR_SUCCESS;
}

static int
instance_device_config_virtual_mac(instance_struct_t *instance, 
										uint32 dev_id, uint8 *virtualMac) {
	struct device_struct *dev;

	if (unlikely(!virtualMac)) {
		pppoe_log(LOG_WARNING, "input virtualMac is NULL\n");
		return PPPOEERR_EINVAL;
	}

	dev = device_find(instance, dev_id);
	if (!dev) {
		pppoe_log(LOG_WARNING, "dev(ID:%d) is not exist\n", dev_id);
		return PPPOEERR_ENOEXIST;
	}

	if (DEVICE_RUN == dev->state) {
		pppoe_log(LOG_WARNING, "dev(ID:%d) service enable, please disable it\n", dev_id);
		return PPPOEERR_ESERVICE;
	}

	return pppoe_discover_config_virtual_mac(dev->config.discover, virtualMac);
}


/* minIP, maxIP 0: no sess ipaddr*/
static int
instance_device_config_sess_ipaddr(instance_struct_t *instance, uint32 dev_id,
										uint32 minIP, uint32 maxIP) {
	struct device_struct *dev;
	int ret;	

	dev = device_find(instance, dev_id);
	if (!dev) {
		pppoe_log(LOG_WARNING, "dev(ID:%d) is not exist\n", dev_id);
		ret = PPPOEERR_ENOEXIST;
		goto error;
	}

	if (DEVICE_RUN == dev->state) {
		pppoe_log(LOG_WARNING, "dev(ID:%d) service enable, please disable it\n", dev_id);
		ret = PPPOEERR_ESERVICE;
		goto error;
	}

	ret = pppoe_manage_config_sessions_ipaddr(dev->config.manage, minIP, maxIP);
	if (ret) {
		pppoe_log(LOG_WARNING, "manage config fail, ret = %d\n", ret);
		goto error;
	}
	
	if (maxIP) {
		dev->configFlag |= CONFIG_SESS_IPADDR;
	} else {
		dev->configFlag &= ~CONFIG_SESS_IPADDR;
	}
	
	return PPPOEERR_SUCCESS;

error:
	return ret;
}

/* minIP, maxIP 0: no sess ipaddr*/
static int
instance_device_config_sess_dns(instance_struct_t *instance, uint32 dev_id,
									uint32 dns1, uint32 dns2) {
	struct device_struct *dev;
	int ret;
	
	dev = device_find(instance, dev_id);
	if (!dev) {
		pppoe_log(LOG_WARNING, "dev(ID:%d) is not exist\n", dev_id);
		ret = PPPOEERR_ENOEXIST;
		goto error;
	}

	if (DEVICE_RUN == dev->state) {
		pppoe_log(LOG_WARNING, "dev(ID:%d) service enable, please disable it\n", dev_id);
		ret = PPPOEERR_ESERVICE;
		goto error;
	}

	ret = pppoe_manage_config_sessions_dns(dev->config.manage, dns1, dns2);
	if (ret) {
		pppoe_log(LOG_WARNING, "manage config fail, ret = %d\n", ret);
		goto error;
	}
	
	return PPPOEERR_SUCCESS;

error:
	return ret;
}


/* ipaddr 0: no nasip*/
static int
instance_device_config_nas_ipaddr(instance_struct_t *instance, uint32 dev_id, uint32 ipaddr) {
	struct device_struct *dev;
	int ret;

	dev = device_find(instance, dev_id);
	if (!dev) {
		pppoe_log(LOG_WARNING, "ddev(ID:%d) is not exist\n", dev_id);
		ret = PPPOEERR_ENOEXIST;
		goto error;
	}

	if (DEVICE_RUN == dev->state) {
		pppoe_log(LOG_WARNING, "dev(ID:%d) service enable, please disable it\n", dev_id);
		ret = PPPOEERR_ESERVICE;
		goto error;
	}

	ret = pppoe_radius_config_nas_ipaddr(dev->config.radius, ipaddr);
	if (ret) {
		pppoe_log(LOG_WARNING, "manage config failed, ret = %d\n", ret);
		goto error;
	}

	if (ipaddr) {
		dev->configFlag |= CONFIG_RADIUS_NASIP;
	} else {
		dev->configFlag &= ~CONFIG_RADIUS_NASIP;
	}	
	return PPPOEERR_SUCCESS;

error:
	return ret;
}

/* ipaddr 0: no nasip*/
static int
instance_device_config_radius_rdc(instance_struct_t *instance, uint32 dev_id, 
									uint32 state, uint32 slot_id, uint32 instance_id) {
	struct device_struct *dev;
	int ret;

	dev = device_find(instance, dev_id);
	if (!dev) {
		pppoe_log(LOG_WARNING, "ddev(ID:%d) is not exist\n", dev_id);
		ret = PPPOEERR_ENOEXIST;
		goto error;
	}

	if (DEVICE_RUN == dev->state) {
		pppoe_log(LOG_WARNING, "dev(ID:%d) service enable, please disable it\n", dev_id);
		ret = PPPOEERR_ESERVICE;
		goto error;
	}

	ret = pppoe_radius_config_rdc(dev->config.radius, state, slot_id, instance_id);
	if (ret) {
		pppoe_log(LOG_WARNING, "manage config failed, ret = %d\n", ret);
		goto error;
	}

	if (state) {
		dev->configFlag |= CONFIG_RADIUS_RDC;
	} else {
		dev->configFlag &= ~CONFIG_RADIUS_RDC;
	}	
	return PPPOEERR_SUCCESS;

error:
	return ret;
}

static int
instance_device_config_radius_server(instance_struct_t *instance, uint32 dev_id, struct radius_srv *srv) {
	struct device_struct *dev;
	int ret;
	
	dev = device_find(instance, dev_id);
	if (!dev) {
		pppoe_log(LOG_WARNING, "dev(ID:%d) is not exist\n", dev_id);
		ret = PPPOEERR_ENOEXIST;
		goto error;
	}

	if (DEVICE_RUN == dev->state) {
		pppoe_log(LOG_WARNING, "dev(ID:%d) service enable, please disable it\n", dev_id);
		ret = PPPOEERR_ESERVICE;
		goto error;
	}

	ret = pppoe_radius_config_auth_and_acct_server(dev->config.radius, srv);
	if (ret) {
		pppoe_log(LOG_WARNING, "radius config fail, ret = %d\n", ret);
		goto error;
	}

	if (srv) {
		dev->configFlag |= CONFIG_RADIUS_SERVER;
	} else {
		dev->configFlag &= ~CONFIG_RADIUS_SERVER;
	}
	return PPPOEERR_SUCCESS;

error:
	return ret;
}

/* sname NULL: no service name*/
static int
instance_device_config_service_name(instance_struct_t *instance, uint32 dev_id, char *sname) {
	struct device_struct *dev;
	uint32 length = sname ? strlen(sname) : 0;
	
	dev = device_find(instance, dev_id);
	if (!dev) {
		pppoe_log(LOG_WARNING, "dev(ID:%d) is not exist\n", dev_id);
		return PPPOEERR_ENOEXIST;
	}

	if (DEVICE_RUN == dev->state) {
		pppoe_log(LOG_WARNING, "dev(ID:%d) service enable, please disable it\n", dev_id);
		return PPPOEERR_ESERVICE;
	}

	memset(dev->config.sname, 0, sizeof(dev->config.sname));
	if (length) {
		memcpy(dev->config.sname, sname, length);
		dev->configFlag |= CONFIG_SERVICE_NAME;
	} else {
		dev->configFlag &= ~CONFIG_SERVICE_NAME;	
	}
	
	return PPPOEERR_SUCCESS;
}

static int
instance_device_kick_session_by_sid(instance_struct_t *instance, uint32 dev_id, uint32 sid) {
	struct device_struct *dev;
	struct session_info info;
	int ret;

	if (BACKUP_STANDBY == backup_status(instance->backup)) {
		pppoe_log(LOG_WARNING, "instance backup status is standby\n");
		return PPPOEERR_ESTATE;
	}

	dev = device_find(instance, dev_id);
	if (!dev) {
		pppoe_log(LOG_WARNING, "dev(ID:%u) is not exist\n", dev_id);
		return PPPOEERR_ENOEXIST;
	}

	if (DEVICE_RUN != dev->state) {
		pppoe_log(LOG_WARNING, "dev(ID:%d) is not running\n", dev_id);
		return PPPOEERR_ESERVICE;
	}

	memset(&info, 0, sizeof(info));
	info.sid = sid;

	ret = tbus_send_method_call_with_reply(instance->connection, dev->id,
										PPPOE_METHOD_SESSION_KICK_BY_SESSION_ID, 
										&info, dev->thread.manage, NULL, 0, 0);
	if (ret) {
		pppoe_log(LOG_WARNING, "tbus send method call fail, ret = %d\n", ret);
	}
	
	return ret;
}

static int
instance_device_kick_session_by_mac(instance_struct_t *instance, uint32 dev_id, unsigned char *mac) {
	struct device_struct *dev;
	struct session_info info;
	int ret;

	if (unlikely(!mac))
		return PPPOEERR_EINVAL;

	if (BACKUP_STANDBY == backup_status(instance->backup)) {
		pppoe_log(LOG_WARNING, "instance backup status is standby\n");
		return PPPOEERR_ESTATE;
	}

	dev = device_find(instance, dev_id);
	if (!dev) {
		pppoe_log(LOG_WARNING, "dev(ID:%u) is not exist\n", dev_id);
		return PPPOEERR_ENOEXIST;
	}

	if (DEVICE_RUN != dev->state) {
		pppoe_log(LOG_WARNING, "dev(ID:%d) is not running\n", dev_id);
		return PPPOEERR_ESERVICE;
	}

	memset(&info, 0, sizeof(info));
	memcpy(info.mac, mac, ETH_ALEN);

	
	ret = tbus_send_method_call_with_reply(instance->connection, dev->id, 
										PPPOE_METHOD_SESSION_KICK_BY_MAC, 
										&info, dev->thread.manage, NULL, 0, 0);
	if (ret) {
		pppoe_log(LOG_WARNING, "tbus send method call fail, ret = %d\n", ret);
	}
	
	return ret;
}


static int
instance_device_detect_exist(instance_struct_t *instance, uint32 dev_id) {
	if (device_find(instance, dev_id))
		return PPPOEERR_SUCCESS;

	return PPPOEERR_ENOEXIST;
}
static inline int
instance_device_thread_create(instance_struct_t *instance, uint32 dev_id) {
	struct device_struct *dev;
	int ret;

	dev = device_find(instance, dev_id);
	if (!dev) {
		pppoe_log(LOG_WARNING, "dev(ID:%d) is not exist\n", dev_id);
		ret = PPPOEERR_ENOEXIST;
		goto error;
	}

	if (DEVICE_RUN == dev->state) {
		pppoe_log(LOG_WARNING, "dev(ID:%d) is already create thread\n", dev_id);
		ret = PPPOEERR_ESERVICE;
		goto error;
	}

	if (DEVICE_UP != dev->state) {
		pppoe_log(LOG_WARNING, "dev(ID:%d) is not up\n", dev_id);
		ret = PPPOEERR_ESTATE;
		goto error;
	}

	if (CONFIG_READY != (CONFIG_READY & dev->configFlag)) {
		pppoe_log(LOG_WARNING, "dev(ID:%d) config is not ready\n", dev_id);
		ret = PPPOEERR_ECONFIG;
		goto error;
	}

	/* must setup config before create thread */
	device_config_setup(instance, dev);

	if (device_thread_init(instance, dev)) {
		pppoe_log(LOG_WARNING, "pppoe thread init fail\n");
		ret = PPPOEERR_EPTHREAD;
		goto error;
	}
	
	if (pthread_create(&dev->thread_id, &instance->attr, device_thread_run, &dev->thread)) {
		pppoe_log(LOG_WARNING, "pppoe thread create fail\n");
		ret = PPPOEERR_EPTHREAD;
		goto error1;
	}

	/* wait for thread start */
	while (1) {
		if (DEV_THR_START == dev->thread.state) {
			break;
		} else if (DEV_THR_DEAD == dev->thread.state) {
			pppoe_log(LOG_WARNING, "pppoe thread start failed\n");
			ret = PPPOEERR_EPTHREAD;
			goto error1;
		}
		
		usleep(250000);		/*wait 0.25s*/
	}

	dev->state = DEVICE_RUN;
	dev->config.status = 1;	/*service enable*/
	pppoe_log(LOG_INFO, "pppoe create dev %s thread success\n", dev->config.ifname);
	return PPPOEERR_SUCCESS;


error1:
	device_thread_exit(dev);
error:
	return ret;
};

static inline int
instance_device_thread_destroy(instance_struct_t *instance, uint32 dev_id) {
	struct device_struct *dev;

	dev = device_find(instance, dev_id);
	if (!dev) {
		pppoe_log(LOG_INFO, "dev(%d) is not exist\n", dev_id);
		return PPPOEERR_ENOEXIST;
	}

	if (DEVICE_RUN == dev->state) {		
		/* need edit tell thread exit....*/
		if (dev->thread.running) {		
			dev->thread.running = THREAD_STOP;

			/* wait for thread exit */
			while (DEV_THR_DEAD != dev->thread.state) {
				usleep(250000);		/*wait 0.25s*/
			}
		}
		device_thread_exit(dev);
		dev->state = DEVICE_UP;
		dev->config.status = 0;	/*service disable*/
	}

	return PPPOEERR_SUCCESS;
}

static inline int
instance_device_base_interface(instance_struct_t *instance, uint32 dev_id, char *base_ifname) {
	struct device_struct *dev;
	int ret;

	if (unlikely(!base_ifname))
		return PPPOEERR_EINVAL;

	dev = device_find(instance, dev_id);
	if (!dev) {
		pppoe_log(LOG_WARNING, "dev(ID:%d) is not exist\n", dev_id);
		return PPPOEERR_ENOEXIST;
	}

	if (DEVICE_UP == dev->state ||
		dev->config.base_ifname[0]) {
		pppoe_log(LOG_WARNING, "dev(ID:%d) is already base\n", dev_id);
		return PPPOEERR_EEXIST;
	}

	if (DEVICE_DOWN != dev->state) {
		pppoe_log(LOG_WARNING, "dev(ID:%d) wrong state\n", dev_id);
		return PPPOEERR_ESTATE;
	}

	ret = netlink_base_interface(instance->sk, 
								pbuf_init(instance->pbuf), 
								dev->ifindex, base_ifname);
	if (unlikely(ret)) {
		pppoe_log(LOG_WARNING, "kernel base dev(ID:%d) fail\n", dev_id);
		return ret;
	}
	
	strncpy(dev->config.base_ifname, base_ifname, sizeof(dev->config.base_ifname) - 1);
	dev->state = DEVICE_UP;
	dev->configFlag |= CONFIG_DEV_BASE;
	return PPPOEERR_SUCCESS;
}

static inline int
instance_device_unbase_interface(instance_struct_t *instance, uint32 dev_id) {
	struct device_struct *dev;
	int ret;

	dev = device_find(instance, dev_id);
	if (!dev) {
		pppoe_log(LOG_WARNING, "dev(ID:%d) is not exist\n", dev_id);
		return PPPOEERR_ENOEXIST;
	}

	if (DEVICE_RUN == dev->state) {
		pppoe_log(LOG_WARNING, "dev(ID:%d) service is enable, "
								"please disable it frist\n", dev_id);
		return PPPOEERR_ESERVICE;
	}

	if (DEVICE_UP != dev->state) {
		pppoe_log(LOG_WARNING, "dev(ID:%d) is not base interface\n", dev_id);
		return PPPOEERR_ESTATE;
	}

	ret = netlink_unbase_interface(instance->sk, 
									pbuf_init(instance->pbuf), 
									dev->ifindex);
	if (unlikely(ret)) {
		pppoe_log(LOG_WARNING, "kernel unbase dev(ID:%d) fail\n", dev_id);
		return ret;
	}

	memset(dev->config.base_ifname, 0, sizeof(dev->config.base_ifname));
	dev->state = DEVICE_DOWN;
	dev->configFlag &= ~CONFIG_DEV_BASE;
	return PPPOEERR_SUCCESS;
}

static inline int
instance_device_apply_interface(instance_struct_t *instance, uint32 dev_id, char *apply_ifname) {
	struct device_struct *dev;

	if (unlikely(!apply_ifname))
		return PPPOEERR_EINVAL;

	dev = device_find(instance, dev_id);
	if (!dev) {
		pppoe_log(LOG_WARNING, "dev(ID:%d) is not exist\n", dev_id);
		return PPPOEERR_ENOEXIST;
	}

	if (DEVICE_RUN == dev->state) {
		pppoe_log(LOG_WARNING, "dev(ID:%d) service is enable, "
								"please disable it frist\n", dev_id);
		return PPPOEERR_ESERVICE;
	}

	memset(dev->config.apply_ifname, 0, sizeof(dev->config.apply_ifname));
	strncpy(dev->config.apply_ifname, apply_ifname, sizeof(dev->config.apply_ifname) - 1);
	return PPPOEERR_SUCCESS;
}

static inline int
instance_device_unapply_interface(instance_struct_t *instance, uint32 dev_id) {
	struct device_struct *dev;

	dev = device_find(instance, dev_id);
	if (!dev) {
		pppoe_log(LOG_WARNING, "dev(ID:%d) is not exist\n", dev_id);
		return PPPOEERR_ENOEXIST;
	}

	if (DEVICE_RUN == dev->state) {
		pppoe_log(LOG_WARNING, "dev(ID:%d) service is enable, "
								"please disable it frist\n", dev_id);
		return PPPOEERR_ESERVICE;
	}

	memset(dev->config.apply_ifname, 0, sizeof(dev->config.apply_ifname));	
	return PPPOEERR_SUCCESS;
}

static int
instance_device_create(instance_struct_t *instance, 
					uint32 dev_id, char *ifname, char *dev_desc) {
	struct device_struct *dev;
	int ret;

	if (unlikely(!ifname || !dev_desc))
		return PPPOEERR_EINVAL;	

	if (unlikely(!dev_id || dev_id > DEV_MAX_NUM))
		return PPPOEERR_EINVAL;

	if (device_find(instance, dev_id)) {
		pppoe_log(LOG_INFO, "the dev(ID:%d) is exist\n", dev_id);
		ret = PPPOEERR_EEXIST;
		goto error;
	}

	dev = mem_cache_alloc(instance->cache);
	if (unlikely(!dev)) {
		pppoe_log(LOG_WARNING, "device mem cache create fail\n");
		ret = PPPOEERR_ENOMEM;
		goto error;
	}

	memset(dev, 0, sizeof(*dev));
	
	/*we need tell kernel create pppoe device......*/
	ret = netlink_create_interface(instance->sk, 
									pbuf_init(instance->pbuf), 
									ifname);
	if (ret <= 0) {
		pppoe_log(LOG_WARNING, "kernel create interface %s fail, ret = %d\n", 
								ifname, ret);
		goto error1;
	}

	dev->id = dev_id;
	dev->ifindex = ret;

	ret = device_config_init(&dev->config);
	if (ret) {
		pppoe_log(LOG_WARNING, "pppoe config init failed, ret = %d\n", ret);
		goto error2;
	}

	/* config setup */
	dev->config.dev_id = dev_id;
	strncpy(dev->config.ifname, ifname, IFNAMSIZ - 1);
	strncpy(dev->config.dev_desc, dev_desc, DEV_DESC_LEN - 1);
	
	/* set pppoe device state */
	dev->state = DEVICE_CREATE;

	instance->dev[dev_id] = dev;
	instance->count++;
	dev->state = DEVICE_DOWN;
	return PPPOEERR_SUCCESS;

error2:
	netlink_destroy_interface(instance->sk, 
							pbuf_init(instance->pbuf), 
							dev->ifindex);
error1:
	mem_cache_free(instance->cache, dev);
error:
	return ret;
}

static int
instance_device_destroy(instance_struct_t *instance, uint32 dev_id) {
	struct device_struct *dev;

	dev = device_find(instance, dev_id);
	if (!dev) {
		pppoe_log(LOG_INFO, "dev (%d) is not exist\n", dev_id);
		return PPPOEERR_ENOEXIST;
	}
	
	if (DEVICE_RUN == dev->state) {
		pppoe_log(LOG_WARNING, "device(ID:%d) service is enable, "
								"please disable it frist\n", dev_id);
		return PPPOEERR_ESERVICE;
	}
	
	dev->state = DEVICE_DESTROY;
	
	if (dev->config.base_ifname[0]) {
		netlink_unbase_interface(instance->sk, 
								pbuf_init(instance->pbuf), 
								dev->ifindex);
	}
	
	device_config_exit(&dev->config);	
	return device_destroy(instance, dev);
}

static int
instance_device_show_basic_info(instance_struct_t *instance, 
						struct pppoeDevBasicInfo **array, uint32 *num) {
	struct device_struct *dev;
	int i;	

	if (unlikely(!array || !num))
		return PPPOEERR_EINVAL;

	*array = NULL;
	*num = 0;

	if (!instance->count)
		return PPPOEERR_SUCCESS;

	*array = (struct pppoeDevBasicInfo *)calloc(instance->count, sizeof(struct pppoeDevBasicInfo));
	if (!*array) {
		pppoe_log(LOG_WARNING, "malloc dev array fail\n");
		return PPPOEERR_ENOMEM;
	}

	for (i = 1; i <= DEV_MAX_NUM; i++) {
		dev = device_find(instance, i);
		if (dev) {
			struct pppoeDevBasicInfo *info = &(*array)[*num];
			struct device_config *config = &dev->config;
			
			info->state = dev->state;
			info->dev_id = config->dev_id;
			info->ipaddr = config->ipaddr;
			info->mask = config->mask;
			memcpy(info->ifname, config->ifname, IFNAMSIZ);
			memcpy(info->base_ifname, config->base_ifname, IFNAMSIZ);
			memcpy(info->dev_desc, config->dev_desc, DEV_DESC_LEN);
			
			(*num)++;
		}
	}

	return PPPOEERR_SUCCESS;
}

static inline int
instance_device_show_online_user(instance_struct_t *instance, uint32 dev_id, 
									struct pppoeUserInfo **userList, uint32 *userNum) {
	struct device_struct *dev = device_find(instance, dev_id);
	if (!dev)
		return PPPOEERR_ENOEXIST;

	if (DEVICE_RUN != dev->state)
		return PPPOEERR_ESERVICE;

	return pppoe_manage_show_online_user(dev->thread.manage, userList, userNum);
}

static inline int
instance_device_show_pfm_entry(instance_struct_t *instance, 
									uint32 dev_id, struct pfm_table_entry *entry) {
	struct device_struct *dev = device_find(instance, dev_id);
	if (!dev)
		return PPPOEERR_ENOEXIST;

	memset(entry, 0, sizeof(*entry));
	if (!dev->config.apply_ifname[0])
		return PPPOEERR_SUCCESS;

	entry->opt_para = 0;
	entry->src_port = 0;
	entry->dest_port = 0;	
	entry->slot_id = instance->slot_id;
	entry->sendto = ifname_get_slot_id(dev->config.apply_ifname);
	if (!entry->sendto) {
		pppoe_log(LOG_WARNING, "ifname %s get slot id failed\n",
								dev->config.apply_ifname);
		return PPPOEERR_EINVAL;
	}

	memcpy(entry->ifname, dev->config.apply_ifname, IFNAMSIZ);
	strncpy(entry->src_ipaddr, "all", sizeof(entry->src_ipaddr) - 1);
	snprintf(entry->dest_ipaddr, sizeof(entry->dest_ipaddr), "%u.%u.%u.%u/%u", 
					HIPQUAD(dev->config.ipaddr), dev->config.mask);
	return PPPOEERR_SUCCESS;
}


static inline void
instance_device_free_running_config(char **configCmd, uint32 num) {
	int i;

	if (unlikely(!configCmd))
		return;

	for (i = 0; i < num; i++) {
		PPPOE_FREE(configCmd[i]);
	}

	return;
}

static inline int
instance_device_show_running_config(instance_struct_t *instance, char **configCmd, uint32 *num) {
	struct device_struct *dev;
	char *cmd;
	int i;

	if (unlikely(!configCmd || !num))
		return PPPOEERR_EINVAL;

	memset(configCmd, 0, DEV_MAX_NUM * sizeof(char *));
	*num = 0;

	for (i = 1; i <= DEV_MAX_NUM; i++) {
		dev = device_find(instance, i);
		if (dev) {
			if (PPPOEERR_SUCCESS == device_config_show(&dev->config, &cmd)) {
				configCmd[*num] = cmd;
				(*num)++;
			}
		}
	}

	return PPPOEERR_SUCCESS;
}

static DBusMessage *
dbus_device_create(DBusMessage *message, void *user_data) {
	DBusMessage *reply;	
	DBusError err;
	DBusMessageIter	iter;
	char *ifname, *dev_desc;
	unsigned int dev_id;
	int ret;
	
	dbus_error_init(&err);
	if (!(dbus_message_get_args(message, &err,
					DBUS_TYPE_UINT32, &dev_id,
					DBUS_TYPE_STRING, &ifname,
					DBUS_TYPE_STRING, &dev_desc,
					DBUS_TYPE_INVALID))){
		pppoe_log(LOG_WARNING, "Unable to get input args\n");
		if (dbus_error_is_set(&err)) {
			pppoe_log(LOG_WARNING, "%s raised: %s\n", err.name, err.message);
			dbus_error_free(&err);
		}
		return NULL;
	}

	if (!ifname[0]) {
		pppoe_log(LOG_WARNING, "the input ifname is empty\n");
		ret = PPPOEERR_EINVAL;
		goto out;
	}

	if (strlen(ifname) > (IFNAMSIZ - 1) ||
		strlen(dev_desc) > (DEV_DESC_LEN - 1)) {
		pppoe_log(LOG_WARNING, "the input name length is over max\n");
		ret = PPPOEERR_ELENGTH;
		goto out;
	}

	ret = instance_device_create(user_data, dev_id, ifname, dev_desc);
	pppoe_log(LOG_DEBUG, "after instance_device_create, ret = %d\n", ret);
	
out:	
	reply = dbus_message_new_method_return(message);
	if (unlikely(!reply)) {
		pppoe_log(LOG_ERR, "dbus new method reply failed\n");
		return NULL;
	}	
					
	dbus_message_iter_init_append (reply, &iter);	
	dbus_message_iter_append_basic (&iter, DBUS_TYPE_UINT32, &ret); 

	return reply;
}

static DBusMessage *
dbus_device_destroy(DBusMessage *message, void *user_data) {
	DBusMessage *reply;	
	DBusError err;
	DBusMessageIter	iter;
	unsigned int dev_id;
	int ret;

	dbus_error_init(&err);
	if (!(dbus_message_get_args(message, &err,
					DBUS_TYPE_UINT32, &dev_id,
					DBUS_TYPE_INVALID))){
		pppoe_log(LOG_WARNING, "Unable to get input args\n");
		if (dbus_error_is_set(&err)) {
			pppoe_log(LOG_WARNING, "%s raised: %s\n", err.name, err.message);
			dbus_error_free(&err);
		}
		return NULL;
	}

	ret = instance_device_destroy(user_data, dev_id);
	pppoe_log(LOG_DEBUG, "after instance_device_destroy, ret = %d\n", ret);
	
	reply = dbus_message_new_method_return(message);
	if (unlikely(!reply)) {
		pppoe_log(LOG_ERR, "dbus new method reply failed\n");
		return NULL;
	}	
					
	dbus_message_iter_init_append (reply, &iter);	
	dbus_message_iter_append_basic (&iter, DBUS_TYPE_UINT32, &ret); 

	return reply;
}

static DBusMessage *
dbus_device_config_base(DBusMessage *message, void *user_data) {
	DBusMessage *reply;	
	DBusError err;
	DBusMessageIter	iter;
	unsigned int dev_id;
	char *base_ifname;	
	int ret;

	dbus_error_init(&err);
	if (!(dbus_message_get_args(message, &err,
					DBUS_TYPE_UINT32, &dev_id,
					DBUS_TYPE_STRING, &base_ifname,
					DBUS_TYPE_INVALID))){
		pppoe_log(LOG_WARNING, "Unable to get input args\n");
		if (dbus_error_is_set(&err)) {
			pppoe_log(LOG_WARNING, "%s raised: %s\n", err.name, err.message);
			dbus_error_free(&err);
		}
		return NULL;
	}
	
	if (strlen(base_ifname) > (IFNAMSIZ - 1)) {
		pppoe_log(LOG_WARNING, "the input base_ifname length is over max\n");
		ret = PPPOEERR_ELENGTH;
		goto out;
	}

	if (base_ifname[0]) {
		ret = instance_device_base_interface(user_data, dev_id, base_ifname);	
		pppoe_log(LOG_DEBUG, "after instance_device_base_interface, ret = %d\n", ret);
	
	} else {
		ret = instance_device_unbase_interface(user_data, dev_id);
		pppoe_log(LOG_DEBUG, "after instance_device_unbase_interface, ret = %d\n", ret);
	}
	
out:	
	reply = dbus_message_new_method_return(message);
	if (unlikely(!reply)) {
		pppoe_log(LOG_ERR, "dbus new method reply failed\n");
		return NULL;
	}	
					
	dbus_message_iter_init_append (reply, &iter);	
	dbus_message_iter_append_basic (&iter, DBUS_TYPE_UINT32, &ret); 

	return reply;
}

static DBusMessage *
dbus_device_config_apply(DBusMessage *message, void *user_data) {
	DBusMessage *reply;	
	DBusError err;
	DBusMessageIter	iter;
	unsigned int dev_id;
	char *apply_ifname;	
	int ret;

	dbus_error_init(&err);
	if (!(dbus_message_get_args(message, &err,
					DBUS_TYPE_UINT32, &dev_id,
					DBUS_TYPE_STRING, &apply_ifname,
					DBUS_TYPE_INVALID))){
		pppoe_log(LOG_WARNING, "Unable to get input args\n");
		if (dbus_error_is_set(&err)) {
			pppoe_log(LOG_WARNING, "%s raised: %s\n", err.name, err.message);
			dbus_error_free(&err);
		}
		return NULL;
	}
	
	if (strlen(apply_ifname) > (IFNAMSIZ - 1)) {
		pppoe_log(LOG_WARNING, "the input apply_ifname length is over max\n");
		ret = PPPOEERR_ELENGTH;
		goto out;
	}

	if (apply_ifname[0]) {
		ret = instance_device_apply_interface(user_data, dev_id, apply_ifname);
		pppoe_log(LOG_DEBUG, "after instance_device_apply_interface, ret = %d\n", ret);
	} else {
		ret = instance_device_unapply_interface(user_data, dev_id);
		pppoe_log(LOG_DEBUG, "after instance_device_unapply_interface, ret = %d\n", ret);
	}
	
out:	
	reply = dbus_message_new_method_return(message);
	if (unlikely(!reply)) {
		pppoe_log(LOG_ERR, "dbus new method reply failed\n");
		return NULL;
	}	
					
	dbus_message_iter_init_append(reply, &iter);	
	dbus_message_iter_append_basic(&iter, DBUS_TYPE_UINT32, &ret); 

	return reply;
}

static DBusMessage *
dbus_device_config_service(DBusMessage *message, void *user_data) {
	DBusMessage *reply;	
	DBusError err;
	DBusMessageIter	iter;
	uint32 dev_id, state;
	int ret;

	dbus_error_init(&err);
	if (!(dbus_message_get_args(message, &err,
					DBUS_TYPE_UINT32, &dev_id,
					DBUS_TYPE_UINT32, &state,
					DBUS_TYPE_INVALID))){
		pppoe_log(LOG_WARNING, "Unable to get input args\n");
		if (dbus_error_is_set(&err)) {
			pppoe_log(LOG_WARNING, "%s raised: %s\n", err.name, err.message);
			dbus_error_free(&err);
		}
		return NULL;
	}

	if (state) {			
		ret = instance_device_thread_create(user_data, dev_id);
		pppoe_log(LOG_DEBUG, "after instance_device_thread_create, ret = %d\n", ret);
	} else {
		ret = instance_device_thread_destroy(user_data, dev_id);
		pppoe_log(LOG_DEBUG, "after instance_device_thread_destroy, ret = %d\n", ret);
	}

	reply = dbus_message_new_method_return(message);
	if (unlikely(!reply)) {
		pppoe_log(LOG_ERR, "dbus new method reply failed\n");
		return NULL;
	}	
				
	dbus_message_iter_init_append (reply, &iter);	
	dbus_message_iter_append_basic (&iter, DBUS_TYPE_UINT32, &ret); 

	return reply;
}

static DBusMessage *
dbus_device_show_pfm_entry(DBusMessage *message, void *user_data) {
	DBusMessage *reply;	
	DBusError err;
	DBusMessageIter iter;		
	struct pfm_table_entry entry;
	char *ifname, *srcip, *dstip;	
	uint32 dev_id;
	int ret;	

	dbus_error_init(&err);
	if (!(dbus_message_get_args(message, &err,
					DBUS_TYPE_UINT32, &dev_id,
					DBUS_TYPE_INVALID))){
		pppoe_log(LOG_WARNING, "Unable to get input args\n");
		if (dbus_error_is_set(&err)) {
			pppoe_log(LOG_WARNING, "%s raised: %s\n", err.name, err.message);
			dbus_error_free(&err);
		}
		return NULL;
	}

	ret = instance_device_show_pfm_entry(user_data, dev_id, &entry);
	pppoe_log(LOG_DEBUG, "instance_device_show_pfm_entry, ret %d\n", ret);
    
	reply = dbus_message_new_method_return(message);
	if (unlikely(!reply)) {
		pppoe_log(LOG_ERR, "dbus new method reply failed\n");
		return NULL;
	}	

	dbus_message_iter_init_append(reply, &iter);

	dbus_message_iter_append_basic(&iter,
								DBUS_TYPE_UINT32,
								&ret);
	if (PPPOEERR_SUCCESS == ret) {
		ifname = entry.ifname;
		srcip = entry.src_ipaddr;
		dstip = entry.dest_ipaddr;

		dbus_message_iter_append_basic(&iter,
									DBUS_TYPE_UINT32,
									&entry.opt_para);
		
		dbus_message_iter_append_basic(&iter,
									DBUS_TYPE_STRING,
									&ifname);

		dbus_message_iter_append_basic(&iter,
									DBUS_TYPE_STRING,
									&srcip);
		
		dbus_message_iter_append_basic(&iter,
									DBUS_TYPE_UINT32,
									&entry.src_port);

		dbus_message_iter_append_basic(&iter,
									DBUS_TYPE_STRING,
									&dstip);
		
		dbus_message_iter_append_basic(&iter,
									DBUS_TYPE_UINT32,
									&entry.dest_port);

		dbus_message_iter_append_basic(&iter,
									DBUS_TYPE_UINT32,
									&entry.sendto);

		dbus_message_iter_append_basic(&iter,
									DBUS_TYPE_UINT32,
									&entry.slot_id);		
	} 

	return reply;        
}

static DBusMessage *
dbus_detect_device_exist(DBusMessage *message, void *user_data) {
	DBusMessage *reply;	
	DBusError err;
	DBusMessageIter	iter;
	uint32 dev_id;
	int ret;
	
	dbus_error_init(&err);
	if (!(dbus_message_get_args(message, &err,
					DBUS_TYPE_UINT32, &dev_id,
					DBUS_TYPE_INVALID))){
		pppoe_log(LOG_WARNING, "Unable to get input args\n");
		if (dbus_error_is_set(&err)) {
			pppoe_log(LOG_WARNING, "%s raised: %s\n", err.name, err.message);
			dbus_error_free(&err);
		}
		return NULL;
	}

	ret = instance_device_detect_exist(user_data, dev_id);
	
	reply = dbus_message_new_method_return(message);
	if (unlikely(!reply)) {
		pppoe_log(LOG_ERR, "dbus new method reply failed\n");
		return NULL;
	}	
					
	dbus_message_iter_init_append (reply, &iter);	
	dbus_message_iter_append_basic (&iter, DBUS_TYPE_UINT32, &ret); 

	return reply;
}

static DBusMessage *
dbus_device_config_ipaddr(DBusMessage *message, void *user_data) {
	DBusMessage *reply;	
	DBusError err;
	DBusMessageIter	iter;
	uint32 dev_id, ipaddr, mask;
	int ret;
	
	dbus_error_init(&err);
	if (!(dbus_message_get_args(message, &err,
					DBUS_TYPE_UINT32, &dev_id,
					DBUS_TYPE_UINT32, &ipaddr,
					DBUS_TYPE_UINT32, &mask,
					DBUS_TYPE_INVALID))){
		pppoe_log(LOG_WARNING, "Unable to get input args\n");
		if (dbus_error_is_set(&err)) {
			pppoe_log(LOG_WARNING, "%s raised: %s\n", err.name, err.message);
			dbus_error_free(&err);
		}
		return NULL;
	}

	if ((!ipaddr && mask) || (ipaddr && !mask)) {
		ret = PPPOEERR_EINVAL;
		goto out;
	}
	
	if (ipaddr && !(ipaddr & 0xff000000)) {
		ret = PPPOEERR_EINVAL;
		goto out;
	}

	if (mask > 32) {
		ret = PPPOEERR_EINVAL;
		goto out;
	}

	ret = instance_device_config_ipaddr(user_data, dev_id, ipaddr, mask);
	pppoe_log(LOG_DEBUG, "after instance_device_config_ipaddr, ret = %d\n", ret);
	
out:	
	reply = dbus_message_new_method_return(message);
	if (unlikely(!reply)) {
		pppoe_log(LOG_ERR, "dbus new method reply failed\n");
		return NULL;
	}	
					
	dbus_message_iter_init_append (reply, &iter);	
	dbus_message_iter_append_basic (&iter, DBUS_TYPE_UINT32, &ret); 

	return reply;
}

static DBusMessage *
dbus_device_config_virtual_mac(DBusMessage *message, void *user_data) {
	DBusMessage *reply;	
	DBusError err;
	DBusMessageIter	iter;
	uint32 dev_id;
	uint8 virtualMac[ETH_ALEN];
	int ret;
	
	dbus_error_init(&err);
	if (!(dbus_message_get_args(message, &err,
					DBUS_TYPE_UINT32, &dev_id,
					DBUS_TYPE_BYTE, &virtualMac[0],
					DBUS_TYPE_BYTE, &virtualMac[1],
					DBUS_TYPE_BYTE, &virtualMac[2],
					DBUS_TYPE_BYTE, &virtualMac[3],
					DBUS_TYPE_BYTE, &virtualMac[4],
					DBUS_TYPE_BYTE, &virtualMac[5],
					DBUS_TYPE_INVALID))){
		pppoe_log(LOG_WARNING, "Unable to get input args\n");
		if (dbus_error_is_set(&err)) {
			pppoe_log(LOG_WARNING, "%s raised: %s\n", err.name, err.message);
			dbus_error_free(&err);
		}
		return NULL;
	}

	ret = instance_device_config_virtual_mac(user_data, dev_id, virtualMac);
	pppoe_log(LOG_DEBUG, "after instance_device_config_virtual_mac, ret = %d\n", ret);
	
	reply = dbus_message_new_method_return(message);
	if (unlikely(!reply)) {
		pppoe_log(LOG_ERR, "dbus new method reply failed\n");
		return NULL;
	}	
					
	dbus_message_iter_init_append (reply, &iter);	
	dbus_message_iter_append_basic (&iter, DBUS_TYPE_UINT32, &ret); 

	return reply;
}


static DBusMessage *
dbus_device_config_session_ipaddr(DBusMessage *message, void *user_data) {
	DBusMessage *reply;	
	DBusError err;
	DBusMessageIter	iter;
	uint32 dev_id, minIP, maxIP;
	int ret;
	
	dbus_error_init(&err);
	if (!(dbus_message_get_args(message, &err,
					DBUS_TYPE_UINT32, &dev_id,
					DBUS_TYPE_UINT32, &minIP,
					DBUS_TYPE_UINT32, &maxIP,
					DBUS_TYPE_INVALID))){
		pppoe_log(LOG_WARNING, "Unable to get input args\n");
		if (dbus_error_is_set(&err)) {
			pppoe_log(LOG_WARNING, "%s raised: %s\n", err.name, err.message);
			dbus_error_free(&err);
		}
		return NULL;
	}

	if (minIP && !(minIP & 0xff000000)) {
		ret = PPPOEERR_EINVAL;
		goto out;
	}

	if (maxIP && !(maxIP & 0xff000000)) {
		ret = PPPOEERR_EINVAL;
		goto out;
	}

	if ((minIP > maxIP) || (!minIP && maxIP)) {
		ret = PPPOEERR_EINVAL;
		goto out;
	}
	
	ret = instance_device_config_sess_ipaddr(user_data, dev_id, minIP, maxIP);
	pppoe_log(LOG_DEBUG, "after instance_device_config_sess_ipaddr, ret = %d\n", ret);

out:
	reply = dbus_message_new_method_return(message);
	if (unlikely(!reply)) {
		pppoe_log(LOG_ERR, "dbus new method reply failed\n");
		return NULL;
	}	
					
	dbus_message_iter_init_append (reply, &iter);	
	dbus_message_iter_append_basic (&iter, DBUS_TYPE_UINT32, &ret); 

	return reply;
}

static DBusMessage *
dbus_device_config_session_dns(DBusMessage *message, void *user_data) {
	DBusMessage *reply;	
	DBusError err;
	DBusMessageIter	iter;
	uint32 dev_id, dns1, dns2;
	int ret;
	
	dbus_error_init(&err);
	if (!(dbus_message_get_args(message, &err,
					DBUS_TYPE_UINT32, &dev_id,
					DBUS_TYPE_UINT32, &dns1,
					DBUS_TYPE_UINT32, &dns2,
					DBUS_TYPE_INVALID))){
		pppoe_log(LOG_WARNING, "Unable to get input args\n");
		if (dbus_error_is_set(&err)) {
			pppoe_log(LOG_WARNING, "%s raised: %s\n", err.name, err.message);
			dbus_error_free(&err);
		}
		return NULL;
	}

	if (dns1 && !(dns1 & 0xff000000)) {
		ret = PPPOEERR_EINVAL;
		goto out;
	}

	if (dns2 && !(dns2 & 0xff000000)) {
		ret = PPPOEERR_EINVAL;
		goto out;
	}

	if (!dns1 && dns2) {
		ret = PPPOEERR_EINVAL;
		goto out;
	}
	
	ret = instance_device_config_sess_dns(user_data, dev_id, dns1, dns2);
	pppoe_log(LOG_DEBUG, "after instance_device_config_sess_dns, ret = %d\n", ret);

out:
	reply = dbus_message_new_method_return(message);
	if (unlikely(!reply)) {
		pppoe_log(LOG_ERR, "dbus new method reply failed\n");
		return NULL;
	}	
					
	dbus_message_iter_init_append (reply, &iter);	
	dbus_message_iter_append_basic (&iter, DBUS_TYPE_UINT32, &ret); 

	return reply;
}

static DBusMessage *
dbus_device_config_nas_ipaddr(DBusMessage *message, void *user_data) {
	DBusMessage *reply;	
	DBusError err;
	DBusMessageIter	iter;
	uint32 dev_id, nasip;
	int ret;
	
	dbus_error_init(&err);
	if (!(dbus_message_get_args(message, &err,
					DBUS_TYPE_UINT32, &dev_id,
					DBUS_TYPE_UINT32, &nasip,
					DBUS_TYPE_INVALID))){
		pppoe_log(LOG_WARNING, "Unable to get input args\n");
		if (dbus_error_is_set(&err)) {
			pppoe_log(LOG_WARNING, "%s raised: %s\n", err.name, err.message);
			dbus_error_free(&err);
		}
		return NULL;
	}

	if (nasip && !(nasip & 0xff000000)) {
		ret = PPPOEERR_EINVAL;
		goto out;
	}
		
	ret = instance_device_config_nas_ipaddr(user_data, dev_id, nasip);
	pppoe_log(LOG_DEBUG, "after instance_device_config_nas_ipaddr, ret = %d\n", ret);

out:
	reply = dbus_message_new_method_return(message);
	if (unlikely(!reply)) {
		pppoe_log(LOG_ERR, "dbus new method reply failed\n");
		return NULL;
	}	
					
	dbus_message_iter_init_append (reply, &iter);	
	dbus_message_iter_append_basic (&iter, DBUS_TYPE_UINT32, &ret); 

	return reply;
}

static DBusMessage *
dbus_device_config_radius_rdc(DBusMessage *message, void *user_data) {
	DBusMessage *reply;	
	DBusError err;
	DBusMessageIter	iter;
	uint32 dev_id, state, slot_id, instance_id;
	int ret;
	
	dbus_error_init(&err);
	if (!(dbus_message_get_args(message, &err,
					DBUS_TYPE_UINT32, &dev_id,
					DBUS_TYPE_UINT32, &state,
					DBUS_TYPE_UINT32, &slot_id,
					DBUS_TYPE_UINT32, &instance_id,
					DBUS_TYPE_INVALID))){
		pppoe_log(LOG_WARNING, "Unable to get input args\n");
		if (dbus_error_is_set(&err)) {
			pppoe_log(LOG_WARNING, "%s raised: %s\n", err.name, err.message);
			dbus_error_free(&err);
		}
		return NULL;
	}

	ret = instance_device_config_radius_rdc(user_data, dev_id, 
									state, slot_id, instance_id);
	pppoe_log(LOG_DEBUG, "after instance_device_config_radius_rdc, ret %d\n", ret);

	reply = dbus_message_new_method_return(message);
	if (unlikely(!reply)) {
		pppoe_log(LOG_ERR, "dbus new method reply failed\n");
		return NULL;
	}	
					
	dbus_message_iter_init_append (reply, &iter);	
	dbus_message_iter_append_basic (&iter, DBUS_TYPE_UINT32, &ret); 

	return reply;
}


static DBusMessage *
dbus_device_config_radius_server(DBusMessage *message, void *user_data) {
	DBusMessage *reply;	
	DBusError err;
	DBusMessageIter	iter;
	uint32 dev_id;
	struct radius_srv srv;
	char *auth_secret = NULL, 
		*acct_secret = NULL, 
		*backup_auth_secret = NULL,
		*backup_acct_secret = NULL;
	int ret;

	memset(&srv, 0, sizeof(struct radius_srv));

	dbus_error_init(&err);	
	if (!(dbus_message_get_args(message, &err,
					DBUS_TYPE_UINT32, &dev_id,
					DBUS_TYPE_UINT32, &srv.auth.ip,
					DBUS_TYPE_UINT32, &srv.auth.port,
					DBUS_TYPE_UINT32, &srv.auth.secretlen,
					DBUS_TYPE_STRING, &auth_secret,
					DBUS_TYPE_UINT32, &srv.acct.ip,
					DBUS_TYPE_UINT32, &srv.acct.port,
					DBUS_TYPE_UINT32, &srv.acct.secretlen,
					DBUS_TYPE_STRING, &acct_secret,
					DBUS_TYPE_UINT32, &srv.backup_auth.ip,
					DBUS_TYPE_UINT32, &srv.backup_auth.port,
					DBUS_TYPE_UINT32, &srv.backup_auth.secretlen,
					DBUS_TYPE_STRING, &backup_auth_secret,
					DBUS_TYPE_UINT32, &srv.backup_acct.ip,
					DBUS_TYPE_UINT32, &srv.backup_acct.port,
					DBUS_TYPE_UINT32, &srv.backup_acct.secretlen,
					DBUS_TYPE_STRING, &backup_acct_secret,
					DBUS_TYPE_INVALID))){
		pppoe_log(LOG_WARNING, "Unable to get input args\n");
		if (dbus_error_is_set(&err)) {
			pppoe_log(LOG_WARNING, "%s raised: %s\n", err.name, err.message);
			dbus_error_free(&err);
		}
		return NULL;
	}

	if (!srv.auth.ip && srv.backup_auth.ip) {
		ret = PPPOEERR_EINVAL;
		goto out;
	}

	if (!srv.acct.ip && srv.backup_acct.ip) {
		ret = PPPOEERR_EINVAL;
		goto out;
	}

	if ((!srv.auth.ip && srv.acct.ip) || 
		(srv.auth.ip && !srv.acct.ip)) {
		ret = PPPOEERR_EINVAL;
		goto out;
	}

	if ((!srv.backup_auth.ip && srv.backup_acct.ip) || 
		(srv.backup_auth.ip && !srv.backup_acct.ip)) {
		ret = PPPOEERR_EINVAL;
		goto out;
	}

	if (!srv.auth.ip && !srv.acct.ip) {
		ret = instance_device_config_radius_server(user_data, dev_id, NULL);
	} else {
		if (!auth_secret || !acct_secret || !backup_auth_secret || !backup_acct_secret) {
			ret = PPPOEERR_EINVAL;
			goto out;	
		}

		if (strlen(auth_secret) != srv.auth.secretlen || 
			srv.auth.secretlen > (sizeof(srv.auth.secret) - 1)) {
			ret = PPPOEERR_EINVAL;
			goto out;	
		}
		
		if (strlen(acct_secret) != srv.acct.secretlen || 
			srv.acct.secretlen > (sizeof(srv.acct.secret) - 1)) {
			ret = PPPOEERR_EINVAL;
			goto out;	
		}

		memcpy(srv.auth.secret, auth_secret, srv.auth.secretlen);
		memcpy(srv.acct.secret, acct_secret, srv.acct.secretlen);
		

		if (srv.backup_auth.ip && srv.backup_acct.ip &&
			srv.backup_auth.secretlen && srv.backup_acct.secretlen) {
			if (strlen(backup_auth_secret) != srv.backup_auth.secretlen || 
				srv.backup_auth.secretlen > (sizeof(srv.backup_auth.secret) - 1)) {
				ret = PPPOEERR_EINVAL;
				goto out;	
			}

			if (strlen(backup_acct_secret) != srv.backup_acct.secretlen || 
				srv.backup_acct.secretlen > (sizeof(srv.backup_acct.secret) - 1)) {
				ret = PPPOEERR_EINVAL;
				goto out;	
			}
			
			memcpy(srv.backup_auth.secret, backup_auth_secret, srv.backup_auth.secretlen);
			memcpy(srv.backup_acct.secret, backup_acct_secret, srv.backup_acct.secretlen);
		}
		
		ret = instance_device_config_radius_server(user_data, dev_id, &srv);
	}
	pppoe_log(LOG_DEBUG, "after instance_device_config_radius_server, ret = %d\n", ret);
	
out:
	reply = dbus_message_new_method_return(message);
	if (unlikely(!reply)) {
		pppoe_log(LOG_ERR, "dbus new method reply failed\n");
		return NULL;
	}	
					
	dbus_message_iter_init_append (reply, &iter);	
	dbus_message_iter_append_basic (&iter, DBUS_TYPE_UINT32, &ret); 

	return reply;
}


static DBusMessage *
dbus_device_config_sname(DBusMessage *message, void *user_data) {
	DBusMessage *reply;	
	DBusError err;
	DBusMessageIter	iter;
	uint32 dev_id;
	char *sname = NULL;
	int ret;
	
	dbus_error_init(&err);
	if (!(dbus_message_get_args(message, &err,
					DBUS_TYPE_UINT32, &dev_id,
					DBUS_TYPE_STRING, &sname,
					DBUS_TYPE_INVALID))){
		pppoe_log(LOG_WARNING, "Unable to get input args\n");
		if (dbus_error_is_set(&err)) {
			pppoe_log(LOG_WARNING, "%s raised: %s\n", err.name, err.message);
			dbus_error_free(&err);
		}
		return NULL;
	}

	if (!sname || 0 == strlen(sname)) {
		sname = NULL;
	} else if (strlen(sname) > (PPPOE_NAMELEN -1)) {
		pppoe_log(LOG_WARNING, "input service name length is too long\n");
		ret = PPPOEERR_EINVAL;
		goto out;
	}

	ret = instance_device_config_service_name(user_data, dev_id, sname);
	pppoe_log(LOG_DEBUG, "after instance_device_config_service_name, ret = %d\n", ret);

out:
	reply = dbus_message_new_method_return(message);
	if (unlikely(!reply)) {
		pppoe_log(LOG_ERR, "dbus new method reply failed\n");
		return NULL;
	}	
					
	dbus_message_iter_init_append (reply, &iter);	
	dbus_message_iter_append_basic (&iter, DBUS_TYPE_UINT32, &ret); 

	return reply;
}


static DBusMessage *
dbus_device_kick_user(DBusMessage *message, void *user_data) {
	DBusMessage *reply;	
	DBusError err;
	DBusMessageIter	iter;
	uint32 dev_id, sid;
	uint8 mac[ETH_ALEN];
	int ret;
	
	dbus_error_init(&err);
	if (!(dbus_message_get_args(message, &err,
					DBUS_TYPE_UINT32, &dev_id,
					DBUS_TYPE_UINT32, &sid,
					DBUS_TYPE_BYTE,	&mac[0],
					DBUS_TYPE_BYTE,	&mac[1],
					DBUS_TYPE_BYTE,	&mac[2],
					DBUS_TYPE_BYTE,	&mac[3],
					DBUS_TYPE_BYTE,	&mac[4],
					DBUS_TYPE_BYTE,	&mac[5],
					DBUS_TYPE_INVALID))){
		pppoe_log(LOG_WARNING, "Unable to get input args\n");
		if (dbus_error_is_set(&err)) {
			pppoe_log(LOG_WARNING, "%s raised: %s\n", err.name, err.message);
			dbus_error_free(&err);
		}
		return NULL;
	}

	if (sid) {
		ret = instance_device_kick_session_by_sid(user_data, dev_id, sid);
	} else {
		ret = instance_device_kick_session_by_mac(user_data, dev_id, mac);
	}
	pppoe_log(LOG_DEBUG, "after device %u kick session, sid %u, "
					"mac %02X:%02X:%02X:%02X:%02X:%02X, ret %d\n", dev_id, sid,
					mac[0], mac[1], mac[2], mac[3], mac[4], mac[5], ret);

	reply = dbus_message_new_method_return(message);
	if (unlikely(!reply)) {
		pppoe_log(LOG_ERR, "dbus new method reply failed\n");
		return NULL;
	}	
					
	dbus_message_iter_init_append (reply, &iter);	
	dbus_message_iter_append_basic (&iter, DBUS_TYPE_UINT32, &ret); 

	return reply;
}

static DBusMessage *
dbus_device_show_list(DBusMessage *message, void *user_data) {
	DBusMessage *reply;	
	DBusMessageIter iter;		
	DBusMessageIter iter_array;	
	DBusMessageIter iter_struct;
	char *ifname, *base_ifname, *dev_desc;
	struct pppoeDevBasicInfo *array;
	uint32 num;
	int ret;	

	ret = instance_device_show_basic_info(user_data, &array, &num);
	pppoe_log(LOG_DEBUG, "instance_device_show_basic_info, "
						"ret = %d, num = %d\n", ret, num);
    
	reply = dbus_message_new_method_return(message);
	if (unlikely(!reply)) {
		pppoe_log(LOG_ERR, "dbus new method reply failed\n");
		PPPOE_FREE(array);
		return NULL;
	}	

	dbus_message_iter_init_append(reply, &iter);

	dbus_message_iter_append_basic(&iter,
								DBUS_TYPE_UINT32,
								&ret);

	dbus_message_iter_append_basic(&iter,
								DBUS_TYPE_UINT32,
								&num);

    
	dbus_message_iter_open_container(&iter,
								DBUS_TYPE_ARRAY,
								DBUS_STRUCT_BEGIN_CHAR_AS_STRING

									DBUS_TYPE_UINT32_AS_STRING	/*dev_id*/
									DBUS_TYPE_UINT32_AS_STRING	/*state*/
									DBUS_TYPE_UINT32_AS_STRING	/*ipaddr*/
									DBUS_TYPE_UINT32_AS_STRING	/*mask*/
									DBUS_TYPE_STRING_AS_STRING	/*ifname*/
									DBUS_TYPE_STRING_AS_STRING	/*base_ifname*/
									DBUS_TYPE_STRING_AS_STRING	/*dev_desc*/

								DBUS_STRUCT_END_CHAR_AS_STRING,
								&iter_array);
                                    
	if (PPPOEERR_SUCCESS == ret) {   

		int i = 0;
		for (i = 0; i < num; i++) {
			dbus_message_iter_open_container(&iter_array,
										DBUS_TYPE_STRUCT,
										NULL,
										&iter_struct);

			ifname = array[i].ifname;
			base_ifname = array[i].base_ifname;
			dev_desc = array[i].dev_desc;

			dbus_message_iter_append_basic(&iter_struct,
										DBUS_TYPE_UINT32,
										&array[i].dev_id);

			dbus_message_iter_append_basic(&iter_struct,
										DBUS_TYPE_UINT32,
										&array[i].state);

			dbus_message_iter_append_basic(&iter_struct,
										DBUS_TYPE_UINT32,
										&array[i].ipaddr);

			dbus_message_iter_append_basic(&iter_struct,
										DBUS_TYPE_UINT32,
										&array[i].mask);
			
			dbus_message_iter_append_basic(&iter_struct,
										DBUS_TYPE_STRING,
										&ifname);  

			dbus_message_iter_append_basic(&iter_struct,
										DBUS_TYPE_STRING,
										&base_ifname); 

			dbus_message_iter_append_basic(&iter_struct,
										DBUS_TYPE_STRING,
										&dev_desc);  

			dbus_message_iter_close_container(&iter_array, &iter_struct);
		}
	}
	dbus_message_iter_close_container(&iter, &iter_array);    

	PPPOE_FREE(array);
	return reply;        
}


static DBusMessage *
dbus_device_show_online_user(DBusMessage *message, void *user_data) {
	DBusMessage *reply;	
	DBusError err;
	DBusMessageIter iter;		
	DBusMessageIter iter_array;	
	DBusMessageIter iter_struct;
	struct pppoeUserInfo *userarray;
	uint32 dev_id, userNum;
	char *userName;
	int ret;	

	dbus_error_init(&err);
	if (!(dbus_message_get_args(message, &err,
					DBUS_TYPE_UINT32, &dev_id,
					DBUS_TYPE_INVALID))){
		pppoe_log(LOG_WARNING, "Unable to get input args\n");
		if (dbus_error_is_set(&err)) {
			pppoe_log(LOG_WARNING, "%s raised: %s\n", err.name, err.message);
			dbus_error_free(&err);
		}
		return NULL;
	}

	ret = instance_device_show_online_user(user_data, dev_id, &userarray, &userNum);
	pppoe_log(LOG_DEBUG, "instance_device_show_online_user, "
						"ret = %d, num = %d\n", ret, userNum);
    
	reply = dbus_message_new_method_return(message);
	if (unlikely(!reply)) {
		pppoe_log(LOG_ERR, "dbus new method reply failed\n");
		PPPOE_FREE(userarray);
		return NULL;
	}	

	dbus_message_iter_init_append(reply, &iter);

	dbus_message_iter_append_basic(&iter,
								DBUS_TYPE_UINT32,
								&ret);

	dbus_message_iter_append_basic(&iter,
								DBUS_TYPE_UINT32,
								&userNum);

    
	dbus_message_iter_open_container(&iter,
								DBUS_TYPE_ARRAY,
								DBUS_STRUCT_BEGIN_CHAR_AS_STRING

									DBUS_TYPE_UINT32_AS_STRING	/*sid*/
									DBUS_TYPE_UINT32_AS_STRING	/*ipaddr*/
									DBUS_TYPE_UINT32_AS_STRING	/*sessTime*/

									DBUS_TYPE_BYTE_AS_STRING	/*MAC[0]*/
									DBUS_TYPE_BYTE_AS_STRING	/*MAC[1]*/
									DBUS_TYPE_BYTE_AS_STRING	/*MAC[2]*/
									DBUS_TYPE_BYTE_AS_STRING	/*MAC[3]*/
									DBUS_TYPE_BYTE_AS_STRING	/*MAC[4]*/
									DBUS_TYPE_BYTE_AS_STRING	/*MAC[5]*/
	
									DBUS_TYPE_STRING_AS_STRING	/*userName*/

								DBUS_STRUCT_END_CHAR_AS_STRING,
								&iter_array);
                                    
	if (PPPOEERR_SUCCESS == ret && userNum) { 
		int i;
		for (i = 0; i < userNum; i++) {
			dbus_message_iter_open_container(&iter_array,
										DBUS_TYPE_STRUCT,
										NULL,
										&iter_struct);

			userName = userarray[i].username;

			dbus_message_iter_append_basic(&iter_struct,
										DBUS_TYPE_UINT32,
										&userarray[i].sid);

			dbus_message_iter_append_basic(&iter_struct,
										DBUS_TYPE_UINT32,
										&userarray[i].ip);

			dbus_message_iter_append_basic(&iter_struct,
										DBUS_TYPE_UINT32,
										&userarray[i].sessTime);
			

			dbus_message_iter_append_basic(&iter_struct,
										DBUS_TYPE_BYTE,
										&userarray[i].mac[0]);
			
			dbus_message_iter_append_basic(&iter_struct,
										DBUS_TYPE_BYTE,
										&userarray[i].mac[1]);
			
			dbus_message_iter_append_basic(&iter_struct,
										DBUS_TYPE_BYTE,
										&userarray[i].mac[2]);
			
			dbus_message_iter_append_basic(&iter_struct,
										DBUS_TYPE_BYTE,
										&userarray[i].mac[3]);

			dbus_message_iter_append_basic(&iter_struct,
										DBUS_TYPE_BYTE,
										&userarray[i].mac[4]);
			
			dbus_message_iter_append_basic(&iter_struct,
										DBUS_TYPE_BYTE,
										&userarray[i].mac[5]);


			dbus_message_iter_append_basic(&iter_struct,
										DBUS_TYPE_STRING,
										&userName);  

			dbus_message_iter_close_container(&iter_array, &iter_struct);
		}
	}
	dbus_message_iter_close_container(&iter, &iter_array);

	PPPOE_FREE(userarray);
	return reply;        
}

static DBusMessage *
dbus_show_running_config(DBusMessage *message, void *user_data) {
	DBusMessage *reply;	
	DBusMessageIter iter;		
	DBusMessageIter iter_array;	
	DBusMessageIter iter_struct;
	char *configCmd[DEV_MAX_NUM];
	uint32 num;
	int ret;	

	if (unlikely(!user_data)) {
		pppoe_log(LOG_WARNING, "input user_data is NULL\n");		
		return NULL;
	}

	ret = instance_device_show_running_config(user_data, configCmd, &num);
	pppoe_log(LOG_DEBUG, "after instance_device_show_running_config, num %u\n", num);
	
	reply = dbus_message_new_method_return(message);
	if (unlikely(!reply)) {
		pppoe_log(LOG_ERR, "dbus new method reply failed\n");
		return NULL;
	}	

	dbus_message_iter_init_append(reply, &iter);

	dbus_message_iter_append_basic(&iter,
								DBUS_TYPE_UINT32,
								&ret);

	dbus_message_iter_append_basic(&iter,
								DBUS_TYPE_UINT32,
								&num);

	dbus_message_iter_open_container(&iter,
								DBUS_TYPE_ARRAY,
								DBUS_STRUCT_BEGIN_CHAR_AS_STRING

									DBUS_TYPE_STRING_AS_STRING	/*cmd*/

								DBUS_STRUCT_END_CHAR_AS_STRING,
								&iter_array);	

	if (PPPOEERR_SUCCESS == ret) {   
		int i;
		for (i = 0; i < num; i++) {
			dbus_message_iter_open_container(&iter_array,
										DBUS_TYPE_STRUCT,
										NULL,
										&iter_struct);
			
			dbus_message_iter_append_basic(&iter_struct,
										DBUS_TYPE_STRING,
										&configCmd[i]);  

			dbus_message_iter_close_container(&iter_array, &iter_struct);
		}
		instance_device_free_running_config(configCmd, num);
	}
	dbus_message_iter_close_container(&iter, &iter_array);    

	return reply;        
}


static inline void
instance_pthread_attr_init(instance_struct_t *instance) {
	pthread_attr_init(&instance->attr);  
	pthread_attr_setdetachstate(&instance->attr, PTHREAD_CREATE_DETACHED);
}

static inline void
instance_pthread_attr_exit(instance_struct_t *instance) {
	pthread_attr_destroy(&instance->attr); 
}

static inline void
instance_get_coremask(instance_struct_t *instance) {
	FILE *fp;

    fp = fopen("/proc/coremask", "r");
	if (unlikely(!fp)) 
		goto error;

	if (1 != fscanf(fp, "0x%x", &instance->coremask))
		goto error1;

	fclose(fp);
	return;

error1:
	fclose(fp);
error:	
	pppoe_log(LOG_NOTICE, "get coremask failed, so set coremask 0x1\n");
	instance->coremask = 0x1;
}

static inline void
unbase_interface(struct device_struct *dev) {
	switch (dev->state) {
		case DEVICE_UP:
			memset(dev->config.base_ifname, 0, sizeof(dev->config.base_ifname));
			dev->state = DEVICE_DOWN;
			dev->configFlag &= ~CONFIG_DEV_BASE;
			break;
			
		case DEVICE_RUN:
			if (dev->thread.running) {		
				dev->thread.running = THREAD_STOP;

				/* wait for thread exit */
				while (DEV_THR_DEAD != dev->thread.state) {
					usleep(250000);		/*wait 0.25s*/
				}
			}
			device_thread_exit(dev);
			memset(dev->config.base_ifname, 0, sizeof(dev->config.base_ifname));
			dev->state = DEVICE_DOWN;
			dev->configFlag &= ~CONFIG_DEV_BASE;
			dev->config.status = 0;	/*service disable*/
			break;

		default:	
			break;
	}
}

static inline int
kernel_unbase_interface(instance_struct_t *instance, int ifindex) {
	struct device_struct *dev;
	uint32 i;

	if (!instance->count) {
		pppoe_log(LOG_WARNING, "ifindex %d not in pppoe device array", ifindex);
		return PPPOEERR_ENOEXIST;
	}
	
	for (i = 1; i <= DEV_MAX_NUM; i++) {
		dev = device_find(instance, i);		
		if (dev && dev->ifindex == ifindex) {
			unbase_interface(dev);		
			break;
		}
	}

	return PPPOEERR_SUCCESS;
}

static int
instance_process(thread_struct_t *thread) {
	instance_struct_t *instance = thread_get_arg(thread);
	struct pppoe_message *mess;
	int ret;

	ret = netlink_recv_message(instance->sk, pbuf_init(instance->pbuf));
	if (unlikely(ret)) {
		pppoe_log(LOG_WARNING, "recv message fail, ret = %d\n", ret);
		goto out;
	}

	mess = (struct pppoe_message *)instance->pbuf->data;
	if (mess->type != PPPOE_MESSAGE_SIGNAL) {
		pppoe_log(LOG_WARNING, "recv message is not signal\n");
		ret =  PPPOEERR_EINVAL;
		goto out;
	}

	pppoe_log(LOG_INFO, "instance recv %d signal from kernel\n", mess->code);

	switch (mess->code) {
	case PPPOE_INTERFACE_UNBASE:
		if (mess->datalen < sizeof(struct pppoe_interface_msg)) {
			pppoe_log(LOG_WARNING, "signal length is error\n");
			ret = PPPOEERR_ELENGTH;
			goto out;
		}
		pppoe_log(LOG_INFO, "recv %s unbase interface from kernel\n", 
							mess->data.m_interface.name);
		ret = kernel_unbase_interface(instance, mess->data.m_interface.ifindex);
		break;

	default:
		ret = PPPOEERR_EINVAL;
		break;
	}

out:
	return ret;
}


static inline int
instance_notify_vrrp_backup_finished(instance_struct_t *instance){
	struct vrrp_instance_info info;
	int ret;

	memset(&info, 0, sizeof(info));
	info.state = 0;	/* backup finished */
	info.local_id = instance->local_id;
	info.instance_id = instance->instance_id;

	ret = pppoe_method_perform(PPPOE_METHOD_INSTANCE_NOTITY_VRRP_BACKUP, &info, NULL);
	if (ret) {
		pppoe_log(LOG_ERR, "instance notify vrrp failed, ret %d\n", ret);			
	}

	return ret;
}

static inline int
instance_notify_active_backup_finished(instance_struct_t *instance) {
	struct instance_sync *sync;
	struct pppoe_buf *pbuf;	
	backup_task_t *task;
	int ret;
	
	pbuf = pbuf_alloc(sizeof(struct instance_sync) + BACKUP_HEADER_LEN);
	if (unlikely(!pbuf)) {
		pppoe_log(LOG_WARNING, "notify alloc pbuf failed\n");
		ret = PPPOEERR_ENOMEM;
		goto error;
	}
	
	pbuf_reserve(pbuf, BACKUP_HEADER_LEN);
	sync = (struct instance_sync *)pbuf_put(pbuf, sizeof(struct instance_sync));
	sync->code = htons(INSTANCE_BACKUP_FINISHED);
	sync->dev_id = 0;	

	task = backup_task_create(instance->backup, pbuf, BACKUP_INSTANCE_SYNC);
	if (unlikely(!task)) {
		pppoe_log(LOG_WARNING, "backup task create failed\n");
		ret = PPPOEERR_ENOMEM;
		goto error1;
	}

	ret = backup_task_add(instance->backup, task);
	if (ret) {
		pppoe_log(LOG_WARNING, "backup task add failed, ret %d\n", ret);
		goto error2;
	}

	pppoe_log(LOG_INFO, "add task: notify active backup finished\n");
	return PPPOEERR_SUCCESS;

error2:
	backup_task_destroy(instance->backup, &task);
error1:
	PBUF_FREE(pbuf);
error:
	return ret;
}
static int
instance_active_sync_process(struct pppoe_buf *pbuf, void *sync_para) {
	instance_struct_t *instance = (instance_struct_t *)sync_para;
	struct instance_sync *sync;
	struct device_struct *dev;
	backup_task_t *task;
	uint32 sync_method;
	int ret;

	if (unlikely(!pbuf)) {
		pppoe_log(LOG_WARNING, "input pppoe buf is NULL\n");
		return PPPOEERR_EINVAL;
	}

	if (unlikely(!instance)) {
		pppoe_log(LOG_WARNING, "input instance point is NULL\n");
		ret = PPPOEERR_EINVAL;
		goto error;
	}

	if (unlikely(pbuf_may_pull(pbuf, sizeof(struct instance_sync)))) {
		pppoe_log(LOG_WARNING, "pbuf length %u is error\n", pbuf->len);
		ret = PPPOEERR_ELENGTH;
		goto error;
	}

	sync = (struct instance_sync *)pbuf->data;
	switch (ntohs(sync->code)) {
	case INSTANCE_SESSSYNC_REQUEST:
		pppoe_log(LOG_INFO, "recv device %u session sync request\n", ntohs(sync->dev_id));		
		dev = device_find(instance, ntohs(sync->dev_id));
		if (!dev) {
			/*
			 * device is not exist when recv session sync request. 
			 * may standby configuration is not synchronized,
			 * so need send sync finished.
			 */
			pppoe_log(LOG_NOTICE, "device %u is not exist", ntohs(sync->dev_id));
			goto finished;
		}

		if (dev->thread.running != THREAD_RUNNING) {
			/*
			 * device is not running when recv session sync request. 
			 * may standby configuration is not synchronized,
			 * so need send sync finished.
			 */
			pppoe_log(LOG_NOTICE, "device %u is not running", dev->id);
			goto finished;
		}

		sync_method = PPPOE_METHOD_SESSION_SYNC;
		break;

	case INSTANCE_BACKUP_FINISHED:
		pppoe_log(LOG_NOTICE, "recv standby backup finished message\n");
		instance_notify_vrrp_backup_finished(instance);
		return PPPOEERR_SUCCESS;

	default:
		pppoe_log(LOG_WARNING, "instance sync code %u unknown\n", ntohs(sync->code));
		ret = PPPOEERR_ENOEXIST;
		goto error;
	}

	if (dev->thread.running != THREAD_RUNNING) {
		pppoe_log(LOG_NOTICE, "device %u is not running\n", dev->id);
		goto error;
	}

	pbuf_pull(pbuf, sizeof(struct instance_sync));
	return tbus_send_signal(instance->connection, dev->id, 
							sync_method, pbuf, dev->thread.manage, 
							(tbusDataFree)pbuf_free, 0);

finished:
	sync->code = htons(INSTANCE_SESSSYNC_FINISHED);	
	task = backup_task_create(instance->backup, pbuf, BACKUP_INSTANCE_SYNC);
	if (unlikely(!task)) {
		pppoe_log(LOG_ERR, "device %u create session sync finished task failed\n",
							ntohs(sync->dev_id));
		ret = PPPOEERR_ENOMEM;
		goto error;
	}

	if (backup_task_add(instance->backup, task)) {
		pppoe_log(LOG_ERR, "device %u backup task add failed\n",
							ntohs(sync->dev_id));
		ret = PPPOEERR_ESTATE;
		goto error1;
	}
	
	return PPPOEERR_SUCCESS;

error1:
	backup_task_destroy(instance->backup, &task);
error:
	PBUF_FREE(pbuf);
	return ret;
}

static int
instance_standby_sync_process(struct pppoe_buf *pbuf, void *sync_para) {
	instance_struct_t *instance = (instance_struct_t *)sync_para;
	struct instance_sync *sync;
	struct device_struct *dev;
	uint32 sync_method;
	int ret;

	if (unlikely(!pbuf)) {
		pppoe_log(LOG_WARNING, "input pppoe buf is NULL\n");
		return PPPOEERR_EINVAL;
	}

	if (unlikely(!instance)) {
		pppoe_log(LOG_WARNING, "input instance point is NULL\n");
		ret = PPPOEERR_EINVAL;
		goto error;
	}

	if (unlikely(pbuf_may_pull(pbuf, sizeof(struct instance_sync)))) {
		pppoe_log(LOG_WARNING, "pbuf length %u is error\n", pbuf->len);
		ret = PPPOEERR_ELENGTH;
		goto error;
	}

	sync = (struct instance_sync *)pbuf->data;
	dev = device_find(instance, ntohs(sync->dev_id));
	if (unlikely(!dev)) {
		pppoe_log(LOG_NOTICE, "device %u is not exist\n", ntohs(sync->dev_id));
		ret = PPPOEERR_ENOEXIST;
		goto error;
	}

	if (dev->thread.running != THREAD_RUNNING) {
		pppoe_log(LOG_NOTICE, "device %u is not running\n", dev->id);
		ret = PPPOEERR_ENOEXIST;
		goto error;
	}
	
	switch (sync->code) {
	case INSTANCE_SESSONLINE_SYNC:
		sync_method = PPPOE_METHOD_SESSION_ONLINE_SYNC;
		break;
		
	case INSTANCE_SESSOFFLINE_SYNC:
		sync_method = PPPOE_METHOD_SESSION_OFFLINE_SYNC;
		break;
	
	case INSTANCE_SESSUPDATE_SYNC:
		sync_method = PPPOE_METHOD_SESSION_UPDATE_SYNC;
		break;

	case INSTANCE_SESSCLEAR_SYNC:
		sync_method = PPPOE_METHOD_SESSION_CLEAR_V2;
		break;

	case INSTANCE_SESSSYNC_FINISHED:
		sync_method = PPPOE_METHOD_SESSION_SYNC_FINISHED;
		break;
		
	default:
		pppoe_log(LOG_WARNING, "instance sync code %u unknown\n", sync->code);
		ret = PPPOEERR_ENOEXIST;
		goto error;
	}

	pbuf_pull(pbuf, sizeof(struct instance_sync));
	return tbus_send_signal(instance->connection, dev->id, 
							sync_method, pbuf, dev->thread.manage, 
							(tbusDataFree)pbuf_free, 0);
	
error:
	PBUF_FREE(pbuf);
	return ret;
}

static inline void
instance_device_exit(instance_struct_t *instance) {
	struct device_struct *dev;
	int i;

	/* free pppoe device */
	for (i = 1; i <= DEV_MAX_NUM; i++) {
		dev = device_find(instance, i);		
		if (dev) {
			if (DEVICE_RUN == dev->state) {
				/* need edit tell thread exit....*/
				if (dev->thread.running) {		
					dev->thread.running = THREAD_STOP;

					/* wait for thread exit */
					while (DEV_THR_DEAD != dev->thread.state) {
						usleep(10000);		/*wait 0.01s*/
					}
				}
				device_thread_exit(dev);
				dev->state = DEVICE_UP;
				dev->config.status = 0;	/*service disable*/
			}
			
			/* we need tell kernel destroy pppoe device......*/
			device_destroy(instance, dev);		
		}
	}	
}

static int
instance_backup_finished_func(thread_struct_t *thread) {
	instance_struct_t *instance = thread_get_arg(thread);
	struct device_struct *dev;
	int i;
	
	for (i = 1; i <= DEV_MAX_NUM; i++) {
		dev = device_find(instance, i);		
		if (dev) {
			if (DEVICE_RUN == dev->state) {
				/* check manage sync finished flag */
				if (!manage_sync_flag(dev->thread.manage)) {
					pppoe_log(LOG_DEBUG, "device %u sync not finished, "
										"need try again\n", dev->id);
					goto again;
				}	
			}
		}
	}

	pppoe_log(LOG_INFO, "standby backup finished, notify active and vrrp\n");
	instance_notify_vrrp_backup_finished(instance);	
	instance_notify_active_backup_finished(instance);

	THREAD_CANCEL(instance->timer);
	return PPPOEERR_SUCCESS;

again:		
	thread_update_timer(thread, 1, THREAD_EXTIME_NONE);
	return PPPOEERR_EAGAIN;
}

static int
instance_backup_clear_func(thread_struct_t *thread) {
	instance_struct_t *instance = thread_get_arg(thread);
	struct device_struct *dev;
	int i;
	
	for (i = 1; i <= DEV_MAX_NUM; i++) {
		dev = device_find(instance, i);
		if (!dev)
			continue;

		if (dev->thread.running != THREAD_RUNNING)
			continue;

		/* tell device thread clear session */
		tbus_send_signal(instance->connection, dev->id,
						PPPOE_METHOD_SESSION_CLEAR_V2, 
						NULL, dev->thread.manage, NULL, 0);	
	}

	THREAD_CANCEL(instance->timer);
	return PPPOEERR_SUCCESS;
}

static inline int
instance_backup_setup(instance_struct_t *instance) {
	uint16 s_port, d_port;
	uint32 status;
	int ret;

	THREAD_CANCEL(instance->timer);	/* need clear instance timer */
	switch (backup_status(instance->backup)) {
	case BACKUP_NONE:
		break;
		
	case BACKUP_ACTIVE:
		backup_proto_unregister(instance->backup, BACKUP_INSTANCE_SYNC);
		break;
		
	case BACKUP_STANDBY:
		backup_proto_unregister(instance->backup, BACKUP_INSTANCE_SYNC);
		break;
		
	case BACKUP_DISABLE:
		break;
	}

	switch (instance->state) {
	case VRRP_STATE_INIT:
		pppoe_log(LOG_NOTICE, "instance init, no need handle\n");
		ret = PPPOEERR_ESTATE;
		goto out;
		
	case VRRP_STATE_BACK:
		s_port = 0;
		d_port = INSTANCE_BACKUP_PORT;
		status = BACKUP_STANDBY;
		ret = backup_proto_register(instance->backup, 
									BACKUP_INSTANCE_SYNC,
									instance_standby_sync_process,
									instance);
		if (unlikely(ret)) {
			pppoe_log(LOG_ERR, "bakup proto %u register failed\n",
								BACKUP_INSTANCE_SYNC);
			goto out;
		}

		instance->timer = thread_add_timer(instance->master, 
										instance_backup_finished_func, 
										instance, 1, THREAD_EXTIME_NONE);
		if (unlikely(!instance->timer)) {
			pppoe_log(LOG_ERR, "instance standby add backup finished timer failed\n");
			backup_proto_unregister(instance->backup, BACKUP_INSTANCE_SYNC);
			goto out;
		}
		break;
		
	case VRRP_STATE_MAST:
		s_port = INSTANCE_BACKUP_PORT;
		d_port = 0;
		status = BACKUP_ACTIVE;
		ret = backup_proto_register(instance->backup, 
									BACKUP_INSTANCE_SYNC,
									instance_active_sync_process,
									instance);
		if (unlikely(ret)) {
			pppoe_log(LOG_ERR, "bakup proto %u register failed\n",
								BACKUP_INSTANCE_SYNC);
			goto out;
		}
		break;

	case VRRP_STATE_LEARN:
		pppoe_log(LOG_NOTICE, "instance learning, no need handle\n");
		ret = PPPOEERR_ESTATE;
		goto out;

	case VRRP_STATE_NONE:
		s_port = 0;
		d_port = 0;		
		status = BACKUP_NONE;
		break;

	case VRRP_STATE_TRANSFER:
		pppoe_log(LOG_NOTICE, "instance transfer, no need handle\n");
		ret = PPPOEERR_ESTATE;
		goto out;

	case VRRP_STATE_DISABLE:
		s_port = 0;
		d_port = 0;
		status = BACKUP_DISABLE;
		break;

	default:
		pppoe_log(LOG_WARNING, "unknow instance state %u\n", instance->state);
		ret = PPPOEERR_EINVAL;
		goto out;
	}

	ret = backup_status_setup(instance->backup, status,
							instance->heartlink_local_ip, s_port, 
							instance->heartlink_opposite_ip, d_port);
	pppoe_log(LOG_INFO, "backup status %u setup, ret %d\n", status, ret);

out:	
	return ret;	
}


static inline void
instance_backup_switch(instance_struct_t *instance) {
	struct device_struct *dev;
	uint32 i;

	for (i = 1; i <= DEV_MAX_NUM; i++) {
		dev = device_find(instance, i);
		if (!dev)
			continue;

		if (dev->thread.running != THREAD_RUNNING)
			continue;

		dev->thread.running = THREAD_RESTART;
		pppoe_log(LOG_INFO, "instance tell thread %u restart\n", i);
	}

	for (i = 1; i <= DEV_MAX_NUM; i++) {
		dev = device_find(instance, i);
		if (!dev)
			continue;

		while (THREAD_RESTART == dev->thread.running) {
			usleep(10000);		/*wait 0.01s*/
		}

		if (THREAD_RUNNING == dev->thread.running) {
			pppoe_log(LOG_INFO, "instance thread %u restart success\n", i);
		}
	}	
}

static int
instance_setup_func(thread_struct_t *thread) {
	instance_struct_t *instance = thread_get_arg(thread);
	struct vrrp_instance_info info;
	int ret;
	
	memset(&info, 0, sizeof(info));
	info.local_id = instance->local_id;
	info.instance_id = instance->instance_id;

	/* dbus get instance info from vrrp */
	ret = pppoe_method_perform(PPPOE_METHOD_INSTANCE_SHOW_VRRP_INFO, &info, NULL);
	switch (ret) {
	case PPPOEERR_SUCCESS:
		instance->state = info.state;
		instance->heartlink_local_ip = info.heartlink_local_ip;
		instance->heartlink_opposite_ip = info.heartlink_opposite_ip;
		pppoe_log(LOG_INFO, "instance_id(%u) state %u, "
							"heartlink local_ip %u.%u.%u.%u, "
							"heartlink opposite_ip %u.%u.%u.%u\n",
							instance->instance_id, instance->state,
							HIPQUAD(instance->heartlink_local_ip), 
							HIPQUAD(instance->heartlink_opposite_ip));
		break;

	case PPPOEERR_ENOEXIST:
		pppoe_log(LOG_INFO, "%s hansi instance %u config is not exist\n",
							info.local_id ? "local" : "remote", info.instance_id);
		instance->state = VRRP_STATE_NONE;
		break;
		
	case PPPOEERR_EDBUS:
		pppoe_log(LOG_NOTICE, "show %s hansi instance %u dbus error\n",
							info.local_id ? "local" : "remote", info.instance_id);
		goto update;
		
	default:
		pppoe_log(LOG_WARNING, "%s hansi instance %u get info failed!\n",
							info.local_id ? "local" : "remote", info.instance_id);
		instance->state = VRRP_STATE_DISABLE;
		break;
	}

	instance_backup_setup(instance);
	instance_backup_switch(instance);
	return ret;

update:
	return thread_update_timer(thread, 1, THREAD_EXTIME_NONE);
}


static inline void
instance_setup(instance_struct_t *instance) {
	instance->timer = thread_add_timer(instance->master, 
									instance_setup_func, 
									instance, 1, THREAD_EXTIME_NONE);
	if (unlikely(!instance->timer)) {
		pppoe_log(LOG_ERR, "instance backup setup timer add failed\n");
	}

	/* when instance start, frist setup instance vrrp disable */
	instance->state = VRRP_STATE_DISABLE;
	backup_status_setup(instance->backup, BACKUP_DISABLE, 0, 0, 0, 0);
}

static DBusMessage *
dbus_instance_vrrp_switch(DBusMessage *message, void *user_data) {
	DBusMessage	*reply;
	DBusMessageIter iter;
	DBusError err;
	
	instance_struct_t *instance = (instance_struct_t *)user_data;
	uint32 instance_id, state;
	uint32 heartlink_local_ip, heartlink_opposite_ip;
	char *interface_name;
	int ret = 0;

	dbus_error_init( &err );
	dbus_message_iter_init(message, &iter);
	dbus_message_iter_get_basic(&iter, &instance_id);

	if (instance_id != instance->instance_id) {
		pppoe_log(LOG_ERR, "dbus get instance_id(%u) is not match instance %u\n",
							instance_id, instance->instance_id);
		ret = -1;
		goto out;
	}
	
	dbus_message_iter_next(&iter);	
	dbus_message_iter_get_basic(&iter,&state);
	
	dbus_message_iter_next(&iter);					/* uplink_cnt */
	dbus_message_iter_next(&iter);					/* uplink_array */
	dbus_message_iter_next(&iter);					/* downlink_cnt */							
	dbus_message_iter_next(&iter);					/* downlink_array */
	dbus_message_iter_next(&iter);					/* interface_name */					
	dbus_message_iter_get_basic(&iter, &interface_name);
	
	dbus_message_iter_next(&iter);					/* heartlink_local_ip */	
	dbus_message_iter_get_basic(&iter, &heartlink_local_ip);
	
	dbus_message_iter_next(&iter);					/* heartlink_opposite_ip */
	dbus_message_iter_get_basic(&iter, &heartlink_opposite_ip); 
	
	dbus_message_iter_next(&iter);					/* vgateway_cnt */	
	dbus_message_iter_next(&iter);					/* vgateway_array */

	if (state == instance->state) {
		pppoe_log(LOG_NOTICE, "instance state %u not switch\n", state);
		goto out;
	}

	switch (state) {
	case VRRP_STATE_INIT:
	case VRRP_STATE_LEARN:
	case VRRP_STATE_TRANSFER:
		pppoe_log(LOG_NOTICE, "instace state %u is transition, "
								"pppoe do not process it\n", state);			
		goto out;
	}

	pppoe_log(LOG_INFO, "instance_id(%u) state switch from %u to %u, "
						"interface %s, heartlink local_ip %u.%u.%u.%u, "
						"heartlink opposite_ip %u.%u.%u.%u\n",
						instance_id, instance->state, state, interface_name,
						HIPQUAD(heartlink_local_ip), HIPQUAD(heartlink_opposite_ip));
	
	instance->state = state;
	instance->heartlink_local_ip = heartlink_local_ip;
	instance->heartlink_opposite_ip = heartlink_opposite_ip;
	
	instance_backup_setup(instance);
	instance_backup_switch(instance);

out:	
	reply = dbus_message_new_method_return(message);
	if (unlikely(!reply)) {
		pppoe_log(LOG_ERR, "dbus new method reply failed\n");
		return NULL;
	}

	dbus_message_iter_init_append(reply, &iter);
	dbus_message_iter_append_basic(&iter, DBUS_TYPE_UINT32, &ret);
	return reply;
}



static inline void
backup_connect_event(instance_struct_t *instance) {/* this need through thread bus */
	struct device_struct *dev;
	int i;

	switch (backup_status(instance->backup)) {
	case BACKUP_ACTIVE:
		pppoe_log(LOG_NOTICE, "active process connect event\n");		
		break;

	case BACKUP_STANDBY:
		THREAD_CANCEL(instance->timer);	/* this may exist backup clear timer */
		for (i = 1; i <= DEV_MAX_NUM; i++) {
			dev = device_find(instance, i);
			if (!dev)
				continue;

			if (!dev->thread.sync_switch)	/* wait for manage start success */
				continue;

			/* tell device thread clear session */
			tbus_send_signal(instance->connection, dev->id,
							PPPOE_METHOD_SESSION_CLEAR_V2, 
							NULL, dev->thread.manage, NULL, 0);	

			thread_sync_request(&dev->thread);
		}
		pppoe_log(LOG_NOTICE, "standby process connect event\n");
		break;
	}
}

static inline void
backup_disconnect_event(instance_struct_t *instance) {/* this need through thread bus */
	switch (backup_status(instance->backup)) {
	case BACKUP_ACTIVE:
		pppoe_log(LOG_NOTICE, "active process disconnect event\n");
		break;

	case BACKUP_STANDBY:
		THREAD_CANCEL(instance->timer);	/* this may exist backup finished timer */
		instance->timer = thread_add_timer(instance->master, 
										instance_backup_clear_func, 
										instance, 60, THREAD_EXTIMER1);
		if (unlikely(!instance->timer)) {
			pppoe_log(LOG_ERR, "standby add backup clear timer failed\n");
		}

		pppoe_log(LOG_NOTICE, "standby process disconnect event\n");		
		break;
	}
}


static int
instance_backup_event(struct notifier_struct *this, uint32 event, void *ptr) {
	instance_struct_t *instance = (instance_struct_t *)this->arg;

	if (unlikely(!instance)) {
		pppoe_log(LOG_ERR, "input ptr(instance) is NULL\n");
		return PPPOEERR_EINVAL;
	}
	
	switch (event) {
	case CHANNEL_CONNECT:
		backup_connect_event(instance);
		break;
		
	case CHANNEL_DISCONNECT:
		backup_disconnect_event(instance);	
		break;

	default:
		return PPPOEERR_ENOEXIST;
	}

	return PPPOEERR_SUCCESS;
}

static inline int
instance_backup_chain_register(instance_struct_t *instance) {
	instance->notifier.priority = 0;
	instance->notifier.call = instance_backup_event;
	instance->notifier.arg = instance;

	return backup_notifier_register(instance->backup, &instance->notifier);
}

static inline int
instance_backup_chain_unregister(instance_struct_t *instance) {
	return backup_notifier_unregister(instance->backup, &instance->notifier);
}

static inline void
instance_dbus_method_register(instance_struct_t *instance) {
	pppoe_dbus_method_register(PPPOE_DBUS_DEVICE_CREATE, 
							dbus_device_create, instance);
	pppoe_dbus_method_register(PPPOE_DBUS_DEVICE_DESTROY, 
							dbus_device_destroy, instance);
	pppoe_dbus_method_register(PPPOE_DBUS_DEVICE_CONFIG_BASE, 
							dbus_device_config_base, instance);
	pppoe_dbus_method_register(PPPOE_DBUS_DEVICE_CONFIG_APPLY, 
							dbus_device_config_apply, instance);
	pppoe_dbus_method_register(PPPOE_DBUS_DEVICE_CONFIG_SERVICE, 
							dbus_device_config_service, instance);
	pppoe_dbus_method_register(PPPOE_DBUS_DEVICE_CONFIG_IPADDR, 
							dbus_device_config_ipaddr, instance);
	pppoe_dbus_method_register(PPPOE_DBUS_DEVICE_CONFIG_VIRTUAL_MAC, 
							dbus_device_config_virtual_mac, instance);
	pppoe_dbus_method_register(PPPOE_DBUS_DEVICE_CONFIG_SESSION_IPADDR, 
							dbus_device_config_session_ipaddr, instance);
	pppoe_dbus_method_register(PPPOE_DBUS_DEVICE_CONFIG_SESSION_DNS, 
							dbus_device_config_session_dns, instance);
	pppoe_dbus_method_register(PPPOE_DBUS_DEVICE_CONFIG_NAS_IPADDR, 
							dbus_device_config_nas_ipaddr, instance);
	pppoe_dbus_method_register(PPPOE_DBUS_DEVICE_CONFIG_RADIUS_RDC, 
							dbus_device_config_radius_rdc, instance);
	pppoe_dbus_method_register(PPPOE_DBUS_DEVICE_CONFIG_RADIUS_SERVER, 
							dbus_device_config_radius_server, instance);
	pppoe_dbus_method_register(PPPOE_DBUS_DEVICE_CONFIG_SNAME, 
							dbus_device_config_sname, instance);
	pppoe_dbus_method_register(PPPOE_DBUS_DEVICE_KICK_USER, 
							dbus_device_kick_user, instance);
	pppoe_dbus_method_register(PPPOE_DBUS_INSTANCE_VRRP_SWITCH,
							dbus_instance_vrrp_switch, instance);
	pppoe_dbus_method_register(PPPOE_DBUS_SHOW_PFM_ENTRY,
							dbus_device_show_pfm_entry, instance);	
	pppoe_dbus_method_register(PPPOE_DBUS_SHOW_ONLINE_USER, 
							dbus_device_show_online_user, instance);
	pppoe_dbus_method_register(PPPOE_DBUS_SHOW_DEVICE_LIST, 
							dbus_device_show_list, instance);
	pppoe_dbus_method_register(PPPOE_DBUS_DETECT_DEVICE_EXIST, 
							dbus_detect_device_exist, instance);
	pppoe_dbus_method_register(PPPOE_DBUS_SHOW_RUNNING_CONFIG, 
							dbus_show_running_config, instance);
}

static inline void
instance_dbus_method_unregister(instance_struct_t *instance) {
	pppoe_dbus_method_unregister(PPPOE_DBUS_DEVICE_CREATE);
	pppoe_dbus_method_unregister(PPPOE_DBUS_DEVICE_DESTROY);
	pppoe_dbus_method_unregister(PPPOE_DBUS_DEVICE_CONFIG_BASE);
	pppoe_dbus_method_unregister(PPPOE_DBUS_DEVICE_CONFIG_APPLY);
	pppoe_dbus_method_unregister(PPPOE_DBUS_DEVICE_CONFIG_SERVICE);
	pppoe_dbus_method_unregister(PPPOE_DBUS_DEVICE_CONFIG_IPADDR);
	pppoe_dbus_method_unregister(PPPOE_DBUS_DEVICE_CONFIG_VIRTUAL_MAC);	
	pppoe_dbus_method_unregister(PPPOE_DBUS_DEVICE_CONFIG_SESSION_IPADDR);
	pppoe_dbus_method_unregister(PPPOE_DBUS_DEVICE_CONFIG_SESSION_DNS);
	pppoe_dbus_method_unregister(PPPOE_DBUS_DEVICE_CONFIG_NAS_IPADDR);
	pppoe_dbus_method_unregister(PPPOE_DBUS_DEVICE_CONFIG_RADIUS_RDC);	
	pppoe_dbus_method_unregister(PPPOE_DBUS_DEVICE_CONFIG_RADIUS_SERVER);
	pppoe_dbus_method_unregister(PPPOE_DBUS_DEVICE_CONFIG_SNAME);
	pppoe_dbus_method_unregister(PPPOE_DBUS_DEVICE_KICK_USER);
	pppoe_dbus_method_unregister(PPPOE_DBUS_INSTANCE_VRRP_SWITCH);	
	pppoe_dbus_method_unregister(PPPOE_DBUS_SHOW_PFM_ENTRY);
	pppoe_dbus_method_unregister(PPPOE_DBUS_SHOW_ONLINE_USER);
	pppoe_dbus_method_unregister(PPPOE_DBUS_SHOW_DEVICE_LIST);
	pppoe_dbus_method_unregister(PPPOE_DBUS_DETECT_DEVICE_EXIST);
	pppoe_dbus_method_unregister(PPPOE_DBUS_SHOW_RUNNING_CONFIG);
}

void
pppoe_instance_dispatch(instance_struct_t *instance, uint32 msec) {
	struct timeval tvp;
	tvp.tv_sec = msec / 1000;;
	tvp.tv_usec = (msec - (1000 * tvp.tv_sec)) * 1000;

	thread_dispatch(instance->master, tvp);
	tbus_connection_dispatch(instance->connection, 0);
}

instance_struct_t *
pppoe_instance_init(uint32 slot_id, uint32 local_id, uint32 instance_id) {
	instance_struct_t *instance 
		= (instance_struct_t *)malloc(sizeof(instance_struct_t));
	if (unlikely(!instance)) {
		pppoe_log(LOG_ERR, "instance alloc failed!\n");
		goto error;
	}

	memset(instance, 0, sizeof(*instance));
	instance->slot_id = slot_id;
	instance->local_id = local_id;
	instance->instance_id = instance_id;
	instance_get_coremask(instance);
	pppoe_log(LOG_INFO, "instance get coremask is 0x%x\n", instance->coremask);

	/* init dev list netlink socket*/
	instance->sk = netlink_init();
	if (unlikely(instance->sk <= 0)) {
		pppoe_log(LOG_ERR, "netlink init failed\n");
		goto error1;
	}

	/* alloc instance buf */
	instance->pbuf = pbuf_alloc(DEFAULT_BUF_SIZE);
	if (unlikely(!instance->pbuf)) {
		pppoe_log(LOG_ERR, "pbuf alloc failed\n");
		goto error2;
	}

	/* register netlink socket for recv kernel signal */
	if (unlikely(netlink_register(instance->sk, pbuf_init(instance->pbuf), 
							instance->local_id, instance->instance_id))) {
		pppoe_log(LOG_ERR, "netlink register failed\n");
		goto error3;
	}

	instance->cache = mem_cache_create(INSTANCE_CACHE_NAME, 
						INSTANCE_CACHE_MAX_BLKNUM, INSTANCE_CACHE_EMPTY_BLKNUM,
						sizeof(struct device_struct), INSTANCE_CACHE_BLK_ITEMNUM);
	if (unlikely(!instance->cache)) {
		pppoe_log(LOG_ERR, "mem cache create failed\n");
		goto error4;
	}	
	
	instance->tbus = thread_bus_init(DEV_MAX_NUM + 1);
	if (unlikely(!instance->tbus)) {
		pppoe_log(LOG_ERR, "thread bus init failed\n");
		goto error5;
	}

	instance->connection = tbus_connection_create(instance->tbus, 0);
	if (unlikely(!instance->connection))	{
		pppoe_log(LOG_ERR, "tbus connection create failed\n");
		goto error6;
	}

	instance->master = thread_master_create();
	if (unlikely(!instance->master)) {
		pppoe_log(LOG_ERR, "thread master create failed!\n");
		goto error7;
	}

	instance->thread = thread_add_read(instance->master, 
										instance_process, 
										instance, instance->sk);
	if (unlikely(!instance->thread)) {
		pppoe_log(LOG_ERR, "add thread read failed\n");
		goto error8;
	}
	
	instance->backup = backup_init(instance->master);
	if (unlikely(!instance->backup)) {
		pppoe_log(LOG_ERR, "backup init failed\n");
		goto error9;
	}

	if (unlikely(instance_backup_chain_register(instance))) {
		pppoe_log(LOG_ERR, "backup chain register failed\n");
		goto error10;
	}
	
	instance_dbus_method_register(instance);
	instance_pthread_attr_init(instance);
	instance_setup(instance);

	pppoe_log(LOG_INFO, "pppoe instance %u-%u-%u init success\n",
				instance->slot_id, instance->local_id, instance->instance_id);
	return instance;

error10:
	backup_exit(&instance->backup);
error9:
	THREAD_CANCEL(instance->thread);
error8:
	thread_master_destroy(instance->master);
	instance->master = NULL;	
error7:
	tbus_connection_destroy(&instance->connection);
error6:
	thread_bus_exit(&instance->tbus);
error5:
	mem_cache_destroy(&instance->cache);
error4:
	netlink_unregister(instance->sk, pbuf_init(instance->pbuf), 
						local_id, instance_id);
error3:
	PBUF_FREE(instance->pbuf);
error2:
	PPPOE_CLOSE(instance->sk);
error1:
	PPPOE_FREE(instance);
error:
	return NULL;
}

void
pppoe_instance_exit(instance_struct_t **instance) {
	if (!instance || !(*instance))
		return;

	instance_backup_chain_unregister(*instance);
	instance_dbus_method_unregister(*instance);
	instance_pthread_attr_exit(*instance);	
	instance_device_exit(*instance);
	backup_exit(&(*instance)->backup);
	THREAD_CANCEL((*instance)->timer);
	THREAD_CANCEL((*instance)->thread);
	thread_master_destroy((*instance)->master);
	tbus_connection_destroy(&(*instance)->connection);
	thread_bus_exit(&(*instance)->tbus);
	mem_cache_destroy(&(*instance)->cache);
	netlink_unregister((*instance)->sk, pbuf_init((*instance)->pbuf), 
						(*instance)->local_id, (*instance)->instance_id);
	PBUF_FREE((*instance)->pbuf);
	PPPOE_CLOSE((*instance)->sk);

	pppoe_log(LOG_INFO, "pppoe instance %u-%u-%u exit success\n",
				(*instance)->slot_id, (*instance)->local_id, (*instance)->instance_id);
	PPPOE_FREE(*instance);
}


