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
*	RCSfile		:  eag_iphash.h
*
*	Author		:  fanyiming
*
*	Revision	:  1.00
*
*	Date		:  2011-12-20
********************************************************************************/

#ifndef _EAG_IPHASH
#define _EAG_IPHASH

#include <sys/types.h>
#include <netdb.h>

#define IP_SET_PROTOCOL_VERSION	4	/* ipset hash protocol version */
#define SO_IP_SET		83	/* socket protocol */

#define LIST_TRIES 		5
#define IP_SET_MAXNAMELEN 	32
#define MAX_CAPTIVE_ID		16
#define IP_SET_MAX_BINDINGS	6

typedef uint16_t ip_set_id_t;
typedef uint32_t ip_set_ip_t;
typedef uint64_t ip_sizet;

#if 0
enum exittype {
	OTHER_PROBLEM = 1,
	PARAMETER_PROBLEM,
	VERSION_PROBLEM
};
#endif

union ip_set_name_index {
	char name[IP_SET_MAXNAMELEN];
	ip_set_id_t index;
};
/*****************************kernel defined***********************************/
/* Get set index by name */
#define IP_SET_OP_GET_BYNAME	0x00000006
struct ip_set_req_get_set {
	unsigned op;
	unsigned version;
	union ip_set_name_index set;
};
/* Get set and type */
#define IP_SET_OP_ADT_GET	0x00000010
struct ip_set_req_adt_get {
	unsigned op;
	unsigned version;
	union ip_set_name_index set;
	char typename[IP_SET_MAXNAMELEN];
};

#define IP_SET_OP_ADD_IP	0x00000101	/* Add an IP to a set */
#define IP_SET_OP_DEL_IP	0x00000102	/* Remove an IP from a set */
#define IP_SET_OP_TEST_IP	0x00000103	/* Test an IP in a set */

/* Get max_sets and the index of a queried set */
#define IP_SET_OP_MAX_SETS	0x00000020
struct ip_set_req_max_sets {
	unsigned op;
	unsigned version;
	ip_set_id_t max_sets;		/* max_sets */
	ip_set_id_t sets;		/* real number of sets */
	union ip_set_name_index set;	/* index of set if name used */
};


/* Get the id and name of the sets plus size for next step */
#define IP_SET_OP_LIST_SIZE	0x00000201
struct ip_set_req_setnames {
	unsigned op;
	ip_set_id_t index;		/* set to list/save */
	ip_sizet size;			/* size to get setdata/bindings */
	/* followed by sets number of struct ip_set_name_list */
};

struct ip_set_name_list {
	char name[IP_SET_MAXNAMELEN];
	char typename[IP_SET_MAXNAMELEN];
	ip_set_id_t index;
	ip_set_id_t id;
};

/* The actual list operation */
#define IP_SET_OP_LIST		0x00000203
struct ip_set_req_list {
	unsigned op;
	ip_set_id_t index;
};

struct ip_set_list {
	ip_set_id_t index;
	ip_set_id_t binding;
	u_int32_t ref;
	ip_sizet header_size;	/* Set header data of header_size */
	ip_sizet members_size;	/* Set members data of members_size */
	ip_sizet bindings_size;	/* Set bindings data of bindings_size */
};

struct ip_set_req_iphash {
	ip_set_ip_t ip;
};

struct ip_set_req_adt {
	unsigned op;
	ip_set_id_t index;
};
/******************************************************************************/
/* linux/netfilter_ipv4/ipt_set.h */
struct ipt_set_info {
	ip_set_id_t index;
	u_int32_t flags[IP_SET_MAX_BINDINGS + 1];
};

/* match info */
struct ipt_set_info_match {
	struct ipt_set_info match_set;
};

struct ipt_set_info_target {
	struct ipt_set_info add_set;
	struct ipt_set_info del_set;
};
/******************************************************************************/

/* Description:	Get ips of set_name 	
 * Input:	Name of set 
 * Output:	A pointer is used by saving ips  
 * Ruturn:	Success:0
 * 注意:	用完ips以后要释放	
 */
uint32_t list_members(const char *set_name, ip_set_ip_t **ips);

/* Description:	Add an ip to a set
 * Input:	Set_name and ip
 * Output:	None	
 * Ruturn:	Success:0
 */
int add_ip_to_iphashset(const char *set_name, const ip_set_ip_t ip);

/* Description:	Del an ip from a set		
 * Input:	Set_name and ip	
 * Output:	None
 * Ruturn:	Success:0
 */
int del_ip_from_iphashset(const char *set_name, const ip_set_ip_t ip);

int add_user_to_set(const int user_id, const int hansitype,
		const ip_set_ip_t user_ip);

int del_user_from_set(const int user_id, const int hansitype,
		const ip_set_ip_t user_ip);

int add_preauth_user_to_set(const int user_id, const int hansitype,
		const ip_set_ip_t user_ip);

int del_preauth_user_from_set(const int user_id, const int hansitype,
		const ip_set_ip_t user_ip);

ip_set_id_t eag_get_set_byname(const char *setname);

int eag_ipset_init();

int eag_ipset_exit();

#endif
