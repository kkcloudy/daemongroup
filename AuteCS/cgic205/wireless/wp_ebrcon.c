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
* wp_ebrcon.c
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
#include "wcpss/wid/WID.h"
#include "ws_dcli_ebr.h"
#include "ws_dcli_wlans.h"
#include "ws_dbus_list_interface.h"


int ShowEbrconPage(char *m,char *n,char * instance_id,char *pn,instance_parameter *ins_para,struct list *lpublic,struct list *lwlan); 
void config_ebr(instance_parameter *ins_para,int id,struct list *lpublic,struct list *lwlan);


int cgiMain()
{
  char encry[BUF_LEN] = { 0 };			  
  char ID[10] = { 0 };
  char pno[10] = { 0 };  
  char *str = NULL;
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
  memset(pno,0,sizeof(pno));
  memset(instance_id,0,sizeof(instance_id));
  if(cgiFormStringNoNewlines("UN", encry, BUF_LEN)!=cgiFormNotFound )  /*首次进入该页*/
  {    
	cgiFormStringNoNewlines("ID", ID, 10);		
	cgiFormStringNoNewlines("PN",pno,10);
	cgiFormStringNoNewlines("INSTANCE_ID", instance_id, 10);
  }
  else
  {    
	cgiFormStringNoNewlines("encry_conebr",encry,BUF_LEN);
	cgiFormStringNoNewlines("ebr_id",ID,10);	  
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
	ShowErrorPage(search(lpublic,"ill_user"));			/*用户非法*/
  else
	ShowEbrconPage(encry,ID,instance_id,pno,paraHead1,lpublic,lwlan);
  
  release(lpublic);  
  release(lwlan);  
  free_instance_parameter_list(&paraHead1);
  destroy_ccgi_dbus();
  return 0;
}

int ShowEbrconPage(char *m,char *n,char * instance_id,char *pn,instance_parameter *ins_para,struct list *lpublic,struct list *lwlan)
{  
  FILE *fp = NULL;
  int i = 0,status = -1;
  char BindInter[20] = { 0 };
  char *endptr = NULL;
  char *retu = NULL;
  int ebrID = 0;
  int result = 0;
  DCLI_EBR_API_GROUP *ebrinfo = NULL;
  EBR_IF_LIST *head = NULL;
  
  ebrID= strtoul(n,&endptr,10);	  /*char转成int，10代表十进制*/  
  cgiHeaderContentType("text/html");
  fprintf(cgiOut,"<html xmlns=\"http://www.w3.org/1999/xhtml\"><head>");
  fprintf(cgiOut,"<meta http-equiv=Content-Type content=text/html; charset=gb2312>");
  fprintf(cgiOut,"<title>Wtp</title>");
  fprintf(cgiOut,"<link rel=stylesheet href=/style.css type=text/css>"\
  "</head>");   
  if(cgiFormSubmitClicked("ebrcon_apply") == cgiFormSuccess)
  {
  	if(ins_para)
	{
		config_ebr(ins_para,ebrID,lpublic,lwlan);
	}
  }  
  fprintf(cgiOut,"<body>"\
  "<form method=post>"\
  "<div align=center>"\
  "<table width=976 border=0 cellpadding=0 cellspacing=0>"\
  "<tr>"\
    "<td width=8 align=left valign=top background=/images/di22.jpg><img src=/images/youce4.jpg width=8 height=30/></td>"\
    "<td width=51 align=left valign=bottom background=/images/di22.jpg><img src=/images/youce33.jpg width=37 height=24/></td>"\
    "<td width=153 align=left valign=bottom background=/images/di22.jpg><font id=titleen>EBR</font><font id=%s> %s</font></td>",search(lpublic,"title_style"),search(lpublic,"management"));
    fprintf(cgiOut,"<td width=690 align=right valign=bottom background=/images/di22.jpg>");
    	  fprintf(cgiOut,"<table width=155 border=0 cellspacing=0 cellpadding=0>"\
          "<tr>"\
          "<td width=62 align=center><input id=but type=submit name=ebrcon_apply style=background-image:url(/images/%s) value=""></td>",search(lpublic,"img_ok"));
   fprintf(cgiOut,"<td width=62 align=center><a href=wp_ebrlis.cgi?UN=%s&PN=%s&INSTANCE_ID=%s target=mainFrame><img src=/images/%s border=0 width=62 height=20/></a></td>",m,pn,instance_id,search(lpublic,"img_cancel"));
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
                    "<td align=left id=tdleft background=/images/bottom_bg.gif style=\"border-right:0\"><font id=%s>%s</font><font id=yingwen_san> EBR</font></td>",search(lpublic,"menu_san"),search(lpublic,"config"));   /*突出显示*/
                  fprintf(cgiOut,"</tr>"\
                  "<tr height=25>"\
					"<td align=left id=tdleft><a href=wp_ebrnew.cgi?UN=%s target=mainFrame class=top><font id=%s>%s</font><font id=yingwen_san> EBR</font></a></td>",m,search(lpublic,"menu_san"),search(lpublic,"create"));                       
				  fprintf(cgiOut,"</tr>");
				  for(i=0;i<10;i++)
	              {
  				    fprintf(cgiOut,"<tr height=25>"\
                      "<td id=tdleft>&nbsp;</td>"\
                    "</tr>");
	              }
                fprintf(cgiOut,"</table>"\
              "</td>"\
              "<td align=left style=\"background-color:#ffffff; border-right:1px solid #707070; padding-left:30px\">"\
                      "<table width=620 border=0 cellspacing=0 cellpadding=0>");
		          fprintf(cgiOut,"<tr>"\
                     "<td id=ins_style>%s:%s</td>",search(lpublic,"instance"),instance_id);
                  fprintf(cgiOut,"</tr>");
				  fprintf(cgiOut,"<tr>"\
						  "<td>");
				  status = system("ebr_bind_inter.sh"); 				  
	             fprintf(cgiOut,"<table width=620 border=0 cellspacing=0 cellpadding=0>"\
				   "<tr height=30 align=left>"\
				   "<td id=thead5 align=left>%s EBR %d</td>",search(lpublic,"configure"),ebrID);		
				   fprintf(cgiOut,"</tr>"\
				   "</table>"\
				  "</td>"\
				"</tr>"\
				"<tr><td align=center style=\"padding-left:20px\">");
				fprintf(cgiOut,"<table width=620 border=0 cellspacing=0 cellpadding=0>");
                fprintf(cgiOut,"<tr height=30>"\
                 "<td>EBR %s:</td>",search(lwlan,"state"));
                 fprintf(cgiOut,"<td align=left><select name=ebr_use id=ebr_use style=width:100px>"\
				 	"<option value=>"\
					"<option value=disable>disable"\
  				    "<option value=enable>enable"\
	              "</select></td>"\
				 "<td align=left style=\"padding-left:30px\"><font color=red>(%s)</font></td>",search(lwlan,"ebr_able"));
                fprintf(cgiOut,"</tr>");
				fprintf(cgiOut,"<tr height=30 valign=top>");
				   fprintf(cgiOut,"<td width=150>%s:</td>",search(lwlan,"bind_interface"));				 
                 fprintf(cgiOut,"<td width=100 align=left>");
				 if(status==0)
				 {
				   fprintf(cgiOut,"<select name=bind_interface id=bind_interface multiple=multiple size=4 style=width:100px>");
				   if((fp=fopen("/var/run/apache2/ebr_bind_inter.tmp","r"))==NULL)		 /*以只读方式打开资源文件*/
				   {
					   ShowAlert(search(lpublic,"error_open"));
				   }
				   else
				   {
					   memset(BindInter,0,sizeof(BindInter));
					   retu=fgets(BindInter,20,fp);
					   while(retu!=NULL)
					   {
							if(0 == strncmp(retu,"ve",2))
							{
								char *temp_ve = NULL;
								char temp_ve1[20];

								temp_ve = strchr(retu,'@');
								if(temp_ve)
								{
									memset(temp_ve1,0,sizeof(temp_ve1));
									strncpy(temp_ve1,retu,temp_ve-retu);

									memset(retu,0,20);
									strcpy(retu,temp_ve1);
								}
							}
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
		        fprintf(cgiOut,"</td>");
				  fprintf(cgiOut,"<td width=370 align=left style=\"padding-left:30px\"><font color=red>(%s)</font></td>",search(lwlan,"ebr_bind"));
                fprintf(cgiOut,"</tr>");

				fprintf(cgiOut,"<tr height=30 valign=top style=\"padding-top:5px\">");
				   fprintf(cgiOut,"<td>%s:</td>",search(lwlan,"set_uplink_interface"));				 
				   fprintf(cgiOut,"<td align=left>");
				    fprintf(cgiOut,"<select name=set_uplink_interface id=set_uplink_interface multiple=multiple size=4 style=width:100px>");
					if(ins_para)
					{
						result=show_ethereal_bridge_one(ins_para->parameter,ins_para->connection,n,&ebrinfo);
					}
					if(result == 1)
					{
						if((ebrinfo)&&(ebrinfo->EBR[0])&&(ebrinfo->EBR[0]->iflist))
						{
							for(head = ebrinfo->EBR[0]->iflist; (NULL != head); head = head->ifnext)
							{
								if(head->ifname)
								{
									fprintf(cgiOut,"<option value=%s>%s",head->ifname,head->ifname);
								}
							}					
						}
					}
				    fprintf(cgiOut,"</select>");				   
		        fprintf(cgiOut,"</td>");
				  fprintf(cgiOut,"<td align=left style=\"padding-left:30px\"><font color=red>(%s)</font></td>",search(lwlan,"set_ebr_uplink_if"));
                fprintf(cgiOut,"</tr>");
				fprintf(cgiOut,"<tr height=30>"\
                 "<td>EBR %s:</td>",search(lwlan,"isolate_state")); 
				 fprintf(cgiOut,"<td align=left><select name=iso_use id=iso_use style=width:100px>"\
				   "<option value=>"\
				   "<option value=enable>enable"\
				   "<option value=disable>disable"\
	             "</select></td>"\
				  "<td align=left style=\"padding-left:30px\"><font color=red>(%s)</font></td>",search(lwlan,"isolate_able"));
                fprintf(cgiOut,"</tr>");
				fprintf(cgiOut,"<tr height=30>"\
                 "<td>EBR %s:</td>",search(lwlan,"mult_state"));
				 fprintf(cgiOut,"<td align=left><select name=mult_use id=mult_use style=width:100px>"\
				   "<option value=>"\
  				   "<option value=enable>enable"\
  				   "<option value=disable>disable"\
	             "</select></td>"\
				  "<td align=left style=\"padding-left:30px\"><font color=red>(%s)</font></td>",search(lwlan,"mult_able"));
                fprintf(cgiOut,"</tr>");
				fprintf(cgiOut,"<tr height=30>"\
                 "<td>%s:</td>",search(lwlan,"spswitch"));
				fprintf(cgiOut,"<td align=left><select name=sameport_switch id=sameport_switch style=width:100px>"\
				   "<option value=>"\
  				   "<option value=enable>enable"\
  				   "<option value=disable>disable"\
	             "</select></td>"\
				  "<td align=left style=\"padding-left:30px\"><font color=red>(%s)</font></td>",search(lwlan,"spswitch_able"));
                fprintf(cgiOut,"</tr>"\
					"<tr>"\
					"<td><input type=hidden name=encry_conebr value=%s></td>",m);
					fprintf(cgiOut,"<td><input type=hidden name=ebr_id value=%s></td>",n); 
					fprintf(cgiOut,"<td><input type=hidden name=page_no value=%s></td>",pn);
					fprintf(cgiOut,"<td><input type=hidden name=instance_id value=%s></td>",instance_id);
					fprintf(cgiOut,"</tr>"\
				    "</table>");
				fprintf(cgiOut,"</td></tr>"\
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
  Free_ethereal_bridge_one_head(ebrinfo);
}				
return 0;
}


void config_ebr(instance_parameter *ins_para,int id,struct list *lpublic,struct list *lwlan)   /*返回0表示失败，返回1表示成功*/
{
  int flag = 1,ret = 0;
  int result = cgiFormNotFound,i = 0,bind_result = 0;
  char **responses;
  char state[20] = { 0 };  
  char temp[100] = { 0 };
  char ebr_id[10] = { 0 };
  

  /***********************config ethereal bridge enable**************************/  
  memset(state,0,sizeof(state));
  cgiFormStringNoNewlines("ebr_use",state,20);	
  if(strcmp(state,"")!=0)
  {
	  ret=config_ethereal_bridge_enable_cmd(ins_para->parameter,ins_para->connection,id,state);	/*返回0表示失败，返回1表示成功，返回-1表示input parameter should only be 'enable' or 'disable'*/
																								/*返回-2表示ebr id does not exist，返回-3表示ebr if error，返回-4表示system cmd process error，返回-5表示error*/
	  switch(ret)
	  {
	  	case SNMPD_CONNECTION_ERROR:
		case 0:ShowAlert(search(lwlan,"con_ebr_state_fail"));
			   flag=0;
			   break;
		case 1:break;
		case -1:ShowAlert(search(lpublic,"input_para_error"));
				flag=0;
				break;
		case -2:ShowAlert(search(lwlan,"ebr_not_exist"));
				flag=0;
				break;
		case -3:ShowAlert(search(lwlan,"ebr_if_error"));
				flag=0;
				break;
		case -4:ShowAlert(search(lpublic,"sys_cmd_error"));
				flag=0;
				break;
		case -5:ShowAlert(search(lpublic,"error"));
				flag=0;
				break;
	  }
  }

  
  /***********************set ebr add if cmd**************************/  
  result = cgiFormStringMultiple("bind_interface", &responses);
  if(result != cgiFormNotFound) 
  {
  	i = 0;
	bind_result=1;
	while((responses[i])&&(bind_result==1))
	{
		ret=set_ebr_add_del_if_cmd(ins_para->parameter,ins_para->connection,id,"add",responses[i]);  /*返回0表示失败，返回1表示成功，返回-1表示input parameter should only be 'add' or 'delete'*/
																									/*返回-2表示if name too long，返回-3表示ebr id does not exist，返回-4表示ebr should be disable first*/
																									/*返回-5表示if_name already exist/remove some br,or system cmd process error，返回-6表示input ifname error*/
																									/*返回-7表示ebr if error，返回-8表示error，返回-9示EBR ID非法*/
																									/*返回-10表示you want to delete wlan, please do not operate like this*/
																									/*返回-11表示please check the interface's wlanid, you maybe have delete this wlan*/
		switch(ret)
		{
		  case SNMPD_CONNECTION_ERROR:
		  case 0:ShowAlert(search(lwlan,"bind_interface_fail"));
				 bind_result=0;
				 break;
		  case 1:break;
		  case -1:ShowAlert(search(lpublic,"input_para_error"));
				  bind_result=0;
				  break;
		  case -2:ShowAlert(search(lpublic,"if_name_long"));
				  bind_result=0;
				  break;
		  case -3:ShowAlert(search(lwlan,"ebr_not_exist"));
				  bind_result=0;
				  break;
		  case -4:ShowAlert(search(lwlan,"dis_ebr"));
				  bind_result=0;
				  break;
		  case -5:ShowAlert(search(lwlan,"if_name_exist"));
				  bind_result=0;
				  break;
		  case -6:ShowAlert(search(lpublic,"input_ifname_error"));
				  bind_result=0;
				  break;
		  case -7:ShowAlert(search(lwlan,"ebr_if_error"));
				  bind_result=0;
				  break;
		  case -8:ShowAlert(search(lpublic,"error"));
				  bind_result=0;
				  break;
		  case -9:{
				     memset(temp,0,sizeof(temp));
				     strncpy(temp,search(lwlan,"ebr_id_1"),sizeof(temp)-1);
					 memset(ebr_id,0,sizeof(ebr_id));
					 snprintf(ebr_id,sizeof(ebr_id)-1,"%d",EBR_NUM-1);
					 strncat(temp,ebr_id,sizeof(temp)-strlen(temp)-1);
					 strncat(temp,search(lwlan,"ebr_id_2"),sizeof(temp)-strlen(temp)-1);
					 ShowAlert(temp);
					 bind_result=0;
					 break;
				  }
		  case -10:ShowAlert(search(lwlan,"dont_del_wlan"));
				   bind_result=0;
				   break;
		  case -11:ShowAlert(search(lwlan,"have_del_wlan"));
				   bind_result=0;
				   break;
		}
		i++;
	}
	cgiStringArrayFree(responses);
	if(bind_result==0)
	  flag = 0;
  }

  /***********************set ebr add uplink cmd**************************/  
  result = cgiFormStringMultiple("set_uplink_interface", &responses);
  if(result != cgiFormNotFound) 
  {
  	i = 0;
	bind_result=1;
	while((responses[i])&&(bind_result==1))
	{
		ret=set_ebr_add_del_uplink_cmd(ins_para->parameter,ins_para->connection,id,"add",responses[i]); /*返回0表示失败，返回1表示成功，返回-1表示error*/
																									  /*返回-2表示input parameter should only be 'add' or 'delete'，返回-3表示if name too long*/
																									  /*返回-4表示malloc error，返回-5表示ebr should be disable first*/
																									  /*返回-6表示already exist/remove some br,or system cmd process error，返回-7表示input ifname error*/
																									  /*返回-8表示ebr if error，返回-9表示interface does not add to br or br uplink，返回-10表示ebr id does not exist*/
																									  /*返回-11示EBR ID非法*/
		switch(ret)
		{
		  case SNMPD_CONNECTION_ERROR:
		  case 0:ShowAlert(search(lwlan,"set_uplink_interface_fail"));
				 bind_result=0;
				 break;
		  case 1:break;
		  case -1:ShowAlert(search(lpublic,"error"));
				  bind_result=0;
				  break;
		  case -2:ShowAlert(search(lpublic,"input_para_error"));
				  bind_result=0;
				  break;
		  case -3:ShowAlert(search(lpublic,"if_name_long"));
				  bind_result=0;
				  break;
		  case -4:ShowAlert(search(lpublic,"malloc_error"));
				  bind_result=0;
				  break;	
		  case -5:ShowAlert(search(lwlan,"dis_ebr"));
				  bind_result=0;
				  break;
		  case -6:ShowAlert(search(lwlan,"if_name_exist"));
				  bind_result=0;
				  break;
		  case -7:ShowAlert(search(lpublic,"input_ifname_error"));
				  bind_result=0;
				  break;
		  case -8:ShowAlert(search(lwlan,"ebr_if_error"));
				  bind_result=0;
				  break;
		  case -9:ShowAlert(search(lwlan,"if_not_add_to_br"));
				  bind_result=0;
				  break;				  
		  case -10:ShowAlert(search(lwlan,"ebr_not_exist"));
				   bind_result=0;
				   break;	
		  case -11:{
					  memset(temp,0,sizeof(temp));
					  strncpy(temp,search(lwlan,"ebr_id_1"),sizeof(temp)-1);
					  memset(ebr_id,0,sizeof(ebr_id));
					  snprintf(ebr_id,sizeof(ebr_id)-1,"%d",EBR_NUM-1);
					  strncat(temp,ebr_id,sizeof(temp)-strlen(temp)-1);
					  strncat(temp,search(lwlan,"ebr_id_2"),sizeof(temp)-strlen(temp)-1);
					  ShowAlert(temp);
					  bind_result=0;
					  break;
				   }
		}
		i++;
	}
	cgiStringArrayFree(responses);
	if(bind_result==0)
	  flag = 0;
  }  

  /***********************ebr set bridge isolation func**************************/  
  memset(state,0,sizeof(state));
  cgiFormStringNoNewlines("iso_use",state,20);	
  if(strcmp(state,"")!=0)
  {
	  ret=ebr_set_bridge_isolation_func(ins_para->parameter,ins_para->connection,id,state);	/*返回0表示失败，返回1表示成功，返回-1表示input parameter should only be 'enable' or 'disable'*/
																							/*返回-2表示ebr id does not exist，返回-3表示ebr should be disable first，返回-4表示ebr if error*/
																							/*返回-5表示system cmd process error，返回-6表示sameportswitch and isolation are conflict,disable sameportswitch first*/
																							/*返回-7表示error，返回-8示EBR ID非法，返回-9表示apply security in this wlan first*/
	  switch(ret)
	  {
	    case SNMPD_CONNECTION_ERROR:
		case 0:ShowAlert(search(lwlan,"con_ebr_isolate_fail"));
			   flag=0;  
			   break;
		case 1:break;
		case -1:ShowAlert(search(lpublic,"input_para_error"));
				flag=0;
				break;
		case -2:ShowAlert(search(lwlan,"ebr_not_exist"));
				flag=0;
				break;
		case -3:ShowAlert(search(lwlan,"dis_ebr"));
			    flag=0;
			    break;
		case -4:ShowAlert(search(lwlan,"ebr_if_error"));
				flag=0;
				break;
		case -5:ShowAlert(search(lpublic,"sys_cmd_error"));
				flag=0;
				break;
		case -6:ShowAlert(search(lwlan,"spswi_iso_conflict"));
				flag=0;
				break;
		case -7:ShowAlert(search(lpublic,"error"));
				flag=0;
				break;
		case -8:{
				  memset(temp,0,100);
				  strcpy(temp,search(lwlan,"ebr_id_1"));
				  memset(ebr_id,0,10);
				  sprintf(ebr_id,"%d",EBR_NUM-1);
				  strcat(temp,ebr_id);
				  strcat(temp,search(lwlan,"ebr_id_2"));
				  ShowAlert(temp);
				  flag=0;
				  break;
				}	
		case -9:ShowAlert(search(lwlan,"con_bind"));
				flag=0;
				break;
	  }
  }
  

  
  /***********************ebr set bridge multicast isolation func**************************/  
  memset(state,0,sizeof(state));
  cgiFormStringNoNewlines("mult_use",state,20); 
  if(strcmp(state,"")!=0)
  {
	  ret=ebr_set_bridge_multicast_isolation_func(ins_para->parameter,ins_para->connection,id,state);	/*返回0表示失败，返回1表示成功，返回-1表示input parameter should only be 'enable' or 'disable'*/
																										/*返回-2表示ebr id does not exist，返回-3表示ebr should be disable first，返回-4表示ebr if error*/
																										/*返回-5表示system cmd process error，返回-6表示sameportswitch and isolation are conflict,disable sameportswitch first*/
																										/*返回-7表示error，返回-8示EBR ID非法，返回-9表示apply security in this wlan first*/
	  switch(ret)
	  {
	    case SNMPD_CONNECTION_ERROR:
		case 0:ShowAlert(search(lwlan,"con_ebr_mult_fail"));
			   flag=0;  
			   break;
		case 1:break;
		case -1:ShowAlert(search(lpublic,"input_para_error"));
				flag=0;
				break;
		case -2:ShowAlert(search(lwlan,"ebr_not_exist"));
				flag=0;
				break;
		case -3:ShowAlert(search(lwlan,"dis_ebr"));
			    flag=0;
			    break;
		case -4:ShowAlert(search(lwlan,"ebr_if_error"));
				flag=0;
				break;
		case -5:ShowAlert(search(lpublic,"sys_cmd_error"));
				flag=0;
				break;
		case -6:ShowAlert(search(lwlan,"spswi_iso_conflict"));
				flag=0;
				break;
		case -7:ShowAlert(search(lpublic,"error"));
				flag=0;
				break;
		case -8:{
				  memset(temp,0,100);
				  strcpy(temp,search(lwlan,"ebr_id_1"));
				  memset(ebr_id,0,10);
				  sprintf(ebr_id,"%d",EBR_NUM-1);
				  strcat(temp,ebr_id);
				  strcat(temp,search(lwlan,"ebr_id_2"));
				  ShowAlert(temp);
				  flag=0;
				  break;
				}	
		case -9:ShowAlert(search(lwlan,"con_bind"));
				flag=0;
				break;
	  }
  }

  /***********************ebr set bridge sameportswitch func**************************/  
	memset(state,0,sizeof(state));
	cgiFormStringNoNewlines("sameport_switch",state,20); 
	if(strcmp(state,"")!=0)
	{
		ret=ebr_set_bridge_sameportswitch_func(ins_para->parameter,ins_para->connection,id,state);	/*返回0表示失败，返回1表示成功，返回-1表示input parameter should only be 'enable' or 'disable'*/
																									/*返回-2表示ebr id does not exist，返回-3表示ebr should be disable first，返回-4表示ebr if error*/
																									/*返回-5表示system cmd process error，返回-6表示isolation or multicast are enable,disable isolation and multicast first*/
																									/*返回-7表示error，返回-8示EBR ID非法*/
																									/*返回SNMPD_CONNECTION_ERROR表示connection error*/
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
		  case -2:ShowAlert(search(lwlan,"ebr_not_exist"));
				  flag=0;
				  break;
		  case -3:ShowAlert(search(lwlan,"dis_ebr"));
				  flag=0;
				  break;
		  case -4:ShowAlert(search(lwlan,"ebr_if_error"));
				  flag=0;
				  break;
		  case -5:ShowAlert(search(lpublic,"sys_cmd_error"));
				  flag=0;
				  break;
		  case -6:ShowAlert(search(lwlan,"spswi_iso_conflict2"));
				  flag=0;
				  break;
		  case -7:ShowAlert(search(lpublic,"error"));
				  flag=0;
				  break;
		  case -8:{
				    memset(temp,0,100);
				    strcpy(temp,search(lwlan,"ebr_id_1"));
				    memset(ebr_id,0,10);
				    sprintf(ebr_id,"%d",EBR_NUM-1);
				    strcat(temp,ebr_id);
				    strcat(temp,search(lwlan,"ebr_id_2"));
				    ShowAlert(temp);
				    flag=0;
				    break;
				  }	
		}
	}

  
  if(flag==1)
  	ShowAlert(search(lpublic,"oper_succ"));  
}

