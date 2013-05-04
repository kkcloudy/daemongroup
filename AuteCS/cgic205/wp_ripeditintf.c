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
* wp_ripeditintf.c
*
*
* CREATOR:
* autelan.software.Network Dep. team
* tangsq@autelan.com
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
#include "ws_usrinfo.h"
#include "ws_err.h"
#include "ws_ec.h"


#include <sys/wait.h>

#define Info_Num 20

int ShowEditIntfPage(); 
int  interfaceInfo(char *iname,char * intfname[],int * infoNum,struct list *lpublic);
int executeconfig(char * intfname,char * address,char * pre_distance,struct list * lcontrol,struct list * lpublic);





int cgiMain()
{
 ShowEditIntfPage();
 return 0;
}

int ShowEditIntfPage()
{
	struct list *lpublic;	/*解析public.txt文件的链表头*/
	struct list *lcontrol; 	/*解析help.txt文件的链表头*/
	lpublic=get_chain_head("../htdocs/text/public.txt");
	lcontrol=get_chain_head("../htdocs/text/control.txt"); 

	FILE *fp;
	char lan[3];
	char *encry=(char *)malloc(BUF_LEN);
	char *IntfName=(char *)malloc(21);
	memset(IntfName,0,21);
	char *IntfNameLater=(char *)malloc(21);
	memset(IntfNameLater,0,21);
	char *str;
	int i;
	char encry_editIntf[BUF_LEN];
	char * intfItem[Info_Num];
	char * mode=(char *)malloc(10);
	memset(mode,0,10);
	char * authString=(char *)malloc(50);
	memset(authString,0,50);
	char * sendversion=(char *)malloc(10);
	memset(sendversion,0,10);
	char * recversion=(char *)malloc(10);
	memset(recversion,0,10);
	char * horizon=(char *)malloc(10);
	memset(horizon,0,10);


	char * modeLater=(char *)malloc(10);
	memset(modeLater,0,10);
	char * authStringLater=(char *)malloc(50);
	memset(authStringLater,0,50);
	char * sendversionLater=(char *)malloc(10);
	memset(sendversionLater,0,10);
	char * recversionLater=(char *)malloc(10);
	memset(recversionLater,0,10);
	char * horizonLater=(char *)malloc(80);
	memset(horizonLater,0,80);
	char * networkLater=(char *)malloc(20);
	memset(networkLater,0,20);
	char * passive=(char *)malloc(10);
	memset(passive,0,10);
	char * distance=(char *)malloc(10);
	memset(distance,0,10);
	char * distance_LA=(char *)malloc(10);
	memset(distance_LA,0,10);

	char * networkLater_SEND=(char *)malloc(20);
	memset(networkLater_SEND,0,20);
	

    //fprintf(stderr,"networkLater=%s",networkLater);
    
	//fprintf(stderr,"modeLater=%s-authStringLater=%s-recversionLater=%s-sendversionLater=%s-horizonLater=%s-networkLater=%s",modeLater,authStringLater,recversionLater,sendversionLater,horizonLater,networkLater);
    
	for(i=0;i<Info_Num;i++)
	{
		intfItem[i]=(char *)malloc(60);
		memset(intfItem[i],0,60);
	}

	if(cgiFormSubmitClicked("submit_editIntf") != cgiFormSuccess)
	{
	  memset(encry,0,BUF_LEN);
	  memset(IntfName,0,21);
	  cgiFormStringNoNewlines("UN", encry, BUF_LEN);
	  cgiFormStringNoNewlines("INTFNAME", IntfName, 21);
	  cgiFormStringNoNewlines("MODE",modeLater,10);
      cgiFormStringNoNewlines("AUTHSTRING",authStringLater,50);
      cgiFormStringNoNewlines("REVVER",recversionLater,10);
      cgiFormStringNoNewlines("SENDVER",sendversionLater,10);
      cgiFormStringNoNewlines("HORIZON",horizonLater,80);
      cgiFormStringNoNewlines("NETWORK",networkLater,20);
      cgiFormStringNoNewlines("PASSIVE",passive,20);
      cgiFormStringNoNewlines("DISTANCE",distance,10);
	  fprintf(stderr,"IntfName=%s--modeLater=%s--authStringLater=%s--recversionLater=%s--SENDVER=%s--HORIZON=%s--NETWORK=%s--PASSIVE=%s--DISTANCE=%s\n",IntfName,modeLater,authStringLater,recversionLater,sendversionLater,horizonLater,networkLater,passive,distance);
	  str=dcryption(encry);
	  if(str==NULL)
	  {
		ShowErrorPage(search(lpublic,"ill_user")); 		 /*用户非法*/
		return 0;
	  }
	  memset(encry_editIntf,0,BUF_LEN);					 /*清空临时变量*/
	}
    cgiFormStringNoNewlines("editIntf_encry",encry_editIntf,BUF_LEN);
    cgiFormStringNoNewlines("IFNAME",IntfNameLater,21);
    cgiFormStringNoNewlines("ADDRESS",networkLater_SEND,20);
    
    //fprintf(stderr,"encry_editIntf=%s-IntfNameLater=%s",encry_editIntf,IntfNameLater);
   cgiHeaderContentType("text/html");
   fprintf(cgiOut,"<html xmlns=\"http://www.w3.org/1999/xhtml\"><head>");
   fprintf(cgiOut,"<meta http-equiv=Content-Type content=text/html; charset=gb2312>");
   fprintf(cgiOut,"<title>%s</title>",search(lcontrol,"route_manage"));
   fprintf(cgiOut,"<link rel=stylesheet href=/style.css type=text/css>"\
  	  	"<style type=text/css>"\
	  ".a3{width:30;border:0; text-align:center}"\
	  "</style>"\
  "</head>"\
   "<script src=/ip.js>"\
  "</script>"\
  "<script language=javascript>"\
  "function changestate(obj)"\
  "{"\
      "if(obj.value==\"NoAuth\" || obj==\"a\")"\
      "{"\
      		"document.formadd.simplepwd.disabled=true;"\
      		"document.formadd.MD5pwd.disabled=true;"\
      		"document.formadd.simplepwd.style.backgroundColor = \"#ccc\";"\
      		"document.formadd.MD5pwd.style.backgroundColor = \"#ccc\";"\
      "}"\
      "else if(obj.value==\"simple\" || obj==\"b\")"\
      "{"\
      		"document.formadd.simplepwd.disabled=false;"\
      		"document.formadd.MD5pwd.disabled=true;"\
      		"document.formadd.simplepwd.style.backgroundColor = \"#fff\";"\
      		"document.formadd.MD5pwd.style.backgroundColor = \"#ccc\";"\

      "}"\
      "else if(obj.value==\"MD5\" || obj==\"c\")"\
      "{"\
      		"document.formadd.simplepwd.disabled=true;"\
      		"document.formadd.MD5pwd.disabled=false;"\
      		"document.formadd.MD5pwd.style.backgroundColor = \"#fff\";"\
      		"document.formadd.simplepwd.style.backgroundColor = \"#ccc\";"\
      "}"\
  "}"\
  "</script>");
  if(strcmp(modeLater,"")==0)
  	fprintf(cgiOut,"<body onload=changestate(\"a\")>");
  else if(strcmp(modeLater,"text")==0)
  	fprintf(cgiOut,"<body onload=changestate(\"b\")>");
  else if(strcmp(modeLater,"md5")==0)
  	fprintf(cgiOut,"<body onload=changestate(\"c\")>");
  if(cgiFormSubmitClicked("submit_editIntf") == cgiFormSuccess)
  {
  	 cgiFormStringNoNewlines("DISTANCE",distance_LA,10);
  	
  	 //fprintf(stderr,"IntfNameLater=%s-networkLater=%s",IntfNameLater,networkLater);
     executeconfig(IntfNameLater,networkLater_SEND,distance_LA,lcontrol,lpublic);
     fprintf( cgiOut, "<script type='text/javascript'>\n" );
   	 fprintf( cgiOut, "window.location.href='wp_riplist.cgi?UN=%s';\n", encry_editIntf);
   	 fprintf( cgiOut, "</script>\n" );
  }
 
  
  fprintf(cgiOut,"<form method=post name=formadd>");
  fprintf(cgiOut,"<div align=center>"\
  "<table width=976 border=0 cellpadding=0 cellspacing=0>"\
  "<tr>"\
    "<td width=8 align=left valign=top background=/images/di22.jpg><img src=/images/youce4.jpg width=8 height=30/></td>"\
    "<td width=51 align=left valign=bottom background=/images/di22.jpg><img src=/images/youce33.jpg width=37 height=24/></td>"\
    "<td width=153 align=left valign=bottom id=%s background=/images/di22.jpg>%s</td>",search(lpublic,"title_style"),search(lcontrol,"route_manage"));
    fprintf(cgiOut,"<td width=690 align=right valign=bottom background=/images/di22.jpg>");
	    if((fp=fopen("../htdocs/text/public.txt","r"))==NULL)		 /*以只读方式打开资源文件*/
		{
			ShowAlert(search(lpublic,"error_open"));
	    }
		else
		{
			fseek(fp,4,0);						/*将文件指针移到离文件首4个字节处，即lan=之后*/
			fgets(lan,3,fp);	   
			fclose(fp);
		}
	
    	  fprintf(cgiOut,"<table width=130 border=0 cellspacing=0 cellpadding=0>"\
          "<tr>"\
          "<td width=62 align=center><input id=but type=submit name=submit_editIntf style=background-image:url(/images/%s) value=""></td>",search(lpublic,"img_ok")); 
		  if(cgiFormSubmitClicked("submit_editIntf") != cgiFormSuccess)
            fprintf(cgiOut,"<td width=62 align=left><a href=wp_riplist.cgi?UN=%s target=mainFrame><img src=/images/%s border=0 width=62 height=20/></a></td>",encry,search(lpublic,"img_cancel"));
		  else                                         
     		fprintf(cgiOut,"<td width=62 align=left><a href=wp_riplist.cgi?UN=%s target=mainFrame><img src=/images/%s border=0 width=62 height=20/></a></td>",encry_editIntf,search(lpublic,"img_cancel"));
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
                		if(cgiFormSubmitClicked("submit_editIntf") != cgiFormSuccess)
                		{
						  	fprintf(cgiOut,"<tr height=26>"\
                			"<td align=left id=tdleft background=/images/bottom_bg.gif style=\"border-right:0\"><font id=%s>%s</font></td>",search(lpublic,"menu_san"),search(lcontrol,"edit_rip_intf"));   /*突出显示*/
                		  	fprintf(cgiOut,"</tr>");
                		}
                		else if(cgiFormSubmitClicked("submit_editIntf") == cgiFormSuccess)					
                		{
						  	fprintf(cgiOut,"<tr height=26>"\
                			"<td align=left id=tdleft background=/images/bottom_bg.gif style=\"border-right:0\"><font id=%s>%s</font></td>",search(lpublic,"menu_san"),search(lcontrol,"edit_rip_intf"));   /*突出显示*/
                		  	fprintf(cgiOut,"</tr>");

                		}
					  for(i=0;i<17;i++)
					  {
						fprintf(cgiOut,"<tr height=25>"\
						  "<td id=tdleft>&nbsp;</td>"\
						"</tr>");
					  }

				  fprintf(cgiOut,"</table>"\
              "</td>"\
              "<td align=left valign=top style=\"background-color:#ffffff; border-right:1px solid #707070; padding-left:30px; padding-top:10px\">"\
					  "<table width=640 height=340 border=0 cellspacing=0 cellpadding=0>");
					  	fprintf(cgiOut,"<tr>"\
							"<td id=sec1 style=\"border-bottom:2px solid #53868b;font-size:14px\">%s</td>",search(lcontrol,"interface_info"));
							fprintf(cgiOut,"</tr>"\
						"<tr>"\
						  "<td align=left valign=top style=padding-top:10px>"\
						  "<table width=600 border=0 cellspacing=0 cellpadding=0>");
				  			//int routeUpdate=30;
							fprintf(cgiOut,"<tr align=left height=35>"\
							"<td width=10 style=color:red>*</td>");
							fprintf(cgiOut,"<td align=left width=70>%s: </td>",search(lcontrol,"interface"));
							fprintf(cgiOut,"<td align=left width=100>%s</td>",IntfName);
							fprintf(cgiOut,"<td align=left style=padding-left:60px width=40>%s: </td>",search(lcontrol,"network"));
							fprintf(cgiOut,"<td style=padding-left:5px width=380>%s</td>",networkLater);
							//fprintf(stderr,"77IntfName=%s77",IntfName);
							fprintf(cgiOut,"</tr>"\
							
							"</table>"\
						  "</td>"\
						"</tr>"\
						"<tr>"\
							"<td id=sec1 style=\"border-bottom:2px solid #53868b;font-size:14px\">%s</td>",search(lcontrol,"General_set"));
						fprintf(cgiOut,"</tr>"\
						"<tr>"\
						"<td>"\
						"<table width=500 border=0 cellspacing=0 cellpadding=0>"\
						"<tr align=left height=35>");
						fprintf(cgiOut,"<td width=10 style=color:red>*</td>");
						fprintf(cgiOut,"<td align=left width=140>%s: </td>",search(lcontrol,"send_version"));
							fprintf(stderr,"\nsendversionLater=%s--recversionLater=%s",sendversionLater,recversionLater);
							if(strcmp(sendversionLater,"")==0 || strcmp(sendversionLater,"1,2")==0)
							{
    							fprintf(cgiOut, "<td width=70 align=left><input type=radio name=send_select value=1>%s1</td>",search(lcontrol,"version"));
								fprintf(cgiOut, "<td width=70 align=left><input type=radio name=send_select  value=2>%s2</td>",search(lcontrol,"version"));
								fprintf(cgiOut, "<td width=150 align=left><input type=radio name=send_select checked value=3>%s1 %s %s2</td>",search(lcontrol,"version"),search(lcontrol,"and"),search(lcontrol,"version"));
							}
							if(strcmp(sendversionLater,"1")==0)
							{
    							fprintf(cgiOut, "<td width=70 align=left><input type=radio name=send_select checked value=1>%s1</td>",search(lcontrol,"version"));
								fprintf(cgiOut, "<td width=70 align=left><input type=radio name=send_select value=2>%s2</td>",search(lcontrol,"version"));
								fprintf(cgiOut, "<td width=150 align=left><input type=radio name=send_select value=3>%s1 %s %s2</td>",search(lcontrol,"version"),search(lcontrol,"and"),search(lcontrol,"version"));
							}
							else if(strcmp(sendversionLater,"2")==0)
							{
    							fprintf(cgiOut, "<td width=70 align=left><input type=radio name=send_select value=1>%s1</td>",search(lcontrol,"version"));
								fprintf(cgiOut, "<td width=70 align=left><input type=radio name=send_select checked value=2>%s2</td>",search(lcontrol,"version"));
								fprintf(cgiOut, "<td width=150 align=left><input type=radio name=send_select value=3>%s1 %s %s2</td>",search(lcontrol,"version"),search(lcontrol,"and"),search(lcontrol,"version"));
							}
							fprintf(cgiOut,"<td width=60>&nbsp;</td>");
							fprintf(cgiOut,"</tr>");

						fprintf(cgiOut,"<tr align=left height=35>");
							fprintf(cgiOut,"<td width=10 style=color:red>*</td>");
							fprintf(cgiOut,"<td align=left width=140>%s: </td>",search(lcontrol,"Rev_version"));
							if(strcmp(recversionLater,"")==0 || strcmp(recversionLater,"1,2")==0 )
							{
     							fprintf(cgiOut, "<td width=70 align=left><input type=radio name=receive_select value=1>%s1</td>",search(lcontrol,"version"));
								fprintf(cgiOut, "<td width=70 align=left><input type=radio name=receive_select value=2>%s2</td>",search(lcontrol,"version"));
								fprintf(cgiOut, "<td width=150 align=left><input type=radio name=receive_select checked value=3>%s1 %s %s2</td>",search(lcontrol,"version"),search(lcontrol,"and"),search(lcontrol,"version"));
							}
							else if(strcmp(recversionLater,"1")==0)
							{
     							fprintf(cgiOut, "<td width=70 align=left><input type=radio name=receive_select checked value=1>%s1</td>",search(lcontrol,"version"));
								fprintf(cgiOut, "<td width=70 align=left><input type=radio name=receive_select value=2>%s2</td>",search(lcontrol,"version"));
								fprintf(cgiOut, "<td width=150 align=left><input type=radio name=receive_select value=3>%s1 %s %s2</td>",search(lcontrol,"version"),search(lcontrol,"and"),search(lcontrol,"version"));
							}
							else if(strcmp(recversionLater,"2")==0)
							{
     							fprintf(cgiOut, "<td width=70 align=left><input type=radio name=receive_select value=1>%s1</td>",search(lcontrol,"version"));
								fprintf(cgiOut, "<td width=70 align=left><input type=radio name=receive_select checked value=2>%s2</td>",search(lcontrol,"version"));
								fprintf(cgiOut, "<td width=150 align=left><input type=radio name=receive_select value=3>%s1 %s %s2</td>",search(lcontrol,"version"),search(lcontrol,"and"),search(lcontrol,"version"));
							}
							fprintf(cgiOut,"<td width=60>&nbsp;</td>");
							fprintf(cgiOut,"</tr>");
							 
						fprintf(cgiOut,"<tr align=left height=35>"\
							"<td width=10 style=color:red>*</td>");
							fprintf(cgiOut,"<td align=left width=100>%s: </td>","Horizon");
							fprintf(cgiOut,"<td align=left width=120 colspan=4>");
							fprintf(cgiOut, "<select name=Rip_loop>");
							if(strcmp(horizonLater,"")==0)
							{
								fprintf(cgiOut, "<option value=open_Horizon>%s",search(lcontrol,"open_Horizon"));
								fprintf(cgiOut, "<option value=horizon_poison>%s",search(lcontrol,"open_Horizon_poison_reverse"));
								fprintf(cgiOut, "<option value=None>None");
							}
							else if(strcmp(horizonLater,"split-horizon-poisoned-reverse")==0)
							{
								fprintf(cgiOut, "<option value=horizon_poison>%s",search(lcontrol,"open_Horizon_poison_reverse"));
								fprintf(cgiOut, "<option value=open_Horizon>%s",search(lcontrol,"open_Horizon"));
								fprintf(cgiOut, "<option value=None>None");
							}
							else if(strcmp(horizonLater,"None")==0)
							{
                              	fprintf(cgiOut, "<option value=None>None");
                              	fprintf(cgiOut, "<option value=horizon_poison>%s",search(lcontrol,"open_Horizon_poison_reverse"));
     							fprintf(cgiOut, "<option value=open_Horizon>%s",search(lcontrol,"open_Horizon"));
							}
							else
							{
								fprintf(cgiOut, "<option value=open_Horizon>%s",search(lcontrol,"open_Horizon"));
								fprintf(cgiOut, "<option value=horizon_poison>%s",search(lcontrol,"open_Horizon_poison_reverse"));
								fprintf(cgiOut, "<option value=None>None");
							}
                         	fprintf(cgiOut, "</select>");
							fprintf(cgiOut,"</td>");
							//fprintf(cgiOut,"<td colspan=2 width=270>&nbsp;</td>");
							fprintf(cgiOut,"</tr>"\
						"<tr align=left height=35>");
						fprintf(stderr,"passive=%s",passive);
						if(strcmp(passive,"ON")==0)
							fprintf(cgiOut, "<td colspan=2><input type=checkbox name=passive value=passive checked>%s</td>",search(lcontrol,"passive_mode"));
						else
							fprintf(cgiOut, "<td colspan=2><input type=checkbox name=passive value=passive>%s</td>",search(lcontrol,"passive_mode"));
						fprintf(cgiOut, "<td align=right>%s:</td>",search(lcontrol,"distance"));
						if(strcmp(distance,"")==0)
							fprintf(cgiOut, "<td colspan=2><input type=text name=intf_distance size=8 value=></td>");
						else if(strcmp(distance,"")!=0)
							fprintf(cgiOut, "<td colspan=2><input type=text name=intf_distance size=8 value=%s></td>",distance);
						fprintf(cgiOut,"</tr>"\
						"</table>"\
						"</td>"\
						"</tr>"\
						"<tr height=35>"\
							"<td id=sec1 style=\"border-bottom:2px solid #53868b;font-size:14px\">%s</td>",search(lcontrol,"Authentication"));
							fprintf(cgiOut,"</tr>"\
						"<tr>"\
						"<td>"\
						"<table>"\
						"<tr align=left height=35>");
						if(strcmp(modeLater,"")==0)
						{
     						fprintf(cgiOut,"<td><input type=radio name=radiobutton value=NoAuth onclick=\"changestate(this);\" checked></td>"\
     						"<td>None</td>"\
     						"</tr>"\
     						"<tr height=35 align=left>");
     						fprintf(cgiOut,"<td><input type=radio name=radiobutton value=simple onclick=\"changestate(this);\"></td>");
     						fprintf(cgiOut,"<td>%s</td>",search(lcontrol,"simple_passwd"));
     						fprintf(cgiOut,"<td><input type=text name=simplepwd></td>"\
     						"</tr>"\
     						
     						"<tr align=left height=35>");
     						fprintf(cgiOut,"<td><input type=radio name=radiobutton value=MD5 onclick=\"changestate(this);\"></td>");
     						fprintf(cgiOut,"<td>%s</td>",search(lcontrol,"MD5_passwd"));
     						fprintf(cgiOut,"<td><input type=text name=MD5pwd></td>");
     					}
     					else if(strcmp(modeLater,"text")==0)
						{
     						fprintf(cgiOut,"<td><input type=radio name=radiobutton value=NoAuth onclick=\"changestate(this);\"></td>"\
     						"<td>None</td>"\
     						"</tr>"\
     						"<tr height=35 align=left>");
     						fprintf(cgiOut,"<td><input type=radio name=radiobutton value=simple onclick=\"changestate(this);\" checked></td>");
     						fprintf(cgiOut,"<td>%s</td>",search(lcontrol,"simple_passwd"));
     						fprintf(cgiOut,"<td><input type=text name=simplepwd value=%s></td>",authStringLater);
     						fprintf(cgiOut,"</tr>"\
     						
     						"<tr align=left height=35>");
     						fprintf(cgiOut,"<td><input type=radio name=radiobutton value=MD5 onclick=\"changestate(this);\"></td>");
     						fprintf(cgiOut,"<td>%s</td>",search(lcontrol,"MD5_passwd"));
     						fprintf(cgiOut,"<td><input type=text name=MD5pwd></td>");
     					}
     					else if(strcmp(modeLater,"md5")==0)
						{
     						fprintf(cgiOut,"<td><input type=radio name=radiobutton value=NoAuth onclick=\"changestate(this);\"></td>"\
     						"<td>None</td>"\
     						"</tr>"\
     						"<tr height=35 align=left>");
     						fprintf(cgiOut,"<td><input type=radio name=radiobutton value=simple onclick=\"changestate(this);\"></td>");
     						fprintf(cgiOut,"<td>%s</td>",search(lcontrol,"simple_passwd"));
     						fprintf(cgiOut,"<td><input type=text name=simplepwd></td>"\
     						"</tr>"\
     						
     						"<tr align=left height=35>");
     						fprintf(cgiOut,"<td><input type=radio name=radiobutton value=MD5 onclick=\"changestate(this);\" checked></td>");
     						fprintf(cgiOut,"<td>%s</td>",search(lcontrol,"MD5_passwd"));
     						fprintf(cgiOut,"<td><input type=text name=MD5pwd value=%s></td>",authStringLater);
     					}
     					fprintf(cgiOut,"</tr>"\
						"</table>"\
						"</td>"\
						"</tr>");
						/*fprintf(cgiOut,"<tr>");
						fprintf(cgiOut,"<td><input type=hidden name=UN value=%s></td>",encry);
						fprintf(cgiOut,"</tr>");*/
						if(cgiFormSubmitClicked("submit_routelist") != cgiFormSuccess)
						 {
						   fprintf(cgiOut,"<td><input type=hidden name=editIntf_encry value=%s></td>",encry);
						   fprintf(cgiOut,"<td><input type=hidden name=IFNAME value=%s></td>",IntfName);
						   fprintf(cgiOut,"<td><input type=hidden name=ADDRESS value=%s></td>",networkLater);
						   fprintf(cgiOut,"<td><input type=hidden name=DISTANCE value=%s></td>",distance);
						 }
						 else if(cgiFormSubmitClicked("submit_routelist") == cgiFormSuccess)
						 {
						   fprintf(cgiOut,"<td><input type=hidden name=editIntf_encry value=%s></td>",encry_editIntf);
						   fprintf(cgiOut,"<td><input type=hidden name=IFNAME value=%s></td>",IntfNameLater);
						   fprintf(cgiOut,"<td><input type=hidden name=ADDRESS value=%s></td>",networkLater_SEND);
						   fprintf(cgiOut,"<td><input type=hidden name=DISTANCE value=%s></td>",distance);
						 }
								  fprintf(cgiOut,"</table>"\

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
for(i=0;i<Info_Num;i++)
{
  free(intfItem[i]);   
}

free(distance_LA);
free(IntfNameLater);
free(horizon);
free(recversion);
free(sendversion);
free(authString);
free(mode);
free(encry);
free(horizonLater);
free(recversionLater);
free(sendversionLater);
free(authStringLater);
free(modeLater);
free(networkLater);
free(networkLater_SEND);
free(passive);
free(distance);
free(IntfName);
release(lpublic);  
release(lcontrol);

return 0;

}

int  interfaceInfo(char *iname,char * intfname[],int * infoNum,struct list * lpublic)
{
	FILE * ft;
	char * syscommand=(char *)malloc(300);
	memset(syscommand,0,300);
	int i;
	
	sprintf(syscommand,"show_run_conf.sh | awk 'BEGIN{FS=\"\\n\";RS=\"!\";ORS=\"#\\n\";OFS=\"|\"}/interface/{$1=$1;print}' | awk '{RS=\"#\";FS=\"|\";ORS=\"\\n\";OFS=\"#\"}/rip/{$1=$1;print}' | awk 'BEGIN{FS=\"#\";OFS=\"#\\n\"}/%s/{$1=$1;gsub(\" \",\"#\",$0);print}' >/var/run/apache2/RIPIntf.txt",iname);
	int status = system(syscommand);
	int ret = WEXITSTATUS(status);			 
	if(0==ret)
		{}
	else ShowAlert(search(lpublic,"bash_fail"));
	if((ft=fopen("/var/run/apache2/RIPIntf.txt","r"))==NULL)
		{
			ShowAlert(search(lpublic,"error_open"));
			return 0;
		}
	char  temp[60];
	memset(temp,0,60);
	i=0;
	while((fgets(temp,60,ft)) != NULL)
		{
			strcat(intfname[i],temp);
			i++;
			memset(temp,0,60);
		}
	fclose(ft);
	*infoNum=i;
	free(syscommand);
	return 1;
}

int executeconfig(char * intfname,char * address,char * pre_distance,struct list * lcontrol,struct list * lpublic)
{
	char * intfNamesys=(char * )malloc(250);
	memset(intfNamesys,0,250);
	char * networktemp=(char *)malloc(30);
	memset(networktemp,0,30);
	char * Intfname=(char *)malloc(20);
	memset(Intfname,0,20);
	char * Rip_loop=(char *)malloc(20);
	memset(Rip_loop,0,20);
	char * intf_distance=(char *)malloc(20);
	memset(intf_distance,0,20);
	char * radiobutton=(char *)malloc(10);
	memset(radiobutton,0,10);
	char * simplepwd=(char *)malloc(30);
	memset(simplepwd,0,30);
	char * MD5pwd=(char *)malloc(30);
	memset(MD5pwd,0,30);
	

	cgiFormString("simplepwd", simplepwd, 30);
	cgiFormString("MD5pwd", MD5pwd, 30);
	
	if((strlen(simplepwd) >= 16) || (strlen(MD5pwd) >= 16))
    {
    	ShowAlert(search(lcontrol,"text_key_can_not_too_long"));
    	return -1;
    }
	
	//cgiFormStringNoNewlines("Rip_intf",Intfname,20);
	strcpy(Intfname,intfname);
	strcpy(networktemp,address);
	//fprintf(stderr,"Intfname=%s-networktemp=%s",Intfname,networktemp);
/////////////////////////////////////////////////////////////////////////////////////
	memset(intfNamesys,0,250);	
	int result;
 	//char **responses1;
 	//result = cgiFormStringMultiple("send_select", &responses1);
	char sendversion[10];
	cgiFormString("send_select", sendversion, 10);
	//fprintf(stderr,"sendversion=%s",sendversion);

	strcat(intfNamesys,"rip_send_version.sh");
	strcat(intfNamesys," ");
	strcat(intfNamesys,"on");
	strcat(intfNamesys," ");
	strcat(intfNamesys,Intfname);
	strcat(intfNamesys," ");
	strcat(intfNamesys,sendversion);
	strcat(intfNamesys," ");
	strcat(intfNamesys,">/dev/null");
	system(intfNamesys);
//////////////////////////////////////////////////////////////////////////////////////////

	memset(intfNamesys,0,250);	
	result=0;
	char revversion[10];
 	//char **responses2;
 	//result = cgiFormStringMultiple("receive_select", &responses2);
	cgiFormString("receive_select", revversion, 10);
	fprintf(stderr,"revversion=%s",revversion);

	strcat(intfNamesys,"rip_receive_version.sh");
	strcat(intfNamesys," ");
	strcat(intfNamesys,"on");
	strcat(intfNamesys," ");
	strcat(intfNamesys,Intfname);
	strcat(intfNamesys," ");
	strcat(intfNamesys,revversion);
	strcat(intfNamesys," ");
	strcat(intfNamesys,">/dev/null");
	system(intfNamesys);
		
//////////////////////////////////////////////////////////////////////////////////////////////
	//fprintf(stderr,"11111111");
	memset(intfNamesys,0,250);
	cgiFormStringNoNewlines("Rip_loop",Rip_loop,20);
	if(strcmp(Rip_loop,"horizon_poison")==0)
	{
		strcat(intfNamesys,"rip_split.sh");
		strcat(intfNamesys," ");
		strcat(intfNamesys,"on");
		strcat(intfNamesys," ");
		strcat(intfNamesys,Intfname);
		strcat(intfNamesys," ");
		strcat(intfNamesys,"poisoned");
		strcat(intfNamesys," ");
		strcat(intfNamesys,">/dev/null");
		system(intfNamesys);
		//fprintf(stderr,"splitintfNamesys=%s",intfNamesys);
	}
	else if(strcmp(Rip_loop,"open_Horizon")==0)
	{
		strcat(intfNamesys,"rip_split.sh");
		strcat(intfNamesys," ");
		strcat(intfNamesys,"on");
		strcat(intfNamesys," ");
		strcat(intfNamesys,Intfname);
		strcat(intfNamesys," ");
		strcat(intfNamesys,"normal");
		strcat(intfNamesys," ");
		strcat(intfNamesys,">/dev/null");
		system(intfNamesys);
		//fprintf(stderr,"splitintfNamesys=%s",intfNamesys);
	}
	else if(strcmp(Rip_loop,"None")==0)
	{
		strcat(intfNamesys,"rip_split.sh");
		strcat(intfNamesys," ");
		strcat(intfNamesys,"off");
		strcat(intfNamesys," ");
		strcat(intfNamesys,Intfname);
		strcat(intfNamesys," ");
		strcat(intfNamesys,"normal");
		strcat(intfNamesys," ");
		strcat(intfNamesys,">/dev/null");
		system(intfNamesys);
		//fprintf(stderr,"splitintfNamesys=%s",intfNamesys);

		strcat(intfNamesys,"rip_split.sh");
		strcat(intfNamesys," ");
		strcat(intfNamesys,"off");
		strcat(intfNamesys," ");
		strcat(intfNamesys,Intfname);
		strcat(intfNamesys," ");
		strcat(intfNamesys,"poisoned");
		strcat(intfNamesys," ");
		strcat(intfNamesys,">/dev/null");
		system(intfNamesys);
		//fprintf(stderr,"splitintfNamesys=%s",intfNamesys);
	}
///////////////////////////////////////////////////////////////////////////////////////////////

	memset(intfNamesys,0,250);
	result=0;
	char **responses3;
	result = cgiFormStringMultiple("passive", &responses3);
	if(responses3[0])
	{
			
		strcat(intfNamesys,"rip_passive_int.sh");
		strcat(intfNamesys," ");
		strcat(intfNamesys,"on");
		strcat(intfNamesys," ");
		strcat(intfNamesys,Intfname);
		strcat(intfNamesys," ");
		strcat(intfNamesys,">/dev/null");
		system(intfNamesys);
		//fprintf(stderr,"passintfNamesys=%s",intfNamesys);
	}
	else
	{
		
		strcat(intfNamesys,"rip_passive_int.sh");
		strcat(intfNamesys," ");
		strcat(intfNamesys,"off");
		strcat(intfNamesys," ");
		strcat(intfNamesys,Intfname);
		strcat(intfNamesys," ");
		strcat(intfNamesys,">/dev/null");
		system(intfNamesys);
		//fprintf(stderr,"@passoffintfNamesys=%s@",intfNamesys);
	}
	memset(intfNamesys,0,250);
	cgiFormStringNoNewlines("intf_distance",intf_distance,20);
	fprintf(stderr,"intf_distance=%s",intf_distance);
	if(strcmp(intf_distance,"")!=0)
	{
		strcat(intfNamesys,"distance.sh");
		strcat(intfNamesys," ");
		strcat(intfNamesys,"on");
		strcat(intfNamesys," ");
		strcat(intfNamesys,intf_distance);
		strcat(intfNamesys," ");
		strcat(intfNamesys,networktemp);
		strcat(intfNamesys," ");
		strcat(intfNamesys,">/dev/null");
		system(intfNamesys);
		//fprintf(stderr,"distanceintfNamesys=%s",intfNamesys);
	}
	else
	{
		sprintf(intfNamesys,"distance.sh off %s %s >/dev/null",pre_distance,networktemp);
		system(intfNamesys);
		fprintf(stderr,"intfNamesys=%s",intfNamesys);
	}
////////////////////////////////////////////////////////////////////////////////auth


	cgiFormString("radiobutton", radiobutton, 10);
	memset(intfNamesys,0,250);
	if(strcmp(radiobutton,"NoAuth")==0)
	{
		strcat(intfNamesys,"rip_auth_mode.sh");
		strcat(intfNamesys," ");
		strcat(intfNamesys,"off");
		strcat(intfNamesys," ");
		strcat(intfNamesys,Intfname);
		strcat(intfNamesys," ");
		strcat(intfNamesys,"text");
		strcat(intfNamesys," ");
		strcat(intfNamesys,">/dev/null");
		system(intfNamesys);
		//fprintf(stderr,"modeintfNamesys=%s",intfNamesys);

		strcat(intfNamesys,"rip_auth_mode.sh");
		strcat(intfNamesys," ");
		strcat(intfNamesys,"off");
		strcat(intfNamesys," ");
		strcat(intfNamesys,Intfname);
		strcat(intfNamesys," ");
		strcat(intfNamesys,"md5");
		strcat(intfNamesys," ");
		strcat(intfNamesys,">/dev/null");
		system(intfNamesys);
		//fprintf(stderr,"modeintfNamesys=%s",intfNamesys);
		
		//cgiFormString("simplepwd", simplepwd, 20);
		memset(intfNamesys,0,250);
		strcat(intfNamesys,"rip_auth_string.sh");
		strcat(intfNamesys," ");
		strcat(intfNamesys,"off");
		strcat(intfNamesys," ");
		strcat(intfNamesys,Intfname);
		strcat(intfNamesys," ");
		strcat(intfNamesys,simplepwd);
		strcat(intfNamesys," ");
		strcat(intfNamesys,">/dev/null");
		//fprintf(stderr,"stringintfNamesys=%s",intfNamesys);
		system(intfNamesys);

		//cgiFormString("MD5pwd", simplepwd, 20);
		memset(intfNamesys,0,250);
		strcat(intfNamesys,"rip_auth_string.sh");
		strcat(intfNamesys," ");
		strcat(intfNamesys,"off");
		strcat(intfNamesys," ");
		strcat(intfNamesys,Intfname);
		strcat(intfNamesys," ");
		strcat(intfNamesys,MD5pwd);
		strcat(intfNamesys," ");
		strcat(intfNamesys,">/dev/null");
		//fprintf(stderr,"stringintfNamesys=%s",intfNamesys);
		system(intfNamesys);
		
	}
	else if(strcmp(radiobutton,"simple")==0)
	{
		strcat(intfNamesys,"rip_auth_mode.sh");
		strcat(intfNamesys," ");
		strcat(intfNamesys,"on");
		strcat(intfNamesys," ");
		strcat(intfNamesys,Intfname);
		strcat(intfNamesys," ");
		strcat(intfNamesys,"text");
		strcat(intfNamesys," ");
		strcat(intfNamesys,">/dev/null");
		system(intfNamesys);
		//fprintf(stderr,"modeintfNamesys=%s",intfNamesys);
		
		//cgiFormString("simplepwd", simplepwd, 20);
		memset(intfNamesys,0,250);
		strcat(intfNamesys,"rip_auth_string.sh");
		strcat(intfNamesys," ");
		strcat(intfNamesys,"on");
		strcat(intfNamesys," ");
		strcat(intfNamesys,Intfname);
		strcat(intfNamesys," ");
		strcat(intfNamesys,simplepwd);
		strcat(intfNamesys," ");
		strcat(intfNamesys,">/dev/null");
		//fprintf(stderr,"stringintfNamesys=%s",intfNamesys);
		system(intfNamesys);
	}
	else if(strcmp(radiobutton,"MD5")==0)
	{
		strcat(intfNamesys,"rip_auth_mode.sh");
		strcat(intfNamesys," ");
		strcat(intfNamesys,"on");
		strcat(intfNamesys," ");
		strcat(intfNamesys,Intfname);
		strcat(intfNamesys," ");
		strcat(intfNamesys,"md5");
		strcat(intfNamesys," ");
		strcat(intfNamesys,">/dev/null");
		system(intfNamesys);
		//fprintf(stderr,"modeintfNamesys=%s",intfNamesys);
		
		//cgiFormString("MD5pwd", simplepwd, 20);
		memset(intfNamesys,0,250);
		strcat(intfNamesys,"rip_auth_string.sh");
		strcat(intfNamesys," ");
		strcat(intfNamesys,"on");
		strcat(intfNamesys," ");
		strcat(intfNamesys,Intfname);
		strcat(intfNamesys," ");
		strcat(intfNamesys,MD5pwd);
		strcat(intfNamesys," ");
		strcat(intfNamesys,">/dev/null");
		system(intfNamesys);
		//fprintf(stderr,"stringintfNamesys=%s",intfNamesys);
	}
	ShowAlert(search(lcontrol,"rip_config_suc"));


	free(networktemp);
	free(Intfname);
	free(Rip_loop);
	free(simplepwd);
	free(MD5pwd);
	free(radiobutton);
	free(intf_distance);
	free(intfNamesys);
	return 0;
}


