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
* wp_modpass.c
*
*
* CREATOR:
* autelan.software.Network Dep. team
* zhouym@autelan.com
*
* DESCRIPTION:
* system infos 
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

#include "ws_md5.h"

#define HTDIGEST "/etc/htdigest"

//add by zengxx@autelan for htdigest.c
int HtdigestPasswd(const char *,const char  *);
void Md5(const char *, char *);

int ShowModifyPasswdPage(struct list *lpublic, struct list *lsystem);
void modifypasswd_hand(char *str,struct list *lpublic, struct list *lsystem); 

int cgiMain()
{
	struct list *lpublic;	/*解析public.txt文件的链表头*/
	struct list *lsystem;	  /*解析system.txt文件的链表头*/	
	lpublic=get_chain_head("../htdocs/text/public.txt");
	lsystem=get_chain_head("../htdocs/text/system.txt"); 

	ShowModifyPasswdPage(lpublic,lsystem);
  	release(lpublic);  
  	release(lsystem); 

	return 0;
}

int ShowModifyPasswdPage(struct list *lpublic, struct list *lsystem)
{ 
 
  char *encry=(char *)malloc(BUF_LEN);				/*存储从wp_usrmag.cgi带入的加密字符串*/
  char *str;	
  int i;
  char mod_encry[BUF_LEN]; 
  char * USERNAME=(char *)malloc(33);
  memset(USERNAME,0,33);
  
  if(cgiFormSubmitClicked("submit_modifyadmin") != cgiFormSuccess)
  {
	  memset(encry,0,BUF_LEN);
    cgiFormStringNoNewlines("UN", encry, BUF_LEN);
    cgiFormStringNoNewlines("USERNAME",USERNAME, 33);  
    str=dcryption(encry);
    if(str==NULL)
    {
      ShowErrorPage(search(lpublic,"ill_user")); 	 /*用户非法*/
      return 0;
	}
	memset(mod_encry,0,BUF_LEN);
  }
  else
  	{
  		cgiFormStringNoNewlines("USERNAME_commit",USERNAME,33);
		//fprintf(stderr,"USERNAME=%s",USERNAME);
  	}
  char * procductId=readproductID();
  cgiFormStringNoNewlines("encry_modadm",mod_encry,BUF_LEN);
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
  if(cgiFormSubmitClicked("submit_modifyadmin") == cgiFormSuccess)
  {
    str=dcryption(mod_encry);
	//fprintf(stderr,"str=%s",str);
    modifypasswd_hand(str,lpublic,lsystem);
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
          "<td width=62 align=center><input id=but type=submit name=submit_modifyadmin style=background-image:url(/images/%s) value=""></td>",search(lpublic,"img_ok"));		  
		  if(cgiFormSubmitClicked("submit_modifyadmin") != cgiFormSuccess)
		  {
		

		  	fprintf(cgiOut,"<td width=62 align=left><a href=wp_sysmagic.cgi?UN=%s target=mainFrame><img src=/images/%s border=0 width=62 height=20/></a></td>",encry,search(lpublic,"img_cancel"));


		  }
		  else
		  {

		 
 				fprintf(cgiOut,"<td width=62 align=left><a href=wp_sysmagic.cgi?UN=%s target=mainFrame><img src=/images/%s border=0 width=62 height=20/></a></td>",mod_encry,search(lpublic,"img_cancel"));

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
				  if(cgiFormSubmitClicked("submit_modifyadmin") != cgiFormSuccess)
				  {
					  fprintf(cgiOut,"<tr height=25>"\
						"<td align=left id=tdleft><a href=wp_usrlis.cgi?UN=%s target=mainFrame class=top><font id=%s>%s</font></a></td>",encry,search(lpublic,"menu_san"),search(lsystem,"user_list"));					   
					  fprintf(cgiOut,"</tr>"\
					  "<tr height=25>"\
						"<td align=left id=tdleft><a href=wp_addusr.cgi?UN=%s target=mainFrame class=top><font id=%s>%s</font></a></td>",encry,search(lpublic,"menu_san"),search(lsystem,"add_user"));
					  fprintf(cgiOut,"</tr>"\
					  "<tr height=26>"\
						  "<td align=left id=tdleft background=/images/bottom_bg.gif style=\"border-right:0\"><font id=%s>%s</font></td>",search(lpublic,"menu_san"),search(lsystem,"modify_password"));   /*突出显示*/
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
				  else if(cgiFormSubmitClicked("submit_modifyadmin") == cgiFormSuccess)				  
				  {
                    fprintf(cgiOut,"<tr height=25>"\
					  "<td align=left id=tdleft><a href=wp_usrlis.cgi?UN=%s target=mainFrame class=top><font id=%s>%s</font></a></td>",mod_encry,search(lpublic,"menu_san"),search(lsystem,"user_list"));                       
                    fprintf(cgiOut,"</tr>"\
                    "<tr height=25>"\
                      "<td align=left id=tdleft><a href=wp_addusr.cgi?UN=%s target=mainFrame class=top><font id=%s>%s</font></a></td>",mod_encry,search(lpublic,"menu_san"),search(lsystem,"add_user"));
                    fprintf(cgiOut,"</tr>"\
					"<tr height=26>"\
						"<td align=left id=tdleft background=/images/bottom_bg.gif style=\"border-right:0\"><font id=%s>%s</font></td>",search(lpublic,"menu_san"),search(lsystem,"modify_password"));   /*突出显示*/
                    fprintf(cgiOut,"</tr>"\
  				    "<tr height=25>"\
                      "<td align=left id=tdleft><a href=wp_modpri.cgi?UN=%s target=mainFrame class=top><font id=%s>%s</font></a></td>",mod_encry,search(lpublic,"menu_san"),search(lsystem,"modify_privilege"));
                    fprintf(cgiOut,"</tr>"\
					"<tr height=25>"\
      					  "<td align=left id=tdleft><a href=wp_modsyslog.cgi?UN=%s target=mainFrame class=top><font id=%s>%s</font></a></td>",mod_encry,search(lpublic,"menu_san"),search(lsystem,"modify_loglevel"));
      					fprintf(cgiOut,"</tr>");

						fprintf(cgiOut,"<tr height=25>"\
					  "<td align=left id=tdleft><a href=wp_view_user.cgi?UN=%s target=mainFrame class=top><font id=%s>%s</font></a></td>",mod_encry,search(lpublic,"menu_san"),search(lpublic,"login_info"));
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
                "<table border=0  width=505 cellspacing=0 cellpadding=0>"\
              "<tr height=30>"\
                "<td width=105 id=tdprompt>%s:</td>",search(lsystem,"user_na"));
                if(strcmp(USERNAME,"")==0)
                	fprintf(cgiOut,"<td colspan=2><input type=text name=a_name size=21></td>");
                else
                	{
                		fprintf(cgiOut,"<td><input type=text name=a_name_x size=21 value=%s enable></td>",USERNAME);
						fprintf(cgiOut,"<td><input type=hidden name=a_name size=21 value=%s ></td>",USERNAME);
                	}
              fprintf(cgiOut,"</tr>"\


				"<tr height=30>"\
				  "<td width=105 id=tdprompt>%s:</td>",search(lsystem,"admin_pass"));
				  fprintf(cgiOut,"<td width=150><input type=password name=a_pass3 size=21></td>"\
				  "<td style=font-size:12px;color:#FF0000  width=250>(%s)</td>",search(lsystem,"pwd_dep"));
				fprintf(cgiOut,"</tr>"\

              "<tr height=30>"\
                "<td id=tdprompt>%s:</td>",search(lsystem,"new_pass"));
                fprintf(cgiOut,"<td><input type=password name=a_pass1 size=21></td>"\
                "<td style=font-size:12px;color:#FF0000>(%s)</td>",search(lsystem,"pwd_dep"));
				fprintf(cgiOut,"</tr>"\
              "<tr height=30>"\
                "<td id=tdprompt>%s:</td>",search(lsystem,"con_pass"));
                fprintf(cgiOut,"<td><input width=140 type=password name=a_pass2 size=21></td>"\
                "<td style=font-size:12px;color:#FF0000>(%s)</td>",search(lsystem,"pwd_dep"));
				fprintf(cgiOut,"</tr>"\
              "<tr>");
			  if(cgiFormSubmitClicked("submit_modifyadmin") != cgiFormSuccess)
			  {
                fprintf(cgiOut,"<td colspan=3><input type=hidden name=encry_modadm value=%s></td>",encry);
				fprintf(cgiOut,"<td colspan=3><input type=hidden name=USERNAME_commit value=%s></td>",USERNAME);
				
			  }
			  else if(cgiFormSubmitClicked("submit_modifyadmin") == cgiFormSuccess)
              {
                fprintf(cgiOut,"<td colspan=3><input type=hidden name=encry_modadm value=%s></td>",mod_encry);
				fprintf(cgiOut,"<td colspan=3><input type=hidden name=USERNAME_commit value=%s></td>",USERNAME);
			  }
			  fprintf(cgiOut,"</tr>"\
		"</table>");
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

free(USERNAME);
free(procductId);  
free(encry);
return 0;
}

void modifypasswd_hand(char *str,struct list *lpublic, struct list *lsystem)
{

  char na[N],pass1[N],pass2[N],admin_pass[N];  
  int ret=0;
  int ret1;
  int status,status1;
  char *command1,*command;
  memset(na,0,N);					 /*清空临时变量*/
  memset(pass1,0,N);
  memset(pass2,0,N);
  cgiFormStringNoNewlines("a_name", na, N);
  cgiFormStringNoNewlines("a_pass1", pass1, N);
  cgiFormStringNoNewlines("a_pass2", pass2, N);
  cgiFormStringNoNewlines("a_pass3", admin_pass, N);
  fprintf(stderr,"admin_pass=%s",admin_pass);
  fprintf(stderr,"a_pass1=%s",pass1);
  fprintf(stderr,"a_name=%s",na);

  /********changed by qiandawei 2008.5.20********/
  command1 = (char *)malloc(PATH_LENG);
  memset(command1,0,PATH_LENG);
  strcat(command1,"checkpwd.sh");   
  strcat(command1," ");
  strcat(command1,str);
  strcat(command1," ");
  strcat(command1,admin_pass);
  strcat(command1," ");
  strcat(command1,"11");
  
  status1 = system(command1); 	   /*检查修改密码操作是否为用户本人*/
  ret1 = WEXITSTATUS(status1);
  if(ret1 == 1)
   {
		if(strcmp(na,"")!=0)
		  {
		    if(checkuser_exist(na)==0)                                     /*用户存在*/
		    	{
				  if((strcmp(pass1,"")!=0)&&(strcmp(pass2,"")!=0))				 /*新输入的密码非空*/
				  {
				  	     
				  		 //if((strlen(pass1)>=6)&&(strlen(pass1)<=16))
				  		 //kehao modified 03-10-2011 14:26
				  		 if((strlen(pass1)>=4)&&(strlen(pass1)<=16))   //跟进底层的命令行功能修改,把密码长度设置为最小4位
				  		 /////////////////////////////////////////////////////////////////////////////////////////////////
						 {
						 	if(checkpassword(pass1)==0)
						 	{
								if(strcmp(pass1,pass2)==0)			 /*新密码和确认密码一致*/
							    { 	
									 command = (char*)malloc(PATH_LENG); /*修改密码参数*/
									 memset(command, 0, PATH_LENG);
									 strcat(command,"chpass.sh"); 
									 strcat(command," ");
									 strcat(command,na);
									 strcat(command," ");
									 strcat(command,pass1);
									 strcat(command," ");
									 strcat(command,"normal");
									  /*调用修改密码脚本*/
									 status = system(command);
									 ret = WEXITSTATUS(status);

									 if(ret == 0){
										 HtdigestPasswd(na,pass1);
										 ShowAlert(search(lpublic,"oper_succ"));
									 }
									else{
										ShowAlert(search(lpublic,"oper_fail"));					  
									}
									 free(command);
								}
								else													 /*密码不一致*/
								{
									  ShowAlert(search(lsystem,"pass_incon")); 
								}
						 	}
							else
							{
								ShowAlert(search(lsystem,"pwd_dep"));//密码含有非法字符
							}
						 }
						 else
						 {
						 		ShowAlert(search(lsystem,"pwd_len"));
						 }
				  }
				  else
				  {
						  ShowAlert(search(lpublic,"pass_not_null"));	/*密码不能为空*/ 
				  }
				  	
		    	}
			else
				{
				  ShowAlert(search(lsystem,"user_not_exist"));        /*用户不存在*/
				}
		  }
		else
		  {
			  ShowAlert(search(lpublic,"name_not_null"));
		  }

   }
  else
   {
	 ShowAlert(search(lsystem,"admin_pass_err"));			/*用户非法*/
   }

  /***************change end******************/


  free(command1);
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
HtdigestPasswd(const char *name,const char *passwd){
	FILE *fp;	
	if(NULL == (fp = fopen(HTDIGEST,"r+"))){
		return -1;
	}

	char buf[1024] = {0};
	char user_name[32] = {0};
	char user_passwd[32 + 1] = {0};

	char value[1024] = {0};	
	char md5[32 + 1] = {0};
	
	while(NULL != fgets(buf,1024,fp)){
		sscanf(buf,"%[^:]:%*[^:]:%[^\n]",user_name,user_passwd);	
		if( !strcmp(user_name,name)){
			
			sprintf(value,"%s:AuteLAN:%s",name,passwd);
			Md5(value,md5);

			fseek(fp,-33,SEEK_CUR);
			fprintf(fp,"%s",md5);
			fclose(fp);
			return 1;
		}
		memset(buf,0,1024);	
		memset(user_name,0,32);
		memset(user_passwd,0,32);
	}
}

