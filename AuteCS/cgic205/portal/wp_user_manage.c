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
* wp_user_manage.c
*
*
* CREATOR:
* autelan.software.Network Dep. team
* tangsq@autelan.com
*
* DESCRIPTION: 
*
*
*******************************************************************************/
#include <stdio.h>
#include <stdlib.h>
#include <sys/wait.h>

#include<sys/socket.h>
#include<netinet/in.h>
#include<arpa/inet.h>

#include "ws_module_container.h"
#include "ws_portal_container.h"
#include "cgic.h"
#include "ws_err.h"
#include "ws_usrinfo.h"
#include "ws_ec.h"
#include "ws_conf_engine.h"
#include "ws_user_manage.h"

/***************************************************************
定义页面要用到的结构体
****************************************************************/
//#define MAX_URL_LEN         256

char *search_type[] = {
	"user_index",
	"user_name",
	"ip_addr",
	"mac_addr"
};


typedef struct{
//	STModuleContainer *pstModuleContainer;
	STPortalContainer *pstPortalContainer;
	struct list *lpublic;/*解析public.txt文件的链表头*/
	struct list *lauth;
	char encry[BUF_LEN];
	char *username_encry;	         /*存储解密后的当前登陆用户名*/
	int iUserGroup;	//为0时表示是管理员。
	FILE *fp;
		
	int formProcessError;
	
	STUserManagePkg *pstReq;
	STUserManagePkg *pstRsp;	
} STPageInfo;

//STOnlineUserInfo search_user;


#define SUBMIT_NAME_USER_MANAGE	"user_mng_submit"
#define DATA_NUM_EVERYPAGE 10


char  PNtemp[10];  		//transfer PageNum
char  search_content[30];	
int FirstPage, LastPage, pageNum, totalNum,search_point;
int idx_begin;
int idx_end ;
int typeChoice = 0;
int search_switch = 0 ; // 1 为打开查询
//struct list * portal_lcon;


static int s_usr_mng_prefix_of_page( STPageInfo *pstPageInfo );
static int s_usr_mng_content_of_page( STPageInfo *pstPageInfo );
static int doUserCommand( STPageInfo *pstPageInfo );
static int doRedir( STPageInfo *pstPageInfo  );
static int  search_user_info(int type, int max_data, char * content, int * select, STPageInfo *pstPageInfo );
static int USER_WEB_Pagination_get_range(int tatol_num, int * pageNumForREQ, int perPage_show_data_num, int * showHead, int *showTail);

int cgiMain()
{
	STPageInfo stPageInfo;
	//portal_lcon = get_chain_head("../../htdocs/text/control.txt");
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
//	stPageInfo.formProcessError = getUserInput( &(stPageInfo.stUserInput) );
    if( cgiFormSubmitClicked(SUBMIT_NAME_USER_MANAGE) == cgiFormSuccess  )
    {
    	if( 0 == stPageInfo.formProcessError )
    	{
    		//ShowAlert( search( lpublic, "ip_not_null" ) );
   			stPageInfo.formProcessError = doUserCommand( &(stPageInfo) );
   			doRedir( &stPageInfo );
    	}
    }
	
	char  url[256];
	memset( url, 0, 256 );

	MC_setActiveLabel( stPageInfo.pstPortalContainer->pstModuleContainer, 3 );

	MC_setPrefixOfPageCallBack( stPageInfo.pstPortalContainer->pstModuleContainer, (MC_CALLBACK)s_usr_mng_prefix_of_page, &stPageInfo );
	MC_setContentOfPageCallBack( stPageInfo.pstPortalContainer->pstModuleContainer, (MC_CALLBACK)s_usr_mng_content_of_page, &stPageInfo );

	
	MC_setOutPutFileHandle( stPageInfo.pstPortalContainer->pstModuleContainer, cgiOut );

	MC_setModuleContainerDomainValue( stPageInfo.pstPortalContainer->pstModuleContainer, FORM_ONSIBMIT, "return true;" );
	//可以设置为一个javascript函数,这个js函数的实现放在prefix回调函数中就可以了。
	MC_setModuleContainerDomainValue( stPageInfo.pstPortalContainer->pstModuleContainer, FORM_METHOD, "post" );
	MC_setModuleContainerDomainValue( stPageInfo.pstPortalContainer->pstModuleContainer, FORM_ACTION, "wp_user_manage.cgi" );
	MC_setModuleContainerDomainValue( stPageInfo.pstPortalContainer->pstModuleContainer, PUBLIC_INPUT_ENCRY, stPageInfo.encry );
	
	
	MC_setModuleContainerDomainValue( stPageInfo.pstPortalContainer->pstModuleContainer, BTN_OK_IMG, search(stPageInfo.lpublic,"img_ok") );
	MC_setModuleContainerDomainValue( stPageInfo.pstPortalContainer->pstModuleContainer, BTN_OK_SUBMIT_NAME, SUBMIT_NAME_USER_MANAGE );

	
	MC_setModuleContainerDomainValue( stPageInfo.pstPortalContainer->pstModuleContainer, LABLE_TOP_HIGHT, "25" );
	MC_setModuleContainerDomainValue( stPageInfo.pstPortalContainer->pstModuleContainer, BTN_OK_SUBMIT_NAME, "submit_user_manage" );
	
	snprintf( url, sizeof(url), "wp_authentication.cgi.cgi?UN=%s", stPageInfo.encry );
	MC_setModuleContainerDomainValue( stPageInfo.pstPortalContainer->pstModuleContainer, BTN_CANCEL_URL, url );
	
	MC_writeHtml( stPageInfo.pstPortalContainer->pstModuleContainer );
	
	release_portal_container(&(stPageInfo.pstPortalContainer));
	
	
	return 0;
}


static int s_usr_mng_prefix_of_page( STPageInfo *pstPageInfo )
{

	FILE *fp = pstPageInfo->fp;
	fprintf(fp, "<style type=text/css>"\
  				"#div1{ width:62px; height:18px; border:1px solid #666666; background-color:#f9f8f7;}"\
    			"#div2{ width:60px; height:15px; padding-left:5px; padding-top:3px}"\
   				"#link{ text-decoration:none; font-size: 12px}"\
   				"</style>");
	fprintf(fp,	"<script type=\"text/javascript\">"\
				"function popMenu(objId)"\
				"{"\
			   		"var obj = document.getElementById(objId);"\
			   		"if (obj.style.display == 'none')"\
			   		"{"\
				 		"obj.style.display = 'block';"\
			   		"}"\
			   		"else"\
			   		"{"\
				 		"obj.style.display = 'none';"\
			   		"}"\
		  	 	"}"\
		   		"</script>");
	struct list * portal_lcon 		= pstPageInfo->pstPortalContainer->lcon;
	struct list * portal_lauth = pstPageInfo->lauth;
		
	char all_data_num[10];
	memset( all_data_num, 0, 10 );
	
	char downflag[20];
	memset( downflag, 0, 20 );
	char c_userip[20];
	memset( c_userip, 0, 20 );
	int i_userip = 0;
	char *endptr;

	//memset( &search_user, 0, sizeof(STOnlineUserInfo) );
	//memset(total_num,0,10);
	
	////////全局初始化////////////
	memset(PNtemp,0,10);
	FirstPage = 1;//首页为第一页
	LastPage = 0;
	pageNum = 1; //初始进第一页
	totalNum = 0;
	search_switch = 0;
	search_point = 0;
	
	cgiFormStringNoNewlines( "TOTAL", all_data_num, 10 );
	//fprintf( stderr, "all_data_num=%s\n" ,all_data_num );
	if( strcmp(all_data_num,"") != 0)
	{
    	totalNum=atoi(all_data_num);
	}
	
    cgiFormStringNoNewlines( "PN", PNtemp, 10 );
	if( strcmp(PNtemp,"") != 0)
	{
    	pageNum=atoi(PNtemp);
	}
	
	//fprintf( stderr, "pageNum=%d--totalNumPage=%d\n" ,pageNum, totalNum );
	//////////强制下线///////////////////////
	cgiFormStringNoNewlines( "DOWNRULE", downflag, 20 );
	if( !strcmp(downflag, "USERDOWN") )
	{
		cgiFormStringNoNewlines( "USERIP", c_userip, 20 );
		i_userip = strtoul(c_userip, &endptr, 0);
		fprintf( stderr, "i_userip=%d", i_userip );

		pstPageInfo->pstReq = createRequirePkg( REQ_LOGOUT_BY_INFO, &i_userip, NULL );
		if( NULL == pstPageInfo->pstReq )
		{
			fprintf( stderr, "create failed !!");
			return 0;
		}
		int down_ret = doRequire( pstPageInfo->pstReq, ntohl(inet_addr("127.0.0.1")), 2001, 5, &(pstPageInfo->pstRsp) );
		if( NULL == pstPageInfo->pstRsp )
		{
			fprintf( stderr, "get respones failed!\n" );
		}
		if( pstPageInfo->pstRsp->data.ack.LOGOUT_FLAG == 1 )
		{
			ShowAlert(search(portal_lcon, "Operation_Success"));
		}
	}
	
	
	if( cgiFormSubmitClicked("submit_user_manage") == cgiFormSuccess )
	{
		fprintf( fp,	"<script type='text/javascript'>");
		fprintf( fp,	"window.location.href = 'wp_authentication.cgi?UN=%s';\n", pstPageInfo->encry);
		fprintf( fp,	"</script>");
	}

	idx_begin = 0;
	idx_end = 0;

	int page_ret = 0;

	page_ret = USER_WEB_Pagination_get_range( totalNum, &pageNum, DATA_NUM_EVERYPAGE, &idx_begin, &idx_end );
	//fprintf( stderr, "idx_begin=%d--idx_end=%d--page_ret=%d--totalNum=%d\n" ,idx_begin,idx_end, page_ret,totalNum );
	if( -4 == page_ret )
	{
		pageNum++;
		ShowAlert(search(portal_lcon, "Page_Begin"));
	}
	if( -3 == page_ret )
	{
		pageNum--;
		ShowAlert(search(portal_lcon, "Page_end"));
	}
	
	
	pstPageInfo->pstReq = createRequirePkg( REQ_GET_USR_BY_INDEX_RANG, &idx_begin, &idx_end );
	if( NULL == pstPageInfo->pstReq )
	{
		fprintf( stderr, "create failed !!");
		return 0;
	}
	fprintf(stderr,"idx_begin=%d--idx_end=%d",idx_begin,idx_end);
	fprintf( stderr, "inet_addr(\"127.0.0.1\") = %d\n", ntohl(inet_addr("127.0.0.1")) );
	int do_ret = doRequire( pstPageInfo->pstReq, ntohl(inet_addr("127.0.0.1")), 2001, 5, &(pstPageInfo->pstRsp) );
	//fprintf( stderr, "do_ret=%d\n",do_ret );
	if( do_ret == -1 )
		{
			idx_begin = 0;
			idx_end = 0;
			return 0;
		}
	
	
	//计算出最后一页的页数
	totalNum = pstPageInfo->pstRsp->all_user_num;
   if( (totalNum%DATA_NUM_EVERYPAGE) == 0 || totalNum == 0 )
       LastPage = (totalNum/DATA_NUM_EVERYPAGE); //计算出最大页数
   else	
       LastPage = (totalNum/DATA_NUM_EVERYPAGE)+1; //计算出最大页数
	
	#if 1
	fprintf( stderr, "all_user_num=%d",pstPageInfo->pstRsp->all_user_num);
	fprintf( stderr, "len=%d",pstPageInfo->pstRsp->len);
	fprintf( stderr, "user_num_in_pkg=%d",pstPageInfo->pstRsp->data.ack.user_num_in_pkg);
	fprintf( stderr, "users[0].index=%d",pstPageInfo->pstRsp->data.ack.users[0].index);
	fprintf( stderr, "users[0].mac=%2.2X:%2.2X:%2.2X:%2.2X:%2.2X:%2.2X",pstPageInfo->pstRsp->data.ack.users[0].usermac[0],pstPageInfo->pstRsp->data.ack.users[0].usermac[1],pstPageInfo->pstRsp->data.ack.users[0].usermac[2],pstPageInfo->pstRsp->data.ack.users[0].usermac[3],pstPageInfo->pstRsp->data.ack.users[0].usermac[4],pstPageInfo->pstRsp->data.ack.users[0].usermac[5]);
	fprintf( stderr, "users[0].username=%s",pstPageInfo->pstRsp->data.ack.users[0].username);
	fprintf( stderr, "users[0].ipaddr=%x",pstPageInfo->pstRsp->data.ack.users[0].ipaddr);
	fprintf( stderr, "users[0].input_octets=%lld",pstPageInfo->pstRsp->data.ack.users[0].input_octets);
	fprintf( stderr, "users[0].output_octets=%lld",pstPageInfo->pstRsp->data.ack.users[0].output_octets);
	#endif
	
	if( NULL == pstPageInfo->pstRsp )
	{
		fprintf( stderr, "get respones failed!\n" );
	}

	#if 1
		if(pstPageInfo->pstRsp->data.ack.user_num_in_pkg == 0 && pageNum > 1 )
		{
			ShowAlert(search(portal_lcon, "Page_end"));
			pageNum -- ;
		}
		if( pstPageInfo->pstRsp->data.ack.user_num_in_pkg < DATA_NUM_EVERYPAGE	)
		{
			idx_end = idx_begin + pstPageInfo->pstRsp->data.ack.user_num_in_pkg - 1;
		}
		//fprintf(stderr,"LA idx_end=%d",idx_end);
	#endif


	
	memset( search_content, 0, 30 );
	if( cgiFormSubmitClicked("search_but") == cgiFormSuccess )
	{
		int check_ret = 0;
		typeChoice = 0;
		cgiFormSelectSingle("search_type", search_type, 4, &typeChoice, 0);

		cgiFormStringNoNewlines("search_content", search_content, 30);
		//fprintf(stderr,"typeChoice=%d--search_content=%s",typeChoice,search_content);
		if( !strcmp(search_content, "") )
		{
			ShowAlert( search(portal_lauth, "search_not_null") );
			fprintf(stderr, "check_ret = %d",check_ret);
			return 0;
		}
		if( typeChoice == 2 )
		{
			check_ret = Input_IP_address_Check(search_content);
		}
		else if( typeChoice == 3 )
		{
			check_ret = Input_MAC_address_Check(search_content);
		}
		else if( typeChoice == 1 )
		{
			check_ret = Input_User_Name_Check(search_content);
		}
		else if( typeChoice == 0 )
		{
			check_ret = Input_User_Index_Check(search_content);
		}
		else
		{
			ShowAlert("Not pass search type!\n");
		}

		if( check_ret != 0 )
		{
			ShowAlert( search(portal_lauth, "input_char_error") );
			fprintf(stderr, "check_ret = %d",check_ret);
			return 0;
		}
		memset(search_content, 0, 30);
		cgiFormStringNoNewlines("search_content", search_content, 30);
		////////start search///////////////////////
		fprintf(stderr, "start enter searching---search_content =%s !\n",search_content);
		search_user_info( typeChoice, totalNum, search_content, &search_point , pstPageInfo);
		//fprintf(stderr, "search_point = %d---search_switch=%d",search_point,search_switch);

		
	}
	if( cgiFormSubmitClicked("ret_but") == cgiFormSuccess )
	{
		cgiFormStringNoNewlines("Pnum", PNtemp , 10);
		if( strcmp(PNtemp,"") != 0)
		{
	    	pageNum=atoi(PNtemp);
		}
		USER_WEB_Pagination_get_range( totalNum, &pageNum, DATA_NUM_EVERYPAGE, &idx_begin, &idx_end );
		//////////////数据修正///////////////////
		if(pstPageInfo->pstRsp->data.ack.user_num_in_pkg == 0 && pageNum > 1 )
		{
			ShowAlert(search(portal_lcon, "Page_end"));
			pageNum -- ;
		}
		if( pstPageInfo->pstRsp->data.ack.user_num_in_pkg < DATA_NUM_EVERYPAGE	)
		{
			idx_end = idx_begin + pstPageInfo->pstRsp->data.ack.user_num_in_pkg ;
		}
	}



	return 0;	
}

static int s_usr_mng_content_of_page( STPageInfo *pstPageInfo )
{
	int i;
	FILE *fp = pstPageInfo->fp;
	struct list * portal_lpublic	= pstPageInfo->lpublic;
	struct list * portal_lauth 		= pstPageInfo->lauth;
	struct list * portal_lcon 		= pstPageInfo->pstPortalContainer->lcon;
	char menu[21];
	char i_char[10];
	memset(i_char,0,10);
	
	//struct list * portal_lcon 		= get_chain_head("../../htdocs/text/control.txt");

	unsigned char  macChar[20];
	memset( macChar, 0, 20 );
	//char  ser_t[20];
	//memset( ser_t, 0, 20 );
	
	fprintf(fp,	"<table border=0 cellspacing=0 cellpadding=0>"\
				"<tr>"\
  				"<td id=sec1 colspan=4 style=\"border-bottom:2px solid #53868b;font-size:14px;padding-left:0px;padding-top:0px\">%s</td>",search(portal_lauth,"auth_user_info"));
  	fprintf(fp, "</tr>");
	
	fprintf(fp,	"<tr style=padding-top:5px><td>");
	fprintf(fp,	"<table border=0 cellspacing=0 cellpadding=0>"\
				"<tr align=left>"\
				"<td><select name=\"search_type\">");
	//cgiFormStringNoNewlines("search_type", ser_t, 20);
	
	if( typeChoice == 0 )
	{
		fprintf(fp, "<option value=user_index selected=selected>%s",search(portal_lauth,"user_index"));
		fprintf(fp,	"<option value=user_name>%s",search(portal_lauth,"user_name"));
		fprintf(fp,	"<option value=ip_addr>IP %s",search(portal_lpublic,"addr"));
		fprintf(fp,	"<option value=mac_addr>MAC %s",search(portal_lpublic,"addr"));
	}
	else if( typeChoice == 1 )
	{
		fprintf(fp, "<option value=user_index>%s",search(portal_lauth,"user_index"));
		fprintf(fp,	"<option value=user_name selected=selected>%s",search(portal_lauth,"user_name"));
		fprintf(fp,	"<option value=ip_addr>IP %s",search(portal_lpublic,"addr"));
		fprintf(fp,	"<option value=mac_addr>MAC %s",search(portal_lpublic,"addr"));
	}
	else if( typeChoice == 2 )
	{
		fprintf(fp, "<option value=user_index>%s",search(portal_lauth,"user_index"));
		fprintf(fp,	"<option value=user_name>%s",search(portal_lauth,"user_name"));
		fprintf(fp,	"<option value=ip_addr selected=selected>IP %s",search(portal_lpublic,"addr"));
		fprintf(fp,	"<option value=mac_addr>MAC %s",search(portal_lpublic,"addr"));
	}
	else if( typeChoice == 3 )
	{
		fprintf(fp, "<option value=user_index>%s",search(portal_lauth,"user_index"));
		fprintf(fp,	"<option value=user_name>%s",search(portal_lauth,"user_name"));
		fprintf(fp,	"<option value=ip_addr>IP %s",search(portal_lpublic,"addr"));
		fprintf(fp,	"<option value=mac_addr selected=selected>MAC %s",search(portal_lpublic,"addr"));
	}

	
	fprintf(fp,	"</select></td>"\

				"<td style=padding-left:2px><input type=text name=search_content size=22 value=%s ></td>",search_content);
	fprintf(fp,	"<td style=padding-left:5px><input type=submit name=search_but style=background-image:url(/images/SubBackGif.gif) value=%s></td>",search(portal_lpublic,"search"));
	fprintf(fp,	"<td style=padding-left:2px><input type=submit name=ret_but style=background-image:url(/images/SubBackGif.gif)  value=%s></td>",search(portal_lpublic,"return"));
	fprintf(fp,	"</tr>"\
				"</table>");
	fprintf(fp,	"</td></tr>");

	fprintf(fp,	"<tr style=padding-top:15px><td>");
	if(strcmp(search(portal_lpublic,"addr"),"Address")==0)/*英文*/
		fprintf(fp, "<table border=0 width=723 cellspacing=0 cellpadding=0>");
	else
		fprintf(fp, "<table border=0 width=683 cellspacing=0 cellpadding=0>");
	
	fprintf(fp, "<tr align=left>"\
	"<th width=100>%s</th>",search(portal_lauth,"user_index"));
	if(strcmp(search(portal_lpublic,"addr"),"Address")==0)/*英文*/
		fprintf(fp, "<th width=90><font id=%s>%s</font></th>",search(portal_lpublic,"menu_thead"),search(portal_lauth,"user_name"));
	else
		fprintf(fp, "<th width=70><font id=%s>%s</font></th>",search(portal_lpublic,"menu_thead"),search(portal_lauth,"user_name"));
	fprintf(fp, "<th width=120><font id=yingwen_thead>IP </font><font id=%s>%s</font></th>",search(portal_lpublic,"menu_thead"),search(portal_lpublic,"addr"));
	fprintf(fp, "<th width=130><font id=yingwen_thead>MAC </font><font id=%s>%s</font></th>",search(portal_lpublic,"menu_thead"),search(portal_lpublic,"addr"));

	fprintf(fp, "<th width=100><font id=%s>%s</font></th>",search(portal_lpublic,"menu_thead"),search(portal_lauth,"on_time"));
	fprintf(fp, "<th width=80><font id=%s>%s(KBytes)</font></th>",search(portal_lpublic,"menu_thead"),search(portal_lauth,"up_octets"));
	if(strcmp(search(portal_lpublic,"addr"),"Address")==0)/*英文*/
		fprintf(fp, "<th width=100><font id=%s>%s(KBytes)</font></th>",search(portal_lpublic,"menu_thead"),search(portal_lauth,"down_octets"));
	else
		fprintf(fp, "<th width=80><font id=%s>%s(KBytes)</font></th>",search(portal_lpublic,"menu_thead"),search(portal_lauth,"down_octets"));
	fprintf(fp, "<th width=13>&nbsp;</th>");
	fprintf(fp,	"</tr>");
	struct in_addr addrtemp;
	fprintf(stderr,"idx_begin=%d--idx_end=%d--search_switch=%d---search_point=%d", idx_begin, idx_end,search_switch,search_point);
	if(!search_switch)
	{
		fprintf(stderr,"start enter show!");
		int begin,end;
		begin=0;
		if((pageNum==1)&&(totalNum<=DATA_NUM_EVERYPAGE))
			end=totalNum-1;
		else
			end=idx_end-idx_begin;

		for( i = begin; i <= end; i++ )
		{
			//fprintf(stderr,"enter show!");
			memset(menu,0,21);
			strcpy(menu,"menulist");
			sprintf(i_char,"%d",i+1);
			strcat(menu,i_char);
			
			memset(&addrtemp, 0 ,sizeof(struct in_addr));
			addrtemp.s_addr = pstPageInfo->pstRsp->data.ack.users[i].ipaddr;
			memset( macChar, 0, 20 );
			sprintf( macChar, "%2.2X:%2.2X:%2.2X:%2.2X:%2.2X:%2.2X", pstPageInfo->pstRsp->data.ack.users[i].usermac[0],pstPageInfo->pstRsp->data.ack.users[i].usermac[1],pstPageInfo->pstRsp->data.ack.users[i].usermac[2],pstPageInfo->pstRsp->data.ack.users[i].usermac[3],pstPageInfo->pstRsp->data.ack.users[i].usermac[4],pstPageInfo->pstRsp->data.ack.users[i].usermac[5] );
			//fprintf(cgiOut,"macChar=%s",macChar);
			fprintf(fp,	"<tr align=left bgcolor=%s height=30>",setclour((i+1)%2) );
			fprintf(fp, "<td>%d</td>",pstPageInfo->pstRsp->data.ack.users[i].index);
			fprintf(fp, "<td>%s</td>",pstPageInfo->pstRsp->data.ack.users[i].username);
			fprintf(fp, "<td>%s</td>",inet_ntoa(addrtemp));
			fprintf(fp, "<td>%s</td>",macChar);
			
			int hour,min,sec;
	
			hour=pstPageInfo->pstRsp->data.ack.users[i].session_time/3600;
			min=(pstPageInfo->pstRsp->data.ack.users[i].session_time-hour*3600)/60;
			sec=(pstPageInfo->pstRsp->data.ack.users[i].session_time-hour*3600)%60;
			
			fprintf(fp, "<td>%d:%d:%d</td>",hour,min,sec);
			fprintf(fp, "<td>%lld</td>",(pstPageInfo->pstRsp->data.ack.users[i].output_octets/1024));
			fprintf(fp, "<td>%lld</td>",(pstPageInfo->pstRsp->data.ack.users[i].input_octets/1024));
			fprintf(fp,	"<td>");
           	fprintf(fp,	"<div style=\"position:relative; z-index:%d\" onmouseover=\"popMenu('%s');\" onmouseout=\"popMenu('%s');\">",(idx_end-idx_begin-i),menu,menu);
			fprintf(fp,	"<img src=/images/detail.gif>"\
						"<div id=%s style=\"display:none; position:absolute; top:5px; left:0;\">",menu);
			fprintf(fp,	"<div id=div1>");
			fprintf(fp,	"<div id=div2 onmouseover=\"this.style.backgroundColor='#b6bdd2'\" onmouseout=\"this.style.backgroundColor='#f9f8f7'\"><a id=link href=wp_user_manage.cgi?UN=%s&DOWNRULE=%s&USERIP=%d target=mainFrame>%s</a></div>", pstPageInfo->encry , "USERDOWN" , addrtemp.s_addr ,search(portal_lauth,"force_user_down"));
		 	fprintf(fp, "</div>"\
			  			"</div>"\
			   			"</div>");
		   	fprintf(fp,"</td>");
		   
			fprintf(fp,	"</tr>");
		}
	}
	else if( search_point != -1)	//显示查询结果
	{
			//fprintf(stderr, "start show searching !\n");
			memset(menu,0,21);
			strcpy(menu,"menulist");
			
			memset(&addrtemp, 0 ,sizeof(struct in_addr));
			addrtemp.s_addr = pstPageInfo->pstRsp->data.ack.users[search_point].ipaddr;
			memset( macChar, 0, 20 );
			sprintf( macChar, "%2.2X:%2.2X:%2.2X:%2.2X:%2.2X:%2.2X", pstPageInfo->pstRsp->data.ack.users[search_point].usermac[0],pstPageInfo->pstRsp->data.ack.users[search_point].usermac[1],pstPageInfo->pstRsp->data.ack.users[search_point].usermac[2],pstPageInfo->pstRsp->data.ack.users[search_point].usermac[3],pstPageInfo->pstRsp->data.ack.users[search_point].usermac[4],pstPageInfo->pstRsp->data.ack.users[search_point].usermac[5] );
			fprintf(stderr, "macChar=%s!\n",macChar);
			fprintf(fp,	"<tr align=left bgcolor=%s height=30>",setclour((search_point+1)%2) );
			fprintf(fp, "<td>%d</td>",pstPageInfo->pstRsp->data.ack.users[search_point].index);
			fprintf(fp, "<td>%s</td>",pstPageInfo->pstRsp->data.ack.users[search_point].username);
			fprintf(fp, "<td>%s</td>",inet_ntoa(addrtemp));
			fprintf(fp, "<td>%s</td>",macChar);
			//fprintf(stderr, "end show searching !\n");
			int hour,min,sec;
	
			hour=pstPageInfo->pstRsp->data.ack.users[search_point].session_time/3600;
			min=(pstPageInfo->pstRsp->data.ack.users[search_point].session_time-hour*3600)/60;
			sec=(pstPageInfo->pstRsp->data.ack.users[search_point].session_time-hour*3600)%60;
			
			fprintf(fp, "<td>%d:%d:%d</td>",hour,min,sec);
			fprintf(fp, "<td>%lld</td>",(pstPageInfo->pstRsp->data.ack.users[search_point].output_octets/1024));
			fprintf(fp, "<td>%lld</td>",(pstPageInfo->pstRsp->data.ack.users[search_point].input_octets/1024));
			
			fprintf(fp,	"<td>");
           	fprintf(fp,	"<div style=\"position:relative; z-index:%d\" onmouseover=\"popMenu('%s');\" onmouseout=\"popMenu('%s');\">", 0, menu,menu);
			fprintf(fp,	"<img src=/images/detail.gif>"\
						"<div id=%s style=\"display:none; position:absolute; top:5px; left:0;\">", menu);
			fprintf(fp,	"<div id=div1>");
			fprintf(fp,	"<div id=div2 onmouseover=\"this.style.backgroundColor='#b6bdd2'\" onmouseout=\"this.style.backgroundColor='#f9f8f7'\"><a id=link href=wp_user_manage.cgi?UN=%s&DOWNRULE=%s&USERIP=%d target=mainFrame>%s</a></div>", pstPageInfo->encry , "USERDOWN" , addrtemp.s_addr ,search(portal_lauth,"force_user_down"));
		 	fprintf(fp, "</div>"\
			  			"</div>"\
			   			"</div>");
		   	fprintf(fp,"</td>");
			
			fprintf(fp,	"</tr>");
	}
	else if( search_point == -1)
	{
			ShowAlert( search(portal_lauth, "canot_find") );
	}
	//fprintf(stderr,"end enter show!");
	fprintf(fp, "</table>");
	fprintf(fp, "</td></tr>");
	
	fprintf(fp, "<tr style=padding-top:30px><td>");
	fprintf(fp,	"<table border=0 cellspacing=0 cellpadding=0 width=430  height=50><tr valign=bottom height=30 align=center>");

	//sprintf(total_num,"%d",pstPageInfo->pstRsp->all_user_num);
	
	if( idx_end != 0 )//无数据不显示
	{
		fprintf(fp, "<td><a href=wp_user_manage.cgi?UN=%s&PN=%d&TOTAL=%u>%s</td>", pstPageInfo->encry,FirstPage, pstPageInfo->pstRsp->all_user_num, search(portal_lcon,"Page_First"));
	    fprintf(fp, "<td><a href=wp_user_manage.cgi?UN=%s&PN=%d&TOTAL=%u>%s</td>", pstPageInfo->encry,pageNum-1, pstPageInfo->pstRsp->all_user_num, search(portal_lcon,"page_up"));
	    fprintf(fp, "<td><a href=wp_user_manage.cgi?UN=%s&PN=%d&TOTAL=%u>%s</td>", pstPageInfo->encry,pageNum+1, pstPageInfo->pstRsp->all_user_num, search(portal_lcon,"page_down"));
	    fprintf(fp, "<td><a href=wp_user_manage.cgi?UN=%s&PN=%d&TOTAL=%u>%s</td>", pstPageInfo->encry,LastPage, pstPageInfo->pstRsp->all_user_num, search(portal_lcon,"Page_Last"));
	}
	fprintf(fp, "</tr>"\
				"<tr align=center valign=bottom>"\
    			"<td colspan=4>%s%d%s(%s%d%s)</td>",search(portal_lpublic,"current_sort"),pageNum,search(portal_lpublic,"page"),search(portal_lpublic,"total"),LastPage,search(portal_lpublic,"page"));
    fprintf(fp, "</tr>"\
	  			"</table>");
	//fprintf(fp, "<input type=hidden name=ser_type value=%d>", typeChoice );
	//fprintf(fp, "<input type=hidden name=ser_content value=%s>", search_content);
	fprintf(fp, "<input type=hidden name=Pnum value=%d>", pageNum );
	fprintf(fp,	"</td></tr></table>");


	//fprintf(stderr,"end return !");
	return 0;	
}

static int doUserCommand( STPageInfo *pstPageInfo )
{
	return 0;	
}

static int doRedir( STPageInfo *pstPageInfo  )
{
	return 0;	
}

static int USER_WEB_Pagination_get_range(int tatol_num, int * pageNumForREQ, int perPage_show_data_num, int * showHead, int *showTail)
{
	int start = 0;
	int end = 9;
	if( perPage_show_data_num == 0 )
	{
		return -1; //首页前翻
	}
	if( perPage_show_data_num < 0 || perPage_show_data_num < 1 )
	{
		return -2; //输入不合法
	}
	
	start = (*pageNumForREQ - 1)*perPage_show_data_num;
	end = (*pageNumForREQ)*perPage_show_data_num - 1;

	if( tatol_num == 0 )
	{
		*showHead = start;
		*showTail = end;
	}
	else
	{
		if( start >= tatol_num )
		{
			*showHead = start - perPage_show_data_num;   //修正数据
			if((end - perPage_show_data_num)>=tatol_num)
				*showTail = tatol_num - 1;
			else
				*showTail = end - perPage_show_data_num;	 //修正数据
			//*pageNumForREQ -- ;
			return -3; //请求数据数超过总数
		}
		if( start < 0 )
		{
			*showHead = start + perPage_show_data_num;	 //修正数据  
			*showTail = end + perPage_show_data_num;	 //修正数据
			//*pageNumForREQ ++ ;
			return -4; //请求数据数到最前
		}

		
		*showHead = start;
		if(end>=tatol_num)
			*showTail = tatol_num - 1;
		else
			*showTail = end;
	}
	return 0;
}
/*type   0--user index
		1--user name
		2--user ip
		3--user mac
*/
static int  search_user_info(int type, int max_data, char * content, int * select, STPageInfo *pstPageInfo )
{
	if( content == NULL || pstPageInfo == NULL )
	{
		return -1; //NULL
	}
	//STUserManagePkg * Search_User_Manage;
	//Search_User_Manage = ( STUserManagePkg * )malloc( sizeof(STUserManagePkg) );
	//memset( Search_User_Manage, 0, sizeof(STUserManagePkg) );
	
	int index = 0, i = 0, start = 0;
	int input_ip;
	char * endptr = NULL ;
	unsigned char macTemp[30];
	memset( macTemp, 0, 30 );

	unsigned char  mac_one[6];
	memset( mac_one, 0, 6 );
	unsigned int a1,a2,a3,a4,a5,a6;
	
	search_switch = 1; //开启查询标识
	if( type == 0 )
	{
		index = strtoul( content, &endptr, 0);
		fprintf(stderr, "index=%d", index);
	}
	else if( type == 2 )
	{
		input_ip = htonl( inet_addr(content) );
		fprintf(stderr, "input_ip = %d", input_ip);
	}
	else if( type == 3 )
	{
		fprintf(stderr, "content = %s", content);
		int num = sscanf( content, "%02X:%02X:%02X:%02X:%02X:%02X", &a1, &a2, &a3, &a4, &a5, &a6 );
		if( num == 6 && a1 <= 255 && a2 <= 255 && a3<=255 && a4<=255 && a5<=255 && a6<=255 )
			{
				mac_one[0] = a1;
				mac_one[1] = a2;
				mac_one[2] = a3;
				mac_one[3] = a4;
				mac_one[4] = a5;
				mac_one[5] = a6;
			}
		fprintf(stderr, "num= %d----mac_one0 = %d---mac_one1 = %d---mac_one2 = %d---mac_one3 = %d---mac_one4 = %d---mac_one5 = %d---", num, mac_one[0], mac_one[1], mac_one[2], mac_one[3], mac_one[4], mac_one[5] );
	}

	
	fprintf(stderr, "content=%s", content);

	switch(type)
	{
		case 0 : 
		{
			
			pstPageInfo->pstRsp = createRequirePkg( REQ_GET_BY_INDEX, &index, NULL );

			doRequire( pstPageInfo->pstRsp, ntohl(inet_addr("127.0.0.1")), 2001, 5, &(pstPageInfo->pstRsp) );
		
			if( index == pstPageInfo->pstRsp->data.ack.users[0].index && pstPageInfo->pstRsp->data.ack.user_num_in_pkg >= 1 )
			{
				*select = i;
				fprintf(stderr,"index has find!");
				return 0;
			}
			search_point = -1;//-1代表未找到

			
			break;
		}
		
		case 1 : 
		{
			pstPageInfo->pstRsp = createRequirePkg( REQ_GET_BY_NAME, content, NULL );

			doRequire( pstPageInfo->pstRsp, ntohl(inet_addr("127.0.0.1")), 2001, 5, &(pstPageInfo->pstRsp) );
			
			if( !strcmp(pstPageInfo->pstRsp->data.ack.users[0].username, content) )
			{
				*select = i;
				fprintf(stderr,"name has find!");
				return 0;
			}
			search_point = -1;//-1代表未找到
			
			break;
		}
		case 2 : 
		{
			pstPageInfo->pstRsp = createRequirePkg( REQ_GET_BY_IP, &input_ip, NULL );

			doRequire( pstPageInfo->pstRsp, ntohl(inet_addr("127.0.0.1")), 2001, 5, &(pstPageInfo->pstRsp) );
			if( input_ip == pstPageInfo->pstRsp->data.ack.users[0].ipaddr )
			{
				*select = i;
				fprintf(stderr,"ip has find!");
				return 0;
			}
			search_point = -1;//-1代表未找到
	
			break;
		}
		case 3 : 
		{
			pstPageInfo->pstRsp = createRequirePkg( REQ_GET_BY_MAC, mac_one, NULL );

			doRequire( pstPageInfo->pstRsp, ntohl(inet_addr("127.0.0.1")), 2001, 5, &(pstPageInfo->pstRsp) );
			
			sprintf( macTemp, "%2.2X:%2.2X:%2.2X:%2.2X:%2.2X:%2.2X", pstPageInfo->pstRsp->data.ack.users[0].usermac[0],pstPageInfo->pstRsp->data.ack.users[0].usermac[1],pstPageInfo->pstRsp->data.ack.users[0].usermac[2],pstPageInfo->pstRsp->data.ack.users[0].usermac[3],pstPageInfo->pstRsp->data.ack.users[0].usermac[4],pstPageInfo->pstRsp->data.ack.users[0].usermac[5] );
			fprintf(stderr, "macTemp=%s\n",macTemp);
			if( !strcmp(macTemp, content) )
			{
				*select = i;
				fprintf(stderr,"mac has find!");
				return 0;
			}
			search_point = -1;//-1代表未找到
			
			break;
		}
		default : break;
	}
	return 0;
}

