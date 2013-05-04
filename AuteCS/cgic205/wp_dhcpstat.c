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
* wp_dhcpstat.c
*
*
* CREATOR:
* autelan.software.Network Dep. team
* liutao@autelan.com
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
#include <sys/types.h>
#include <unistd.h>
#include <libxml/xpathInternals.h>


int ShowDhcpconPage(struct list *lcontrol,struct list *lpublic);
void ShowIPaddr(struct list *lcontrol,struct list *lpublic,char* addn);

void xml_creat();
//void count_range();
int cgiMain()
{
	
	
	struct list *lcontrol;
	struct list *lpublic;
	lcontrol = get_chain_head("../htdocs/text/control.txt");
	lpublic= get_chain_head("../htdocs/text/public.txt");
//	count_range();
	xml_creat();
	ShowDhcpconPage(lcontrol,lpublic);
	release(lcontrol);
	release(lpublic); 
	
	return 0;
}

void xml_creat()
{
		
		char buf[256];
		char *seg=(char*)malloc(5);
		char *tmp = (char*)malloc(40);
		char *tmp2 = (char*)malloc(40);
		char *whole2=(char*)malloc(40);
		unsigned long int whole = 0;
		char *count1 = (char*)malloc(40);
		char *count2 = (char*)malloc(40);
		memset(count1,0,40);
		memset(count2,0,40);
		memset(tmp,0,40);
		memset(tmp2,0,40);
		memset(whole2,0,40);
		memset(seg,0,5);
		int t = 0;
		int m = 0;
		unsigned long int t11 = 0;
		unsigned long int t12 = 0;
		unsigned long int t13 = 0;
		unsigned long int t14 = 0;
		unsigned long int t21 = 0;
		unsigned long int t22 = 0;
		unsigned long int t23 = 0;
		unsigned long int t24 = 0;
		int retu = system("ip_ser.sh");
		if(retu != 0)
			{
			fprintf(stderr,"exec ip_ser.sh failed!");
				return;
				}
		int ret=system("count_rn.sh");
		if(ret!=0)
			{
				fprintf(stderr,"exec count_rn.sh failed!");
				return;
			}
		FILE*fd;
		fd=fopen("/var/run/apache2/dhcp_rang.tmp","r");
		if(fd==NULL)
			{
				fprintf(stderr,"can not open dhcp_rang.tmp");
				return;
			}
		 fseek(fd,0,0);
		while(fgets(buf,256,fd))
			{
				t =0;
				tmp = strtok(buf," ");
					while(tmp!=NULL)
						{
							t++;					
							if(t==1)
								{
									strcpy(count1,tmp);
								}
							else if(t==2)
								{
									strcpy(count2,tmp);
								}
						tmp=strtok(NULL," ");
						}
			tmp2 = strtok(count1,".");
			m = 0;
			while(tmp2!=NULL)
				{
					m++;
					if(m ==1)
						{
							t11 = strtol(tmp2,NULL,10);
					//		fprintf(stderr,"t11======%ld",t11);
						}
					else if(m==2)
						{
							t12 = strtol(tmp2,NULL,10);
					//		fprintf(stderr,"t12======%ld",t12);
						}
					else if(m==3)
						{
							t13 = strtol(tmp2,NULL,10);
							//fprintf(stderr,"t13======%ld",t13);
						}
					else if(m==4)
						{
							t14 = strtol(tmp2,NULL,10);
							//fprintf(stderr,"t14======%ld",t14);
						}
				tmp2 = strtok(NULL,".");
				}
			tmp2 = strtok(count2,".");
			m = 0;
			while(tmp2!=NULL)
				{
					m++;
					if(m ==1)
						{
							t21 = strtol(tmp2,NULL,10);
						}
					else if(m==2)
						{
							t22 = strtol(tmp2,NULL,10);		
						}
					else if(m==3)
						{
							t23 = strtol(tmp2,NULL,10);
						}
					else if(m==4)
						{
							t24 = strtol(tmp2,NULL,10);
						}
				tmp2 = strtok(NULL,".");
				}
			
			t21 = t21-t11;
			t22 = t22-t12;
			t23 = t23-t13;
			t24 = t24-t14+1;
		//	fprintf(stderr,"t21======%ld\n",t21);
		//	fprintf(stderr,"t22======%ld\n",t22);
		//	fprintf(stderr,"t23======%ld\n",t23);
		//	fprintf(stderr,"t24======%ld\n",t24);
			whole = whole + t21*255*255*255+t22*255*255+t23*255+t24;
			}
	    FILE * fd2=fopen("/var/run/apache2/dhcp_stat.tmp","r");
		if(fd2==NULL)
			{
				fprintf(stderr,"can not open dhcp_stat!");
			}
		else
		{
			fgets(seg,5,fd2);
			fclose(fd2);
		}
		sprintf(whole2,"%ld",whole);
		xmlDocPtr doc = NULL;		
		xmlNodePtr root_node = NULL, node = NULL;
		doc = xmlNewDoc(BAD_CAST "1.0");
		root_node = xmlNewNode(NULL, BAD_CAST "root");
		
		xmlDocSetRootElement(doc, root_node);
		
		xmlNewChild(root_node, NULL, BAD_CAST "whole",BAD_CAST whole2);
		
		node=xmlNewChild(root_node, NULL, BAD_CAST "seg",BAD_CAST seg);
		
		xmlSaveFormatFileEnc("../htdocs/dhcp/pool_seg.xml", doc, "UTF-8", 1);
		//xmlSaveFile("../htdocs/ApMonitor/ApInfo.xml",doc);
		
		xmlFreeDoc(doc);
		xmlCleanupParser();
		xmlMemoryDump();//debug memory for regression tests
		free(tmp);
		free(tmp2);
		free(seg);
		free(count1);
		free(count2);
		free(whole2);
		fclose(fd);
		   
  return ;
}


int ShowDhcpconPage(struct list *lcontrol,struct list *lpublic)
{ 
  char *encry=(char *)malloc(BUF_LEN);
  
  FILE *fp;
  char lan[3];
  
  char dhcp_encry[BUF_LEN]; 
  char addn[N];         
 
  int i = 0;   

 
  
  	
 
  cgiFormStringNoNewlines("encry_dhcp",dhcp_encry,BUF_LEN);
  
  cgiHeaderContentType("text/html");
  fprintf(cgiOut,"<html xmlns=\"http://www.w3.org/1999/xhtml\"><head>");
  fprintf(cgiOut,"<meta http-equiv=Content-Type content=text/html; charset=gb2312>");
  	//下面三句话用于禁止页面缓存
  fprintf( cgiOut, "<META   HTTP-EQUIV=\"pragma\"   CONTENT=\"no-cache\"> \n");
  fprintf( cgiOut, "<META   HTTP-EQUIV=\"Cache-Control\"   CONTENT=\"no-cache,   must-revalidate\"> \n" );
  fprintf( cgiOut, "<META   HTTP-EQUIV=\"expires\"   CONTENT=\"Wed,   26   Feb   1997   08:21:57   GMT\">	\n");
  fprintf(cgiOut,"<link rel=stylesheet href=/style.css type=text/css>"\
  	"<style type=text/css>"\
  	".a3{width:30;border:0; text-align:center}"\
  	"</style>"\
  "</head>"\
  
  "<script type=\"text/javascript\">"\
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
  "</script>"\
  "<script language=javascript src=/ip.js>"\
  "</script>"\
  "<body>");

  
  fprintf(cgiOut,"<form method=post encType=multipart/form-data>"\
  "<div align=center>"\
  "<table width=976 border=0 cellpadding=0 cellspacing=0>"\
  "<tr>"\
    "<td width=8 align=left valign=top background=/images/di22.jpg><img src=/images/youce4.jpg width=8 height=30/></td>"\
    "<td width=51 align=left valign=bottom background=/images/di22.jpg><img src=/images/youce33.jpg width=37 height=24/></td>"\
    "<td width=153 align=left valign=bottom id=%s background=/images/di22.jpg>%s</td>",search(lpublic,"title_style"),"DHCP");
    fprintf(cgiOut,"<td width=690 align=right valign=bottom background=/images/di22.jpg>");
	    if((fp=fopen("../htdocs/text/public.txt","r"))==NULL)		 /*以只读方式打开资源文件*/
		{
			ShowAlert(search(lcontrol,"error_open"));
	    }
	    else
	    {
			fseek(fp,4,0);						/*将文件指针移到离文件首4个字节处，即lan=之后*/
			fgets(lan,3,fp);	   
			fclose(fp);
	    }
	    if(strcmp(lan,"ch")==0)
    	{	
    	  fprintf(cgiOut,"<table width=130 border=0 cellspacing=0 cellpadding=0>"\
          "<tr>");
          if(checkuser_group(addn)==0)  /*管理员*/
          	{
				 fprintf(cgiOut,"<td width=62 align=left><a href=wp_contrl.cgi?UN=%s target=mainFrame><img src=/images/%s border=0 width=62 height=20/></a></td>",encry,search(lpublic,"img_ok"));	
		  	}
		  else
		  	{	
            fprintf(cgiOut,"<td width=62 align=left><a href=wp_contrl.cgi?UN=%s target=mainFrame><img src=/images/%s border=0 width=62 height=20/></a></td>",encry,search(lpublic,"img_ok"));	  
		  	}
            fprintf(cgiOut,"<td width=62 align=left><a href=wp_contrl.cgi?UN=%s target=mainFrame><img src=/images/%s border=0 width=62 height=20/></a></td>",encry,search(lpublic,"img_cancel"));
		  fprintf(cgiOut,"</tr>"\
          "</table>");
		}		
		else			
		{	
		  fprintf(cgiOut,"<table width=130 border=0 cellspacing=0 cellpadding=0>"\
		  "<tr>");
		 if(checkuser_group(addn)==0)  /*管理员*/
		 	{
		  		fprintf(cgiOut,"<td width=62 align=center><input id=but type=submit name=dhcp_conf style=background-image:url(/images/ok-en.jpg) value=""></td>");		  
		 	}
		 else
		 	{
			 if(cgiFormSubmitClicked("dhcp_conf") != cgiFormSuccess)
				 fprintf(cgiOut,"<td width=62 align=left><a href=wp_contrl.cgi?UN=%s target=mainFrame><img src=/images/ok-en.jpg border=0 width=62 height=20/></a></td>",encry);
			  else
				fprintf(cgiOut,"<td width=62 align=left><a href=wp_contrl.cgi?UN=%s target=mainFrame><img src=/images/ok-en.jpg border=0 width=62 height=20/></a></td>",dhcp_encry);
				
		 	}
		  if(cgiFormSubmitClicked("dhcp_conf") != cgiFormSuccess)
		    fprintf(cgiOut,"<td width=62 align=left><a href=wp_contrl.cgi?UN=%s target=mainFrame><img src=/images/cancel-en.jpg border=0 width=62 height=20/></a></td>",encry);
		  else
		    fprintf(cgiOut,"<td width=62 align=left><a href=wp_contrl.cgi?UN=%s target=mainFrame><img src=/images/cancel-en.jpg border=0 width=62 height=20/></a></td>",dhcp_encry);
		  fprintf(cgiOut,"</tr>"\
		  "</table>");
		}
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
         			"<td align=left id=tdleft background=/images/bottom_bg.gif style=\"border-right:0\"><font id=%s> %s</font></td>",search(lpublic,"menu_san"),search(lcontrol,"stat"));   /*突出显示*/
         			fprintf(cgiOut,"</tr>");
				for(i=0;i<20;i++)
					{
						fprintf(cgiOut,"<tr height=25>"\
						  "<td id=tdleft>&nbsp;</td>"\
						"</tr>");
					}
 fprintf(cgiOut,"</table>"\
              "</td>"\
              "<td align=left valign=top style=\"background-color:#ffffff; border-right:1px solid #707070; padding-left:30px; padding-top:10px\">");





					
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
return 0;
}

