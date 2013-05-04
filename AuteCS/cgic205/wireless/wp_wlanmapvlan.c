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
* wp_wlanmapvlan.c
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
#include "ws_usrinfo.h"
#include "ws_err.h"
#include "ws_ec.h"
#include "wcpss/asd/asd.h"
#include "wcpss/wid/WID.h"
#include "dbus/wcpss/dcli_wid_wtp.h"
#include "dbus/wcpss/dcli_wid_wlan.h"
#include "ws_dcli_wlans.h"
#include "ws_dbus_list_interface.h"
#include "ws_init_dbus.h"

int ShowWlanVlanMapPage(char *m,char *n,char *f,char *pn,char *ins_id,instance_parameter *ins_para,struct list *lpublic,struct list *lwlan); 
void WlanVlan_Map(instance_parameter *ins_para,int id,struct list *lpublic,struct list *lwlan);


int cgiMain()
{
  char encry[BUF_LEN] = { 0 };      	  
  char ID[10] = { 0 };
  char flag[10] = { 0 };
  char *str = NULL;      
  char pno[10] = { 0 }; 
  char instance_id[10] = { 0 };
  struct list *lpublic = NULL;   /*解析public.txt文件的链表头*/
  struct list *lwlan = NULL;     /*解析wlan.txt文件的链表头*/  
  instance_parameter *paraHead1 = NULL;
  dbus_parameter ins_para;
  
  lpublic=get_chain_head("../htdocs/text/public.txt");
  lwlan=get_chain_head("../htdocs/text/wlan.txt");  
  
  DcliWInit();
  ccgi_dbus_init();
  memset(encry,0,sizeof(encry));
  memset(ID,0,sizeof(ID));
  memset(flag,0,sizeof(flag));
  memset(pno,0,sizeof(pno)); 
  memset(instance_id,0,sizeof(instance_id));
  if(cgiFormStringNoNewlines("UN", encry, BUF_LEN)!=cgiFormNotFound )  /*首次进入该页*/
  {    
    cgiFormStringNoNewlines("ID", ID, 10);		
	cgiFormStringNoNewlines("FL", flag, 10);	
	cgiFormStringNoNewlines("PN",pno,10);
	cgiFormStringNoNewlines("INSTANCE_ID", instance_id, 10);
  }
  else
  {    
    cgiFormStringNoNewlines("encry_wvmap",encry,BUF_LEN);
    cgiFormStringNoNewlines("wlan_id",ID,10);	  
	cgiFormStringNoNewlines("if_flag",flag,10);	
	cgiFormStringNoNewlines("page_no",pno,10);
	cgiFormStringNoNewlines("instance_id",instance_id,10);  
  }
  
  if(strcmp(instance_id,"")==0)
  {	
	list_instance_parameter(&paraHead1, INSTANCE_STATE_WEB);	
	if(paraHead1)
	{
		snprintf(instance_id,sizeof(instance_id)-1,"%d-%d-%d",paraHead1->parameter.slot_id,paraHead1->parameter.local_id,paraHead1->parameter.instance_id); 
	}
  }
  else
  {
	get_slotID_localID_instanceID(instance_id,&ins_para);	
	get_instance_dbus_connection(ins_para, &paraHead1, INSTANCE_STATE_WEB);
  }

  str=dcryption(encry);
  if(str==NULL)
    ShowErrorPage(search(lpublic,"ill_user"));          /*用户非法*/
  else
    ShowWlanVlanMapPage(encry,ID,flag,pno,instance_id,paraHead1,lpublic,lwlan);
  
  release(lpublic);  
  release(lwlan);
  free_instance_parameter_list(&paraHead1);
  destroy_ccgi_dbus();
  return 0;
}

int ShowWlanVlanMapPage(char *m,char *n,char *f,char *pn,char *ins_id,instance_parameter *ins_para,struct list *lpublic,struct list *lwlan)
{  
  FILE *fp = NULL;
  int i = 0,limit = 0,status = 1;  
  char *endptr = NULL;  
  int wlan_id = 0;
  int result1 = 0;
  DCLI_WLAN_API_GROUP *wlan_vlan = NULL;
  char BindInter[20] = { 0 };
  char *retu = NULL;
  int result2 = 0;
  DCLI_WLAN_API_GROUP *tunnel_wlan_vlan = NULL;
  struct WID_TUNNEL_WLAN_VLAN *head = NULL;
  int twv_num = 0;
  char command[30] = { 0 };
  cgiHeaderContentType("text/html");
  fprintf(cgiOut,"<html xmlns=\"http://www.w3.org/1999/xhtml\"><head>");
  fprintf(cgiOut,"<meta http-equiv=Content-Type content=text/html; charset=gb2312>");
  fprintf(cgiOut,"<title>WLAN</title>");
  fprintf(cgiOut,"<link rel=stylesheet href=/style.css type=text/css>"\
  "</head>"\
  "<body>");
  wlan_id= strtoul(n,&endptr,10);    /*char转成int，10代表十进制*/	
  if(cgiFormSubmitClicked("wvmap_apply") == cgiFormSuccess)
  {
  	if(ins_para)
	{
		WlanVlan_Map(ins_para,wlan_id,lpublic,lwlan);
	} 
  }  
  fprintf(cgiOut,"<form method=post>"\
  "<div align=center>"\
  "<table width=976 border=0 cellpadding=0 cellspacing=0>"\
  "<tr>"\
    "<td width=8 align=left valign=top background=/images/di22.jpg><img src=/images/youce4.jpg width=8 height=30/></td>"\
    "<td width=51 align=left valign=bottom background=/images/di22.jpg><img src=/images/youce33.jpg width=37 height=24/></td>"\
    "<td width=153 align=left valign=bottom id=titleen background=/images/di22.jpg>WLAN</td>"\
    "<td width=690 align=right valign=bottom background=/images/di22.jpg>");
        
          fprintf(cgiOut,"<table width=155 border=0 cellspacing=0 cellpadding=0>"\
          "<tr>"\
          "<td width=62 align=center><input id=but type=submit name=wvmap_apply style=background-image:url(/images/%s) value=""></td>",search(lpublic,"img_ok"));
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
					"<td align=left id=tdleft background=/images/bottom_bg.gif style=\"border-right:0\"><font id=yingwen_san>WLAN_VLAN</font><font id=%s>%s</font></td>",search(lpublic,"menu_san"),search(lpublic,"map"));	 /*突出显示*/
				  fprintf(cgiOut,"</tr>"\
				  "<tr height=25>"\
					"<td align=left id=tdleft><a href=wp_wlannew.cgi?UN=%s target=mainFrame class=top><font id=%s>%s</font><font id=yingwen_san> WLAN</font></a></td>",m,search(lpublic,"menu_san"),search(lpublic,"create"));						 
				  fprintf(cgiOut,"</tr>");
					 fprintf(cgiOut,"<tr height=25>"\
					  "<td align=left id=tdleft><a href=wp_wlanbw.cgi?UN=%s target=mainFrame class=top><font id=yingwen_san>MAC</font><font id=%s> %s</font></a></td>",m,search(lpublic,"menu_san"),search(lwlan,"mac_filter"));					   
				  fprintf(cgiOut,"</tr>");
				  if(strcmp(f,"NO_IF")==0)  /*local模式*/
				  	limit=3;
				  else
				  	limit=5;
                  for(i=0;i<limit;i++)
                  {
                    fprintf(cgiOut,"<tr height=25>"\
                      "<td id=tdleft>&nbsp;</td>"\
                    "</tr>");
                  }     						
				  if(ins_para)
				  {
					  result1=show_wlan_vlan_info(ins_para->parameter,ins_para->connection,wlan_id,&wlan_vlan);
					  result2=show_tunnel_wlan_vlan_cmd_func(ins_para->parameter,ins_para->connection,wlan_id,&tunnel_wlan_vlan);
				  } 
				  if(result2 == 1)
				  {
				  	if((tunnel_wlan_vlan)&&(tunnel_wlan_vlan->WLAN[0]))
					{
						for(head = tunnel_wlan_vlan->WLAN[0]->tunnel_wlan_vlan; (NULL != head); head = head->ifnext)
						{
							twv_num++;
						}
					}
				  }
				  
                fprintf(cgiOut,"</table>"\
              "</td>"\
              "<td align=left style=\"background-color:#ffffff; border-right:1px solid #707070; padding-left:30px\">"\
                      "<table width=320 border=0 cellspacing=0 cellpadding=0>"\
						"<tr>"\
						  "<td id=ins_style>%s:%s</td>",search(lpublic,"instance"),ins_id);
						fprintf(cgiOut,"</tr>"\
                		"<tr>"\
                          "<td><table width=320 border=0 cellspacing=0 cellpadding=0>"\
                               "<tr height=30 align=left>");
							   if(strcmp(f,"NO_IF")==0)  /*local模式*/
                                 fprintf(cgiOut,"<td id=thead5 align=left>Local%sWLAN_VLAN%s</td>",search(lwlan,"under_mode"),search(lpublic,"map"));		
							   else
							   	 fprintf(cgiOut,"<td id=thead5 align=left>Tunnel%sWLAN_VLAN%s</td>",search(lwlan,"under_mode"),search(lpublic,"map"));		
                               fprintf(cgiOut,"</tr>"\
                        	   "</table>"\
                		  "</td>"\
                		"</tr>"\
                        "<tr><td align=center style=\"padding-left:20px\"><table width=320 border=0 cellspacing=0 cellpadding=0>");
					if(strcmp(f,"NO_IF")==0)  /*local模式*/
					{
						fprintf(cgiOut,"<tr height=30>"\
					  	  "<td width=120>WLAN_VLAN%s:</td>",search(lpublic,"map"));
	                      fprintf(cgiOut,"<td align=left width=100><input type=text name=local_wv_map size=10 maxLength=4 onkeypress=\"return event.keyCode>=48&&event.keyCode<=57\"></td>"\
	                      "<td align=left width=100><font color=red>(1--4094)</font></td>"\
	                    "</tr>"\
	                    "<tr height=30>"\
	                      "<td>WLAN_VLAN%s:</td>",search(lpublic,"unmap"));
	                      fprintf(cgiOut,"<td align=left colspan=2>"\
							"<select name=local_wv_unmap id=local_wv_unmap style=width:72px>");
							fprintf(cgiOut,"<option value=>");
							if((result1==1)&&(wlan_vlan)&&(wlan_vlan->WLAN[0])&&(wlan_vlan->WLAN[0]->vlanid!=0))
							{
								fprintf(cgiOut,"<option value=%d>%d",wlan_vlan->WLAN[0]->vlanid,wlan_vlan->WLAN[0]->vlanid);
							}
							fprintf(cgiOut,"</select>"\
						  "</td>"\
	                    "</tr>"\
	                    "<tr height=30>"\
	                      "<td>%s:</td>",search(lpublic,"priority"));
	                    fprintf(cgiOut,"<td align=left><input type=text name=wv_pri size=10 maxLength=1 onkeypress=\"return event.keyCode>=48&&event.keyCode<=57\"></td>"\
	                    "<td align=left><font color=red>(0--7)</font></td>"\
	                   "</tr>");
				  	}
					else
					{
						fprintf(cgiOut,"<tr height=30>"\
					  	  "<td width=120>WLAN %s:</td>",search(lwlan,"isolate_state"));
	                      fprintf(cgiOut,"<td align=left width=100>"\
						  	"<select name=wlan_iso_state id=wlan_iso_state style=width:100px>"\
						  	"<option value=>"\
		  				    "<option value=enable>enable"\
		  				    "<option value=disable>disable"\
						  "</select></td>"\
						  "<td align=left width=100>&nbsp;</td>"\
	                    "</tr>"\
	                    "<tr height=30>"\
	                      "<td>WLAN %s:</td>",search(lwlan,"mult_state"));
	                    fprintf(cgiOut,"<td align=left colspan=2>"\
						  "<select name=wlan_mult_state id=wlan_mult_state style=width:100px>"\
						  	"<option value=>"\
		  				    "<option value=enable>enable"\
		  				    "<option value=disable>disable"\
						"</select></td>"\
	                   "</tr>"\
	                   "<tr height=30>"\
	                      "<td>%s:</td>",search(lwlan,"bind_interface"));
	                    fprintf(cgiOut,"<td align=left colspan=2>");
						memset(command,0,sizeof(command));
						strncpy(command,"WlanMapVlan.sh ",sizeof(command)-1);
						strncat(command,n,sizeof(command)-strlen(command)-1);		
						status = system(command); 	
						if(status==0)
						{
						  fprintf(cgiOut,"<select name=bind_interface id=bind_interface style=width:100px>"\
						    "<option value=>");
						    if((fp=fopen("/var/run/apache2/WlanMapVlan.tmp","r"))==NULL)		 /*以只读方式打开资源文件*/
							{
								ShowAlert(search(lpublic,"error_open"));
						    }
							else
							{
								memset(BindInter,0,sizeof(BindInter));
								retu=fgets(BindInter,20,fp);
								while(retu!=NULL)
								{
								  fprintf(cgiOut,"<option value=%s>%s",retu,retu);
								  memset(BindInter,0,sizeof(BindInter));
								  retu=fgets(BindInter,20,fp);
								}				   
								fclose(fp); 
							}
						  fprintf(cgiOut,"</select>");	
						}
						else
						{
		  				  fprintf(cgiOut,"%s",search(lpublic,"exec_shell_fail"));
						}
						fprintf(cgiOut,"</td>"\
	                   "</tr>"\
	                   "<tr height=30>"\
	                      "<td>%s:</td>",search(lwlan,"unbind_interface"));
	                    fprintf(cgiOut,"<td align=left colspan=2>"\
						  "<select name=unbind_interface id=unbind_interface style=width:100px>"\
						  "<option value=>");
						  if((result2==1)&&(twv_num>0))
						  {
						    if((tunnel_wlan_vlan)&&(tunnel_wlan_vlan->WLAN[0]))
							{
								for(head = tunnel_wlan_vlan->WLAN[0]->tunnel_wlan_vlan; (NULL != head); head = head->ifnext)
								{
									fprintf(cgiOut,"<option value=%s>%s",head->ifname,head->ifname);
								}
							}
						  }
 						fprintf(cgiOut,"</select></td>"\
	                   "</tr>"\
					   "<tr height=30>"\
					  	  "<td>%s:</td>",search(lwlan,"spswitch"));
	                      fprintf(cgiOut,"<td align=left colspan=2>"\
						  	"<select name=sameport_switch id=sameport_switch style=width:100px>"\
						  	"<option value=>"\
		  				    "<option value=enable>enable"\
		  				    "<option value=disable>disable"\
						  "</select></td>"\
	                    "</tr>");
					}
			 	  fprintf(cgiOut,"<tr>"\
                    "<td><input type=hidden name=encry_wvmap value=%s></td>",m);
                    fprintf(cgiOut,"<td><input type=hidden name=wlan_id value=%s></td>",n);		
					fprintf(cgiOut,"<td><input type=hidden name=if_flag value=%s></td>",f);	
                    fprintf(cgiOut,"</tr>"\
				  "<tr>"\
                "<td><input type=hidden name=page_no value=%s></td>",pn);
				fprintf(cgiOut,"<td colspan=2><input type=hidden name=instance_id value=%s></td>",ins_id);
	          fprintf(cgiOut,"</tr>"\
                "</table></td></tr>"\
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
{
  Free_wlan_vlan_info(wlan_vlan);
}
if(result2==1)
{
  Free_tunnel_wlan_vlan_head(tunnel_wlan_vlan);
}
return 0;
}

void WlanVlan_Map(instance_parameter *ins_para,int id,struct list *lpublic,struct list *lwlan)
{
  int ret = 0,flag = 1,all_null = 0;  /*all_null==0表示没有做任何操作*/
  char *endptr = NULL;
  char temp[100] = { 0 };
  char local_wv_map[10] = { 0 };
  char local_wv_unmap[10] = { 0 };
  char wv_pri[10] = { 0 };
  char wlan_iso_state[10] = { 0 };
  char wlan_mult_state[10] = { 0 };
  char interf[20] = { 0 };
  char state[20] = { 0 };  
  char max_wlan_num[10] = { 0 };

  /****************set wlan vlan id****************/
   memset(local_wv_map,0,sizeof(local_wv_map));
   cgiFormStringNoNewlines("local_wv_map",local_wv_map,10);
   if(strcmp(local_wv_map,""))
   {
     all_null=1;
   	 ret=set_wlan_vlan_id(ins_para->parameter,ins_para->connection,id,local_wv_map); /*返回0表示失败，返回1表示成功，返回-1表示unknown input*/
																					/*返回-2表示input parameter should be 1 to 4094，返回-3表示wlan id does not exist*/
																					/*返回-4表示wlan is in other L3 interface，返回-5表示wlan should be disable first*/
																					/*返回-6表示error*/
     switch(ret)
     {
       case SNMPD_CONNECTION_ERROR:
       case 0:{
            	flag=0;
				memset(temp,0,sizeof(temp));
				strncpy(temp,"WLAN_VLAN",sizeof(temp)-1);
				strncat(temp,search(lpublic,"map_fail"),sizeof(temp)-strlen(temp)-1);
                ShowAlert(temp);
            	break;
        	  }
       case 1:break;
       case -1:{
            	 flag=0;
                 ShowAlert(search(lpublic,"unknown_input"));
            	 break;
        	   }
       case -2:{
            	 flag=0;
                 memset(temp,0,sizeof(temp));
                 strncpy(temp,search(lpublic,"input_para_1to"),sizeof(temp)-1);
                 strncat(temp,"4094",sizeof(temp)-strlen(temp)-1);
                 strncat(temp,search(lwlan,"qos_id_2"),sizeof(temp)-strlen(temp)-1);
                 ShowAlert(temp);
            	 break;
        	   }
       case -3:{
            	 flag=0;
                 ShowAlert(search(lwlan,"wlan_not_exist"));
            	 break;
        	   }
       case -4:{
            	 flag=0;
                 ShowAlert(search(lwlan,"wlan_in_oth_l3"));
            	 break;
        	   }
	   case -5:{
            	 flag=0;
                 ShowAlert(search(lwlan,"dis_wlan"));
            	 break;
        	   }
	   case -6:{
	   			 flag=0;
				 ShowAlert(search(lpublic,"error"));			   
				 break; 
			   }
     }
   }

  /****************undo wlan vlan cmd****************/
   memset(local_wv_unmap,0,sizeof(local_wv_unmap));
   cgiFormStringNoNewlines("local_wv_unmap",local_wv_unmap,10);
   if(strcmp(local_wv_unmap,""))
   {
     all_null=1;
   	 ret=undo_wlan_vlan_cmd(ins_para->parameter,ins_para->connection,id);/*返回0表示失败，返回1表示成功，返回-1表示wlan id does not exist*/
																		/*返回-2表示wlan is in other L3 interface，返回-3表示wlan should be disable first，返回-4表示error*/
     switch(ret)
     {
       case SNMPD_CONNECTION_ERROR:
       case 0:{
            	flag=0;
				memset(temp,0,sizeof(temp));
				strncpy(temp,"WLAN_VLAN",sizeof(temp)-1);
				strncat(temp,search(lpublic,"unmap_fail"),sizeof(temp)-strlen(temp)-1);
                ShowAlert(temp);
            	break;
        	  }
       case 1:break;
       case -1:{
            	 flag=0;
                 ShowAlert(search(lwlan,"wlan_not_exist"));
            	 break;
        	   }
       case -2:{
            	 flag=0;
                 ShowAlert(search(lwlan,"wlan_in_oth_l3"));
            	 break;
        	   }
       case -3:{
            	 flag=0;
                 ShowAlert(search(lwlan,"dis_wlan"));
            	 break;
        	   }
       case -4:{
	   			 flag=0;
				 ShowAlert(search(lpublic,"error"));			   
				 break; 
			   }
     }
   }

   

   /****************set wlan vlan priority****************/
   memset(wv_pri,0,sizeof(wv_pri));
   cgiFormStringNoNewlines("wv_pri",wv_pri,10);
   if(strcmp(wv_pri,"")!=0) 
   {
     all_null=1;
   	 ret=set_wlan_vlan_priority(ins_para->parameter,ins_para->connection,id,wv_pri); /*返回0表示失败，返回1表示成功，返回-1表示unknown input*/
																					/*返回-2表示input parameter should be 0 to 7，返回-3表示wlan id does not exist*/
																					/*返回-4表示wlan is in other L3 interface，返回-5表示wlan has not binding vlan*/
																					/*返回-6表示wlan should be disable first，返回-7表示wlan is under tunnel wlan-vlan policy*/
																					/*返回-8表示error*/
	 switch(ret)
	 {
	   case SNMPD_CONNECTION_ERROR:
	   case 0:{
	   			flag=0;
				ShowAlert(search(lwlan,"set_wv_pri_fail"));
				break;
			  }
	   case 1:break;   
	   case -1:{
	   			 flag=0;
				 ShowAlert(search(lpublic,"unknown_input"));			   
				 break; 
			   }
	   case -2:{
            	 flag=0;
                 memset(temp,0,sizeof(temp));
                 strncpy(temp,search(lpublic,"input_para_0to"),sizeof(temp)-1);
                 strncat(temp,"7",sizeof(temp)-strlen(temp)-1);
                 strncat(temp,search(lwlan,"qos_id_2"),sizeof(temp)-strlen(temp)-1);
                 ShowAlert(temp);
            	 break;
        	   }
	   case -3:{
            	 flag=0;
                 ShowAlert(search(lwlan,"wlan_not_exist"));
            	 break;
        	   }
	   case -4:{
            	 flag=0;
                 ShowAlert(search(lwlan,"wlan_in_oth_l3"));
            	 break;
        	   }
	   case -5:{
            	 flag=0;
                 ShowAlert(search(lwlan,"wlan_nobind_vlan"));
            	 break;
        	   }
	   case -6:{
            	 flag=0;
                 ShowAlert(search(lwlan,"dis_wlan"));
            	 break;
        	   }
	   case -7:{
            	 flag=0;
                 ShowAlert(search(lwlan,"wlan_under_tunnel_mode"));
            	 break;
        	   }
	   case -8:{
	   			 flag=0;
				 ShowAlert(search(lpublic,"error"));			   
				 break; 
			   }
	 }
   }


   /****************wlan set bridge isolation func****************/
   memset(wlan_iso_state,0,sizeof(wlan_iso_state));
   cgiFormStringNoNewlines("wlan_iso_state",wlan_iso_state,10);
   if(strcmp(wlan_iso_state,"")!=0) 
   {
     all_null=1;
     ret=wlan_set_bridge_isolation_func(ins_para->parameter,ins_para->connection,id,wlan_iso_state); /*返回0表示失败，返回1表示成功，返回-1表示input parameter should only be 'enable' or 'disable'*/
																									/*返回-2表示wlan id does not exist，返回-3表示wlan is not wlan if policy，返回-4表示wlan should be disable first*/
																									/*返回-5表示wlan bridge error，返回-6表示system cmd process error*/
																									/*返回-7表示sameportswitch and isolation are conflict,disable sameportswitch first，返回-8表示error*/
																									/*返回-9示WLAN ID非法，返回-10表示apply security in this wlan first*/
																									/*返回SNMPD_CONNECTION_ERROR表示connection error*/
	 switch(ret)
	 {
	   case SNMPD_CONNECTION_ERROR:
	   case 0:{
	   			flag=0;
				ShowAlert(search(lwlan,"con_wlan_isolate_fail"));
				break;
			  }
	   case 1:break;   
	   case -1:{
	   			 flag=0;
				 ShowAlert(search(lpublic,"input_para_error"));			   
				 break; 
			   }
	   case -2:{
            	 flag=0;
                 ShowAlert(search(lwlan,"wlan_not_exist"));
            	 break;
        	   }
	   case -3:{
            	 flag=0;
                 ShowAlert(search(lwlan,"wlan_no_ifpocy"));
            	 break;
        	   }
	   case -4:{
            	 flag=0;
                 ShowAlert(search(lwlan,"dis_wlan"));
            	 break;
        	   }
	   case -5:{
            	 flag=0;
                 ShowAlert(search(lwlan,"wlan_brg_error"));
            	 break;
        	   }
	   case -6:{
            	 flag=0;
                 ShowAlert(search(lpublic,"sys_cmd_error"));
            	 break;
        	   }
	   case -7:{
            	 flag=0;
                 ShowAlert(search(lwlan,"spswi_iso_conflict"));
            	 break;
        	   }
	   case -8:{
	   			 flag=0;
				 ShowAlert(search(lpublic,"error"));			   
				 break; 
			   }
	   case -9:{													/*WLAN ID非法*/
			      flag=0;
		          memset(temp,0,sizeof(temp));
			      strncpy(temp,search(lwlan,"wlan_id_illegal1"),sizeof(temp)-1);
			      memset(max_wlan_num,0,sizeof(max_wlan_num));
			      snprintf(max_wlan_num,sizeof(max_wlan_num)-1,"%d",WLAN_NUM-1);
			      strncat(temp,max_wlan_num,sizeof(temp)-strlen(temp)-1);
			      strncat(temp,search(lwlan,"wlan_id_illegal2"),sizeof(temp)-strlen(temp)-1);
			      ShowAlert(temp);
			      break;
		        }
	   case -10:{
            	  flag=0;
                  ShowAlert(search(lwlan,"con_bind"));
            	  break;
        	    }
	 }
   }


   
   /****************wlan set bridge multicast  isolation func****************/
  memset(wlan_mult_state,0,sizeof(wlan_mult_state));
  cgiFormStringNoNewlines("wlan_mult_state",wlan_mult_state,10);
  if(strcmp(wlan_mult_state,"")!=0) 
  {
    all_null=1;
	ret=wlan_set_bridge_multicast_isolation_func(ins_para->parameter,ins_para->connection,id,wlan_mult_state);   /*返回0表示失败，返回1表示成功，返回-1表示input parameter should only be 'enable' or 'disable'*/
																												/*返回-2表示wlan id does not exist，返回-3表示wlan is not wlan if policy，返回-4表示wlan should be disable first*/
																												/*返回-5表示wlan bridge error，返回-6表示system cmd process error*/
																												/*返回-7表示sameportswitch and isolation are conflict,disable sameportswitch first，返回-8表示error*/
																												/*返回-9示WLAN ID非法，返回-10表示apply security in this wlan first*/
																												/*返回SNMPD_CONNECTION_ERROR表示connection error*/
	switch(ret)
	{
	  case SNMPD_CONNECTION_ERROR:
	  case 0:{
			   flag=0;
			   ShowAlert(search(lwlan,"con_wlan_mult_fail"));
			   break;
			 }
	  case 1:break;   
	  case -1:{
				flag=0;
				ShowAlert(search(lpublic,"input_para_error"));			  
				break; 
			  }
	  case -2:{
				flag=0;
				ShowAlert(search(lwlan,"wlan_not_exist"));
				break;
			  }
	  case -3:{
				flag=0;
				ShowAlert(search(lwlan,"wlan_no_ifpocy"));
				break;
			  }
	  case -4:{
				flag=0;
				ShowAlert(search(lwlan,"dis_wlan"));
				break;
			  }
	  case -5:{
				flag=0;
				ShowAlert(search(lwlan,"wlan_brg_error"));
				break;
			  }
	  case -6:{
				flag=0;
				ShowAlert(search(lpublic,"sys_cmd_error"));
				break;
			  }
	  case -7:{
				flag=0;
				ShowAlert(search(lwlan,"spswi_iso_conflict"));
				break;
			  }
	  case -8:{
				flag=0;
				ShowAlert(search(lpublic,"error")); 			  
				break; 
			  }
	  case -9:{													/*WLAN ID非法*/
		        flag=0;
	            memset(temp,0,sizeof(temp));
		        strncpy(temp,search(lwlan,"wlan_id_illegal1"),sizeof(temp)-1);
		        memset(max_wlan_num,0,sizeof(max_wlan_num));
		        snprintf(max_wlan_num,sizeof(max_wlan_num)-1,"%d",WLAN_NUM-1);
		        strncat(temp,max_wlan_num,sizeof(temp)-strlen(temp)-1);
		        strncat(temp,search(lwlan,"wlan_id_illegal2"),sizeof(temp)-strlen(temp)-1);
		        ShowAlert(temp);
		        break;
	          }
	   case -10:{
            	  flag=0;
                  ShowAlert(search(lwlan,"con_bind"));
            	  break;
        	    }
	}
  }


  /***********************set tunnel wlan vlan uplink**************************/  
  memset(interf,0,sizeof(interf));
  cgiFormStringNoNewlines("bind_interface",interf,20);
  if(strcmp(interf,"")!=0) 
  {
      all_null=1;
	  ret=set_tunnel_wlan_vlan_cmd_func(ins_para->parameter,ins_para->connection,id,"add",interf);/*返回0表示失败，返回1表示成功，返回-1表示input parameter should only be 'add' or 'delete'*/
										  														 /*返回-2表示input interface name should only start with 'radio',other interface you should use ebr configuration*/
																								 /*返回-3表示if name too long，返回-4表示malloc error，返回-5表示wlan id does not exist*/
																								 /*返回-6表示wlan is in local wlan-vlan interface，返回-7表示wlan should be disable first，返回-8表示input ifname is wrong*/
																								 /*返回-9表示wlan is not in tunnel mode，返回-10表示if is already STATE,or system cmd error，返回-11表示error*/
	  switch(ret)
	  {
	    case SNMPD_CONNECTION_ERROR:
	  	case 0:ShowAlert(search(lwlan,"bind_interface_fail"));
			   flag=0;
			   break;
		case 1:break;
		case -1:ShowAlert(search(lpublic,"input_para_error"));
			    flag=0;
			    break;
		case -2:ShowAlert(search(lwlan,"if_start_with_radio"));
				flag=0;
				break;
		case -3:ShowAlert(search(lpublic,"if_name_long"));
			    flag=0;
			    break;
		case -4:ShowAlert(search(lpublic,"malloc_error"));
			    flag=0;
			    break;				
		case -5:ShowAlert(search(lwlan,"wlan_not_exist"));
			    flag=0;
			    break;
		case -6:ShowAlert(search(lwlan,"wlan_under_local_mode"));
			    flag=0;
			    break;
		case -7:ShowAlert(search(lwlan,"dis_wlan"));
			    flag=0;
			    break;
		case -8:ShowAlert(search(lpublic,"input_ifname_error"));
			    flag=0;
			    break;
		case -9:ShowAlert(search(lwlan,"wlan_notin_tunmode"));
			    flag=0;
			    break;				
		case -10:ShowAlert(search(lwlan,"if_is_uplink_or_cmd_error"));
			    flag=0;
			    break;
		case -11:ShowAlert(search(lpublic,"error"));
			     flag=0;
			     break;
	  }
  }   


  
  /***********************set tunnel wlan vlan downlink**************************/  
	memset(interf,0,sizeof(interf));
	cgiFormStringNoNewlines("unbind_interface",interf,20);
	if(strcmp(interf,"")!=0) 
	{
		all_null=1;
		ret=set_tunnel_wlan_vlan_cmd_func(ins_para->parameter,ins_para->connection,id,"delete",interf);/*返回0表示失败，返回1表示成功，返回-1表示input parameter should only be 'add' or 'delete'*/
																									  /*返回-2表示input interface name should only start with 'radio',other interface you should use ebr configuration*/
																									  /*返回-3表示if name too long，返回-4表示malloc error，返回-5表示wlan id does not exist*/
																									  /*返回-6表示wlan is in local wlan-vlan interface，返回-7表示wlan should be disable first，返回-8表示input ifname is wrong*/
																									  /*返回-9表示wlan is not in tunnel mode，返回-10表示if is already STATE,or system cmd error，返回-11表示error*/
		switch(ret)
		{
		  case SNMPD_CONNECTION_ERROR:
		  case 0:ShowAlert(search(lwlan,"unbind_inter_fail"));
				 flag=0;
				 break;
		  case 1:break;
		  case -1:ShowAlert(search(lpublic,"input_para_error"));
				  flag=0;
				  break;
		  case -2:ShowAlert(search(lwlan,"if_start_with_radio"));
				  flag=0;
				  break;
		  case -3:ShowAlert(search(lpublic,"if_name_long"));
				  flag=0;
				  break;
		  case -4:ShowAlert(search(lpublic,"malloc_error"));
				  flag=0;
				  break;			  
		  case -5:ShowAlert(search(lwlan,"wlan_not_exist"));
				  flag=0;
				  break;
		  case -6:ShowAlert(search(lwlan,"wlan_under_local_mode"));
				  flag=0;
				  break;
		  case -7:ShowAlert(search(lwlan,"dis_wlan"));
				  flag=0;
				  break;
		  case -8:ShowAlert(search(lpublic,"input_ifname_error"));
				  flag=0;
				  break;
		  case -9:ShowAlert(search(lwlan,"wlan_notin_tunmode"));
				  flag=0;
				  break;			  
		  case -10:ShowAlert(search(lwlan,"if_is_downlink_or_cmd_error"));
				  flag=0;
				  break;
		  case -11:ShowAlert(search(lpublic,"error"));
				   flag=0;
				   break;
		}
	}	

  
  /***********************wlan set sameportswitch func**************************/  
  memset(state,0,sizeof(state));
  cgiFormStringNoNewlines("sameport_switch",state,20); 
  if(strcmp(state,"")!=0)
  {
  	  all_null=1;
	  ret=wlan_set_sameportswitch_func(ins_para->parameter,ins_para->connection,id,state);   /*返回0表示失败，返回1表示成功，返回-1表示input parameter should only be 'enable' or 'disable'*/
																							/*返回-2表示wlan id does not exist，返回-3表示wlan is not wlan if policy，返回-4表示wlan should be disable first*/
																							/*返回-5表示wlan bridge error，返回-6表示system cmd process error*/
																							/*返回-7表示sameportswitch and isolation are conflict,disable isolation first，返回-8表示error*/
																							/*返回-9示WLAN ID非法，返回-10表示apply security in this wlan first*/
	  switch(ret)
	  {
	    case SNMPD_CONNECTION_ERROR:
		case 0:ShowAlert(search(lwlan,"con_spswitch_fail"));
			   flag=0;	
			   break;
		case 1:break;
		case -1:ShowAlert(search(lpublic,"input_para_error"));
				flag=0;
				break;
		case -2:ShowAlert(search(lwlan,"wlan_not_exist"));
				flag=0;
				break;
		case -3:ShowAlert(search(lwlan,"wlan_no_ifpocy"));
				flag=0;
				break;
		case -4:ShowAlert(search(lwlan,"dis_wlan"));
				flag=0;
				break;
		case -5:ShowAlert(search(lwlan,"wlan_brg_error"));
				flag=0;
				break;
		case -6:ShowAlert(search(lpublic,"sys_cmd_error"));
				flag=0;
				break;
		case -7:ShowAlert(search(lwlan,"spswi_iso_conflict1"));
				flag=0;
				break;
		case -8:ShowAlert(search(lpublic,"error"));
				flag=0;
				break;
		case -9:{													/*WLAN ID非法*/
		          flag=0;
	              memset(temp,0,sizeof(temp));
		          strncpy(temp,search(lwlan,"wlan_id_illegal1"),sizeof(temp)-1);
		          memset(max_wlan_num,0,sizeof(max_wlan_num));
		          snprintf(max_wlan_num,sizeof(max_wlan_num)-1,"%d",WLAN_NUM-1);
		          strncat(temp,max_wlan_num,sizeof(temp)-strlen(temp)-1);
		          strncat(temp,search(lwlan,"wlan_id_illegal2"),sizeof(temp)-strlen(temp)-1);
		          ShowAlert(temp);
		          break;
	            }
	    case -10:{
            	  flag=0;
                  ShowAlert(search(lwlan,"con_bind"));
            	  break;
        	    }
	  }
  }

  if((flag==1)&&(all_null==1))
    ShowAlert(search(lpublic,"oper_succ"));
}


