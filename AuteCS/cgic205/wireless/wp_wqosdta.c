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
* wp_wqosdta.c
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
#include "wcpss/wid/WID.h"
#include "ws_dcli_wqos.h"
#include "wcpss/asd/asd.h"
#include "ws_sta.h"
#include "ws_usrinfo.h"
#include "ws_err.h"
#include "ws_ec.h"
#include "ws_dbus_list_interface.h"

char *qos_info[] = {
	"BESTEFFORT",
	"BACKGROUND",
	"VIDEO",
	"VOICE",
};


void ShowWqosdtaPage(char *m,char *n,char *t,char *ins_id,struct list *lpublic,struct list *lwlan);  


int cgiMain()
{
  char encry[BUF_LEN] = { 0 };     
  char ID[10] = { 0 };
  char instance_id[10] = { 0 };
  char *str = NULL;          
  struct list *lpublic = NULL;   /*解析public.txt文件的链表头*/
  struct list *lwlan = NULL;     /*解析wlan.txt文件的链表头*/  
  lpublic=get_chain_head("../htdocs/text/public.txt");
  lwlan=get_chain_head("../htdocs/text/wlan.txt");
  
  DcliWInit();
  ccgi_dbus_init();
  memset(encry,0,sizeof(encry));
  memset(ID,0,sizeof(ID));
  memset(instance_id,0,sizeof(instance_id));
  cgiFormStringNoNewlines("UN", encry, BUF_LEN);  /*首次进入该页*/
  cgiFormStringNoNewlines("ID", ID, 10); 
  cgiFormStringNoNewlines("INSTANCE_ID", instance_id, 10);
  str=dcryption(encry);
  if(str==NULL)
    ShowErrorPage(search(lpublic,"ill_user"));		 /*用户非法*/
  else
    ShowWqosdtaPage(encry,str,ID,instance_id,lpublic,lwlan);
  release(lpublic);  
  release(lwlan);
  destroy_ccgi_dbus();
  return 0;
}

void ShowWqosdtaPage(char *m,char *n,char *t,char *ins_id,struct list *lpublic,struct list *lwlan)
{  
  DCLI_WQOS *wqos = NULL;	
  char *endptr = NULL; 
  int i = 0,j = 0,retu = 1,cl = 1,result = 0,limit = 0;                 /*颜色初值为#f9fafe*/
  char temp[100] = { 0 };
  char wqos_num[5] = { 0 };
  instance_parameter *paraHead1 = NULL;
  dbus_parameter ins_para;

  cgiHeaderContentType("text/html");
  fprintf(cgiOut,"<html xmlns=\"http://www.w3.org/1999/xhtml\"><head>");
  fprintf(cgiOut,"<meta http-equiv=Content-Type content=text/html; charset=gb2312>");
  fprintf(cgiOut,"<title>WQOS</title>");
  fprintf(cgiOut,"<link rel=stylesheet href=/style.css type=text/css>");
  fprintf(cgiOut,"<style>"\
  	"th{ font-family:Arial, Helvetica, sans-serif; font-weight:bold; font-size:12px; color:#0a4e70}"\
"</style>"\
  "</head>"\
  "<body>"\
  "<form>"\
  "<div align=center>"\
  "<table width=976 border=0 cellpadding=0 cellspacing=0>"\
  "<tr>"\
    "<td width=8 align=left valign=top background=/images/di22.jpg><img src=/images/youce4.jpg width=8 height=30/></td>"\
    "<td width=51 align=left valign=bottom background=/images/di22.jpg><img src=/images/youce33.jpg width=37 height=24/></td>"\
    "<td width=153 align=left valign=bottom background=/images/di22.jpg><font id=%s>%s</font><font id=titleen>QOS</font></td>",search(lpublic,"title_style"),search(lwlan,"wireless"));
    fprintf(cgiOut,"<td width=690 align=right valign=bottom background=/images/di22.jpg>");
  
    	  fprintf(cgiOut,"<table width=155 border=0 cellspacing=0 cellpadding=0>"\
          "<tr>"\
		  "<td width=62 align=center><a href=wp_wqoslis.cgi?UN=%s&INSTANCE_ID=%s target=mainFrame><img src=/images/%s border=0 width=62 height=20/></a></td>",m,ins_id,search(lpublic,"img_ok"));
		  fprintf(cgiOut,"<td width=62 align=center><a href=wp_wqoslis.cgi?UN=%s&INSTANCE_ID=%s target=mainFrame><img src=/images/%s border=0 width=62 height=20/></a></td>",m,ins_id,search(lpublic,"img_cancel"));
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
                    "<td align=left id=tdleft background=/images/bottom_bg.gif style=\"border-right:0\"><font id=%s>%s</font><font id=yingwen_san>QOS</font><font id=%s> %s</font></td>",search(lpublic,"menu_san"),search(lwlan,"wireless"),search(lpublic,"menu_san"),search(lpublic,"details"));   /*突出显示*/
                  fprintf(cgiOut,"</tr>");				  
				  retu=checkuser_group(n);
				  if(retu==0) /*管理员*/
				  {
                    fprintf(cgiOut,"<tr height=25>");
					if(strcmp(search(lwlan,"wireless"),"Wireless ")==0)/*英文界面*/
					  fprintf(cgiOut,"<td align=left id=tdleft><a href=wp_wqosnew.cgi?UN=%s target=mainFrame class=top><font id=%s>%s</font><font id=%s> %s</font><font id=yingwen_san>QOS</font></a></td>",m,search(lpublic,"menu_san"),search(lpublic,"create"),search(lpublic,"menu_san"),search(lwlan,"wireless"));                       
					else
					  fprintf(cgiOut,"<td align=left id=tdleft><a href=wp_wqosnew.cgi?UN=%s target=mainFrame class=top><font id=%s>%s</font><font id=%s>%s</font><font id=yingwen_san>QOS</font></a></td>",m,search(lpublic,"menu_san"),search(lpublic,"create"),search(lpublic,"menu_san"),search(lwlan,"wireless"));                       
                    fprintf(cgiOut,"</tr>");
				  }				  
				  
				  get_slotID_localID_instanceID(ins_id,&ins_para);	
				  get_instance_dbus_connection(ins_para, &paraHead1, INSTANCE_STATE_WEB);

				  if(paraHead1)
				  {
					  result=show_qos_one(paraHead1->parameter,paraHead1->connection,t,&wqos);
				  }
                  limit=15;     
				  if(result == 1)
				  {
					  for(i=0;i<4;i++)
					  {
						if((wqos)&&(wqos->qos[0])&&(wqos->qos[0]->radio_qos[i])&&(wqos->qos[0]->radio_qos[i]->mapstate == 1))
						{
						  limit+=1;
						}
					  }
					  for(i=0;i<4;i++)
					  {
					  	if((wqos)&&(wqos->qos[0])&&(wqos->qos[0]->radio_qos[i]))
					  	{
							limit+=wqos->qos[0]->radio_qos[i]->dot1p_map_wmm_num;
					  	}
					  }
				  }
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
 "<table width=510 border=0 bgcolor=#ffffff cellspacing=0 cellpadding=0>"\
  "<tr>"\
    "<td align=center>");
			if(result==1)	
			{	
			   fprintf(cgiOut,"<table width=510 border=0 cellspacing=0 cellpadding=0>"\
			   "<tr>"\
		         "<td id=ins_style>%s:%s</td>",search(lpublic,"instance"),ins_id);
		       fprintf(cgiOut,"</tr>"\
	           "<tr align=left height=10 valign=top>"\
	           "<td id=thead1>WQOS %s</td>",search(lpublic,"details"));
	           fprintf(cgiOut,"</tr>"\
               "<tr>"\
               "<td align=left style=\"padding-left:20px\">"\
			   "<table frame=below rules=rows width=180 border=1>"\
			   "<tr align=left>"\
				  "<td id=td1 width=60>ID</td>"\
				  "<td id=td2 width=120>%s</td>",t);
			   fprintf(cgiOut,"</tr>"\
			   "<tr align=left>"\
			   "<td id=td1>%s</td>",search(lpublic,"name"));
			   if((wqos)&&(wqos->qos[0])&&(wqos->qos[0]->name))
		  	   {
				   fprintf(cgiOut,"<td id=td2>%s</td>",wqos->qos[0]->name);
		  	   }
			  fprintf(cgiOut,"</tr>"\
			  "</table></td>"\
				"</tr>"\
				"<tr align=left style=\"padding-top:20px\">"\
				  "<td id=thead2>%s</td>","radio qos information"\
				"</tr>"\
				"<tr>"\
				"<td align=center style=\"padding-left:20px\"><table align=left frame=below rules=rows width=510 border=1>"\
				"<tr align=left height=20>"\
				  "<th width=100>&nbsp;</th>"\
				  "<th width=90>QueueDepth</th>"\
				  "<th width=70>CWMins</th>"\
				  "<th width=70>CWMax</th>"\
				  "<th width=50>AIFS</th>"\
				  "<th width=80>TXOPlimit</th>"\
				  "<th width=50>ACK</th>"\
				"</tr>");
				for(i=0;i<4;i++)
				{		
				  fprintf(cgiOut,"<tr bgcolor=%s>",setclour(cl));
				  fprintf(cgiOut,"<td id=td3>%s:</td>",qos_info[i]);
				  if((wqos)&&(wqos->qos[0])&&(wqos->qos[0]->radio_qos[i]))
		  	      {
					  fprintf(cgiOut,"<td id=td3>%d</td>",wqos->qos[0]->radio_qos[i]->QueueDepth);
					  fprintf(cgiOut,"<td id=td3>%d</td>",wqos->qos[0]->radio_qos[i]->CWMin);
					  fprintf(cgiOut,"<td id=td3>%d</td>",wqos->qos[0]->radio_qos[i]->CWMax);
					  fprintf(cgiOut,"<td id=td3>%d</td>",wqos->qos[0]->radio_qos[i]->AIFS);
					  fprintf(cgiOut,"<td id=td3>%d</td>",wqos->qos[0]->radio_qos[i]->TXOPlimit);
		  	      }
				  if((wqos)&&(wqos->qos[0])&&(wqos->qos[0]->radio_qos[i])&&(wqos->qos[0]->radio_qos[i]->ACK==1))
				    fprintf(cgiOut,"<td id=td3>ACK</td>");
				  else
				  	fprintf(cgiOut,"<td id=td3>NOACK</td>");
				  fprintf(cgiOut,"</tr>");	  
				  cl=!cl;
				} 
			  fprintf(cgiOut,"</table>"\
				  "</td>"\
				"</tr>"\
                "<tr align=left style=\"padding-top:20px\">"\
				  "<td id=thead2>%s</td>","client qos information"\
				"</tr>"\
				"<tr>"\
				"<td align=center style=\"padding-left:20px\"><table align=left frame=below rules=rows width=460 border=1>"\
				"<tr align=left height=20>"\
				  "<th width=100>&nbsp;</th>"\
				  "<th width=90>QueueDepth</th>"\
				  "<th width=70>CWMins</th>"\
				  "<th width=70>CWMax</th>"\
				  "<th width=50>AIFS</th>"\
				  "<th width=80>TXOPlimit</th>"\
				"</tr>");
				cl=1;
				for(i=0;i<4;i++)
				{		
				  fprintf(cgiOut,"<tr bgcolor=%s>",setclour(cl));
				  fprintf(cgiOut,"<td id=td3>%s:</td>",qos_info[i]);
				  if((wqos)&&(wqos->qos[0])&&(wqos->qos[0]->client_qos[i]))
				  {
					  fprintf(cgiOut,"<td id=td3>%d</td>",wqos->qos[0]->client_qos[i]->QueueDepth);
					  fprintf(cgiOut,"<td id=td3>%d</td>",wqos->qos[0]->client_qos[i]->CWMin);
					  fprintf(cgiOut,"<td id=td3>%d</td>",wqos->qos[0]->client_qos[i]->CWMax);
					  fprintf(cgiOut,"<td id=td3>%d</td>",wqos->qos[0]->client_qos[i]->AIFS);
					  fprintf(cgiOut,"<td id=td3>%d</td>",wqos->qos[0]->client_qos[i]->TXOPlimit);
				  }
				  fprintf(cgiOut,"</tr>");	  
				  cl=!cl;
				} 
			  fprintf(cgiOut,"</table>"\
				  "</td>"\
				"</tr>"\
				"<tr align=left style=\"padding-top:20px\">"\
				  "<td id=thead2>%s</td>","WMM map information"\
				"</tr>"\
				"<tr>"\
				"<td align=left style=\"padding-left:20px\"><table frame=below rules=rows width=330 border=1>");
			  for(i=0;i<4;i++)
			  {
			    if((wqos)&&(wqos->qos[0])&&(wqos->qos[0]->radio_qos[i])&&(wqos->qos[0]->radio_qos[i]->mapstate == 1))
			    {
			      fprintf(cgiOut,"<tr align=left>");
				  if(i==0)
				  	fprintf(cgiOut,"<td id=td1 width=210>WMM &nbsp; %s &nbsp;&nbsp;&nbsp;&nbsp; map &nbsp; dot1p:</td>",qos_info[i]);
				  else
				    fprintf(cgiOut,"<td id=td1 width=210>WMM &nbsp; %s &nbsp; map &nbsp; dot1p:</td>",qos_info[i]);
				    fprintf(cgiOut,"<td id=td2 width=120>%d</td>",wqos->qos[0]->radio_qos[i]->wmm_map_dot1p);
			      fprintf(cgiOut,"</tr>");
			    }
			  }
			  fprintf(cgiOut,"</table>"\
				  "</td>"\
				"</tr>"\
			  "<tr align=left style=\"padding-top:20px\">"\
				  "<td id=thead2>%s</td>","Dotlp map information"\
				"</tr>"\
				"<tr>"\
				"<td align=left style=\"padding-left:20px\"><table frame=below rules=rows width=310 border=1>");
			  for(i=0;i<4;i++)
			  {
			    if((wqos)&&(wqos->qos[0])&&(wqos->qos[0]->radio_qos[i])&&(wqos->qos[0]->radio_qos[i]->dot1p_map_wmm_num!=0))
			    {
			      for(j=0;j<wqos->qos[0]->radio_qos[i]->dot1p_map_wmm_num;j++)
			      {
			        fprintf(cgiOut,"<tr align=left>"\
				      "<td id=td1>Dot1p &nbsp; %d &nbsp; map &nbsp; %s</td>",wqos->qos[0]->radio_qos[i]->dot1p_map_wmm[j],qos_info[i]);
			        fprintf(cgiOut,"</tr>");
			      }
			    }
			  }
			  fprintf(cgiOut,"</table>"\
				  "</td>"\
				"</tr>"\
			  "</table>");
			}
            else if(result==-1)
		      fprintf(cgiOut,"%s",search(lpublic,"unknown_id_format")); 	 
			else if(result==-2)
			{
	  		  memset(temp,0,sizeof(temp));
	  		  strncpy(temp,"WQOS ID",sizeof(temp)-1);
	  		  strncat(temp,search(lwlan,"wqos_id_illegal"),sizeof(temp)-strlen(temp)-1);
	 		  memset(wqos_num,0,sizeof(wqos_num));
			  snprintf(wqos_num,sizeof(wqos_num)-1,"%d",QOS_NUM-1);   /*int转成char*/
			  strncat(temp,wqos_num,sizeof(temp)-strlen(temp)-1);
			  strncat(temp,search(lwlan,"bss_id_2"),sizeof(temp)-strlen(temp)-1);
			  fprintf(cgiOut,"%s",temp); 
			}
			else if(result==-3)
		      fprintf(cgiOut,"%s",search(lwlan,"wqos_not_exist")); 
			else
			  fprintf(cgiOut,"%s",search(lpublic,"error"));
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
if(result==1)
{
	Free_qos_one(wqos);
}
free_instance_parameter_list(&paraHead1);
}


