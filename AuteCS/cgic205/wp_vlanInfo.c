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
* wp_vlanInfo.c
*
*
* CREATOR:
* autelan.software.Network Dep. team
* tangsq@autelan.com
*
* DESCRIPTION:
* system contrl for vlan detail info
*
*
*******************************************************************************/
#include <stdio.h>
#include "cgic.h"
#include <string.h>
#include <stdlib.h>
#include "util/npd_list.h"
#include "sysdef/npd_sysdef.h"
#include "npd/nam/npd_amapi.h"
#include "ws_ec.h"
#include "ws_public.h"
#include "ws_trunk.h"
#include "ws_usrinfo.h"
#include "ws_err.h"
#include "ws_init_dbus.h"
#include "ws_dcli_vlan.h"


void ShowVlandtaPage(char *m,char *n,char *t,char * ta,struct list *lpublic,struct list *lcon);  
//int show_vlan_IP(unsigned short vlanID,char *ipstr,struct list *lpublic);
int show_vlan_IP(unsigned short vlanID,char *ipstr[],struct list *lpublic);

int cgiMain()
{
	char encry[BUF_LEN] = {0}; 
	char ID[10] = {0};
	char vName[21] = {0};
	char *str;          
	struct list *lpublic;   /*解析public.txt文件的链表头*/
	struct list *lcon;     /*解析wlan.txt文件的链表头*/  
	lpublic=get_chain_head("../htdocs/text/public.txt");
	lcon=get_chain_head("../htdocs/text/control.txt");
	ccgi_dbus_init();
	if(cgiFormStringNoNewlines("UN", encry, BUF_LEN)!=cgiFormNotFound )  /*首次进入该页*/
	{    
		cgiFormStringNoNewlines("VID", ID, 10); 
		cgiFormStringNoNewlines("VNAME", vName, 21);
	}
	else
	{
		cgiFormStringNoNewlines("encry_vlandta",encry,BUF_LEN);
		cgiFormStringNoNewlines("vlanid",ID,10);
	}  
	str=dcryption(encry);
	if(str==NULL)
	{
		ShowErrorPage(search(lpublic,"ill_user"));       /*用户非法*/
	}
	else
	{
		ShowVlandtaPage(encry,str,ID,vName,lpublic,lcon);
	}
	release(lpublic);  
	release(lcon);
	return 0;
}

void ShowVlandtaPage(char *m,char *n,char *t,char * ta,struct list *lpublic,struct list *lcon)
{  
  struct Trunklist trunk,*q; 
  memset(&trunk,0,sizeof(struct Trunklist));
  struct Vlanid_info vhead,*vq;
  memset(&vhead,0,sizeof(struct Vlanid_info));
  
  int trunkNum=0;
  int i = 0;
  int cl=1;
  int retu= 0 ;
  int result1 = 0;
  int result2= 0;
  int limit = 0;;                 /*颜色初值为#f9fafe*/
  int vlan_id = 0;

    
  cgiHeaderContentType("text/html");
  fprintf(cgiOut,"<html xmlns=\"http://www.w3.org/1999/xhtml\"><head>");
  fprintf(cgiOut,"<meta http-equiv=Content-Type content=text/html; charset=gb2312>");
  fprintf(cgiOut,"<title>Wtp</title>");
  fprintf(cgiOut,"<link rel=stylesheet href=/style.css type=text/css>");
  fprintf(cgiOut,"<style>"\
    "th{ font-family:Arial, Helvetica, sans-serif; font-weight:bold; font-size:12px; color:#0a4e70}"\
    ".vlanInfo{ overflow-x:hidden; overflow:auto; height=340; clip: rect( ); padding-top: 0px; padding-right: 0px; padding-bottom: 0px; padding-left: 0px }"\
  "</style>"\
  "</head>"\
  "<body>"\
  "<form>"\
  "<div align=center>"\
  "<table width=976 border=0 cellpadding=0 cellspacing=0>"\
  "<tr>"\
    "<td width=8 align=left valign=top background=/images/di22.jpg><img src=/images/youce4.jpg width=8 height=30/></td>"\
    "<td width=51 align=left valign=bottom background=/images/di22.jpg><img src=/images/youce33.jpg width=37 height=24/></td>"\
    "<td width=153 align=left valign=bottom id=%s background=/images/di22.jpg>%s</td>",search(lpublic,"title_style"),search(lcon,"vlan_manage"));
    fprintf(cgiOut,"<td width=690 align=right valign=bottom background=/images/di22.jpg>");
		
	fprintf(cgiOut,"<table width=155 border=0 cellspacing=0 cellpadding=0>"\
	"<tr>"\
	"<td width=62 align=center><a href=wp_configvlan.cgi?UN=%s target=mainFrame><img src=/images/%s border=0 width=62 height=20/></a></td>",m,search(lpublic,"img_ok"));
	fprintf(cgiOut,"<td width=62 align=center><a href=wp_configvlan.cgi?UN=%s target=mainFrame><img src=/images/%s border=0 width=62 height=20/></a></td>",m,search(lpublic,"img_cancel"));
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
                  retu=checkuser_group(n);
                  fprintf(cgiOut,"<tr height=26>"\
                    "<td align=left id=tdleft background=/images/bottom_bg.gif style=\"border-right:0\"><font id=%s>VLAN %s<font></td>",search(lpublic,"menu_san"),search(lcon,"details"));   /*突出显示*/
                  fprintf(cgiOut,"</tr>");
                  if(retu==0) /*管理员*/
                  {
                        fprintf(cgiOut,"<tr height=25>"\
                          "<td align=left id=tdleft><a href=wp_addvlan.cgi?UN=%s target=mainFrame class=top><font id=%s>%s</font><font id=yingwen_san> VLAN</font></a></td>",m,search(lpublic,"menu_san"),search(lcon,"add"));			  
                        fprintf(cgiOut,"</tr>"); 
        		  }
                        fprintf(cgiOut,"<tr height=25>"\
                          "<td align=left id=tdleft><a href=wp_show_pvlan.cgi?UN=%s target=mainFrame class=top><font id=yingwen_san>PVLAN </font><font id=%s>%s</font></a></td>",m,search(lpublic,"menu_san"),search(lcon,"list"));		  
                        fprintf(cgiOut,"</tr>");
                  if(retu==0) /*管理员*/
                  {
                        fprintf(cgiOut,"<tr height=25>"\
                          "<td align=left id=tdleft><a href=wp_config_pvlan.cgi?UN=%s target=mainFrame class=top><font id=%s>%s</font><font id=yingwen_san> PVLAN</font></a></td>",m,search(lpublic,"menu_san"),search(lcon,"pvlan_add"));			  
                        fprintf(cgiOut,"</tr>");
        		  }
                  limit=0;
                  result1=show_trunklis_by_vlanId(t,&trunk,&trunkNum);
                  result2 = show_vlanid_portlist(t,&vhead);
                  limit=12;
                  if(retu==1) /*普通用户*/
                  	limit+=1;
                  for(i=0;i<limit;i++)
                  {
                    fprintf(cgiOut,"<tr height=25>"\
                      "<td id=tdleft>&nbsp;</td>"\
                    "</tr>");
                  }
                fprintf(cgiOut,"</table>"\
              "</td>"\
              "<td align=left valign=top style=\"background-color:#ffffff; border-right:1px solid #707070; padding-left:30px; padding-top:10px\">"\
         "<table width=500 border=0 bgcolor=#ffffff cellspacing=0 cellpadding=0>"\
          "<tr valign=middle>"\
           "<td align=center valign=top style=\"padding-top:20px\">");
				fprintf(cgiOut,"<div class=vlanInfo><table width=750 border=0 cellspacing=0 cellpadding=0>"\
				"<tr align=left height=20 valign=top>"\
				"<td id=thead1 colspan=2>VLAN %s</td>",search(lpublic,"details"));
				fprintf(cgiOut,"</tr>"\
				"<tr>"\
				"<td colspan=2 align=left style=\"padding-left:40px\">"\
				"<table frame=below rules=rows width=400 border=1>");

				fprintf(cgiOut,"<tr align=left>"\
				"<td id=td1 width=170>VLAN ID</td>"\
				"<td id=td2 width=150>%s</td>",t);
				fprintf(cgiOut,"</tr>"\
				"<tr align=left>"\
				"<td id=td1>VLAN %s</td>",search(lpublic,"name"));
				fprintf(cgiOut,"<td id=td2>%s</td>",ta);
				fprintf(cgiOut,"</tr>");
				
				vlan_id = strtoul(t,0,10);
				//kehao add  20110516
				//char ipstr[128] = {0};
                char *ipstr[8];
				//////////////////////////
				int j=0;
				for( j = 0; j<8 ;j++ )
				{
					ipstr[j] = (char *)malloc(128);
					if(ipstr[j] != NULL)
					{
					 memset( ipstr[j] ,0, 128 );
					}
				}
				//kehao modify  20110517
				/////////////////////////////
				show_vlan_IP(vlan_id, ipstr,lpublic);
				//////////////////////////////////

				fprintf(cgiOut,"<tr align=left>");
				//kehao modify 20110517
				//if( strcmp(ipstr,"") != 0 )
				if( strcmp(ipstr[0],"") != 0 )
			    ////////////////////////////////
				{
					fprintf(cgiOut,"<td id=td1>%s: </td>",search(lcon,"Vlan_L3_IP"));
				}
				else
				{
					fprintf(cgiOut,"<td id=td1>&nbsp;</td>");
				}
				////kehao modify
				//fprintf(cgiOut,"<td id=td2>%s</td>",ipstr);
				int k=0;
				for(k=0;ipstr[k]!=NULL&&ipstr[k]!=0&&k < 8;k++)
				{
					fprintf(cgiOut,"<tr height=25 align=left>");
				  	fprintf(cgiOut,"<td id=td2>%s</td>",ipstr[k]);
					fprintf(cgiOut,"</tr>");
					free(ipstr[k]);
					ipstr[k] = NULL;
					
				}
				
				#if 0
				for(k = 0;k<8;k++)
				{
					free(ipstr[k]);
				}
				#endif 
				//////////////////////////////////////////////////
				fprintf(cgiOut,"</tr>");
				fprintf(cgiOut,"</table></td>"\
				"</tr>"\
				"<tr align=left style=\"padding-top:20px\">");			  
				fprintf(cgiOut,"<td id=thead2>VLAN%s %s</td>",t,search(lpublic,"member_port"));
				fprintf(cgiOut,"<td id=thead2>VLAN%s %s</td>",t,search(lpublic,"member_trunk"));
				fprintf(cgiOut,"</tr>"\
				"<tr>"\
				"<td width=280 align=left valign=top padding-top:10px\"><table align=left frame=below rules=rows width=280 border=1>"\
				"<tr align=left height=20>"\
				"<th width=50>%s</th>",search(lcon,"port_no"));
				fprintf(cgiOut,"<th width=60>Tag %s</th>",search(lpublic,"mode"));
				fprintf(cgiOut,"<th width=80>%s</th>",search(lcon,"promis_mode"));
				fprintf(cgiOut,"</tr>");

				if(result2==1)
				{
           			cl = 1;
					vq = vhead.next;
					while (vq != NULL)
					{
						if (0 == vq->iftag)
						{
							fprintf(cgiOut,"<tr bgcolor=%s>",setclour(cl));							
							fprintf(cgiOut,"<td id=td3>%s</td>",vq->untagport);
							fprintf(cgiOut,"<td id=td3>untag</td>");
							if (vq->untagflag == 1)
							{
								fprintf(cgiOut,"<td id=td3>promis</td>");
							}
							else
							{
								fprintf(cgiOut,"<td id=td3>none</td>");
							}
							fprintf(cgiOut,"</tr>");	
						}
						else
						{
							fprintf(cgiOut,"<tr bgcolor=%s>",setclour(cl));
							fprintf(cgiOut,"<td id=td3>%s</td>",vq->tagport);
							fprintf(cgiOut,"<td id=td3>tag</td>");
							if (vq->tagflag == 1)
							{
								fprintf(cgiOut,"<td id=td3>promis</td>");
							}
							else
							{
								fprintf(cgiOut,"<td id=td3>none</td>");
							}
							fprintf(cgiOut,"</tr>");	
						}
						cl=!cl;
						vq = vq->next;					
					}								
				} 


	
				fprintf(cgiOut,"</table>"\
				"</td>"\
				"<td width=220 align=left valign=top padding-top:10px\">");

				fprintf(cgiOut,"<table align=left frame=below rules=rows width=220 border=1>"\
				"<tr align=left height=20>"\
				"<th width=80>Tag %s</th>",search(lpublic,"mode"));
				fprintf(cgiOut,"<th width=80>Trunk ID</th>");
				fprintf(cgiOut,"</tr>");
				
				if(result1==1)
				{
					cl=1;
					q=trunk.next;
					while(q != NULL)
					{				  
						fprintf(cgiOut,"<tr bgcolor=%s>",setclour(cl));
						if(q->tagMode==2)
						{
							fprintf(cgiOut,"<td id=td3>tag</td>");
						}
						else if(q->tagMode==1)
						{
							fprintf(cgiOut,"<td id=td3>untag</td>");
						}
						fprintf(cgiOut,"<td id=td3>%d</td>",q->TrunkId);
						fprintf(cgiOut,"</tr>");	  
						cl=!cl;
						q=q->next;
					} 
				}

				fprintf(cgiOut,"</table>");

				fprintf(cgiOut,"</td>"\
				"</tr>"\
				"<tr>"\
				"<td><input type=hidden name=encry_vlandta value=%s></td>",m);
				fprintf(cgiOut,"<td><input type=hidden name=vlanid value=%s></td>",t);
				fprintf(cgiOut,"</tr>"\
				"</table></div>");
    fprintf(cgiOut,"</td>"\
 " </tr>"\
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

	if(result1 == 1)
	{
	  Free_vlan_trunk(&trunk);
	}

	if(result2 == 1)
	{
	  Free_vlanid_info(&vhead);
	}


}

//kehao modify  20110517
//int show_vlan_IP(unsigned short vlanID,char *ipstr,struct list *lpublic)
int show_vlan_IP(unsigned short vlanID,char *ipstr[],struct list *lpublic)
{
	
	FILE * ft;
	int tempID=vlanID;
	char command[150] = {0};
	char temp[30] = {0};
	sprintf(temp,"vlan%d",tempID);
    char buf[128] = {0};
	//kehao add  20110516
	///////////////////
  
	int i = 0;
	///////////////////////////////////
	strcat(command,"show_intf_ip.sh");
	strcat(command," ");
	strcat(command,temp);
	strcat(command," ");
	strcat(command,"2>/dev/null | awk '{if($1==\"inet\") {print $2}}' >/var/run/apache2/vlan_intf_ip.txt");
	system(command);
	if((ft=fopen("/var/run/apache2/vlan_intf_ip.txt","r"))==NULL)
	{
		ShowAlert(search(lpublic,"error_open"));
		return 0;
	}
	memset(buf , 0, 128);
	//kehao modify  20110516
	//fgets(buf,128,ft);
	if(ft != NULL)
	{
	   while((fgets(buf,128,ft) != NULL)  && i < 8)
	   {
		
	/////////////////////////////
	      strcpy(ipstr[i],buf);
	      i++;
		  memset(buf,0,128);

	    }
	}
	fclose(ft);
	return 0;
}
