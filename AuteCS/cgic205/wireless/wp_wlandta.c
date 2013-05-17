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
* wp_wlandta.c
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
#include "wcpss/asd/asd.h"
#include "wcpss/wid/WID.h"
#include "dbus/wcpss/dcli_wid_wtp.h"
#include "dbus/wcpss/dcli_wid_wlan.h"
#include "ws_dcli_wlans.h"
#include "ws_sta.h"
#include "ws_security.h"
#include "ws_public.h"
#include "ws_usrinfo.h"
#include "ws_err.h"
#include "ws_ec.h"
#include "ws_dbus_list_interface.h"

void ShowWlandtaPage(char *m,char *n,char *t,char *f,char *pn,char *ins_id,struct list *lpublic,struct list *lwlan,struct list *lsecu);  


int cgiMain()
{
  char encry[BUF_LEN] = { 0 };              
  char *str = NULL;      
  char ID[5] = { 0 };
  char flag[10] = { 0 };
  char pno[10] = { 0 }; 
  char instance_id[10] = { 0 };
  struct list *lpublic = NULL;   /*解析public.txt文件的链表头*/
  struct list *lwlan = NULL;     /*解析wlan.txt文件的链表头*/  
  struct list *lsecu = NULL;     /*解析security.txt文件的链表头*/  
  lpublic=get_chain_head("../htdocs/text/public.txt");
  lwlan=get_chain_head("../htdocs/text/wlan.txt");
  lsecu=get_chain_head("../htdocs/text/security.txt");
  
  DcliWInit();
  ccgi_dbus_init();
  memset(encry,0,sizeof(encry));
  memset(ID,0,sizeof(ID));
  memset(flag,0,sizeof(flag));
  memset(pno,0,sizeof(pno)); 
  memset(instance_id,0,sizeof(instance_id));
  cgiFormStringNoNewlines("INSTANCE_ID", instance_id, 10);
  if(cgiFormStringNoNewlines("UN", encry, BUF_LEN)!=cgiFormNotFound )  /*首次进入该页*/
  {
    cgiFormStringNoNewlines("ID", ID, 5);
	cgiFormStringNoNewlines("FL", flag, 10);
	cgiFormStringNoNewlines("PN",pno,10);
  }
  else
  {
    cgiFormStringNoNewlines("encry_wlandta",encry,BUF_LEN);
    cgiFormStringNoNewlines("wlan_id",ID,5);
	cgiFormStringNoNewlines("if_flag",flag,10);	
	cgiFormStringNoNewlines("page_no",pno,10);
  }  
  str=dcryption(encry);
  if(str==NULL)
	ShowErrorPage(search(lpublic,"ill_user"));		   /*用户非法*/
  else
	ShowWlandtaPage(encry,str,ID,flag,pno,instance_id,lpublic,lwlan,lsecu);
  release(lpublic);  
  release(lwlan);
  release(lsecu);
  destroy_ccgi_dbus();
  return 0;
}

void ShowWlandtaPage(char *m,char *n,char *t,char *f,char *pn,char *ins_id,struct list *lpublic,struct list *lwlan,struct list *lsecu)
{    
  DCLI_WLAN_API_GROUP *wlan = NULL;
  struct ifi *q=NULL;
  int inter_num = 0;             /*存放interface的个数*/
  char nas_id_str[NAS_IDENTIFIER_NAME + 1] = {0};
  char bindinter[100] = { 0 };
  char SecurityType[20] = { 0 };
  char EncryptionType[20] = { 0 };
  char whichinterface[WLAN_IF_NAME_LEN] = { 0 };
  char *endptr = NULL;  
  char alt[100] = { 0 };
  char max_wlan_num[10] = { 0 };
  int i = 0,j = 0,wlan_id = 0,retu = 1,result = 0,limit = 0;     
  int result1 = 0,black_num = 0,white_num = 0;
  int result2 = 0;
  DCLI_WLAN_API_GROUP *wlan_vlan = NULL;
  int result3 = 0,result4 = 0;
  DCLI_WLAN_API_GROUP *bridge_isolation = NULL;
  DCLI_WLAN_API_GROUP *tunnel_wlan_vlan = NULL;
  struct WID_TUNNEL_WLAN_VLAN *head = NULL;
  int twv_num = 0;  
  struct dcli_wlan_info *wlanz = NULL;
  char *maclist_name[3]={"none","black","white"};
  char if_name[20] = { 0 };
  int ret = -1;
  if_list_p interf;
  if3 *iq = NULL;
  instance_parameter *paraHead1 = NULL;
  dbus_parameter ins_para;

  cgiHeaderContentType("text/html");
  fprintf(cgiOut,"<html xmlns=\"http://www.w3.org/1999/xhtml\"><head>");
  fprintf(cgiOut,"<meta http-equiv=Content-Type content=text/html; charset=gb2312>");
  fprintf(cgiOut,"<title>Wtp</title>");
  fprintf(cgiOut,"<link rel=stylesheet href=/style.css type=text/css>");
  fprintf(cgiOut,"<style>"\
    ".iflis {overflow-x:hidden;	overflow:auto; width: 350; height: 105; clip: rect( ); padding-top: 0px; padding-right: 0px; padding-bottom: 0px; padding-left: 0px} "\
  "</style>"\
  "</head>"\
  "<body>"\
  "<form>"\
  "<div align=center>"\
  "<table width=976 border=0 cellpadding=0 cellspacing=0>"\
  "<tr>"\
    "<td width=8 align=left valign=top background=/images/di22.jpg><img src=/images/youce4.jpg width=8 height=30/></td>"\
    "<td width=51 align=left valign=bottom background=/images/di22.jpg><img src=/images/youce33.jpg width=37 height=24/></td>"\
    "<td width=153 align=left valign=bottom id=titleen background=/images/di22.jpg>WLAN</td>"\
    "<td width=690 align=right valign=bottom background=/images/di22.jpg>");        
	 
    	  fprintf(cgiOut,"<table width=155 border=0 cellspacing=0 cellpadding=0>"\
          "<tr>"\
     	  "<td width=62 align=center><a href=wp_wlanlis.cgi?UN=%s&PN=%s&INSTANCE_ID=%s target=mainFrame><img src=/images/%s border=0 width=62 height=20/></a></td>",m,pn,ins_id,search(lpublic,"img_ok"));
          fprintf(cgiOut,"<td width=62 align=center><a href=wp_wlanlis.cgi?UN=%s&PN=%s&INSTANCE_ID=%s target=mainFrame><img src=/images/%s border=0 width=62 height=20/></a></td>",m,pn,ins_id,search(lpublic,"img_cancel"));
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
                    "<td align=left id=tdleft background=/images/bottom_bg.gif style=\"border-right:0\"><font id=yingwen_san>WLAN</font><font id=%s> %s</font></td>",search(lpublic,"menu_san"),search(lpublic,"details"));   /*突出显示*/
                  fprintf(cgiOut,"</tr>");				  
				  retu=checkuser_group(n);		  
				  if(retu==0)  /*管理员*/
				  {
                    fprintf(cgiOut,"<tr height=25>"\
					  "<td align=left id=tdleft><a href=wp_wlannew.cgi?UN=%s target=mainFrame class=top><font id=%s>%s</font><font id=yingwen_san> WLAN</font></a></td>",m,search(lpublic,"menu_san"),search(lpublic,"create"));                       
                    fprintf(cgiOut,"</tr>");
					fprintf(cgiOut,"<tr height=25>"\
					  "<td align=left id=tdleft><a href=wp_wlanbw.cgi?UN=%s target=mainFrame class=top><font id=yingwen_san>MAC</font><font id=%s> %s</font></a></td>",m,search(lpublic,"menu_san"),search(lwlan,"mac_filter"));                       
                    fprintf(cgiOut,"</tr>");
				  }
				  
				  get_slotID_localID_instanceID(ins_id,&ins_para);	
				  get_instance_dbus_connection(ins_para, &paraHead1, INSTANCE_STATE_WEB);
				  
				  wlan_id= strtoul(t,&endptr,10);	/*char转成int，10代表十进制*/	  
				  if(paraHead1)
				  {
					  result=show_wlan_one(paraHead1->parameter,paraHead1->connection,wlan_id,&wlan);	 /*删除interface后重新显示wlan信息*/
				  }
				  if(result == 1)
				  {
				  	if((wlan != NULL)&&(wlan->WLAN[0] != NULL))
					{
						for(q = wlan->WLAN[0]->Wlan_Ifi; (NULL != q); q = q->ifi_next)
						{
							inter_num++;	
						}
					}
				  }

				  if(paraHead1)
				  {
					  result1=show_wlan_mac_list(paraHead1->parameter,paraHead1->connection,t,&wlanz);
				  }
				  if(result1 == 1)
				  {
				  	black_num = wlanz->acl_conf.num_deny_mac;
                    white_num = wlanz->acl_conf.num_accept_mac;
				  }		

				  if(paraHead1)
				  {
					  result2=show_wlan_vlan_info(paraHead1->parameter,paraHead1->connection,wlan_id,&wlan_vlan);
					  result3=wlan_show_bridge_isolation_func(paraHead1->parameter,paraHead1->connection,wlan_id,&bridge_isolation);
					  result4=show_tunnel_wlan_vlan_cmd_func(paraHead1->parameter,paraHead1->connection,wlan_id,&tunnel_wlan_vlan);
				  }
				  if(result4 == 1)
				  {
				  	if((tunnel_wlan_vlan)&&(tunnel_wlan_vlan->WLAN[0]))
					{
						for(head = tunnel_wlan_vlan->WLAN[0]->tunnel_wlan_vlan; (NULL != head); head = head->ifnext)
						{
							twv_num++;
						}
					}
				  }

				  if(inter_num>5)/*用div限制页面高度*/
				  {
					  if(retu==0)  /*管理员*/
						limit=20;
					  else
						limit=22;
				  }
				  else
				  {
					  if(retu==0)  /*管理员*/
						limit=15+inter_num;
					  else
						limit=17+inter_num;
				  }
				  if(result1==1)
				  	limit+=1;
				  if(strcmp(f,"NO_IF")==0)  /*local模式*/
				  {
					  if((result2==1)&&(wlan_vlan)&&(wlan_vlan->WLAN[0])&&(wlan_vlan->WLAN[0]->vlanid!=0))
						limit+=2;
				  }
				  else
				  {
				  	  if(result3==1)
					  	limit+=3;
					  if((result4==1)&&(twv_num>0))
					  	limit+=twv_num;
				  }

				  /*获取wlan三层接口信息*/
				  if((result == 1)&&(wlan != NULL)&&(wlan->WLAN[0] != NULL))
				  {
					  memset(if_name,0,sizeof(if_name));
					  if(paraHead1)
					  {
						  if(paraHead1->parameter.local_id == SNMPD_LOCAL_INSTANCE)
							snprintf(if_name,sizeof(if_name)-1,"wlan%d-%d",paraHead1->parameter.instance_id,wlan->WLAN[0]->WlanID);
						  else
							snprintf(if_name,sizeof(if_name)-1,"wlan%d-%d-%d",paraHead1->parameter.slot_id,paraHead1->parameter.instance_id,wlan->WLAN[0]->WlanID);
					  }
				  }
				  ret = get_all_if_info(&interf);
				  if((ret == 0)&&(interf.if_head))
				  {
				  	for(i = 0,iq = interf.if_head->next;
						((i<interf.if_num)&&(NULL != iq));
						i++,iq = iq->next)
				  	{
						if((strlen(if_name)==strlen(iq->ifname))&&(strcmp(if_name,iq->ifname) == 0))
						{
							limit++;
						}
				  	}
				  }
				  
				  for(i=0;i<limit;i++)
	              {
  				    fprintf(cgiOut,"<tr height=25>"\
                      "<td id=tdleft>&nbsp;</td>"\
                    "</tr>");
	              }
                fprintf(cgiOut,"</table>"\
              "</td>"\
              "<td align=left style=\"background-color:#ffffff; border-right:1px solid #707070; padding-left:30px; padding-top:10px\">"\
 "<table width=320 border=0 bgcolor=#ffffff cellspacing=0 cellpadding=0>"\
  "<tr valign=middle>"\
    "<td align=center>");
			if(result==1)	
			{	
				memset(SecurityType, 0, sizeof(SecurityType));
				memset(EncryptionType, 0, sizeof(EncryptionType));
				if((wlan)&&(wlan->WLAN[0]))
				{
					CheckSecurityType(SecurityType, wlan->WLAN[0]->SecurityType);
					CheckEncryptionType(EncryptionType, wlan->WLAN[0]->EncryptionType);
				}
			   fprintf(cgiOut,"<table width=350 border=0 cellspacing=0 cellpadding=0>"\
			   "<tr>"\
		         "<td id=ins_style>%s:%s</td>",search(lpublic,"instance"),ins_id);
		       fprintf(cgiOut,"</tr>"\
	           "<tr align=left height=10 valign=top>"\
	           "<td id=thead1>WLAN %s</td>",search(lpublic,"details"));
	           fprintf(cgiOut,"</tr>"\
               "<tr>"\
               "<td align=center style=\"padding-left:20px\">"\
			   "<table frame=below rules=rows width=350 border=1>"\
			   "<tr align=left>"\
			   "<td id=td1 width=170>%s</td>",search(lpublic,"name"));
			   if((wlan)&&(wlan->WLAN[0])&&(wlan->WLAN[0]->WlanName))
			   {
			   	   fprintf(cgiOut,"<td id=td2 width=180>%s</td>",wlan->WLAN[0]->WlanName);
			   }
			  fprintf(cgiOut,"</tr>"\
			  	"<tr align=left>"\
				  "<td id=td1>%s</td>",search(lwlan,"wlan_sta_max"));
			  	  if((wlan)&&(wlan->WLAN[0]))
			      {
					  fprintf(cgiOut,"<td id=td2>%u</td>",wlan->WLAN[0]->wlan_max_allowed_sta_num);
			      }
			  fprintf(cgiOut,"</tr>"\
			  	  	"<tr align=left>"\
				  "<td id=td1>%s</td>",search(lwlan,"balance_para_by_sta"));
			  	  if((wlan)&&(wlan->WLAN[0]))
			      {
					  fprintf(cgiOut,"<td id=td2>%d</td>",wlan->WLAN[0]->balance_para);
			      }
			  fprintf(cgiOut,"</tr>"\
			  	"<tr align=left>"\
				  "<td id=td1>%s</td>",search(lwlan,"balance_para_by_flow"));
			  	  if((wlan)&&(wlan->WLAN[0]))
			      {
					  fprintf(cgiOut,"<td id=td2>%d</td>",wlan->WLAN[0]->flow_balance_para);
			      }
			  fprintf(cgiOut,"</tr>"\
			  	  	"<tr align=left>"\
				  "<td id=td1>%s</td>",search(lwlan,"balance_wlan"));
			  if((wlan)&&(wlan->WLAN[0])&&(wlan->WLAN[0]->balance_switch==1))
			  {
				fprintf(cgiOut,"<td id=td2>enable</td>");
			  } 
			  else
			  {
				fprintf(cgiOut,"<td id=td2>disable</td>");
			  }
			  fprintf(cgiOut,"</tr>");
			  if((wlan)&&(wlan->WLAN[0])&&(wlan->WLAN[0]->balance_switch==1))
			  {
			    fprintf(cgiOut,"<tr align=left>"\
				  "<td id=td1>%s</td>",search(lwlan,"balance_method"));
				if((wlan)&&(wlan->WLAN[0])&&(wlan->WLAN[0]->balance_method==1))
			      fprintf(cgiOut,"<td id=td2>number</td>");
				else
				  fprintf(cgiOut,"<td id=td2>flow</td>");
			  fprintf(cgiOut,"</tr>"); 
			  }
				fprintf(cgiOut,"<tr align=left>"\
				  "<td id=td1>ID</td>");
				  if((wlan)&&(wlan->WLAN[0]))
				  {
					  fprintf(cgiOut,"<td id=td2>%d</td>",wlan->WLAN[0]->WlanID);
				  }
			  fprintf(cgiOut,"</tr>"\
				"<tr align=left>"\
				  "<td id=td1>ESSID</td>");
			  	  if((wlan)&&(wlan->WLAN[0])&&(wlan->WLAN[0]->ESSID))
				  {
					  fprintf(cgiOut,"<td id=td2>%s</td>",wlan->WLAN[0]->ESSID);
				  }
			  fprintf(cgiOut,"</tr>"\
			  	"<tr align=left>"\
				  "<td id=td1>%sESSID</td>",search(lwlan,"hidden"));
			      if((wlan)&&(wlan->WLAN[0])&&(wlan->WLAN[0]->HideESSid==1))
				    fprintf(cgiOut,"<td id=td2>yes</td>");
				  else
				  	fprintf(cgiOut,"<td id=td2>no</td>");
			  fprintf(cgiOut,"</tr>"\
				"<tr align=left>"\
				"<td id=td1>%s</td>",search(lwlan,"status"));	  
			  if((wlan)&&(wlan->WLAN[0])&&(wlan->WLAN[0]->Status==1))
			    fprintf(cgiOut,"<td id=td2>disable</td>");
			  else
			  	fprintf(cgiOut,"<td id=td2>enable</td>");
			  fprintf(cgiOut,"</tr>"\
				"<tr align=left>"\
				  "<td id=td1>%s ID</td>",search(lpublic,"security"));
			  if((wlan)&&(wlan->WLAN[0]))
			  {
				  if(wlan->WLAN[0]->SecurityID==0)
				  {
					fprintf(cgiOut,"<td id=td2>%s</td>","NONE");
				  }
				  else
				  {
					fprintf(cgiOut,"<td id=td2>%d</td>",wlan->WLAN[0]->SecurityID);
				  }
			  }
			  fprintf(cgiOut,"</tr>"\
			    "<tr align=left>"\
				  "<td id=td1>%s</td>",search(lsecu,"secur_type"));
				  fprintf(cgiOut,"<td id=td2>%s</td>",SecurityType);
			  fprintf(cgiOut,"</tr>"\
				"<tr align=left>"\
				  "<td id=td1>%s</td>",search(lsecu,"encry_type"));
			      fprintf(cgiOut,"<td id=td2>%s</td>",EncryptionType);			  
			  fprintf(cgiOut,"</tr>"\
			  	"<tr align=left>");
			  	  if((wlan)&&(wlan->WLAN[0]))
			      {
					  fprintf(cgiOut,"<td id=td1>%s(%s)</td>",search(lsecu,"key"),(wlan->WLAN[0]->asic_hex==0)?"ASCII":((wlan->WLAN[0]->asic_hex==1)?"HEX":"Unknown"));
					  fprintf(cgiOut,"<td id=td2>%s</td>",wlan->WLAN[0]->WlanKey);			  
			      }
			  fprintf(cgiOut,"</tr>"\
			  	"<tr align=left>"\
				  "<td id=td1>SecurityIndex</td>");
			  	  if((wlan)&&(wlan->WLAN[0]))
			      {
					  fprintf(cgiOut,"<td id=td2>%d</td>",wlan->WLAN[0]->SecurityIndex);			  
			      }
			  fprintf(cgiOut,"</tr>"\
			  	"<tr align=left>"\
				  "<td id=td1>%s</td>",search(lwlan,"if_policy"));
			      memset(whichinterface,0,sizeof(whichinterface));
				  if((wlan)&&(wlan->WLAN[0]))
			      {
					  CheckWIDIfPolicy(whichinterface,wlan->WLAN[0]->wlan_if_policy);
			      }
			  fprintf(cgiOut,"<td id=td2>%s</td>",whichinterface);
			  fprintf(cgiOut,"</tr>");
			  if(strcmp(if_name,""))
			  {
				  j=0;
				  if((ret == 0)&&(interf.if_head))
				  {
					for(i = 0,iq = interf.if_head->next;
						((i<interf.if_num)&&(NULL != iq));
						i++,iq = iq->next)
					{
						if((strlen(if_name)==strlen(iq->ifname))&&(strcmp(if_name,iq->ifname) == 0))
						{
							if(j == 0)
							{
								fprintf(cgiOut,"<tr align=left>"\
								  "<td id=td1>%s</td>",search(lpublic,"l3_interface_name_ip_mask"));
								fprintf(cgiOut,"<td id=td2>%s: %s/%d</td>",if_name,iq->ipaddr,iq->mask);
								fprintf(cgiOut,"</tr>");
							}
							else
							{
								fprintf(cgiOut,"<tr align=left>"\
								  "<td id=td1>&nbsp;</td>"\
								  "<td id=td2>%s: %s/%d</td>",if_name,iq->ipaddr,iq->mask);
								fprintf(cgiOut,"</tr>");
							}
							j++;
						}
					}
				  }
			  }
			  if(inter_num>0)
			  {
				  if((wlan != NULL)&&(wlan->WLAN[0] != NULL))
				  {
				  	if(inter_num>5)
				    {
					  fprintf(cgiOut,"<tr align=left><td colspan=2>"\
						"<div class=iflis><table frame=below rules=rows width=350 border=1>");
				    }	
				  	i = 1;
					for(q = wlan->WLAN[0]->Wlan_Ifi; (NULL != q); q=q->ifi_next)
					{
						memset(nas_id_str, 0, NAS_IDENTIFIER_NAME + 1);
						memcpy(nas_id_str, q->nas_id, NAS_IDENTIFIER_NAME);
					
						memset(bindinter,0,sizeof(bindinter));
						strncat(bindinter,q->ifi_name,sizeof(bindinter)-strlen(bindinter)-1);
						strncat(bindinter,"(NASID:",sizeof(bindinter)-strlen(bindinter)-1);
						strncat(bindinter,nas_id_str,sizeof(bindinter)-strlen(bindinter)-1);
						strncat(bindinter,")",sizeof(bindinter)-strlen(bindinter)-1);	

						fprintf(cgiOut,"<tr align=left>");
						if(1 == i)
						{
							  fprintf(cgiOut,"<td id=td1 width=170>%s</td>",search(lwlan,"apply_interface"));
						}
						else
						{
							  fprintf(cgiOut,"<td id=td1 width=170>&nbsp;</td>");
						}
						  fprintf(cgiOut,"<td id=td2 width=180>%s</td>",bindinter);					
						fprintf(cgiOut,"</tr>");
						i++;
					}
					if(inter_num>5)
				    {
					  fprintf(cgiOut,"</table></div></td></tr>");
				    }
				  }
				  else
				  {				  	
					fprintf(cgiOut,"<tr align=left>"\
					  "<td id=td1>%s</td>",search(lwlan,"apply_interface"));
					fprintf(cgiOut,"<td id=td2>%s (nas_id :%s)</td>","NONE","NONE"\
					"</tr>");
				  }
			  }
			  if(result1==1)
			  {
			    fprintf(cgiOut,"<tr align=left>"\
				  "<td id=td1>%s</td>",search(lwlan,"mac_filter_type"));			      
			      fprintf(cgiOut,"<td id=td2>%s</td>",maclist_name[wlanz->acl_conf.macaddr_acl]);
			  fprintf(cgiOut,"</tr>");
			  }
			  if(strcmp(f,"NO_IF")==0)  /*local模式*/
			  {
				  if((result2==1)&&(wlan_vlan)&&(wlan_vlan->WLAN[0])&&(wlan_vlan->WLAN[0]->vlanid!=0))
				  {
					fprintf(cgiOut,"<tr align=left>"\
					  "<td id=td1>WLAN_VLAN%s</td>",search(lpublic,"map")); 			  
					  fprintf(cgiOut,"<td id=td2>%d</td>",wlan_vlan->WLAN[0]->vlanid);
					fprintf(cgiOut,"</tr>");
					fprintf(cgiOut,"<tr align=left>"\
					  "<td id=td1>WLAN_VLAN %s</td>",search(lpublic,"priority"));				  
					  fprintf(cgiOut,"<td id=td2>%d</td>",wlan_vlan->WLAN[0]->wlan_1p_priority);
					fprintf(cgiOut,"</tr>");
				  }
			  }
			  else
			  {
			  	  if(result3==1)
			  	  {
					fprintf(cgiOut,"<tr align=left>"\
					  "<td id=td1>WLAN %s</td>",search(lwlan,"isolation")); 
					  if((bridge_isolation)&&(bridge_isolation->WLAN[0])&&(bridge_isolation->WLAN[0]->isolation_policy==0))
					    fprintf(cgiOut,"<td id=td2>disable</td>");
					  else
					  	fprintf(cgiOut,"<td id=td2>enable</td>");
					fprintf(cgiOut,"</tr>");
					fprintf(cgiOut,"<tr align=left>"\
					  "<td id=td1>WLAN %s</td>",search(lwlan,"multicast"));				  
					  if((bridge_isolation)&&(bridge_isolation->WLAN[0])&&(bridge_isolation->WLAN[0]->multicast_isolation_policy==0))
					  	fprintf(cgiOut,"<td id=td2>disable</td>");
					  else
					  	fprintf(cgiOut,"<td id=td2>enable</td>");
					fprintf(cgiOut,"</tr>");	
					fprintf(cgiOut,"<tr align=left>"\
					  "<td id=td1>%s</td>",search(lwlan,"spswitch"));				  
					  if((bridge_isolation)&&(bridge_isolation->WLAN[0])&&(bridge_isolation->WLAN[0]->sameportswitch==0))
					  	fprintf(cgiOut,"<td id=td2>disable</td>");
					  else
					  	fprintf(cgiOut,"<td id=td2>enable</td>");
					fprintf(cgiOut,"</tr>");	
				  }
				  if((result4==1)&&(twv_num>0))
				  {
				  	  if((tunnel_wlan_vlan)&&(tunnel_wlan_vlan->WLAN[0]))
					  {
						i=0;
						for(head = tunnel_wlan_vlan->WLAN[0]->tunnel_wlan_vlan; (NULL != head); head = head->ifnext)
						{
							fprintf(cgiOut,"<tr align=left>");
							if(i==0)
							  fprintf(cgiOut,"<td id=td1>WLAN_VLAN %s</td>",search(lwlan,"interface")); 
							else
							  fprintf(cgiOut,"<td id=td1></td>");
							fprintf(cgiOut,"<td id=td2>%s</td>",head->ifname);
							fprintf(cgiOut,"</tr>");
							i++;
						}
					  }
				  }
			  }
			  fprintf(cgiOut,"</tr>"\
				"<tr align=left>"\
				  "<td id=td1>%s</td>",search(lwlan,"roam_switch")); 
			      if((wlan)&&(wlan->WLAN[0])&&(wlan->WLAN[0]->Roaming_Policy==0))
					fprintf(cgiOut,"<td id=td2>disable</td>");
				  else
					fprintf(cgiOut,"<td id=td2>enable</td>");
			  fprintf(cgiOut,"</tr>"\
			  "<tr align=left>"\
				  "<td id=td1>WLAN%s</td>",search(lpublic,"uplink_traffic_limit_threshold"));
			  		if((wlan)&&(wlan->WLAN[0]))
			  		{
						fprintf(cgiOut,"<td id=td2>%d</td>",wlan->WLAN[0]->wlan_traffic_limit);
			  		}
			  fprintf(cgiOut,"</tr>"\
			  "<tr align=left>"\
				  "<td id=td1>WLAN%s</td>",search(lpublic,"downlink_traffic_limit_threshold"));
			  	  if((wlan)&&(wlan->WLAN[0]))
		  		  {
					  fprintf(cgiOut,"<td id=td2>%d</td>",wlan->WLAN[0]->wlan_send_traffic_limit);
		  		  }
			  fprintf(cgiOut,"</tr>"\
			    "<tr align=left>");
			  	  if(1 == get_product_info("/var/run/mesh_flag"))
				  	fprintf(cgiOut,"<td id=td1>Mesh</td>"); 
				  else
					fprintf(cgiOut,"<td id=td1>WDS</td>"); 
			      if((wlan)&&(wlan->WLAN[0])&&(wlan->WLAN[0]->WDSStat==1))
				  	fprintf(cgiOut,"<td id=td2>enable</td>");
				  else
				  	fprintf(cgiOut,"<td id=td2>disable</td>");
			  fprintf(cgiOut,"</tr>"\
			  "</table></td>"\
			  "</tr>"\
			  "<tr>"\
			    "<td><input type=hidden name=encry_wlandta value=%s></td>",m);
			  fprintf(cgiOut,"</tr>"\
			  "<tr>"\
			    "<td><input type=hidden name=wlan_id value=%s></td>",t);
			  fprintf(cgiOut,"</tr>"\
			  "<tr>"\
			    "<td><input type=hidden name=if_flag value=%s></td>",f);
			  fprintf(cgiOut,"</tr>"\
			  "<tr>"\
                "<td><input type=hidden name=page_no value=%s></td>",pn);
	          fprintf(cgiOut,"</tr>"\
			  "</table>");
			}
            else if(result==-1)
            {
            	memset(alt,0,sizeof(alt));
				strncpy(alt,search(lwlan,"wlan_id_illegal1"),sizeof(alt)-1);
				memset(max_wlan_num,0,sizeof(max_wlan_num));
				snprintf(max_wlan_num,sizeof(max_wlan_num)-1,"%d",WLAN_NUM-1);
				strncat(alt,max_wlan_num,sizeof(alt)-strlen(alt)-1);
				strncat(alt,search(lwlan,"wlan_id_illegal2"),sizeof(alt)-strlen(alt)-1);
		  	    fprintf(cgiOut,"%s",alt);	 
            }
			else if(result==-2)
				fprintf(cgiOut,"%s",search(lwlan,"wlan_not_exist"));   
			else if(result==-3)
				fprintf(cgiOut,"%s",search(lpublic,"error"));   
			else
			    fprintf(cgiOut,"%s",search(lpublic,"contact_adm"));	
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
if(ret == 0)
{
	Free_get_all_if_info(&interf);
}
if(result==1)
{
  Free_one_wlan_head(wlan);
}
if(result1==1)
{
  Free_sta_bywlanid(wlanz);  
}
if(result2==1)
{
  Free_wlan_vlan_info(wlan_vlan);
}
if(result3==1)
{
  Free_bridge_isolation(bridge_isolation);
}
if(result4==1)
{
  Free_tunnel_wlan_vlan_head(tunnel_wlan_vlan);
}
free_instance_parameter_list(&paraHead1);
}


