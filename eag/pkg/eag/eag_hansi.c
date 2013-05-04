/*******************************************************************************
Copyright (C) Autelan Technology

This software file is owned and distributed by Autelan Technology 
********************************************************************************

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND 
ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED 
WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE 
DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE LIABLE FOR 
ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES 
(INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; 
LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON 
ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT 
(INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS 
SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
********************************************************************************
* eag_hansi.c
*
* CREATOR:
* autelan.software.xxx. team
*
* DESCRIPTION:
* xxx module main routine
*
*******************************************************************************/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <unistd.h>
#include <dbus/dbus.h>

#include "nm_list.h"
#include "eag_errcode.h"
#include "eag_mem.h"
#include "eag_util.h"
#include "eag_blkmem.h"
#include "eag_log.h"
#include "eag_dbus.h"
#include "eag_thread.h"
#include "session.h"
#include "eag_time.h"
#include "hashtable.h"
#include "eag_interface.h"
#include "eag_hansi.h"
#include "had_vrrpd.h"
#include "dbus/npd/npd_dbus_def.h"


#define BACKUP_PACKET_SIZE		4096

#define MAX_BACKUP_TYPE_NAME_LEN		32
#define CHECK_HANSI_CONNECT_INTERVAL	10

#define EAG_HANSI_TASK_BLKMEM_NAME 			"hansi_task_blkmem"
#define EAG_HANSI_TASK_BLKMEM_ITEMNUM		128
#define EAG_HANSI_TASK_BLKMEM_MAXNUM		128

#define BACKUP_TYPE_SYN_TIME	"bktype_syn_time"
#define BACKUP_TYPE_ACK_TIME	"bktype_ack_time"

#define VRRP_DBUS_BUSNAME			"aw.vrrpcli"
#define VRRP_DBUS_OBJPATH 			"/aw/vrrp"
#define VRRP_DBUS_INTERFACE 		"aw.vrrp"
#define VRRP_DBUS_METHOD_SET_PORTAL_TRANSFER_STATE          "vrrp_set_portal_transfer_state"

typedef struct eag_hansi_proc eag_hansi_proc_t;
typedef struct eag_hansi_task eag_hansi_task_t;

struct backup_packet {
	struct timeval tv;
    uint32_t type;
	uint32_t pktlen;
	uint8_t data[BACKUP_PACKET_SIZE];
};

#define BACKUP_PACKET_HEADSIZE	offsetof(struct backup_packet ,data)

struct eag_hansi_task {
	struct list_head node;
	char type_name[MAX_BACKUP_TYPE_NAME_LEN];
	struct backup_packet packet;
	eag_hansi_proc_t *proc;
	eag_thread_t *t_write;
	uint32_t writen;
};

struct eag_hansi_backup_type {
	struct list_head node;
	uint32_t type;/*auto increase on register*/
	char type_name[MAX_BACKUP_TYPE_NAME_LEN];
	void *cbp;
	ON_GET_BACKUP_DATA get_backup_data;
};

struct eag_hansi_intf {
	char if_name[MAX_IF_NAME_LEN];
	uint32_t virtual_ip;
	uint32_t backp_ip;
	uint32_t master_ip;
};

#define MAX_PHA_IF_NUM		8

struct eag_hansi_param
{
	uint32_t vrid;
	uint32_t prev_state;
	uint32_t state;
	uint32_t uplink_cnt;
	struct eag_hansi_intf uplink_if[MAX_PHA_IF_NUM];
	uint32_t downlink_cnt;
	struct eag_hansi_intf downlink_if[MAX_PHA_IF_NUM];
	uint32_t vgateway_cnt;
	struct eag_hansi_intf vgateway_if[MAX_PHA_IF_NUM];
	char heartlink_if_name[MAX_IF_NAME_LEN];
	uint32_t heartlink_local_ip;
	uint32_t heartlink_opposite_ip;
};

struct eag_hansi_proc {
	int conn_fd;
	eag_hansi_t *eaghansi;
	eag_thread_master_t *master;
	eag_thread_t *t_read;
	eag_thread_t *t_heartbeat; /* for heart beat */
	struct backup_packet readbuf;
	uint32_t readlen;
	
	struct list_head task_fifo;
	eag_blk_mem_t *task_blkmem;
};

#define MAX_FORWARD_NUM		8
struct eag_hansi_param_forward {
	char objpath[MAX_DBUS_OBJPATH_LEN];
	char name[MAX_DBUS_BUSNAME_LEN];
	char interfacex[MAX_DBUS_INTERFACE_LEN];
	char method[MAX_DBUS_METHOD_LEN];
};

struct eag_hansi {
	int listen_fd;
	uint32_t ins_id;
	HANSI_TYPE hansi_type;
	int server_state;
	/* int state;  */
	time_t last_check_connect_time;
	uint16_t backup_port;
	struct eag_hansi_param params;
	
	struct list_head bktype_head;
	uint32_t bktype_num;

	eag_dbus_t *eagdbus;
	eag_thread_master_t *master;
	eag_thread_t *t_accept;
	eag_thread_t *t_timeout;

	EAG_HANSI_ON_MASTER_ACCEPT on_master_accept;
	void *on_master_accept_param;
	
	EAG_HANSI_ON_BACKUP_CONNECT on_backup_connect;
	void *on_backup_connect_param;
	
	EAG_HANSI_ON_BACKUP_HEARTBEAT_TIMEOUT on_backup_heartbeat_timeout;
	void *on_backup_heartbeat_timeout_param;
	
	EAG_HANSI_ON_STATUS_CHANGE on_status_change;
	void *on_status_change_param;
	
	eag_hansi_proc_t *proc;

	struct eag_hansi_param_forward forward[MAX_FORWARD_NUM];
	int forward_num;

	backup_type_t *syn_time;
	backup_type_t *ack_time;
};

static int 
eag_hansi_proc_stop(eag_hansi_proc_t *proc);
static int
eag_hansi_task_start_write(eag_hansi_task_t *task);
static int
eag_hansi_master_start(eag_hansi_t * eaghansi);
static int
eag_hansi_backup_start(eag_hansi_t * eaghansi);
static int
eag_hansi_master_stop(eag_hansi_t *eaghansi);
static int
eag_hansi_backup_stop(eag_hansi_t *eaghansi);

char *
hansi_state2str(int state)
{
	switch (state)
	{
		case VRRP_STATE_INIT:
			return "INIT";
		case VRRP_STATE_BACK:
			return "BACK";
		case VRRP_STATE_MAST:
			return "MASTER";
		case VRRP_STATE_LEARN:
			return "LEARNING";
		case VRRP_STATE_NONE:
			return "NONE";
		case VRRP_STATE_TRANSFER:
			return "TRANSFER";
		case VRRP_STATE_DISABLE:
			return "DISABLE";
		default:
			return "unknow";
	}
}

static eag_hansi_task_t *
eag_hansi_task_new(eag_hansi_proc_t *proc,
						backup_type_t *bktype,
						void *data,
						uint32_t datalen)
{
	eag_hansi_task_t *task = NULL;

	if (NULL == proc || NULL == bktype
		|| NULL == data || 0 == datalen) {
		eag_log_err("eag_hansi_task_new input error");
		return NULL;
	}

	task = eag_blkmem_malloc_item(proc->task_blkmem);
	if (NULL == task) {
		eag_log_err("eag_hansi_task_new blkmem_malloc_item failed");
		return NULL;
	}

	memset(task, 0, sizeof(eag_hansi_task_t));
	strncpy(task->type_name, bktype->type_name, sizeof(task->type_name)-1);
	task->packet.type = htonl(bktype->type);
	task->packet.pktlen = htonl(datalen+BACKUP_PACKET_HEADSIZE);
	memcpy(task->packet.data, data, datalen);
	task->proc = proc;

	eag_log_debug("eag_hansi", "eag_hansi_task new ok");
	return task;
}

static int
eag_hansi_task_free(eag_hansi_task_t *task)
{
	eag_hansi_proc_t *proc = NULL;

	if (NULL == task) {
		eag_log_err("eag_hansi_task_free input error");
		return -1;
	}
	if (NULL != task->t_write) {
		eag_thread_cancel(task->t_write);
		task->t_write = NULL;
	}
	
	proc = task->proc;
	if (NULL == proc) {
		eag_log_err("eag_hansi_task_free proc null");
		return -1;
	}

	eag_blkmem_free_item(proc->task_blkmem, task);

	eag_log_debug("eag_hansi", "eag_hansi_task free ok");
	return 0;
}

static int
eag_hansi_fifo_push(eag_hansi_proc_t *proc,
						eag_hansi_task_t *task)
{
	if (NULL == proc || NULL == task) {
		eag_log_err("eag_hansi_fifo_push input error");
		return -1;
	}

	list_add_tail(&(task->node), &(proc->task_fifo));

	return 0;
}

static eag_hansi_task_t *
eag_hansi_fifo_pop(eag_hansi_proc_t *proc)
{
	eag_hansi_task_t *task = NULL;
	
	if (NULL == proc) {
		eag_log_err("eag_hansi_fifo_pop input error");
		return NULL;
	}

	if (list_empty(&(proc->task_fifo))) {
		return NULL;
	}
	task = list_entry(proc->task_fifo.next, eag_hansi_task_t, node);
	list_del(&(task->node));
	
	return task;
}

static int
eag_hansi_fifo_empty(eag_hansi_proc_t *proc)
{
	if (NULL == proc) {
		eag_log_err("eag_hansi_fifo_empty input error");
		return 1;
	}

	return list_empty(&(proc->task_fifo));
}

static eag_hansi_task_t *
eag_hansi_fifo_peek(eag_hansi_proc_t *proc)
{
	if (NULL == proc) {
		eag_log_err("eag_hansi_fifo_peek input error");
		return NULL;
	}

	if (list_empty(&(proc->task_fifo))) {
		return NULL;
	}
	return list_entry(proc->task_fifo.next, eag_hansi_task_t, node);
}

static int
eag_hansi_fifo_jump(eag_hansi_proc_t *proc,
						eag_hansi_task_t *task)
{
	if (NULL == proc || NULL == task) {
		eag_log_err("eag_hansi_fifo_jump input error");
		return -1;
	}

	if (list_empty(&(proc->task_fifo))) {
		list_add(&(task->node), &(proc->task_fifo));
	}
	else {
		list_add(&(task->node), proc->task_fifo.next);
	}

	return 0;
}

static int
eag_hansi_fifo_clear(eag_hansi_proc_t *proc)
{
	eag_hansi_task_t *task = NULL;
	eag_hansi_task_t *next = NULL;

	if (NULL == proc) {
		eag_log_err("eag_hansi_fifo_clear input error");
		return -1;
	}
	
	list_for_each_entry_safe(task, next, &(proc->task_fifo), node) {
		list_del(&(task->node));
		eag_hansi_task_free(task);
	}

	return 0;
}

static int
eag_hansi_task_write(eag_thread_t *thread)
{
	eag_hansi_task_t *task = NULL;
	eag_hansi_proc_t *proc = NULL;
	ssize_t writen = 0;
	struct timeval tv;

	if (NULL == thread) {
		eag_log_err("eag_hansi_task_write input error");
		return EAG_ERR_INPUT_PARAM_ERR;
	}
	task = eag_thread_get_arg(thread);
	if (NULL == task) {
		eag_log_err("eag_hansi_task_write task is null");
		return EAG_ERR_UNKNOWN;
	}
	proc = task->proc;
	
	/* time_proc */
	eag_time_gettimeofday(&tv, NULL);
	eag_log_debug("eag_hansi","eag_hansi_task_write type=%d, time=%lu",task->packet.type,tv.tv_sec);
	task->packet.tv.tv_sec = htonl(tv.tv_sec);
	task->packet.tv.tv_usec = htonl(tv.tv_usec);
	
	writen = write(proc->conn_fd, (uint8_t *)&(task->packet)+task->writen,
				ntohl(task->packet.pktlen)-task->writen);
	if (writen < 0) {
		eag_log_err("eag_hansi_task_write write failed, fd(%d), %s", 
					proc->conn_fd, safe_strerror(errno));
		if (EAGAIN != errno && EWOULDBLOCK != errno && EINTR != errno) {
			eag_hansi_proc_stop(proc);
			return -1;
		}
	}
	else if (0 == writen) {
		eag_log_warning("eag_hansi_task_write writen = 0");
	}
	else {
		task->writen += writen;
		eag_log_debug("eag_hansi", "eag_hansi_task_write writen=%d", writen);
	}

	if (task->writen >= ntohl(task->packet.pktlen)) {
		eag_thread_cancel(task->t_write);
		task->t_write = NULL;
		eag_log_debug("eag_hansi", "eag_hansi_task_write, write one task ok");
		if (eag_hansi_is_backup(proc->eaghansi))
		{
			task = eag_hansi_fifo_pop(proc);
			if (NULL != task) {
				eag_hansi_task_free(task);
			}
			else {
				eag_log_err("eag_hansi_task_write no task in backup fifo");
			}
			task = eag_hansi_fifo_peek(proc);
			if (NULL != task) {
				eag_log_err("eag_hansi_task_write has more tasks in backup fifo");
				eag_hansi_task_start_write(task);
			}
		}
	}
	else {
		eag_log_debug("eag_hansi", "eag_hansi_task_write data not end, wait next write");
	}

	return EAG_RETURN_OK;
}

static int
eag_hansi_task_start_write(eag_hansi_task_t *task)
{
	eag_hansi_proc_t *proc = NULL;
	
	if (NULL == task) {
		eag_log_err("eag_hansi_task_start_write input error");
		return -1;
	}
	proc = task->proc;
	if (NULL == proc) {
		eag_log_err("eag_hansi_task_start_write proc is null");
		return -1;
	}
	if (proc->conn_fd < 0) {
		eag_log_err("eag_hansi_task_start_write proc connfd invalid");
		return -1;
	}
	
	task->t_write = eag_thread_add_write(proc->master,
					eag_hansi_task_write,
					task,
					proc->conn_fd);
	if (NULL == task->t_write) {
		eag_log_err("eag_hansi_task_start_write thread_add_write failed");
		eag_hansi_proc_stop(proc);
		return EAG_ERR_UNKNOWN;
	}

	eag_log_debug("eag_hansi", "eag_hansi_task_start_write ok");
	
	return EAG_RETURN_OK;
}

backup_type_t *
eag_hansi_register_backup_type(eag_hansi_t *eaghansi,
								const char *type_name,
								void *cbp,
								ON_GET_BACKUP_DATA on_get_backup_data)
{
	static uint32_t type = 0;
	struct eag_hansi_backup_type *bktype = NULL;

	bktype = eag_malloc(sizeof(backup_type_t));
	if( NULL == bktype ){
		eag_log_err("eag_hansi_register_backup_type malloc failed");
		return NULL;
	}
	type++;
	memset(bktype, 0, sizeof(backup_type_t));
	bktype->type = type;
	strncpy(bktype->type_name, type_name, sizeof(bktype->type_name)-1);
	bktype->cbp = cbp;
	bktype->get_backup_data = on_get_backup_data;
	
	list_add_tail(&(bktype->node), &(eaghansi->bktype_head));

	eag_log_info("eag_hansi_register_backup_type id = %u : %s",type, type_name);
	return bktype;
}

static backup_type_t *
eag_hansi_get_backup_type_by_id(eag_hansi_t *eaghansi, 
											uint32_t type)
{
	backup_type_t *bktype = NULL;
	
	list_for_each_entry(bktype, &(eaghansi->bktype_head), node) {
		if (type == bktype->type) {
			eag_log_debug("eag_hansi","eag_hansi_get_backup_type type %d,"
						"type_name is %s", bktype->type, bktype->type_name);
			return bktype;
		}
	}

	eag_log_err("eag_hansi_get_backup_type type %d no in list", type);
	return NULL;
}

int
eag_hansi_queue_data(eag_hansi_t *eaghansi,
						backup_type_t *bktype,
						void *data,
						uint32_t datalen)
{
	backup_type_t *tmp_bktype = NULL;
	eag_hansi_task_t *task = NULL;
		
	if (NULL == eaghansi || NULL == bktype
		|| NULL == data || 0 == datalen)
	{
		eag_log_err("eag_hansi_queue_data input error");
		return EAG_ERR_INPUT_PARAM_ERR;
	}

	if (datalen > sizeof(task->packet.data)) {
		eag_log_err("eag_hansi_queue_data datalen is over the limit");
		return EAG_ERR_UNKNOWN;
	}

	if (NULL == eaghansi->proc) {
		eag_log_err("eag_hansi_queue_data proc is null");
		return -1;
	}
	if ( eaghansi->proc->conn_fd < 0) {
		eag_log_debug("eag_hansi", "eag_hansi_queue_data backup not connect");
		return 0;
	}

	tmp_bktype = eag_hansi_get_backup_type_by_id(eaghansi, bktype->type);
	if (NULL == tmp_bktype || tmp_bktype != bktype) {
		eag_log_err("eag_hansi_queue_data get_backup_type_by_id error");
		return EAG_ERR_UNKNOWN;
	}

	task = eag_hansi_task_new(eaghansi->proc, bktype, data, datalen);
	if (NULL == task) {
		eag_log_err("eag_hansi_queue_data eag_hansi_task_new failed");
		return -1;
	}
	if (eag_hansi_fifo_empty(eaghansi->proc)) {
		eag_log_debug("eag_hansi", "eag_hansi_queue_data fifo empty, write now");
		eag_hansi_fifo_push(eaghansi->proc, task);
		eag_hansi_task_start_write(task);
	}
	else {
		eag_log_debug("eag_hansi", "eag_hansi_queue_data fifo not empty, only push");
		eag_hansi_fifo_push(eaghansi->proc, task);
	}
	
	return 0;
}

int
eag_hansi_jump_data(eag_hansi_t *eaghansi,
						backup_type_t *bktype,
						void *data,
						uint32_t datalen)
{
	backup_type_t *tmp_bktype = NULL;
	eag_hansi_task_t *task = NULL;
		
	if (NULL == eaghansi || NULL == bktype
		|| NULL == data || 0 == datalen)
	{
		eag_log_err("eag_hansi_jump_data input error");
		return EAG_ERR_INPUT_PARAM_ERR;
	}

	if (datalen > sizeof(task->packet.data)) {
		eag_log_err("eag_hansi_jump_data datalen is over the limit");
		return EAG_ERR_UNKNOWN;
	}

	if (NULL == eaghansi->proc) {
		eag_log_err("eag_hansi_jump_data proc is null");
		return -1;
	}
	if ( eaghansi->proc->conn_fd < 0) {
		eag_log_debug("eag_hansi", "eag_hansi_jump_data backup not connect");
		return 0;
	}

	tmp_bktype = eag_hansi_get_backup_type_by_id(eaghansi, bktype->type);
	if (NULL == tmp_bktype || tmp_bktype != bktype) {
		eag_log_err("eag_hansi_jump_data get_backup_type_by_id error");
		return EAG_ERR_UNKNOWN;
	}

	task = eag_hansi_task_new(eaghansi->proc, bktype, data, datalen);
	if (NULL == task) {
		eag_log_err("eag_hansi_jump_data eag_hansi_task_new failed");
		return -1;
	}
	if (eag_hansi_fifo_empty(eaghansi->proc)) {
		eag_log_debug("eag_hansi", "eag_hansi_jump_data fifo empty, write now");
		eag_hansi_fifo_jump(eaghansi->proc, task);
		eag_hansi_task_start_write(task);
	}
	else {
		eag_log_debug("eag_hansi", "eag_hansi_jump_data fifo not empty, only jump");
		eag_hansi_fifo_jump(eaghansi->proc, task);
	}
	
	return 0;
}

int
eag_hansi_prev_is_enable(eag_hansi_t * eaghansi)
{
	if (NULL == eaghansi) {
		eag_log_err("eag_hansi_prev_is_enable input error");
		return 0;
	}

	if (VRRP_STATE_INIT == eaghansi->params.prev_state
		||  VRRP_STATE_BACK == eaghansi->params.prev_state 
		||  VRRP_STATE_MAST == eaghansi->params.prev_state 
		||  VRRP_STATE_LEARN == eaghansi->params.prev_state 
		||  VRRP_STATE_NONE == eaghansi->params.prev_state 
		||  VRRP_STATE_TRANSFER == eaghansi->params.prev_state) {
		return 1;
	}

	return 0;
}

int
eag_hansi_prev_is_master(eag_hansi_t * eaghansi)
{
    if (NULL == eaghansi) {
        eag_log_err("eag_hansi_prev_is_master input error");
        return 0;
    }
	
    return VRRP_STATE_MAST == eaghansi->params.prev_state;
}

int
eag_hansi_is_enable(eag_hansi_t * eaghansi)
{
	if( NULL == eaghansi ){
		eag_log_err("eag_hansi_is_enable input error");
		return 0;
	}

	if (VRRP_STATE_INIT == eaghansi->params.state 
		||  VRRP_STATE_BACK == eaghansi->params.state 
		||  VRRP_STATE_MAST == eaghansi->params.state 
		||  VRRP_STATE_LEARN == eaghansi->params.state 
		||  VRRP_STATE_NONE == eaghansi->params.state 
		||  VRRP_STATE_TRANSFER == eaghansi->params.state) {
		return 1;
	}

	return 0;
}

int
eag_hansi_is_master(eag_hansi_t * eaghansi)
{
    if (NULL == eaghansi) {
        eag_log_err("eag_hansi_is_master input error");
        return 0;
    }
	
    return VRRP_STATE_MAST == eaghansi->params.state;
}

int
eag_hansi_is_backup(eag_hansi_t * eaghansi)
{
    if (NULL == eaghansi){
        eag_log_err("eag_hansi_is_backup input error");
        return 0;
    }
	
    return (VRRP_STATE_BACK == eaghansi->params.state
		|| VRRP_STATE_TRANSFER == eaghansi->params.state);
}

static int
eag_hansi_do_ack_data(eag_hansi_proc_t *proc)
{
	eag_hansi_t *eaghansi = NULL;
	backup_type_t *bktype = NULL;
	eag_hansi_task_t *task = NULL;
	int ret = 0;

	if (NULL == proc) {
		eag_log_err("eag_hansi_do_ack_data input error");
		return -1;
	}
	eaghansi = proc->eaghansi;
	
	bktype = eag_hansi_get_backup_type_by_id(eaghansi, proc->readbuf.type);
	if (NULL == bktype) {
		eag_log_err("eag_hansi_do_ack_data get an unknown backup type %d",
				proc->readbuf.type);
		return -1;
	}
	eag_log_debug("eag_hansi", "eag_hansi_do_ack_data "
				"get backup type:%d, type_name:%s",
				bktype->type, bktype->type_name);
	if (NULL == bktype->get_backup_data) {
		eag_log_err("eag_hansi_do_ack_data get_backup_data is null");
		return -1;
	}

	ret = bktype->get_backup_data(bktype->cbp, proc->readbuf.data, (void *)0);
	if (EAG_RETURN_OK != ret) {
		eag_log_err("eag_hansi_do_ack_data get_backup_data error");
	}
	
	task = eag_hansi_fifo_pop(proc);
	if (NULL != task) {
		eag_hansi_task_free(task);
	}
	else {
		eag_log_err("eag_hansi_do_ack_data has not task in fifo");
	}
	task = eag_hansi_fifo_peek(proc);
	if (NULL != task) {
		eag_log_debug("eag_hansi", "eag_hansi_do_ack_data have other tasks,"
			" continue write next task");
		eag_hansi_task_start_write(task);
	}
	else {
		eag_log_debug("eag_hansi", "eag_hansi_do_ack_data all tasks completed");
	}
	
	return 0;
}

static int
eag_hansi_do_syn_data(eag_hansi_proc_t *proc)
{
	eag_hansi_t *eaghansi = NULL;
	backup_type_t *bktype = NULL;
	int ret = 0;
	struct timeval *syn_tv;

	if (NULL == proc) {
		eag_log_err("eag_hansi_do_syn_data input error");
		return -1;
	}
	eaghansi = proc->eaghansi;
	
	bktype = eag_hansi_get_backup_type_by_id(eaghansi, proc->readbuf.type);
	if (NULL == bktype) {
		eag_log_err("eag_hansi_do_syn_data get an unknown backup type %d",
				proc->readbuf.type);
		return -1;
	}

	syn_tv = &(proc->readbuf.tv);	
	eag_log_debug("eag_hansi", "eag_hansi_do_syn_data "
				"get backup type:%d, type_name:%s,last_time=%lu",
				bktype->type, bktype->type_name, syn_tv->tv_sec);
	if (NULL == bktype->get_backup_data) {
		eag_log_err("eag_hansi_do_syn_data get_backup_data is null");
		return -1;
	}

	ret = bktype->get_backup_data(bktype->cbp, proc->readbuf.data, syn_tv);
	if (EAG_RETURN_OK != ret) {
		eag_log_err("eag_hansi_do_syn_data get_backup_data error");
	}

	return 0;
}

static void
eag_hansi_on_master_accept(eag_hansi_t *eaghansi)
{
	int ret = 0;
	
	if (NULL == eaghansi) {
		eag_log_err("eag_hansi_on_master_accept input error");
		return;
	}
	
	if (NULL != eaghansi->on_master_accept) {
		ret = eaghansi->on_master_accept(eaghansi,
							eaghansi->on_master_accept_param);
		if (EAG_RETURN_OK != ret) {
			eag_log_warning("eag_hansi_on_master_accept "
					"on_master_accept return %d", ret);
		}
	}
	else {
		eag_log_warning("eag_hansi_on_master_accept on_master_accept is null");
	}
}

static void
eag_hansi_on_backup_connect(eag_hansi_t *eaghansi)
{
	int ret = 0;

	if (NULL == eaghansi) {
		eag_log_err("eag_hansi_on_backup_connect input error");
		return;
	}

	if (NULL != eaghansi->on_backup_connect) {
		ret = eaghansi->on_backup_connect(eaghansi,
							eaghansi->on_backup_connect_param);
		if (EAG_RETURN_OK != ret) {
			eag_log_warning("eag_hansi_on_backup_connect "
					"on_backup_connect return %d", ret);
		}
	}
	else {
		eag_log_warning("eag_hansi_on_backup_connect on_backup_connect is null");
	}
}

static void
eag_hansi_on_status_change(eag_hansi_t *eaghansi)
{
	int ret = 0;
		
	if (NULL == eaghansi) {
		eag_log_err("eag_hansi_on_status_change input error");
		return;
	}
		
	if (NULL != eaghansi->on_status_change) {
		ret = eaghansi->on_status_change(eaghansi,
							eaghansi->on_status_change_param);
		if (EAG_RETURN_OK != ret) {
			eag_log_warning("eag_hansi_on_status_change "
					"on_status_change return %d", ret);
		}
	}
	else {
		eag_log_warning("eag_hansi_on_status_change on_status_change is null");
	}
}

static void
eag_hansi_on_backup_heartbeat_timeout( eag_hansi_t *eaghansi )
{
	int ret = 0;
		
	if (NULL == eaghansi) {
		eag_log_err("eag_hansi_on_backup_heartbeat_timeout input error");
		return;
	}
		
	if (NULL != eaghansi->on_backup_heartbeat_timeout) {
		ret = eaghansi->on_backup_heartbeat_timeout(eaghansi,
							eaghansi->on_backup_heartbeat_timeout_param);
		if (EAG_RETURN_OK != ret) {
			eag_log_warning("eag_hansi_on_backup_heartbeat_timeout "
					"callback return %d", ret);
		}
	}
	else {
		eag_log_warning("eag_hansi_on_backup_heartbeat_timeout callback is null");
	}
}

static int
eag_hansi_read_on_master(eag_thread_t *thread)
{
	eag_hansi_proc_t *proc = NULL;
	ssize_t recvlen = 0;
	
	if (NULL == thread) {
		eag_log_err("eag_hansi_read_on_master input error");
		return -1;
	}
    proc = eag_thread_get_arg(thread);
	if (NULL == proc) {
		eag_log_err("eag_hansi_read_on_master proc null");
		return -1;
	}
	
	recvlen = recv(proc->conn_fd, (uint8_t *)&(proc->readbuf)+proc->readlen,
					sizeof(proc->readbuf)-proc->readlen, 0);
	if (recvlen < 0) {
		eag_log_warning("eag_hansi_read_on_master recv failed, fd(%d): %s",
				proc->conn_fd, safe_strerror(errno));
		if (EINTR != errno && EAGAIN != errno && EWOULDBLOCK != errno) {
			eag_hansi_proc_stop(proc);
			return -1;
		}
	}
	else if (0 == recvlen) { /* the peer shutdown */
		eag_log_warning("eag_hansi_read_on_master fd(%d), recvlen = 0",
				proc->conn_fd);
		eag_hansi_proc_stop(proc);
		return -1;
	}
	else {  /* recvlen > 0 */
		proc->readlen += recvlen;
	}
	eag_log_debug("eag_hansi", "eag_hansi_read_on_master readlen=%d",
				proc->readlen);
	if (proc->readlen >= BACKUP_PACKET_HEADSIZE
		&& proc->readlen >= ntohl(proc->readbuf.pktlen)) {
		proc->readbuf.pktlen = ntohl(proc->readbuf.pktlen);
		proc->readbuf.type = ntohl(proc->readbuf.type);
		if (proc->readbuf.pktlen < BACKUP_PACKET_HEADSIZE) {
			eag_log_warning("eag_hansi_read_on_master abnormal, "
				"packet length %d < packet head size %d",
				proc->readbuf.pktlen, BACKUP_PACKET_HEADSIZE);
		}

		eag_hansi_do_ack_data(proc);
		memset(&(proc->readbuf), 0, sizeof(proc->readbuf));
		proc->readlen = 0;
	}
	else {
		eag_log_debug("eag_hansi",
			"eag_hansi_read_on_master data not end, wait next recv");
	}
	
	return EAG_RETURN_OK;
}

static int
eag_hansi_heartbeat_timeout_on_master(eag_thread_t *thread)
{
	eag_hansi_proc_t *proc = NULL;
	eag_hansi_t *eaghansi=NULL;
	
	if (NULL == thread) {
		eag_log_err("eag_hansi_heartbeat_on_master input error");
		return -1;
	}
	proc = eag_thread_get_arg(thread);
	if (NULL == proc) {
		eag_log_err("eag_hansi_heartbeat_on_master proc null");
		return -1;
	}
	eaghansi = proc->eaghansi;
	if (NULL == eaghansi) {
		eag_log_err("eag_hansi_heartbeat_on_master eaghansi null");
		return -1;
	}
	eag_thread_cancel(proc->t_heartbeat);
	proc->t_heartbeat = NULL;
	eag_hansi_syn_time(eaghansi);
	proc->t_heartbeat = eag_thread_add_timer(proc->master,
					eag_hansi_heartbeat_timeout_on_master,proc,EAG_TIME_SYN_BEAT_TIME);
	
	return EAG_RETURN_OK;
}


static int
eag_hansi_read_on_backup(eag_thread_t *thread)
{
	eag_hansi_proc_t *proc = NULL;
	ssize_t recvlen = 0;

	if (NULL == thread) {
		eag_log_err("eag_hansi_read_on_backup input error");
		return -1;
	}
	proc = eag_thread_get_arg(thread);
	if (NULL == proc) {
		eag_log_err("eag_hansi_read_on_backup proc null");
		return -1;
	}

	recvlen = recv(proc->conn_fd, (uint8_t *)&(proc->readbuf)+proc->readlen,
					sizeof(proc->readbuf)-proc->readlen, 0);
	if (recvlen < 0) {
		eag_log_warning("eag_hansi_read_on_backup recv failed, fd(%d): %s",
				proc->conn_fd, safe_strerror(errno));
		if (EINTR != errno && EAGAIN != errno && EWOULDBLOCK != errno) {
			eag_hansi_proc_stop(proc);
			return -1;
		}
	}
	else if (0 == recvlen) { /* the peer shutdown */
		eag_log_warning("eag_hansi_read_on_backup fd(%d), recvlen = 0",
				proc->conn_fd);
		eag_hansi_proc_stop(proc);
		return -1;
	}
	else {  /* recvlen > 0 */
		proc->readlen += recvlen;
	}
	eag_log_debug("eag_hansi", "eag_hansi_read_on_backup readlen=%d",
				proc->readlen);
	if (proc->readlen >= BACKUP_PACKET_HEADSIZE
		&& proc->readlen >= ntohl(proc->readbuf.pktlen)) {
		proc->readbuf.pktlen = ntohl(proc->readbuf.pktlen);
		proc->readbuf.type = ntohl(proc->readbuf.type);
		proc->readbuf.tv.tv_sec = ntohl(proc->readbuf.tv.tv_sec);
		proc->readbuf.tv.tv_usec = ntohl(proc->readbuf.tv.tv_usec);
		eag_log_debug("eag_hansi","eag_hansi_read_on_backup  type=%d,tv=%p",proc->readbuf.type,&(proc->readbuf.tv));
		if (proc->readbuf.pktlen < BACKUP_PACKET_HEADSIZE) {
			eag_log_warning("eag_hansi_read_on_backup abnormal, "
				"packet length %d < packet head size %d",
				proc->readbuf.pktlen, BACKUP_PACKET_HEADSIZE);
		}

		eag_hansi_do_syn_data(proc);
		memset(&(proc->readbuf), 0, sizeof(proc->readbuf));
		proc->readlen = 0;
	}
	else {
		eag_log_debug("eag_hansi",
			"eag_hansi_read_on_backup data not end, wait next recv");
	}

	return EAG_RETURN_OK;
}

static int
eag_hansi_heartbeat_timeout_on_backup(eag_thread_t *thread)
{
	eag_hansi_proc_t *proc = NULL;
	eag_hansi_t *eaghansi=NULL;
	
	if (NULL == thread) {
		eag_log_err("eag_hansi_heartbeat_on_backup input error");
		return -1;
	}
	proc = eag_thread_get_arg(thread);
	if (NULL == proc) {
		eag_log_err("eag_hansi_heartbeat_on_backup proc null");
		return -1;
	}
	eaghansi = proc->eaghansi;
	if (NULL == eaghansi) {
		eag_log_err("eag_hansi_heartbeat_on_backup eaghansi null");
		return -1;
	}
	eag_thread_cancel( proc->t_heartbeat );
	proc->t_heartbeat = NULL;
	
	//usually will not be called unless the tcp connect has error!
	eag_hansi_backup_stop(eaghansi);
	eag_hansi_on_backup_heartbeat_timeout(eaghansi);
	eag_hansi_backup_start(eaghansi);

	return EAG_RETURN_OK;
}


static eag_hansi_proc_t *
eag_hansi_proc_new(eag_hansi_t *eaghansi)
{
	eag_hansi_proc_t *proc = NULL;

	if (NULL == eaghansi) {
		eag_log_err("eag_hansi_proc_new input error");
		return NULL;
	}
	proc = eag_malloc(sizeof(eag_hansi_proc_t));
	if (NULL == proc) {
		eag_log_err("eag_hansi_proc_new malloc failed");
		return NULL;
	}
	
	memset(proc, 0, sizeof(eag_hansi_proc_t));
	if (EAG_RETURN_OK != eag_blkmem_create(&(proc->task_blkmem),
							EAG_HANSI_TASK_BLKMEM_NAME,
							sizeof(eag_hansi_task_t), 
							EAG_HANSI_TASK_BLKMEM_ITEMNUM,
							EAG_HANSI_TASK_BLKMEM_MAXNUM)) {
		eag_log_err("eag_hansi_proc_new eag_blkmem_create failed");
		eag_free(proc);
		return NULL;
	}

	proc->conn_fd = -1;
	proc->eaghansi = eaghansi;
	proc->master = eaghansi->master;
	INIT_LIST_HEAD(&(proc->task_fifo));

	eag_log_info("eag_hansi_proc_new ok");
	return proc;
}

static int
eag_hansi_proc_free(eag_hansi_proc_t *proc)
{
	if (NULL == proc) {
		eag_log_err("eag_hansi_proc_free input error");
		return EAG_ERR_INPUT_PARAM_ERR;
	}

	if (NULL != proc->t_read) {
		eag_thread_cancel(proc->t_read);
		proc->t_read = NULL;
	}
	if (proc->conn_fd >= 0) {
		close(proc->conn_fd);
		proc->conn_fd = -1;
	}

	eag_hansi_fifo_clear(proc);
	
	if (NULL != proc->task_blkmem) {
		eag_blkmem_destroy(&(proc->task_blkmem));
	}
	eag_free(proc);

	eag_log_info("eag_hansi_proc_free ok");
	return EAG_RETURN_OK;
}

static int 
eag_hansi_proc_stop(eag_hansi_proc_t *proc)
{
	if (NULL == proc) {
		eag_log_err("eag_hansi_proc_stop input error");
		return EAG_ERR_INPUT_PARAM_ERR;
	}
	
	if (NULL != proc->t_read) {
		eag_thread_cancel(proc->t_read);
		proc->t_read = NULL;
	}
	if (proc->conn_fd >= 0) {
		close(proc->conn_fd);
		proc->conn_fd = -1;
	}
	
	if (NULL != proc->t_heartbeat) {
		eag_thread_cancel(proc->t_heartbeat);
		proc->t_heartbeat = NULL;
	}
	
	eag_hansi_fifo_clear(proc);
	
	memset(&(proc->readbuf), 0, sizeof(proc->readbuf));
	proc->readlen = 0;
	eag_log_info("eag_hansi_proc_stop ok");
	
	return EAG_RETURN_OK;
}

static int 
eag_hansi_proc_start_on_master(eag_hansi_proc_t *proc)
{
	if (NULL == proc) {
		eag_log_err("eag_hansi_proc_start_on_master input null");
		return -1;
	}
	
	proc->t_read = eag_thread_add_read(proc->master,
						eag_hansi_read_on_master, proc, proc->conn_fd);
	if (NULL == proc->t_read) {
		eag_log_err("eag_hansi_proc_start_on_master thread_add_read failed");
		eag_hansi_proc_stop(proc);
		return EAG_ERR_UNKNOWN;
	}
	proc->t_heartbeat = eag_thread_add_timer(proc->master,
						eag_hansi_heartbeat_timeout_on_master,
						proc,EAG_TIME_SYN_BEAT_TIME);
	if (NULL == proc->t_heartbeat) {
		eag_log_err("eag_hansi_proc_start_on_master eag_thread_add_timer failed");
		eag_hansi_proc_stop(proc);
		return EAG_ERR_UNKNOWN;
	}

	eag_log_info("eag_hansi_proc_start_on_master ok");

	return EAG_RETURN_OK;
}

static int 
eag_hansi_proc_start_on_backup(eag_hansi_proc_t *proc)
{
	if (NULL == proc) {
		eag_log_err("eag_hansi_proc_start_on_backup input null");
		return -1;
	}
	
	proc->t_read = eag_thread_add_read(proc->master,
					eag_hansi_read_on_backup, proc, proc->conn_fd);
	if (NULL == proc->t_read) {
		eag_log_err("eag_hansi_proc_start_on_backup thread_add_read failed");
		eag_hansi_proc_stop(proc);
		return EAG_ERR_UNKNOWN;
	}
	
	proc->t_heartbeat= eag_thread_add_timer(proc->master,
					eag_hansi_heartbeat_timeout_on_backup, 
					proc, EAG_TIME_SYN_BEAT_TIME*3 );
	if (NULL == proc->t_heartbeat) {
		eag_log_err("eag_hansi_proc_start_on_backup eag_thread_add_timer failed");
		eag_hansi_proc_stop(proc);
		return EAG_ERR_UNKNOWN;
	}
	eag_log_info("eag_hansi_proc_start_on_backup ok");

	return EAG_RETURN_OK;
}

void
eag_hansi_set_on_master_accept_cb(eag_hansi_t *eaghansi,
											EAG_HANSI_ON_MASTER_ACCEPT on_master_accept,
											void *param)
{
	if (NULL == eaghansi) {
		eag_log_err("eag_hansi_set_on_backup_connect_cb input error");
		return;
	}
	
	eaghansi->on_master_accept = on_master_accept;
	eaghansi->on_master_accept_param = param;
}

void
eag_hansi_set_on_backup_connect_cb(eag_hansi_t *eaghansi,
											EAG_HANSI_ON_BACKUP_CONNECT on_backup_connect,
											void *param)
{
	if (NULL == eaghansi) {
		eag_log_err("eag_hansi_set_on_backup_connect_cb input error");
		return;
	}

	eaghansi->on_backup_connect = on_backup_connect;
	eaghansi->on_backup_connect_param = param;
}

void
eag_hansi_set_on_status_change_cb( eag_hansi_t *eaghansi,
											EAG_HANSI_ON_STATUS_CHANGE on_status_change,
											void *param)
{
	if (NULL == eaghansi) {
		eag_log_err("eag_hansi_set_on_backup_connect_cb input error");
		return;
	}
		
	eaghansi->on_status_change = on_status_change;
	eaghansi->on_status_change_param = param;
}

void
eag_hansi_set_on_backup_heartbeat_timeout_cb( eag_hansi_t *eaghansi,
							EAG_HANSI_ON_BACKUP_HEARTBEAT_TIMEOUT on_backup_heartbeat_timeout,
							void *param )
{
	if (NULL == eaghansi) {
		eag_log_err("eag_hansi_set_on_backup_connect_cb input error");
		return;
	}
		
	eaghansi->on_backup_heartbeat_timeout = on_backup_heartbeat_timeout;
	eaghansi->on_backup_heartbeat_timeout_param = param;
}

int
eag_hansi_add_hansi_param_forward( eag_hansi_t *eaghansi,
				char *objpath, char *name, char *interfacex, char *method)
{
	int num = 0;
	if( NULL == eaghansi || NULL == objpath 
		|| NULL == name || NULL == interfacex
		|| NULL==method
		|| strlen(objpath) == 0 || strlen(objpath)>=MAX_DBUS_OBJPATH_LEN
		|| strlen(name) == 0 || strlen(name)>=MAX_DBUS_BUSNAME_LEN
		|| strlen(interfacex) == 0 || strlen(interfacex)>=MAX_DBUS_INTERFACE_LEN
		|| strlen(method) == 0 || strlen(method)>=MAX_DBUS_METHOD_LEN )
	{
		eag_log_err("eag_hansi_add_hansi_param_forward input param error!");
		return EAG_ERR_INPUT_PARAM_ERR;
	}

 	num = eaghansi->forward_num;
	if( num >= MAX_FORWARD_NUM ){
		eag_log_err("eag_hansi_add_hansi_param_forward num %d limite!",
						MAX_FORWARD_NUM);
		return EAG_ERR_UNKNOWN;
	}

	strcpy( eaghansi->forward[num].objpath, objpath );
	strcpy( eaghansi->forward[num].name, name );
	strcpy( eaghansi->forward[num].interfacex, interfacex );
	strcpy( eaghansi->forward[num].method, method );
	eaghansi->forward_num++;
	return EAG_RETURN_OK;
}

int
eag_hansi_del_hansi_param_forward( eag_hansi_t *eaghansi,
				char *objpath, char *name, char *interfacex, char *method )
{
	int num = 0;
	int i;
	if( NULL == eaghansi || NULL == objpath 
		|| NULL == name || NULL == interfacex
		|| NULL==method
		|| strlen(objpath) == 0 || strlen(objpath)>=MAX_DBUS_OBJPATH_LEN
		|| strlen(name) == 0 || strlen(name)>=MAX_DBUS_BUSNAME_LEN
		|| strlen(interfacex) == 0 || strlen(interfacex)>=MAX_DBUS_INTERFACE_LEN
		|| strlen(method) == 0 || strlen(method)>=MAX_DBUS_METHOD_LEN )
	{
		eag_log_err("eag_hansi_del_hansi_param_forward input param error!");
		return EAG_ERR_INPUT_PARAM_ERR;
	}

	num = eaghansi->forward_num;
	if( num > MAX_FORWARD_NUM ){
		eag_log_err("eag_hansi_del_hansi_param_forward num %d limite!",
						MAX_FORWARD_NUM);
		return EAG_ERR_UNKNOWN;
	}

	for( i=0;i<num;i++){
		if( 0 == strcmp( eaghansi->forward[i].objpath, objpath )
			&& 0 ==	strcmp( eaghansi->forward[i].name, name )
			&& 0 == strcmp( eaghansi->forward[i].interfacex, interfacex )
			&& 0 == strcmp( eaghansi->forward[i].method, method ))
		{
			break;
		}
	}

	if( i != num ){
		for( ;i<num-1;i++){
			strcpy( eaghansi->forward[i].objpath, 
						eaghansi->forward[i+1].objpath);
			strcpy( eaghansi->forward[i].name, 
						eaghansi->forward[i+1].name );
			strcpy( eaghansi->forward[i].interfacex, 
						eaghansi->forward[i+1].interfacex );
			strcpy( eaghansi->forward[i].method, 
						eaghansi->forward[i+1].method );
		}
		eaghansi->forward_num--;
		return EAG_RETURN_OK;
	}else{
		eag_log_err("eag_hansi_add_hansi_param_forward not find!");
	}
	
	return EAG_ERR_UNKNOWN;
}

static void
eag_hansi_forward_param( eag_hansi_t *eaghansi, 
						struct eag_hansi_param_forward *forward )
{
	if( NULL == eaghansi || NULL == forward ){
		return;
	}
	struct eag_hansi_param *params = NULL;
	DBusMessage *query = NULL;
	DBusMessage *reply = NULL;
	DBusError err;
	DBusMessageIter iter;
	DBusMessageIter iter_array, iter_array1, iter_array_vgw;
	
	int op_ret = 0;
	int master_uplinkip = 0;
	int master_downlinkip = 0;
	int back_uplinkip = 0;
	int back_downlinkip = 0;
	char *uplink_ifname = NULL;
	char *downlink_ifname = NULL;
	char *heartbeatlink_if = NULL;
	char *vgateway_if = NULL;
	unsigned long vgateway_ip = 0;
	unsigned long virtual_uplink_ip = 0;
	unsigned long virtual_downlink_ip = 0;
	int uplink_cnt = 0, downlink_cnt = 0, vgateway_cnt = 0;
	int i = 0;	

	params = &(eaghansi->params);
	/*
	DBUS_TYPE_UINT32					// vrid
	DBUS_TYPE_UINT32					// state
	DBUS_TYPE_UINT32					// count of uplink interfaces
	Array of uplink
		DBUS_TYPE_UINT32_AS_STRING		// master uplnik ip address
		DBUS_TYPE_UINT32_AS_STRING		// back uplink ip address
		DBUS_TYPE_UINT32_AS_STRING		// virtual uplink ip address
		DBUS_TYPE_STRING_AS_STRING		// uplink interface name
	DBUS_TYPE_UINT32					// count of downlink interfaces
	Array of uplink
		DBUS_TYPE_UINT32_AS_STRING		// master downlnik ip address
		DBUS_TYPE_UINT32_AS_STRING		// back downlink ip address
		DBUS_TYPE_UINT32_AS_STRING		// virtual downlink ip address
		DBUS_TYPE_STRING_AS_STRING		// downlink interface name
	DBUS_TYPE_STRING					// heartbeat interface name
	DBUS_TYPE_UINT32					// heartbeat ip address
	DBUS_TYPE_UINT32					// opposite heartbeat ip address
	*/
	query = dbus_message_new_method_call(	 forward->name,
											 forward->objpath,
											 forward->interfacex,
											 forward->method );
	dbus_error_init(&err);
	
	dbus_message_iter_init_append(query, &iter);
	
	dbus_message_iter_append_basic(&iter,
									DBUS_TYPE_UINT32, &(eaghansi->ins_id));
	dbus_message_iter_append_basic(&iter,
									DBUS_TYPE_UINT32, &(params->state));

	/*up link*/
	uplink_cnt = params->uplink_cnt;
	dbus_message_iter_append_basic(&iter,
									DBUS_TYPE_UINT32, &uplink_cnt);

	dbus_message_iter_open_container (&iter,
									DBUS_TYPE_ARRAY,
									DBUS_STRUCT_BEGIN_CHAR_AS_STRING
									DBUS_TYPE_UINT32_AS_STRING
									DBUS_TYPE_UINT32_AS_STRING
									DBUS_TYPE_UINT32_AS_STRING
									DBUS_TYPE_STRING_AS_STRING
									DBUS_STRUCT_END_CHAR_AS_STRING,
									&iter_array);
	
	if (uplink_cnt > 0)
	{
		for (i = 0; i < uplink_cnt; i++)
		{
			/* init */
			uplink_ifname = NULL;
			virtual_uplink_ip = 0;
			master_uplinkip = 0;
			back_uplinkip = 0;

			uplink_ifname = params->uplink_if[i].if_name;
			virtual_uplink_ip = params->uplink_if[i].virtual_ip;
			back_uplinkip = params->uplink_if[i].backp_ip;
			master_uplinkip = params->uplink_if[i].master_ip;

			DBusMessageIter iter_struct;
			dbus_message_iter_open_container(&iter_array,
											DBUS_TYPE_STRUCT,
											NULL,
											&iter_struct);
			dbus_message_iter_append_basic(&iter_struct,
						  DBUS_TYPE_UINT32, &master_uplinkip);
			dbus_message_iter_append_basic(&iter_struct,
						  DBUS_TYPE_UINT32, &back_uplinkip);
			dbus_message_iter_append_basic(&iter_struct,
						  DBUS_TYPE_UINT32, &virtual_uplink_ip);
			dbus_message_iter_append_basic(&iter_struct,
						  DBUS_TYPE_STRING, &uplink_ifname);

			dbus_message_iter_close_container (&iter_array, &iter_struct);
		}
	}
	dbus_message_iter_close_container(&iter, &iter_array);

	/*down link*/
	downlink_cnt = params->downlink_cnt;
	dbus_message_iter_append_basic(&iter,
									DBUS_TYPE_UINT32, &downlink_cnt);

	dbus_message_iter_open_container (&iter,
									DBUS_TYPE_ARRAY,
									DBUS_STRUCT_BEGIN_CHAR_AS_STRING
									DBUS_TYPE_UINT32_AS_STRING
									DBUS_TYPE_UINT32_AS_STRING
									DBUS_TYPE_UINT32_AS_STRING
									DBUS_TYPE_STRING_AS_STRING
									DBUS_STRUCT_END_CHAR_AS_STRING,
									&iter_array1);
		
	if (downlink_cnt > 0)
	{
		for (i = 0; i < downlink_cnt; i++)
		{
			/* init */
			downlink_ifname = NULL;
			virtual_downlink_ip = 0;
			master_downlinkip = 0;
			back_downlinkip = 0;
		
			downlink_ifname = params->downlink_if[i].if_name;
			virtual_downlink_ip = params->downlink_if[i].virtual_ip;
			
			back_downlinkip = params->downlink_if[i].backp_ip;
			master_downlinkip = params->downlink_if[i].master_ip;
			
			DBusMessageIter iter_struct1;
			dbus_message_iter_open_container(&iter_array1,
											DBUS_TYPE_STRUCT,
											NULL,
											&iter_struct1);
			dbus_message_iter_append_basic(&iter_struct1,
						  DBUS_TYPE_UINT32, &master_downlinkip);
			dbus_message_iter_append_basic(&iter_struct1,
						  DBUS_TYPE_UINT32, &back_downlinkip);
			dbus_message_iter_append_basic(&iter_struct1,
						  DBUS_TYPE_UINT32, &virtual_downlink_ip);
			dbus_message_iter_append_basic(&iter_struct1,
						  DBUS_TYPE_STRING, &downlink_ifname);
			dbus_message_iter_close_container (&iter_array1, &iter_struct1);
		}
	}
	dbus_message_iter_close_container(&iter, &iter_array1);
	
		/*heart link*/
	heartbeatlink_if = params->heartlink_if_name;
	
	dbus_message_iter_append_basic(&iter,
									DBUS_TYPE_STRING, &heartbeatlink_if);
	dbus_message_iter_append_basic(&iter,
									DBUS_TYPE_UINT32, &(params->heartlink_local_ip));
	dbus_message_iter_append_basic(&iter,
									DBUS_TYPE_UINT32, &(params->heartlink_opposite_ip));


		/* vgateway interface */
	dbus_message_iter_append_basic(&iter,
									DBUS_TYPE_UINT32, &params->vgateway_cnt);
	dbus_message_iter_open_container (&iter,
									DBUS_TYPE_ARRAY,
									DBUS_STRUCT_BEGIN_CHAR_AS_STRING
									DBUS_TYPE_STRING_AS_STRING
									DBUS_TYPE_UINT32_AS_STRING
									DBUS_STRUCT_END_CHAR_AS_STRING,
									&iter_array_vgw);
	if (params->vgateway_cnt > 0)
	{
		vgateway_cnt = params->vgateway_cnt;
		for (i = 0; i < vgateway_cnt; i++)
		{
			vgateway_if = params->vgateway_if[i].if_name;
			vgateway_ip = params->vgateway_if[i].virtual_ip;
				
			DBusMessageIter iter_struct_vgw;
			dbus_message_iter_open_container(&iter_array_vgw,
											DBUS_TYPE_STRUCT,
											NULL,
											&iter_struct_vgw);
			dbus_message_iter_append_basic(&iter_struct_vgw,
						  DBUS_TYPE_STRING, &vgateway_if);
			dbus_message_iter_append_basic(&iter_struct_vgw,
						  DBUS_TYPE_UINT32, &vgateway_ip);
			dbus_message_iter_close_container (&iter_array_vgw, &iter_struct_vgw);

			vgateway_if = NULL;	
		}
	}
	dbus_message_iter_close_container(&iter, &iter_array_vgw);

		
	reply = dbus_connection_send_with_reply_and_block (
					eag_dbus_get_dbus_conn(eaghansi->eagdbus),query, 5000, &err);
	
	dbus_message_unref(query);
	if (NULL == reply) {
		eag_log_err("hansi param forward to %s time out!\n", 
							forward->name );
		if (dbus_error_is_set(&err)) {
			dbus_error_free(&err);
		}
	}
	else{
		if (dbus_message_get_args ( reply, &err,
									DBUS_TYPE_UINT32,&op_ret,
									DBUS_TYPE_INVALID))
		{
			eag_log_info("hansi param forward to %s resoult %d!\n", 
							forward->name, op_ret);
		} 
		else {
			eag_log_err("hansi param forward to %s failed!\n", 
							forward->name );
			if (dbus_error_is_set(&err)) {
				dbus_error_free(&err);
			}
		}
		dbus_message_unref(reply);
	}
		
	return;
}

void
eag_hansi_forward_all_param(eag_hansi_t *eaghansi)
{
	int i = 0;
	struct eag_hansi_param_forward *forward = NULL;
		
	eag_log_info("eag_hansi_forward_all_param forward_num=%d\n",
					eaghansi->forward_num );
	for (i = 0; i < eaghansi->forward_num; i++) {
		forward = &(eaghansi->forward[i]);
		eag_log_info("before eag_hansi_forward_all_param to %s %s %s %s\n",
								forward->objpath,
								forward->name,
								forward->interfacex,
								forward->method );
		eag_hansi_forward_param( eaghansi, forward );
	}

	return;
}

static int
eag_hansi_state_init(eag_hansi_t * eaghansi) /*get params and status */
{
	DBusMessage *query = NULL;
	DBusMessage *reply = NULL;
	DBusMessageIter iter;
	DBusMessageIter	 iter_array;
	DBusMessageIter	 iter_array1;
	DBusMessageIter	 iter_array2;
	DBusError err;
	struct eag_hansi_param params = {0};
	char *interface_name = "";
	char master_ip_char[32]="";
	char backup_ip_char[32]="";
	char virtual_ip_char[32]="";
	char vrrp_dbus_name[64] = "";
	char vrrp_obj_path[64] = "";
	int get_hansi_state_flag = 1;
	uint32_t num = 0;
	int i = 0;
	int j = 0;
	
	eag_log_info("eag_hansi_state_init hansi-type:%d, hansi-id:%d",
		eaghansi->hansi_type, eaghansi->ins_id);
	
	memset(&params, 0, sizeof(params));
	memset(&(eaghansi->params), 0, sizeof((eaghansi->params)));
	eaghansi->params.vrid = eaghansi->ins_id;
	eaghansi->params.prev_state = VRRP_STATE_DISABLE;
	eaghansi->params.state = VRRP_STATE_DISABLE;

	snprintf(vrrp_dbus_name, sizeof(vrrp_dbus_name),
			"%s%d", VRRP_DBUS_BUSNAME, eaghansi->ins_id);
	snprintf(vrrp_obj_path, sizeof(vrrp_obj_path),
			"%s%d", VRRP_DBUS_OBJPATH, eaghansi->ins_id);
	query = dbus_message_new_method_call(vrrp_dbus_name, vrrp_obj_path,
						VRRP_DBUS_INTERFACE, VRRP_DBUS_METHOD_SET_PROTAL);
	if (NULL == query) {
		eag_log_err("eag_hansi_state_init new_method_call failed");
		return -1;
	}
	dbus_error_init(&err);
	dbus_message_append_args(query,
						DBUS_TYPE_UINT32, &get_hansi_state_flag,
						DBUS_TYPE_INVALID);
	reply = dbus_connection_send_with_reply_and_block(
					eag_dbus_get_dbus_conn(eaghansi->eagdbus), query, -1, &err);
	dbus_message_unref(query);
	if (NULL == reply) {
		if (dbus_error_is_set(&err)) {
			eag_log_err("eag_hansi_state_init dbus_connection_send %s :%s",
							err.name, err.message);
			dbus_error_free(&err);
		}
		eag_log_err("eag_hansi_state_init dbus_connection_send failed");
		return -1;
	}

	dbus_message_iter_init(reply, &iter);
	dbus_message_iter_get_basic(&iter, &num);
	eag_log_info("eag_hansi_state_init, get hansi num %d", num);
	for (i = 0; i < num; i++)
	{
		memset(&params, 0, sizeof(params));
		
		dbus_message_iter_next(&iter);
		dbus_message_iter_get_basic(&iter, &(params.vrid));
		
		dbus_message_iter_next(&iter);
		dbus_message_iter_get_basic(&iter, &(params.state));
		
		eag_log_info("...NO.%d vrid %d state %s",
				i, params.vrid, hansi_state2str(params.state));

		dbus_message_iter_next(&iter);	
		dbus_message_iter_get_basic(&iter,&(params.uplink_cnt));
		
		eag_log_info("...uplink num %d", params.uplink_cnt);

		dbus_message_iter_next(&iter);	
		dbus_message_iter_recurse(&iter, &iter_array);
		for (j = 0; j < params.uplink_cnt; j++)
		{
			DBusMessageIter iter_struct;
			if(j >= MAX_PHA_IF_NUM)
			{
				eag_log_err("params.uplink_cnt=%u > "
					"MAX_PHA_IF_NUM=%d, ignore other interfaces",
						 params.uplink_cnt, MAX_PHA_IF_NUM);
				break;
			}
			dbus_message_iter_recurse(&iter_array,&iter_struct);
			dbus_message_iter_get_basic(&iter_struct,
						&(params.uplink_if[j].master_ip));

			dbus_message_iter_next(&iter_struct);
			dbus_message_iter_get_basic(&iter_struct,
						&(params.uplink_if[j].backp_ip));

			dbus_message_iter_next(&iter_struct);
			dbus_message_iter_get_basic(&iter_struct,
						&(params.uplink_if[j].virtual_ip));
		
			dbus_message_iter_next(&iter_struct);
			dbus_message_iter_get_basic(&iter_struct,
						&(interface_name));
			strncpy(params.uplink_if[j].if_name, interface_name, 
					sizeof(params.uplink_if[j].if_name)-1);
		
			dbus_message_iter_next(&iter_array); 
		
			eag_log_info("...uplink %d: master %s backup %s virtual %s ifname %s",
					j,
					ip2str(params.uplink_if[j].master_ip,
							master_ip_char, sizeof(master_ip_char)),
					ip2str(params.uplink_if[j].backp_ip,
							backup_ip_char, sizeof(backup_ip_char)),
					ip2str(params.uplink_if[j].virtual_ip,
							virtual_ip_char, sizeof(virtual_ip_char)),
					params.uplink_if[j].if_name);
		}

		dbus_message_iter_next(&iter);	
		dbus_message_iter_get_basic(&iter,&(params.downlink_cnt));
		
		eag_log_info("...downlink num %d", params.downlink_cnt);

		dbus_message_iter_next(&iter);
		dbus_message_iter_recurse(&iter, &iter_array1);
		for (j = 0; j < params.downlink_cnt; j++)
		{
			DBusMessageIter iter_struct1;
			if(j >= MAX_PHA_IF_NUM)
			{
				eag_log_err("params.downlink_cnt=%u > "
					"MAX_PHA_IF_NUM=%d, ignore other interfaces",
						 params.downlink_cnt, MAX_PHA_IF_NUM);
				break;
			}
			
			dbus_message_iter_recurse(&iter_array1, &iter_struct1);
			dbus_message_iter_get_basic(&iter_struct1,
						&(params.downlink_if[j].master_ip));

			dbus_message_iter_next(&iter_struct1);
			dbus_message_iter_get_basic(&iter_struct1,
						&(params.downlink_if[j].backp_ip));

			dbus_message_iter_next(&iter_struct1);
			dbus_message_iter_get_basic(&iter_struct1,
						&(params.downlink_if[j].virtual_ip));
		
			dbus_message_iter_next(&iter_struct1);
			dbus_message_iter_get_basic(&iter_struct1,
						&(interface_name));

			strncpy( params.downlink_if[j].if_name,
				interface_name, sizeof(params.downlink_if[j].if_name)-1);
		
			dbus_message_iter_next(&iter_array1);
		
			eag_log_info("...downlink %d: master %s backup %s virtual %s ifname %s",
					j,
					ip2str(params.downlink_if[j].master_ip,
							master_ip_char,sizeof(master_ip_char)),
					ip2str(params.downlink_if[j].backp_ip,
							backup_ip_char,sizeof(backup_ip_char)),
					ip2str(params.downlink_if[j].virtual_ip,
							virtual_ip_char,sizeof(virtual_ip_char)),
					params.downlink_if[j].if_name);
		}

		dbus_message_iter_next(&iter);	
		dbus_message_iter_get_basic(&iter, &(interface_name));
		strncpy(params.heartlink_if_name, interface_name,
					sizeof(params.heartlink_if_name)-1);
	
		dbus_message_iter_next(&iter);
		dbus_message_iter_get_basic(&iter, &(params.heartlink_local_ip));
	
		dbus_message_iter_next(&iter);
		dbus_message_iter_get_basic(&iter, &(params.heartlink_opposite_ip));

		eag_log_info("...heartlink: name %s local_ip %s opposite_ip %s",
					params.heartlink_if_name,
					ip2str(params.heartlink_local_ip,
							master_ip_char,sizeof(master_ip_char)),
					ip2str(params.heartlink_opposite_ip,
							backup_ip_char,sizeof(backup_ip_char)));

		dbus_message_iter_next(&iter);
		dbus_message_iter_get_basic(&iter,&(params.vgateway_cnt));
		
		eag_log_info("...vgateway num %d", params.vgateway_cnt);

		dbus_message_iter_next(&iter);	
		dbus_message_iter_recurse(&iter,&iter_array2);
		
		for (j = 0; j < params.vgateway_cnt; j++)
		{
			DBusMessageIter iter_struct2;
			if(j >= MAX_PHA_IF_NUM)
			{
				eag_log_err("params.vgateway_cnt=%u > "
					"MAX_PHA_IF_NUM=%d, ignore other interfaces",
						 params.vgateway_cnt, MAX_PHA_IF_NUM);
				break;
			}
			
			dbus_message_iter_recurse(&iter_array2,&iter_struct2);	
			dbus_message_iter_get_basic(&iter_struct2, &(interface_name));
		
			dbus_message_iter_next(&iter_struct2);
			dbus_message_iter_get_basic(&iter_struct2,
					&(params.vgateway_if[j].virtual_ip));
		
			strncpy( params.vgateway_if[j].if_name, interface_name,
					sizeof(params.vgateway_if[j].if_name)-1);
		
			dbus_message_iter_next(&iter_array2);
	
			eag_log_info("...vgateway %d: virtual %s ifname %s",
				j,
				ip2str(params.vgateway_if[j].virtual_ip,
							virtual_ip_char,sizeof(virtual_ip_char)),
				params.vgateway_if[j].if_name );
		}
		if (params.vrid == eaghansi->ins_id) {
			memcpy(&(eaghansi->params), &(params), sizeof(eaghansi->params));
			eaghansi->params.prev_state = VRRP_STATE_DISABLE;
			eag_log_info("eag_hansi_state_init get params from hansi %u",
					eaghansi->ins_id);
		}
	}

	dbus_message_unref(reply);
	eag_log_info("eag_hansi_state_init end");
	
    return EAG_RETURN_OK;
}

static int
notify_had_backup_finished_direct(eag_thread_t *thread)
{
	eag_hansi_t *eaghansi = NULL;

	if (NULL == thread) {
		eag_log_err("notify_had_backup_finished_direct input error");
		return -1;
	}

	eaghansi = eag_thread_get_arg(thread);
	if (NULL == eaghansi) {
		eag_log_err("notify_had_backup_finished_direct eaghansi null");
		return -1;
	}
	
	if (NULL != eaghansi->t_timeout) {
		eag_thread_cancel(eaghansi->t_timeout);
		eaghansi->t_timeout = NULL;
	}

	eag_log_info("eag server is not started, notify had backup finished direct");

	eag_hansi_notify_had_backup_finished(eaghansi);

	return 0;
}

static int
eag_hansi_state_change( eag_hansi_t *eaghansi, 
					struct eag_hansi_param *curparams ) 
{
	uint32_t prev_state = VRRP_STATE_DISABLE;

	prev_state = eaghansi->params.state;
	
	if( 0 == eaghansi->server_state ){
		memcpy( &(eaghansi->params), curparams, 
				sizeof(struct eag_hansi_param));
		eaghansi->params.prev_state = prev_state;
		eag_log_info("eag_hansi_state_change server not start!");
		if (VRRP_STATE_TRANSFER == curparams->state
			|| VRRP_STATE_BACK == curparams->state
			|| VRRP_STATE_MAST == curparams->state) {
			if (NULL != eaghansi->t_timeout) {
				eag_thread_cancel(eaghansi->t_timeout);
				eaghansi->t_timeout = NULL;
			}
			eaghansi->t_timeout =
				eag_thread_add_timer(eaghansi->master, 
					notify_had_backup_finished_direct,
					eaghansi, 1);
			if (NULL == eaghansi->t_timeout) {
				eag_log_err("eag_hansi_state_change thread_add_timer failed");
			}
		}
		return EAG_RETURN_OK;
	}
	if( eaghansi->params.state == curparams->state ){
		memcpy( &(eaghansi->params), curparams, 
				sizeof(struct eag_hansi_param));
		eaghansi->params.prev_state = prev_state;
		eag_log_warning("eag_hansi_state_change status not changed!");
		return EAG_RETURN_OK;
	}

	eag_log_info("eag_hansi->params.state is %u\n",eaghansi->params.state);
	switch(eaghansi->params.state){
	case VRRP_STATE_INIT:
		break;
	case VRRP_STATE_TRANSFER:
		eag_log_err("eag_hansi_state_change on transfer! "\
					"might data backup not complete!");
	case VRRP_STATE_BACK:
		eag_hansi_backup_stop(eaghansi);
		break;
	case VRRP_STATE_MAST:
		eag_hansi_master_stop(eaghansi);
		break;
	case VRRP_STATE_LEARN:
		break;
	case VRRP_STATE_NONE:
		break;
	case VRRP_STATE_DISABLE:
		break;
	default:
		eag_log_err("eag_hansi_state_change unknow vrrp state = %u",
					eaghansi->params.state);
		break;
	}

	memcpy( &(eaghansi->params), curparams, 
				sizeof(struct eag_hansi_param));
	eaghansi->params.prev_state = prev_state;

	switch(eaghansi->params.state){
	case VRRP_STATE_INIT:
		break;
	case VRRP_STATE_TRANSFER:
	case VRRP_STATE_BACK:
		eag_hansi_backup_start(eaghansi);
		break;
	case VRRP_STATE_MAST:
		eag_hansi_master_start(eaghansi);
		break;
	case VRRP_STATE_LEARN:
		break;
	case VRRP_STATE_NONE:
		break;
	case VRRP_STATE_DISABLE:
		break;
	default:
		eag_log_err("eag_hansi_state_change unknow vrrp state = %u",
					eaghansi->params.state);
		break;
	}	
	eag_hansi_on_status_change(eaghansi);
	return EAG_RETURN_OK;
}

DBusMessage *
eag_hansi_dbus_vrrp_state_change_func(
			    DBusConnection *conn, 
			    DBusMessage *msg, 
			    void *user_data )
{
	DBusMessage* reply;
	DBusMessageIter iter = {0};
	DBusMessageIter  iter_array, iter_array1, iter_array2;	
	DBusError		err;

	char master_ip_char[32]="";
	char backup_ip_char[32]="";
	char virtual_ip_char[32]="";
	int j = 0;
	char * interface_name;
	struct eag_hansi_param params;
	int ret = 0;
	
	eag_hansi_t *eaghansi = (eag_hansi_t *)user_data;

	dbus_error_init( &err );
	
/*	DBUS_TYPE_UINT32					// vrid
	DBUS_TYPE_UINT32					// state
	DBUS_TYPE_UINT32					// count of uplink interfaces
	Array of uplink
		DBUS_TYPE_UINT32_AS_STRING		// master uplnik ip address
		DBUS_TYPE_UINT32_AS_STRING		// back uplink ip address
		DBUS_TYPE_UINT32_AS_STRING		// virtual uplink ip address
		DBUS_TYPE_STRING_AS_STRING		// uplink interface name
	DBUS_TYPE_UINT32					// count of downlink interfaces
	Array of uplink
		DBUS_TYPE_UINT32_AS_STRING		// master downlnik ip address
		DBUS_TYPE_UINT32_AS_STRING		// back downlink ip address
		DBUS_TYPE_UINT32_AS_STRING		// virtual downlink ip address
		DBUS_TYPE_STRING_AS_STRING		// downlink interface name
	DBUS_TYPE_STRING					// heartbeat interface name
	DBUS_TYPE_UINT32					// heartbeat ip address
	DBUS_TYPE_UINT32					// opposite heartbeat ip address
*/
	memset( &params, 0, sizeof(params));

	dbus_message_iter_init(msg,&iter);
	dbus_message_iter_get_basic(&iter,&(params.vrid));
	
	dbus_message_iter_next(&iter);	
	dbus_message_iter_get_basic(&iter,&(params.state));
	
	dbus_message_iter_next(&iter);	
	dbus_message_iter_get_basic(&iter,&(params.uplink_cnt));
	
	dbus_message_iter_next(&iter);	
	dbus_message_iter_recurse(&iter,&iter_array);
	for (j = 0; j < params.uplink_cnt; j++)
	{
		DBusMessageIter iter_struct;
		if(j >= MAX_PHA_IF_NUM)
		{
			eag_log_err("error!params.downlink_cnt=%u > "\
						"MAX_PHA_IF_NUM=%d ingnor other if",
						 params.uplink_cnt, MAX_PHA_IF_NUM);
			break;
		}		
		dbus_message_iter_recurse(&iter_array,&iter_struct);	
		dbus_message_iter_get_basic(&iter_struct,&(params.uplink_if[j].master_ip));		

		dbus_message_iter_next(&iter_struct);
		dbus_message_iter_get_basic(&iter_struct,&(params.uplink_if[j].backp_ip));

		dbus_message_iter_next(&iter_struct);
		dbus_message_iter_get_basic(&iter_struct,&(params.uplink_if[j].virtual_ip));
		
		dbus_message_iter_next(&iter_struct);
		dbus_message_iter_get_basic(&iter_struct,&(interface_name));

		strncpy( params.uplink_if[j].if_name, interface_name, 
					sizeof(params.uplink_if[j].if_name)-1);
		
		dbus_message_iter_next(&iter_array); 
		
		eag_log_info("...uplink %d: master %s backup %s virtual %s ifname %s",
					j,
					ip2str(params.uplink_if[j].master_ip, 
							master_ip_char, sizeof(master_ip_char) ),
					ip2str(params.uplink_if[j].backp_ip, 
							backup_ip_char, sizeof(backup_ip_char) ),
					ip2str(params.uplink_if[j].virtual_ip, 
							virtual_ip_char, sizeof(virtual_ip_char) ),
					params.uplink_if[j].if_name );
	}

	dbus_message_iter_next(&iter);	
	dbus_message_iter_get_basic(&iter,&(params.downlink_cnt));
	
	dbus_message_iter_next(&iter);	
	dbus_message_iter_recurse(&iter,&iter_array1);
	for (j = 0; j < params.downlink_cnt; j++)
	{
		DBusMessageIter iter_struct1;
		if(j >= MAX_PHA_IF_NUM)
		{
			eag_log_err("error!params.downlink_cnt=%u > MAX_PHA_IF_NUM=%d ingnor other if",
					params.downlink_cnt, MAX_PHA_IF_NUM);
			break;
		}
		
		dbus_message_iter_recurse(&iter_array1,&iter_struct1);	
		dbus_message_iter_get_basic(&iter_struct1,&(params.downlink_if[j].master_ip));		

		dbus_message_iter_next(&iter_struct1);
		dbus_message_iter_get_basic(&iter_struct1,&(params.downlink_if[j].backp_ip));

		dbus_message_iter_next(&iter_struct1);
		dbus_message_iter_get_basic(&iter_struct1,&(params.downlink_if[j].virtual_ip));
		
		dbus_message_iter_next(&iter_struct1);
		dbus_message_iter_get_basic(&iter_struct1,&(interface_name));

		strncpy( params.downlink_if[j].if_name, 
				interface_name, sizeof(params.downlink_if[j].if_name)-1);
		
		dbus_message_iter_next(&iter_array1); 
		
		eag_log_info("...downlink %d: master %s backup %s virtual %s ifname %s",
					j,
					ip2str(params.downlink_if[j].master_ip,
							master_ip_char,sizeof(master_ip_char)),
					ip2str(params.downlink_if[j].backp_ip,
							backup_ip_char,sizeof(backup_ip_char)),
					ip2str(params.downlink_if[j].virtual_ip,
							virtual_ip_char,sizeof(virtual_ip_char)),
					params.downlink_if[j].if_name );
	}

	dbus_message_iter_next(&iter);	
	dbus_message_iter_get_basic(&iter,&(interface_name));
	strncpy( params.heartlink_if_name, interface_name, sizeof(params.heartlink_if_name)-1);
	
	dbus_message_iter_next(&iter);
	dbus_message_iter_get_basic(&iter,&(params.heartlink_local_ip));
	
	dbus_message_iter_next(&iter);
	dbus_message_iter_get_basic(&iter,&(params.heartlink_opposite_ip)); 

	eag_log_info("...heartlink: name %s local_ip %s opposite_ip %s",
					params.heartlink_if_name,
					ip2str(params.heartlink_local_ip,
							master_ip_char,sizeof(master_ip_char)),
					ip2str(params.heartlink_opposite_ip,
							backup_ip_char,sizeof(backup_ip_char)) );
	
	dbus_message_iter_next(&iter);	
	dbus_message_iter_get_basic(&iter,&(params.vgateway_cnt));
	
	dbus_message_iter_next(&iter);	
	dbus_message_iter_recurse(&iter,&iter_array2);
	
	for (j = 0; j < params.vgateway_cnt; j++)
	{
		if(j >= MAX_PHA_IF_NUM)
		{
			eag_log_err("error! params.vgateway_cnt=%u > MAX_PHA_IF_NUM=%d",
					params.vgateway_cnt, MAX_PHA_IF_NUM);
			break;
		}
		DBusMessageIter iter_struct2;
		dbus_message_iter_recurse(&iter_array2,&iter_struct2);	
		dbus_message_iter_get_basic(&iter_struct2,&(interface_name));
		
		dbus_message_iter_next(&iter_struct2);
		dbus_message_iter_get_basic(&iter_struct2,
					&(params.vgateway_if[j].virtual_ip));
		
		strncpy( params.vgateway_if[j].if_name, interface_name, 
					sizeof(params.vgateway_if[j].if_name)-1);
		
		dbus_message_iter_next(&iter_array2); 
	
		eag_log_info("...vgateway %d: virtual %s ifname %s",
				j,
				ip2str(params.vgateway_if[j].virtual_ip,
							virtual_ip_char,sizeof(virtual_ip_char)),
				params.vgateway_if[j].if_name );
	}

	if (params.vrid == eaghansi->ins_id) {
		eag_log_info("eag_hansi_dbus_vrrp_state_change_func prev"\
					" state=%u new state=%u",
					eaghansi->params.state,
					params.state );
		eag_hansi_state_change( eaghansi, &params );
		eag_hansi_forward_all_param(eaghansi);
	}
	else {
		eag_log_err("eag_hansi_dbus_vrrp_state_change_func get "\
					"vrrpid(%u) different to inst_id(%u)", 
					params.vrid, eaghansi->ins_id );
		ret = -1;
	}
	
	reply = dbus_message_new_method_return(msg);
	if(NULL==reply)
	{
		eag_log_err("portal HA dbus set state get reply message null error!\n");
		return reply;
	}

	dbus_message_iter_init_append (reply, &iter);
	dbus_message_iter_append_basic (&iter,DBUS_TYPE_UINT32,&(ret));

	return reply;
}

int 
eag_hansi_notify_had_backup_finished(eag_hansi_t *eaghansi )
{
	DBusMessage *query, *reply;
	DBusError err;
	DBusMessageIter	 iter;
	unsigned int ret;
	unsigned int flag = 0;
	char vrrp_dbus_name[64]="";
	char vrrp_obj_path[64]="";	

	if( NULL == eaghansi ){
		eag_log_err("eag_hansi_notify_had_backup_finished prarm eaghansi is NULL");
		return EAG_ERR_UNKNOWN;
	}
	
	eag_dbus_t *eagdbus = eaghansi->eagdbus;
	int vrid = eaghansi->ins_id;

	eag_log_info("eag_hansi_notify_had_backup_finished hansi %d", vrid);
	memset(vrrp_dbus_name, 0, sizeof(vrrp_dbus_name));
	memset(vrrp_obj_path, 0, sizeof(vrrp_obj_path));
	sprintf( vrrp_dbus_name, "%s%d", VRRP_DBUS_BUSNAME, vrid);
	sprintf( vrrp_obj_path, "%s%d", VRRP_DBUS_OBJPATH, vrid);	
	query = dbus_message_new_method_call(vrrp_dbus_name,vrrp_obj_path,\
						VRRP_DBUS_INTERFACE,VRRP_DBUS_METHOD_SET_PORTAL_TRANSFER_STATE);
	dbus_error_init(&err);
	dbus_message_append_args(query,
						 DBUS_TYPE_UINT32, &vrid,
						 DBUS_TYPE_UINT32, &flag,
						 DBUS_TYPE_INVALID);
	reply = dbus_connection_send_with_reply_and_block ( 
					eag_dbus_get_dbus_conn(eagdbus),query,-1, &err);	
	dbus_message_unref(query);
	if (NULL == reply) {
		if (dbus_error_is_set(&err)) {
			dbus_error_free(&err);
		}
		eag_log_err("notify had err!!!!!");
		return EAG_ERR_INPUT_PARAM_ERR;
	}
	dbus_message_iter_init(reply,&iter);
	dbus_message_iter_get_basic(&iter,&ret);
	dbus_message_unref(reply);
	eag_log_info("notify had sucess!vrid=%d",vrid);
	return EAG_RETURN_OK;
}

static int
eag_hansi_do_syn_time_data(void *cbp, void *data, struct timeval *cb_tv)
{
	int ret = 0;
	eag_hansi_t *eaghansi = cbp;
	
#if 0	
	ret = eag_time_set_time((struct timeval *)data);
	if (EAG_RETURN_OK != ret) {
		eag_log_err("eag_hansi_do_syn_time_data error, ret=%d", ret);
	}
#endif
	if (NULL == eaghansi) {
		eag_log_err("eag_hansi_do_syn_time_data eaghansi is null!");
		return EAG_ERR_UNKNOWN;
	}
	if (NULL == eaghansi->proc) {
		eag_log_err("eag_hansi_do_syn_time_data proc is null!");
		return EAG_ERR_UNKNOWN;
	}
	if (NULL == eaghansi->proc->t_heartbeat) {
		eag_log_err("eag_hansi_do_syn_time_data t_heartbeat is NULL!");
		return EAG_ERR_UNKNOWN;
	}

	eag_thread_cancel(eaghansi->proc->t_heartbeat);
	eaghansi->proc->t_heartbeat = NULL;
	eaghansi->proc->t_heartbeat = eag_thread_add_timer(eaghansi->master,
						eag_hansi_heartbeat_timeout_on_backup,
						eaghansi->proc,EAG_TIME_SYN_BEAT_TIME*3);
	if (NULL == eaghansi->proc->t_heartbeat) {
		eag_log_err("eag_hansi_do_syn_time_data add t_heartbeat failed!");
		return EAG_ERR_UNKNOWN;
	}
	eag_hansi_queue_data(eaghansi, eaghansi->ack_time, &ret, sizeof(ret));
	
	eag_log_debug("eag_hansi", "eag_hansi_do_syn_time_data ok");
	return EAG_RETURN_OK;
}

static int
eag_hansi_do_ack_time_data(void *cbp, void *data, struct timeval *cb_tv)
{
	int ret = *((int *)data);
	
	if (EAG_RETURN_OK != ret) {
		eag_log_err("eag_hansi_do_ack_time_data error, ret=%d", ret);
	}
	eag_log_debug("eag_hansi", "eag_hansi_do_ack_time_data ret=%d", ret);

	return EAG_RETURN_OK;
}

static int
eag_hansi_master_accept(eag_thread_t *thread)
{
	eag_hansi_t *eaghansi = NULL;
	int conn_fd = -1;
	struct sockaddr_in addr;
	socklen_t addrlen = 0;
	uint32_t ip = 0;
	uint16_t port = 0;
	char ipstr[32] = "";
	char backup_ipstr[32] = "";

	if (NULL == thread) {
		eag_log_err("eag_hansi_master_accept input error");
		return EAG_ERR_INPUT_PARAM_ERR;
	}
    eaghansi = eag_thread_get_arg(thread);
	if (NULL == eaghansi) {
		eag_log_err("eag_hansi_master_accept eaghansi null");
		return EAG_ERR_INPUT_PARAM_ERR;
	}

	addrlen = sizeof(addr);
	conn_fd = accept(eaghansi->listen_fd, (struct sockaddr *)&addr, &addrlen);
	if (conn_fd < 0) {
		eag_log_err("eag_hansi_master_accept accept failed, fd(%d): %s",
				eaghansi->listen_fd, safe_strerror(errno));
		return EAG_ERR_UNKNOWN;
	}
	ip = ntohl(addr.sin_addr.s_addr);
	port = ntohs(addr.sin_port);
	ip2str(ip, ipstr, sizeof(ipstr));
	ip2str(eaghansi->params.heartlink_opposite_ip,
			backup_ipstr, sizeof(backup_ipstr));
	eag_log_info("eag_hansi_master_accept accept from %s:%u, opposite_ip:%s",
			ipstr, port, backup_ipstr);
	if (eaghansi->params.heartlink_opposite_ip != 0
		&& ip != eaghansi->params.heartlink_opposite_ip) {
		close(conn_fd);
		conn_fd = -1;
		eag_log_warning("eag_hansi_master_accept failed,"
			"accept from ip:%s is not opposite_ip:%s",
			ipstr, backup_ipstr);
		return -1;
	}
	
	if (0 != set_nonblocking(conn_fd)){
		eag_log_err("eag_hansi_master_accept set socket nonblocking failed");
		close(conn_fd);
		conn_fd = -1;
		return EAG_ERR_SOCKET_OPT_FAILED;
	}

	eag_hansi_proc_stop(eaghansi->proc);
	eaghansi->proc->conn_fd = conn_fd;
	
	/* master send all data to back */
	eag_log_info("eag_hansi_master_accept accept from backup");
	eag_hansi_on_master_accept(eaghansi);

	eag_hansi_proc_start_on_master(eaghansi->proc);

	eag_log_debug("eag_hansi","eag_hansi_master_accept ok");

	return EAG_RETURN_OK;
}

static int
eag_hansi_backup_connect_master(eag_hansi_t *eaghansi)
{
	eag_hansi_proc_t *proc = NULL;
	
	if (NULL == eaghansi) {
		eag_log_err("eag_hansi_backup_connect_master input null");
		return -1;
	}
	proc = eaghansi->proc;
	
	if (proc->conn_fd >= 0)
	{
		eag_log_warning("eag_hansi_backup_connect_master conn_fd >= 0");
		return 0;
	}
	
	proc->conn_fd = conn_to_server(eaghansi->params.heartlink_opposite_ip,
						eaghansi->backup_port);
	if (proc->conn_fd < 0) {
		eag_log_err("eag_hansi_backup_connect_master conn to server failed");
		eag_hansi_proc_stop(proc);
		return EAG_ERR_UNKNOWN;
	}
	
	eag_hansi_on_backup_connect(eaghansi);
	
	eag_hansi_proc_start_on_backup(proc);
	
	eag_log_info("eag_hansi_backup_connect_master ok");
	
	return EAG_RETURN_OK;
}

static int
eag_hansi_master_start(eag_hansi_t * eaghansi)
{
	if (NULL == eaghansi) {
		eag_log_err("eag_hansi_master_start input error");
		return -1;
	}
	if (eaghansi->listen_fd >= 0 ) {
		eag_log_err("eag_hansi_master_start master already start");
		return EAG_RETURN_OK;
	}

	if (NULL != eaghansi->t_accept) {
		eag_log_err("eag_hansi_master_start t_accept not null");
		eag_thread_cancel(eaghansi->t_accept);
		eaghansi->t_accept = NULL;
		/* return EAG_ERR_UNKNOWN; */
	}

	eaghansi->listen_fd = 
			create_tcp_server(eaghansi->params.heartlink_local_ip,
					eaghansi->backup_port);
	if (eaghansi->listen_fd < 0) {
		eag_log_err("eag_hansi_master_start create_tcp_server failed");
		return EAG_ERR_SOCKET_FAILED;
	}

	eaghansi->t_accept = eag_thread_add_read(eaghansi->master,
					eag_hansi_master_accept,
					eaghansi,
					eaghansi->listen_fd);
	if (NULL == eaghansi->t_accept) {
		eag_log_err("eag_hansi_master_start thread_add_read failed");
		close(eaghansi->listen_fd);
		eaghansi->listen_fd = -1;
		return EAG_ERR_THREAD_CREATE_FAILED;
	}

	eag_log_info("eag_hansi_master_start ok");
	return EAG_RETURN_OK;
}

static int
eag_hansi_backup_start(eag_hansi_t * eaghansi)
{
	time_t timenow = 0;
	struct timeval tv;
	
	if (NULL == eaghansi) {
		eag_log_err("eag_hansi_backup_start input null");
		return -1;
	}

	eag_time_gettimeofday(&tv, NULL);
	timenow = tv.tv_sec;
	
	eag_hansi_backup_connect_master(eaghansi);
	eaghansi->last_check_connect_time = timenow;
	
	eag_log_info("eag_hansi_backup_start ok");
	
	return EAG_RETURN_OK;
}

static int
eag_hansi_master_stop(eag_hansi_t *eaghansi)
{
	if (NULL == eaghansi) {
		eag_log_err("eag_hansi_master_stop input error");
		return -1;
	}
	
	if (NULL != eaghansi->t_accept) {
		eag_thread_cancel(eaghansi->t_accept);
		eaghansi->t_accept = NULL;
	}

	if (eaghansi->listen_fd >= 0) {
		close(eaghansi->listen_fd);
		eaghansi->listen_fd = -1;
	}
	
	eag_hansi_proc_stop(eaghansi->proc);

	eag_log_info("eag_hansi_master_stop ok");
	
	return EAG_RETURN_OK;
}

static int
eag_hansi_backup_stop(eag_hansi_t *eaghansi)
{
	if (NULL == eaghansi) {
		eag_log_err("eag_hansi_backup_stop input error");
		return EAG_ERR_UNKNOWN;
	}

	eag_hansi_proc_stop(eaghansi->proc);

	eag_log_info("eag_hansi_backup_stop ok");

	return EAG_RETURN_OK;
}

eag_hansi_t *
eag_hansi_new(eag_dbus_t *eagdbus,
						int ins_id,
						int hansi_type,
						eag_thread_master_t *master)
{
	eag_hansi_t *eaghansi = NULL;

	if (NULL == eagdbus || NULL == master) {
		eag_log_err("eag_hansi_new input error");
		return NULL;
    }

	eaghansi = eag_malloc(sizeof(eag_hansi_t));
	if (NULL == eaghansi) {
        eag_log_err("eag_hansi_new malloc failed");
        return NULL;
    }

	memset(eaghansi, 0, sizeof(eag_hansi_t));
	eaghansi->listen_fd = -1;
	eaghansi->ins_id = ins_id;
	eaghansi->hansi_type = hansi_type;
	eaghansi->eagdbus = eagdbus;
	eaghansi->master = master;
	eaghansi->proc = eag_hansi_proc_new(eaghansi);
	if (NULL == eaghansi->proc) {
		eag_log_err("eag_hansi_new eag_hansi_proc_new failed");
		eag_free(eaghansi);
		return NULL;
	}
	INIT_LIST_HEAD(&(eaghansi->bktype_head));
	eag_hansi_state_init(eaghansi);

	eaghansi->syn_time = eag_hansi_register_backup_type(eaghansi,
									BACKUP_TYPE_SYN_TIME,
									eaghansi,
									eag_hansi_do_syn_time_data);
	eaghansi->ack_time = eag_hansi_register_backup_type(eaghansi,
									BACKUP_TYPE_ACK_TIME,
									eaghansi,
									eag_hansi_do_ack_time_data);
	eag_log_info("eag_hansi_new ok");
	return eaghansi;
}

int
eag_hansi_free(eag_hansi_t *eaghansi)
{
	backup_type_t *bktype = NULL;
	backup_type_t *next = NULL;
	
	if (NULL == eaghansi) {
		eag_log_err("eag_hansi_free input error");
		return EAG_ERR_INPUT_PARAM_ERR;
	}

	list_for_each_entry_safe(bktype, next, &(eaghansi->bktype_head), node) {
			list_del(&(bktype->node));
			eag_free(bktype);
	}
	if (NULL != eaghansi->t_accept) {
		eag_thread_cancel(eaghansi->t_accept);
		eaghansi->t_accept = NULL;
	}
	if (eaghansi->listen_fd >= 0) {
		close(eaghansi->listen_fd);
		eaghansi->listen_fd = -1;
	}
	
	if (NULL != eaghansi->proc) {
		eag_hansi_proc_free(eaghansi->proc);
	}
	eag_free(eaghansi);

	eag_log_info("eag_hansi_free ok");
	return EAG_RETURN_OK;
}

int
eag_hansi_start(eag_hansi_t *eaghansi)
{
	int ret = 0;
	
	if (NULL == eaghansi) {
		eag_log_err("eag_hansi_start input error");
		return EAG_ERR_INPUT_PARAM_ERR;
	}
	if (1 == eaghansi->server_state) {
		eag_log_info("eag_hansi_start eaghansi already start");
		return EAG_RETURN_OK;
	}

	if (VRRP_STATE_TRANSFER == eaghansi->params.state
		|| VRRP_STATE_BACK == eaghansi->params.state) {
		ret = eag_hansi_backup_start(eaghansi);
		if (EAG_RETURN_OK != ret) {
			eag_log_err("eag_hansi_start eag_hansi_backup_start failed");
			return ret;
		}
	}
	else if (VRRP_STATE_MAST == eaghansi->params.state) {
		ret = eag_hansi_master_start(eaghansi);
		if (EAG_RETURN_OK != ret) {
			eag_log_err("eag_hansi_start eag_hansi_master_start failed");
			return ret;
		}
	}
	else {
		eag_log_info("eag_hansi_start, hansi state=%d", 
					eaghansi->params.state);
	}

	eag_log_info("eag_hansi_start ok");
	
	eaghansi->server_state = 1;
	
	return EAG_RETURN_OK;
}

int
eag_hansi_stop(eag_hansi_t *eaghansi)
{
	int ret = 0;
	
	if (NULL == eaghansi) {
		eag_log_err("eag_hansi_stop input error");
		return EAG_ERR_INPUT_PARAM_ERR;
	}
	if (0 == eaghansi->server_state) {
		eag_log_info("eag_hansi_stop error eaghansi already stop");
		return EAG_RETURN_OK;
	}

	if (VRRP_STATE_TRANSFER == eaghansi->params.state
		|| VRRP_STATE_BACK == eaghansi->params.state) {
		ret = eag_hansi_backup_stop(eaghansi);
		if (EAG_RETURN_OK != ret) {
			eag_log_err("eag_hansi_stop eag_hansi_backup_stop failed");
			return ret;
		}
	}
	else if (VRRP_STATE_MAST == eaghansi->params.state) {
		ret = eag_hansi_master_stop(eaghansi);
		if (EAG_RETURN_OK != ret) {
			eag_log_err("eag_hansi_stop eag_hansi_master_stop failed");
			return ret;
		}
	}
	else {
		eag_log_info("eag_hansi_stop, hansi state=%d", 
					eaghansi->params.state);
	}

	eag_log_info("eag_hansi_stop ok");
	eaghansi->server_state = 0;
	
	return EAG_RETURN_OK;
}

int
eag_hansi_set_backup_port(eag_hansi_t *eaghansi,
								uint16_t port)
{
	if (NULL == eaghansi) {
		eag_log_err("eag_hansi_set_backup_port input error");
		return EAG_ERR_NULL_POINTER;
	}

	eaghansi->backup_port = port;
	
	return EAG_RETURN_OK;
}

int
get_virtual_ip_by_down_interface(eag_hansi_t *eaghansi, char *intf, uint32_t *virtual_ip)
{
	int i = 0;
	
	if( NULL == eaghansi || NULL == intf || NULL == virtual_ip)
	{
		eag_log_err("get_virtual_ip_by_down_interface input error!");
		return EAG_ERR_UNKNOWN;
	}

	for( i = 0; i < eaghansi->params.downlink_cnt; i++)
	{
		if( 0 == strcmp(eaghansi->params.downlink_if[i].if_name, intf) )
		{
			*virtual_ip = eaghansi->params.downlink_if[i].virtual_ip;
			eag_log_debug("eag_hansi","get_appconn_virtual_ip by downlink interface = %s,virtual_ip=%#X",intf,*virtual_ip);
			return EAG_RETURN_OK;
		}
	}

	for( i = 0; i < eaghansi->params.vgateway_cnt; i++)
	{
		if( 0 == strcmp(eaghansi->params.vgateway_if[i].if_name, intf) )
		{
			*virtual_ip = eaghansi->params.vgateway_if[i].virtual_ip;
			eag_log_debug("eag_hansi","get_appconn_virtual_ip by vgateway interface = %s,virtual_ip=%#X",intf,*virtual_ip);
			return EAG_RETURN_OK;
		}
	}
	
	
	eag_log_err("get_virtual_ip_by_down_interface cant find appconn interface in pha,"\
				"user if = %s",intf);
	return EAG_ERR_UNKNOWN;
}

int
set_down_interface_by_virtual_ip (eag_hansi_t *eaghansi,uint32_t virtual_ip,char * intf)
{
	int i = 0;
	
	if( NULL == eaghansi || NULL == intf )
	{
		eag_log_err("set_down_interface_by_virtual_ip input error!");
		return EAG_ERR_UNKNOWN;
	}
	
	for( i = 0; i < eaghansi->params.downlink_cnt; i++)
	{
		if( virtual_ip == eaghansi->params.downlink_if[i].virtual_ip)
		{
			strncpy(intf, eaghansi->params.downlink_if[i].if_name,
					MAX_IF_NAME_LEN-1 );
			eag_log_debug("eag_hansi","set_appconn_interface_by downlink virtual_ip = %s,virtual_ip = %#X",intf,virtual_ip);
			return EAG_RETURN_OK;
		}		
	}
	
	for( i = 0; i < eaghansi->params.vgateway_cnt; i++)
	{
		if( virtual_ip == eaghansi->params.vgateway_if[i].virtual_ip)
		{
			strncpy(intf, eaghansi->params.vgateway_if[i].if_name,
					MAX_IF_NAME_LEN-1 );
			eag_log_debug("eag_hansi","set_appconn_interface_by vgateway down_if = %s,virtual_ip = %#X",intf,virtual_ip);
			return EAG_RETURN_OK;
		}		
	}

	
	eag_log_err("error! cant find appconn interface in eaghansi=%p,virtual_ip=%d,intf=%s\n",eaghansi, virtual_ip,intf);
	return EAG_ERR_UNKNOWN;
}

int
eag_hansi_check_connect_state(eag_hansi_t *eaghansi)
{
	time_t timenow = 0;
	struct timeval tv;

	if (NULL == eaghansi) {
		eag_log_err("eag_hansi_check_connect_state input error");
		return -1;
	}

	eag_time_gettimeofday(&tv, NULL);
	timenow = tv.tv_sec;

	if (timenow - eaghansi->last_check_connect_time > CHECK_HANSI_CONNECT_INTERVAL
			|| timenow < eaghansi->last_check_connect_time) {
		if (eag_hansi_is_backup(eaghansi) 
			&& 1 ==	eaghansi->server_state
			&& eaghansi->proc->conn_fd < 0) {
			eag_log_info("eag_hansi_check_connect_state backup connect close, need reconnect");
			eag_hansi_backup_connect_master(eaghansi);
		}
		eaghansi->last_check_connect_time = timenow;
	}
	
	return 0;
}

int
eag_hansi_is_connected(eag_hansi_t *eaghansi)
{
	if (NULL == eaghansi) {
		eag_log_err("eag_hansi_is_connected input error");
		return 0;
	}

	if (NULL == eaghansi->proc) {
		eag_log_err("eag_hansi_is_connected proc is null");
		return 0;
	}
	
	return eaghansi->proc->conn_fd >= 0;
}

int
eag_hansi_syn_time(eag_hansi_t *eaghansi)
{
	struct timeval tv = {0};

	if (eag_hansi_is_master(eaghansi)
		&& eag_hansi_is_connected(eaghansi)) {
		eag_time_gettimeofday(&tv, NULL);
		eag_hansi_jump_data(eaghansi, eaghansi->syn_time, &tv, sizeof(tv));
	}

	return 0;
}

