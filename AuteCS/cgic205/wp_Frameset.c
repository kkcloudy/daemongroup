/* cgicTempDir is the only setting you are likely to need
	to change in this file. */

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
* wp_Frameset.c
*
*
* CREATOR:
* autelan.software.Network Dep. team
* qiaojie@autelan.com
*
* DESCRIPTION:
*
*
*
*******************************************************************************/


#include <stdio.h>
#include "cgic.h"
#include <string.h>
#include <stdlib.h>
#include <sys/wait.h>
#include "ws_err.h"
#include "ws_usrinfo.h"
#include "ws_ec.h"
#include "ws_license.h"   //控制条目是否出现
#include <syslog.h>
  
#define SCRIPT_NAME "/cgi-bin/wp_login.cgi"
void ShowFramesetPage(char *m,char *n);   /*m代表加密后的字符串,n代表当前操作系统的默认语言，k代表设备类型*/
static int cgi_user_passwd_valid(const char *username, const char *password, const char *lang);  

int cgiMain()
{
  struct list *lpublic;	/*解析public.txt文件的链表头*/
  int ret,status;
  char user_name[N];    /*存储从登陆页面获取的用户名*/
  char password[N];     /*存储从登陆页面获取的用户*/
  char lan[10];         /*存储从登录页面获取的当前操作系统的默认语言*/
  char *encry=(char *)malloc(BUF_LEN);                /*存储从wp_usrmag.cgi带入的加密字符串*/
  char *str; 
  char *command = (char *)malloc(PATH_LENG); 
  int create_result;
  char *session_id= (char *)malloc(N);  
  char realm[] = "AuteLAN";
  char info[64] = {0};  
  char cflag[8] = {0};
  int lockflag = 0;

  char *cgiRemoteAddr = NULL;
  
  memset(encry,0,BUF_LEN);
  memset(lan,0,10);

  lpublic=get_chain_head("../htdocs/text/public.txt");
  


  if(cgiFormStringNoNewlines("UN", encry, BUF_LEN)!=cgiFormNotFound )
  {

    cgiFormStringNoNewlines("LAN", lan, 10);
    if(strcmp(lan,"zh-cn")==0)
	{
      memset(lan,0,10);
	  strcpy(lan,"chinese");
	}
	else
	{
	  memset(lan,0,10);
	  strcpy(lan,"english");

	}
    str=dcryption(encry);
    if(str==NULL)
    {
	    ShowErrorPage(search(lpublic,"ill_user"));   	
		free(encry); 
		free(command);
		free(session_id);
		release(lpublic);  
		return 0;
    }
    else
      ShowFramesetPage(encry,lan);   
  }
  else                       /*从设备登录*/
  {
    cgiFormStringNoNewlines("LAN", lan, 10);
    if(strcmp(lan,"zh-cn")==0)
	{
      memset(lan,0,10);
	  strcpy(lan,"chinese");
	}
	else
	{
	  memset(lan,0,10);
	  strcpy(lan,"english");

        }
        cgiFormStringNoNewlines("user_name", user_name, N);
        cgiFormStringNoNewlines("user_pass", password, N);
        cgiFormStringNoNewlines("cflag", cflag, 8);
        sprintf(info,"%s:%s",user_name,password);

#if 0
        if(strcmp(user_name,"")==0)
        {
            if(strcmp(lan,"chinese")==0)
                ShowErrorPage("用户名不能为空！");
            else
                ShowErrorPage("User name can not be empty!");   	 
        }
        else if(strcmp(password,"")==0)	     
        {
            if(strcmp(lan,"chinese")==0)
                ShowErrorPage("密码不能为空！");
            else
                ShowErrorPage("Passwords can not be empty!");
        }
        else
#endif
        if(cgi_user_passwd_valid(user_name, password, lan) == 0)
        {
            /**********************************************************************************************************/
            memset(command,0,PATH_LENG);
            strcat(command,"checkpwd.sh");   
            strcat(command," ");
            strcat(command,user_name);
            strcat(command," ");
            strcat(command,password);
            strcat(command," ");
            strcat(command,"11");

            status = system(command);        /*登陆验证*/
            ret = WEXITSTATUS(status);
            cgiRemoteAddr = getenv("REMOTE_ADDR");

            if(ret == 1)                    /*验证成功*/  
            {
                openlog("login", LOG_PID, LOG_DAEMON);
                syslog( LOG_INFO|LOG_LOCAL7, "user %s IP %s login successfully!" ,user_name,cgiRemoteAddr);
                create_result=create_user_infor(user_name);
                if(create_result==1)
                {
                    memset(session_id,0,N);
                    Search_user_infor_byName(user_name,1,session_id);
                    cgiHeaderCookieSetString(realm,info,86400,SCRIPT_NAME,cgiServerName);
                    cgiHeaderContentType("text/html");			
                    fprintf(cgiOut,"<script type='text/javascript'>\n"); 
                    //fprintf(cgiOut,"document.cookie = \"%s\" + \"&\" + \"%s\" ;",user_name,session_id);
                    fprintf(cgiOut,"document.cookie = \"user&\" + \"%s\" ;",session_id);
                    fprintf(cgiOut,"</script>\n");
                    fprintf(cgiOut,"<script type='text/javascript'>\n"); 
                    fprintf(cgiOut,"window.location.href='wp_Frameset.cgi?UN=%s&LAN=%s';\n", session_id, lan); 	
                    fprintf(cgiOut,"</script>\n");
                    exit(0);
                }			 
                else
                {
                    LogoffPage(user_name);
                }	
            }
            if(ret == 2)
            {		 
                if(!strcmp(cflag,"not_set")){
                    openlog("login", LOG_PID, LOG_DAEMON);
                    syslog( LOG_INFO|LOG_LOCAL7, "user %s IP %s login failed!" ,user_name,cgiRemoteAddr);
                        if(strcmp(lan,"chinese")==0)
                            ShowErrorPage("用户名或者密码错误！");
                        else
                            ShowErrorPage("User name or password is error!");
                }
                else{
                    cgiHeaderCookieSetString(realm,"",-3600,SCRIPT_NAME,cgiServerName);
                    cgiHeaderContentType("text/html");			
                    fprintf(cgiOut, "<html>\n");
                    fprintf(cgiOut, "<head>\n");
                    fprintf(cgiOut, "<script type='text/javascript'>\n");
                    fprintf(cgiOut, "window.location.href='wp_login.cgi'\n");
                    fprintf(cgiOut, "</script>\n");
                    fprintf(cgiOut, "</head>\n");
                    fprintf(cgiOut, "<body></body>\n");
                    fprintf(cgiOut, "</html>\n");
                }
            }
            cgiRemoteAddr = NULL;
        }
    }
    free(encry); 
    free(command);
    free(session_id);
    release(lpublic);  
    /**********************************************************************************************************/
    return 0; 
}

void ShowFramesetPage(char *m,char *n)
{
	cgiHeaderContentType("text/html");
	fprintf(cgiOut,"<html><head>");
	fprintf(cgiOut,"<meta http-equiv=Content-Type content=text/html; charset=gb2312>");
	fprintf(cgiOut,"<title>Access Control System</title>");
	fprintf(cgiOut,"</head>");

	fprintf(cgiOut,"<script type='text/javascript'>\n"); 
	fprintf(cgiOut,"var checkstr = \"user&\" + \"%s\";"\
	"var cookiestr = document.cookie;"\
	"if (cookiestr != checkstr)"\
	"{"\
	"window.location.href='wp_login.cgi';\n"\	
	"}",m);
	fprintf(cgiOut,"</script>\n");

	fprintf(cgiOut,"<frameset rows=110,* cols=* frameborder=no border=0 framespacing=0>"\
	"<frame src=wp_topFrame.cgi?UN=%s&LAN=%s name=topFrame id=topFrame title=topFrame>",m,n);

int flag=-1;
flag=get_license_state(10);
if(flag != SYSTEM_ITEM)	
fprintf(cgiOut,"<frame src=wp_sysmagic.cgi?UN=%s name=mainFrame id=mainFrame title=mainFrame>",m);
else
fprintf(cgiOut,"<frame name=mainFrame id=mainFrame title=mainFrame>");

//////////////////////////////////////////
		fprintf(cgiOut,"</frameset>"\
	"<noframes>"\
	"<body>"\
    "很抱歉，您使用的浏览器不支持框架功能，请转用新的浏览器。"\
	"</body>"\
	"</noframes></html>");
}

static int cgi_user_passwd_valid(const char *username, const char *password, const char *lang)  /* zengxx@autelan.com fix bug for xss injection*/
{
    char *msg_err1, *msg_err2, tmp[1024] = {0};
    int i;
    if(NULL == username && NULL == password)
    {
        return -1;
    }

    if(!strncmp(lang, "chinese", strlen("chinese"))){
        msg_err1 = "用户名或密码为空";
        msg_err2 = "用户名或密码不合法";
    }
    else {
        msg_err1 = "username or password empty";
        msg_err2 = "username or password valid";
    }

    if(!strcmp(username, "") || !strcmp(password, ""))
    {
        ShowErrorPage(msg_err1);
        return -1;
    }

    if(strlen(username) > 64 || strlen(password) > 64)
    {
        ShowErrorPage(msg_err2);
        return -1;
    }

    sprintf(tmp, "%s%s", username, password);

    for(i = 0; i < strlen(tmp); i++)
    {
        fprintf(stderr,"%c\n", tmp[i]);
        if((isalpha(tmp[i])) || (isdigit(tmp[i])) || (tmp[i] == '_') || (tmp[i] == ' ')) {
            continue;
        }
        else {
            ShowErrorPage(msg_err2);
            return -1;
        }
    }
    return 0;
}
