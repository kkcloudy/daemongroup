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
* wp_user_portal.c
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
#include "cgic.h"
#include <string.h>
#include <stdlib.h>
#include "ws_usrinfo.h"
#include "ws_err.h"
#include "ws_ec.h"
#include <fcntl.h>
#include <sys/wait.h>
#include "ws_portal_container.h"
#include "ws_public.h"

int getRecordById( int id, char *record, int len );
int del_portal( int id );
int ShowportalPage(struct list *lsecu,struct list *lpublic);
void portal_conf(struct list *lsecu,struct list *lpublic, int id);
int getInterfaceNum();

#define PROFILE_PATH "/opt/services/conf/portal_conf.conf"


typedef struct{
	int cur_id;
	int cur_id_state;
	char cur_id_record[256];
	char username[30];
	char ip_addr[20];
	char if_info[256];
	char ip1[4];
	char ip2[4];
	char ip3[4];
	char ip4[4];	
	char port[16];
	int reset_cur_id;
}STUserInput;

typedef struct{
//	STModuleContainer *pstModuleContainer;
	STPortalContainer *pstPortalContainer;
	struct list *lpublic;
	struct list *lauth;
	struct list *lsystem;
	char encry[BUF_LEN];
	char *username_encry;	         /*存储解密后的当前登陆用户名*/
	int iUserGroup;	//为0时表示是管理员。
	FILE *fp;
	STUserInput stUserInput;
	
	int formProcessError;
} STPageInfo;


STPageInfo stPageInfo;
STPageInfo *pstPageInfo;





struct list *lpublic;
int cgiMain()
{
	struct list *lLicense;
	struct list *lsystem;
	
	pstPageInfo = &stPageInfo;
	
	memset( pstPageInfo, 0, sizeof(STPageInfo) );
	
	lsystem= get_chain_head("../htdocs/text/system.txt");
	lLicense =get_chain_head("../htdocs/text/authentication.txt");
	lpublic=get_chain_head("../htdocs/text/public.txt");
	ShowportalPage(lLicense,lsystem);
	release(lLicense);
	release(lsystem); 
	release(lpublic); 
	return 0;
}


int ShowportalPage(struct list *lLicense,struct list *lsystem)
{

  int i;
  char buf_if_line[256];

  char *tmp;

  char id_str[10]="";
  int interfaceNum = 0;


	memset(pstPageInfo->encry,0,BUF_LEN);
    cgiFormStringNoNewlines("UN", pstPageInfo->encry, BUF_LEN); 
    pstPageInfo->username_encry=dcryption(pstPageInfo->encry); 
    
    if(pstPageInfo->username_encry==NULL)
    {
      ShowErrorPage(search(lpublic,"ill_user")); 	       /*用户非法*/
      return 0;
	}
	pstPageInfo->iUserGroup = checkuser_group(pstPageInfo->username_encry);
  cgiHeaderContentType("text/html");
 
  {
  
  	cgiFormStringNoNewlines( "record_id", id_str, sizeof(id_str) );
  	sscanf( id_str, "%d", &(pstPageInfo->stUserInput.cur_id) );
  	if( pstPageInfo->stUserInput.cur_id < 0 || pstPageInfo->stUserInput.cur_id > 7 )
  		pstPageInfo->stUserInput.cur_id = 0;
  	
  	memset( id_str, 0, sizeof(id_str) );
  	cgiFormStringNoNewlines( "reset_curid", id_str, sizeof(id_str) );
  	if( strcmp( id_str,"yes" ) == 0 )
  	{
  		pstPageInfo->stUserInput.reset_cur_id = 1;//表明要reset 当前 id
  	}
  }
 
  fprintf(cgiOut,"<html xmlns=\"http://www.w3.org/1999/xhtml\"><head>\n");
  fprintf(cgiOut,"<meta http-equiv=Content-Type content=text/html; charset=gb2312>\n");
	//下面三句话用于禁止页面缓存
	
	fprintf( cgiOut, "<META   HTTP-EQUIV=\"pragma\"   CONTENT=\"no-cache\"> \n");
	fprintf( cgiOut, "<META   HTTP-EQUIV=\"Cache-Control\"   CONTENT=\"no-cache,   must-revalidate\"> \n" );
	fprintf( cgiOut, "<META   HTTP-EQUIV=\"expires\"   CONTENT=\"Wed,   26   Feb   1997   08:21:57   GMT\">	\n");
 	  
  fprintf(cgiOut,"<title>%s</title>","Captive Protal");
  fprintf(cgiOut,"<link rel=stylesheet href=/style.css type=text/css>\n"\
  	"<style type=text/css>\n"\
  	".a3{width:30;border:0; text-align:center}\n"\
  	".dhcplis {overflow-x:hidden; overflow:auto; width: 400px; height: 150px; clip: rect( ); padding-top: 0px; padding-right: 0px; padding-bottom: 0px; padding-left: 0px}"\
  	"</style>\n"\
  "<script language=javascript src=/ip.js>\n"\
  	"</script>\n"\
  	"<script type=\"text/javascript\">\n"\
			"function popMenu(objId)\n"\
			"{\n"\
			   "var obj = document.getElementById(objId);\n"\
			   "if (obj.style.display == 'none')\n"\
			   "{\n"\
				 "obj.style.display = 'block';\n"\
			   "}\n"\
			   "else\n"\
			   "{\n"\
				 "obj.style.display = 'none';\n"\
			   "}\n"\
		   "}");
  fprintf(cgiOut,"function mysubmit(){\n"\
  			"var username=document.all.a_name.value;\n"\
 			"var pass1=document.all.a_pass1.value;\n"\
 			"var pass2=document.all.a_pass2.value;\n"\
 			"if(username==\"\")\n"\
 				"{\n"\
					"alert(\"user name is empty!\");\n"\
					"window.event.returnValue = false;\n"\
					"return false;\n"\
  					"}\n"\
  			"if(pass1==\"\")"\
 			"{\n"\
					"alert(\"password is empty!\");\n"\
					"window.event.returnValue = false;\n"\
					"return false;\n"\
  			"}\n"\
 			"if(pass1!=pass2)"\
  			"{\n"\
  				"alert(\"%s\");\n"\
  				"window.event.returnValue = false;\n"\
  				"return false;\n"\
  				"}\n"\
  		  "var value1 = document.all.ip1.value;\n"\
		  "var value2 = document.all.ip2.value;\n"\
		  "var value3 = document.all.ip3.value;\n"\
		  "var value4 = document.all.ip4.value;\n"\
		  "if(value1==\"\" ||value2==\"\" ||value3==\"\" ||value4==\"\"){\n"\
		  "alert(\"%s\");\n"\
		  "window.event.returnValue = false;\n"\
		  "return false;\n"\
		  "}\n"\
		  "if( (document.all.portal_port.value-0).toString() == 'NaN' || document.all.portal_port.value-0 > 65535 || document.all.portal_port.value-0 < 0 )\n"\
		  "{\n"\
		  "		alert(\"Port num is error!\");\n"\
	 			 "window.event.returnValue = false;\n"\
			"		return false;\n"\
			"}\n"\
		  "}",search(lsystem,"pass_incon"),search(lLicense,"ip_error"));

  fprintf(cgiOut,"</script>\n"\
  	"</head>\n"\
  	"<body>");
  
	fprintf( cgiOut, "<script type=text/javascript>\n" );
			//fprintf( cgiOut, "alert('111111');\n" );
			fprintf( cgiOut, " var reset_cur_id=false;\n" );
			if( 1 == pstPageInfo->stUserInput.reset_cur_id )
				fprintf( cgiOut, " reset_cur_id=true;\n" );
			fprintf( cgiOut, " var cur_id=%d;\n", pstPageInfo->stUserInput.cur_id );
			fprintf( cgiOut,"		function IfInfo( info )\n"\
				"		{\n"\
				"			re = /^(.*)#(.*)#(.*)#(.*)#(.*)$/;\n"\
				"			this.legal = false;\n"\
				"			if(re.test(info))\n"\
				"			{\n"\
				"				this.legal = true;\n"\
				"				this.interface = RegExp.$1;\n"\
				"				this.remark = '';\n"\
				"				this.id = RegExp.$4;\n"\
				"				if( RegExp.$2 == 'true' )\n"\
				"					this.useable = true;\n"\
				"				else\n"\
				"					this.useable = false;\n"\
				"				if( RegExp.$3 == 'has no ip' )\n"\
				"				{\n"\
				"					this.ip = '';\n" );
fprintf( cgiOut,"					this.remark = '%s'\n", search(lLicense,"portal_err_noIP") );
fprintf( cgiOut,"				}\n"\
				"				else\n"\
				"					this.ip = RegExp.$3\n" );
fprintf( cgiOut,"				if( RegExp.$4 != 'x' && this.remark == '' ){\n" );
fprintf( cgiOut,"					this.remark = '%s' + RegExp.$4;\n", search(lLicense,"portal_err_usedbyid") );
fprintf( cgiOut,"				 }\n "\
				"				if( RegExp.$5 == 'MASK_ERR' && this.remark == '' )\n" );
fprintf( cgiOut,"					this.remark = '%s';\n", search(lLicense,"portal_err_mask") );
fprintf( cgiOut,"			}\n"\
				"			return this;\n"\
				"		}\n" );
			//fprintf( cgiOut, "alert('222222');\n" );
			fprintf( cgiOut, "function AllIfInfo( _name, func )\n"\
				"		{\n"\
				"			this.allIf = new Array();\n"\
				"			this._name = _name;\n"\
				"			this.func = func;\n"\
				"			this.addIf = function( IfInfo )\n"\
				"				{\n"\
				"					if( IfInfo.legal == true )\n"\
				"						this.allIf[this.allIf.length] = IfInfo;\n"\
				"				}\n"\
				"			this.showAllIfInfo = function ()\n"\
				"				{\n"\
				"					document.write( '<input type=hidden name=input' + this._name + ' value=\\'\\' />' );\n"\
				"					document.write( '<table style=\\'text-align:left\\'>' );\n"\
				"					document.write( '<tr><th width=100px></th><th width=120px></th></tr>' );\n"\
				"					for( var i = 0; i < this.allIf.length; i++ )\n"\
				"					{\n"\
				"						//if( this.allIf[i].useable == true )\n"\
				"						//{\n"\
				"							document.write( '<tr><td>' );\n" );

fprintf( cgiOut, "						if( this.allIf[i].useable == true || ( reset_cur_id == true && this.allIf[i].id == cur_id ))\n" );
fprintf( cgiOut, "							document.write( '<input type=checkbox id=' + _name + this.allIf[i].interface + ' onclick=' + this.func + '(this,\\'' + _name + '\\') value=' + this.allIf[i].interface + ' name=interfacecheckbox />' );\n"\
				"						else\n"\
				"							document.write( '<input type=checkbox id=' + _name + this.allIf[i].interface + ' onclick=' + this.func + '(this,\\'' + _name + '\\') value=' + this.allIf[i].interface + ' disabled=disabled name=interfacecheckbox />' );\n"\
			 	"							document.write( '<label for=' + _name + this.allIf[i].interface + '>' );\n"\
			 	"							document.write( this.allIf[i].interface );\n"\
			 	"							document.write( '</label></td>' );\n"\
			 	"							document.write( '<td><label for=' + _name + this.allIf[i].interface +'>' + this.allIf[i].ip + '</label></td>' );\n"\
			 	"							document.write( '<td>' + this.allIf[i].remark + '</td></tr>' );\n"\
				"						//}\n"\
				"					}\n"\
				"					document.write( '</table>' )\n"\
				"				}\n"\
				"		}\n" );
				//fprintf( cgiOut, "alert('333333');\n" );
				fprintf( cgiOut, "function checkbox_onclick( obj, _name )\n"\
				"		{\n"\
				"			var input_if_select = document.getElementsByName( 'input'+_name )[0];\n"\
				"			if( obj.checked == true )\n"\
				"			{\n"\
				"				input_if_select.value = input_if_select.value + obj.value + ',';\n"\
				"			}\n"\
				"			else\n"\
				"			{\n"\
				"				var re = new RegExp( obj.value+',' );\n"\
				"				input_if_select.value = input_if_select.value.replace( re, '' );\n"\
				"			}\n"\
				"			//alert(input_if_select.value);\n"\
				"		}\n" );
				//fprintf( cgiOut, "alert('444444');\n" );
				fprintf( cgiOut, "var ifinfo = new AllIfInfo( 'interface', 'checkbox_onclick' );\n" );

				#if 0
				FILE* ifInfo = popen( "cp_if_info.sh", "r" );

				memset( buf_if_line, 0, sizeof(buf_if_line) );
			 	while( fgets(buf_if_line,sizeof(buf_if_line),ifInfo) )
				{
					if( strlen( buf_if_line ) > 0 )
					{
						//去掉换行符
						char *temp;
						interfaceNum++;
						for( temp = buf_if_line; *temp != 0 && *temp != 0x0d && *temp != 0x0a; temp++ );
						*temp = 0;
						fprintf(cgiOut,"ifinfo.addIf( new IfInfo( \"%s\") );\n",buf_if_line);
					}
					memset( buf_if_line, 0, sizeof(buf_if_line) );
				}
				
				pclose(ifInfo);
				#else

				infi  interf;
				interface_list_ioctl (0,&interf);
				infi * q ;
				q = interf.next;
				//use infi.if_stat to record which cp_id used
				while(q)
				{
					strcpy(q->if_stat,"x");
					q=q->next;
				}
				
				char buf_file [65536];
				char buf_if_file[65536];
				char used_cp_id[32];
				memset(buf_file,0,sizeof(buf_file));
				memset(buf_if_file,0,sizeof(buf_if_file));
				memset(used_cp_id,0,sizeof(used_cp_id));

				//read conf form conf_file
				FILE * pf_portal_conf;
				char * portal_conf_file_path = "/opt/services/conf/portal_conf.conf";
				pf_portal_conf = fopen( portal_conf_file_path, "r" );
				q = interf.next;
				if( q && pf_portal_conf )
				{
					memset( buf_file, 0, sizeof(buf_file));
					while( fgets(buf_file, sizeof(buf_file), pf_portal_conf) )
					{
						memset(buf_if_file,0,sizeof(buf_if_file));
						memset(used_cp_id,0,sizeof(used_cp_id));
						sscanf(buf_file,"%s %*s %*s %*s %*s %s",used_cp_id,buf_if_file);
						
						//fprintf( stderr, "q->if_name==buf_if_file==%s\n",buf_if_file);
						char * p_if = NULL;
						for(p_if = strtok(buf_if_file,","); p_if; p_if = strtok(NULL,","))
						{
							//fprintf( stderr, "p_if==%s,q->if_name==%s\n",p_if,q->if_name);
							q = interf.next;
							while(q)
							{
								if(!strcmp(p_if,q->if_name))
								{
									//use infi.if_stat to record which cp_id used
									strcpy(q->if_stat,used_cp_id);									
									break;
								}
								q=q->next;
							}
						}
					}
					
				}
				if(pf_portal_conf)fclose(pf_portal_conf);
				
				char buf_if [512];
				memset(buf_if,0,sizeof(buf_if));
				int is_if_used = 0;
				int is_has_ip = 0;
				q = interf.next;
				while(q)
				{
					if( !strcmp(q->if_name,"lo") )
					{
						q = q->next;
						continue;
					}

					memset(buf_if,0,sizeof(buf_if));
					strcat(buf_if,q->if_name);
					
					strcat(buf_if,"#");
					
					if(strcmp(q->if_addr,"") && !strcmp(q->if_stat,"x"))
					{
						strcat(buf_if,"true#");
					}else
					{
						strcat(buf_if,"false#");					
					}

					int i_if_mask = inet_addr(q->if_mask);
					i_if_mask = ((i_if_mask&0xAAAAAAAA)>>1) + (i_if_mask&0x55555555) ;
					i_if_mask = ((i_if_mask&0xCCCCCCCC)>>2) + (i_if_mask&0x33333333) ;
					i_if_mask = ((i_if_mask&0xF0F0F0F0)>>4) + (i_if_mask&0x0F0F0F0F) ;
					i_if_mask = ((i_if_mask&0xFF00FF00)>>8) + (i_if_mask&0x00FF00FF) ;					
					i_if_mask = ((i_if_mask&0xFFFF0000)>>16) + (i_if_mask&0x0000FFFF) ;
					
					if( strcmp(q->if_addr,"") )
					{
						//strcat(buf_if,q->if_addr);
						sprintf(buf_if,"%s%s/%d",buf_if,q->if_addr,i_if_mask);
					}else
					{
						strcat(buf_if,"has no ip");
					}
					
					//strcat(buf_if,"");
					sprintf(buf_if,"%s#%s#",buf_if,q->if_stat);
					if(i_if_mask >= 16)
					{
						strcat(buf_if,"MASK_OK");
					}else
					{
						strcat(buf_if,"MASK_ERR");
					}
					
					fprintf(cgiOut,"ifinfo.addIf( new IfInfo( \"%s\") );\n",buf_if);
					fprintf(stderr,"buf_if====%s\n",buf_if);
					q = q->next;
				}
			    free_inf(&interf);
				#endif	
				//fprintf( cgiOut, "alert( 'before  show if' );\n" );	
	fprintf( cgiOut, "</script>\n" );

  if(cgiFormSubmitClicked("submit_adduser") == cgiFormSuccess)
  {
    portal_conf(lLicense,lsystem, pstPageInfo->stUserInput.cur_id );
  }
  pstPageInfo->stUserInput.cur_id_state = getRecordById( pstPageInfo->stUserInput.cur_id, 
  								pstPageInfo->stUserInput.cur_id_record, sizeof(pstPageInfo->stUserInput.cur_id_record) );
  
//  fprintf(cgiOut,"<form action=wp_user_portal.cgi method=post encType=multipart/form-data onsubmit=mysubmit() >\n" );
  fprintf(cgiOut,"<form action=wp_user_portal.cgi method=post onsubmit=mysubmit() >\n" );
  fprintf( cgiOut, "<input type=hidden name=record_id value=%d />", pstPageInfo->stUserInput.cur_id );
  fprintf( cgiOut, "<div align=center>\n"\
  "<table width=976 border=0 cellpadding=0 cellspacing=0>\n"\
"<tr>\n"\
    "<td width=8 align=left valign=top background=/images/di22.jpg><img src=/images/youce4.jpg width=8 height=30/></td>\n"\
    "<td width=51 align=left valign=bottom background=/images/di22.jpg><img src=/images/youce33.jpg width=37 height=24/></td>\n"\
    "<td width=153 align=left valign=bottom id=%s background=/images/di22.jpg>%s</td>",search(lpublic,"title_style"),"Captive Portal");
    fprintf(cgiOut,"<td width=690px align=right valign=bottom background=/images/di22.jpg>");
	  {   
		  fprintf(cgiOut,"<table width=130 border=0 cellspacing=0 cellpadding=0>\n"\
		"<tr>\n" );
		if( (pstPageInfo->stUserInput.cur_id_state == 0 || 1 == pstPageInfo->stUserInput.reset_cur_id) && pstPageInfo->iUserGroup == 0)//如果当前id有内容，就不能提交
			fprintf( cgiOut, "<td width=62 align=center><input id=but type=submit name=submit_adduser style=background-image:url(/images/%s) value=""></td>",search(lpublic,"img_ok"));
		else
			fprintf(cgiOut,"<td width=62 align=left><a href=wp_authentication.cgi?UN=%s target=mainFrame><img src=/images/%s border=0 width=62 height=20/></a></td>",pstPageInfo->encry,search(lpublic,"img_ok"));
			
		  fprintf(cgiOut,"<td width=62 align=left><a href=wp_authentication.cgi?UN=%s target=mainFrame><img src=/images/%s border=0 width=62 height=20/></a></td>",pstPageInfo->encry,search(lpublic,"img_cancel"));

		fprintf(cgiOut,"</tr>\n"\
		"</table>");
	  } 	  
	fprintf(cgiOut,"</td>\n"\
    "<td width=74 align=right valign=top background=/images/di22.jpg><img src=/images/youce3.jpg width=31 height=30/></td>\n"\
"</tr>\n"\
"<tr>\n"\
    "<td colspan=5 align=center valign=middle><table width=976 border=0 cellpadding=0 cellspacing=0 bgcolor=#f0eff0>\n"\
	"<tr>\n"\
        "<td width=12 align=left valign=top background=/images/di888.jpg>&nbsp;</td>\n"\
        "<td width=948><table width=947 border=0 cellspacing=0 cellpadding=0>\n"\
		  "<tr height=4 valign=bottom>\n"\
              "<td width=120>&nbsp;</td>\n"\
              "<td width=827 valign=bottom><img src=/images/bottom_05.gif width=827 height=4/></td>\n"\
		  "</tr>\n"\
		  "<tr>\n"\
              "<td><table width=120 border=0 cellspacing=0 cellpadding=0>\n"\
				 "<tr height=25>\n"\
				  "<td id=tdleft>&nbsp;</td>\n"\
				"</tr>");				
		 
				fprintf(cgiOut,"<tr height=26>\n"\
					"<td align=left id=tdleft background=/images/bottom_bg.gif style=\"border-right:0\" ><font id=%s>%s<font></td>",search(lpublic,"menu_san"),search(lLicense,"captive_Portal"));	/*突出显示*/
				fprintf(cgiOut,"</tr>");
				
				
				
				//白名单
				fprintf( cgiOut, "<tr height=25><td align=left id=tdleft><a href='wp_white_list.cgi?UN=%s&portal_id=0' target=mainFrame class=top><font id=%s>%s</font></td></tr> \n",pstPageInfo->encry,search(lpublic,"menu_san"), search( lLicense, "portal_white_list") );
				//eag
				fprintf( cgiOut, "<tr height=25><td align=left id=tdleft><a href='wp_eag_conf.cgi?UN=%s' target=mainFrame class=top><font id=%s>%s</font></td></tr> \n",pstPageInfo->encry,search(lpublic,"menu_san"), search( lLicense, "eag_title") );
				//user manage
				fprintf( cgiOut, "<tr height=25><td align=left id=tdleft><a href='wp_user_manage.cgi?UN=%s' target=mainFrame class=top><font id=%s>%s</font></td></tr> \n",pstPageInfo->encry,search(lpublic,"menu_san"), search( lLicense, "user_mng") );								
				//nas
				fprintf( cgiOut, "<tr height=25><td align=left id=tdleft><a href='wp_nasid_byvlan.cgi?UN=%s' target=mainFrame class=top><font id=%s>%s</font></td></tr> \n",pstPageInfo->encry,search(lpublic,"menu_san"), search( lLicense, "nasid_management") );	
				//multi portal
				fprintf( cgiOut, "<tr height=25><td align=left id=tdleft><a href='wp_multi_portal.cgi?UN=%s' target=mainFrame class=top><font id=%s>%s</font></td></tr> \n",pstPageInfo->encry,search(lpublic,"menu_san"), search( lLicense, "multi_portal_management") );	
				//黑名单
				fprintf( cgiOut, "<tr height=25><td align=left id=tdleft><a href='wp_black_list.cgi?UN=%s' target=mainFrame class=top><font id=%s>%s</font></td></tr> \n",pstPageInfo->encry,search(lpublic,"menu_san"), search( lLicense, "portal_black_list") );	

				//ftp server
				fprintf( cgiOut, "<tr height=25><td align=left id=tdleft><a href='wp_portal_ftp.cgi?UN=%s' target=mainFrame class=top><font id=%s>%s</font></td></tr> \n",pstPageInfo->encry,search(lpublic,"menu_san"), search( lLicense, "portal_ftp") );	
				
				//multi raidus
				fprintf( cgiOut, "<tr height=25><td align=left id=tdleft><a href='wp_multi_radius.cgi?UN=%s' target=mainFrame class=top><font id=%s>%s</font></td></tr> \n",pstPageInfo->encry, search(lpublic,"menu_san"),search( lLicense, "multi_radius_management") );	

				//vlan map
				fprintf( cgiOut, "<tr height=25><td align=left id=tdleft><a href='wp_wtpwlan_map_vlan.cgi?UN=%s' target=mainFrame class=top><font id=%s>%s</font></td></tr> \n",pstPageInfo->encry, search(lpublic,"menu_san"),search( lLicense, "vlan_maping") );	
				
				for(i=0;i<4+interfaceNum;i++)
				{
				  fprintf(cgiOut,"<tr height=25>\n"\
					"<td id=tdleft>&nbsp;</td>\n"\
				  	"</tr>");
				}
			  fprintf(cgiOut,"</table>\n"\
			"</td>\n"\
			"<td align=left style=\"background-color:#ffffff; border-right:1px solid #707070; padding-left:30px\">\n"\
			  "<table border=0 cellspacing=0 cellpadding=0>");
		   		// fprintf(cgiOut,"cur_id_state = %d<br />",pstPageInfo->stUserInput.cur_id_state);
				if( 1 == pstPageInfo->stUserInput.cur_id_state )
				{
		   			i = 0;
		   			while( pstPageInfo->stUserInput.cur_id_record[i] && pstPageInfo->stUserInput.cur_id_record[i] != 0x0d && pstPageInfo->stUserInput.cur_id_record[i] != 0x0a )
		   			{
		   				i++;
		   			}
		   			pstPageInfo->stUserInput.cur_id_record[i] = 0;
		   			i=0;
					tmp = strtok(pstPageInfo->stUserInput.cur_id_record,"\t");
					while(tmp != NULL)
						{
							i++;
							if(i==2)
								strcpy(pstPageInfo->stUserInput.ip_addr,tmp);
							if( i==3 )
								strcpy( pstPageInfo->stUserInput.port, tmp );
							if(i==4)
							{
								strcpy( pstPageInfo->stUserInput.username, tmp );
							}
							if(i==6)
								strcpy( pstPageInfo->stUserInput.if_info, tmp );
						tmp = strtok(NULL,"\t");
					}
					i = 0;
					//memset(tmp,0,30);
					tmp = strtok(pstPageInfo->stUserInput.ip_addr,".");
					while(tmp != NULL)
					{
						i++;
						if(i == 1)
						{		strcpy(pstPageInfo->stUserInput.ip1,tmp);
							}
						else if(i == 2)
							{	strcpy(pstPageInfo->stUserInput.ip2,tmp);
							}
						else if(i == 3)
							{	strcpy(pstPageInfo->stUserInput.ip3,tmp);
							}
						else if(i ==4)
							{	strcpy(pstPageInfo->stUserInput.ip4,tmp);
							}
						tmp= strtok(NULL,".");
					}
				}
			  	

				 fprintf( cgiOut, "<tr height=30>\n"\
				 	"<td>ID</td>\n" );
				 fprintf( cgiOut, "<td width=680px><select name=portal_id onchange='on_id_change(this);'><option value=0>0</option>\n"\
				 		"<option value=1>1</option>\n"\
				 		"<option value=2>2</option>\n"\
				 		"<option value=3>3</option>\n"\
				 		"<option value=4>4</option>\n"\
				 		"<option value=5>5</option>\n"\
				 		"<option value=6>6</option>\n"\
				 		"<option value=7>7</option></select>\n" );
				 fprintf( cgiOut, "<script type='text/javascript'>\n" );
				 fprintf( cgiOut, "document.getElementsByName( 'portal_id' )[0].value = '%d';\n", pstPageInfo->stUserInput.cur_id );
				 fprintf( cgiOut, "function on_id_change( obj ){\n" );
				 fprintf( cgiOut, "	window.location.href='wp_user_portal.cgi?UN=%s&record_id='+obj.value;\n", pstPageInfo->encry );
				 fprintf( cgiOut, "}\n" );
				 fprintf( cgiOut, "</script>" );
				 if( 1 == pstPageInfo->stUserInput.cur_id_state && pstPageInfo->stUserInput.reset_cur_id != 1 && 0 == pstPageInfo->iUserGroup )
				 {
				 	fprintf( cgiOut, "<a href=wp_user_portal.cgi?UN=%s&record_id=%d&reset_curid=yes&anything=>%s</a>", pstPageInfo->encry, pstPageInfo->stUserInput.cur_id,search(lLicense,"portal_reset_id")  );
				}
				 fprintf( cgiOut, "</td></tr>\n" );

			 		  	fprintf(cgiOut,	"<tr height=30>\n"\
			        			"<td width=105 id=tdprompt>%s:</td>",search(lsystem,"user_na"));
			 			if( 0 == pstPageInfo->stUserInput.cur_id_state && 0 == pstPageInfo->iUserGroup )//为0表示这个还没有被设置
        				{
        					fprintf(cgiOut,"<td><input type=text name=a_name size=20></td>");
			 			}
			  			else
			  			{
			  				if( pstPageInfo->stUserInput.reset_cur_id == 1 && 0 == pstPageInfo->iUserGroup )
			  					fprintf(cgiOut,"<td><input type=text name=a_name value=\"%s\" size=20></td>", pstPageInfo->stUserInput.username);
			  				else
								fprintf(cgiOut,"<td><input type=text name=a_name value=\"%s\" disabled size=20></td>", pstPageInfo->stUserInput.username);
			  			}
						fprintf(cgiOut,"</tr>\n"\
        		  					"<tr height=30>\n"\
        							"<td id=tdprompt>%s:</td>",search(lsystem,"password"));
		  				if(  0 == pstPageInfo->stUserInput.cur_id_state && 0 == pstPageInfo->iUserGroup)
        				{
        					fprintf(cgiOut,"<td><input type=password name=a_pass1 size=21></td>\n");
        		
				  		}
						else		   		
		   				{
		   					if( 1 == pstPageInfo->stUserInput.reset_cur_id && 0 == pstPageInfo->iUserGroup )
		   						fprintf(cgiOut,"<td><input type=password name=a_pass1 size=21 /></td>\n");
		   					else
		   						fprintf(cgiOut,"<td><input type=password name=a_pass1 disabled size=21 /></td>\n");
					 	}
		 fprintf(cgiOut,"</tr>\n"\
        		  "<tr height=30>\n"\
        			"<td id=tdprompt>%s:</td>",search(lsystem,"con_pass"));
		 if(  0 == pstPageInfo->stUserInput.cur_id_state && 0 == pstPageInfo->iUserGroup )
        {
        fprintf(cgiOut,"<td width=140><input type=password name=a_pass2  size=21></td>\n");
		 	}
		 else
		 	{
		 		if( 1 == pstPageInfo->stUserInput.reset_cur_id && 0 == pstPageInfo->iUserGroup )
		 			fprintf(cgiOut,"<td width=140><input type=password name=a_pass2 size=21 /></td>\n");
		 		else
		 			fprintf(cgiOut,"<td width=140><input type=password disabled name=a_pass2 size=21 /></td>\n");
		 	}
		fprintf(cgiOut,"</tr>\n"\
        		  "<tr height=30>\n"\
        			"<td id=tdprompt>%s:</td>\n",search(lLicense,"host_ip"));
      	fprintf(cgiOut,"<td width=160>\n"\
	  			        "<div style=\"border-width:1;border-color:#a5acb2;border-style:solid;width:160;font-size:9pt\">\n");
		if(  0 == pstPageInfo->stUserInput.cur_id_state && 0 == pstPageInfo->iUserGroup )
		{
	  		fprintf(cgiOut,"<input type=text   name=ip1 value=\"\"  maxlength=3 class=a3 onKeyUp=\"mask(this,'%s');\" onbeforepaste=mask_c() />.\n",search(lLicense,"ip_error")); 
	  	    fprintf(cgiOut,"<input type=text  name=ip2 value=\"\" maxlength=3 class=a3 onKeyUp=\"mask(this,'%s');\" onbeforepaste=mask_c() />.\n" ,search(lLicense,"ip_error"));
 		    fprintf(cgiOut,"<input type=text  name=ip3 value=\"\" maxlength=3 class=a3 onKeyUp=\"mask(this,'%s');\" onbeforepaste=mask_c() />.\n",search(lLicense,"ip_error"));
		    fprintf(cgiOut,"<input type=text  name=ip4 value=\"\" maxlength=3 class=a3 onKeyUp=\"mask(this,'%s');\" onbeforepaste=mask_c() />\n",search(lLicense,"ip_error"));
		}
		else
		{
			if( 1 == pstPageInfo->stUserInput.reset_cur_id && 0 == pstPageInfo->iUserGroup )
			{
				fprintf(cgiOut,"<input type=text name=ip1 value=\"%s\"  maxlength=3 class=a3 onKeyUp=\"mask(this,'%s');\" onbeforepaste=mask_c() />.\n",pstPageInfo->stUserInput.ip1,search(lLicense,"ip_error")); 
		  	    fprintf(cgiOut,"<input type=text name=ip2 value=\"%s\" maxlength=3 class=a3 onKeyUp=\"mask(this,'%s');\" onbeforepaste=mask_c() />.\n" ,pstPageInfo->stUserInput.ip2,search(lLicense,"ip_error"));
	 		    fprintf(cgiOut,"<input type=text name=ip3 value=\"%s\" maxlength=3 class=a3 onKeyUp=\"mask(this,'%s');\" onbeforepaste=mask_c() />.\n",pstPageInfo->stUserInput.ip3,search(lLicense,"ip_error"));
			    fprintf(cgiOut,"<input type=text name=ip4 value=\"%s\" maxlength=3 class=a3 onKeyUp=\"mask(this,'%s');\" onbeforepaste=mask_c() />\n",pstPageInfo->stUserInput.ip4,search(lLicense,"ip_error"));
			}
			else
			{
				fprintf(cgiOut,"<input type=text disabled name=ip1 value=\"%s\"  maxlength=3 class=a3 onKeyUp=\"mask(this,'%s');\" onbeforepaste=mask_c() />.\n",pstPageInfo->stUserInput.ip1,search(lLicense,"ip_error")); 
		  	    fprintf(cgiOut,"<input type=text disabled name=ip2 value=\"%s\" maxlength=3 class=a3 onKeyUp=\"mask(this,'%s');\" onbeforepaste=mask_c() />.\n" ,pstPageInfo->stUserInput.ip2,search(lLicense,"ip_error"));
	 		    fprintf(cgiOut,"<input type=text disabled name=ip3 value=\"%s\" maxlength=3 class=a3 onKeyUp=\"mask(this,'%s');\" onbeforepaste=mask_c() />.\n",pstPageInfo->stUserInput.ip3,search(lLicense,"ip_error"));
			    fprintf(cgiOut,"<input type=text disabled name=ip4 value=\"%s\" maxlength=3 class=a3 onKeyUp=\"mask(this,'%s');\" onbeforepaste=mask_c() />\n",pstPageInfo->stUserInput.ip4,search(lLicense,"ip_error"));
			}
			
		}
			fprintf(cgiOut,"</div>\n"\
						"</td>\n"\
        		  "</tr>\n" );
        	if( strcmp(pstPageInfo->stUserInput.port,"default") == 0 )
        	{
        		strcpy( pstPageInfo->stUserInput.port, "" );	
        	}
			fprintf( cgiOut, "<tr><td>%s</td><td><input type=text name=portal_port maxlength=5 value='%s' %s><font color=red>(%s)</font></td></tr>",search(lLicense,"server_port"),pstPageInfo->stUserInput.port, 
						((!(0 == pstPageInfo->stUserInput.cur_id_state && 0 == pstPageInfo->iUserGroup))&&(!(1 == pstPageInfo->stUserInput.reset_cur_id && 0 == pstPageInfo->iUserGroup)))?"disabled":"", search(lLicense,"enmpty_default_port"));
			fprintf(cgiOut,"<tr>");

/****************************************************************************************************
		add by shaojunwu     2008-9-4 17:50:01
		for get  interface information
*********************************************************************************************************/	


			fprintf( cgiOut, "<td>%s</td><td>\n", search(lLicense,"interface") );
			fprintf( cgiOut,"<div class=dhcplis>");
			fprintf( cgiOut, "\n<script type='text/javascript'>\n" );
				fprintf( cgiOut, "ifinfo.showAllIfInfo();\n" );
				//fprintf( cgiOut, "alert( 'after show if' );\n" );
//根据文件 设置选中的interface

				if( 1 == pstPageInfo->stUserInput.cur_id_state && strlen( pstPageInfo->stUserInput.if_info ) > 0 )
				{
					char *iface;
					
					fprintf( cgiOut, "document.getElementsByName( 'inputinterface' )[0].value = '%s'+',';\n", pstPageInfo->stUserInput.if_info );
					//fprintf( cgiOut, "alert( document.getElementsByName( 'inputinterface' )[0].value );\n" );
					iface = strtok( pstPageInfo->stUserInput.if_info, "," );
					while( iface )
					{
						fprintf( cgiOut, "document.getElementById( 'interface%s' ).checked = 'checked';\n", iface );
						iface = strtok( NULL, "," );
					}
					
					//所有的checkbox都不能选择
					if( 1 != pstPageInfo->stUserInput.reset_cur_id  || pstPageInfo->iUserGroup != 0 )
					{
						fprintf( cgiOut, "var ifCheckBoxs = document.getElementsByName( 'interfacecheckbox' );\n"\
							"	for( var j=0;j < ifCheckBoxs.length; j++ ){\n"\
							"	ifCheckBoxs[j].disabled = true;\n"\
							"}\n" );
					}
#if 0					
					else
					{
						fprintf( cgiOut, "var ifCheckBoxs = document.getElementsByName( 'interfacecheckbox' );\n"\
							"	for( var j=0;j < ifCheckBoxs.length; j++ ){\n"\
							"	ifCheckBoxs[j].disabled = false;\n"\
							"}\n" );
					}
#endif					
				}
				fprintf( cgiOut, "</script>\n");
					
				fprintf( cgiOut, "</div></td></tr><tr>\n" );
/*********************************************************************************************
		end of add by shaojunwu

*********************************************************************************************/

        		fprintf(cgiOut,"<td colspan=3><input type=hidden name=UN value=%s></td>",pstPageInfo->encry);

        		  fprintf(cgiOut,"</tr></tr></table>");

	fprintf(cgiOut,"</td>\n"\
		  "</tr>\n"\
		  "<tr height=4 valign=top>\n"\
              "<td width=120 height=4 align=right valign=top><img src=/images/bottom_07.gif width=1 height=10/></td>\n"\
              "<td width=827 height=4 valign=top bgcolor=#FFFFFF><img src=/images/bottom_06.gif width=827 height=15/></td>\n"\
		  "</tr>\n"\
		"</table>\n"\
	  "</td>\n"\
	  "<td width=15 background=/images/di999.jpg>&nbsp;</td>\n"\
	"</tr>\n"\
  "</table></td>\n"\
"</tr>\n"\
"<tr>\n"\
    "<td colspan=3 align=left valign=top background=/images/di777.jpg><img src=/images/di555.jpg width=61 height=62/></td>\n"\
  "<td align=left valign=top background=/images/di777.jpg>&nbsp;</td>\n"\
    "<td align=left valign=top background=/images/di777.jpg><img src=/images/di666.jpg width=74 height=62/></td>\n"\
"</tr>\n"\
"</table>\n"\
"</div>\n"\
"</form>\n"\
"</body>\n");

fprintf( cgiOut, "</html>");

if( 1 == pstPageInfo->stUserInput.reset_cur_id )
	del_portal( pstPageInfo->stUserInput.cur_id );


return 0;
}

int getInterfaceNum()
{
	int iRet = 0;
	char buf_if_line[256];
	
	FILE* ifInfo = popen( "cp_if_info.sh", "r" );
#if 1
	memset( buf_if_line, 0, sizeof(buf_if_line) );
 	while( fgets(buf_if_line,sizeof(buf_if_line),ifInfo) )
	{
		if( strlen( buf_if_line ) > 0 )
		{
			iRet++;
		}
		memset( buf_if_line, 0, sizeof(buf_if_line) );
	}
#endif					
	pclose(ifInfo);
	
	return iRet;
}

void portal_conf(struct list *lLicense,struct list *lsystem, int cur_id )
{
  char name[N],pass1[N],pass2[N];    
  char ip1[4],ip2[4],ip3[4],ip4[4];
  char ip[16];
  char port[16]="";
    int rec;
    int ret;
    
  memset(name,0,N);					 /*清空临时变量*/
  memset(pass1,0,N);
  memset(pass2,0,N);
  char interfaces[65536];
  char *command1 = (char*)malloc(PATH_LENG);
  char *command2 = (char*)malloc(PATH_LENG);
  memset(command1,0,PATH_LENG);
  memset(command2,0,PATH_LENG);
  cgiFormStringNoNewlines("a_name",name,N);
  cgiFormStringNoNewlines("a_pass1",pass1,N);
  cgiFormStringNoNewlines("a_pass2",pass2,N);
  cgiFormStringNoNewlines("ip1",ip1,N);
  cgiFormStringNoNewlines("ip2",ip2,N);
  cgiFormStringNoNewlines("ip3",ip3,N);
  cgiFormStringNoNewlines("ip4",ip4,N);
  cgiFormStringNoNewlines("portal_port",port,sizeof(port));  
  cgiFormStringNoNewlines("inputinterface",interfaces,sizeof(interfaces));
  //去掉interface最后的‘,’
  if( interfaces[strlen(interfaces)-1] == ',' )
 	interfaces[strlen(interfaces)-1] = '\0';
  
  strcpy(ip,ip1);
  strcat(ip,".");
  strcat(ip,ip2);
  strcat(ip,".");
  strcat(ip,ip3);
  strcat(ip,".");
  strcat(ip,ip4);
  //fprintf(cgiOut,"%s,%s,%s,%s",name,pass1,pass2,ip);
//  sprintf(command1,"cp_create_profile.sh %d ", cur_id );
//  strcat(command1,ip);
//  strcat(command1," ");
//  strcat(command1,name);
//  strcat(command1," ");
//  strcat(command1,pass1);
	sprintf( command1,"sudo cp_create_profile.sh %d %s %s %s %s >/dev/null 2>&1", 
							cur_id, ip, (port[0])?port:"default",name, pass1 );
  //当设置一个大于65535的端口时，将使用默认端口。
  //当port大于65535时不使用端口，针对eag修改的脚本。

	
  rec = system(command1);
 	ret = WEXITSTATUS(rec);

	switch( ret )
	{
		case 0:
			break;
		case 3:
			ShowAlert( search(lLicense,"portal_err_used_id") );
			break;
		case 4:
			ShowAlert( search(lLicense,"portal_err_ip") );
			break;
		case 5:
			ShowAlert( search(lLicense,"portal_err_used_user") );
			break;
		case 1:
		case 2:
		default:
			ShowAlert( search(lLicense,"portal_err_undefine") );
			break;
	}
	if( 0 != rec )
		return;

 sprintf(command2,"sudo cp_apply_if.sh %d %s >/dev/null 2>&1", cur_id, interfaces );
 
// strcat(command2,interfaces);	

 
 rec = system(command2);
  ret = WEXITSTATUS(rec);
 if(rec == 0)
	 {
	  ShowAlert(search(lLicense,"log_suc"));
	 }
  else{
  			ShowAlert(search(lLicense,interfaces));
		  ShowAlert(search(lLicense,"log_err"));
	 }


}

int del_portal( int id )
{
	char command[256];
	
	sprintf( command, "sudo cp_del_portal_id.sh %d", id );
	
	int status = system(command); 	 
	int ret = WEXITSTATUS(status);	
	
	return ret;
}


int getRecordById( int id, char *record, int len )
{
	FILE *db;
	
	db= fopen( PROFILE_PATH,"r");
	
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

