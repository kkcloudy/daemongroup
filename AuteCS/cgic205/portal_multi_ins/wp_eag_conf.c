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
* wp_eag_conf.c
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

#include <stdio.h>
#include <stdlib.h>
#include <sys/wait.h>
#include "ws_module_container.h"
#include "ws_portal_container.h"
#include "cgic.h"
#include "ws_err.h"
#include "ws_usrinfo.h"
#include "ws_ec.h"
#include "ws_conf_engine.h"

#include "ws_eag_conf.h"

#define _DEBUG	0

#if _DEBUG
#define debug_printf(a...) fprintf(a)
#else
#define debug_printf(a...)
#endif


/***************************************************************
定义页面要用到的结构体
****************************************************************/
//#define MAX_URL_LEN         256


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
} STPageInfo;

char *radiusType[] = {
	"general",
	"rj_sam",
};




/***************************************************************
*USEAGE:	输出一个ip地址的输入控件
*Param:		base_name -> 控件的基础名称，实际每个输入框在这个的基础上加_ip1,_ip2,_ip3,_ip4
*Return:	0 -> success
*			!= 0 -> failure。
*Auther:shao jun wu
*Date:2009-1-19 14:25:40
*Modify:(include modifyer,for what resease,date)
****************************************************************/
int output_ip_input( char *base_name, FILE *fp, char *err_msg, char *def_value, int flag )
{
	char ip1[5]="";
	char ip2[5]="";
	char ip3[5]="";
	char ip4[5]="";
	char *p;
	char *def_value_temp;
	
#define clear_all_ip	memset( ip1, 0, sizeof(ip1) );\
						memset( ip2, 0, sizeof(ip2) );\
						memset( ip3, 0, sizeof(ip3) );\
						memset( ip4, 0, sizeof(ip4) );\
						goto label;
	
	def_value_temp = strdup( def_value );
	
	p = strtok( def_value_temp, "." );
	if( NULL == p )
	{
		clear_all_ip;
	}else
	{
		strncpy( ip1, p, sizeof(ip1) );
	}
	
	p = strtok( NULL, "." );
	if( NULL == p )
	{
		clear_all_ip;
	}
	strncpy( ip2, p, sizeof(ip2) );

	p = strtok( NULL, "." );
	if( NULL == p )
	{
		clear_all_ip;
	}
	strncpy( ip3, p, sizeof(ip3) );
	
	p = strtok( NULL, "." );
	if( NULL == p )
	{
		clear_all_ip;
	}
	strncpy( ip4, p, sizeof(ip4) );	
	
	//sscanf( def_value, "%s.%s.%s.%s", ip1, ip2,ip3,ip4 );
	
label:		
	fprintf( fp, "<div style='border-width:1px;border-color:#a5acb2;border-style:solid;width:147px;font-size:9pt'>");
	fprintf( fp, "<input type=text name='%s_ip1' maxlength=3 class=a3 onKeyUp=\"mask(this,%s)\" onbeforepaste=mask_c() value='%s' %s/>.",base_name,err_msg, ip1, flag?"disabled":"");
	fprintf( fp, "<input type=text name='%s_ip2' maxlength=3 class=a3 onKeyUp=\"mask(this,%s)\" onbeforepaste=mask_c() value='%s' %s/>.",base_name,err_msg, ip2, flag?"disabled":"");
	fprintf( fp, "<input type=text name='%s_ip3' maxlength=3 class=a3 onKeyUp=\"mask(this,%s)\" onbeforepaste=mask_c() value='%s' %s/>.",base_name,err_msg, ip3, flag?"disabled":"");
	fprintf( fp, "<input type=text name='%s_ip4' maxlength=3 class=a3 onKeyUp=\"mask(this,%s)\" onbeforepaste=mask_c() value='%s' %s/>",base_name,err_msg, ip4, flag?"disabled":"");
	fprintf( fp, "</div>\n" );	
	
	free( def_value_temp );
	return 0;
}

/*****************************************
item  call back function  public    output
*********************************************/
int  output_html_obj_ip_input( struct st_conf_item *this, void *param )
{
	STPageInfo *pstPageInfo;
	
	pstPageInfo = param;
	
	return output_ip_input( this->conf_name, pstPageInfo->fp, search(pstPageInfo->lpublic,"ip_error"), this->conf_value, pstPageInfo->iUserGroup );
}

int  output_html_obj_normal_input( struct st_conf_item *this,  void *param )
{
	STPageInfo *pstPageInfo;
	
	pstPageInfo = param;
		
	fprintf( pstPageInfo->fp, "<input type=text name=%s value='%s' maxLength=%d %s />", 
								this->conf_name, this->conf_value, sizeof(this->conf_value)-1,(pstPageInfo->iUserGroup)?"disabled":"" );	
	return 0;
}

int output_html_auth_type( struct st_conf_item *this,  void *param  )
{
	STPageInfo *pstPageInfo;
	pstPageInfo = param;
	int ispap=0;

	if( strcasecmp("PAP",this->conf_value) == 0 ) ispap = 1;

	fprintf( pstPageInfo->fp,"<input type='radio' name='%s' value='CHAP' %s %s/>CHAP&nbsp&nbsp&nbsp"\
							"<input type='radio' name='%s' value='PAP' %s %s/>PAP",
							this->conf_name,
							ispap?"":"checked", (pstPageInfo->iUserGroup)?"disabled":"",
							this->conf_name,
							ispap?"checked":"", (pstPageInfo->iUserGroup)?"disabled":"" );

}

int output_html_radius_type( struct st_conf_item *this,  void *param  )
{
	STPageInfo *pstPageInfo;
	pstPageInfo = param;
	int ispap=0;

	if( strcasecmp("rj_sam",this->conf_value) == 0 ) ispap = 1;
	
	if( !ispap )
	{
		fprintf( pstPageInfo->fp,	"<select name=radiusT>" \
									"<option value=general selected=selected>general"\
									"<option value=rj_sam>rj_sam"\
									"</select>");
	}
	else
	{
		fprintf( pstPageInfo->fp,	"<select name=radiusT>" \
									"<option value=rj_sam selected=selected>rj_sam"\
									"<option value=general>general"\
									"</select>");
	}
}


#if 0
int  get_auth_type( char *name, char *value, int value_max_len )
{
	cgiFormStringNoNewlines( name, value, value_max_len );
}
#endif


int  output_html_eag_status_checkbox( struct st_conf_item *this,  void *param )
{
	STPageInfo *pstPageInfo;
	
	pstPageInfo = param;
	
	
	fprintf( pstPageInfo->fp, "<input type=checkbox name=%s value=%s %s />", 
								this->conf_name, this->conf_value, (strcmp(this->conf_value,"start")==0)?"checked":"" );	
	return 0;
}



int  output_html_eag_status_select( struct st_conf_item *this,  void *param )
{
	STPageInfo *pstPageInfo;
	
	pstPageInfo = param;
	char *urlnode=(char *)malloc(20);
	memset(urlnode,0,20);
	cgiFormStringNoNewlines( "plotid", urlnode, 20 );
	if(strcmp(urlnode,"")!=0)
		strcpy(this->conf_value,urlnode);
	else
		strcpy(this->conf_value,PLOTID_ONE);
	
	fprintf( pstPageInfo->fp, "<select name=%s value=%s onchange=plotid_change(this) style=width:60px><option value=%s  %s>%s</option><option value=%s  %s>%s</option>"\
		"<option value=%s %s>%s</option><option value=%s  %s>%s</option><option value=%s  %s>%s</option></select>", 
								this->conf_name, this->conf_value,PLOTID_ONE,(strcmp(this->conf_value,PLOTID_ONE)==0)?"selected":"",
								PLOTID_ONE,PLOTID_TWO,(strcmp(this->conf_value,PLOTID_TWO)==0)?"selected":"",PLOTID_TWO,
								PLOTID_THREE,(strcmp(this->conf_value,PLOTID_THREE)==0)?"selected":"",PLOTID_THREE,
								PLOTID_FOUR,(strcmp(this->conf_value,PLOTID_FOUR)==0)?"selected":"",PLOTID_FOUR,
								PLOTID_FIVE,(strcmp(this->conf_value,PLOTID_FIVE)==0)?"selected":"",PLOTID_FIVE);	


   free(urlnode);
	return 0;
}

int  output_html_eag_radius_select( struct st_conf_item *this,  void *param)
{
	STPageInfo *pstPageInfo;	
	pstPageInfo = param;

	struct optionz c_head;
	memset(&c_head,0,sizeof(c_head));
	
	int cnum,cflag=-1;

	cflag=read_optionz_xml(MULTI_RADIUS_F, &c_head, &cnum, MTR_N);
    fprintf(pstPageInfo->fp,"<select name=%s value=%s style=width:60px>",this->conf_name, this->conf_value);
    if(cflag==0)
	{   
        if(strcmp(c_head.content0,"")!=0)
		fprintf( pstPageInfo->fp,"<option value=%s  %s>%s</option>",PLOTID_ZEAO,(strcmp(this->conf_value,PLOTID_ONE)==0)?"selected":"",MTD_N);
		if(strcmp(c_head.content1,"")!=0)
		fprintf( pstPageInfo->fp,"<option value=%s  %s>%s</option>",PLOTID_ONE,(strcmp(this->conf_value,PLOTID_ONE)==0)?"selected":"",PLOTID_ONE);
        if(strcmp(c_head.content2,"")!=0)
		fprintf( pstPageInfo->fp,"<option value=%s  %s>%s</option>",PLOTID_TWO,(strcmp(this->conf_value,PLOTID_TWO)==0)?"selected":"",PLOTID_TWO);
        if(strcmp(c_head.content3,"")!=0)
		fprintf( pstPageInfo->fp,"<option value=%s  %s>%s</option>",PLOTID_THREE,(strcmp(this->conf_value,PLOTID_THREE)==0)?"selected":"",PLOTID_THREE);
        if(strcmp(c_head.content4,"")!=0)
		fprintf( pstPageInfo->fp,"<option value=%s  %s>%s</option>",PLOTID_FOUR,(strcmp(this->conf_value,PLOTID_FOUR)==0)?"selected":"",PLOTID_FOUR);
        if(strcmp(c_head.content5,"")!=0)
		fprintf( pstPageInfo->fp,"<option value=%s  %s>%s</option>",PLOTID_FIVE,(strcmp(this->conf_value,PLOTID_FIVE)==0)?"selected":"",PLOTID_FIVE);

	}
	fprintf(pstPageInfo->fp,"</select>");
	
	return 0;
}

int  output_html_eag_portal_select( struct st_conf_item *this,  void *param)
{
	STPageInfo *pstPageInfo;	
	pstPageInfo = param;

	struct optionz c_head;
	memset(&c_head,0,sizeof(c_head));

	int cnum,cflag=-1;

	cflag=read_optionz_xml(MULTI_PORTAL_F, &c_head, &cnum, MTP_N);
    fprintf(pstPageInfo->fp,"<select name=%s value=%s style=width:60px>",this->conf_name, this->conf_value);
	if(cflag==0)
	{   
        if(strcmp(c_head.content0,"")!=0)
		fprintf( pstPageInfo->fp,"<option value=%s  %s>%s</option>",PLOTID_ZEAO,(strcmp(this->conf_value,PLOTID_ONE)==0)?"selected":"",MTD_N);
		if(strcmp(c_head.content1,"")!=0)
		fprintf( pstPageInfo->fp,"<option value=%s  %s>%s</option>",PLOTID_ONE,(strcmp(this->conf_value,PLOTID_ONE)==0)?"selected":"",PLOTID_ONE);
        if(strcmp(c_head.content2,"")!=0)
		fprintf( pstPageInfo->fp,"<option value=%s  %s>%s</option>",PLOTID_TWO,(strcmp(this->conf_value,PLOTID_TWO)==0)?"selected":"",PLOTID_TWO);
        if(strcmp(c_head.content3,"")!=0)
		fprintf( pstPageInfo->fp,"<option value=%s  %s>%s</option>",PLOTID_THREE,(strcmp(this->conf_value,PLOTID_THREE)==0)?"selected":"",PLOTID_THREE);
        if(strcmp(c_head.content4,"")!=0)
		fprintf( pstPageInfo->fp,"<option value=%s  %s>%s</option>",PLOTID_FOUR,(strcmp(this->conf_value,PLOTID_FOUR)==0)?"selected":"",PLOTID_FOUR);
        if(strcmp(c_head.content5,"")!=0)
		fprintf( pstPageInfo->fp,"<option value=%s  %s>%s</option>",PLOTID_FIVE,(strcmp(this->conf_value,PLOTID_FIVE)==0)?"selected":"",PLOTID_FIVE);

	}
	fprintf(pstPageInfo->fp,"</select>");
	
	return 0;
}
int  output_html_eag_nas_select( struct st_conf_item *this,  void *param)
{
	STPageInfo *pstPageInfo;	
	pstPageInfo = param;

	struct optionz c_head;
	memset(&c_head,0,sizeof(c_head));
	
	int cnum,cflag=-1;

	cflag=read_optionz_xml(MULTI_NAS_F, &c_head, &cnum, MTN_N);
	fprintf(pstPageInfo->fp,"<select name=%s value=%s style=width:60px>",this->conf_name, this->conf_value);
	if(cflag==0)
	{   
        if(strcmp(c_head.content0,"")!=0)
		fprintf( pstPageInfo->fp,"<option value=%s  %s>%s</option>",PLOTID_ZEAO,(strcmp(this->conf_value,PLOTID_ONE)==0)?"selected":"",MTD_N);
		if(strcmp(c_head.content1,"")!=0)
		fprintf( pstPageInfo->fp,"<option value=%s  %s>%s</option>",PLOTID_ONE,(strcmp(this->conf_value,PLOTID_ONE)==0)?"selected":"",PLOTID_ONE);
        if(strcmp(c_head.content2,"")!=0)
		fprintf( pstPageInfo->fp,"<option value=%s  %s>%s</option>",PLOTID_TWO,(strcmp(this->conf_value,PLOTID_TWO)==0)?"selected":"",PLOTID_TWO);
        if(strcmp(c_head.content3,"")!=0)
		fprintf( pstPageInfo->fp,"<option value=%s  %s>%s</option>",PLOTID_THREE,(strcmp(this->conf_value,PLOTID_THREE)==0)?"selected":"",PLOTID_THREE);
        if(strcmp(c_head.content4,"")!=0)
		fprintf( pstPageInfo->fp,"<option value=%s  %s>%s</option>",PLOTID_FOUR,(strcmp(this->conf_value,PLOTID_FOUR)==0)?"selected":"",PLOTID_FOUR);
        if(strcmp(c_head.content5,"")!=0)
		fprintf( pstPageInfo->fp,"<option value=%s  %s>%s</option>",PLOTID_FIVE,(strcmp(this->conf_value,PLOTID_FIVE)==0)?"selected":"",PLOTID_FIVE);

	}
	fprintf(pstPageInfo->fp,"</select>");
	return 0;
}

int  output_html_eag_wwv_select( struct st_conf_item *this,  void *param)
{
	STPageInfo *pstPageInfo;	
	pstPageInfo = param;

	struct optionz c_head;
	memset(&c_head,0,sizeof(c_head));

	int cnum,cflag=-1;

	cflag=read_optionz_xml(MULTI_WWV_F, &c_head, &cnum, MTW_N);
    fprintf(pstPageInfo->fp,"<select name=%s value=%s style=width:60px>",this->conf_name, this->conf_value);
	if(cflag==0)
	{   
	    if(strcmp(c_head.content0,"")!=0)
		fprintf( pstPageInfo->fp,"<option value=%s  %s>%s</option>",PLOTID_ZEAO,(strcmp(this->conf_value,PLOTID_ONE)==0)?"selected":"",MTD_N);
		if(strcmp(c_head.content1,"")!=0)
		fprintf( pstPageInfo->fp,"<option value=%s  %s>%s</option>",PLOTID_ONE,(strcmp(this->conf_value,PLOTID_ONE)==0)?"selected":"",PLOTID_ONE);
        if(strcmp(c_head.content2,"")!=0)
		fprintf( pstPageInfo->fp,"<option value=%s  %s>%s</option>",PLOTID_TWO,(strcmp(this->conf_value,PLOTID_TWO)==0)?"selected":"",PLOTID_TWO);
        if(strcmp(c_head.content3,"")!=0)
		fprintf( pstPageInfo->fp,"<option value=%s  %s>%s</option>",PLOTID_THREE,(strcmp(this->conf_value,PLOTID_THREE)==0)?"selected":"",PLOTID_THREE);
        if(strcmp(c_head.content4,"")!=0)
		fprintf( pstPageInfo->fp,"<option value=%s  %s>%s</option>",PLOTID_FOUR,(strcmp(this->conf_value,PLOTID_FOUR)==0)?"selected":"",PLOTID_FOUR);
        if(strcmp(c_head.content5,"")!=0)
		fprintf( pstPageInfo->fp,"<option value=%s  %s>%s</option>",PLOTID_FIVE,(strcmp(this->conf_value,PLOTID_FIVE)==0)?"selected":"",PLOTID_FIVE);

	}
	fprintf(pstPageInfo->fp,"</select>");
	
	return 0;
}

int  output_html_idle_tick_checkbox( struct st_conf_item *this,  void *param )
{
	STPageInfo *pstPageInfo;
	
	pstPageInfo = param;
	
	
	fprintf( pstPageInfo->fp, "<input type=checkbox name=%s value=%s %s />", 
								this->conf_name, this->conf_value, (strcmp(this->conf_value,"start")==0)?"checked":"" );	
	return 0;
}


/*****************************************
item  call back function  public    get value
*********************************************/
//可以用于普通的input passwd select 等类型的value的获取
int  get_value_user_input_normal( char *name, char *value, int value_max_len )//得到相应的用户输入，并进行错误检查,返回错误id
{		
	memset( value, 0, value_max_len );
	return cgiFormStringNoNewlines( name, value, value_max_len );
}

//可以用于ip地址的value的获取
int  get_value_user_input_ip( char *name, char *value, int value_max_len )
{
	char temp_name[64]="";
	char ip1[5]="";
	char ip2[5]="";
	char ip3[5]="";
	char ip4[5]="";
	
	memset( value, 0, value_max_len );
	
	snprintf( temp_name, sizeof(temp_name), "%s_ip1", name );
	cgiFormStringNoNewlines( temp_name, ip1, sizeof(ip1) );

	snprintf( temp_name, sizeof(temp_name), "%s_ip2", name );
	cgiFormStringNoNewlines( temp_name, ip2, sizeof(ip2) );
	
	snprintf( temp_name, sizeof(temp_name), "%s_ip3", name );
	cgiFormStringNoNewlines( temp_name, ip3, sizeof(ip3) );
	
	snprintf( temp_name, sizeof(temp_name), "%s_ip4", name );
	cgiFormStringNoNewlines( temp_name, ip4, sizeof(ip4) );
	
	if( strlen(ip1)==0 || strlen(ip2)==0 || strlen(ip3)==0 || strlen(ip4)==0 )
	{
		return -1;
	}
	
	snprintf( value, value_max_len, "%s.%s.%s.%s", ip1, ip2, ip3, ip4 );
	return 0;
}


/******************************************
获取各个属性value的函数，同时对输入进行错误校验，基本上是每个参数都有一个对应的处理函数。
*******************************************/
//用于获取用户输入的eag状态
int get_eag_status( t_conf_item *this, void *param )
{
	char **status;
	STPageInfo *pstPageInfo;
	
	pstPageInfo = (STPageInfo *)param;
	
	if( cgiFormNotFound != cgiFormStringMultiple( this->conf_name, &status) )
	{
		strncpy( this->conf_value, "start", sizeof(this->conf_value) );
	}
	else
	{
		strncpy( this->conf_value, "stop", sizeof(this->conf_value) );
	}	
	
	
	return 0;
}

int get_idle_tick_status(t_conf_item *this, void *param )
{
	char **status;
	STPageInfo *pstPageInfo;
	
	pstPageInfo = (STPageInfo *)param;
	
	if( cgiFormNotFound != cgiFormStringMultiple( this->conf_name, &status) )
	{
		strncpy( this->conf_value, "start", sizeof(this->conf_value) );
	}
	else
	{
		strncpy( this->conf_value, "stop", sizeof(this->conf_value) );
	}	
	
	
	return 0;

}

//public normal input get 
int get_normal_input_conf( t_conf_item *this, void *param )
{
	char get_value[MAX_CONF_VALUE_LEN];
	int iRet;
	STPageInfo *pstPageInfo;
	
	pstPageInfo = (STPageInfo *)param;
	iRet = get_value_user_input_normal( this->conf_name, get_value, sizeof(get_value) );
	
	if( 0 == iRet && strlen(get_value) != 0 )
	{
		strncpy( this->conf_value, get_value, sizeof(this->conf_value) );
	}
	else
	{
		this->error = search( pstPageInfo->lauth, "input_null" );
	}
	
	return iRet;
}

//public select input get 
int get_select_input_conf( t_conf_item *this, void *param )
{
	char get_value[MAX_CONF_VALUE_LEN];
	int iRet;
	STPageInfo *pstPageInfo;
	
	pstPageInfo = (STPageInfo *)param;
	int choice;
	
	iRet = cgiFormSelectSingle("radiusT", radiusType, 2, &choice, 0);
	
	if( !choice )
		{
			strcpy(get_value,radiusType[0]);
		}
	else
		{
			strcpy(get_value,radiusType[1]);
		}
		strncpy( this->conf_value, get_value, sizeof(this->conf_value) );
	
	return iRet;
}

//public select input get 
int get_select_option_conf( t_conf_item *this, void *param )
{
	char get_value[MAX_CONF_VALUE_LEN];
	int iRet;
	STPageInfo *pstPageInfo;

	char plotid[20];
	memset(plotid,0,20);
	
	pstPageInfo = (STPageInfo *)param;	
	
	iRet = cgiFormStringNoNewlines( this->conf_name, plotid, 20 );

    strcpy(get_value,plotid);
	
	strncpy( this->conf_value, get_value, sizeof(this->conf_value) );
	
	return iRet;
}

//public get ip input value
int get_public_ip_conf( t_conf_item *this, void *param )
{
	STPageInfo *pstPageInfo;
	int iRet;
	char get_value[MAX_CONF_VALUE_LEN];//现将获得的结果保存到临时变量，当判断输入正确后才保存到实际的变量中。
	
	pstPageInfo = (STPageInfo *)param;
	
	iRet = get_value_user_input_ip( this->conf_name, get_value, sizeof(get_value) );
	if( 0 != iRet )
	{
		this->error = search( pstPageInfo->lpublic, "ip_not_null" );
		return iRet;
	}
	
	memcpy( this->conf_value, get_value, sizeof(this->conf_value) );
	return iRet;	
}

static int is_legal_digit( char *str )
{
	if( NULL == str ) return 0;
	
	for(;*str&&*str>='0'&&*str<='9';str++ );
	
	return !(*str);
}


//radius 认证端口  计费端口。
int get_tcpip_port( t_conf_item *this, void *param )
{
	STPageInfo *pstPageInfo;
	int iRet;
	char get_value[MAX_CONF_VALUE_LEN];//现将获得的结果保存到临时变量，当判断输入正确后才保存到实际的变量中。
	int port_num;
	
	pstPageInfo = (STPageInfo *)param;
	
	iRet = get_value_user_input_normal( this->conf_name, get_value, sizeof(get_value) );
	if( 0 != iRet )
	{
		this->error = search( pstPageInfo->lauth, "input_null" );
		return iRet;
	}
	
	if( ! is_legal_digit( get_value ) )
	{
		this->error = search( pstPageInfo->lauth, "whitelist_port_err" );
		return -1;	
	}
	
	port_num = atoi( get_value );
	if( 0 > port_num || port_num > 65535 )
	{
		this->error = search( pstPageInfo->lauth, "port_outoff_range" );
		return -1;
	}
	
	strcpy( this->conf_value, get_value );
	
	return iRet;
}
//max idle time
int get_max_idle_time( t_conf_item *this, void *param )
{
	STPageInfo *pstPageInfo;
	char get_value[MAX_CONF_VALUE_LEN];
	int idle_time;
	int iRet;
	
	pstPageInfo = (STPageInfo *)param;
	
	iRet = get_value_user_input_normal( this->conf_name, get_value, sizeof(get_value) );
	if( 0 != iRet )
	{
		this->error = search( pstPageInfo->lauth, "input_null" );
		return -1;
	}

	if( ! is_legal_digit( get_value ) )
	{
		this->error = search( pstPageInfo->lauth, "input_error" );
		return -1;		
	}
		
	idle_time = atoi( get_value );
	if( idle_time < 0 || idle_time > 24*60*60)   // 24hour 
	{
		this->error = search( pstPageInfo->lauth, "input_error" );
		return -1;			
	}
	
	strncpy( this->conf_value, get_value, sizeof(this->conf_value) );
	
	return 0;
	
}

// vrrp id
int get_vrrp_id( t_conf_item *this, void *param )
{
	STPageInfo *pstPageInfo;
	char get_value[MAX_CONF_VALUE_LEN];
	int vrrp_id;
	int iRet;
	
	pstPageInfo = (STPageInfo *)param;
	
	iRet = get_value_user_input_normal( this->conf_name, get_value, sizeof(get_value) );
	if( 0 != iRet )
	{
		this->error = search( pstPageInfo->lauth, "input_null" );
		return -1;
	}

	if( ! is_legal_digit( get_value ) )
	{
		this->error = search( pstPageInfo->lauth, "input_error" );
		return -1;		
	}
		
	vrrp_id = atoi( get_value );
	if( vrrp_id < 0 || vrrp_id > 16)
	{
		this->error = search( pstPageInfo->lauth, "input_error" );
		return -1;			
	}
	
	strncpy( this->conf_value, get_value, sizeof(this->conf_value) );
	
	return 0;
	
}

// max_httprsp
int get_max_httprsp( t_conf_item *this, void *param )
{
	STPageInfo *pstPageInfo;
	char get_value[MAX_CONF_VALUE_LEN];
	int max_httprsp;
	int iRet;
	
	pstPageInfo = (STPageInfo *)param;
	
	iRet = get_value_user_input_normal( this->conf_name, get_value, sizeof(get_value) );
	if( 0 != iRet )
	{
		this->error = search( pstPageInfo->lauth, "input_null" );
		return -1;
	}

	if( ! is_legal_digit( get_value ) )
	{
		this->error = search( pstPageInfo->lauth, "input_error" );
		return -1;		
	}
		
	max_httprsp = atoi( get_value );
	if( max_httprsp < 0 || max_httprsp > 100)
	{
		this->error = search( pstPageInfo->lauth, "input_error" );
		return -1;			
	}
	
	strncpy( this->conf_value, get_value, sizeof(this->conf_value) );
	
	return 0;
	
} 

/********************************************************
	全局的结构体item，当需要添加新的配置选项时，直接在这里添加就可以了,
	可能需要为其添加对应的输出html的函数，获取value的函数，错误检查的函数。
**********************************************************/

t_conf_item all_conf_item[]={
	{HS_PLOT_ID,"1",HS_PLOT_ID, 1, NULL,output_html_eag_status_select, get_select_option_conf},
	{HS_STATUS,"stop",HS_STATUS, 1, NULL,output_html_eag_status_checkbox, get_eag_status},//必须放在第一个。
	{HS_STATUS_KICK,"stop",HS_STATUS_KICK, 1, NULL,output_html_idle_tick_checkbox, get_idle_tick_status},
	{HS_DEBUG_LOG, "stop",HS_DEBUG_LOG, 1, NULL, output_html_eag_status_checkbox, get_eag_status},
	{HS_WANIF, "eth0-1", HS_WANIF, 0, NULL, NULL, NULL },/*set when limit user speed in radius,just kan be select!*/
	{HS_LANIF, "eth.p.0-4", HS_LANIF, 0, NULL, NULL, NULL },/*the if set in captive portal is LANIF,her is no use*/
	{HS_NETWORK, "10.1.1.0", HS_NETWORK, 0, NULL, NULL, NULL },
	{HS_NETMASK, "255.255.255.0", HS_NETMASK, 0, NULL, NULL, NULL },
	{HS_UAMLISTEN, "10.1.1.254", HS_UAMLISTEN, 1, NULL, output_html_obj_ip_input, get_public_ip_conf },
	{HS_UAMPORT, "3990", HS_UAMPORT, 1, NULL, output_html_obj_normal_input, get_tcpip_port },
	{HS_NAS_PT, "1", HS_NAS_PT, 1, NULL, output_html_eag_nas_select, get_select_option_conf },
	{HS_RADIUS_PT, "1", HS_RADIUS_PT, 1, NULL, output_html_eag_radius_select, get_select_option_conf },
	{HS_PORTAL_PT,"1",HS_PORTAL_PT,1,NULL,output_html_eag_portal_select,get_select_option_conf},/*if use inner portal server,set value as  HS_UAMLISTEN. other set as china mobile portal server's ip addr*/
	{HS_WWV_PT, "1", HS_WWV_PT, 1, NULL, output_html_eag_wwv_select, get_select_option_conf },
	{HS_DEFIDLETIMEOUT,"60",HS_DEFIDLETIMEOUT,1,NULL, output_html_obj_normal_input, get_max_idle_time },
	{HS_VRRPID,"1",HS_VRRPID,1,NULL, output_html_obj_normal_input, get_vrrp_id },
	//{HS_PPI_PORT,"1",HS_PPI_PORT,1,NULL, output_html_obj_normal_input, get_max_idle_time },
	{HS_PPI_PORT,"1",HS_PPI_PORT,1,NULL, output_html_obj_normal_input, get_tcpip_port },
	{HS_MAX_HTTPRSP,"35",HS_MAX_HTTPRSP,1,NULL, output_html_obj_normal_input, get_max_httprsp },

/*below twe usual no useage */
#if 0
	{"HS_ADMUSR","admin","HS_ADMUSR",1,NULL,output_html_obj_normal_input,get_normal_input_conf},
	{"HS_ADMPWD","admin","HS_ADMPWD",1,NULL,output_html_obj_normal_input,get_normal_input_conf},
#endif

};

#define MAX_ITEM_NUM	sizeof(all_conf_item)/sizeof(all_conf_item[0])



#define SUBMIT_NAME		"submit_save_conf"
#define CONF_FILE_PATH	"/opt/services/conf/eag_conf.conf"




/***************************************************************
申明回调函数
****************************************************************/
static int s_eagconf_prefix_of_page( STPageInfo *pstPageInfo );
static int s_eagconf_content_of_page( STPageInfo *pstPageInfo );
static int doUserCommand( STPageInfo *pstPageInfo );
static int doRedir( STPageInfo *pstPageInfo ,char *plot_id );

/***************************************************************
*USEAGE:	主函数
*Param:		
*Return:	
*			
*Auther:shao jun wu
*Date:2008-12-30 14:12:46
*Modify:(include modifyer,for what resease,date)
****************************************************************/
int cgiMain()
{
	STPageInfo stPageInfo;

	char plot_id[20];
	memset(plot_id,0,20);

	char *tmp=(char *)malloc(64);
	int ret;
	if(access(MULTI_EAG_F,0)!=0)
	{
		create_eag_xml(MULTI_EAG_F);
		write_status_file( MULTI_EAG_STATUS, "start" );
	}
	else
	{
		ret=if_xml_file_z(MULTI_EAG_F);
		if(ret!=0)
		{
		   memset(tmp,0,64);
		   sprintf(tmp,"sudo rm  %s > /dev/null",MULTI_EAG_F);
		   system(tmp);
		   create_eag_xml(MULTI_EAG_F);
		   write_status_file( MULTI_EAG_STATUS, "start" );
		
		}
	}
    free(tmp);

	
//初始化常用公共变量
	memset( &stPageInfo, 0,sizeof(STPageInfo) );

	cgiFormStringNoNewlines("UN", stPageInfo.encry, BUF_LEN);
	cgiFormStringNoNewlines("plotid", plot_id, 20);
	
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

    if(strcmp(plot_id,"")==0)
			strcpy(plot_id,PLOTID_ONE);

    if( cgiFormSubmitClicked(SUBMIT_NAME) == cgiFormSuccess  )
    {
    	if( 0 == stPageInfo.formProcessError )
    	{
   			stPageInfo.formProcessError = doUserCommand( &(stPageInfo) );
			fprintf(stderr,"stPageInfo.formProcessError = %d", stPageInfo.formProcessError );
   			doRedir( &stPageInfo ,plot_id);
    	}
    }
    else
    {
        //modify new functions
		load_conf_file( CONF_FILE_PATH, all_conf_item, MAX_ITEM_NUM ,plot_id);	
    }
	            

	MC_setActiveLabel( stPageInfo.pstPortalContainer->pstModuleContainer, 2 );

	MC_setPrefixOfPageCallBack( stPageInfo.pstPortalContainer->pstModuleContainer, (MC_CALLBACK)s_eagconf_prefix_of_page, &stPageInfo );
	MC_setContentOfPageCallBack( stPageInfo.pstPortalContainer->pstModuleContainer, (MC_CALLBACK)s_eagconf_content_of_page, &stPageInfo );

	
	MC_setOutPutFileHandle( stPageInfo.pstPortalContainer->pstModuleContainer, cgiOut );

	MC_setModuleContainerDomainValue( stPageInfo.pstPortalContainer->pstModuleContainer, FORM_ONSIBMIT, "return true;" );
	//可以设置为一个javascript函数,这个js函数的实现放在prefix回调函数中就可以了。
	MC_setModuleContainerDomainValue( stPageInfo.pstPortalContainer->pstModuleContainer, FORM_METHOD, "post" );
	MC_setModuleContainerDomainValue( stPageInfo.pstPortalContainer->pstModuleContainer, FORM_ACTION, "wp_eag_conf.cgi" );
	MC_setModuleContainerDomainValue( stPageInfo.pstPortalContainer->pstModuleContainer, PUBLIC_INPUT_ENCRY, stPageInfo.encry );
	
	
	MC_setModuleContainerDomainValue( stPageInfo.pstPortalContainer->pstModuleContainer, BTN_OK_IMG, search(stPageInfo.lpublic,"img_ok") );
	MC_setModuleContainerDomainValue( stPageInfo.pstPortalContainer->pstModuleContainer, BTN_OK_SUBMIT_NAME, SUBMIT_NAME );

	
	MC_setModuleContainerDomainValue( stPageInfo.pstPortalContainer->pstModuleContainer, LABLE_TOP_HIGHT, "25" );

	
	MC_writeHtml( stPageInfo.pstPortalContainer->pstModuleContainer );
	
	release_portal_container(&(stPageInfo.pstPortalContainer));
	
	
	return 0;
}


static int s_eagconf_prefix_of_page( STPageInfo *pstPageInfo )
{
	int i=0;

	if( NULL == pstPageInfo )
	{
		return -1;
	}
		
	for( i=0; i<MAX_ITEM_NUM; i++ )
	{
	 	if( NULL != all_conf_item[i].error )
	 	{
	 		char err_msg[512];
	 		
	 		snprintf( err_msg, sizeof(err_msg), "%s:%s\\n%s", search(pstPageInfo->lauth,all_conf_item[i].conf_name), all_conf_item[i].error,
	 								search(pstPageInfo->lauth,"conf_save_failed" ) );
	 	    ShowAlert( err_msg );
	 	}
	}
 	
 	fprintf( cgiOut, "<style type=text/css>.a3{width:30px;border:0px; text-align:center}</style>" );
 	fprintf( cgiOut, "<script language=javascript src=/ip.js></script>\n" );
 	fprintf( cgiOut, "<script language=javascript src=/fw.js></script>\n" );

	fprintf(cgiOut,"<script type=text/javascript>\n");
   	fprintf(cgiOut,"function plotid_change( obj )\n"\
   	"{\n"\
   	"var plotid = obj.options[obj.selectedIndex].value;\n"\
   	"var url = 'wp_eag_conf.cgi?UN=%s&plotid='+plotid;\n"\
   	"window.location.href = url;\n"\
   	"}\n", pstPageInfo->encry);
    fprintf(cgiOut,"</script>\n" );

	return 0;		
}
 
static int s_eagconf_content_of_page( STPageInfo *pstPageInfo )
{
	int i;
	
	if( NULL == pstPageInfo )
	{
		return -1;
	}
	
	fprintf( pstPageInfo->fp, "<table>\n" );
	for( i=0; i<MAX_ITEM_NUM ; i++ )
	{
		if( 1 == all_conf_item[i].show_flag )
		{
			fprintf( pstPageInfo->fp, "<tr><td>%s</td><td><b>:</b></td><td>", search(pstPageInfo->lauth,all_conf_item[i].show_key) );
			if( NULL != all_conf_item[i].output_html_obj )
			{
				all_conf_item[i].output_html_obj( &all_conf_item[i], (void*)pstPageInfo );
			}
			fprintf( pstPageInfo->fp, "</td></tr>\n" );
		}
	}
	fprintf(pstPageInfo->fp,"<tr><td colspan=2><input type=hidden name=plotid value=\"%s\"></td></tr>\n",all_conf_item[0].conf_value);
	fprintf( pstPageInfo->fp, "</table>\n" );
	
	return 0;	
}


static int doUserCommand( STPageInfo *pstPageInfo )
{
	int iRet=0;
	int i;
	
	for( i=0; i<MAX_ITEM_NUM; i++ )
	{
		if(  1 == all_conf_item[i].show_flag  &&  NULL != all_conf_item[i].get_value_user_input )
		{
			fprintf(stderr, "%s = %s",all_conf_item[i].conf_name,all_conf_item[i].conf_value);
			all_conf_item[i].get_value_user_input( &all_conf_item[i], pstPageInfo );
		}			
	}
	
    //modify new functions
	iRet = save_conf_file( MULTI_EAG_F, all_conf_item, MAX_ITEM_NUM );


{//根据输入的状态，确定是否需要启动服务,关于状态的设置必须在第一个。
	int status_user_input=0;
	int status_current=0;
	
	if( strcmp( all_conf_item[1].conf_value, "start" ) == 0 )
	{
		fprintf(stderr,"instance status start!\n");
		status_user_input = 1;
	}
	else
	{
		fprintf(stderr,"instance status stop!\n");	
	}
	
	status_current=eag_services_status();
	fprintf(stderr,"status_current=%d",status_current );
#if 0	
	if( 1==status_user_input && 0==status_current )
	{
		eag_services_start();
	}
	else if( 0==status_user_input && 1==status_current )
	{
		/*eag_services_stop();*//*this start is this instance!  if has instance start should determine in */
		eag_services_restart();
	}
	else if( 1==status_user_input && 1==status_current )
#endif		
	{
		eag_services_restart();
	}

	fprintf(stderr,"after doUserCommand = %d", iRet );
}	
	return iRet;
}


static int doRedir( STPageInfo *pstPageInfo ,char *plot_id )
{
	fprintf(stderr, "doRedir before xxxxx\n" );

	cgiHeaderContentType("text/html");
	
	fprintf(stderr, "pstPageInfo->encry = %p  %s\n", pstPageInfo->encry, pstPageInfo->encry );
	fprintf(cgiOut, "<script type=text/javascript>\nwindow.location.href='wp_eag_conf.cgi?UN=%s&plotid=%s';\n</script>", 
					pstPageInfo->encry,plot_id );
	fprintf(stderr, "doRedir before ret\n" );
	exit(0);
		
}

