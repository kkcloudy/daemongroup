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
* wp_multi_radius.c
*
*
* CREATOR:
* autelan.software.Network Dep. team
* shaojw@autelan.com
*
* DESCRIPTION: 
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

#ifndef MULTI_RADIUS
#define MULTI_RADIUS

#define SUBMIT_NAME "submit_multi_radius"

#define MULTI_RADIUS_STATUS_FILE_PATH	"/opt/services/status/multiradius_status.status"

#define MULTI_RADIUS_CONF_FILE_PATH	"/opt/services/conf/multiradius_conf.conf"



//#define MULTI_STATUS_FILE_PATH "/opt/services/conf/multi_radius.conf"

//#define MULTI_radius_CONF_PATH "/opt/services/conf/multi_radius.conf"

//#define MAX_INDEX_LEN 32

#endif

//#define SUBMIT_NAME "submit_nasid_by_vlan"



typedef struct{
	STPortalContainer *pstradiusContainer;
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
static int s_multiRadius_prefix_of_page( STPageInfo *pstPageInfo );
static int s_multiRadius_content_of_page( STPageInfo *pstPageInfo );



/***************************************************************
*USEAGE:	主函数
*Param:		
*Return:	
*			
*Auther:WangKe
*Date:2009-8-3
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
	init_portal_container(&(stPageInfo.pstradiusContainer));
	if( NULL == stPageInfo.pstradiusContainer )
	{
		return 0;
	}
	stPageInfo.lpublic=stPageInfo.pstradiusContainer->lpublic;//get_chain_head("../htdocs/text/public.txt");
	stPageInfo.lauth=stPageInfo.pstradiusContainer->llocal;//get_chain_head("../htdocs/text/authentication.txt");
	
	stPageInfo.fp = cgiOut;
//初始化完毕

	
//处理表单


	

	MC_setActiveLabel( stPageInfo.pstradiusContainer->pstModuleContainer, 8 );

	MC_setPrefixOfPageCallBack( stPageInfo.pstradiusContainer->pstModuleContainer, (MC_CALLBACK)s_multiRadius_prefix_of_page, &stPageInfo );
	MC_setContentOfPageCallBack( stPageInfo.pstradiusContainer->pstModuleContainer, (MC_CALLBACK)s_multiRadius_content_of_page, &stPageInfo );

	
	MC_setOutPutFileHandle( stPageInfo.pstradiusContainer->pstModuleContainer, cgiOut );

	MC_setModuleContainerDomainValue( stPageInfo.pstradiusContainer->pstModuleContainer, FORM_ONSIBMIT, "return true;" );
	//可以设置为一个javascript函数,这个js函数的实现放在prefix回调函数中就可以了。
	MC_setModuleContainerDomainValue( stPageInfo.pstradiusContainer->pstModuleContainer, FORM_METHOD, "post" );

	MC_setModuleContainerDomainValue( stPageInfo.pstradiusContainer->pstModuleContainer, PUBLIC_INPUT_ENCRY, stPageInfo.encry );
	
	
	MC_setModuleContainerDomainValue( stPageInfo.pstradiusContainer->pstModuleContainer, BTN_OK_IMG, search(stPageInfo.lpublic,"img_ok") );
	MC_setModuleContainerDomainValue( stPageInfo.pstradiusContainer->pstModuleContainer, BTN_OK_SUBMIT_NAME, SUBMIT_NAME );

	
	MC_setModuleContainerDomainValue( stPageInfo.pstradiusContainer->pstModuleContainer, LABLE_TOP_HIGHT, "25" );

	
	MC_writeHtml( stPageInfo.pstradiusContainer->pstModuleContainer );
	
	release_portal_container(&(stPageInfo.pstradiusContainer));
	
	
	return 0;
}


static int s_multiRadius_prefix_of_page( STPageInfo *pstPageInfo )
{
	char del_rule[10] = "";
	char index[32] = "";
	FILE * fp = pstPageInfo->fp;

	//if file not exist,creat it and write "start" in it
	FILE * fp1 =NULL ;
	char buf_start[]="start\n";
	if( (fp1 = fopen(MULTI_RADIUS_STATUS_FILE_PATH,"w+")) != NULL )
	{

		fwrite(buf_start,strlen(buf_start),1,fp1);
		//fprintf( stderr, "write status\n");	
		fclose(fp1);
	}
	

	
	fprintf(fp, "<style type=text/css>"\
	 	 		"#div1{ width:58px; height:18px; border:1px solid #666666; background-color:#f9f8f7;}"\
	  			"#div2{ width:56px; height:15px; padding-left:3px; padding-top:3px}"\
	  			"#link{ text-decoration:none; font-size: 12px}</style>" );


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

	if( cgiFormSubmitClicked(SUBMIT_NAME) == cgiFormSuccess )
	{
		fprintf( fp, "<script type='text/javascript'>\n" );
		fprintf( fp, "window.location.href='wp_multi_radius.cgi?UN=%s';\n", pstPageInfo->encry );
		fprintf( fp, "</script>\n" );
	}
	cgiFormStringNoNewlines( "DELRULE", del_rule, 10 );
	if( !strcmp(del_rule, "delete") )
	{
		cgiFormStringNoNewlines( "INDEX", index, 32 );
		char * ptr ;
		int i_index = strtoul( index, &ptr, 0 );
		
		//fprintf( stderr, "index=%s",index );
		FILE * fp = NULL;
		int  i, revnum=0, location=-1;
		
		int temp=-1;
		int del_loc = 0;
		char * revinfo[512];
		char buf[2048];
		if( (fp = fopen(MULTI_RADIUS_CONF_FILE_PATH,"r+")) != NULL )
		{
			//fprintf(stderr,"fp=%x",fp);//输出到出错文档?
			//读取操作
			memset( buf, 0, sizeof(buf));
			while( fgets(buf, sizeof(buf), fp) )//
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
			fclose(fp);
			fp = fopen( MULTI_RADIUS_CONF_FILE_PATH, "w+" );
			////写回操作
			for( i=0; i<revnum; i++ )
			{
				if( i == location )
					continue;
				
				fwrite( revinfo[i], strlen(revinfo[i]), 1, fp );

				free(revinfo[i]);//add by wk

			}
			fclose(fp);
		}
	}
	return 0;
}

static int s_multiRadius_content_of_page( STPageInfo *pstPageInfo )
{
	
	FILE * fp = pstPageInfo->fp;
	struct list * radius_public = pstPageInfo->lpublic;
	struct list * radius_auth = pstPageInfo->lauth;
	int i, j,cl = 0;

	char menu[21]="";
	char i_char[10]="";

	/////////////读取数据/////////////////////////
	char buf[2048];
	int por_num = 0;
	
	char domain_name[256],radius_server_ip[32],radius_server_port[32],radius_server_key[256],radius_server_portal[32],
				charging_server_ip[32],charging_server_port[32],charging_server_key[256],
				backup_radius_server_ip[32],backup_radius_server_port[32],backup_radius_server_key[256],backup_radius_server_portal[32],
				backup_charging_server_ip[32],backup_charging_server_port[32],backup_charging_server_key[256];
	
	FILE * fd = NULL ;
	
#if 1
	fprintf(fp,	"<table border=0 cellspacing=0 cellpadding=0>"\
					"<tr>"\
						"<td><a id=link href=wp_add_multi_radius.cgi?UN=%s>%s</a></td>", pstPageInfo->encry,search(radius_auth,"add_multi_radius") );
	fprintf(fp, "</tr>"\
				"</table>");
#endif

	
	fprintf(fp,	"<table border=0 width=700 cellspacing=0 cellpadding=0>"\
				"<tr height=30 align=left bgcolor=#eaeff9>");
				
	fprintf(fp, "<th width=400>%s</th>", search(radius_auth,"domain_name") );	
	fprintf(fp, "<th width=400>%s</th>", search(radius_auth,"radius_server_ip") );	
	fprintf(fp, "<th width=250>%s</th>", search(radius_auth,"radius_server_port") );	
	fprintf(fp, "<th width=400>%s</th>", search(radius_auth,"charging_server_ip") );
	fprintf(fp, "<th width=250>%s</th>", search(radius_auth,"charging_server_port") );

	fprintf(fp, "<th width=13>&nbsp;</th>"\
				"</tr>");
	//"</tr>"\
	//"<tr height=30 align=left"\
	//"<th width=80>2</th>"\

	if( NULL == (fd = fopen(MULTI_RADIUS_CONF_FILE_PATH, "r")))
	{
		fprintf(fp,	"</table>");
		return 0;//无文件不显示
	}
	int locate = 0;
	memset(buf, 0, sizeof(buf));

	
	char *pa[15];
	char *pbuf;
	pa[0]=domain_name;
	pa[1]=radius_server_ip;
	pa[2]=radius_server_port;
	pa[3]=radius_server_key;
	pa[4]=radius_server_portal;
	pa[5]=charging_server_ip;
	pa[6]=charging_server_port;
	pa[7]=charging_server_key;
	pa[8]=backup_radius_server_ip;
	pa[9]=backup_radius_server_port;
	pa[10]=backup_radius_server_key;
	pa[11]=backup_radius_server_portal;
	pa[12]=backup_charging_server_ip;
	pa[13]=backup_charging_server_port;
	pa[14]=backup_charging_server_key;


	while( (fgets( buf, sizeof(buf), fd )) != NULL )
	{
		//fprintf(stderr,"read=%s",buf);
		locate ++;
		if( !strcmp(buf,"") )
		{
			continue;
		}
	
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

		

	
		

		/*
		pbuf=strtok(buf,";");
		i = 0;
		while(pbuf)
		{
		  //printf("%s\n",s);
		  strcpy(pa[i],pbuf);
		  i++;
		  //fprintf(stderr,"pbuf%d:%s\n",i,pbuf);
		  //pbuf='\0';
		  pbuf=strtok(NULL,";");		  
		}*/

		//while()
	
		pbuf = buf;
		i = 0;
		//char temp_buf;
		//fprintf(stderr,"pbuf:%s\n",pbuf);
		j=0;
		//char temp_char[] = "aaaa";
		//pa[0]=temp_char;
		while(*pbuf != '\0')
		{

			
			//fprintf(stderr,"i:\n",i);
			//fprintf(stderr,"%d:%s\n",i,pa[i]);
			//fprintf(stderr,"pbuf:%c\n",*pbuf);
			if(*pbuf == ';' || *pbuf == '\n')
			{
				*(pa[i]+j)='\0';
				i++;
				j=0;
			}else
			{
				*(pa[i]+j)=*pbuf;
				j++;
				//strcat(pa[i],pbuf);
			}
			pbuf++;
			//fprintf(stderr,"%d:%s\n",i,pa[i]);
			//if(i==16)break;//fprintf(stderr,"pbuf%d:%s\n",i,pbuf);	
			//i++;

			//fprintf(stderr,"pbuf:%s,pa[i]:%s\n",pbuf,&pa[i]+j);		
		}
		
		//for(i=0;i<15;i++)
		//{
		//	fprintf(stderr,"%d:%s\n",i,pa[i]);
		//}
		

		
		/*
		sscanf(buf, "%[^;];%[^;];%[^;];%[^;];%[^;];%[^;];%[^;];%[^;];%[^;];%[^;];%[^;];%[^;];%[^;];%[^;];%[^\n]\n",
				domain_name,
				radius_server_ip,radius_server_port,radius_server_key,radius_server_portal,
				charging_server_ip,charging_server_port,charging_server_key,
				backup_radius_server_ip,backup_radius_server_port,backup_radius_server_key,backup_radius_server_portal,
				backup_charging_server_ip,backup_charging_server_port,backup_charging_server_key);
*/
		//fprintf(stderr,"buf=%s",buf);
		//fprintf(stderr,"index=%s-%s",a1,a2);

		memset(menu,0,21);
		strcpy(menu,"menulist");
		//sprintf(i_char,"%d",nas_num+1);
		//strcat(menu,i_char);
		
		char temp[32];
		memset(&temp,0,32);
		sprintf(temp,"%d",locate);		
		strcat(menu,temp);
		
		fprintf(fp,	"<tr height=30 align=left bgcolor=%s>", setclour(cl) );
		//fprintf(fp,	"<td>%d</td>", locate);
		
		fprintf(fp,	"<td>%s</td>", domain_name );
		fprintf(fp,	"<td>%s</td>", radius_server_ip);
		fprintf(fp,	"<td>%s</td>", radius_server_port);
		fprintf(fp,	"<td>%s</td>", charging_server_ip);
		fprintf(fp,	"<td>%s</td>", charging_server_port);
		/*
		for (i=0;i<5;i++)
		{
			fprintf(fp,	"<td>%s</td>", pa[i]);
		}
		*/


		
		//fprintf(fp,	"<td>%s</td>", );
		fprintf(fp, "<td>");
       	fprintf(fp, "<div style=\"position:relative; z-index:%d\" onmouseover=\"popMenu('%s');\" onmouseout=\"popMenu('%s');\">",(512-locate),menu,menu);
		fprintf(fp, "<img src=/images/detail.gif>"\
					"<div id=%s style=\"display:none; position:absolute; top:5px; left:0;\">",menu);
		fprintf(fp, "<div id=div1>");
		fprintf(fp, "<div id=div2 onmouseover=\"this.style.backgroundColor='#b6bdd2'\" onmouseout=\"this.style.backgroundColor='#f9f8f7'\"><a id=link href=wp_multi_radius.cgi?UN=%s&DELRULE=%s&INDEX=%d target=mainFrame>%s</a></div>", pstPageInfo->encry , "delete" , locate ,search(radius_public,"delete"));
		fprintf(fp, "<div id=div2 onmouseover=\"this.style.backgroundColor='#b6bdd2'\" onmouseout=\"this.style.backgroundColor='#f9f8f7'\"><a id=link href=wp_edit_multi_radius.cgi?UN=%s&EDITCONF=%s&INDEX=%d target=mainFrame>%s</a></div>", pstPageInfo->encry , "edit" , locate ,search(radius_public,"configure"));
		fprintf(fp, "</div>"\
		  			"</div>"\
		   			"</div>");
	   	fprintf(fp,	"</td>");
		fprintf(fp,	"</tr>");
		
		memset(buf, '\0', sizeof(buf));
		cl = !cl;
		por_num++ ;
		
		
	}
	fclose(fd);			
	fprintf(fp,	"</table>");
	return 0;	
}



