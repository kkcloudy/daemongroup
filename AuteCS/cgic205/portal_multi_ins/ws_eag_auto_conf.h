#define MAX_NAS_TYPE_LEN 			32
#define MAX_NAS_TART_LEN 			32
#define MAX_NAS_END_LEN 			32
#define MAX_NASID_TYPE_LEN		16
#define MAX_NASID_VALUE_LEN		32


#define DBUS_SUCCESS				0
#define ERR_DBUS_UNKNOW			-1	//未知错误
#define ERR_DBUS_BUS_GET			-2  //dbus_bus_get失败返回NULL
#define ERR_DBUS_REQUEST_NAME		-3  //dbus_bus_request_name "DBUS_REQUEST_NAME_REPLY_PRIMARY_OWNER != ret"
#define ERR_DBUS_METHOD_CALL		-4	//dbus_message_new_method_call error
#define ERR_DBUS_OUT_OF_MEMORY	-5	//Out Of Memory
#define ERR_DBUS_PENDINF_CALL		-6  //pending Call
#define ERR_DBUS_REPLY_NULL		-7	//Reply Null

#define EAG_AURO_SERVER		"eag.conf.server"
#define EAG_AURO_CONF_OBJECT	"/eag/conf/object"

// interface to call on
#define INTERFACE_AURO_NAS			"interface.conf.nas"
#define INTERFACE_AURO_VLANMAPING	"interface.conf.vlanmaping"
#define INTERFACE_AURO_DEBUG_FILTER	"interface.conf.debug_filter"
#define INTERFACE_AURO_DEBUG_FILTER_SHOW	"interface.conf.debug_filter_show"

// method name
#define METHOD_NAS             "method_nas"
#define METHOD_VLANMAPING     "method_vlanmaping"
#define METHOD_DEBUG_FILTER             "method_debug_filter"
#define METHOD_DEBUG_FILTERS_SHOW	"method_show_all_debug_filters"
#define METHOD_VLAN_NASPORTID   	"method_vlan_nasportid_map"

enum {
	DEBUG_FILTER_ADD = 1,
	DEBUG_FILTER_DEL,
	DEBUG_FILTER_CLEAN,
	DEBUG_FILTER_SHOW,
}DEBUG_FILTER_OP;

enum {
	DEBUG_FILTER_IP = 1,
	DEBUG_FILTER_MAC,
	DEBUG_FILTER_USERNAME,
	DEBUG_FILTER_ALL,
}DEBUG_FILTER_TYPE;

#define FLAG_EDIT		0
#define FLAG_ADD		1
#define FLAG_DELETE	2

#define FLAG_DEFAULT		1
#define FLAG_NOT_DEFAULT	0

/*NAS动态配置*/
typedef struct {
	int n_flag;//标记位，2表示删除,1表示addnasid，0表示editnasid
	int n_default;//1代表默认，0表示非默认
	int n_plotid;//nas策略
	char n_nastype[MAX_NAS_TYPE_LEN];//类型
	char n_start[MAX_NAS_TART_LEN];//起始
	char n_end[MAX_NAS_END_LEN];//结束
	char n_nasid[MAX_NASID_VALUE_LEN];//nasid
	char n_syntaxis[MAX_NASID_VALUE_LEN];//汇聚点	
	char n_attz[128];
}dbus_nas_conf;


/*wlan wtp  - nasportid  map */
typedef struct {
	int v_flag;			/*2 :delete  1 :add，0 :edit*/
	int v_strategy_id;		 /*nasportid map policy ID*/
	char v_wlan_begin_id[10];		/*wlan start id*/
	char v_wlan_end_id[10];		/*wlan end id*/
	char v_wtp_begin_id[10];		/*wtp start id*/
	char v_wtp_end_id[10];		/*wtp end id*/
	char nasportid[10];			/*nasportid*/
	char v_attz[128];				/*XML node attribute*/
}dbus_vlan_conf;

/*vlan-id  nasportid map*/
typedef struct {
	int  n_flag;		 		/*2 :delete  1 :add，0 :edit*/
	int  n_strategy_id;		 /*nasportid map policy ID*/
	char vlan_begin_id[10];	/*begin of vlanid  */
	char vlan_end_id[10];		/*end of vlanid*/
	char nasportid[10];		/*nasportid*/
	char n_attz[128];			/*XML node attribute*/
}dbus_vlan_nasportid_conf;

/* add/del debug-filter 动态配置*/
typedef struct {
	int status;/* debug-filter type*/
	int strategy_id;/* 策略ID */
	int key;/* 'ip'=1,'mac'=2,'username'=3 or 'all'=4 */
	char value[20];
}dbus_debug_filter_conf;

typedef struct {
	int strategy_id;
	char list[1024];
}dbus_debug_filter_list;


extern int ccgi_dbus_eag_show_debug_filter(DBusConnection *conn,
										int strategy_id,
										dbus_debug_filter_list *dbug_filter_list);
extern int ccgi_dbus_eag_conf_debug_filter(DBusConnection *conn, 
										dbus_debug_filter_conf debug_filter_conf);
extern int ccgi_dbus_eag_conf_nas(DBusConnection *conn, dbus_nas_conf eag_conf);
extern int ccgi_dbus_eag_conf_vlanmaping(DBusConnection *conn, dbus_vlan_conf vlan_conf);
