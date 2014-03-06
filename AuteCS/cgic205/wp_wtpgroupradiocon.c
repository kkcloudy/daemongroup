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
* wp_radcon.c
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
#include "ws_dcli_wqos.h"
#include "ws_dcli_vrrp.h"
#include "ws_init_dbus.h"
#include "ws_dbus_list_interface.h"


int ShowGroupRadioconPage(char *m, char *t,struct list *lpublic,struct list *lwlan); 
void config_group_radio(instance_parameter *ins_para,int group_id,int radio_id,struct list *lpublic,struct list *lwlan);

int cgiMain()
{
	struct list *lpublic = NULL;   /*解析public.txt文件的链表头*/
	struct list *lwlan = NULL;     /*解析wlan.txt文件的链表头*/  
	char encry[BUF_LEN] = { 0 };  
	char *str = NULL;      
	
	lpublic=get_chain_head("../htdocs/text/public.txt");
	lwlan=get_chain_head("../htdocs/text/wlan.txt");  
	
	DcliWInit();
	ccgi_dbus_init();
	
	memset(encry,0,sizeof(encry));
	cgiFormStringNoNewlines("UN", encry, BUF_LEN); 
	str=dcryption(encry);
	if(str==NULL)
	{
	      ShowErrorPage(search(lpublic,"ill_user"));
	}/*用户非法*/
	else
	{
		ShowGroupRadioconPage(encry,str,lpublic,lwlan);	
	}
	  release(lpublic);  
	  release(lwlan);
	  destroy_ccgi_dbus();
	  return 0;
}

int ShowGroupRadioconPage(char *m, char *t,struct list *lpublic,struct list *lwlan)
{  

  int i=0, result=0;
  char *endptr =NULL;
  char instance_id[10] = { 0 };
  char groupID[10] = { 0 };
  char groupname[32] = { 0 };
  char radio_id[5] = { 0 };
  char page[5] = { 0 };
  int group_id=0;
  int r_id=0;
  int wnum = 0;
  instance_parameter *paraHead1 = NULL;
  dbus_parameter ins_para;
  DCLI_WLAN_API_GROUP *WLANINFO = NULL;


  
  memset(instance_id,0,sizeof(instance_id));  
  memset(groupID,0,sizeof(groupID));  
  memset(groupname,0,sizeof(groupname));  
  memset(radio_id,0,sizeof(radio_id));  
  memset(page,0,sizeof(page));  
  cgiFormStringNoNewlines("INSTANCE_ID",instance_id,10);  
  cgiFormStringNoNewlines("groupID",groupID,10);  
  cgiFormStringNoNewlines("groupname",groupname,32);  
  cgiFormStringNoNewlines("RADIO_ID",radio_id,5);  
  cgiFormStringNoNewlines("PN",page,5);  
  fprintf ( stderr, "instance_id =%s\n", instance_id);   
  fprintf ( stderr, "groupID =%s\n", groupID);   
  fprintf ( stderr, "groupname =%s\n", groupname);   
  fprintf ( stderr, "radio_id =%s\n", radio_id);   
  fprintf ( stderr, "page =%s\n", page);   

  group_id= strtoul(groupID,&endptr,10);    /*char转成int，10代表十进制*/
  r_id= strtoul(radio_id,&endptr,10);
  fprintf ( stderr, "--------------group_id =%d\n", group_id);   
  fprintf ( stderr, "-----------------------r_id =%d\n", r_id);   

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

  
  cgiHeaderContentType("text/html");
  fprintf(cgiOut,"<html xmlns=\"http://www.w3.org/1999/xhtml\"><head>");
  fprintf(cgiOut,"<meta http-equiv=Content-Type content=text/html; charset=gb2312>");
  fprintf(cgiOut,"<title>Wtp</title>");
  fprintf(cgiOut,"<link rel=stylesheet href=/style.css type=text/css>"\
  "</head>"\
  "<script type=\"text/javascript\">"\
  	 "function disable_td()"\
	 "{"\
		"var s = 1;"\
		"var obj = document.getElementsByName(\"txpower_type\");"\
	    "for(var i=0;i<obj.length;i++)"\
		"{"\
	        "if(obj[i].checked)"\
			"{"\
				"s = i+1;"\
	        "}"\
	    "}"\
					
		"var tx_power = document.getElementById(\"tx_power\");"\
		
		"var rad_txpower_offset_step = document.getElementById(\"rad_txpower_offset_step\");"\
		"var rad_txpower_offset = document.getElementById(\"rad_txpower_offset\");"\
		"if(s == \"1\")"\
		"{"\
			"tx_power.disabled=false;"\
			
			"rad_txpower_offset_step.disabled=true;"\
			"rad_txpower_offset.disabled=true;"\
		"}"\
		"else if(s == \"2\")"\
		"{"\
			"tx_power.disabled=true;"\
			
			"rad_txpower_offset_step.disabled=false;"\
			"rad_txpower_offset.disabled=false;"\
		"}"\
	 "}"\
  "</script>"\
  "<body>");
  
  if(cgiFormSubmitClicked("radiocon_apply") == cgiFormSuccess)
  {
  	if(paraHead1)
	{
		config_group_radio(paraHead1,group_id,r_id,lpublic,lwlan);
	} 
  }
  fprintf(cgiOut,"<form method=post>"\
  "<div align=center>"\
  "<table width=976 border=0 cellpadding=0 cellspacing=0>"\
  "<tr>"\
    "<td width=8 align=left valign=top background=/images/di22.jpg><img src=/images/youce4.jpg width=8 height=30/></td>"\
    "<td width=51 align=left valign=bottom background=/images/di22.jpg><img src=/images/youce33.jpg width=37 height=24/></td>");
      fprintf(cgiOut,"<td width=153 align=left valign=bottom background=/images/di22.jpg><font id=titleen>RF</font><font id=%s> %s</font></td>",search(lpublic,"title_style"),search(lpublic,"management"));
    fprintf(cgiOut,"<td width=690 align=right valign=bottom background=/images/di22.jpg>");
        
          fprintf(cgiOut,"<table width=155 border=0 cellspacing=0 cellpadding=0>"\
          "<tr>"\
          "<td width=62 align=center><input id=but type=submit name=radiocon_apply style=background-image:url(/images/%s) value=""></td>",search(lpublic,"img_ok"));
            fprintf(cgiOut,"<td width=62 align=center><a href=wp_group_radiolis.cgi?UN=%s&PN=%s&INSTANCE_ID=%s target=mainFrame><img src=/images/%s border=0 width=62 height=20/></a></td>",m,page,instance_id,search(lpublic,"img_cancel"));
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
			   "<td align=left id=tdleft><a href=wp_radiolis.cgi?UN=%s target=mainFrame class=top><font id=yingwen_san>Radio</font><font id=%s> %s</font></a></td>",m,search(lpublic,"menu_san"),search(lpublic,"list"));			    
		     fprintf(cgiOut,"</tr>");
		     
	       fprintf(cgiOut,"<tr height=25>"\
		       "<td align=left id=tdleft><a href=wp_bssbw.cgi?UN=%s target=mainFrame class=top><font id=yingwen_san>MAC </font><font id=%s>%s</font></a></td>",m,search(lpublic,"menu_san"),search(lwlan,"mac_filter"));		       
	       fprintf(cgiOut,"</tr>");
	       
	     fprintf(cgiOut,"<tr height=25>"\
		 "<td align=left id=tdleft background=/images/bottom_bg.gif style=\"border-right:0\"><font id=%s>%s</font><font id=yingwen_san>group-radio</font></td>",search(lpublic,"menu_san"),search(lpublic,"config"));   /*突出显示*/
	       fprintf(cgiOut,"</tr>");

                  for(i=0;i<6;i++)
                  {
                    fprintf(cgiOut,"<tr height=25>"\
                      "<td id=tdleft>&nbsp;</td>"\
                    "</tr>");
                  }     		  
		  if(paraHead1)
		  {
			  result=show_wlan_list(paraHead1->parameter,paraHead1->connection,&WLANINFO);
		  } 
		  if(result == 1)
		  {
		  	wnum = WLANINFO->wlan_num;
		  }
                fprintf(cgiOut,"</table>"\
              "</td>"\
              "<td align=left style=\"background-color:#ffffff; border-right:1px solid #707070; padding-left:30px\">"\
                      "<table width=500 border=0 cellspacing=0 cellpadding=0>"\
                        "<tr height=10>"\
				"<td>&nbsp;&nbsp;&nbsp;&nbsp;</td>");
			fprintf(cgiOut,"</tr>"\
			"<tr>"\
			  	"<td id=ins_style>%s:%s</td>",search(lpublic,"instance"),instance_id);
			fprintf(cgiOut,"</tr>"\
			"<tr>"\
			      "<td id=thead5>%s group-radio %d%s%d</td>",search(lpublic,"configure"),group_id,"-",r_id);
			fprintf(cgiOut,"</tr>");
			//网页中表单
	fprintf(cgiOut,"<tr><td align=center style=\"padding-left:20px\">"\
	"<table width=500 border=0 cellspacing=0 cellpadding=0>");
		   /*fprintf(cgiOut, "<tr height=30>"\
                    "<td align=right width=110>%s:</td>",search(lwlan,"channel"));
                    fprintf(cgiOut,"<td width=200 align=left  colspan=3>");
			fprintf(cgiOut,"<input type=text name=rad_channel size=10 maxLength=4>");
		fprintf(cgiOut,"</td>"\
                  "</tr>");
		
                 fprintf(cgiOut, "<tr height=30>"\
                    "<td width=110  align=right><input type='radio' name='txpower_type' value='by_txpower' checked='checked' onclick='disable_td();'>&nbsp;%s:</td>",search(lwlan,"tx_power"));
                    fprintf(cgiOut,"<td width=200 align=left  colspan=3>");
			fprintf(cgiOut,"<input type=text name=tx_power id=tx_power size=10 maxLength=3 onkeypress=\"return event.keyCode>=48&&event.keyCode<=57\">");
		fprintf(cgiOut,"</td>"\
                  "</tr>");
		  fprintf(cgiOut,"<tr height=30>"\
			"<td width=110 align=right><input type='radio' name='txpower_type' value='by_txpower_offset' onclick='disable_td();'>&nbsp;%s:</td>",search(lwlan,"tx_power_offset_step"));
			fprintf(cgiOut,"<td align=left width=200>"\
			"<input type=text name=rad_txpower_offset_step id=rad_txpower_offset_step size=10 maxLength=3 onkeypress=\"return event.keyCode>=48&&event.keyCode<=57\" onpaste=\"return false\"><font color=red>(>0DBm)</font></td>"\
                    "<td width=90  align=right>%s:</td>",search(lwlan,"tx_power_offset"));
		fprintf(cgiOut,"<td align=left width=100>"\
		"<input type=text name=rad_txpower_offset id=rad_txpower_offset size=10 maxLength=3 onkeypress=\"return ((event.keyCode>=48&&event.keyCode<=57)||event.keyCode==45)\" onpaste=\"return false\">"\
		"</td>"\
                  "</tr>");
					

                  fprintf(cgiOut,"<tr height=30>"\
                    "<td  align=right width=110>Beacon:</td>"\
                    "<td align=left width=200   colspan=3>");
			fprintf(cgiOut,"<input type=text name=rad_beacon size=10 maxLength=4 onkeypress=\"return event.keyCode>=48&&event.keyCode<=57\">");
					fprintf(cgiOut,"<font color=red>(25--1000)ms</font></td>"\
                  "</tr>"\
                  "<tr height=30>"\
                    "<td  align=right width=110>%s:</td>",search(lwlan,"fragment"));
                    fprintf(cgiOut,"<td align=left width=200  colspan=3>");
		fprintf(cgiOut,"<input type=text name=rad_fragment size=10 maxLength=4 onkeypress=\"return event.keyCode>=48&&event.keyCode<=57\">");
		fprintf(cgiOut,"<font color=red>(256--2346)byte</font></td>"\
                  "</tr>"
                  "<tr height=30>"\
                    "<td  align=right width=110>Dtim:</td>"\
                    "<td align=left width=200   colspan=3>");
		fprintf(cgiOut,"<input type=text name=rad_dtim size=10 maxLength=2 onkeypress=\"return event.keyCode>=48&&event.keyCode<=57\">"\
			"<font color=red>(1--15)</font></td>");
                    fprintf(cgiOut,"</tr>");
                  fprintf(cgiOut,"<tr height=30>"\
                    "<td  align=right width=110>RTS:</td>"\
                    "<td align=left width=200   colspan=3>");
			fprintf(cgiOut,"<input type=text name=rad_rts size=10 maxLength=4 onkeypress=\"return event.keyCode>=48&&event.keyCode<=57\">");
		fprintf(cgiOut,"<font color=red>(256--2347)</font></td>"\
                  "</tr>"
                  "<tr height=30>"\
                    "<td  align=right width=110>%s:</td>",search(lwlan,"pream"));
                    fprintf(cgiOut,"<td align=left width=200  colspan=3>");
		fprintf(cgiOut,"<select name=radio_pream id=radio_pream style=width:72px>");
               fprintf(cgiOut,"<option value=none >"\
	       	"<option value=short >short"\
               "<option value=long>long");
             fprintf(cgiOut,"</select>"\
                    "</td>"\
                  "</tr>"\
                  "<tr height=30>"\
                    "<td  align=right width=110>ShortRetry:</td>"\
                    "<td align=left width=200  colspan=3>");
	  	fprintf(cgiOut,"<input type=text name=rad_shortretry size=10 maxLength=2 onkeypress=\"return event.keyCode>=48&&event.keyCode<=57\">");
					fprintf(cgiOut,"<font color=red>(1--15)</font></td>"\
                  "</tr>"
                  "<tr height=30>"\
                    "<td  align=right width=110>LongRetry:</td>"\
                    "<td align=left width=200  colspan=3>");
	  	fprintf(cgiOut,"<input type=text name=rad_longretry size=10 maxLength=2 onkeypress=\"return event.keyCode>=48&&event.keyCode<=57\">");
					fprintf(cgiOut,"<font color=red>(1--15)</font></td>"\
                  "</tr>"
                  "<tr height=30>"\
                    "<td  align=right width=110>%s:</td>",search(lwlan,"service"));
                    fprintf(cgiOut,"<td align=left width=200  colspan=3>"\
                                     "<select name=radio_service id=radio_service style=width:72px>");
                                        fprintf(cgiOut,"<option value=none>"\
						"<option value=disable>disable"\
                                        "<option value=enable>enable");
                                     fprintf(cgiOut,"</select>"\
                    "</td>"\
                  "</tr>");
						
                  fprintf(cgiOut,"<tr height=30>"\
                    "<td  align=right width=110>%s:</td>",search(lwlan,"rad_max_throughout"));
                    fprintf(cgiOut,"<td align=left width=200  colspan=3><input type=text name=rad_max_through size=10 maxLength=3 onkeypress=\"return event.keyCode>=48&&event.keyCode<=57\">"\
		    	"<font color=red>(1--108)M</font></td>"\
                  "</tr>");*/

		  fprintf(cgiOut,"<tr height=30>"\
		  "<td width=110 align=right>%s WLAN:</td>",search(lpublic,"bind"));
		  fprintf(cgiOut,"<td align=left width=110 colspan=3>"\
			   "<select name=bind_wlan id=bind_wlan style=width:72px>");
				   fprintf(cgiOut,"<option value=>");
				if(result == 1)
				{
					for(i=0;i<wnum;i++)
					{
					  if(WLANINFO->WLAN[i])
					  {
						  fprintf(cgiOut,"<option value=%d>%d",WLANINFO->WLAN[i]->WlanID,WLANINFO->WLAN[i]->WlanID);
					  }
					}
				}
			   fprintf(cgiOut,"</select>"\
		  "</td>"\
		"</tr>");

		
		  fprintf(cgiOut,"<tr height=30>"\
		  "<td width=110 align=right>%s:</td>",search(lwlan,"unbind_wlan"));
		  fprintf(cgiOut,"<td align=left width=110 colspan=3>"\
			   "<select name=unbind_wlan id=unbind_wlan style=width:72px>");
				   fprintf(cgiOut,"<option value=>");
				if(result == 1)
				{
					for(i=0;i<wnum;i++)
					{
					  if(WLANINFO->WLAN[i])
					  {
						  fprintf(cgiOut,"<option value=%d>%d",WLANINFO->WLAN[i]->WlanID,WLANINFO->WLAN[i]->WlanID);
					  }
					}
				}
			   fprintf(cgiOut,"</select>"\
		  "</td>"\
		"</tr>");

		/*fprintf(cgiOut,"<tr height=30>"\
		  "<td	align=right width=110>%s:</td>",search(lwlan,"auto_channel"));
		  fprintf(cgiOut,"<td align=left width=200  colspan=3>"\
			  "<select name=auto_channel id=auto_channel style=width:72px>"\
			    "<option value=>"\
			     "<option value=disable>disable"\
			    "<option value=enable>enable"\
			  "</select>"\
			      "</td>"\
			"</tr>");*/
		
		    
		fprintf(cgiOut, "<tr>"\
                    "<td><input type=hidden name=UN value=%s></td>",m);
		fprintf(cgiOut,"<td><input type=hidden name=INSTANCE_ID value=%s></td>",instance_id);
                    fprintf(cgiOut,"<td><input type=hidden name=RADIO_ID value=%s></td>",radio_id);		
                  fprintf(cgiOut,"</tr>");
		  	
		  fprintf(cgiOut, "<tr>");
		  fprintf(cgiOut,"<td><input type=hidden name=groupname value=%s></td>",groupname); 
		      fprintf(cgiOut,"<td><input type=hidden name=PN value=%s></td>",page);
		      fprintf(cgiOut,"<td><input type=hidden name=groupID value=%s></td>",groupID);
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

if(result == 1)
{
  Free_wlan_head(WLANINFO);
}
free_instance_parameter_list(&paraHead1);
return 0;
}

void config_group_radio(instance_parameter *ins_para,int group_id, int radio_id, struct list *lpublic,struct list *lwlan)
{
   int id=0;
  int ret = 0,flag = 1;
  char temp[100] = { 0 };
  char un_bind_wlan[10] = { 0 };
  char bind_wlan[10] = { 0 };
  char l_bss_num[10] = { 0 };
  char max_wlan_num[10] = { 0 };
  char max_radio_num[10] = { 0 };

  


   
   /****************radio apply wlan  &&   radio apply wlan base vlan****************/
   memset(bind_wlan,0,sizeof(bind_wlan));
   cgiFormStringNoNewlines("bind_wlan",bind_wlan,10);   
   if(strcmp(bind_wlan,""))
   {
	 struct RadioList *RadioList_Head = NULL;
	 ret=radio_apply_wlan_group(ins_para->parameter,ins_para->connection,1,group_id,radio_id,bind_wlan,&RadioList_Head);
	 switch(ret)
	 {
	   case 1:break;
	   case -1:{
				 flag=0;
				 ShowAlert(search(lpublic,"input_para_error"));
				 break;
		}
	   case -2:
	   case -13:{
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
	   case -16:{
				  flag=0;
				  ShowAlert(search(lpublic,"bind_wlan_part_fail"));
				  break;
		}
	   case -17:{
				  flag=0;
				  ShowAlert(search(lpublic,"wtp_group_not_exist"));
				  break;
		}
	   default:
	   {
		   flag=0;
		   ShowAlert(search(lwlan,"radio_apply_wlan_fail"));
		   break;
	   }
	 }
   }
   
   memset(un_bind_wlan,0,sizeof(un_bind_wlan));
   cgiFormStringNoNewlines("unbind_wlan",un_bind_wlan,10);  
   if(strcmp(un_bind_wlan,""))
   {
	 struct RadioList *RadioList_Head = NULL;
	 ret=set_radio_delete_wlan_cmd_group(ins_para->parameter,ins_para->connection,1,group_id,radio_id,un_bind_wlan,&RadioList_Head);
	 switch(ret)
	 {
	   case 1:break;
	   case -1:{
				 flag=0;
				 ShowAlert(search(lpublic,"input_para_error"));
				 break;
		}
	   case -2:
	   case -7:{
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
	   case -9:{
				  flag=0;
				  ShowAlert(search(lpublic,"unbind_wlan_part_fail"));
				  break;
		}
	   case -10:{
				  flag=0;
				  ShowAlert(search(lpublic,"wtp_group_not_exist"));
				  break;
		}
	   default:
	   {
		   flag=0;
		   ShowAlert(search(lwlan,"radio_unapply_wlan_fail"));
		   break;
	   }
	 }
   }

  if(flag==1)
    ShowAlert(search(lpublic,"oper_succ"));
}


