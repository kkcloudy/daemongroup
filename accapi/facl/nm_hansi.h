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
* nm_hansi.h
*
* CREATOR:
* autelan.software.xxx. team
*
* DESCRIPTION:
* xxx module main routine
*
*******************************************************************************/

#ifndef _NM_HANSI_H_
#define _NM_HANSI_H_

#include "nm_dbus.h"
#include "nm_thread.h"

#define NM_TIME_SYN_BEAT_TIME		  60*5 // 10
typedef struct nm_hansi nm_hansi_t;
typedef struct nm_hansi_backup_type backup_type_t;

typedef int (*ON_GET_BACKUP_DATA)(void *cbp, void *data, struct timeval *cb_tv);
typedef int (*NM_HANSI_ON_MASTER_ACCEPT)(nm_hansi_t *nmhansi,
											void *param);
typedef int (*NM_HANSI_ON_BACKUP_CONNECT)(nm_hansi_t *nmhansi,
											void *param);
typedef int (*NM_HANSI_ON_STATUS_CHANGE)(nm_hansi_t *nmhansi,
											void *param);
typedef int (*NM_HANSI_ON_BACKUP_HEARTBEAT_TIMEOUT)(nm_hansi_t *nmhansi,
											void *param);

nm_hansi_t *
nm_hansi_new(nm_dbus_t *nmdbus,
							int ins_id,
							int hansi_type,
							nm_thread_master_t *master);

int 
nm_hansi_free(nm_hansi_t *nmhansi);

int
nm_hansi_start(nm_hansi_t *nmhansi );

int
nm_hansi_stop(nm_hansi_t *nmhansi );

int
nm_hansi_set_backup_port(nm_hansi_t *nmhansi,
									uint16_t port);

int
nm_hansi_prev_is_enable(nm_hansi_t *nmhansi);

int
nm_hansi_prev_is_master(nm_hansi_t *nmhansi);

int 
nm_hansi_is_enable(nm_hansi_t *nmhansi);

int 
nm_hansi_is_master(nm_hansi_t *nmhansi);

int
nm_hansi_is_backup( nm_hansi_t *nmhansi );

backup_type_t *
nm_hansi_register_backup_type(nm_hansi_t *nmhansi, 
								const char* type_name,
								void *cbp, 
								ON_GET_BACKUP_DATA on_get_backup_data);

DBusMessage *
nm_hansi_dbus_vrrp_state_change_func(
			    DBusConnection *conn, 
			    DBusMessage *msg, 
			    void *user_data);

int
nm_hansi_add_hansi_param_forward( nm_hansi_t *nmhansi,
									char *objpath, 
									char *name, 
									char *interfacex, 
									char *method);

int
nm_hansi_queue_data( nm_hansi_t *nmhansi,
							backup_type_t *bktype,
							void *data,
							uint32_t datalen);

int
nm_hansi_jump_data( nm_hansi_t *nmhansi,
							backup_type_t *bktype,
							void *data,
							uint32_t datalen);

int
get_virtual_ip_by_down_interface(nm_hansi_t *nmhansi,
										char *intf, 
										uint32_t *virtual_ip);

int
set_down_interface_by_virtual_ip (nm_hansi_t *nmhansi,
										uint32_t virtual_ip,
										char *intf);

int 
nm_hansi_notify_had_backup_finished( nm_hansi_t *nmhansi);

void
nm_hansi_set_on_master_accept_cb( nm_hansi_t *nmhansi,
							NM_HANSI_ON_MASTER_ACCEPT on_master_accept,
							void *param);

void
nm_hansi_set_on_backup_connect_cb(nm_hansi_t *nmhansi,
							NM_HANSI_ON_BACKUP_CONNECT on_backup_connect,
							void *param);

void
nm_hansi_set_on_backup_heartbeat_timeout_cb( nm_hansi_t *nmhansi,
							NM_HANSI_ON_BACKUP_HEARTBEAT_TIMEOUT on_backup_heartbeat_timeout,
							void *param );

void
nm_hansi_set_on_status_change_cb( nm_hansi_t *nmhansi,
							NM_HANSI_ON_STATUS_CHANGE on_status_change,
							void *param);

int
nm_hansi_check_connect_state(nm_hansi_t *nmhansi);

int
nm_hansi_is_connected(nm_hansi_t *nmhansi);

int
nm_hansi_syn_time(nm_hansi_t *nmhansi);

#endif	/*_NM_HANSI_H_*/
    
