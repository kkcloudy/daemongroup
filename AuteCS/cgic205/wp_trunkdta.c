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
* wp_trunkdta.c
*
*
* CREATOR:
* autelan.software.Network Dep. team
* zhouym@autelan.com
*
* DESCRIPTION:
* system contrl for trunk config
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

void ShowTrunkdtaPage(char *m,char *n,char *t,struct list *lpublic,struct list *lcon);  

int cgiMain()
{
  char *encry=(char *)malloc(BUF_LEN); 
  char *ID=(char *)malloc(10);
  char *str;          
  struct list *lpublic;   /*解析public.txt文件的链表头*/
  struct list *lcon;     /*解析wlan.txt文件的链表头*/  
  lpublic=get_chain_head("../htdocs/text/public.txt");
  lcon=get_chain_head("../htdocs/text/control.txt");
  memset(encry,0,BUF_LEN);
   memset(ID,0,10);
  if(cgiFormStringNoNewlines("UN", encry, BUF_LEN)!=cgiFormNotFound )  /*首次进入该页*/
  {    
	cgiFormStringNoNewlines("ID", ID, 10); 
   }
  else
  {
    cgiFormStringNoNewlines("encry_trunkdta",encry,BUF_LEN);
	cgiFormStringNoNewlines("trunkid",ID,10);
   }  
  str=dcryption(encry);
  if(str==NULL)
    ShowErrorPage(search(lpublic,"ill_user"));		 /*用户非法*/
  else
    ShowTrunkdtaPage(encry,str,ID,lpublic,lcon);
  free(encry);  
   free(ID);
  release(lpublic);  
  release(lcon);
  return 0;
}

void ShowTrunkdtaPage(char *m,char *n,char *t,struct list *lpublic,struct list *lcon)
{  
  FILE *fp;
  char lan[3],pno[10];  
  struct trunk_profile trunk; 
  struct port_profile *pq;
  struct trunk_profile vhead; 
  struct vlan_profile *vq;
  char *endptr = NULL;  
  int i,trunk_id,cl=1,retu,result1,result2,limit;                 /*颜色初值为#f9fafe*/
  unsigned int  tmpVal[2];
  memset(&tmpVal,0,sizeof(tmpVal));
  trunk.port_Cont=0;
  trunk.trunkId=0;
  trunk.mSlotNo=0;
  trunk.mPortNo=0;
  vhead.trunkId=0;
  vhead.mSlotNo=0;
  vhead.mPortNo=0;
  cgiHeaderContentType("text/html");
  fprintf(cgiOut,"<html xmlns=\"http://www.w3.org/1999/xhtml\"><head>");
  fprintf(cgiOut,"<meta http-equiv=Content-Type content=text/html; charset=gb2312>");
  fprintf(cgiOut,"<title>Wtp</title>");
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
    "<td width=153 align=left valign=bottom id=%s background=/images/di22.jpg>%s</td>",search(lpublic,"title_style"),search(lcon,"trunk_manage"));
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
		memset(pno,0,10);
		cgiFormStringNoNewlines("PN",pno,10);
	
	  fprintf(cgiOut,"<table width=155 border=0 cellspacing=0 cellpadding=0>"\
      "<tr>"\
	  "<td width=62 align=center><a href=wp_trunklis.cgi?UN=%s&PN=%s target=mainFrame><img src=/images/%s border=0 width=62 height=20/></a></td>",m ,pno,search(lpublic,"img_ok"));
	  fprintf(cgiOut,"<td width=62 align=center><a href=wp_trunklis.cgi?UN=%s&PN=%s target=mainFrame><img src=/images/%s border=0 width=62 height=20/></a></td>",m ,pno,search(lpublic,"img_cancel"));
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
                    "<td align=left id=tdleft background=/images/bottom_bg.gif style=\"border-right:0\"><font id=%s>%s<font></td>",search(lpublic,"menu_san"),search(lcon,"trunk_det"));   /*突出显示*/
                  fprintf(cgiOut,"</tr>");
				  if(retu==0) /*管理员*/
				  {
                    fprintf(cgiOut,"<tr height=25>"\
					  "<td align=left id=tdleft><a href=wp_trunknew.cgi?UN=%s target=mainFrame class=top><font id=%s>%s<font></a></td>",m ,search(lpublic,"menu_san"),search(lcon,"create_trunk"));                       
                    fprintf(cgiOut,"</tr>");
				  }				  
				  trunk_id= strtoul(t,&endptr,10);	/*char转成int，10代表十进制*/				  
				  ccgi_dbus_init();
				  result1=show_trunk_byid(trunk_id,&trunk); 
				  result2=show_trunk_vlanlist(trunk_id,&vhead);
				  if(vhead.vlan_Cont<trunk.port_Cont)
				    limit=6+trunk.port_Cont;
				  else
				  	limit=6+vhead.vlan_Cont;
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
			if(result1==1)	
			{	
			   fprintf(cgiOut,"<table width=500 border=0 cellspacing=0 cellpadding=0>"\
	           "<tr align=left height=20 valign=top>"\
	           "<td id=thead1 colspan=2>Trunk %s</td>",search(lpublic,"details"));
	           fprintf(cgiOut,"</tr>"\
               "<tr>"\
               "<td colspan=2 align=left style=\"padding-left:40px\">"\
			   "<table frame=below rules=rows width=320 border=1>"\
			   "<tr align=left>"\
				   "<td id=td1 width=170>TRUNK ID</td>"\
				   "<td id=td2 width=150>%d</td>",trunk.trunkId);
			   fprintf(cgiOut,"</tr>"\
			   "<tr align=left>"\
			       "<td id=td1>TRUNK %s</td>",search(lpublic,"name"));
			       fprintf(cgiOut,"<td id=td2>%s</td>",trunk.trunkName);
			   fprintf(cgiOut,"</tr>"\
			   "<tr align=left>"\
			       "<td id=td1>%s</td>",search(lcon,"load_balance"));
			       fprintf(cgiOut,"<td id=td2>%s</td>",trunk.loadBalanc);
			   fprintf(cgiOut,"</tr>"\
				 "<tr align=left>"\
				   "<td id=td1>%s%s</td>",search(lpublic,"master"),search(lpublic,"port"));
			       if(trunk.masterFlag!=0)
				     fprintf(cgiOut,"<td id=td2>%d/%d</td>",trunk.mSlotNo,trunk.mPortNo);
				   else
                     fprintf(cgiOut,"<td id=td2>No masterPort</td>");
			   fprintf(cgiOut,"</tr>"\
			   "</table></td>"\
				"</tr>"\
				"<tr align=left style=\"padding-top:20px\">");			  
  			    if(strcmp(lan,"ch")==0)
  			    {
				  fprintf(cgiOut,"<td id=thead2>Trunk%d的端口成员列表</td>",trunk_id);
				  fprintf(cgiOut,"<td id=thead2>Trunk%d的VLAN成员列表</td>",trunk_id);
  			    }
				else
				{
				  fprintf(cgiOut,"<td id=thead2>Port member list of Trunk%d</td>",trunk_id);
				  fprintf(cgiOut,"<td id=thead2>VLAN member list of Trunk%d</td>",trunk_id);
				}
				fprintf(cgiOut,"</tr>"\
				"<tr>"\
				"<td width=220 align=left valign=top style=\"padding-left:20px; padding-top:10px\"><table align=left frame=below rules=rows width=60 border=1>"\
				"<tr align=left height=20>"\
				  "<th width=60>%s</th>",search(lcon,"port_no"));
				fprintf(cgiOut,"</tr>");
				if(trunk.masterFlag!=0)
                {
                  pq=trunk.portHead->next;
	              for(i=0;i<64;i++)
	              {
	                tmpVal[i/32] = (1<<(i%32));
	                if((trunk.mbrBmp_sp.portMbr[i/32]) & tmpVal[i/32])
	                {
                      fprintf(cgiOut,"<tr bgcolor=%s>",setclour(cl));
				        fprintf(cgiOut,"<td id=td3>%d/%d(e)</td>",pq->slot,pq->port);
				      fprintf(cgiOut,"</tr>");	 
	                  pq=pq->next;
				      cl=!cl;
	                }
	              }

                  memset(&tmpVal,0,sizeof(tmpVal));
				  for(i=0;i<64;i++)
	              {
	                tmpVal[i/32] = (1<<(i%32));
	               //if((trunk.disMbrBmp_sp) & tmpVal) 
					if((trunk.disMbrBmp_sp.portMbr[i/32]) & tmpVal[i/32])
	                {
                      fprintf(cgiOut,"<tr bgcolor=%s>",setclour(cl));
				        fprintf(cgiOut,"<td id=td3>%d/%d(d)</td>",pq->slot,pq->port);
				      fprintf(cgiOut,"</tr>");	 
	                  pq=pq->next;
				      cl=!cl;
	                }
	              }
                }
			  fprintf(cgiOut,"</table>"\
				  "</td>"\
				  "<td width=280 align=left valign=top style=\"padding-left:20px; padding-top:10px\">");
			  if(result2==1)
			  {
			    fprintf(cgiOut,"<table align=left frame=below rules=rows width=200 border=1>"\
				"<tr align=left height=20>"\
				  "<th width=100>Tag %s</th>",search(lpublic,"mode"));
				  fprintf(cgiOut,"<th width=80>VLAN ID</th>"\
			      "<th width=120>VLAN %s</th>",search(lpublic,"name"));
				fprintf(cgiOut,"</tr>");
				cl=1;
				vq=vhead.vlanHead->next;
				for(i=0;i<vhead.vlan_Cont;i++)
				{				  
				  fprintf(cgiOut,"<tr bgcolor=%s>",setclour(cl));
				    if(vq->tagMode==1)
					  fprintf(cgiOut,"<td id=td3>tag</td>");
					else
					  fprintf(cgiOut,"<td id=td3>untag</td>");
				    fprintf(cgiOut,"<td id=td3>%d</td>",vq->vlanId);
				    fprintf(cgiOut,"<td id=td3>%s</td>",vq->vlanName);
  			  	  fprintf(cgiOut,"</tr>");	  
				  cl=!cl;
				  vq=vq->next;
				} 
			    fprintf(cgiOut,"</table>");
			  }
			  else if(result2 == 0)
                fprintf(cgiOut,"%s",search(lpublic,"contact_adm"));	   
			  else if(result2 == -1)
			    fprintf(cgiOut,"%s",search(lcon,"trunk_id_illegal"));	
			  else if(result2 == -2)
			    fprintf(cgiOut,"%s",search(lcon,"trunk_not_exist"));	
			  else 
			    fprintf(cgiOut,"%s",search(lpublic,"error"));	   			
			  fprintf(cgiOut,"</td>"\
				"</tr>"\
				"<tr>"\
			      "<td><input type=hidden name=encry_trunkdta value=%s></td>",m);
			      fprintf(cgiOut,"<td><input type=hidden name=trunkid value=%s></td>",t);
			    fprintf(cgiOut,"</tr>"\
			  "</table>");
			}
            else if(result1 == 0)
              fprintf(cgiOut,"%s",search(lpublic,"contact_adm"));	   
			else if(result1 == -1)
			  fprintf(cgiOut,"%s",search(lcon,"trunk_id_illegal"));	
			else if(result1 == -2)
			  fprintf(cgiOut,"%s",search(lcon,"trunk_not_exist"));	
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
if(result1==1)
  Free_trunk_one(&trunk);
if(result2==1)
  Free_vlanlist_trunk_head(&vhead);
}

