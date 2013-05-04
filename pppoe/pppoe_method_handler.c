
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <pthread.h>
#include <dbus/dbus.h>

#include <sys/socket.h>
#include <linux/if.h>
#include <linux/if_ether.h>

#include "pppoe_def.h"
#include "pppoe_priv_def.h"
#include "pppoe_dbus_def.h"
#include "pppoe_interface_def.h"
#include "pppoe_method_def.h"
#include "kernel/if_pppoe.h"
#include "radius_def.h"

#include "pppoe_log.h"
#include "pppoe_list.h"
#include "mem_cache.h"
#include "pppoe_dbus.h"
#include "pppoe_method.h"
#include "pppoe_buf.h"
#include "pppoe_util.h"
#include "pppoe_thread.h"
#include "notifier_chain.h"
#include "pppoe_backup.h"
#include "pppoe_ippool.h"
#include "pppoe_session.h"
#include "pppoe_manage.h"

#include "wcpss/waw.h"
#include "wcpss/wid/WID.h"
#include "wcpss/asd/asd.h"
#include "dbus/wcpss/ACDBusPath.h"
#include "dbus/asd/ASDDbusPath.h"
#include "dbus/wcpss/ACDbusDef1.h"
#include "dbus/asd/ASDDbusDef1.h"

#include "pppoe_method_handler.h"


static void
wireless_dbus_path_init(uint32 local, uint32 index, char *path, char *newpath){
	char *cursor = newpath;
	
	cursor += sprintf(newpath,"%s%d_%d", path, local, index); 

	if (ASD_DBUS_SECURITY_OBJPATH == path) {
		sprintf(cursor, "/%s", "security");
	} else if (ASD_DBUS_SECURITY_INTERFACE == path) {
		sprintf(cursor, ".%s", "security");			
	} else if (ASD_DBUS_STA_OBJPATH == path) {
		sprintf(cursor, "/%s", "sta");
	} else if (ASD_DBUS_STA_INTERFACE == path) {
		sprintf(cursor,".%s","sta");			
	} else if (WID_DBUS_WLAN_OBJPATH == path) {
		sprintf(cursor, "/%s", "wlan");
	} else if (WID_DBUS_WLAN_INTERFACE == path) {
		sprintf(cursor, ".%s", "wlan");			
	} else if (WID_DBUS_WTP_OBJPATH == path) {
		sprintf(cursor, "/%s", "wtp");
	} else if (WID_DBUS_WTP_INTERFACE == path) {
		sprintf(cursor, ".%s", "wtp");			
	} else if (WID_DBUS_RADIO_OBJPATH == path) {
		sprintf(cursor, "/%s", "radio");
	} else if (WID_DBUS_RADIO_INTERFACE == path) {
		sprintf(cursor, ".%s", "radio");			
	} else if (WID_DBUS_QOS_OBJPATH == path) {
		sprintf(cursor, "/%s", "qos");
	} else if (WID_DBUS_QOS_INTERFACE == path) {
		sprintf(cursor, ".%s", "qos");
	} else if (WID_DBUS_EBR_OBJPATH == path) {
		sprintf(cursor, "/%s", "ebr");
	} else if (WID_DBUS_EBR_INTERFACE == path) {
		sprintf(cursor, ".%s", "ebr");
	} else if (WID_BAK_OBJPATH == path) {
		sprintf(cursor, "/%s", "bak");
	} else if (WID_BAK_INTERFACE == path) {
		sprintf(cursor, ".%s", "bak");
	} else if (WID_DBUS_ACIPLIST_OBJPATH == path) {
		sprintf(cursor, "/%s", "aciplist");
	} else if (WID_DBUS_ACIPLIST_INTERFACE == path) {
		sprintf(cursor, ".%s", "aciplist");
	} else if (ASD_DBUS_AC_GROUP_OBJPATH == path) {
		sprintf(cursor, "/%s", "acgroup");
	} else if (ASD_DBUS_AC_GROUP_INTERFACE == path) {
		sprintf(cursor, ".%s", "acgroup");
	} else if (WID_DBUS_AP_GROUP_OBJPATH == path) {
		sprintf(cursor, "/%s", "apgroup");
	} else if (WID_DBUS_AP_GROUP_INTERFACE == path) {
		sprintf(cursor, ".%s", "apgroup");
	}
}

static int
method_instance_show_vrrp_info(void *data, void *para) {
#define VRRP_DBUS_BUSNAME			"aw.vrrpcli"
#define VRRP_DBUS_OBJPATH			"/aw/vrrp"
#define VRRP_DBUS_INTERFACE			"aw.vrrp"
#define VRRP_DBUS_METHOD_SET_PPPOE	"vrrp_set_pppoe"

	DBusMessage *query, *reply;	
	DBusConnection *connection;
	DBusMessageIter	 iter;
	DBusError	err;
	char BUSNAME[PATH_LEN];
	char OBJPATH[PATH_LEN];
	uint32 instance_flag = 1, num, instance_id;
	struct vrrp_instance_info *info = (struct vrrp_instance_info *)data;
	int i;

	if (unlikely(!info)) {
		pppoe_log(LOG_WARNING, "input data is NULL\n");
		return PPPOEERR_EINVAL;
	}

	if (info->local_id) {
		pppoe_log(LOG_INFO, "local hansi config is not exist\n");
		return PPPOEERR_ENOEXIST;
	}

	connection = pppoe_dbus_get_local_connection();
	if (!connection) {
		pppoe_log(LOG_WARNING, "vrrp instance %u: get local dbus connection failed\n", 
								info->instance_id);
		return PPPOEERR_EDBUS;
	}
	
	snprintf(BUSNAME, sizeof(BUSNAME), "%s%d", 
			VRRP_DBUS_BUSNAME, info->instance_id);
	snprintf(OBJPATH, sizeof(OBJPATH), "%s%d", 
			VRRP_DBUS_OBJPATH, info->instance_id);

	query = dbus_message_new_method_call(BUSNAME, OBJPATH, 
										VRRP_DBUS_INTERFACE, 
										VRRP_DBUS_METHOD_SET_PPPOE);
	if (unlikely(!query)) {
		pppoe_log(LOG_WARNING, "vrrp instance %u: new method call failed\n", 
								info->instance_id);
		return PPPOEERR_ENOMEM;
	}

	dbus_message_append_args(query,
							DBUS_TYPE_UINT32, &instance_flag,
							DBUS_TYPE_INVALID);

	dbus_error_init(&err);
	reply = dbus_connection_send_with_reply_and_block(connection, query, -1, &err);
	dbus_message_unref(query);
	if (!reply) {
		if (dbus_error_is_set(&err)) {
			pppoe_log(LOG_NOTICE, "%s raised: %s\n", err.name, err.message);
			dbus_error_free(&err);
		}
		pppoe_log(LOG_NOTICE, "vrrp instance %u get reply failed\n", info->instance_id);
		return PPPOEERR_EDBUS;
	}

	dbus_message_iter_init(reply, &iter);
	dbus_message_iter_get_basic(&iter, &num);
	pppoe_log(LOG_DEBUG, "reply get num %u instance\n", num);

	for (i = 0; i < num; i++) {
		dbus_message_iter_next(&iter);
		dbus_message_iter_get_basic(&iter, &instance_id);
		
		if (instance_id != info->instance_id) {
			dbus_message_iter_next(&iter);	/* state */
			dbus_message_iter_next(&iter);	/* uplink_cnt */
			dbus_message_iter_next(&iter);	/* uplink_array */
			dbus_message_iter_next(&iter);	/* downlink_cnt */
			dbus_message_iter_next(&iter);	/* downlink_array */
			dbus_message_iter_next(&iter);	/* interface_name */	
			dbus_message_iter_next(&iter);	/* heartlink_local_ip */
			dbus_message_iter_next(&iter);	/* heartlink_opposite_ip */
			dbus_message_iter_next(&iter);	/* vgateway_cnt */
			dbus_message_iter_next(&iter);	/* vgateway_array */
			continue;
		}

		dbus_message_iter_next(&iter);	/* state */
		dbus_message_iter_get_basic(&iter, &info->state);
		
		dbus_message_iter_next(&iter);	/* uplink_cnt */
		dbus_message_iter_next(&iter);	/* uplink_array */
		dbus_message_iter_next(&iter);	/* downlink_cnt */
		dbus_message_iter_next(&iter);	/* downlink_array */
		dbus_message_iter_next(&iter);	/* interface_name */	

		dbus_message_iter_next(&iter);	/* heartlink_local_ip */
		dbus_message_iter_get_basic(&iter, &info->heartlink_local_ip);

		dbus_message_iter_next(&iter);	/* heartlink_opposite_ip */
		dbus_message_iter_get_basic(&iter, &info->heartlink_opposite_ip);
		goto out;
	}

	pppoe_log(LOG_INFO, "instance %u info is not exist\n", info->instance_id);
	return PPPOEERR_ENOEXIST;

out:
	return PPPOEERR_SUCCESS;
}

/* dbus connection is not lock, so this method need call by instance thread */
static int
method_instance_notify_vrrp_backup_finished(void *data, void *para) {
#define VRRP_DBUS_BUSNAME		"aw.vrrpcli"
#define VRRP_DBUS_OBJPATH		"/aw/vrrp"
#define VRRP_DBUS_INTERFACE		"aw.vrrp"
#define VRRP_DBUS_METHOD_SET_PPPOE_TRANSFER_STATE	"vrrp_set_pppoe_transfer_state"

	DBusMessage *query, *reply;
	DBusConnection *connection;
	DBusMessageIter	iter;
	DBusError err;	
	char dbusName[DBUSNAME_LEN];
	char dbusObjPath[DBUSOBJPATH_LEN];
	struct vrrp_instance_info *info = (struct vrrp_instance_info *)data;	
	uint32 ret;

	if (unlikely(!info)) {
		pppoe_log(LOG_WARNING, "input data is NULL\n");
		return PPPOEERR_EINVAL;
	}
	
	pppoe_log(LOG_INFO, "pppoe instance %u notify vrrp backup finished\n", 
						info->instance_id);
	
	connection = pppoe_dbus_get_local_connection();
	if (!connection) {
		pppoe_log(LOG_WARNING, "get local dbus connection failed\n");
		return PPPOEERR_EDBUS;
	}
	
	memset(dbusName, 0, sizeof(dbusName));
	memset(dbusObjPath, 0, sizeof(dbusObjPath));
	snprintf(dbusName, sizeof(dbusName), "%s%u", 
					VRRP_DBUS_BUSNAME, info->instance_id);
	snprintf(dbusObjPath, sizeof(dbusObjPath), "%s%u", 
					VRRP_DBUS_OBJPATH, info->instance_id);

	query = dbus_message_new_method_call(dbusName, dbusObjPath,
										VRRP_DBUS_INTERFACE,
										VRRP_DBUS_METHOD_SET_PPPOE_TRANSFER_STATE);
	if (unlikely(!query)) {
		pppoe_log(LOG_WARNING, "dbus new method call failed\n");
		return PPPOEERR_ENOMEM;
	}

	dbus_message_append_args(query,
						 DBUS_TYPE_UINT32, &info->instance_id,
						 DBUS_TYPE_UINT32, &info->state,
						 DBUS_TYPE_INVALID);

	dbus_error_init(&err);	
	reply = dbus_connection_send_with_reply_and_block(connection, query, -1, &err);	
	dbus_message_unref(query);
	
	if (NULL == reply) {
		if (dbus_error_is_set(&err)) {
			pppoe_log(LOG_WARNING, "%s raised: %s\n", err.name, err.message);			
			dbus_error_free(&err);
		}
		pppoe_log(LOG_WARNING, "dbus get reply failed\n");
		return PPPOEERR_EDBUS;
	}
	
	dbus_message_iter_init(reply, &iter);
	dbus_message_iter_get_basic(&iter, &ret);
	dbus_message_unref(reply);
	
	pppoe_log(LOG_INFO, "instance %u notify vrrp backup finished success", info->instance_id);
	return PPPOEERR_SUCCESS;
}

static int
method_instance_backup_task(void *data, void *para) {
	struct pppoe_buf *pbuf = (struct pppoe_buf *)data;
	backup_struct_t *backup = (backup_struct_t *)para;
	backup_task_t *task;
	int ret;

	if (unlikely(!pbuf || !backup)) {
		return PPPOEERR_EINVAL;
	}

	task = backup_task_create(backup, pbuf, BACKUP_INSTANCE_SYNC);
	if (unlikely(!task)) {
		pppoe_log(LOG_WARNING, "backup task create failed\n");
		ret = PPPOEERR_ENOMEM;
		goto error;
	}

	ret = backup_task_add(backup, task);
	if (ret) {
		pppoe_log(LOG_WARNING, "backup task add failed, ret %d\n", ret);
		goto error1;
	}
	
	return PPPOEERR_SUCCESS;

error1:
	backup_task_destroy(backup, &task);
error:
	return ret;
}


static inline int
session_wireless_info_set(pppoe_manage_t *manage, struct session_wireless_info *sess_info) {
	session_struct_t *sess
		= manage_sess_get_by_sid(manage, sess_info->info.sid);
	if (!sess) {
		pppoe_token_log(TOKEN_METHOD, "session %u is not exist\n", sess_info->info.sid);
		return PPPOEERR_ENOEXIST;
	}

	sess->wtp_id 		= sess_info->wtp_id;
	sess->radio_g_id 	= sess_info->radio_g_id;
	sess->bssindex 		= sess_info->bssindex;
	sess->vlan_id 		= sess_info->vlan_id;
	sess->wlan_id 		= sess_info->wlan_id;
	sess->security_id 	= sess_info->security_id;
	sess->radio_l_id 	= sess_info->radio_l_id;
	memcpy(sess->wtpmac, sess_info->wtpmac, ETH_ALEN);
	session_put(sess);

	pppoe_token_log(TOKEN_METHOD, "session %u: wtp %u radio %u wlan %u\n", 
									sess_info->info.sid, sess_info->wtp_id, 
									sess_info->radio_l_id, sess_info->wlan_id);	
	return PPPOEERR_SUCCESS;
}

/* dbus connection is not lock, so this method need call by instance thread */
static int
method_session_wireless_info_show(void *data, void *para) {
	DBusMessage *query, *reply;	
	DBusConnection *connection;
	DBusMessageIter	iter;
	DBusError err;
	char BUSNAME[PATH_LEN];
	char OBJPATH[PATH_LEN];
	char INTERFACE[PATH_LEN];
	struct session_wireless_info *sta = (struct session_wireless_info *)data;
	pppoe_manage_t *manage = (pppoe_manage_t *)para;
	int ret;

	if (unlikely(!sta))
		return PPPOEERR_EINVAL;

	connection = pppoe_dbus_get_local_connection();
	if (!connection) {
		pppoe_log(LOG_WARNING, "session %u:get local dbus connection failed\n", sta->info.sid);
		return PPPOEERR_EDBUS;
	}

	wireless_dbus_path_init(sta->info.local_id, sta->info.instance_id, ASD_DBUS_BUSNAME, BUSNAME);
	wireless_dbus_path_init(sta->info.local_id, sta->info.instance_id, ASD_DBUS_STA_OBJPATH, OBJPATH);
	wireless_dbus_path_init(sta->info.local_id, sta->info.instance_id, ASD_DBUS_STA_INTERFACE,INTERFACE);
	
	query = dbus_message_new_method_call(BUSNAME, OBJPATH, INTERFACE, 
										ASD_DBUS_STA_METHOD_SHOWSTA_V2);
	if (unlikely(!query)) {
		pppoe_log(LOG_WARNING, "session %u: dbus new method call failed\n", sta->info.sid);
		return PPPOEERR_ENOMEM;
	}
	
	dbus_message_append_args(query,
							 DBUS_TYPE_BYTE,&sta->info.mac[0],
							 DBUS_TYPE_BYTE,&sta->info.mac[1],
							 DBUS_TYPE_BYTE,&sta->info.mac[2],
							 DBUS_TYPE_BYTE,&sta->info.mac[3],
							 DBUS_TYPE_BYTE,&sta->info.mac[4],
							 DBUS_TYPE_BYTE,&sta->info.mac[5],
							 DBUS_TYPE_INVALID);
	
	dbus_error_init(&err);	
	reply = dbus_connection_send_with_reply_and_block(connection, query, -1, &err);
	dbus_message_unref(query);
	if (!reply) {
		if (dbus_error_is_set(&err)) {
			pppoe_token_log(TOKEN_METHOD, "%s raised: %s\n", err.name, err.message);			
			dbus_error_free(&err);
		}
		pppoe_token_log(TOKEN_METHOD, "session %u: dbus fail get reply\n", sta->info.sid);
		return PPPOEERR_EDBUS;
	}

	dbus_message_iter_init(reply, &iter);
	dbus_message_iter_get_basic(&iter, &ret);

	if (ret) {
		pppoe_token_log(TOKEN_METHOD, "session %u get wireless failed\n", sta->info.sid);
		goto out;
	}

	dbus_message_iter_next(&iter);	
	dbus_message_iter_get_basic(&iter, &sta->wlan_id);	

	dbus_message_iter_next(&iter);	
	dbus_message_iter_get_basic(&iter, &sta->radio_g_id);	
	
	dbus_message_iter_next(&iter);	
	dbus_message_iter_get_basic(&iter, &sta->bssindex); 
	
	dbus_message_iter_next(&iter);	
	dbus_message_iter_get_basic(&iter, &sta->security_id);	
	
	dbus_message_iter_next(&iter);	
	dbus_message_iter_get_basic(&iter, &sta->vlan_id);	

	dbus_message_iter_next(&iter);	
	dbus_message_iter_get_basic(&iter, &sta->wtpmac[0]);	
	
	dbus_message_iter_next(&iter);	
	dbus_message_iter_get_basic(&iter, &sta->wtpmac[1]);	
	
	dbus_message_iter_next(&iter);	
	dbus_message_iter_get_basic(&iter, &sta->wtpmac[2]);	
	
	dbus_message_iter_next(&iter);	
	dbus_message_iter_get_basic(&iter, &sta->wtpmac[3]);	
	
	dbus_message_iter_next(&iter);	
	dbus_message_iter_get_basic(&iter, &sta->wtpmac[4]);	
	
	dbus_message_iter_next(&iter);	
	dbus_message_iter_get_basic(&iter, &sta->wtpmac[5]);	
		
	dbus_message_iter_next(&iter);	
	/* essid */
	
	dbus_message_iter_next(&iter);	
	/* flow_check */
	
	dbus_message_iter_next(&iter);	
	/* no_flow_time */
	
	dbus_message_iter_next(&iter);	
	/* limit_flow */
	
	dbus_message_iter_next(&iter);	
	dbus_message_iter_get_basic(&iter,&sta->auth_type); 

	sta->wtp_id = sta->radio_g_id >> 2;
	sta->radio_l_id = sta->radio_g_id & 0x3;

	if (manage) {
		ret = session_wireless_info_set(manage, sta);
	}
	
	pppoe_token_log(TOKEN_METHOD, "session %u get wireless success\n", sta->info.sid);
	
out:	
	dbus_message_unref(reply);
	return ret;
}

static int
method_session_stat_update(void *data, void *para) {
	struct session_info *s_info = (struct session_info *)data;
	pppoe_manage_t *manage = (pppoe_manage_t *)para;
	session_struct_t *sess;
	struct session_stat stat;
	char path[128], buf[1024];	
	uint32 sid;
	FILE *fp;

	if (unlikely(!s_info || !manage))
		return PPPOEERR_EINVAL;

	memset(path, 0, sizeof(path));
	snprintf(path, sizeof(path), "/proc/net/dev_pppoe/%s", s_info->ifname);

	fp = fopen(path, "r");
	if (!fp) {
		pppoe_log(LOG_WARNING, "%s open fail\n", path);
		return PPPOEERR_EOPEN;
	}

	fgets(buf, sizeof(buf), fp);

	while (fgets(buf, sizeof(buf), fp)) {
		if (5 != sscanf(buf, "%u %llu %llu %llu %llu", &sid, 
					&stat.rx_packets, &stat.rx_bytes,
					&stat.tx_packets, &stat.tx_bytes)) {
			pppoe_log(LOG_WARNING, "buf (%s) sscanf fail\n", buf);
			continue;
		}
		
		sess = manage_sess_get_by_sid(manage, sid);
		if (!sess) {
			continue;
		}

		if (sess->state > SESSION_ONLINE) {
			session_put(sess);
			continue;
		}

		sess->stat.rx_bytes = sess->bk_stat.rx_bytes + stat.rx_bytes;
		sess->stat.tx_bytes = sess->bk_stat.tx_bytes + stat.tx_bytes;

		sess->stat.rx_packets = sess->bk_stat.rx_packets + stat.rx_packets;
		sess->stat.tx_packets = sess->bk_stat.tx_packets + stat.tx_packets;
		
		session_put(sess);
	}

	fclose(fp);	
	pppoe_token_log(TOKEN_METHOD, "sessions %s stat update success\n", s_info->ifname);
	return PPPOEERR_SUCCESS;
}

static int
method_session_kick_by_sid(void *data, void *para) {
	struct session_info *info = (struct session_info *)data;
	pppoe_manage_t *manage = (pppoe_manage_t *)para;

	if (unlikely(!info || !info->sid || !manage))
		return PPPOEERR_EINVAL;

	return manage_sess_kick_by_sid(manage, info->sid);
}

static int
method_session_kick_by_mac(void *data, void *para) {
	struct session_info *info = (struct session_info *)data;
	pppoe_manage_t *manage = (pppoe_manage_t *)para;

	if (unlikely(!info || !manage))
		return PPPOEERR_EINVAL;

	return manage_sess_kick_by_mac(manage, info->mac);
}

static int
method_session_online_sync(void *data, void *para) {
	struct pppoe_buf *pbuf = (struct pppoe_buf *)data;
	pppoe_manage_t *manage = (pppoe_manage_t *)para;

	if (unlikely(!pbuf || !manage)) {
		return PPPOEERR_EINVAL;
	}

	return manage_sess_online_sync(manage, pbuf);
}

static int
method_session_offline_sync(void *data, void *para) {
	struct pppoe_buf *pbuf = (struct pppoe_buf *)data;
	pppoe_manage_t *manage = (pppoe_manage_t *)para;

	if (unlikely(!pbuf || !manage)) {
		return PPPOEERR_EINVAL;
	}

	return manage_sess_offline_sync(manage, pbuf);
}

static int
method_session_update_sync(void *data, void *para) {
	struct pppoe_buf *pbuf = (struct pppoe_buf *)data;
	pppoe_manage_t *manage = (pppoe_manage_t *)para;

	if (unlikely(!pbuf || !manage)) {
		return PPPOEERR_EINVAL;
	}

	return manage_sess_update_sync(manage, pbuf);
}


static int
method_session_clear(void *data, void *para) {
	pppoe_manage_t *manage = (pppoe_manage_t *)para;

	if (unlikely(!manage)) {
		return PPPOEERR_EINVAL;
	}

	manage_sessions_clear(manage);
	return PPPOEERR_SUCCESS;
}

static int
method_session_clear_v2(void *data, void *para) {
	pppoe_manage_t *manage = (pppoe_manage_t *)para;

	if (unlikely(!manage)) {
		return PPPOEERR_EINVAL;
	}

	manage_sessions_clear_v2(manage);
	return PPPOEERR_SUCCESS;
}


static int
method_session_sync(void *data, void *para) {
	pppoe_manage_t *manage = (pppoe_manage_t *)para;

	if (unlikely(!manage)) {
		return PPPOEERR_EINVAL;
	}

	manage_sessions_sync(manage);
	return PPPOEERR_SUCCESS;
}

static int
method_session_sync_finished(void *data, void *para) {
	pppoe_manage_t *manage = (pppoe_manage_t *)para;	

	if (unlikely(!manage)) {
		return PPPOEERR_EINVAL;
	}

	manage_sessions_sync_finished(manage);
	return PPPOEERR_SUCCESS;
}


void
pppoe_method_handler_init(void) {
	pppoe_method_register(PPPOE_METHOD_INSTANCE_SHOW_VRRP_INFO, method_instance_show_vrrp_info);
	pppoe_method_register(PPPOE_METHOD_INSTANCE_NOTITY_VRRP_BACKUP, method_instance_notify_vrrp_backup_finished);		
	pppoe_method_register(PPPOE_METHOD_INSTANCE_BACKUP_TASK, method_instance_backup_task);	
	pppoe_method_register(PPPOE_METHOD_SESSION_WIRELESS_INFO_SHOW, method_session_wireless_info_show);
	pppoe_method_register(PPPOE_METHOD_SESSION_STAT_UPDATE, method_session_stat_update);
	pppoe_method_register(PPPOE_METHOD_SESSION_KICK_BY_SESSION_ID, method_session_kick_by_sid);
	pppoe_method_register(PPPOE_METHOD_SESSION_KICK_BY_MAC, method_session_kick_by_mac);
	pppoe_method_register(PPPOE_METHOD_SESSION_ONLINE_SYNC, method_session_online_sync);
	pppoe_method_register(PPPOE_METHOD_SESSION_OFFLINE_SYNC, method_session_offline_sync);
	pppoe_method_register(PPPOE_METHOD_SESSION_UPDATE_SYNC, method_session_update_sync);
	pppoe_method_register(PPPOE_METHOD_SESSION_CLEAR, method_session_clear);	
	pppoe_method_register(PPPOE_METHOD_SESSION_CLEAR_V2, method_session_clear_v2);	
	pppoe_method_register(PPPOE_METHOD_SESSION_SYNC, method_session_sync);	
	pppoe_method_register(PPPOE_METHOD_SESSION_SYNC_FINISHED, method_session_sync_finished);
}

void
pppoe_method_handler_exit(void) {
	pppoe_method_unregister(PPPOE_METHOD_INSTANCE_SHOW_VRRP_INFO);
	pppoe_method_unregister(PPPOE_METHOD_INSTANCE_NOTITY_VRRP_BACKUP);	
	pppoe_method_unregister(PPPOE_METHOD_INSTANCE_BACKUP_TASK);	
	pppoe_method_unregister(PPPOE_METHOD_SESSION_WIRELESS_INFO_SHOW);
	pppoe_method_unregister(PPPOE_METHOD_SESSION_STAT_UPDATE);
	pppoe_method_unregister(PPPOE_METHOD_SESSION_KICK_BY_SESSION_ID);
	pppoe_method_unregister(PPPOE_METHOD_SESSION_KICK_BY_MAC);
	pppoe_method_unregister(PPPOE_METHOD_SESSION_ONLINE_SYNC);
	pppoe_method_unregister(PPPOE_METHOD_SESSION_OFFLINE_SYNC);
	pppoe_method_unregister(PPPOE_METHOD_SESSION_UPDATE_SYNC);	
	pppoe_method_unregister(PPPOE_METHOD_SESSION_CLEAR);
	pppoe_method_unregister(PPPOE_METHOD_SESSION_CLEAR_V2);			
	pppoe_method_unregister(PPPOE_METHOD_SESSION_SYNC);		
	pppoe_method_unregister(PPPOE_METHOD_SESSION_SYNC_FINISHED);		
}

