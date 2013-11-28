#ifndef _AC_MANAGE_INTERFACE_H_
#define _AC_MANAGE_INTERFACE_H_


int check_snmp_name(char *name, int flag);

int manage_get_slot_id_by_ifname(const char *ifname);

unsigned char set_bit(unsigned char num, unsigned char position);

unsigned char clear_bit(unsigned char num, unsigned char position);

unsigned char get_bit(unsigned char num, unsigned char position);

int string_to_trap_group_switch(const char *switch_string, struct trap_group_switch *group_switch);


int snmp_cllection_mode(DBusConnection *connection);

int ac_manage_config_log_debug(DBusConnection *connection, unsigned int state);

int ac_manage_config_token_debug(DBusConnection *connection, unsigned int token, unsigned int state);

int ac_manage_config_snmp_log_debug(DBusConnection *connection, unsigned int debugLevel); 

int ac_manage_config_trap_log_debug(DBusConnection *connection, unsigned int debugLevel);

int ac_manage_show_snmp_log_debug(DBusConnection *connection, unsigned int *debugLevel);

int ac_manage_show_trap_log_debug(DBusConnection *connection, unsigned int *debugLevel);

int ac_manage_config_snmp_manual_instance(DBusConnection *connection, 
                                                        unsigned int local_id, 
                                                        unsigned int instance_id, 
                                                        unsigned int status);

int ac_manage_show_snmp_manual_instance(DBusConnection *connection, unsigned int *manual_instance);

int ac_manage_config_snmp_service(DBusConnection *connection, unsigned int state);

int ac_manage_config_snmp_collection_mode(DBusConnection *connection, unsigned int port);

int ac_manage_config_snmp_pfm_requestpkts(DBusConnection *connection, 
                                                        char *ifName,
                                                        unsigned int port, 
                                                        unsigned int state);

int ac_manage_config_snmp_pfm_requestpkts_ipv6(DBusConnection *connection, 
                                                    char *ifName, 
                                                    unsigned int port,
                                                    unsigned int state);

int ac_manage_config_snmp_version_mode(DBusConnection *connection, unsigned int version, unsigned int state);

int ac_manage_config_snmp_update_sysinfo(DBusConnection *connection);

int ac_manage_config_snmp_add_community(DBusConnection *connection, STCommunity *community_node);

int ac_manage_config_snmp_add_community_ipv6(DBusConnection *connection, IPV6STCommunity *communityIPV6Node);

int ac_manage_config_snmp_set_community(DBusConnection *connection, char *communityName, STCommunity *community_node);

int ac_manage_config_snmp_set_community_ipv6(DBusConnection *connection,char *communityName,IPV6STCommunity *community_node) ;

int ac_manage_config_snmp_del_community(DBusConnection *connection, char *community);

int ac_manage_config_snmp_del_community_ipv6(DBusConnection *connection, char *community);

int ac_manage_config_snmp_view(DBusConnection *connection, char *view_name, unsigned int mode);

int ac_manage_check_snmp_view(DBusConnection *connection, char *view_name);

int ac_manage_config_snmp_view_oid(DBusConnection *connection, 
                                                  char *view_name,
                                                  char *oid,
                                                  unsigned int oid_type,
                                                  unsigned int mode);
                                          
int ac_manage_config_snmp_add_group(DBusConnection *connection, STSNMPGroup *group_node);
                                                 
int ac_manage_config_snmp_del_group(DBusConnection *connection, char *group_name);

int ac_manage_config_snmp_add_v3user(DBusConnection *connection, STSNMPV3User *v3user_node);                                               

int ac_manage_config_snmp_del_v3user(DBusConnection *connection, char *v3user_name);





int ac_manage_show_snmp_state(DBusConnection *connection, unsigned int *snmp_state);

int ac_manage_show_snmp_base_info(DBusConnection *connection, STSNMPSysInfo *snmp_info);


int ac_manage_show_snmp_pfm_interface(DBusConnection *connection, 
                                                    SNMPINTERFACE **interface_array,
                                                    unsigned int *interface_num,
                                                    unsigned int *snmp_port);

int ac_manage_show_snmp_pfm_interface_ipv6(DBusConnection *connection, 
                                                SNMPINTERFACE **interface_array,
                                                unsigned int *interface_num,
                                                unsigned int *snmp_port) ;
                                            
int ac_manage_show_snmp_community(DBusConnection *connection, STCommunity **community_array, unsigned int *community_num);

int ac_manage_show_snmp_community_ipv6(DBusConnection *connection, IPV6STCommunity **community_array,unsigned int *community_num);

void free_ac_manage_show_snmp_view(STSNMPView **view_array, unsigned int view_num);

int ac_manage_show_snmp_view(DBusConnection *connection, 
                                        STSNMPView **view_array,
                                        unsigned int *view_num,
                                        char *view_name);

int ac_manage_show_snmp_group(DBusConnection *connection, 
                                            STSNMPGroup **group_array,
                                            unsigned int *group_num,
                                            char *group_name);

int ac_manage_show_snmp_v3user(DBusConnection *connection, 
                                            STSNMPV3User **v3user_array,
                                            unsigned int *v3user_num,
                                            char *v3user_name);





int ac_manage_config_trap_service(DBusConnection *connection, unsigned int state);

int ac_manage_config_trap_config_receiver(DBusConnection *connection, STSNMPTrapReceiver *receiver, unsigned int type); //type (1 add;  0 set)
                                                    
int ac_manage_config_trap_del_receiver(DBusConnection *connection, char *name);

int ac_manage_config_trap_switch(DBusConnection *connection, unsigned int index, TRAP_DETAIL_CONFIG *trapDetail);

int ac_manage_config_trap_instance_heartbeat(DBusConnection *connection, TRAPHeartbeatIP *heartbeatNode);

int ac_manage_clear_trap_instance_heartbeat(DBusConnection *connection, unsigned int local_id, unsigned int instance_id);

int ac_manage_config_trap_parameter(DBusConnection *connection, char *paraStr, unsigned int paraData);

int ac_manage_show_trap_state(DBusConnection *connection, unsigned int *trap_state);

int ac_manage_show_trap_parameter(DBusConnection *connection, 
                                                TRAPParameter **parameter_array,
                                                unsigned int *parameter_num);

int ac_manage_show_trap_receiver(DBusConnection *connection, 
                                            STSNMPTrapReceiver **receiver_array,
                                            unsigned int *receiver_num);

int ac_manage_show_trap_switch(DBusConnection *connection, 
                                            TRAP_DETAIL_CONFIG **trapDetail_array,
                                            unsigned int *trapDetail_num);

int ac_manage_config_trap_group_switch(DBusConnection *connection, struct trap_group_switch
*group_switch);
                                            
int ac_manage_show_trap_instance_heartbeat(DBusConnection *connection, 
                                                        TRAPHeartbeatIP **heartbeat_array,
                                                        unsigned int *heartbeat_num);



int ac_manage_manual_set_mib_acif_stats(DBusConnection *connection, struct mib_acif_stats *acif_node);

int ac_manage_show_mib_acif_stats(DBusConnection *connection, 
                                            struct mib_acif_stats **acif_array,
                                            unsigned int *acif_num);

int ac_manage_web_edit(DBusConnection *, void *data,  int );

int ac_manage_web_show(DBusConnection *, struct webHostHead*, unsigned int *, unsigned int *);

int ac_manage_web_conf(DBusConnection *, int);

int ac_manage_web_download(DBusConnection *, char **);

int ac_manage_web_show_pages(DBusConnection *, char **, int *);

int ac_manage_web_del_pages(DBusConnection *, const char *);

int  ac_manage_config_ntp_pfm_requestpkts(DBusConnection *connection, char *ifName,char *ipstr, unsigned int state);

int ac_manage_config_snmp_sysoid_boardtype(DBusConnection *connection, unsigned int sysoid);


#endif
