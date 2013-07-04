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
* ws_user_manage.c
*
*
* CREATOR:
* autelan.software.Network Dep. team
*
* DESCRIPTION:
*
*
*
*******************************************************************************/
#if 0
#include "ws_user_manage.h"
#include<sys/socket.h>
#include <sys/un.h>
#include <sys/stat.h>
#include <sys/wait.h>
#include <stdio.h>
#include <string.h>
#include <fcntl.h>
#include <stdlib.h>
#include "../ws_public.h"

/*************************
*创建一个请求的数据包
*在不同的req_type下，param1和param2有不同的意义。
*REQ_GET_ONLINE_NUM:参数无用
REQ_GET_USR_BY_INDEX_RANG:param1 -> begin index, param2 end index; becareful it pointer.
REQ_LOGOUT_BY_INFO:param1 -> pointer of STOnlineUserInfo
REQ_GET_BY_INDEX:param1 -> the index whitch to find
REQ_GET_BY_NAME:param1 -> user name
REQ_GET_BY_MAC:param1 -> user mac
REQ_GET_BY_IP:param1 -> user ip
***************************/
STUserManagePkg *createRequirePkg( USER_MNG_TYPE req_type, void *param1, void *param2 )
{
	STUserManagePkg *pstPkgRet;
	
	pstPkgRet = (STUserManagePkg *)malloc(sizeof(STUserManagePkg));
	if( NULL == pstPkgRet )
	{
		return NULL;	
	}
	memset( pstPkgRet, 0, sizeof(STUserManagePkg) );           
	
	pstPkgRet->len = sizeof(STUserManagePkg);
	pstPkgRet->pkg_type = req_type;
	//fprintf(stderr,"req_type=%d",req_type);
	switch( req_type )
	{
		case REQ_GET_ONLINE_NUM:
		case REQ_GET_EAG_USER_INFO:
		case REQ_GET_AP_USER_INFO:
		case REQ_GET_BSS_USER_INFO:
		case REQ_GET_USER_INFO:
		case REQ_GET_EAG_ERRORCODE:
			break;
		case REQ_GET_AUTH_TYPE:
			break;	
		case REQ_GET_USR_BY_INDEX_RANG:
			pstPkgRet->data.req.reqinfo.idx.idx_begin = *((int*)param1);
			pstPkgRet->data.req.reqinfo.idx.idx_end = *((int*)param2);
			break;
		case REQ_LOGOUT_BY_INFO:
			//memcpy( &(pstPkgRet->data.req.reqinfo.user_info), param1, sizeof(STOnlineUserInfo) );
			pstPkgRet->data.req.reqinfo.user_ip = *((unsigned int*)param1);
			break;
		case REQ_LOGOUT_BY_INFO_MAC:
			//memcpy( &(pstPkgRet->data.req.reqinfo.user_info), param1, sizeof(STOnlineUserInfo) );
			memcpy( pstPkgRet->data.req.reqinfo.user_mac, param1, sizeof(pstPkgRet->data.req.reqinfo.user_mac) );
			break;
			
		case REQ_LOGOUT_BY_INFO_NAME:
			//memcpy( &(pstPkgRet->data.req.reqinfo.user_info), param1, sizeof(STOnlineUserInfo) );
			memcpy( pstPkgRet->data.req.reqinfo.username, param1, sizeof(pstPkgRet->data.req.reqinfo.username) );
			break;
			
		case REQ_GET_BY_INDEX:
			pstPkgRet->data.req.reqinfo.idx_get = *((int*)param1);
			break;
		case REQ_GET_BY_NAME:
			memcpy( pstPkgRet->data.req.reqinfo.username, param1, sizeof(pstPkgRet->data.req.reqinfo.username) );
			break;
		case REQ_GET_BY_MAC:
			memcpy( pstPkgRet->data.req.reqinfo.user_mac, param1, sizeof(pstPkgRet->data.req.reqinfo.user_mac) );
			break;
		case REQ_GET_BY_IP:
			pstPkgRet->data.req.reqinfo.user_ip = *((unsigned int*)param1);
			break;
		case ACK_GET_ONLINE_NUM:
		case ACK_GET_USR_BY_INDEX_RANG:
		case ACK_LOGOUT_BY_INFO:
		case ACK_GET_BY_INDEX:
		case ACK_GET_BY_NAME:
		case ACK_GET_BY_MAC:
		case ACK_GET_BY_IP:
		case ACK_GET_EAG_ERROECODE:
		default:
			free( pstPkgRet );
			return NULL;
			break;
	}
	
	return pstPkgRet;
}

#if 0
int create_unix_socket( char *sockpath )
{
	struct sockaddr_un remote;
	int len;
	int fd=-1;

	fd = socket(AF_UNIX, SOCK_DGRAM, 0);
	if ( fd == -1) {
		fprintf(stderr,"create_unix_socket create unix socket failed");
		return fd;
	}

	remote.sun_family = AF_UNIX;
	strcpy(remote.sun_path, sockpath);/*how len of sun_path??*/
	len = strlen(remote.sun_path) + sizeof(remote.sun_family);
	unlink(remote.sun_path);
	if (bind(fd, (struct sockaddr *)&remote, len) == -1) {
		fprintf(stderr, "create_unix_socket socket fd %d bind error %d", fd, errno);
		close(fd);
		fd = -1;
		return fd;
	}

	return fd;
}
#else
void chmod_file( char *path )
{
	#if 0
	char cmd[1024];

	if( NULL != path && *path != 0 )
	{
		sprintf( cmd, "sudo chmod 777 %s 2>/dev/null", path );
		system( cmd );
	}
	#endif
	chmod(path, S_IRWXU|S_IRWXG|S_IRWXO);

	return;
}
int conn_to_unix_socket( char *sockpath )
	{
		struct sockaddr_un remote;
		int len;
		int fd=-1;

		chmod_file( sockpath );
		fd = socket(AF_UNIX, SOCK_STREAM, 0);
		if ( fd == -1) {
/*			fprintf(stderr,"create_unix_socket create unix socket failed\n"); */
			return fd;
		}
	
		remote.sun_family = AF_UNIX;
		strcpy(remote.sun_path, sockpath);/*how len of sun_path??*/
		len = strlen(remote.sun_path) + sizeof(remote.sun_family);
		if (connect(fd, (struct sockaddr *)&remote, len) == -1) {
			//fprintf(stderr, "conn_to_unix_socket socket fd %d    %s\n", fd, strerror(errno));
			close(fd);
			fd = -1; 
			return fd;
		}
	
		return fd;
	}


#endif



/*发送请求，等待相应。函数的返回可能根据网络状况有一定的延时，但最长不超过wait指定的秒数，
如果wait<=0,则默认延迟3秒，最长不超过15s*/
int doRequire( STUserManagePkg *pkg, int ip, int port, int wait, STUserManagePkg **p_pkg_get )
{
	int fd;
	int len,ret;
	int data_len;
	struct sockaddr_in remoteaddr;
	int begintime;
	
	char data[1024];

	int delay = 0;
	char path[1024];
	
	//fprintf(stderr, "doRequire  \n");
	if( wait <= 0 )
	{
		delay = 3;	
	}
	else if( wait >15 )
	{
		delay = 15;	
	}
	else
	{
		delay = wait;	
	}
		
	/*创建连接*/
#if 0	
	fd = socket(AF_INET, SOCK_STREAM, 0);
	remoteaddr.sin_family = AF_INET;
	remoteaddr.sin_addr.s_addr = htonl(ip);/*连接到自己*/
	remoteaddr.sin_port = htons(port);
	len = sizeof(remoteaddr);
	fprintf(stderr, "before connect!!  \n");
	ret = connect(fd, (struct sockaddr *)&remoteaddr, len);
	if(ret == -1) 
	{
		fprintf(stderr, "connect() error\n");
		return -1;
	}
	fprintf(stderr, "after connect() ret=%d\n", ret );
	/*发送请求*/
#else
	//fprintf(stderr, "after connect() begintime=%d\n", begintime );
	if( pkg->eagins_id < EAGINS_ID_BEGIN || pkg->eagins_id>MAX_EAGINS_NUM )
	{
		sprintf( path, EAGINS_UNIXSOCK_PREFIX"%d", 0 );
	}
	else
		
	{
		sprintf( path, EAGINS_UNIXSOCK_PREFIX"%d", pkg->eagins_id );
	}
	/*modify */
	fd = conn_to_unix_socket( path );/*just get ins 2 for test*/
	if( fd <= -1 )
	{
/*		fprintf(stderr,"failed create unix socket\n"); */
		return -1;
	}
#endif	
	begintime = time(0);
	//fprintf(stderr, "after connect() begintime=%d\n", begintime );	
	if( 0 != sendpkg( fd, delay, pkg ) )
	{
		fprintf( stderr, "sendpkg error!" );
		close(fd);
		return -1;
	}
	//fprintf(stderr, "after send() time(0)=%d\n", time(0) );		
	/*接收响应*/
	if( time(0) - begintime >= delay )
	{
		fprintf( stderr, "time out!!!" );		
		close(fd);
		return -1;	
	}
	
	//fprintf(stderr, "before getpkg!!  time(0) = %d\n", time(0));
	ret = getpkg( fd, delay - (time(0) - begintime), p_pkg_get );
	//fprintf( stderr, "after getpkg ret = %d\n", ret );
	close( fd );
	return ret;
}


/*return  0 为认证用户
		-1为非认证用户
*/
int snmp_get_userstate_bymac(char * mac)
{
	STUserManagePkg * req = NULL;
	STUserManagePkg * rsv = NULL;
	
	req = createRequirePkg( REQ_GET_BY_MAC, mac, NULL );
	if( req == NULL )
	{
		printf(  "create req pkg failed !!");
		return -1;
	}
	int ret = doRequire( req, ntohl(inet_addr("127.0.0.1")), 2001, 5, &rsv );
	if( rsv == NULL )
	{
		printf( "get respones failed!\n" );
		return -1;
	}
	printf( "rsv->all_user_num=%d!\n", rsv->all_user_num );

/*判断用户是否认证*/
	if( rsv->all_user_num == 0 )
		return -1;  /*no authed*/
	else
		return 0; /*authed*/
	
}




STUserManagePkg * show_auth_users_info(int * start, int * end)
{
	STUserManagePkg * req = NULL;
	STUserManagePkg * rsv = NULL;


	req = createRequirePkg( REQ_GET_USR_BY_INDEX_RANG, start, end );

	if( req == NULL )
	{
		fprintf( stderr, "create req pkg failed !!");
		return NULL;
	}

	int ret = doRequire( req, ntohl(inet_addr("127.0.0.1")), 2001, 5, &rsv );

	if( rsv == NULL )
	{
		fprintf( stderr, "get respones failed!\n" );
		return NULL;
	}
	
	return rsv;
}

#define PORTAL_CONF_PATH	"/opt/services/conf/portal_conf.conf"

inline cp_interface_link new_interface_node(char *if_name, int captive_portal_id,cp_interface_link next)
{
	cp_interface_link t = malloc(sizeof(*t));
	t->if_name = if_name;
	t->captive_portal_id=captive_portal_id;
	t->next = next;
	return t;
}

inline cp_interface_link interface_list_init(void) 
{
	return new_interface_node(NULL,-1, NULL); 
}

 void  interface_list_destroy(cp_interface_link head)
{
	struct cp_interface_t *f1,*f2;
	f1=head->next;		 
	f2=f1->next;

	while(f2!=NULL)
	{
	  free(f1->if_name);
	  
	  free(f1);
	  f1=f2;
	  f2=f2->next;
	}
	free(f1);
	head=NULL;
}

inline void interface_insert_node(cp_interface_link head, int captive_portal_id,char *if_name)
{
	head->next = new_interface_node(if_name,captive_portal_id, head->next);
}
	
inline int interface_list_find_node(cp_interface_link head, int captive_portal_id,char *if_name)
{
	cp_interface_link t;
	for (t=head->next; t; t=t->next)
		if (strcmp(t->if_name, if_name) == 0 && ( t->captive_portal_id== captive_portal_id ))
			break;
	return t != NULL;
}

static int  check_captiveportal_id( int id, char *record, int len )
{
	FILE *db;
	
	db= fopen( PORTAL_CONF_PATH,"r");
	
	if( NULL == record )
		return 0;
	
	if( id < 0 || id > 7 )
	{
		return 0;
	}
	
	if( NULL == db )
	{
		*record = 0;
		return 0;	
	}
	
	while( fgets( record, len, db ) )
	{
		if( id == record[0]-'0' )
		{
			fclose( db );
			return 1;
		}
	}
	
	*record = 0;
	fclose( db );
	return 0;
}

static int  check_interfaces_format(const char *str, char *err, int size)
{
	char if_tmp[256], record[256], buf_if[256];
	char *token_if_tmp = NULL, *save_if_tmp = NULL, *token_buf_if = NULL, *save_buf_if = NULL;
	FILE *fp = NULL;
	infi  interf;
	infi *q = NULL;
	int cp_id;
	cp_interface_link head = NULL;

	memset(if_tmp, 0, sizeof(if_tmp));
	memset(record, 0, sizeof(record));
	memset(buf_if, 0, sizeof(buf_if));
	strncpy(if_tmp, str, sizeof(if_tmp)-1);

	const char *p = NULL;
	int fmt_ok = 1;
	if (NULL ==  str || '\0' == str[0] || ',' == str[0])
		fmt_ok = 0;
	for (p = str; *p; p++)
		if (',' == *p && ',' == *(p+1))
			fmt_ok = 0;
	if (',' == *(p-1))
		fmt_ok = 0;
	if (!fmt_ok){
		strncpy(err, "interfaces formated wrongly", size);
			return INTERFACES_CHECK_FAILURE;
	}
	
//	head = interface_list_init();
	for (token_if_tmp = strtok_r(if_tmp, ",", &save_if_tmp); token_if_tmp; token_if_tmp = strtok_r(NULL, ",", &save_if_tmp)){
		if (strcmp(token_if_tmp, "lo") == 0){
			strncpy(err, "lo isn't a physical interface", size);
			return INTERFACES_CHECK_FAILURE;
		}

		interface_list_ioctl(0, &interf);
		for (q = interf.next; q; q = q->next)
			if (strcmp(q->if_name, token_if_tmp) == 0) break;
		if (NULL == q){
			snprintf(err, size, "interface %s not exist or no ip address", token_if_tmp);
			return INTERFACES_CHECK_FAILURE;
		}
		
		if ( (fp = fopen(PORTAL_CONF_PATH, "r")) != NULL){
			while (fgets(record, sizeof(record), fp) != NULL){
				sscanf(record ,"%d %*s %*s %*s %*s %s", &cp_id, buf_if);
				for (token_buf_if = strtok_r(buf_if, ",", &save_buf_if); token_buf_if; token_buf_if = strtok_r(NULL, ",", &save_buf_if))
					if (strcmp(token_buf_if, token_if_tmp) == 0){
						snprintf(err, size, "interface %s has been used by captive portal %d", token_if_tmp, cp_id);
						fclose(fp);
						return INTERFACES_CHECK_FAILURE;
					}
			}
			fclose(fp);
		}
/*
		if (interface_list_find_node(head, token_if_tmp)){
			snprintf(err, size, "interface %s is duplicated", token_if_tmp);
			return INTERFACES_CHECK_FAILURE;
		}
		else
		{
			interface_insert_node(head, token_if_tmp);
		}
*/
	}

//	interface_list_destroy(head);
	return INTERFACES_CHECK_SUCCESS;
}

/*for SNMP */
int  add_interface_to_captive_portal(int captive_portal_id,char  *input_interface)
{
	int ret, rec;
	char  interfaces[256], command[256],err[100];
	char record[256]={0};
	memset(command, 0, sizeof(command));
	memset(err, 0, sizeof(err));

	if (captive_portal_id < 0 || captive_portal_id > 7){
		/*"captive portal id must range <0-7>*/
		return  -1;
	}
	
	ret = check_interfaces_format(input_interface, err, sizeof(err)-1);
	if ( INTERFACES_CHECK_SUCCESS != ret){
		return  -1;
	}

	memset(record, 0, sizeof(record));
	ret = check_captiveportal_id(captive_portal_id, record, sizeof(record));
	if (0 == ret)
	{
		printf ("captive portal %d has't been configured!\n", captive_portal_id);
		return  -2;
	}
	sprintf( command, "sudo /usr/bin/cp_add_portal_interface.sh %d  %s >/dev/null 2>&1", 
							captive_portal_id, input_interface);
	rec = system(command);
 	ret = WEXITSTATUS(rec);
#if  0
	switch( ret )
	{
		case 0:
			break;
		case 3:
			printf "The ID has been setted!\n");
			break;
		case 4:
			printf "IP address is an other portal server!\n");
			break;
		case 5:
			printf "The user has been used!\n");
			break;
		case 1:
		case 2:
		default:
			printf "Unknown Error!\n");
			break;
	}
#endif
	if( 0 != ret )
	{	
		printf("add iptables rules failed!\n");
		return  -1;
	}
	else
	{
		printf("add iptables rules success!\n");
		return  0;
	}
}

static int  check_id_interface_from_file( int id, char *cp_interface )
{
	char  buf_if[256]={0},  record[256]={0};
	char  *token_buf_if = NULL, *save_buf_if = NULL;
	int   cp_id=-1;
	FILE *fp=NULL;
	if ( (fp = fopen(PORTAL_CONF_PATH, "r")) != NULL)
	{
		while (fgets(record, sizeof(record), fp) != NULL)
		{
			sscanf(record ,"%d %*s %*s %*s %*s %s", &cp_id, buf_if);
			for (token_buf_if = strtok_r(buf_if, ",", &save_buf_if); token_buf_if; token_buf_if = strtok_r(NULL, ",", &save_buf_if))
			if ((strcmp(token_buf_if, cp_interface) == 0) && (id== cp_id))
			{
				printf("interface %s has been used by captive portal %d\n", cp_interface, cp_id);
				fclose(fp);
				return INTERFACES_CHECK_SUCCESS;
			}
		}
		fclose(fp);
	}
	return  INTERFACES_CHECK_FAILURE;
}

int  delete_interface_of_captive_portal(int captive_portal_id,char  *input_interface)
{
	int cp_id, ret, status;
	char command[256], record[256];
	if( NULL == input_interface )
	return  -2;
	
	if( captive_portal_id < 0 || captive_portal_id > 7 )
	{
		return  -2;
	}

	ret=check_id_interface_from_file(captive_portal_id,input_interface);
	if ( INTERFACES_CHECK_SUCCESS ==ret )
	{
		memset(command, 0, sizeof(command));
		sprintf(command,"sudo /usr/bin/cp_del_portal_interface.sh %d  %s > /dev/null 2 >&1",captive_portal_id,input_interface);
		status = system(command); 	 
		ret = WEXITSTATUS(status);	
		/*reset the captive portal ID, when  only one interface configured in this captive portal ID*/
		if (ret == 2 )
		{
			memset(record, 0, sizeof(record));
			ret = check_captiveportal_id(captive_portal_id, record, sizeof(record));
			if (0 == ret)
			{
				printf ("error:captive portal %d has't been configured!\n", captive_portal_id);
				return  -2;
			}

			sprintf(command, "sudo cp_del_portal_id.sh %d > /dev/null 2>&1", captive_portal_id);
			printf ("reset cp cmd = %s\n", command);
			status = system(command); 	 
			ret = WEXITSTATUS(status);		
			if( 0== ret )
			{
				return 0;
				printf ("clear captive portal %d successed!\n", captive_portal_id);
			}
		}
		else
		{
			return 0;
		}
	}
	else
	{
		printf("parameter error");
		return -1;
	}
}

int   get_captive_portal_info( cp_interface_link captive_portal_info ,int * captive_num)
{
	int ret, id;
	char record[256];
	cp_interface_link head = NULL;
	int cative_node_num=0;

	memset(record, 0, sizeof(record));
	int cur_id, i, num = 0;
	char ip_addr[16], if_info[256], strPort[10], *p=NULL, *token=NULL;

	memset(ip_addr, 0, sizeof(ip_addr));
	memset(if_info, 0, sizeof(if_info));
	for (i = 0; i <= 7; i++)
	if (check_captiveportal_id(i, record, sizeof(record)) == 1) num++;
	if( num > 0)
	{
		//printf("captive num=%d\n",num);
		head = interface_list_init();
	}

	cp_interface_link  ctail=NULL;
	captive_portal_info->next=NULL;
	ctail=captive_portal_info;
	
	for (i = 0; i <= 7; i++)    /*if there is serveral line in  /opt/services/conf/portal_conf.conf*/
		if (check_captiveportal_id(i, record, sizeof(record)) == 1)    /*if there are serveral interface in one captive-portal ID*/
		{
			for (p = record; *p; p++)
			{
				if ('\r' == *p || '\n' == *p)
				{
						*p = '\0';
						break;
				}
			}
			sscanf(record, "%d	%s	%s %*s %*s %s", &cur_id, ip_addr, strPort, if_info);
			//vty_out(vty,"%-16d %-16s %-16s", cur_id, ip_addr, strPort);
			for (token = strtok(if_info, ","); token; token = strtok(NULL, ","))
			{
				
				cative_node_num++;
				//printf("captive_node_num=%d\n",cative_node_num);
				if ( interface_list_find_node( head,cur_id,token) == 0)
				 {
					cp_interface_link cq;
					cq=(cp_interface_link)malloc(sizeof(struct cp_interface_t)+1);
					memset(cq,0,sizeof(struct cp_interface_t)+1);
					if(NULL == cq)
					{
						return  -1;
					}

					cq->if_name=(char *)malloc(50);			
					memset(cq->if_name,0,50);

					cq->captive_portal_id=cur_id;
					//printf("token interface=%s\n",token);
					strcpy(cq->if_name,token);
					//printf("cq->if_name=%s\n",cq->if_name);

					 cq->next = NULL;
				     	 ctail->next = cq;
				     	 ctail = cq;
  			       }

			}
		}
		*captive_num=cative_node_num;
		return 0;
}

#if 0
int main(int argc,char * argv[])
{
	struct cp_interface_t captive_portal_info,*cq=NULL;
       //struct web_service_st  https_head,*dq=NULL;	
       int captive_num=0;
	get_captive_portal_info(&captive_portal_info,&captive_num);
	//while( &captive_portal_info!=NULL && &(captive_portal_info.next)  != NULL )
	  if( (&captive_portal_info!=NULL) &&(captive_portal_info.next  != NULL) )
            {
        	      cq=captive_portal_info.next;
                    while(cq!=NULL)
                    {
                      printf("ID : ");
                      printf( "%d\t",cq->captive_portal_id);
			  printf(" inteface : ");
                   	  printf("%s\n",cq->if_name);
                      cq=cq->next;
                    }
	   }
	printf("captive_num=%d\n",captive_num);
	if ( NULL != &captive_portal_info && captive_num>0 )
	{
		interface_list_destroy( &captive_portal_info);		
	}
	return 0;
}
#endif
#endif
