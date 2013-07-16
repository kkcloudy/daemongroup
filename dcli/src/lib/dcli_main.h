#ifndef __DCLI_MAIN_H__
#define __DCLI_MAIN_H__


#define VLANID_RANGE_MIN			1
#define VLANID_RANGE_MAX			4094
/*macro for distributed flag, add by caojia*/
#define DISTRIBUTED_SYSTEM 1
#define NON_DISTRIBUTED_SYSTEM 0

#define SERVICE_STR "Configuring service."

extern DBusConnection *dcli_dbus_connection;
extern DBusConnection *config_trap_dbus_connection ;
#ifdef DISTRIBUT
extern DBusConnection *dcli_dbus_connection_local;
extern DBusConnection *dcli_dbus_connection_remote;
#endif

#if (defined _D_WCPSS_|| defined _D_CC_)
extern int distributFag;
extern int HostSlotId;
extern int is_active_master;
extern int active_master_slot;
extern int is_master;
#endif
__inline__ int parse_slotport_no(char *str,unsigned char *slotno,unsigned char *portno);
__inline__ int parse_vlan_no(char* str,unsigned short* vlanId); 
void dcli_config_write(char * str, int local,int slotID, int hansiID,int opened, int closed);
void cli_syslog_info(char *format,...); //fengwenchao add 20111031
struct dbus_connection
{
	DBusConnection *dcli_dbus_connection;
	int 			slot_id;
	int 			board_type;
	int 			board_state;
	};
typedef struct dbus_connection dbus_connection;

extern dbus_connection *dbus_connection_dcli[];
void dbus_error_free_for_dcli(DBusError *error);
int dcli_dbus_init_remote(void);

#endif
