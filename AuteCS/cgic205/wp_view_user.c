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
* ws_view_user.c
*
*
* CREATOR:
* autelan.software.Network Dep. team
* zhouym@autelan.com
*
* DESCRIPTION:
* *
*
***************************************************************************/
#include <stdio.h>
#include "cgic.h"
#include <string.h>
#include <stdlib.h>
#include <sys/wait.h>
#include "ws_usrinfo.h"
#include "ws_err.h"
#include "ws_ec.h"



int ShowExportConfPage(struct list *lpublic, struct list *lsystem);     /*m代表加密后的字符串*/
int destname(char *name);
int config_passwd(struct list *lpublic, struct list *lsystem);
void config_timeout_threshold(struct list *lpublic,struct list *lsystem);/*add by qj*/


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
	 
     
	  int i;
	  int retu;
	  FILE *pp;
	
	  char buff[128];
	  memset(buff,0,128);

	  char cmd[128];
	  memset(cmd,0,128);	 
	 /*add by qj*/
	  FILE *fp = NULL;
	  char timeout_threshold[20] = { 0 };

	  if(access(TIMEOUT_THRESHOLD_STATUS,0) != 0)
	  {
	  	if(( fp = fopen( TIMEOUT_THRESHOLD_STATUS ,"w")) != NULL)
		{			
			system("sudo chmod 666 "TIMEOUT_THRESHOLD_STATUS"\n");
			fwrite( "start", strlen("start"), 1, fp );
			fclose(fp);
		}
	  }
	  /*end of add by qj*/
	   memset(encry,0,BUF_LEN);
		cgiFormStringNoNewlines("UN", encry, BUF_LEN); 
		str=dcryption(encry);
		if(str==NULL)
		{
		  ShowErrorPage(search(lpublic,"ill_user"));		   /*用户非法*/
		  return 0;
		}
	retu = checkuser_group(str);
	  	
	  /***********************2008.5.26*********************/
	  cgiHeaderContentType("text/html");
	 // char * procductId=readproductID();
	  fprintf(cgiOut,"<html xmlns=\"http://www.w3.org/1999/xhtml\"><head>");
	  fprintf(cgiOut,"<meta http-equiv=Content-Type content=text/html; charset=gb2312>");
	  fprintf(cgiOut,"<title>%s</title>",search(lsystem,"user_manage"));	  
	  fprintf(cgiOut,"<link rel=stylesheet href=/style.css type=text/css>"\
  	 "<style type=text/css>"\
  	  "#div1{ width:62px; height:18px; border:1px solid #666666; background-color:#f9f8f7;}"\
	  "#div2{ width:60px; height:15px; padding-left:5px; padding-top:3px}"\
	  "#link{ text-decoration:none; font-size: 12px}"\
	  ".usrlis {overflow-x:hidden;	overflow:auto; width: 700px; height: 100px; clip: rect( ); padding-top: 0px; padding-right: 0px; padding-bottom: 0px; padding-left: 0px} "\
	  "</style>"\
	  "</head>"\
	  "<body>");

	if(cgiFormSubmitClicked("submit_config") == cgiFormSuccess)
	{
		if(0 == retu)
		{
			config_passwd(lpublic, lsystem);
		    config_timeout_threshold(lpublic,lsystem);
		}
	}

	if(cgiFormSubmitClicked("no_maxerror") != cgiFormSuccess)
	{
		ccgi_no_passwd_max_error();
	}
	
	  fprintf(cgiOut,"<form method=post encType=multipart/form-data>"\
	  "<div align=center>"\
	  "<table width=976 border=0 cellpadding=0 cellspacing=0>");  //1111111111111111
	  fprintf(cgiOut,"<tr>"\
		"<td width=8 align=left valign=top background=/images/di22.jpg><img src=/images/youce4.jpg width=8 height=30/></td>"\
		"<td width=51 align=left valign=bottom background=/images/di22.jpg><img src=/images/youce33.jpg width=37 height=24/></td>"\
	  "<td width=153 align=left valign=bottom id=%s background=/images/di22.jpg>%s</td>",search(lpublic,"title_style"),search(lsystem,"user_manage"));
		fprintf(cgiOut,"<td width=690 align=right valign=bottom background=/images/di22.jpg>");
	  
	    fprintf(cgiOut,"<input type=hidden name=UN value=%s />",encry);
	 
			  fprintf(cgiOut,"<table width=130 border=0 cellspacing=0 cellpadding=0>");  //2222222222222
		fprintf(cgiOut,"<tr>");

		 fprintf(cgiOut,"<td width=62 align=center><input id=but type=submit name=submit_config style=background-image:url(/images/%s) value=\"\"></td>",search(lpublic,"img_ok"));
			
         fprintf(cgiOut,"<td width=62 align=left><a href=wp_sysmagic.cgi?UN=%s target=mainFrame><img src=/images/%s border=0 width=62 height=20/></a></td>",encry,search(lpublic,"img_cancel"));

     	
		fprintf(cgiOut,"</tr>"\
		"</table>");  //22222222222
	 
	 
	  
	
		fprintf(cgiOut,"</td>"\
		"<td width=74 align=right valign=top background=/images/di22.jpg><img src=/images/youce3.jpg width=31 height=30/></td>"\
	"</tr>"\
	"<tr>"\
		"<td colspan=5 align=center valign=middle><table width=976 border=0 cellpadding=0 cellspacing=0 bgcolor=#f0eff0>"\
		"<tr>"); //333333333333333
		
			fprintf(cgiOut,"<td width=12 align=left valign=top background=/images/di888.jpg>&nbsp;</td>"\
			"<td width=948><table width=947 border=0 cellspacing=0 cellpadding=0>"); //4444444444
			  fprintf(cgiOut,"<tr height=4 valign=bottom>"\
				  "<td width=120>&nbsp;</td>"\
				  "<td width=827 valign=bottom><img src=/images/bottom_05.gif width=827 height=4/></td>"\
			  "</tr>"\
			  "<tr>"\
				  "<td>");
			  fprintf(cgiOut,"<table width=120 border=0 cellspacing=0 cellpadding=0>"); //55555555555555

			  fprintf(cgiOut,"<tr height=25>"\
						"<td id=tdleft>&nbsp;</td>"\
					  "</tr>");
			  
			  
					{					 
					
					if(retu==0)
						{
						  fprintf(cgiOut,"<tr height=25>"\
						"<td align=left id=tdleft><a href=wp_usrlis.cgi?UN=%s target=mainFrame class=top><font id=%s>%s</font></a></td>",encry,search(lpublic,"menu_san"),search(lsystem,"user_list"));					   
					  fprintf(cgiOut,"</tr>");
					  fprintf(cgiOut,"<tr height=25>"\
						"<td align=left id=tdleft><a href=wp_addusr.cgi?UN=%s target=mainFrame class=top><font id=%s>%s</font></a></td>",encry,search(lpublic,"menu_san"),search(lsystem,"add_user"));
					  fprintf(cgiOut,"</tr>"\
					  "<tr height=25>"\
						  "<td align=left id=tdleft><a href=wp_modpass.cgi?UN=%s target=mainFrame class=top><font id=%s>%s</font></a></td>",encry,search(lpublic,"menu_san"),search(lsystem,"modify_password"));  
					  fprintf(cgiOut,"</tr>"\
					  "<tr height=25>"\
						"<td align=left id=tdleft><a href=wp_modpri.cgi?UN=%s target=mainFrame class=top><font id=%s>%s</font></a></td>",encry,search(lpublic,"menu_san"),search(lsystem,"modify_privilege"));
					  fprintf(cgiOut,"</tr>"\
					  "<tr height=25>"\
      					  "<td align=left id=tdleft><a href=wp_modsyslog.cgi?UN=%s target=mainFrame class=top><font id=%s>%s</font></a></td>",encry,search(lpublic,"menu_san"),search(lsystem,"modify_loglevel"));
      					fprintf(cgiOut,"</tr>");
						}
					else
						{
                      fprintf(cgiOut,"<tr height=25>"\
						"<td align=left id=tdleft><a href=wp_usrlis.cgi?UN=%s target=mainFrame class=top><font id=%s>%s</font></a></td>",encry,search(lpublic,"menu_san"),search(lsystem,"user_list"));					   
					  fprintf(cgiOut,"</tr>");

					  fprintf(cgiOut,"<tr height=25>"\
						  "<td align=left id=tdleft><a href=wp_modusr.cgi?UN=%s target=mainFrame class=top><font id=%s>%s</font></a></td>",encry,search(lpublic,"menu_san"),search(lsystem,"modify_password"));   
					  fprintf(cgiOut,"</tr>");				

						}
					  fprintf(cgiOut,"<tr height=26>"\
							"<td align=left id=tdleft background=/images/bottom_bg.gif style=\"border-right:0\"><font id=%s>%s</font></td>",search(lpublic,"menu_san"),search(lpublic,"login_info"));  /*突出显示*/
					    fprintf(cgiOut,"</tr>");
						
					}
					

                   /*点击延长*/
				  for(i=0;i<6;i++){
                  fprintf(cgiOut,"<tr height=25>"\
						"<td id=tdleft>&nbsp;</td>"\
					  "</tr>");
				  	}
                 
				   
				       fprintf(cgiOut,"</table>"); //555555555555555555
				        fprintf(cgiOut,"</td>"\
				        "<td  align=left style=\"background-color:#ffffff; border-right:1px solid #707070; padding-left:30px\">");

						fprintf(cgiOut,"<table width=700 border=0 cellspacing=0 cellpadding=0>"); //666666666666
					/*add by qj*/
					memset(timeout_threshold,0,20);
					strcpy(timeout_threshold,"180");/*default timeout_threshold is 180s*/
					if(access(TIMEOUT_THRESHOLD_CONF,0)==0)
					{
						system("sudo chmod 666 "TIMEOUT_THRESHOLD_CONF"\n");
						if(( fp = fopen( TIMEOUT_THRESHOLD_CONF ,"r")) != NULL)
						{			
							fgets(timeout_threshold,20,fp);
							fclose(fp);
						}
					}
					fprintf(cgiOut,"<tr style=\"padding-bottom:20px\"><td>%s:  <input type=text name=timeout_threshold size=15 value=%s> <font color=red>(s)</font></td>",search(lpublic,"timeout_threshold"),timeout_threshold);
					/*end of add by qj*/
				    fprintf(cgiOut,"<tr><td><input type=submit name=all value=\"%s\">",search(lpublic,"login_all"));
					fprintf(cgiOut,"<input type=submit name=one value=\"%s\"></td></tr>",search(lpublic,"login_one"));

                    fprintf(cgiOut,"<tr height=12><td></td></tr>");
					  //分割线
					  fprintf(cgiOut, "<tr><td><hr width=100%% size=1 color=#fff align=center noshade /></td><td>"\
					  "<hr width=100%% size=1 color=#fff align=center noshade /></td><td>"\
					  "<hr width=100%% size=1 color=#fff align=center noshade /></td>"\
					  "</tr>" );

					 fprintf(cgiOut,"<tr><td align=left>");

					 fprintf(cgiOut,"<div class=usrlis><table frame=below rules=rows width=500 border=0 cellspacing=0 cellpadding=0>");
                     fprintf(cgiOut,"<tr height=30 bgcolor=#eaeff9 style=font-size:16px>"\
							"</tr>"); 


                       if(cgiFormSubmitClicked("one") == cgiFormSuccess)
					 		{

							str=dcryption(encry);
							fprintf(cgiOut,"<tr><td>%s</td></tr>",str);
					 	#if 0
						     fp=popen("who -m","r");      				      
    								  if(fp==NULL)
    								  fprintf(cgiOut,"error open the pipe");
    							  else
    								  {
    							 fgets(buff,sizeof(buff),fp);	//很重要 ，不然与条目不匹配 						 
    					  do
    					  {		   											 				  
    					      fprintf(cgiOut,"<br>");  							  
    						  fprintf(cgiOut,"%s",buff);  
							  fprintf(cgiOut,"<tr height=5><td></td></tr>");
							  fgets( buff, sizeof(buff),fp); 									   
							   }while(!feof(fp) ); 					   
							  pclose(fp);  
    										  }
                          #endif 
					 		}

					   
					 	if(cgiFormSubmitClicked("all") == cgiFormSuccess)
					 		{
						     pp=popen("who","r");      				      
    								  if(pp==NULL)
    								  fprintf(cgiOut,"error open the pipe");
    							  else
    								  {
    							  fgets(buff,sizeof(buff),pp);	//很重要 ，不然与条目不匹配 						 
    					  do
    					  {		   											 				  
    					      fprintf(cgiOut,"<br>");  							  
    						  fprintf(cgiOut,"%s",buff);  
							  fprintf(cgiOut,"<tr height=5><td></td></tr>");
							  fgets( buff, sizeof(buff),pp); 									   
							   }while(!feof(pp) ); 					   
							  pclose(pp);  
    										  }

					 		}						
						  
	fprintf(cgiOut,"</table></div></td>");	//88888888888888888888888888
	fprintf(cgiOut,"</tr>");    
	//////////////////
	fprintf(cgiOut, "<tr><td><hr width=100%% size=1 color=#fff align=center noshade /></td><td>"\
	"<hr width=100%% size=1 color=#fff align=center noshade /></td><td>"\
	"<hr width=100%% size=1 color=#fff align=center noshade /></td>"\
	"</tr>" );
	int maxdays = 91;
	ccgi_get_login_setting(&maxdays);
	fprintf(cgiOut,"<tr><td><table border=0>\n");
	fprintf(cgiOut,"<tr><td>%s:</td>\n",search(lsystem,"pwd_active"));
	fprintf(cgiOut,"<td colspan=2><input type=text name=activeday value=\"%d\" maxLength=3><font color=red>(1-180)</font></td></tr>\n",maxdays);
	/*改过的*/
	int maxerror = 4;
	/*改过的*/
	ccgi_get_pwd_err_setting(&maxerror);
	fprintf(cgiOut,"<tr><td>%s:</td>\n",search(lsystem,"pwd_maxerror"));
	fprintf(cgiOut,"<td><input type=text name=maxerror value=\"%d\" maxLength=2><font color=red>(3-10)</font></td>\n",maxerror);
	fprintf(cgiOut,"<td align=left style=padding-left:10px><input type=submit style=width:115px; height:36px border=0 name=no_maxerror style=background-image:url(/images/SubBackGif.gif) value=\"%s%s\"></td></tr>",search(lsystem,"cancel"),search(lsystem,"pwd_maxerror"));

	int minlen = 4;
	int unreplynum =3;
	int strongflag = 0;
	ccgi_get_pwd_unrepeat_setting(&unreplynum, &minlen, &strongflag);

	fprintf(cgiOut,"<tr><td>%s:</td>\n",search(lsystem,"pwd_minlen"));
	fprintf(cgiOut,"<td colspan=2><input type=text name=minlen value=\"%d\" maxLength=2><font color=red>(1-8)</font></td></tr>\n",minlen);

	fprintf(cgiOut,"<tr><td>%s:</td>\n",search(lsystem,"pwd_unreply"));
	fprintf(cgiOut,"<td colspan=2><input type=text name=unreap value=\"%d\" maxLength=2><font color=red>(3-10)</font></td></tr>\n",unreplynum);


	fprintf(cgiOut,"</table></td></tr>\n");	
					 //////////////////
			 
					
   fprintf(cgiOut,"</table>"); //666666666666666
			  fprintf(cgiOut,"</td>"\
			  "</tr>"\
			  "<tr height=4 valign=top>"\
				  "<td width=120 height=4 align=right valign=top><img src=/images/bottom_07.gif width=1 height=10/></td>"\
				  "<td width=827 height=4 valign=top bgcolor=#FFFFFF><img src=/images/bottom_06.gif width=827 height=15/></td>"\
			  "</tr>"\
			"</table>");//444444444444
		  fprintf(cgiOut,"</td>"\
		  "<td width=15 background=/images/di999.jpg>&nbsp;</td>"\
		"</tr>"\
	  "</table></td>"); //333333333333
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
//free(procductId);
free(encry); 
return 0;
}  

int config_passwd(struct list * lpublic, struct list * lsystem)
{
	char activeday[10] = {0};
	int activeday_num = 0;
	char maxerror[10] = {0};
	int maxerror_num = 0;	
	char minlen[10] = {0};
	int minlen_num = 0;
	char unrep[10] = {0};
	int unrep_num = 0;
	
	cgiFormStringNoNewlines("activeday", activeday, 10); 	
	activeday_num = strtoul(activeday,0,10);
	if ((activeday_num <= 9999) && (activeday_num > 0))
	{
		ccgi_passwd_alive_time(activeday_num);
	}
	else
	{
		ShowAlert("active day beyond");
	}
	
	cgiFormStringNoNewlines("maxerror", maxerror, 10); 	
	maxerror_num = strtoul(maxerror,0,10);
	if ((maxerror_num <= 10) && (maxerror_num > 2))
	{
		ccgi_passwd_max_error(maxerror_num);
	}
	else
	{
		ShowAlert("maxerror_num error  beyond");
	}

	cgiFormStringNoNewlines("minlen", minlen, 10); 	
	minlen_num = strtoul(minlen,0,10);
	if ((minlen_num <= 8) && (minlen_num > 0))
	{
		ccgi_passwd_min_length(minlen_num);
	}
	else
	{
		ShowAlert("minlen_num  beyond");
	}



	cgiFormStringNoNewlines("unreap", unrep, 10); 	
	unrep_num = strtoul(unrep,0,10);
	if ((unrep_num <= 10) && (unrep_num > 2))
	{
		ccgi_passwd_unrepeat(unrep_num);
	}
	else
	{
		ShowAlert("unrep_num  beyond");
	}

	
	return 0;
}

/*解析文件名*/
//或者可以定义一个 枚举类型

int destname(char *name)
{
char cmd[128];
memset(cmd,0,128);
int ret;
sprintf(cmd,"ifexist.sh /var/log/%s",name);
//sprintf(cmd,"/mnt/shell/ifdir.sh /var/log/%s",name);
ret=system(cmd);
return ret;
}

/*add by qj*/
void config_timeout_threshold(struct list *lpublic,struct list *lsystem)
{
	FILE *fp = NULL;
	char timeout_threshold[20] = { 0 };
	memset(timeout_threshold,0,20);
	int time_num = 0;
	cgiFormStringNoNewlines("timeout_threshold",timeout_threshold,20);
	time_num = strtoul(timeout_threshold,NULL,10);
	if(0 == time_num)
	{
		time_num = 180;
		memset(timeout_threshold,0,20);
		strcpy(timeout_threshold,"180");
	}
	
	if(access(TIMEOUT_THRESHOLD_CONF,0)==0)
	{
		system("sudo chmod 666 "TIMEOUT_THRESHOLD_CONF"\n");
		if(( fp = fopen( TIMEOUT_THRESHOLD_CONF ,"w")) != NULL)
		{			
			fwrite( timeout_threshold, strlen(timeout_threshold), 1, fp );
			fclose(fp);
		}
	}
	else
	{
		if(( fp = fopen( TIMEOUT_THRESHOLD_CONF ,"w")) != NULL)
		{			
			system("sudo chmod 666 "TIMEOUT_THRESHOLD_CONF"\n");
			fwrite( timeout_threshold, strlen(timeout_threshold), 1, fp );
			fclose(fp);
		}
	}
}
