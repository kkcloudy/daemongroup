#ifndef _VERSION_18SP7_
#ifndef _PPPOE_DBUS_INTERFACE_H
#define _PPPOE_DBUS_INTERFACE_H


int string_to_uint(const char *s, unsigned int *i);

int ifname_detect_exist(const char *str);

void insDbusName_init(char *new_name, unsigned int new_len,
			char *name, unsigned int local_id, unsigned int ins_id);

void insIfName_init(char *ifname, 
		unsigned int slot_id, unsigned int local_id, 
		unsigned int ins_id, unsigned int pppoe_id);
	


int pppoe_config_log_debug(DBusConnection *connection, 
						unsigned int local_id, unsigned int ins_id,
						unsigned int type, unsigned int state);

int pppoe_config_log_token(DBusConnection *connection, 
						unsigned int local_id, unsigned int ins_id,
						PPPOELogToken token, unsigned int state);

int pppoe_config_device_create(DBusConnection *connection, 
				unsigned int local_id, unsigned int ins_id, 
				unsigned int dev_id, char *ifname, char *dev_desc);

int pppoe_config_device_destroy(DBusConnection *connection, 
				unsigned int local_id, unsigned int ins_id, unsigned int dev_id);

int pppoe_config_device_base(DBusConnection *connection, 
									unsigned int local_id, unsigned int ins_id, 
									unsigned int dev_id, char *base_ifname);

int pppoe_config_device_apply(DBusConnection *connection, 
									unsigned int local_id, unsigned int ins_id, 
									unsigned int dev_id, char *apply_ifname);

int pppoe_config_device_service(DBusConnection *connection, 
									unsigned int local_id, unsigned int ins_id, 
									unsigned int dev_id, unsigned int state);

int pppoe_config_pfm_entry(DBusConnection *connection, 
				struct pfm_table_entry *entry, unsigned int state);

int pppoe_show_pfm_entry(DBusConnection *connection, 
							unsigned int local_id, unsigned int ins_id, 
							unsigned int dev_id, struct pfm_table_entry *entry);

void pppoe_online_user_free(struct pppoeUserInfo **userarray, unsigned int userNum);
int pppoe_show_online_user_with_sort(DBusConnection *connection, 
						unsigned int local_id, unsigned int ins_id, unsigned int dev_id,
						struct pppoeUserInfo **userarray, unsigned int size, unsigned int *userNum);

int pppoe_show_online_user(DBusConnection *connection, 
						unsigned int local_id, unsigned int ins_id, unsigned int dev_id, 
						struct pppoeUserInfo **userarray, unsigned int *userNum);

int pppoe_show_device_list(DBusConnection *connection, 
						unsigned int local_id, unsigned int ins_id,
						struct pppoeDevBasicInfo **dev_array, unsigned int *num);

int pppoe_detect_device_exist(DBusConnection *connection, 
								unsigned int local_id, 
								unsigned int ins_id,
								unsigned int dev_id);


int pppoe_config_device_ipaddr(DBusConnection *connection, 
								unsigned int local_id, unsigned int ins_id, 
								unsigned int dev_id, unsigned int ipaddr, unsigned int mask);

int pppoe_config_device_virtual_mac(DBusConnection *connection, 
								unsigned int local_id, unsigned int ins_id, 
								unsigned int dev_id, unsigned char *virtualMac);

int pppoe_config_session_ipaddr(DBusConnection *connection, 
								unsigned int local_id, unsigned int ins_id, 
								unsigned int dev_id, unsigned int minIP, unsigned int maxIP);

int pppoe_config_session_dns(DBusConnection *connection, 
								unsigned int local_id, unsigned int ins_id, 
								unsigned int dev_id, unsigned int dns1, unsigned int dns2);

int pppoe_config_nas_ipaddr(DBusConnection *connection, 
								unsigned int local_id, unsigned int ins_id, 
								unsigned int dev_id, unsigned int ipaddr);

int pppoe_config_radius_rdc(DBusConnection *connection, 
								unsigned int local_id, unsigned int ins_id,
								unsigned int dev_id, unsigned int state, 
								unsigned int s_slotid, unsigned int s_insid);

int pppoe_config_radius_server(DBusConnection *connection, 
								unsigned int local_id, unsigned int ins_id, 
								unsigned int dev_id, struct radius_srv *srv);

int pppoe_config_service_name(DBusConnection *connection, 
									unsigned int local_id, unsigned int ins_id, 
									unsigned int dev_id, char *sname);

int pppoe_device_kick_user(DBusConnection *connection, 
								unsigned int local_id, unsigned int ins_id, 
								unsigned int dev_id, unsigned int sid, unsigned char *mac);

int pppoe_show_running_config(DBusConnection *connection, 
			unsigned int local_id, unsigned int ins_id, char **configCmd);

#endif /* !_PPPOE_DBUS_INTERFACE_H */
#endif /* !_VERSION_18SP7_ */