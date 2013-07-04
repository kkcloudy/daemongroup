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
* wp_log_info.c
*
*
* CREATOR:
* autelan.software.Network Dep. team
* zhouym@autelan.com
*
* DESCRIPTION:
* system function for syslog info config
*
*
*******************************************************************************/
#include <stdio.h>
#include "cgic.h"
#include <string.h>
#include <stdlib.h>
#include <sys/wait.h>
#include "ws_usrinfo.h"
#include "ws_err.h"
#include "ws_ec.h"
#include "ws_log_conf.h"


int ShowExportConfPage(struct list *lpublic, struct list *lsystem);     /*m代表加密后的字符串*/

int ruler_info(char * name,char *file_name,struct list *lpublic,char *encry,char *value,char *color);

int mod_info(char * name,char *file_name,struct list *lpublic,char *encry,char *value);

int get_state(char *state);

int cgiMain()
{
	struct list *lpublic;	/*解析public.txt文件的链表头*/
	struct list *lsystem;	  /*解析system.txt文件的链表头*/	
	lpublic=get_chain_head("../htdocs/text/public.txt");
	lsystem=get_chain_head("../htdocs/text/system.txt");

	ShowExportConfPage(lpublic,lsystem);
  	release(lpublic);  
  	release(lsystem);

	return 0;
}

int ShowExportConfPage(struct list *lpublic, struct list *lsystem)
{ 
	 
	  char *encry=(char *)malloc(BUF_LEN);				
	  char *str;

	  char selename[N];    /*下拉框内容*/
      memset(selename,0,N); 

	  char start_on[N];   //捕获状态
	  memset(start_on,0,N);

	  char stop[N];
	  memset(stop,0,N);

	  char state[N];
	  memset(state,0,N);
	  
      int ret,op_ret;
	  char file_name[128];  //读取文件
      memset(file_name,0,128); 
	  
	  char rule_name[128];  //读取文件
      memset(rule_name,0,128);
   
      char addn[N]="";

	  char syslog_status[10];	
	  memset( syslog_status, 0, 10 );
	  
	  //int j;
      //FILE * fp;
	  
	  char log_encry[BUF_LEN];
	 
	  if(cgiFormSubmitClicked("log_config") != cgiFormSuccess)
	  {
		  memset(encry,0,BUF_LEN);
		cgiFormStringNoNewlines("UN", encry, BUF_LEN); 
		str=dcryption(encry);
		if(str==NULL)
		{
		  ShowErrorPage(search(lpublic,"ill_user"));		   /*用户非法*/
		  return 0;
		}
		strcpy(addn,str);
		memset(log_encry,0,BUF_LEN);				   /*清空临时变量*/
	  }
	  else
	  	{
    	cgiFormStringNoNewlines("encry_import",log_encry,BUF_LEN);
        str=dcryption(log_encry);
        if(str==NULL)
        {
          ShowErrorPage(search(lpublic,"ill_user")); 	 /*用户非法*/
  	    }
  		 strcpy(addn,str);
  	     memset(log_encry,0,BUF_LEN);                   /*清空临时变量*/
  	
    	}
	  cgiFormStringNoNewlines("encry_import",log_encry,BUF_LEN);
	  cgiFormStringNoNewlines("UN", encry, BUF_LEN); 
	  /***********************2008.5.26*********************/
	  cgiHeaderContentType("text/html");
	  fprintf(cgiOut,"<html xmlns=\"http://www.w3.org/1999/xhtml\"><head>");
	  fprintf(cgiOut,"<meta http-equiv=Content-Type content=text/html; charset=gb2312>");
	  fprintf(cgiOut,"<title>%s</title>",search(lpublic,"log_info"));
	  fprintf(cgiOut,"<link rel=stylesheet href=/style.css type=text/css>"\
		"<style type=text/css>"\
		".a3{width:30;border:0; text-align:center}"\
		"</style>"\
		"</head>"\
		"<body>");
       #if 0
	   if(cgiFormSubmitClicked("reboot") == cgiFormSuccess)
    	{
    	ret=restart_syslog();
		if(ret==0)
		ShowAlert(search(lpublic,"oper_succ"));
	    else
		ShowAlert(search(lpublic,"oper_fail"));
    	}

	 
	  cgiFormStringNoNewlines("start",start_on,N);
	  if(strcmp(start_on,"true")==0)
	  	{
		memset(state,0,N);
		sprintf(state,"sudo cli.sh %s","on");
        op_ret=system(state);
        if(op_ret==0)
			ShowAlert(search(lpublic,"oper_succ"));
		else
			ShowAlert(search(lpublic,"oper_fail"));
	  	}	

	   cgiFormStringNoNewlines("stop",stop,N);
	   if(strcmp(stop,"true")==0)
	  	{
        memset(state,0,N);
		sprintf(state,"sudo cli.sh %s","off");
        op_ret=system(state);
        if(op_ret==0)
			ShowAlert(search(lpublic,"oper_succ"));
		else
			ShowAlert(search(lpublic,"oper_fail"));
	  	}	
	 //////////////////////////操作状态
      memset(start_on,0,N);
      cgiFormStringNoNewlines("state",start_on,N);
	  if(strcmp(start_on,"start")==0)
	  	{
		memset(state,0,N);		
    	sprintf(state,"%s %s",S_PATH,S_START); // 开启 syslog 服务
       
	  	}	
	   else if(strcmp(start_on,"stop")==0)
	  	{
		memset(state,0,N);	    

    	sprintf(state,"%s %s",S_PATH,S_STOP);  //停止 syslog 服务
        op_ret=system(state);
		
	  	}	
	  #endif
	 
	  fprintf(cgiOut,"<form method=post encType=multipart/form-data>"\
	  "<div align=center>"\
	  "<table width=976 border=0 cellpadding=0 cellspacing=0>");  
	  fprintf(cgiOut,"<tr>"\
		"<td width=8 align=left valign=top background=/images/di22.jpg><img src=/images/youce4.jpg width=8 height=30/></td>"\
		"<td width=51 align=left valign=bottom background=/images/di22.jpg><img src=/images/youce33.jpg width=37 height=24/></td>"\
	  "<td width=153 align=left valign=bottom id=%s background=/images/di22.jpg>%s</td>",search(lpublic,"title_style"),search(lsystem,"sys_function"));
		fprintf(cgiOut,"<td width=690 align=right valign=bottom background=/images/di22.jpg>");
	  
	    fprintf(cgiOut,"<input type=hidden name=UN value=%s />",encry);
	 
			  fprintf(cgiOut,"<table width=130 border=0 cellspacing=0 cellpadding=0>");  
		fprintf(cgiOut,"<tr>"\
		"<td width=62 align=center><input id=but type=submit name=log_config style=background-image:url(/images/%s) value=""></td>",search(lpublic,"img_ok"));
	
		if(cgiFormSubmitClicked("log_config") != cgiFormSuccess)
		{
     		  fprintf(cgiOut,"<td width=62 align=left><a href=wp_log_info.cgi?UN=%s target=mainFrame><img src=/images/%s border=0 width=62 height=20/></a></td>",encry,search(lpublic,"img_cancel"));

		}
     	else
     	{
     		  fprintf(cgiOut,"<td width=62 align=left><a href=wp_log_info.cgi?UN=%s target=mainFrame><img src=/images/%s border=0 width=62 height=20/></a></td>",log_encry,search(lpublic,"img_cancel"));

		}
		fprintf(cgiOut,"</tr>"\
		"</table>"); 
	 
	 
	  
	
		fprintf(cgiOut,"</td>"\
		"<td width=74 align=right valign=top background=/images/di22.jpg><img src=/images/youce3.jpg width=31 height=30/></td>"\
	"</tr>"\
	"<tr>"\
		"<td colspan=5 align=center valign=middle><table width=976 border=0 cellpadding=0 cellspacing=0 bgcolor=#f0eff0>"\
		"<tr>"); 
		
			fprintf(cgiOut,"<td width=12 align=left valign=top background=/images/di888.jpg>&nbsp;</td>"\
			"<td width=948><table width=947 border=0 cellspacing=0 cellpadding=0>"); 
			  fprintf(cgiOut,"<tr height=4 valign=bottom>"\
				  "<td width=120>&nbsp;</td>"\
				  "<td width=827 valign=bottom><img src=/images/bottom_05.gif width=827 height=4/></td>"\
			  "</tr>"\
			  "<tr>"\
				  "<td>");
			  fprintf(cgiOut,"<table width=120 border=0 cellspacing=0 cellpadding=0>");

			  fprintf(cgiOut,"<tr height=25>"\
				  "<td id=tdleft>&nbsp;</td>"\
				"</tr>"); 
                     /*管理员*/
                     if(checkuser_group(addn)==0)
					{
						if(cgiFormSubmitClicked("log_config") != cgiFormSuccess)
						{					 
							fprintf(cgiOut,"<tr height=25>"\
							"<td align=left id=tdleft><a href=wp_sysinfo.cgi?UN=%s target=mainFrame class=top><font id=%s>%s</font></a></td>",encry,search(lpublic,"menu_san"),search(lsystem,"sys_infor"));
							fprintf(cgiOut,"</tr>"\
							"<tr height=25>"\
							"<td align=left id=tdleft><a href=wp_sysconfig.cgi?UN=%s target=mainFrame class=top><font id=%s>%s</font></a></td>",encry,search(lpublic,"menu_san"),search(lsystem,"sys_config"));
							fprintf(cgiOut,"</tr>"\
							"<tr height=25>"\
							"<td align=left id=tdleft><a href=wp_impconf.cgi?UN=%s target=mainFrame class=top><font id=%s>%s</font></a></td>",encry,search(lpublic,"menu_san"),search(lsystem,"import_config"));
							fprintf(cgiOut,"</tr>");
							fprintf(cgiOut,"<tr height=25>"\
							"<td align=left id=tdleft><a href=wp_export.cgi?UN=%s target=mainFrame class=top><font id=%s>%s</font></a></td>",encry,search(lpublic,"menu_san"),search(lsystem,"export_config"));
							fprintf(cgiOut,"</tr>");						
							//新增条目
							fprintf(cgiOut,"<tr height=25>"\
							"<td align=left id=tdleft><a href=wp_version_upgrade.cgi?UN=%s target=mainFrame class=top><font id=%s>%s</font></a></td>",encry,search(lpublic,"menu_san"),search(lpublic,"version_up"));
							fprintf(cgiOut,"</tr>");

							//boot upgrade 
							fprintf(cgiOut,"<tr height=25>"\
							"<td align=left id=tdleft><a href=wp_boot_upgrade.cgi?UN=%s target=mainFrame class=top><font id=%s>%s</font></a></td>",encry,search(lpublic,"menu_san"),search(lsystem,"boot_item"));
							fprintf(cgiOut,"</tr>");

							fprintf(cgiOut,"<tr height=26>"\
							"<td align=left id=tdleft background=/images/bottom_bg.gif style=\"border-right:0\"><font id=%s>%s</font></td>",search(lpublic,"menu_san"),search(lpublic,"log_info"));  /*突出显示*/
							fprintf(cgiOut,"</tr>");

							fprintf(cgiOut,"<tr height=25>"\
							"<td align=left id=tdleft><a href=wp_login_limit.cgi?UN=%s target=mainFrame class=top><font id=%s>%s</font></a></td>",encry,search(lpublic,"menu_san"),search(lpublic,"l_user"));
							fprintf(cgiOut,"</tr>");

							//新增时间条目
							fprintf(cgiOut,"<tr height=25>"\
							"<td align=left id=tdleft><a href=wp_showtime.cgi?UN=%s target=mainFrame class=top><font id=%s>%s</font></a></td>",encry,search(lpublic,"menu_san"),search(lsystem,"systime"));
							fprintf(cgiOut,"</tr>");

							//新增条目
							fprintf(cgiOut,"<tr height=25>"\
							"<td align=left id=tdleft><a href=wp_ntp.cgi?UN=%s target=mainFrame class=top><font id=%s>%s</font></a></td>",encry,search(lpublic,"menu_san"),search(lpublic,"ntp_s"));
							fprintf(cgiOut,"</tr>");
							//新增pppoe条目
							fprintf(cgiOut,"<tr height=25>"\
							"<td align=left id=tdleft><a href=wp_pppoe_server.cgi?UN=%s target=mainFrame class=top><font id=%s>%s</font></a></td>",encry,search(lpublic,"menu_san"),"PPPOE");
							fprintf(cgiOut,"</tr>");
							
							//新增时间条目
							fprintf(cgiOut,"<tr height=26>"\
							"<td align=left id=tdleft><a href=wp_webservice.cgi?UN=%s target=mainFrame class=top><font id=%s>%s</font></a></td>",encry,search(lpublic,"menu_san"),search(lpublic,"web_service"));
							fprintf(cgiOut,"</tr>");
						}
						else if(cgiFormSubmitClicked("log_config") == cgiFormSuccess)			
						{
							fprintf(cgiOut,"<tr height=25>"\
							"<td align=left id=tdleft><a href=wp_sysinfo.cgi?UN=%s target=mainFrame class=top><font id=%s>%s</font></a></td>",log_encry,search(lpublic,"menu_san"),search(lsystem,"sys_infor"));
							fprintf(cgiOut,"</tr>"\
							"<tr height=25>"\
							"<td align=left id=tdleft><a href=wp_sysconfig.cgi?UN=%s target=mainFrame class=top><font id=%s>%s</font></a></td>",log_encry,search(lpublic,"menu_san"),search(lsystem,"sys_config"));
							fprintf(cgiOut,"</tr>"\
							"<tr height=25>"\
							"<td align=left id=tdleft><a href=wp_impconf.cgi?UN=%s target=mainFrame class=top><font id=%s>%s</font></a></td>",log_encry,search(lpublic,"menu_san"),search(lsystem,"import_config"));
							fprintf(cgiOut,"</tr>");
							fprintf(cgiOut,"<tr height=25>");
							fprintf(cgiOut,"<td align=left id=tdleft><a href=wp_export.cgi?UN=%s target=mainFrame class=top><font id=%s>%s</font></a></td>",log_encry,search(lpublic,"menu_san"),search(lsystem,"export_config"));;
							fprintf(cgiOut,"</tr>");					  						
							//新增条目
							fprintf(cgiOut,"<tr height=25>"\
							"<td align=left id=tdleft><a href=wp_version_upgrade.cgi?UN=%s target=mainFrame class=top><font id=%s>%s</font></a></td>",log_encry,search(lpublic,"menu_san"),search(lpublic,"version_up"));
							fprintf(cgiOut,"</tr>");

							//boot upgrade 
							fprintf(cgiOut,"<tr height=25>"\
							"<td align=left id=tdleft><a href=wp_boot_upgrade.cgi?UN=%s target=mainFrame class=top><font id=%s>%s</font></a></td>",log_encry,search(lpublic,"menu_san"),search(lsystem,"boot_item"));
							fprintf(cgiOut,"</tr>");


							fprintf(cgiOut,"<tr height=26>"\
							"<td align=left id=tdleft background=/images/bottom_bg.gif style=\"border-right:0\"><font id=%s>%s</font></td>",search(lpublic,"menu_san"),search(lpublic,"log_info"));  /*突出显示*/
							fprintf(cgiOut,"</tr>");


							//新增条目
							fprintf(cgiOut,"<tr height=25>"\
							"<td align=left id=tdleft><a href=wp_login_limit.cgi?UN=%s target=mainFrame class=top><font id=%s>%s</font></a></td>",log_encry,search(lpublic,"menu_san"),search(lpublic,"l_user"));
							fprintf(cgiOut,"</tr>");

							//新增时间条目
							fprintf(cgiOut,"<tr height=25>"\
							"<td align=left id=tdleft><a href=wp_showtime.cgi?UN=%s target=mainFrame class=top><font id=%s>%s</font></a></td>",encry,search(lpublic,"menu_san"),search(lsystem,"systime"));
							fprintf(cgiOut,"</tr>");

							//新增条目
							fprintf(cgiOut,"<tr height=25>"\
							"<td align=left id=tdleft><a href=wp_ntp.cgi?UN=%s target=mainFrame class=top><font id=%s>%s</font></a></td>",log_encry,search(lpublic,"menu_san"),search(lpublic,"ntp_s"));
							fprintf(cgiOut,"</tr>");
							//新增pppoe条目
							fprintf(cgiOut,"<tr height=25>"\
							"<td align=left id=tdleft><a href=wp_pppoe_server.cgi?UN=%s target=mainFrame class=top><font id=%s>%s</font></a></td>",log_encry,search(lpublic,"menu_san"),"PPPOE");
							fprintf(cgiOut,"</tr>");
							
							//新增时间条目
							fprintf(cgiOut,"<tr height=26>"\
							"<td align=left id=tdleft><a href=wp_webservice.cgi?UN=%s target=mainFrame class=top><font id=%s>%s</font></a></td>",encry,search(lpublic,"menu_san"),search(lpublic,"web_service"));
							fprintf(cgiOut,"</tr>");
						}
					}
					else
					{

						if(cgiFormSubmitClicked("log_config") != cgiFormSuccess)
						{					 
							fprintf(cgiOut,"<tr height=25>"\
							"<td align=left id=tdleft><a href=wp_sysinfo.cgi?UN=%s target=mainFrame class=top><font id=%s>%s</font></a></td>",encry,search(lpublic,"menu_san"),search(lsystem,"sys_infor"));
							fprintf(cgiOut,"</tr>");							



							//新增时间条目
							fprintf(cgiOut,"<tr height=25>"\
							"<td align=left id=tdleft><a href=wp_showtime.cgi?UN=%s target=mainFrame class=top><font id=%s>%s</font></a></td>",encry,search(lpublic,"menu_san"),search(lsystem,"systime"));
							fprintf(cgiOut,"</tr>");
						}
						else if(cgiFormSubmitClicked("log_config") == cgiFormSuccess)			
						{
							fprintf(cgiOut,"<tr height=25>"\
							"<td align=left id=tdleft><a href=wp_sysinfo.cgi?UN=%s target=mainFrame class=top><font id=%s>%s</font></a></td>",log_encry,search(lpublic,"menu_san"),search(lsystem,"sys_infor"));
							fprintf(cgiOut,"</tr>");					


							//新增时间条目
							fprintf(cgiOut,"<tr height=25>"\
							"<td align=left id=tdleft><a href=wp_showtime.cgi?UN=%s target=mainFrame class=top><font id=%s>%s</font></a></td>",encry,search(lpublic,"menu_san"),search(lsystem,"systime"));
							fprintf(cgiOut,"</tr>");

						}
					}
					#if 0
					for(j=0;j<2;j++)
					{
					    fprintf(cgiOut,"<tr height=25>"\
						"<td id=tdleft>&nbsp;</td>"\
					  "</tr>");
					}
                    #endif
                  
					fprintf(cgiOut,"</table>"); 
					fprintf(cgiOut,"</td>"\
					"<td  align=left style=\"background-color:#ffffff; border-right:1px solid #707070; padding-left:30px\">");

					fprintf(cgiOut,"<table width=400 border=0 cellspacing=0 cellpadding=0>"); 

           ///////////////////////
            #if 0
           fprintf(cgiOut,"<tr><td>\n");
		   
		   ret=get_state("start");		   
           
		   if(ret==0)
		   	{
		        fprintf(cgiOut,"%s:  %s&nbsp;|&nbsp;<a href=wp_log_info.cgi?UN=%s&state=%s target=mainFrame><font color=blue>%s</font></a>",search(lpublic,"log_state"),search(lpublic,"log_off"),encry,"start",search(lpublic,"log_start_n"));
    		    if(( fp = fopen( STATUS_FPATH ,"w+"))!=NULL)
                {
               
	            memset( syslog_status, 0, 10 );
            	strcpy( syslog_status, "stop" );
            	fwrite( syslog_status, strlen(syslog_status), 1, fp );
            	fflush( fp );
            	fclose(fp);
                }    
		    }
		   else if(ret==1)
		   	{
		       fprintf(cgiOut,"%s:  %s&nbsp;|&nbsp;<a href=wp_log_info.cgi?UN=%s&state=%s target=mainFrame><font color=blue>%s</font></a>",search(lpublic,"log_state"),search(lpublic,"log_on"),encry,"stop",search(lpublic,"log_stop_n"));		   
               if(( fp = fopen( STATUS_FPATH ,"w+"))!=NULL)
                {
                memset( syslog_status, 0, 10 );
            	strcpy( syslog_status, "start" );
            	fwrite( syslog_status, strlen(syslog_status), 1, fp );
            	fflush( fp );
            	fclose(fp);
                }    
		   }

		   	
		   
		   fprintf(cgiOut,"</td>");
		   fprintf(cgiOut,"</tr>");

           
		   fprintf(cgiOut,"<tr height=7><td></td></tr>");
            	 
		   //////////////////////get state 
              fprintf(cgiOut,"<tr>\n");
		      fprintf(cgiOut,"<td>\n");
					 fprintf(cgiOut,"%s:",search(lpublic,"log_vty"));

			  int vty_flag=-1;
			  vty_flag=get_cli_syslog_state();

			  if(vty_flag==1)
			  {
					 fprintf(cgiOut,"&nbsp;&nbsp;&nbsp;");
                     fprintf(cgiOut,"<a href=wp_log_info.cgi?UN=%s&start=%s target=mainFrame>%s</a>",encry,"true",search(lpublic,"log_start"));
					 fprintf(cgiOut,"&nbsp;&nbsp;|&nbsp;");
                     fprintf(cgiOut,"<a href=wp_log_info.cgi?UN=%s&stop=%s target=mainFrame><font color=blue>%s</font></a>",encry,"true",search(lpublic,"log_stop"));
			  }
			  else if(vty_flag==0)
			  {
					 fprintf(cgiOut,"&nbsp;&nbsp;&nbsp;");
                     fprintf(cgiOut,"<a href=wp_log_info.cgi?UN=%s&stop=%s target=mainFrame>%s</a>",encry,"true",search(lpublic,"log_stop"));
					 fprintf(cgiOut,"&nbsp;&nbsp;|&nbsp;");
                     fprintf(cgiOut,"<a href=wp_log_info.cgi?UN=%s&start=%s target=mainFrame><font color=blue>%s</font></a>",encry,"true",search(lpublic,"log_start"));

			  }
			  fprintf(cgiOut,"</td>\n");
			  fprintf(cgiOut,"</tr>\n");
					 
		   ////////////////////////

		  fprintf(cgiOut,"<tr height=7><td></td></tr>");

				     fprintf(cgiOut,"<tr height=30>"\
					    "<td colspan=2>");
					 fprintf(cgiOut, "<select style=width:140px  name=selname>\n" );  
					 fprintf(cgiOut,"<option value=view>%s</option>",search(lpublic,"log_view"));
                     fprintf(cgiOut,"<option value=conf>%s</option>",search(lpublic,"log_conf"));                     
                     fprintf(cgiOut, "</select>\n" );	
					
					 fprintf(cgiOut,"<input type=submit name=reboot value=\"%s\">",search(lpublic,"log_reboot"));				
				
					 fprintf(cgiOut,"</td><tr>");

					 fprintf(cgiOut,"<tr height=5><td></td></tr>\n");

					 

					  //分割线
					  fprintf(cgiOut, "<tr><td><hr width=100%% size=1 color=#fff align=center noshade /></td><td>"\
					  "<hr width=100%% size=1 color=#fff align=center noshade /></td><td>"\
					  "<hr width=100%% size=1 color=#fff align=center noshade /></td>"\
					  "</tr>" );


					   fprintf(cgiOut,"<tr height=15><td></td></tr>");
					   
					 /*对下拉框的操作*/
                    //cgiFormStringNoNewlines("selname",selename,N);
					 
					if(cgiFormSubmitClicked("log_config") == cgiFormSuccess)
						{
						cgiFormStringNoNewlines("selname",selename,N);

						/*查看日志信息*/
						if(strcmp(selename,"view")==0)
							{	
							   // ruler_info(L_NOTICE, file_name,lpublic,encry,"log_notice_n","");
							   
							    ruler_info(L_CRIT, file_name,lpublic,encry,"log_crit_n","FFA500");  

							   	ruler_info(L_ERR, file_name,lpublic,encry,"log_err_n","FF4500"); 

			                    ruler_info(L_WARN, file_name,lpublic,encry,"log_warn_n","FFFF00");
			                    
						        ruler_info(L_NOTICE, file_name,lpublic,encry,"log_notice_n","CAFF70");  

							    ruler_info(L_INFO, file_name,lpublic,encry,"log_info_n","C1FFC1");

								ruler_info(L_DEBUG, file_name,lpublic,encry,"log_debug_n","C1FFC1");


								

					         }
						
                         /*配置日志信息*/
						 	if(strcmp(selename,"conf")==0)
						
                            {	
							    mod_info(L_CRIT, file_name,lpublic,encry,"log_crit_n");
								
								mod_info(L_ERR, file_name,lpublic,encry,"log_err_n");
								
			                    mod_info(L_WARN, file_name,lpublic,encry,"log_warn_n");

                     			mod_info(L_NOTICE, file_name,lpublic,encry,"log_notice_n");

							    mod_info(L_INFO, file_name,lpublic,encry,"log_info_n");

							    mod_info(L_DEBUG, file_name,lpublic,encry,"log_debug_n");

                     			 
                     	    }

						}
					
                    fprintf(cgiOut,"<tr height=25><td></td></tr>");
					#endif
				   if(cgiFormSubmitClicked("log_config") != cgiFormSuccess)
				   {
					 fprintf(cgiOut,"<tr><td colspan=3><input type=hidden name=encry_import value=%s></td></tr>",encry);
				   }
				   else if(cgiFormSubmitClicked("log_config") == cgiFormSuccess)
				   {
					 fprintf(cgiOut,"<tr><td colspan=3><input type=hidden name=encry_import value=%s></td></tr>",log_encry);
				   }	
		
			fprintf(cgiOut, "<tr><td><hr width=100%% size=1 color=#fff align=center noshade /></td><td>"\
			"<hr width=100%% size=1 color=#fff align=center noshade /></td><td>"\
			"<hr width=100%% size=1 color=#fff align=center noshade /></td>"\
			"</tr>" );

            fprintf(cgiOut,"<tr height=15><td></td></tr>");
			 
		    fprintf(cgiOut,"<tr>\n");
			fprintf(cgiOut,"<td>\n");
			fprintf(cgiOut,"%s",search(lpublic,"log_sysconf"));
			fprintf(cgiOut,"</td>\n");
			fprintf(cgiOut,"<td>\n");
			fprintf(cgiOut,"<a href=wp_logsys_conf.cgi?UN=%s target=mainFrame><font color=blue>%s</font></a>",encry,search(lpublic,"log_mod"));
			fprintf(cgiOut,"</td>\n");
			fprintf(cgiOut,"</tr>\n");

			fprintf(cgiOut,"<tr height=15><td></td></tr>");

		    fprintf(cgiOut,"<tr>\n");
			fprintf(cgiOut,"<td>\n");
			fprintf(cgiOut,"%s",search(lpublic,"log_sysinfo"));
			fprintf(cgiOut,"</td>\n");
			fprintf(cgiOut,"<td>\n");
			fprintf(cgiOut,"<a href=wp_logsys_view.cgi?UN=%s target=mainFrame><font color=blue>%s</font></a>",encry,search(lpublic,"log_scan"));
			fprintf(cgiOut,"</td>\n");
			fprintf(cgiOut,"<td>\n");
			fprintf(cgiOut,"<a href=wp_logsys_export.cgi?UN=%s target=mainFrame><font color=blue>%s</font></a>",encry,search(lpublic,"log_export"));
			fprintf(cgiOut,"</td>\n");
			fprintf(cgiOut,"</tr>\n");

		  
		  //分割线
			fprintf(cgiOut, "<tr><td><hr width=100%% size=1 color=#fff align=center noshade /></td><td>"\
			"<hr width=100%% size=1 color=#fff align=center noshade /></td><td>"\
			"<hr width=100%% size=1 color=#fff align=center noshade /></td>"\
			"</tr>" );					 
					
   fprintf(cgiOut,"</table>"); 
			  fprintf(cgiOut,"</td>"\
			  "</tr>"\
			  "<tr height=4 valign=top>"\
				  "<td width=120 height=4 align=right valign=top><img src=/images/bottom_07.gif width=1 height=10/></td>"\
				  "<td width=827 height=4 valign=top bgcolor=#FFFFFF><img src=/images/bottom_06.gif width=827 height=15/></td>"\
			  "</tr>"\
			"</table>");
		  fprintf(cgiOut,"</td>"\
		  "<td width=15 background=/images/di999.jpg>&nbsp;</td>"\
		"</tr>"\
	  "</table></td>");
	fprintf(cgiOut,"</tr>"\
	"<tr>"\
		"<td colspan=3 align=left valign=top background=/images/di777.jpg><img src=/images/di555.jpg width=61 height=62/></td>"\
	  "<td align=left valign=top background=/images/di777.jpg>&nbsp;</td>"\
		"<td align=left valign=top background=/images/di777.jpg><img src=/images/di666.jpg width=74 height=62/></td>"\
	"</tr>"\
	"</table>");//111111111111111
	fprintf(cgiOut,"</div>"\
	"</form>"\
	"</body>"\
	"</html>");
free(encry); 
return 0;
}  

/*规则信息,传值*/
int ruler_info(char * name,char *file_name,struct list *lpublic,char *encry,char *value,char *color)
{
     char temp[30];
     memset(temp,0,30);
	 int ret;
     sprintf(temp,"/var/log/%s",name);
     ret=is_file_exist(temp);
     if(ret==0){
     fprintf(cgiOut,"<tr height=15>");		  
     fprintf(cgiOut,"<td>%s%s</td>",search(lpublic,value),search(lpublic,"log_log"));
     strcpy(file_name,name);
     fprintf(cgiOut,"<td align=center><a href=wp_log_view.cgi?UN=%s&Nb=%s&Nc=%s target=mainFrame><font color=blue>%s</font></a></td>",encry,file_name,color,search(lpublic,"log_scan"));
     fprintf(cgiOut,"<td align=left><a href=wp_logsave.cgi?UN=%s&Nb=%s target=mainFrame><font color=blue>%s</font></a></td>",encry,file_name,search(lpublic,"log_save"));
     fprintf(cgiOut,"</tr>");
     fprintf(cgiOut,"<tr height=10><td></td></tr>");
     }   

	 return 0;
	 }

int mod_info(char * name,char *file_name,struct list *lpublic,char *encry,char *value)
{

	 fprintf(cgiOut,"<tr>");
	 fprintf(cgiOut,"<td align=left width=165>%s%s</td>",search(lpublic,value),search(lpublic,"log_rule"));
	 strcpy(file_name,name);
	 fprintf(cgiOut,"<td align=right><a href=wp_log_mod.cgi?UN=%s&Nb=%s target=mainFrame><font color=blue>%s</font></a></td>",encry,file_name,search(lpublic,"log_mod"));
	 fprintf(cgiOut,"</tr>");
		
	 fprintf(cgiOut,"<tr height=10><td></td></tr>");
     return 0;
}

//获取日志服务状态, 返回0 表示相反的状态 1表示是此状态
int get_state(char *state)
{
    int flag=-1;
    int status,ret;
    char init_command[128];
	memset(init_command,0,128);	
  
    sprintf( init_command, "%s %s",S_PATH,state);
    strcat( init_command, " >/dev/null");
    status = system(init_command);	 
    ret = WEXITSTATUS(status);
    if( 0 == ret )
    {
    	flag=0;     //当此状态成功的时候，证明是相反的状态
    }
    else
    {
    	flag=1;
    }
    
    return flag;

}
