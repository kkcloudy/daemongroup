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
* wp_ebrdta.c
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
#include "ws_dcli_ebr.h"
#include "ws_dcli_wlans.h"
#include "ws_public.h"
#include "ws_usrinfo.h"
#include "ws_err.h"
#include "ws_ec.h"
#include "ws_dbus_list_interface.h"
#include "ws_init_dbus.h"
#include <sys/wait.h>

void ShowEbrdtaPage(char *m,char *n,char *instance_id,char *t,struct list *lpublic,struct list *lwlan);  
void DownLinkIf(instance_parameter *ins_para,int id,struct list *lpublic,struct list *lwlan);
void DeleteUplinkIf(instance_parameter *ins_para,int id,struct list *lpublic,struct list *lwlan);
void DeleteIfIP(instance_parameter *ins_para,char *id,char *ipaddr,struct list *lpublic);


int cgiMain()
{
  char encry[BUF_LEN] = { 0 };     
  char ID[10] = { 0 };
  char *str = NULL;    
  char instance_id[10] = { 0 };
  struct list *lpublic = NULL;   /*解析public.txt文件的链表头*/
  struct list *lwlan = NULL;     /*解析wlan.txt文件的链表头*/  
  lpublic=get_chain_head("../htdocs/text/public.txt");
  lwlan=get_chain_head("../htdocs/text/wlan.txt");
  
  DcliWInit();
  ccgi_dbus_init();
  memset(encry,0,sizeof(encry));
  memset(ID,0,sizeof(ID));
  if(cgiFormStringNoNewlines("UN", encry, BUF_LEN)!=cgiFormNotFound )  /*首次进入该页*/
  {    
	cgiFormStringNoNewlines("ID", ID, 10); 
	cgiFormStringNoNewlines("INSTANCE_ID", instance_id, 10);
  }
  else
  {
    cgiFormStringNoNewlines("encry_ebrdta",encry,BUF_LEN);
	cgiFormStringNoNewlines("ebrid",ID,10);
	cgiFormStringNoNewlines("instance_id", instance_id, 10);
  }  
  str=dcryption(encry);
  if(str==NULL)
    ShowErrorPage(search(lpublic,"ill_user"));		 /*用户非法*/
  else
    ShowEbrdtaPage(encry,str,instance_id,ID,lpublic,lwlan);

  release(lpublic);  
  release(lwlan);
  destroy_ccgi_dbus();
  return 0;
}

void ShowEbrdtaPage(char *m,char *n,char *instance_id,char *t,struct list *lpublic,struct list *lwlan)
{  
  char pno[10] = { 0 };  
  EBR_IF_LIST *head = NULL;
  char *endptr = NULL;  
  int inter_num = 0,ebr_id = 0,i = 0,j = 0,retu = 1,result = 0,limit = 0;                 /*颜色初值为#f9fafe*/
  char temp[100] = { 0 };
  char eid[10] = { 0 };  
  DCLI_EBR_API_GROUP *ebrinfo = NULL; 
  char IsSubmit[5] = { 0 };
  int uplink_if_num = 0;
  char if_name[20] = { 0 };
  int ret = -1;
  if_list_p interf;
  if3 *q = NULL;
  instance_parameter *paraHead1 = NULL;
  dbus_parameter ins_para;
  char menu_id[10] = { 0 };
  char menu[15] = { 0 };
  char IsDeleeteIP[10] = { 0 };
  char ipaddr[20] = { 0 };
  cgiHeaderContentType("text/html");
  fprintf(cgiOut,"<html xmlns=\"http://www.w3.org/1999/xhtml\"><head>");
  fprintf(cgiOut,"<meta http-equiv=Content-Type content=text/html; charset=gb2312>");
  fprintf(cgiOut,"<title>EBR</title>");
  fprintf(cgiOut,"<link rel=stylesheet href=/style.css type=text/css>");
  fprintf(cgiOut,"<style>"\
  	"th{ font-family:Arial, Helvetica, sans-serif; font-weight:bold; font-size:12px; color:#0a4e70}"\
    ".iflis1 {overflow-x:hidden;	overflow:auto; width: 310; height: 260; clip: rect( ); padding-top: 0px; padding-right: 0px; padding-bottom: 0px; padding-left: 0px} "\
    ".iflis2 {overflow-x:hidden;	overflow:auto; width: 310; height: 210; clip: rect( ); padding-top: 0px; padding-right: 0px; padding-bottom: 0px; padding-left: 0px} "\
    "#div1{ width:52px; height:18px; border:1px solid #666666; background-color:#f9f8f7;}"\
	"#div2{ width:50px; height:15px; padding-left:5px; padding-top:3px}"\
  "</style>"\
  "</head>"\
  "<script type=\"text/javascript\">"\
   "function checkAll(e, itemName)"\
   "{"\
	 "var aa = document.getElementsByName(itemName);"\
	 "for (var x=0; x<aa.length; x++)"\
	   "aa[x].checked = e.checked;"\
   "}"\
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
  "<body>"); 

  get_slotID_localID_instanceID(instance_id,&ins_para);	
  get_instance_dbus_connection(ins_para, &paraHead1, INSTANCE_STATE_WEB);

  ebr_id= strtoul(t,&endptr,10);	/*char转成int，10代表十进制*/
  retu=checkuser_group(n);
  
  memset(IsSubmit,0,sizeof(IsSubmit));  
  cgiFormStringNoNewlines("SubmitFlag", IsSubmit, 5);
  if((cgiFormSubmitClicked("DownlinkIf_apply") == cgiFormSuccess)&&(strcmp(IsSubmit,"")))
  {
  	if(paraHead1)
	{
		DownLinkIf(paraHead1,ebr_id,lpublic,lwlan);
	} 
  }
  if((cgiFormSubmitClicked("delete_uplinkIf_apply") == cgiFormSuccess)&&(strcmp(IsSubmit,"")))
  {
  	if(paraHead1)
	{
		DeleteUplinkIf(paraHead1,ebr_id,lpublic,lwlan);
	} 
  }

  if(paraHead1)
  {
	  result=show_ethereal_bridge_one(paraHead1->parameter,paraHead1->connection,t,&ebrinfo);
  } 
  if(result == 1)
  {
    inter_num = 0;
    if((ebrinfo)&&(ebrinfo->EBR[0])&&(ebrinfo->EBR[0]->iflist))
	{	
		head = ebrinfo->EBR[0]->iflist;
		while(head != NULL)
		{
			inter_num++;
			head = head->ifnext;
		}
	}

	uplink_if_num=0;
	if((ebrinfo)&&(ebrinfo->EBR[0])&&(ebrinfo->EBR[0]->uplinklist))
	{
		head = ebrinfo->EBR[0]->uplinklist;
		while(head != NULL)
		{
			uplink_if_num++;
			head = head->ifnext;
		}
	}
  }
  else
  {
	  inter_num = 0;  
	  uplink_if_num=0;
  }

  memset(IsDeleeteIP,0,sizeof(IsDeleeteIP));
  cgiFormStringNoNewlines("DeletEbrIP", IsDeleeteIP, sizeof(IsDeleeteIP));
  if(strcmp(IsDeleeteIP,"true")==0)
  {
  	memset(ipaddr,0,sizeof(ipaddr));
	cgiFormStringNoNewlines("IPADDR", ipaddr, sizeof(ipaddr));
	if(paraHead1)
	{
		DeleteIfIP(paraHead1,t,ipaddr,lpublic);
	}
  }
  fprintf(cgiOut,"<form>"\
  "<div align=center>"\
  "<table width=976 border=0 cellpadding=0 cellspacing=0>"\
  "<tr>"\
    "<td width=8 align=left valign=top background=/images/di22.jpg><img src=/images/youce4.jpg width=8 height=30/></td>"\
    "<td width=51 align=left valign=bottom background=/images/di22.jpg><img src=/images/youce33.jpg width=37 height=24/></td>"\
    "<td width=153 align=left valign=bottom background=/images/di22.jpg><font id=titleen>EBR</font><font id=%s> %s</font></td>",search(lpublic,"title_style"),search(lpublic,"management"));
    fprintf(cgiOut,"<td width=690 align=right valign=bottom background=/images/di22.jpg>");
		memset(pno,0,sizeof(pno));
		cgiFormStringNoNewlines("PN",pno,10);
	  
    	  fprintf(cgiOut,"<table width=155 border=0 cellspacing=0 cellpadding=0>"\
          "<tr>"\
		  "<td width=62 align=center><a href=wp_ebrlis.cgi?UN=%s&PN=%s&INSTANCE_ID=%s target=mainFrame><img src=/images/%s border=0 width=62 height=20/></a></td>",m,pno,instance_id,search(lpublic,"img_ok"));
		  fprintf(cgiOut,"<td width=62 align=center><a href=wp_ebrlis.cgi?UN=%s&PN=%s&INSTANCE_ID=%s target=mainFrame><img src=/images/%s border=0 width=62 height=20/></a></td>",m,pno,instance_id,search(lpublic,"img_cancel"));
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
                    "<td align=left id=tdleft background=/images/bottom_bg.gif style=\"border-right:0\"><font id=yingwen_san>EBR</font><font id=%s> %s</font></td>",search(lpublic,"menu_san"),search(lpublic,"details"));   /*突出显示*/
                  fprintf(cgiOut,"</tr>");
				  if(retu==0) /*管理员*/
				  {
                    fprintf(cgiOut,"<tr height=25>"\
					  "<td align=left id=tdleft><a href=wp_ebrnew.cgi?UN=%s target=mainFrame class=top><font id=%s>%s</font><font id=yingwen_san> EBR</font></a></td>",m,search(lpublic,"menu_san"),search(lpublic,"create"));                       
                    fprintf(cgiOut,"</tr>");
				  }	

				  if((inter_num>10)&&(uplink_if_num>10))	/*用div限制页面高度*/
				  {
				  	if(retu==0)  /*管理员*/
					  limit=30;				  
					else
					  limit=23;
				  }
				  else if((inter_num<11)&&(uplink_if_num<11))
				  {
				      limit=5;
					  limit+=inter_num+uplink_if_num;
					  if((retu==0)&&(inter_num>0))
						limit+=2;
					  if((retu==0)&&(uplink_if_num>0))
					  	limit+=2;
				  }
				  else
				  {
				  	if(retu==0)  /*管理员*/
					  limit=15;				  
					else
					  limit=14;
					if(inter_num<11)
					  limit+=inter_num;
					if((uplink_if_num<11))
					  limit+=uplink_if_num;
					if((retu==0)&&(inter_num>0))
					  limit+=2;
				    if((retu==0)&&(uplink_if_num>0))
				  	  limit+=2;
				  }

				  /*获取ebr三层接口信息*/
				  if((result == 1)&&(ebrinfo)&&(ebrinfo->EBR[0]))
				  {
					  memset(if_name,0,sizeof(if_name));					  
					  if(paraHead1)
					  {
						  if(paraHead1->parameter.local_id == SNMPD_LOCAL_INSTANCE)
							snprintf(if_name,sizeof(if_name)-1,"ebrl%d-%d-%d",paraHead1->parameter.slot_id,paraHead1->parameter.instance_id,ebrinfo->EBR[0]->EBRID);
						  else
							snprintf(if_name,sizeof(if_name)-1,"ebr%d-%d-%d",paraHead1->parameter.slot_id,paraHead1->parameter.instance_id,ebrinfo->EBR[0]->EBRID);
					  } 
				  }
				  ret = get_all_if_info(&interf);
				  if((ret == 0)&&(interf.if_head))
				  {
				  	for(i = 0,q = interf.if_head->next;
						((i<interf.if_num)&&(NULL != q));
						i++,q = q->next)
				  	{
						if((strlen(if_name)==strlen(q->ifname))&&(strcmp(if_name,q->ifname) == 0))
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
              "<td align=left valign=top style=\"background-color:#ffffff; border-right:1px solid #707070; padding-left:30px; padding-top:10px\">"\
 "<table width=323 border=0 bgcolor=#ffffff cellspacing=0 cellpadding=0>");
  fprintf(cgiOut,"<tr>"\
                     "<td id=ins_style>%s:%s</td>",search(lpublic,"instance"),instance_id);
                  fprintf(cgiOut,"</tr>");			
  fprintf(cgiOut,"<tr valign=middle>"\
    "<td align=center>");
			if(result==1)	
			{	
			   fprintf(cgiOut,"<table width=323 border=0 cellspacing=0 cellpadding=0>"\
	           "<tr align=left height=10 valign=top>"\
	           "<td id=thead1>EBR %s</td>",search(lpublic,"details"));
	           fprintf(cgiOut,"</tr>"\
               "<tr>"\
               "<td align=center style=\"padding-left:20px\">"\
			   "<table frame=below rules=rows width=323 border=1>"\
				 "<tr align=left>"\
				   "<td id=td1 width=140>ID</td>");
			   	   if((ebrinfo)&&(ebrinfo->EBR[0]))
			   	   {
					   fprintf(cgiOut,"<td id=td2 width=170>%d</td>",ebrinfo->EBR[0]->EBRID);
			   	   }
			   	   fprintf(cgiOut,"<td width=13>&nbsp;</td>"\
			   "</tr>"\
			   "<tr align=left>"\
			   "<td id=td1>%s</td>",search(lpublic,"name"));
			   if((ebrinfo)&&(ebrinfo->EBR[0])&&(ebrinfo->EBR[0]->name))
		   	   {
				   fprintf(cgiOut,"<td id=td2>%s</td>",ebrinfo->EBR[0]->name);
		   	   }
			   fprintf(cgiOut,"<td>&nbsp;</td>"\
			  "</tr>");
			  if(strcmp(if_name,""))
			  {
				  j=0;
				  if((ret == 0)&&(interf.if_head))
				  {
					for(i = 0,q = interf.if_head->next;
						((i<interf.if_num)&&(NULL != q));
						i++,q = q->next)
					{
						memset(menu,0,15);
						strncat(menu,"menuLists",sizeof(menu)-strlen(menu)-1);
						snprintf(menu_id,sizeof(menu_id)-1,"%d",i+1); 
						strncat(menu,menu_id,sizeof(menu)-strlen(menu)-1);
						if((strlen(if_name)==strlen(q->ifname))&&(strcmp(if_name,q->ifname) == 0))
						{
							fprintf(cgiOut,"<tr align=left>");
							if(j == 0)
							{
							  fprintf(cgiOut,"<td id=td1>%s</td>",search(lpublic,"l3_interface_name_ip_mask"));
							}
							else
							{
							  fprintf(cgiOut,"<td id=td1>&nbsp;</td>");
							}
							  fprintf(cgiOut,"<td id=td2>%s: %s/%d</td>",if_name,q->ipaddr,q->mask);
							  fprintf(cgiOut,"<td>"\
									"<div style=\"position:relative; z-index:%d\" onmouseover=\"popMenu('%s');\" onmouseout=\"popMenu('%s');\">",(interf.if_num-i),menu,menu);
								fprintf(cgiOut,"<img src=/images/detail.gif>"\
									"<div id=%s style=\"display:none; position:absolute; top:5px; left:0;\">",menu);
								fprintf(cgiOut,"<div id=div1>");
								if(retu==0)  /*管理员*/
								{
								  fprintf(cgiOut,"<div id=div2 onmouseover=\"this.style.backgroundColor='#b6bdd2'\" onmouseout=\"this.style.backgroundColor='#f9f8f7'\"><a id=link href=wp_ebrdta.cgi?UN=%s&ID=%s&PN=%s&DeletEbrIP=%s&INSTANCE_ID=%s&IPADDR=%s/%d target=mainFrame>%sIP</a></div>",m,t,pno,"true",instance_id,q->ipaddr,q->mask,search(lpublic,"delete"));
								}
								fprintf(cgiOut,"</div>"\
								"</div>"\
								"</div>"\
								"</td>"\
							"</tr>");
							j++;
						}
					}
				  }
			  }
			  fprintf(cgiOut,"<tr align=left>"\
				"<td id=td1>%s</td>",search(lwlan,"state"));			  
			  if((ebrinfo->EBR[0])&&(ebrinfo->EBR[0]->state==1))
			    fprintf(cgiOut,"<td id=td2>enable</td>");
			  else
			  	fprintf(cgiOut,"<td id=td2>disable</td>");
			    fprintf(cgiOut,"<td>&nbsp;</td>"\
			  "</tr>"\
			  "<tr align=left>"\
				"<td id=td1>%s</td>",search(lwlan,"isolation"));
			  if((ebrinfo->EBR[0])&&(ebrinfo->EBR[0]->isolation_policy==1))
			    fprintf(cgiOut,"<td id=td2>enable</td>");
			  else
			  	fprintf(cgiOut,"<td id=td2>disable</td>");
			    fprintf(cgiOut,"<td>&nbsp;</td>"\
			  "</tr>"\
			  "<tr align=left>"\
				"<td id=td1>%s</td>",search(lwlan,"multicast"));
			  if((ebrinfo->EBR[0])&&(ebrinfo->EBR[0]->multicast_isolation_policy==1))
			    fprintf(cgiOut,"<td id=td2>enable</td>");
			  else
			  	fprintf(cgiOut,"<td id=td2>disable</td>");
			    fprintf(cgiOut,"<td>&nbsp;</td>"\
			  "</tr>"\
			  "<tr align=left>"\
				"<td id=td1>%s</td>",search(lwlan,"spswitch"));
			  if((ebrinfo->EBR[0])&&(ebrinfo->EBR[0]->sameportswitch==1))
			    fprintf(cgiOut,"<td id=td2>enable</td>");
			  else
			  	fprintf(cgiOut,"<td id=td2>disable</td>");
			    fprintf(cgiOut,"<td>&nbsp;</td>"\
			  "</tr>");
			  
			  if(inter_num>0)
			  {
			    if(retu==0)  /*管理员*/
			    {
                fprintf(cgiOut,"<tr align=left>");
				  fprintf(cgiOut,"<td id=td1>%s</td>",search(lwlan,"interface"));
					fprintf(cgiOut,"<td id=td2><input type=submit style=\"width:40px; margin-left:5px\" border=0 name=DownlinkIf_apply style=background-image:url(/images/SubBackGif.gif) value=\"%s\"></td>",search(lpublic,"delete"));			
					fprintf(cgiOut,"<td>&nbsp;</td>"\
			      "</tr>");
				  if(inter_num>10)
				  {
					  fprintf(cgiOut,"<tr align=left><td colspan=3>"\
						"<div class=iflis1><table frame=below rules=rows width=310 border=1>");
				  }				  	
				  if((ebrinfo)&&(ebrinfo->EBR[0])&&(ebrinfo->EBR[0]->iflist))
				  {	
					head = ebrinfo->EBR[0]->iflist;
					while(head != NULL)
					{
						fprintf(cgiOut,"<tr align=left>");
						if(head->ifname)
						{
							fprintf(cgiOut,"<td colspan=2 id=td2 style=\"padding-left:150px\"><input type=checkbox name=own_if value=%s>%s</td>",head->ifname,head->ifname);
						}
					    fprintf(cgiOut,"</tr>");
						head = head->ifnext;
					}
				  }
				  if(inter_num>10)
				  {
					  fprintf(cgiOut,"</table></div></td></tr>");
				  }
				  fprintf(cgiOut,"<tr align=left>");
				      fprintf(cgiOut,"<td colspan=3 id=td1 style=\"padding-left:150px\"><input type=checkbox name=ownif_all value=all onclick=\"checkAll(this,'own_if')\">%s</td>",search(lwlan,"all"));
				  fprintf(cgiOut,"</tr>");
		      	}
				else
			    {
			      i=0;
				  if(inter_num>10)
				  {
					  fprintf(cgiOut,"<tr align=left><td colspan=3>"\
						"<div class=iflis2><table frame=below rules=rows width=310 border=1>");
				  }				  	
				  if((ebrinfo)&&(ebrinfo->EBR[0])&&(ebrinfo->EBR[0]->iflist))
				  {
					  head = ebrinfo->EBR[0]->iflist;
					  while(head != NULL)
					  {
						  fprintf(cgiOut,"<tr align=left>");
						  if(i==0)
							fprintf(cgiOut,"<td id=td1>%s</td>",search(lwlan,"interface"));
						  else
							fprintf(cgiOut,"<td id=td1></td>");
						  i++;
						if(head->ifname)
						{
							fprintf(cgiOut,"<td id=td2>%s</td>",head->ifname);
						}
						fprintf(cgiOut,"</tr>");
						head = head->ifnext;
					  }
				  }
				  if(inter_num>10)
				  {
					  fprintf(cgiOut,"</table></div></td></tr>");
				  }
			    }
			  }

			  if(uplink_if_num>0)
			  {
			    if(retu==0)  /*管理员*/
			    {
                fprintf(cgiOut,"<tr align=left>");
				  fprintf(cgiOut,"<td id=td1>%s</td>",search(lwlan,"uplink_interface"));
					fprintf(cgiOut,"<td id=td2><input type=submit style=\"width:40px; margin-left:5px\" border=0 name=delete_uplinkIf_apply style=background-image:url(/images/SubBackGif.gif) value=\"%s\"></td>",search(lpublic,"delete"));			
					fprintf(cgiOut,"<td>&nbsp;</td>"\
			      "</tr>");
				  if(uplink_if_num>10)
				  {
					  fprintf(cgiOut,"<tr align=left><td colspan=3>"\
						"<div class=iflis1><table frame=below rules=rows width=310 border=1>");
				  }		
				  if((ebrinfo)&&(ebrinfo->EBR[0])&&(ebrinfo->EBR[0]->uplinklist))
				  {	
					head = ebrinfo->EBR[0]->uplinklist;
					while(head != NULL)
					{
						fprintf(cgiOut,"<tr align=left>");
						if(head->ifname)
						{
							fprintf(cgiOut,"<td colspan=2 id=td2 style=\"padding-left:150px\"><input type=checkbox name=own_uplink_if value=%s>%s</td>",head->ifname,head->ifname);
						}
					    fprintf(cgiOut,"</tr>");
						head = head->ifnext;
					}
				  }
				  if(uplink_if_num>10)
				  {
					  fprintf(cgiOut,"</table></div></td></tr>");
				  }
				  fprintf(cgiOut,"<tr align=left>");
				      fprintf(cgiOut,"<td colspan=3 id=td1 style=\"padding-left:150px\"><input type=checkbox name=ownif_all value=all onclick=\"checkAll(this,'own_uplink_if')\">%s</td>",search(lwlan,"all"));
				  fprintf(cgiOut,"</tr>");
		      	}
				else
			    {
			      i=0;
				  if(uplink_if_num>10)
				  {
					  fprintf(cgiOut,"<tr align=left><td colspan=3>"\
						"<div class=iflis2><table frame=below rules=rows width=310 border=1>");
				  }		
				  if((ebrinfo)&&(ebrinfo->EBR[0])&&(ebrinfo->EBR[0]->uplinklist))
				  {
					  head = ebrinfo->EBR[0]->uplinklist;
					  while(head != NULL)
					  {
						  fprintf(cgiOut,"<tr align=left>");
						  if(i==0)
							fprintf(cgiOut,"<td id=td1>%s</td>",search(lwlan,"uplink_interface"));
						  else
							fprintf(cgiOut,"<td id=td1></td>");
						  i++;
						if(head->ifname)
						{
							fprintf(cgiOut,"<td id=td2>%s</td>",head->ifname);
						}
						fprintf(cgiOut,"</tr>");
						head = head->ifnext;
					  }
				  }
				  if(uplink_if_num>10)
				  {
					  fprintf(cgiOut,"</table></div></td></tr>");
				  }
			    }
			  }
			  fprintf(cgiOut,"</table></td>"\
				"</tr>"\
				"<tr>"\
			      "<td><input type=hidden name=encry_ebrdta value=%s></td>",m);
			    fprintf(cgiOut,"</tr>"\
				"<tr>"\
			      "<td><input type=hidden name=ebrid value=%s></td>",t);
			    fprintf(cgiOut,"</tr>"\
				"<tr>"\
			      "<td><input type=hidden name=instance_id value=%s></td>",instance_id);
			    fprintf(cgiOut,"</tr>"\
				"<tr>"\
			      "<td><input type=hidden name=SubmitFlag value=%d></td>",1);
			    fprintf(cgiOut,"</tr>"\
			  "</table>");
			}
            else if((result==0)||(result == SNMPD_CONNECTION_ERROR))
		      fprintf(cgiOut,"%s",search(lpublic,"contact_adm")); 	 
			else if(result==-1)
			  fprintf(cgiOut,"%s",search(lpublic,"unknown_id_format"));    	
			else if(result==-2)
			{
			  memset(temp,0,sizeof(temp));
			  strncpy(temp,search(lwlan,"ebr_id_1"),sizeof(temp)-1);
              memset(eid,0,sizeof(eid));
              snprintf(eid,sizeof(eid)-1,"%d",EBR_NUM-1);
              strncat(temp,eid,sizeof(temp)-strlen(temp)-1);
              strncat(temp,search(lwlan,"ebr_id_2"),sizeof(temp)-strlen(temp)-1);
			  fprintf(cgiOut,"%s",temp); 
			}
			else if(result==-3)
			  fprintf(cgiOut,"%s",search(lwlan,"ebr_not_exist")); 
			else if(result==-4)
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
if(ret == 0)
{
	Free_get_all_if_info(&interf);
}
if(result==1)
{
  Free_ethereal_bridge_one_head(ebrinfo);
}
free_instance_parameter_list(&paraHead1);
}

void DownLinkIf(instance_parameter *ins_para,int id,struct list *lpublic,struct list *lwlan)
{
  int result = cgiFormNotFound;   
  char **responses;  
  int ret = 0,flag = 1;
  result = cgiFormStringMultiple("own_if", &responses);
  if(result == cgiFormNotFound)           
  {
    flag=0;
    ShowAlert(search(lwlan,"select_inter"));
  }
  else                  
  {
    int i = 0;	
    while((responses[i])&&(flag))
    {
	  ret=set_ebr_add_del_if_cmd(ins_para->parameter,ins_para->connection,id,"delete",responses[i]);/*返回0表示失败，返回1表示成功，返回-1表示input parameter should only be 'add' or 'delete'*/
																									/*返回-2表示if name too long，返回-3表示ebr id does not exist，返回-4表示ebr should be disable first*/
																									/*返回-5表示if_name already exist/remove some br,or system cmd process error，返回-6表示input ifname error*/
																									/*返回-7表示ebr if error，返回-8表示error*/
	  switch(ret)
	  {
	    case SNMPD_CONNECTION_ERROR:
	  	case 0:ShowAlert(search(lwlan,"delete_inter_fail"));
			   flag=0;
			   break;
		case 1:break;
		case -1:ShowAlert(search(lpublic,"input_para_error"));
			    flag=0;
			    break;
		case -2:ShowAlert(search(lpublic,"if_name_long"));
			    flag=0;
			    break;
		case -3:ShowAlert(search(lwlan,"ebr_not_exist"));
			    flag=0;
			    break;
		case -4:ShowAlert(search(lwlan,"dis_ebr"));
			    flag=0;
			    break;
		case -5:ShowAlert(search(lwlan,"if_name_exist"));
			    flag=0;
			    break;
		case -6:ShowAlert(search(lpublic,"input_ifname_error"));
			    flag=0;
			    break;
		case -7:ShowAlert(search(lwlan,"ebr_if_error"));
			    flag=0;
			    break;
		case -8:ShowAlert(search(lpublic,"error"));
			    flag=0;
			    break;
	  }
	  i++;
	}
  }  
  cgiStringArrayFree(responses);
  
  if(flag==1)
  	ShowAlert(search(lwlan,"delete_inter_succ"));
}

void DeleteUplinkIf(instance_parameter *ins_para,int id,struct list *lpublic,struct list *lwlan)
{
  int result = cgiFormNotFound;   
  char **responses;  
  int ret = 0,flag = 1;
  char temp[100] = { 0 };
  char ebr_id[10] = { 0 };
  
  result = cgiFormStringMultiple("own_uplink_if", &responses);
  if(result == cgiFormNotFound)           
  {
    flag=0;
    ShowAlert(search(lwlan,"select_inter"));
  }
  else                  
  {
    int i = 0;	
    while((responses[i])&&(flag))
    {
	  ret=set_ebr_add_del_uplink_cmd(ins_para->parameter,ins_para->connection,id,"delete",responses[i]);/*返回0表示失败，返回1表示成功，返回-1表示error*/
																									  /*返回-2表示input parameter should only be 'add' or 'delete'，返回-3表示if name too long*/
																									  /*返回-4表示malloc error，返回-5表示ebr should be disable first*/
																									  /*返回-6表示already exist/remove some br,or system cmd process error，返回-7表示input ifname error*/
																									  /*返回-8表示ebr if error，返回-9表示interface does not add to br or br uplink，返回-10表示ebr id does not exist*/
																									  /*返回-11示EBR ID非法*/
	  switch(ret)
	  {
	      case SNMPD_CONNECTION_ERROR:
		  case 0:ShowAlert(search(lwlan,"set_uplink_interface_fail"));
				 flag=0;
				 break;
		  case 1:break;
		  case -1:ShowAlert(search(lpublic,"error"));
				  flag=0;
				  break;
		  case -2:ShowAlert(search(lpublic,"input_para_error"));
				  flag=0;
				  break;
		  case -3:ShowAlert(search(lpublic,"if_name_long"));
				  flag=0;
				  break;
		  case -4:ShowAlert(search(lpublic,"malloc_error"));
				  flag=0;
				  break;	
		  case -5:ShowAlert(search(lwlan,"dis_ebr"));
				  flag=0;
				  break;
		  case -6:ShowAlert(search(lwlan,"if_name_exist"));
				  flag=0;
				  break;
		  case -7:ShowAlert(search(lpublic,"input_ifname_error"));
				  flag=0;
				  break;
		  case -8:ShowAlert(search(lwlan,"ebr_if_error"));
				  flag=0;
				  break;
		  case -9:ShowAlert(search(lwlan,"if_not_add_to_br"));
				  flag=0;
				  break;				  
		  case -10:ShowAlert(search(lwlan,"ebr_not_exist"));
				   flag=0;
				   break;	
		  case -11:{
					  memset(temp,0,sizeof(temp));
					  strncpy(temp,search(lwlan,"ebr_id_1"),sizeof(temp)-1);
					  memset(ebr_id,0,sizeof(ebr_id));
					  snprintf(ebr_id,sizeof(ebr_id)-1,"%d",EBR_NUM-1);
					  strncat(temp,ebr_id,sizeof(temp)-strlen(temp)-1);
					  strncat(temp,search(lwlan,"ebr_id_2"),sizeof(temp)-strlen(temp)-1);
					  ShowAlert(temp);
					  flag=0;
					  break;
				   }
		}
	  i++;
	}
  }  
  cgiStringArrayFree(responses);
  
  if(flag==1)
  	ShowAlert(search(lwlan,"delete_uplink_inter_succ"));
}

void DeleteIfIP(instance_parameter *ins_para,char *id,char *ipaddr,struct list *lpublic)
{
	int status,retu = -1;	
	char *endptr = NULL; 
	char command[100] = { 0 };
	char tmp[10] = { 0 };
  
	memset(command,0,sizeof(command));
	strncat(command,"del_intf_ip_ins.sh ",sizeof(command)-strlen(command)-1);	
	memset(tmp,0,sizeof(tmp));
	snprintf(tmp,sizeof(tmp)-1,"%d",ins_para->parameter.slot_id);
	strncat(command,tmp,sizeof(command)-strlen(command)-1);

	strncat(command," ",sizeof(command)-strlen(command)-1);
	memset(tmp,0,sizeof(tmp));
	snprintf(tmp,sizeof(tmp)-1,"%d",ins_para->parameter.local_id);
	strncat(command,tmp,sizeof(command)-strlen(command)-1);

	strncat(command," ",sizeof(command)-strlen(command)-1);
	memset(tmp,0,sizeof(tmp));
	snprintf(tmp,sizeof(tmp)-1,"%d",ins_para->parameter.instance_id);
	strncat(command,tmp,sizeof(command)-strlen(command)-1);

	strncat(command," ",sizeof(command)-strlen(command)-1);
	strncat(command,"ebr",sizeof(command)-strlen(command)-1);
	strncat(command,id,sizeof(command)-strlen(command)-1);
	
	strncat(command," ",sizeof(command)-strlen(command)-1);
	strncat(command,ipaddr,sizeof(command)-strlen(command)-1);
	strncat(command," > /dev/null 2>&1",sizeof(command)-strlen(command)-1);
	if(strstr(command,";") == NULL)
	{
		status = system(command);	
		retu = WEXITSTATUS(status);
		if(0 == retu)
		{
			ShowAlert(search(lpublic,"delete_ip_succ"));
		}
		else
		{
			ShowAlert(search(lpublic,"delete_ip_fail"));
		}		  
	}
}

