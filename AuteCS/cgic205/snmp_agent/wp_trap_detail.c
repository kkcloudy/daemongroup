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
* wp_trap_detail.c
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
#include <dbus/dbus.h>
#include "ws_snmpd_engine.h"
#include "ac_manage_interface.h"
#include "ws_dcli_wlans.h"
#include "ws_usrinfo.h"
#include "ws_err.h"
#include "ws_ec.h"
#include "ws_init_dbus.h"
#include "ws_dbus_list_interface.h"
#include "ac_manage_def.h"
#include "ac_manage_snmp_config.h"

void ShowTrapDetailPage(char *m,char *n,struct list *lpublic,struct list *lsnmpd);  
void ConfigTrapSwitch(int trap_num,DBusConnection *select_connection,struct list *lpublic,struct list *lsnmpd);


/*0-54,89,90*/
int ap_trap_index[]={0,1,2,3,4,5,6,7,8,9,10,11,12,13,14,15,16,17,18,19,20,21,22,23,24,25,26,
					 27,28,29,30,31,32,33,34,35,36,37,38,39,40,41,42,43,44,45,46,47,48,49,50,
					 51,52,53,54,89,90};

/*55-88*/
int ac_trap_index[]={55,56,57,58,59,60,61,62,63,64,65,66,67,68,69,70,71,72,73,74,75,76,77,78,
					79,80,81,82,83,84,85,86,87,88};

int cgiMain()
{
  char encry[BUF_LEN] = { 0 };              
  char *str = NULL;      
  struct list *lpublic = NULL;   /*解析public.txt文件的链表头*/
  struct list *lsnmpd = NULL;     /*解析wlan.txt文件的链表头*/  
  lpublic=get_chain_head("../htdocs/text/public.txt");
  lsnmpd=get_chain_head("../htdocs/text/snmpd.txt");
  
  DcliWInit();
  ccgi_dbus_init();
  memset(encry,0,sizeof(encry));
  if(cgiFormStringNoNewlines("UN", encry, BUF_LEN)!=cgiFormNotFound )  /*首次进入该页*/
  {
  	;
  }
  else
  {
    cgiFormStringNoNewlines("encry_trapdetail",encry,BUF_LEN);
  }  
  str=dcryption(encry);
  if(str==NULL)
	ShowErrorPage(search(lpublic,"ill_user"));		   /*用户非法*/
  else
	ShowTrapDetailPage(encry,str,lpublic,lsnmpd);
  release(lpublic);  
  release(lsnmpd);
  destroy_ccgi_dbus();
  return 0;
}

void ShowTrapDetailPage(char *m,char *n,struct list *lpublic,struct list *lsnmpd)
{    
  char IsSubmit[5] = { 0 };
  int i = 0,retu = 1,limit = 0;     
  char select_slotid[10] = { 0 };
  char temp[10] = { 0 };
  unsigned int slot_id = 0;
  DBusConnection *select_connection = NULL;
  instance_parameter *paraHead = NULL,*paraNode = NULL;
  char *endptr = NULL;	
  unsigned int ret = AC_MANAGE_DBUS_ERROR;
  TRAP_DETAIL_CONFIG *trapDetail_array = NULL;
  int j1 = 0,j2 = 0;
  int is_even = 1;	/*1表示偶数，0表示奇数*/
  unsigned int trapDetail_num = 0;
  int ap_trap_num = sizeof(ap_trap_index)/sizeof(ap_trap_index[0]);
  int ac_trap_num = sizeof(ac_trap_index)/sizeof(ac_trap_index[0]);
  int trap_num = 0;

  cgiHeaderContentType("text/html");
  fprintf(cgiOut,"<html xmlns=\"http://www.w3.org/1999/xhtml\"><head>");
  fprintf(cgiOut,"<meta http-equiv=Content-Type content=text/html; charset=gb2312>");
  fprintf(cgiOut,"<title>TRAP</title>");
  fprintf(cgiOut,"<link rel=stylesheet href=/style.css type=text/css>"\
  "</head>"\
  "<script type=\"text/javascript\">"\
    "function checkAll(e, itemName)"\
    "{"\
	  "var aa = document.getElementsByName(itemName);"\
	  "for (var x=0; x<aa.length; x++)"\
	    "aa[x].checked = e.checked;"\
    "}"\
  "</script>"\
  "<script src=/slotid_onchange.js>"\
  "</script>"\
  "<body>");
  memset(select_slotid,0,sizeof(select_slotid));
  cgiFormStringNoNewlines( "SLOT_ID", select_slotid, 10 );
  list_instance_parameter(&paraHead, SNMPD_SLOT_CONNECT);
  if(strcmp(select_slotid,"")==0)
  { 
	if(paraHead)
	{
		snprintf(select_slotid,sizeof(select_slotid)-1,"%d",paraHead->parameter.slot_id);
	} 
  }  
  slot_id=strtoul(select_slotid,&endptr,10);
  get_slot_dbus_connection(slot_id,&select_connection,SNMPD_INSTANCE_MASTER_V3);

  memset(IsSubmit,0,sizeof(IsSubmit));  
  cgiFormStringNoNewlines("SubmitFlag", IsSubmit, 5);
  if((cgiFormSubmitClicked("trapdetail_apply") == cgiFormSuccess)&&(strcmp(IsSubmit,"")))
  {
  	trap_num = ap_trap_num + ac_trap_num;
  	ConfigTrapSwitch(trap_num,select_connection,lpublic,lsnmpd);
  }
  fprintf(cgiOut,"<form method=post>"\
  "<div align=center>"\
  "<table width=976 border=0 cellpadding=0 cellspacing=0>"\
  "<tr>"\
    "<td width=8 align=left valign=top background=/images/di22.jpg><img src=/images/youce4.jpg width=8 height=30/></td>"\
    "<td width=51 align=left valign=bottom background=/images/di22.jpg><img src=/images/youce33.jpg width=37 height=24/></td>"\
    "<td width=153 align=left valign=bottom id=titleen background=/images/di22.jpg>TRAP</td>"\
    "<td width=690 align=right valign=bottom background=/images/di22.jpg>");        
	 
    	  fprintf(cgiOut,"<table width=155 border=0 cellspacing=0 cellpadding=0>"\
          "<tr>");		  
		  retu=checkuser_group(n);		  
		  if(retu == 0)  /*管理员*/
		  {
			  fprintf(cgiOut,"<td width=62 align=center><input id=but type=submit name=trapdetail_apply style=background-image:url(/images/%s) value=""></td>",search(lpublic,"img_ok"));
		  }
		  else
		  {
		  	  fprintf(cgiOut,"<td width=62 align=center><a href=wp_snmp.cgi?UN=%s target=mainFrame><img src=/images/%s border=0 width=62 height=20/></a></td>",m,search(lpublic,"img_ok"));
		  }
          fprintf(cgiOut,"<td width=62 align=center><a href=wp_snmp.cgi?UN=%s target=mainFrame><img src=/images/%s border=0 width=62 height=20/></a></td>",m,search(lpublic,"img_cancel"));
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
					"<td align=left id=tdleft><a href=wp_trap_summary.cgi?UN=%s target=mainFrame class=top><font id=yingwen_san>TRAP</font><font id=%s> %s</font></a></td>",m,search(lpublic,"menu_san"),search(lpublic,"info"));
				  fprintf(cgiOut,"</tr>");				  			  
				  if(retu==0)  /*管理员*/
				  {
                    fprintf(cgiOut,"<tr height=25>"\
					  "<td align=left id=tdleft><a href=wp_trap_config.cgi?UN=%s target=mainFrame class=top><font id=%s>%s</font><font id=yingwen_san> TRAP</font></a></td>",m,search(lpublic,"menu_san"),search(lpublic,"config"));
                    fprintf(cgiOut,"</tr>");
				  }	
				  fprintf(cgiOut,"<tr height=25>"\
					"<td align=left id=tdleft><a href=wp_trap_list.cgi?UN=%s target=mainFrame class=top><font id=yingwen_san>TRAP</font><font id=%s> %s</font></a></td>",m,search(lpublic,"menu_san"),search(lpublic,"list"));
                  fprintf(cgiOut,"</tr>");
				  if(retu==0)  /*管理员*/
				  {
                    fprintf(cgiOut,"<tr height=25>"\
					  "<td align=left id=tdleft><a href=wp_trap_add.cgi?UN=%s target=mainFrame class=top><font id=%s>%s </font><font id=yingwen_san>TRAP</font></a></td>",m,search(lpublic,"menu_san"),search(lpublic,"ntp_add"));
                    fprintf(cgiOut,"</tr>");
				  }	
				  fprintf(cgiOut,"<tr height=26>"\
                    "<td align=left id=tdleft background=/images/bottom_bg.gif style=\"border-right:0\"><font id=yingwen_san>TRAP</font><font id=%s> %s</font></td>",search(lpublic,"menu_san"),search(lpublic,"switch"));   /*突出显示*/
                  fprintf(cgiOut,"</tr>");	
				  
				  ret = ac_manage_show_trap_switch(select_connection, &trapDetail_array, &trapDetail_num);

				  if(retu==0)  /*管理员*/
				  	limit = 47;
				  else
				  	limit = 48;
				  
				  for(i=0;i<limit;i++)
	              {
  				    fprintf(cgiOut,"<tr height=25>"\
                      "<td id=tdleft>&nbsp;</td>"\
                    "</tr>");
	              }				  
                fprintf(cgiOut,"</table>"\
              "</td>"\
              "<td align=left valign=top style=\"background-color:#ffffff; border-right:1px solid #707070; padding-left:30px; padding-top:10px\">"\
 "<table width=520 border=0 bgcolor=#ffffff cellspacing=0 cellpadding=0>"\
  "<tr style=\"padding-bottom:15px\">"\
	"<td width=70><b>SLOT ID:</b></td>"\
	  "<td width=450>"\
		  "<select name=slot_id id=slot_id style=width:45px onchange=slotid_change(this,\"wp_trap_detail.cgi\",\"%s\")>",m);
		  for(paraNode = paraHead; paraNode; paraNode = paraNode->next)
		  {
			 memset(temp,0,sizeof(temp));
			 snprintf(temp,sizeof(temp)-1,"%d",paraNode->parameter.slot_id);
  
			 if(strcmp(select_slotid,temp) == 0)
			   fprintf(cgiOut,"<option value='%s' selected=selected>%s",temp,temp);
			 else
			   fprintf(cgiOut,"<option value='%s'>%s",temp,temp);
		  } 		  
		  fprintf(cgiOut,"</select>"\
	  "</td>"\
  "</tr>");
  if((AC_MANAGE_SUCCESS == ret)&&(trapDetail_array))
  {
	   fprintf(cgiOut,"<tr>"\
		   "<td>AP%s</td>",search(lsnmpd,"system_trap"));
	   	   if(retu == 0)  /*管理员*/
	   	   {
			   fprintf(cgiOut,"<td><input type=checkbox name=ap_trap_list_all value=all onclick=\"checkAll(this,'ap_trap_list')\">%s</td>",search(lpublic,"all"));
	   	   }
		   else
		   {
		   	   fprintf(cgiOut,"<td>&nbsp;</td>");
	   	   }
	   fprintf(cgiOut,"</tr>"\
	   "<tr>"\
		 "<td colspan=2 valign=top align=center style=\"padding-bottom:5px\">"\
			 "<table width=520 border=1>");
				  if(ap_trap_num%2) 
				  {
					  is_even = 0;
				  }
				  for(i = 0; i < ap_trap_num; i = i+2)
				  {
				  	 j1 =  ap_trap_index[i];
					 if((0 == is_even)&&(i == ap_trap_num-1))/*奇数个*/
					 {
					 	 j2 = 0;
					 }
					 else
					 {
						 j2 =  ap_trap_index[i+1];
					 }
					 if((j1<trapDetail_num)&&(j2<trapDetail_num))
					 {
						 if(strcmp(search(lsnmpd,"close"),"close") == 0)/*english*/
						 {						 	 
							 if(retu == 0)  /*管理员*/
							 {
								 fprintf(cgiOut,"<tr align=left>"\
									"<td width=260><input type=checkbox name=ap_trap_list value=%d %s>%s</td>",j1,(trapDetail_array[j1].trapSwitch ? "checked" : ""),trapDetail_array[j1].trapName);
									if((0 == is_even)&&(i == ap_trap_num-1))/*奇数个*/
									  fprintf(cgiOut,"<td width=260>&nbsp;</td>");
									else
									  fprintf(cgiOut,"<td width=260><input type=checkbox name=ap_trap_list value=%d %s>%s</td>",j2,(trapDetail_array[j2].trapSwitch ? "checked" : ""),trapDetail_array[j2].trapName);
								 fprintf(cgiOut,"</tr>");
							 }
							 else		   /*普通用户*/
							 {
								 fprintf(cgiOut,"<tr align=left>"\
									"<td width=260><input type=checkbox name=ap_trap_list disabled value=%d %s>%s</td>",j1,(trapDetail_array[j1].trapSwitch ? "checked" : ""),trapDetail_array[j1].trapName);
									if((0 == is_even)&&(i == ap_trap_num-1))/*奇数个*/
									  fprintf(cgiOut,"<td width=260>&nbsp;</td>");
									else
									  fprintf(cgiOut,"<td width=260><input type=checkbox name=ap_trap_list disabled value=%d %s>%s</td>",j2,(trapDetail_array[j2].trapSwitch ? "checked" : ""),trapDetail_array[j2].trapName);
								 fprintf(cgiOut,"</tr>");
							 }
						 }	
						 else
						 {
						 	 if(retu == 0)  /*管理员*/
						 	 {
								 fprintf(cgiOut,"<tr align=left>"\
									"<td width=260><input type=checkbox name=ap_trap_list value=%d %s>%s</td>",j1,(trapDetail_array[j1].trapSwitch ? "checked" : ""),ALL_TRAP[j1].trapDes);
									if((0 == is_even)&&(i == ap_trap_num-1))/*奇数个*/
									  fprintf(cgiOut,"<td width=260>&nbsp;</td>");
									else
									  fprintf(cgiOut,"<td width=260><input type=checkbox name=ap_trap_list value=%d %s>%s</td>",j2,(trapDetail_array[j2].trapSwitch ? "checked" : ""),ALL_TRAP[j2].trapDes);
								 fprintf(cgiOut,"</tr>");
						 	 }
							 else		   /*普通用户*/
							 {
								 fprintf(cgiOut,"<tr align=left>"\
									"<td width=260><input type=checkbox name=ap_trap_list disabled value=%d %s>%s</td>",j1,(trapDetail_array[j1].trapSwitch ? "checked" : ""),ALL_TRAP[j1].trapDes);
									if((0 == is_even)&&(i == ap_trap_num-1))/*奇数个*/
									  fprintf(cgiOut,"<td width=260>&nbsp;</td>");
									else
									  fprintf(cgiOut,"<td width=260><input type=checkbox name=ap_trap_list disabled value=%d %s>%s</td>",j2,(trapDetail_array[j2].trapSwitch ? "checked" : ""),ALL_TRAP[j2].trapDes);
								 fprintf(cgiOut,"</tr>");
						 	 }
						 }
					 }
				  }
		   fprintf(cgiOut,"</table>"\
		 "</td>"\
	  "</tr>"\
	  "<tr style=\"padding-top:15px\">"\
		   "<td>AC%s</td>",search(lsnmpd,"system_trap"));
		   if(retu == 0)  /*管理员*/
		   {
			   fprintf(cgiOut,"<td><input type=checkbox name=ap_trap_list_all value=all onclick=\"checkAll(this,'ac_trap_list')\">%s</td>",search(lpublic,"all"));
		   }
		   else
		   {
		   	   fprintf(cgiOut,"<td>&nbsp;</td>");
	   	   }
	   fprintf(cgiOut,"</tr>"\
	   "<tr>"\
		 "<td colspan=2 valign=top align=center style=\"padding-bottom:5px\">"\
			 "<table width=520 border=1>");
	   			  is_even = 1;
				  if(ac_trap_num%2) 
				  {
					  is_even = 0;
				  }
				  for(i = 0; i < ac_trap_num; i = i+2)
				  {
				  	 j1 =  ac_trap_index[i];
					 if((0 == is_even)&&(i == ac_trap_num-1))/*奇数个*/
					 {
					 	 j2 = 0;
					 }
					 else
					 {
						 j2 =  ac_trap_index[i+1];
					 }
					 if((j1<trapDetail_num)&&(j2<trapDetail_num))
					 {
						 if(strcmp(search(lsnmpd,"close"),"close") == 0)/*english*/
						 {
						 	 if(retu == 0)  /*管理员*/
						 	 {
								 fprintf(cgiOut,"<tr align=left>"\
									"<td width=260><input type=checkbox name=ac_trap_list value=%d %s>%s</td>",j1,(trapDetail_array[j1].trapSwitch ? "checked" : ""),trapDetail_array[j1].trapName);
									if((0 == is_even)&&(i == ac_trap_num-1))/*奇数个*/
									  fprintf(cgiOut,"<td width=260>&nbsp;</td>");
									else
									  fprintf(cgiOut,"<td width=260><input type=checkbox name=ac_trap_list value=%d %s>%s</td>",j2,(trapDetail_array[j2].trapSwitch ? "checked" : ""),trapDetail_array[j2].trapName);
								 fprintf(cgiOut,"</tr>");
						 	 }
							 else		   /*普通用户*/
							 {
								 fprintf(cgiOut,"<tr align=left>"\
									"<td width=260><input type=checkbox name=ac_trap_list disabled value=%d %s>%s</td>",j1,(trapDetail_array[j1].trapSwitch ? "checked" : ""),trapDetail_array[j1].trapName);
									if((0 == is_even)&&(i == ac_trap_num-1))/*奇数个*/
									  fprintf(cgiOut,"<td width=260>&nbsp;</td>");
									else
									  fprintf(cgiOut,"<td width=260><input type=checkbox name=ac_trap_list disabled value=%d %s>%s</td>",j2,(trapDetail_array[j2].trapSwitch ? "checked" : ""),trapDetail_array[j2].trapName);
								 fprintf(cgiOut,"</tr>");
						 	 }
						 }	
						 else
						 {
						 	 if(retu == 0)  /*管理员*/
						 	 {
								 fprintf(cgiOut,"<tr align=left>"\
									"<td width=260><input type=checkbox name=ac_trap_list value=%d %s>%s</td>",j1,(trapDetail_array[j1].trapSwitch ? "checked" : ""),ALL_TRAP[j1].trapDes);
									if((0 == is_even)&&(i == ac_trap_num-1))/*奇数个*/
									  fprintf(cgiOut,"<td width=260>&nbsp;</td>");
									else
									  fprintf(cgiOut,"<td width=260><input type=checkbox name=ac_trap_list value=%d %s>%s</td>",j2,(trapDetail_array[j2].trapSwitch ? "checked" : ""),ALL_TRAP[j2].trapDes);
								 fprintf(cgiOut,"</tr>");
						 	 }
							 else		   /*普通用户*/
							 {
								 fprintf(cgiOut,"<tr align=left>"\
									"<td width=260><input type=checkbox name=ac_trap_list disabled value=%d %s>%s</td>",j1,(trapDetail_array[j1].trapSwitch ? "checked" : ""),ALL_TRAP[j1].trapDes);
									if((0 == is_even)&&(i == ac_trap_num-1))/*奇数个*/
									  fprintf(cgiOut,"<td width=260>&nbsp;</td>");
									else
									  fprintf(cgiOut,"<td width=260><input type=checkbox name=ac_trap_list disabled value=%d %s>%s</td>",j2,(trapDetail_array[j2].trapSwitch ? "checked" : ""),ALL_TRAP[j2].trapDes);
								 fprintf(cgiOut,"</tr>");
						 	 }
						 }
					 }
				  }
		   fprintf(cgiOut,"</table>"\
		 "</td>"\
	  "</tr>");
	  MANAGE_FREE(trapDetail_array);
  }
  else
  {
	  fprintf(cgiOut,"<tr>"\
		   "<td  colspan=2>%s</td>",search(lpublic,"contact_adm"));
	  fprintf(cgiOut,"</tr>");
  }
  fprintf(cgiOut,"<tr>"\
	  "<td><input type=hidden name=encry_trapdetail value=%s></td>",m);
  	  fprintf(cgiOut,"<td><input type=hidden name=SubmitFlag value=%d></td>",1);
  fprintf(cgiOut,"</tr>"\
  "<tr>"\
	  "<td colspan=2><input type=hidden name=SLOT_ID value=%s></td>",select_slotid);
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
free_instance_parameter_list(&paraHead);
}

void ConfigTrapSwitch(int trap_num,DBusConnection *select_connection,struct list *lpublic,struct list *lsnmpd)
{
	int result = cgiFormNotFound;
    char **responses1,**responses2;
	int i = 0;	
    int ret = AC_MANAGE_DBUS_ERROR;
	unsigned int index = 0;
    struct trap_group_switch group_switch = { 0 };
	
	result = cgiFormStringMultiple("ap_trap_list", &responses1);
	if(result != cgiFormNotFound)
	{
		while(responses1[i])
		{		
			index = atoi(responses1[i]);
			if(index < 64)
			{
				group_switch.low_switch |= (unsigned long long)0x1 << index;
			}
			else 
			{
				group_switch.high_switch |= (unsigned long long)0x1 << (index - 64);
			}
			i++;
		}
	}
	cgiStringArrayFree(responses1);  

	result = cgiFormNotFound;
	result = cgiFormStringMultiple("ac_trap_list", &responses2);
	if(result != cgiFormNotFound)
	{
		i = 0;
		while(responses2[i])
		{		
			index = atoi(responses2[i]);
			if(index < 64)
			{
				group_switch.low_switch |= (unsigned long long)0x1 << index;
			}
			else
			{
				group_switch.high_switch |= (unsigned long long)0x1 << (index - 64);
			}
			i++;
		}
	}
	cgiStringArrayFree(responses2);  
	
	ret = ac_manage_config_trap_group_switch(select_connection, &group_switch);
	if(AC_MANAGE_SUCCESS == ret) 
	{
		ShowAlert(search(lsnmpd,"config_trap_switch_succ"));
    }
    else if(AC_MANAGE_DBUS_ERROR == ret) 
	{
		ShowAlert(search(lsnmpd,"config_trap_switch_fail"));
    }
    else if(AC_MANAGE_SERVICE_ENABLE == ret) 
	{
		ShowAlert(search(lsnmpd,"disable_trap_service"));
    }
    else if(AC_MANAGE_INPUT_TYPE_ERROR == ret) 
	{
		ShowAlert(search(lsnmpd,"input_type_error"));
    }
    else
	{
		ShowAlert(search(lsnmpd,"config_trap_switch_fail"));
    }
}

