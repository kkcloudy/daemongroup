/* trap-util.c */

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <signal.h>
#include <syslog.h>
#include <ctype.h>
#include <math.h>
#include <sys/stat.h>
#include <time.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/ioctl.h>
#include <sys/stat.h>
#include <netinet/in.h>
#include <net/if.h>
#include <errno.h>

#include <net-snmp/net-snmp-config.h>
#include <net-snmp/net-snmp-includes.h>
#include <net-snmp/agent/net-snmp-agent-includes.h>


#include "trap-def.h"
#include "nm_list.h"
#include "trap-list.h"
#include "trap-util.h"

#include "ac_manage_def.h"
#include "ws_snmpd_engine.h"
#include "ws_snmpd_manual.h"
#include "ac_manage_interface.h"

#define IS_HEX(c) ((c) >= '0' && (c) <= '9' || (c) >= 'a' && (c) <= 'f' || (c) >= 'A' && (c) <= 'F')

#define LOG_LEVEL_NODE_NAME    "loglevel"

GlobalInfo gGlobalInfo = {0};
SystemInfo gSysInfo = {0};
HeartbeatInfo gHeartbeatInfo[VRRP_TYPE_NUM][INSTANCE_NUM + 1] = {0};


int 
trap_get_product_info(char *filename)
{
    int fd;
    char buff[16] = {0};
    unsigned int data;

    if(NULL == filename) {
        trap_syslog(LOG_WARNING, "trap_get_product_info: Input filename is NULL\n");
        return 0;
    }

    fd = open(filename, O_RDONLY, 0);
    if(fd >= 0) {
        if(read(fd, buff, 16) < 0) {
            trap_syslog(LOG_WARNING, "trap_get_product_info: Read error : no value\n");
            close(fd);
            return 0;
        }    
    }
    else {        
        trap_syslog(LOG_WARNING, "trap_get_product_info: Open file:%s error!\n", filename);
        return 0;
    }
    
    data = strtoul(buff, NULL, 10);

    close(fd);

    return data;
}


void trap_set_system_signal_handler (int sig, SystemSignalHandlerFunc func)
{
	struct sigaction act;
	sigset_t empty_mask;
  
	sigemptyset (&empty_mask);
	act.sa_handler = func;
	act.sa_mask    = empty_mask;
	act.sa_flags   = 0;
	sigaction (sig,  &act, NULL);
}

void trap_openlog (void)
{
	openlog("trap-helper", LOG_PID, LOG_DAEMON);
}

void trap_syslog (int priority, const char *message, ...)
{
	va_list args;

	va_start(args, message);
	
	trap_syslogv(priority, message, args);
	
	va_end (args);
}

void trap_syslogv(int priority, const char *message, va_list args)
{
	vsyslog(priority, message, args);

	if (LOG_ERR == priority)
		exit(1);
}

int trap_file_exists (const char *file)
{
	return (access (file, F_OK) == 0);
}

int trap_write_pid_file(const char *filename) //add single process judge 2010-12-6
{

	int fd,ret;
	char pid_buf[10]={0};
	struct flock file_lock;
	memset (&file_lock,0,sizeof(file_lock));
	
	if ((fd = open(filename, O_RDWR |O_CREAT, S_IRUSR |S_IWUSR)) < 0){
		trap_syslog(LOG_INFO, "open file %s error!process quit\n",filename);
		exit(-1);
	}

	file_lock.l_type = F_WRLCK;
	if((ret=fcntl(fd, F_GETLK, &file_lock))<0){
		trap_syslog(LOG_INFO, "fcntl error:can't get file lock state!process quit ret=%d error:%s\n",ret, strerror(errno));
		//exit(0);
	}
	if (F_UNLCK!=file_lock.l_type)
	{
		trap_syslog(LOG_INFO,"proess exist! process quit\n");
		exit(-1);
	}	
	file_lock.l_type=F_WRLCK;
	if((ret=fcntl(fd, F_SETLK, &file_lock))<0){
		trap_syslog(LOG_INFO, "process set lock error! process quit! ret=%d\n",ret);
		exit(-1);
	}

	snprintf(pid_buf,sizeof(pid_buf),"%lu",getpid());
	trap_syslog(LOG_DEBUG,"sizeof(pid_buf)=%d,pid=%lu,pid_buf=%s\n",sizeof(pid_buf),getpid(),pid_buf);
	write(fd, pid_buf, strlen(pid_buf));
	//close(fp); can't closed because of the file lock will cleared 
	
	return 1;
#if 0
	FILE *fp;
	mode_t oldmask;
	
	oldmask = umask(022);
	fp = fopen(filename, "w");
	umask(oldmask);
	
	if(!fp) return 0;
	fprintf(fp, "%lu", getpid());
	fclose(fp);

	return 1;
#endif
}

int trap_become_daemon(void)
{
	return daemon(1, 1) == 0;
}

char *trap_get_enterprise_oid(char *enterprise_oid, int len)
{
	FILE *fp = NULL;
	char buf[16];
	char *p;
	
	memset(enterprise_oid, 0, len);
	memset(buf, 0, sizeof(buf));

	strcpy(enterprise_oid, TRAP_ENTERPRISE_OID);
	if( (fp = fopen(ENTERPRISE_OID_FILE, "r")) == NULL)
		return enterprise_oid;
	if (fgets(buf, sizeof(buf), fp) == NULL) {
		fclose(fp);
		return enterprise_oid;
	}
	fclose(fp);

	if (strlen(buf) > 0 && '\n' == buf[strlen(buf)-1])
		buf[strlen(buf)-1] = '\0';

	if (strlen(buf) > 10)
		return enterprise_oid;
	
	for (p = buf; *p; p++)
		if (!isdigit(*p))
			return enterprise_oid;

	if (strcmp(buf, "0") == 0 || strcmp(buf, "") == 0)
		return enterprise_oid;
	
	snprintf(enterprise_oid, len, ".%s", buf);

	return enterprise_oid;
}

char *trap_get_product_oid(char *product_oid, int len)
{
	FILE *fp = NULL;
	char buf[32];
	char *p;
	
	memset(product_oid, 0, len);
	memset(buf, 0, sizeof(buf));

	strcpy(product_oid, TRAP_PRODUCT_OID);
	if( (fp = fopen(PRODUCT_OID_FILE, "r")) == NULL)
		return product_oid;
	if (fgets(buf, sizeof(buf), fp) == NULL) {
		fclose(fp);
		return product_oid;
	}
	fclose(fp);
	
	if (strlen(buf) > 0 && '\n' == buf[strlen(buf)-1])
		buf[strlen(buf)-1] = '\0';

	if (strlen(buf) > 10)
		return product_oid;
	
	for (p = buf; *p; p++)
		if (!isdigit(*p) && '.' != *p)
			return product_oid;

	if (strcmp(buf, "0") == 0 || strcmp(buf, "") == 0)
		return product_oid;
	
	snprintf(product_oid, len, ".%s", buf);

	return product_oid;
}

int char_to_hex(int c)
{
	if (c >= '0' && c <= '9')
		return c - '0';
	else if (c >= 'a' && c <= 'f')
		return c - 'a' + 10;
	else if (c >= 'A' && c <= 'F')
		return c - 'A' + 10;
	else 
		return -1;
}


/*
unsigned char mac[6];
len = 6;
*/
unsigned char *trap_get_ac_mac(unsigned char *mac, int len)
{
	FILE *fp = NULL;
	char buf[32];  /* at least 16 bytes.  mac (6*2)  + '\n' (1) + '\0'(1) */
	char *p;
	int i;
	
	memset(mac, 0, len);
	memset(buf, 0, sizeof(buf));
	
	strcpy(buf, "000000000000");
	if( (fp = fopen(MAC_FILE, "r")) == NULL)
		return mac;
	if (fgets(buf, sizeof(buf), fp) == NULL) {
		fclose(fp);
		return mac;
	}
	fclose(fp);

	for (p = buf; ' ' == *p || '\t' == *p; p++);

	for (i = 0; i < 12; i++)
		if (!IS_HEX(p[i]))
			return mac;

	for (i = 0; i < 6; i++)
		mac[i] = char_to_hex(p[i*2]) * 16 + char_to_hex(p[i*2+1]);
	
	return mac;
}

char *trap_get_hostname(char *hostname, int len)
{
	FILE *fp = NULL;
	char buf[1024];
	
	memset(hostname, 0, len);
	strncpy(hostname, "SYSTEM", len);
	if ( (fp = popen("hostname", "r")) == NULL)
		return hostname;
	
	if (fgets(buf, sizeof(buf), fp) == NULL){
		pclose(fp);
		return hostname;
	}
	pclose(fp);
	
	if (strlen(buf) > 0 && '\n' == buf[strlen(buf)-1])
		buf[strlen(buf)-1] = '\0';

	strncpy(hostname, buf, len);

	return hostname;
}

void trap_system_info_get(SystemInfo *sysInfo)
{
	trap_get_enterprise_oid(sysInfo->enterprise_oid, 16);
	trap_get_product_oid(sysInfo->product_oid, 32);
	trap_get_ac_mac(sysInfo->ac_mac, 6);
	trap_get_hostname(sysInfo->hostname, 128);
}

/* time_str like 2010-07/19-11:03:45, len >= 20 */
char *trap_get_time_str(char *time_str, int len)
{
	time_t t;
	struct tm *p;
	
	time(&t);
	p = localtime(&t);
	snprintf(time_str, len, "%d/%d/%d-%d:%d:%d", 
			1900+p->tm_year, 1+p->tm_mon, p->tm_mday, p->tm_hour, p->tm_min, p->tm_sec);
	
	return time_str;
}

int
trap_instance_heartbeat_init(HeartbeatInfo *info, unsigned int local_id, unsigned int instance_id)
{
    if(NULL == info || local_id >= VRRP_TYPE_NUM || 0 == instance_id || instance_id > INSTANCE_NUM) {
        trap_syslog(LOG_WARNING, "trap_instance_heartbeat_init: input para error, info = %p, local_id = %d, instance_id = %d!\n",
                                    info, local_id, instance_id);
        return TRAP_ERROR;
    }

    memset(info, 0 , sizeof(HeartbeatInfo));
    
	int interval = 30;
	int mode = 0;
	time_t cur_time = time(NULL);
	char buf[26] = { 0 };

    TRAPParameter *parameter_array = NULL;
    unsigned int parameter_num = 0;
    int ret = ac_manage_show_trap_parameter(ccgi_dbus_connection, &parameter_array, &parameter_num);
    if(AC_MANAGE_SUCCESS == ret) {
        int i = 0;
        for(i = 0; i < parameter_num; i++) {
            if(0 == strcmp(parameter_array[i].paraStr, HEARTBEAT_INTERVAL)) {
                interval = parameter_array[i].data;
            }
            else if(0 == strcmp(parameter_array[i].paraStr, HEARTBEAT_MODE)){
                mode = parameter_array[i].data;
            }
        }
    }
    
    info->local_id = local_id;
    info->instance_id = instance_id;
    
	info->mode = mode;
	info->interval = (interval < 5) ? 30 : interval;
	info->last_heartbeat_send_time = cur_time;
	info->last_trap_send_time = cur_time;
    
    TRAPHeartbeatIP *ip_array = NULL;
    unsigned int ip_num = 0;
    int ip_ret = ac_manage_show_trap_instance_heartbeat(ccgi_dbus_connection, &ip_array, &ip_num);
    if(AC_MANAGE_SUCCESS == ip_ret) {
        int i = 0;
        for(i = 0; i < ip_num; i++) {            
            if(local_id == ip_array[i].local_id && instance_id == ip_array[i].instance_id) {
                strncpy(info->ac_ip, ip_array[i].ipAddr, sizeof(info->ac_ip) - 1);
                        
                trap_syslog(LOG_INFO, "trap_instance_heartbeat_init: %s instance %d, manual set heartbeat ip %s\n",
                                        local_id ? "Local-hansi" : "Remote-hansi", instance_id, info->ac_ip);
                        
            }           
        }
    }

    if(0 == info->ac_ip[0]) {
        trap_get_ac_ip(info->ac_ip);
    }

	trap_syslog(LOG_INFO, "trap_instance_heartbeat_init: %s instance %d, mode = %d, interval = %d, last_heartbeat_time = last_trap_time = %s, ac_ip = %s\n",
				            info->local_id ? "Local-hansi" : "Remote-hansi", info->instance_id, info->mode, 
				            info->interval, ctime_r(&cur_time, buf), info->ac_ip);   
    return TRAP_OK;
}

int trap_heartbeat_is_demanded(HeartbeatInfo *info)
{
	time_t cur_time = time(NULL);
	time_t diff_time = 0;
	int ret = 0;
	char cur_buf[26] = {0}, last_buf[26] = {0};

    trap_syslog(LOG_DEBUG, "trap_heartbeat_is_demanded :local_id = %d, instance_id = %d, ac_ip = %s\n", info->local_id, info->instance_id, info->ac_ip); 

	if (0 == info->mode ) {
	    diff_time = difftime(cur_time, info->last_trap_send_time);
		if (diff_time >= info->interval || diff_time < 0) {	
			ret = 1;
	    }		
		trap_syslog(LOG_DEBUG, "heartbeat demand :ac_ip=%s, mode=%d, cur_time=%s, last_trap_time=%s, interval=%d, return %d\n",
						info->ac_ip,
						info->mode,
						ctime_r(&cur_time, cur_buf),
						ctime_r(&info->last_trap_send_time, last_buf),
						info->interval, ret);
	}
	else if (1 == info->mode) {
	    diff_time = difftime(cur_time, info->last_heartbeat_send_time);            
		if (diff_time >= info->interval || diff_time < 0) {
			ret = 1;
	    }		
		trap_syslog(LOG_DEBUG, "heartbeat demand : ac_ip=%s, mode=%d, cur_time=%s, last_heartbeat_time=%s, interval=%d, return %d\n",
						info->ac_ip,
						info->mode,
						ctime_r(&cur_time, cur_buf),
						ctime_r(&info->last_heartbeat_send_time, last_buf),
						info->interval, ret);
	}
	else {
		trap_syslog(LOG_DEBUG, "heartbeat demand : unexpected mode %d\n", info->mode);
		ret = 0;
	}
	
	return ret;
}

void trap_heartbeat_update_last_time(HeartbeatInfo *info, int is_heartbeat_trap)
{
	time_t cur_time = time(NULL);
	char trap_buf[26] = {0}, heartbeat_buf[26] = {0};
	
	info->last_trap_send_time = cur_time;
	if (is_heartbeat_trap)
		info->last_heartbeat_send_time = cur_time;
	
	trap_syslog(LOG_DEBUG, "heartbeat update last time : is_heartbeat_trap=%d, last_trap_time=%s, last_heartbeat_time=%s\n",
				is_heartbeat_trap,
				ctime_r(&info->last_trap_send_time, trap_buf),
				ctime_r(&info->last_heartbeat_send_time, heartbeat_buf));
}

int trap_heartbeat_get_timeout(HeartbeatInfo *info)
{
	time_t cur_time = time(NULL);
	char cur_buf[26] = {0}, last_buf[26] = {0};
	int timeout = info->interval;
	
	if (0 == info->mode) {
		timeout = info->interval - (long)difftime(cur_time, info->last_trap_send_time) % info->interval;
		trap_syslog(LOG_DEBUG, "heartbeat get timeout : mode=%d, interval=%d, cur_time=%s, last_trap_time=%s, timeout=%d\n",
						info->mode,
						info->interval,
						ctime_r(&cur_time, cur_buf),
						ctime_r(&info->last_trap_send_time, last_buf),
						timeout);
	}
	else if (1 == info->mode) {
		timeout = info->interval - (long)difftime(cur_time, info->last_heartbeat_send_time) % info->interval;
		trap_syslog(LOG_DEBUG, "heartbeat get timeout : mode=%d, interval=%d, cur_time=%s, last_trap_time=%s, timeout=%d\n",
						info->mode,
						info->interval,
						ctime_r(&cur_time, cur_buf),
						ctime_r(&info->last_heartbeat_send_time, last_buf),
						timeout);
	}
	else
		trap_syslog(LOG_DEBUG, "heartbeat get timeout : unexpected mode %d\n", info->mode);

	return timeout;
}

void trap_enable_debug(void)
{
	unsigned int debugLevel = 0;
	netsnmp_log_handler *logh;
	
	int ret = ac_manage_show_trap_log_debug(ccgi_dbus_connection, &debugLevel);
	if(0 != debugLevel){
		if(2 == debugLevel){
			snmp_set_do_debugging(1);
			logh = netsnmp_register_loghandler(NETSNMP_LOGHANDLER_SYSLOG, LOG_DEBUG);
			if (logh) {
				int facility = LOG_DAEMON; 
				if (facility == -1)  return;
				logh->pri_max = LOG_EMERG;
				logh->token   = strdup(snmp_log_syslogname("trap-helper"));
				logh->magic   = (void *)(intptr_t)facility;
				snmp_enable_syslog_ident(snmp_log_syslogname("trap-helper"), facility);
			}
		}
		setlogmask(LOG_UPTO(LOG_DEBUG));
	}else
		setlogmask(LOG_UPTO(LOG_INFO));

	syslog(LOG_DEBUG, "log level =%s", (0 != debugLevel)?"debug" : "least_info");
}

int  ac_trap_ip_param_by_shell( char *shell_name, char *content  )
{
		FILE * fp = NULL;
		char buf[128];
		if( NULL != (fp = popen (shell_name, "r")) )
		{
			fgets(buf,128,fp);
			strncpy (content, buf, ( strcmp(buf, "") )?(strlen(buf)-1) : 0);
			pclose(fp);
		}
		return 0;
}

int trap_judge_ac_netstate(char *ip)
{
	int i;
	for (i=0;i<3;i++){
		if(!trap_get_ac_ip(ip)){
			trap_syslog(LOG_INFO,"trap_get_ac_ip is %s\n",ip);
			return 1;
		}
		sleep(10);
	}
	trap_syslog(LOG_INFO,"trap_get_ac_ip failed ip is %s\n",ip);
	return 0;
}

void trap_wait_ac_configure_down(int interval)
{
	unsigned char buf[4] = {0};
	int aw_state_fd=-1;

	while (1)
	{
		aw_state_fd = open("/var/run/aw.state",O_RDONLY);
		if(aw_state_fd < 0)
		{
			trap_syslog(LOG_INFO,"open /var/run/aw.state error: ret = %d", aw_state_fd);
			sleep(interval);
			continue;
		}

		read(aw_state_fd, buf, 1);
		if( '1' == buf[0]) 
		{
			trap_syslog(LOG_INFO,"ac config load down! start trap-helper\n");
			close(aw_state_fd);
			break;
		}
		else
		{
			trap_syslog(LOG_INFO,"ac config load down not already! buf = %s , sleep %ds", buf, interval);
			sleep(interval);
		}
		close(aw_state_fd);
		aw_state_fd=-1;
	}
	return ;
}

int trap_get_ac_ip(char *ip)
{
	int sockfd, n, flags ;
	int numreqs=10;
	char *ip_tmp=NULL;
	struct ifconf ifc;
	struct ifreq ifr_tmp,*ifr=NULL;
	struct sockaddr_in *m_addr=NULL;
	strncpy(ip,"default_ac_ip",13);

	sockfd=socket(AF_INET, SOCK_DGRAM, 0);
	if (sockfd < 0) {
		trap_syslog(LOG_INFO, "trap_get_ac_ip: warning: no inet socket available: %s\n",strerror(errno));
		goto error;
	}

	ifc.ifc_buf = NULL;
	for (n=0;;) {
		ifc.ifc_len = sizeof(struct ifreq) * numreqs;
		ifc.ifc_buf = realloc(ifc.ifc_buf, ifc.ifc_len);
		if(!ifc.ifc_buf){
			trap_syslog(LOG_INFO,"trap_get_ac_ip: realloc error\n");
			goto error;
		}

		if (ioctl(sockfd, SIOCGIFCONF, &ifc) < 0) {
			trap_syslog(LOG_INFO,"trap_get_ac_ip: ioctl SIOCGIFCONF error\n");
			goto error;
		}
		ifr = ifc.ifc_req;
		for (ifr+=n/sizeof(struct ifreq); n < ifc.ifc_len; n += sizeof(struct ifreq)) {
			m_addr=(struct sockaddr_in *)&ifr->ifr_addr;

			strncpy(ifr_tmp.ifr_name,ifr->ifr_name,IFNAMSIZ);
			if(ioctl(sockfd, SIOCGIFFLAGS, &ifr_tmp) == -1)
			{
				trap_syslog(LOG_INFO,"trap_get_ac_ip: Error getting flags for interface :");
				goto error;
			}

			flags=ifr_tmp.ifr_flags;
			ip_tmp = (char *)inet_ntoa(m_addr->sin_addr);
			trap_syslog(LOG_DEBUG,"interface name : %s ip %s flags %d\n",ifr->ifr_name, ip_tmp, flags);
			if (flags & IFF_LOOPBACK){
				ifr++;
				continue;
			}else if (flags & IFF_RUNNING){
				strncpy(ip, ip_tmp, ACIPSIZ);
				trap_syslog(LOG_DEBUG,"Heart time interface %s ip address is %s return\n",ifr->ifr_name,ip);
				goto out;
			}
			#if 0
			//ifr->ifr_flags;
			if (flags == 0)
				printf("[NO FLAGS] ");
			if (flags & IFF_UP)
				printf("UP ");
			if (flags & IFF_BROADCAST)
				printf("BROADCAST ");
			if (flags & IFF_DEBUG)
				printf("DEBUG ");
			if (flags & IFF_LOOPBACK)
				printf("LOOPBACK ");
			if (flags & IFF_POINTOPOINT)
				printf("POINTOPOINT ");
			if (flags & IFF_NOTRAILERS)
				printf("NOTRAILERS ");
			if (flags & IFF_RUNNING)
				printf("RUNNING ");
			if (flags & IFF_NOARP)
				printf("NOARP ");
			if (flags & IFF_PROMISC)
				printf("PROMISC ");
			if (flags & IFF_ALLMULTI)
				printf("ALLMULTI ");
			if (flags & IFF_SLAVE)
				printf("SLAVE ");
			if (flags & IFF_MASTER)
				printf("MASTER ");
			if (flags & IFF_MULTICAST)
				printf("MULTICAST ");
			printf(">\n");
			#endif
			ifr++;
		}
		if (ifc.ifc_len == (sizeof(struct ifreq) * numreqs)) {
			/* assume it overflowed and try again */
			numreqs += 10;
			continue;
		}
		break;
	}

error:
	free(ifc.ifc_buf);
	return -1;

out:
	free(ifc.ifc_buf);
	return 0;	

}

void *trap_malloc (unsigned int malloc_size,
								const char * malloc_file_name,
								const char * malloc_func_name,
								int 	malloc_line_num )
{
	void *ptr=NULL;
	if (NULL != (ptr=malloc(malloc_size))){ 
		trap_syslog(LOG_DEBUG,"[%s@%s,%d]malloc: address is %p size is %d\n",malloc_func_name,malloc_file_name,malloc_line_num,ptr,malloc_size);
		return ptr;
	}else
	{
		trap_syslog(LOG_INFO,"[%s@%s,%d]malloc: malloc error!\n",malloc_func_name,malloc_file_name,malloc_line_num);
		return NULL;
	}
}

void trap_util_test(void)
{
	SystemInfo info;
	
	trap_system_info_get(&info);
	printf("enterprise_oid = %s\n", info.enterprise_oid);
	printf("product_oid = %s\n", info.product_oid);
	printf("ac_mac = %02X-%02X-%02X-%02X-%02X-%02X\n", 
		info.ac_mac[0], info.ac_mac[1], info.ac_mac[2], 
		info.ac_mac[3], info.ac_mac[4], info.ac_mac[5]);
	printf("hostname = %s\n", info.hostname);

	char tm_str[20];
	printf("time = %s\n", trap_get_time_str(tm_str, 20));
}

