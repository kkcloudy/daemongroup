#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/socket.h>
#include <linux/netlink.h>
#include <pthread.h>
#include <string.h>
#include <syslog.h>
#include <linux/if.h>
#include <linux/sockios.h>

#include "dbus/dbus.h"
#include "npd_dbus_def.h"
#include "dcli_sys_manage.h"
#include "sfd.h"

static DBusConnection *sfd_dbus_connection = NULL;
static int sd;//netlink socket
static int var_flag;/*sfd dbus send message flag*/
static int sfd_enable;
static int log_enable;
static int debug_enable;
static int tcp_enable;
static int icmp_enable;
static int snmp_enable;
static int dns_enable;
static int capwap_enable;

static int timespan;
static int limitpacket;
static int limitpacket_snmp;
static int limitpacket_tcp;
static int limitpacket_icmp;
static int limitpacket_capwap;

static int arp_enable;
static int arp_timespan;
static int limitpacket_arp;

static int
macstr2mac(char *macstr, unsigned char *mac)
{
	int n = 0;
	char *tmp = NULL;
	char macstr_tmp[18];
	
	strcpy(macstr_tmp, macstr);
	
	tmp = strtok(macstr_tmp, ":");
	if(tmp == NULL)/*coverity modify for CID 18038 */
		return -1;
	if(strlen(tmp) != 2)
		return -1;
	mac[n++] = (unsigned char)strtol(tmp, NULL, 16);

	for(; n < ETH_ALEN; ++n) {
		tmp = strtok(NULL, ":");
		if(!tmp || strlen(tmp) != 2)
			break;
		mac[n] = (unsigned char)strtol(tmp, NULL, 16);
	}

	if(n != ETH_ALEN) {
		return -1;
	}

	return 0;
}

static int
ipv4str2ip(char *ipstr, unsigned int *ip)
{
	int n = 0;
	char *tmp;
	char ipstr_tmp[16];
	
	strcpy(ipstr_tmp, ipstr);

	tmp = strtok(ipstr_tmp, ".");
	((char *)ip)[n++] = atoi(tmp);

	for(; n < 4; ++n)
	{
		tmp = strtok(NULL, ".");
		if(!tmp)
			break;
		((char *)ip)[n] = atoi(tmp);
	}

	if(n != 4) {
		return -1;
	}
	return 0;
}

static void
sfd_init_system_log (void)
{
	openlog("sfd_daemon", LOG_NDELAY, LOG_DAEMON);
}

static void
sfd_system_log(SfdLogSeverity severity, const char *msg, ...)
{
	int flags;
	va_list args;
	va_start(args, msg);

	switch(severity) {
		case SFD_LOG_INFO:
			flags = LOG_INFO;
			break;
		case SFD_LOG_DEBUG:
			flags = LOG_DEBUG;
			break;
		case SFD_LOG_ERR:
			flags = LOG_ERR;
			break;
		default:
			return;
	}

	vsyslog(flags, msg, args);
	vprintf(msg, args);
	va_end(args);
}

#if 0
static int
show_netlink_msg_info(void *data)
{
	sfd_system_log(SFD_LOG_INFO,
					"sfd message information: cmd = %d, data = %d\n",
					((sfdMsg *)data)->cmd, ((sfdMsg *)data)->data);
	return 0;
}
#endif

static int
send_netlink_message(void *data, int len)
{
	struct nlmsghdr *nlhdr = NULL;
	struct msghdr msg;
	struct iovec iov;
	struct sockaddr_nl daddr;
	int ret = 0;

	nlhdr = (struct nlmsghdr *)malloc(NLMSG_SPACE(len));

	if(NULL == nlhdr) {
		sfd_system_log(SFD_LOG_ERR, "sfd message malloc failed\n");
		goto err;
	}

	//show_netlink_msg_info(data);

	memcpy(NLMSG_DATA(nlhdr), data, len);
	memset(&msg, 0 ,sizeof(struct msghdr));

	/* set nlhdr */
	nlhdr->nlmsg_len = NLMSG_LENGTH(len);
	nlhdr->nlmsg_pid = getpid();  
	nlhdr->nlmsg_flags = 0;

	/* set daddr */
	daddr.nl_family = AF_NETLINK;
	daddr.nl_pid = 0;
	daddr.nl_groups = 0;

	/* set message */
    iov.iov_base = (void *)nlhdr;
    iov.iov_len = nlhdr->nlmsg_len;
    msg.msg_name = (void *)&daddr;
    msg.msg_namelen = sizeof(daddr);
    msg.msg_iov = &iov;
    msg.msg_iovlen = 1;

	/* send message */
	if (sendmsg(sd, &msg, 0) == -1) {
		sfd_system_log(SFD_LOG_ERR, "send sfd message failed\n");
		goto err;
	}

done:
	if(nlhdr)
		free(nlhdr);
	return ret;

err:
	ret = -1;
	goto done;
}

static int
init_netlink()
{
	struct sockaddr_nl saddr;

	sd = socket(AF_NETLINK, SOCK_RAW, SFD_NETLINK_ID);
	if(0 == sd) {
		sfd_system_log(SFD_LOG_ERR, "creat netlink socket failed\n");
		return -1;
	}

	saddr.nl_family = AF_NETLINK;
	saddr.nl_pid = getpid();
	saddr.nl_groups	= 0;

	if(0 != bind(sd,(struct sockaddr*)&saddr, sizeof(saddr))) {
		sfd_system_log(SFD_LOG_ERR, "bind saddr error\n");
		return -1;
	}

	sfdMsg msg;
	msg.cmd = sfdcmd_daemon;
	msg.datalen = sizeof(saddr.nl_pid);
	*(int *)msg.data = saddr.nl_pid;
	if(send_netlink_message((void *)&msg, sizeof(msg))) {
		sfd_system_log(SFD_LOG_ERR,
						"send netlink msg to register "
						"in sfd module failed\n");
		return -1;
	}

	return 0;
}

static int
logswitch(int cmd)
{
	sfdMsg msg;

	msg.cmd = sfdcmd_logswitch;
	msg.datalen = sizeof(cmd);
	*(int *)msg.data = cmd;

	if(send_netlink_message((void *)&msg, sizeof(msg))) {
		sfd_system_log(SFD_LOG_ERR,
						"send netlink msg to sfd failed\n");
		return -1;
	}

	return 0;
}

static int
debugswitch(int cmd)
{
	sfdMsg msg;

	msg.cmd = sfdcmd_debugswitch;
	msg.datalen = sizeof(cmd);
	*(int *)msg.data = cmd;

	if(send_netlink_message((void *)&msg, sizeof(msg))) {
		sfd_system_log(SFD_LOG_ERR, 
						"send netlink msg to sfd failed\n");
		return -1;
	}

	return 0;
}

/* sfd */
static int
sfd_switch(int cmd,int sfdflag)
{
	sfdMsg msg;
	msg.cmd = sfdcmd_switch;
	msg.datalen = sizeof(cmd)+sizeof(sfdflag);
	*(int *)msg.data = cmd;
	*((int *)msg.data+1) = sfdflag;
	if(send_netlink_message((void *)&msg, sizeof(msg))) {
		sfd_system_log(SFD_LOG_ERR, "send netlink msg to sfd failed\n");
		return -1;
	}

	return 0;
}

static int
sfd_timespan(int timespan)
{
	sfdMsg msg;

	msg.cmd = sfdcmd_timespan;
	msg.datalen = sizeof(timespan);
	*(int *)msg.data = timespan;

	if(send_netlink_message((void *)&msg, sizeof(msg))) {
		sfd_system_log(SFD_LOG_ERR, "send netlink msg to sfd failed\n");
		return -1;
	}

	return 0;
}

static int
sfd_limitpacket(int proto_flag,int limit)
{
	sfdMsg msg;

	msg.cmd = sfdcmd_limitpacket;
	msg.datalen = sizeof(limit)+sizeof(proto_flag);
	*(int *)msg.data = proto_flag;
	*((int *)msg.data+1) = limit;

	if(send_netlink_message((void *)&msg, sizeof(msg))) {
		sfd_system_log(SFD_LOG_ERR,
						"send netlink msg to sfd failed\n");
		return -1;
	}

	return 0;
}

static int
sfd_useradd(char *macstr, char *ipstr)
{
	sfdMsg msg;
	sfdMember *sfduser = NULL;

	msg.cmd = sfdcmd_newmember;
	msg.datalen = sizeof(sfdMember);

	sfduser = (sfdMember *)msg.data;
	sfduser->type = sfdiptype_ipv4;
	if(macstr2mac(macstr, sfduser->mac)) {
		sfd_system_log(SFD_LOG_ERR,
						"MAC format is incorrect\n");
		return -1;
	}
	
	if(ipv4str2ip(ipstr, (unsigned int *)sfduser->ip)) {
		sfd_system_log(SFD_LOG_ERR,
						"IP format is incorrect\n");
		return -1;
	}

	if(send_netlink_message((void *)&msg, sizeof(msg))) {
		sfd_system_log(SFD_LOG_ERR,
						"send netlink msg to sfd failed\n");
		return -1;
	}

	return 0;
}

static int
sfd_userdel(char *macstr)
{
	sfdMsg msg;
	sfdMember *sfduser = NULL;

	msg.cmd = sfdcmd_delmember;
	msg.datalen = sizeof(sfdMember);

	sfduser = (sfdMember *)msg.data;
	if(macstr2mac(macstr, sfduser->mac)) {
		sfd_system_log(SFD_LOG_ERR,
						"MAC format is incorrect\n");
		return -1;
	}

	if(send_netlink_message((void *)&msg, sizeof(msg))) {
		sfd_system_log(SFD_LOG_ERR,
						"send netlink msg to sfd failed\n");
		return -1;
	}

	return 0;
}
static int sfd_variables(int var)
{
 	sfdMsg msg;

	msg.cmd = sfdcmd_variables;
	msg.datalen = sizeof(var);
	*(int *)msg.data = var;

	if(send_netlink_message((void *)&msg, sizeof(msg))) {
		sfd_system_log(SFD_LOG_ERR,
						"send netlink msg to sfd failed\n");
		return -1;
	}

	return 0;
}

/* arp */
int check_sfdkern_is_install(void )
{
	
	FILE *fp=NULL;	
	char line[MEM_COMMAND_LEN];
	char module[64];
	unsigned int size;
	int used;
	char used_by[256];
	int ret=0;
	fp = popen("sudo lsmod","r"); 
		
	if(NULL == fp)	
	{		
		sfd_system_log(SFD_LOG_ERR, "Find module error\n");
		return -1; 
	} 

	memset(line,0,MEM_COMMAND_LEN);
	fgets(line,MEM_COMMAND_LEN,fp); 
	while(fscanf(fp,"%s%u%d%s",module,&size,&used,used_by)!= -1)
	{
		if(strcmp("sfd_kern",module)==0)
			ret=1;
	}
	pclose(fp);
	return ret;
}

static int
arp_switch(int cmd)
{
	sfdMsg msg;

	msg.cmd = sfdcmd_arpswitch;
	msg.datalen = sizeof(cmd);
	*(int *)msg.data = cmd;

	if(send_netlink_message((void *)&msg, sizeof(msg))) {
		sfd_system_log(SFD_LOG_ERR, "send netlink msg to sfd failed\n");
		return -1;
	}

	return 0;
}

static int
arp_limitpacket(int limit)
{
	sfdMsg msg;

	msg.cmd = sfdcmd_arplimitpacket;
	msg.datalen = sizeof(limit);
	*(int *)msg.data = limit;

	if(send_netlink_message((void *)&msg, sizeof(msg))) {
		sfd_system_log(SFD_LOG_ERR,
						"send netlink msg to sfd failed\n");
		return -1;
	}

	return 0;
}
static int
arp_useradd(char *macstr)
{
	sfdMsg msg;
	sfdMember *arpuser = NULL;

	msg.cmd = sfdcmd_arpnewmember;
	msg.datalen = sizeof(sfdMember);

	arpuser = (sfdMember *)msg.data;
	arpuser->type = sfdiptype_ipv4;
	if(macstr2mac(macstr, arpuser->mac)) {
		sfd_system_log(SFD_LOG_ERR,
						"ARP MAC format is incorrect\n");
		return -1;
	}
	
	if(send_netlink_message((void *)&msg, sizeof(msg))) {
		sfd_system_log(SFD_LOG_ERR,
						"send netlink msg to sfd failed\n");
		return -1;
	}

	return 0;
}
static int
arp_userdel(char *macstr)
{
	sfdMsg msg;
	sfdMember *arpuser = NULL;

	msg.cmd = sfdcmd_arpdelmember;
	msg.datalen = sizeof(sfdMember);

	arpuser= (sfdMember *)msg.data;
	if(macstr2mac(macstr, arpuser->mac)) {
		sfd_system_log(SFD_LOG_ERR,
						"ARP MAC format is incorrect\n");
		return -1;
	}

	if(send_netlink_message((void *)&msg, sizeof(msg)))
	{
		sfd_system_log(SFD_LOG_ERR,
						"send netlink msg to sfd failed\n");
		return -1;
	}
	return 0;
}

static DBusMessage*
logswitch_process(DBusConnection *conn, DBusMessage *msg, void *user_data)
{
	DBusMessage* reply;
	DBusError err;
	int ret = 0;
	int ret_is_sfdkern = 0;
	int cmd;
	if(check_sfdkern_is_install())
	{
		ret_is_sfdkern = 1;
		dbus_error_init(&err);
		if(!(dbus_message_get_args(msg, &err,
										DBUS_TYPE_INT32, &cmd,
										DBUS_TYPE_INVALID))) 
		{
			sfd_system_log(SFD_LOG_ERR, "dbus failed get args\n");
			if(dbus_error_is_set(&err))
			{
				sfd_system_log(SFD_LOG_ERR,
									"SFD Daemon: %s raised:%s\n",
									err.name, err.message);
				dbus_error_free(&err);
			}
			return NULL;
		}
		
		ret = logswitch(cmd);
	}
	
	reply = dbus_message_new_method_return(msg);
	dbus_message_append_args(reply,
									DBUS_TYPE_INT32, &ret_is_sfdkern,
									DBUS_TYPE_INT32, &ret,
									DBUS_TYPE_INVALID);
	return reply;
}

static DBusMessage*
debugswitch_process(DBusConnection *conn, DBusMessage *msg, void *user_data)
{
	DBusMessage* reply;
	DBusError err;
	int ret = 0;
	int ret_is_sfdkern = 0;
	int cmd;
	if(check_sfdkern_is_install())
	{
		ret_is_sfdkern = 1;
		dbus_error_init(&err);
		if(!(dbus_message_get_args(msg, &err,
									DBUS_TYPE_INT32, &cmd,
									DBUS_TYPE_INVALID))) 
		{
			sfd_system_log(SFD_LOG_ERR, "dbus failed get args\n");
			if(dbus_error_is_set(&err))
			{
				sfd_system_log(SFD_LOG_ERR, "SFD Daemon: %s raised:%s\n", err.name, err.message);
				dbus_error_free(&err);
			}
			return NULL;
		}
	
		ret = debugswitch(cmd);
	}
	reply = dbus_message_new_method_return(msg);
	dbus_message_append_args(reply,
									DBUS_TYPE_INT32, &ret_is_sfdkern,
									DBUS_TYPE_INT32, &ret,
									DBUS_TYPE_INVALID);
	return reply;
}

/* sfd */
static DBusMessage*
sfd_switch_process(DBusConnection *conn, DBusMessage *msg, void *user_data)
{
	DBusMessage* reply;
	DBusError err;
	int ret = 0;
	int cmd;
	int sfdflag;
	
	dbus_error_init(&err);
	if(!(dbus_message_get_args(msg, &err,
								DBUS_TYPE_INT32, &cmd,
								DBUS_TYPE_INT32, &sfdflag,
								DBUS_TYPE_INVALID))) {
		sfd_system_log(SFD_LOG_ERR, "dbus failed get args\n");
		if(dbus_error_is_set(&err)) {
			sfd_system_log(SFD_LOG_ERR, "SFD Daemon: %s raised:%s\n", err.name, err.message);
			dbus_error_free(&err);
		}
		return NULL;
	}
	ret = sfd_switch(cmd,sfdflag);
	

	reply = dbus_message_new_method_return(msg);
	dbus_message_append_args(reply,
									DBUS_TYPE_INT32, &ret,
									DBUS_TYPE_INVALID);
	return reply;
}

static DBusMessage*
sfd_switch_slot_process(DBusConnection *conn, DBusMessage *msg, void *user_data)
{
	DBusMessage* reply;
	DBusError err;
	int ret = 0;
	int cmd;
	int sfdflag;
	int ret_is_sfdkern = 0;
	int ret_cmd;
	int ret_type; 
	
	dbus_error_init(&err);
	if(!(dbus_message_get_args(msg, &err,
								DBUS_TYPE_INT32, &cmd,
								DBUS_TYPE_INT32, &sfdflag,
								DBUS_TYPE_INVALID))) 
	{
		sfd_system_log(SFD_LOG_ERR, "dbus failed get args\n");
		if(dbus_error_is_set(&err)) 
		{
			sfd_system_log(SFD_LOG_ERR, "SFD Daemon: %s raised:%s\n", err.name, err.message);
			dbus_error_free(&err);
		}
		return NULL;
	}
	ret_is_sfdkern=check_sfdkern_is_install();
	if(ret_is_sfdkern==1)
	{
		if((sfdflag!=ENABLE_SFD)&&(sfdflag!=DISABLE_SFD))
		{
			ret = sfd_switch(cmd,sfdflag);
		}		
		if(sfdflag==DISABLE_SFD)
		{
			/*shutdown(sd,SHUT_RDWR);*/
			ret = sfd_switch(cmd,sfdflag);
			sleep(1);
			close(sd);
			system("sudo rmmod sfd_kern");
		}
					
	}
	else if(ret_is_sfdkern==0)
	{
		if(sfdflag==ENABLE_SFD)
		{	
			
			system("sudo modprobe sfd_kern");
			sleep(1);
			if(-1 == init_netlink()) 
			{
				sfd_system_log(SFD_LOG_ERR, "init_netlink error\n");
			}
			ret = sfd_switch(cmd,sfdflag);
			ret = sfd_variables(cmd);
			sfd_system_log(SFD_LOG_ERR, "modprobe sfd_kern success\n");
		}
	}

	reply = dbus_message_new_method_return(msg);
	dbus_message_append_args(reply,
									DBUS_TYPE_INT32, &ret_is_sfdkern,
									DBUS_TYPE_INT32, &sfdflag,
									DBUS_TYPE_INT32, &ret,
									DBUS_TYPE_INVALID);
	return reply;
}

static DBusMessage*
sfd_timespan_process(DBusConnection *conn, DBusMessage *msg, void *user_data)
{
	DBusMessage* reply;
	DBusError err;
	int ret = 0;
	int ret_is_sfdkern = 0;
	int timespan;
	

	if(check_sfdkern_is_install())
	{
		ret_is_sfdkern = 1;
		dbus_error_init(&err);
			if(!(dbus_message_get_args(msg, &err,
										DBUS_TYPE_INT32, &timespan,
										DBUS_TYPE_INVALID)))
			{
				sfd_system_log(SFD_LOG_ERR, "dbus failed get args\n");
				if(dbus_error_is_set(&err))
				{
					sfd_system_log(SFD_LOG_ERR, "SFD Daemon: %s raised:%s\n", err.name, err.message);
					dbus_error_free(&err);
				}
				return NULL;
			}
		
			ret = sfd_timespan(timespan);
	
	}
	reply = dbus_message_new_method_return(msg);
	dbus_message_append_args(reply,
									DBUS_TYPE_INT32, &ret_is_sfdkern,
									DBUS_TYPE_INT32, &ret,
									DBUS_TYPE_INVALID);
	return reply;
}

static DBusMessage*
sfd_limitpacket_process(DBusConnection *conn, DBusMessage *msg, void *user_data)
{
	DBusMessage* reply;
	DBusError err;
	int ret = 0;
	int limit;
	int proto_flag;
	int ret_is_sfdkern = 0;
	
	if(check_sfdkern_is_install())
	{
		ret_is_sfdkern = 1;
		dbus_error_init(&err);
		if(!(dbus_message_get_args(msg, &err,
									DBUS_TYPE_INT32, &proto_flag,
									DBUS_TYPE_INT32, &limit,
									DBUS_TYPE_INVALID))) 
		{
			sfd_system_log(SFD_LOG_ERR, "dbus failed get args\n");
			if(dbus_error_is_set(&err)) {
				sfd_system_log(SFD_LOG_ERR, "SFD Daemon: %s raised:%s\n", err.name, err.message);
				dbus_error_free(&err);
			}
			return NULL;
		}
		ret = sfd_limitpacket(proto_flag,limit);
	}
	

	reply = dbus_message_new_method_return(msg);
	dbus_message_append_args(reply,
									DBUS_TYPE_INT32, &ret_is_sfdkern,
									DBUS_TYPE_INT32, &ret,
									DBUS_TYPE_INVALID);
	return reply;
}

static DBusMessage*
sfd_useradd_process(DBusConnection *conn, DBusMessage *msg, void *user_data)
{
	DBusMessage* reply;
	DBusError err;
	int ret = 0;
	char *mac, *ip;
	int ret_is_sfdkern = 0;
		
	if(check_sfdkern_is_install())
	{
		ret_is_sfdkern = 1;
		dbus_error_init(&err);
		if(!(dbus_message_get_args(msg, &err,
									DBUS_TYPE_STRING, &mac,
									DBUS_TYPE_STRING, &ip,
									DBUS_TYPE_INVALID)))
		{
			sfd_system_log(SFD_LOG_ERR, "dbus failed get args\n");
			if(dbus_error_is_set(&err))
			{
				sfd_system_log(SFD_LOG_ERR, "SFD Daemon: %s raised:%s\n", err.name, err.message);
				dbus_error_free(&err);
			}
			return NULL;
		}
	
		ret = sfd_useradd(mac, ip);
	}

	reply = dbus_message_new_method_return(msg);
	dbus_message_append_args(reply,
									DBUS_TYPE_INT32, &ret_is_sfdkern,
									DBUS_TYPE_INT32, &ret,
									DBUS_TYPE_INVALID);
	return reply;
}

static DBusMessage*
sfd_userdel_process(DBusConnection *conn, DBusMessage *msg, void *user_data)
{
	DBusMessage* reply;
	DBusError err;
	int ret = 0;
	char *mac;
	int ret_is_sfdkern = 0;
			
	if(check_sfdkern_is_install())
	{
		ret_is_sfdkern = 1;
		dbus_error_init(&err);
		if(!(dbus_message_get_args(msg, &err,
									DBUS_TYPE_STRING, &mac,
									DBUS_TYPE_INVALID))) 
		{
			sfd_system_log(SFD_LOG_ERR, "dbus failed get args\n");
			if(dbus_error_is_set(&err)) 
			{
				sfd_system_log(SFD_LOG_ERR, "SFD Daemon: %s raised:%s\n", err.name, err.message);
				dbus_error_free(&err);
			}
			return NULL;
		}

		ret = sfd_userdel(mac);
	}
	
	reply = dbus_message_new_method_return(msg);
	dbus_message_append_args(reply,
								DBUS_TYPE_INT32, &ret_is_sfdkern,
								DBUS_TYPE_INT32, &ret,
								DBUS_TYPE_INVALID);
	return reply;
}

/* arp */
static DBusMessage*
arp_switch_process(DBusConnection *conn, DBusMessage *msg, void *user_data)
{
	DBusMessage* reply;
	DBusError err;
	int ret = 0;
	int cmd;
	int ret_is_sfdkern = 0;
			
	if(check_sfdkern_is_install())
	{
		ret_is_sfdkern = 1;
		dbus_error_init(&err);
		if(!(dbus_message_get_args(msg, &err,
									DBUS_TYPE_INT32, &cmd,
									DBUS_TYPE_INVALID)))
		{
			sfd_system_log(SFD_LOG_ERR, "dbus failed get args\n");
			if(dbus_error_is_set(&err)) 
			{
				sfd_system_log(SFD_LOG_ERR, "SFD Daemon: %s raised:%s\n", err.name, err.message);
				dbus_error_free(&err);
			}
			return NULL;
		}

		ret = arp_switch(cmd);
	}
	reply = dbus_message_new_method_return(msg);
	dbus_message_append_args(reply,
									DBUS_TYPE_INT32, &ret_is_sfdkern,
									DBUS_TYPE_INT32, &ret,
									DBUS_TYPE_INVALID);
	return reply;
}

static DBusMessage*
arp_limitpacket_process(DBusConnection *conn, DBusMessage *msg, void *user_data)
{
	DBusMessage* reply;
	DBusError err;
	int ret = 0;
	int limit;
	int ret_is_sfdkern = 0;
			
	if(check_sfdkern_is_install())
	{
		ret_is_sfdkern = 1;
		dbus_error_init(&err);
		if(!(dbus_message_get_args(msg, &err,
									DBUS_TYPE_INT32, &limit,
									DBUS_TYPE_INVALID)))
		{
			sfd_system_log(SFD_LOG_ERR, "dbus failed get args\n");
			if(dbus_error_is_set(&err))
			{
				sfd_system_log(SFD_LOG_ERR, "SFD Daemon: %s raised:%s\n", err.name, err.message);
				dbus_error_free(&err);
			}
			return NULL;
		}

		ret = arp_limitpacket(limit);
	}
	
	reply = dbus_message_new_method_return(msg);
	dbus_message_append_args(reply,
									DBUS_TYPE_INT32, &ret_is_sfdkern,
									DBUS_TYPE_INT32, &ret,
									DBUS_TYPE_INVALID);
	return reply;
}
static DBusMessage*
sfd_arpuseradd_process(DBusConnection *conn, DBusMessage *msg, void *user_data)
{
	DBusMessage* reply;
	DBusError err;
	int ret = 0;
	char *mac;
	int ret_is_sfdkern = 0;
		
	if(check_sfdkern_is_install())
	{
		ret_is_sfdkern = 1;
		dbus_error_init(&err);
		if(!(dbus_message_get_args(msg, &err,
									DBUS_TYPE_STRING, &mac,
									DBUS_TYPE_INVALID)))
		{
			sfd_system_log(SFD_LOG_ERR, "dbus failed get args\n");
			if(dbus_error_is_set(&err))
			{
				sfd_system_log(SFD_LOG_ERR, "SFD Daemon: %s raised:%s\n", err.name, err.message);
				dbus_error_free(&err);
			}
			return NULL;
		}
	
		ret = arp_useradd(mac);
	}

	reply = dbus_message_new_method_return(msg);
	dbus_message_append_args(reply,
									DBUS_TYPE_INT32, &ret_is_sfdkern,
									DBUS_TYPE_INT32, &ret,
									DBUS_TYPE_INVALID);
	return reply;
}

static DBusMessage*
sfd_arpuserdel_process(DBusConnection *conn, DBusMessage *msg, void *user_data)
{
	DBusMessage* reply;
	DBusError err;
	int ret = 0;
	char *mac;
	int ret_is_sfdkern = 0;
			
	if(check_sfdkern_is_install())
	{
		ret_is_sfdkern = 1;
		dbus_error_init(&err);
		if(!(dbus_message_get_args(msg, &err,
									DBUS_TYPE_STRING, &mac,
									DBUS_TYPE_INVALID))) 
		{
			sfd_system_log(SFD_LOG_ERR, "dbus failed get args\n");
			if(dbus_error_is_set(&err)) 
			{
				sfd_system_log(SFD_LOG_ERR, "SFD Daemon: %s raised:%s\n", err.name, err.message);
				dbus_error_free(&err);
			}
			return NULL;
		}
		ret = arp_userdel(mac);
	}
	
	reply = dbus_message_new_method_return(msg);
	dbus_message_append_args(reply,
								DBUS_TYPE_INT32, &ret_is_sfdkern,
								DBUS_TYPE_INT32, &ret,
								DBUS_TYPE_INVALID);
	return reply;
}
#if 0
static DBusMessage*
sfd_variables_process(DBusConnection *conn, DBusMessage *msg, void *user_data)
{
	DBusMessage* reply;
	DBusMessageIter  iter;
	DBusError err;
	int ret = 0;
	int var;
	int var_num = 0;
	int i;
	int ret_is_sfdkern = 0;
	char file_name[64][256];
	char *tem_name[64] = {0};
	
	if(check_sfdkern_is_install())
	{
		ret_is_sfdkern = 1;
		dbus_error_init(&err);
		if(!(dbus_message_get_args(msg, &err,
									DBUS_TYPE_INT32, &var,
									DBUS_TYPE_INVALID))) 
		{
			sfd_system_log(SFD_LOG_ERR, "dbus failed get args\n");
			if(dbus_error_is_set(&err))
			{
				sfd_system_log(SFD_LOG_ERR, "SFD Daemon: %s raised:%s\n", err.name, err.message);
				dbus_error_free(&err);
			}
			return NULL;
		}

		ret = sfd_variables(var);
		sleep(1);
		if(sfd_enable)
			sprintf(file_name[var_num++],"service sfd is enable\n");
		else
			sprintf(file_name[var_num++],"service sfd is disable\n");
		
		if(log_enable)
			sprintf(file_name[var_num++],"service sfd log is enable\n");
		else
			sprintf(file_name[var_num++],"service sfd log is disable\n");
		
		if(debug_enable)
			sprintf(file_name[var_num++],"service sfd debug is enable\n");
		else
			sprintf(file_name[var_num++],"service sfd debug is disable\n");
		
		sprintf(file_name[var_num++],"service sfd timespan is %dms\n",timespan);
		
		if(tcp_enable)
			sprintf(file_name[var_num++],"service sfd tcp  is enable  LIMITPACKET is %d\n",limitpacket_tcp);
		else
			sprintf(file_name[var_num++],"service sfd tcp  is disable LIMITPACKET is %d\n",limitpacket_tcp);
		
		if(icmp_enable)
			sprintf(file_name[var_num++],"service sfd icmp is enable  LIMITPACKET is %d\n",limitpacket_icmp);
		else
			sprintf(file_name[var_num++],"service sfd icmp is disable LIMITPACKET is %d\n",limitpacket_icmp);
		
		if(snmp_enable)
			sprintf(file_name[var_num++],"service sfd snmp is enable  LIMITPACKET is %d\n",limitpacket_snmp);
		else
			sprintf(file_name[var_num++],"service sfd snmp is disable LIMITPACKET is %d\n",limitpacket_snmp);

		if(arp_enable)
			sprintf(file_name[var_num++],"service sfd arp  is enable  LIMITPACKET is %d\n",limitpacket_arp);
		else
			sprintf(file_name[var_num++],"service sfd arp  is disable LIMITPACKET is %d\n",limitpacket_arp);
		
		if(dns_enable)
			sprintf(file_name[var_num++],"service sfd dns  is enable\n");
		else
			sprintf(file_name[var_num++],"service sfd dns  is disable\n");
		
	}
	
	reply = dbus_message_new_method_return(msg);
	dbus_message_iter_init_append (reply, &iter);
	if(ret_is_sfdkern==0)
	{
		dbus_message_iter_append_basic (&iter,DBUS_TYPE_INT32,&ret_is_sfdkern);
	}
	else
	{
		dbus_message_iter_append_basic (&iter,DBUS_TYPE_INT32,&ret_is_sfdkern);
		dbus_message_iter_append_basic (&iter,DBUS_TYPE_INT32,&var_num);
		for(i=0;i<var_num;i++)
		{
			tem_name[i] = file_name[i];
			dbus_message_iter_append_basic (&iter,DBUS_TYPE_STRING,&tem_name[i]);
		}		
	}
	return reply;
}
#else
static DBusMessage*
sfd_variables_process(DBusConnection *conn, DBusMessage *msg, void *user_data)
{
	DBusMessage* reply;
	DBusMessageIter  iter;
	DBusError err;
	int ret = 0;
	int var;
	int var_num = 0;
	int i;
	int ret_is_sfdkern = 0;
	if(check_sfdkern_is_install())
	{
		ret_is_sfdkern = 1;
		dbus_error_init(&err);
		if(!(dbus_message_get_args(msg, &err,
									DBUS_TYPE_INT32, &var,
									DBUS_TYPE_INVALID))) 
		{
			sfd_system_log(SFD_LOG_ERR, "dbus failed get args\n");
			if(dbus_error_is_set(&err))
			{
				sfd_system_log(SFD_LOG_ERR, "SFD Daemon: %s raised:%s\n", err.name, err.message);
				dbus_error_free(&err);
			}
			return NULL;
		}

		ret = sfd_variables(var);
		usleep(10000);	
	}
	
	reply = dbus_message_new_method_return(msg);
	dbus_message_iter_init_append (reply, &iter);
	if(ret_is_sfdkern==0)
	{
		dbus_message_iter_append_basic (&iter,DBUS_TYPE_INT32,&ret_is_sfdkern);
	}
	else
	{
		dbus_message_iter_append_basic (&iter,DBUS_TYPE_INT32,&ret_is_sfdkern);
		dbus_message_iter_append_basic (&iter,DBUS_TYPE_INT32,&sfd_enable);
		dbus_message_iter_append_basic (&iter,DBUS_TYPE_INT32,&log_enable);
		dbus_message_iter_append_basic (&iter,DBUS_TYPE_INT32,&debug_enable);
		
		dbus_message_iter_append_basic (&iter,DBUS_TYPE_INT32,&timespan);
		dbus_message_iter_append_basic (&iter,DBUS_TYPE_INT32,&tcp_enable);
		dbus_message_iter_append_basic (&iter,DBUS_TYPE_INT32,&limitpacket_tcp);
		dbus_message_iter_append_basic (&iter,DBUS_TYPE_INT32,&icmp_enable);
		dbus_message_iter_append_basic (&iter,DBUS_TYPE_INT32,&limitpacket_icmp);
		dbus_message_iter_append_basic (&iter,DBUS_TYPE_INT32,&snmp_enable);
		dbus_message_iter_append_basic (&iter,DBUS_TYPE_INT32,&limitpacket_snmp);
		dbus_message_iter_append_basic (&iter,DBUS_TYPE_INT32,&arp_enable);
		dbus_message_iter_append_basic (&iter,DBUS_TYPE_INT32,&limitpacket_arp);
		dbus_message_iter_append_basic (&iter,DBUS_TYPE_INT32,&dns_enable);
		dbus_message_iter_append_basic (&iter,DBUS_TYPE_INT32,&capwap_enable);
		dbus_message_iter_append_basic (&iter,DBUS_TYPE_INT32,&limitpacket_capwap);
	}
	return reply;
}

#endif
static DBusMessage*
sfd_running_process(DBusConnection *conn, DBusMessage *msg, void *user_data)
{
	DBusMessage* reply;
	DBusMessageIter  iter;
	DBusError err;
	int ret = 0;
	int var;
	int i;
	int var_num = 0;
	int ret_is_sfdkern = 0;
	char file_name[64][256];
	char *tem_name[64] = {0};
			
	if(check_sfdkern_is_install())
	{
		ret_is_sfdkern = 1;
		dbus_error_init(&err);
		
		if(sfd_enable)
			sprintf(file_name[var_num++],"service sfd enable\n");
		
		if(log_enable)
			sprintf(file_name[var_num++],"service sfd log enable\n");
		
		
		if(debug_enable)
			sprintf(file_name[var_num++],"service sfd debug enable\n");
	
		
		if(tcp_enable)
			sprintf(file_name[var_num++],"service sfdtcp enable\n");
		
		
		if(icmp_enable)
			sprintf(file_name[var_num++],"service sfdicmp enable\n");
		
		
		if(snmp_enable)
			sprintf(file_name[var_num++],"service sfdsnmp enable\n");
		
		
		if(dns_enable)
			sprintf(file_name[var_num++],"service sfddns enable\n");
		
		
		if(arp_enable)
			sprintf(file_name[var_num++],"service sfdarp enable\n");
		
		if(timespan!=TIMESPAN_DEF)
			sprintf(file_name[var_num++],"service sfd timespan %d\n",timespan);
		
		if(limitpacket_tcp!=TCP_PACKET_DEF)
			sprintf(file_name[var_num++],"service sfd packetlimit tcp %d\n",limitpacket_tcp);
		
		if(limitpacket_icmp!=ICMP_PACKET_DEF)
			sprintf(file_name[var_num++],"service sfd packetlimit icmp %d\n",limitpacket_icmp);

		if(limitpacket_snmp!=SNMP_PACKET_DEF)
			sprintf(file_name[var_num++],"service sfd packetlimit snmp %d\n",limitpacket_snmp);
		if(limitpacket_arp!=ARP_PACKET_DEF)
			sprintf(file_name[var_num++],"service sfd packetlimit arp %d\n",limitpacket_arp);
		
	}
		
	
	reply = dbus_message_new_method_return(msg);
	dbus_message_iter_init_append (reply, &iter);
	if(ret_is_sfdkern==0)
	{
		dbus_message_iter_append_basic (&iter,DBUS_TYPE_INT32,&ret_is_sfdkern);
	}
	else
	{
		dbus_message_iter_append_basic (&iter,DBUS_TYPE_INT32,&ret_is_sfdkern);
		dbus_message_iter_append_basic (&iter,DBUS_TYPE_INT32,&var_num);
		for(i=0;i<var_num;i++)
		{
			tem_name[i] = file_name[i];
			dbus_message_iter_append_basic (&iter,DBUS_TYPE_STRING,&tem_name[i]);
		}		
	}
	return reply;
}


static DBusHandlerResult 
sfd_dbus_message_handler(DBusConnection *connection, DBusMessage *message, void *user_data)
{
	DBusMessage *reply = NULL;

	if(strcmp(dbus_message_get_path(message), SFD_DBUS_OBJPATH) == 0) {
		if (dbus_message_is_method_call(message, SFD_DBUS_INTERFACE,
			SFD_DBUS_METHOD_LOG)) {
			reply = logswitch_process(connection ,message, user_data);
		}
		else if (dbus_message_is_method_call(message, SFD_DBUS_INTERFACE,
			SFD_DBUS_METHOD_DEBUG)) {
			reply = debugswitch_process(connection ,message, user_data);
		}
		else if (dbus_message_is_method_call(message, SFD_DBUS_INTERFACE,
			SFD_DBUS_METHOD_SWT)) {
			reply = sfd_switch_process(connection, message, user_data);
		}
		else if (dbus_message_is_method_call(message, SFD_DBUS_INTERFACE,
			SFD_DBUS_METHOD_SWT_SLOT)) {
			reply = sfd_switch_slot_process(connection, message, user_data);
		}
		else if (dbus_message_is_method_call(message, SFD_DBUS_INTERFACE,
			SFD_DBUS_METHOD_VAR)) {
			reply = sfd_variables_process(connection, message, user_data);
		}
		else if (dbus_message_is_method_call(message, SFD_DBUS_INTERFACE,
			SFD_DBUS_METHOD_RUN)) {
			reply = sfd_running_process(connection, message, user_data);
		}
		else if (dbus_message_is_method_call(message, SFD_DBUS_INTERFACE,
			SFD_DBUS_METHOD_TMS)) {
			reply = sfd_timespan_process(connection ,message, user_data);
		}
		else if (dbus_message_is_method_call(message, SFD_DBUS_INTERFACE,
			SFD_DBUS_METHOD_LMT)) {
			reply = sfd_limitpacket_process(connection ,message, user_data);
		}
		else if (dbus_message_is_method_call(message, SFD_DBUS_INTERFACE,
			SFD_DBUS_METHOD_USERADD)) {
			reply = sfd_useradd_process(connection ,message, user_data);
		}
		else if (dbus_message_is_method_call(message, SFD_DBUS_INTERFACE,
			SFD_DBUS_METHOD_USERDEL)) {
			reply = sfd_userdel_process(connection ,message, user_data);
		}
		else if (dbus_message_is_method_call(message, SFD_DBUS_INTERFACE,
			SFD_DBUS_METHOD_ARP_SWT)) {
			reply = arp_switch_process(connection ,message, user_data);
		}
		else if (dbus_message_is_method_call(message, SFD_DBUS_INTERFACE,
			SFD_DBUS_METHOD_ARP_LMT)) {
			reply = arp_limitpacket_process(connection ,message, user_data);
		}
		else if (dbus_message_is_method_call(message, SFD_DBUS_INTERFACE,
			SFD_DBUS_METHOD_ARPUSERADD)) {
			reply = sfd_arpuseradd_process(connection ,message, user_data);
		}
		else if (dbus_message_is_method_call(message, SFD_DBUS_INTERFACE,
			SFD_DBUS_METHOD_ARPUSERDEL)) {
			reply = sfd_arpuserdel_process(connection ,message, user_data);
		}
	}

	if(reply) {
		dbus_connection_send(connection, reply, NULL);
		dbus_connection_flush(connection);
		dbus_message_unref(reply);
	}

	return DBUS_HANDLER_RESULT_HANDLED;
}
	
DBusHandlerResult 
sfd_dbus_filter_function(DBusConnection *connection,
										DBusMessage *message, 
										void *user_data)
{
	if(dbus_message_is_signal(message, DBUS_INTERFACE_LOCAL, "Disconnected") &&
		strcmp (dbus_message_get_path(message), DBUS_PATH_LOCAL) == 0) {
		/* this is a local message; e.g. from libdbus in this process */
		dbus_connection_unref(sfd_dbus_connection);
		sfd_dbus_connection = NULL;
	}
	else {
		return 1;
	}

	return DBUS_HANDLER_RESULT_HANDLED;
}

int
sfd_dbus_init()
{
	DBusError dbus_error;
	DBusObjectPathVTable sfd_vtable = {NULL, &sfd_dbus_message_handler, NULL, NULL, NULL, NULL};

	dbus_error_init(&dbus_error);
	sfd_dbus_connection = dbus_bus_get(DBUS_BUS_SYSTEM, &dbus_error);
	if(sfd_dbus_connection == NULL) {
		sfd_system_log(SFD_LOG_ERR, "init dbus dbus_bus_get(): %s\n", dbus_error.message);
		return -1;
	}

	if(!dbus_connection_register_fallback(sfd_dbus_connection, SFD_DBUS_OBJPATH, &sfd_vtable, NULL)) {
		sfd_system_log(SFD_LOG_ERR, "can't register D-BUS handlers (fallback dhcpsnp). cannot continue.\n");
		return -1;
	}

	dbus_bus_request_name(sfd_dbus_connection, SFD_DBUS_BUSNAME, 0, &dbus_error);

	if(dbus_error_is_set(&dbus_error)) {
		sfd_system_log(SFD_LOG_ERR, "dbus request bus name error: %s\n", dbus_error.message);
		return -1;
	}

	dbus_connection_add_filter(sfd_dbus_connection, sfd_dbus_filter_function, NULL, NULL);

	dbus_bus_add_match(sfd_dbus_connection,
						"type='signal'"
						",interface='"DBUS_INTERFACE_DBUS"'"
						",sender='"DBUS_SERVICE_DBUS"'"
						",member='NameOwnerChanged'",
						NULL);
	return 0;
}

void *
sfd_thread_dbus_main(void *arg)
{
	sfd_system_log(SFD_LOG_INFO, "sfd dbus thread start...");

	if(-1 == sfd_dbus_init()) {
		sfd_system_log(SFD_LOG_ERR, "sfd_dbus_init error\n");
		exit(-1);
	}
	sfd_system_log(SFD_LOG_DEBUG, "sfd_dbus_init OK\n");

	while(dbus_connection_read_write_dispatch(sfd_dbus_connection, -1));

	return ((void *)0);
}

pthread_t
sfd_thread_create(char *name, void *(*entry_point)(void*), void *arglist)
{
	pthread_attr_t thread_attr;
	pthread_t dbus_thread = (-1UL);
	int ret;

	pthread_attr_init(&thread_attr);

	ret = pthread_create(&dbus_thread, &thread_attr, (void *)entry_point, arglist);
	if(ret != 0) {
		sfd_system_log(SFD_LOG_ERR, "creat %s thread failed\n", name);;
		return (-1UL);
	}

	sfd_system_log(SFD_LOG_INFO, "creat %s thread 0x%08x success!\n", name, (unsigned long)dbus_thread);
	return dbus_thread;
}

static void
sfd_send_dbus_signal(const char *name, const char *str)
{
	DBusMessage *query;
	DBusError err;
	char exec = 1;
	int str_len = strlen(str) + 1;
	unsigned char *config_cmd = malloc(str_len); 

	memset(config_cmd, 0, str_len);
	strcpy(config_cmd, str);

	query = dbus_message_new_signal(RTDRV_DBUS_OBJPATH,
										"aw.trap", name);
	dbus_error_init(&err);

	dbus_message_append_args(query,
								DBUS_TYPE_BYTE, &exec,								
								DBUS_TYPE_STRING, &(config_cmd),
								DBUS_TYPE_INVALID);
	dbus_connection_send(sfd_dbus_connection, query, NULL);
	dbus_connection_flush(sfd_dbus_connection);
	dbus_message_unref(query);

	free(config_cmd);
}

void
arp_attacker_notify(unsigned char *mac_addr)
{
	char mac[64];
	sprintf(mac,
			"%02X:%02X:%02X:%02X:%02X:%02X",
			mac_addr[0], mac_addr[1], mac_addr[2],
			mac_addr[3], mac_addr[4], mac_addr[5]);
	
	sfd_system_log(SFD_LOG_DEBUG, "arp attacker: %s\n", mac);
	sfd_send_dbus_signal("arp_attacker", mac);
}

void
sfd_variables_recv(int *var)
{
	sfd_enable = *(var);
	log_enable = *(var+1);
	debug_enable = *(var+2);
	tcp_enable = *(var+3);
	icmp_enable = *(var+4);
	snmp_enable = *(var+5);
	dns_enable = *(var+6);
	arp_enable = *(var+7);
	timespan = *(var+8);
	limitpacket_snmp = *(var+9);
	limitpacket_tcp = *(var+10);
	limitpacket_icmp = *(var+11);
	limitpacket_arp = *(var+12);
	capwap_enable = *(var+13);
	limitpacket_capwap = *(var+14);
	sfd_system_log(SFD_LOG_DEBUG, "variables receive success\n");
	return;
}
void sfd_switch_recv(int enable)
{
	/*if(!enable)*/
	close(sd);
	return;
}

void
mainloop()
{
	struct nlmsghdr *msg = NULL;
	struct sockaddr_nl daddr;
	int socklen = 0, rcvlen = 0, msglen = 0;

	msglen = NLMSG_SPACE(sizeof(sfdMsg));
	msg = (struct nlmsghdr *)malloc(msglen);
	socklen = sizeof(struct sockaddr_nl);

	daddr.nl_family = AF_NETLINK;
	daddr.nl_pid = 0;
	daddr.nl_groups = 0;

	for(;;) 
	{
		rcvlen = recvfrom(sd, msg, msglen,
							0, (struct sockaddr *)&daddr, &socklen);
		if(rcvlen != msglen) 
		{
		/*
			sfd_system_log(SFD_LOG_DEBUG,
							"recv a broken netlink message, ignored\n");
		*/
			sleep(1);
			continue;
		}
#if 0
		sfd_system_log(SFD_LOG_DEBUG,
						"recv a netlink message, length = %u\n",
						rcvlen);
#endif
		sfdMsg *data = (sfdMsg *)NLMSG_DATA(msg);
		if(data->cmd == sfdcmd_arpwarning)
		{
			arp_attacker_notify(data->data);
		}
		else if(data->cmd == sfdcmd_variables)
		{
			sfd_variables_recv((int *)data->data);
		}
		else if(data->cmd == sfdcmd_switch)
		{
			sfd_switch_recv(*(int *)data->data);
		}
#if 0
		else {
			sfd_system_log(SFD_LOG_DEBUG, "type is %u, ignored\n", data->cmd);
		}
#endif
	}

	if(msg)
		free(msg);
}

int
main(int argc, char **argv)
{
	sfd_init_system_log();
#if 0
	if(-1 == init_netlink()) {
		sfd_system_log(SFD_LOG_ERR, "init_netlink error\n");
		return -1;
	}
#endif	
	sfd_system_log(SFD_LOG_DEBUG, "netlink init success\n");

	if(sfd_thread_create("sfd_dbus", sfd_thread_dbus_main, NULL) == (-1UL)) {
		return -1;
	}

	/* init */
	arp_switch(0);
	arp_limitpacket(50);

	/* loop recv netlink */
	mainloop();

	return 0;
}
