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
* dcli_pppoe.c
*
* CREATOR:
*		zhouym@atuelan.com
*
* DESCRIPTION:
*		CLI definition for pppoe module.
*
* DATE:
*		2011-01-17 10:23
*
* FILE REVISION NUMBER:
*  			
*******************************************************************************/
#ifdef __cplusplus

extern "C"
{
#endif

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <dbus/dbus.h>

#include <sys/socket.h>
#include <linux/if.h>
#include <linux/if_ether.h>

#include "pppoe_def.h"
#include "pppoe_dbus_def.h"
#include "pppoe_interface_def.h"
#include "pppoe_dbus_interface.h"

#include "command.h"

#include "ws_public.h"

#include "dcli_main.h"
#include "dcli_pppoe.h" 


static struct cmd_node pppoe_node = {
	PPPOE_NODE,
	"%s(config-pppoe)# "
};

static struct cmd_node hansi_pppoe_node =  {
	HANSI_PPPOE_DEVICE_NODE,
	"%s(hansi-pppoe %d-%d-%d)# ",
};

static struct cmd_node local_hansi_pppoe_node = {
	LOCAL_HANSI_PPPOE_DEVICE_NODE,
	"%s(local-hansi-pppoe %d-%d-%d)# ",
};

static inline int
str2ipv4(char *str, unsigned int *ip, unsigned int *mask) {
	unsigned int length;
	char *p, *pm;
	
	p = strchr(str, '/');
	if (NULL == p)
		return -1;
	
	*p = '\0';
	if (1 == inet_pton(AF_INET, str, ip)) {
		if (0 == ((*ip) & 0xff000000))
			goto error;
	}	

	pm = p + 1;
	length = strlen(pm);
	if (!length || length > 2)
		goto error;

	if (1 == length) {
		if (*pm > '9' || *pm < '1')
			goto error;

		*mask = *pm - '0';
	} else {
		if (*pm > '3' || *pm < '1' || *(pm + 1) > '9' || *(pm + 1) < '0')
			goto error;

		*mask = (*pm - '0') * 10 + (*(pm + 1) - '0');
	}

	return 0;

error:
	*p = '/';
	return -1;
}

static inline int
str2mac(char *str, unsigned char *mac) {
	char *cursor = str;
	unsigned int len = strlen(str);
	int i;
	
	if (17 != len) {
		return -1;
	}

	for (i = 0; i < 6; i++) {
		if (*cursor >= '0' && *cursor <= '9') {
			mac[i] = (*cursor - '0') << 4;
		} else if (*cursor >= 'a' && *cursor <= 'f') {
			mac[i] = (*cursor - 'a' + 10) << 4;
		} else if (*cursor >= 'A' && *cursor <= 'F') {
			mac[i] = (*cursor - 'A' + 10) << 4;
		} else {
			return -1;
		}
		cursor ++;
		len --;

		if (*cursor >= '0' && *cursor <= '9') {
			mac[i] += *cursor - '0';
		} else if (*cursor >= 'a' && *cursor <= 'f') {
			mac[i] += *cursor - 'a' + 10;
		} else if (*cursor >= 'A' && *cursor <= 'F') {
			mac[i] += *cursor - 'A' + 10;
		} else {
			return -1;
		}
		cursor ++;
		len --;
		
		if (!len) {
			break;
		}
		
		if (*cursor != ':') {
			return -1;
		}
		cursor ++;
		len --;
	}

	return 0;
}

static inline int
ifname_legal_input(const char *str, char *ifname, unsigned int size,
			unsigned int slot_id, unsigned int local_id, unsigned int instance_id){
	int count = 0, flag = 0;
	char 	*cursor,
			*ebr_str = "ebr", 
			*wlan_str = "wlan",
			*radio_str = "r",
			*ve_str	= "ve";
	unsigned int 	len = strlen(str), 
					elen = strlen(ebr_str), 
					wlen = strlen(wlan_str),
					rlen = strlen(radio_str),
					vlen = strlen(ve_str);

	if (!len || len > (IFNAMSIZ - 1)) {
		return -1;
	}

	memset(ifname, 0, size);

	if (!strncmp(str, ebr_str, elen)) {
		cursor = str + elen;
		
		if ('l' == *cursor) {
			flag = 1;
			cursor++;
		}

		while (*cursor) {
			if ('-' == *cursor) {
				count++;
			} else if (*cursor < '0' || *cursor > '9') {
				goto error;
			}
			
			cursor++;
		}

		if (!count) {
			if (flag) {
				goto error;
			}
			
			memcpy(ifname, str, elen);
			snprintf(ifname + elen, size - elen, "%s%u-%u-%s", local_id ? "l" : "", 
								slot_id, instance_id, str + elen);
			goto out;
		}
	} else if (!strncmp(str, "wlan", wlen)) {
		cursor = str + wlen;
		
		if ('l' == *cursor) {
			flag = 1;
			cursor++;
		}

		while (*cursor) {
			if ('-' == *cursor) {
				count++;
			} else if (*cursor < '0' || *cursor > '9') {
				goto error;
			}
			
			cursor++;
		}

		if (!count) {
			if (flag) {
				goto error;
			}
			
			memcpy(ifname, str, wlen);
			snprintf(ifname + wlen, size - wlen, "%s%u-%u-%s", local_id ? "l" : "", 
								slot_id, instance_id, str + wlen);
			goto out;
		}
	} else if (!strncmp(str, radio_str, rlen)) {
		goto error;
	} else if (!strncmp(str, ve_str, vlen)) {
		if (ve_interface_parse(str, ifname, size))
			goto error;

		goto out;
	}

	memcpy(ifname, str, len);

out:
	return 0;

error:
	return -1;
}

static inline int
rdc_legal_input(const char *str, unsigned int *slot_id, unsigned int *instance_id) {
	if(2 != sscanf(str, "%u-%u", slot_id, instance_id))
		return -1;

	if (!(*slot_id) || (*slot_id) > SLOT_MAX_NUM) 
		return -1;

	if (!(*instance_id) || (*instance_id) > HANSI_MAX_ID)
		return -1;

	return 0;
}

static inline DBusConnection *
get_slot_connection(unsigned int slot_id) {
	if (active_master_slot == slot_id) 
		return dcli_dbus_connection;
		
	return dbus_connection_dcli[slot_id]->dcli_dbus_connection;
}

DEFUN(conf_pppoe_func,
	conf_pppoe_cmd,
	"config pppoe",
	CONFIG_STR
	"config pppoe module!\n"
)
{
	vty->node = PPPOE_NODE;
	return CMD_SUCCESS;
}   



DEFUN(create_pppoe_device_func,
	create_pppoe_device_cmd,
	"create pppoe-device PPPOEID PPPOEDESC",
	"pppoe config\n"
	"pppoe device\n"
	"assign PPPOE ID for pppoe\n"
	"assign PPPOE DESCRIPTION\n"
)
{
	DBusConnection *connection;
	unsigned int slot_id, local_id, ins_id, dev_id;
	char *pppoe_desc = (char *)argv[1];
	char ifname[IFNAMSIZ];
	int ret;

	if (HANSI_NODE != vty->node &&
		LOCAL_HANSI_NODE != vty->node) {
		vty_out (vty, "<error> Terminal mode change must under hansi configure mode!\n");
		return CMD_WARNING;
	}

	if (string_to_uint((char *)argv[0], &dev_id)) {
		vty_out (vty, "<error> pppoe id format is error\n");
		return CMD_WARNING;
	}

	if (!dev_id || dev_id > DEV_MAX_NUM) {
		vty_out (vty, "<error> pppoe id should be 1 to %d\n", DEV_MAX_NUM);
		return CMD_WARNING;
	}

	if (strlen(pppoe_desc) > (DEV_DESC_LEN - 1)) {
		vty_out (vty, "<error> pppoe desc length is out max(%d)!\n", DEV_DESC_LEN - 1);
		return CMD_WARNING;
	}

	slot_id = vty->slotindex;
	local_id = vty->local;
	ins_id = (unsigned int)vty->index;

	if (NULL == (connection = get_slot_connection(slot_id))) {
		vty_out (vty, "<error> slot %d is not connect\n", slot_id);
		return CMD_WARNING;
	}

	insIfName_init(ifname, slot_id, local_id, ins_id, dev_id);

	ret = pppoe_config_device_create(connection, local_id, ins_id,
									dev_id, ifname, pppoe_desc);
	if (PPPOEERR_SUCCESS == ret) {
		return CMD_SUCCESS;
	} else switch(ret) {
		case PPPOEERR_EINVAL:
			vty_out (vty, "<error> input para is error\n");
			break;
						
		case PPPOEERR_ENOMEM:
			vty_out (vty, "<error> mem malloc fail\n");
			break;

		case PPPOEERR_EEXIST:
			vty_out (vty, "<error> device is already exist\n");
			break;			
			
		case PPPOEERR_ESYSCALL:
			vty_out (vty, "<error> call kernel netlink fail\n");
			break;
			
		case PPPOEERR_EDBUS:
			vty_out (vty, "<error> fail get reply!\n");
			break;
			
		default:
			vty_out (vty, "<error> unknow fail reason\n");
			break;
	}
	
	return CMD_WARNING;
}

DEFUN(delete_pppoe_device_func,
	delete_pppoe_device_cmd,
	"delete pppoe-device PPPOEID",
	"pppoe config\n"
	"pppoe device\n"
	"assign PPPOE ID for pppoe\n"
)
{
	DBusConnection *connection;
	unsigned int slot_id, local_id, ins_id, dev_id;
	int ret;

	if (HANSI_NODE != vty->node &&
		LOCAL_HANSI_NODE != vty->node) {
		vty_out (vty, "<error> Terminal mode change must under hansi configure mode!\n");
		return CMD_WARNING;
	}

	if (string_to_uint((char *)argv[0], &dev_id)) {
		vty_out (vty, "<error> pppoe id format is error\n");
		return CMD_WARNING;
	}

	if (!dev_id || dev_id > DEV_MAX_NUM) {
		vty_out (vty, "<error> pppoe id should be 1 to %d\n", DEV_MAX_NUM);
		return CMD_WARNING;
	}

	slot_id = vty->slotindex;
	local_id = vty->local;
	ins_id = (unsigned int)vty->index;

	if (NULL == (connection = get_slot_connection(slot_id))) {
		vty_out (vty, "<error> slot %d is not connect\n", slot_id);
		return CMD_WARNING;
	}

	ret = pppoe_config_device_destroy(connection, local_id, ins_id, dev_id);
	if (PPPOEERR_SUCCESS == ret) {
		return CMD_SUCCESS;
	} else switch(ret) {
		case PPPOEERR_EINVAL:
			vty_out (vty, "<error> input para is error\n");
			break;
			
		case PPPOEERR_ENOEXIST:
			vty_out (vty, "<error> device is not exist\n");
			break;
						
		case PPPOEERR_ESYSCALL:
			vty_out (vty, "<error> call kernel netlink fail\n");
			break;
			
		case PPPOEERR_EDBUS:
			vty_out (vty, "<error> fail get reply!\n");
			break;

		case PPPOEERR_ESERVICE:
			vty_out (vty, "<error> device service is enable, please disable it frist\n");
			break;
			
		default:
			vty_out (vty, "<error> unknow fail reason\n");
			break;
	}
	
	return CMD_WARNING;
}


DEFUN(show_pppoe_device_list_func,
	show_pppoe_device_list_cmd,
	"show pppoe device list",
	"pppoe info\n"
	"pppoe device list\n"
)
{
	DBusConnection *connection;
	unsigned int slot_id, local_id, ins_id;
	struct pppoeDevBasicInfo *dev_array;
	char str_ipaddr[32];
	unsigned int num;
	int ret;

	if (HANSI_NODE != vty->node &&
		LOCAL_HANSI_NODE != vty->node) {
		vty_out (vty, "<error> Terminal mode change must under hansi configure mode!\n");
		return CMD_WARNING;
	}

	slot_id = vty->slotindex;
	local_id = vty->local;
	ins_id = (unsigned int)vty->index;

	if (NULL == (connection = get_slot_connection(slot_id))) {
		vty_out(vty, "<error> slot %d is not connect\n", slot_id);
		return CMD_WARNING;
	}

	ret = pppoe_show_device_list(connection, local_id, ins_id,
								&dev_array, &num);
	if (PPPOEERR_SUCCESS == ret) {
		vty_out(vty,"==============================================================================\n");
		vty_out(vty,"%-6s %-16s %-16s %-19s %-7s\n","DevID","DevName","BaseDev","DevIP","State");
		if (dev_array) {
			int i;
			for (i = 0; i < num; i++) {
				if (dev_array[i].ipaddr) {
					memset(str_ipaddr, 0, sizeof(str_ipaddr));
					snprintf(str_ipaddr, sizeof(str_ipaddr), "%u.%u.%u.%u/%u",
										(dev_array[i].ipaddr >> 24) & 0xff, 
										(dev_array[i].ipaddr >> 16) & 0xff,
										(dev_array[i].ipaddr >> 8) & 0xff, 
										dev_array[i].ipaddr & 0xff, dev_array[i].mask);
					
					vty_out(vty, "%-6u %-16s %-16s %-19s %-7s\n", 
								dev_array[i].dev_id, dev_array[i].ifname, 
								dev_array[i].base_ifname[0] ? dev_array[i].base_ifname : "NULL", str_ipaddr,
								2 == dev_array[i].state ? "DOWN" : (3 == dev_array[i].state ? "UP" : 
								(4 == dev_array[i].state ? "RUN" : "CREATE")));
				} else {
					vty_out(vty, "%-6u %-16s %-16s %-19s %-7s\n", 
								dev_array[i].dev_id, dev_array[i].ifname,
								dev_array[i].base_ifname[0] ? dev_array[i].base_ifname : "NULL", "NULL",
								2 == dev_array[i].state ? "DOWN" : (3 == dev_array[i].state ? "UP" : 
								(4 == dev_array[i].state ? "RUN" : "CREATE")));
				}
			}
			free(dev_array);
		}
		vty_out(vty,"==============================================================================\n");
		return CMD_SUCCESS;
		
	} switch(ret) {
		case PPPOEERR_EINVAL:
			vty_out(vty, "<error> input para is error\n");
			break;
			
		case PPPOEERR_EDBUS:
			vty_out(vty, "<error> fail get reply!\n");
			break;

		default:
			vty_out(vty, "<error> unknow fail reason\n");
			break;
	}
	
	return CMD_WARNING;
}


DEFUN(config_pppoe_device_func,
	config_pppoe_device_cmd,
	"config pppoe-device PPPOEID",
	CONFIG_STR
	"config pppoe device\n"
	"pppoe id that you want to config\n"
)
{
	DBusConnection *connection;
	unsigned int slot_id, local_id, ins_id, dev_id;
	int ret;

	if (HANSI_NODE != vty->node &&
		LOCAL_HANSI_NODE != vty->node) {
		vty_out (vty, "<error> Terminal mode change must under hansi configure mode!\n");
		return CMD_WARNING;
	}

	if (string_to_uint((char *)argv[0], &dev_id)) {
		vty_out (vty, "<error> pppoe id format is error\n");
		return CMD_WARNING;
	}

	if (!dev_id || dev_id > DEV_MAX_NUM) {
		vty_out (vty, "<error> pppoe id should be 1 to %d\n", DEV_MAX_NUM);
		return CMD_WARNING;
	}

	slot_id = vty->slotindex;
	local_id = vty->local;
	ins_id = (unsigned int)vty->index;
	
	if (NULL == (connection = get_slot_connection(slot_id))) {
		vty_out (vty, "<error> slot %d is not connect\n", slot_id);
		return CMD_WARNING;
	}

	ret = pppoe_detect_device_exist(connection,
									local_id,
									ins_id,
									dev_id);
	if (PPPOEERR_SUCCESS == ret) {
		if (local_id) {
			vty->node = LOCAL_HANSI_PPPOE_DEVICE_NODE;
			vty->index_sub = (void *)dev_id;
		} else {
			vty->node = HANSI_PPPOE_DEVICE_NODE;
			vty->index_sub = (void *)dev_id;
		}
		return CMD_SUCCESS;
	} else if (PPPOEERR_ENOEXIST == ret) {
		vty_out (vty, "<error> pppoe device %d is not exist\n", dev_id);
		return CMD_WARNING;
	} else if (PPPOEERR_EDBUS == ret) {
		vty_out (vty, "<error> fail get reply.\n");
		return CMD_WARNING;
	} 

	vty_out (vty, "<error> unknow fail reason.\n");
	return CMD_WARNING;
}

DEFUN(base_pppoe_device_func,
	base_pppoe_device_cmd,
	"base interface IFNAME",
	"pppoe device config\n"
	"pppoe device base interface\n"
	"interface name\n"
)
{
	DBusConnection *connection;
	unsigned int slot_id, local_id, ins_id, dev_id;
	char ifname[IFNAMSIZ];
	int ret;
	
	if (HANSI_PPPOE_DEVICE_NODE != vty->node &&
		LOCAL_HANSI_PPPOE_DEVICE_NODE != vty->node) {
		vty_out (vty, "<error> Terminal mode change must under pppoe configure mode!\n");
		return CMD_WARNING;
	}

	slot_id = vty->slotindex;
	local_id = vty->local;
	ins_id = (unsigned int)vty->index;
	dev_id = (unsigned int)vty->index_sub;

	if (ifname_legal_input((char *)argv[0], ifname, sizeof(ifname),
							slot_id, local_id, ins_id)) {
		vty_out (vty, "<error> Input ifname is error\n");
		return CMD_WARNING;
	}

	if (NULL == (connection = get_slot_connection(slot_id))) {
		vty_out (vty, "<error> slot %d is not connect\n", slot_id);
		return CMD_WARNING;
	}

	ret = pppoe_config_device_base(connection, local_id, ins_id, dev_id, ifname);
	if (PPPOEERR_SUCCESS == ret) {
		return CMD_SUCCESS;
	} else switch(ret) {
		case PPPOEERR_EINVAL:
			vty_out (vty, "<error> input para is error\n");
			break;
						
		case PPPOEERR_EEXIST:
			vty_out (vty, "<error> device is already base interface\n");
			break;

		case PPPOEERR_ENOEXIST:
			vty_out (vty, "<error> interface(%s) is not exist\n", ifname);
			break;			
						
		case PPPOEERR_ESYSCALL:
			vty_out (vty, "<error> call kernel netlink fail\n");
			break;
			
		case PPPOEERR_EDBUS:
			vty_out (vty, "<error> fail get reply!\n");
			break;

		case PPPOEERR_ESERVICE:
			vty_out (vty, "<error> device service is enable, please disable it frist\n");
			break;
			
		default:
			vty_out (vty, "<error> unknow fail reason\n");
			break;
	}
	
	return CMD_WARNING;
}

DEFUN(no_base_pppoe_device_func,
	no_base_pppoe_device_cmd,
	"no base interface",
	"pppoe device config\n"
	"pppoe device unbase interface\n"
)
{
	DBusConnection *connection;
	unsigned int slot_id, local_id, ins_id, dev_id;
	int ret;
	
	if (HANSI_PPPOE_DEVICE_NODE != vty->node &&
		LOCAL_HANSI_PPPOE_DEVICE_NODE != vty->node) {
		vty_out (vty, "<error> Terminal mode change must under pppoe configure mode!\n");
		return CMD_WARNING;
	}

	slot_id = vty->slotindex;
	local_id = vty->local;
	ins_id = (unsigned int)vty->index;
	dev_id = (unsigned int)vty->index_sub;

	if (NULL == (connection = get_slot_connection(slot_id))) {
		vty_out (vty, "<error> slot %d is not connect\n", slot_id);
		return CMD_WARNING;
	}


	ret = pppoe_config_device_base(connection, local_id, ins_id, dev_id, NULL);
	if (PPPOEERR_SUCCESS == ret) {
		return CMD_SUCCESS;
	} else switch(ret) {
		case PPPOEERR_EINVAL:
			vty_out (vty, "<error> input para is error\n");
			break;
			
		case PPPOEERR_ENOEXIST:
			vty_out (vty, "<error> device is not exist\n");
			break;
	
		case PPPOEERR_ESYSCALL:
			vty_out (vty, "<error> call kernel netlink fail\n");
			break;
			
		case PPPOEERR_EDBUS:
			vty_out (vty, "<error> fail get reply!\n");
			break;

		case PPPOEERR_ESERVICE:
			vty_out (vty, "<error> device service is enable, please disable it frist\n");
			break;

		case PPPOEERR_ESTATE:
			vty_out (vty, "<error> device is not base interface\n");
			break;
			
		default:
			vty_out (vty, "<error> unknow fail reason\n");
			break;
	}
	
	return CMD_WARNING;
}

/*	lixiang modify 2012-12-06 #########start#########
	this cmd to be replaced by "packet-reforward (srcip|dstip) (A.B.C.D/M | all) slot SLOT_ID (enable|disable) [ipv6]"*/
#if 0
DEFUN(apply_pppoe_device_func,
	apply_pppoe_device_cmd,
	"apply interface IFNAME",
	"pppoe device config\n"
	"pppoe device apply interface\n"
	"interface name\n"
)
{
	DBusConnection *connection;
	unsigned int slot_id, local_id, ins_id, dev_id;
	char ifname[IFNAMSIZ];
	int ret;
	
	if (HANSI_PPPOE_DEVICE_NODE != vty->node &&
		LOCAL_HANSI_PPPOE_DEVICE_NODE != vty->node) {
		vty_out (vty, "<error> Terminal mode change must under pppoe configure mode!\n");
		return CMD_WARNING;
	}

	slot_id = vty->slotindex;
	local_id = vty->local;
	ins_id = (unsigned int)vty->index;
	dev_id = (unsigned int)vty->index_sub;

	if (ifname_legal_input((char *)argv[0], ifname, sizeof(ifname),
							slot_id, local_id, ins_id)) {
		vty_out (vty, "<error> Input ifname is error\n");
		return CMD_WARNING;
	}

	if (NULL == (connection = get_slot_connection(slot_id))) {
		vty_out (vty, "<error> slot %d is not connect\n", slot_id);
		return CMD_WARNING;
	}

	ret = pppoe_config_device_apply(connection, local_id, ins_id, dev_id, ifname);
	if (PPPOEERR_SUCCESS == ret) {
		return CMD_SUCCESS;
	} else switch(ret) {
		case PPPOEERR_EINVAL:
			vty_out (vty, "<error> input para is error\n");
			break;
						
		case PPPOEERR_EEXIST:
			vty_out (vty, "<error> device is already apply interface\n");
			break;

		case PPPOEERR_ENOEXIST:
			vty_out (vty, "<error> interface(%s) is not exist\n", ifname);
			break;			
						
		case PPPOEERR_ESYSCALL:
			vty_out (vty, "<error> call kernel netlink fail\n");
			break;
			
		case PPPOEERR_EDBUS:
			vty_out (vty, "<error> fail get reply!\n");
			break;

		case PPPOEERR_ESERVICE:
			vty_out (vty, "<error> device service is enable, please disable it frist\n");
			break;
			
		default:
			vty_out (vty, "<error> unknow fail reason\n");
			break;
	}
	
	return CMD_WARNING;
}

DEFUN(no_apply_pppoe_device_func,
	no_apply_pppoe_device_cmd,
	"no apply interface",
	"pppoe device config\n"
	"pppoe device unbase interface\n"
)
{
	DBusConnection *connection;
	unsigned int slot_id, local_id, ins_id, dev_id;
	int ret;
	
	if (HANSI_PPPOE_DEVICE_NODE != vty->node &&
		LOCAL_HANSI_PPPOE_DEVICE_NODE != vty->node) {
		vty_out (vty, "<error> Terminal mode change must under pppoe configure mode!\n");
		return CMD_WARNING;
	}

	slot_id = vty->slotindex;
	local_id = vty->local;
	ins_id = (unsigned int)vty->index;
	dev_id = (unsigned int)vty->index_sub;

	if (NULL == (connection = get_slot_connection(slot_id))) {
		vty_out (vty, "<error> slot %d is not connect\n", slot_id);
		return CMD_WARNING;
	}


	ret = pppoe_config_device_apply(connection, local_id, ins_id, dev_id, NULL);
	if (PPPOEERR_SUCCESS == ret) {
		return CMD_SUCCESS;
	} else switch(ret) {
		case PPPOEERR_EINVAL:
			vty_out (vty, "<error> input para is error\n");
			break;
			
		case PPPOEERR_ENOEXIST:
			vty_out (vty, "<error> device is not exist\n");
			break;
				
		case PPPOEERR_EDBUS:
			vty_out (vty, "<error> fail get reply!\n");
			break;

		case PPPOEERR_ESERVICE:
			vty_out (vty, "<error> device service is enable, please disable it frist\n");
			break;
			
		default:
			vty_out (vty, "<error> unknow fail reason\n");
			break;
	}
	
	return CMD_WARNING;
}
#endif
/*	lixiang modify 2012-12-06 #########end######### */

DEFUN(pppoe_device_ipaddr_func,
	pppoe_device_ipaddr_cmd,
	"ip address A.B.C.D/M",
	"pppoe device ipaddr config\n"
)
{
	DBusConnection *connection;
	unsigned int slot_id, local_id, ins_id, dev_id;
	unsigned int ipaddr, mask;
	int ret;
	
	if (HANSI_PPPOE_DEVICE_NODE != vty->node &&
		LOCAL_HANSI_PPPOE_DEVICE_NODE != vty->node) {
		vty_out (vty, "<error> Terminal mode change must under pppoe configure mode!\n");
		return CMD_WARNING;
	}

	if (str2ipv4((char *)argv[0], &ipaddr, &mask))	 {
		vty_out(vty, "<error> invalid ip address: %s\n", (char *)argv[0]);
		return CMD_WARNING;
	}

	slot_id = vty->slotindex;
	local_id = vty->local;
	ins_id = (unsigned int)vty->index;
	dev_id = (unsigned int)vty->index_sub;

	if (NULL == (connection = get_slot_connection(slot_id))) {
		vty_out(vty, "<error> slot %d is not connect\n", slot_id);
		return CMD_WARNING;
	}

	ret = pppoe_config_device_ipaddr(connection, local_id, ins_id, dev_id, ipaddr, mask);
	if (PPPOEERR_SUCCESS == ret) {
		return CMD_SUCCESS;
	} else switch(ret) {
		case PPPOEERR_EINVAL:
			vty_out (vty, "<error> input para is error\n");
			break;
			
		case PPPOEERR_ENOEXIST:
			vty_out (vty, "<error> device is not exist\n");
			break;
						
		case PPPOEERR_ESYSCALL:
			vty_out (vty, "<error> system call fail\n");
			break;
			
		case PPPOEERR_EDBUS:
			vty_out (vty, "<error> fail get reply!\n");
			break;

		case PPPOEERR_ESERVICE:
			vty_out (vty, "<error> device service is enable, please disable it frist\n");
			break;
			
		default:
			vty_out (vty, "<error> unknow fail reason\n");
			break;
	}
	
	return CMD_WARNING;
}

DEFUN(no_pppoe_device_ipaddr_func,
	no_pppoe_device_ipaddr_cmd,
	"no ip address",
	"pppoe device ipaddr config\n"
)
{
	DBusConnection *connection;
	unsigned int slot_id, local_id, ins_id, dev_id;
	int ret;
	
	if (HANSI_PPPOE_DEVICE_NODE != vty->node &&
		LOCAL_HANSI_PPPOE_DEVICE_NODE != vty->node) {
		vty_out (vty, "<error> Terminal mode change must under pppoe configure mode!\n");
		return CMD_WARNING;
	}

	slot_id = vty->slotindex;
	local_id = vty->local;
	ins_id = (unsigned int)vty->index;
	dev_id = (unsigned int)vty->index_sub;

	if (NULL == (connection = get_slot_connection(slot_id))) {
		vty_out (vty, "<error> slot %d is not connect\n", slot_id);
		return CMD_WARNING;
	}

	ret = pppoe_config_device_ipaddr(connection, local_id, ins_id, dev_id, 0, 0);
	if (PPPOEERR_SUCCESS == ret) {
		return CMD_SUCCESS;
	} else switch(ret) {
		case PPPOEERR_EINVAL:
			vty_out (vty, "<error> input para is error\n");
			break;
			
		case PPPOEERR_ENOEXIST:
			vty_out (vty, "<error> device is not exist\n");
			break;
						
		case PPPOEERR_ESYSCALL:
			vty_out (vty, "<error> system call fail\n");
			break;
			
		case PPPOEERR_EDBUS:
			vty_out (vty, "<error> fail get reply!\n");
			break;

		case PPPOEERR_ESERVICE:
			vty_out (vty, "<error> device service is enable, please disable it frist\n");
			break;
			
		default:
			vty_out (vty, "<error> unknow fail reason\n");
			break;
	}
	
	return CMD_WARNING;
}

DEFUN(pppoe_device_virtual_mac_func,
	pppoe_device_virtual_mac_cmd,
	"virtual mac MAC",
	"pppoe device virtual mac config\n"
	"virtual mac\n"
	"MAC Address format as HH:HH:HH:HH:HH:HH\n"
)
{
	DBusConnection *connection;
	unsigned int slot_id, local_id, ins_id, dev_id;
	unsigned char virtualMac[ETH_ALEN];
	int ret;
	
	if (HANSI_PPPOE_DEVICE_NODE != vty->node &&
		LOCAL_HANSI_PPPOE_DEVICE_NODE != vty->node) {
		vty_out (vty, "<error> Terminal mode change must under pppoe configure mode!\n");
		return CMD_WARNING;
	}

	if (str2mac((char *)argv[0], virtualMac)) {
		vty_out(vty, "<error> invalid mac address: %s\n", (char *)argv[0]);
		return CMD_WARNING;
	}

	if (!(virtualMac[0] | virtualMac[1] | virtualMac[2]
		| virtualMac[3] | virtualMac[4] | virtualMac[5])) {
		vty_out(vty, "<error> invalid mac address: %s\n", (char *)argv[0]);
		return CMD_WARNING;
	}

	if (virtualMac[0] & 0x01) {
		vty_out(vty, "<error> invalid mac address: %s is non-unicast\n", (char *)argv[0]);
		return CMD_WARNING;
	}

	slot_id = vty->slotindex;
	local_id = vty->local;
	ins_id = (unsigned int)vty->index;
	dev_id = (unsigned int)vty->index_sub;

	if (NULL == (connection = get_slot_connection(slot_id))) {
		vty_out(vty, "<error> slot %d is not connect\n", slot_id);
		return CMD_WARNING;
	}

	ret = pppoe_config_device_virtual_mac(connection, local_id, ins_id, dev_id, virtualMac);
	if (PPPOEERR_SUCCESS == ret) {
		return CMD_SUCCESS;
	} else switch(ret) {
		case PPPOEERR_EINVAL:
			vty_out (vty, "<error> input para is error\n");
			break;
			
		case PPPOEERR_ENOEXIST:
			vty_out (vty, "<error> device is not exist\n");
			break;
			
		case PPPOEERR_EDBUS:
			vty_out (vty, "<error> fail get reply!\n");
			break;

		case PPPOEERR_ESERVICE:
			vty_out (vty, "<error> device service is enable, please disable it frist\n");
			break;
			
		default:
			vty_out (vty, "<error> unknow fail reason\n");
			break;
	}
	
	return CMD_WARNING;
}

DEFUN(no_pppoe_device_virtual_mac_func,
	no_pppoe_device_virtual_mac_cmd,
	"no virtual mac",
	"pppoe device ipaddr config\n"
	"virtual mac\n"	
)
{
	DBusConnection *connection;
	unsigned int slot_id, local_id, ins_id, dev_id;
	int ret;
	
	if (HANSI_PPPOE_DEVICE_NODE != vty->node &&
		LOCAL_HANSI_PPPOE_DEVICE_NODE != vty->node) {
		vty_out (vty, "<error> Terminal mode change must under pppoe configure mode!\n");
		return CMD_WARNING;
	}

	slot_id = vty->slotindex;
	local_id = vty->local;
	ins_id = (unsigned int)vty->index;
	dev_id = (unsigned int)vty->index_sub;

	if (NULL == (connection = get_slot_connection(slot_id))) {
		vty_out (vty, "<error> slot %d is not connect\n", slot_id);
		return CMD_WARNING;
	}

	ret = pppoe_config_device_virtual_mac(connection, local_id, ins_id, dev_id, NULL);
	if (PPPOEERR_SUCCESS == ret) {
		return CMD_SUCCESS;
	} else switch(ret) {
		case PPPOEERR_EINVAL:
			vty_out (vty, "<error> input para is error\n");
			break;
			
		case PPPOEERR_ENOEXIST:
			vty_out (vty, "<error> device is not exist\n");
			break;
			
		case PPPOEERR_EDBUS:
			vty_out (vty, "<error> fail get reply!\n");
			break;

		case PPPOEERR_ESERVICE:
			vty_out (vty, "<error> device service is enable, please disable it frist\n");
			break;
			
		default:
			vty_out (vty, "<error> unknow fail reason\n");
			break;
	}
	
	return CMD_WARNING;
}


DEFUN(pppoe_session_ipaddr_func,
	pppoe_session_ipaddr_cmd,
	"session ip address range A.B.C.D A.B.C.D",
	"pppoe session ipaddr config\n"
)
{
	DBusConnection *connection;
	unsigned int slot_id, local_id, ins_id, dev_id;
	unsigned int minIP, maxIP;
	int ret;
	
	if (HANSI_PPPOE_DEVICE_NODE != vty->node &&
		LOCAL_HANSI_PPPOE_DEVICE_NODE != vty->node) {
		vty_out (vty, "<error> Terminal mode change must under pppoe configure mode!\n");
		return CMD_WARNING;
	}

	slot_id = vty->slotindex;
	local_id = vty->local;
	ins_id = (unsigned int)vty->index;
	dev_id = (unsigned int)vty->index_sub;

	if (NULL == (connection = get_slot_connection(slot_id))) {
		vty_out(vty, "<error> slot %d is not connect\n", slot_id);
		return CMD_WARNING;
	}

	if (1 == inet_pton(AF_INET, (char *)argv[0], &minIP)) {
		if (0 == (minIP & 0xff000000)) {
			vty_out(vty, "<error> invalid min ip address: %s(0.X.X.X)\n", (char *)argv[0]);
			return CMD_WARNING;
		}
	} else {
		vty_out(vty, "<error> invalid min ip address: %s\n", (char *)argv[0]);
		return CMD_WARNING;
	}

	if (1 == inet_pton(AF_INET, (char *)argv[1], &maxIP)) {
		if (0 == (maxIP & 0xff000000)) {
			vty_out(vty, "<error> invalid max ip address: %s(0.X.X.X)\n", (char *)argv[0]);
			return CMD_WARNING;
		}
	} else {
		vty_out(vty, "<error> invalid ip address: %s\n", (char *)argv[1]);
		return CMD_WARNING;
	}
	
	ret = pppoe_config_session_ipaddr(connection, local_id, ins_id, dev_id, minIP, maxIP);
	if (PPPOEERR_SUCCESS == ret) {
		return CMD_SUCCESS;
	} else switch(ret) {
		case PPPOEERR_EINVAL:
			vty_out (vty, "<error> input para is error\n");
			break;
			
		case PPPOEERR_ENOEXIST:
			vty_out (vty, "<error> device is not exist\n");
			break;
									
		case PPPOEERR_EDBUS:
			vty_out (vty, "<error> fail get reply!\n");
			break;

		case PPPOEERR_ESERVICE:
			vty_out (vty, "<error> device service is enable, please disable it frist\n");
			break;
			
		default:
			vty_out (vty, "<error> unknow fail reason\n");
			break;
	}
	
	return CMD_WARNING;
}

DEFUN(no_pppoe_session_ipaddr_func,
	no_pppoe_session_ipaddr_cmd,
	"no session ip address",
	"pppoe session ipaddr config\n"
)
{
	DBusConnection *connection;
	unsigned int slot_id, local_id, ins_id, dev_id;
	int ret;
	
	if (HANSI_PPPOE_DEVICE_NODE != vty->node &&
		LOCAL_HANSI_PPPOE_DEVICE_NODE != vty->node) {
		vty_out (vty, "<error> Terminal mode change must under pppoe configure mode!\n");
		return CMD_WARNING;
	}

	slot_id = vty->slotindex;
	local_id = vty->local;
	ins_id = (unsigned int)vty->index;
	dev_id = (unsigned int)vty->index_sub;

	if (NULL == (connection = get_slot_connection(slot_id))) {
		vty_out (vty, "<error> slot %d is not connect\n", slot_id);
		return CMD_WARNING;
	}

	ret = pppoe_config_session_ipaddr(connection, local_id, ins_id, dev_id, 0, 0);
	if (PPPOEERR_SUCCESS == ret) {
		return CMD_SUCCESS;
	} else switch(ret) {
		case PPPOEERR_EINVAL:
			vty_out (vty, "<error> input para is error\n");
			break;
			
		case PPPOEERR_ENOEXIST:
			vty_out (vty, "<error> device is not exist\n");
			break;
						
		case PPPOEERR_EDBUS:
			vty_out (vty, "<error> fail get reply!\n");
			break;

		case PPPOEERR_ESERVICE:
			vty_out (vty, "<error> device service is enable, please disable it frist\n");
			break;
			
		default:
			vty_out (vty, "<error> unknow fail reason\n");
			break;
	}
	
	return CMD_WARNING;
}


DEFUN(pppoe_session_dns_func,
	pppoe_session_dns_cmd,
	"session dns A.B.C.D [A.B.C.D]",
	"pppoe session dns config\n"
)
{
	DBusConnection *connection;
	unsigned int slot_id, local_id, ins_id, dev_id;
	unsigned int dns1, dns2;
	int ret;
	
	if (HANSI_PPPOE_DEVICE_NODE != vty->node &&
		LOCAL_HANSI_PPPOE_DEVICE_NODE != vty->node) {
		vty_out (vty, "<error> Terminal mode change must under pppoe configure mode!\n");
		return CMD_WARNING;
	}

	slot_id = vty->slotindex;
	local_id = vty->local;
	ins_id = (unsigned int)vty->index;
	dev_id = (unsigned int)vty->index_sub;

	if (NULL == (connection = get_slot_connection(slot_id))) {
		vty_out(vty, "<error> slot %d is not connect\n", slot_id);
		return CMD_WARNING;
	}

	if (1 == inet_pton(AF_INET, (char *)argv[0], &dns1)) {
		if (0 == (dns1 & 0xff000000)) {
			vty_out(vty, "<error> invalid dns1: %s(0.X.X.X)\n", (char *)argv[0]);
			return CMD_WARNING;
		}
	} else {
		vty_out(vty, "<error> invalid dns1: %s\n", (char *)argv[0]);
		return CMD_WARNING;
	}

	if (2 == argc) {
		if (1 == inet_pton(AF_INET, (char *)argv[1], &dns2)) {
			if (0 == (dns2 & 0xff000000)) {
				vty_out(vty, "<error> invalid dns2: %s(0.X.X.X)\n", (char *)argv[0]);
				return CMD_WARNING;
			}
		} else {
			vty_out(vty, "<error> invalid dns2: %s\n", (char *)argv[1]);
			return CMD_WARNING;
		}
	} else {
		dns2 = 0;
	}
	
	ret = pppoe_config_session_dns(connection, local_id, ins_id, dev_id, dns1, dns2);
	if (PPPOEERR_SUCCESS == ret) {
		return CMD_SUCCESS;
	} else switch(ret) {
		case PPPOEERR_EINVAL:
			vty_out (vty, "<error> input para is error\n");
			break;
			
		case PPPOEERR_ENOEXIST:
			vty_out (vty, "<error> device is not exist\n");
			break;
									
		case PPPOEERR_EDBUS:
			vty_out (vty, "<error> fail get reply!\n");
			break;

		case PPPOEERR_ESERVICE:
			vty_out (vty, "<error> device service is enable, please disable it frist\n");
			break;
			
		default:
			vty_out (vty, "<error> unknow fail reason\n");
			break;
	}
	
	return CMD_WARNING;
}

DEFUN(no_pppoe_session_dns_func,
	no_pppoe_session_dns_cmd,
	"no session dns",
	"pppoe session dns config\n"
)
{
	DBusConnection *connection;
	unsigned int slot_id, local_id, ins_id, dev_id;
	int ret;
	
	if (HANSI_PPPOE_DEVICE_NODE != vty->node &&
		LOCAL_HANSI_PPPOE_DEVICE_NODE != vty->node) {
		vty_out (vty, "<error> Terminal mode change must under pppoe configure mode!\n");
		return CMD_WARNING;
	}

	slot_id = vty->slotindex;
	local_id = vty->local;
	ins_id = (unsigned int)vty->index;
	dev_id = (unsigned int)vty->index_sub;

	if (NULL == (connection = get_slot_connection(slot_id))) {
		vty_out (vty, "<error> slot %d is not connect\n", slot_id);
		return CMD_WARNING;
	}

	ret = pppoe_config_session_dns(connection, local_id, ins_id, dev_id, 0, 0);
	if (PPPOEERR_SUCCESS == ret) {
		return CMD_SUCCESS;
	} else switch(ret) {
		case PPPOEERR_EINVAL:
			vty_out (vty, "<error> input para is error\n");
			break;
			
		case PPPOEERR_ENOEXIST:
			vty_out (vty, "<error> device is not exist\n");
			break;
						
		case PPPOEERR_EDBUS:
			vty_out (vty, "<error> fail get reply!\n");
			break;

		case PPPOEERR_ESERVICE:
			vty_out (vty, "<error> device service is enable, please disable it frist\n");
			break;
			
		default:
			vty_out (vty, "<error> unknow fail reason\n");
			break;
	}
	
	return CMD_WARNING;
}

DEFUN(pppoe_nas_ipaddr_func,
	pppoe_nas_ipaddr_cmd,
	"nas ip address A.B.C.D",
	"pppoe session nas ipaddr config\n"
)
{
	DBusConnection *connection;
	unsigned int slot_id, local_id, ins_id, dev_id;
	unsigned int nasip;
	int ret;
	
	if (HANSI_PPPOE_DEVICE_NODE != vty->node &&
		LOCAL_HANSI_PPPOE_DEVICE_NODE != vty->node) {
		vty_out (vty, "<error> Terminal mode change must under pppoe configure mode!\n");
		return CMD_WARNING;
	}

	slot_id = vty->slotindex;
	local_id = vty->local;
	ins_id = (unsigned int)vty->index;
	dev_id = (unsigned int)vty->index_sub;

	if (NULL == (connection = get_slot_connection(slot_id))) {
		vty_out(vty, "<error> slot %d is not connect\n", slot_id);
		return CMD_WARNING;
	}

	if (1 == inet_pton(AF_INET, (char *)argv[0], &nasip)) {
		if (0 == (nasip & 0xff000000)) {
			vty_out(vty, "<error> invalid nasip: %s(0.X.X.X)\n", (char *)argv[0]);
			return CMD_WARNING;
		}
	} else {
		vty_out(vty, "<error> invalid nasip: %s\n", (char *)argv[0]);
		return CMD_WARNING;
	}
	
	ret = pppoe_config_nas_ipaddr(connection, local_id, ins_id, dev_id, nasip);
	if (PPPOEERR_SUCCESS == ret) {
		return CMD_SUCCESS;
	} else switch(ret) {
		case PPPOEERR_EINVAL:
			vty_out (vty, "<error> input para is error\n");
			break;
			
		case PPPOEERR_ENOEXIST:
			vty_out (vty, "<error> device is not exist\n");
			break;
									
		case PPPOEERR_EDBUS:
			vty_out (vty, "<error> fail get reply!\n");
			break;

		case PPPOEERR_ESERVICE:
			vty_out (vty, "<error> device service is enable, please disable it frist\n");
			break;
			
		default:
			vty_out (vty, "<error> unknow fail reason\n");
			break;
	}
	
	return CMD_WARNING;
}

DEFUN(no_pppoe_nas_ipadd_func,
	no_pppoe_nas_ipaddr_cmd,
	"no nas ip address",
	"pppoe session dns config\n"
)
{
	DBusConnection *connection;
	unsigned int slot_id, local_id, ins_id, dev_id;
	int ret;
	
	if (HANSI_PPPOE_DEVICE_NODE != vty->node &&
		LOCAL_HANSI_PPPOE_DEVICE_NODE != vty->node) {
		vty_out (vty, "<error> Terminal mode change must under pppoe configure mode!\n");
		return CMD_WARNING;
	}

	slot_id = vty->slotindex;
	local_id = vty->local;
	ins_id = (unsigned int)vty->index;
	dev_id = (unsigned int)vty->index_sub;

	if (NULL == (connection = get_slot_connection(slot_id))) {
		vty_out (vty, "<error> slot %d is not connect\n", slot_id);
		return CMD_WARNING;
	}

	ret = pppoe_config_nas_ipaddr(connection, local_id, ins_id, dev_id, 0);
	if (PPPOEERR_SUCCESS == ret) {
		return CMD_SUCCESS;
	} else switch(ret) {
		case PPPOEERR_EINVAL:
			vty_out (vty, "<error> input para is error\n");
			break;
			
		case PPPOEERR_ENOEXIST:
			vty_out (vty, "<error> device is not exist\n");
			break;
						
		case PPPOEERR_EDBUS:
			vty_out (vty, "<error> fail get reply!\n");
			break;

		case PPPOEERR_ESERVICE:
			vty_out (vty, "<error> device service is enable, please disable it frist\n");
			break;
			
		default:
			vty_out (vty, "<error> unknow fail reason\n");
			break;
	}
	
	return CMD_WARNING;
}

DEFUN(config_radius_rdc_func,
	config_radius_rdc_cmd,
	"radius rdc PARAM",
	"radius rdc\n"
	"radius rdc config\n"
	"rdc parameter, format: A-B (A is slot id, B is instance id)\n"
)
{
	DBusConnection *connection;
	unsigned int slot_id, local_id, ins_id, dev_id, s_slotid, s_insid;
	unsigned int state;
	int ret;

	if (HANSI_PPPOE_DEVICE_NODE != vty->node &&
		LOCAL_HANSI_PPPOE_DEVICE_NODE != vty->node) {
		vty_out (vty, "<error> Terminal mode change must under pppoe configure mode!\n");
		return CMD_WARNING;
	}

	if (rdc_legal_input((char *)argv[0], &s_slotid, &s_insid)) {
		vty_out (vty, "<error> input wrong para(%s)\n", argv[0]);
		return CMD_WARNING;
	}

	slot_id = vty->slotindex;
	local_id = vty->local;
	ins_id = (unsigned int)vty->index;
	dev_id = (unsigned int)vty->index_sub;

	if (NULL == (connection = get_slot_connection(slot_id))) {
		vty_out (vty, "<error> slot %d is not connect\n", slot_id);
		return CMD_WARNING;
	}

	ret = pppoe_config_radius_rdc(connection, 
								local_id, ins_id,
								dev_id, 1/*add*/,
								s_slotid, s_insid);
	if (PPPOEERR_SUCCESS == ret) {
		return CMD_SUCCESS;
	} else switch(ret) {
		case PPPOEERR_EINVAL:
			vty_out (vty, "<error> input para is error\n");
			break;
						
		case PPPOEERR_EDBUS:
			vty_out (vty, "<error> fail get reply!\n");
			break;
			
		default:
			vty_out (vty, "<error> unknow fail reason\n");
			break;
	}

	return CMD_WARNING;
}

DEFUN(no_radius_rdc_func,
	no_radius_rdc_cmd,
	"no radius rdc",
	"radius rdc delete\n"
)
{
	DBusConnection *connection;
	unsigned int slot_id, local_id, ins_id, dev_id;
	unsigned int state;
	int ret;

	if (HANSI_PPPOE_DEVICE_NODE != vty->node &&
		LOCAL_HANSI_PPPOE_DEVICE_NODE != vty->node) {
		vty_out (vty, "<error> Terminal mode change must under pppoe configure mode!\n");
		return CMD_WARNING;
	}

	slot_id = vty->slotindex;
	local_id = vty->local;
	ins_id = (unsigned int)vty->index;
	dev_id = (unsigned int)vty->index_sub;	

	if (NULL == (connection = get_slot_connection(slot_id))) {
		vty_out (vty, "<error> slot %d is not connect\n", slot_id);
		return CMD_WARNING;
	}

	ret = pppoe_config_radius_rdc(connection, 
								local_id, ins_id,
								dev_id, 0/*del*/, 0, 0);
	if (PPPOEERR_SUCCESS == ret) {
		return CMD_SUCCESS;
	} else switch(ret) {
		case PPPOEERR_EINVAL:
			vty_out (vty, "<error> input para is error\n");
			break;
						
		case PPPOEERR_EDBUS:
			vty_out (vty, "<error> fail get reply!\n");
			break;
			
		default:
			vty_out (vty, "<error> unknow fail reason\n");
			break;
	}

	return CMD_WARNING;
}



DEFUN(pppoe_radius_server_func,
	pppoe_radius_server_cmd,
	"radius server auth A.B.C.D <1-65535> SECRET acct A.B.C.D <1-65535> SECRET",
	"radius server config\n"
	"radius server author\n"	
	"radius server author ip\n"		
	"radius server author port\n"	
	"radius server author secret\n"
	"radius server account\n"	
	"radius server account ip\n"		
	"radius server account port\n"	
	"radius server account secret\n"
)
{
	DBusConnection *connection;
	unsigned int slot_id, local_id, ins_id, dev_id;
	struct radius_srv srv;
	int ret;
	
	if (HANSI_PPPOE_DEVICE_NODE != vty->node &&
		LOCAL_HANSI_PPPOE_DEVICE_NODE != vty->node) {
		vty_out (vty, "<error> Terminal mode change must under pppoe configure mode!\n");
		return CMD_WARNING;
	}

	slot_id = vty->slotindex;
	local_id = vty->local;
	ins_id = (unsigned int)vty->index;
	dev_id = (unsigned int)vty->index_sub;

	if (NULL == (connection = get_slot_connection(slot_id))) {
		vty_out (vty, "<error> slot %d is not connect\n", slot_id);
		return CMD_WARNING;
	}

	memset(&srv, 0, sizeof(struct radius_srv));
	
	if (1 == inet_pton(AF_INET, (char *)argv[0], &srv.auth.ip)) {
		if (0 == (srv.auth.ip & 0xff000000)) {
			vty_out(vty, "<error> invalid auth ip address: %s(0.X.X.X)\n", (char *)argv[0]);
			return CMD_WARNING;
		}
	} else {
		vty_out(vty, "<error> invalid auth ip address: %s\n", (char *)argv[0]);
		return CMD_WARNING;
	}

	srv.auth.port = atoi((char *)argv[1]);
	srv.auth.secretlen = strlen((char *)argv[2]);
	if (srv.auth.secretlen > (RADIUS_SECRETSIZE - 1)) {
		vty_out(vty, "<error> auth secret over max length\n");
		return CMD_WARNING;
	}
	memcpy(srv.auth.secret, argv[2], srv.auth.secretlen);

	if (1 == inet_pton(AF_INET, (char *)argv[3], &srv.acct.ip)) {
		if (0 == (srv.acct.ip & 0xff000000)) {
			vty_out(vty, "<error> invalid acct ip address: %s(0.X.X.X)\n", (char *)argv[3]);
			return CMD_WARNING;
		}
	} else {
		vty_out(vty, "<error> invalid acct ip address: %s\n", (char *)argv[3]);
		return CMD_WARNING;
	}
	srv.acct.port = atoi((char *)argv[4]);
	srv.acct.secretlen = strlen((char *)argv[5]);
	if (srv.acct.secretlen > (RADIUS_SECRETSIZE - 1)) {
		vty_out(vty, "<error> acct secret over max length\n");
		return CMD_WARNING;
	}
	memcpy(srv.acct.secret, argv[5], srv.acct.secretlen);

	if (12 == argc) {
		if (1 == inet_pton(AF_INET, (char *)argv[6], &srv.backup_auth.ip)) {
			if (0 == (srv.backup_auth.ip & 0xff000000)) {
				vty_out(vty, "<error> invalid backup auth ip address: %s(0.X.X.X)\n", (char *)argv[6]);
				return CMD_WARNING;
			}
		} else {
			vty_out(vty, "<error> invalid backup auth ip address: %s\n", (char *)argv[6]);
			return CMD_WARNING;
		}

		srv.backup_auth.port = atoi((char *)argv[7]);
		srv.backup_auth.secretlen = strlen((char *)argv[8]);
		if (srv.backup_auth.secretlen > (RADIUS_SECRETSIZE - 1)) {
			vty_out(vty, "<error> backup auth secret over max length\n");
			return CMD_WARNING;
		}
		memcpy(srv.backup_auth.secret, argv[8], srv.backup_auth.secretlen);

		if (1 == inet_pton(AF_INET, (char *)argv[9], &srv.backup_acct.ip)) {
			if (0 == (srv.backup_acct.ip & 0xff000000)) {
				vty_out(vty, "<error> invalid backup ip address: %s(0.X.X.X)\n", (char *)argv[9]);
				return CMD_WARNING;
			}
		} else {
			vty_out(vty, "<error> invalid backup ip address: %s\n", (char *)argv[9]);
			return CMD_WARNING;
		}
		srv.backup_acct.port = atoi((char *)argv[10]);
		srv.backup_acct.secretlen = strlen((char *)argv[11]);
		if (srv.backup_acct.secretlen > (RADIUS_SECRETSIZE - 1)) {
			vty_out(vty, "<error> backup acct secret over max length\n");
			return CMD_WARNING;
		}
		memcpy(srv.backup_acct.secret, argv[11], srv.backup_acct.secretlen);
	}
	

	ret = pppoe_config_radius_server(connection, local_id, ins_id, dev_id, &srv);
	if (PPPOEERR_SUCCESS == ret) {
		return CMD_SUCCESS;
	} else switch(ret) {
		case PPPOEERR_EINVAL:
			vty_out (vty, "<error> input para is error\n");
			break;
			
		case PPPOEERR_ENOEXIST:
			vty_out (vty, "<error> device is not exist\n");
			break;
						
		case PPPOEERR_EDBUS:
			vty_out (vty, "<error> fail get reply!\n");
			break;

		case PPPOEERR_ESERVICE:
			vty_out (vty, "<error> device service is enable, please disable it frist\n");
			break;
			
		default:
			vty_out (vty, "<error> unknow fail reason\n");
			break;
	}
	
	return CMD_WARNING;
}

ALIAS(pppoe_radius_server_func,
	pppoe_radius_bakup_server_cmd,
	"radius server auth A.B.C.D <1-65535> SECRET acct A.B.C.D <1-65535> SECRET "\
				"backup-auth A.B.C.D <1-65535> SECRET backup-acct A.B.C.D <1-65535> SECRET",
	"radius server config\n"
	"radius server author\n"	
	"radius server author ip\n"		
	"radius server author port\n"	
	"radius server author secret\n"
	"radius server account\n"	
	"radius server account ip\n"		
	"radius server account port\n"	
	"radius server account secret\n"
	"radius server backup author\n"	
	"radius server backup author ip\n"		
	"radius server backup author port\n"	
	"radius server backup author secret\n"
	"radius server backup account\n"	
	"radius server backup account ip\n"		
	"radius server backup account port\n"	
	"radius server backup account secret\n"	
)

DEFUN(no_pppoe_radius_server_func,
	no_pppoe_radius_server_cmd,
	"no radius server",
	"pppoe radius server config\n"
	"radius server config\n"
)
{
	DBusConnection *connection;
	unsigned int slot_id, local_id, ins_id, dev_id;
	struct radius_srv srv;
	int ret;
	
	if (HANSI_PPPOE_DEVICE_NODE != vty->node &&
		LOCAL_HANSI_PPPOE_DEVICE_NODE != vty->node) {
		vty_out (vty, "<error> Terminal mode change must under pppoe configure mode!\n");
		return CMD_WARNING;
	}

	slot_id = vty->slotindex;
	local_id = vty->local;
	ins_id = (unsigned int)vty->index;
	dev_id = (unsigned int)vty->index_sub;

	if (NULL == (connection = get_slot_connection(slot_id))) {
		vty_out (vty, "<error> slot %d is not connect\n", slot_id);
		return CMD_WARNING;
	}

	memset(&srv, 0, sizeof(struct radius_srv));
	ret = pppoe_config_radius_server(connection, local_id, ins_id, dev_id, &srv);
	if (PPPOEERR_SUCCESS == ret) {
		return CMD_SUCCESS;
	} else switch(ret) {
		case PPPOEERR_EINVAL:
			vty_out (vty, "<error> input para is error\n");
			break;
			
		case PPPOEERR_ENOEXIST:
			vty_out (vty, "<error> device is not exist\n");
			break;
						
		case PPPOEERR_EDBUS:
			vty_out (vty, "<error> fail get reply!\n");
			break;

		case PPPOEERR_ESERVICE:
			vty_out (vty, "<error> device service is enable, please disable it frist\n");
			break;
			
		default:
			vty_out (vty, "<error> unknow fail reason\n");
			break;
	}
	
	return CMD_WARNING;
}

DEFUN(pppoe_service_name_func,
	pppoe_service_name_cmd,
	"service name SNAME",
	"pppoe service name config\n"
)
{
	DBusConnection *connection;
	unsigned int slot_id, local_id, ins_id, dev_id;
	int ret;
	
	if (HANSI_PPPOE_DEVICE_NODE != vty->node &&
		LOCAL_HANSI_PPPOE_DEVICE_NODE != vty->node) {
		vty_out (vty, "<error> Terminal mode change must under pppoe configure mode!\n");
		return CMD_WARNING;
	}

	slot_id = vty->slotindex;
	local_id = vty->local;
	ins_id = (unsigned int)vty->index;
	dev_id = (unsigned int)vty->index_sub;

	if (NULL == (connection = get_slot_connection(slot_id))) {
		vty_out(vty, "<error> slot %d is not connect\n", slot_id);
		return CMD_WARNING;
	}

	if (strlen(argv[0]) > (PPPOE_NAMELEN - 1)) {
		vty_out(vty, "<error> service name over max length\n");
		return CMD_WARNING;
	}

	ret = pppoe_config_service_name(connection, local_id, ins_id, dev_id, (char *)argv[0]);
	if (PPPOEERR_SUCCESS == ret) {
		return CMD_SUCCESS;
	} else switch(ret) {
		case PPPOEERR_EINVAL:
			vty_out (vty, "<error> input para is error\n");
			break;
			
		case PPPOEERR_ENOEXIST:
			vty_out (vty, "<error> device is not exist\n");
			break;
						
		case PPPOEERR_EDBUS:
			vty_out (vty, "<error> fail get reply!\n");
			break;

		case PPPOEERR_ESERVICE:
			vty_out (vty, "<error> device service is enable, please disable it frist\n");
			break;
			
		default:
			vty_out (vty, "<error> unknow fail reason\n");
			break;
	}
	
	return CMD_WARNING;
}

DEFUN(no_pppoe_service_name_func,
	no_pppoe_service_name_cmd,
	"no service name",
	"pppoe service name delete\n"
	"service name delete\n"
)
{
	DBusConnection *connection;
	unsigned int slot_id, local_id, ins_id, dev_id;
	int ret;
	
	if (HANSI_PPPOE_DEVICE_NODE != vty->node &&
		LOCAL_HANSI_PPPOE_DEVICE_NODE != vty->node) {
		vty_out (vty, "<error> Terminal mode change must under pppoe configure mode!\n");
		return CMD_WARNING;
	}

	slot_id = vty->slotindex;
	local_id = vty->local;
	ins_id = (unsigned int)vty->index;
	dev_id = (unsigned int)vty->index_sub;

	if (NULL == (connection = get_slot_connection(slot_id))) {
		vty_out (vty, "<error> slot %d is not connect\n", slot_id);
		return CMD_WARNING;
	}

	ret = pppoe_config_service_name(connection, local_id, ins_id, dev_id, NULL);
	if (PPPOEERR_SUCCESS == ret) {
		return CMD_SUCCESS;
	} else switch(ret) {
		case PPPOEERR_EINVAL:
			vty_out (vty, "<error> input para is error\n");
			break;
			
		case PPPOEERR_ENOEXIST:
			vty_out (vty, "<error> device is not exist\n");
			break;
						
		case PPPOEERR_EDBUS:
			vty_out (vty, "<error> fail get reply!\n");
			break;

		case PPPOEERR_ESERVICE:
			vty_out (vty, "<error> device service is enable, please disable it frist\n");
			break;
			
		default:
			vty_out (vty, "<error> unknow fail reason\n");
			break;
	}
	
	return CMD_WARNING;
}



DEFUN(config_pppoe_device_service_func,
	config_pppoe_device_service_cmd,
	"service (enable|disable)",
	"pppoe device config\n"
	"pppoe service enable\n"
	"pppoe service disable\n"
)
{
	DBusConnection *connection, *pfm_connection;
	unsigned int slot_id, local_id, ins_id, dev_id, state;
	struct pfm_table_entry entry;
	unsigned int pfm_flag = 1;
	int ret;

	if (HANSI_PPPOE_DEVICE_NODE != vty->node &&
		LOCAL_HANSI_PPPOE_DEVICE_NODE != vty->node) {
		vty_out (vty, "<error> Terminal mode change must under pppoe configure mode!\n");
		return CMD_WARNING;
	}

	if (!strncmp(argv[0], "e", 1)) {
		state = 1;
	} else if (!strncmp(argv[0], "d", 1)){
		state = 0;
	} else {
		vty_out (vty, "<error> input wrong action(%s)\n", argv[0]);
		return CMD_WARNING;
	}

	slot_id = vty->slotindex;
	local_id = vty->local;
	ins_id = (unsigned int)vty->index;
	dev_id = (unsigned int)vty->index_sub;

	if (NULL == (connection = get_slot_connection(slot_id))) {
		vty_out (vty, "<error> slot %d is not connect\n", slot_id);
		return CMD_WARNING;
	}

/*	lixiang modify 2012-12-06 #########start######### */	
#if 0
	ret = pppoe_show_pfm_entry(connection, local_id, ins_id, dev_id, &entry);
	if (ret) {
		vty_out(vty, "<error> pppoe pfm entry error\n");
		return CMD_WARNING;
	}

	if (!entry.ifname[0] || entry.sendto == entry.slot_id) {
		pfm_flag = 0;	/* no need config */
		goto service;
	}

	if (!entry.sendto || entry.sendto > SLOT_MAX_NUM) {
		vty_out(vty, "<error> pppoe pfm sendto(%u) error\n",
							entry.sendto);
		return CMD_WARNING;
	}

	if (NULL == (pfm_connection = get_slot_connection(entry.sendto))) {
		vty_out(vty, "<error> pppoe pfm sendto(%u) is not connect\n",
							entry.sendto);
		return CMD_WARNING;
	}

	ret = pppoe_config_pfm_entry(pfm_connection, &entry, !state/* do pfm config */);
	switch (ret) {
	case PPPOEERR_SUCCESS:
		break;
		
	case PPPOEERR_EINVAL:
		vty_out(vty, "<error> pppoe pfm para error\n");
		return CMD_WARNING;

	case PPPOEERR_ESYSCALL:
		vty_out(vty, "<error> pppoe call pfm para failed\n");
		return CMD_WARNING;

	case PPPOEERR_EDBUS:
		vty_out(vty, "<error> fail get reply!\n");
		return CMD_WARNING;
		
	default:
		vty_out(vty, "<error> unknow pppoe pfm fail reason\n");
		return CMD_WARNING;
	}

service:
#endif
/*	lixiang modify 2012-12-06 #########end######### */
	
	ret = pppoe_config_device_service(connection, local_id, ins_id,
									dev_id, state);
	if (PPPOEERR_SUCCESS == ret) {
		return CMD_SUCCESS;
	} else switch(ret) {
		case PPPOEERR_EINVAL:
			vty_out(vty, "<error> input para is error\n");
			break;
			
		case PPPOEERR_ENOEXIST:
			vty_out(vty, "<error> device is not exist\n");
			break;
						
		case PPPOEERR_EDBUS:
			vty_out(vty, "<error> fail get reply!\n");
			break;

		case PPPOEERR_EPTHREAD:
			vty_out(vty, "<error> device create thread fail\n");
			break;

		case PPPOEERR_ESERVICE:
			vty_out(vty, "<error> device service is already enbale\n");
			break;

		case PPPOEERR_ESTATE:
			vty_out(vty, "<error> device is not up, please base interface frist\n");
			break;

		case PPPOEERR_ECONFIG:
			vty_out (vty, "<error> device config is not ready\n");
			break;
			
		default:
			vty_out (vty, "<error> unknow fail reason\n");
			break;
	}

/*	lixiang modify 2012-12-06 #########start######### */
#if 0
	if (pfm_flag) {
		pppoe_config_pfm_entry(pfm_connection, &entry, state/* undo pfm config */);
	}
#endif	
/*	lixiang modify 2012-12-06 #########end######### */

	return CMD_WARNING;
}

DEFUN(pppoe_device_kick_user_by_sid_func,
	pppoe_device_kick_user_by_sid_cmd,
	"kick user id SID",
	"pppoe user kick\n"
	"user offline\n"
	"user session id\n"
)
{
	DBusConnection *connection;
	unsigned int slot_id, local_id, ins_id, dev_id, sid;
	int ret;
	
	if (HANSI_PPPOE_DEVICE_NODE != vty->node &&
		LOCAL_HANSI_PPPOE_DEVICE_NODE != vty->node) {
		vty_out (vty, "<error> Terminal mode change must under pppoe configure mode!\n");
		return CMD_WARNING;
	}

	if (string_to_uint((char *)argv[0], &sid)) {
		vty_out (vty, "<error> user session id format is error\n");
		return CMD_WARNING;
	}

	if (!sid || sid > 0xffff) {
		vty_out (vty, "<error> user session id should be 1 to %u\n", 0xffff);
		return CMD_WARNING;
	}

	slot_id = vty->slotindex;
	local_id = vty->local;
	ins_id = (unsigned int)vty->index;
	dev_id = (unsigned int)vty->index_sub;

	if (NULL == (connection = get_slot_connection(slot_id))) {
		vty_out (vty, "<error> slot %d is not connect\n", slot_id);
		return CMD_WARNING;
	}
	
	ret = pppoe_device_kick_user(connection, local_id, ins_id, dev_id, sid, NULL);
	if (PPPOEERR_SUCCESS == ret) {
		return CMD_SUCCESS;
	} else switch(ret) {
		case PPPOEERR_EINVAL:
			vty_out (vty, "<error> input para is error\n");
			break;
			
		case PPPOEERR_ENOEXIST:
			vty_out (vty, "<error> user %u is not exist\n", sid);
			break;
						
		case PPPOEERR_EDBUS:
			vty_out (vty, "<error> fail get reply!\n");
			break;

		case PPPOEERR_ESERVICE:
			vty_out (vty, "<error> device service is not run\n");
			break;

		case PPPOEERR_ESTATE:
			vty_out (vty, "<error> hansi state is back\n");
			break;
			
		default:
			vty_out (vty, "<error> unknow fail reason\n");
			break;
	}
	
	return CMD_WARNING;
}

DEFUN(pppoe_device_kick_user_by_mac_func,
	pppoe_device_kick_user_by_mac_cmd,
	"kick user mac MAC",
	"pppoe user kick\n"
	"user offline\n"
	"user mac address\n"
)
{
	DBusConnection *connection;
	unsigned int slot_id, local_id, ins_id, dev_id;
	unsigned char mac[ETH_ALEN];
	int ret;
	
	if (HANSI_PPPOE_DEVICE_NODE != vty->node &&
		LOCAL_HANSI_PPPOE_DEVICE_NODE != vty->node) {
		vty_out (vty, "<error> Terminal mode change must under pppoe configure mode!\n");
		return CMD_WARNING;
	}

	if (str2mac((char *)argv[0], mac)) {
		vty_out (vty, "<error> invalid mac address: %s\n", (char *)argv[0]);
		return CMD_WARNING;
	}

	if (!(mac[0] | mac[1] | mac[2] | mac[3] | mac[4] | mac[5])) {
		vty_out(vty, "<error> invalid mac address: %s\n", (char *)argv[0]);
		return CMD_WARNING;
	}

	if (mac[0] & 0x01) {
		vty_out(vty, "<error> invalid mac address: %s is non-unicast\n", (char *)argv[0]);
		return CMD_WARNING;
	}

	slot_id = vty->slotindex;
	local_id = vty->local;
	ins_id = (unsigned int)vty->index;
	dev_id = (unsigned int)vty->index_sub;

	if (NULL == (connection = get_slot_connection(slot_id))) {
		vty_out (vty, "<error> slot %d is not connect\n", slot_id);
		return CMD_WARNING;
	}

	ret = pppoe_device_kick_user(connection, local_id, ins_id, dev_id, 0, mac);
	if (PPPOEERR_SUCCESS == ret) {
		return CMD_SUCCESS;
	} else switch(ret) {
		case PPPOEERR_EINVAL:
			vty_out (vty, "<error> input para is error\n");
			break;
			
		case PPPOEERR_ENOEXIST:
			vty_out (vty, "<error> user %s is not exist\n", (char *)argv[0]);
			break;
						
		case PPPOEERR_EDBUS:
			vty_out (vty, "<error> fail get reply!\n");
			break;

		case PPPOEERR_ESERVICE:
			vty_out (vty, "<error> device service is not run\n");
			break;

		case PPPOEERR_ESTATE:
			vty_out (vty, "<error> hansi state is back\n");
			break;
			
		default:
			vty_out (vty, "<error> unknow fail reason\n");
			break;
	}
	
	return CMD_WARNING;
}


DEFUN(show_pppoe_user_list_func,
	show_pppoe_user_list_cmd,
	"show user list",
	"pppoe user info\n"
)
{
	DBusConnection *connection;
	unsigned int slot_id, local_id, ins_id, dev_id;
	struct pppoeUserInfo **userarray;
	char str_ipaddr[32];
	unsigned int userNum;
	int ret;

	if (HANSI_PPPOE_DEVICE_NODE != vty->node &&
		LOCAL_HANSI_PPPOE_DEVICE_NODE != vty->node) {
		vty_out (vty, "<error> Terminal mode change must under pppoe configure mode!\n");
		return CMD_WARNING;
	}

	slot_id = vty->slotindex;
	local_id = vty->local;
	ins_id = (unsigned int)vty->index;
	dev_id = (unsigned int)vty->index_sub;

	if (NULL == (connection = get_slot_connection(slot_id))) {
		vty_out(vty, "<error> slot %d is not connect\n", slot_id);
		return CMD_WARNING;
	}

	userarray = (struct pppoeUserInfo **)calloc(DEFAULT_MAX_SESSIONID, sizeof(struct pppoeUserInfo *));
	if (!userarray) {
		vty_out(vty, "<error> memory alloc failed\n");
		return CMD_WARNING;
	}

	ret = pppoe_show_online_user_with_sort(connection, local_id, ins_id, dev_id,
								userarray, DEFAULT_MAX_SESSIONID, &userNum);
	
	vty_out(vty, "user num : %u\n", ret ? 0 : userNum);
	vty_out(vty,"==============================================================================\n");
	vty_out(vty,"%-7s %-17s %-18s %-16s %-10s\n","UserID","UserName","UserMAC","UserIP","OnlineTime");
	
	if (PPPOEERR_SUCCESS == ret && userNum) {
		int i;
		for (i = 0; i < userNum; i++) {
			unsigned int hour = userarray[i]->sessTime / 3600;
			unsigned int minute = userarray[i]->sessTime / 60 - hour * 60;
			unsigned int second = userarray[i]->sessTime % 60;

			memset(str_ipaddr, 0, sizeof(str_ipaddr));
			snprintf(str_ipaddr, sizeof(str_ipaddr), "%u.%u.%u.%u",
					(userarray[i]->ip >> 24) & 0xff, (userarray[i]->ip >> 16) & 0xff,
					(userarray[i]->ip >> 8) & 0xff, userarray[i]->ip & 0xff);

			vty_out(vty, hour < 100 ? "%-7u %-17s %02X:%02X:%02X:%02X:%02X:%02X  %-16s %02u:%02u:%02u\n" 
						: "%-7u %-17s %02X:%02X:%02X:%02X:%02X:%02X  %-16s %u:%02u:%02u\n", 
						userarray[i]->sid, userarray[i]->username, 
						userarray[i]->mac[0], userarray[i]->mac[1], userarray[i]->mac[2],
						userarray[i]->mac[3], userarray[i]->mac[4], userarray[i]->mac[5],
						str_ipaddr, hour, minute, second);
		}
		pppoe_online_user_free(userarray, userNum);	
	}
	vty_out(vty,"==============================================================================\n");

	free(userarray);		
	return CMD_SUCCESS;
}

DEFUN(config_pppoe_log_debug_func,
	config_pppoe_log_debug_cmd,
	"pppoe log debug (enable|disable)",
	"pppoe config\n"
	"pppoe log\n"
	"log debug\n"
	"open pppoe log debug\n"
	"close pppoe log debug\n"
)
{
	DBusConnection *connection;
	unsigned int slot_id, local_id, ins_id;
	unsigned int state;
	int ret;

	if (HANSI_NODE != vty->node &&
		LOCAL_HANSI_NODE != vty->node) {
		vty_out (vty, "<error> Terminal mode change must under hansi configure mode!\n");
		return CMD_WARNING;
	}

	if (!strncmp(argv[0], "e", 1)) {
		state = PPPOE_LOG_DEBUG_ON;
	} else if (!strncmp(argv[0], "d", 1)){
		state = PPPOE_LOG_DEBUG_OFF;
	} else {
		vty_out (vty, "<error> input wrong action(%s)\n", argv[0]);
		return CMD_WARNING;
	}
	
	slot_id = vty->slotindex;
	local_id = vty->local;
	ins_id = (unsigned int)vty->index;

	if (NULL == (connection = get_slot_connection(slot_id))) {
		vty_out (vty, "<error> slot %d is not connect\n", slot_id);
		return CMD_WARNING;
	}
	
	ret = pppoe_config_log_debug(connection, local_id, ins_id,
							0, state);

	if (ret) {
		vty_out (vty, "<error> %s pppoe log debug fail\n", argv[0]);
		return CMD_WARNING;
	}
	
	return CMD_SUCCESS;
}

DEFUN(config_pppoe_log_token_func,
	config_pppoe_log_token_cmd,
	"pppoe log token (instance|backup|netlink|thread|tbus|method|"
					"manage|radius|discover|control|lcp|chap|ccp|ipcp|rdc) (enable|disable)",
	"pppoe config\n"
	"pppoe log\n"
	"log token\n"
	"instance token\n"
	"backup token\n"
	"netlink token\n"
	"thread token\n"
	"tbus token\n"
	"method token\n"
	"manage token\n"
	"radius token\n"
	"discover token\n"
	"control token\n"
	"lcp token\n"
	"chap token\n"
	"ccp token\n"
	"ipcp token\n"
	"rdc token\n"
	"open pppoe log token\n"
	"close pppoe log token\n"
)
{
	DBusConnection *connection;
	unsigned int slot_id, local_id, ins_id;
	PPPOELogToken token;
	unsigned int state;
	int ret;

	if (HANSI_NODE != vty->node &&
		LOCAL_HANSI_NODE != vty->node) {
		vty_out (vty, "<error> Terminal mode change must under hansi configure mode!\n");
		return CMD_WARNING;
	}

	if (!strcmp(argv[0], "instance")) {
		token = TOKEN_INSTANCE;
	} else if (!strcmp(argv[0], "backup")){
		token = TOKEN_BACKUP;
	} else if (!strcmp(argv[0], "netlink")) {
		token = TOKEN_NETLINK;
	} else if (!strcmp(argv[0], "thread")) {
		token = TOKEN_THREAD;
	} else if (!strcmp(argv[0], "tbus")) {
		token = TOKEN_TBUS;
	} else if (!strcmp(argv[0], "method")) {
		token = TOKEN_METHOD;
	} else if (!strcmp(argv[0], "manage")) {
		token = TOKEN_MANAGE;
	} else if (!strcmp(argv[0], "radius")) {
		token = TOKEN_RADIUS;
	} else if (!strcmp(argv[0], "discover")) {
		token = TOKEN_DISCOVER;
	} else if (!strcmp(argv[0], "control")) {
		token = TOKEN_CONTROL;
	} else if (!strcmp(argv[0], "lcp")) {
		token = TOKEN_LCP;
	} else if (!strcmp(argv[0], "chap")) {
		token = TOKEN_CHAP;
	} else if (!strcmp(argv[0], "ccp")) {
		token = TOKEN_CCP;
	} else if (!strcmp(argv[0], "ipcp")) {
		token = TOKEN_IPCP;
	} else if (!strcmp(argv[0], "rdc")) {
		token = TOKEN_RDC;
	} else {
		vty_out (vty, "<error> input wrong token(%s)\n", argv[0]);
		return CMD_WARNING;
	}

	if (!strncmp(argv[1], "e", 1)) {
		state = PPPOE_LOG_DEBUG_ON;
	} else if (!strncmp(argv[1], "d", 1)){
		state = PPPOE_LOG_DEBUG_OFF;
	} else {
		vty_out (vty, "<error> input wrong action(%s)\n", argv[1]);
		return CMD_WARNING;
	}
	
	slot_id = vty->slotindex;
	local_id = vty->local;
	ins_id = (unsigned int)vty->index;

	if (NULL == (connection = get_slot_connection(slot_id))) {
		vty_out (vty, "<error> slot %d is not connect\n", slot_id);
		return CMD_WARNING;
	}
	
	ret = pppoe_config_log_token(connection, local_id, ins_id,
								token, state);

	if (ret) {
		vty_out (vty, "<error> %s pppoe log token fail\n", argv[1]);
		return CMD_WARNING;
	}
	
	return CMD_SUCCESS;
}

DEFUN(show_pppoe_running_config_func,
	show_pppoe_running_config_cmd,
	"show pppoe running config",
	"pppoe running config\n"
)
{
	DBusConnection *connection;
	unsigned int slot_id, local_id, ins_id;
	char *configCmd;
	int ret;

	if (HANSI_NODE != vty->node &&
		LOCAL_HANSI_NODE != vty->node) {
		vty_out (vty, "<error> Terminal mode change must under hansi configure mode!\n");
		return CMD_WARNING;
	}

	slot_id = vty->slotindex;
	local_id = vty->local;
	ins_id = (unsigned int)vty->index;

	if (NULL == (connection = get_slot_connection(slot_id))) {
		vty_out(vty, "<error> slot %d is not connect\n", slot_id);
		return CMD_WARNING;
	}

	ret = pppoe_show_running_config(connection, local_id, ins_id, &configCmd);
	if (PPPOEERR_SUCCESS == ret && configCmd) {
		vty_out(vty,"==============================================================================\n");
		vty_out(vty,"%s", configCmd);
		vty_out(vty,"==============================================================================\n");

		free(configCmd);
		return CMD_SUCCESS;
	} 
	
	return CMD_WARNING;
}


char *
dcli_pppoe_show_running_config(int localid, int slot_id, int index) {
	DBusConnection *connection;
	char *configCmd;
	
	if (localid < 0 || localid > 1 || slot_id < 1 || slot_id > 16 || index < 1 || index > 16) {
		return NULL;
	}
	
	if (NULL == (connection = get_slot_connection(slot_id))) {
		return NULL;
	}
	
	if (!pppoe_show_running_config(connection, localid, index, &configCmd)) {
		return configCmd;
	}	

	return NULL;
}



void 
dcli_pppoe_init(void) {

	install_node(&pppoe_node, NULL, "pppoe_node");
	install_default(PPPOE_NODE);
	install_element(CONFIG_NODE, &conf_pppoe_cmd);
	    
	install_node(&hansi_pppoe_node, NULL, "hansi_pppoe_node");
	install_node(&local_hansi_pppoe_node, NULL, "local_hansi_pppoe_node");

	install_default(HANSI_PPPOE_DEVICE_NODE);
	install_default(LOCAL_HANSI_PPPOE_DEVICE_NODE);

	install_element(HANSI_NODE, &show_pppoe_running_config_cmd); 
	install_element(LOCAL_HANSI_NODE, &show_pppoe_running_config_cmd);

	install_element(HANSI_NODE, &create_pppoe_device_cmd); 
	install_element(LOCAL_HANSI_NODE, &create_pppoe_device_cmd); 

	install_element(HANSI_NODE, &delete_pppoe_device_cmd); 
	install_element(LOCAL_HANSI_NODE, &delete_pppoe_device_cmd); 

	install_element(HANSI_NODE, &show_pppoe_device_list_cmd); 
	install_element(LOCAL_HANSI_NODE, &show_pppoe_device_list_cmd);

	install_element(HANSI_NODE, &config_pppoe_device_cmd); 
	install_element(LOCAL_HANSI_NODE, &config_pppoe_device_cmd); 

	install_element(HANSI_NODE, &config_pppoe_log_debug_cmd); 
	install_element(LOCAL_HANSI_NODE, &config_pppoe_log_debug_cmd); 

	install_element(HANSI_NODE, &config_pppoe_log_token_cmd); 
	install_element(LOCAL_HANSI_NODE, &config_pppoe_log_token_cmd); 
	
	install_element(HANSI_PPPOE_DEVICE_NODE, &base_pppoe_device_cmd); 
	install_element(LOCAL_HANSI_PPPOE_DEVICE_NODE, &base_pppoe_device_cmd);

	install_element(HANSI_PPPOE_DEVICE_NODE, &no_base_pppoe_device_cmd); 
	install_element(LOCAL_HANSI_PPPOE_DEVICE_NODE, &no_base_pppoe_device_cmd);

/*	lixiang modify 2012-12-06 #########start######### */	
#if 0
	install_element(HANSI_PPPOE_DEVICE_NODE, &apply_pppoe_device_cmd); 
	install_element(LOCAL_HANSI_PPPOE_DEVICE_NODE, &apply_pppoe_device_cmd);

	install_element(HANSI_PPPOE_DEVICE_NODE, &no_apply_pppoe_device_cmd); 
	install_element(LOCAL_HANSI_PPPOE_DEVICE_NODE, &no_apply_pppoe_device_cmd);
#endif
/*	lixiang modify 2012-12-06 #########end######### */

	install_element(HANSI_PPPOE_DEVICE_NODE, &pppoe_device_ipaddr_cmd); 
	install_element(LOCAL_HANSI_PPPOE_DEVICE_NODE, &pppoe_device_ipaddr_cmd);

	install_element(HANSI_PPPOE_DEVICE_NODE, &no_pppoe_device_ipaddr_cmd); 
	install_element(LOCAL_HANSI_PPPOE_DEVICE_NODE, &no_pppoe_device_ipaddr_cmd);

	install_element(HANSI_PPPOE_DEVICE_NODE, &pppoe_device_virtual_mac_cmd); 
	install_element(LOCAL_HANSI_PPPOE_DEVICE_NODE, &pppoe_device_virtual_mac_cmd);

	install_element(HANSI_PPPOE_DEVICE_NODE, &no_pppoe_device_virtual_mac_cmd); 
	install_element(LOCAL_HANSI_PPPOE_DEVICE_NODE, &no_pppoe_device_virtual_mac_cmd);

	install_element(HANSI_PPPOE_DEVICE_NODE, &pppoe_session_ipaddr_cmd); 
	install_element(LOCAL_HANSI_PPPOE_DEVICE_NODE, &pppoe_session_ipaddr_cmd);

	install_element(HANSI_PPPOE_DEVICE_NODE, &no_pppoe_session_ipaddr_cmd); 
	install_element(LOCAL_HANSI_PPPOE_DEVICE_NODE, &no_pppoe_session_ipaddr_cmd);

	install_element(HANSI_PPPOE_DEVICE_NODE, &pppoe_session_dns_cmd); 
	install_element(LOCAL_HANSI_PPPOE_DEVICE_NODE, &pppoe_session_dns_cmd);

	install_element(HANSI_PPPOE_DEVICE_NODE, &no_pppoe_session_dns_cmd); 
	install_element(LOCAL_HANSI_PPPOE_DEVICE_NODE, &no_pppoe_session_dns_cmd);

	install_element(HANSI_PPPOE_DEVICE_NODE, &pppoe_nas_ipaddr_cmd); 
	install_element(LOCAL_HANSI_PPPOE_DEVICE_NODE, &pppoe_nas_ipaddr_cmd);

	install_element(HANSI_PPPOE_DEVICE_NODE, &no_pppoe_nas_ipaddr_cmd); 
	install_element(LOCAL_HANSI_PPPOE_DEVICE_NODE, &no_pppoe_nas_ipaddr_cmd);

	install_element(HANSI_PPPOE_DEVICE_NODE, &config_radius_rdc_cmd); 
	install_element(LOCAL_HANSI_PPPOE_DEVICE_NODE, &config_radius_rdc_cmd); 

	install_element(HANSI_PPPOE_DEVICE_NODE, &no_radius_rdc_cmd); 
	install_element(LOCAL_HANSI_PPPOE_DEVICE_NODE, &no_radius_rdc_cmd); 
	
	install_element(HANSI_PPPOE_DEVICE_NODE, &pppoe_radius_server_cmd); 
	install_element(LOCAL_HANSI_PPPOE_DEVICE_NODE, &pppoe_radius_server_cmd);

	install_element(HANSI_PPPOE_DEVICE_NODE, &pppoe_radius_bakup_server_cmd); 
	install_element(LOCAL_HANSI_PPPOE_DEVICE_NODE, &pppoe_radius_bakup_server_cmd);

	install_element(HANSI_PPPOE_DEVICE_NODE, &no_pppoe_radius_server_cmd); 
	install_element(LOCAL_HANSI_PPPOE_DEVICE_NODE, &no_pppoe_radius_server_cmd);
	
	install_element(HANSI_PPPOE_DEVICE_NODE, &pppoe_service_name_cmd); 
	install_element(LOCAL_HANSI_PPPOE_DEVICE_NODE, &pppoe_service_name_cmd);

	install_element(HANSI_PPPOE_DEVICE_NODE, &no_pppoe_service_name_cmd); 
	install_element(LOCAL_HANSI_PPPOE_DEVICE_NODE, &no_pppoe_service_name_cmd);

	install_element(HANSI_PPPOE_DEVICE_NODE, &config_pppoe_device_service_cmd); 
	install_element(LOCAL_HANSI_PPPOE_DEVICE_NODE, &config_pppoe_device_service_cmd);

	install_element(HANSI_PPPOE_DEVICE_NODE, &show_pppoe_user_list_cmd); 
	install_element(LOCAL_HANSI_PPPOE_DEVICE_NODE, &show_pppoe_user_list_cmd);

	install_element(HANSI_PPPOE_DEVICE_NODE, &pppoe_device_kick_user_by_sid_cmd); 
	install_element(LOCAL_HANSI_PPPOE_DEVICE_NODE, &pppoe_device_kick_user_by_sid_cmd);

	install_element(HANSI_PPPOE_DEVICE_NODE, &pppoe_device_kick_user_by_mac_cmd); 
	install_element(LOCAL_HANSI_PPPOE_DEVICE_NODE, &pppoe_device_kick_user_by_mac_cmd);
}

#ifdef __cplusplus
}
#endif
