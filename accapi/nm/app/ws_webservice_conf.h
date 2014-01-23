#ifndef  WS_WEBSERVICE_CONF_H
#define  WS_WEBSERVICE_CONF_H

#define MAX_VHOST_NUM       8
#define MAX_INTERFACE_NUM   4

#define PFM_DBUS_BUSNAME				"pfm.daemon"
#define PFM_DBUS_OBJPATH				"/pfm/daemon"
#define PFM_DBUS_INTERFACE				"pfm.daemon"
#define PFM_DBUS_METHOD_PFM_TABLE 	"pfm_maintain_table"
#define PFM_DBUS_METHOD_PFM_DEAL_SERVICE 	"pfm_deal_service"


#define LOG(format, args...) syslog(LOG_DEBUG,"%s:%d:%s -> " format "\n", __FILE__, __LINE__, __func__, ##args)

/*
 * error code definition 
 */
enum{
	WEB_FAILURE = -1,
	WEB_SUCCESS,
	WEB_DUPLICATE,
	WEB_OVERMAX,
	WEB_EMPTY,
    WEB_RUNNING,
	WEB_NOTFOUND,
	WEB_ENABLE,
	WEB_DISABLE,
	WEB_NO_PERMISSION,
	WEB_CONN_REF,
	WEB_DIR_EMPRY,
    WEB_EXISIT,
    WEB_IP_PORT_ERROR,
    WEB_NAME_EXISIT,
    WEB_PORTAL_ERROR
};

/* 
 *service control definition 	
 */
#define WEB_START 					1
#define WEB_STOP					2	
#define PORTAL_START				3	
#define PORTAL_STOP					4	

/* 
 *service type definition  
 */
#define HTTP_SERVICE  			    1	
#define HTTPS_SERVICE  			 	2	
#define PORTAL_SERVICE              3
#define PORTAL_HTTP_SERVICE  	    4
#define PORTAL_HTTPS_SERVICE  	    5

#define HOST_ADD                    1
#define HOST_DEL                    2
#define IFNAME_ADD                  3 
#define IFNAME_DEL                  4

/* 
 *service status definition  
 */
#define WEB_SERVICE_ENABLE 			(1<<0L) 
#define PORTAL_SERVICE_ENABLE 		(1<<1L)

/* 
 * config file definition 
 */
#define PORTS_CONF  				"/etc/apache2/ports.conf"
#define SITES_AVALIB_DEFAULT 		"/etc/apache2/sites-available/default"
#define SITES_AVALIB_DEFDIGEST		"/etc/apache2/sites-available/defaultdigest"
#define SITES_AVALIB_SSLDEF			"/etc/apache2/sites-available/ssldefault"
#define SITES_AVALIB_SSLDEFGEST		"/etc/apache2/sites-available/ssldefaultdigest"
#define SITES_AVALIB_PORTAL 		"/etc/apache2/sites-available/portal"
#define SITES_AVALIB_PORTALNORAMAL 	"/etc/apache2/sites-available/portalnormal"
#define SITES_AVALIB_PORTALSSL 		"/etc/apache2/sites-available/portalssl"

#define SITES_ENABEL				"/etc/apache2/sites-enabled"
#define SITES_ENABEL_DEFAULT 		"/etc/apache2/sites-enabled/default"
#define SITES_ENABEL_SSLDEF			"/etc/apache2/sites-enabled/ssldefault"
#define SITES_ENABEL_PORTAL 		"/etc/apache2/sites-enabled/portal"
#define SITES_ENABEL_PORTALNORMAL 	"/etc/apache2/sites-enabled/portalnormal"
#define SITES_ENABEL_PORTALSSL 		"/etc/apache2/sites-enabled/portalssl"

#define SEM_IS_DISTRIBUTED_PATH   "/dbm/product/is_distributed"
#define SEM_LOCAL_SLOT_ID_PATH    "/dbm/local_board/slot_id"
#define SEM_SLOT_COUNT_PATH       "/dbm/product/slotcount"
#define SEM_PRODUCT_TYPE_PATH     "/dbm/product/product_type"
#define SEM_MASTER_SLOT_COUNT_PATH "/dbm/product/master_slot_count"
#define SEM_ACTIVE_MASTER_SLOT_ID_PATH "/dbm/product/active_master_slot_id"



/*
 * List definitions.
 */
#define	LINK_HEAD(name, type)						\
struct name {										\
	struct type *lh_first;	/* first element */		\
}

#define	LINK_HEAD_INITIALIZER(head)					\
	{ NULL }

#define	LINK_ENTRY(type)							\
struct {											\
	struct type *le_next;	/* next element */		\
	struct type **le_prev;	/* address of previous next element */	\
}

/*
 * List functions.
 */
#define	LINK_INIT(head) do {						\
	(head)->lh_first = NULL;						\
} while (/*CONSTCOND*/0)

#define	LINK_INSERT_AFTER(listelm, elm, field) do {			\
	if (((elm)->field.le_next = (listelm)->field.le_next) != NULL)	\
		(listelm)->field.le_next->field.le_prev =			\
		    &(elm)->field.le_next;							\
	(listelm)->field.le_next = (elm);						\
	(elm)->field.le_prev = &(listelm)->field.le_next;		\
} while (/*CONSTCOND*/0)

#define	LINK_INSERT_BEFORE(listelm, elm, field) do {		\
	(elm)->field.le_prev = (listelm)->field.le_prev;		\
	(elm)->field.le_next = (listelm);						\
	*(listelm)->field.le_prev = (elm);						\
	(listelm)->field.le_prev = &(elm)->field.le_next;		\
} while (/*CONSTCOND*/0)

#define	LINK_INSERT_HEAD(head, elm, field) do {				\
	if (((elm)->field.le_next = (head)->lh_first) != NULL)	\
		(head)->lh_first->field.le_prev = &(elm)->field.le_next;\
	(head)->lh_first = (elm);								\
	(elm)->field.le_prev = &(head)->lh_first;				\
} while (/*CONSTCOND*/0)

#define	LINK_REMOVE(elm, field) do {						\
	if ((elm)->field.le_next != NULL)						\
		(elm)->field.le_next->field.le_prev = 				\
		    (elm)->field.le_prev;							\
	*(elm)->field.le_prev = (elm)->field.le_next;			\
} while (/*CONSTCOND*/0)

#define	LINK_FOREACH(var, head, field)						\
	for ((var) = ((head)->lh_first);						\
		(var);												\
		(var) = ((var)->field.le_next))

/*
 * List access methods.
 */
#define	LINK_EMPTY(head)		((head)->lh_first == NULL)
#define	LINK_FIRST(head)		((head)->lh_first)
#define	LINK_NEXT(elm, field)		((elm)->field.le_next)

/*
 * Person access definition.
 */
#define WEB_TYPE_INVALID  "",0,0,""

#define WEB_WEBIFNAME_SIZE (sizeof(struct w_if))
LINK_HEAD(webIfHead,w_if);
typedef struct w_if
{	
    char *name;
    char *ifname;
    unsigned int slot;
    unsigned int opt;
	LINK_ENTRY(w_if) entries;
}*webIfPtr,webIf;

#define WEB_WEBHOST_SIZE (sizeof(struct w_host))
LINK_HEAD(webHostHead,w_host);
typedef struct w_host
{	
    char *name;
	char *address;
	unsigned int port;
	unsigned int type;	
    unsigned int count;
    struct webIfHead head;
	LINK_ENTRY(w_host) entries;
}*webHostPtr,webHost;

#define WEB_WEBINFO_SIZE (sizeof(struct w_info))
LINK_HEAD(webInfoHead, w_info);
typedef struct w_info
{
	unsigned int server_stat;
	unsigned int portal_flag;
	unsigned int slotid;
	struct webHostHead head;
	LINK_ENTRY(w_info) entries;
}*webInfoPtr,webInfo;

#define MAX_INTERNAL_PORTAL 4
#define WEB_WEBDIR_SIZE (sizeof(struct w_dir))
LINK_HEAD(webDirHead, w_dir);
typedef struct w_dir{
	char *dir[MAX_INTERNAL_PORTAL];
	unsigned int slot;
	unsigned int count;
	LINK_ENTRY(w_dir) entries;
}*webDirPtr, webDir; 


struct web_info{
	char *name;
	int type;
	char *address;
	int port;
	char *infname;
	struct web_info *next;
};




extern int ccgi_set_interval_portal_cmd(char *name, char *type, char *ip_addr, char *port, char *hansi);
extern int ccgi_enable_interval_portal_service_cmd();
extern int ccgi_add_http_https_ip_port_cmd(char *name,char *type,char *ip_addr, char *port);
extern int ccgi_add_web_forword_cmd(char *webname, char *infname);
extern int ccgi_show_webservice_info_cmd(struct web_info *WtpIfHead,int *num,int *slot);
extern int ccgi_show_interval_portalservice_info_cmd(struct web_info *WtpIfHead,int *num,int slot);
extern int ccgi_delete_portal_config_cmd(char *name);
int ccgi_delete_http_https_config_cmd(char *name);
extern int ccgi_contrl_disable_webservice_cmd();
extern int ccgi_contrl_enable_webservice_cmd();
extern void ccgi_free_show_webservice_info_cmd(struct web_info *WtpIfHead);
extern void ccgi_free_show_interval_portalservice_info_cmd(struct web_info *WtpIfHead);

#endif /* WS_WEBSERVICE_CONF_H */
