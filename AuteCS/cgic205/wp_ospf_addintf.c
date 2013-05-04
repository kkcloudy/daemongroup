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
* wp_ospf_addintf.c
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
#include "ws_init_dbus.h"


#include <sys/wait.h>
#define Info_Num 500
#define Network_Num 8
#define INTF_NAME_LENTH 20



int ShowAddIntfPage();
int ReadConfig_ospf(char * ripInfo[],int * infoNum,struct list * lpublic);
int ReadConfig_INTF(char * ospfInfo[],int * infoNum,struct list * lpublic);
int interfaceInfo(char * IntfInfo[],int * Num,struct list * lpublic);
int executeconfig(char * IntfInfo[],int num,struct list *lcontrol,struct list * lpublic);
char * nameToIP(char * intfname,struct list * lpublic);
int show_intf_network(char * intfname,char * revIntfNet[],int * Num ,struct list *lpublic);


int cgiMain()
{
 ShowAddIntfPage();
 return 0;
}

int ShowAddIntfPage()
{ 
	struct list *lpublic;	/*解析public.txt文件的链表头*/
	struct list *lcontrol; 	/*解析help.txt文件的链表头*/
	lpublic=get_chain_head("../htdocs/text/public.txt");
	lcontrol=get_chain_head("../htdocs/text/control.txt"); 

	FILE *fp;
	char lan[3];
	char *encry=(char *)malloc(BUF_LEN);			  
	char *str;
	int i;
	char encry_addintf[BUF_LEN];
	char * intfName[Info_Num];
	char * Network[Network_Num];
	
	char * select_opt = (char *)malloc(INTF_NAME_LENTH);
	memset( select_opt, 0, INTF_NAME_LENTH );
	int IntfNum=0;
	for( i = 0; i< Network_Num ; i++ )
		{
			Network[i]=(char *)malloc(30);
			memset(Network[i],0,30);
		}
	for(i=0;i<Info_Num;i++)
		{
			intfName[i]=(char *)malloc(20);
			memset(intfName[i],0,20);
		}
	memset(encry,0,BUF_LEN);
	if(cgiFormStringNoNewlines("UN", encry, BUF_LEN) != cgiFormNotFound)
	{
	 	  
	  str=dcryption(encry);
	  if(str==NULL)
	  {
		ShowErrorPage(search(lpublic,"ill_user")); 		 /*用户非法*/
		return 0;
	  }
	  memset(encry_addintf,0,BUF_LEN);					 /*清空临时变量*/
	}
	else
	{
		cgiFormStringNoNewlines("encry_addintf",encry,BUF_LEN);
	}
  cgiFormStringNoNewlines("OSPF_intf",select_opt,INTF_NAME_LENTH);
  fprintf(stderr,"select_opt=%s",select_opt);
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

      "if(obj.value==\"NoAuth\" || obj==\"abc\")"\
      "{"\
      		"document.formadd.simplepwd.disabled=true;"\
      		"document.formadd.MD5pwd.disabled=true;"\
      		"document.formadd.MD5key_id.disabled=true;"\
      		"document.formadd.simplepwd.style.backgroundColor = \"#ccc\";"\
      		"document.formadd.MD5pwd.style.backgroundColor = \"#ccc\";"\
  			"document.formadd.MD5key_id.style.backgroundColor = \"#ccc\";"\
      "}"\
      "else if(obj.value==\"text\")"\
      "{"\
      		"document.formadd.simplepwd.disabled=false;"\
      		"document.formadd.MD5pwd.disabled=true;"\
      		"document.formadd.MD5key_id.disabled=true;"\
      		"document.formadd.simplepwd.style.backgroundColor = \"#fff\";"\
      		"document.formadd.MD5pwd.style.backgroundColor = \"#ccc\";"\
      		"document.formadd.MD5key_id.style.backgroundColor = \"#ccc\";"\

      "}"\
      "else if(obj.value==\"MD5\")"\
      "{"\
      		"document.formadd.simplepwd.disabled=true;"\
      		"document.formadd.MD5pwd.disabled=false;"\
      		"document.formadd.MD5key_id.disabled=false;"\
      		"document.formadd.MD5pwd.style.backgroundColor = \"#fff\";"\
      		"document.formadd.MD5key_id.style.backgroundColor = \"#fff\";"\
      		"document.formadd.simplepwd.style.backgroundColor = \"#ccc\";"\
      "}"\
      "if(document.formadd.IS_ABR.checked==true)"\
      "{"\

  			"document.formadd.advertise_param.disabled=false;"\
  			"document.formadd.no_summary.disabled=false;"\
  			"document.formadd.route_range.disabled=false;"\
  			"document.formadd.src_ip1.disabled=false;"\
  			"document.formadd.src_ip2.disabled=false;"\
  			"document.formadd.src_ip3.disabled=false;"\
  			"document.formadd.src_ip4.disabled=false;"\
  			"document.formadd.masklen.disabled=false;"\
  			"document.formadd.masklen.style.backgroundColor = \"#fff\";"\

  			"if(document.formadd.area_type.value==\"nssa\")"\
  			"{"\
	        	"document.formadd.nssa_param.disabled=false;"\
	        "}"\
	        "else if(document.formadd.area_type.value==\"stub\")"\
	        "{"\
	        	"document.formadd.nssa_param.disabled=true;"\
	        "}"\
      		
      "}"\
      "else if(document.formadd.IS_ABR.checked==false)"\
      "{"\

  			"document.formadd.advertise_param.disabled=true;"\
  			"document.formadd.no_summary.disabled=true;"\
			"document.formadd.route_range.disabled=true;"\
  
  			"document.formadd.src_ip1.disabled=true;"\
  			"document.formadd.src_ip2.disabled=true;"\
  			"document.formadd.src_ip3.disabled=true;"\
  			"document.formadd.src_ip4.disabled=true;"\
  			"document.formadd.masklen.disabled=true;"\
  			"document.formadd.masklen.style.backgroundColor = \"#ccc\";"\

  			"document.formadd.nssa_param.disabled=true;"\
      "}"\
  "}"\
  "</script>"\
  "<body onload=changestate(\"abc\")>");
  ccgi_dbus_init();
  if(cgiFormSubmitClicked("submit_addintf") == cgiFormSuccess)
  {
  		interfaceInfo(intfName,&IntfNum,lpublic);
    	executeconfig(intfName,IntfNum,lcontrol,lpublic);
  }

  fprintf(cgiOut,"<form method=post name=formadd>"\
  "<div align=center>"\
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
          "<td width=62 align=center><input id=but type=submit name=submit_addintf style=background-image:url(/images/%s) value=""></td>",search(lpublic,"img_ok"));
          fprintf(cgiOut,"<td width=62 align=left><a href=wp_srouter.cgi?UN=%s target=mainFrame><img src=/images/%s border=0 width=62 height=20/></a></td>",encry,search(lpublic,"img_cancel"));
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

        			fprintf(cgiOut,"<tr height=25>"\
 					"<td align=left id=tdleft><a href=wp_ospf_bcon.cgi?UN=%s target=mainFrame style=color:#000000><font id=yingwen_san>OSPF</font><font id=%s>%s</font></a></td>",encry,search(lpublic,"menu_san"),search(lcontrol,"rip_conf"));						
 					fprintf(cgiOut,"</tr>");
 					fprintf(cgiOut,"<tr height=25>"\
 					"<td align=left id=tdleft><a href=wp_ospf_intf.cgi?UN=%s target=mainFrame style=color:#000000><font id=yingwen_san>OSPF</font><font id=%s>%s</font></a></td>",encry,search(lpublic,"menu_san"),search(lcontrol,"riplist"));						
 					fprintf(cgiOut,"</tr>");
				  	fprintf(cgiOut,"<tr height=26>"\
        			"<td align=left id=tdleft background=/images/bottom_bg.gif style=\"border-right:0\"><font id=%s>%s</font></td>",search(lpublic,"menu_san"),search(lcontrol,"add_ospf_intf"));   /*突出显示*/
        		  	fprintf(cgiOut,"</tr>");
 

					  for(i=0;i<20;i++)
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
						  "<table width=500 border=0 cellspacing=0 cellpadding=0>");

				  			interfaceInfo(intfName,&IntfNum,lpublic);
							fprintf(cgiOut,"<tr align=left height=35>"\
							"<td width=10 style=color:red>*</td>");
							fprintf(cgiOut,"<td align=left width=60>%s: </td>",search(lcontrol,"interface"));
							fprintf(cgiOut,"<td align=left width=40>");
							fprintf(cgiOut, "<select name=\"OSPF_intf\" onChange=\"javacript:this.form.submit();\">");
							for(i=0;i<IntfNum;i++)
							{
								if(strcmp(intfName[i],"lo")!=0)
									{
										if(strcmp(intfName[i],select_opt) == 0)
											fprintf(cgiOut, "<option  selected=selected value=%s>%s",intfName[i],intfName[i]);
										else
											fprintf(cgiOut, "<option value=%s>%s",intfName[i],intfName[i]);
									}
							}
                         	fprintf(cgiOut, "</select>\n");
							fprintf(cgiOut,"</td>");
							/////add at 2009-3-3/////

							int Net_num = 0;
							if( strcmp(select_opt , "") != 0 )
								show_intf_network( select_opt ,Network ,&Net_num ,lpublic ) ;
							else if( IntfNum > 0 )
								show_intf_network( intfName[0] ,Network ,&Net_num ,lpublic );
							
							//fprintf(stderr,"intfName[0]=%s-Net_num=%d",intfName[0],Net_num);
							fprintf(cgiOut,"<td width=10 style=color:red>*</td>");
							fprintf(cgiOut,"<td align=left width=60>%s: </td>",search(lcontrol,"network"));
							fprintf(cgiOut,"<td align=left width=40>");
							fprintf(cgiOut, "<select name=\"intf_network\">");
							for(i=0;i<Net_num;i++)
							{
                     				fprintf(cgiOut, "<option value=%s>%s",Network[i],Network[i]);
							}
                         	fprintf(cgiOut, "</select>\n");
							fprintf(cgiOut,"</td>");
							fprintf(cgiOut,"<td width=10 style=color:red>*</td>");
							fprintf(cgiOut,"<td align=right>%s: </td>",search(lcontrol,"area_ID"));
							fprintf(cgiOut,"<td align=left><input type=text name=areaID size=12></td>");
							fprintf(cgiOut,"</tr>"\
							
							"</table>"\
						  "</td>"\
						"</tr>"\
						"<tr>"\
							"<td id=sec1 style=\"border-bottom:2px solid #53868b;font-size:14px;padding-top:12px\">%s</td>",search(lcontrol,"interface_set"));
							fprintf(cgiOut,"</tr>"\
						"<tr>"\
						"<td>"\
						"<table width=500 border=0 cellspacing=0 cellpadding=0>"\
						"<tr height=35 style=padding-top:5px>"\
						"<td width=10>&nbsp;</td>");
						fprintf(cgiOut,"<td align=left>%s: </td>",search(lcontrol,"intf_net_type"));
						fprintf(cgiOut,"<td align=left width=120 colspan=4>");
						fprintf(cgiOut, "<select name=if_netType>");
						fprintf(cgiOut, "<option value=broadcast>%s",search(lcontrol,"broadcast"));
                     	fprintf(cgiOut, "<option value=non-broadcast>NBMA");	
						fprintf(cgiOut, "<option value=point-to-multipoint>%s",search(lcontrol,"point_to_mulpoint"));
						fprintf(cgiOut, "<option value=point-to-point>%s",search(lcontrol,"point_to_point"));
                     	fprintf(cgiOut, "</select>");
						fprintf(cgiOut,"</td>"\
						"</tr>"\
						"<tr align=left height=35>"\
						"<td width=10>&nbsp;</td>");
						fprintf(cgiOut,"<td align=left width=160>%s: </td>",search(lcontrol,"hello_interval"));
						fprintf(cgiOut, "<td width=90 align=left><input type=text name=hello_text size=6 value=10></td>");
						fprintf(cgiOut,"<td align=left width=160 colspan=2>%s: </td>",search(lcontrol,"dead_interval"));
						fprintf(cgiOut, "<td width=90 align=left><input type=text name=dead_text size=6 value=40></td>");
						fprintf(cgiOut,"</tr>");
						
						fprintf(cgiOut,"<tr align=left height=35>"\
						"<td width=10>&nbsp;</td>");
						fprintf(cgiOut,"<td align=left width=160>%s: </td>",search(lcontrol,"LSA_transmit"));
						fprintf(cgiOut, "<td width=90 align=left><input type=text name=LSA_transmit size=6 value=1></td>");
						fprintf(cgiOut,"<td align=left width=160 colspan=2>%s: </td>",search(lcontrol,"LSA_retransmit"));
						fprintf(cgiOut, "<td width=90 align=left><input type=text name=LSA_retransmit size=6 value=5></td>");
						fprintf(cgiOut,"</tr>");

						fprintf(cgiOut,"<tr align=left height=35>");	
						fprintf(cgiOut, "<td colspan=2><input type=checkbox name=mtu_ignore value=mtu_ig>%s</td>",search(lcontrol,"mtu_ignore"));
						fprintf(cgiOut, "<td align=right>%s: </td>",search(lcontrol,"prior"));
						fprintf(cgiOut, "<td align=left><input type=text name=Priority size=6 value=1></td>");
						fprintf(cgiOut, "<td align=right>%s: </td>",search(lcontrol,"cost"));
						fprintf(cgiOut, "<td><input type=text name=Cost size=6 value=10></td>");
						fprintf(cgiOut,"</tr>"\
						/*"<tr align=left height=35>");
						fprintf(cgiOut, "<td>&nbsp;</td>");

						fprintf(cgiOut,"</tr>"\*/
						"</table>"\
						"</td>"\
						"</tr>"\
/////////////////////////////////////area configure/////////////////////////////////////
						"<tr>"\
							"<td id=sec1 style=\"border-bottom:2px solid #53868b;font-size:14px;padding-top:12px\">%s</td>",search(lcontrol,"area_set"));
							fprintf(cgiOut,"</tr>"\
						"<tr>"\
						"<td>"\

						"<table width=500 border=0 cellspacing=0 cellpadding=0>"\
						"<tr>"\
						"<td colspan=5><input type=checkbox name=IS_ABR value=area_abr onclick=changestate(this)>%s</td>",search(lcontrol,"IS_ABR"));
						fprintf(cgiOut,"</tr>"\
						"<tr height=35 style=padding-top:5px>");
						
						//fprintf(cgiOut,"<td align=left width=110>%s: </td>",search(lcontrol,"area_type"));
						fprintf(cgiOut,"<td width=110><input type=checkbox name=area_type_check value=area_type_check>%s</td>",search(lcontrol,"area_type"));
						fprintf(cgiOut,"<td align=left width=40>");
						fprintf(cgiOut, "<select name=area_type onchange=changestate(this)>");
                     	fprintf(cgiOut, "<option value=nssa>nssa");
						fprintf(cgiOut, "<option value=stub>stub");
                     	fprintf(cgiOut, "</select>");
						fprintf(cgiOut,"</td>");
						fprintf(cgiOut,"<td align=left width=120>");
						fprintf(cgiOut, "<select name=nssa_param>");
                     	fprintf(cgiOut, "<option value=candidate>translate-candidate");
						fprintf(cgiOut, "<option value=never>translate-never");
						fprintf(cgiOut, "<option value=always>translate-always");
                     	fprintf(cgiOut, "</select>");
						fprintf(cgiOut,"</td>"\
						"<td width=240 align=left><input type=checkbox name=no_summary value=no_summary>no-summary</td>"\
						"</tr>"\
						"</table>"\
						"</td>"\
						"</tr>"\

						"<tr>"\
						"<td>"\
						"<table width=500 border=0 cellspacing=0 cellpadding=0>"\
						"<tr align=left height=35>"\
				        "<td width=110><input type=checkbox name=route_range value=route_range>%s</td>",search(lcontrol,"route_range"));
						fprintf(cgiOut,"<td align=left width=140>"\
					    "<div style=\"border-width:1;border-color:#a5acb2;border-style:solid;width:140;font-size:9pt\">");
					    fprintf(cgiOut,"<input type=text name=src_ip1 maxlength=3 class=a3 onKeyUp=\"mask(this,%s)\" onbeforepaste=mask_c()>.",search(lpublic,"ip_error"));
					    fprintf(cgiOut,"<input type=text name=src_ip2 maxlength=3 class=a3 onKeyUp=\"mask(this,%s)\" onbeforepaste=mask_c()>.",search(lpublic,"ip_error"));
					    fprintf(cgiOut,"<input type=text name=src_ip3 maxlength=3 class=a3 onKeyUp=\"mask(this,%s)\" onbeforepaste=mask_c()>.",search(lpublic,"ip_error"));
					    fprintf(cgiOut,"<input type=text name=src_ip4 maxlength=3 class=a3 onKeyUp=\"mask(this,%s)\" onbeforepaste=mask_c()>",search(lpublic,"ip_error"));
					    fprintf(cgiOut,"</div>");
					    fprintf(cgiOut,"</td>");
						fprintf(cgiOut,"<td width=10 style=padding-left:2px align=center>%s</td>","/");
    					fprintf(cgiOut,"<td  align=left style=padding-left:2px  width=15><input type=text name=masklen size=3></td>");
						fprintf(cgiOut,"<td align=left width=230 style=padding-left:2px>");
						fprintf(cgiOut, "<select name=advertise_param>");
                     	fprintf(cgiOut, "<option value=advertise>advertise");
						fprintf(cgiOut, "<option value=not-advertise>not-advertise");
                     	fprintf(cgiOut, "</select>");
						fprintf(cgiOut,"</td>");
						fprintf(cgiOut,"</tr>"\

						"<tr align=left height=35>"\
				        "<td><input type=checkbox name=virtual_link value=virtual_link>%s</td>",search(lcontrol,"virtual_link"));
						fprintf(cgiOut,"<td align=left>"\
					    "<div style=\"border-width:1;border-color:#a5acb2;border-style:solid;width:140;font-size:9pt\">");
					    fprintf(cgiOut,"<input type=text name=vlink_ip1 maxlength=3 class=a3 onKeyUp=\"mask(this,%s)\" onbeforepaste=mask_c()>.",search(lpublic,"ip_error"));
					    fprintf(cgiOut,"<input type=text name=vlink_ip2 maxlength=3 class=a3 onKeyUp=\"mask(this,%s)\" onbeforepaste=mask_c()>.",search(lpublic,"ip_error"));
					    fprintf(cgiOut,"<input type=text name=vlink_ip3 maxlength=3 class=a3 onKeyUp=\"mask(this,%s)\" onbeforepaste=mask_c()>.",search(lpublic,"ip_error"));
					    fprintf(cgiOut,"<input type=text name=vlink_ip4 maxlength=3 class=a3 onKeyUp=\"mask(this,%s)\" onbeforepaste=mask_c()>",search(lpublic,"ip_error"));
					    fprintf(cgiOut,"</div>");
					    fprintf(cgiOut,"</td>");
						fprintf(cgiOut,"<td style=padding-left:2px align=left colspan=3>(%s)</td>",search(lcontrol,"Router_ID"));
						fprintf(cgiOut,"</tr>"\

						"</table>"\
						"</td>"\
						"</tr>"\

						"<tr>"\
						"<td>"\
						"<table>"\
						"<tr>"\
						"<td><input type=checkbox name=distribute_in onclick=check_ospf_enable(this)>%s: </td>",search(lcontrol,"distribute_in"));
						fprintf(cgiOut,"<td align=left>%s: </td>",search(lcontrol,"access_index"));
						fprintf(cgiOut,"<td align=left><input type=text name=acl_index_in size=6 ></td>"\
						"</tr>"\
						"<tr>"\
						"<td><input type=checkbox name=distribute_out onclick=check_ospf_enable(this)>%s: </td>",search(lcontrol,"distribute_out"));
						fprintf(cgiOut,"<td align=left>%s: </td>",search(lcontrol,"access_index"));
						fprintf(cgiOut,"<td align=left><input type=text name=acl_index_out size=6 ></td>"\
						"</tr>"\
						"</table>"\
						
						"</td>"\
						"</tr>"\
						"<tr height=35>"\
							"<td id=sec1 style=\"border-bottom:2px solid #53868b;font-size:14px;padding-top:12px\">%s</td>",search(lcontrol,"Authentication"));
							fprintf(cgiOut,"</tr>"\
						"<tr>"\
						"<td>"\
						"<table>"\
						"<tr align=left height=35>"\
						"<td><input type=radio name=radiobutton value=NoAuth onclick=\"changestate(this);\" checked></td>"\
						"<td>None</td>"\
						"</tr>"\
						"<tr height=35 align=left>");
						fprintf(cgiOut,"<td><input type=radio name=radiobutton value=text onclick=\"changestate(this);\"></td>");
						fprintf(cgiOut,"<td>%s: </td>",search(lcontrol,"simple_passwd"));
						fprintf(cgiOut,"<td><input type=text name=simplepwd maxLength=8></td>"\
						"<td colspan=4><font color=red>(%s%d%s)</font></td>",search(lpublic,"most1"),8,search(lpublic,"most2"));
						fprintf(cgiOut,"</tr>"\
						
						"<tr align=left height=35>");
						fprintf(cgiOut,"<td><input type=radio name=radiobutton value=MD5 onclick=\"changestate(this);\"></td>");
						fprintf(cgiOut,"<td>%s: </td>",search(lcontrol,"MD5_passwd"));
						fprintf(cgiOut,"<td><input type=text name=MD5pwd maxLength=16></td>"\
						"<td><font color=red>(%s%d%s)</font></td>",search(lpublic,"most1"),16,search(lpublic,"most2"));
						fprintf(cgiOut,"<td style=padding-left:5px> Key-ID: </td>"\
						"<td><input type=text size=6 name=MD5key_id maxLength=3></td>"\
						"<td><font color=red>(1--255)</font></td>"\
						"</tr>"\
						"</table>"\
						"</td>"\
						"</tr>"\
						"<tr>");

    					fprintf(cgiOut,"<td><input type=hidden name=encry_addintf value=%s></td>",encry);


						fprintf(cgiOut,"</tr>"\
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
for(i=0;i<Info_Num;i++)
{
	free(intfName[i]);
}
for( i = 0; i< Network_Num ; i++ )
{
	free(Network[i]);
}

free(select_opt);
release(lpublic);  
release(lcontrol);

return 0;

}

int interfaceInfo(char * IntfInfo[],int * Num,struct list * lpublic)
{
	FILE * ft;
	char * syscommand=(char *)malloc(200);
	memset(syscommand,0,200);
	strcat(syscommand,"ip_addr.sh 2>/dev/null;");
	strcat(syscommand,"awk '{print $1}' /var/run/apache2/ip_addr.file> /var/run/apache2/InterfaceName.txt");
	int status =system(syscommand);
	int ret = WEXITSTATUS(status);
						 
	if(0 != ret)
		ShowAlert(search(lpublic,"bash_fail"));
	

	char temp[20];
	memset(temp,0,20);
	if((ft=fopen("/var/run/apache2/InterfaceName.txt","r"))==NULL)
		{
			ShowAlert(search(lpublic,"error_open"));
			return 0;
		}
	int i=0;
	while((fgets(temp,18,ft)) != NULL)
		{
			strncpy(IntfInfo[i],temp,strlen(temp)-1);
			i++;
			memset(temp,0,20);
			//fprintf(stderr,"IntfInfo[%d]=%s",i,IntfInfo[i]);
		}
	fclose(ft);
	*Num=i;
	free(syscommand);
	return 1;
}

int executeconfig(char * IntfInfo[],int num,struct list * lcontrol,struct list * lpublic)
{
	char * intfNamesys=(char * )malloc(300);
	memset(intfNamesys,0,300);
	char * Intfname=(char *)malloc(20);
	memset(Intfname,0,20);
	char * areaID=(char *)malloc(30);
	memset(areaID,0,30);
	char * if_netType=(char *)malloc(30);
	memset(if_netType,0,30);
	
	char * hello_text=(char *)malloc(10);
	memset(hello_text,0,10);
	char * dead_text=(char *)malloc(10);
	memset(dead_text,0,10);
	char * transmit=(char *)malloc(10);
	memset(transmit,0,10);
	char * retransmit=(char *)malloc(10);
	memset(retransmit,0,10);
	char * Priority=(char *)malloc(10);
	memset(Priority,0,10);
	char * Cost=(char *)malloc(10);
	memset(Cost,0,10);
	char * area_type=(char *)malloc(10);
	memset(area_type,0,10);
	char * nssa_param=(char *)malloc(30);
	memset(nssa_param,0,30);
	char * advertise_param=(char *)malloc(30);
	memset(advertise_param,0,30);
	char * acl_index_out=(char *)malloc(10);
	memset(acl_index_out,0,10);
	char * acl_index_in=(char *)malloc(10);
	memset(acl_index_in,0,10);
	
	char * radiobutton=(char *)malloc(10);
	memset(radiobutton,0,10);
	char * simplepwd=(char *)malloc(20);
	memset(simplepwd,0,20);
	char * MD5key_id=(char *)malloc(20);
	memset(MD5key_id,0,20);
	char *endptr = NULL;  
	int md5key = 0;
	


	char dip1[4],dip2[4],dip3[4],dip4[4];
	char * dstip=(char *)malloc(20);
	memset(dstip,0,20);
	char * dmasklength=(char *)malloc(10);
	memset(dmasklength,0,10);
	//////////////////////////////////////////////////
	char * IFNameCompare=(char *)malloc(30);
	memset(IFNameCompare,0,30);
	char * Adjacent_neighbor_count=(char *)malloc(10);
	memset(Adjacent_neighbor_count,0,10);
	//////////////////////////////////////
	char * networktemp = (char *)malloc(30);
	memset( networktemp , 0 , 30 );
	int flag=0,i=0;
	
	
	cgiFormStringNoNewlines("OSPF_intf",Intfname,20);
	cgiFormStringNoNewlines("intf_network",networktemp,30);
	cgiFormStringNoNewlines("areaID",areaID,30);
	fprintf(stderr,"networktemp=%s",networktemp);
	//networktemp=nameToIP(Intfname,lpublic);
	//fprintf(stderr,"networktemp=%s",networktemp);
	//fprintf(stderr,"areaID=%s",areaID);
	////////////////////查询已配置过的接口///////////////////
	char * OSPF_intf_info[Info_Num]; 
	for(i=0;i<Info_Num;i++)
	{
		OSPF_intf_info[i]=(char *)malloc(60);
		memset(OSPF_intf_info[i],0,60);
	}
	int Ospf_item_num=0,j=0;
	ReadConfig_INTF(OSPF_intf_info,&Ospf_item_num,lpublic);
	char * revOSPFRouteInfo[5]={NULL};
	for(i=0;i<Ospf_item_num;i++)
		{
			if(strcmp(Adjacent_neighbor_count,"")!=0)
				{
					
				}
			revOSPFRouteInfo[0]=strtok(OSPF_intf_info[i],"-");
			if(strstr(revOSPFRouteInfo[0],"Ifname")!=NULL)
				{
					j=0;
						while(revOSPFRouteInfo[j]!=NULL && j<1)
							{                    							
								revOSPFRouteInfo[j+1]=strtok(NULL,"-\n");
								j++;
							}
						strcpy(IFNameCompare,revOSPFRouteInfo[1]);
						//fprintf(stderr,"IFNameCompare=%s",IFNameCompare);
				}
			if(strstr(revOSPFRouteInfo[0],"Adjacent neighbor")!=NULL)
				{
					j=0;
						while(revOSPFRouteInfo[j]!=NULL && j<1)
							{
								revOSPFRouteInfo[j+1]=strtok(NULL,"-\n");
								j++;
							}
						strcpy(Adjacent_neighbor_count,revOSPFRouteInfo[1]);
						//fprintf(stderr,"Adjacent_neighbor_count=%s",revOSPFRouteInfo[1]);
				}
			if(strcmp(IFNameCompare,Intfname)==0)
				flag=1;
		}
	
	//fprintf(stderr,"flag=%d",flag);
	////////////////////////////这里开始作用脚本///////////////////////////////////////
	//fprintf(stderr,"dddddaaaanetworktemp=%s",networktemp);
	int status=0,ret=0;
	if(flag!=1)
		{
			if(strcmp(areaID,"")!=0)
				{
        	    	strcat(intfNamesys,"ospf_network.sh");
        	    	strcat(intfNamesys," ");
        	    	strcat(intfNamesys,"on");
        	    	strcat(intfNamesys," ");
        	    	strcat(intfNamesys,networktemp);
        	    	strcat(intfNamesys," ");
        	    	strcat(intfNamesys,areaID);
					strcat(intfNamesys," ");
        	    	strcat(intfNamesys,">/dev/null");
					fprintf(stderr,"intfNamesys=%s",intfNamesys);
        	    	status = system(intfNamesys);
        	    	ret = WEXITSTATUS(status);
        	        					 
        	        	if(-3==ret)
        	        		ShowAlert(search(lpublic,"bash_fail"));
				}
			else
				{
					ShowAlert(search(lcontrol,"area_not_null"));
					return -1;
				}
      /////////////////////////////////////////////////////////////////////////
			memset(intfNamesys,0,300);
			cgiFormStringNoNewlines("if_netType",if_netType,30);
			//fprintf(stderr,"if_netType=%s",if_netType);
			strcat(intfNamesys,"ospf_if_network.sh");
	    	strcat(intfNamesys," ");
	    	strcat(intfNamesys,Intfname);
	    	strcat(intfNamesys," ");
			strcat(intfNamesys,"on");
			strcat(intfNamesys," ");
			strcat(intfNamesys,if_netType);
			strcat(intfNamesys," ");
			strcat(intfNamesys,">/dev/null");
			status = system(intfNamesys);
        	ret = WEXITSTATUS(status);
            					 
            	if(-3==ret)
            		ShowAlert(search(lpublic,"bash_fail"));
		///////////////////////////////////////////////////////////////////////
			memset(intfNamesys,0,300);
			cgiFormStringNoNewlines("hello_text",hello_text,10);
			if(strcmp(hello_text,"10")!=0)
				{
        			strcat(intfNamesys,"ospf_if_hellointervla.sh");
        	    	strcat(intfNamesys," ");
        	    	strcat(intfNamesys,Intfname);
        	    	strcat(intfNamesys," ");
        			strcat(intfNamesys,"on");
        			strcat(intfNamesys," ");
        			strcat(intfNamesys,hello_text);
        			strcat(intfNamesys," ");
        			strcat(intfNamesys,"non");
        			strcat(intfNamesys," ");
        			strcat(intfNamesys,">/dev/null");
        			status = system(intfNamesys);
                	ret = WEXITSTATUS(status);
                    					 
                    	if(-3==ret)
                    		ShowAlert(search(lpublic,"bash_fail"));
				}
		//////////////////////////////////////////////////////////////////////
			memset(intfNamesys,0,300);
			cgiFormStringNoNewlines("dead_text",dead_text,10);
			if(strcmp(dead_text,"40")!=0)
				{
        			strcat(intfNamesys,"ospf_if_deadinterval.sh");
        	    	strcat(intfNamesys," ");
        	    	strcat(intfNamesys,Intfname);
        	    	strcat(intfNamesys," ");
        			strcat(intfNamesys,"on");
        			strcat(intfNamesys," ");
        			strcat(intfNamesys,dead_text);
        			strcat(intfNamesys," ");
        			strcat(intfNamesys,"non");
        			strcat(intfNamesys," ");
        			strcat(intfNamesys,">/dev/null");
        			status = system(intfNamesys);
                	ret = WEXITSTATUS(status);
                    					 
                	if(-3==ret)
                		ShowAlert(search(lpublic,"bash_fail"));
				}
		///////////////////////////////////////////////////////////////////////
			memset(intfNamesys,0,300);
			cgiFormStringNoNewlines("LSA_transmit",transmit,10);
			if(strcmp(transmit,"1")!=0)
				{
        			strcat(intfNamesys,"ospf_if_tran.sh");
        	    	strcat(intfNamesys," ");
        	    	strcat(intfNamesys,Intfname);
        	    	strcat(intfNamesys," ");
        			strcat(intfNamesys,"on");
        			strcat(intfNamesys," ");
        			strcat(intfNamesys,transmit);
        			strcat(intfNamesys," ");
        			strcat(intfNamesys,"non");
        			strcat(intfNamesys," ");
        			strcat(intfNamesys,">/dev/null");
        			status = system(intfNamesys);
                	ret = WEXITSTATUS(status);
                    					 
                	if(-3==ret)
                		ShowAlert(search(lpublic,"bash_fail"));
				}
		//////////////////////////////////////////////////////////////////////
			memset(intfNamesys,0,300);
			cgiFormStringNoNewlines("LSA_retransmit",retransmit,10);
			if(strcmp(retransmit,"5")!=0)
				{
        			strcat(intfNamesys,"ospf_if_tran_inter.sh");
        	    	strcat(intfNamesys," ");
        	    	strcat(intfNamesys,Intfname);
        	    	strcat(intfNamesys," ");
        			strcat(intfNamesys,"on");
        			strcat(intfNamesys," ");
        			strcat(intfNamesys,retransmit);
        			strcat(intfNamesys," ");
        			strcat(intfNamesys,"non");
        			strcat(intfNamesys," ");
        			strcat(intfNamesys,">/dev/null");
        			status = system(intfNamesys);
                	ret = WEXITSTATUS(status);
                    					 
                	if(-3==ret)
                		ShowAlert(search(lpublic,"bash_fail"));
				}
		/////////////////////////////////////////////////////////////////////
			memset(intfNamesys,0,300);
         	int result=0;
         	char **responses;
         	result = cgiFormStringMultiple("mtu_ignore", &responses);
         	if(responses[0])
         	{
         		strcat(intfNamesys,"ospf_if_mtu.sh");
				strcat(intfNamesys," ");
        	    strcat(intfNamesys,Intfname);
         		strcat(intfNamesys," ");
         		strcat(intfNamesys,"on");
         		strcat(intfNamesys," ");
         		strcat(intfNamesys,"non");
				strcat(intfNamesys," ");
             	strcat(intfNamesys,">/dev/null");
         		system(intfNamesys);
				status = system(intfNamesys);
            	ret = WEXITSTATUS(status);
                					 
            	if(-3==ret)
            		ShowAlert(search(lpublic,"bash_fail"));
         	}
		////////////////////////////////////////////////////////////////////
			memset(intfNamesys,0,300);
			cgiFormStringNoNewlines("Priority",Priority,10);
			if(strcmp(hello_text,"1")!=0)
				{
        			strcat(intfNamesys,"ospf_if_pri.sh");
        	    	strcat(intfNamesys," ");
        	    	strcat(intfNamesys,Intfname);
        	    	strcat(intfNamesys," ");
        			strcat(intfNamesys,"on");
        			strcat(intfNamesys," ");
        			strcat(intfNamesys,Priority);
        			strcat(intfNamesys," ");
        			strcat(intfNamesys,"non");
        			strcat(intfNamesys," ");
        			strcat(intfNamesys,">/dev/null");
        			status = system(intfNamesys);
                	ret = WEXITSTATUS(status);
                    					 
                	if(-3==ret)
                		ShowAlert(search(lpublic,"bash_fail"));
				}
		//////////////////////////////////////////////////////////////////////
			memset(intfNamesys,0,300);
			cgiFormStringNoNewlines("Cost",Cost,10);
			if(strcmp(Cost,"")!=0)
				{
        			strcat(intfNamesys,"ospf_if_cost.sh");
        	    	strcat(intfNamesys," ");
        	    	strcat(intfNamesys,Intfname);
        	    	strcat(intfNamesys," ");
        			strcat(intfNamesys,"on");
        			strcat(intfNamesys," ");
        			strcat(intfNamesys,Cost);
        			strcat(intfNamesys," ");
        			strcat(intfNamesys,"non");
        			strcat(intfNamesys," ");
        			strcat(intfNamesys,">/dev/null");
        			status = system(intfNamesys);
                	ret = WEXITSTATUS(status);
                    					 
                	if(-3==ret)
                		ShowAlert(search(lpublic,"bash_fail"));
				}
				
		///////////////////////////下面是区域//////////////////////////////////////////
			memset(intfNamesys,0,300);
         	result=0;
         	char **responses2;
			char **responses1;
         	result = cgiFormStringMultiple("IS_ABR", &responses2);
			cgiFormStringNoNewlines("area_type",area_type,10);
			cgiFormStringNoNewlines("nssa_param",nssa_param,30);
			
			char **responses3;
         	result = cgiFormStringMultiple("no_summary", &responses3);
			result=0;
			result = cgiFormStringMultiple("area_type_check", &responses1);
         	//if(responses2[0])
         	//fprintf(stderr,"area_type=%s-nssa_param=%s",area_type,nssa_param);
			if(responses1[0])
				{
            		if(strcmp(area_type,"nssa")==0)
            			{
            				strcat(intfNamesys,"ospf_nssa.sh");
                	    	strcat(intfNamesys," ");
                	    	strcat(intfNamesys,areaID);
                	    	strcat(intfNamesys," ");
                			strcat(intfNamesys,"on");
       					if(responses2[0] && strcmp(nssa_param,"")!=0)
       						{
       		         			strcat(intfNamesys," ");
       		         			strcat(intfNamesys,nssa_param);
       						}
       					if(responses2[0] && responses3[0])
       						{
       		         			strcat(intfNamesys," ");
       		         			strcat(intfNamesys,"no-summary");
       						}
                			strcat(intfNamesys," ");
                			strcat(intfNamesys,">/dev/null");
                			status = system(intfNamesys);
                        	ret = WEXITSTATUS(status);
                            					 
                        	if(-3==ret)
                        		ShowAlert(search(lpublic,"bash_fail"));
                 		}
       			  else
       				{
       					strcat(intfNamesys,"ospf_stub.sh");
                	    	strcat(intfNamesys," ");
                	    	strcat(intfNamesys,areaID);
                	    	strcat(intfNamesys," ");
                			strcat(intfNamesys,"on");
       					if(responses2[0] && responses3[0])
       						{
       		         			strcat(intfNamesys," ");
       		         			strcat(intfNamesys,"no-summary");
       						}
                			strcat(intfNamesys," ");
                			strcat(intfNamesys,">/dev/null");
                			status = system(intfNamesys);
                        	ret = WEXITSTATUS(status);
                            					 
                        	if(-3==ret || ret==-2 )
                        		ShowAlert(search(lpublic,"bash_fail"));
       				}
				}
		////////////////////////////////聚合//////////////////////////////////////////
				memset(intfNamesys,0,300);
                char **responses4;
         		result = cgiFormStringMultiple("route_range", &responses4);
     			memset(dip1,0,4);
             	cgiFormStringNoNewlines("src_ip1",dip1,4);	 
             	strcat(dstip,dip1);
             	strcat(dstip,".");
             	memset(dip2,0,4);
             	cgiFormStringNoNewlines("src_ip2",dip2,4); 
             	strcat(dstip,dip2);  
             	strcat(dstip,".");
             	memset(dip3,0,4);
             	cgiFormStringNoNewlines("src_ip3",dip3,4); 
             	strcat(dstip,dip3);  
             	strcat(dstip,".");
             	memset(dip4,0,4);
             	cgiFormStringNoNewlines("src_ip4",dip4,4);
             	strcat(dstip,dip4);

				cgiFormStringNoNewlines("masklen",dmasklength,10);
				
             	sprintf(dstip,"%s/%s",dstip,dmasklength);

				cgiFormStringNoNewlines("advertise_param",advertise_param,30);
				//fprintf(stderr,"advertise_param=%s-dmasklength=%s",advertise_param,dmasklength);
				if(responses4[0])
					{
						strcat(intfNamesys,"ospf_range.sh");
	         	    	strcat(intfNamesys," ");
	         	    	strcat(intfNamesys,areaID);
	         	    	strcat(intfNamesys," ");
	         			strcat(intfNamesys,"on");
						strcat(intfNamesys," ");
	         			strcat(intfNamesys,dstip);
						if(strcmp(advertise_param,"")!=0)
							{
								strcat(intfNamesys," ");
	         					strcat(intfNamesys,advertise_param);
							}
						strcat(intfNamesys," ");
	         			strcat(intfNamesys,">/dev/null");
						status = system(intfNamesys);
                     	ret = WEXITSTATUS(status);
                         					 
                     	if(-3==ret)
                     		ShowAlert(search(lpublic,"bash_fail"));
					}

				
		///////////////////////////虚连接//////////////////////////////////////
				memset(intfNamesys,0,300);
				memset(dmasklength,0,10);
				memset(dstip,0,20);
                char **responses5;
				result = cgiFormStringMultiple("virtual_link", &responses5);
     			memset(dip1,0,4);
             	cgiFormStringNoNewlines("vlink_ip1",dip1,4);	 
             	strcat(dstip,dip1);
             	strcat(dstip,".");
             	memset(dip2,0,4);
             	cgiFormStringNoNewlines("vlink_ip2",dip2,4); 
             	strcat(dstip,dip2);  
             	strcat(dstip,".");
             	memset(dip3,0,4);
             	cgiFormStringNoNewlines("vlink_ip3",dip3,4); 
             	strcat(dstip,dip3);  
             	strcat(dstip,".");
             	memset(dip4,0,4);
             	cgiFormStringNoNewlines("vlink_ip4",dip4,4);
             	strcat(dstip,dip4);
             	//sprintf(dstip,"%s/%s",dstip,dmasklength);
				
				if(responses5[0])
					{
						strcat(intfNamesys,"ospf_virtual-link.sh");
						strcat(intfNamesys," ");
	         			strcat(intfNamesys,"on");
	         	    	strcat(intfNamesys," ");
	         	    	strcat(intfNamesys,areaID);
						strcat(intfNamesys," ");
	         			strcat(intfNamesys,dstip);	
						strcat(intfNamesys," ");
	         			strcat(intfNamesys,">/dev/null");
						status = system(intfNamesys);
                     	ret = WEXITSTATUS(status);
                         					 
                     	if(-3==ret)
                     		ShowAlert(search(lpublic,"bash_fail"));
					}
		///////////////////////////////////////过滤///////////////////////////////
				memset(intfNamesys,0,300);
				char **responses6;
				result = cgiFormStringMultiple("distribute_in", &responses6);
				char **responses7;
				result = cgiFormStringMultiple("distribute_out", &responses7);
				cgiFormStringNoNewlines("acl_index_in",acl_index_in,10);
				cgiFormStringNoNewlines("acl_index_out",acl_index_out,10);
				//fprintf(stderr,"acl_index_in=%s-acl_index_out=%s",acl_index_in,acl_index_out);
				if(responses6[0])
					{
						strcat(intfNamesys,"ospf_filter_list.sh");
	         	    	strcat(intfNamesys," ");
	         	    	strcat(intfNamesys,areaID);
						strcat(intfNamesys," ");
	         			strcat(intfNamesys,"on");
						strcat(intfNamesys," ");
	         			strcat(intfNamesys,acl_index_in);
						strcat(intfNamesys," ");
	         			strcat(intfNamesys,"in");
						strcat(intfNamesys," ");
	         			strcat(intfNamesys,">/dev/null");
						status = system(intfNamesys);
                     	ret = WEXITSTATUS(status);
                         					 
                     	if(-3==ret)
                     		ShowAlert(search(lpublic,"bash_fail"));
					}
				memset(intfNamesys,0,300);
				if(responses7[0])
					{
						strcat(intfNamesys,"ospf_filter_list.sh");
	         	    	strcat(intfNamesys," ");
	         	    	strcat(intfNamesys,areaID);
						strcat(intfNamesys," ");
	         			strcat(intfNamesys,"on");
						strcat(intfNamesys," ");
	         			strcat(intfNamesys,acl_index_out);
						strcat(intfNamesys," ");
	         			strcat(intfNamesys,"out");
						strcat(intfNamesys," ");
	         			strcat(intfNamesys,">/dev/null");
						status = system(intfNamesys);
                     	ret = WEXITSTATUS(status);
                         					 
                     	if(-3==ret)
                     		ShowAlert(search(lpublic,"bash_fail"));
					}
		///////////////////////////////////认证///////////////////////////////////////////

				
				memset(intfNamesys,0,300);
				cgiFormString("radiobutton", radiobutton, 10);
             	if(strcmp(radiobutton,"NoAuth")==0)
             	{}
             	else if(strcmp(radiobutton,"text")==0)
             	{
             		
             		
             		cgiFormString("simplepwd", simplepwd, 20);
					if(strcmp(simplepwd,"")==0)
					{
						ShowAlert(search(lpublic,"pass_not_null"));
						return -1;
					}
					
					if(strlen(simplepwd)>8)
					{
						ShowAlert(search(lpublic,"pass_too_long"));
						return -1;
					}
             		strcat(intfNamesys,"ospf_if_authentication_key.sh");
					strcat(intfNamesys," ");
             		strcat(intfNamesys,Intfname);
             		strcat(intfNamesys," ");
             		strcat(intfNamesys,"ON");
             		strcat(intfNamesys," ");
             		strcat(intfNamesys,simplepwd);
					strcat(intfNamesys," ");
             		strcat(intfNamesys,"NON");
					strcat(intfNamesys," ");
             		strcat(intfNamesys,">/dev/null");
					status = system(intfNamesys);
                 	ret = WEXITSTATUS(status);
                     					 
                 	if(-3==ret)
                 		ShowAlert(search(lpublic,"bash_fail"));
					memset(intfNamesys,0,300);
					strcat(intfNamesys,"ospf_if_authentication.sh");
					strcat(intfNamesys," ");
             		strcat(intfNamesys,Intfname);
             		strcat(intfNamesys," ");
             		strcat(intfNamesys,"on");
             		strcat(intfNamesys," ");
             		strcat(intfNamesys,"text");
					strcat(intfNamesys," ");
             		strcat(intfNamesys,"text_param4");
					strcat(intfNamesys," ");
             		strcat(intfNamesys,">/dev/null");
             		status = system(intfNamesys);
                 	ret = WEXITSTATUS(status);
                     					 
                 	if(-3==ret)
                 		ShowAlert(search(lpublic,"bash_fail"));
             

             	}
             	else if(strcmp(radiobutton,"MD5")==0)
             	{
             		memset(simplepwd,0,20);
             		cgiFormString("MD5pwd", simplepwd, 20);
					cgiFormString("MD5key_id", MD5key_id, 20);
					if(strcmp(simplepwd,"")==0)
					{
						ShowAlert(search(lpublic,"pass_not_null"));
						return -1;
					}
					
					if(strlen(simplepwd)>16)
					{
						ShowAlert(search(lpublic,"pass_too_long"));
						return -1;
					}
					
					if(strcmp(MD5key_id,"")==0)
					{
						ShowAlert(search(lpublic,"key_id_not_null"));
						return -1;
					}
					md5key=strtoul(MD5key_id,&endptr,10);
					if((md5key<1)||(md5key>255))
					{
						ShowAlert(search(lpublic,"key_id_illegal"));
						return -1;
					}
					//fprintf(stderr,"@@MD5key_id=%s",MD5key_id);
					
             		strcat(intfNamesys,"ospf_if_md5_key.sh");
					strcat(intfNamesys," ");
             		strcat(intfNamesys,Intfname);
             		strcat(intfNamesys," ");
             		strcat(intfNamesys,"on");
					strcat(intfNamesys," ");
             		strcat(intfNamesys,MD5key_id);
             		strcat(intfNamesys," ");
             		strcat(intfNamesys,simplepwd);
					strcat(intfNamesys," ");
             		strcat(intfNamesys,"non");
					strcat(intfNamesys," ");
             		strcat(intfNamesys,">/dev/null");
             		status = system(intfNamesys);
                 	ret = WEXITSTATUS(status);
                     					 
                 	if(-3==ret)
                 		ShowAlert(search(lpublic,"bash_fail"));
             		
             		memset(intfNamesys,0,300);
             		strcat(intfNamesys,"ospf_if_authentication.sh");
					strcat(intfNamesys," ");
             		strcat(intfNamesys,Intfname);
             		strcat(intfNamesys," ");
             		strcat(intfNamesys,"on");
             		strcat(intfNamesys," ");
             		strcat(intfNamesys,"message-digest");
					strcat(intfNamesys," ");
             		strcat(intfNamesys,"non");
					strcat(intfNamesys," ");
             		strcat(intfNamesys,">/dev/null");
             		status = system(intfNamesys);
                 	ret = WEXITSTATUS(status);
                     					 
                 	if(-3==ret)
                 		ShowAlert(search(lpublic,"bash_fail"));
             	}


		
				
			ShowAlert(search(lcontrol,"ospf_add_suc"));
		}
	else ShowAlert(search(lcontrol,"ospf_intf_exist"));


	for(i=0;i<Info_Num;i++)
	{
		free(OSPF_intf_info[i]);	
	}

	free(MD5key_id);
	free(networktemp);
	free(simplepwd);
	free(radiobutton);
	free(acl_index_in);
	free(acl_index_out);
	free(advertise_param);
	free(dmasklength);
	free(dstip);
	free(nssa_param);
	free(area_type);
	free(Priority);
	free(Cost);
	free(dead_text);
	free(hello_text);
	free(Adjacent_neighbor_count);
	free(IFNameCompare);
	free(if_netType);
	free(areaID);
	free(Intfname);
	return 0;
}


char * nameToIP(char * intfname,struct list * lpublic)
{
	FILE * fd;
	char * command=(char *)malloc(100);
	memset(command,0,100);
	strcat(command,"NameToIP.sh");
	strcat(command," ");
	strcat(command,intfname);
	strcat(command," ");
	strcat(command,">/var/run/apache2/NameToIp.txt");
	system(command);
	fprintf(stderr,"command=%s",command);
	if((fd=fopen("/var/run/apache2/NameToIp.txt","r"))==NULL)
			{
				ShowAlert(search(lpublic,"error_open"));
				return 0;
			}
	#if 1 
	char * temp=(char *)malloc(30);
	memset(temp,0,30);
	char * temp1;
	char * temp2;

	char * templater=(char *)malloc(50);
	memset(templater,0,50);

	fgets(temp,28,fd);
	fclose(fd);

	temp1=strtok(temp,"-/\n");
	temp2=strtok(NULL,"-/\n");
	strcpy(templater,temp1);
	sprintf(templater,"%s/%s",temp1,temp2);

	free(temp);
	free(command);
	#endif

	
	return templater;
}

int ReadConfig_ospf(char * ripInfo[],int * infoNum,struct list * lpublic)
{
	int i;
	char * command=(char *)malloc(200);
	memset(command,0,200);
	strcat(command,"show_run_conf.sh | awk 'BEGIN{FS=\"\\n\";RS=\"!\"}/router ospf/{print}'| awk '{OFS=\"-\";ORS=\"-\\n\"}{$1=$1;print}'  >/var/run/apache2/OSPF_temp_Info.txt");
	int status = system(command);
	int ret = WEXITSTATUS(status);				 
	if(0==ret)
		{}
	else ShowAlert(search(lpublic,"bash_fail"));

	FILE *fd;
	char  temp[60];
	memset(temp,0,60);
	if((fd=fopen("/var/run/apache2/OSPF_temp_Info.txt","r"))==NULL)
		{
			ShowAlert(search(lpublic,"error_open"));
			return 0;
		}
	i=0;
	while((fgets(temp,60,fd)) != NULL)
		{
			strcat(ripInfo[i],temp);
			i++;
			memset(temp,0,60);
		}
	fclose(fd);
	*infoNum=i;
	
	free(command);
	return 1;
}
int ReadConfig_INTF(char * ospfInfo[],int * infoNum,struct list * lpublic)
{
	int i;
	char * command=(char *)malloc(200);
	memset(command,0,200);
	strcat(command,"ospf_show_ip_ospf_if.sh NON | awk 'BEGIN{FS=\"\\n\";RS=\"\\n\\n\"}!/OSPF status/{print}' | awk 'BEGIN{FS=\":\";OFS=\"-\"}{$1=$1;print}' >/var/run/apache2/OSPFIntf.txt");
	int status = system(command);
	int ret = WEXITSTATUS(status);				 
	if(0==ret)
		{}
	else ShowAlert(search(lpublic,"bash_fail"));

	FILE *fd;
	char  temp[60];
	memset(temp,0,60);
	if((fd=fopen("/var/run/apache2/OSPFIntf.txt","r"))==NULL)
		{
			ShowAlert(search(lpublic,"error_open"));
			return 0;
		}
	i=0;
	while((fgets(temp,60,fd)) != NULL)
		{
			strcat(ospfInfo[i],temp);
			i++;
			memset(temp,0,60);
		}
	fclose(fd);
	*infoNum=i;
	
	free(command);
	return 1;
}


int show_intf_network(char * intfname,char * revIntfNet[],int * Num ,struct list *lpublic)
{
	FILE * ft;
	char * command=(char * )malloc(250);
	memset(command,0,250);
	char temp[30];
	strcat(command,"show_intf_ip.sh");
	strcat(command," ");
	strcat(command,intfname);
	strcat(command," ");
	strcat(command,"2>/dev/null | awk '{if($1==\"inet\") {print $2}}' >/var/run/apache2/vlan_intf_ip.txt");
	system(command);
	if((ft=fopen("/var/run/apache2/vlan_intf_ip.txt","r"))==NULL)
		{
			ShowAlert(search(lpublic,"error_open"));
			return 0;
		}
	memset(temp , 0, 30);
	int i = 0;
	while(fgets(temp,28,ft))
		{
			strncpy(revIntfNet[i],temp,strlen(temp)-1);
			i++;
			memset(temp,0,30);
		}
	fclose(ft);
	*Num = i;
	free(command);
	return 0;
}


