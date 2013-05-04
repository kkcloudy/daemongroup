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
* wp_addusr.c
*
*
* CREATOR:
* autelan.software.Network Dep. team
* zhouym@autelan.com
*
* DESCRIPTION:
* system infos for add users
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
#include <sys/wait.h>

#include <sys/types.h>
#include <stdarg.h>
#include <errno.h>
#include <sys/resource.h>
#include <sys/stat.h>
#include <shadow.h>

#include "ws_md5.h"
#define HTDIGEST "/etc/htdigest"

int ShowAdduserPage(struct list *lpublic, struct list *lsystem); 
void adduser_hand(struct list *lpublic, struct list *lsystem); 
int checkname(char *name);           //判断用户名是否合法
int get_max();
void Md5(const char* ,  char *);
int HtdigestAdd(const char *,const char *);

int cgiMain()
{
	struct list *lpublic;	/*解析public.txt文件的链表头*/
	struct list *lsystem;	  /*解析system.txt文件的链表头*/	
	lpublic=get_chain_head("../htdocs/text/public.txt");
	lsystem=get_chain_head("../htdocs/text/system.txt"); 

 	ShowAdduserPage(lpublic,lsystem);
  	release(lpublic);  
  	release(lsystem); 	
 	return 0;
}

int ShowAdduserPage(struct list *lpublic, struct list *lsystem)
{ 
  int max=0;
  char *encry=(char *)malloc(BUF_LEN);				
  char *str;
  int i;
  char add_encry[BUF_LEN];  
	if(cgiFormSubmitClicked("submit_adduser") != cgiFormSuccess)
	{
		memset(encry,0,BUF_LEN);
		cgiFormStringNoNewlines("UN", encry, BUF_LEN); 
		str=dcryption(encry);
		if(str==NULL)
		{
			ShowErrorPage(search(lpublic,"ill_user")); 	       /*用户非法*/
			return 0;
		}
	memset(add_encry,0,BUF_LEN);                   /*清空临时变量*/
  }
  char * procductId=readproductID();
  
  cgiFormStringNoNewlines("encry_addusr",add_encry,BUF_LEN);
  cgiHeaderContentType("text/html");
  fprintf(cgiOut,"<html xmlns=\"http://www.w3.org/1999/xhtml\"><head>");
  fprintf(cgiOut,"<meta http-equiv=Content-Type content=text/html; charset=gb2312>");
  fprintf(cgiOut,"<title>%s</title>",search(lsystem,"user_manage"));
  fprintf(cgiOut,"<link rel=stylesheet href=/style.css type=text/css>"\
  	"<style type=text/css>"\
  	".a3{width:30;border:0; text-align:center}"\
  	"</style>"\
  "</head>"\
  "<script src=/ip.js>"\
  "</script>"\
  "<body>");

  if(cgiFormSubmitClicked("submit_adduser") == cgiFormSuccess)
  {
   
    adduser_hand(lpublic,lsystem);
		
  }
  fprintf(cgiOut,"<form method=post>"\
  "<div align=center>"\
  "<table width=976 border=0 cellpadding=0 cellspacing=0>"\
"<tr>"\
    "<td width=8 align=left valign=top background=/images/di22.jpg><img src=/images/youce4.jpg width=8 height=30/></td>"\
    "<td width=51 align=left valign=bottom background=/images/di22.jpg><img src=/images/youce33.jpg width=37 height=24/></td>"\
    "<td width=153 align=left valign=bottom id=%s background=/images/di22.jpg>%s</td>",search(lpublic,"title_style"),search(lsystem,"user_manage"));
    fprintf(cgiOut,"<td width=690 align=right valign=bottom background=/images/di22.jpg>");
	

	    
    	  fprintf(cgiOut,"<table width=130 border=0 cellspacing=0 cellpadding=0>"\
		"<tr>"\
		"<td width=62 align=center><input id=but type=submit name=submit_adduser style=background-image:url(/images/%s) value=""></td>",search(lpublic,"img_ok"));			
     		if(cgiFormSubmitClicked("submit_adduser") != cgiFormSuccess)
     		{
     		 
			  fprintf(cgiOut,"<td width=62 align=left><a href=wp_sysmagic.cgi?UN=%s target=mainFrame><img src=/images/%s border=0 width=62 height=20/></a></td>",encry,search(lpublic,"img_cancel"));


			}
      		else
      		{
      		

			  fprintf(cgiOut,"<td width=62 align=left><a href=wp_sysmagic.cgi?UN=%s target=mainFrame><img src=/images/%s border=0 width=62 height=20/></a></td>",add_encry,search(lpublic,"img_cancel"));

			}
      	 fprintf(cgiOut,"</tr>"\
		"</table>");
				  
	   	  
	fprintf(cgiOut,"</td>"\
    "<td width=74 align=right valign=top background=/images/di22.jpg><img src=/images/youce3.jpg width=31 height=30/></td>"\
"</tr>"\
"<tr>"\
    "<td colspan=5 align=center valign=middle><table width=976 border=0 cellpadding=0 cellspacing=0 bgcolor=#f0eff0>"\
	"<tr>"\
        "<td width=12 align=left valign=top background=/images/di888.jpg>&nbsp;</td>"\
        "<td width=948><table width=947 border=0 cellspacing=0 cellpadding=0>"\
		  "<tr height=4 valign=bottom>"\
              "<td width=120>&nbsp;</td>"\
              "<td width=827 valign=bottom><img src=/images/bottom_05.gif width=827 height=4/></td>"\
		  "</tr>"\
		  "<tr>"\
              "<td><table width=120 border=0 cellspacing=0 cellpadding=0>"\
				 "<tr height=25>"\
				  "<td id=tdleft>&nbsp;</td>"\
				"</tr>");				
				if(cgiFormSubmitClicked("submit_adduser") != cgiFormSuccess)
				{
					fprintf(cgiOut,"<tr height=25>"\
					  "<td align=left id=tdleft><a href=wp_usrlis.cgi?UN=%s target=mainFrame class=top><font id=%s>%s</font></a></td>",encry,search(lpublic,"menu_san"),search(lsystem,"user_list"));						 
					fprintf(cgiOut,"</tr>"\
					"<tr height=26>"\
						"<td align=left id=tdleft background=/images/bottom_bg.gif style=\"border-right:0\"><font id=%s>%s</font></td>",search(lpublic,"menu_san"),search(lsystem,"add_user"));	/*突出显示*/
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

					
						fprintf(cgiOut,"<tr height=25>"\
					  "<td align=left id=tdleft><a href=wp_view_user.cgi?UN=%s target=mainFrame class=top><font id=%s>%s</font></a></td>",encry,search(lpublic,"menu_san"),search(lpublic,"login_info"));
					fprintf(cgiOut,"</tr>");

				}
				else if(cgiFormSubmitClicked("submit_adduser") == cgiFormSuccess)				
				{
					fprintf(cgiOut,"<tr height=25>"\
					  "<td align=left id=tdleft><a href=wp_usrlis.cgi?UN=%s target=mainFrame class=top><font id=%s>%s</font></a></td>",add_encry,search(lpublic,"menu_san"),search(lsystem,"user_list"));						 
					fprintf(cgiOut,"</tr>"\
					"<tr height=26>"\
						"<td align=left id=tdleft background=/images/bottom_bg.gif style=\"border-right:0\"><font id=%s>%s</font></td>",search(lpublic,"menu_san"),search(lsystem,"add_user"));	/*突出显示*/
					fprintf(cgiOut,"</tr>"\
					"<tr height=25>"\
					  "<td align=left id=tdleft><a href=wp_modpass.cgi?UN=%s target=mainFrame class=top><font id=%s>%s</font></a></td>",add_encry,search(lpublic,"menu_san"),search(lsystem,"modify_password"));
					fprintf(cgiOut,"</tr>"\
					"<tr height=25>"\
					  "<td align=left id=tdleft><a href=wp_modpri.cgi?UN=%s target=mainFrame class=top><font id=%s>%s</font></a></td>",add_encry,search(lpublic,"menu_san"),search(lsystem,"modify_privilege"));
					fprintf(cgiOut,"</tr>"\
					"<tr height=25>"\
      					  "<td align=left id=tdleft><a href=wp_modsyslog.cgi?UN=%s target=mainFrame class=top><font id=%s>%s</font></a></td>",add_encry,search(lpublic,"menu_san"),search(lsystem,"modify_loglevel"));
      					fprintf(cgiOut,"</tr>");

					
						fprintf(cgiOut,"<tr height=25>"\
					  "<td align=left id=tdleft><a href=wp_view_user.cgi?UN=%s target=mainFrame class=top><font id=%s>%s</font></a></td>",add_encry,search(lpublic,"menu_san"),search(lpublic,"login_info"));
					fprintf(cgiOut,"</tr>");
				}
				for(i=0;i<2;i++)
				{
				  fprintf(cgiOut,"<tr height=25>"\
					"<td id=tdleft>&nbsp;</td>"\
				  "</tr>");
				}
			  fprintf(cgiOut,"</table>"\
			"</td>"\
			"<td align=left style=\"background-color:#ffffff; border-right:1px solid #707070; padding-left:30px\">"\
                "<table border=0 cellspacing=0 cellpadding=0>"\
        		 "<tr height=30>"\
        			"<td width=105 id=tdprompt>%s:</td>",search(lsystem,"user_na"));
        			fprintf(cgiOut,"<td><input type=text name=a_name size=21></td>"\
        				"<td style=font-size:12px;color:#FF0000;padding-left:5px>(%s)</td>",search(lsystem,"userna_dep"));
						fprintf(cgiOut,"</tr>"\
        		  "<tr height=30>"\
        			"<td id=tdprompt>%s:</td>",search(lsystem,"password"));
        			fprintf(cgiOut,"<td><input type=password name=a_pass1 size=21></td>"\
        				"<td style=font-size:12px;color:#FF0000;padding-left:5px>(%s)</td>",search(lsystem,"pwd_dep"));
						fprintf(cgiOut,"</tr>"\
        		  "<tr height=30>"\
        			"<td id=tdprompt>%s:</td>",search(lsystem,"con_pass"));
        			fprintf(cgiOut,"<td width=140 colspan=2><input type=password name=a_pass2 size=21></td>"\
        		  "</tr>"\
        		  "<tr height=30>"\
        			"<td id=tdprompt>%s:</td>",search(lsystem,"pri"));
        			fprintf(cgiOut,"<td colspan=2><select name=privilege style=width:130px>"
        				"<option value=enable>administrator"\
        				"<option value=view>user"\
        				"</select></td>"\
        		  "</tr>"\
        		  "<tr>");
				  max=get_max();
				  fprintf(cgiOut,"<tr><td id=tdprompt>%s:</td>",search(lpublic,"cur_num"));
				  fprintf(cgiOut,"<td colspan=2>%d</td>",max);
				  fprintf(cgiOut,"</tr>");
        		  if(cgiFormSubmitClicked("submit_adduser") != cgiFormSuccess)
        		  {
        			fprintf(cgiOut,"<td colspan=3><input type=hidden name=encry_addusr value=%s></td>",encry);
        		  }
        		  else if(cgiFormSubmitClicked("submit_adduser") == cgiFormSuccess)
        		  {
        			fprintf(cgiOut,"<td colspan=3><input type=hidden name=encry_addusr value=%s></td>",add_encry);
        		  }
        		  fprintf(cgiOut,"</tr></tr></table>");




	fprintf(cgiOut,"</td>"\
		  "</tr>"\
		  "<tr height=4 valign=top>"\
              "<td width=120 height=4 align=right valign=top><img src=/images/bottom_07.gif width=1 height=10/></td>"\
              "<td width=827 height=4 valign=top bgcolor=#FFFFFF><img src=/images/bottom_06.gif width=827 height=15/></td>"\
		  "</tr>"\
		"</table>"\
	  "</td>"\
	  "<td width=15 background=/images/di999.jpg>&nbsp;</td>"\
	"</tr>"\
  "</table></td>"\
"</tr>"\
"<tr>"\
    "<td colspan=3 align=left valign=top background=/images/di777.jpg><img src=/images/di555.jpg width=61 height=62/></td>"\
  "<td align=left valign=top background=/images/di777.jpg>&nbsp;</td>"\
    "<td align=left valign=top background=/images/di777.jpg><img src=/images/di666.jpg width=74 height=62/></td>"\
"</tr>"\
"</table>"\
"</div>"\
"</form>"\
"</body>"\
"</html>");

free(procductId);
free(encry); 
return 0;
}


void adduser_hand(struct list *lpublic, struct list *lsystem)
{

  char name[N],pass1[N],pass2[N],pri[N];    
  memset(name,0,N);					 /*清空临时变量*/
  memset(pass1,0,N);
  memset(pass2,0,N);
  memset(pri,0,N);
  char *command = (char*)malloc(PATH_LENG);
  memset(command,0,PATH_LENG);
  int ret=-1;
  int max=0;
  int status;
  cgiFormStringNoNewlines("a_name",name,N);
  cgiFormStringNoNewlines("a_pass1",pass1,N);
  cgiFormStringNoNewlines("a_pass2",pass2,N);
  cgiFormStringNoNewlines("privilege",pri,N);
  strcat(command,"useradd.sh");
  strcat(command," ");
  strcat(command,name);
  strcat(command," ");
  strcat(command,pass1);
  strcat(command," ");
  strcat(command,pri);
  strcat(command," ");
  strcat(command,"normal");

  max=get_max();
  
  if(strcmp(name,"")!=0)
  {
	  if((strlen(name)>=4)&&(strlen(name)<=32))
	  {
		  if(checkname(name)==0)
		  {
			  if(checkuser_exist(name) != 0)
			  {
				  if((strlen(pass1)>=6)&&(strlen(pass1)<=16))
				  {
				  	if(checkpassword(pass1)==0)
				  	{
						  if(strcmp(pass1,"")!=0)
						  {
							  if(strcmp(pass1,pass2)==0)
							  {
							         if(max>MAX_USER_NUM)
                                  	   {
                                  	     ShowAlert(search(lpublic,"user_max"));
                                  	   }
									 else
                                  	 {
									    status = system(command); 
									    ret = WEXITSTATUS(status);
									    if(ret == 0)
									    {
										    	HtdigestAdd(name,pass1);
									    	add_user_syslog_by_name(name);
									    	ShowAlert(search(lpublic,"oper_succ"));
									    }
								        else
									    ShowAlert(search(lpublic,"oper_fail"));		
                                  	 }
									  
		  
								 				  
							  }
							  else
							  {
								  ShowAlert(search(lsystem,"pass_incon"));
							  }
						  }
						  else
						  {
							  ShowAlert(search(lpublic,"pass_not_null"));
						  }
				  	}
					else
					{
						ShowAlert(search(lpublic,"pwd_dep"));//密码含有非法字符
					}
				  }
				  else
				  {
					  ShowAlert(search(lsystem,"pwd_len"));
				  }
			  }
			  else
			  {
				  ShowAlert(search(lsystem,"user_exist"));
			  }
		  
		  }
		  else
		  {
		  		ShowAlert(search(lsystem,"userna_std"));
			  //用户名含有非法字符
		  }
	  
	  }
	  else
	  {
		  ShowAlert(search(lsystem,"userna_len"));
	  }
  }
  else
  {
	  ShowAlert(search(lpublic,"name_not_null"));
  }

  
  free(command);
}


int checkname(char *name)
{
        int len,i;
        len = strlen(name);
        if(isalpha(name[0])==0)
                return -1;
        for(i=1;i<len;i++)
        {
                if((isalpha(name[i])==0)&&(isdigit(name[i])==0)&&(name[i]!='_'))
                {
                		
                        return -1;
                }
        }
        return 0;
}



/*get max user num*/
int get_max()
{
 struct group * grentry = NULL;
 char *ptr=NULL;
 int adminNum=0;
 int viewNum=0;
 int maxnum=0;
 int i;
 grentry = getgrnam(ADMINGROUP);
 if (grentry)
 {
    for(i=0;(ptr=grentry->gr_mem[i])!=NULL;i++)
    {      
 	}
  	adminNum=i;
 }
 
 
 grentry = getgrnam(VIEWGROUP);
 if (grentry)
 {
    for(i=0;(ptr=grentry->gr_mem[i])!=NULL;i++)
  	{  	  
  	}
    viewNum=i;
 }
maxnum=adminNum+viewNum;
return maxnum;
}

void 
Md5(const char *str , char *md5){

	md5_state_t state;
	md5_byte_t digest[16];
	char hex_output[16*2 + 1];

	md5_init(&state);
	md5_append(&state,(const md5_byte_t *)str,strlen(str));
	md5_finish(&state , digest);

	int di;

	for(di = 0 ; di < 16 ; di++){
		sprintf(hex_output + di*2 , "%02x",digest[di]);
	}

	strcpy(md5,hex_output);
}

int 
HtdigestAdd(const char *name,const char *passwd){
	FILE *fp;	
	if(NULL == (fp = fopen(HTDIGEST,"a"))){
		return -1;
	}

	char value[1024] = {0};	
	char md5[32 + 1];
	sprintf(value,"%s:AuteLAN:%s",name,passwd);
	Md5(value,md5);
	fprintf(fp,"%s:AuteLAN:%s\n",name,md5);
	fclose(fp);
	return 1;
}
