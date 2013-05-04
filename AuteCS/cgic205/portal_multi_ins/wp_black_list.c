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
* wp_black_list.c
*
*
* CREATOR:
* autelan.software.Network Dep. team
* shaojw@autelan.com
*
* DESCRIPTION:
* system infos 
*
*
*******************************************************************************/
#include <stdio.h>
#include <stdlib.h>
#include <sys/wait.h>
#include "ws_module_container.h"
#include "ws_portal_container.h"
#include "cgic.h"
#include "ws_err.h"
#include "ws_usrinfo.h"
#include "ws_ec.h"

#define _DEBUG	0

#if _DEBUG
#define debug_printf(a...) fprintf(a)
#else
#define debug_printf(a...)
#endif

/*
#define SCRIPT_PATH			"sudo /usr/bin/"
#define ADD_BLACK_LIST_CMD_SINGIP	SCRIPT_PATH"cp_add_white_list.sh %s %s %s > /dev/null 2>&1"
#define DEL_BLACK_LIST_CMD_SINGIP	SCRIPT_PATH"cp_del_white_list.sh %s %s %s > /dev/null 2>&1"

#define ADD_BLACK_LIST_CMD	SCRIPT_PATH"cp_add_white_list.sh %s %s-%s %s > /dev/null 2>&1"
#define DEL_BLACK_LIST_CMD	SCRIPT_PATH"cp_del_white_list.sh %s %s-%s %s > /dev/null 2>&1"
#define GET_BLACK_LIST_CMD	SCRIPT_PATH"cp_get_white_list.sh %s 2>/dev/null"

#define PORTAL_CONF_PATH	"/opt/services/conf/portal_conf.conf"
#define PORTAL_LIST_PATH	"/opt/services/option/portal_option"
#define GET_ID_LIST_CMD		"sudo cat "PORTAL_CONF_PATH" | awk '{print $1}' | sort"
#define CHECK_ID_INUSE_CMD	"sudo grep \"^%s\" "PORTAL_CONF_PATH" >/dev/null 2>&1"
*/


#define SCRIPT_PATH			"sudo /usr/bin/"
//#define SCRIPT_PATH			"sudo /opt/www"
#define ADD_BLACK_LIST_CMD_SINGIP	SCRIPT_PATH"cp_add_black_list.sh %s %s %s > /dev/null 2>&1"
#define DEL_BLACK_LIST_CMD_SINGIP	SCRIPT_PATH"cp_del_black_list.sh %s %s %s > /dev/null 2>&1"

#define ADD_BLACK_LIST_CMD	SCRIPT_PATH"cp_add_black_list.sh %s %s-%s %s > /dev/null 2>&1"
#define DEL_BLACK_LIST_CMD	SCRIPT_PATH"cp_del_black_list.sh %s %s-%s %s > /dev/null 2>&1"
#define GET_BLACK_LIST_CMD	SCRIPT_PATH"cp_get_black_list.sh %s 2>/dev/null"

#define PORTAL_CONF_PATH	"/opt/services/conf/portal_conf.conf"
#define PORTAL_LIST_PATH	"/opt/services/option/portal_option"
#define GET_ID_LIST_CMD		"sudo cat "PORTAL_CONF_PATH" | awk '{print $1}' | sort"
#define CHECK_ID_INUSE_CMD	"sudo grep \"^%s\" "PORTAL_CONF_PATH" >/dev/null 2>&1"




#define DEL_ENTER_CHAR(line)	{for(;strlen(line)>0&&(line[strlen(line)-1]==0x0d||line[strlen(line)-1]==0x0a);line[strlen(line)-1]='\0');}
#define IS_LEGAL_ID(id)		(id[0]>='0'&&id[0]<='7')




#define MAX_CMD_LEN			256
#define IP_ADDR_LEN			24
#define MAX_PORT_LEN		32
#define MAX_BLACK_LIST_NUM	50
#define MAX_ID_NUM			8

//定义错误
#define ERR_DEFAULT			-1
#define ERR_NOT_LEGAL_ID	(ERR_DEFAULT-1)
#define ERR_IPADDR_FORMAT	(ERR_DEFAULT-2)
#define ERR_DO_COMMAND		(ERR_DEFAULT-3)
#define ERR_PORTAL_ID		(ERR_DEFAULT-4)
#define ERR_IP_ADDR			(ERR_DEFAULT-5)
#define ERR_PORTAL_ID_NOUSE	(ERR_DEFAULT-6)
#define ERR_IP_NOT_IN_RANGE	(ERR_DEFAULT-7)
#define ERR_IP_HAS_IN_LIST	(ERR_DEFAULT-8)
#define ERR_IP_NOT_IN_LIST	(ERR_DEFAULT-9)
#define ERR_BLACK_LIST_MAX	(ERR_DEFAULT-10)
#define ERR_PORT_FORMAT		(ERR_DEFAULT-11)

//shell 返回的错误
#define ERR_OUT_OF_RANGE	1

//定义用到的结构体
typedef struct{
	char ipaddr_begin[IP_ADDR_LEN];
	char ipaddr_end[IP_ADDR_LEN];//can be empty
	char port[MAX_PORT_LEN+1];
}black_list_item_t;

typedef struct{
	char id[4];
	int num;
	black_list_item_t ipaddr_list[MAX_BLACK_LIST_NUM];
}black_list_t;






//定义全局变量

//定义静态函数
static int is_legal_ip( char *ipaddr )
{
	unsigned int ip0=0,ip1=0,ip2=0,ip3=0;	
	int iRet;
	
	iRet = sscanf( ipaddr,"%u.%u.%u.%u", &ip0, &ip1, &ip2, &ip3 );
	
	debug_printf( stderr, "is_legal_ip %u.%u.%u.%u\n", ip0,ip1,ip2,ip3 );
	if( 4 == iRet )
	{
		if( (ip0 > 0 && ip0 < 255) || (ip1 >= 0 && ip1 < 255) || (ip2 >= 0 && ip2 < 255) || (ip3 >= 0 && ip3 < 255) )
		{
			debug_printf( stderr, "is_legal_ip not legal ip\n" );
			return 1;
		}
	}
	
	debug_printf( stderr, "is_legal_ip return 0\n" );
	return 0;	//1表示是正确的ipaddr格式
}

static int is_legal_port( char *port )
{
	char *port_temp;
	char *p;
	char port_str[10];
	int port_num;
	
	port_temp = strdup(port);
	debug_printf( stderr, "is_legal_port  begin port = %s\n", port );

	p = port_temp;//just allow number and ',' ---add by wk
	while(*p != '\0')
	{
		if( (*p < '0' || *p > '9') && *p != ',')
		{
			free( port_temp );
			return 0;	
		}
		p++;
	}
	
	p = strtok( port_temp, "," );
	while( NULL != p )
	{
		debug_printf( stderr, "is_legal_port  p = %s\n", p );
		strcpy( port_str, p );
		port_num = atoi(port_str);
		if( port_num <= 0 || port_num > 65535 )
		{
			free( port_temp );
			return 0;	
		}
		p = strtok( NULL, "," );
	}
	debug_printf( stderr, "is_legal_port  end port = %s\n", port );
	free( port_temp );
	return 1;
}



static int cmp_black_list_item( black_list_item_t *p_item1, black_list_item_t *p_item2 )
{
	int ret;
	
	if( NULL == p_item1 || NULL == p_item2 )
	{
		return -1;	
	}
	
	ret = strcmp( p_item1->ipaddr_begin, p_item2->ipaddr_begin );
	if( ret != 0 )
	{
		return ret;	
	}
	
	ret = strcmp( p_item1->ipaddr_end, p_item2->ipaddr_end );
	if( ret != 0 )
	{
		return ret;	
	}

	ret = strcmp( p_item1->port, p_item2->port );
	if( ret != 0 )
	{
		return ret;	
	}
	
	return 0;
}

static int is_ipaddr_in_list( black_list_t *p_list,black_list_item_t *p_item )
{
	int i;
	
	if( NULL == p_list || NULL == p_item )
	{
		return 0;	
	}
	
	for( i=0; i<p_list->num; i++ )
	{
		if( cmp_black_list_item(&(p_list->ipaddr_list[i]),p_item) == 0 )	
		{
			return 1;
		}
	}
	
	return 0;
}

static int is_id_in_use( char *id )
{
	int status, ret;
	char cmd[256];


	snprintf( cmd, sizeof(cmd)-1, CHECK_ID_INUSE_CMD, id );

	debug_printf( stderr, "is_id_in_use  cmd = %s\n", cmd );
	status = system(cmd);
	ret = WEXITSTATUS(status);
	
	debug_printf( stderr, "is_id_in_use  id=%s  status=%d ret = %d\n", id, status, ret );	

	return (ret!=0)?0:1;
}

static int add_addr_to_list( black_list_t *p_list, char *ipaddr_begin, char *ipaddr_end, char *port )
{	
	black_list_item_t t_list_item;
	
	if( NULL == p_list || NULL == ipaddr_begin || NULL == ipaddr_end || NULL == port )
	{
		return -1;	
	}
	
	strncpy( t_list_item.ipaddr_begin, ipaddr_begin, sizeof(t_list_item.ipaddr_begin) );
	strncpy( t_list_item.ipaddr_end, ipaddr_end, sizeof(t_list_item.ipaddr_end) );
	strncpy( t_list_item.port, port, sizeof(t_list_item.port) );
	if( is_ipaddr_in_list( p_list, &t_list_item ) )
	{
		return 0;
	}
	
	if( p_list->num < MAX_BLACK_LIST_NUM )
	{
		strcpy( p_list->ipaddr_list[p_list->num].ipaddr_begin, ipaddr_begin );
		strcpy( p_list->ipaddr_list[p_list->num].ipaddr_end, ipaddr_end );
		strcpy( p_list->ipaddr_list[p_list->num].port, port );
		p_list->num++;
	}
	else
	{
		return ERR_BLACK_LIST_MAX;
	}

	return 0;
}

static int del_addr_from_list( black_list_t *p_list, char *ipaddr_begin, char *ipaddr_end, char *port )
{	
	int i;
	black_list_item_t t_list_item;	
	
	if( NULL == p_list || NULL == ipaddr_begin || NULL == ipaddr_end || NULL == port )
	{
		return -1;	
	}
	
	strncpy( t_list_item.ipaddr_begin, ipaddr_begin, sizeof(t_list_item.ipaddr_begin) );
	strncpy( t_list_item.ipaddr_end, ipaddr_end, sizeof(t_list_item.ipaddr_end) );
	strncpy( t_list_item.port, port, sizeof(t_list_item.port) );
	if( is_ipaddr_in_list( p_list, &t_list_item ) )
	{
		return 0;
	}
	
	for( i=0; i<p_list->num; i++ )
	{
		if( 0 != strcmp( p_list->ipaddr_list[i].ipaddr_begin, ipaddr_begin ) )
		{
			continue;
		}

		if( 0 != strcmp( p_list->ipaddr_list[i].ipaddr_end, ipaddr_end ) )
		{
			continue;
		}
		
		if( 0 != strcmp( p_list->ipaddr_list[i].port, port ) )
		{
			continue;
		}
		break;
	}
	
	if( i < p_list->num )
	{
		memcpy( &(p_list->ipaddr_list[i]), &(p_list->ipaddr_list[i+1]), (p_list->num-i)*sizeof(p_list->ipaddr_list[0]) );
	}
	p_list->num--;
	return 0;
}
/***************************************************************
*USEAGE:	将 ipaddr添加到id的白名单中
*Param:		
*Return:	0 -> success
*			!= 0 -> failure
*Auther:shao jun wu
*Date:2008-12-30 12:06:46
*Modify:(include modifyer,for what resease,date)
****************************************************************/
int add_black_list( black_list_t *p_list, char *ipaddr_begin, char *ipaddr_end, char *port )
{
	char cmd[MAX_CMD_LEN]="";
	int status,ret;
	black_list_item_t t_list_item;	
	
	if( NULL==p_list || (! IS_LEGAL_ID(p_list->id)) )	
	{
		return ERR_NOT_LEGAL_ID;
	}
	
	if( NULL==ipaddr_begin || 1 != is_legal_ip( ipaddr_begin ) )
	{
		return 	ERR_IPADDR_FORMAT;
	}
	
	if( NULL != ipaddr_end && strlen(ipaddr_end) > 0 && 1 != is_legal_ip( ipaddr_end ) )
	{
		return 	ERR_IPADDR_FORMAT;
	}
	
	if( NULL != port && strlen(ipaddr_end) > 0 && strcmp(port, "all") != 0 && 1 != is_legal_port( port ) )
	{
		return 	ERR_PORT_FORMAT;
	}
#if 0	
	if( ! is_id_in_use(p_list->id) )
	{
		return ERR_PORTAL_ID_NOUSE;
	}
#endif	
	strncpy( t_list_item.ipaddr_begin, ipaddr_begin, sizeof(t_list_item.ipaddr_begin) );
	strncpy( t_list_item.ipaddr_end, ipaddr_end, sizeof(t_list_item.ipaddr_end) );
	strncpy( t_list_item.port, port, sizeof(t_list_item.port) );
	if( is_ipaddr_in_list( p_list, &t_list_item ) )
	{
		return ERR_IP_HAS_IN_LIST;
	}
	
	if( strlen(ipaddr_end ) == 0 )
	{
		snprintf( cmd, sizeof(cmd)-1, ADD_BLACK_LIST_CMD_SINGIP	, p_list->id, ipaddr_begin, port );
	}
	else
	{
		snprintf( cmd, sizeof(cmd)-1, ADD_BLACK_LIST_CMD, p_list->id, ipaddr_begin, ipaddr_end, port );
	}
	
	debug_printf( stderr, "add_black_list cmd = %s\n", cmd );
	
	status = system(cmd);
	ret = WEXITSTATUS(status);
	
	debug_printf( stderr, "add_black_list ret = %d \n", ret );
	if( 0 == ret )
	{
		add_addr_to_list( p_list, ipaddr_begin, ipaddr_end, port );
	}
	return ret;//脚本中应该返回0表示这个ip添加成功，否则添加失败。
}

/***************************************************************
*USEAGE:	将 ipaddr从id的白名单中删除
*Param:		
*Return:	0 -> success
*			!= 0 -> failure
*Auther:shao jun wu
*Date:2008-12-30 12:06:51
*Modify:(include modifyer,for what resease,date)
****************************************************************/
int del_black_list( black_list_t *p_list, char *ipaddr_begin, char *ipaddr_end, char *port )
{
	char cmd[MAX_CMD_LEN]="";
	int status,ret;
	
	if( NULL==p_list->id || (! IS_LEGAL_ID(p_list->id)) )	
	{
		return ERR_NOT_LEGAL_ID;
	}
	
	if( NULL==ipaddr_begin || 1 != is_legal_ip(ipaddr_begin) || 
		( NULL != ipaddr_end && strlen(ipaddr_end) > 0 && 1 != is_legal_ip(ipaddr_end) ) )
	{
		return 	ERR_IPADDR_FORMAT;
	}

 	if( NULL != port && strlen(port) > 0 && strcmp(port, "all") != 0 && 1 != is_legal_port(port) )
 	{
 		return ERR_PORT_FORMAT;
 	} 	
	
	if( ! is_id_in_use(p_list->id) )
	{
		return ERR_PORTAL_ID_NOUSE;
	}	

#if 0
	strncpy( t_list_item.ipaddr_begin, ipaddr_begin, sizeof(t_list_item.ipaddr_begin) );
	strncpy( t_list_item.ipaddr_end, ipaddr_end, sizeof(t_list_item.ipaddr_end) );
	strncpy( t_list_item.port, port, sizeof(t_list_item.port) );
	if( is_ipaddr_in_list( p_list, &t_list_item ) )
	{
		return ERR_IP_HAS_IN_LIST;
	}
#endif
	if( strlen(ipaddr_end ) == 0 )
	{
		snprintf( cmd, sizeof(cmd)-1, DEL_BLACK_LIST_CMD_SINGIP	, p_list->id, ipaddr_begin, port );
	}
	else
	{
		snprintf( cmd, sizeof(cmd)-1, DEL_BLACK_LIST_CMD, p_list->id, ipaddr_begin, ipaddr_end, port );
	}
	debug_printf( stderr, "del_black_list cmd = %s\n", cmd );
	status = system(cmd);
	ret = WEXITSTATUS(status);
	
	if( 0 == ret )
	{
		del_addr_from_list( p_list, ipaddr_begin, ipaddr_end, port );
	}
	debug_printf( stderr, "del_black_list ret = %d \n", ret );
	
	return ret;//脚本中应该返回0表示这个ip添加成功，否则添加失败。失败的原因主要有两个一个是已经加入到白名单中，另一个是ip不在允许的范围。错误返回值待定。	
}

/***************************************************************
*USEAGE:	得到id的白名单，
*Param:		id  ->  portal id
			p_list -> 存放列表的结构体
*Return:	0 -> success
*			!= 0 -> failure。
*Auther:shao jun wu
*Date:2008-12-30 12:11:52
*Modify:(include modifyer,for what resease,date)
****************************************************************/

int get_black_list( char *id, black_list_t *p_list )
{
	char cmd[MAX_CMD_LEN]="";
	char line[256];
	char ipaddr_begin[24],ipaddr_end[24],port[10];
	FILE *fp;
	
	debug_printf( stderr, "get_black_list !!!!\n" );
	if( NULL==id || (! IS_LEGAL_ID(id)) )	
	{
		return ERR_NOT_LEGAL_ID;
	}
	
	if( NULL == p_list )
	{
		return ERR_DEFAULT;
	}
	
	p_list->num=0;
	
	strncpy( p_list->id, id, sizeof(p_list->id)-1 );
	
	debug_printf( stderr, "before do cmd !!!!\n" );
	snprintf( cmd, sizeof(cmd)-1, GET_BLACK_LIST_CMD, id );
	fp = popen( cmd, "r" );
	debug_printf( stderr, "after do cmd fp=%x!!!!\n",(unsigned int)fp );
	if( NULL == fp )
	{
		return 	ERR_DO_COMMAND;
	}

	while( !feof( fp ) && p_list->num<MAX_BLACK_LIST_NUM)
	{
		char *p1,*p2;
		
		memset( ipaddr_begin, 0, sizeof(ipaddr_begin) );
		memset( ipaddr_end, 0, sizeof(ipaddr_end) );
		memset( port, 0, sizeof(port) );
		
		fgets( line, sizeof(line), fp );
		
		DEL_ENTER_CHAR(line);

		p1 = line;
		p2 = strchr( p1, '-' );
		if( NULL != p2 )
		{
			strncpy( ipaddr_begin, p1, (unsigned int)p2 - (unsigned int)p1 );
			p1 = p2+1;
			
			p2 = strchr( p1, ':' );
			if( NULL == p2 )
			{
				continue;	
			}
			strncpy( ipaddr_end, p1, (unsigned int)p2 - (unsigned int)p1 );
		}
		else
		{
			p2 = strchr( p1, ':' );
			if( NULL == p2 ) continue;
				
			strncpy( ipaddr_begin, p1, (unsigned int)p2 - (unsigned int)p1 );
		}
		
		strcpy( port, p2+1 );

		add_addr_to_list( p_list, ipaddr_begin, ipaddr_end, port );
	}
	
	pclose( fp );
	
	return 0;
}


/******************************************************************
上面的都是页面调用是需要用到的api，下面才是和页面相关的东西
******************************************************************/
#define SUBMIT_NAME		"submit_add_black_list"


/***************************************************************
定义页面要用到的结构体
****************************************************************/
#define MAX_URL_LEN         256


typedef struct{
	char id[4];
	black_list_item_t ipaddr_add;
	black_list_item_t ipaddr_del;
}STUserInput;

typedef struct{
//	STModuleContainer *pstModuleContainer;
	STPortalContainer *pstPortalContainer;
	struct list *lpublic;/*解析public.txt文件的链表头*/
	struct list *lauth;/*解析control.txt文件的链表头*/
	char encry[BUF_LEN];
	char *username_encry;	         /*存储解密后的当前登陆用户名*/
	int iUserGroup;	//为0时表示是管理员。
	FILE *fp;
	STUserInput stUserInput;
	black_list_t st_black_list;
	
	int formProcessError;
} STPageInfo;

/***************************************************************
申明回调函数
****************************************************************/
static int s_black_prefix_of_page( STPageInfo *pstPageInfo );
static int s_black_content_of_page( STPageInfo *pstPageInfo );
static int getUserInput( STUserInput *pstUserInput );
static int doUserCommand( STPageInfo *pstPageInfo );
static int doRedir( STPageInfo *pstPageInfo  );

/***************************************************************
*USEAGE:	得到已经使用的id列表
*Param:		p_used_id -> 存放id列表的结构体
*Return:	0 -> success
*			!= 0 -> failure。
*Auther:shao jun wu
*Date:2008-12-30 14:12:46
*Modify:(include modifyer,for what resease,date)
****************************************************************/
int cgiMain()
{
	STPageInfo stPageInfo;
	
//初始化常用公共变量
	memset( &stPageInfo, 0,sizeof(STPageInfo) );

	cgiFormStringNoNewlines("UN", stPageInfo.encry, BUF_LEN);
	
	stPageInfo.username_encry=dcryption(stPageInfo.encry);
    if( NULL == stPageInfo.username_encry )
    {
	    ShowErrorPage(search(stPageInfo.lpublic,"ill_user")); 	  /*用户非法*/
		return 0;
	}
	stPageInfo.iUserGroup = checkuser_group( stPageInfo.username_encry );

	//stPageInfo.pstModuleContainer = MC_create_module_container();
	init_portal_container(&(stPageInfo.pstPortalContainer));
	if( NULL == stPageInfo.pstPortalContainer )
	{
		return 0;
	}
	stPageInfo.lpublic=stPageInfo.pstPortalContainer->lpublic;//get_chain_head("../htdocs/text/public.txt");
	stPageInfo.lauth=stPageInfo.pstPortalContainer->llocal;//get_chain_head("../htdocs/text/authentication.txt");
	
	stPageInfo.fp = cgiOut;
//初始化完毕

//处理表单
	stPageInfo.formProcessError = getUserInput( &(stPageInfo.stUserInput) );
	//get_black_list( stPageInfo.stUserInput.id, &(stPageInfo.st_black_list) );
    if( cgiFormSubmitClicked(SUBMIT_NAME) == cgiFormSuccess || stPageInfo.stUserInput.ipaddr_del.ipaddr_begin[0] > '\0' )
    {
    	debug_printf( stderr, "stPageInfo.formProcessError = %d\n",stPageInfo.formProcessError );
    	if( 0 == stPageInfo.formProcessError )
    	{
    		//ShowAlert( search( lpublic, "ip_not_null" ) );
   			stPageInfo.formProcessError = doUserCommand( &(stPageInfo) );
   			doRedir( &stPageInfo );
    	}
    }
	
	
	//当前页面是static arp的页面，所以设置活动label为2
	MC_setActiveLabel( stPageInfo.pstPortalContainer->pstModuleContainer, 6 );

	MC_setPrefixOfPageCallBack( stPageInfo.pstPortalContainer->pstModuleContainer, (MC_CALLBACK)s_black_prefix_of_page, &stPageInfo );
	MC_setContentOfPageCallBack( stPageInfo.pstPortalContainer->pstModuleContainer, (MC_CALLBACK)s_black_content_of_page, &stPageInfo );

	
	MC_setOutPutFileHandle( stPageInfo.pstPortalContainer->pstModuleContainer, cgiOut );

	MC_setModuleContainerDomainValue( stPageInfo.pstPortalContainer->pstModuleContainer, FORM_ONSIBMIT, "return true;" );
	//可以设置为一个javascript函数,这个js函数的实现放在prefix回调函数中就可以了。
	MC_setModuleContainerDomainValue( stPageInfo.pstPortalContainer->pstModuleContainer, FORM_METHOD, "post" );
	MC_setModuleContainerDomainValue( stPageInfo.pstPortalContainer->pstModuleContainer, FORM_ACTION, "wp_black_list.cgi" );
	MC_setModuleContainerDomainValue( stPageInfo.pstPortalContainer->pstModuleContainer, PUBLIC_INPUT_ENCRY, stPageInfo.encry );
	
	
	MC_setModuleContainerDomainValue( stPageInfo.pstPortalContainer->pstModuleContainer, BTN_OK_IMG, search(stPageInfo.lpublic,"img_ok") );
	MC_setModuleContainerDomainValue( stPageInfo.pstPortalContainer->pstModuleContainer, BTN_OK_SUBMIT_NAME, SUBMIT_NAME );

	
	MC_setModuleContainerDomainValue( stPageInfo.pstPortalContainer->pstModuleContainer, LABLE_TOP_HIGHT, "25" );

	
	MC_writeHtml( stPageInfo.pstPortalContainer->pstModuleContainer );
	release(stPageInfo.lauth);
	release(stPageInfo.lpublic);
	
	release_portal_container(&(stPageInfo.pstPortalContainer));
	
	
	return 0;
}


static int s_black_prefix_of_page( STPageInfo *pstPageInfo )
{
   char *error_message=NULL;

	if( NULL == pstPageInfo )
	{
		return -1;
	}
		
 	switch( pstPageInfo->formProcessError )
 	{
		case 0:
			break;
 	    case ERR_IPADDR_FORMAT:
 	    case ERR_IP_ADDR:
 	    	error_message = search( pstPageInfo->lpublic, "err_ip_format" );
 	    	break;
 	    case ERR_PORTAL_ID_NOUSE:
 	    	error_message = search( pstPageInfo->lauth, "portal_id_nouse" );
 	    	break;
 	    case ERR_PORT_FORMAT:
 	    	error_message = search( pstPageInfo->lauth, "blacklist_port_err" );
 	    	break;
 	    case ERR_IP_HAS_IN_LIST:
 	    	error_message = search( pstPageInfo->lauth, "blacklist_ip_inlist" );
 	    	break;
 	    case ERR_OUT_OF_RANGE:
 	    	error_message = search( pstPageInfo->lauth, "blacklist_script_err" );
 	    	//error_message = "blacklist_add_err1" ;
 	    	break;	
		case ERR_BLACK_LIST_MAX:
			error_message = search( pstPageInfo->lauth, "black_list_max" );
			break;
 	    case ERR_IP_NOT_IN_RANGE:
 	    case ERR_PORTAL_ID:	
 	    case ERR_DO_COMMAND:    
 	    case ERR_NOT_LEGAL_ID:
 	    case ERR_DEFAULT:
 	    default:
 	    	error_message = search( pstPageInfo->lauth, "blacklist_add_err" );
 	    	//error_message = "blacklist_add_err2" ;
 	    	break;
	}	
	
 	if( NULL != error_message )
 	{
 	    ShowAlert( error_message );
 	}
 	
 	fprintf( cgiOut, "<style type=text/css>.a3{width:30px;border:0px; text-align:center}</style>" );
 	fprintf( cgiOut, "<script language=javascript src=/ip.js></script>\n" );
 	fprintf( cgiOut, "<script language=javascript src=/fw.js></script>\n" );
	return 0;		
}

static int s_black_content_of_page( STPageInfo *pstPageInfo )
{
	int i;
	
	if( NULL == pstPageInfo )
	{
		return -1;
	}
	
	fprintf(cgiOut,"	<table>\n");
	fprintf(cgiOut,"		<tr>\n"\
					"			<td colspan='3'>\n"\
					"				<table>\n"\
					"					<tr>\n"\
					"						<td>ID:</td>\n"\
					"						<td>\n"\
					"						<select name='portal_id' style='width:100%%;height:auto' onchange='on_id_change(this);'>\n");
	for(i=0;i<MAX_ID_NUM;i++)
	{
		if( pstPageInfo->stUserInput.id[0] == i+'0' )
		{
			fprintf(cgiOut,"						<option value='%d' selected>%d</option>",i,i);
		}
		else
		{
			fprintf(cgiOut,"						<option value='%d'>%d</option>",i,i);
		}
	}
	fprintf(cgiOut,"						</select>\n" );
	fprintf( cgiOut,"			<script type=text/javascript>\n"\
					"			function on_id_change(obj)\n"
					"			{\n"\
					"				window.location.href='wp_black_list.cgi?UN=%s&portal_id='+obj.value;\n"\
					"			}\n",pstPageInfo->encry );
	fprintf( cgiOut, "			</script>\n" );
	fprintf(cgiOut,"						</td>\n"\
					"					</tr>\n");
	fprintf( cgiOut,"<tr>\n"\
					"<td>Mode:</td>\n"\
					"<td>\n"\
					"<select name='portal_id' style='width:100%%;height:auto' onchange='on_mode_change(this);'>\n"\
					"<option value=0 selected>Ip</option>"\
					"<option value=1 >Domain</option>"\
					"</select>\n");
	fprintf( cgiOut,"<script type=text/javascript>\n"\
					"function on_mode_change(obj)\n"
					"{\n"\
					"window.location.href='wp_black_list_domain.cgi?UN=%s&portal_id='+%d;\n",pstPageInfo->encry,pstPageInfo->stUserInput.id[0]-'0');
	fprintf( cgiOut,"}\n");
	fprintf( cgiOut, "</script>\n  </td>\n	</tr>\n"\
					"					<tr>\n"\
					"						<td>IP begin:</td>\n"\
					"						<td>\n" );
	fprintf( cgiOut, "<div style='border-width:1px;border-color:#a5acb2;border-style:solid;width:147px;font-size:9pt'>");
	fprintf( cgiOut, "<input type=text name='begin_ip1' maxlength=3 class=a3 onKeyUp=\"mask(this,%s)\" onbeforepaste=mask_c() />.",search(pstPageInfo->lpublic,"ip_error"));
	fprintf( cgiOut, "<input type=text name='begin_ip2' maxlength=3 class=a3 onKeyUp=\"mask(this,%s)\" onbeforepaste=mask_c() />.",search(pstPageInfo->lpublic,"ip_error"));
	fprintf( cgiOut, "<input type=text name='begin_ip3' maxlength=3 class=a3 onKeyUp=\"mask(this,%s)\" onbeforepaste=mask_c() />.",search(pstPageInfo->lpublic,"ip_error"));
	fprintf( cgiOut, "<input type=text name='begin_ip4' maxlength=3 class=a3 onKeyUp=\"mask(this,%s)\" onbeforepaste=mask_c() />",search(pstPageInfo->lpublic,"ip_error"));
	fprintf( cgiOut, "</div>\n" );		
					
	fprintf( cgiOut,"				</td>\n"\
					"						<td></td>\n"\
					"					</tr>\n" );
	//ip end
	fprintf( cgiOut,"					<tr>\n"\
					"						<td>IP end:</td>\n"\
					"						<td>\n" );
	fprintf( cgiOut, "<div style='border-width:1px;border-color:#a5acb2;border-style:solid;width:147px;font-size:9pt'>");
	fprintf( cgiOut, "<input type=text name='end_ip1' maxlength=3 class=a3 onKeyUp=\"mask(this,%s)\" onbeforepaste=mask_c() />.",search(pstPageInfo->lpublic,"ip_error"));
	fprintf( cgiOut, "<input type=text name='end_ip2' maxlength=3 class=a3 onKeyUp=\"mask(this,%s)\" onbeforepaste=mask_c() />.",search(pstPageInfo->lpublic,"ip_error"));
	fprintf( cgiOut, "<input type=text name='end_ip3' maxlength=3 class=a3 onKeyUp=\"mask(this,%s)\" onbeforepaste=mask_c() />.",search(pstPageInfo->lpublic,"ip_error"));
	fprintf( cgiOut, "<input type=text name='end_ip4' maxlength=3 class=a3 onKeyUp=\"mask(this,%s)\" onbeforepaste=mask_c() />",search(pstPageInfo->lpublic,"ip_error"));
	fprintf( cgiOut, "</div>\n" );		
	fprintf( cgiOut,"				</td>\n"\
					"						<td></td>\n"\
					"					</tr>\n" );	
	// port
	fprintf( cgiOut,"					<tr>\n"\
					"						<td>Port:</td>\n"\
					"						<td><input type=text name=port value='' /></td>\n"\
					"					</tr>\n" );
	
	
	fprintf( cgiOut,"				</table>\n"\
					"			</td>\n"\
				  	"		</tr>\n");
	fprintf(cgiOut,"		<tr>\n");
	fprintf(cgiOut,"			<td>\n"\
						"				<table frame=below rules=rows border=1 style='border-collapse:collapse;align:left;overflow:visible'>\n" );
#if 1
	fprintf( cgiOut, "					<tr height=30 bgcolor=#eaeff9 id=td1 align=left>\n");
	fprintf(cgiOut,"						<td width=150 style=font-size:12px><font id=%s>IP</font></th>\n",search(pstPageInfo->lpublic,"menu_thead"));
	fprintf(cgiOut,"						<td width=20></th>\n");
	fprintf(cgiOut,"					</tr>\n");
#endif	
	
	for( i=0; i<pstPageInfo->st_black_list.num; i++ )
	{
		fprintf(cgiOut,"					<tr height=25>\n");
		fprintf(cgiOut,"						<td style=font-size:12px align=left>%s%s%s:%s</td>\n",
													pstPageInfo->st_black_list.ipaddr_list[i].ipaddr_begin,
													pstPageInfo->st_black_list.ipaddr_list[i].ipaddr_end[0]?"-":"",
													pstPageInfo->st_black_list.ipaddr_list[i].ipaddr_end[0]?pstPageInfo->st_black_list.ipaddr_list[i].ipaddr_end:"",
													pstPageInfo->st_black_list.ipaddr_list[i].port );
		fprintf(cgiOut,"						<td>\n" );
		fprintf( cgiOut, "						<script type=text/javascript>\n" );
		fprintf( cgiOut, "							var popMenu%d = new popMenu('popMenu%d','%d');\n", i, i, pstPageInfo->st_black_list.num-i );
		fprintf( cgiOut, "							popMenu%d.addItem( new popMenuItem( '%s', 'wp_black_list.cgi?UN=%s&portal_id=%s&del_black_list=%s&ip_end=%s&port=%s' ) );\n",
							i,"delete", pstPageInfo->encry, pstPageInfo->st_black_list.id,pstPageInfo->st_black_list.ipaddr_list[i].ipaddr_begin,
										pstPageInfo->st_black_list.ipaddr_list[i].ipaddr_end, pstPageInfo->st_black_list.ipaddr_list[i].port ); 
		fprintf( cgiOut, "							popMenu%d.show();\n", i );
		fprintf( cgiOut, "						</script>\n" );
		fprintf( cgiOut, "						</td>\n");
		fprintf(cgiOut,"					</tr>\n");
	}
	fprintf(cgiOut,"				</table>"\
					"			</td>");
	fprintf(cgiOut,"			<td width='50'>&nbsp;</td>\n");

	fprintf( cgiOut, "<td></td>" );
				
	fprintf(cgiOut,"		</tr>\n"\
					"	</table>\n");

	return 0;	
}



static int getUserInput( STUserInput *pstUserInput )
{
	char begin_ip1[5],begin_ip2[5],begin_ip3[5],begin_ip4[5];
	char end_ip1[5],end_ip2[5],end_ip3[5],end_ip4[5];
	char port[MAX_PORT_LEN];
	int iRet=0;
	
	if( NULL == pstUserInput )
	{
		return ERR_DEFAULT;	
	}
	
	if( 0 == cgiFormStringNoNewlines("portal_id", pstUserInput->id, sizeof(pstUserInput->id)) )
	{
		if( strlen(pstUserInput->id) != 1 || pstUserInput->id[0] < '0' || pstUserInput->id[0] > '7' )
		{
			return ERR_PORTAL_ID;
		}
	}
	else
	{
		strcpy( pstUserInput->id, "0" );
	}
	
	// port 
	memset( port, 0, sizeof(port) );
	cgiFormStringNoNewlines( "port", port, sizeof(port) );
	if( strlen(port) > 0 && 0 != strcmp(port, "all") && ! is_legal_port(port) )
	{
		return ERR_PORT_FORMAT;
	}
	else if( strlen(port)==0 )
	{
		strcpy( port, "all" );
	}
	
	
	
	
//获得输入的ip地址,只有当点击提交的时候才获得这个ip地址
	if( cgiFormSubmitClicked(SUBMIT_NAME) == cgiFormSuccess  )
	{
		//ip地址 begin
		memset( begin_ip1, 0, sizeof(begin_ip1) );
		cgiFormStringNoNewlines( "begin_ip1", begin_ip1, sizeof(begin_ip1) );
		memset( begin_ip2, 0, sizeof(begin_ip2) );
		cgiFormStringNoNewlines( "begin_ip2", begin_ip2, sizeof(begin_ip2) );
		memset( begin_ip3, 0, sizeof(begin_ip3) );
		cgiFormStringNoNewlines( "begin_ip3", begin_ip3, sizeof(begin_ip3) );
		memset( begin_ip4, 0, sizeof(begin_ip4) );
		cgiFormStringNoNewlines( "begin_ip4", begin_ip4, sizeof(begin_ip4) );

	//fprintf( cgiOut, "begin_ip1 = %s  <br /> begin_ip1 = %s  <br />begin_ip1 = %s  <br />begin_ip1 = %s  <br />\n", begin_ip1, begin_ip2, begin_ip3, begin_ip4 );
		if( strlen(begin_ip1) == 0 || strlen(begin_ip2) == 0 || strlen(begin_ip3) == 0 || strlen(begin_ip4) == 0  )
		{
			return ERR_IP_ADDR;
		}	
		sprintf( pstUserInput->ipaddr_add.ipaddr_begin,"%s.%s.%s.%s", begin_ip1, begin_ip2, begin_ip3, begin_ip4 );

		//ip地址 end
		memset( end_ip1, 0, sizeof(end_ip1) );
		cgiFormStringNoNewlines( "end_ip1", end_ip1, sizeof(end_ip1) );
		memset( end_ip2, 0, sizeof(end_ip2) );
		cgiFormStringNoNewlines( "end_ip2", end_ip2, sizeof(end_ip2) );
		memset( end_ip3, 0, sizeof(end_ip3) );
		cgiFormStringNoNewlines( "end_ip3", end_ip3, sizeof(end_ip3) );
		memset( end_ip4, 0, sizeof(end_ip4) );
		cgiFormStringNoNewlines( "end_ip4", end_ip4, sizeof(end_ip4) );

	//fprintf( cgiOut, "end_ip1 = %s  <br /> end_ip1 = %s  <br />end_ip1 = %s  <br />end_ip1 = %s  <br />\n", end_ip1, end_ip2, end_ip3, end_ip4 );
		if( strlen(end_ip1) == 0 && strlen(end_ip2) == 0 && strlen(end_ip3) == 0 && strlen(end_ip4) == 0  )
		{
			memset( pstUserInput->ipaddr_add.ipaddr_end, 0, sizeof(pstUserInput->ipaddr_add.ipaddr_end) );
		}	
		else if( strlen(end_ip1) == 0 || strlen(end_ip2) == 0 || strlen(end_ip3) == 0 || strlen(end_ip4) == 0 )
		{
			return ERR_IP_ADDR;
		}
		else
		{
			sprintf( pstUserInput->ipaddr_add.ipaddr_end,"%s.%s.%s.%s", end_ip1, end_ip2, end_ip3, end_ip4 );	
		}
		
		strncpy( pstUserInput->ipaddr_add.port, port, sizeof(pstUserInput->ipaddr_add.port) );
	}
	else
	{
//如果是删除，url上代了id，以及 del_black_list，如果没有得到这个参数，表示不是删除
		cgiFormStringNoNewlines("del_black_list", pstUserInput->ipaddr_del.ipaddr_begin, sizeof(pstUserInput->ipaddr_del.ipaddr_begin));
		cgiFormStringNoNewlines("ip_end", pstUserInput->ipaddr_del.ipaddr_end, sizeof(pstUserInput->ipaddr_del.ipaddr_end));
		strncpy( pstUserInput->ipaddr_del.port, port, sizeof(pstUserInput->ipaddr_del.port) );
		
		if( strlen(pstUserInput->ipaddr_del.ipaddr_begin) > 0 && 1 != is_legal_ip(pstUserInput->ipaddr_del.ipaddr_begin) )
		{
			iRet = 	ERR_IPADDR_FORMAT;
		}
		
		if( strlen(pstUserInput->ipaddr_del.ipaddr_end) > 0 && 1 != is_legal_ip(pstUserInput->ipaddr_del.ipaddr_end) )
		{
			iRet = 	ERR_IPADDR_FORMAT;
		}
	}
	
//	debug_printf(stderr,"id=%s :: ipaddr_add=%s :: ipaddr_del=%s\n", pstUserInput->id, pstUserInput->ipaddr_add, pstUserInput->ipaddr_del );
	
	return iRet;
}


static int doUserCommand( STPageInfo *pstPageInfo )
{
	int iRet=0;
	//添加
	if( cgiFormSubmitClicked(SUBMIT_NAME) == cgiFormSuccess  )
	{
		debug_printf( stderr, "doUserCommand  pstPageInfo->stUserInput.ipaddr_add.port = %s\n", pstPageInfo->stUserInput.ipaddr_add.port );
		iRet=add_black_list( &(pstPageInfo->st_black_list), 
							 pstPageInfo->stUserInput.ipaddr_add.ipaddr_begin,
							 pstPageInfo->stUserInput.ipaddr_add.ipaddr_end,
							 pstPageInfo->stUserInput.ipaddr_add.port );
	}
	else if( strlen(pstPageInfo->stUserInput.ipaddr_del.ipaddr_begin) > 0 )
	{
	//删除，将页面重新定位一次，去掉其中的del_black_list属性，否则点击刷新的时候还会再删除一次。
		iRet=del_black_list( &(pstPageInfo->st_black_list), 
							 pstPageInfo->stUserInput.ipaddr_del.ipaddr_begin,
							 pstPageInfo->stUserInput.ipaddr_del.ipaddr_end,
							 pstPageInfo->stUserInput.ipaddr_del.port );		
	}
	
	return iRet;
}

static int doRedir( STPageInfo *pstPageInfo  )
{

	cgiHeaderContentType("text/html");
	
	s_black_prefix_of_page(pstPageInfo);
	
	fprintf(cgiOut, "<script type=text/javascript>\n	window.location.href='wp_black_list.cgi?UN=%s&portal_id=%s';\n</script>", 
					pstPageInfo->encry, pstPageInfo->stUserInput.id );
	
	exit(0);
	
}


