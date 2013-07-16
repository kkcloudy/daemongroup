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
* wp_auth_license.c
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
#include "cgic.h"
#include <string.h>
#include <fcntl.h> 
#include <sys/stat.h> 
#include <stdlib.h>
#include "ws_err.h"
#include "ws_usrinfo.h"
#include "ws_ec.h"
#include "ws_dcli_license.h"
#include "ws_init_dbus.h"
#include <sys/wait.h>

          
int ShowLicensePage(struct list *lpublic, struct list *lsystem);     /*m代表加密后的字符串*/
int line_num(char *fpath,char *cmd);		

//view line num function
int  line_num(char *fpath,char *cmd)  
{

	FILE *fp;
	char buff[128];
	char temp[30];
	memset(temp,0,30);
	sprintf(temp,"%s %s",cmd,fpath);

	fp=popen(temp,"r");
	if(fp != NULL)
	{
		fgets( buff, sizeof(buff), fp );  
	}


	char input[128];
	memset(input,0,128);
	strcpy(input,buff);

	char *p;
	int i;

	p=strtok(input," ");
	if(p)
	i=atoi(p);

	if(fp != NULL)
	{
		pclose(fp); 
	}

	return i;	

}

int cgiMain()
{
	struct list *lpublic;	  /*解析public.txt文件的链表头*/
	struct list *lsystem;	  /*解析system.txt文件的链表头*/	
	lpublic=get_chain_head("../htdocs/text/public.txt");
	lsystem=get_chain_head("../htdocs/text/system.txt");

	ShowLicensePage(lpublic,lsystem);
  	release(lpublic);  
  	release(lsystem);

	return 0;
}

int ShowLicensePage(struct list *lpublic, struct list *lsystem)
{ 
  
  int ret_license;
  char tmp_license[50];//存放授权码
  FILE *fp_license = NULL;
  
  ret_license = license_request_cmd_func();  //通过这个函数，取到了机器码
  if( 0 == ret_license)
   {	  
	  fp_license = fopen("/var/run/license.tmp","r");  //机器码保存var/run/license.tmp?fp_license保存了机器码
	  if(NULL != fp_license)
	   {
		  fscanf(fp_license,"%s",tmp_license);
	   }
   }

  char *encry=(char *)malloc(BUF_LEN);      		
  char *str;
  int i;
  /*get serial number*/
  char serialtext[128];  //存放序列号
  memset(serialtext,0,128);
  
  memset(encry,0,BUF_LEN);
  cgiFormStringNoNewlines("UN", encry, BUF_LEN); 
  str=dcryption(encry);
  if(str==NULL)
  {
      ShowErrorPage(search(lpublic,"ill_user"));          /*用户非法*/
      return 0;
  }
  /***********************2009.6.16*********************/
  cgiHeaderContentType("text/html");

  fprintf(cgiOut,"<html xmlns=\"http://www.w3.org/1999/xhtml\"><head>");
  fprintf(cgiOut,"<meta http-equiv=Content-Type content=text/html; charset=gb2312>");
  fprintf(cgiOut,"<title>%s</title>",search(lsystem,"auth_title")); 
  fprintf(cgiOut,"<link rel=stylesheet href=/style.css type=text/css>"\
  	"<style type=text/css>"\
  	".a3{width:30;border:0; text-align:center}"\
  	"</style>"\
  "</head>"\
  "<body>");
           
  if(cgiFormSubmitClicked("install") == cgiFormSuccess)
  {
   int err_flag=-1;
   memset(serialtext,0,128);
   cgiFormStringNoNewlines("authtext", serialtext, 128); 
   
   ccgi_dbus_init(); // 调用底层初始化函数
   
   if(strcmp(serialtext,"")==0)
   	ShowAlert(search(lsystem,"auth_serial_null"));
   else
   {
        err_flag=license_install_cmd_func(serialtext);  //嗲用ws_dcli_license.c的函数
		
    	if(err_flag==1)
    	  ShowAlert(search(lpublic,"oper_succ"));   //弹出框提示 操作成功
    	else if(err_flag==0)
    	  ShowAlert(search(lpublic,"oper_fail"));  //弹出框提示 操作失败
    	else if(err_flag==-1)
          ShowAlert(search(lsystem,"auth_serial_err"));	 //弹出框提示 序列号错误
   }
    
  } 
  fprintf(cgiOut,"<form method=post encType=multipart/form-data>"\
  "<div align=center>"\
  "<table width=976 border=0 cellpadding=0 cellspacing=0>"\
"<tr>"\
    "<td width=8 align=left valign=top background=/images/di22.jpg><img src=/images/youce4.jpg width=8 height=30/></td>"\
    "<td width=51 align=left valign=bottom background=/images/di22.jpg><img src=/images/youce33.jpg width=37 height=24/></td>"\
  "<td width=153 align=left valign=bottom id=%s background=/images/di22.jpg>%s</td>",search(lpublic,"title_style"),search(lsystem,"auth_title"));
    fprintf(cgiOut,"<td width=690 align=right valign=bottom background=/images/di22.jpg>");
    fprintf(cgiOut,"<table width=130 border=0 cellspacing=0 cellpadding=0>"\
	"<tr>");
	fprintf(cgiOut,"<input type=hidden name=UN value=\"%s\">",encry);
	fprintf(cgiOut,"<td width=62 align=left><a href=wp_sysmagic.cgi?UN=%s target=mainFrame><img src=/images/%s border=0 width=62 height=20/></a></td>",encry,search(lpublic,"img_ok"));
	fprintf(cgiOut,"<td width=62 align=left><a href=wp_sysmagic.cgi?UN=%s target=mainFrame><img src=/images/%s border=0 width=62 height=20/></a></td>",encry,search(lpublic,"img_cancel"));
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
					//current only one item
					fprintf(cgiOut,"<tr height=26>"\
						"<td align=left id=tdleft background=/images/bottom_bg.gif style=\"border-right:0\"><font id=%s>%s</font></td>",search(lpublic,"menu_san"),search(lsystem,"auth_ap"));  /*突出显示*/
					fprintf(cgiOut,"</tr>");
				
				for(i=0;i<=12;i++)
				{
				  fprintf(cgiOut,"<tr height=25>"\
					"<td id=tdleft>&nbsp;</td>"\
				  "</tr>");
				}
			   fprintf(cgiOut,"</table>"\
			    "</td>"\
			    "<td align=left style=\"background-color:#ffffff; border-right:1px solid #707070; padding-left:30px\">"\
                "<table width=680 border=0 cellspacing=0 cellpadding=2>"\
			    "<tr>"\
				"<td width=60 align=left>");	
			   fprintf(cgiOut,"%s:",search(lsystem,"auth_ap"));//对应"ap授权"文本框钱的字体
               fprintf(cgiOut,"</td><td align=left>");
			   
			   //text and button			   
			   fprintf(cgiOut,"<input type=text name=authtext value=\"\" style=\"width:240px;\">");
			   fprintf(cgiOut,"<input type=submit name=install value=\"%s\" style=\"width:70px;\">",search(lsystem,"auth_install"));//对应"安装"按钮
			   
			   
			    
			   fprintf(cgiOut,"</td>"\
			   "</tr>");

			    fprintf(cgiOut,"<tr>"\
				"<td width=60 align=left>");	
			   fprintf(cgiOut,"%s:",search(lsystem,"machine_code")); //对应"机器码"
               fprintf(cgiOut,"</td><td align=left>");   
			   fprintf(cgiOut,"<input type=text name=authtext value=\"%s\" style=\"width:310px;\">",tmp_license);  // tmp_license保存的是机器码，把它打印出来
			   fprintf(cgiOut,"</td>"\
			   "</tr>");
			   
			   fprintf(cgiOut,"<tr height=15><td colspan=2>");
			   fprintf(cgiOut,"<font face=verdana color=green>%s:\n</font>", search(lsystem,"auth_alreadyindex")); 
			   			   fprintf(cgiOut,"<hr>"); 
			   

//display license_text_all_cmd

			   int ret_alreadyindex;
			   FILE *fp_alreadyindex = NULL;
			   int num = 0;  //license_number=num-2,num is the text line.
			   char alreadyindex[1024];
              
		      ret_alreadyindex = license_text_cmd_func();  //from this func ,get the license_text_all content.
              if( 0 == ret_alreadyindex)
              {	  
	            fp_alreadyindex = fopen("/var/run/maxwtpcount.lic","r");
	            //fp_alreadyindex = fopen("/var/run/stp.log","r"); //for test
	            if(fp_alreadyindex != NULL)
	             {
	             	num=line_num("/var/run/maxwtpcount.lic","wc -l");
	             	//num=line_num("/var/run/stp.log","wc -l"); //none of sense,only for test.
	             	if(num == 2)
	             	{
						fprintf(cgiOut,"<font face=verdana color=gray>%s</font>","No license text!");
	             	}
	            	else
	             	{
		             	num=num-2;
		             	//fprintf(cgiOut,"%d",num);
		             	fgets(alreadyindex,sizeof(alreadyindex),fp_alreadyindex);
		             	fprintf(cgiOut,"<textarea cols=105 rows=10 style=\"overflow:auto;border:0px;font-size:14px\" readonly=readonly>");
		             	while(num)
	             		{
		             	 	num--;
						 	fgets(alreadyindex,sizeof(alreadyindex),fp_alreadyindex);
						 	fprintf(cgiOut,"%s",alreadyindex);
						 	//fprintf(cgiOut,"%s---%d\n",alreadyindex,num);
						}
						fprintf(cgiOut,"</textarea>");
	             	}
					fclose(fp_alreadyindex); 

	             }
	            
	          }			    
			   fprintf(cgiOut,"</td></tr>");	            
			   fprintf(cgiOut,"<tr>"\
			   "<td colspan=2  style=\"padding-left:23px;width=500; border-bottom:2px solid #53868b;font-size:14px\">%s</td>",search(lpublic,"description"));//对应说明
			   fprintf(cgiOut,"</tr>");
			   fprintf(cgiOut,"<tr height=7><td></td></tr>\n");
			   //auth describe
			   fprintf(cgiOut,"<tr height=25 style=\"padding-left:23px; padding-top:2px\">"\
					 "<td colspan=2 style=font-size:14px;color:#FF0000>%s</td>"\
				   "</tr>",search(lsystem,"auth_ins_des"));
  fprintf(cgiOut,"<tr height=50></tr>"\
			"</table>"\
			"</td>"\
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
free(encry); 
return 0;
}         

