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
* wp_login_config.c
*
*
* CREATOR:
* autelan.software.Network Dep. team
* shaojw@autelan.com
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
#include "ws_user_limit.h"


int ShowExportConfPage(struct list *lpublic, struct list *lsystem);     /*m代表加密后的字符串*/

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
	  char buff[128];
	  memset(buff,0,128);

	  int i;
	  /*处理文本框内容和下拉框内容*/
	  char state[50];
	  memset(state,0,50);

	  char name[50];
	  memset(name,0,50);

	  char ruler[50];
	  memset(ruler,0,50);

	  char device[50];
	  memset(device,0,50);

	  char number[50];
	  memset(number,0,50); 
	
      char addn[N]="";	
	  char ID[N];   //获取页面传参id
	  memset(ID,0,N);
  
	  cgiFormStringNoNewlines("ID", ID, N); 

	  ST_LIMIT_ALL limitall;   //初始化总的
      memset(&limitall,0,sizeof(limitall));

      ST_LIMIT_INFO limitkeyl; //初始化单个的
      memset(&limitkeyl,0,sizeof(limitkeyl));

	  ST_LIMIT_INFO limitnew; //初始化单个的
      memset(&limitnew,0,sizeof(limitnew));

	
	  char log_encry[BUF_LEN];

	  int ret;
	 
	  if(cgiFormSubmitClicked("conf") != cgiFormSuccess)
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
	  fprintf(cgiOut,"<html xmlns=\"http://www.w3.org/1999/xhtml\"><head>\n");
	  fprintf(cgiOut,"<meta http-equiv=Content-Type content=text/html; charset=gb2312>\n");
	  fprintf(cgiOut,"<title>%s</title>\n",search(lsystem," "));
	   fprintf(cgiOut,"<link rel=stylesheet href=/style.css type=text/css>"\
  	  "<style type=text/css>"\
  	  "#div1{ width:62px; height:18px; border:1px solid #666666; background-color:#f9f8f7;}"\
	  "#div2{ width:60px; height:15px; padding-left:5px; padding-top:3px}"\
	  "#link{ text-decoration:none; font-size: 12px}"\
	  ".usrlis {overflow-x:hidden;	overflow:auto; width: 500px; height: 100px; clip: rect( ); padding-top: 0px; padding-right: 0px; padding-bottom: 0px; padding-left: 0px} "\
	  "</style>"\
	  "</head>"\
	  "<body>");

	   	if(cgiFormSubmitClicked("conf") == cgiFormSuccess)
	   		{
           //捕捉所有的值，进行对原来的修改，
           cgiFormStringNoNewlines("state", state, 50); 
		   cgiFormStringNoNewlines("name", name, 50);
		   cgiFormStringNoNewlines("ruler", ruler, 50);
		   cgiFormStringNoNewlines("device", device, 50); 
		   cgiFormStringNoNewlines("number", number, 50); 
		   cgiFormStringNoNewlines("id", ID, N); 
		   
		   if((strcmp(name,"") != 0) && (strcmp(number,"") !=0))
		   	{
     		   strcpy( limitnew.state,state);
     		   strcpy( limitnew.name,name);
     		   strcpy( limitnew.ruler,ruler);
     		   strcpy( limitnew.device,device);		   
     		   strcpy( limitnew.number,number);
     		   mod_limit_node(LIMIT_XML_PATH, LIMIT_NODE, LIMIT_NODE_ATT, ID,limitnew);
                //从xml文件中写入到conf文件中去
     		    read_limit_xml(LIMIT_XML_PATH, &limitall);
                ret=write_limit_config(&limitall,LIMIT_CONF_PATH);	
				if(ret==0)
		        ShowAlert(search(lpublic,"oper_succ"));
	            else
		        ShowAlert(search(lpublic,"oper_fail"));
				
     		   	}
     		   else
     		   	{
                  ShowAlert(search(lpublic,"l_alert"));
     		   	}
	   		}
		
	  fprintf(cgiOut,"<form method=post encType=multipart/form-data>\n"\
	  "<div align=center>\n"\
	  "<table width=976 border=0 cellpadding=0 cellspacing=0>\n");  //1111111111111111
	  fprintf(cgiOut,"<tr>\n"\
		"<td width=8 align=left valign=top background=/images/di22.jpg><img src=/images/youce4.jpg width=8 height=30/></td>"\
		"<td width=51 align=left valign=bottom background=/images/di22.jpg><img src=/images/youce33.jpg width=37 height=24/></td>"\
	  "<td width=153 align=left valign=bottom id=%s background=/images/di22.jpg>%s</td>",search(lpublic,"title_style"),search(lsystem,"sys_function"));
		fprintf(cgiOut,"<td width=690 align=right valign=bottom background=/images/di22.jpg>");
	  
	    fprintf(cgiOut,"<input type=hidden name=UN value=%s />",encry);
	 
			  fprintf(cgiOut,"<table width=130 border=0 cellspacing=0 cellpadding=0>");  //2222222222222
		fprintf(cgiOut,"<tr>"\
		"<td width=62 align=center><input id=but type=submit name=conf style=background-image:url(/images/%s) value=""></td>",search(lpublic,"img_ok"));
	
		if(cgiFormSubmitClicked("conf") != cgiFormSuccess)
		{
     		  fprintf(cgiOut,"<td width=62 align=left><a href=wp_login_limit.cgi?UN=%s target=mainFrame><img src=/images/%s border=0 width=62 height=20/></a></td>",encry,search(lpublic,"img_cancel"));

		}
     	else
     	{
       	    fprintf(cgiOut,"<td width=62 align=left><a href=wp_login_limit.cgi?UN=%s target=mainFrame><img src=/images/%s border=0 width=62 height=20/></a></td>",log_encry,search(lpublic,"img_cancel"));

			}
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
                     	
						

                            for(i=0;i<8;i++)
             					{
             					    fprintf(cgiOut,"<tr height=25>"\
             						"<td id=tdleft>&nbsp;</td>"\
             					  "</tr>");
             					}               

                
				       fprintf(cgiOut,"</table>"); //555555555555555555
				        fprintf(cgiOut,"</td>"\
				        "<td  align=left style=\"background-color:#ffffff; border-right:1px solid #707070; padding-left:30px\">\n");

						fprintf(cgiOut,"<table width=560 border=0 cellspacing=0 cellpadding=0>"); //666666666666

////////////////////////开始
                     fprintf(cgiOut,"<tr>");
                     fprintf(cgiOut,"<th></th>");
					 fprintf(cgiOut,"<th></th>");
					 fprintf(cgiOut,"<th></th>");
					 fprintf(cgiOut,"</tr>");

                     ret=find_limit_node(LIMIT_XML_PATH, LIMIT_NODE, LIMIT_NODE_ATT, ID, &limitkeyl);

                     fprintf(cgiOut,"<tr>");
					 fprintf(cgiOut,"<td>");				 
					 fprintf(cgiOut,"<font color=red>%s:&nbsp;</font>",search(lpublic,"l_state"));
					 fprintf(cgiOut,"</td>");
					 /*state*/
					 fprintf(cgiOut,"<td>");
					 fprintf(cgiOut,"<select  style=width:120px  name=state>\n");
					 if(strcmp(limitkeyl.state,"on") == 0){
						fprintf(cgiOut,"<option value=off>%s</option>","off");
					 	fprintf(cgiOut,"<option selected value=on>%s</option>","on"); 	

					 }else{
						fprintf(cgiOut,"<option selected value=off>%s</option>","off");
					 	fprintf(cgiOut,"<option value=on>%s</option>","on");
					 
					 }
					 fprintf(cgiOut,"</select>\n");	
					 fprintf(cgiOut,"</td><td>\n");
					 fprintf(cgiOut,"<font color=red size=2>(%s)</font>",search(lpublic,"l_state_des"));
					 fprintf(cgiOut,"</td>");
					 fprintf(cgiOut,"</tr>\n");

					 fprintf(cgiOut,"<tr height=15><td></td></tr>\n");

					 
				     fprintf(cgiOut,"<tr>");
					 fprintf(cgiOut,"<td>");				 
					 fprintf(cgiOut,"<font color=red>%s:&nbsp;</font>",search(lpublic,"l_name"));
					 fprintf(cgiOut,"</td>");
					 /*domain*/
					 fprintf(cgiOut,"<td>");
					 fprintf(cgiOut,"<input type=text name=name value=\"%s\" style=width:120px>\n",limitkeyl.name);	
					 fprintf(cgiOut,"</td><td>\n");
					 fprintf(cgiOut,"<font color=red size=2>(%s)</font>",search(lpublic,"l_name_des"));
					 fprintf(cgiOut,"</td>");
					 fprintf(cgiOut,"</tr>\n");

					 fprintf(cgiOut,"<tr height=15><td></td></tr>\n");

					 /*type*/
					 fprintf(cgiOut,"<tr>");
                     fprintf(cgiOut,"<td><font color=red>%s:&nbsp;</font>",search(lpublic,"l_type"));
					 fprintf(cgiOut,"</td>");
					 fprintf(cgiOut,"<td>");
					 fprintf(cgiOut,"<select  style=width:120px  name=ruler>\n");
					 if(strcmp("soft",limitkeyl.ruler) == 0){
						fprintf(cgiOut,"<option selected value=soft>%s</option>","soft");
						fprintf(cgiOut,"<option value=hard>%s</option>","hard");
						fprintf(cgiOut,"<option value=all>%s</option>","all");
					 }else if(strcmp("hard",limitkeyl.ruler) == 0){
						fprintf(cgiOut,"<option value=soft>%s</option>","soft");
						fprintf(cgiOut,"<option selected value=hard>%s</option>","hard");
						fprintf(cgiOut,"<option value=all>%s</option>","all");
					 }else{
						fprintf(cgiOut,"<option value=soft>%s</option>","soft");
						fprintf(cgiOut,"<option value=hard>%s</option>","hard");
						fprintf(cgiOut,"<option selected value=all>%s</option>","all");
					 }
					 
					 fprintf(cgiOut,"</select>\n");
					 fprintf(cgiOut,"</td><td>");
					 fprintf(cgiOut,"<font color=red size=2>(%s)</font>",search(lpublic,"l_type_des"));
					 fprintf(cgiOut,"</td>");
					 fprintf(cgiOut,"</tr>");

                     fprintf(cgiOut,"<tr height=15><td></td></tr>\n");
					 
					 /*item*/
					 fprintf(cgiOut,"<tr>");
					 fprintf(cgiOut,"<td><font color=red>%s:&nbsp;</font>",search(lpublic,"l_item"));
					 fprintf(cgiOut,"</td>");
					 fprintf(cgiOut,"<td>");
					 fprintf(cgiOut,"<select  style=width:120px  name=device>\n");
					 if(strcmp("maxlogins",limitkeyl.device) == 0){
						 fprintf(cgiOut,"<option selected value=maxlogins>%s</option>","maxlogins");
						 fprintf(cgiOut,"<option value=maxsyslogins>%s</option>","maxsyslogins");
						 fprintf(cgiOut,"<option value=nproc>%s</option>","nproc");
						 fprintf(cgiOut,"<option value=rss>%s</option>","rss");
						 fprintf(cgiOut,"<option value=core>%s</option>","core");
					 }else if(strcmp("maxsyslogins",limitkeyl.device) == 0){
						 fprintf(cgiOut,"<option value=maxlogins>%s</option>","maxlogins");
						 fprintf(cgiOut,"<option selected value=maxsyslogins>%s</option>","maxsyslogins");
						 fprintf(cgiOut,"<option value=nproc>%s</option>","nproc");
						 fprintf(cgiOut,"<option value=rss>%s</option>","rss");
						 fprintf(cgiOut,"<option value=core>%s</option>","core");
					 }else if(strcmp("nproc",limitkeyl.device) == 0){
						 fprintf(cgiOut,"<option value=maxlogins>%s</option>","maxlogins");
						 fprintf(cgiOut,"<option value=maxsyslogins>%s</option>","maxsyslogins");
						 fprintf(cgiOut,"<option selected value=nproc>%s</option>","nproc");
						 fprintf(cgiOut,"<option value=rss>%s</option>","rss");
						 fprintf(cgiOut,"<option value=core>%s</option>","core");
					 }else if(strcmp("rss",limitkeyl.device) == 0){
						 fprintf(cgiOut,"<option value=maxlogins>%s</option>","maxlogins");
						 fprintf(cgiOut,"<option value=maxsyslogins>%s</option>","maxsyslogins");
						 fprintf(cgiOut,"<option value=nproc>%s</option>","nproc");
						 fprintf(cgiOut,"<option selected value=rss>%s</option>","rss");
						 fprintf(cgiOut,"<option value=core>%s</option>","core");
					 }else{
						 fprintf(cgiOut,"<option value=maxlogins>%s</option>","maxlogins");
						 fprintf(cgiOut,"<option value=maxsyslogins>%s</option>","maxsyslogins");
						 fprintf(cgiOut,"<option value=nproc>%s</option>","nproc");
						 fprintf(cgiOut,"<option value=rss>%s</option>","rss");
						 fprintf(cgiOut,"<option selected value=core>%s</option>","core");
					 }

					 fprintf(cgiOut,"</select>\n");
					 fprintf(cgiOut,"</td><td>");
					 fprintf(cgiOut,"<font color=red size=2>(%s)</font>",search(lpublic,"l_item_des"));
					 fprintf(cgiOut,"</td>");
					 fprintf(cgiOut,"</tr>");

                     fprintf(cgiOut,"<tr height=15><td></td></tr>\n");

					  /*value*/
					 fprintf(cgiOut,"<tr>");
					 fprintf(cgiOut,"<td><font color=red>%s:&nbsp;</font>",search(lpublic,"l_num"));
					 fprintf(cgiOut,"</td>");
					 fprintf(cgiOut,"<td>");
					 fprintf(cgiOut,"<input type=text name=number value=\"%s\" style=width:120px>\n",limitkeyl.number);
                     fprintf(cgiOut,"</td><td>");
					 fprintf(cgiOut,"<font color=red size=2>(%s)</font>",search(lpublic,"l_num_des"));
					 fprintf(cgiOut,"</td>");
					 
					 fprintf(cgiOut,"</tr>");
					 	
//////////////////////////结束

			
                    fprintf(cgiOut,"<tr height=25><td></td></tr>\n");
					
				   if(cgiFormSubmitClicked("conf") != cgiFormSuccess)
				   {
					 fprintf(cgiOut,"<tr><td><input type=hidden name=encry_import value=%s><input type=hidden name=id value=%s></td></tr>\n",encry,ID);
				   }
				   else if(cgiFormSubmitClicked("conf") == cgiFormSuccess)
				   {
					 fprintf(cgiOut,"<tr><td><input type=hidden name=encry_import value=%s><input type=hidden name=id value=%s></td></tr>\n",log_encry,ID);
				   }	 
							
   fprintf(cgiOut,"</table>"); //666666666666666
			  fprintf(cgiOut,"</td>\n"\
			  "</tr>\n"\
			  "<tr height=4 valign=top>\n"\
				  "<td width=120 height=4 align=right valign=top><img src=/images/bottom_07.gif width=1 height=10/></td>\n"\
				  "<td width=827 height=4 valign=top bgcolor=#FFFFFF><img src=/images/bottom_06.gif width=827 height=15/></td>\n"\
			  "</tr>"\
			"</table>");//444444444444
		  fprintf(cgiOut,"</td>\n"\
		  "<td width=15 background=/images/di999.jpg>&nbsp;</td>\n"\
		"</tr>\n"\
	  "</table></td>\n"); //333333333333
	fprintf(cgiOut,"</tr>\n"\
	"<tr>\n"\
		"<td colspan=3 align=left valign=top background=/images/di777.jpg><img src=/images/di555.jpg width=61 height=62/></td>\n"\
	  "<td align=left valign=top background=/images/di777.jpg>&nbsp;</td>\n"\
		"<td align=left valign=top background=/images/di777.jpg><img src=/images/di666.jpg width=74 height=62/></td>\n"\
	"</tr>\n"\
	"</table>\n");//111111111111111
	fprintf(cgiOut,"</div>\n"\
	"</form>\n"\
	"</body>\n"\
	"</html>\n");
free(encry); 
return 0;
}  


#if 0//these functions are defined in ws_log_conf.c
/*测试是否含有字母*/
int check_abc_d(char *ff)
{
 int pf,i,j;
 pf = strtoul(ff,0,10);//参数合法性检查，如果是字符串，就返回了零	不确切			
 char *df;
 df=(char *)malloc(sizeof(ff));
 
 sprintf(df,"%d",pf);
 
 i=strlen(ff);
 j=strlen(df);
 free(df);
 
 if(i!=j)
 	return -1;  //有错
 else
     return 0;

}

#endif
