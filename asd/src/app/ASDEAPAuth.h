#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <assert.h>
#include <arpa/inet.h>
#include <sys/errno.h>
#include <sys/wait.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/stat.h>

#include "asd.h"
#include "wcpss/asd/asd.h"

#define IPTABLES_LOCK_FILE	"/var/run/eag_iptables_lock"

#define IP_SET_OP_GET_BYNAME	0x00000006
#define IP_SET_PROTOCOL_VERSION	4	/* ipset hash protocol version */
#define SO_IP_SET		83	/* socket protocol */

#define IP_SET_MAXNAMELEN 	32

//eap auth type
#define EAP_AUTH_IPTABLE	1
#define EAP_AUTH_IPSET		0

//IP OP
#define IP_SET_OP_ADD_IP	0x00000101	
#define IP_SET_OP_DEL_IP	0x00000102	
//#define IP_SET_OP_TEST_IP	0x00000103	

#define IP_SET_OP_MAX_SETS	0x00000020

#define LOCK_FILE_MODE		(S_IRUSR|S_IWUSR|S_IRGRP|S_IWGRP|S_IROTH|S_IWOTH)

typedef uint16_t ip_set_id_t;
typedef uint32_t ip_set_ip_t;

struct eap_nmp_mutex {
	int fd;
	char filename[128];
};

typedef struct eap_nmp_mutex eap_nmp_mutex_t;

union ip_set_name_index {
	char name[IP_SET_MAXNAMELEN];
	ip_set_id_t index;
};

struct ip_set_req_get_set {
	unsigned op;
	unsigned version;
	union ip_set_name_index set;
};
struct ip_set_req_iphash {
	ip_set_ip_t ip;
};
struct ip_set_req_adt {
	unsigned op;
	ip_set_id_t index;
};
struct eap_auth {
	int hansi_type;
	int hansi_id;
	int auth_type;
	char hansi_info[16];

	int (*do_auth) (const char *, const uint32_t);
	int (*undo_auth) (const char *, uint32_t);
};
int eap_auth_init();
int eap_auth_exit();
int eap_clean_all_user();
int eap_connect_up(uint32_t userip);
int eap_connect_down(uint32_t userip);

