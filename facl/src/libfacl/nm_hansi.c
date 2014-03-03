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
* nm_hansi.c
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
#include "nm_errcode.h"
#include "nm_mem.h"
#include "nm_util.h"
#include "nm_blkmem.h"
#include "nm_log.h"
#include "nm_dbus.h"
#include "nm_thread.h"
#include "nm_time.h"
#include "nm_hashtable.h"
#include "nm_hansi.h"
#include "nm_limits.h"
#include "had_vrrpd.h"
#include "dbus/npd/npd_dbus_def.h"


#define BACKUP_PACKET_SIZE		4096

#define MAX_BACKUP_TYPE_NAME_LEN		32
#define CHECK_HANSI_CONNECT_INTERVAL	10

#define NM_HANSI_TASK_BLKMEM_NAME 			"hansi_task_blkmem"
#define NM_HANSI_TASK_BLKMEM_ITEMNUM		128
#define NM_HANSI_TASK_BLKMEM_MAXNUM		128

#define BACKUP_TYPE_SYN_TIME	"bktype_syn_time"
#define BACKUP_TYPE_ACK_TIME	"bktype_ack_time"

#define VRRP_DBUS_BUSNAME			"aw.vrrpcli"
#define VRRP_DBUS_OBJPATH 			"/aw/vrrp"
#define VRRP_DBUS_INTERFACE 		"aw.vrrp"
#define VRRP_DBUS_METHOD_SET_PORTAL_TRANSFER_STATE          "vrrp_set_portal_transfer_state"

typedef struct nm_hansi_proc nm_hansi_proc_t;
typedef struct nm_hansi_task nm_hansi_task_t;

typedef enum {
	HANSI_REMOTE = 0,
	HANSI_LOCAL
} HANSI_TYPE;


struct backup_packet {
	struct timeval tv;
    uint32_t type;
	uint32_t pktlen;
	uint8_t data[BACKUP_PACKET_SIZE];
};

#define BACKUP_PACKET_HEADSIZE	offsetof(struct backup_packet ,data)

struct nm_hansi_task {
	struct list_head node;
	char type_name[MAX_BACKUP_TYPE_NAME_LEN];
	struct backup_packet packet;
	nm_hansi_proc_t *proc;
	nm_thread_t *t_write;
	uint32_t writen;
};

struct nm_hansi_backup_type {
	struct list_head node;
	uint32_t type;/*auto increase on register*/
	char type_name[MAX_BACKUP_TYPE_NAME_LEN];
	void *cbp;
	ON_GET_BACKUP_DATA get_backup_data;
};

struct nm_hansi_intf {
	char if_name[MAX_IF_NAME_LEN];
	uint32_t virtual_ip;
	uint32_t backp_ip;
	uint32_t master_ip;
};

#define MAX_PHA_IF_NUM		8

struct nm_hansi_param
{
	uint32_t vrid;
	uint32_t prev_state;
	uint32_t state;
	uint32_t uplink_cnt;
	struct nm_hansi_intf uplink_if[MAX_PHA_IF_NUM];
	uint32_t downlink_cnt;
	struct nm_hansi_intf downlink_if[MAX_PHA_IF_NUM];
	uint32_t vgateway_cnt;
	struct nm_hansi_intf vgateway_if[MAX_PHA_IF_NUM];
	char heartlink_if_name[MAX_IF_NAME_LEN];
	uint32_t heartlink_local_ip;
	uint32_t heartlink_opposite_ip;
};

struct nm_hansi_proc {
	int conn_fd;
	nm_hansi_t *nmhansi;
	nm_thread_master_t *master;
	nm_thread_t *t_read;
	nm_thread_t *t_heartbeat; /* for heart beat */
	struct backup_packet readbuf;
	uint32_t readlen;
	
	struct list_head task_fifo;
	nm_blk_mem_t *task_blkmem;
};

#define MAX_FORWARD_NUM		8
struct nm_hansi_param_forward {
	char objpath[MAX_DBUS_OBJPATH_LEN];
	char name[MAX_DBUS_BUSNAME_LEN];
	char interfacex[MAX_DBUS_INTERFACE_LEN];
	char method[MAX_DBUS_METHOD_LEN];
};

struct nm_hansi {
	int listen_fd;
	uint32_t ins_id;
	HANSI_TYPE hansi_type;
	int server_state;
	/* int state;  */
	time_t last_check_connect_time;
	uint16_t backup_port;
	struct nm_hansi_param params;
	
	struct list_head bktype_head;
	uint32_t bktype_num;

	nm_dbus_t *nmdbus;
	nm_thread_master_t *master;
	nm_thread_t *t_accept;
	nm_thread_t *t_timeout;

	NM_HANSI_ON_MASTER_ACCEPT on_master_accept;
	void *on_master_accept_param;
	
	NM_HANSI_ON_BACKUP_CONNECT on_backup_connect;
	void *on_backup_connect_param;
	
	NM_HANSI_ON_BACKUP_HEARTBEAT_TIMEOUT on_backup_heartbeat_timeout;
	void *on_backup_heartbeat_timeout_param;
	
	NM_HANSI_ON_STATUS_CHANGE on_status_change;
	void *on_status_change_param;
	
	nm_hansi_proc_t *proc;

	struct nm_hansi_param_forward forward[MAX_FORWARD_NUM];
	int forward_num;

	backup_type_t *syn_time;
	backup_type_t *ack_time;
};

static int 
nm_hansi_proc_stop(nm_hansi_proc_t *proc);
static int
nm_hansi_task_start_write(nm_hansi_task_t *task);
static int
nm_hansi_master_start(nm_hansi_t * nmhansi);
static int
nm_hansi_backup_start(nm_hansi_t * nmhansi);
static int
nm_hansi_master_stop(nm_hansi_t *nmhansi);
static int
nm_hansi_backup_stop(nm_hansi_t *nmhansi);

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

static nm_hansi_task_t *
nm_hansi_task_new(nm_hansi_proc_t *proc,
						backup_type_t *bktype,
						void *data,
						uint32_t datalen)
{
	nm_hansi_task_t *task = NULL;

	if (NULL == proc || NULL == bktype
		|| NULL == data || 0 == datalen) {
		nm_log_err("nm_hansi_task_new input error");
		return NULL;
	}

	task = nm_blkmem_malloc_item(proc->task_blkmem);
	if (NULL == task) {
		nm_log_err("nm_hansi_task_new blkmem_malloc_item failed");
		return NULL;
	}

	memset(task, 0, sizeof(nm_hansi_task_t));
	strncpy(task->type_name, bktype->type_name, sizeof(task->type_name)-1);
	task->packet.type = htonl(bktype->type);
	task->packet.pktlen = htonl(datalen+BACKUP_PACKET_HEADSIZE);
	memcpy(task->packet.data, data, datalen);
	task->proc = proc;

	nm_log_debug("nm_hansi", "nm_hansi_task new ok");
	return task;
}

static int
nm_hansi_task_free(nm_hansi_task_t *task)
{
	nm_hansi_proc_t *proc = NULL;

	if (NULL == task) {
		nm_log_err("nm_hansi_task_free input error");
		return -1;
	}
	if (NULL != task->t_write) {
		nm_thread_cancel(task->t_write);
		task->t_write = NULL;
	}
	
	proc = task->proc;
	if (NULL == proc) {
		nm_log_err("nm_hansi_task_free proc null");
		return -1;
	}

	nm_blkmem_free_item(proc->task_blkmem, task);

	nm_log_debug("nm_hansi", "nm_hansi_task free ok");
	return 0;
}

static int
nm_hansi_fifo_push(nm_hansi_proc_t *proc,
						nm_hansi_task_t *task)
{
	if (NULL == proc || NULL == task) {
		nm_log_err("nm_hansi_fifo_push input error");
		return -1;
	}

	list_add_tail(&(task->node), &(proc->task_fifo));

	return 0;
}

static nm_hansi_task_t *
nm_hansi_fifo_pop(nm_hansi_proc_t *proc)
{
	nm_hansi_task_t *task = NULL;
	
	if (NULL == proc) {
		nm_log_err("nm_hansi_fifo_pop input error");
		return NULL;
	}

	if (list_empty(&(proc->task_fifo))) {
		return NULL;
	}
	task = list_entry(proc->task_fifo.next, nm_hansi_task_t, node);
	list_del(&(task->node));
	
	return task;
}

static int
nm_hansi_fifo_empty(nm_hansi_proc_t *proc)
{
	if (NULL == proc) {
		nm_log_err("nm_hansi_fifo_empty input error");
		return 1;
	}

	return list_empty(&(proc->task_fifo));
}

static nm_hansi_task_t *
nm_hansi_fifo_peek(nm_hansi_proc_t *proc)
{
	if (NULL == proc) {
		nm_log_err("nm_hansi_fifo_peek input error");
		return NULL;
	}

	if (list_empty(&(proc->task_fifo))) {
		return NULL;
	}
	return list_entry(proc->task_fifo.next, nm_hansi_task_t, node);
}

static int
nm_hansi_fifo_jump(nm_hansi_proc_t *proc,
						nm_hansi_task_t *task)
{
	if (NULL == proc || NULL == task) {
		nm_log_err("nm_hansi_fifo_jump input error");
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
nm_hansi_fifo_clear(nm_hansi_proc_t *proc)
{
	nm_hansi_task_t *task = NULL;
	nm_hansi_task_t *next = NULL;

	if (NULL == proc) {
		nm_log_err("nm_hansi_fifo_clear input error");
		return -1;
	}
	
	list_for_each_entry_safe(task, next, &(proc->task_fifo), node) {
		list_del(&(task->node));
		nm_hansi_task_free(task);
	}

	return 0;
}

static int
nm_hansi_task_write(nm_thread_t *thread)
{
	nm_hansi_task_t *task = NULL;
	nm_hansi_proc_t *proc = NULL;
	ssize_t writen = 0;
	struct timeval tv;

	if (NULL == thread) {
		nm_log_err("nm_hansi_task_write input error");
		return NM_ERR_INPUT_PARAM_ERR;
	}
	task = nm_thread_get_arg(thread);
	if (NULL == task) {
		nm_log_err("nm_hansi_task_write task is null");
		return NM_ERR_UNKNOWN;
	}
	proc = task->proc;
	
	/* time_proc */
	nm_time_gettimeofday(&tv, NULL);
	nm_log_debug("nm_hansi","nm_hansi_task_write type=%d, time=%lu",task->packet.type,tv.tv_sec);
	task->packet.tv.tv_sec = htonl(tv.tv_sec);
	task->packet.tv.tv_usec = htonl(tv.tv_usec);
	
	writen = write(proc->conn_fd, (uint8_t *)&(task->packet)+task->writen,
				ntohl(task->packet.pktlen)-task->writen);
	if (writen < 0) {
		nm_log_err("nm_hansi_task_write write failed, fd(%d), %s", 
					proc->conn_fd, safe_strerror(errno));
		if (EAGAIN != errno && EWOULDBLOCK != errno && EINTR != errno) {
			nm_hansi_proc_stop(proc);
			return -1;
		}
	}
	else if (0 == writen) {
		nm_log_warning("nm_hansi_task_write writen = 0");
	}
	else {
		task->writen += writen;
		nm_log_debug("nm_hansi", "nm_hansi_task_write writen=%d", writen);
	}

	if (task->writen >= ntohl(task->packet.pktlen)) {
		nm_thread_cancel(task->t_write);
		task->t_write = NULL;
		nm_log_debug("nm_hansi", "nm_hansi_task_write, write one task ok");
		if (nm_hansi_is_backup(proc->nmhansi))
		{
			task = nm_hansi_fifo_pop(proc);
			if (NULL != task) {
				nm_hansi_task_free(task);
			}
			else {
				nm_log_err("nm_hansi_task_write no task in backup fifo");
			}
			task = nm_hansi_fifo_peek(proc);
			if (NULL != task) {
				nm_log_err("nm_hansi_task_write has more tasks in backup fifo");
				nm_hansi_task_start_write(task);
			}
		}
	}
	else {
		nm_log_debug("nm_hansi", "nm_hansi_task_write data not end, wait next write");
	}

	return NM_RETURN_OK;
}

static int
nm_hansi_task_start_write(nm_hansi_task_t *task)
{
	nm_hansi_proc_t *proc = NULL;
	
	if (NULL == task) {
		nm_log_err("nm_hansi_task_start_write input error");
		return -1;
	}
	proc = task->proc;
	if (NULL == proc) {
		nm_log_err("nm_hansi_task_start_write proc is null");
		return -1;
	}
	if (proc->conn_fd < 0) {
		nm_log_err("nm_hansi_task_start_write proc connfd invalid");
		return -1;
	}
	
	task->t_write = nm_thread_add_write(proc->master,
					nm_hansi_task_write,
					task,
					proc->conn_fd);
	if (NULL == task->t_write) {
		nm_log_err("nm_hansi_task_start_write thread_add_write failed");
		nm_hansi_proc_stop(proc);
		return NM_ERR_UNKNOWN;
	}

	nm_log_debug("nm_hansi", "nm_hansi_task_start_write ok");
	
	return NM_RETURN_OK;
}

backup_type_t *
nm_hansi_register_backup_type(nm_hansi_t *nmhansi,
								const char *type_name,
								void *cbp,
								ON_GET_BACKUP_DATA on_get_backup_data)
{
	static uint32_t type = 0;
	struct nm_hansi_backup_type *bktype = NULL;

	bktype = nm_malloc(sizeof(backup_type_t));
	if( NULL == bktype ){
		nm_log_err("nm_hansi_register_backup_type malloc failed");
		return NULL;
	}
	type++;
	memset(bktype, 0, sizeof(backup_type_t));
	bktype->type = type;
	strncpy(bktype->type_name, type_name, sizeof(bktype->type_name)-1);
	bktype->cbp = cbp;
	bktype->get_backup_data = on_get_backup_data;
	
	list_add_tail(&(bktype->node), &(nmhansi->bktype_head));

	nm_log_info("nm_hansi_register_backup_type id = %u : %s",type, type_name);
	return bktype;
}

static backup_type_t *
nm_hansi_get_backup_type_by_id(nm_hansi_t *nmhansi, 
											uint32_t type)
{
	backup_type_t *bktype = NULL;
	
	list_for_each_entry(bktype, &(nmhansi->bktype_head), node) {
		if (type == bktype->type) {
			nm_log_debug("nm_hansi","nm_hansi_get_backup_type type %d,"
						"type_name is %s", bktype->type, bktype->type_name);
			return bktype;
		}
	}

	nm_log_err("nm_hansi_get_backup_type type %d no in list", type);
	return NULL;
}

int
nm_hansi_queue_data(nm_hansi_t *nmhansi,
						backup_type_t *bktype,
						void *data,
						uint32_t datalen)
{
	backup_type_t *tmp_bktype = NULL;
	nm_hansi_task_t *task = NULL;
		
	if (NULL == nmhansi || NULL == bktype
		|| NULL == data || 0 == datalen)
	{
		nm_log_err("nm_hansi_queue_data input error");
		return NM_ERR_INPUT_PARAM_ERR;
	}

	if (datalen > sizeof(task->packet.data)) {
		nm_log_err("nm_hansi_queue_data datalen is over the limit");
		return NM_ERR_UNKNOWN;
	}

	if (NULL == nmhansi->proc) {
		nm_log_err("nm_hansi_queue_data proc is null");
		return -1;
	}
	if ( nmhansi->proc->conn_fd < 0) {
		nm_log_debug("nm_hansi", "nm_hansi_queue_data backup not connect");
		return 0;
	}

	tmp_bktype = nm_hansi_get_backup_type_by_id(nmhansi, bktype->type);
	if (NULL == tmp_bktype || tmp_bktype != bktype) {
		nm_log_err("nm_hansi_queue_data get_backup_type_by_id error");
		return NM_ERR_UNKNOWN;
	}

	task = nm_hansi_task_new(nmhansi->proc, bktype, data, datalen);
	if (NULL == task) {
		nm_log_err("nm_hansi_queue_data nm_hansi_task_new failed");
		return -1;
	}
	if (nm_hansi_fifo_empty(nmhansi->proc)) {
		nm_log_debug("nm_hansi", "nm_hansi_queue_data fifo empty, write now");
		nm_hansi_fifo_push(nmhansi->proc, task);
		nm_hansi_task_start_write(task);
	}
	else {
		nm_log_debug("nm_hansi", "nm_hansi_queue_data fifo not empty, only push");
		nm_hansi_fifo_push(nmhansi->proc, task);
	}
	
	return 0;
}

int
nm_hansi_jump_data(nm_hansi_t *nmhansi,
						backup_type_t *bktype,
						void *data,
						uint32_t datalen)
{
	backup_type_t *tmp_bktype = NULL;
	nm_hansi_task_t *task = NULL;
		
	if (NULL == nmhansi || NULL == bktype
		|| NULL == data || 0 == datalen)
	{
		nm_log_err("nm_hansi_jump_data input error");
		return NM_ERR_INPUT_PARAM_ERR;
	}

	if (datalen > sizeof(task->packet.data)) {
		nm_log_err("nm_hansi_jump_data datalen is over the limit");
		return NM_ERR_UNKNOWN;
	}

	if (NULL == nmhansi->proc) {
		nm_log_err("nm_hansi_jump_data proc is null");
		return -1;
	}
	if ( nmhansi->proc->conn_fd < 0) {
		nm_log_debug("nm_hansi", "nm_hansi_jump_data backup not connect");
		return 0;
	}

	tmp_bktype = nm_hansi_get_backup_type_by_id(nmhansi, bktype->type);
	if (NULL == tmp_bktype || tmp_bktype != bktype) {
		nm_log_err("nm_hansi_jump_data get_backup_type_by_id error");
		return NM_ERR_UNKNOWN;
	}

	task = nm_hansi_task_new(nmhansi->proc, bktype, data, datalen);
	if (NULL == task) {
		nm_log_err("nm_hansi_jump_data nm_hansi_task_new failed");
		return -1;
	}
	if (nm_hansi_fifo_empty(nmhansi->proc)) {
		nm_log_debug("nm_hansi", "nm_hansi_jump_data fifo empty, write now");
		nm_hansi_fifo_jump(nmhansi->proc, task);
		nm_hansi_task_start_write(task);
	}
	else {
		nm_log_debug("nm_hansi", "nm_hansi_jump_data fifo not empty, only jump");
		nm_hansi_fifo_jump(nmhansi->proc, task);
	}
	
	return 0;
}

int
nm_hansi_prev_is_enable(nm_hansi_t * nmhansi)
{
	if (NULL == nmhansi) {
		nm_log_err("nm_hansi_prev_is_enable input error");
		return 0;
	}

	if (VRRP_STATE_INIT == nmhansi->params.prev_state
		||  VRRP_STATE_BACK == nmhansi->params.prev_state 
		||  VRRP_STATE_MAST == nmhansi->params.prev_state 
		||  VRRP_STATE_LEARN == nmhansi->params.prev_state 
		||  VRRP_STATE_NONE == nmhansi->params.prev_state 
		||  VRRP_STATE_TRANSFER == nmhansi->params.prev_state) {
		return 1;
	}

	return 0;
}

int
nm_hansi_prev_is_master(nm_hansi_t * nmhansi)
{
    if (NULL == nmhansi) {
        nm_log_err("nm_hansi_prev_is_master input error");
        return 0;
    }
	
    return VRRP_STATE_MAST == nmhansi->params.prev_state;
}

int
nm_hansi_is_enable(nm_hansi_t * nmhansi)
{
	if( NULL == nmhansi ){
		nm_log_err("nm_hansi_is_enable input error");
		return 0;
	}

	if (VRRP_STATE_INIT == nmhansi->params.state 
		||  VRRP_STATE_BACK == nmhansi->params.state 
		||  VRRP_STATE_MAST == nmhansi->params.state 
		||  VRRP_STATE_LEARN == nmhansi->params.state 
		||  VRRP_STATE_NONE == nmhansi->params.state 
		||  VRRP_STATE_TRANSFER == nmhansi->params.state) {
		return 1;
	}

	return 0;
}

int
nm_hansi_is_master(nm_hansi_t * nmhansi)
{
    if (NULL == nmhansi) {
        nm_log_err("nm_hansi_is_master input error");
        return 0;
    }
	
    return VRRP_STATE_MAST == nmhansi->params.state;
}

int
nm_hansi_is_backup(nm_hansi_t * nmhansi)
{
    if (NULL == nmhansi){
        nm_log_err("nm_hansi_is_backup input error");
        return 0;
    }
	
    return (VRRP_STATE_BACK == nmhansi->params.state
		|| VRRP_STATE_TRANSFER == nmhansi->params.state);
}

static int
nm_hansi_do_ack_data(nm_hansi_proc_t *proc)
{
	nm_hansi_t *nmhansi = NULL;
	backup_type_t *bktype = NULL;
	nm_hansi_task_t *task = NULL;
	int ret = 0;

	if (NULL == proc) {
		nm_log_err("nm_hansi_do_ack_data input error");
		return -1;
	}
	nmhansi = proc->nmhansi;
	
	bktype = nm_hansi_get_backup_type_by_id(nmhansi, proc->readbuf.type);
	if (NULL == bktype) {
		nm_log_err("nm_hansi_do_ack_data get an unknown backup type %d",
				proc->readbuf.type);
		return -1;
	}
	nm_log_debug("nm_hansi", "nm_hansi_do_ack_data "
				"get backup type:%d, type_name:%s",
				bktype->type, bktype->type_name);
	if (NULL == bktype->get_backup_data) {
		nm_log_err("nm_hansi_do_ack_data get_backup_data is null");
		return -1;
	}

	ret = bktype->get_backup_data(bktype->cbp, proc->readbuf.data, (void *)0);
	if (NM_RETURN_OK != ret) {
		nm_log_err("nm_hansi_do_ack_data get_backup_data error");
	}
	
	task = nm_hansi_fifo_pop(proc);
	if (NULL != task) {
		nm_hansi_task_free(task);
	}
	else {
		nm_log_err("nm_hansi_do_ack_data has not task in fifo");
	}
	task = nm_hansi_fifo_peek(proc);
	if (NULL != task) {
		nm_log_debug("nm_hansi", "nm_hansi_do_ack_data have other tasks,"
			" continue write next task");
		nm_hansi_task_start_write(task);
	}
	else {
		nm_log_debug("nm_hansi", "nm_hansi_do_ack_data all tasks completed");
	}
	
	return 0;
}

static int
nm_hansi_do_syn_data(nm_hansi_proc_t *proc)
{
	nm_hansi_t *nmhansi = NULL;
	backup_type_t *bktype = NULL;
	int ret = 0;
	struct timeval *syn_tv;

	if (NULL == proc) {
		nm_log_err("nm_hansi_do_syn_data input error");
		return -1;
	}
	nmhansi = proc->nmhansi;
	
	bktype = nm_hansi_get_backup_type_by_id(nmhansi, proc->readbuf.type);
	if (NULL == bktype) {
		nm_log_err("nm_hansi_do_syn_data get an unknown backup type %d",
				proc->readbuf.type);
		return -1;
	}

	syn_tv = &(proc->readbuf.tv);	
	nm_log_debug("nm_hansi", "nm_hansi_do_syn_data "
				"get backup type:%d, type_name:%s,last_time=%lu",
				bktype->type, bktype->type_name, syn_tv->tv_sec);
	if (NULL == bktype->get_backup_data) {
		nm_log_err("nm_hansi_do_syn_data get_backup_data is null");
		return -1;
	}

	ret = bktype->get_backup_data(bktype->cbp, proc->readbuf.data, syn_tv);
	if (NM_RETURN_OK != ret) {
		nm_log_err("nm_hansi_do_syn_data get_backup_data error");
	}

	return 0;
}

static void
nm_hansi_on_master_accept(nm_hansi_t *nmhansi)
{
	int ret = 0;
	
	if (NULL == nmhansi) {
		nm_log_err("nm_hansi_on_master_accept input error");
		return;
	}
	
	if (NULL != nmhansi->on_master_accept) {
		ret = nmhansi->on_master_accept(nmhansi,
							nmhansi->on_master_accept_param);
		if (NM_RETURN_OK != ret) {
			nm_log_warning("nm_hansi_on_master_accept "
					"on_master_accept return %d", ret);
		}
	}
	else {
		nm_log_warning("nm_hansi_on_master_accept on_master_accept is null");
	}
}

static void
nm_hansi_on_backup_connect(nm_hansi_t *nmhansi)
{
	int ret = 0;

	if (NULL == nmhansi) {
		nm_log_err("nm_hansi_on_backup_connect input error");
		return;
	}

	if (NULL != nmhansi->on_backup_connect) {
		ret = nmhansi->on_backup_connect(nmhansi,
							nmhansi->on_backup_connect_param);
		if (NM_RETURN_OK != ret) {
			nm_log_warning("nm_hansi_on_backup_connect "
					"on_backup_connect return %d", ret);
		}
	}
	else {
		nm_log_warning("nm_hansi_on_backup_connect on_backup_connect is null");
	}
}

static void
nm_hansi_on_status_change(nm_hansi_t *nmhansi)
{
	int ret = 0;
		
	if (NULL == nmhansi) {
		nm_log_err("nm_hansi_on_status_change input error");
		return;
	}
		
	if (NULL != nmhansi->on_status_change) {
		ret = nmhansi->on_status_change(nmhansi,
							nmhansi->on_status_change_param);
		if (NM_RETURN_OK != ret) {
			nm_log_warning("nm_hansi_on_status_change "
					"on_status_change return %d", ret);
		}
	}
	else {
		nm_log_warning("nm_hansi_on_status_change on_status_change is null");
	}
}

static void
nm_hansi_on_backup_heartbeat_timeout( nm_hansi_t *nmhansi )
{
	int ret = 0;
		
	if (NULL == nmhansi) {
		nm_log_err("nm_hansi_on_backup_heartbeat_timeout input error");
		return;
	}
		
	if (NULL != nmhansi->on_backup_heartbeat_timeout) {
		ret = nmhansi->on_backup_heartbeat_timeout(nmhansi,
							nmhansi->on_backup_heartbeat_timeout_param);
		if (NM_RETURN_OK != ret) {
			nm_log_warning("nm_hansi_on_backup_heartbeat_timeout "
					"callback return %d", ret);
		}
	}
	else {
		nm_log_warning("nm_hansi_on_backup_heartbeat_timeout callback is null");
	}
}

static int
nm_hansi_read_on_master(nm_thread_t *thread)
{
	nm_hansi_proc_t *proc = NULL;
	ssize_t recvlen = 0;
	
	if (NULL == thread) {
		nm_log_err("nm_hansi_read_on_master input error");
		return -1;
	}
    proc = nm_thread_get_arg(thread);
	if (NULL == proc) {
		nm_log_err("nm_hansi_read_on_master proc null");
		return -1;
	}
	
	recvlen = recv(proc->conn_fd, (uint8_t *)&(proc->readbuf)+proc->readlen,
					sizeof(proc->readbuf)-proc->readlen, 0);
	if (recvlen < 0) {
		nm_log_warning("nm_hansi_read_on_master recv failed, fd(%d): %s",
				proc->conn_fd, safe_strerror(errno));
		if (EINTR != errno && EAGAIN != errno && EWOULDBLOCK != errno) {
			nm_hansi_proc_stop(proc);
			return -1;
		}
	}
	else if (0 == recvlen) { /* the peer shutdown */
		nm_log_warning("nm_hansi_read_on_master fd(%d), recvlen = 0",
				proc->conn_fd);
		nm_hansi_proc_stop(proc);
		return -1;
	}
	else {  /* recvlen > 0 */
		proc->readlen += recvlen;
	}
	nm_log_debug("nm_hansi", "nm_hansi_read_on_master readlen=%d",
				proc->readlen);
	if (proc->readlen >= BACKUP_PACKET_HEADSIZE
		&& proc->readlen >= ntohl(proc->readbuf.pktlen)) {
		proc->readbuf.pktlen = ntohl(proc->readbuf.pktlen);
		proc->readbuf.type = ntohl(proc->readbuf.type);
		if (proc->readbuf.pktlen < BACKUP_PACKET_HEADSIZE) {
			nm_log_warning("nm_hansi_read_on_master abnormal, "
				"packet length %d < packet head size %d",
				proc->readbuf.pktlen, BACKUP_PACKET_HEADSIZE);
		}

		nm_hansi_do_ack_data(proc);
		memset(&(proc->readbuf), 0, sizeof(proc->readbuf));
		proc->readlen = 0;
	}
	else {
		nm_log_debug("nm_hansi",
			"nm_hansi_read_on_master data not end, wait next recv");
	}
	
	return NM_RETURN_OK;
}

static int
nm_hansi_heartbeat_timeout_on_master(nm_thread_t *thread)
{
	nm_hansi_proc_t *proc = NULL;
	nm_hansi_t *nmhansi=NULL;
	
	if (NULL == thread) {
		nm_log_err("nm_hansi_heartbeat_on_master input error");
		return -1;
	}
	proc = nm_thread_get_arg(thread);
	if (NULL == proc) {
		nm_log_err("nm_hansi_heartbeat_on_master proc null");
		return -1;
	}
	nmhansi = proc->nmhansi;
	if (NULL == nmhansi) {
		nm_log_err("nm_hansi_heartbeat_on_master nmhansi null");
		return -1;
	}
	nm_thread_cancel(proc->t_heartbeat);
	proc->t_heartbeat = NULL;
	nm_hansi_syn_time(nmhansi);
	proc->t_heartbeat = nm_thread_add_timer(proc->master,
					nm_hansi_heartbeat_timeout_on_master,proc,NM_TIME_SYN_BEAT_TIME);
	
	return NM_RETURN_OK;
}


static int
nm_hansi_read_on_backup(nm_thread_t *thread)
{
	nm_hansi_proc_t *proc = NULL;
	ssize_t recvlen = 0;

	if (NULL == thread) {
		nm_log_err("nm_hansi_read_on_backup input error");
		return -1;
	}
	proc = nm_thread_get_arg(thread);
	if (NULL == proc) {
		nm_log_err("nm_hansi_read_on_backup proc null");
		return -1;
	}

	recvlen = recv(proc->conn_fd, (uint8_t *)&(proc->readbuf)+proc->readlen,
					sizeof(proc->readbuf)-proc->readlen, 0);
	if (recvlen < 0) {
		nm_log_warning("nm_hansi_read_on_backup recv failed, fd(%d): %s",
				proc->conn_fd, safe_strerror(errno));
		if (EINTR != errno && EAGAIN != errno && EWOULDBLOCK != errno) {
			nm_hansi_proc_stop(proc);
			return -1;
		}
	}
	else if (0 == recvlen) { /* the peer shutdown */
		nm_log_warning("nm_hansi_read_on_backup fd(%d), recvlen = 0",
				proc->conn_fd);
		nm_hansi_proc_stop(proc);
		return -1;
	}
	else {  /* recvlen > 0 */
		proc->readlen += recvlen;
	}
	nm_log_debug("nm_hansi", "nm_hansi_read_on_backup readlen=%d",
				proc->readlen);
	if (proc->readlen >= BACKUP_PACKET_HEADSIZE
		&& proc->readlen >= ntohl(proc->readbuf.pktlen)) {
		proc->readbuf.pktlen = ntohl(proc->readbuf.pktlen);
		proc->readbuf.type = ntohl(proc->readbuf.type);
		proc->readbuf.tv.tv_sec = ntohl(proc->readbuf.tv.tv_sec);
		proc->readbuf.tv.tv_usec = ntohl(proc->readbuf.tv.tv_usec);
		nm_log_debug("nm_hansi","nm_hansi_read_on_backup  type=%d,tv=%p",proc->readbuf.type,&(proc->readbuf.tv));
		if (proc->readbuf.pktlen < BACKUP_PACKET_HEADSIZE) {
			nm_log_warning("nm_hansi_read_on_backup abnormal, "
				"packet length %d < packet head size %d",
				proc->readbuf.pktlen, BACKUP_PACKET_HEADSIZE);
		}

		nm_hansi_do_syn_data(proc);
		memset(&(proc->readbuf), 0, sizeof(proc->readbuf));
		proc->readlen = 0;
	}
	else {
		nm_log_debug("nm_hansi",
			"nm_hansi_read_on_backup data not end, wait next recv");
	}

	return NM_RETURN_OK;
}

static int
nm_hansi_heartbeat_timeout_on_backup(nm_thread_t *thread)
{
	nm_hansi_proc_t *proc = NULL;
	nm_hansi_t *nmhansi=NULL;
	
	if (NULL == thread) {
		nm_log_err("nm_hansi_heartbeat_on_backup input error");
		return -1;
	}
	proc = nm_thread_get_arg(thread);
	if (NULL == proc) {
		nm_log_err("nm_hansi_heartbeat_on_backup proc null");
		return -1;
	}
	nmhansi = proc->nmhansi;
	if (NULL == nmhansi) {
		nm_log_err("nm_hansi_heartbeat_on_backup nmhansi null");
		return -1;
	}
	nm_thread_cancel( proc->t_heartbeat );
	proc->t_heartbeat = NULL;
	
	//usually will not be called unless the tcp connect has error!
	nm_hansi_backup_stop(nmhansi);
	nm_hansi_on_backup_heartbeat_timeout(nmhansi);
	nm_hansi_backup_start(nmhansi);

	return NM_RETURN_OK;
}


static nm_hansi_proc_t *
nm_hansi_proc_new(nm_hansi_t *nmhansi)
{
	nm_hansi_proc_t *proc = NULL;

	if (NULL == nmhansi) {
		nm_log_err("nm_hansi_proc_new input error");
		return NULL;
	}
	proc = nm_malloc(sizeof(nm_hansi_proc_t));
	if (NULL == proc) {
		nm_log_err("nm_hansi_proc_new malloc failed");
		return NULL;
	}
	
	memset(proc, 0, sizeof(nm_hansi_proc_t));
	if (NM_RETURN_OK != nm_blkmem_create(&(proc->task_blkmem),
							NM_HANSI_TASK_BLKMEM_NAME,
							sizeof(nm_hansi_task_t), 
							NM_HANSI_TASK_BLKMEM_ITEMNUM,
							NM_HANSI_TASK_BLKMEM_MAXNUM)) {
		nm_log_err("nm_hansi_proc_new nm_blkmem_create failed");
		nm_free(proc);
		return NULL;
	}

	proc->conn_fd = -1;
	proc->nmhansi = nmhansi;
	proc->master = nmhansi->master;
	INIT_LIST_HEAD(&(proc->task_fifo));

	nm_log_info("nm_hansi_proc_new ok");
	return proc;
}

static int
nm_hansi_proc_free(nm_hansi_proc_t *proc)
{
	if (NULL == proc) {
		nm_log_err("nm_hansi_proc_free input error");
		return NM_ERR_INPUT_PARAM_ERR;
	}

	if (NULL != proc->t_read) {
		nm_thread_cancel(proc->t_read);
		proc->t_read = NULL;
	}
	if (proc->conn_fd >= 0) {
		close(proc->conn_fd);
		proc->conn_fd = -1;
	}

	nm_hansi_fifo_clear(proc);
	
	if (NULL != proc->task_blkmem) {
		nm_blkmem_destroy(&(proc->task_blkmem));
	}
	nm_free(proc);

	nm_log_info("nm_hansi_proc_free ok");
	return NM_RETURN_OK;
}

static int 
nm_hansi_proc_stop(nm_hansi_proc_t *proc)
{
	if (NULL == proc) {
		nm_log_err("nm_hansi_proc_stop input error");
		return NM_ERR_INPUT_PARAM_ERR;
	}
	
	if (NULL != proc->t_read) {
		nm_thread_cancel(proc->t_read);
		proc->t_read = NULL;
	}
	if (proc->conn_fd >= 0) {
		close(proc->conn_fd);
		proc->conn_fd = -1;
	}
	
	if (NULL != proc->t_heartbeat) {
		nm_thread_cancel(proc->t_heartbeat);
		proc->t_heartbeat = NULL;
	}
	
	nm_hansi_fifo_clear(proc);
	
	memset(&(proc->readbuf), 0, sizeof(proc->readbuf));
	proc->readlen = 0;
	nm_log_info("nm_hansi_proc_stop ok");
	
	return NM_RETURN_OK;
}

static int 
nm_hansi_proc_start_on_master(nm_hansi_proc_t *proc)
{
	if (NULL == proc) {
		nm_log_err("nm_hansi_proc_start_on_master input null");
		return -1;
	}
	
	proc->t_read = nm_thread_add_read(proc->master,
						nm_hansi_read_on_master, proc, proc->conn_fd);
	if (NULL == proc->t_read) {
		nm_log_err("nm_hansi_proc_start_on_master thread_add_read failed");
		nm_hansi_proc_stop(proc);
		return NM_ERR_UNKNOWN;
	}
	proc->t_heartbeat = nm_thread_add_timer(proc->master,
						nm_hansi_heartbeat_timeout_on_master,
						proc,NM_TIME_SYN_BEAT_TIME);
	if (NULL == proc->t_heartbeat) {
		nm_log_err("nm_hansi_proc_start_on_master nm_thread_add_timer failed");
		nm_hansi_proc_stop(proc);
		return NM_ERR_UNKNOWN;
	}

	nm_log_info("nm_hansi_proc_start_on_master ok");

	return NM_RETURN_OK;
}

static int 
nm_hansi_proc_start_on_backup(nm_hansi_proc_t *proc)
{
	if (NULL == proc) {
		nm_log_err("nm_hansi_proc_start_on_backup input null");
		return -1;
	}
	
	proc->t_read = nm_thread_add_read(proc->master,
					nm_hansi_read_on_backup, proc, proc->conn_fd);
	if (NULL == proc->t_read) {
		nm_log_err("nm_hansi_proc_start_on_backup thread_add_read failed");
		nm_hansi_proc_stop(proc);
		return NM_ERR_UNKNOWN;
	}
	
	proc->t_heartbeat= nm_thread_add_timer(proc->master,
					nm_hansi_heartbeat_timeout_on_backup, 
					proc, NM_TIME_SYN_BEAT_TIME*3 );
	if (NULL == proc->t_heartbeat) {
		nm_log_err("nm_hansi_proc_start_on_backup nm_thread_add_timer failed");
		nm_hansi_proc_stop(proc);
		return NM_ERR_UNKNOWN;
	}
	nm_log_info("nm_hansi_proc_start_on_backup ok");

	return NM_RETURN_OK;
}

void
nm_hansi_set_on_master_accept_cb(nm_hansi_t *nmhansi,
											NM_HANSI_ON_MASTER_ACCEPT on_master_accept,
											void *param)
{
	if (NULL == nmhansi) {
		nm_log_err("nm_hansi_set_on_backup_connect_cb input error");
		return;
	}
	
	nmhansi->on_master_accept = on_master_accept;
	nmhansi->on_master_accept_param = param;
}

void
nm_hansi_set_on_backup_connect_cb(nm_hansi_t *nmhansi,
											NM_HANSI_ON_BACKUP_CONNECT on_backup_connect,
											void *param)
{
	if (NULL == nmhansi) {
		nm_log_err("nm_hansi_set_on_backup_connect_cb input error");
		return;
	}

	nmhansi->on_backup_connect = on_backup_connect;
	nmhansi->on_backup_connect_param = param;
}

void
nm_hansi_set_on_status_change_cb( nm_hansi_t *nmhansi,
											NM_HANSI_ON_STATUS_CHANGE on_status_change,
											void *param)
{
	if (NULL == nmhansi) {
		nm_log_err("nm_hansi_set_on_backup_connect_cb input error");
		return;
	}
		
	nmhansi->on_status_change = on_status_change;
	nmhansi->on_status_change_param = param;
}

void
nm_hansi_set_on_backup_heartbeat_timeout_cb( nm_hansi_t *nmhansi,
							NM_HANSI_ON_BACKUP_HEARTBEAT_TIMEOUT on_backup_heartbeat_timeout,
							void *param )
{
	if (NULL == nmhansi) {
		nm_log_err("nm_hansi_set_on_backup_connect_cb input error");
		return;
	}
		
	nmhansi->on_backup_heartbeat_timeout = on_backup_heartbeat_timeout;
	nmhansi->on_backup_heartbeat_timeout_param = param;
}

int
nm_hansi_add_hansi_param_forward( nm_hansi_t *nmhansi,
				char *objpath, char *name, char *interfacex, char *method)
{
	int num = 0;
	if( NULL == nmhansi || NULL == objpath 
		|| NULL == name || NULL == interfacex
		|| NULL==method
		|| strlen(objpath) == 0 || strlen(objpath)>=MAX_DBUS_OBJPATH_LEN
		|| strlen(name) == 0 || strlen(name)>=MAX_DBUS_BUSNAME_LEN
		|| strlen(interfacex) == 0 || strlen(interfacex)>=MAX_DBUS_INTERFACE_LEN
		|| strlen(method) == 0 || strlen(method)>=MAX_DBUS_METHOD_LEN )
	{
		nm_log_err("nm_hansi_add_hansi_param_forward input param error!");
		return NM_ERR_INPUT_PARAM_ERR;
	}

 	num = nmhansi->forward_num;
	if( num >= MAX_FORWARD_NUM ){
		nm_log_err("nm_hansi_add_hansi_param_forward num %d limite!",
						MAX_FORWARD_NUM);
		return NM_ERR_UNKNOWN;
	}

	strcpy( nmhansi->forward[num].objpath, objpath );
	strcpy( nmhansi->forward[num].name, name );
	strcpy( nmhansi->forward[num].interfacex, interfacex );
	strcpy( nmhansi->forward[num].method, method );
	nmhansi->forward_num++;
	return NM_RETURN_OK;
}

int
nm_hansi_del_hansi_param_forward( nm_hansi_t *nmhansi,
				char *objpath, char *name, char *interfacex, char *method )
{
	int num = 0;
	int i;
	if( NULL == nmhansi || NULL == objpath 
		|| NULL == name || NULL == interfacex
		|| NULL==method
		|| strlen(objpath) == 0 || strlen(objpath)>=MAX_DBUS_OBJPATH_LEN
		|| strlen(name) == 0 || strlen(name)>=MAX_DBUS_BUSNAME_LEN
		|| strlen(interfacex) == 0 || strlen(interfacex)>=MAX_DBUS_INTERFACE_LEN
		|| strlen(method) == 0 || strlen(method)>=MAX_DBUS_METHOD_LEN )
	{
		nm_log_err("nm_hansi_del_hansi_param_forward input param error!");
		return NM_ERR_INPUT_PARAM_ERR;
	}

	num = nmhansi->forward_num;
	if( num > MAX_FORWARD_NUM ){
		nm_log_err("nm_hansi_del_hansi_param_forward num %d limite!",
						MAX_FORWARD_NUM);
		return NM_ERR_UNKNOWN;
	}

	for( i=0;i<num;i++){
		if( 0 == strcmp( nmhansi->forward[i].objpath, objpath )
			&& 0 ==	strcmp( nmhansi->forward[i].name, name )
			&& 0 == strcmp( nmhansi->forward[i].interfacex, interfacex )
			&& 0 == strcmp( nmhansi->forward[i].method, method ))
		{
			break;
		}
	}

	if( i != num ){
		for( ;i<num-1;i++){
			strcpy( nmhansi->forward[i].objpath, 
						nmhansi->forward[i+1].objpath);
			strcpy( nmhansi->forward[i].name, 
						nmhansi->forward[i+1].name );
			strcpy( nmhansi->forward[i].interfacex, 
						nmhansi->forward[i+1].interfacex );
			strcpy( nmhansi->forward[i].method, 
						nmhansi->forward[i+1].method );
		}
		nmhansi->forward_num--;
		return NM_RETURN_OK;
	}else{
		nm_log_err("nm_hansi_add_hansi_param_forward not find!");
	}
	
	return NM_ERR_UNKNOWN;
}

static void
nm_hansi_forward_param( nm_hansi_t *nmhansi, 
						struct nm_hansi_param_forward *forward )
{
	if( NULL == nmhansi || NULL == forward ){
		return;
	}
	struct nm_hansi_param *params = NULL;
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

	params = &(nmhansi->params);
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
									DBUS_TYPE_UINT32, &(nmhansi->ins_id));
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
					nm_dbus_get_dbus_conn(nmhansi->nmdbus),query, 5000, &err);
	
	dbus_message_unref(query);
	if (NULL == reply) {
		nm_log_err("hansi param forward to %s time out!\n", 
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
			nm_log_info("hansi param forward to %s resoult %d!\n", 
							forward->name, op_ret);
		} 
		else {
			nm_log_err("hansi param forward to %s failed!\n", 
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
nm_hansi_forward_all_param(nm_hansi_t *nmhansi)
{
	int i = 0;
	struct nm_hansi_param_forward *forward = NULL;
		
	nm_log_info("nm_hansi_forward_all_param forward_num=%d\n",
					nmhansi->forward_num );
	for (i = 0; i < nmhansi->forward_num; i++) {
		forward = &(nmhansi->forward[i]);
		nm_log_info("before nm_hansi_forward_all_param to %s %s %s %s\n",
								forward->objpath,
								forward->name,
								forward->interfacex,
								forward->method );
		nm_hansi_forward_param( nmhansi, forward );
	}

	return;
}

static int
nm_hansi_state_init(nm_hansi_t * nmhansi) /*get params and status */
{
	DBusMessage *query = NULL;
	DBusMessage *reply = NULL;
	DBusMessageIter iter;
	DBusMessageIter	 iter_array;
	DBusMessageIter	 iter_array1;
	DBusMessageIter	 iter_array2;
	DBusError err;
	struct nm_hansi_param params = {0};
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
	
	nm_log_info("nm_hansi_state_init hansi-type:%d, hansi-id:%d",
		nmhansi->hansi_type, nmhansi->ins_id);
	
	memset(&params, 0, sizeof(params));
	memset(&(nmhansi->params), 0, sizeof((nmhansi->params)));
	nmhansi->params.vrid = nmhansi->ins_id;
	nmhansi->params.prev_state = VRRP_STATE_DISABLE;
	nmhansi->params.state = VRRP_STATE_DISABLE;

	snprintf(vrrp_dbus_name, sizeof(vrrp_dbus_name),
			"%s%d", VRRP_DBUS_BUSNAME, nmhansi->ins_id);
	snprintf(vrrp_obj_path, sizeof(vrrp_obj_path),
			"%s%d", VRRP_DBUS_OBJPATH, nmhansi->ins_id);
	query = dbus_message_new_method_call(vrrp_dbus_name, vrrp_obj_path,
						VRRP_DBUS_INTERFACE, VRRP_DBUS_METHOD_SET_PROTAL);
	if (NULL == query) {
		nm_log_err("nm_hansi_state_init new_method_call failed");
		return -1;
	}
	dbus_error_init(&err);
	dbus_message_append_args(query,
						DBUS_TYPE_UINT32, &get_hansi_state_flag,
						DBUS_TYPE_INVALID);
	reply = dbus_connection_send_with_reply_and_block(
					nm_dbus_get_dbus_conn(nmhansi->nmdbus), query, -1, &err);
	dbus_message_unref(query);
	if (NULL == reply) {
		if (dbus_error_is_set(&err)) {
			nm_log_err("nm_hansi_state_init dbus_connection_send %s :%s",
							err.name, err.message);
			dbus_error_free(&err);
		}
		nm_log_err("nm_hansi_state_init dbus_connection_send failed");
		return -1;
	}

	dbus_message_iter_init(reply, &iter);
	dbus_message_iter_get_basic(&iter, &num);
	nm_log_info("nm_hansi_state_init, get hansi num %d", num);
	for (i = 0; i < num; i++)
	{
		memset(&params, 0, sizeof(params));
		
		dbus_message_iter_next(&iter);
		dbus_message_iter_get_basic(&iter, &(params.vrid));
		
		dbus_message_iter_next(&iter);
		dbus_message_iter_get_basic(&iter, &(params.state));
		
		nm_log_info("...NO.%d vrid %d state %s",
				i, params.vrid, hansi_state2str(params.state));

		dbus_message_iter_next(&iter);	
		dbus_message_iter_get_basic(&iter,&(params.uplink_cnt));
		
		nm_log_info("...uplink num %d", params.uplink_cnt);

		dbus_message_iter_next(&iter);	
		dbus_message_iter_recurse(&iter, &iter_array);
		for (j = 0; j < params.uplink_cnt; j++)
		{
			DBusMessageIter iter_struct;
			if(j >= MAX_PHA_IF_NUM)
			{
				nm_log_err("params.uplink_cnt=%u > "
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
		
			nm_log_info("...uplink %d: master %s backup %s virtual %s ifname %s",
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
		
		nm_log_info("...downlink num %d", params.downlink_cnt);

		dbus_message_iter_next(&iter);
		dbus_message_iter_recurse(&iter, &iter_array1);
		for (j = 0; j < params.downlink_cnt; j++)
		{
			DBusMessageIter iter_struct1;
			if(j >= MAX_PHA_IF_NUM)
			{
				nm_log_err("params.downlink_cnt=%u > "
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
		
			nm_log_info("...downlink %d: master %s backup %s virtual %s ifname %s",
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

		nm_log_info("...heartlink: name %s local_ip %s opposite_ip %s",
					params.heartlink_if_name,
					ip2str(params.heartlink_local_ip,
							master_ip_char,sizeof(master_ip_char)),
					ip2str(params.heartlink_opposite_ip,
							backup_ip_char,sizeof(backup_ip_char)));

		dbus_message_iter_next(&iter);
		dbus_message_iter_get_basic(&iter,&(params.vgateway_cnt));
		
		nm_log_info("...vgateway num %d", params.vgateway_cnt);

		dbus_message_iter_next(&iter);	
		dbus_message_iter_recurse(&iter,&iter_array2);
		
		for (j = 0; j < params.vgateway_cnt; j++)
		{
			DBusMessageIter iter_struct2;
			if(j >= MAX_PHA_IF_NUM)
			{
				nm_log_err("params.vgateway_cnt=%u > "
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
	
			nm_log_info("...vgateway %d: virtual %s ifname %s",
				j,
				ip2str(params.vgateway_if[j].virtual_ip,
							virtual_ip_char,sizeof(virtual_ip_char)),
				params.vgateway_if[j].if_name );
		}
		if (params.vrid == nmhansi->ins_id) {
			memcpy(&(nmhansi->params), &(params), sizeof(nmhansi->params));
			nmhansi->params.prev_state = VRRP_STATE_DISABLE;
			nm_log_info("nm_hansi_state_init get params from hansi %u",
					nmhansi->ins_id);
		}
	}

	dbus_message_unref(reply);
	nm_log_info("nm_hansi_state_init end");
	
    return NM_RETURN_OK;
}

static int
notify_had_backup_finished_direct(nm_thread_t *thread)
{
	nm_hansi_t *nmhansi = NULL;

	if (NULL == thread) {
		nm_log_err("notify_had_backup_finished_direct input error");
		return -1;
	}

	nmhansi = nm_thread_get_arg(thread);
	if (NULL == nmhansi) {
		nm_log_err("notify_had_backup_finished_direct nmhansi null");
		return -1;
	}
	
	if (NULL != nmhansi->t_timeout) {
		nm_thread_cancel(nmhansi->t_timeout);
		nmhansi->t_timeout = NULL;
	}

	nm_log_info("nm server is not started, notify had backup finished direct");

	nm_hansi_notify_had_backup_finished(nmhansi);

	return 0;
}

static int
nm_hansi_state_change( nm_hansi_t *nmhansi, 
					struct nm_hansi_param *curparams ) 
{
	uint32_t prev_state = VRRP_STATE_DISABLE;

	prev_state = nmhansi->params.state;
	
	if( 0 == nmhansi->server_state ){
		memcpy( &(nmhansi->params), curparams, 
				sizeof(struct nm_hansi_param));
		nmhansi->params.prev_state = prev_state;
		nm_log_info("nm_hansi_state_change server not start!");
		if (VRRP_STATE_TRANSFER == curparams->state
			|| VRRP_STATE_BACK == curparams->state
			|| VRRP_STATE_MAST == curparams->state) {
			if (NULL != nmhansi->t_timeout) {
				nm_thread_cancel(nmhansi->t_timeout);
				nmhansi->t_timeout = NULL;
			}
			nmhansi->t_timeout =
				nm_thread_add_timer(nmhansi->master, 
					notify_had_backup_finished_direct,
					nmhansi, 1);
			if (NULL == nmhansi->t_timeout) {
				nm_log_err("nm_hansi_state_change thread_add_timer failed");
			}
		}
		return NM_RETURN_OK;
	}
	if( nmhansi->params.state == curparams->state ){
		memcpy( &(nmhansi->params), curparams, 
				sizeof(struct nm_hansi_param));
		nmhansi->params.prev_state = prev_state;
		nm_log_warning("nm_hansi_state_change status not changed!");
		return NM_RETURN_OK;
	}

	nm_log_info("nm_hansi->params.state is %u\n",nmhansi->params.state);
	switch(nmhansi->params.state){
	case VRRP_STATE_INIT:
		break;
	case VRRP_STATE_TRANSFER:
		nm_log_err("nm_hansi_state_change on transfer! "\
					"might data backup not complete!");
	case VRRP_STATE_BACK:
		nm_hansi_backup_stop(nmhansi);
		break;
	case VRRP_STATE_MAST:
		nm_hansi_master_stop(nmhansi);
		break;
	case VRRP_STATE_LEARN:
		break;
	case VRRP_STATE_NONE:
		break;
	case VRRP_STATE_DISABLE:
		break;
	default:
		nm_log_err("nm_hansi_state_change unknow vrrp state = %u",
					nmhansi->params.state);
		break;
	}

	memcpy( &(nmhansi->params), curparams, 
				sizeof(struct nm_hansi_param));
	nmhansi->params.prev_state = prev_state;

	switch(nmhansi->params.state){
	case VRRP_STATE_INIT:
		break;
	case VRRP_STATE_TRANSFER:
	case VRRP_STATE_BACK:
		nm_hansi_backup_start(nmhansi);
		break;
	case VRRP_STATE_MAST:
		nm_hansi_master_start(nmhansi);
		break;
	case VRRP_STATE_LEARN:
		break;
	case VRRP_STATE_NONE:
		break;
	case VRRP_STATE_DISABLE:
		break;
	default:
		nm_log_err("nm_hansi_state_change unknow vrrp state = %u",
					nmhansi->params.state);
		break;
	}	
	nm_hansi_on_status_change(nmhansi);
	return NM_RETURN_OK;
}

DBusMessage *
nm_hansi_dbus_vrrp_state_change_func(
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
	struct nm_hansi_param params;
	int ret = 0;
	
	nm_hansi_t *nmhansi = (nm_hansi_t *)user_data;

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
			nm_log_err("error!params.downlink_cnt=%u > "\
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
		
		nm_log_info("...uplink %d: master %s backup %s virtual %s ifname %s",
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
			nm_log_err("error!params.downlink_cnt=%u > MAX_PHA_IF_NUM=%d ingnor other if",
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
		
		nm_log_info("...downlink %d: master %s backup %s virtual %s ifname %s",
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

	nm_log_info("...heartlink: name %s local_ip %s opposite_ip %s",
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
			nm_log_err("error! params.vgateway_cnt=%u > MAX_PHA_IF_NUM=%d",
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
	
		nm_log_info("...vgateway %d: virtual %s ifname %s",
				j,
				ip2str(params.vgateway_if[j].virtual_ip,
							virtual_ip_char,sizeof(virtual_ip_char)),
				params.vgateway_if[j].if_name );
	}

	if (params.vrid == nmhansi->ins_id) {
		nm_log_info("nm_hansi_dbus_vrrp_state_change_func prev"\
					" state=%u new state=%u",
					nmhansi->params.state,
					params.state );
		nm_hansi_state_change( nmhansi, &params );
		nm_hansi_forward_all_param(nmhansi);
	}
	else {
		nm_log_err("nm_hansi_dbus_vrrp_state_change_func get "\
					"vrrpid(%u) different to inst_id(%u)", 
					params.vrid, nmhansi->ins_id );
		ret = -1;
	}
	
	reply = dbus_message_new_method_return(msg);
	if(NULL==reply)
	{
		nm_log_err("portal HA dbus set state get reply message null error!\n");
		return reply;
	}

	dbus_message_iter_init_append (reply, &iter);
	dbus_message_iter_append_basic (&iter,DBUS_TYPE_UINT32,&(ret));

	return reply;
}

int 
nm_hansi_notify_had_backup_finished(nm_hansi_t *nmhansi )
{
	DBusMessage *query, *reply;
	DBusError err;
	DBusMessageIter	 iter;
	unsigned int ret;
	unsigned int flag = 0;
	char vrrp_dbus_name[64]="";
	char vrrp_obj_path[64]="";	

	if( NULL == nmhansi ){
		nm_log_err("nm_hansi_notify_had_backup_finished prarm nmhansi is NULL");
		return NM_ERR_UNKNOWN;
	}
	
	nm_dbus_t *nmdbus = nmhansi->nmdbus;
	int vrid = nmhansi->ins_id;

	nm_log_info("nm_hansi_notify_had_backup_finished hansi %d", vrid);
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
					nm_dbus_get_dbus_conn(nmdbus),query,-1, &err);	
	dbus_message_unref(query);
	if (NULL == reply) {
		if (dbus_error_is_set(&err)) {
			dbus_error_free(&err);
		}
		nm_log_err("notify had err!!!!!");
		return NM_ERR_INPUT_PARAM_ERR;
	}
	dbus_message_iter_init(reply,&iter);
	dbus_message_iter_get_basic(&iter,&ret);
	dbus_message_unref(reply);
	nm_log_info("notify had sucess!vrid=%d",vrid);
	return NM_RETURN_OK;
}

static int
nm_hansi_do_syn_time_data(void *cbp, void *data, struct timeval *cb_tv)
{
	int ret = 0;
	nm_hansi_t *nmhansi = cbp;
	
#if 0	
	ret = nm_time_set_time((struct timeval *)data);
	if (NM_RETURN_OK != ret) {
		nm_log_err("nm_hansi_do_syn_time_data error, ret=%d", ret);
	}
#endif
	if (NULL == nmhansi) {
		nm_log_err("nm_hansi_do_syn_time_data nmhansi is null!");
		return NM_ERR_UNKNOWN;
	}
	if (NULL == nmhansi->proc) {
		nm_log_err("nm_hansi_do_syn_time_data proc is null!");
		return NM_ERR_UNKNOWN;
	}
	if (NULL == nmhansi->proc->t_heartbeat) {
		nm_log_err("nm_hansi_do_syn_time_data t_heartbeat is NULL!");
		return NM_ERR_UNKNOWN;
	}

	nm_thread_cancel(nmhansi->proc->t_heartbeat);
	nmhansi->proc->t_heartbeat = NULL;
	nmhansi->proc->t_heartbeat = nm_thread_add_timer(nmhansi->master,
						nm_hansi_heartbeat_timeout_on_backup,
						nmhansi->proc,NM_TIME_SYN_BEAT_TIME*3);
	if (NULL == nmhansi->proc->t_heartbeat) {
		nm_log_err("nm_hansi_do_syn_time_data add t_heartbeat failed!");
		return NM_ERR_UNKNOWN;
	}
	nm_hansi_queue_data(nmhansi, nmhansi->ack_time, &ret, sizeof(ret));
	
	nm_log_debug("nm_hansi", "nm_hansi_do_syn_time_data ok");
	return NM_RETURN_OK;
}

static int
nm_hansi_do_ack_time_data(void *cbp, void *data, struct timeval *cb_tv)
{
	int ret = *((int *)data);
	
	if (NM_RETURN_OK != ret) {
		nm_log_err("nm_hansi_do_ack_time_data error, ret=%d", ret);
	}
	nm_log_debug("nm_hansi", "nm_hansi_do_ack_time_data ret=%d", ret);

	return NM_RETURN_OK;
}

static int
nm_hansi_master_accept(nm_thread_t *thread)
{
	nm_hansi_t *nmhansi = NULL;
	int conn_fd = -1;
	struct sockaddr_in addr;
	socklen_t addrlen = 0;
	uint32_t ip = 0;
	uint16_t port = 0;
	char ipstr[32] = "";
	char backup_ipstr[32] = "";

	if (NULL == thread) {
		nm_log_err("nm_hansi_master_accept input error");
		return NM_ERR_INPUT_PARAM_ERR;
	}
    nmhansi = nm_thread_get_arg(thread);
	if (NULL == nmhansi) {
		nm_log_err("nm_hansi_master_accept nmhansi null");
		return NM_ERR_INPUT_PARAM_ERR;
	}

	addrlen = sizeof(addr);
	conn_fd = accept(nmhansi->listen_fd, (struct sockaddr *)&addr, &addrlen);
	if (conn_fd < 0) {
		nm_log_err("nm_hansi_master_accept accept failed, fd(%d): %s",
				nmhansi->listen_fd, safe_strerror(errno));
		return NM_ERR_UNKNOWN;
	}
	ip = ntohl(addr.sin_addr.s_addr);
	port = ntohs(addr.sin_port);
	ip2str(ip, ipstr, sizeof(ipstr));
	ip2str(nmhansi->params.heartlink_opposite_ip,
			backup_ipstr, sizeof(backup_ipstr));
	nm_log_info("nm_hansi_master_accept accept from %s:%u, opposite_ip:%s",
			ipstr, port, backup_ipstr);
	if (nmhansi->params.heartlink_opposite_ip != 0
		&& ip != nmhansi->params.heartlink_opposite_ip) {
		close(conn_fd);
		conn_fd = -1;
		nm_log_warning("nm_hansi_master_accept failed,"
			"accept from ip:%s is not opposite_ip:%s",
			ipstr, backup_ipstr);
		return -1;
	}
	
	if (0 != set_nonblocking(conn_fd)){
		nm_log_err("nm_hansi_master_accept set socket nonblocking failed");
		close(conn_fd);
		conn_fd = -1;
		return NM_ERR_SOCKET_OPT_FAILED;
	}

	nm_hansi_proc_stop(nmhansi->proc);
	nmhansi->proc->conn_fd = conn_fd;
	
	/* master send all data to back */
	nm_log_info("nm_hansi_master_accept accept from backup");
	nm_hansi_on_master_accept(nmhansi);

	nm_hansi_proc_start_on_master(nmhansi->proc);

	nm_log_debug("nm_hansi","nm_hansi_master_accept ok");

	return NM_RETURN_OK;
}

static int
nm_hansi_backup_connect_master(nm_hansi_t *nmhansi)
{
	nm_hansi_proc_t *proc = NULL;
	
	if (NULL == nmhansi) {
		nm_log_err("nm_hansi_backup_connect_master input null");
		return -1;
	}
	proc = nmhansi->proc;
	
	if (proc->conn_fd >= 0)
	{
		nm_log_warning("nm_hansi_backup_connect_master conn_fd >= 0");
		return 0;
	}
	
	proc->conn_fd = conn_to_server(nmhansi->params.heartlink_opposite_ip,
						nmhansi->backup_port);
	if (proc->conn_fd < 0) {
		nm_log_err("nm_hansi_backup_connect_master conn to server failed");
		nm_hansi_proc_stop(proc);
		return NM_ERR_UNKNOWN;
	}
	
	nm_hansi_on_backup_connect(nmhansi);
	
	nm_hansi_proc_start_on_backup(proc);
	
	nm_log_info("nm_hansi_backup_connect_master ok");
	
	return NM_RETURN_OK;
}

static int
nm_hansi_master_start(nm_hansi_t * nmhansi)
{
	if (NULL == nmhansi) {
		nm_log_err("nm_hansi_master_start input error");
		return -1;
	}
	if (nmhansi->listen_fd >= 0 ) {
		nm_log_err("nm_hansi_master_start master already start");
		return NM_RETURN_OK;
	}

	if (NULL != nmhansi->t_accept) {
		nm_log_err("nm_hansi_master_start t_accept not null");
		nm_thread_cancel(nmhansi->t_accept);
		nmhansi->t_accept = NULL;
		/* return NM_ERR_UNKNOWN; */
	}

	nmhansi->listen_fd = 
			create_tcp_server(nmhansi->params.heartlink_local_ip,
					nmhansi->backup_port);
	if (nmhansi->listen_fd < 0) {
		nm_log_err("nm_hansi_master_start create_tcp_server failed");
		return NM_ERR_SOCKET_FAILED;
	}

	nmhansi->t_accept = nm_thread_add_read(nmhansi->master,
					nm_hansi_master_accept,
					nmhansi,
					nmhansi->listen_fd);
	if (NULL == nmhansi->t_accept) {
		nm_log_err("nm_hansi_master_start thread_add_read failed");
		close(nmhansi->listen_fd);
		nmhansi->listen_fd = -1;
		return NM_ERR_THREAD_CREATE_FAILED;
	}

	nm_log_info("nm_hansi_master_start ok");
	return NM_RETURN_OK;
}

static int
nm_hansi_backup_start(nm_hansi_t * nmhansi)
{
	time_t timenow = 0;
	struct timeval tv;
	
	if (NULL == nmhansi) {
		nm_log_err("nm_hansi_backup_start input null");
		return -1;
	}

	nm_time_gettimeofday(&tv, NULL);
	timenow = tv.tv_sec;
	
	nm_hansi_backup_connect_master(nmhansi);
	nmhansi->last_check_connect_time = timenow;
	
	nm_log_info("nm_hansi_backup_start ok");
	
	return NM_RETURN_OK;
}

static int
nm_hansi_master_stop(nm_hansi_t *nmhansi)
{
	if (NULL == nmhansi) {
		nm_log_err("nm_hansi_master_stop input error");
		return -1;
	}
	
	if (NULL != nmhansi->t_accept) {
		nm_thread_cancel(nmhansi->t_accept);
		nmhansi->t_accept = NULL;
	}

	if (nmhansi->listen_fd >= 0) {
		close(nmhansi->listen_fd);
		nmhansi->listen_fd = -1;
	}
	
	nm_hansi_proc_stop(nmhansi->proc);

	nm_log_info("nm_hansi_master_stop ok");
	
	return NM_RETURN_OK;
}

static int
nm_hansi_backup_stop(nm_hansi_t *nmhansi)
{
	if (NULL == nmhansi) {
		nm_log_err("nm_hansi_backup_stop input error");
		return NM_ERR_UNKNOWN;
	}

	nm_hansi_proc_stop(nmhansi->proc);

	nm_log_info("nm_hansi_backup_stop ok");

	return NM_RETURN_OK;
}

nm_hansi_t *
nm_hansi_new(nm_dbus_t *nmdbus,
						int ins_id,
						int hansi_type,
						nm_thread_master_t *master)
{
	nm_hansi_t *nmhansi = NULL;

	if (NULL == nmdbus || NULL == master) {
		nm_log_err("nm_hansi_new input error");
		return NULL;
    }

	nmhansi = nm_malloc(sizeof(nm_hansi_t));
	if (NULL == nmhansi) {
        nm_log_err("nm_hansi_new malloc failed");
        return NULL;
    }

	memset(nmhansi, 0, sizeof(nm_hansi_t));
	nmhansi->listen_fd = -1;
	nmhansi->ins_id = ins_id;
	nmhansi->hansi_type = hansi_type;
	nmhansi->nmdbus = nmdbus;
	nmhansi->master = master;
	nmhansi->proc = nm_hansi_proc_new(nmhansi);
	if (NULL == nmhansi->proc) {
		nm_log_err("nm_hansi_new nm_hansi_proc_new failed");
		nm_free(nmhansi);
		return NULL;
	}
	INIT_LIST_HEAD(&(nmhansi->bktype_head));
	nm_hansi_state_init(nmhansi);

	nmhansi->syn_time = nm_hansi_register_backup_type(nmhansi,
									BACKUP_TYPE_SYN_TIME,
									nmhansi,
									nm_hansi_do_syn_time_data);
	nmhansi->ack_time = nm_hansi_register_backup_type(nmhansi,
									BACKUP_TYPE_ACK_TIME,
									nmhansi,
									nm_hansi_do_ack_time_data);
	nm_log_info("nm_hansi_new ok");
	return nmhansi;
}

int
nm_hansi_free(nm_hansi_t *nmhansi)
{
	backup_type_t *bktype = NULL;
	backup_type_t *next = NULL;
	
	if (NULL == nmhansi) {
		nm_log_err("nm_hansi_free input error");
		return NM_ERR_INPUT_PARAM_ERR;
	}

	list_for_each_entry_safe(bktype, next, &(nmhansi->bktype_head), node) {
			list_del(&(bktype->node));
			nm_free(bktype);
	}
	if (NULL != nmhansi->t_accept) {
		nm_thread_cancel(nmhansi->t_accept);
		nmhansi->t_accept = NULL;
	}
	if (nmhansi->listen_fd >= 0) {
		close(nmhansi->listen_fd);
		nmhansi->listen_fd = -1;
	}
	
	if (NULL != nmhansi->proc) {
		nm_hansi_proc_free(nmhansi->proc);
	}
	nm_free(nmhansi);

	nm_log_info("nm_hansi_free ok");
	return NM_RETURN_OK;
}

int
nm_hansi_start(nm_hansi_t *nmhansi)
{
	int ret = 0;
	
	if (NULL == nmhansi) {
		nm_log_err("nm_hansi_start input error");
		return NM_ERR_INPUT_PARAM_ERR;
	}
	if (1 == nmhansi->server_state) {
		nm_log_info("nm_hansi_start nmhansi already start");
		return NM_RETURN_OK;
	}

	if (VRRP_STATE_TRANSFER == nmhansi->params.state
		|| VRRP_STATE_BACK == nmhansi->params.state) {
		ret = nm_hansi_backup_start(nmhansi);
		if (NM_RETURN_OK != ret) {
			nm_log_err("nm_hansi_start nm_hansi_backup_start failed");
			return ret;
		}
	}
	else if (VRRP_STATE_MAST == nmhansi->params.state) {
		ret = nm_hansi_master_start(nmhansi);
		if (NM_RETURN_OK != ret) {
			nm_log_err("nm_hansi_start nm_hansi_master_start failed");
			return ret;
		}
	}
	else {
		nm_log_info("nm_hansi_start, hansi state=%d", 
					nmhansi->params.state);
	}

	nm_log_info("nm_hansi_start ok");
	
	nmhansi->server_state = 1;
	
	return NM_RETURN_OK;
}

int
nm_hansi_stop(nm_hansi_t *nmhansi)
{
	int ret = 0;
	
	if (NULL == nmhansi) {
		nm_log_err("nm_hansi_stop input error");
		return NM_ERR_INPUT_PARAM_ERR;
	}
	if (0 == nmhansi->server_state) {
		nm_log_info("nm_hansi_stop error nmhansi already stop");
		return NM_RETURN_OK;
	}

	if (VRRP_STATE_TRANSFER == nmhansi->params.state
		|| VRRP_STATE_BACK == nmhansi->params.state) {
		ret = nm_hansi_backup_stop(nmhansi);
		if (NM_RETURN_OK != ret) {
			nm_log_err("nm_hansi_stop nm_hansi_backup_stop failed");
			return ret;
		}
	}
	else if (VRRP_STATE_MAST == nmhansi->params.state) {
		ret = nm_hansi_master_stop(nmhansi);
		if (NM_RETURN_OK != ret) {
			nm_log_err("nm_hansi_stop nm_hansi_master_stop failed");
			return ret;
		}
	}
	else {
		nm_log_info("nm_hansi_stop, hansi state=%d", 
					nmhansi->params.state);
	}

	nm_log_info("nm_hansi_stop ok");
	nmhansi->server_state = 0;
	
	return NM_RETURN_OK;
}

int
nm_hansi_set_backup_port(nm_hansi_t *nmhansi,
								uint16_t port)
{
	if (NULL == nmhansi) {
		nm_log_err("nm_hansi_set_backup_port input error");
		return NM_ERR_NULL_POINTER;
	}

	nmhansi->backup_port = port;
	
	return NM_RETURN_OK;
}

int
get_virtual_ip_by_down_interface(nm_hansi_t *nmhansi, char *intf, uint32_t *virtual_ip)
{
	int i = 0;
	
	if( NULL == nmhansi || NULL == intf || NULL == virtual_ip)
	{
		nm_log_err("get_virtual_ip_by_down_interface input error!");
		return NM_ERR_UNKNOWN;
	}

	for( i = 0; i < nmhansi->params.downlink_cnt; i++)
	{
		if( 0 == strcmp(nmhansi->params.downlink_if[i].if_name, intf) )
		{
			*virtual_ip = nmhansi->params.downlink_if[i].virtual_ip;
			nm_log_debug("nm_hansi","get_appconn_virtual_ip by downlink interface = %s,virtual_ip=%#X",intf,*virtual_ip);
			return NM_RETURN_OK;
		}
	}

	for( i = 0; i < nmhansi->params.vgateway_cnt; i++)
	{
		if( 0 == strcmp(nmhansi->params.vgateway_if[i].if_name, intf) )
		{
			*virtual_ip = nmhansi->params.vgateway_if[i].virtual_ip;
			nm_log_debug("nm_hansi","get_appconn_virtual_ip by vgateway interface = %s,virtual_ip=%#X",intf,*virtual_ip);
			return NM_RETURN_OK;
		}
	}
	
	
	nm_log_err("get_virtual_ip_by_down_interface cant find appconn interface in pha,"\
				"user if = %s",intf);
	return NM_ERR_UNKNOWN;
}

int
set_down_interface_by_virtual_ip (nm_hansi_t *nmhansi,uint32_t virtual_ip,char * intf)
{
	int i = 0;
	
	if( NULL == nmhansi || NULL == intf )
	{
		nm_log_err("set_down_interface_by_virtual_ip input error!");
		return NM_ERR_UNKNOWN;
	}
	
	for( i = 0; i < nmhansi->params.downlink_cnt; i++)
	{
		if( virtual_ip == nmhansi->params.downlink_if[i].virtual_ip)
		{
			strncpy(intf, nmhansi->params.downlink_if[i].if_name,
					MAX_IF_NAME_LEN-1 );
			nm_log_debug("nm_hansi","set_appconn_interface_by downlink virtual_ip = %s,virtual_ip = %#X",intf,virtual_ip);
			return NM_RETURN_OK;
		}		
	}
	
	for( i = 0; i < nmhansi->params.vgateway_cnt; i++)
	{
		if( virtual_ip == nmhansi->params.vgateway_if[i].virtual_ip)
		{
			strncpy(intf, nmhansi->params.vgateway_if[i].if_name,
					MAX_IF_NAME_LEN-1 );
			nm_log_debug("nm_hansi","set_appconn_interface_by vgateway down_if = %s,virtual_ip = %#X",intf,virtual_ip);
			return NM_RETURN_OK;
		}		
	}

	
	nm_log_err("error! cant find appconn interface in nmhansi=%p,virtual_ip=%d,intf=%s\n",nmhansi, virtual_ip,intf);
	return NM_ERR_UNKNOWN;
}

int
nm_hansi_check_connect_state(nm_hansi_t *nmhansi)
{
	time_t timenow = 0;
	struct timeval tv;

	if (NULL == nmhansi) {
		nm_log_err("nm_hansi_check_connect_state input error");
		return -1;
	}

	nm_time_gettimeofday(&tv, NULL);
	timenow = tv.tv_sec;

	if (timenow - nmhansi->last_check_connect_time > CHECK_HANSI_CONNECT_INTERVAL
			|| timenow < nmhansi->last_check_connect_time) {
		if (nm_hansi_is_backup(nmhansi) 
			&& 1 ==	nmhansi->server_state
			&& nmhansi->proc->conn_fd < 0) {
			nm_log_info("nm_hansi_check_connect_state backup connect close, need reconnect");
			nm_hansi_backup_connect_master(nmhansi);
		}
		nmhansi->last_check_connect_time = timenow;
	}
	
	return 0;
}

int
nm_hansi_is_connected(nm_hansi_t *nmhansi)
{
	if (NULL == nmhansi) {
		nm_log_err("nm_hansi_is_connected input error");
		return 0;
	}

	if (NULL == nmhansi->proc) {
		nm_log_err("nm_hansi_is_connected proc is null");
		return 0;
	}
	
	return nmhansi->proc->conn_fd >= 0;
}

int
nm_hansi_syn_time(nm_hansi_t *nmhansi)
{
	struct timeval tv = {0};

	if (nm_hansi_is_master(nmhansi)
		&& nm_hansi_is_connected(nmhansi)) {
		nm_time_gettimeofday(&tv, NULL);
		nm_hansi_jump_data(nmhansi, nmhansi->syn_time, &tv, sizeof(tv));
	}

	return 0;
}

