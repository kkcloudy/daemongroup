#ifdef __cplusplus
extern "C"
{
#endif

#include <string.h>
#include <dbus/dbus.h>
#include <stdbool.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <fcntl.h>
#include <net/if.h>
#include <omapip/omapip_p.h>
#include <syslog.h>
#include <linux/rtnetlink.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <sys/ioctl.h>
#include <linux/rtnetlink.h>
#include <bits/sockaddr.h>

#include "pppoe_snp_dbus.h"
#include "pppoe_snp_netlink.h"

#include "sysdef/returncode.h"
#include "dbus/dhcp/dhcp_dbus_def.h"

#include "dhcpd.h"

unsigned int pppoe_snp_enable_flag = 0;
unsigned int dba_server_enable_flag = 0;
struct pppoe_snp_cfg *pppoe_snp_cfg_head = NULL;

char *pppoe_snp_opcode_to_string(unsigned int opcode)
{
	switch (opcode) {
		case PPPoE_SNP_RETURN_CODE_SUCCESS:
			return "";
		case PPPoE_SNP_RETURN_CODE_FAIL:
			return "%% failed";
		case PPPoE_SNP_RETURN_CODE_NO_SUCH_INTERFACE:
			return "%% No such interface";
		case PPPoE_SNP_RETURN_CODE_INVALID_MRU:
			return "%% Invalid MRU, range 60 - 1492 bytes.";			
		default :
			return "%% failed";

	}
	return NULL;
}

struct pppoe_snp_cfg * pppoe_snp_cfg_find_by_ifname(char *ifname)
{
	struct pppoe_snp_cfg *tmp = NULL;
	
	if (!ifname) {
		return NULL;
	}

	tmp = pppoe_snp_cfg_head;
	for (; tmp; tmp = tmp->next) {
		log_debug("pppoe snp find cfg by ifname %s, tmp ifnmae %s \n", ifname, tmp->ifname);
		if (!strncmp(tmp->ifname, ifname, IFNAMSIZ-1)) {
			break;
		}
	}
	
	return tmp;
}

int pppoe_snp_iface_add(struct pppoe_snp_cfg *iface)
{
	struct pppoe_snp_cfg *node = NULL;
	
	if (!iface) {
		return -1;
	}
	
	node = (struct pppoe_snp_cfg *)malloc(sizeof(struct pppoe_snp_cfg));
	if (!node) {
		log_error("malloc pppoe cfg failed: %m.\n");		
		return -1;
	}
	
	memset(node, 0, sizeof(*node));
	node->mru = iface->mru;
	memcpy(node->ifname, iface->ifname, IFNAMSIZ-1);

	if (!pppoe_snp_cfg_head) {
		pppoe_snp_cfg_head = node;
		node->next = NULL;
	} else {
		node->next = pppoe_snp_cfg_head;
		pppoe_snp_cfg_head = node;
	}

    return 0;
}


int pppoe_snp_iface_del(char *ifname)
{
	struct pppoe_snp_cfg *tmp = NULL;
	struct pppoe_snp_cfg *next = NULL, *prev = NULL;

	if (!pppoe_snp_cfg_head) {
		return -1;
	}
	
	if (!strncmp(pppoe_snp_cfg_head->ifname, ifname, IFNAMSIZ-1)) {
		tmp = pppoe_snp_cfg_head;
		pppoe_snp_cfg_head = tmp->next;
		free(tmp);
		return 0;
	}
	
	prev = pppoe_snp_cfg_head;
	next = pppoe_snp_cfg_head->next;
	while (next) {
		if (!strncmp(next->ifname, ifname, IFNAMSIZ-1)) {
			prev->next = next->next;
			free(next);
			return 0;
		}
		prev = next;
		next = next->next;
	}

    return -1;
}

/**********************************************************************************
 * pppoe_snp_set_udf_flag
 *		set interface user define flag
 *
 *	INPUT:
 *		char *ifname
 *		unsigned int flag
 *		unsigned int set_flag	-set or clear
 *
 *	OUTPUT:
 *		NULL
 *
 *	RETURN:
 *		0		- success
 *	 	-1		- fail
 **********************************************************************************/
int pppoe_snp_set_udf_flag
(
	char *ifname,
	unsigned int flag,
	unsigned int set_flag
)
{
	int fd = -1;
	struct ifreq tmp;

	if(!ifname) {
		log_error("pppoe snp set interface udf flag filed, parameters null!\n");
		return -1;
	}
	
	if ((fd = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP)) < 0) {
		log_error("dhcp set udf flag failed: %m\n");
		return -1;
	}
	
	memset(&tmp, 0, sizeof(tmp));
	strcpy(tmp.ifr_name, ifname);
	if (ioctl(fd, SIOCGIFUDFFLAGS, &tmp) < 0) {
		log_error("ioctl get interface %s udf flags failed: %m!\n", ifname);
		close(fd);
		return -1;
	}
	log_debug("ioctl get interface %s udf flags %x.\n", ifname, tmp.ifr_flags);
	
	if (set_flag) {
		tmp.ifr_flags |= flag;		
		if (ioctl(fd, SIOCSIFUDFFLAGS, &tmp) < 0) {
			log_error("%% ioctl set interface %s udf flags failed: %m!\n", ifname);
			close(fd);
			return -1;
		}
	} else {
		tmp.ifr_flags &= ~flag;
		if (ioctl(fd, SIOCSIFUDFFLAGS, &tmp) < 0) {
			log_error("ioctl set interface %s udf flags failed: %m!\n", ifname);
			close(fd);
			return -1;
		}
	}
	log_debug("ioctl %s interface %s %x flag.\n", set_flag ? "SET" : "CLR", ifname, flag);
	
	close(fd);
	
	return 0;
}


DBusMessage * 
pppoe_snp_dbus_enable
(	
	DBusConnection *conn, 
	DBusMessage *msg, 
	void *user_data
)
{
	DBusMessage *reply = NULL;
	DBusMessageIter	iter;
	DBusError err;
	unsigned int enable = 0;
	unsigned int op_ret = 0;

	dbus_error_init(&err);
	
	if (!(dbus_message_get_args(msg, &err,
			DBUS_TYPE_UINT32, &enable,
			DBUS_TYPE_INVALID))) {
		log_error("Unable to get input args ");
		if (dbus_error_is_set(&err)) {
			log_error("%s raised: %s",err.name,err.message);
			dbus_error_free(&err);
		}
		return NULL;
	}

	if (pppoe_snp_server_enable(enable)) {
		op_ret = PPPoE_SNP_RETURN_CODE_FAIL;
	} else {
		pppoe_snp_enable_flag = enable;
		op_ret = PPPoE_SNP_RETURN_CODE_SUCCESS;			
	}

	log_info("pppoe snp is %s.\n", (pppoe_snp_enable_flag) ? "enalbe" : "disable");

	reply = dbus_message_new_method_return(msg);
	dbus_message_iter_init_append (reply, &iter);	
	dbus_message_iter_append_basic (&iter, DBUS_TYPE_UINT32, &op_ret);
	
	return reply;	
}


DBusMessage * 
dhcp_pppoe_snooping_iface_enable
(	
	DBusConnection *conn, 
	DBusMessage *msg, 
	void *user_data
)
{
	DBusMessage *reply = NULL;
	DBusMessageIter iter;
	DBusError err;
	struct pppoe_snp_cfg *tmp = NULL;
	struct pppoe_snp_cfg node;
	char *ifname = NULL;
	unsigned int enable = 0;
	unsigned short mru = 0;
	unsigned int op_ret = 0;

	dbus_error_init(&err);
	
	if (!(dbus_message_get_args(msg, &err,
				DBUS_TYPE_STRING, &ifname, 
				DBUS_TYPE_UINT32, &enable,
				DBUS_TYPE_UINT16, &mru,
				DBUS_TYPE_INVALID))) {
		log_error("Unable to get input args ");
		if (dbus_error_is_set(&err)) {
			log_error("%s raised: %s",err.name,err.message);
			dbus_error_free(&err);
		}
		return NULL;
	}

	log_info("pppoe snp cfg: ifname %s %s mru %d\n", ifname, enable ? "enable" : "disable", mru);

	if (!if_nametoindex(ifname)) {
		op_ret = PPPoE_SNP_RETURN_CODE_NO_SUCH_INTERFACE;
		goto out;
	}

	if (mru && ((mru < MRU_MIN) || (mru > MRU_MAX))) {
		op_ret = PPPoE_SNP_RETURN_CODE_INVALID_MRU;
		goto out;
	}
	
	if (enable) {

		tmp = pppoe_snp_cfg_find_by_ifname(ifname);
		if (tmp) {
			log_debug("pppoe snp cfg: find ifname %s, update mru %d\n", ifname, mru);
			
			tmp->mru = mru;
		} else {
			memset((char *)&node, 0, sizeof(node));
			strncpy(node.ifname, ifname, IFNAMSIZ-1);
			node.mru = mru;
			pppoe_snp_iface_add(&node);
		}

		if (pppoe_snp_set_udf_flag(ifname, IFF_PPP_SNP, SET_FLAG)) {
			op_ret = PPPoE_SNP_RETURN_CODE_FAIL;
		} else if (pppoe_snp_netlink_mru_msg(mru)) {
			op_ret = PPPoE_SNP_RETURN_CODE_FAIL;
		}

	} else {
		if (pppoe_snp_cfg_find_by_ifname(ifname)) {
			pppoe_snp_iface_del(ifname);
		}
		if (pppoe_snp_set_udf_flag(ifname, IFF_PPP_SNP, CLR_FLAG)) {
			op_ret = PPPoE_SNP_RETURN_CODE_FAIL;
		}
	}
out:
	reply = dbus_message_new_method_return(msg);
	dbus_message_iter_init_append (reply, &iter);
	dbus_message_iter_append_basic (&iter, DBUS_TYPE_UINT32, &op_ret);
	
	return reply;
}


DBusMessage*
pppoe_snp_dbus_set_debug_state
(	
	DBusConnection *conn, 
	DBusMessage *msg, 
	void *user_data
)
{
	DBusMessage *reply = NULL;
	DBusMessageIter	 iter;
	unsigned int set_flag = 0;
	unsigned int debug_type = 0;
	unsigned int op_ret = 0;
	DBusError err;

	dbus_error_init(&err);

	if (!(dbus_message_get_args(msg, &err,
						DBUS_TYPE_UINT32, &set_flag,
						DBUS_TYPE_UINT32, &debug_type,
						DBUS_TYPE_INVALID))) {
		log_error("while set_debug_state,unable to get input args");
		if (dbus_error_is_set(&err)) {
			 log_error("%s raised: %s",err.name,err.message);
			dbus_error_free(&err);
		}
		return NULL;
	}	
		
	if (pppoe_snp_netlink_debug_msg(set_flag, debug_type)) {
		op_ret = PPPoE_SNP_RETURN_CODE_FAIL;
	}
	log_debug("pppoe snp %s debug %x \n", set_flag ? "SET" : "CLR", debug_type);
	
	
	reply = dbus_message_new_method_return(msg);
	dbus_message_iter_init_append (reply, &iter);
	dbus_message_iter_append_basic (&iter, DBUS_TYPE_UINT32, &op_ret);

	return reply;
}


DBusMessage * 
dba_dbus_server_enable
(	
	DBusConnection *conn, 
	DBusMessage *msg, 
	void *user_data
)
{
	DBusMessage *reply = NULL;
	DBusMessageIter	iter;
	DBusError err;
	unsigned int enable = 0;
	unsigned int op_ret = 0;

	dbus_error_init(&err);
	
	if (!(dbus_message_get_args(msg, &err,
			DBUS_TYPE_UINT32, &enable,
			DBUS_TYPE_INVALID))) {
		log_error("Unable to get input args ");
		if (dbus_error_is_set(&err)) {
			log_error("%s raised: %s",err.name,err.message);
			dbus_error_free(&err);
		}
		return NULL;
	}

	if (dba_server_enable(enable)) {
		op_ret = PPPoE_SNP_RETURN_CODE_FAIL;
	} else {
		dba_server_enable_flag = enable;
		op_ret = PPPoE_SNP_RETURN_CODE_SUCCESS;		
	}

	log_info("Direcet Broadcast DHCP is %s\n", (dba_server_enable_flag) ? "enalbe" : "disable");

	reply = dbus_message_new_method_return(msg);
	dbus_message_iter_init_append (reply, &iter);	
	dbus_message_iter_append_basic (&iter, DBUS_TYPE_UINT32, &op_ret);
	
	return reply;	
}


#ifdef __cplusplus
}
#endif
