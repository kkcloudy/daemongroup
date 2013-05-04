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
* wp_addnasid.c
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

#ifndef ADD_NASID  
#define ADD_NASID

#define SUBMIT_NAME 	"submit_add_nasid"
#endif
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
static int s_addnasid_prefix_of_page( STPageInfo *pstPageInfo );
static int s_addnasid_content_of_page( STPageInfo *pstPageInfo );


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
    system("echo \"1\" > /opt/services/status/nasidvlan_status.status");
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


	

	MC_setActiveLabel( stPageInfo.pstPortalContainer->pstModuleContainer, 4 );

	MC_setPrefixOfPageCallBack( stPageInfo.pstPortalContainer->pstModuleContainer, (MC_CALLBACK)s_addnasid_prefix_of_page, &stPageInfo );
	MC_setContentOfPageCallBack( stPageInfo.pstPortalContainer->pstModuleContainer, (MC_CALLBACK)s_addnasid_content_of_page, &stPageInfo );

	
	MC_setOutPutFileHandle( stPageInfo.pstPortalContainer->pstModuleContainer, cgiOut );

	LB_changelabelName_Byindex(stPageInfo.pstPortalContainer->pstModuleContainer,search(stPageInfo.lauth,"add_nasid"),5);
	sprintf(url,"%s?UN=%s","wp_addnasid.cgi",stPageInfo.encry);
	LB_changelabelUrl_Byindex(stPageInfo.pstPortalContainer->pstModuleContainer,url,5);

	MC_setModuleContainerDomainValue( stPageInfo.pstPortalContainer->pstModuleContainer, FORM_ONSIBMIT, "return true;" );
	//可以设置为一个javascript函数,这个js函数的实现放在prefix回调函数中就可以了。
	MC_setModuleContainerDomainValue( stPageInfo.pstPortalContainer->pstModuleContainer, FORM_METHOD, "post" );


	MC_setModuleContainerDomainValue( stPageInfo.pstPortalContainer->pstModuleContainer, PUBLIC_INPUT_ENCRY, stPageInfo.encry );
	
	
	MC_setModuleContainerDomainValue( stPageInfo.pstPortalContainer->pstModuleContainer, BTN_OK_IMG, search(stPageInfo.lpublic,"img_ok") );
	MC_setModuleContainerDomainValue( stPageInfo.pstPortalContainer->pstModuleContainer, BTN_OK_SUBMIT_NAME, SUBMIT_NAME );
	
	snprintf( url, sizeof(url), "wp_nasid_byvlan.cgi?UN=%s", stPageInfo.encry );
	MC_setModuleContainerDomainValue( stPageInfo.pstPortalContainer->pstModuleContainer, BTN_CANCEL_URL, url );
	
	MC_setModuleContainerDomainValue( stPageInfo.pstPortalContainer->pstModuleContainer, LABLE_TOP_HIGHT, "25" );

	
	MC_writeHtml( stPageInfo.pstPortalContainer->pstModuleContainer );
	
	release_portal_container(&(stPageInfo.pstPortalContainer));
	
	
	return 0;
}

int Input_Nas_ID_Check(char * NasID)
{
	if( NULL == NasID )
	{
		return INPUT_NULL;
	}
	int i = 0;
	char * src = NasID ;
	while( src[i] != '\0')
		{
			if( (src[i] >= 32 && src[i] <= 126) 	
			  )
			{
				//fprintf(stderr,"boby check ok!\n");
				i++;
				continue;
			}
			else
			{
				return INPUT_CHAR_ERROR;
			}
		
		}

	return INPUT_OK;
}

static int s_addnasid_prefix_of_page( STPageInfo *pstPageInfo )
{
	FILE * pp = pstPageInfo->fp;
	struct list * portal_public = pstPageInfo->lpublic;
	struct list * portal_auth = pstPageInfo->lauth;

	
	char checkd[10],plot_id[30],nas_type[MAX_INDEX_LEN], start_point[MAX_INDEX_LEN], end_point[MAX_INDEX_LEN], nasid[MAX_INDEX_LEN],syntaxis_point[MAX_INDEX_LEN];
	int  check_ret2 = 0, check_ret3 = 0, check_ret4 = 0, check_ret5 = 0;
	
	//////////memset////////////////
	memset( checkd,0,10);
	memset( plot_id,0,30);
	memset( nas_type, 0, MAX_INDEX_LEN );
	memset( start_point, 0, MAX_INDEX_LEN );
	memset( end_point, 0, MAX_INDEX_LEN );
	memset( nasid, 0, MAX_INDEX_LEN );
	memset( syntaxis_point, 0, MAX_INDEX_LEN );

	if( cgiFormSubmitClicked(SUBMIT_NAME) == cgiFormSuccess )
	{
	    cgiFormStringNoNewlines("ifdefault",checkd, 10 );
		cgiFormStringNoNewlines("plot_id",plot_id, 30 );
		cgiFormStringNoNewlines("nas_type",nas_type, MAX_INDEX_LEN );
		cgiFormStringNoNewlines("start_point",start_point, MAX_INDEX_LEN );
		cgiFormStringNoNewlines("end_point",end_point, MAX_INDEX_LEN );
		cgiFormStringNoNewlines("nasid",nasid, MAX_INDEX_LEN );
		cgiFormStringNoNewlines("syntaxis_point",syntaxis_point, MAX_INDEX_LEN );


        if(strcmp(plot_id,"")==0)
		  cgiFormStringNoNewlines("plotid",plot_id, sizeof(plot_id));
		
		//////////check input valid///////////////////
		if( !strcmp(nas_type, "vlan") )
		{
			check_ret2 = check_input_digit_valid( start_point, 1, 4094 );
			check_ret3 = check_input_digit_valid( end_point, 1, 4094 );
			check_ret4 = Input_Nas_ID_Check(nasid);
			check_ret5 = check_input_digit_valid(syntaxis_point, 0, 99);
		}
		else if( !strcmp(nas_type, "subintf") )
		{
			check_ret4 = Input_Nas_ID_Check(nasid);
			check_ret5 = check_input_digit_valid(syntaxis_point, 0, 99);
		}
		else if( !strcmp(nas_type, "wlan") )
		{
			check_ret2 = check_input_digit_valid( start_point, 1, 4094 );
			check_ret3 = check_input_digit_valid( end_point, 1, 4094 );
			check_ret4 = Input_Nas_ID_Check(nasid);
			check_ret5 = check_input_digit_valid(syntaxis_point, 0, 99);
		}
		else if( !strcmp(nas_type, "wtp") )
		{
			check_ret2 = check_input_digit_valid( start_point, 1, 4094 );
			check_ret3 = check_input_digit_valid( end_point, 1, 4094 );
			check_ret4 = Input_Nas_ID_Check(nasid);
			check_ret5 = check_input_digit_valid(syntaxis_point, 0, 99);			
		}
		else if( !strcmp(nas_type, "ipAddress") )
		{
			check_ret2 = Input_IP_address_Check(start_point);
			check_ret3 = Input_IP_address_Check(end_point);
			check_ret4 = Input_Nas_ID_Check(nasid);
			check_ret5 = check_input_digit_valid(syntaxis_point, 0, 99);
		}

			
		if( check_ret2 == 0 && check_ret3 == 0 && check_ret4 == 0 && check_ret5 == 0 )
		{
			ShowAlert( search(portal_auth, "add_nasid_suc") );
		}
		else
		{
			ShowAlert( search(portal_public, "input_illegal") );
			return 0;  
		}
		fprintf(stderr,"nas_type=%s--start_point=%s--end_point=%s--nasid=%s--syntaxis_point=%ss",nas_type, start_point ,end_point, nasid, syntaxis_point );
		/////////////处理数据///////////////////
		FILE * fp = NULL;
		char content[256];
		memset( content, 0, 256 );
		sprintf( content, "%s=%s=%s=%s=%s\n", nas_type, start_point, end_point, nasid, syntaxis_point );
        #if 0
		fprintf(stderr,"content=%s",content);
		if( NULL != (fp = fopen(NASID_CONF_PATH,"a+")) )
		{
				fwrite(content, strlen(content), 1, fp);
				//fflush(fp);
				fclose(fp);
		}
		#endif
		char *tmpz=(char *)malloc(20);
		memset(tmpz,0,20);
		sprintf(tmpz,"%s%s",MTN_N,plot_id);
		char *tpz=(char *)malloc(64);
		memset(tpz,0,64);		

		int flag=1;
		flag=if_design_node(MULTI_NAS_F, tmpz, ATT_Z, MTD_N);
		if(flag==-1)
		{
			strcpy(tpz,MTD_N);
		}
		else if(flag==0)
		{
			if(strcmp(checkd,"yes")==0)
				strcpy(tpz,MTD_N);
			else
				//strcpy(tpz,nas_type);
				snprintf(tpz,64,"%s_%s_%s",nas_type,start_point,end_point);

		}
		
		add_eag_node_attr(MULTI_NAS_F, tmpz, ATT_Z, tpz);
        mod_eag_node(MULTI_NAS_F, tmpz, ATT_Z, tpz, NTYPE, nas_type);
	    mod_eag_node(MULTI_NAS_F, tmpz, ATT_Z, tpz, NSTART, start_point);
        mod_eag_node(MULTI_NAS_F, tmpz, ATT_Z, tpz, NEND, end_point);
        mod_eag_node(MULTI_NAS_F, tmpz, ATT_Z, tpz, NNASID, nasid);
        mod_eag_node(MULTI_NAS_F, tmpz, ATT_Z, tpz, NCOVER, syntaxis_point);
		free(tmpz);
		free(tpz);
		
		write_status_file( MULTI_NAS_STATUS, "start" );
		fprintf( pp, "<script type='text/javascript'>\n" );
		fprintf( pp, "window.location.href='wp_nasid_byvlan.cgi?UN=%s&plotid=%s';\n", pstPageInfo->encry,plot_id);
		fprintf( pp, "</script>\n" );
	}
	return 0;		
}

static int s_addnasid_content_of_page( STPageInfo *pstPageInfo )
{
	FILE * fp = pstPageInfo->fp;
	struct list * portal_public = pstPageInfo->lpublic;
	struct list * portal_auth = pstPageInfo->lauth;

	char *urlnode=(char *)malloc(20);
	memset(urlnode,0,20);
	cgiFormStringNoNewlines( "plotid", urlnode, 20 );

    char *tempz=(char *)malloc(20);
	memset(tempz,0,20);
    sprintf(tempz,"%s%s",MTN_N,urlnode);

    int flag=1;
	flag=if_design_node(MULTI_NAS_F, tempz, ATT_Z, MTD_N);
    free(tempz);

	fprintf(fp,	"<table border=0 width=280 cellspacing=0 cellpadding=0>");
	fprintf(fp, "<tr height=30 align=left>");
	fprintf(fp,"<td width=250>default</td>");
	if(flag==-1)
	fprintf(fp,"<td width=250><input type=checkbox name=ifdefault value=yes checked disabled></td>");
	else
	fprintf(fp,"<td width=250><input type=checkbox name=ifdefault value=yes></td>");
	
    fprintf(fp,"</tr>");
	fprintf(fp,"<tr height=30 align=left>");
	fprintf(fp,"<td width=250>%s</td>",search(portal_auth,"plot_idz"));
	fprintf(fp,"<td width=250>");
	fprintf(fp,"<select name=plot_id disabled>\n");
	if(strcmp(urlnode,"")==0)
	{
		fprintf(fp,"<option value='%s'>1</option>",PLOTID_ONE);
		fprintf(fp,"<option value='%s'>2</option>",PLOTID_TWO);
		fprintf(fp,"<option value='%s'>3</option>",PLOTID_THREE);
		fprintf(fp,"<option value='%s'>4</option>",PLOTID_FOUR);
		fprintf(fp,"<option value='%s'>5</option>",PLOTID_FIVE);
	}
	else
	{
	    if(strcmp(urlnode,PLOTID_ONE)==0)
			fprintf(fp,"<option value='%s' selected>1</option>",PLOTID_ONE);
		else
			fprintf(fp,"<option value='%s'>1</option>",PLOTID_ONE);

		if(strcmp(urlnode,PLOTID_TWO)==0)
			fprintf(fp,"<option value='%s' selected>2</option>",PLOTID_TWO);
		else
			fprintf(fp,"<option value='%s'>2</option>",PLOTID_TWO);

		if(strcmp(urlnode,PLOTID_THREE)==0)
			fprintf(fp,"<option value='%s' selected>3</option>",PLOTID_THREE);
		else
			fprintf(fp,"<option value='%s'>3</option>",PLOTID_THREE);

		if(strcmp(urlnode,PLOTID_FOUR)==0)
			fprintf(fp,"<option value='%s' selected>4</option>",PLOTID_FOUR);
		else
			fprintf(fp,"<option value='%s'>4</option>",PLOTID_FOUR);

		if(strcmp(urlnode,PLOTID_FIVE)==0)
			fprintf(fp,"<option value='%s' selected>5</option>",PLOTID_FIVE);
		else
			fprintf(fp,"<option value='%s'>5</option>",PLOTID_FIVE);
	}
	fprintf(fp,"</select>");
	fprintf(fp,"</td>"\
				"</tr>\n");

	fprintf(fp, "<tr height=30 align=left>"\
				"<td width=100>TPYE:</td>"\
				"<td width=180><select name=nas_type onchange=\"nas_type_change()\">"\
				"<option value=vlan>VLAN"\
				"<option value=subintf>SUBINTF"\
				"<option value=wlan>WLAN"\
				"<option value=ipAddress>IP MASK"\
				"</select>"\
				"</td>");
	fprintf(fp,	"</tr>"\

				"<tr height=30 align=left>"\
				"<td width=100>%s: </td>",search(portal_auth,"start"));

	//if(strcmp(urlnode,PLOTID_ZEAO)==0)
	//fprintf(fp,	"<td width=180><input type=text name=start_point size=15 value=\"%s\" readonly></td>",PLOTID_ONE);
	//else
	fprintf(fp,	"<td width=180><input type=text name=start_point size=15></td>");
	
	fprintf(fp, "</tr>"\
				"<tr height=30 align=left>"\
				"<td width=100>%s: </td>",search(portal_auth,"end"));

	//if(strcmp(urlnode,PLOTID_ZEAO)==0)
	//fprintf(fp,	"<td width=180><input type=text name=end_point size=15 value=\"%s\" readonly></td>",PLOTID_ONE);
	//else
	fprintf(fp,	"<td width=180><input type=text name=end_point size=15></td>");

	
	fprintf(fp, "</tr>"\

				"<tr height=30 align=left>"\
				"<td width=100>nasid:</td>"\
				"<td width=180><input type=text name=nasid size=15></td>"\
				"</tr>"\
				
				"<tr height=30 align=left>"\
				"<td width=100>%s: </td>",search(portal_auth,"syntaxis"));
	fprintf(fp,	"<td width=180><input type=text name=syntaxis_point size=15></td>"\
				"</tr>");
				
	fprintf(fp,"<tr><td colspan=2><input type=hidden name=plotid value=\"%s\"></td></tr>\n",urlnode);				
	fprintf(fp,	"</table>");

	free(urlnode);
	
	return 0;	
}


/*************************************
检查shuzi的输入合法性
*************************************/
int check_input_digit_valid(char * vlanid, int min, int max)
{
	if( vlanid == NULL || 0 == strcmp(vlanid, "") )
	{
		return -1;
	}
	
	char * src = vlanid;
	int i = 0;
	while(src[i] != '\0')
	{
		if( isdigit(src[i]) )
			i++;
		else
			return -2;
	}
	
	char * endptr = NULL;
	int i_vlan = strtoul( vlanid, &endptr, 0 );

	if( i_vlan < min || i_vlan > max )
	{
		return -3;
	}
	
	return 0;
}

/*************************************
检查vlan id的输入合法性
*************************************/
int check_nasid_valid(char * vlanid)
{
	if( vlanid == NULL )
	{
		return -1;
	}
	
	char * src = vlanid;
	int i = 0;
	while(src[i] != '\0')
	{
		if( isdigit(src[i]) )
			i++;
		else
			return -2;
	}
	
	char * endptr = NULL;
	int i_vlan = strtoul( vlanid, &endptr, 0 );
	
	if( i_vlan < 0 || i_vlan > 512 )
	{
		return -3;
	}
	
	return 0;
}





