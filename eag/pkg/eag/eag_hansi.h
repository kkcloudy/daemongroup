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
* eag_hansi.h
*
* CREATOR:
* autelan.software.xxx. team
*
* DESCRIPTION:
* xxx module main routine
*
*******************************************************************************/

#ifndef _EAG_HANSI_H_
#define _EAG_HANSI_H_

#include "eag_dbus.h"
#include "eag_thread.h"

#define EAG_TIME_SYN_BEAT_TIME		  60*5 // 10
typedef struct eag_hansi eag_hansi_t;
typedef struct eag_hansi_backup_type backup_type_t;

typedef int (*ON_GET_BACKUP_DATA)(void *cbp, void *data, struct timeval *cb_tv);
typedef int (*EAG_HANSI_ON_MASTER_ACCEPT)(eag_hansi_t *eaghansi,
											void *param);
typedef int (*EAG_HANSI_ON_BACKUP_CONNECT)(eag_hansi_t *eaghansi,
											void *param);
typedef int (*EAG_HANSI_ON_STATUS_CHANGE)(eag_hansi_t *eaghansi,
											void *param);
typedef int (*EAG_HANSI_ON_BACKUP_HEARTBEAT_TIMEOUT)(eag_hansi_t *eaghansi,
											void *param);

eag_hansi_t *
eag_hansi_new(eag_dbus_t *eagdbus,
							int ins_id,
							int hansi_type,
							eag_thread_master_t *master);

int 
eag_hansi_free(eag_hansi_t *eaghansi);

int
eag_hansi_start(eag_hansi_t *eaghansi );

int
eag_hansi_stop(eag_hansi_t *eaghansi );

int
eag_hansi_set_backup_port(eag_hansi_t *eaghansi,
									uint16_t port);

int
eag_hansi_prev_is_enable(eag_hansi_t *eaghansi);

int
eag_hansi_prev_is_master(eag_hansi_t *eaghansi);

int 
eag_hansi_is_enable(eag_hansi_t *eaghansi);

int 
eag_hansi_is_master(eag_hansi_t *eaghansi);

int
eag_hansi_is_backup( eag_hansi_t *eaghansi );

backup_type_t *
eag_hansi_register_backup_type(eag_hansi_t *eaghansi, 
								const char* type_name,
								void *cbp, 
								ON_GET_BACKUP_DATA on_get_backup_data);

DBusMessage *
eag_hansi_dbus_vrrp_state_change_func(
			    DBusConnection *conn, 
			    DBusMessage *msg, 
			    void *user_data);

int
eag_hansi_add_hansi_param_forward( eag_hansi_t *eaghansi,
									char *objpath, 
									char *name, 
									char *interfacex, 
									char *method);

int
eag_hansi_queue_data( eag_hansi_t *eaghansi,
							backup_type_t *bktype,
							void *data,
							uint32_t datalen);

int
eag_hansi_jump_data( eag_hansi_t *eaghansi,
							backup_type_t *bktype,
							void *data,
							uint32_t datalen);

int
get_virtual_ip_by_down_interface(eag_hansi_t *eaghansi,
										char *intf, 
										uint32_t *virtual_ip);

int
set_down_interface_by_virtual_ip (eag_hansi_t *eaghansi,
										uint32_t virtual_ip,
										char *intf);

int 
eag_hansi_notify_had_backup_finished( eag_hansi_t *eaghansi);

void
eag_hansi_set_on_master_accept_cb( eag_hansi_t *eaghansi,
							EAG_HANSI_ON_MASTER_ACCEPT on_master_accept,
							void *param);

void
eag_hansi_set_on_backup_connect_cb(eag_hansi_t *eaghansi,
							EAG_HANSI_ON_BACKUP_CONNECT on_backup_connect,
							void *param);

void
eag_hansi_set_on_backup_heartbeat_timeout_cb( eag_hansi_t *eaghansi,
							EAG_HANSI_ON_BACKUP_HEARTBEAT_TIMEOUT on_backup_heartbeat_timeout,
							void *param );

void
eag_hansi_set_on_status_change_cb( eag_hansi_t *eaghansi,
							EAG_HANSI_ON_STATUS_CHANGE on_status_change,
							void *param);

int
eag_hansi_check_connect_state(eag_hansi_t *eaghansi);

int
eag_hansi_is_connected(eag_hansi_t *eaghansi);

int
eag_hansi_syn_time(eag_hansi_t *eaghansi);

#endif	/*_EAG_HANSI_H_*/
    
