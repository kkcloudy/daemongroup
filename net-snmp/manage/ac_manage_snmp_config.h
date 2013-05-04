#ifndef _AC_MANAGE_SNMP_CONFIG_H_
#define _AC_MANAGE_SNMP_CONFIG_H_


#define DEFAULT_SNMPD_CACHETIME			300
#define DEFAULT_TRAP_HEARTBEAT_INTERVAL	30
#define DEFAULT_TRAP_HEARTBEAT_MODE		0
#define DEFAULT_TRAP_RESEND_INTERVAL		300
#define DEFAULT_TRAP_RESEND_TIMES			0

typedef struct snmp_summary_s{
	char				info[SNMP_ENGINE_MAX_INFO_LEN];
	int 				version;
	
	STSNMPSysInfo		snmp_sysinfo;
	
	unsigned int        interface_num;
	SNMPINTERFACE       *interfaceHead;
	 
	unsigned int 		community_num;	
	STCommunity			*communityHead;
	
	unsigned int		v3user_num;
	STSNMPV3User		*v3userHead;

	unsigned int 		group_num;
	STSNMPGroup			*groupHead;
	
	unsigned int 	    view_num;
	STSNMPView			*viewHead;
}STSNMPSummary;


typedef struct trap_summary_s {

    unsigned int        receiver_num;
    STSNMPTrapReceiver  *receiverHead;
    
    unsigned int        trapDetailNUM;
    TRAP_DETAIL_CONFIG  *trapDetail;

    unsigned int        trapParaNUM;
    TRAPParameter       *trapParameter;

    unsigned int        heartbeatIP_num;
    TRAPHeartbeatIP     *heartbeatIPHead;

}STTRAPSummary;


extern TRAP_DETAIL_CONFIG ALL_TRAP[];


int snmp_show_running_config(struct running_config **configHead, unsigned int type);

int snmp_manual_set_instance_status(unsigned int local_id, unsigned int instance_id, unsigned int status);

int snmp_show_manual_set_instance_master(unsigned int *instance_state);

int snmp_show_service_state(void);

int snmp_show_sysinfo(STSNMPSysInfo **snmp_info);

int snmp_show_pfm_interface(SNMPINTERFACE **interface_array, unsigned int *interface_num);

int snmp_show_community(STCommunity **community_array, unsigned int *community_num) ;

void free_snmp_show_view(STSNMPView **view_array, unsigned int view_num);

int snmp_show_view(STSNMPView **view_array, unsigned int *view_num, char *view_name);

int snmp_show_group(STSNMPGroup **group_array, unsigned int *group_num, char *group_name);

int snmp_show_v3user(STSNMPV3User **v3user_array, unsigned int *v3user_num, char *v3user_name);



void update_snmp_sysinfo(void);

int snmp_config_service(unsigned int service_state);

int snmp_set_sysinfo(STSNMPSysInfo *snmp_info);

void snmp_clear_pfm_interface(void);

int snmp_add_pfm_interface(char *ifName, unsigned int port);

int snmp_del_pfm_interface(char *ifName, unsigned int port);

int snmp_add_community(STCommunity pstCommunity);

int snmp_del_community(char *community);

int snmp_set_community(char *old_community, STCommunity pstCommunity);

int snmp_create_view(char *view_name);

int snmp_check_view(char *view_name);

int snmp_add_view_oid(char *view_name, char *oid, unsigned int mode);

int snmp_del_view_oid(char *view_name, char *oid, unsigned int mode);

int snmp_del_view(char *view_name);

int snmp_add_group(char *group_name, char *view_name, unsigned int access_mode, unsigned int sec_level);

int snmp_del_group(char *group_name);

int snmp_add_v3user(char *v3user_name, 
                            unsigned int auth_protocal, 
                            char *ath_passwd, 
                            unsigned int priv_protocal,
                            char *priv_passwd,
                            char *group_name,
                            unsigned int state);
                            
int snmp_del_v3user(char *v3user_name);


int trap_show_service_state(void);

int trap_config_service(unsigned int service_state);

int trap_add_receiver(STSNMPTrapReceiver *receiver);

int trap_set_receiver(STSNMPTrapReceiver *receiver);

int trap_del_receiver(char *receiverName);

int trap_set_switch(unsigned int trapIndex, char *trapName, char *trapEDes, unsigned int state);

int trap_set_group_switch(struct trap_group_switch *group_switch);

int trap_set_heartbeat_ip(TRAPHeartbeatIP *heartbeatIP);

int trap_clear_heartbeat_ip(unsigned int local_id, unsigned int instance_id);

int trap_set_parameter(char *paraStr, unsigned int data);


int trap_show_receiver(STSNMPTrapReceiver **receiver_array, unsigned int *receiver_num);

int trap_show_switch(TRAP_DETAIL_CONFIG **trapDetail_array, unsigned int *trapDetail_num);

int trap_show_heartbeat_ip(TRAPHeartbeatIP **heartbeat_array, unsigned int *heartbeat_num);

int trap_show_parameter(TRAPParameter **parameter_array, unsigned int *parameter_num);

int trap_set_debug_level(unsigned int debugLevel);

int trap_show_debug_level(unsigned int *debugLevel);

void config_snmp_sysoid_boardtype(unsigned int sysoid);





void init_snmp_config(void);
void uninit_snmp_config(void);
void init_trap_config(void);


#endif
