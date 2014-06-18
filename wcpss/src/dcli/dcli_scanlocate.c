#ifdef _D_WCPSS_
#include <string.h>
#include <zebra.h>
#include <dbus/dbus.h>
#include <sys/stat.h>
#include <sys/wait.h>
#include <fcntl.h>
#include <unistd.h>
#include "command.h"
#include "vtysh/vtysh.h"

#include "../dcli_main.h"

#include "wcpss/waw.h"
#include "wcpss/wid/WID.h"
#include "wid_ac.h"
#include "wid_wtp.h"
#include "dcli_wtp.h"
#include "dcli_scanlocate.h"
#include "dcli_radio.h"
#include "wid_scanlocate.h"
#include "wid_radio.h"
#include "dbus/wcpss/dcli_wid_wtp.h"
#include "dbus/asd/ASDDbusDef1.h"
#include "dbus/wcpss/ACDbusDef1.h"
#include "memory.h"
#include "sysdef/npd_sysdef.h"
#include "wcpss/asd/asd.h"
#include <dirent.h>

struct cmd_node scanlocate_node =
{
	SCANLOCATE_NODE,
	"%s(config-air-monitor-service)# "
};

struct cmd_node hansi_scanlocate_node =
{
	HANSI_SCANLOCATE_NODE,
	"%s(hansi-air-monitor-service %d-%d)# "
};

/*
struct cmd_node scanlocate_node1 =
{
	SCANLOCATE_NODE1,
	" "
};
*/

char *dcli_u32ip2str(unsigned int u32_ipaddr)
{	
#if 1
	struct in_addr inaddr;

	inaddr.s_addr = u32_ipaddr;

	return inet_ntoa(inaddr);
#else
	int len = sizeof("255.255.255.255\0");
	
	memset(static_buffer, 0, len);
	snprintf(static_buffer, sizeof(static_buffer), "%u.%u:%u.%u", 
		((u32_ipaddr >> 24) & 0xff), ((u32_ipaddr >> 16) & 0xff),
		((u32_ipaddr >> 8) & 0xff), ((u32_ipaddr >> 0) & 0xff));
	
	return static_buffer;

#endif
}


DEFUN(conf_scanlocate_func,
	conf_scanlocate_cmd,
	"config air-monitor-service",
	CONFIG_STR
	"config air-monitor-service\n"
)
{
	if (CONFIG_NODE == vty->node)
	{
		vty->node = SCANLOCATE_NODE;
		//vty->index = NULL;	
	}
	else if (HANSI_NODE == vty->node)
	{
		vty->node = HANSI_SCANLOCATE_NODE;
	}
	else
	{
		vty_out (vty, "Terminal mode change must under configure mode!\n");
		return CMD_WARNING;
	}

	return CMD_SUCCESS;
}

DEFUN(create_scanlocate_channel_cmd_func,
		create_scanlocate_channel_cmd,
		"add service (wifi|rrm|rfid) INDEX scan-general channel CHANNEL interval TIME dwell TIME [2.4G/5.8G]",
		"add service\n"
		"scan-locate service\n"
		"wifi service\n"
		"rrm service\n"
		"rfid scan service\n"
		"service INDEX\n"
		"general scan config\n"
		"scan channel\n"
		"CHANNEL in 2.4G(1-13)  eg: 1-3,12\n"
		"channel scan interval\n"
		"NTERVAL(ms)  like: 0\n"
		"channe scan dwell time\n"
		"DWELL(ms)  like: 200\n")
{
	unsigned int ret = 0;
	unsigned char type = 0;
	unsigned int index= 0;
	int localid = 0;
	int slot_id = HostSlotId;
	unsigned int id = 0;
	unsigned int channel_scan_interval = 0;	
	unsigned int channel_scan_dwell = 0;	
	unsigned long long channel = 0;
	unsigned char channel_flag = 0;

	//handle parameter 0
	if (!strncmp("wifi", argv[0], strlen(argv[0])))
	{
		type = 1;
	}
	else if (!strncmp("rrm", argv[0], strlen(argv[0])))
	{
		type = 2;
		vty_out(vty, "%% this version not support rrm\n");
		return CMD_SUCCESS;
	}
	else if (!strncmp("rfid", argv[0], strlen(argv[0])))
	{
		type = 3;
		vty_out(vty, "%% this version not support rfid\n");
		return CMD_SUCCESS;
	}
	else
	{
		vty_out(vty,"%% unknown input\n");
		return CMD_SUCCESS;
	}

	//handle parameter 1
	ret = parse_int_ID((char *)argv[1], &id);
	if (WID_DBUS_SUCCESS != ret)
	{
		vty_out(vty, "%% unknown index format\n");
		return CMD_SUCCESS;
	}
	
	if ((WIFI_LOCATE_CONFIG_GROUP_SIZE < id) || ( WIFI_LOCATE_DEFAULT_CONFIG >id)) 
	{
		vty_out(vty, "%% id should be %d to %d\n",
				WIFI_LOCATE_DEFAULT_CONFIG,
				WIFI_LOCATE_CONFIG_GROUP_SIZE);
		return CMD_SUCCESS;
	}
	
	ret = parse_int_ID((char *)argv[3], &channel_scan_interval);
	if (WID_DBUS_SUCCESS != ret)
	{
		vty_out(vty, "%% illegal channel scan interval time\n");
		return CMD_SUCCESS;
	}

	ret = parse_int_ID((char *)argv[4], &channel_scan_dwell);
	if (WID_DBUS_SUCCESS != ret)
	{
		vty_out(vty, "%% illegal channel dwell time\n");
		return CMD_SUCCESS;
	}

	if(argc == 6)
	{	//handle parameter 5
		if (!strncmp("2.4G", argv[5], strlen(argv[5])))
		{
			channel_flag = WIFI_LOCATE_2_4G;
			
			//handle parameter 2
			ret = change_channel_to_bit_map((unsigned char *)argv[2], strlen(argv[2]), &channel);
			if (WID_DBUS_SUCCESS != ret)
			{
				vty_out(vty, "%% illegal channel list\n");
				return CMD_SUCCESS;
			}
		}
		else if (!strncmp("5.8G", argv[5], strlen(argv[5])))
		{
			channel_flag = WIFI_LOCATE_5_8G;
			
			//handle parameter 2
			ret = change_channel_to_bit_map_5_8G((unsigned char *)argv[2], strlen(argv[2]), &channel);
			if (WID_DBUS_SUCCESS != ret)
			{
				vty_out(vty, "%% illegal channel list\n");
				return CMD_SUCCESS;
			}
		}
	}
	else if(argc == 5)
	{
		ret = change_channel_to_bit_map((unsigned char *)argv[2], strlen(argv[2]), &channel);
		if (WID_DBUS_SUCCESS != ret)
		{
			vty_out(vty, "%% illegal channel list\n");
			return CMD_SUCCESS;
		}

	}
	
	if (SCANLOCATE_NODE == vty->node)
	{
		index = 0;
	}
	else if (HANSI_SCANLOCATE_NODE == vty->node)
	{
		index = (unsigned int)vty->index;
		localid = vty->local;
		slot_id = vty->slotindex;
	}
	else
	{
		vty_out(vty,"%% unsupport mode\n");
		return CMD_WARNING;
	}

	DBusConnection *dcli_dbus_connection = NULL;
	ReInitDbusConnection(&dcli_dbus_connection,slot_id,distributFag);
	
	ret  = wid_add_scanlocate_general_channel(localid, index, type, id, channel, 
										(unsigned short)channel_scan_interval,
										(unsigned short)channel_scan_dwell,
										channel_flag, dcli_dbus_connection);
	
	if (WID_DBUS_SUCCESS == ret)
	{
		vty_out(vty, "set %s config group %d scan-general-channel successfully\n", (type==1)?"wifi":((type==2)?"rrm":"rfid"), id);
	}
	else
	{
		vty_out(vty, "error: %d\n", ret);
	}
	
	return CMD_SUCCESS;
}

DEFUN(create_scanlocate_report_cmd_func,
		create_scanlocate_report_cmd,
		"add service (wifi|rrm|rfid) INDEX scan-general report (tcp|udp) SERVER-IP PORT <1000-20000> [2.4G/5.8G]",
		"add service\n"
		"scan-locate service\n"
		"wifi service\n"
		"rrm service\n"
		"rfid scan service\n"
		"service INDEX\n"
		"general scan config\n"
		"scan report\n"
		"report to server by tcp\n"
		"report to server by udp\n"
		"SERVER-IP like: 192.168.1.1\n"
		"SERVER-PORT like: 5400\n"
		"REPORT INTERVAL(ms)  min: 1000 max: 20000  like: 1000\n")
{
	unsigned int ret = 0;
	unsigned char type = 0;
	unsigned int index= 0;
	unsigned int id = 0;
	unsigned char report_pattern = 0;
	unsigned int server_ip = 0;
	unsigned int server_port = 0;
	unsigned int report_interval = 0;	
	unsigned char channel_flag = 0;
	int localid = 0;
	int slot_id = HostSlotId;
	
	if (!strncmp("wifi", argv[0], strlen(argv[0])))
	{
		type = 1;
	}
	else if (!strncmp("rrm", argv[0], strlen(argv[0])))
	{
		type = 2;
		vty_out(vty, "%% this version not support rrm\n");
		return CMD_SUCCESS;
	}
	else if (!strncmp("rfid", argv[0], strlen(argv[0])))
	{
		type = 3;
		vty_out(vty, "%% this version not support rfid\n");
		return CMD_SUCCESS;
	}
	else
	{
		vty_out(vty,"%% unknown input\n");
		return CMD_SUCCESS;
	}

	ret = parse_int_ID((char *)argv[1], &id);
	if (WID_DBUS_SUCCESS != ret)
	{
		vty_out(vty, "%% unknown index format\n");
		return CMD_SUCCESS;
	}
	
	if ((WIFI_LOCATE_CONFIG_GROUP_SIZE < id) || ( WIFI_LOCATE_DEFAULT_CONFIG >id)) 
	{
		vty_out(vty, "%% id should be %d to %d\n",
				WIFI_LOCATE_DEFAULT_CONFIG,
				WIFI_LOCATE_CONFIG_GROUP_SIZE);
		return CMD_SUCCESS;
	}

	if (!strncmp("tcp", argv[2], strlen(argv[2])))
	{
		report_pattern = 2;
	}
	else if (!strncmp("udp", argv[2], strlen(argv[2])))
	{
		report_pattern = 1;
	}
	else
	{
		vty_out(vty,"%% unknown input\n");
		return CMD_SUCCESS;
	}

	ret = WID_Check_IP_Format((char*)argv[3]);
	if(WID_DBUS_SUCCESS != ret)
	{
		vty_out(vty,"%% unknown ip format\n");
		return CMD_SUCCESS;
	}
	server_ip = dcli_ip2ulong((char*)argv[3]);

	ret = parse_int_ID((char *)argv[4], &server_port);
	if (WID_DBUS_SUCCESS != ret)
	{
		vty_out(vty, "%% illegal server port\n");
		return CMD_SUCCESS;
	}

	ret = parse_int_ID((char *)argv[5], &report_interval);
	if (WID_DBUS_SUCCESS != ret)
	{
		vty_out(vty, "%% illegal report time interval\n");
		return CMD_SUCCESS;
	}

	if(argc == 7)
	{	
		if (!strncmp("2.4G", argv[6], strlen(argv[6])))
		{
			channel_flag = WIFI_LOCATE_2_4G;
		}
		else if (!strncmp("5.8G", argv[6], strlen(argv[6])))
		{
			channel_flag = WIFI_LOCATE_5_8G;
		}
	}
	
	if (SCANLOCATE_NODE == vty->node)
	{
		index = 0;
	}
	else if (HANSI_SCANLOCATE_NODE == vty->node)
	{
		index = (unsigned int)vty->index;
		localid = vty->local;
		slot_id = vty->slotindex;
	}
	else
	{
		vty_out(vty,"%% unsupport mode\n");
		return CMD_WARNING;
	}

	DBusConnection *dcli_dbus_connection = NULL;
	ReInitDbusConnection(&dcli_dbus_connection,slot_id,distributFag);
	
	ret  = wid_add_scanlocate_general_report(localid, index, type, id, report_pattern, server_ip,
										(unsigned short)server_port,
										(unsigned short)report_interval,
										channel_flag,
										dcli_dbus_connection);

	if (WID_DBUS_SUCCESS == ret)
	{
		vty_out(vty, "set %s config group %d scan general report successfully\n", (type==1)?"wifi":((type==2)?"rrm":"rfid"), id);
	}
	else
	{
		//vty_out(vty, "%s\n", dcli_wid_opcode2string(ret));
	}
	
	return CMD_SUCCESS;
}

DEFUN(create_scanlocate_wifi_specail_cmd_func,
		create_scanlocate_wifi_specail_cmd,
		"add service wifi INDEX scan-special scan-type <0-2> rssi <0-95> [2.4G/5.8G]",
		"add service\n"
		"scan-locate service\n"
		"wifi service\n"
		"service INDEX\n"
		"special scan config\n"
		"scan type\n"
		"SCAN-TYPE: 0:all 1:assocation 2: no-assocation\n"
		"rssi\n"
		"RSSI like: 20\n")
{
	unsigned int ret = 0;
	unsigned int index= 0;
	unsigned int id = 0;
	unsigned int scan_type = 0;
	unsigned int rssi = 0;
	unsigned char channel_flag = 0;
	int localid = 0;
	int slot_id = HostSlotId;
	
	ret = parse_int_ID((char *)argv[0], &id);
	if (WID_DBUS_SUCCESS != ret)
	{
		vty_out(vty, "%% unknown index format\n");
		return CMD_SUCCESS;
	}
	
	if ((WIFI_LOCATE_CONFIG_GROUP_SIZE < id) || ( WIFI_LOCATE_DEFAULT_CONFIG >id)) 
	{
		vty_out(vty, "%% id should be %d to %d\n",
				WIFI_LOCATE_DEFAULT_CONFIG,
				WIFI_LOCATE_CONFIG_GROUP_SIZE);
		return CMD_SUCCESS;
	}

	ret = parse_int_ID((char *)argv[1], &scan_type);
	if (WID_DBUS_SUCCESS != ret)
	{
		vty_out(vty, "%% Illegal scan typen");
		return CMD_SUCCESS;
	}

	ret = parse_int_ID((char *)argv[2], &rssi);
	if (WID_DBUS_SUCCESS != ret)
	{
		vty_out(vty, "%% Illegal rssi\n");
		return CMD_SUCCESS;
	}

	if(argc == 4)
	{	
		if (!strncmp("2.4G", argv[3], strlen(argv[3])))
		{
			channel_flag = WIFI_LOCATE_2_4G;
		}
		else if (!strncmp("5.8G", argv[3], strlen(argv[3])))
		{
			channel_flag = WIFI_LOCATE_5_8G;
		}
	}

	if (SCANLOCATE_NODE == vty->node)
	{
		index = 0;
	}
	else if (HANSI_SCANLOCATE_NODE == vty->node)
	{
		index = (unsigned int)vty->index;
		localid = vty->local;
		slot_id = vty->slotindex;
	}
	else
	{
		vty_out(vty,"%% unsupport mode\n");
		return CMD_WARNING;
	}

	DBusConnection *dcli_dbus_connection = NULL;
	ReInitDbusConnection(&dcli_dbus_connection,slot_id,distributFag);
	ret  = wid_add_scanlocate_wifi_special_config(localid, index, id,(unsigned char)scan_type, 
											(unsigned char)rssi,
											channel_flag,
											dcli_dbus_connection);

	if (WID_DBUS_SUCCESS == ret)
	{
		vty_out(vty, "set wifi config group %d scan-special-config successfully\n", id);
	}
	else
	{
		//vty_out(vty, "%s\n", dcli_wid_opcode2string(ret));
	}
	
	return CMD_SUCCESS;
}

DEFUN(delete_scanlocate_group_cmd_func,
		delete_scanlocate_group_cmd,
		"delete service (wifi|rrm|rfid) INDEX",
		"delete service\n"
		"scan-locate service\n"
		"wifi service\n"
		"rrm service\n"
		"rfid scan service\n"
		"service INDEX\n")
{
	unsigned int ret = 0;
	int len = 0;
	unsigned char type = 0;
	unsigned int index= 0;
	unsigned int id= 0;
	unsigned int channel_scan_interval = 0;	
	unsigned int channel_scan_dwell = 0;	
	int localid = 0;
	int slot_id = HostSlotId;
	
	if (!strncmp("wifi", argv[0], strlen(argv[0])))
	{
		type = 1;
	}
	else if (!strncmp("rrm", argv[0], strlen(argv[0])))
	{
		type = 2;
		vty_out(vty, "%% this version not support rrm\n");
		return CMD_SUCCESS;
	}
	else if (!strncmp("rfid", argv[0], strlen(argv[0])))
	{
		type = 3;
		vty_out(vty, "%% this version not support rfid\n");
		return CMD_SUCCESS;
	}
	else
	{
		vty_out(vty,"%% unknown input\n");
		return CMD_SUCCESS;
	}

	ret = parse_int_ID((char *)argv[1], &id);
	if (WID_DBUS_SUCCESS != ret)
	{
		vty_out(vty, "%% unknown index format\n");
		return CMD_SUCCESS;
	}
	
	if ((WIFI_LOCATE_CONFIG_GROUP_SIZE < id)
		|| ( WIFI_LOCATE_CONFIG_GROUP_BEGIN >id)) 
	{
		vty_out(vty, "%% id should be %d to %d\n", WIFI_LOCATE_CONFIG_GROUP_BEGIN,
											WIFI_LOCATE_CONFIG_GROUP_SIZE);
		return CMD_SUCCESS;
	}

	if (SCANLOCATE_NODE == vty->node)
	{
		index = 0;
	}
	else if (HANSI_SCANLOCATE_NODE == vty->node)
	{
		index = (unsigned int)vty->index;
		localid = vty->local;
		slot_id = vty->slotindex;
	}
	else
	{
		vty_out(vty,"%% unsupport mode\n");
		return CMD_WARNING;
	}

	DBusConnection *dcli_dbus_connection = NULL;
	ReInitDbusConnection(&dcli_dbus_connection,slot_id,distributFag);
	ret = wid_delete_scanlocate_general_channel(localid, index, type, id, dcli_dbus_connection);
	
	if (WID_DBUS_SUCCESS == ret)
	{
		vty_out(vty, "delete %s config group %d successfully\n", (type==1)?"wifi":((type==2)?"rrm":"rfid"), id);
	}
	else
	{
		//vty_out(vty, "%s\n", dcli_wid_opcode2string(ret));
	}
	
	return CMD_SUCCESS;
}

DEFUN(modify_scanlocate_general_config_cmd_func,
		modify_scanlocate_general_config_cmd,
		"modify service (wifi|rrm|rfid) INDEX scan-general (channel|dwell|scan-interval|tcp|udp|report-interval|server-ip|server-port) PARAMTER (2.4G|5.8G) [SNMP-FLAG] [WTP_MAC]",
		"delete service\n"
		"scan-locate service\n"
		"wifi service\n"
		"rrm service\n"
		"rfid scan service\n"
		"service INDEX\n"
		"general scan config\n"
		"scan channel\n"
		"channe scan dwell time\n"
		"channel scan interval\n"
		"report to server by tcp\n"
		"report to server by udp\n"
		"report interval\n"
		"server ip\n"
		"server port\n"
		"PARAMTER\n")
{
	unsigned int ret = 0;
	unsigned char type = 0;
	unsigned char modify_type = 0;
	unsigned int index= 0;
	unsigned int id= 0;
	unsigned long long  channellist = 0;
	unsigned int channel_scan_interval = 0;	
	unsigned int channel_scan_dwell = 0;	
	unsigned char report_pattern = 0;
	unsigned int server_ip = 0;
	unsigned int server_port = 0;
	unsigned int report_interval = 0;	
	unsigned int snmp_flag = 0;
	WIDMACADDR  wtp_mac;
	unsigned char channel_flag = 0;
	int localid = 0;
	int slot_id = HostSlotId;
	
	memset(&wtp_mac, 0, sizeof(WIDMACADDR));
	
	if (!strncmp("wifi", argv[0], strlen(argv[0])))
	{
		type = 1;
	}
	else if (!strncmp("rrm", argv[0], strlen(argv[0])))
	{
		type = 2;
		vty_out(vty, "%% this version not support rrm\n");
		return CMD_SUCCESS;
	}
	else if (!strncmp("rfid", argv[0], strlen(argv[0])))
	{
		type = 3;
		vty_out(vty, "%% this version not support rfid\n");
		return CMD_SUCCESS;
	}
	else
	{
		vty_out(vty,"%% unknown input\n");
		return CMD_SUCCESS;
	}

	ret = parse_int_ID((char *)argv[1], &id);
	if (WID_DBUS_SUCCESS != ret)
	{
		vty_out(vty, "%% unknown index format\n");
		return CMD_SUCCESS;
	}
	
	if ((WIFI_LOCATE_CONFIG_GROUP_SIZE < id)
		|| ( WIFI_LOCATE_CONFIG_GROUP_BEGIN >id)) 
	{
		vty_out(vty, "%% id should be %d to %d\n", WIFI_LOCATE_CONFIG_GROUP_SIZE,
											WIFI_LOCATE_CONFIG_GROUP_BEGIN);
		return CMD_SUCCESS;
	}

	if (!strncmp("channel", argv[2], strlen(argv[2])))
	{
		modify_type = 1;
	}
	else if (!strncmp("dwell", argv[2], strlen(argv[2])))
	{
		modify_type = 2;
		ret = parse_int_ID((char *)argv[3], &channel_scan_dwell);
		if (WID_DBUS_SUCCESS != ret)
		{
			vty_out(vty, "%% illegal channel scan dwell time\n");
			return CMD_SUCCESS;
		}
	}
	else if (!strncmp("scan-interval", argv[2], strlen(argv[2])))
	{
		modify_type = 3;
		ret = parse_int_ID((char *)argv[3], &channel_scan_interval);
		if (WID_DBUS_SUCCESS != ret)
		{
			vty_out(vty, "%% illegal channel scan interval time\n");
			return CMD_SUCCESS;
		}
	}
	else if (!strncmp("tcp", argv[2], strlen(argv[2])))
	{
		modify_type = 4;
		report_pattern = 2;
	}
	else if (!strncmp("udp", argv[2], strlen(argv[2])))
	{
		modify_type = 4;
		report_pattern = 1;
	}
	else if (!strncmp("report-interval", argv[2], strlen(argv[2])))
	{
		modify_type = 5;
		ret = parse_int_ID((char *)argv[3], &report_interval);
		if (WID_DBUS_SUCCESS != ret)
		{
			vty_out(vty, "%% illegal report interval\n");
			return CMD_SUCCESS;
		}
	}
	else if (!strncmp("server-ip", argv[2], strlen(argv[2])))
	{
		modify_type = 6;
		ret = WID_Check_IP_Format((char*)argv[3]);
		if(WID_DBUS_SUCCESS != ret)
		{
			vty_out(vty,"%% unknown ip format\n");
			return CMD_SUCCESS;
		}
		server_ip = dcli_ip2ulong((char*)argv[3]);
	}
	else if (!strncmp("server-port", argv[2], strlen(argv[2])))
	{
		modify_type = 7;
		ret = parse_int_ID((char *)argv[3], &server_port);
		if (WID_DBUS_SUCCESS != ret)
		{
			vty_out(vty, "%% illegal server port\n");
			return CMD_SUCCESS;
		}
	}
	else
	{
		vty_out(vty,"%% unknown input\n");
		return CMD_SUCCESS;
	}

	if (!strncmp("2.4G", argv[4], strlen(argv[4])))
	{
		channel_flag = WIFI_LOCATE_2_4G;
		if(modify_type == 1)
		{
			ret = change_channel_to_bit_map((unsigned char *)argv[3],strlen(argv[3]),&channellist);
			if (WID_DBUS_SUCCESS != ret)
			{
				vty_out(vty, "%% illegal channel list\n");
				return CMD_SUCCESS;
			}
		}
	}
	else if (!strncmp("5.8G", argv[4], strlen(argv[4])))
	{
		channel_flag = WIFI_LOCATE_5_8G;
		if(modify_type == 1)
		{
			ret = change_channel_to_bit_map_5_8G((unsigned char *)argv[3],strlen(argv[3]),&channellist);
			if (WID_DBUS_SUCCESS != ret)
			{
				vty_out(vty, "%% illegal channel list\n");
				return CMD_SUCCESS;
			}
		}
	}

	if(argc == 7)
	{
		ret = parse_int_ID((char *)argv[5], &snmp_flag);
		if (WID_DBUS_SUCCESS != ret)
		{
			vty_out(vty, "%% illegal snmp flag\n");
			return CMD_SUCCESS;
		}

		ret = wid_parse_mac_addr((char *)argv[6],&wtp_mac);
		if (CMD_FAILURE == ret) 
		{
			vty_out(vty,"%% unknown mac addr format.\n");
			return CMD_WARNING;
		}
	}

	if (SCANLOCATE_NODE == vty->node)
	{
		index = 0;
	}
	else if (HANSI_SCANLOCATE_NODE == vty->node)
	{
		index = (unsigned int)vty->index;
		localid = vty->local;
		slot_id = vty->slotindex;
	}
	else
	{
		vty_out(vty,"%% unsupport mode\n");
		return CMD_WARNING;
	}

	DBusConnection *dcli_dbus_connection = NULL;
	ReInitDbusConnection(&dcli_dbus_connection,slot_id,distributFag);
	ret = wid_modify_scanlocate_general_config(localid, index, type, id, modify_type, channellist,
										(unsigned short)channel_scan_dwell,
										(unsigned short)channel_scan_interval,
										report_pattern,  (unsigned short)report_interval,
										server_ip, (unsigned short)server_port,wtp_mac.macaddr, 
										(unsigned char)snmp_flag,channel_flag,
										dcli_dbus_connection);
	
	if (WID_DBUS_SUCCESS == ret)
	{
		vty_out(vty, "modify general config successfully\n");
	}
	else
	{
		//vty_out(vty, "%s\n", dcli_wid_opcode2string(ret));
	}
	
	return CMD_SUCCESS;
}

DEFUN(modify_scanlocate_wifi_specal_config_cmd_func,
		modify_scanlocate_wifi_specal_config_cmd,
		"modify service wifi INDEX scan-special (rssi|scan-type) PARAMTER (2.4G|5.8G) [SNMP-FLAG] [WTP_MAC]",
		"delete service\n"
		"scan-locate service\n"
		"wifi service\n"
		"service INDEX\n"
		"special scan config\n"
		"scan type\n"
		"rssi\n"
		"PARAMTER\n")
{
	unsigned int ret = 0;
	unsigned char modify_type = 0;
	unsigned int index= 0;
	unsigned int id= 0;
	unsigned int scan_type = 0;
	unsigned int rssi = 0;
	unsigned int snmp_flag = 0;
	unsigned char channel_flag = 0;
	int localid = 0;
	int slot_id = HostSlotId;
	
	ret = parse_int_ID((char *)argv[0], &id);
	if (WID_DBUS_SUCCESS != ret)
	{
		vty_out(vty, "%% unknown index format\n");
		return CMD_SUCCESS;
	}
	
	if ((WIFI_LOCATE_CONFIG_GROUP_SIZE < id) || ( WIFI_LOCATE_CONFIG_GROUP_BEGIN >id)) 
	{
		vty_out(vty, "%% id should be %d to %d\n",
				WIFI_LOCATE_CONFIG_GROUP_BEGIN,
				WIFI_LOCATE_CONFIG_GROUP_SIZE);
		return CMD_SUCCESS;
	}

	if (!strncmp("rssi", argv[1], strlen(argv[1])))
	{
		modify_type = 1;
		ret = parse_int_ID((char *)argv[2], &rssi);
		if (WID_DBUS_SUCCESS != ret)
		{
			vty_out(vty, "%% Illegal rssi\n");
			return CMD_SUCCESS;
		}
	}
	else if (!strncmp("scan-type", argv[1], strlen(argv[1])))
	{
		modify_type = 2;
		ret = parse_int_ID((char *)argv[2], &scan_type);
		if (WID_DBUS_SUCCESS != ret)
		{
			vty_out(vty, "%% Illegal scan type\n");
			return CMD_SUCCESS;
		}
	}
	else
	{
		vty_out(vty,"%% unknown input\n");
		return CMD_SUCCESS;
	}

	if (!strncmp("2.4G", argv[3], strlen(argv[3])))
	{
		channel_flag = WIFI_LOCATE_2_4G;
	}
	else if (!strncmp("5.8G", argv[3], strlen(argv[3])))
	{
		channel_flag = WIFI_LOCATE_5_8G;
	}

	WIDMACADDR  wtp_mac;
	memset(&wtp_mac, 0, sizeof(WIDMACADDR));
	if(argc == 6)
	{
		ret = parse_int_ID((char *)argv[4], &snmp_flag);
		if (WID_DBUS_SUCCESS != ret)
		{
			vty_out(vty, "%% Illegal server port\n");
			return CMD_SUCCESS;
		}
		

		ret = wid_parse_mac_addr((char *)argv[5],&wtp_mac);
		if (CMD_FAILURE == ret) 
		{
			vty_out(vty,"%% Unknown mac addr format.\n");
			return CMD_WARNING;
		}
	}

	
	if (SCANLOCATE_NODE == vty->node)
	{
		index = 0;
	}
	else if (HANSI_SCANLOCATE_NODE == vty->node)
	{
		index = (unsigned int)vty->index;
		localid = vty->local;
		slot_id = vty->slotindex;
	}
	else
	{
		vty_out(vty,"%% unsupport mode\n");
		return CMD_WARNING;
	}

	vty_out(vty," modify type %d\n", modify_type);
	vty_out(vty," snmp_flag: %d\n", snmp_flag);
	vty_out(vty," wtp: "MACSTR" \n", MAC2STR(wtp_mac.macaddr));
	vty_out(vty," id: %d\n", id);
	vty_out(vty," rssi: %d\n", rssi);

	DBusConnection *dcli_dbus_connection = NULL;
	ReInitDbusConnection(&dcli_dbus_connection,slot_id,distributFag);
	ret = wid_modify_scanlocate_wifi_special_config(localid, index,  id, modify_type, 
											(unsigned char)rssi, 
											(unsigned char)scan_type, 
											wtp_mac.macaddr,
											(unsigned char)snmp_flag,
											channel_flag,
											dcli_dbus_connection);
	
	if (WID_DBUS_SUCCESS == ret)
	{
		vty_out(vty, "modify special config successfully\n");
	}
	else
	{
		//vty_out(vty, "%s\n", dcli_wid_opcode2string(ret));
	}
	
	return CMD_SUCCESS;
}

DEFUN(set_radiolist_bind_mac_filter_group_cmd_func,
	set_radiolist_bind_mac_filter_group_cmd,
	"(add|delete) service (rfid|rrm|wifi) INDEX apply radio RADIOIDLIST",
	"add serivce to radiolist\n"
	"delete serivce to radiolist\n"
	"scan-locate service\n"
	"wifi service\n"
	"rrm service\n"
	"rfid scan service\n"
	"service INDEX\n"
	"set scan-locate service apply radiolist\n"
	RADIO_LIST
	RADIO_LIST_STR
	)
{
	unsigned int i = 0;
	unsigned int index = 0;
	unsigned int group_id = 0;
	unsigned char isadd = 0;
	unsigned char type = 0;
	unsigned int id = 0;
	update_radio_list *radiolist = NULL;
	struct res_head res_head;
	unsigned int ret = 0;
	int localid = 0;
	int slot_id = HostSlotId;
	
	if (SCANLOCATE_NODE == vty->node)
	{
		index = 0;
	}
	else if (HANSI_SCANLOCATE_NODE == vty->node)
	{
		index = (unsigned int)vty->index;
		localid = vty->local;
		slot_id = vty->slotindex;
	}
	else
	{
		vty_out(vty,"%% unsupport mode\n");
		return CMD_WARNING;
	}

	if (!strncmp("add", argv[0], strlen(argv[0])))
	{
		isadd = 1;
	}
	else if (!strncmp("delete", argv[0], strlen(argv[0])))
	{
		isadd = 0;
	}
	else
	{
		vty_out(vty,"%% unknown input\n");
		return CMD_SUCCESS;
	}

	if (!strncmp("wifi", argv[1], strlen(argv[1])))
	{
		type = 1;
	}
	else if (!strncmp("rrm", argv[1], strlen(argv[1])))
	{
		type = 2;
		vty_out(vty, "%% this version not support rrm\n");
		return CMD_SUCCESS;
	}
	else if (!strncmp("rfid", argv[1], strlen(argv[1])))
	{
		type = 3;
		vty_out(vty, "%% this version not support rfid\n");
		return CMD_SUCCESS;
	}
	else
	{
		vty_out(vty,"%% unknown input\n");
		return CMD_SUCCESS;
	}

	ret = parse_int_ID((char *)argv[2], &id);
	if (WID_DBUS_SUCCESS != ret)
	{
		vty_out(vty, "%% unknown index format\n");
		return CMD_SUCCESS;
	}
	
	if ((WIFI_LOCATE_CONFIG_GROUP_SIZE < id) || ( WIFI_LOCATE_DEFAULT_CONFIG >id)) 
	{
		vty_out(vty, "%% id should be %d to %d\n",
				WIFI_LOCATE_DEFAULT_CONFIG,
				WIFI_LOCATE_CONFIG_GROUP_SIZE);
		return CMD_SUCCESS;
	}
	
	radiolist = (struct tag_wtpid_list*)malloc(sizeof(struct tag_radioid_list));
	if (!radiolist)
	{	
		vty_out(vty, "%% No enough memory.\n");
		return CMD_SUCCESS;
	}
	memset(radiolist, 0, sizeof(struct tag_radioid_list));

	ret = parse_radio_list((char*)argv[3], radiolist);
	if(ret != 0)
	{
		vty_out(vty, "%% input radioid format wrong,should like 4,8,3-0,3-1\n");
		destroy_input_radio_list(radiolist);
		return CMD_SUCCESS;
	}
	else
	{
		delsame_radio(radiolist);		
	}

	memset(&res_head, 0, sizeof(res_head));

	DBusConnection *dcli_dbus_connection = NULL;
	ReInitDbusConnection(&dcli_dbus_connection,slot_id,distributFag);
	ret = wid_set_scanlocate_config_to_radiolist(localid, index, type, id,radiolist, isadd, &res_head, dcli_dbus_connection);

	if(WID_DBUS_SUCCESS == ret)
	{
		vty_out(vty, "%s service %s  %s apply radio list successfully\n", argv[0],argv[1],argv[2]);
	}
	if (CONFIG_GROUP_NOT_EXIST == ret)
	{
	vty_out(vty,"wifi group %d not exist\n",id);
	}
	
	for (i = 0; i < res_head.num; i++)
	{
		if (res_head.node[i].res)
		{
		/*
			vty_out(vty, "RADIO %d-%d(%d): %s\n",
				res_head.node[i].u.radioid / L_RADIO_NUM, 
				res_head.node[i].u.radioid % L_RADIO_NUM, 
				res_head.node[i].u.radioid, 
				dcli_wid_opcode2string(res_head.node[i].res));
				*/
		}
	}

	if (radiolist)
	{
		destroy_input_radio_list(radiolist);
	}

	if (res_head.node)
	{
		free(res_head.node);
		res_head.node = NULL;
		res_head.num = 0;
	}

	return CMD_SUCCESS;    
}

/*for debug wifi-locate*/
DEFUN(set_wtp_wifi_locate_general_config_cmd_func,
		set_wtp_wifi_locate_general_config_cmd,
		"set wifi-locate wtp MAC (enable|disable) type <0-2> report <1000-20000>  rssi RSSI serverip IP port PORT channel-scan TIME interval TIME (2.4G|5.8G) channel CHANNEL ",
		WTP_SET
		"wifi-locate\n"
		WTP_WTP
		"Wtp Mac\n"
		"set wifi-locate enable\n"
		"set wifi-locate disable\n"
		"scan-type\n"
		"scan-type: 0,1,2\n"
		"report-interval\n"
		"report-interval: 1-20 seconds\n"
		"rssi\n"
		"RSSI(0-95) eg:  90 \n"
		"server ip \n"
		"SERVER IP eg: 192.168.3.1\n"
		"server port\n"
		"server port eg: 5244\n"
		"channel scan dwell time\n"
		"channel scan dwell time\n"
		"channel scan interval time\n"
		"channel scan interval time\n"
		"channel list\n"
		"channel list like:1-5,6\n"
		)
{
	
	int temp_time = 0;
	unsigned int ret = 0;
	unsigned int index = 0;
	unsigned char isenable = 0;   
	unsigned short time_interval = 0;
	unsigned int scan_type = 0;
	unsigned int report_interval = 0;
	unsigned int rssi = 0;
	unsigned int server_ip = 0;
	unsigned int server_port = 0;
	WIDMACADDR  wtp_mac;
	unsigned int channel_scan_time = 0;	
	unsigned int channel_scan_interval = 0;
	unsigned long long channel = 0;
	unsigned char channel_flag = 0;   
	int localid = 0;
	int slot_id = HostSlotId;
	
	memset(&wtp_mac, 0, sizeof(WIDMACADDR));
	
	ret = wid_parse_mac_addr((char *)argv[0],&wtp_mac);
	if (CMD_FAILURE == ret) 
	{
		vty_out(vty,"%% Unknown mac addr format.\n");
		return CMD_WARNING;
	}

	if (!strncmp("enable", argv[1], strlen(argv[1])))
	{
		isenable = 1;
	}
	else if (!strncmp("disable", argv[1], strlen(argv[1])))
	{
		isenable = 0;
	}
	else
	{
		vty_out(vty,"%% unknown input\n");
		return CMD_SUCCESS;		
	}

	ret = parse_int_ID((char *)argv[2], &scan_type);
	if (WID_DBUS_SUCCESS != ret)
	{
		vty_out(vty, "%% Illegal scan type, range <0-2>\n");
		return CMD_SUCCESS;
	}

	ret = parse_int_ID((char *)argv[3], &report_interval);
	if (WID_DBUS_SUCCESS != ret)
	{
		vty_out(vty, "%% Illegal report_interval, range <1-20>\n");
		return CMD_SUCCESS;
	}

	ret = parse_int_ID((char *)argv[4], &rssi);
	if (WID_DBUS_SUCCESS != ret)
	{
		vty_out(vty, "%% Illegal rssi\n");
		return CMD_SUCCESS;
	}

	server_ip = dcli_ip2ulong((char*)argv[5]);

	ret = parse_int_ID((char *)argv[6], &server_port);
	if (WID_DBUS_SUCCESS != ret)
	{
		vty_out(vty, "%% Illegal server_port\n");
		return CMD_SUCCESS;
	}

	ret = parse_int_ID((char *)argv[7], &channel_scan_time);
	if (WID_DBUS_SUCCESS != ret)
	{
		vty_out(vty, "%% Illegal channel_scan_time\n");
		return CMD_SUCCESS;
	}

	ret = parse_int_ID((char *)argv[8], &channel_scan_interval);
	if (WID_DBUS_SUCCESS != ret)
	{
		vty_out(vty, "%% Illegal channel_scan_interval\n");
		return CMD_SUCCESS;
	}

	if (!strncmp("2.4G", argv[9], strlen(argv[9])))
	{
		channel_flag = WIFI_LOCATE_2_4G;
		ret = change_channel_to_bit_map((unsigned char *)argv[10],strlen(argv[10]),&channel);
		if (WID_DBUS_SUCCESS != ret)
		{
			vty_out(vty, "%% illegal channel list (1-13)\n");
			return CMD_SUCCESS;
		}
	}
	else if (!strncmp("5.8G", argv[9], strlen(argv[9])))
	{
		channel_flag = WIFI_LOCATE_5_8G;
		ret = change_channel_to_bit_map_5_8G((unsigned char *)argv[10],strlen(argv[10]),&channel);
		if (WID_DBUS_SUCCESS != ret)
		{
			vty_out(vty, "%% illegal channel list (1-13)\n");
			return CMD_SUCCESS;
		}
	}

	if (SCANLOCATE_NODE == vty->node)
	{
		index = 0;
	}
	else if (HANSI_SCANLOCATE_NODE == vty->node)
	{
		index = (unsigned int)vty->index;
		localid = vty->local;
		slot_id = vty->slotindex;
	}
	else
	{
		vty_out(vty,"%% unsupport mode\n");
		return CMD_WARNING;
	}

	DBusConnection *dcli_dbus_connection = NULL;
	ReInitDbusConnection(&dcli_dbus_connection,slot_id,distributFag);
	ret = snmp_wifi_locate_set_public_config(localid, index, isenable, wtp_mac.macaddr, (unsigned short)report_interval, (unsigned char)scan_type, 
										channel, (unsigned short)channel_scan_interval,
									 (unsigned short)channel_scan_time, (unsigned char)rssi, server_ip,
									 (unsigned short)server_port ,channel_flag, dcli_dbus_connection);
	if(WID_DBUS_SUCCESS == ret)
	{
		vty_out(vty, "set wifi-locate enable successfully\n");
	}
	else
	{
		//vty_out(vty, "%s\n", dcli_wid_opcode2string(ret));
	}
	
	return CMD_SUCCESS; 
}

DEFUN(show_wtp_wifi_locate_public_onfig_cmd_func,
		show_wtp_wifi_locate_public_config_cmd,
		"show wifi-locate config wtp MAC (2.4G|5.8G)",
		SHOW_STR
		"wifi-locate\n"
		"wifi-locate config\n"
		"wtp\n"
		"wtp mac eg:00:1f:64:35:2e:17\n")
{
	int i=0, j=0;
	unsigned int ret = 0;
	unsigned int index = 0;
	WIDMACADDR  wtp_mac;
	int localid = 0;
	int slot_id = HostSlotId;
	
	struct dcli_wifi_locate_public_config public_config;
	memset(&public_config, 0, sizeof(struct dcli_wifi_locate_public_config));
	unsigned char channel_flag = 0;  

	ret = wid_parse_mac_addr((char *)argv[0],&wtp_mac);
	if (CMD_FAILURE == ret) 
	{
		vty_out(vty,"%% Unknown mac addr format.\n");
		return CMD_WARNING;
	}

	if (!strncmp("2.4G", argv[1], strlen(argv[1])))
	{
		channel_flag = WIFI_LOCATE_2_4G;
	}
	else if (!strncmp("5.8G", argv[1], strlen(argv[1])))
	{
		channel_flag = WIFI_LOCATE_5_8G;
	}
	
	if (SCANLOCATE_NODE == vty->node)
	{
		index = 0;
	}
	else if (HANSI_SCANLOCATE_NODE == vty->node)
	{
		index = (unsigned int)vty->index;
		localid = vty->local;
		slot_id = vty->slotindex;
	}
	else
	{
		vty_out(vty,"%% unsupport mode\n");
		return CMD_WARNING;
	}
	
	DBusConnection *dcli_dbus_connection = NULL;
	ReInitDbusConnection(&dcli_dbus_connection,slot_id,distributFag);
	ret = snmp_wifi_locate_show_public_config_wtp(localid, index, &public_config, wtp_mac.macaddr, channel_flag, dcli_dbus_connection);


	if (WID_DBUS_SUCCESS == ret )
	{
		vty_out(vty,"CSI: CHANNEL SCAN INTERVAL (ms)\n");
		vty_out(vty,"CST: CHANNEL SCAN TIME (ms)\n");
		vty_out(vty,"RI:  REPORT INTERVAL (ms)\n");
		vty_out(vty,"TYPE:\n");
		vty_out(vty,"     ASS: scan associated station\n");
		vty_out(vty,"     NON: scan non-associated station \n");
		vty_out(vty,"     ALL: scan ASS and NON\n");
		vty_out(vty,"=======================================================================\n");
		vty_out(vty, "%-*s %-*s %-*s %-*s %-*s %-*s %-*s %-*s %-*s\n",
				strlen("XX:XX:XX:XX:XX:XX"), "WTPMAC",
				strlen("DISABLE"), "STATE",
				strlen("CSI"), "CSI",
				strlen("CST"), "CST",
				strlen("XXXX"), "RI",
				strlen("RSSI"), "RSSI",
				strlen("TYPE"), "TYPE",
				strlen("XXX.XXX.XXX.XXX"), "SERVER-IP",
				strlen("PORT"), "PORT"
				);

		vty_out(vty, MACSTR" %-*s %-*d %-*d %-*d %-*d %-*s %-*s %-*d \n",
			MAC2STR(wtp_mac.macaddr),
			strlen("DISABLE"), (public_config.state ==0)?"disable":"enable",
			strlen("CSI"),  public_config.channel_scan_interval,
			strlen("CST"), public_config.channel_scan_time,
			strlen("XXXX"), public_config.report_interval,
			strlen("RSSI"), public_config.rssi,
			strlen("TYPE"), (public_config.scan_type==0)?"ALL":((public_config.scan_type==1)?"ASS":"NON"),
			strlen("XXX.XXX.XXX.XXX"), dcli_u32ip2str(public_config.server_ip),
			strlen("PORT"), public_config.server_port
			);
		
		switch(channel_flag)
		{
			case WIFI_LOCATE_2_4G:
				vty_out(vty, "%-*s %s",0, "", "2.4G channel:");
				for(j=0; j<MAX_CHANNEL_2_4G; j++)
				{
					if((public_config.channel & BIT(j)) == BIT(j))
					{
						vty_out(vty, "%d ", j+1);
					}
				}
				vty_out(vty, "\n");
				break;
			case WIFI_LOCATE_5_8G:
				vty_out(vty, "%-*s %s",0, "", "5.8G channel:");
				for(j=16; j<64; j++)
				{
					if((public_config.channel & ((unsigned long long)(1)<<(j))) == ((unsigned long long)(1)<<(j)))
					{
						vty_out(vty, "%d ", change_to_5Gchannel(j+1));
					}
				}
				vty_out(vty, "\n");
		}

		vty_out(vty,"=======================================================================\n");	
		
	}
	 if(MAC_DOESNOT_EXIT ==  ret)
	{
		vty_out(vty, "mac %s in not exist\n",argv[0]);
	}

	return CMD_SUCCESS; 
}

DEFUN(show_wtp_wifi_locate_public_onfig_cmd_all_func,
		show_wtp_wifi_locate_public_config_all_cmd,
		"show wifi-locate config all (2.4G|5.8G)",
		SHOW_STR
		"wifi-locate\n"
		"wifi-locate config\n"
		"wtp\n"
		"wtp mac eg:00:1f:64:35:2e:17\n")
{
	int i=0, j=0;
	unsigned int ret = 0;
	unsigned int index = 0;
	unsigned wtp_num = 0;
	struct dcli_wifi_locate_public_config *public_config = NULL;
	unsigned char channel_flag = 0;
	int localid = 0;
	int slot_id = HostSlotId;
	
	if (!strncmp("2.4G", argv[0], strlen(argv[0])))
	{
		channel_flag = WIFI_LOCATE_2_4G;
	}
	else if (!strncmp("5.8G", argv[0], strlen(argv[0])))
	{
		channel_flag = WIFI_LOCATE_5_8G;
	}

	if (SCANLOCATE_NODE == vty->node)
	{
		index = 0;
	}
	else if (HANSI_SCANLOCATE_NODE == vty->node)
	{
		index = (unsigned int)vty->index;
		localid = vty->local;
		slot_id = vty->slotindex;
	}
	else
	{
		vty_out(vty,"%% unsupport mode\n");
		return CMD_WARNING;
	}

	DBusConnection *dcli_dbus_connection = NULL;
	ReInitDbusConnection(&dcli_dbus_connection,slot_id,distributFag);
	ret = snmp_wifi_locate_show_public_config_all_wtp(localid, index, &public_config, &wtp_num, channel_flag, dcli_dbus_connection);

	if(WID_DBUS_SUCCESS != ret)
	{
		//vty_out(vty, "%s\n", dcli_wid_opcode2string(ret));
	}
	else
	{
		vty_out(vty,"CSI: CHANNEL SCAN INTERVAL (ms)\n");
		vty_out(vty,"CST: CHANNEL SCAN TIME (ms)\n");
		vty_out(vty,"RI:  REPORT INTERVAL (ms)\n");
		vty_out(vty,"TYPE:\n");
		vty_out(vty,"     ASS: scan associated station\n");
		vty_out(vty,"     NON: scan non-associated station \n");
		vty_out(vty,"     ALL: scan ASS and NON\n");
		vty_out(vty,"=============================================================================\n");
		vty_out(vty, "%-*s  %-*s %-*s %-*s %-*s %-*s %-*s %-*s %-*s %-*s\n",
				strlen("WTPID"), "WTPID",
				strlen("XX:XX:XX:XX:XX:XX"), "WTPMAC",
				strlen("DISABLE"), "STATE",
				strlen("CSI"), "CSI",
				strlen("CST"), "CST",
				strlen("XXXX"), "RI",
				strlen("RSSI"), "RSSI",
				strlen("TYPE"), "TYPE",
				strlen("XXX.XXX.XXX.XXX"), "SERVER-IP",
				strlen("PORT"), "PORT"
				);

		for(i=0; i<wtp_num; i++)
		{
			vty_out(vty, "%-*d  "MACSTR" %-*s %-*d %-*d %-*d %-*d %-*s %-*s %-*d \n",
			strlen("WTPID"), public_config[i].wtpid,
			MAC2STR(public_config[i].wtp_mac),
			strlen("DISABLE"), (public_config[i].state ==0)?"disable":"enable",
			strlen("CSI"),  public_config[i].channel_scan_interval,
			strlen("CST"), public_config[i].channel_scan_time,
			strlen("XXXX"), public_config[i].report_interval,
			strlen("RSSI"), public_config[i].rssi,
			strlen("TYPE"), (public_config[i].scan_type==0)?"ALL":((public_config[i].scan_type==1)?"ASS":"NON"),
			strlen("XXX.XXX.XXX.XXX"), dcli_u32ip2str(public_config[i].server_ip),
			strlen("PORT"), public_config[i].server_port
			);
			switch(channel_flag)
			{
				case WIFI_LOCATE_2_4G:
					vty_out(vty, "%-*s %s",7, " ", "2.4G channel:");
					for(j=0; j<MAX_CHANNEL_2_4G; j++)
					{
						if((public_config[i].channel & BIT(j)) == BIT(j))
						{
							vty_out(vty, "%d ", j+1);
						}
					}
					vty_out(vty, "\n");
					break;
				case WIFI_LOCATE_5_8G:
					vty_out(vty, "%-*s %s",7, " ", "5.8G channel:");
					for(j=16; j<64; j++)
					{
						if((public_config[i].channel & ((unsigned long long)(1)<<(j))) == ((unsigned long long)(1)<<(j)))
						{
							vty_out(vty, "%d ", change_to_5Gchannel(j+1));
						}
					}
					vty_out(vty, "\n");
			}
			vty_out(vty, "\n");

		}
		vty_out(vty,"=============================================================================\n");	
	}

	if(public_config!= NULL)
	{
		free(public_config);
		public_config = NULL;
	}
	return CMD_SUCCESS; 
}

DEFUN(set_wtp_wifi_locate_on_off_cmd_func,
		set_wtp_wifi_locate_on_off_cmd,
		"set wifi-locate wtp MAC (enable|disable) (2.4G|5.8G)",
		SHOW_STR
		"wifi-locate\n"
		"wifi-locate config\n"
		"wtp\n"
		"wtp mac eg:00:1f:64:35:2e:17\n")
{
	int i=0, j=0;
	unsigned int ret = 0;
	unsigned int index = 0;
	unsigned wtp_num = 0;
	unsigned char isenable = 0;
	WIDMACADDR  wtp_mac;
	unsigned char channel_flag = 0;
	int localid = 0;
	int slot_id = HostSlotId;
	
	if (SCANLOCATE_NODE == vty->node)
	{
		index = 0;
	}
	else if (HANSI_SCANLOCATE_NODE == vty->node)
	{
		index = (unsigned int)vty->index;
		localid = vty->local;
		slot_id = vty->slotindex;
	}
	else
	{
		vty_out(vty,"%% unsupport mode\n");
		return CMD_WARNING;
	}

	ret = wid_parse_mac_addr((char *)argv[0],&wtp_mac);
	if (CMD_FAILURE == ret) 
	{
		vty_out(vty,"%% Unknown mac addr format.\n");
		return CMD_WARNING;
	}

	if (!strncmp("enable", argv[1], strlen(argv[1])))
	{
		isenable = 1;
	}
	else if (!strncmp("disable", argv[1], strlen(argv[1])))
	{
		isenable = 0;
	}
	else
	{
		vty_out(vty,"%% unknown input\n");
		return CMD_SUCCESS;
	}

	if (!strncmp("2.4G", argv[2], strlen(argv[2])))
	{
		channel_flag = WIFI_LOCATE_2_4G;
	}
	else if (!strncmp("5.8G", argv[2], strlen(argv[2])))
	{
		channel_flag = WIFI_LOCATE_5_8G;
	}

	DBusConnection *dcli_dbus_connection = NULL;
	ReInitDbusConnection(&dcli_dbus_connection,slot_id,distributFag);
	ret = snmp_wifi_locate_change_on_off(localid, index, wtp_mac.macaddr, isenable, channel_flag, dcli_dbus_connection);

	if(WID_DBUS_SUCCESS == ret)
	{
		vty_out(vty, "set wifi-locate wtp %s %s %s successfully \n",argv[0],argv[1],argv[2] );
	}
	
	else if(MAC_DOESNOT_EXIT ==  ret)
	{
		vty_out(vty, "mac %s in not exist\n",argv[0]);
	}
	else
		vty_out(vty, "<err> %d\n",ret);


	return CMD_SUCCESS; 
}

DEFUN(set_wtp_wifi_locate_on_off_all_cmd_func,
		set_wtp_wifi_locate_on_off_all_cmd,
		"set wifi-locate all (enable|disable) (2.4G|5.8G)",
		SHOW_STR
		"wifi-locate\n"
		"wifi-locate config\n"
		"wtp\n"
		"wtp mac eg:00:1f:64:35:2e:17\n")
{
	int i=0, j=0;
	unsigned int ret = 0;
	unsigned int index = 0;
	unsigned wtp_num = 0;
	unsigned char isenable = 0;
	WIDMACADDR  wtp_mac;
	unsigned char channel_flag = 0;
	int localid = 0;
	int slot_id = HostSlotId;
	
	if (SCANLOCATE_NODE == vty->node)
	{
		index = 0;
	}
	else if (HANSI_SCANLOCATE_NODE == vty->node)
	{
		index = (unsigned int)vty->index;
		localid = vty->local;
		slot_id = vty->slotindex;
	}
	else
	{
		vty_out(vty,"%% unsupport mode\n");
		return CMD_WARNING;
	}

	if (!strncmp("enable", argv[0], strlen(argv[0])))
	{
		isenable = 1;
	}
	else if (!strncmp("disable", argv[0], strlen(argv[0])))
	{
		isenable = 0;
	}
	else
	{
		vty_out(vty,"%% unknown input\n");
		return CMD_SUCCESS;
	}

	if (!strncmp("2.4G", argv[1], strlen(argv[1])))
	{
		channel_flag = WIFI_LOCATE_2_4G;
	}
	else if (!strncmp("5.8G", argv[1], strlen(argv[1])))
	{
		channel_flag = WIFI_LOCATE_5_8G;
	}

	DBusConnection *dcli_dbus_connection = NULL;
	ReInitDbusConnection(&dcli_dbus_connection,slot_id,distributFag);
	ret = snmp_wifi_locate_change_on_off_all_wtp(localid, index, isenable, channel_flag, dcli_dbus_connection);

	if(WID_DBUS_SUCCESS == ret)
	{
		vty_out(vty, "set wifi-locate all %s %s successfully\n",argv[0],argv[1]);
	}
	else
		vty_out(vty, "<err> %d\n",ret);


	return CMD_SUCCESS; 
}

DEFUN(show_wifi_config_cmd_func,
		show_wifi_config_cmd,
		"show service wifi INDEX",
		"show service\n"
		"wifi service\n"
		"wifi service\n"
		"service INDEX\n")
{
	unsigned int ret = 0;
	unsigned int index= 0;
	unsigned int id = 0;
	int i = 0;
	int localid = 0;
	int slot_id = HostSlotId;
	
	struct dcli_wifi_locate_public_config public_config;
	memset(&public_config, 0, sizeof(struct dcli_wifi_locate_public_config));
	
	ret = parse_int_ID((char *)argv[0], &id);
	if (WID_DBUS_SUCCESS != ret)
	{
		vty_out(vty, "%% unknown index format\n");
		return CMD_SUCCESS;
	}
	
	if ((WIFI_LOCATE_CONFIG_GROUP_SIZE < id) || ( WIFI_LOCATE_DEFAULT_CONFIG > id)) 
	{
		vty_out(vty, "%% index should be %d to %d\n", WIFI_LOCATE_DEFAULT_CONFIG, 
											WIFI_LOCATE_CONFIG_GROUP_SIZE);
		return CMD_SUCCESS;
	}
	
	if (SCANLOCATE_NODE == vty->node)
	{
		index = 0;
	}
	else if (HANSI_SCANLOCATE_NODE == vty->node)
	{
		index = (unsigned int)vty->index;
		localid = vty->local;
		slot_id = vty->slotindex;
	}
	else
	{
		vty_out(vty,"%% unsupport mode\n");
		return CMD_WARNING;
	}

	DBusConnection *dcli_dbus_connection = NULL;
	ReInitDbusConnection(&dcli_dbus_connection,slot_id,distributFag);
	ret = wid_show_wifi_locate_config_group(localid, index, &public_config, id, dcli_dbus_connection);
	
	if (WID_DBUS_SUCCESS != ret)
	{
		//vty_out(vty, "%s\n", dcli_wid_opcode2string(ret));
	}
	else
	{
		vty_out(vty, "=========================================\n");
		vty_out(vty, "index: %d\n", public_config.groupid);
		vty_out(vty, "scan_type: %d\n", public_config.scan_type);
		vty_out(vty, "rssi: %d\n", public_config.rssi);
		vty_out(vty, "2.4G channel: ");
		for(i=0; i<MAX_CHANNEL_2_4G; i++)
		{
			if((public_config.channel & BIT(i)) == BIT(i))
			{
				vty_out(vty, "%d ", i+1);
			}
		}
		vty_out(vty, "\n");
		vty_out(vty, "5.8G channel: ");
		for(i=16; i<64; i++)
		{
			if((public_config.channel & ((unsigned long long)(1)<<(i))) == ((unsigned long long)(1)<<(i)))
			{
				vty_out(vty, "%d ", change_to_5Gchannel(i+1));
			}
		}
		vty_out(vty, "\n");
		vty_out(vty, "channel scan time: %d\n", public_config.channel_scan_time);
		vty_out(vty, "channel scan interval: %d\n", public_config.channel_scan_interval);
		vty_out(vty, "report interval: %d\n", public_config.report_interval);
		vty_out(vty, "server ip: %s\n", dcli_u32ip2str(public_config.server_ip));
		vty_out(vty, "server port: %d\n", public_config.server_port);
		vty_out(vty, "bind radio:  ");
		
		for(i=0; i<public_config.radio_count; i++)
		{
			vty_out(vty, "%d ",public_config.radiolist[i]);
		}
		
		vty_out(vty, "\n");
		vty_out(vty, "=========================================");
	}

	if(public_config.radiolist != NULL)
	{
		free(public_config.radiolist);
		public_config.radiolist = NULL;
	}
	
	return CMD_SUCCESS;
}

DEFUN(show_wifi_config_all_cmd_func,
		show_wifi_config_all_cmd,
		"show service wifi all",
		"show service\n"
		"wifi service\n"
		"wifi service\n"
		"all wifi service\n")
{
	int i=0, j=0;
	unsigned int ret = 0;
	unsigned int index = 0;
	unsigned int group_num = 0;
	struct dcli_wifi_locate_public_config *public_config = NULL;
	int localid = 0;
	int slot_id = HostSlotId;
	
	if (SCANLOCATE_NODE == vty->node)
	{
		index = 0;
	}
	else if (HANSI_SCANLOCATE_NODE == vty->node)
	{
		index = (unsigned int)vty->index;
		localid = vty->local;
		slot_id = vty->slotindex;
	}
	else
	{
		vty_out(vty,"%% unsupport mode\n");
		return CMD_WARNING;
	}

	DBusConnection *dcli_dbus_connection = NULL;
	ReInitDbusConnection(&dcli_dbus_connection,slot_id,distributFag);
	ret = wid_show_wifi_locate_config_group_all(localid, index, &public_config, &group_num, dcli_dbus_connection);
	
	if(WID_DBUS_SUCCESS != ret)
	{
		//vty_out(vty, "%s\n", dcli_wid_opcode2string(ret));
	}
	else
	{
		vty_out(vty," service count:  %d\n", group_num);
		vty_out(vty, "====================================================\n");
		
		for(i=0; i<group_num; i++)
		{	
			vty_out(vty, "index: %d ", public_config[i].groupid);
			if(public_config[i].groupid == 0)
			{
				vty_out(vty, "(default global service)\n");
			}
			else if(public_config[i].groupid > WIFI_LOCATE_CONFIG_GROUP_CONFIG_SIZE)
			{
				vty_out(vty, "(snmp service)\n");
			}
			else
			{
				vty_out(vty, "\n");
			}

			vty_out(vty, "2.4G channel: ");
			for(j=0; j<MAX_CHANNEL_2_4G; j++)
			{
				if((public_config[i].channel & BIT(j)) == BIT(j))
				{
					vty_out(vty, "%d ", j+1);
				}
			}
			vty_out(vty, "\n");
			vty_out(vty, "5.8G channel: ");
			for(j=16; j<64; j++)
			{
				if((public_config[i].channel & ((unsigned long long)(1)<<(j))) == ((unsigned long long)(1)<<(j)))
				{
					vty_out(vty, "%d ", change_to_5Gchannel(j+1));
				}
			}
			vty_out(vty, "\n");
			
			vty_out(vty, "channel_scan_interval: %d\n", public_config[i].channel_scan_interval);
			vty_out(vty, "channel_scan_time: %d\n", public_config[i].channel_scan_time);
			vty_out(vty, "report_interval: %d\n", public_config[i].report_interval);
			vty_out(vty, "rssi: %d\n", public_config[i].rssi);
			vty_out(vty, "scan_type: %d\n", public_config[i].scan_type);
			vty_out(vty, "server_ip: %s\n", dcli_u32ip2str(public_config[i].server_ip));
			vty_out(vty, "server_port: %d\n", public_config[i].server_port);
			
			vty_out(vty, "bind radio:  ");
			
			for(j=0; j<public_config[i].radio_count; j++)
			{
				vty_out(vty, "%d ",public_config[i].radiolist[j]);
			}
			
			vty_out(vty, "\n");
			
			vty_out(vty, "====================================================\n");
		}
	
	}

	if(public_config!= NULL)
	{

		for(i=0;i<group_num;i++)
		{
			if(public_config[i].radiolist !=NULL)
			{
				free(public_config[i].radiolist);
				public_config[i].radiolist = NULL;
			}
		}
		
		free(public_config);
		public_config = NULL;
	}
	return CMD_SUCCESS; 
}

DEFUN(show_wifi_config_default_5_8G_cmd_func,
		show_wifi_config_default_5_8G_cmd,
		"show service wifi default_5_8G",
		"show service\n"
		"wifi service\n"
		"wifi service\n"
		"default 5_8G\n")
{
	unsigned int ret = 0;
	unsigned int index= 0;
	int i = 0;
	int localid = 0;
	int slot_id = HostSlotId;
	
	struct dcli_wifi_locate_public_config public_config;
	memset(&public_config, 0, sizeof(struct dcli_wifi_locate_public_config));
	
	if (SCANLOCATE_NODE == vty->node)
	{
		index = 0;
	}
	else if (HANSI_SCANLOCATE_NODE == vty->node)
	{
		index = (unsigned int)vty->index;
		localid = vty->local;
		slot_id = vty->slotindex;
	}
	else
	{
		vty_out(vty,"%% unsupport mode\n");
		return CMD_WARNING;
	}

	DBusConnection *dcli_dbus_connection = NULL;
	ReInitDbusConnection(&dcli_dbus_connection,slot_id,distributFag);
	ret = wid_show_wifi_locate_config_group_default_5_8G(localid, index, &public_config, dcli_dbus_connection);
	
	if (WID_DBUS_SUCCESS != ret)
	{
		//vty_out(vty, "%s\n", dcli_wid_opcode2string(ret));
	}
	else
	{
		vty_out(vty, "=========================================\n");
		vty_out(vty, "index: %d\n", public_config.groupid);
		vty_out(vty, "scan_type: %d\n", public_config.scan_type);
		vty_out(vty, "rssi: %d\n", public_config.rssi);
		vty_out(vty, "5.8G channel: ");
		for(i=16; i<64; i++)
		{
			if((public_config.channel & ((unsigned long long)(1)<<(i))) == ((unsigned long long)(1)<<(i)))
			{
				vty_out(vty, "%d ", change_to_5Gchannel(i+1));
			}
		}
		vty_out(vty, "\n");
		vty_out(vty, "channel scan time: %d\n", public_config.channel_scan_time);
		vty_out(vty, "channel scan interval: %d\n", public_config.channel_scan_interval);
		vty_out(vty, "report interval: %d\n", public_config.report_interval);
		vty_out(vty, "server ip: %s\n", dcli_u32ip2str(public_config.server_ip));
		vty_out(vty, "server port: %d\n", public_config.server_port);
		vty_out(vty, "bind radio:  ");
		
		for(i=0; i<public_config.radio_count; i++)
		{
			vty_out(vty, "%d ",public_config.radiolist[i]);
		}
		
		vty_out(vty, "\n");
		vty_out(vty, "=========================================");
	}

	if(public_config.radiolist != NULL)
	{
		free(public_config.radiolist);
		public_config.radiolist = NULL;
	}
	
	return CMD_SUCCESS;
}

#if 0
int dcli_scanlocate_show_running_config_start(struct vty *vty) 
{			
	char *showStr = NULL, *cursor = NULL, ch = 0;
	char tmpBuf[SHOWRUN_PERLINE_SIZE] = {0};
	DBusMessage *query = NULL, *reply = NULL;
	DBusError err;
	int res1 = 0, res2 = 0;
	int tmp = 0;	
	int index = 0;
	char BUSNAME[PATH_LEN] = {0};
	char OBJPATH[PATH_LEN]= {0};
	char INTERFACE[PATH_LEN] = {0};
	int localid = 1;
	int slot_id = HostSlotId;
	
	ReInitDbusPath_V2(localid, index, WID_DBUS_BUSNAME, BUSNAME);
	ReInitDbusPath_V2(localid, index, WID_DBUS_OBJPATH, OBJPATH);
	ReInitDbusPath_V2(localid, index, WID_DBUS_INTERFACE, INTERFACE);
	query = dbus_message_new_method_call(BUSNAME, OBJPATH, INTERFACE,
							WID_DBUS_CONF_METHOD_SCANLOCATE_SHOW_RUNNING_CONFIG_START);

	dbus_error_init(&err);

	DBusConnection *dcli_dbus_connection = NULL;
	ReInitDbusConnection(&dcli_dbus_connection,slot_id,distributFag);
	reply = dbus_connection_send_with_reply_and_block (dcli_dbus_connection, query, 300000, &err);  

	dbus_message_unref(query);
	if (NULL == reply) 
	{		
		if (dbus_error_is_set(&err)) 
		{			
			dbus_error_free(&err);
		}
		return 1;
	}

	if (dbus_message_get_args ( reply, &err,
					DBUS_TYPE_STRING, &showStr,
					DBUS_TYPE_INVALID)) 
	{
		char _tmpstr[DEFAULT_LEN] = {0};
		
		sprintf(_tmpstr, BUILDING_MOUDLE, "SCANLOCATE1");
		dcli_config_write(showStr,0,0,0,0,0);
		vtysh_add_show_string(_tmpstr);
		vtysh_add_show_string(showStr);
		dbus_message_unref(reply);
	} 
	else 
	{		
		if (dbus_error_is_set(&err)) 
		{			
			dbus_error_free(&err);
		}
		dbus_message_unref(reply);
		return 1;
	}	

	return 0;	
}

char* dcli_hansi_scanlocate_show_running_config_start(int index) 
{
	char *showStr = NULL,*cursor = NULL,ch = 0;
	char tmpBuf[SHOWRUN_PERLINE_SIZE] = {0};
	DBusMessage *query = NULL, *reply = NULL;
	DBusError err;
	int res1 = 0, res2 = 0;
	char *tmp = NULL;
	char BUSNAME[PATH_LEN] = {0};
	char OBJPATH[PATH_LEN] = {0};
	char INTERFACE[PATH_LEN] = {0};
	int localid = 1;
	int slot_id = HostSlotId;
	
	ReInitDbusPath_V2(localid, index, WID_DBUS_BUSNAME, BUSNAME);
	ReInitDbusPath_V2(localid, index, WID_DBUS_OBJPATH, OBJPATH);
	ReInitDbusPath_V2(localid, index, WID_DBUS_INTERFACE, INTERFACE);
	query = dbus_message_new_method_call(BUSNAME, OBJPATH, INTERFACE,
									WID_DBUS_CONF_METHOD_SCANLOCATE_SHOW_RUNNING_CONFIG_START);


	dbus_error_init(&err);

	DBusConnection *dcli_dbus_connection = NULL;
	ReInitDbusConnection(&dcli_dbus_connection,slot_id,distributFag);
	reply = dbus_connection_send_with_reply_and_block (dcli_dbus_connection, query, 300000, &err); 

	dbus_message_unref(query);
	if (NULL == reply) 
	{
		if (dbus_error_is_set(&err)) 
		{
			dbus_error_free(&err);
		}
		return NULL;
	}

	if (dbus_message_get_args (reply, &err,
					DBUS_TYPE_STRING, &showStr,
					DBUS_TYPE_INVALID)) 
	{
		tmp = (char *)malloc(strlen(showStr)+1);
		if(NULL == tmp)
		{
			dbus_message_unref(reply);
			return NULL;
		}
		memset(tmp, 0, strlen(showStr)+1);
		memcpy(tmp, showStr, strlen(showStr));	
		dbus_message_unref(reply);
		return tmp;	
	} 
	else 
	{
		if (dbus_error_is_set(&err)) 
		{
			dbus_error_free(&err);
		}
		dbus_message_unref(reply);
		return NULL;
	}
	
	return NULL;	
}

#endif

void dcli_scanlocate_init(void)
{
#if 0
	//install_node(&scanlocate_node, NULL, "SCANLOCATE_NODE");
	//install_node(&scanlocate_node, dcli_scanlocate_show_running_config_start, "SCANLOCATE_NODE");
	install_node(&scanlocate_node, dcli_hansi_scanlocate_show_running_config_start, "SCANLOCATE_NODE");
	//install_node(&scanlocate_node1, NULL, "SCALLOCATE_NODE1");
	install_default(SCANLOCATE_NODE);
    //vtysh_install_debug_cmd(SCANLOCATE_NODE);


	/**********************************************CONFIG_NODE**************************************************/
	install_element(CONFIG_NODE, &conf_scanlocate_cmd);	

	/****************************************************SCANLOCATE_NODE**************************************************/
	install_element(SCANLOCATE_NODE,&conf_scanlocate_cmd);
	install_element(SCANLOCATE_NODE, &create_scanlocate_channel_cmd);	
	install_element(SCANLOCATE_NODE, &create_scanlocate_report_cmd);	
	install_element(SCANLOCATE_NODE, &create_scanlocate_wifi_specail_cmd);	
	install_element(SCANLOCATE_NODE, &delete_scanlocate_group_cmd);
	install_element(SCANLOCATE_NODE, &set_radiolist_bind_mac_filter_group_cmd);
	install_element(SCANLOCATE_NODE, &modify_scanlocate_wifi_specal_config_cmd);
	install_element(SCANLOCATE_NODE, &modify_scanlocate_general_config_cmd);
	install_element(SCANLOCATE_NODE, &set_wtp_wifi_locate_general_config_cmd);
	install_element(SCANLOCATE_NODE, &show_wtp_wifi_locate_public_config_cmd);
	install_element(SCANLOCATE_NODE, &show_wtp_wifi_locate_public_config_all_cmd);
	install_element(SCANLOCATE_NODE, &set_wtp_wifi_locate_on_off_cmd);
	install_element(SCANLOCATE_NODE, &set_wtp_wifi_locate_on_off_all_cmd);
	install_element(SCANLOCATE_NODE, &show_wifi_config_cmd);
	install_element(SCANLOCATE_NODE, &show_wifi_config_all_cmd);
	install_element(SCANLOCATE_NODE, &show_wifi_config_default_5_8G_cmd);
	
#endif	
	


	/*****************************************HANSI_NODE****************************************************/
	install_node(&hansi_scanlocate_node,NULL,"HANSI_SCANLOCATE_NODE");
	install_default(HANSI_SCANLOCATE_NODE);
	//vtysh_install_debug_cmd(HANSI_SCANLOCATE_NODE);

	install_element(HANSI_NODE, &conf_scanlocate_cmd);

#if 1

	/*****************************************HANSI_SCANLOCATE_NODE****************************************************/
	install_element(HANSI_SCANLOCATE_NODE, &create_scanlocate_channel_cmd);	
	install_element(HANSI_SCANLOCATE_NODE, &create_scanlocate_report_cmd);	
	install_element(HANSI_SCANLOCATE_NODE, &create_scanlocate_wifi_specail_cmd);
	install_element(HANSI_SCANLOCATE_NODE, &delete_scanlocate_group_cmd);
	install_element(HANSI_SCANLOCATE_NODE, &set_radiolist_bind_mac_filter_group_cmd);
	install_element(HANSI_SCANLOCATE_NODE, &modify_scanlocate_wifi_specal_config_cmd);
	install_element(HANSI_SCANLOCATE_NODE, &modify_scanlocate_general_config_cmd);
	install_element(HANSI_SCANLOCATE_NODE, &set_wtp_wifi_locate_general_config_cmd);
	install_element(HANSI_SCANLOCATE_NODE, &show_wtp_wifi_locate_public_config_cmd);
	install_element(HANSI_SCANLOCATE_NODE, &show_wtp_wifi_locate_public_config_all_cmd);
	install_element(HANSI_SCANLOCATE_NODE, &set_wtp_wifi_locate_on_off_cmd);
	install_element(HANSI_SCANLOCATE_NODE, &set_wtp_wifi_locate_on_off_all_cmd);
	install_element(HANSI_SCANLOCATE_NODE, &show_wifi_config_cmd);
	install_element(HANSI_SCANLOCATE_NODE, &show_wifi_config_all_cmd);
	install_element(HANSI_SCANLOCATE_NODE, &show_wifi_config_default_5_8G_cmd);

#endif
	return;
	
}

#endif

