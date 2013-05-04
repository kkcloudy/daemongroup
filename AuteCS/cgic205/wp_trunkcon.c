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
* wp_trunkcon.c
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
#include "ws_usrinfo.h"
#include "ws_err.h"
#include "ws_ec.h"
#include "util/npd_list.h"
#include "sysdef/npd_sysdef.h"
#include "npd/nam/npd_amapi.h"
#include "ws_public.h"
#include "ws_trunk.h"
#include "ws_dcli_portconf.h"

int ShowTrunkconPage(char *m,char *n,char *t,char *pn,struct list *lpublic,struct list *lcon);  
void Add_Delete_port(int id,char *flag,struct list *lpublic,struct list *lcon);
void Set_Master_port(int id,struct list *lpublic,struct list *lcon);
void Allow_Refuse_vlan(int trunk_id,char *flag,struct list *lpublic,struct list *lcon);
void Config_port_state(int id,struct list *lpublic,struct list *lcon);
void Config_load_balance(struct list *lpublic,struct list *lcon);

int cgiMain()
{
  char *encry=(char *)malloc(BUF_LEN); 
  char *ID=(char *)malloc(10);
  char *pno=(char *)malloc(10);  
  char *str;
  struct list *lpublic;   /*解析public.txt文件的链表头*/
  struct list *lcon;     /*解析wlan.txt文件的链表头*/  
  lpublic=get_chain_head("../htdocs/text/public.txt");
  lcon=get_chain_head("../htdocs/text/control.txt");
  memset(encry,0,BUF_LEN);
  memset(ID,0,10);
  memset(pno,0,10);
  if(cgiFormStringNoNewlines("UN", encry, BUF_LEN)!=cgiFormNotFound )  /*首次进入该页*/
  {
    cgiFormStringNoNewlines("ID", ID, 10); 
	cgiFormStringNoNewlines("PN",pno,10);
  }
  else
  {
    cgiFormStringNoNewlines("encry_contrunk",encry,BUF_LEN);
	cgiFormStringNoNewlines("trunkid",ID,10);
	cgiFormStringNoNewlines("page_no",pno,10);
  }
  str=dcryption(encry);
  if(str==NULL)
    ShowErrorPage(search(lpublic,"ill_user"));		 /*用户非法*/
  else
	ShowTrunkconPage(encry,str,ID,pno,lpublic,lcon);
  free(encry);
  free(ID);
  free(pno);
  release(lpublic);  
  release(lcon);
  return 0;
}

int ShowTrunkconPage(char *m,char *n,char *t,char *pn,struct list *lpublic,struct list *lcon)
{  
  FILE *fp;
  char lan[3];  
  struct trunk_profile trunk; 
  struct port_profile *pq;
  char *endptr = NULL;
  int i,trunk_id,retu,result;
  unsigned int  tmpVal[2];
  memset(&tmpVal,0,sizeof(tmpVal));
  trunk.trunkId=0;
  trunk.mSlotNo=0;
  trunk.mPortNo=0;
  
  ETH_SLOT_LIST  head,*p;
  ETH_PORT_LIST *pp;
  int ret,num=0;
  cgiHeaderContentType("text/html");
  fprintf(cgiOut,"<html xmlns=\"http://www.w3.org/1999/xhtml\"><head>");
  fprintf(cgiOut,"<meta http-equiv=Content-Type content=text/html; charset=gb2312>");
  fprintf(cgiOut,"<title>Wlan</title>");
  fprintf(cgiOut,"<link rel=stylesheet href=/style.css type=text/css>"\
  "</head>"\
  "<body>");
  trunk_id= strtoul(t,&endptr,10);	/*char转成int，10代表十进制*/	
  if(cgiFormSubmitClicked("add_port") == cgiFormSuccess)         /*添加端口*/
  {
    Add_Delete_port(trunk_id,"add",lpublic,lcon);
  }
  if(cgiFormSubmitClicked("delete_port") == cgiFormSuccess)      /*删除端口*/
  {
    Add_Delete_port(trunk_id,"delete",lpublic,lcon);
  }
  if(cgiFormSubmitClicked("set_mport") == cgiFormSuccess)        /*设置主端口*/
  {
    Set_Master_port(trunk_id,lpublic,lcon);
  }
  if(cgiFormSubmitClicked("allow_vlan") == cgiFormSuccess)       /*允许VLAN*/
  {
    Allow_Refuse_vlan(trunk_id,"allow",lpublic,lcon);
  }
  if(cgiFormSubmitClicked("refuse_vlan") == cgiFormSuccess)      /*禁止VLAN*/
  {
    Allow_Refuse_vlan(trunk_id,"refuse",lpublic,lcon);
  }
  if(cgiFormSubmitClicked("config_portState") == cgiFormSuccess) /*配置port state*/
  {
    Config_port_state(trunk_id,lpublic,lcon);
  }
  if(cgiFormSubmitClicked("config_loadBalance") == cgiFormSuccess) /*配置load balance*/
  {
    Config_load_balance(lpublic,lcon);
  }
  fprintf(cgiOut,"<form>"\
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

	  fprintf(cgiOut,"<table width=155 border=0 cellspacing=0 cellpadding=0>"\
      "<tr>"\
      "<td width=62 align=center><a href=wp_trunklis.cgi?UN=%s&PN=%s target=mainFrame><img src=/images/%s border=0 width=62 height=20/></a></td>",m,pn,search(lpublic,"img_ok"));
      fprintf(cgiOut,"<td width=62 align=center><a href=wp_trunklis.cgi?UN=%s&PN=%s target=mainFrame><img src=/images/%s border=0 width=62 height=20/></a></td>",m,pn,search(lpublic,"img_cancel"));
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
                    "<td align=left id=tdleft background=/images/bottom_bg.gif style=\"border-right:0\"><font id=%s>%s<font></td>",search(lpublic,"menu_san"),search(lcon,"con_trunk"));   /*突出显示*/
                  fprintf(cgiOut,"</tr>");
				  if(retu==0) /*管理员*/
				  {
                    fprintf(cgiOut,"<tr height=25>"\
					  "<td align=left id=tdleft><a href=wp_trunknew.cgi?UN=%s target=mainFrame class=top><font id=%s>%s<font></a></td>",m,search(lpublic,"menu_san"),search(lcon,"create_trunk"));                       
                    fprintf(cgiOut,"</tr>");
				  }		
                  for(i=0;i<3;i++)
	              {
  				    fprintf(cgiOut,"<tr height=25>"\
                      "<td id=tdleft>&nbsp;</td>"\
                    "</tr>");
	              }
                fprintf(cgiOut,"</table>"\
              "</td>"\
              "<td align=left style=\"background-color:#ffffff; border-right:1px solid #707070; padding-left:30px\">");
    fprintf(cgiOut,"<table width=600 border=0 cellspacing=0 cellpadding=0>"\
    "<tr height=30>"\
	"<td colspan=5>%s Trunk %d</td>",search(lcon,"config"),trunk_id);
	fprintf(cgiOut,"</tr>");
    fprintf(cgiOut,"<tr height=30>");

	fprintf(cgiOut,"<td width=90 style=\"padding-left:20px\">%s:</td>",search(lcon,"port_no"));	
    //fprintf(cgiOut,"<td width=70 align=left><input type=text name=port size=7></td>");
	fprintf(cgiOut,"<td width=70 align=left>\n");
	fprintf(cgiOut,"<select name=port id=port style=width:70px>\n");
	ccgi_dbus_init();	
	ret=show_ethport_list(&head,&num);
	p=head.next;
	if(p!=NULL)
	{
		while(p!=NULL)
		{
			pp=p->port.next;
			while(pp!=NULL)
			{
				fprintf(cgiOut,"<option value='%d/%d'>%d/%d</option>\n",p->slot_no,pp->port_no,p->slot_no,pp->port_no);
				pp=pp->next;
			}
			p=p->next;
		}
	 }
	fprintf(cgiOut,"</select>\n");
	fprintf(cgiOut,"</td>\n");

	fprintf(cgiOut,"<td width=80><input type=submit style=width:60px; height:36px  border=0 name=add_port style=background-image:url(/images/SubBackGif.gif) value=\"%s\"></td>",search(lcon,"add"));	  
	fprintf(cgiOut,"<td width=70><input type=submit style=width:60px; height:36px  border=0 name=delete_port style=background-image:url(/images/SubBackGif.gif) value=\"%s\"></td>",search(lcon,"delete"));
	fprintf(cgiOut,"<td width=100><input type=submit style=width:80px; height:36px  border=0 name=set_mport style=background-image:url(/images/SubBackGif.gif) value=\"%s\"></td>",search(lcon,"Set_Mport"));
  
  fprintf(cgiOut,"</tr>"\
  "<tr height=30>"\
    "<td style=\"padding-left:20px\">VLAN ID:</td>"\
	"<td align=left><input type=text name=vlan_id size=7></td>"\
    "<td align=left>"\
      "<select name=vlan_mode id=vlan_mode style=width:70px>"\
		"<option value=untag>untag"\
  		"<option value=tag>tag"\
	  "</select>"\
    "</td>");
  fprintf(cgiOut,"<td><input type=submit style=width:60px; height:36px  border=0 name=allow_vlan style=background-image:url(/images/SubBackGif.gif) value=\"%s\"></td>",search(lcon,"Allow"));	  
  fprintf(cgiOut,"<td><input type=submit style=width:60px; height:36px  border=0 name=refuse_vlan style=background-image:url(/images/SubBackGif.gif) value=\"%s\"></td>",search(lcon,"Refuse"));
  fprintf(cgiOut,"</tr>"\
  "<tr height=30>"\
    "<td style=\"padding-left:20px\">%s:</td>",search(lcon,"port_no"));
    ccgi_dbus_init();
    result=show_trunk_byid(trunk_id,&trunk); 
    fprintf(cgiOut,"<td align=left>"\
		"<select name=port_num id=port_num style=width:53px>");
	    if(trunk.masterFlag!=0)
        {
          pq=trunk.portHead->next;
	      for(i=0;i<64;i++)
	      {
	        tmpVal[i/32] = (1<<(i%32));
	        if((trunk.mbrBmp_sp.portMbr[i/32]) & tmpVal[i/32])
	        {
	          fprintf(cgiOut,"<option value='%d/%d'>%d/%d",pq->slot,pq->port,pq->slot,pq->port);
	          pq=pq->next;
	        }
	      }

          memset(&tmpVal,0,sizeof(tmpVal));
		  for(i=0;i<64;i++)
          {
            tmpVal[i/32] = (1<<(i%32));
			if((trunk.disMbrBmp_sp.portMbr[i/32]) & tmpVal[i/32])
	        {
	          fprintf(cgiOut,"<option value='%d/%d'>%d/%d",pq->slot,pq->port,pq->slot,pq->port);
	          pq=pq->next;
	        }
	      }
        }
	  fprintf(cgiOut,"</select>"\
	"</td>"\
    "<td align=left>"\
		"<select name=port_state id=port_state style=width:70px>"\
		"<option value=disable>disable"\
  		"<option value=enable>enable"\
	  "</select>"\
	"</td>");

  fprintf(cgiOut,"<td colspan=2><input type=submit style=width:60px; height:36px  border=0 name=config_portState style=background-image:url(/images/SubBackGif.gif) value=\"%s\"></td>",search(lcon,"config"));
  fprintf(cgiOut,"</tr>"\
  "<tr height=30>"\
    "<td style=\"padding-left:20px\">%s:</td>",search(lcon,"load_balance"));
    fprintf(cgiOut,"<td align=left colspan=2>"\
      "<select name=load_balance id=load_balance style=width:90px>"\
		"<option value=based-port>based-port"\
  		"<option value=based-mac>based-mac"\
  		"<option value=based-ip>based-ip"\
  		"<option value=based-L4>based-L4"\
  		"<option value=mac+ip>mac+ip"\
  		"<option value=mac+L4>mac+L4"\
  		"<option value=ip+L4>ip+L4"\
  		"<option value=mac+ip+L4>mac+ip+L4"\
	  "</select>"\
    "</td>");

  //kehao modify  2011-04-26
  //fprintf(cgiOut,"<td colspan=2><input type=submit style=width:60px; height:36px  border=0 name=config_portState style=background-image:url(/images/SubBackGif.gif) value=\"%s\"></td>",search(lcon,"config"));  
  fprintf(cgiOut,"<td colspan=2><input type=submit style=width:60px; height:36px  border=0 name=config_loadBalance style=background-image:url(/images/SubBackGif.gif) value=\"%s\"></td>",search(lcon,"config")); 
  //
  fprintf(cgiOut,"</tr>"\
  "<tr>"\
    "<td><input type=hidden name=encry_contrunk value=%s></td>",m);
    fprintf(cgiOut,"<td><input type=hidden name=trunkid value=%s></td>",t);
	fprintf(cgiOut,"<td colspan=3><input type=hidden name=page_no value=%s></td>",pn);
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
	if(result==1)
	{
	 	Free_trunk_one(&trunk);
	}

	if((ret==0)&&(num>0))
	{
		Free_ethslot_head(&head);
	}

return 0;
}

void Add_Delete_port(int id,char *flag,struct list *lpublic,struct list *lcon)
{
  int ret;  
  char *port_name=(char *)malloc(10);
  memset(port_name,0,10);
  cgiFormStringNoNewlines("port",port_name,10);  
  if((strcmp(port_name,"")!=0)&&(strchr(port_name,' ')==NULL))            /*port不能为空*/
  {
    ccgi_dbus_init();
    ret=add_delete_trunk_port(id,flag,port_name);	   
	switch(ret)
    {
      case 0:{
	  	       if(strcmp(flag,"add")==0)
	  	         ShowAlert(search(lcon,"add_port_fail"));
			   else
			   	 ShowAlert(search(lcon,"delete_port_fail"));
               break;
      	     }
      case 1:{
	  	       if(strcmp(flag,"add")==0)
	  	         ShowAlert(search(lcon,"add_port_succ"));
	           else
			     ShowAlert(search(lcon,"delete_port_succ"));
               break;
      	      }
      case -1:ShowAlert(search(lcon,"unknown_portno_format"));
               break;
      case -2:ShowAlert(search(lcon,"port_not_exist"));
    	      break;			
      case -3:ShowAlert(search(lcon,"trunk_not_exist"));
              break;
	  case -4:ShowAlert(search(lcon,"port_is_trunk_member"));
              break;	
      case -5:ShowAlert(search(lcon,"port_is_not_trunk_member"));
              break;	
	  case -6:ShowAlert(search(lcon,"trunk_port_full"));
    	      break;	
   	  case -7:ShowAlert(search(lcon,"port_own_other_trunk"));
    	      break;
	  case -8:ShowAlert(search(lcon,"port_is_l3"));
    	      break;
	  case -9:ShowAlert(search(lcon,"port_is_master"));
    	      break;
  	  case -10:ShowAlert(search(lpublic,"error"));
    	       break;	
    }   
  }
  else
    ShowAlert(search(lcon,"enter_port"));
  free(port_name);
}
void Set_Master_port(int id,struct list *lpublic,struct list *lcon)
{
  int ret;	
  char *port_name=(char *)malloc(10);
  memset(port_name,0,10);
  cgiFormStringNoNewlines("port",port_name,10);  
  if((strcmp(port_name,"")!=0)&&(strchr(port_name,' ')==NULL))			  /*port不能为空*/
  {
	ccgi_dbus_init();
	ret=set_master_port(id,port_name);
	switch(ret)
	{
	  case 0:ShowAlert(search(lcon,"set_master_port_fail"));
			 break;
	  case 1:ShowAlert(search(lcon,"set_master_port_succ"));
   		     break;
	  case -1:ShowAlert(search(lcon,"unknown_portno_format"));
		      break;
	  case -2:ShowAlert(search(lcon,"port_not_exist"));
			  break;			
	  case -3:ShowAlert(search(lcon,"trunk_not_exist"));
			  break;
	  case -4:ShowAlert(search(lcon,"port_is_not_trunk_member"));
			  break;	
	  case -5:ShowAlert(search(lcon,"port_is_l3"));
			  break;	
	  case -6:ShowAlert(search(lpublic,"error"));
			  break;	
   }   
 }
 else
   ShowAlert(search(lcon,"enter_port"));
 free(port_name);
}

void Allow_Refuse_vlan(int trunk_id,char *flag,struct list *lpublic,struct list *lcon)
{
  int ret,vlanID;  
  char *vid=(char *)malloc(10);  
  char *vMode=(char *)malloc(10);
  char *endptr = NULL;  
  memset(vid,0,10);
  cgiFormStringNoNewlines("vlan_id",vid,10);  
  memset(vMode,0,10);
  cgiFormStringNoNewlines("vlan_mode",vMode,10);
  if(strcmp(vid,"")!=0)            /*vlan id不能为空*/
  {
    vlanID= strtoul(vid,&endptr,10);					   /*char转成int，10代表十进制*/	
	if((vlanID>0)&&(vlanID<4095))           /*最多4094个wlan*/   
	{
      ccgi_dbus_init();
	  ret=allow_refuse_vlan(trunk_id,flag,vid,vMode);  /*flag="allow"或"refuse"，vlan_id范围1-4094*，mod="tag"或"untag"*/
                                                       /*返回0表示失败，返回1表示成功，返回-1表示Unknow vlan format.，返回-2表示vlan not exists*/
                                                       /*返回-3表示vlan is L3 interface，返回-4表示 vlan Already allow in trunk*/
                                                       /*返回-5表示vlan Already allow in other trunk，返回-6表示 There exists no member in trunk*/
                                                       /*返回-7表示Vlan NOT allow in trunk，返回-8表示tagMode error in vlan，返回-9表示error*/
      switch(ret)
      {
        case 0:{
	  	         if(strcmp(flag,"allow")==0)
	  	           ShowAlert(search(lcon,"allow_vlan_fail"));
			     else
			       ShowAlert(search(lcon,"refuse_vlan_fail"));
                 break;
      	       }
        case 1:{
	  	         if(strcmp(flag,"allow")==0)
	  	           ShowAlert(search(lcon,"allow_vlan_succ"));
	             else
			       ShowAlert(search(lcon,"refuse_vlan_succ"));
                 break;
      	       }
        case -1:ShowAlert(search(lcon,"unknown_vlan_format"));
                 break;
        case -2:ShowAlert(search(lcon,"VLAN_NOT_EXITSTS"));
       	        break;			
        case -3:ShowAlert(search(lcon,"vlan_is_l3"));
                break;
	    case -4:ShowAlert(search(lcon,"vlan_is_allow"));
                break;	
        case -5:ShowAlert(search(lcon,"vlan_is_allow_inother"));
                break;	
	    case -6:ShowAlert(search(lcon,"trunk_no_member"));
    	        break;	
		case -7:ShowAlert(search(lcon,"vlan_not_allow"));
       	        break;	
		case -8:ShowAlert(search(lcon,"vlan_tagmode_error"));
       	        break;	
  	    case -9:ShowAlert(search(lpublic,"error"));
       	        break;	
      }
  	}
	else
	  ShowAlert(search(lcon,"vlan_id_illegal"));
  }
  else
    ShowAlert(search(lcon,"enter_vlan_id"));
  free(vid);
  free(vMode);
}

void Config_port_state(int id,struct list *lpublic,struct list *lcon)
{
  int ret;	
  char *port_name=(char *)malloc(10);
  char *port_state=(char *)malloc(10);
  memset(port_name,0,10);
  cgiFormStringNoNewlines("port_num",port_name,10);  
  memset(port_state,0,10);
  cgiFormStringNoNewlines("port_state",port_state,10);  

  ccgi_dbus_init();
  ret=set_port_state(id,port_name,port_state);  
  switch(ret)
  {
	case 0:ShowAlert(search(lcon,"con_port_state_fail"));
		   break;
	case 1:ShowAlert(search(lcon,"con_port_state_succ"));
   	       break;
	case -1:ShowAlert(search(lcon,"unknown_portno_format"));
		    break;
	case -2:ShowAlert(search(lcon,"port_is_not_trunk_member"));
	   	    break;			
    case -3:ShowAlert(search(lcon,"port_already_enable"));
	    	break;
	case -4:ShowAlert(search(lcon,"port_not_enable"));
			break;	
	case -5:ShowAlert(search(lpublic,"error"));
			break;	
 }   
 free(port_name);
 free(port_state);
}

void Config_load_balance(struct list *lpublic,struct list *lcon)
{
  int ret;	
  char *load_balance=(char *)malloc(15);
  memset(load_balance,0,15);
  cgiFormStringNoNewlines("load_balance",load_balance,15);  

  ccgi_dbus_init();
  ret=set_load_balance(load_balance);     /*mode列表:based-port|based-mac|based-ip|based-L4|mac+ip|mac+L4|ip+L4|mac+ip+L4*/
                                          /*返回0表示失败，返回1表示成功，返回-1表示there no trunk exist*/
                                          /*返回-2表示load-balance Mode same to corrent mode*/
										  /*返回-3表示设备不支持这种模式*/	
  switch(ret)
  {
	case 0:ShowAlert(search(lcon,"con_load_balance_fail"));
		   break;
	case 1:ShowAlert(search(lcon,"con_load_balance_succ"));
   	       break;
	case -1:ShowAlert(search(lcon,"no_trunk"));
		    break;
	case -2:ShowAlert(search(lcon,"same_load_balance"));
	   	    break;	
    //kehao add 2011-04-26			
    /////////////////////////////////////////////////////
	case -3:ShowAlert(search(lcon,"device_unsupported_balance_mode"));
	   	    break;
    /////////////////////////////////////////////////////
	
 }   
 free(load_balance);
}

