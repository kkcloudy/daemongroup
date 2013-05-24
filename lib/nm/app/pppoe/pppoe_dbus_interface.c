
#ifndef _VERSION_18SP7_

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <dbus/dbus.h>

#include <sys/ioctl.h>
#include <sys/types.h> 
#include <sys/socket.h>
#include <linux/if.h>
#include <linux/if_ether.h>

#include "pppoe_def.h"
#include "pppoe_dbus_def.h"
#include "pppoe_interface_def.h"

#include "pppoe_dbus_interface.h"


int
string_to_uint(const char *s, unsigned int *i) {
	char *endptr;
	unsigned int length;
	
	if (!s || !i)
		return PPPOEERR_EINVAL;

	if (s[0] < '0' || s[0] > '9')
		return PPPOEERR_EINVAL;

	length = strlen(s);

	*i = strtoul(s, &endptr, 10);
	if ((endptr - s) != length)
		return PPPOEERR_EINVAL;

	return PPPOEERR_SUCCESS;
}

int 
ifname_detect_exist(const char *ifname) {
	struct ifreq ifr;
	int sk, ret;

	if (!ifname || !ifname[0])
		return PPPOEERR_EINVAL;
	
	sk = socket(AF_INET, SOCK_DGRAM, 0);
	if (sk < 0)
		return PPPOEERR_ESOCKET;

	memset(&ifr, 0, sizeof(ifr));
	strncpy(ifr.ifr_name, ifname, sizeof(ifr.ifr_name) - 1);
	ret = ioctl(sk, SIOCGIFINDEX, &ifr);
	
	close(sk);
	return ret ? PPPOEERR_ENOEXIST : PPPOEERR_SUCCESS;
}


void
insDbusName_init(char *new_name, unsigned int new_len,
			char *name, unsigned int local_id, unsigned int ins_id) {
	memset(new_name, 0, new_len);
	snprintf(new_name, new_len, "%s%d_%d", name, local_id, ins_id);
}

void
insIfName_init(char *ifname, 
		unsigned int slot_id, unsigned int local_id, 
		unsigned int ins_id, unsigned int pppoe_id) {
	memset(ifname, 0, IFNAMSIZ);
	if (local_id)
		snprintf(ifname, IFNAMSIZ, "pppoel%d-%d-%d", slot_id, ins_id, pppoe_id);
	else 
		snprintf(ifname, IFNAMSIZ, "pppoe%d-%d-%d", slot_id, ins_id, pppoe_id);
}

static inline int
binary_search(struct pppoeUserInfo **userarray, unsigned int sid, unsigned int count) {
	unsigned int left, right, middle;

	if (!count)
		return 0;
	
	left = 0;
	right = count -1;
	
	while (right >= left) {
		middle = (left + right) >> 1;
		if (userarray[middle]->sid > sid) {
			if (!middle) {
				return 0;
			} else {
				right = middle - 1;
			}	
		} else if (userarray[middle]->sid == sid) {
			return -1;
		} else {
			left = middle + 1;
		}
	}
	
	return left;
}

static inline int
userinfo_array_insert(struct pppoeUserInfo **userarray,
					struct pppoeUserInfo *info, unsigned int count) {	
	int index = binary_search(userarray, info->sid, count);
	if (index < 0)
		return PPPOEERR_EEXIST;
	else if (index > count)
		return PPPOEERR_EINVAL;
	
	memmove(&userarray[index + 1], &userarray[index], 
			(count - index) * sizeof(struct pppoeUserInfo *));
    userarray[index] = info;
	return PPPOEERR_SUCCESS;
}

static int
pfm_table_entry_config(DBusConnection *connection, struct pfm_table_entry *entry) {
	DBusMessage *query, *reply;
	DBusError err;
	char *ifname, *srcip, *dstip;
	int ret;

#define PFM_DBUS_BUSNAME				"pfm.daemon"
#define PFM_DBUS_OBJPATH				"/pfm/daemon"
#define PFM_DBUS_INTERFACE				"pfm.daemon"
#define PFM_DBUS_METHOD_PFM_TABLE 	    "pfm_maintain_table"

	query = dbus_message_new_method_call(
        								PFM_DBUS_BUSNAME,		
        								PFM_DBUS_OBJPATH,	
        								PFM_DBUS_INTERFACE,
        								PFM_DBUS_METHOD_PFM_TABLE);
	if (!query)
		return PPPOEERR_ENOMEM;

	ifname = entry->ifname;
	srcip = entry->src_ipaddr;
	dstip = entry->dest_ipaddr;
	dbus_message_append_args(query,
							DBUS_TYPE_INT32,  &entry->opt,		/* opt 0:add pfm entry, opt 1:del pfm entry */
							DBUS_TYPE_INT32,  &entry->opt_para,
							DBUS_TYPE_UINT16, &entry->protocol,
							DBUS_TYPE_STRING, &ifname,
							DBUS_TYPE_UINT32, &entry->src_port,
							DBUS_TYPE_UINT32, &entry->dest_port,
							DBUS_TYPE_STRING, &srcip,
							DBUS_TYPE_STRING, &dstip,
							DBUS_TYPE_INT32,  &entry->slot_id,
							DBUS_TYPE_INVALID);
	
	dbus_error_init(&err);	
	reply = dbus_connection_send_with_reply_and_block(connection, query, -1, &err);
	dbus_message_unref(query);
	if (NULL == reply){
		if (dbus_error_is_set(&err)){
			dbus_error_free(&err);
		}
		return PPPOEERR_EDBUS;
	}
	
	if (dbus_message_get_args(reply, &err,
								DBUS_TYPE_UINT32,&ret,
								DBUS_TYPE_INVALID))  {
	} else {
		if (dbus_error_is_set(&err)) {
			dbus_error_free(&err);
		}	
		ret = PPPOEERR_ESYSCALL;
	}
	
	dbus_message_unref(reply);
	return ret ? PPPOEERR_ESYSCALL : PPPOEERR_SUCCESS;
}


int
pppoe_config_log_debug(DBusConnection *connection, 
					unsigned int local_id, unsigned int ins_id,
					unsigned int type, unsigned int state) {
	DBusMessage *query, *reply;
	DBusError err;
	DBusMessageIter	 iter;
	char dbusName[DBUSNAME_LEN];
	int ret;

	if (!connection)
		return PPPOEERR_EINVAL;
	
	insDbusName_init(dbusName, DBUSNAME_LEN, PPPOE_DBUS_DBUSNAME, local_id, ins_id);
	query = dbus_message_new_method_call(dbusName, 
										PPPOE_DBUS_OBJPATH,
										PPPOE_DBUS_INTERFACE,
										PPPOE_DBUS_CONFIG_LOG_DEBUG);
	if (!query)
		return PPPOEERR_ENOMEM;
	
	dbus_message_append_args(query,
							DBUS_TYPE_UINT32, &type,
							DBUS_TYPE_UINT32, &state,
							DBUS_TYPE_INVALID);

	dbus_error_init(&err);
	reply = dbus_connection_send_with_reply_and_block(connection, query, -1, &err);
	dbus_message_unref(query);
	if (!reply) {
		if (dbus_error_is_set(&err)) {
			dbus_error_free(&err);
		}
		return PPPOEERR_EDBUS;
	}

	dbus_message_iter_init(reply, &iter);
	dbus_message_iter_get_basic(&iter, &ret);

	dbus_message_unref(reply);

	return ret;
}

int
pppoe_config_log_token(DBusConnection *connection, 
					unsigned int local_id, unsigned int ins_id,
					PPPOELogToken token, unsigned int state) {
	DBusMessage *query, *reply;
	DBusError err;
	DBusMessageIter	 iter;
	char dbusName[DBUSNAME_LEN];
	int ret;

	if (!connection)
		return PPPOEERR_EINVAL;
	
	insDbusName_init(dbusName, DBUSNAME_LEN, PPPOE_DBUS_DBUSNAME, local_id, ins_id);
	query = dbus_message_new_method_call(dbusName, 
										PPPOE_DBUS_OBJPATH,
										PPPOE_DBUS_INTERFACE,
										PPPOE_DBUS_CONFIG_LOG_TOKEN);
	if (!query) 
		return PPPOEERR_ENOMEM;
	
	dbus_message_append_args(query,
							DBUS_TYPE_UINT32, &token,
							DBUS_TYPE_UINT32, &state,
							DBUS_TYPE_INVALID);
	
	dbus_error_init(&err);
	reply = dbus_connection_send_with_reply_and_block(connection, query, -1, &err);
	dbus_message_unref(query);
	if (!reply) {
		if (dbus_error_is_set(&err)) {
			dbus_error_free(&err);
		}
		return PPPOEERR_EDBUS;
	}

	dbus_message_iter_init(reply, &iter);
	dbus_message_iter_get_basic(&iter, &ret);

	dbus_message_unref(reply);

	return ret;
}

int
pppoe_config_device_create(DBusConnection *connection, 
				unsigned int local_id, unsigned int ins_id, 
				unsigned int dev_id, char *ifname, char *dev_desc) {
	DBusMessage *query, *reply;
	DBusError err;
	DBusMessageIter	 iter;
	char dbusName[DBUSNAME_LEN];
	int ret;

	if (!connection || !ifname || !dev_desc)
		return PPPOEERR_EINVAL;

	if (dev_id > DEV_MAX_NUM)
		return PPPOEERR_EINVAL;
	
	insDbusName_init(dbusName, DBUSNAME_LEN, PPPOE_DBUS_DBUSNAME, local_id, ins_id);
	query = dbus_message_new_method_call(dbusName, 
										PPPOE_DBUS_OBJPATH,
										PPPOE_DBUS_INTERFACE,
										PPPOE_DBUS_DEVICE_CREATE);
	if (!query) 
		return PPPOEERR_ENOMEM;
	
	dbus_message_append_args(query,
							DBUS_TYPE_UINT32, &dev_id,
							DBUS_TYPE_STRING, &ifname,
							DBUS_TYPE_STRING, &dev_desc,
							DBUS_TYPE_INVALID);
	
	dbus_error_init(&err);
	reply = dbus_connection_send_with_reply_and_block(connection, query, -1, &err);
	dbus_message_unref(query);
	if (!reply) {
		if (dbus_error_is_set(&err)) {
			dbus_error_free(&err);
		}
		return PPPOEERR_EDBUS;
	}

	dbus_message_iter_init(reply, &iter);
	dbus_message_iter_get_basic(&iter, &ret);

	dbus_message_unref(reply);

	return ret;
}

int
pppoe_config_device_destroy(DBusConnection *connection, 
				unsigned int local_id, unsigned int ins_id, unsigned int dev_id) {
	DBusMessage *query, *reply;
	DBusError err;
	DBusMessageIter	 iter;
	char dbusName[DBUSNAME_LEN];
	int ret;

	if (!connection)
		return PPPOEERR_EINVAL;

	if (dev_id > DEV_MAX_NUM)
		return PPPOEERR_EINVAL;
	
	insDbusName_init(dbusName, DBUSNAME_LEN, PPPOE_DBUS_DBUSNAME, local_id, ins_id);
	query = dbus_message_new_method_call(dbusName, 
										PPPOE_DBUS_OBJPATH,
										PPPOE_DBUS_INTERFACE,
										PPPOE_DBUS_DEVICE_DESTROY);
	if (!query) 
		return PPPOEERR_ENOMEM;
	
	dbus_message_append_args(query,
							DBUS_TYPE_UINT32, &dev_id,
							DBUS_TYPE_INVALID);
	
	dbus_error_init(&err);
	reply = dbus_connection_send_with_reply_and_block(connection, query, -1, &err);
	dbus_message_unref(query);
	if (!reply) {
		if (dbus_error_is_set(&err)) {
			dbus_error_free(&err);
		}
		return PPPOEERR_EDBUS;
	}

	dbus_message_iter_init(reply, &iter);
	dbus_message_iter_get_basic(&iter, &ret);

	dbus_message_unref(reply);

	return ret;
}

int
pppoe_config_device_base(DBusConnection *connection, 
				unsigned int local_id, unsigned int ins_id, 
				unsigned int dev_id, char *base_ifname) {
	DBusMessage *query, *reply;
	DBusError err;
	DBusMessageIter	 iter;
	char dbusName[DBUSNAME_LEN];
	int ret;

	if (!connection)
		return PPPOEERR_EINVAL;

	if (dev_id > DEV_MAX_NUM)
		return PPPOEERR_EINVAL;

	if (!base_ifname) {
		base_ifname = "";
	} else if (strlen(base_ifname) > (IFNAMSIZ - 1)) {
		return PPPOEERR_EINVAL;
	}
	
	insDbusName_init(dbusName, DBUSNAME_LEN, PPPOE_DBUS_DBUSNAME, local_id, ins_id);
	query = dbus_message_new_method_call(dbusName, 
										PPPOE_DBUS_OBJPATH,
										PPPOE_DBUS_INTERFACE,
										PPPOE_DBUS_DEVICE_CONFIG_BASE);
	if (!query) 
		return PPPOEERR_ENOMEM;
	
	dbus_message_append_args(query,
							DBUS_TYPE_UINT32, &dev_id,
							DBUS_TYPE_STRING, &base_ifname,
							DBUS_TYPE_INVALID);
	
	dbus_error_init(&err);
	reply = dbus_connection_send_with_reply_and_block(connection, query, -1, &err);
	dbus_message_unref(query);
	if (!reply) {
		if (dbus_error_is_set(&err)) {
			dbus_error_free(&err);
		}
		return PPPOEERR_EDBUS;
	}

	dbus_message_iter_init(reply, &iter);
	dbus_message_iter_get_basic(&iter, &ret);

	dbus_message_unref(reply);

	return ret;
}

int
pppoe_config_device_apply(DBusConnection *connection, 
				unsigned int local_id, unsigned int ins_id, 
				unsigned int dev_id, char *apply_ifname) {
	DBusMessage *query, *reply;
	DBusError err;
	DBusMessageIter	 iter;
	char dbusName[DBUSNAME_LEN];
	int ret;

	if (!connection)
		return PPPOEERR_EINVAL;

	if (dev_id > DEV_MAX_NUM)
		return PPPOEERR_EINVAL;

	if (!apply_ifname) {
		apply_ifname = "";
	} else if (strlen(apply_ifname) > (IFNAMSIZ - 1)) {
		return PPPOEERR_EINVAL;
	} else if (ifname_detect_exist(apply_ifname)) {	/* apply interface must rpa, */
		return PPPOEERR_ENOEXIST;					/* so check it at master broad. */
	}
	
	insDbusName_init(dbusName, DBUSNAME_LEN, PPPOE_DBUS_DBUSNAME, local_id, ins_id);
	query = dbus_message_new_method_call(dbusName, 
										PPPOE_DBUS_OBJPATH,
										PPPOE_DBUS_INTERFACE,
										PPPOE_DBUS_DEVICE_CONFIG_APPLY);
	if (!query) 
		return PPPOEERR_ENOMEM;
	
	dbus_message_append_args(query,
							DBUS_TYPE_UINT32, &dev_id,
							DBUS_TYPE_STRING, &apply_ifname,
							DBUS_TYPE_INVALID);
	
	dbus_error_init(&err);
	reply = dbus_connection_send_with_reply_and_block(connection, query, -1, &err);
	dbus_message_unref(query);
	if (!reply) {
		if (dbus_error_is_set(&err)) {
			dbus_error_free(&err);
		}
		return PPPOEERR_EDBUS;
	}

	dbus_message_iter_init(reply, &iter);
	dbus_message_iter_get_basic(&iter, &ret);

	dbus_message_unref(reply);

	return ret;
}

int
pppoe_config_device_service(DBusConnection *connection, 
				unsigned int local_id, unsigned int ins_id, 
				unsigned int dev_id, unsigned int state) {
	DBusMessage *query, *reply;
	DBusError err;
	DBusMessageIter	 iter;
	char dbusName[DBUSNAME_LEN];
	int ret;

	if (!connection)
		return PPPOEERR_EINVAL;

	if (dev_id > DEV_MAX_NUM)
		return PPPOEERR_EINVAL;
	
	insDbusName_init(dbusName, DBUSNAME_LEN, PPPOE_DBUS_DBUSNAME, local_id, ins_id);
	query = dbus_message_new_method_call(dbusName, 
										PPPOE_DBUS_OBJPATH,
										PPPOE_DBUS_INTERFACE,
										PPPOE_DBUS_DEVICE_CONFIG_SERVICE);
	if (!query) 
		return PPPOEERR_ENOMEM;
	
	dbus_message_append_args(query,
							DBUS_TYPE_UINT32, &dev_id,
							DBUS_TYPE_UINT32, &state,
							DBUS_TYPE_INVALID);
	
	dbus_error_init(&err);
	reply = dbus_connection_send_with_reply_and_block(connection, query, -1, &err);
	dbus_message_unref(query);
	if (!reply) {
		if (dbus_error_is_set(&err)) {
			dbus_error_free(&err);
		}
		return PPPOEERR_EDBUS;
	}

	dbus_message_iter_init(reply, &iter);
	dbus_message_iter_get_basic(&iter, &ret);

	dbus_message_unref(reply);

	return ret;
}

int
pppoe_config_pfm_entry(DBusConnection *connection, 
			struct pfm_table_entry *entry, unsigned int state) {
	int ret;

	if (!connection || !entry)
		return PPPOEERR_EINVAL;

	entry->opt = state ? 1 : 0;

	entry->protocol = 1;/*ICMP*/
	ret = pfm_table_entry_config(connection, entry);
	if (ret)
		goto out;

	entry->protocol = 6;/*TCP*/
	ret = pfm_table_entry_config(connection, entry);
	if (ret)
		goto out;
	
	entry->protocol = 17;/*UDP*/
	ret = pfm_table_entry_config(connection, entry);
	if (ret)
		goto out;

out:
	return ret;
}


int
pppoe_show_pfm_entry(DBusConnection *connection, 
							unsigned int local_id, unsigned int ins_id, 
							unsigned int dev_id, struct pfm_table_entry *entry) {
	DBusMessage *query, *reply;
	DBusError err;
	DBusMessageIter	iter;
	char dbusName[DBUSNAME_LEN];
	char *ifname, *srcip, *dstip;
	int ret;

	if (!connection || !entry)
		return PPPOEERR_EINVAL;

	if (!dev_id || dev_id > DEV_MAX_NUM)
		return PPPOEERR_EINVAL;

	memset(entry, 0, sizeof(*entry));
	insDbusName_init(dbusName, DBUSNAME_LEN, PPPOE_DBUS_DBUSNAME, local_id, ins_id);
	query = dbus_message_new_method_call(dbusName, 
										PPPOE_DBUS_OBJPATH,
										PPPOE_DBUS_INTERFACE,
										PPPOE_DBUS_SHOW_PFM_ENTRY);
	if (!query) 
		return PPPOEERR_ENOMEM;

	dbus_message_append_args(query,
							DBUS_TYPE_UINT32, &dev_id,
							DBUS_TYPE_INVALID);
	
	dbus_error_init(&err);	
	reply = dbus_connection_send_with_reply_and_block(connection, query, -1, &err);
	dbus_message_unref(query);

	if (NULL == reply) {
		if(dbus_error_is_set(&err)) {
			dbus_error_free(&err);
		}
		return PPPOEERR_EDBUS;
	}
	
	dbus_message_iter_init(reply, &iter);
	dbus_message_iter_get_basic(&iter, &ret);

	if (PPPOEERR_SUCCESS == ret) {
		dbus_message_iter_next(&iter);  
		dbus_message_iter_get_basic(&iter, &entry->opt_para);

		dbus_message_iter_next(&iter);  
		dbus_message_iter_get_basic(&iter, &ifname);

		dbus_message_iter_next(&iter);  
		dbus_message_iter_get_basic(&iter, &srcip);

		dbus_message_iter_next(&iter);  
		dbus_message_iter_get_basic(&iter, &entry->src_port);

		dbus_message_iter_next(&iter);  
		dbus_message_iter_get_basic(&iter, &dstip);
		
		dbus_message_iter_next(&iter);  
		dbus_message_iter_get_basic(&iter, &entry->dest_port);

		dbus_message_iter_next(&iter);  
		dbus_message_iter_get_basic(&iter, &entry->sendto);

		dbus_message_iter_next(&iter);  
		dbus_message_iter_get_basic(&iter, &entry->slot_id);

		strncpy(entry->ifname, ifname, sizeof(entry->ifname) - 1);
		strncpy(entry->src_ipaddr, srcip, sizeof(entry->src_ipaddr) - 1);
		strncpy(entry->dest_ipaddr, dstip, sizeof(entry->dest_ipaddr) - 1);
	}

	dbus_message_unref(reply);
	return ret;
}


int 
pppoe_show_device_list(DBusConnection *connection, 
						unsigned int local_id, unsigned int ins_id, 
						struct pppoeDevBasicInfo **dev_array, unsigned int *num) {
	DBusMessage *query, *reply;
	DBusError err;
	DBusMessageIter	 iter;
	DBusMessageIter	 iter_array;
	DBusMessageIter	 iter_struct;
	char *ifname, *base_ifname, *dev_desc;
	char dbusName[DBUSNAME_LEN];
	unsigned int ret_num;
	int ret;
	
	if (!connection || !dev_array || !num)
		return PPPOEERR_EINVAL;

	*dev_array = NULL;
	*num = 0;
	
	insDbusName_init(dbusName, DBUSNAME_LEN, PPPOE_DBUS_DBUSNAME, local_id, ins_id);
	query = dbus_message_new_method_call(dbusName, 
										PPPOE_DBUS_OBJPATH,
										PPPOE_DBUS_INTERFACE,
										PPPOE_DBUS_SHOW_DEVICE_LIST);
	if (!query) 
		return PPPOEERR_ENOMEM;

	dbus_error_init(&err);		
	reply = dbus_connection_send_with_reply_and_block(connection, query, -1, &err);
	dbus_message_unref(query);

	if (NULL == reply) {
		if(dbus_error_is_set(&err)) {
			dbus_error_free(&err);
		}
		return PPPOEERR_EDBUS;
	}

	dbus_message_iter_init(reply, &iter);
	dbus_message_iter_get_basic(&iter, &ret);

	dbus_message_iter_next(&iter);	
	dbus_message_iter_get_basic(&iter, &ret_num);

	dbus_message_iter_next(&iter);  
	dbus_message_iter_recurse(&iter,&iter_array);   
    
	if (PPPOEERR_SUCCESS == ret && ret_num){
		*dev_array = (struct pppoeDevBasicInfo *)calloc(ret_num, sizeof(struct pppoeDevBasicInfo));
		if (!(*dev_array)) {
			ret = PPPOEERR_ENOMEM;
			goto out;
		}

		int i = 0;
		for (i = 0; i < ret_num; i++) {

			dbus_message_iter_recurse(&iter_array, &iter_struct);
			dbus_message_iter_get_basic(&iter_struct, &(*dev_array)[i].dev_id);

			dbus_message_iter_next(&iter_struct);
			dbus_message_iter_get_basic(&iter_struct, &(*dev_array)[i].state);

			dbus_message_iter_next(&iter_struct);
			dbus_message_iter_get_basic(&iter_struct, &(*dev_array)[i].ipaddr);

			dbus_message_iter_next(&iter_struct);
			dbus_message_iter_get_basic(&iter_struct, &(*dev_array)[i].mask);

			dbus_message_iter_next(&iter_struct);
			dbus_message_iter_get_basic(&iter_struct, &ifname);

			dbus_message_iter_next(&iter_struct);
			dbus_message_iter_get_basic(&iter_struct, &base_ifname);

			dbus_message_iter_next(&iter_struct);
			dbus_message_iter_get_basic(&iter_struct, &dev_desc);

			strncpy((*dev_array)[i].ifname, ifname, IFNAMSIZ - 1);
			strncpy((*dev_array)[i].base_ifname, base_ifname, IFNAMSIZ - 1);
			strncpy((*dev_array)[i].dev_desc, dev_desc, DEV_DESC_LEN - 1);

			dbus_message_iter_next(&iter_array);
		}
		*num = ret_num;
	}

out:	
	dbus_message_unref(reply);
	return ret;    
}

int 
pppoe_show_online_user(DBusConnection *connection, 
						unsigned int local_id, unsigned int ins_id, unsigned int dev_id,
						struct pppoeUserInfo **userarray, unsigned int *userNum) {
	DBusMessage *query, *reply;
	DBusError err;
	DBusMessageIter	 iter;
	DBusMessageIter	 iter_array;
	DBusMessageIter	 iter_struct;
	char *username;
	char dbusName[DBUSNAME_LEN];
	unsigned int ret_num;
	int ret;
	
	if (!connection || !userarray || !userNum)
		return PPPOEERR_EINVAL;

	if (dev_id > DEV_MAX_NUM)
		return PPPOEERR_EINVAL;

	*userarray = NULL;
	*userNum = 0;
	
	insDbusName_init(dbusName, DBUSNAME_LEN, PPPOE_DBUS_DBUSNAME, local_id, ins_id);
	query = dbus_message_new_method_call(dbusName, 
										PPPOE_DBUS_OBJPATH,
										PPPOE_DBUS_INTERFACE,
										PPPOE_DBUS_SHOW_ONLINE_USER);
	if (!query) 
		return PPPOEERR_ENOMEM;
	
	dbus_message_append_args(query,
							DBUS_TYPE_UINT32, &dev_id,
							DBUS_TYPE_INVALID);
	
	dbus_error_init(&err);	
	reply = dbus_connection_send_with_reply_and_block(connection, query, -1, &err);
	dbus_message_unref(query);

	if (NULL == reply) {
		if(dbus_error_is_set(&err)) {
			dbus_error_free(&err);
		}
		return PPPOEERR_EDBUS;
	}

	dbus_message_iter_init(reply, &iter);
	dbus_message_iter_get_basic(&iter, &ret);

	dbus_message_iter_next(&iter);	
	dbus_message_iter_get_basic(&iter, &ret_num);

	dbus_message_iter_next(&iter);  
	dbus_message_iter_recurse(&iter,&iter_array);   
    
	if (PPPOEERR_SUCCESS == ret && ret_num){
		struct pppoeUserInfo *array;
		int i;
		
		array = (struct pppoeUserInfo *)calloc(ret_num, sizeof(struct pppoeUserInfo));
		if (!array) {
			ret = PPPOEERR_ENOMEM;
			goto out;
		}

		for (i = 0; i < ret_num; i++) {			
			dbus_message_iter_recurse(&iter_array, &iter_struct);
			dbus_message_iter_get_basic(&iter_struct, &array[i].sid);

			dbus_message_iter_next(&iter_struct);
			dbus_message_iter_get_basic(&iter_struct, &array[i].ip);

			dbus_message_iter_next(&iter_struct);
			dbus_message_iter_get_basic(&iter_struct, &array[i].sessTime);

			dbus_message_iter_next(&iter_struct);
			dbus_message_iter_get_basic(&iter_struct, &array[i].mac[0]);

			dbus_message_iter_next(&iter_struct);
			dbus_message_iter_get_basic(&iter_struct, &array[i].mac[1]);

			dbus_message_iter_next(&iter_struct);
			dbus_message_iter_get_basic(&iter_struct, &array[i].mac[2]);

			dbus_message_iter_next(&iter_struct);
			dbus_message_iter_get_basic(&iter_struct, &array[i].mac[3]);

			dbus_message_iter_next(&iter_struct);
			dbus_message_iter_get_basic(&iter_struct, &array[i].mac[4]);

			dbus_message_iter_next(&iter_struct);
			dbus_message_iter_get_basic(&iter_struct, &array[i].mac[5]);

			dbus_message_iter_next(&iter_struct);
			dbus_message_iter_get_basic(&iter_struct, &username);

			dbus_message_iter_next(&iter_array);

			strncpy(array[i].username, username, sizeof(array[i].username) - 1);
		}

		*userarray = array;
		*userNum = ret_num;
	}

out:	
	dbus_message_unref(reply);
	return ret;    
}

void
pppoe_online_user_free(struct pppoeUserInfo **userarray, unsigned int userNum) {
	int i;
	for (i = 0; i < userNum; i++) {
		free(userarray[i]);
		userarray[i] = NULL;
	}
}

int 
pppoe_show_online_user_with_sort(DBusConnection *connection, 
						unsigned int local_id, unsigned int ins_id, unsigned int dev_id,
						struct pppoeUserInfo **userarray, unsigned int size, unsigned int *userNum) {
	DBusMessage *query, *reply;
	DBusError err;
	DBusMessageIter	 iter;
	DBusMessageIter	 iter_array;
	DBusMessageIter	 iter_struct;
	char *username;
	char dbusName[DBUSNAME_LEN];
	unsigned int ret_num;
	unsigned int count = 0;
	int ret;
	
	if (!connection || !userarray || !userNum)
		return PPPOEERR_EINVAL;

	if (dev_id > DEV_MAX_NUM)
		return PPPOEERR_EINVAL;

	*userNum = 0;
	
	insDbusName_init(dbusName, DBUSNAME_LEN, PPPOE_DBUS_DBUSNAME, local_id, ins_id);
	query = dbus_message_new_method_call(dbusName, 
										PPPOE_DBUS_OBJPATH,
										PPPOE_DBUS_INTERFACE,
										PPPOE_DBUS_SHOW_ONLINE_USER);
	if (!query) 
		return PPPOEERR_ENOMEM;
	
	dbus_message_append_args(query,
							DBUS_TYPE_UINT32, &dev_id,
							DBUS_TYPE_INVALID);
	
	dbus_error_init(&err);	
	reply = dbus_connection_send_with_reply_and_block(connection, query, -1, &err);
	dbus_message_unref(query);

	if (NULL == reply) {
		if(dbus_error_is_set(&err)) {
			dbus_error_free(&err);
		}
		return PPPOEERR_EDBUS;
	}

	dbus_message_iter_init(reply, &iter);
	dbus_message_iter_get_basic(&iter, &ret);

	dbus_message_iter_next(&iter);	
	dbus_message_iter_get_basic(&iter, &ret_num);
	if (size < ret_num) {
		ret = PPPOEERR_ENOMEM;
		goto out;
	}

	dbus_message_iter_next(&iter);  
	dbus_message_iter_recurse(&iter,&iter_array);   
    
	if (PPPOEERR_SUCCESS == ret && ret_num){
		int i;
		for (i = 0; i < ret_num; i++) {	
			struct pppoeUserInfo *userinfo
				= (struct pppoeUserInfo *)malloc(sizeof(struct pppoeUserInfo));
			if (!userinfo) {
				pppoe_online_user_free(userarray, count);
				ret = PPPOEERR_ENOMEM;
				goto out;
			}

			memset(userinfo, 0, sizeof(*userinfo));

			dbus_message_iter_recurse(&iter_array, &iter_struct);
			dbus_message_iter_get_basic(&iter_struct, &userinfo->sid);

			if (userinfo_array_insert(userarray, userinfo, count))
				goto next_array;
			
			count++;
			
			dbus_message_iter_next(&iter_struct);
			dbus_message_iter_get_basic(&iter_struct, &userinfo->ip);

			dbus_message_iter_next(&iter_struct);
			dbus_message_iter_get_basic(&iter_struct, &userinfo->sessTime);

			dbus_message_iter_next(&iter_struct);
			dbus_message_iter_get_basic(&iter_struct, &userinfo->mac[0]);

			dbus_message_iter_next(&iter_struct);
			dbus_message_iter_get_basic(&iter_struct, &userinfo->mac[1]);

			dbus_message_iter_next(&iter_struct);
			dbus_message_iter_get_basic(&iter_struct, &userinfo->mac[2]);

			dbus_message_iter_next(&iter_struct);
			dbus_message_iter_get_basic(&iter_struct, &userinfo->mac[3]);

			dbus_message_iter_next(&iter_struct);
			dbus_message_iter_get_basic(&iter_struct, &userinfo->mac[4]);

			dbus_message_iter_next(&iter_struct);
			dbus_message_iter_get_basic(&iter_struct, &userinfo->mac[5]);

			dbus_message_iter_next(&iter_struct);
			dbus_message_iter_get_basic(&iter_struct, &username);
			strncpy(userinfo->username, username, sizeof(userinfo->username) - 1);

		next_array:	
			dbus_message_iter_next(&iter_array);
		}

		*userNum = count;
	}

out:	
	dbus_message_unref(reply);
	return ret;    
}


int
pppoe_detect_device_exist(DBusConnection *connection, 
								unsigned int local_id, 
								unsigned int ins_id,
								unsigned int dev_id) {
	DBusMessage *query, *reply;
	DBusError err;
	DBusMessageIter	 iter;
	char dbusName[DBUSNAME_LEN];
	int ret;

	if (!connection)
		return PPPOEERR_EINVAL;

	if (!dev_id || dev_id > DEV_MAX_NUM)
		return PPPOEERR_EINVAL;
	
	insDbusName_init(dbusName, DBUSNAME_LEN, PPPOE_DBUS_DBUSNAME, local_id, ins_id);
	query = dbus_message_new_method_call(dbusName, 
										PPPOE_DBUS_OBJPATH,
										PPPOE_DBUS_INTERFACE,
										PPPOE_DBUS_DETECT_DEVICE_EXIST);
	if (!query) 
		return PPPOEERR_ENOMEM;
	
	dbus_message_append_args(query,
							DBUS_TYPE_UINT32, &dev_id,
							DBUS_TYPE_INVALID);

	dbus_error_init(&err);
	reply = dbus_connection_send_with_reply_and_block(connection, query, -1, &err);
	dbus_message_unref(query);
	if (!reply) {
		if (dbus_error_is_set(&err)) {
			dbus_error_free(&err);
		}
		return PPPOEERR_EDBUS;
	}

	dbus_message_iter_init(reply, &iter);
	dbus_message_iter_get_basic(&iter, &ret);

	dbus_message_unref(reply);

	return ret;
}

int
pppoe_config_device_ipaddr(DBusConnection *connection, 
								unsigned int local_id, unsigned int ins_id, 
								unsigned int dev_id, unsigned int ipaddr, unsigned int mask) {
	DBusMessage *query, *reply;
	DBusError err;
	DBusMessageIter	 iter;
	char dbusName[DBUSNAME_LEN];
	int ret;

	if (!connection)
		return PPPOEERR_EINVAL;

	if (!dev_id || dev_id > DEV_MAX_NUM)
		return PPPOEERR_EINVAL;

	if ((!ipaddr && mask) || (ipaddr && !mask))
		return PPPOEERR_EINVAL;

	if (ipaddr && !(ipaddr & 0xff000000))
		return PPPOEERR_EINVAL;

	if (mask > 32)
		return PPPOEERR_EINVAL;
	
	insDbusName_init(dbusName, DBUSNAME_LEN, PPPOE_DBUS_DBUSNAME, local_id, ins_id);
	query = dbus_message_new_method_call(dbusName, 
										PPPOE_DBUS_OBJPATH,
										PPPOE_DBUS_INTERFACE,
										PPPOE_DBUS_DEVICE_CONFIG_IPADDR);
	if (!query) 
		return PPPOEERR_ENOMEM;
	
	dbus_message_append_args(query,
							DBUS_TYPE_UINT32, &dev_id,
							DBUS_TYPE_UINT32, &ipaddr,
							DBUS_TYPE_UINT32, &mask,
							DBUS_TYPE_INVALID);
	
	dbus_error_init(&err);
	reply = dbus_connection_send_with_reply_and_block(connection, query, -1, &err);
	dbus_message_unref(query);
	if (!reply) {
		if (dbus_error_is_set(&err)) {
			dbus_error_free(&err);
		}
		return PPPOEERR_EDBUS;
	}
	
	dbus_message_iter_init(reply, &iter);
	dbus_message_iter_get_basic(&iter, &ret);

	dbus_message_unref(reply);

	return ret;	
}

int
pppoe_config_device_virtual_mac(DBusConnection *connection, 
									unsigned int local_id, unsigned int ins_id, 
									unsigned int dev_id, unsigned char *virtualMac) {
	DBusMessage *query, *reply;
	DBusError err;
	DBusMessageIter	 iter;
	char dbusName[DBUSNAME_LEN];
	unsigned char mac[ETH_ALEN];
	int ret;

	if (!connection)
		return PPPOEERR_EINVAL;

	if (!dev_id || dev_id > DEV_MAX_NUM)
		return PPPOEERR_EINVAL;

	if (virtualMac) {
		memcpy(mac, virtualMac, sizeof(mac));
	} else {
		memset(mac, 0, sizeof(mac));
	}
	
	insDbusName_init(dbusName, DBUSNAME_LEN, PPPOE_DBUS_DBUSNAME, local_id, ins_id);
	query = dbus_message_new_method_call(dbusName, 
										PPPOE_DBUS_OBJPATH,
										PPPOE_DBUS_INTERFACE,
										PPPOE_DBUS_DEVICE_CONFIG_VIRTUAL_MAC);
	if (!query) 
		return PPPOEERR_ENOMEM;
	
	dbus_message_append_args(query,
							DBUS_TYPE_UINT32, &dev_id,
							DBUS_TYPE_BYTE, &mac[0],
							DBUS_TYPE_BYTE, &mac[1],
							DBUS_TYPE_BYTE, &mac[2],
							DBUS_TYPE_BYTE, &mac[3],
							DBUS_TYPE_BYTE, &mac[4],
							DBUS_TYPE_BYTE, &mac[5],							
							DBUS_TYPE_INVALID);
	
	dbus_error_init(&err);
	reply = dbus_connection_send_with_reply_and_block(connection, query, -1, &err);
	dbus_message_unref(query);
	if (!reply) {
		if (dbus_error_is_set(&err)) {
			dbus_error_free(&err);
		}
		return PPPOEERR_EDBUS;
	}
	
	dbus_message_iter_init(reply, &iter);
	dbus_message_iter_get_basic(&iter, &ret);

	dbus_message_unref(reply);

	return ret;	
}

int
pppoe_config_session_ipaddr(DBusConnection *connection, 
								unsigned int local_id, unsigned int ins_id, 
								unsigned int dev_id, unsigned int minIP, unsigned int maxIP) {

	DBusMessage *query, *reply;
	DBusError err;
	DBusMessageIter	 iter;
	char dbusName[DBUSNAME_LEN];
	int ret;

	if (!connection)
		return PPPOEERR_EINVAL;

	if (!dev_id || dev_id > DEV_MAX_NUM)
		return PPPOEERR_EINVAL;

	if ((minIP > maxIP) || (!minIP && maxIP))
		return PPPOEERR_EINVAL;

	insDbusName_init(dbusName, DBUSNAME_LEN, PPPOE_DBUS_DBUSNAME, local_id, ins_id);
	query = dbus_message_new_method_call(dbusName, 
										PPPOE_DBUS_OBJPATH,
										PPPOE_DBUS_INTERFACE,
										PPPOE_DBUS_DEVICE_CONFIG_SESSION_IPADDR);
	if (!query) 
		return PPPOEERR_ENOMEM;
	
	dbus_message_append_args(query,
							DBUS_TYPE_UINT32, &dev_id,
							DBUS_TYPE_UINT32, &minIP,
							DBUS_TYPE_UINT32, &maxIP,
							DBUS_TYPE_INVALID);
	
	dbus_error_init(&err);
	reply = dbus_connection_send_with_reply_and_block(connection, query, -1, &err);
	dbus_message_unref(query);
	if (!reply) {
		if (dbus_error_is_set(&err)) {
			dbus_error_free(&err);
		}
		return PPPOEERR_EDBUS;
	}
	
	dbus_message_iter_init(reply, &iter);
	dbus_message_iter_get_basic(&iter, &ret);

	dbus_message_unref(reply);

	return ret;	
}


int
pppoe_config_session_dns(DBusConnection *connection, 
								unsigned int local_id, unsigned int ins_id, 
								unsigned int dev_id, unsigned int dns1, unsigned int dns2) {

	DBusMessage *query, *reply;
	DBusError err;
	DBusMessageIter	 iter;
	char dbusName[DBUSNAME_LEN];
	int ret;

	if (!connection)
		return PPPOEERR_EINVAL;

	if (!dev_id || dev_id > DEV_MAX_NUM)
		return PPPOEERR_EINVAL;

	if (!dns1 && dns2)
		return PPPOEERR_EINVAL;

	insDbusName_init(dbusName, DBUSNAME_LEN, PPPOE_DBUS_DBUSNAME, local_id, ins_id);
	query = dbus_message_new_method_call(dbusName, 
										PPPOE_DBUS_OBJPATH,
										PPPOE_DBUS_INTERFACE,
										PPPOE_DBUS_DEVICE_CONFIG_SESSION_DNS);
	if (!query) 
		return PPPOEERR_ENOMEM;
	
	dbus_message_append_args(query,
							DBUS_TYPE_UINT32, &dev_id,
							DBUS_TYPE_UINT32, &dns1,
							DBUS_TYPE_UINT32, &dns2,
							DBUS_TYPE_INVALID);
	
	dbus_error_init(&err);
	reply = dbus_connection_send_with_reply_and_block(connection, query, -1, &err);
	dbus_message_unref(query);
	if (!reply) {
		if (dbus_error_is_set(&err)) {
			dbus_error_free(&err);
		}
		return PPPOEERR_EDBUS;
	}
	
	dbus_message_iter_init(reply, &iter);
	dbus_message_iter_get_basic(&iter, &ret);

	dbus_message_unref(reply);

	return ret;	
}


int
pppoe_config_nas_ipaddr(DBusConnection *connection, 
								unsigned int local_id, unsigned int ins_id, 
								unsigned int dev_id, unsigned int ipaddr) {

	DBusMessage *query, *reply;
	DBusError err;
	DBusMessageIter	 iter;
	char dbusName[DBUSNAME_LEN];
	int ret;

	if (!connection)
		return PPPOEERR_EINVAL;

	if (!dev_id || dev_id > DEV_MAX_NUM)
		return PPPOEERR_EINVAL;

	insDbusName_init(dbusName, DBUSNAME_LEN, PPPOE_DBUS_DBUSNAME, local_id, ins_id);
	query = dbus_message_new_method_call(dbusName, 
										PPPOE_DBUS_OBJPATH,
										PPPOE_DBUS_INTERFACE,
										PPPOE_DBUS_DEVICE_CONFIG_NAS_IPADDR);
	if (!query) 
		return PPPOEERR_ENOMEM;
	
	dbus_message_append_args(query,
							DBUS_TYPE_UINT32, &dev_id,
							DBUS_TYPE_UINT32, &ipaddr,
							DBUS_TYPE_INVALID);
	
	dbus_error_init(&err);
	reply = dbus_connection_send_with_reply_and_block(connection, query, -1, &err);
	dbus_message_unref(query);
	if (!reply) {
		if (dbus_error_is_set(&err)) {
			dbus_error_free(&err);
		}
		return PPPOEERR_EDBUS;
	}
	
	dbus_message_iter_init(reply, &iter);
	dbus_message_iter_get_basic(&iter, &ret);

	dbus_message_unref(reply);

	return ret;	
}

int
pppoe_config_radius_rdc(DBusConnection *connection, 
					unsigned int local_id, unsigned int ins_id,
					unsigned int dev_id, unsigned int state, 
					unsigned int s_slotid, unsigned int s_insid) {
	DBusMessage *query, *reply;
	DBusError err;
	DBusMessageIter	 iter;
	char dbusName[DBUSNAME_LEN];
	int ret;

	if (!connection)
		return PPPOEERR_EINVAL;
	
	insDbusName_init(dbusName, DBUSNAME_LEN, PPPOE_DBUS_DBUSNAME, local_id, ins_id);
	query = dbus_message_new_method_call(dbusName, 
										PPPOE_DBUS_OBJPATH,
										PPPOE_DBUS_INTERFACE,
										PPPOE_DBUS_DEVICE_CONFIG_RADIUS_RDC);
	if (!query) 
		return PPPOEERR_ENOMEM;

	dbus_message_append_args(query,
							DBUS_TYPE_UINT32, &dev_id,
							DBUS_TYPE_UINT32, &state,
							DBUS_TYPE_UINT32, &s_slotid,
							DBUS_TYPE_UINT32, &s_insid,
							DBUS_TYPE_INVALID);

	dbus_error_init(&err);
	reply = dbus_connection_send_with_reply_and_block(connection, query, -1, &err);
	dbus_message_unref(query);
	if (!reply) {
		if (dbus_error_is_set(&err)) {
			dbus_error_free(&err);
		}
		return PPPOEERR_EDBUS;
	}

	dbus_message_iter_init(reply, &iter);
	dbus_message_iter_get_basic(&iter, &ret);

	dbus_message_unref(reply);

	return ret;
}


int
pppoe_config_radius_server(DBusConnection *connection, 
								unsigned int local_id, unsigned int ins_id, 
								unsigned int dev_id, struct radius_srv *srv) {

	DBusMessage *query, *reply;
	DBusError err;
	DBusMessageIter	 iter;
	char dbusName[DBUSNAME_LEN];
	char *auth_secret = NULL, 
		*acct_secret = NULL, 
		*backup_auth_secret = NULL,
		*backup_acct_secret = NULL;
	int ret;

	if (!connection)
		return PPPOEERR_EINVAL;

	if (!dev_id || dev_id > DEV_MAX_NUM)
		return PPPOEERR_EINVAL;

	if (!srv)
		return PPPOEERR_EINVAL;

	if ((!srv->auth.ip && srv->acct.ip) || 
		(srv->auth.ip && !srv->acct.ip)) {
		return PPPOEERR_EINVAL;
	}

	if ((!srv->backup_auth.ip && srv->backup_acct.ip) || 
		(srv->backup_auth.ip && !srv->backup_acct.ip)) {
		return PPPOEERR_EINVAL;
	}

	if (!srv->auth.ip) {
		auth_secret = "";
	} else {
		auth_secret = srv->auth.secret;
	}

	if (!srv->acct.ip) {
		acct_secret = "";
	} else {
		acct_secret = srv->acct.secret;
	}

	if (!srv->backup_auth.ip) {
		backup_auth_secret = "";
	} else {
		backup_auth_secret = srv->backup_auth.secret;
	}

	if (!srv->backup_acct.ip) {
		backup_acct_secret = "";
	} else {
		backup_acct_secret = srv->backup_acct.secret;
	}

	insDbusName_init(dbusName, DBUSNAME_LEN, PPPOE_DBUS_DBUSNAME, local_id, ins_id);
	query = dbus_message_new_method_call(dbusName, 
										PPPOE_DBUS_OBJPATH,
										PPPOE_DBUS_INTERFACE,
										PPPOE_DBUS_DEVICE_CONFIG_RADIUS_SERVER);
	if (!query) 
		return PPPOEERR_ENOMEM;
	
	dbus_message_append_args(query,
							DBUS_TYPE_UINT32, &dev_id,
							DBUS_TYPE_UINT32, &srv->auth.ip,
							DBUS_TYPE_UINT32, &srv->auth.port,
							DBUS_TYPE_UINT32, &srv->auth.secretlen,
							DBUS_TYPE_STRING, &auth_secret,
							DBUS_TYPE_UINT32, &srv->acct.ip,
							DBUS_TYPE_UINT32, &srv->acct.port,
							DBUS_TYPE_UINT32, &srv->acct.secretlen,
							DBUS_TYPE_STRING, &acct_secret,
							DBUS_TYPE_UINT32, &srv->backup_auth.ip,
							DBUS_TYPE_UINT32, &srv->backup_auth.port,
							DBUS_TYPE_UINT32, &srv->backup_auth.secretlen,
							DBUS_TYPE_STRING, &backup_auth_secret,
							DBUS_TYPE_UINT32, &srv->backup_acct.ip,
							DBUS_TYPE_UINT32, &srv->backup_acct.port,
							DBUS_TYPE_UINT32, &srv->backup_acct.secretlen,
							DBUS_TYPE_STRING, &backup_acct_secret,
							DBUS_TYPE_INVALID);
	
	dbus_error_init(&err);
	reply = dbus_connection_send_with_reply_and_block(connection, query, -1, &err);
	dbus_message_unref(query);
	if (!reply) {
		if (dbus_error_is_set(&err)) {
			dbus_error_free(&err);
		}
		return PPPOEERR_EDBUS;
	}
	
	dbus_message_iter_init(reply, &iter);
	dbus_message_iter_get_basic(&iter, &ret);

	dbus_message_unref(reply);

	return ret;	
}


int
pppoe_config_service_name(DBusConnection *connection, 
									unsigned int local_id, unsigned int ins_id, 
									unsigned int dev_id, char *sname) {

	DBusMessage *query, *reply;
	DBusError err;
	DBusMessageIter	 iter;
	char dbusName[DBUSNAME_LEN];
	int ret;

	if (!connection)
		return PPPOEERR_EINVAL;

	if (!dev_id || dev_id > DEV_MAX_NUM)
		return PPPOEERR_EINVAL;

	if (!sname) {
		sname = "";
	} else if (strlen(sname) > (PPPOE_NAMELEN - 1)) {
		return PPPOEERR_EINVAL;
	}

	insDbusName_init(dbusName, DBUSNAME_LEN, PPPOE_DBUS_DBUSNAME, local_id, ins_id);
	query = dbus_message_new_method_call(dbusName, 
										PPPOE_DBUS_OBJPATH,
										PPPOE_DBUS_INTERFACE,
										PPPOE_DBUS_DEVICE_CONFIG_SNAME);
	if (!query) 
		return PPPOEERR_ENOMEM;

	dbus_message_append_args(query,
							DBUS_TYPE_UINT32, &dev_id,
							DBUS_TYPE_STRING, &sname,
							DBUS_TYPE_INVALID);
	
	dbus_error_init(&err);
	reply = dbus_connection_send_with_reply_and_block(connection, query, -1, &err);
	dbus_message_unref(query);
	if (!reply) {
		if (dbus_error_is_set(&err)) {
			dbus_error_free(&err);
		}
		return PPPOEERR_EDBUS;
	}
	
	dbus_message_iter_init(reply, &iter);
	dbus_message_iter_get_basic(&iter, &ret);

	dbus_message_unref(reply);

	return ret;	
}

int
pppoe_device_kick_user(DBusConnection *connection, 
							unsigned int local_id, unsigned int ins_id, 
							unsigned int dev_id, unsigned int sid, unsigned char *mac) {

	DBusMessage *query, *reply;
	DBusError err;
	DBusMessageIter	 iter;
	char dbusName[DBUSNAME_LEN];
	unsigned char tmp[ETH_ALEN] = { 0 };
	int ret;

	if (!connection)
		return PPPOEERR_EINVAL;

	if (!dev_id || dev_id > DEV_MAX_NUM)
		return PPPOEERR_EINVAL;

	if (!sid && !mac)
		return PPPOEERR_EINVAL;

	mac = mac ? : tmp;
	
	insDbusName_init(dbusName, DBUSNAME_LEN, PPPOE_DBUS_DBUSNAME, local_id, ins_id);
	query = dbus_message_new_method_call(dbusName, 
										PPPOE_DBUS_OBJPATH,
										PPPOE_DBUS_INTERFACE,
										PPPOE_DBUS_DEVICE_KICK_USER);
	if (!query) 
		return PPPOEERR_ENOMEM;

	dbus_message_append_args(query,
							DBUS_TYPE_UINT32, &dev_id,
							DBUS_TYPE_UINT32, &sid,
							DBUS_TYPE_BYTE,	&mac[0],
							DBUS_TYPE_BYTE,	&mac[1],
							DBUS_TYPE_BYTE,	&mac[2],
							DBUS_TYPE_BYTE,	&mac[3],
							DBUS_TYPE_BYTE,	&mac[4],
							DBUS_TYPE_BYTE,	&mac[5],
							DBUS_TYPE_INVALID);

	dbus_error_init(&err);
	reply = dbus_connection_send_with_reply_and_block(connection, query, -1, &err);
	dbus_message_unref(query);
	if (!reply) {
		if (dbus_error_is_set(&err)) {
			dbus_error_free(&err);
		}
		return PPPOEERR_EDBUS;
	}
	
	dbus_message_iter_init(reply, &iter);
	dbus_message_iter_get_basic(&iter, &ret);

	dbus_message_unref(reply);

	return ret;	
}

int
pppoe_show_running_config(DBusConnection *connection, 
			unsigned int local_id, unsigned int ins_id, char **configCmd) {
	DBusMessage *query, *reply;
	DBusError err;
	DBusMessageIter	 iter;
	DBusMessageIter	 iter_array;
	DBusMessageIter	 iter_struct;
	char dbusName[DBUSNAME_LEN];
	char *cmd, *cursor;
	unsigned int num, length;
	int ret;

	if (!connection || !configCmd)
		return PPPOEERR_EINVAL;

	*configCmd = NULL;
	
	insDbusName_init(dbusName, DBUSNAME_LEN, PPPOE_DBUS_DBUSNAME, local_id, ins_id);
	query = dbus_message_new_method_call(dbusName, 
										PPPOE_DBUS_OBJPATH,
										PPPOE_DBUS_INTERFACE,
										PPPOE_DBUS_SHOW_RUNNING_CONFIG);
	if (!query) 
		return PPPOEERR_ENOMEM;

	dbus_error_init(&err);	
	reply = dbus_connection_send_with_reply_and_block(connection, query, -1, &err);
	dbus_message_unref(query);

	if (NULL == reply) {
		if(dbus_error_is_set(&err)) {
			dbus_error_free(&err);
		}
		return PPPOEERR_EDBUS;
	}
	
	dbus_message_iter_init(reply, &iter);
	dbus_message_iter_get_basic(&iter, &ret);

	dbus_message_iter_next(&iter);	
	dbus_message_iter_get_basic(&iter, &num);

	dbus_message_iter_next(&iter);  
	dbus_message_iter_recurse(&iter,&iter_array);   

	if (PPPOEERR_SUCCESS == ret && num){
		int i;

		*configCmd = (char *)calloc(num, CONFIGCMD_SIZE);
		if (!(*configCmd)) {
			ret = PPPOEERR_ENOMEM;
			goto out;
		}
		
		cursor = *configCmd;
		for (i = 0; i < num; i++) {
			dbus_message_iter_recurse(&iter_array, &iter_struct);
			dbus_message_iter_get_basic(&iter_struct, &cmd);

			length = strlen(cmd);
			if (length > (num * CONFIGCMD_SIZE - (cursor - (*configCmd)) - 1)) {
				free(*configCmd);
				*configCmd = NULL;
				ret = PPPOEERR_ENOMEM;
				goto out;
			}
			
			memcpy(cursor, cmd, length);
			cursor += length;
			
			dbus_message_iter_next(&iter_array);
		}	
	}
	
out:
	dbus_message_unref(reply);
	return ret;
}

#endif /* !_VERSION_18SP7_ */
