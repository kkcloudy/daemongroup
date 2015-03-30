/*
	add for eap users authorized in asd,will add to kernel iptables
	with the no need auth users 
*/
#include "ASDEAPAuth.h"

static struct eap_auth g_eap_auth;

eap_nmp_mutex_t eap_iptables_lock = {-1, ""};

const size_t adt_size = sizeof(struct ip_set_req_iphash);

//mutex function
int eap_nmp_mutex_init(eap_nmp_mutex_t *mutex, const char *file)
{
	mode_t old_mask = 0;
	
	if (NULL == mutex || NULL == file) {
		asd_printf(ASD_DEFAULT,MSG_ERROR, "eap_nmp_mutex_init input error\n");
		return -1;
	}
	strncpy(mutex->filename, file, sizeof(mutex->filename));

	old_mask = umask(0111);
	mutex->fd = open(mutex->filename, O_CREAT|O_RDWR|O_TRUNC, LOCK_FILE_MODE);
	if (mutex->fd  < 0) {
		asd_printf(ASD_DEFAULT,MSG_ERROR, "eap_nmp_mutex_init open file failed: %s\n", 
				strerror(errno));
		umask(old_mask);
		return -1;
	}
	umask(old_mask);
	
	asd_printf(ASD_DEFAULT,MSG_DEBUG, "eap_nmp_mutex_init mutex->fd = %d\n", mutex->fd);
	
    return 0;
}

int eap_nmp_mutex_destroy(eap_nmp_mutex_t *mutex)
{
	if (NULL == mutex)  {
		asd_printf(ASD_DEFAULT,MSG_ERROR, "eap_nmp_mutex_destroy input error\n");
		return -1;
	}

	if (mutex->fd >= 0) {
		close(mutex->fd);
		mutex->fd = -1;
		return 0;
	}
	asd_printf(ASD_DEFAULT,MSG_DEBUG, "eap_nmp_mutex_destroy ok\n");
	
	return 0;
}

int
eap_nmp_mutex_lock(eap_nmp_mutex_t *mutex)
{
	struct flock lock;
	char buf[32] = {0};

	if (NULL == mutex) {
		asd_printf(ASD_DEFAULT,MSG_INFO, "eap_nmp_mutex_lock, mutex is null\n");
		return -1;
	}

	lock.l_type = F_WRLCK;
	lock.l_start = 0;
	lock.l_whence = SEEK_SET;
	lock.l_len = 0;
    	
	if (fcntl(mutex->fd, F_SETLKW, &lock) < 0) {
		asd_printf(ASD_DEFAULT,MSG_INFO, "eap_nmp_mutex_lock lock failed: fd(%d) %s\n",
			mutex->fd, strerror(errno));
		return -1;
	}
	ftruncate(mutex->fd, 0);
	snprintf(buf, sizeof(buf), "%ld", (long)getpid());
	write(mutex->fd, buf, strlen(buf)+1);

	asd_printf(ASD_DEFAULT,MSG_DEBUG, "eap_nmp_mutex_lock %d,%s ok\n", mutex->fd, mutex->filename);
	
	return 0;
}

int
eap_nmp_mutex_unlock(eap_nmp_mutex_t *mutex)
{
	struct flock lock;

	if (NULL == mutex) {
		asd_printf(ASD_DEFAULT,MSG_INFO, "eap_nmp_mutex_unlock, mutex is null\n");
		return -1;
	}

	lock.l_type = F_UNLCK;
	lock.l_start = 0;
	lock.l_whence = SEEK_SET;
	lock.l_len = 0;
	
	ftruncate(mutex->fd, 0);
	if (fcntl(mutex->fd, F_SETLK, &lock) < 0) {
		asd_printf(ASD_DEFAULT,MSG_INFO, "eap_nmp_mutex_unlock unlock failed: fd(%d) %s\n",
			mutex->fd, strerror(errno));
		return -1;
	}

	asd_printf(ASD_DEFAULT,MSG_DEBUG, "eap_nmp_mutex_unlock %d,%s ok\n",
		mutex->fd, mutex->filename);
	
	return 0;
}
//ipset function
int kernel_getsocket(void)
{
	int sockfd = -1;

	sockfd = socket(AF_INET, SOCK_RAW, IPPROTO_RAW);

	if (sockfd < 0) {
		asd_printf(ASD_DEFAULT,MSG_INFO, "Create socket failed, you need to be root to perform it\n");
		return -1;
	}
	return sockfd;
 }
int kernel_getfrom(void *data, socklen_t *size)
{
	int res = -1;
	int sockfd = kernel_getsocket();

	if( sockfd >= 0 ){
		res = getsockopt(sockfd, SOL_IP, SO_IP_SET, data, size);
		if (res != 0 && errno == ENOPROTOOPT) {
			asd_printf(ASD_DEFAULT,MSG_INFO,"Try insmod ip_set");
		} else if (res != 0) {
			asd_printf(ASD_DEFAULT,MSG_INFO, "getsockopt:%d(%s)", errno, strerror(errno));
		}
		close( sockfd );
	}
	return res;
}

int kernel_sendto(void *data, socklen_t size)
{
	int res = -1;
	int sockfd = kernel_getsocket();

	if( sockfd >= 0 ){
		res = setsockopt(sockfd, SOL_IP, SO_IP_SET, data, size);
		if (res != 0 && errno == ENOPROTOOPT) {
			asd_printf(ASD_DEFAULT,MSG_INFO, "Try insmod ip_set\n");
		} else if (res != 0) {
			asd_printf(ASD_DEFAULT,MSG_INFO, "setsockopt:%d(%s)\n", errno, strerror(errno));
		}
		close( sockfd );
	}
	else
		asd_printf(ASD_DEFAULT,MSG_ERROR,"func:%s init socket error!\n",__func__);
	return res;
}
ip_set_id_t set_index_find_byname(const char *setname)
{
	struct ip_set_req_get_set req = {0};
	socklen_t size = sizeof(struct ip_set_req_get_set);
	int res = 0;

	if (NULL == setname) {
		asd_printf(ASD_DEFAULT,MSG_INFO, "set_index_find_byname setname is null\n");
		return -1;
	}

	req.op = IP_SET_OP_GET_BYNAME;
	req.version = IP_SET_PROTOCOL_VERSION;
	strncpy(req.set.name, setname, IP_SET_MAXNAMELEN - 1);

	res = kernel_getfrom((void *) &req, &size);
	if (res != 0) {
		asd_printf(ASD_DEFAULT,MSG_INFO, "kernel_getfrom error.\n");
		return res;
	}
	return req.set.index;	
}
int set_adtip(ip_set_id_t index, const ip_set_ip_t adt, unsigned op)
{
	struct ip_set_req_adt *req_adt;
	size_t size;
	void *data;
	int res = 0;
	struct ip_set_req_iphash *ipdata = malloc(sizeof(struct ip_set_req_iphash));
	if(ipdata == NULL) {
		asd_printf(ASD_DEFAULT,MSG_INFO, "set_adtip malloc ip error!");
		return -1;
	}
	
	memset(ipdata, 0, adt_size);
	ipdata->ip = adt;
		
	/* Alloc memory for the data to send */
	size = sizeof(struct ip_set_req_adt) + adt_size ;
	data = malloc(size);
	if(data == NULL){
		asd_printf(ASD_DEFAULT,MSG_INFO, "set_adtip malloc data error!");
		os_free(ipdata);   //xk debug:resource leak
		return -1;
	}
	/* Fill out the request */
	req_adt = (struct ip_set_req_adt *) data;
	req_adt->op = op;
	req_adt->index = index;
	memcpy(data + sizeof(struct ip_set_req_adt), ipdata, adt_size);
	
	if (kernel_sendto(data, size) == -1) {
		switch (op) {
		case IP_SET_OP_ADD_IP:
			asd_printf(ASD_DEFAULT,MSG_INFO,"%d is already in set %d.\n", adt, index);
			res = -1;
			break;
		case IP_SET_OP_DEL_IP:
			asd_printf(ASD_DEFAULT,MSG_INFO,"%d is not in set %d.\n", adt, index);
			res = -1;
			break;
		default:
			asd_printf(ASD_DEFAULT,MSG_INFO,"IP %d set %d error!\n",adt,index);
			res = -1;
			break;
		}
	}
	else 
	{
		asd_printf(ASD_DEFAULT,MSG_DEBUG,"ip %d set %d success!\n",adt,index);
		res = 0;
	}
	free(data);
	free(ipdata);

	return res;
}
ip_set_id_t eap_get_set_byname(const char *setname)
{
	return set_index_find_byname(setname);
}

int del_ip_from_iphashset(const char *set_name, const ip_set_ip_t ip)
{
	ip_set_id_t index;
	int res;
	
	index = set_index_find_byname(set_name);
	res = set_adtip(index, ip, IP_SET_OP_DEL_IP);
	if (res != 0) {
		asd_printf(ASD_DEFAULT,MSG_INFO, "set_adtip error\n");
		return -1;
	}

	return 0;		
}

int add_ip_to_iphashset(const char *set_name, const ip_set_ip_t ip)
{
	ip_set_id_t index;
	int res;
	
	index = set_index_find_byname(set_name);
	res = set_adtip(index, ip, IP_SET_OP_ADD_IP);
	if (res != 0) {
		asd_printf(ASD_DEFAULT,MSG_INFO, "set_adtip error\n");
		return -1;
	}

	return 0;		
}
int eap_ipset_do_auth(const char *hansi_info, const uint32_t user_ip)
{
	char set_name[IP_SET_MAXNAMELEN] = {0};

	if (NULL == hansi_info) {
		asd_printf(ASD_DEFAULT,MSG_ERROR, "eap_ipset_do_auth hansi_info is null\n");		
		return -1;
	}
	memset(set_name, 0, sizeof(set_name));
	snprintf(set_name, sizeof(set_name),
		"EAP_%s_AUTH_SET", hansi_info);
	
	return add_ip_to_iphashset(set_name, user_ip);	
}

int eap_ipset_undo_auth(const char *hansi_info, const uint32_t user_ip)
{
	char set_name[IP_SET_MAXNAMELEN] = {0};

	if (NULL == hansi_info) {
		asd_printf(ASD_DEFAULT,MSG_ERROR, "eap_ipset_undo_auth hansi_info is null\n");		
		return -1;
	}
	memset(set_name, 0, sizeof(set_name));
	snprintf(set_name, sizeof(set_name),
		"EAP_%s_AUTH_SET", hansi_info);

	return del_ip_from_iphashset(set_name, user_ip);	
}


int eap_auth_init()
{
	char hansitype = 0;
	char init_cmd[256] = {0};
	int ret = -1;

	if (HANSI_LOCAL != hansitype && HANSI_REMOTE != hansitype) {
		asd_printf(ASD_DEFAULT,MSG_INFO, "eap_auth_init hansi type unknown %d\n", hansitype);
		return -1;
	}
	if (vrrid < 0 || vrrid >= 16) {
		asd_printf(ASD_DEFAULT,MSG_INFO, "eap_auth_init hansi id %d is out of range 0~16\n", vrrid);
		return -1;
	}

	memset(&g_eap_auth, 0, sizeof(g_eap_auth));
	g_eap_auth.hansi_type = local;
	g_eap_auth.hansi_id = vrrid;
	g_eap_auth.auth_type = EAP_AUTH_IPSET;
	g_eap_auth.do_auth = eap_ipset_do_auth;
	g_eap_auth.undo_auth = eap_ipset_undo_auth;

	hansitype = (local == HANSI_LOCAL) ? 'L' : 'R';
	snprintf(g_eap_auth.hansi_info, 15, "%c%d", hansitype, vrrid);

	eap_nmp_mutex_init(&eap_iptables_lock, IPTABLES_LOCK_FILE);
	
	snprintf(init_cmd, sizeof(init_cmd), "/usr/bin/eap_auth_rule_init.sh %c %d", hansitype, vrrid);

	eap_nmp_mutex_lock(&eap_iptables_lock);
	ret = system(init_cmd);
	eap_nmp_mutex_unlock(&eap_iptables_lock);

	ret = WEXITSTATUS(ret);
	asd_printf(ASD_DEFAULT,MSG_DEBUG,"eap_auth_init cmd = %s,ret = %d\n",init_cmd,ret);
	return ret;
}

int eap_auth_exit()
{
	char hansitype = 0;
	char exit_cmd[256] = {0};
	int ret = -1;
	if (g_eap_auth.hansi_type != local || g_eap_auth.hansi_id != vrrid) {
		asd_printf(ASD_DEFAULT,MSG_INFO, "eap_auth_exit input err "
			"hansi_type=%d, hansi_id=%d\n", local, vrrid);
		return -1;
	}
	
	memset(&g_eap_auth, 0, sizeof(g_eap_auth));
	memset(g_eap_auth.hansi_info, 0, sizeof(g_eap_auth.hansi_info));
	
	hansitype = (local == HANSI_LOCAL) ? 'L' : 'R';
	snprintf(exit_cmd, sizeof(exit_cmd), "/usr/bin/eap_auth_rule_exit.sh %c %d", hansitype, vrrid);

	eap_nmp_mutex_lock(&eap_iptables_lock);
	ret = system(exit_cmd);
	eap_nmp_mutex_unlock(&eap_iptables_lock);

	ret = WEXITSTATUS(ret);
	asd_printf(ASD_DEFAULT,MSG_DEBUG,"eap_auth_exit cmd = %s, ret = %d \n",exit_cmd,ret);
	
	ret = eap_nmp_mutex_destroy(&eap_iptables_lock);

	return ret ;
}
int eap_connect_up(uint32_t userip)
{
	int ret = -1;	
	if( 0 == userip)
		return ret;
	asd_printf(ASD_DEFAULT,MSG_DEBUG,"func:%s,userip :%d \n",__func__,userip);
	ret = g_eap_auth.do_auth(g_eap_auth.hansi_info, userip);
	asd_printf(ASD_DEFAULT,MSG_DEBUG,"func:%s  end\n",__func__);

	return ret;
}

int eap_connect_down(uint32_t userip)
{
	int ret = -1;
	if( 0 == userip)
		return ret;
	ret = g_eap_auth.undo_auth(g_eap_auth.hansi_info, userip);
	return ret;
}

int eap_clean_all_user()
{
	char hansitype = 0;
	char clean_cmd[256] = {0};
	int ret = -1;
	if (g_eap_auth.hansi_type != local || g_eap_auth.hansi_id != vrrid) {
		asd_printf(ASD_DEFAULT,MSG_INFO, "eap_clean_all_user input err "
			"hansi_type=%d, hansi_id=%d\n", local, vrrid);
		return -1;
	}

	hansitype = (local == HANSI_LOCAL) ? 'L' : 'R';
	snprintf(clean_cmd, sizeof(clean_cmd), "/usr/bin/eap_auth_clean_all.sh %c %d", hansitype, vrrid);

	eap_nmp_mutex_lock(&eap_iptables_lock);
	ret = system(clean_cmd);
	eap_nmp_mutex_unlock(&eap_iptables_lock);

	ret = WEXITSTATUS(ret);
	asd_printf(ASD_DEFAULT,MSG_INFO, "eap_clean_all_user cmd=%s, ret=%d\n", clean_cmd, ret);

	return ret;	
}

