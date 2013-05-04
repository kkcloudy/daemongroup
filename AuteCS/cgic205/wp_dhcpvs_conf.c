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
* wp_dhcpvs_conf.c
*
*
* CREATOR:
* autelan.software.Network Dep. team
* zhouym@autelan.com
*
* DESCRIPTION:
* system cotrl for dhcp  
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
#include "ws_dhcp_conf.h"

int ShowDhcpconPage(struct list *lcontrol,struct list *lpublic);
void ShowIPvsaddr(struct list *lcontrol,struct list *lpublic,char* addn);


int cgiMain()
{
	char *tmp=(char *)malloc(64);
	int ret=-1;
	
	struct list *lcontrol;
	struct list *lpublic;
	lcontrol = get_chain_head("../htdocs/text/control.txt");
	lpublic= get_chain_head("../htdocs/text/public.txt");

	if(access(DHCPVS_XML,0)!=0)
	{
		create_dhcp_xml(DHCPVS_XML);
	}
	else
	{
	  ret=if_xml_file(DHCPVS_XML);
	  if(ret!=0)
	  {
		   memset(tmp,0,64);
		   sprintf(tmp,"sudo rm  %s > /dev/null",DHCPVS_XML);
		   system(tmp);
		   create_dhcp_xml(DHCPVS_XML);

	  }
	}

	if(access(DHCPVS_XML_STATE,0)!=0)
	{
	   memset(tmp,0,64);
	   sprintf(tmp,"echo \"start\" > %s ",DHCPVS_XML_STATE);
		system(tmp);
	}
	
	
	
	ShowDhcpconPage(lcontrol,lpublic);
	release(lcontrol);
	release(lpublic); 
	free(tmp);
	
	return 0;
}



int ShowDhcpconPage(struct list *lcontrol,struct list *lpublic)
{ 
  char *encry=(char *)malloc(BUF_LEN);
 
  char *str;
 
  struct dvsconfz c_head,*cq;
  int cnum,cflag=-1;
  
  
  char dhcp_encry[BUF_LEN]; 
  char addn[N];         
 
  int i = 0;   
  int cl=1;
  if(cgiFormSubmitClicked("dhcp_conf") != cgiFormSuccess)
  {
	memset(encry,0,BUF_LEN);
    cgiFormStringNoNewlines("UN", encry, BUF_LEN); 
    str=dcryption(encry);
    if(str==NULL)
    {
      ShowErrorPage(search(lpublic,"ill_user")); 	 /*用户非法*/
	}
	strcpy(addn,str);
	memset(dhcp_encry,0,BUF_LEN);                   /*清空临时变量*/
  }
  else
  	{
  	cgiFormStringNoNewlines("encry_dhcp", dhcp_encry, BUF_LEN); 
    str=dcryption(dhcp_encry);
    if(str==NULL)
    {
      ShowErrorPage(search(lpublic,"ill_user")); 	 /*用户非法*/
	}
	strcpy(addn,str);
	memset(dhcp_encry,0,BUF_LEN);                   /*清空临时变量*/
	
  	}
 
  cgiFormStringNoNewlines("encry_dhcp",dhcp_encry,BUF_LEN);
  
  cgiHeaderContentType("text/html");
  fprintf(cgiOut,"<html xmlns=\"http://www.w3.org/1999/xhtml\"><head>");
  fprintf(cgiOut,"<meta http-equiv=Content-Type content=text/html; charset=gb2312>");
  fprintf(cgiOut,"<link rel=stylesheet href=/style.css type=text/css>"\
	  "<style type=text/css>"\
  	  "#div1{ width:62px; height:18px; border:1px solid #666666; background-color:#f9f8f7;}"\
	  "#div2{ width:60px; height:15px; padding-left:5px; padding-top:3px}"\
	  "#link{ text-decoration:none; font-size: 12px}"\
	  ".dhcplis {overflow-x:hidden;	overflow:auto; width: 716px; height: 420px; clip: rect( ); padding-top: 0px; padding-right: 0px; padding-bottom: 0px; padding-left: 0px} "\
	  "</style>"\
  "</head>"\
  "<body>");
  if(cgiFormSubmitClicked("dhcp_conf") == cgiFormSuccess)
	{
	 ShowIPvsaddr(lcontrol,lpublic,addn);
	}

   
  fprintf(cgiOut,"<form method=post >"\
  "<div align=center>"\
  "<table width=976 border=0 cellpadding=0 cellspacing=0>"\
  "<tr>"\
    "<td width=8 align=left valign=top background=/images/di22.jpg><img src=/images/youce4.jpg width=8 height=30/></td>"\
    "<td width=51 align=left valign=bottom background=/images/di22.jpg><img src=/images/youce33.jpg width=37 height=24/></td>"\
    "<td width=153 align=left valign=bottom id=%s background=/images/di22.jpg>%s</td>",search(lpublic,"title_style"),"DHCPV6");
    fprintf(cgiOut,"<td width=690 align=right valign=bottom background=/images/di22.jpg>");
	    
    	  fprintf(cgiOut,"<table width=130 border=0 cellspacing=0 cellpadding=0>"\
          "<tr>");
          if(checkuser_group(addn)==0)  /*管理员*/
          {
				 fprintf(cgiOut,"<td width=62 align=center><input id=but type=submit name=dhcp_conf style=background-image:url(/images/%s) value=""></td>",search(lpublic,"img_ok"));
		  }
		  else
		  {
          	if(cgiFormSubmitClicked("dhcp_conf") != cgiFormSuccess)
            fprintf(cgiOut,"<td width=62 align=left><a href=wp_contrl.cgi?UN=%s target=mainFrame><img src=/images/%s border=0 width=62 height=20/></a></td>",encry,search(lpublic,"img_ok"));
		   else                                         
     		fprintf(cgiOut,"<td width=62 align=left><a href=wp_contrl.cgi?UN=%s target=mainFrame><img src=/images/%s border=0 width=62 height=20/></a></td>",dhcp_encry,search(lpublic,"img_ok"));	  
		  }
		  if(cgiFormSubmitClicked("dhcp_conf") != cgiFormSuccess)
            fprintf(cgiOut,"<td width=62 align=left><a href=wp_dhcpview.cgi?UN=%s target=mainFrame><img src=/images/%s border=0 width=62 height=20/></a></td>",encry,search(lpublic,"img_cancel"));
		  else                                         
     		fprintf(cgiOut,"<td width=62 align=left><a href=wp_dhcpview.cgi?UN=%s target=mainFrame><img src=/images/%s border=0 width=62 height=20/></a></td>",dhcp_encry,search(lpublic,"img_cancel"));
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
         			fprintf(cgiOut,"<tr height=26>"\
         			"<td align=left id=tdleft background=/images/bottom_bg.gif style=\"border-right:0\"><font id=yingwen_er>DHCPV6</font><font id=%s> %s</font></td>",search(lpublic,"menu_san"),search(lcontrol,"dhcp_con"));   /*突出显示*/
         			fprintf(cgiOut,"</tr>");
         		if(cgiFormSubmitClicked("dhcp_conf") != cgiFormSuccess)
         		{ 		
         			 if(checkuser_group(addn)==0)  /*管理员*/
					{
						fprintf(cgiOut,"<tr height=25>"\
						"<td align=left id=tdleft><a href=wp_dhcpvs_add.cgi?UN=%s target=mainFrame class=top><font id=yingwen_er>DHCPV6</font><font id=%s> %s<font></a></td>",encry,search(lpublic,"menu_san"),search(lcontrol,"add"));
						fprintf(cgiOut,"</tr>");   						
						fprintf(cgiOut,"<tr height=25>"\
						"<td align=left id=tdleft><a href=wp_dhcpvs_inf.cgi?UN=%s target=mainFrame class=top><font id=yingwen_er>DHCPV6</font><font id=%s> %s<font></a></td>",encry,search(lpublic,"menu_san"),search(lcontrol,"interface_info"));
						fprintf(cgiOut,"</tr>");   						
					}
				}
         		else if(cgiFormSubmitClicked("dhcp_conf") == cgiFormSuccess) 			  
         		{			
         			if(checkuser_group(addn)==0)  /*管理员*/		   
         			{
	         			fprintf(cgiOut,"<tr height=25>"\
	         			"<td align=left id=tdleft><a href=wp_dhcpvs_add.cgi?UN=%s target=mainFrame class=top><font id=yingwen_er>DHCPV6</font><font id=%s> %s<font></a></td>",dhcp_encry,search(lpublic,"menu_san"),search(lcontrol,"add"));
	         			fprintf(cgiOut,"</tr>");	
	         			fprintf(cgiOut,"<tr height=25>"\
	         			"<td align=left id=tdleft><a href=wp_dhcpvs_inf.cgi?UN=%s target=mainFrame class=top><font id=yingwen_er>DHCPV6</font><font id=%s> %s<font></a></td>",dhcp_encry,search(lpublic,"menu_san"),search(lcontrol,"interface_info"));
	         			fprintf(cgiOut,"</tr>");						
					}
				}
			
			for(i=0;i<18;i++)
			{
						fprintf(cgiOut,"<tr height=25>"\
						  "<td id=tdleft>&nbsp;</td>"\
						"</tr>");
			}

				
			 char *status=(char*)malloc(20);
		     memset(status,0,20);
				 
		     fprintf(cgiOut,"</table>"\
              "</td>"\
              "<td align=left style=\"background-color:#ffffff; border-right:1px solid #707070; padding-left:30px; padding-top:10px\">");
				
				


			fprintf(cgiOut,"<table width=500 border=0 cellspacing=0 cellpadding=0>");					
			FILE* pb=fopen(DHCPVS_XML_STATE,"r");
			if(pb != NULL)
			{
				fgets(status,20,pb);
				fclose(pb);
			}		  
			fprintf(cgiOut,"<tr>"\
			"<td width=80>%s</td>",search(lcontrol,"dhcp_status"));
			if(!strstr(status,"stop"))
			{
				fprintf(cgiOut,"<td width=420 align=left>");
				fprintf(cgiOut,"<select name=State>");
				fprintf(cgiOut,"<option>stop</option>"\
				"<option selected=\"selected\">start</option>"\
				"</select>"\
				"</td>");
			}
			else
			{
				fprintf(cgiOut,"<td width=420 align=left>");
				fprintf(cgiOut,"<select name=State>");
				fprintf(cgiOut,"<option >start</option>"\
				"<option selected=\"selected\">stop</option>"\
				"</select>"\
				"</td>");				
			}


			fprintf(cgiOut,"</tr>"\
			"<tr>"\
			"<td colspan=2 id=sec style=\"border-bottom:2px solid #53868b\">&nbsp;</td>"\
			"</tr>"\
			"<tr>"\
			"<td colspan=2 style=\"padding-top:20px\">");
			fprintf(cgiOut,"<div class=dhcplis>\n");
			fprintf(cgiOut,"<table width=700 border=1 frame=below rules=rows bordercolor=#cccccc cellspacing=0 cellpadding=0>");
			fprintf(cgiOut,"<tr >");
			//fprintf(cgiOut,"<th width=150 align=left style=font-size:14px>%s</th>",search(lpublic,"dhcp_rname"));
			fprintf(cgiOut,"<th width=150 align=left style=font-size:14px>%s</th>",search(lcontrol,"subnet"));
			fprintf(cgiOut,"<th width=150 align=left style=font-size:14px>%s</th>",search(lcontrol,"mask"));
			fprintf(cgiOut,"<th width=80 >&nbsp;</th>");
			fprintf(cgiOut,"<th width=80>&nbsp;</th>");
			fprintf(cgiOut,"</tr>");


			////////////////////////////////////////////modify
             cflag=read_dvsconf_xml(DHCPVS_XML, &c_head, &cnum);
			 if(cflag==0)
			 {
	             cq=c_head.next;
				
	            while(cq !=NULL)
				{
				  fprintf(cgiOut,"<tr bgcolor=%s>\n",setclour(cl));
				  //fprintf(cgiOut,"<td width=150 style=font-size:14px align=left>%s</td>",cq->confname);
				  fprintf(cgiOut,"<td width=150 style=font-size:14px align=left>%s</td>",cq->ssubnet);
				  fprintf(cgiOut,"<td width=150 style=font-size:14px align=left>%s</td>",cq->snetmask);
					if(checkuser_group(addn)==0)  /*管理员*/
					{
						if(cgiFormSubmitClicked("dhcp_conf") != cgiFormSuccess)
						{
							fprintf(cgiOut,"<td  align=left><a href=wp_dhcpvs_edit.cgi?UN=%s&NAME=%s&TYPE=%s target=mainFrame><font color=black>%s</font></td>",encry,cq->ssubnet,"1",search(lcontrol,"edit"));			
							fprintf(cgiOut,"<td  align=left><a href=wp_dhcpvs_edit.cgi?UN=%s&NAME=%s&TYPE=%s target=mainFrame><font color=black>%s</font></td>",encry,cq->ssubnet,"2",search(lcontrol,"del"));
						}
						else if(cgiFormSubmitClicked("dhcp_conf") == cgiFormSuccess)
						{
							fprintf(cgiOut,"<td  align=left><a href=wp_dhcpvs_edit.cgi?UN=%s&NAME=%s&TYPE=%s target=mainFrame><font color=black>%s</font></td>",dhcp_encry,cq->ssubnet,"1",search(lcontrol,"edit"));			
							fprintf(cgiOut,"<td  align=left><a href=wp_dhcpvs_edit.cgi?UN=%s&NAME=%s&TYPE=%s target=mainFrame><font color=black>%s</font></td>",dhcp_encry,cq->ssubnet,"2",search(lcontrol,"del"));
						}
					}
					else
					{
						if(cgiFormSubmitClicked("dhcp_conf") != cgiFormSuccess)
						{
							fprintf(cgiOut,"<td style=font-size:14px align=left>%s</td>",search(lcontrol,"edit"));			
							fprintf(cgiOut,"<td style=font-size:14px align=left>%s</td>",search(lcontrol,"del"));
						}
						else if(cgiFormSubmitClicked("dhcp_conf") == cgiFormSuccess)
						{
							fprintf(cgiOut,"<td style=font-size:14px align=left>%s</td>",search(lcontrol,"edit"));			
							fprintf(cgiOut,"<td style=font-size:14px align=left>%s</td>",search(lcontrol,"del"));
						}

					}
					fprintf(cgiOut,"</tr>");
				  cl=!cl;								
				  cq=cq->next;
				}
		 	}
			////////////////////////////////////////////
			fprintf(cgiOut,"</table>");
			fprintf(cgiOut,"</div>\n");
			fprintf(cgiOut,"</td>"\
			"</tr>"
			"<tr>");
			if(cgiFormSubmitClicked("dhcp_conf") != cgiFormSuccess)
			{
				fprintf(cgiOut,"<td colspan=2><input type=hidden name=encry_dhcp value=%s></td>",encry);
			}
			else if(cgiFormSubmitClicked("dhcp_conf") == cgiFormSuccess)
			{
				fprintf(cgiOut,"<td colspan=2><input type=hidden name=encry_dhcp value=%s></td>",dhcp_encry);
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

free(encry);
free(status);

if((cflag==0 )&& (cnum > 0))
	Free_dvsconfz_info(&c_head);

return 0;
}
						 
void  ShowIPvsaddr(struct list *lcontrol,struct list *lpublic,char*addn)
{
	if(checkuser_group(addn)==0)
	{
		char * status_file = (char*)malloc(20);
		char * status = (char *)malloc(10);

		memset(status,0,10);
		int rec;
		int ff;
		int fd=-1;
		int count_s = 0;
		int count_e = 0;
		int result;	
		char **responses;
		char buf[100];
		char *inte = (char*)malloc(512);
		memset(inte,0,512);
		memset(status_file,0,20);
		strcpy(inte,"INTERFACES=\"");
		cgiFormStringNoNewlines("State",status,N);
		result = cgiFormStringMultiple("vote", &responses);
		int i = 0;
		while (responses[i]) 
		{
			strcat(inte,responses[i]);
			strcat(inte," ");
			i++;
		}
		strcat(inte,"\"");
		cgiStringArrayFree(responses);
		if(!strcmp(status,"start"))
		{	
			ff = stop_dhcpvs();
			rec = start_dhcpvs();
			FILE* pb;
			pb=popen("ls /var/run/","r");
			if(pb== NULL)
			{
				return;
			}
			while(fgets(buf,100,pb) != NULL)
			{
				if(strstr(buf,"dhcpd6.pid") != NULL)
				count_s++;
			}
			if(ff == 0 && rec == 0 && count_s > 0)
			{
				fd = open(DHCPVS_XML_STATE,O_RDWR|O_TRUNC|O_CREAT);
				if(fd == -1)
				{
					fprintf(stderr,"can not open /opt/services/status/dhcp_status.status!\n");
				}

				write(fd,status,10);
				ShowAlert(search(lcontrol,"DHCP_STAT1"));

			}

			else		 
			ShowAlert(search(lcontrol,"DHCP_STAT10"));
			pclose(pb);
			close(fd);
		}
		else
		{
			rec = stop_dhcpvs();
			FILE* pe;
			pe=popen("ls /var/run/","r");
			if(pe== NULL)
			{
				return;
			}
			while(fgets(buf,100,pe) != NULL)
			{
				if(strstr(buf,"dhcpd6.pid") != NULL)
				count_e++;
			}	

			if(  rec == 0 && count_e == 0)
			{
				fd = open(DHCPVS_XML_STATE,O_RDWR|O_TRUNC|O_CREAT);
				if(fd == -1)
				{
					fprintf(stderr,"can not open /opt/services/status/dhcp_status.status!\n");
				}
				write(fd,status,10);
				ShowAlert(search(lcontrol,"DHCP_STAT0"));
			}
			else
			ShowAlert(search(lcontrol,"DHCP_STAT01"));

			pclose(pe);
			close(fd);
		}
		free(inte);
		free(status);
		free(status_file);
	}
}




