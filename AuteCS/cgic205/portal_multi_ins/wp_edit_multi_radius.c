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
* wp_edit_multi_radius.c
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
#ifndef EDIT_MULTI_RADIUS
#define EDIT_MULTI_RADIUS

#define SUBMIT_NAME 	"submit_edit_multi_radius"

//#define MULTI_RADIUS_STATUS_FILE_PATH	"/opt/services/status/multi_radius_status.status"
#define MULTI_RADIUS_CONF_FILE_PATH	"/opt/services/conf/multiradius_conf.conf"

       
#endif

#define MAX_RADIUS_DOMAIN_LEN		128
#define MAX_RADIUS_KEY_LEN		128

int ip_input_is_legal(const char *str);
int port_input_is_legal(const char *str);

/***************************************************************
定义页面要用到的结构体
****************************************************************/


typedef struct{
	STPortalContainer *pstPortalContainer;
	struct list *lpublic;/*解析public.txt文件的链表头*/
	struct list *lauth;
	char encry[BUF_LEN];
	char *username_encry;	         /*存储解密后的当前登陆用户名*/
	int iUserGroup;	//为0时表示是管理员。
	FILE *fp;
	
} STPageInfo;



/***************************************************************
申明回调函数
****************************************************************/
static int s_editMulitRadius_prefix_of_page( STPageInfo *pstPageInfo );
static int s_editMulitRadius_content_of_page( STPageInfo *pstPageInfo );
static int edit_num;

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
	char url[256]="";
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


	

	MC_setActiveLabel( stPageInfo.pstPortalContainer->pstModuleContainer, 8 );

	MC_setPrefixOfPageCallBack( stPageInfo.pstPortalContainer->pstModuleContainer, (MC_CALLBACK)s_editMulitRadius_prefix_of_page, &stPageInfo );
	MC_setContentOfPageCallBack( stPageInfo.pstPortalContainer->pstModuleContainer, (MC_CALLBACK)s_editMulitRadius_content_of_page, &stPageInfo );

	
	MC_setOutPutFileHandle( stPageInfo.pstPortalContainer->pstModuleContainer, cgiOut );

	LB_changelabelName_Byindex(stPageInfo.pstPortalContainer->pstModuleContainer,search(stPageInfo.lauth,"add_multi_radius"),9);
	sprintf(url,"%s?UN=%s","wp_multi_radius.cgi",stPageInfo.encry);
	LB_changelabelUrl_Byindex(stPageInfo.pstPortalContainer->pstModuleContainer,url,7);

	MC_setModuleContainerDomainValue( stPageInfo.pstPortalContainer->pstModuleContainer, FORM_ONSIBMIT, "return true;" );
	//可以设置为一个javascript函数,这个js函数的实现放在prefix回调函数中就可以了。
	MC_setModuleContainerDomainValue( stPageInfo.pstPortalContainer->pstModuleContainer, FORM_METHOD, "post" );


	MC_setModuleContainerDomainValue( stPageInfo.pstPortalContainer->pstModuleContainer, PUBLIC_INPUT_ENCRY, stPageInfo.encry );
	
	
	MC_setModuleContainerDomainValue( stPageInfo.pstPortalContainer->pstModuleContainer, BTN_OK_IMG, search(stPageInfo.lpublic,"img_ok") );
	MC_setModuleContainerDomainValue( stPageInfo.pstPortalContainer->pstModuleContainer, BTN_OK_SUBMIT_NAME, SUBMIT_NAME );
	
	snprintf( url, sizeof(url), "wp_multi_radius.cgi?UN=%s", stPageInfo.encry );
	MC_setModuleContainerDomainValue( stPageInfo.pstPortalContainer->pstModuleContainer, BTN_CANCEL_URL, url );
	
	MC_setModuleContainerDomainValue( stPageInfo.pstPortalContainer->pstModuleContainer, LABLE_TOP_HIGHT, "25" );

	
	MC_writeHtml( stPageInfo.pstPortalContainer->pstModuleContainer );
	
	release_portal_container(&(stPageInfo.pstPortalContainer));
	
	
	return 0;
}


static int s_editMulitRadius_prefix_of_page( STPageInfo *pstPageInfo )
{
	FILE * fp = pstPageInfo->fp;
	char edit_rule[10] = "";
	char index[32] = "";
	
	struct list * radius_public = pstPageInfo->lpublic;
	struct list * radius_auth = pstPageInfo->lauth;

	
	char plot_id[30],domain_name[256],radius_server_type[256],radius_server_ip[32],radius_server_port[32],radius_server_key[256],radius_server_portal[32],
			charging_server_ip[32],charging_server_port[32],charging_server_key[256],
			backup_radius_server_ip[32],backup_radius_server_port[32],backup_radius_server_key[256],backup_radius_server_portal[32],
			backup_charging_server_ip[32],backup_charging_server_port[32],backup_charging_server_key[256],swap_octets[256];

	memset(plot_id,0,sizeof(plot_id));
	memset(domain_name, 0, sizeof(domain_name));	
	memset(radius_server_type, 0, sizeof(radius_server_type));	
	memset(radius_server_ip, 0, sizeof(radius_server_ip));
	memset(radius_server_port, 0, sizeof(radius_server_port));
	memset(radius_server_key, 0, sizeof(radius_server_key));
	memset(radius_server_portal, 0, sizeof(radius_server_portal));
	memset(charging_server_ip, 0, sizeof(charging_server_ip));
	memset(charging_server_port, 0, sizeof(charging_server_port));
	memset(charging_server_key, 0, sizeof(charging_server_key));
	memset(backup_radius_server_ip, 0, sizeof(backup_radius_server_ip));
	memset(backup_radius_server_port, 0, sizeof(backup_radius_server_port));
	memset(backup_radius_server_key, 0, sizeof(backup_radius_server_key));
	memset(backup_radius_server_portal, 0, sizeof(backup_radius_server_portal));
	memset(backup_charging_server_ip, 0, sizeof(backup_charging_server_ip));
	memset(backup_charging_server_port, 0, sizeof(backup_charging_server_port));
	memset(backup_charging_server_key, 0, sizeof(backup_charging_server_key));
	memset(swap_octets, 0, sizeof(swap_octets));

	

	if( cgiFormSubmitClicked(SUBMIT_NAME) == cgiFormSuccess )
	{
		cgiFormStringNoNewlines("plot_id",plot_id, sizeof(plot_id));
		cgiFormStringNoNewlines("domain_name",domain_name, sizeof(domain_name));
		cgiFormStringNoNewlines("radius_server_type",radius_server_type, sizeof(radius_server_type));
		cgiFormStringNoNewlines("radius_server_ip",radius_server_ip, sizeof(radius_server_ip));
		cgiFormStringNoNewlines("radius_server_port",radius_server_port, sizeof(radius_server_port));
		cgiFormStringNoNewlines("radius_server_key",radius_server_key, sizeof(radius_server_key));
		cgiFormStringNoNewlines("radius_server_portal",radius_server_portal, sizeof(radius_server_portal));
		cgiFormStringNoNewlines("charging_server_ip",charging_server_ip, sizeof(charging_server_ip));
		cgiFormStringNoNewlines("charging_server_port",charging_server_port, sizeof(charging_server_port));
		cgiFormStringNoNewlines("charging_server_key",charging_server_key, sizeof(charging_server_key));
		cgiFormStringNoNewlines("backup_radius_server_ip",backup_radius_server_ip, sizeof(backup_radius_server_ip));
		cgiFormStringNoNewlines("backup_radius_server_port",backup_radius_server_port, sizeof(backup_radius_server_port));
		cgiFormStringNoNewlines("backup_radius_server_key",backup_radius_server_key, sizeof(backup_radius_server_key));
		cgiFormStringNoNewlines("backup_radius_server_portal",backup_radius_server_portal, sizeof(backup_radius_server_portal));
		cgiFormStringNoNewlines("backup_charging_server_ip",backup_charging_server_ip, sizeof(backup_charging_server_ip));
		cgiFormStringNoNewlines("backup_charging_server_port",backup_charging_server_port, sizeof(backup_charging_server_port));
		cgiFormStringNoNewlines("backup_charging_server_key",backup_charging_server_key, sizeof(backup_charging_server_key));
	 	cgiFormStringNoNewlines("swap_octets",swap_octets, sizeof(swap_octets));
	 	
		/////////////处理数据///////////////////
		#if 0
		FILE * pfile = NULL;
		char content[2048];
		memset( content, 0, sizeof(content));
		sprintf( content, "%s;%s;%s;%s;%s;%s;%s;%s;%s;%s;%s;%s;%s;%s;%s;%s\n",plot_id,domain_name,
								radius_server_ip,radius_server_port,radius_server_key,radius_server_portal,
								charging_server_ip,charging_server_port,charging_server_key,
								backup_radius_server_ip,backup_radius_server_port,backup_radius_server_key,backup_radius_server_portal,
								backup_charging_server_ip,backup_charging_server_port,backup_charging_server_key);

        #endif 
        if(strcmp(plot_id,"")==0)
		 cgiFormStringNoNewlines("PLOTIDZ",plot_id, sizeof(plot_id));

		if(!strcmp(domain_name,"") || strlen(domain_name) > MAX_RADIUS_DOMAIN_LEN-1)
		{
			ShowAlert( search(radius_public, "input_illegal"));
			return 0;
		}
		if (radius_server_ip != NULL && strcmp(radius_server_ip, "") != 0 && !ip_input_is_legal(radius_server_ip))
		{
			ShowAlert( search(radius_public, "input_illegal"));
			return 0;
		}
		if (charging_server_ip != NULL && strcmp(charging_server_ip, "") != 0 && !ip_input_is_legal(charging_server_ip))
		{
			ShowAlert( search(radius_public, "input_illegal"));
			return 0;
		}
		if (backup_radius_server_ip != NULL && strcmp(backup_radius_server_ip, "") != 0 && !ip_input_is_legal(backup_radius_server_ip))
		{
			ShowAlert( search(radius_public, "input_illegal"));
			return 0;
		}
		if (backup_charging_server_ip != NULL && strcmp(backup_charging_server_ip, "") != 0 && !ip_input_is_legal(backup_charging_server_ip))
		{
			ShowAlert( search(radius_public, "input_illegal"));
			return 0;
		}
		if (radius_server_port != NULL && strcmp(radius_server_port, "") != 0 && !port_input_is_legal(radius_server_port))
		{
			ShowAlert( search(radius_public, "input_illegal"));
			return 0;
		}
		if (charging_server_port != NULL && strcmp(charging_server_port, "") != 0 && !port_input_is_legal(charging_server_port))
		{
			ShowAlert( search(radius_public, "input_illegal"));
			return 0;
		}
		if (backup_radius_server_port != NULL && strcmp(backup_radius_server_port, "") != 0 && !port_input_is_legal(backup_radius_server_port))
		{
			ShowAlert( search(radius_public, "input_illegal"));
			return 0;
		}
		if (backup_charging_server_port != NULL && strcmp(backup_charging_server_port, "") != 0 && !port_input_is_legal(backup_charging_server_port))
		{
			ShowAlert( search(radius_public, "input_illegal"));
			return 0;
		}
		if (strlen(radius_server_key) > MAX_RADIUS_KEY_LEN-1
			|| strlen(charging_server_key) > MAX_RADIUS_KEY_LEN-1
			|| strlen(backup_radius_server_key) > MAX_RADIUS_KEY_LEN-1
			|| strlen(backup_charging_server_key) > MAX_RADIUS_KEY_LEN-1)
		{
			ShowAlert( search(radius_public, "input_illegal"));
			return 0;
		}
		char *tmpz=(char *)malloc(20);
		memset(tmpz,0,20);
		sprintf(tmpz,"%s%s",MTR_N,plot_id);

		add_eag_node_attr(MULTI_RADIUS_F, tmpz, ATT_Z, domain_name);		
        mod_eag_node(MULTI_RADIUS_F, tmpz, ATT_Z, domain_name, RDOMAIN, domain_name);
        mod_eag_node(MULTI_RADIUS_F, tmpz, ATT_Z, domain_name, RRADST, radius_server_type);
		mod_eag_node(MULTI_RADIUS_F, tmpz, ATT_Z, domain_name, RRIP, radius_server_ip);		
        mod_eag_node(MULTI_RADIUS_F, tmpz, ATT_Z, domain_name, RRPORT, radius_server_port);
        mod_eag_node(MULTI_RADIUS_F, tmpz, ATT_Z, domain_name, RRKEY, radius_server_key);
        mod_eag_node(MULTI_RADIUS_F, tmpz, ATT_Z, domain_name, RRPORTAL, radius_server_portal);
        mod_eag_node(MULTI_RADIUS_F, tmpz, ATT_Z, domain_name, RCIP, charging_server_ip);
        mod_eag_node(MULTI_RADIUS_F, tmpz, ATT_Z, domain_name, RCPORT, charging_server_port);
        mod_eag_node(MULTI_RADIUS_F, tmpz, ATT_Z, domain_name, RCKEY, charging_server_key);
        mod_eag_node(MULTI_RADIUS_F, tmpz, ATT_Z, domain_name, RBIP, backup_radius_server_ip);
        mod_eag_node(MULTI_RADIUS_F, tmpz, ATT_Z, domain_name, RBPORT, backup_radius_server_port);
        mod_eag_node(MULTI_RADIUS_F, tmpz, ATT_Z, domain_name, RBKEY, backup_radius_server_key);
        mod_eag_node(MULTI_RADIUS_F, tmpz, ATT_Z, domain_name, RBPORTAL, backup_radius_server_portal);
        mod_eag_node(MULTI_RADIUS_F, tmpz, ATT_Z, domain_name, RBCIP, backup_charging_server_ip);
        mod_eag_node(MULTI_RADIUS_F, tmpz, ATT_Z, domain_name, RBCPORT, backup_charging_server_port);
        mod_eag_node(MULTI_RADIUS_F, tmpz, ATT_Z, domain_name, RBCKEY, backup_charging_server_key);
        mod_eag_node(MULTI_RADIUS_F, tmpz, ATT_Z, domain_name, R_SWAP_OCTETS, swap_octets);
		
		free(tmpz);
		#if 0
		char * ptr ;
		int i_index;
	    char  s_index[256];
		cgiFormStringNoNewlines("i_index",s_index,sizeof(s_index));
		i_index= atoi(s_index);
		
		int  i, revnum=0, location=-1;
		
		int temp=-1;
		int del_loc = 0;
		char * revinfo[512];
		char buf[2048];
		
		if( (pfile = fopen(MULTI_RADIUS_CONF_FILE_PATH,"r+")) != NULL )
		{
			//fprintf(stderr,"fp=%x",fp);//输出到出错文档?
			//读取操作
			memset( buf, 0, sizeof(buf));
			while( fgets(buf, sizeof(buf), pfile) )//
			{
				del_loc ++;
				revinfo[revnum] = (char *)malloc(sizeof(buf));
				memset( revinfo[revnum], 0, sizeof(buf) );
				strncpy( revinfo[revnum], buf, strlen(buf) );
				//fprintf( stderr, "buf=%s",buf );
				//sscanf( revinfo[revnum], "%[^=]=%[^=]=%[^=]=%[^\n]\n",a1,a2,a3,a4);
				//sscanf( revinfo[revnum], "%[^=]=%[^=]=%[^=]=%[^\n]\n",a1,a2,a3,a4);
				//fprintf( stderr, "%s--%s",a1,a2);
				if( del_loc == i_index )
				{
					location = revnum;
				}
				
				memset( buf, 0, sizeof(buf) );
				revnum ++ ;
			}
			fclose(pfile);
			pfile = fopen( MULTI_RADIUS_CONF_FILE_PATH, "w+" );
			////写回操作
			for( i=0; i<revnum; i++ )
			{
				if( i == location )
				{
					fwrite(content, strlen(content), 1, pfile);
					//fprintf( stderr, "WRITE~~" );	
					ShowAlert( search(radius_auth, "edit_multi_radius_suc") );		
					continue;
				}				
				fwrite( revinfo[i], strlen(revinfo[i]), 1, pfile);
				free(revinfo[i]);//add by wk
			}
			fclose(pfile);
		}		
		#endif

		fprintf( fp, "<script type='text/javascript'>\n" );
		fprintf( fp, "window.location.href='wp_multi_radius.cgi?UN=%s&plotid=%s';\n", pstPageInfo->encry,plot_id);
		fprintf( fp, "</script>\n" );		
	}
	return 0;		
}

static int s_editMulitRadius_content_of_page( STPageInfo *pstPageInfo )
{
	FILE * fp = pstPageInfo->fp;
	struct list * radius_public = pstPageInfo->lpublic;
	struct list * radius_auth = pstPageInfo->lauth;

	struct st_radiusz cq;
		memset(&cq,0,sizeof(cq));

    #if 0    
	char plot_id[30],domain_name[256],radius_server_ip[32],radius_server_port[32],radius_server_key[256],radius_server_portal[32],
			charging_server_ip[32],charging_server_port[32],charging_server_key[256],
			backup_radius_server_ip[32],backup_radius_server_port[32],backup_radius_server_key[256],backup_radius_server_portal[32],
			backup_charging_server_ip[32],backup_charging_server_port[32],backup_charging_server_key[256];

    memset(plot_id,0,sizeof(plot_id));
	memset(domain_name, 0, sizeof(domain_name));		
	memset(radius_server_ip, 0, sizeof(radius_server_ip));
	memset(radius_server_port, 0, sizeof(radius_server_port));
	memset(radius_server_key, 0, sizeof(radius_server_key));
	memset(radius_server_portal, 0, sizeof(radius_server_portal));
	memset(charging_server_ip, 0, sizeof(charging_server_ip));
	memset(charging_server_port, 0, sizeof(charging_server_port));
	memset(charging_server_key, 0, sizeof(charging_server_key));
	memset(backup_radius_server_ip, 0, sizeof(backup_radius_server_ip));
	memset(backup_radius_server_port, 0, sizeof(backup_radius_server_port));
	memset(backup_radius_server_key, 0, sizeof(backup_radius_server_key));
	memset(backup_radius_server_portal, 0, sizeof(backup_radius_server_portal));
	memset(backup_charging_server_ip, 0, sizeof(backup_charging_server_ip));
	memset(backup_charging_server_port, 0, sizeof(backup_charging_server_port));
	memset(backup_charging_server_key, 0, sizeof(backup_charging_server_key));

    #endif
	int i,j;
	
	//char a[15][256];
	/*
	for(i=0;i<15*256;i++)
	{
		**(a+i)=0;
	}*/
	//read the config
	char edit_rule[10] = "";
	char index[32] = "";
	char nodez[32];
	memset(nodez,0,32);
	char attz[32];
	memset(attz,0,32);
	char plotidz[32];
	memset(plotidz,0,32);
 
	
	int i_index ;
#if 0 
	char *pa[16];
	char *pbuf;
	pa[0]=plot_id;
	pa[1]=domain_name;
	pa[2]=radius_server_ip;
	pa[3]=radius_server_port;
	pa[4]=radius_server_key;
	pa[5]=radius_server_portal;
	pa[6]=charging_server_ip;
	pa[7]=charging_server_port;
	pa[8]=charging_server_key;
	pa[9]=backup_radius_server_ip;
	pa[10]=backup_radius_server_port;
	pa[11]=backup_radius_server_key;
	pa[12]=backup_radius_server_portal;
	pa[13]=backup_charging_server_ip;
	pa[14]=backup_charging_server_port;
	pa[15]=backup_charging_server_key;
	

   #endif
	
	cgiFormStringNoNewlines( "EDITCONF", edit_rule, 10 );
	//fprintf( stderr, "edit_rule=%s",edit_rule );
	if( !strcmp(edit_rule, "edit") )
	{
		cgiFormStringNoNewlines( "INDEX", index, 32 );
		cgiFormStringNoNewlines( "NODEZ", nodez, 32 );
		cgiFormStringNoNewlines( "ATTZ", attz, 32 );
		cgiFormStringNoNewlines( "PLOTIDZ", plotidz, 32 );


		char * ptr ;
		i_index = strtoul( index, &ptr, 0 );
		//fprintf( stderr, "i_index=%d",i_index );
		
		//fprintf( stderr, "edit_num1=%d",edit_num );
		
		//fprintf( stderr, "index=%s",index );
		FILE * pfile= NULL;
		int   revnum=0, location=-1;
		
		int temp=-1;
		int del_loc = 0;
		char * revinfo[512];
		char buf[2048];


		get_radius_struct(MULTI_RADIUS_F, nodez, ATT_Z, attz, &cq);

	}
	
	char check_chap[]="checked";
	char check_pap[]="checked";
	char backup_check_chap[]="checked";
	char backup_check_pap[]="checked";

	//fprintf(stderr,"chap_or_pap===%s",chap_or_pap);

	if(strcmp(cq.radius_server_portal,"CHAP"))
	{
		memset(check_chap,0,sizeof(check_chap));	
		//fprintf(stderr,"chap 0");
	}else
	{
		memset(check_pap,0,sizeof(check_pap));	
		//fprintf(stderr,"pap 0");
	}
		
	if(strcmp(cq.backup_radius_server_portal,"CHAP"))
	{
		memset(backup_check_chap,0,sizeof(backup_check_chap));	
		//fprintf(stderr,"chap 0");
	}else
	{
		memset(backup_check_pap,0,sizeof(backup_check_pap));	
		//fprintf(stderr,"pap 0");
	}

	fprintf(fp,"<table border=0 width=500 cellspacing=0 cellpadding=0>\n");

	fprintf(fp,"<tr height=30 align=left>");
	fprintf(fp,"<td width=250>%s</td>",search(radius_auth,"plot_idz"));
	fprintf(fp,"<td width=250>");
	fprintf(fp,"<select name=plot_id disabled>\n");

	if(strcmp(plotidz,PLOTID_ZEAO)==0)
	fprintf(fp,"<option value='%s' selected>%s</option>",PLOTID_ZEAO,MTD_N);
	else
	fprintf(fp,"<option value='%s'>%s</option>",PLOTID_ZEAO,MTD_N);

	if(strcmp(plotidz,PLOTID_ONE)==0)
	fprintf(fp,"<option value='%s' selected>1</option>",PLOTID_ONE);
	else
	fprintf(fp,"<option value='%s'>1</option>",PLOTID_ONE);

	if(strcmp(plotidz,PLOTID_TWO)==0)
	fprintf(fp,"<option value='%s' selected>2</option>",PLOTID_TWO);
	else
	fprintf(fp,"<option value='%s'>2</option>",PLOTID_TWO);

	if(strcmp(plotidz,PLOTID_THREE)==0)
	fprintf(fp,"<option value='%s' selected>3</option>",PLOTID_THREE);
	else
	fprintf(fp,"<option value='%s'>3</option>",PLOTID_THREE);

	if(strcmp(plotidz,PLOTID_FOUR)==0)
	fprintf(fp,"<option value='%s' selected>4</option>",PLOTID_FOUR);
	else
	fprintf(fp,"<option value='%s'>4</option>",PLOTID_FOUR);

	if(strcmp(plotidz,PLOTID_FIVE)==0)
	fprintf(fp,"<option value='%s' selected>5</option>",PLOTID_FIVE);
	else
	fprintf(fp,"<option value='%s'>5</option>",PLOTID_FIVE);
	
	fprintf(fp,"</select>");
	fprintf(fp,"</td>"\
				"</tr>\n");	
	fprintf(fp,"<tr height=30 align=left>");	
	fprintf(fp,"<td width=250>%s</td>",search(radius_auth,"domain_name"));	
	fprintf(fp,"<td width=250><input type=text name=domain_name value=\"%s\" size=15 readonly></td>",cq.domain_name);
	fprintf(fp,"</tr>\n");
	//radius server type
	fprintf(fp,"<tr height=30 align=left>");	
	fprintf(fp,"<td width=250>%s</td>",search(radius_auth,"radius_server_type"));
	fprintf(fp,"<td width=250>");
	fprintf(fp,"<select name=radius_server_type>\n");
	if(strcmp(cq.radius_server_type,"default")==0)
	fprintf(fp,"<option value='%s' selected>%s</option>","default","default");
	else
	fprintf(fp,"<option value='%s'>%s</option>","default","default");
	
	if(strcmp(cq.radius_server_type,RADIUS_SERVER_TYPE_RJ)==0)
		fprintf(fp,"<option value='%s' selected>%s</option>",RADIUS_SERVER_TYPE_RJ,RADIUS_SERVER_TYPE_RJ);
	else
		fprintf(fp,"<option value='%s'>%s</option>",RADIUS_SERVER_TYPE_RJ,RADIUS_SERVER_TYPE_RJ);

	if(strcmp(cq.radius_server_type,RADIUS_SERVER_TYPE_HW)==0)
		fprintf(fp,"<option value='%s' selected>%s</option>",RADIUS_SERVER_TYPE_HW,RADIUS_SERVER_TYPE_HW);
	else
		fprintf(fp,"<option value='%s'>%s</option>",RADIUS_SERVER_TYPE_HW,RADIUS_SERVER_TYPE_HW);
	
	fprintf(fp,"</select>\n");
	fprintf(fp,"</td>"\
				"</tr>\n");
	fprintf(fp,"<tr height=30 align=left>");	
	fprintf(fp,"<td width=250>%s</td>",search(radius_auth,"radius_server_ip"));
	fprintf(fp,"<td width=250><input type=text name=radius_server_ip value=\"%s\" size=15></td>",cq.radius_server_ip);
	fprintf(fp,"</tr>\n"\
				"<tr height=30 align=left>");	
	fprintf(fp,"<td width=250>%s</td>",search(radius_auth,"radius_server_port"));
	fprintf(fp,"<td width=250><input type=text name=radius_server_port value=\"%s\" size=15></td>",cq.radius_server_port);
	fprintf(fp,"</tr>\n"\
				"<tr height=30 align=left>");	
	fprintf(fp,"<td width=250>%s</td>",search(radius_auth,"radius_server_key"));
	fprintf(fp,"<td width=250><input type=text name=radius_server_key value=\"%s\" size=15></td>",cq.radius_server_key);
	fprintf(fp,"</tr>\n");	
	
	fprintf(fp,"<tr height=30 align=left>");	
	fprintf(fp, "<td width=250>%s</td>",search(radius_auth,"radius_server_portal"));	
	fprintf(fp, "<td width=100><input type=\"radio\" name=\"radius_server_portal\" value=\"CHAP\" %s>CHAP</td>\n",check_chap);
	fprintf(fp, "<td width=100><input type=\"radio\" name=\"radius_server_portal\" value=\"PAP\" %s>PAP</td>\n",check_pap);
	fprintf(fp,"</tr>\n");

	fprintf(fp,"<tr height=30 align=left><td width=250>%s</td>",search(radius_auth,"charging_server_ip"));
	fprintf(fp,"<td width=250><input type=text name=charging_server_ip value=\"%s\" size=15></td>",cq.charging_server_ip);
	fprintf(fp,"</tr>\n"\
				"<tr height=30 align=left>");
	fprintf(fp,"<td width=250>%s</td>",search(radius_auth,"charging_server_port"));
	fprintf(fp,"<td width=250><input type=text name=charging_server_port value=\"%s\" size=15></td>",cq.charging_server_port);
	fprintf(fp,"</tr>\n"\
				"<tr height=30 align=left>");
	fprintf(fp,"<td width=250>%s</td>",search(radius_auth,"charging_server_key"));
	fprintf(fp,"<td width=250><input type=text name=charging_server_key value=\"%s\" size=15></td>",cq.charging_server_key);
	fprintf(fp,"</tr>\n"\
				"<tr height=30 align=left>");	
	fprintf(fp,"<td width=250>%s</td>",search(radius_auth,"backup_radius_server_ip"));
	fprintf(fp,"<td width=250><input type=text name=backup_radius_server_ip value=\"%s\" size=15></td>",cq.backup_radius_server_ip);
	fprintf(fp,"</tr>\n"\
				"<tr height=30 align=left>");	
	fprintf(fp,"<td width=250>%s</td>",search(radius_auth,"backup_radius_server_port"));
	fprintf(fp,"<td width=250><input type=text name=backup_radius_server_port value=\"%s\" size=15></td>",cq.backup_radius_server_port);
	fprintf(fp,"</tr>\n"\
				"<tr height=30 align=left>");
	fprintf(fp,"<td width=250>%s</td>",search(radius_auth,"backup_radius_server_key"));
	fprintf(fp,"<td width=250><input type=text name=backup_radius_server_key value=\"%s\" size=15></td>",cq.backup_radius_server_key);
	fprintf(fp,"</tr>\n");	

	fprintf(fp,"<tr height=30 align=left>");	
	fprintf(fp, "<td width=250>%s</td>",search(radius_auth,"backup_radius_server_portal"));	
	fprintf(fp, "<td width=100><input type=\"radio\" name=\"backup_radius_server_portal\" value=\"CHAP\" %s>CHAP</td>\n",backup_check_chap);
	fprintf(fp, "<td width=100><input type=\"radio\" name=\"backup_radius_server_portal\" value=\"PAP\" %s>PAP</td>\n",backup_check_pap);
	fprintf(fp,"</tr>\n");

	fprintf(fp,"<tr height=30 align=left><td width=250>%s</td>",search(radius_auth,"backup_charging_server_ip"));
	fprintf(fp,"<td width=250><input type=text name=backup_charging_server_ip value=\"%s\" size=15></td>",cq.backup_charging_server_ip);
	fprintf(fp,"</tr>\n"\
				"<tr height=30 align=left>");	
	fprintf(fp,"<td width=250>%s</td>",search(radius_auth,"backup_charging_server_port"));
	fprintf(fp,"<td width=250><input type=text name=backup_charging_server_port value=\"%s\" size=15></td>",cq.backup_charging_server_port);
	fprintf(fp,"</tr>\n"\
				"<tr height=30 align=left>");	
	fprintf(fp,"<td width=250>%s</td>",search(radius_auth,"backup_charging_server_key"));
	fprintf(fp,"<td width=250><input type=text name=backup_charging_server_key value=\"%s\" size=15></td>",cq.backup_charging_server_key);
	fprintf(fp,"</tr>\n");					

	fprintf(fp,"<tr height=30 align=left><td width=250>%s</td>",search(radius_auth,"swap_octets"));
	fprintf(fp,"<td width=250><input type=checkbox name=swap_octets value=checked %s size=15></td>"\
				"</tr>\n",cq.swap_octets);

	/*
	//"<td>\n"\
	fprintf( fp, "<tr height=30 align=left>	\n<td width=250>IP end:</td>\n	<td>\n" );
				
	fprintf( fp, "<div style='border-width:1px;border-color:#a5acb2;border-style:solid;width:500px;font-size:9pt'>");
	fprintf( fp, "<input type=text name='begin_ip1' maxlength=3 class=a3 onKeyUp=\"mask(this,%s)\" onbeforepaste=mask_c() />.",search(radius_public,"ip_error"));
	fprintf( fp, "<input type=text name='begin_ip2' maxlength=3 class=a3 onKeyUp=\"mask(this,%s)\" onbeforepaste=mask_c() />.",search(radius_public,"ip_error"));
	fprintf( fp, "<input type=text name='begin_ip3' maxlength=3 class=a3 onKeyUp=\"mask(this,%s)\" onbeforepaste=mask_c() />.",search(radius_public,"ip_error"));
	fprintf( fp, "<input type=text name='begin_ip4' maxlength=3 class=a3 onKeyUp=\"mask(this,%s)\" onbeforepaste=mask_c() />",search(radius_public,"ip_error"));
	fprintf( fp, "</div>\n" );		
					
	fprintf( fp,"</td>\n<td></td>\n</tr>\n");
				//"</td>\n");
				*/
	
	fprintf(fp,	"</table>");

	fprintf(fp,"<input type=hidden name=i_index value=%d>",i_index);	
	fprintf(fp,"<input type=hidden name=NODEZ value=%s>",nodez);	
	fprintf(fp,"<input type=hidden name=ATTZ value=%s>",attz);	
	fprintf(fp,"<input type=hidden name=PLOTIDZ value=%s>",plotidz);	
	
	return 0;
}

int ip_input_is_legal(const char *str)
{
	char *endptr = NULL;
	char* endptr1 = NULL;
	char c;
	int IP,i;
	
	c = str[0];
	if (c>='0'&&c<='9'){
		IP= strtoul(str,&endptr,10);
		if(IP < 0||IP > 255)
			return 0;
		else if(((IP < 10)&&((endptr - str) > 1))||((IP < 100)&&((endptr - str) > 2))||((IP < 256)&&((endptr - str) > 3)))
			return 0;
		for(i = 0; i < 3; i++){
			if(endptr[0] == '\0'||endptr[0] != '.')
				return 0;
			else{
				endptr1 = &endptr[1];
				IP= strtoul(&endptr[1],&endptr,10); 			
				if(IP < 0||IP > 255)
					return 0;				
				else if(((IP < 10)&&((endptr - endptr1) > 1))||((IP < 100)&&((endptr - endptr1) > 2))||((IP < 256)&&((endptr - endptr1) > 3)))
					return 0;
			}
		}
		if(endptr[0] == '\0' && IP >= 0)
			return 1;
		else
			return 0;
	}
	else
		return 0;		
}

int port_input_is_legal(const char *str)
{
	const char *p = NULL;
	int port;
	
	if (NULL == str || '\0' == str[0] || '0' == str[0])
		return 0;

	for (p = str; *p; p++)
		if (*p < '0' || *p > '9')
			return 0;
		
	port = atoi(str);
	if (port < 1 || port > 65535)
		return 0;
	
	return 1;
}


