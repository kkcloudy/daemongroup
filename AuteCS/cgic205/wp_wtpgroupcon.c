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
* wp_wtpgroupcon.c
*
*
* CREATOR:
* autelan.software.Network Dep. team
* hupx@autelan.com
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
#include "ws_dcli_wlans.h"
#include "ws_dbus_list_interface.h"
#include "ws_init_dbus.h"


int ShowWtpGroupconPage(char *m,char *n,struct list *lpublic,struct list *lwlan); 
void config_wtpgroup(instance_parameter *ins_para,char *id,struct list *lpublic,struct list *lwlan);
void showerror(int type,int ret, struct list *lpublic,struct list *lwlan);


int cgiMain()
{
	  char encry[BUF_LEN] = { 0 };	
	  char *str = NULL;	 
	  struct list *lpublic = NULL;	 /*解析public.txt文件的链表头*/
	  struct list *lwlan = NULL;	 /*解析wlan.txt文件的链表头*/  
	  lpublic=get_chain_head("../htdocs/text/public.txt");
	  lwlan=get_chain_head("../htdocs/text/wlan.txt");
	  
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
		ShowWtpGroupconPage(encry,str,lpublic,lwlan);	
	  }
	  release(lpublic);  
	  release(lwlan);
	  destroy_ccgi_dbus();
	  return 0;
}

int ShowWtpGroupconPage(char *m,char *n,struct list *lpublic,struct list *lwlan)
{  
	int i = 0,status = 1; 
	char select_insid[10] = { 0 };
	char groupID[5] = { 0 };
	char IsSubmit[5] = { 0 };
	char groupname[WTP_AP_GROUP_NAME_MAX_LEN+5] = { 0 };
	char BindInter[20] = { 0 };	
	char *Inter = NULL;
	FILE *fp = NULL;
	dbus_parameter ins_para;
	instance_parameter *paraHead1 = NULL;

	memset(select_insid,0,sizeof(select_insid));
	memset(groupID,0,sizeof(groupID));
	memset(groupname,0,sizeof(groupname));
	cgiFormStringNoNewlines("INSTANCE_ID", select_insid, 10);
	cgiFormStringNoNewlines("groupID", groupID, 5);
	cgiFormStringNoNewlines("groupname", groupname, WTP_AP_GROUP_NAME_MAX_LEN+5);

	get_slotID_localID_instanceID(select_insid,&ins_para);  
	get_instance_dbus_connection(ins_para, &paraHead1, INSTANCE_STATE_WEB);

	cgiHeaderContentType("text/html");
	fprintf(cgiOut,"<html xmlns=\"http://www.w3.org/1999/xhtml\"><head>");
	fprintf(cgiOut,"<meta http-equiv=Content-Type content=text/html; charset=gb2312>");
	fprintf(cgiOut,"<title>Wtp</title>");
	fprintf(cgiOut,"<link rel=stylesheet href=/style.css type=text/css>"\
	"</head>");     
  memset(IsSubmit,0,sizeof(IsSubmit));  
  cgiFormStringNoNewlines("SubmitFlag", IsSubmit, 5);
  if((cgiFormSubmitClicked("groupcon_apply") == cgiFormSuccess)&&(strcmp(IsSubmit,"")))
  {
    	if(paraHead1)
	{
		config_wtpgroup(paraHead1,groupID,lpublic,lwlan);	 
	}
  }  
  fprintf(cgiOut,"<body>"\
  "<form method=post>"\
  "<div align=center>"\
  "<table width=976 border=0 cellpadding=0 cellspacing=0>"\
  "<tr>"\
    "<td width=8 align=left valign=top background=/images/di22.jpg><img src=/images/youce4.jpg width=8 height=30/></td>"\
    "<td width=51 align=left valign=bottom background=/images/di22.jpg><img src=/images/youce33.jpg width=37 height=24/></td>"\
    "<td width=153 align=left valign=bottom id=%s background=/images/di22.jpg>%s</td>",search(lpublic,"title_style"),search(lwlan,"ap"));
    fprintf(cgiOut,"<td width=690 align=right valign=bottom background=/images/di22.jpg>");
    	  fprintf(cgiOut,"<table width=155 border=0 cellspacing=0 cellpadding=0>"\
          "<tr>"\
          "<td width=62 align=center><input id=but type=submit name=groupcon_apply style=background-image:url(/images/%s) value=""></td>",search(lpublic,"img_ok"));
	  fprintf(cgiOut,"<td width=62 align=center><a href=wp_wtpgrouplist.cgi?UN=%s&INSTANCE_ID=%s target=mainFrame><img src=/images/%s border=0 width=62 height=20/></a></td>",m,select_insid,search(lpublic,"img_cancel"));
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
			    "<td align=left id=tdleft><a href=wp_wtplis.cgi?UN=%s target=mainFrame class=top><font id=yingwen_san>AP</font><font id=%s> %s</font></a></td>",m,search(lpublic,"menu_san"),search(lpublic,"list"));			
                  fprintf(cgiOut,"</tr>"\
				  "<tr height=25>"\
  					"<td align=left id=tdleft><a href=wp_wtpsearch.cgi?UN=%s target=mainFrame class=top><font id=%s>%s</font><font id=yingwen_san> AP</font></a></td>",m,search(lpublic,"menu_san"),search(lpublic,"Search"));                       
                  fprintf(cgiOut,"</tr>"\
                  "<tr height=25>"\
  					"<td align=left id=tdleft><a href=wp_wtpnew.cgi?UN=%s target=mainFrame class=top><font id=%s>%s</font><font id=yingwen_san> AP</font></a></td>",m,search(lpublic,"menu_san"),search(lpublic,"create"));                       
                  fprintf(cgiOut,"</tr>");

		    fprintf(cgiOut,"<tr height=25>"\
		    "<td align=left id=tdleft background=/images/bottom_bg.gif style=\"border-right:0\"><font id=%s> %s</font></td>",search(lpublic,"menu_san"),search(lwlan,"ap_group_config"));	 /*突出显示*/
		  fprintf(cgiOut,"</tr>");
				    
		    fprintf(cgiOut,"<tr height=25>"\
					  "<td align=left id=tdleft><a href=wp_wtpgroupnew.cgi?UN=%s target=mainFrame class=top><font id=%s>%s</font></a></td>",m,search(lpublic,"menu_san"),search(lwlan,"create_apgroup"));			    
		    fprintf(cgiOut,"</tr>");
		    
		fprintf(cgiOut,"<tr height=25>"\
  					"<td align=left id=tdleft><a href=wp_wtpver.cgi?UN=%s target=mainFrame class=top><font id=yingwen_san>AP</font><font id=%s> %s</font></a></td>",m,search(lpublic,"menu_san"),search(lpublic,"mode"));                       
                  fprintf(cgiOut,"</tr>"\
			      "<tr height=25>"\
  					"<td align=left id=tdleft><a href=wp_wtpdown.cgi?UN=%s target=mainFrame class=top><font id=%s>%s</font></a></td>",m,search(lpublic,"menu_san"),search(lpublic,"version_upload"));                       
                  fprintf(cgiOut,"</tr>"\
				  "<tr height=25>"\
				    "<td align=left id=tdleft><a href=wp_verbind.cgi?UN=%s target=mainFrame class=top><font id=%s>%s</font></a></td>",m,search(lpublic,"menu_san"),search(lwlan,"version_bind"));                       
                  fprintf(cgiOut,"</tr>"\
				  "<tr height=25>"\
  					"<td align=left id=tdleft><a href=wp_wtpupgrade.cgi?UN=%s target=mainFrame class=top><font id=%s>%s</font></a></td>",m,search(lpublic,"menu_san"),search(lwlan,"batch_update"));                       
                  fprintf(cgiOut,"</tr>"\
				  "<tr height=25>"\
  					"<td align=left id=tdleft><a href=wp_wtpbw.cgi?UN=%s target=mainFrame class=top><font id=yingwen_san>MAC </font><font id=%s>%s</font></a></td>",m,search(lpublic,"menu_san"),search(lwlan,"mac_filter"));                       
                  fprintf(cgiOut,"</tr>"\
				  "<tr height=25>"\
  					"<td align=left id=tdleft><a href=wp_showAutoAp.cgi?UN=%s target=mainFrame class=top><font id=%s>%s</font><font id=yingwen_san>AP</font><font id=%s>%s</font></a></td>",m,search(lpublic,"menu_san"),search(lpublic,"show_auto"),search(lpublic,"menu_san"),search(lpublic,"policy"));                       
                  fprintf(cgiOut,"</tr>"\
                  "<tr height=25>"\
  					"<td align=left id=tdleft><a href=wp_conAutoAp.cgi?UN=%s target=mainFrame class=top><font id=%s>%s</font><font id=yingwen_san>AP</font><font id=%s>%s</font></a></td>",m,search(lpublic,"menu_san"),search(lpublic,"config_auto"),search(lpublic,"menu_san"),search(lpublic,"policy"));                       
                  fprintf(cgiOut,"</tr>");
				  for(i=0;i<3;i++)
	              {
  				    fprintf(cgiOut,"<tr height=25>"\
                      "<td id=tdleft>&nbsp;</td>"\
                    "</tr>");
	              }
                fprintf(cgiOut,"</table>"\
              "</td>"\
              "<td align=left style=\"background-color:#ffffff; border-right:1px solid #707070; padding-left:30px\">"\
                      "<table width=720 border=0 cellspacing=0 cellpadding=0>"\
				"<tr>"\
				  "<td id=ins_style>%s:%s</td>",search(lpublic,"instance"),select_insid);
				fprintf(cgiOut,"</tr>"\
				"<tr>"\
				  "<td>");
		  status = system("bind_inter.sh"); 
		             fprintf(cgiOut,"<table width=720 border=0 cellspacing=0 cellpadding=0>"\
					   "<tr height=30 align=left>"\
					   	"<td id=thead5 align=left>%s %s</td>",search(lwlan,"ap_group_config"),groupID);		
			    fprintf(cgiOut,"</tr>"\
					   "</table>"\
				  "</td>"\
				"</tr>"\
				"<tr><td align=center style=\"padding-left:20px\">");
		fprintf(cgiOut,"<table width=720 border=0 cellspacing=0 cellpadding=0>");
		fprintf(cgiOut,"<tr height=30>");
		fprintf(cgiOut,"<td width=150>group %s:</td>",search(lpublic,"name"));				 
                fprintf(cgiOut,"<td width=100 align=left>%s</td>",groupname);

		
		fprintf(cgiOut,"<tr height=30>");
		   fprintf(cgiOut,"<td width=150>%s:</td>",search(lwlan,"bind_interface"));				 
                 fprintf(cgiOut,"<td width=100 align=left>");
		 status = system("bind_inter.sh"); 
		 if(status==0)
		 {
			fprintf(cgiOut,"<select name=bind_interface id=bind_interface style=width:100px>");
			if((fp=fopen("/var/run/apache2/bind_inter.tmp","r"))==NULL)		
			{
				ShowAlert(search(lpublic,"error_open"));
			}
			 else
			 {
				memset(BindInter,0,sizeof(BindInter));
				Inter=fgets(BindInter,20,fp);
				fprintf(cgiOut,"<option value=none></option>");
				while(Inter!=NULL)
				{
				   fprintf(cgiOut,"<option value=%s>%s</option>",Inter,Inter);
				   memset(BindInter,0,sizeof(BindInter));
				   Inter=fgets(BindInter,20,fp);
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
		  fprintf(cgiOut,"<td width=470 align=left style=\"padding-left:30px\"><font color=red>(%s)</font>"\
	  				 "</td>",search(lwlan,"wtp_bind"));
                fprintf(cgiOut,"</tr>");
		
                fprintf(cgiOut,"<tr height=30>"\
	                	       "<td>AP %s:</td>",search(lwlan,"state"));
	                fprintf(cgiOut,"<td align=left><select name=wtp_use id=wtp_use style=width:100px>");
	  		fprintf(cgiOut,"<option value=none></option>"\
					"<option value=unused>unused</option>"\
	  				"<option value=used>used</option>");
		        fprintf(cgiOut,"</select></td>");
			fprintf(cgiOut,"<td align=left style=\"padding-left:30px\"><font color=red>(%s)</font></td>",search(lwlan,"wtp_able"));
                fprintf(cgiOut,"</tr>");
		
		fprintf(cgiOut,"<tr height=30>"\
					"<td>AP %s:</td>",search(lpublic,"name"));
			fprintf(cgiOut,"<td><input name=ap_new_name size=15 maxLength=%d onkeypress=\"return event.keyCode!=32\" value=\"\"></td>",DEFAULT_LEN-1);
			 fprintf(cgiOut,"<td align=left style=\"padding-left:30px\"><font color=red>(%s)</font></td>",search(lwlan,"mod_ap_name"));
		fprintf(cgiOut,"</tr>"\
				"<tr height=30>"\
						"<td>%s:</td>",search(lwlan,"wlan_sta_max"));
				fprintf(cgiOut,"<td><input name=max_num size=15 maxLength=5 onkeypress=\"return event.keyCode>=48&&event.keyCode<=57\" value=\"\"></td>"\
				 				"<td align=left style=\"padding-left:30px\"><font color=red>(0--32767)</font></td>"\
				"</tr>"
				"<tr height=30>"\
						"<td>%s:</td>",search(lwlan,"wtp_triger_num"));
				fprintf(cgiOut,"<td><input name=wtp_triger_num size=15 maxLength=2 onkeypress=\"return event.keyCode>=48&&event.keyCode<=57\" value=\"\"></td>"\
				 				"<td align=left style=\"padding-left:30px\"><font color=red>(1--64)</font></td>"\
				"</tr>"\
				"<tr height=30>"\
						"<td>%s:</td>",search(lwlan,"wtp_flow_triger"));
				fprintf(cgiOut,"<td><input name=wtp_flow_triger size=15 maxLength=4 onkeypress=\"return event.keyCode>=48&&event.keyCode<=57\" value=\"\"></td>"\
				 				"<td align=left style=\"padding-left:30px\"><font color=red>(0--1024)</font></td>"\
				"</tr>"\
				"<tr height=30>"\
						"<td>%s:</td>",search(lpublic,"ntp_syn"));
                fprintf(cgiOut,"<td><select name=ntp_synch_type id=ntp_synch_type style=width:100px>"\
                                  "<option value=></option>"\
                                  "<option value=start>start</option>"\
                                  "<option value=stop>stop</option>"\
                     			"</td>"\
                                "<td align=left style=\"padding-left:30px\"><input name=ntp_synch_value size=15 maxLength=5 onkeypress=\"return event.keyCode>=48&&event.keyCode<=57\"><font color=red>(60--65535)s</font></td>"\
				"</tr>"\
				"<tr height=30>"\
						"<td>%s:</td>",search(lwlan,"ap_sta_info_report_switch"));
                fprintf(cgiOut,"<td colspan=2><select name=ap_sta_info_report_switch id=ap_sta_info_report_switch style=width:100px>"\
                                  "<option value=></option>"\
                                  "<option value=enable>enable</option>"\
                                  "<option value=disable>disable</option>"\
                     			"</td>"\
				"</tr>"\
				"<tr height=30>"\
						"<td>%s:</td>",search(lwlan,"ap_extension_info_switch"));
                fprintf(cgiOut,"<td colspan=2><select name=ap_extension_info_switch id=ap_extension_info_switch style=width:100px>"\
                                  "<option value=></option>"\
                                  "<option value=enable>enable</option>"\
                                  "<option value=disable>disable</option>"\
                     			"</td>"\
				"</tr>"\
				"<tr height=30>"\
						"<td>%s:</td>",search(lwlan,"ap_eth_rate"));
                fprintf(cgiOut,"<td><select name=ap_eth_ifindex id=ap_eth_ifindex style=width:100px>"\
                                  "<option value=></option>");
		   fprintf(cgiOut,"<option value=0>eth0</option>");
                fprintf(cgiOut,"</td>"\
                                "<td align=left style=\"padding-left:30px\"><select name=ap_eth_rate id=ap_eth_rate style=width:100px>"\
                                  "<option value=></option>"\
                                  "<option value=10>10M</option>"\
                                  "<option value=100>100M</option>"\
                                  "<option value=1000>1000M</option>"\
                                "</td>"\
				"</tr>");
				
				fprintf(cgiOut,"<tr>"\
				  "<td><input type=hidden name=UN value=%s></td>",m);
				  fprintf(cgiOut,"<td><input type=hidden name=INSTANCE_ID value=%s></td>",select_insid);
				   fprintf(cgiOut,"<td ><input type=hidden name=groupID value=%s></td>",groupID);
				   fprintf(cgiOut,"<td ><input type=hidden name=groupname value=%s></td>",groupname);
				   fprintf(cgiOut,"<td><input type=hidden name=SubmitFlag value=%d></td>",1);
				fprintf(cgiOut,"</tr>");			
			   fprintf(cgiOut,"</table>");
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

return 0;

}


void config_wtpgroup(instance_parameter *ins_para,char *id,struct list *lpublic,struct list *lwlan)   
{
	int use_state = 0,ret1 = 1,ret2 = 1,ret3 = 1,ret4 = 1,ret5 = 1,ret6 = 1,ret7 = 1,ret8 = 1,ret9 = 1,ret10 = 1;
	const int group_type = 1;
	int group_id= 0, type=0;
	char interf[20] = { 0 };
	char temp[100] = { 0 };
	char state[20] = { 0 };  
	char *endptr = NULL;  
	int hidden = 1;
	int fail_num=0;
	char new_name[DEFAULT_LEN+5] = { 0 };
	char maxnum[10] = { 0 };
	int sta_num = 0;
	char triger_num[10] = { 0 };
	char flow_triger[10] = { 0 };
	char ntp_synch_type[10] = { 0 };
	char ntp_synch_value[10] = { 0 };
	int ntp_synch_v = 3600;
	char ap_sta_info_report_switch[10] = { 0 };
	char ap_extension_info_switch[10] = { 0 };
	char ap_eth_ifindex[5] = { 0 };
	char ap_eth_rate[10] = { 0 };
	int trNum = 0;
	int flowNum = 0;
	struct WtpList *WtpList_Head = NULL;
	group_id=strtoul(id,0,10);

	memset(state,0,sizeof(state));
	memset(interf,0,sizeof(interf));
	
	cgiFormStringNoNewlines("bind_interface",interf,20);
	cgiFormStringNoNewlines("wtp_use",state,20);
	
	if(strcmp(state,"unused")==0)
		use_state=1;
	else if(strcmp(state,"used")==0)
		use_state=0;
	else
		use_state=2;
	
	if(use_state==1)
	{
		fprintf(stderr,"--------enter unused\n");																			  
		/*******************先wtp  unused，在绑定接口showerror1   showerror2******************/
		WtpList_Head = NULL;
		ret1=wtp_used_group(ins_para->parameter,ins_para->connection,group_type,group_id,use_state,&WtpList_Head);
		type = 1;
		showerror(type,ret1,lpublic,lwlan);
		if(strcmp(interf,"none") )
		{
			fprintf(stderr,"--------enter unused  interface\n");																			  
			WtpList_Head = NULL;
			ret2=wtp_apply_interface_group(ins_para->parameter,ins_para->connection,group_type,group_id,interf,&WtpList_Head);
			type = 2;
			showerror(type,ret2,lpublic,lwlan);
		}
	}
	else if(use_state == 0)
	{
		/*******************先绑定接口，在开启wtp  used  showerror2  showerror1******************/
		fprintf(stderr,"--------enter used  interface\n");																			  
		if(strcmp(interf,"none"))
		{
			WtpList_Head = NULL;
			ret2=wtp_apply_interface_group(ins_para->parameter,ins_para->connection,group_type,group_id,interf,&WtpList_Head);
			type = 2;
			showerror(type,ret2,lpublic,lwlan);
		}
		fprintf(stderr,"--------enter used\n");																			  
		WtpList_Head = NULL;
		ret1=wtp_used_group(ins_para->parameter,ins_para->connection,group_type,group_id,use_state,&WtpList_Head);
		type = 1;
		showerror(type,ret1,lpublic,lwlan);	
	}
	else
	{
		/*******************绑定接口showerror2******************/
		if(strcmp(interf,"none"))
		{
			fprintf(stderr,"--------enter none interf\n"); 																		  
			WtpList_Head = NULL;
			ret2=wtp_apply_interface_group(ins_para->parameter,ins_para->connection,group_type,group_id,interf,&WtpList_Head);
			type = 2;
			showerror(type,ret2,lpublic,lwlan);
		}
	}
	
		/*******************设置AP 名称showerror3******************/
	memset(new_name,0,sizeof(new_name));
	cgiFormStringNoNewlines("ap_new_name",new_name,DEFAULT_LEN+5);
	if(strcmp(new_name,"")!=0)
	{
		if(strchr(new_name,' ')==NULL)/*不包含空格*/
		{
			fprintf(stderr,"--------enter wtpname =%s\n",new_name); 																	  
			WtpList_Head = NULL;
			ret3=set_wtp_wtpname_group(ins_para->parameter,ins_para->connection,group_type,group_id,new_name,&WtpList_Head);
			type = 3;
			showerror(type,ret3,lpublic,lwlan);
		}
		else
		{
			ShowAlert(search(lpublic,"input_para_dont_contain_spaces"));
		}
	}

	 	/*******************设置允许最大接入数showerror4******************/
	 cgiFormStringNoNewlines("max_num",maxnum,10);
	 if(strcmp(maxnum,""))
	 {
		 sta_num=strtoul(maxnum,&endptr,10);	/*char转成int，10代表十进制*/
		 if((sta_num>=0)&&(sta_num<32768))
		 {
			WtpList_Head = NULL;
			ret4=config_wtp_max_sta_num_group(ins_para->parameter,ins_para->connection,group_type,group_id,maxnum,&WtpList_Head,&fail_num);
			type = 4;
			showerror(type,ret4,lpublic,lwlan);
		 }
		 else
		 {
		   memset(temp,0,sizeof(temp));
		   strncpy(temp,search(lwlan,"max_sta_num_illegal1"),sizeof(temp)-1);
		   strncat(temp,"32767",sizeof(temp)-strlen(temp)-1);
		   strncat(temp,search(lwlan,"max_sta_num_illegal2"),sizeof(temp)-strlen(temp)-1);
		   ShowAlert(temp);
		 }
	 }
	 /*******************负载均衡触发值(按人数)showerror5******************/
	memset(triger_num,0,sizeof(triger_num));
	cgiFormStringNoNewlines("wtp_triger_num",triger_num,10);
	if(strcmp(triger_num,"")!=0) 
	{
		trNum= strtoul(triger_num,&endptr,10);							
		if((trNum>0)&&(trNum<65)) 
		{
			WtpList_Head = NULL;
			ret5=config_wtp_triger_num_group(ins_para->parameter,ins_para->connection,group_type,group_id,triger_num,&WtpList_Head);
			type = 5;
			showerror(type,ret5,lpublic,lwlan);
		}
		else
		{
		  ShowAlert(search(lwlan,"triger_num_illegal"));
		}
	}
	/*******************负载均衡触发值(按流量)showerror6******************/
	memset(flow_triger,0,sizeof(flow_triger));
	cgiFormStringNoNewlines("wtp_flow_triger",flow_triger,10);
	if(strcmp(flow_triger,"")!=0) 
	{
		flowNum= strtoul(flow_triger,&endptr,10);					   	
		if((flowNum>=0)&&(flowNum<1025)) 
		{
			WtpList_Head = NULL;
			ret6=set_wtp_flow_trige_group(ins_para->parameter,ins_para->connection,group_type,group_id,flow_triger,&WtpList_Head);  
			type = 6;
			showerror(type,ret6,lpublic,lwlan);
		}
		else
		{
			ShowAlert(search(lwlan,"flow_triger_num_illegal"));
		}
	}
	/*******************NTP同步 showerror7******************/
	memset(ntp_synch_type,0,sizeof(ntp_synch_type));
	cgiFormStringNoNewlines("ntp_synch_type",ntp_synch_type,10);
	memset(ntp_synch_value,0,sizeof(ntp_synch_value));
	cgiFormStringNoNewlines("ntp_synch_value",ntp_synch_value,10);
	if((strcmp(ntp_synch_type,"")!=0) && (strcmp(ntp_synch_value,"")!=0))
	{
		fprintf(stderr,"--------enter ntp_synch_type =%s\n",ntp_synch_type); 																	  
		fprintf(stderr,"--------enter ntp_synch_value =%s\n",ntp_synch_value); 																	  
		ntp_synch_v = strtoul(ntp_synch_value,&endptr,10);
		if((ntp_synch_v>59)&&(ntp_synch_v<65536))
		{
			WtpList_Head = NULL;
			ret7=set_ac_ap_ntp_func_group(ins_para->parameter,ins_para->connection,group_type,group_id,ntp_synch_type,ntp_synch_value,&WtpList_Head);
			type = 7;
			showerror(type,ret7,lpublic,lwlan);
		}
		else
		{
			ShowAlert(search(lwlan,"invalid_ntp_interval"));
		}
	}
	/*******************AP信息上报开关 showerror8******************/
	memset(ap_sta_info_report_switch,0,sizeof(ap_sta_info_report_switch));
	cgiFormStringNoNewlines("ap_sta_info_report_switch",ap_sta_info_report_switch,10);
	if(strcmp(ap_sta_info_report_switch,"")!=0)
	{
		WtpList_Head = NULL;
		ret8=set_ap_sta_infomation_report_enable_func_group(ins_para->parameter,ins_para->connection,group_type,group_id,ap_sta_info_report_switch,&WtpList_Head);   
		type = 8;
		showerror(type,ret8,lpublic,lwlan);
	}
	/*******************AP扩展信息上报开关 showerror9******************/
	memset(ap_extension_info_switch,0,sizeof(ap_extension_info_switch));
	cgiFormStringNoNewlines("ap_extension_info_switch",ap_extension_info_switch,10);
	if(strcmp(ap_extension_info_switch,"")!=0)
	{
		WtpList_Head = NULL;
		ret9=set_ap_extension_infomation_enable_group(ins_para->parameter,ins_para->connection,group_type,group_id,ap_extension_info_switch,&WtpList_Head);   
		type = 9;
		showerror(type,ret9,lpublic,lwlan);
	}
	/*******************AP以太网口速率showerror10******************/

	memset(ap_eth_ifindex,0,sizeof(ap_eth_ifindex));
	cgiFormStringNoNewlines("ap_eth_ifindex",ap_eth_ifindex,5);
	memset(ap_eth_rate,0,sizeof(ap_eth_rate));
	cgiFormStringNoNewlines("ap_eth_rate",ap_eth_rate,10);
	if((strcmp(ap_eth_ifindex,"")!=0)&&(strcmp(ap_eth_rate,"")!=0))
	{
		WtpList_Head = NULL;
		ret10=set_ap_if_rate_cmd_group(ins_para->parameter,ins_para->connection,group_type,group_id,ap_eth_ifindex,ap_eth_rate,&WtpList_Head); 
		type = 10;
		showerror(type,ret10,lpublic,lwlan);
	}
	fprintf(stderr,"--------ret1=%d,ret2=%d,ret3=%d,ret4=%d,ret5=%d,ret6=%d\n",ret1,ret2,ret3,ret4,ret5,ret6);																			  
	fprintf(stderr,"--------ret7=%d,ret8=%d,ret9=%d,ret10=%d\n",ret7,ret8,ret9,ret10);																			  

	if((ret1 = 1)&&(ret2==1)&&(ret3==1)&&(ret4==1)&&(ret5==1)&&(ret6==1)&&(ret7==1)&&(ret8==1)&&(ret9==1)&&(ret10==1))
		ShowAlert(search(lpublic,"oper_succ"));  
}

void showerror(int type,int ret, struct list *lpublic,struct list *lwlan)
							/*type == 1:wtp_used_group, type == 2:wtp_apply_interface_group; type == 3:set_wtp_wtpname_group*/
							/*type == 4:config_wtp_max_sta_num_group, type == 5:config_wtp_triger_num_group; type == 6:set_wtp_flow_trige_group*/
							/*type == 7:set_ac_ap_ntp_func_group, type == 8:set_ap_sta_infomation_report_enable_func_group; */
							/*type == 9:set_ap_extension_infomation_enable_group,type == 10:set_ap_if_rate_cmd_group, */
{
	char alt[100] = { 0 };
	char max_wtp_num[10] = { 0 };
	char temp[100] = { 0 };
	char default_len[10] = { 0 };

	if(type == 1)  /*type == 1:wtp_used_group*/
	{
		switch(ret)
		{
			case SNMPD_CONNECTION_ERROR:
			case 0:
			{
				ShowAlert(search(lwlan,"set_wtp_use_fail"));
				break;
			}
			case 1:break;
			case -1:
			{
				ShowAlert(search(lwlan,"wtp_not_exist"));
				break;
			}
			case -2:
			{
				ShowAlert(search(lwlan,"bind_interface_first"));
				break;
			}
			case -3:
			{
				ShowAlert(search(lwlan,"bind_wlan_first"));
				break;
			}
			case -4:
			{
				ShowAlert(search(lwlan,"map_l3_inter_error"));
				break;
			}
			case -5:
			{
				ShowAlert(search(lwlan,"bss_ifpolicy_conflict"));			
				break;
			}
			case -6:
			{
				ShowAlert(search(lpublic,"error"));
				break;
			}
			case -7:
			{
				memset(alt,0,sizeof(alt));
				strncpy(alt,search(lwlan,"wtp_id_illegal1"),sizeof(alt)-1);
				memset(max_wtp_num,0,sizeof(max_wtp_num));
				snprintf(max_wtp_num,sizeof(max_wtp_num)-1,"%d",WTP_NUM-1);
				strncat(alt,max_wtp_num,sizeof(alt)-strlen(alt)-1);
				strncat(alt,search(lwlan,"wtp_id_illegal2"),sizeof(alt)-strlen(alt)-1);
				ShowAlert(alt);
				break;
			}
			case -8:
			{
				ShowAlert(search(lwlan,"wtp_group_not_exist"));
				break;
			}	  
			case -9:
			{
				ShowAlert(search(lwlan,"set_wtp_use_part_fail"));
				break;
			}	  
		}
	}
	else if(type == 2)/*type == 2:wtp_apply_interface_group; */
	{
		switch(ret)
		{
			case SNMPD_CONNECTION_ERROR:
			case 0:
			{
				ShowAlert(search(lwlan,"bind_interface_fail"));
				break;
			}
			case 1:break;
			case -1:
			{
				  ShowAlert(search(lpublic,"interface_too_long"));	
				  break; 
			}
			case -2:
			case -12:
			{
				  ShowAlert(search(lpublic,"interface_not_exist"));
				  break;
			}
			case -3:
			case -13:
			{
				  ShowAlert(search(lwlan,"delete_bind_wlan_first"));
				  break;
			}
			case -4:
			{
				 ShowAlert(search(lpublic,"error"));
				break;
			}
			case -5:
			{
				  memset(alt,0,sizeof(alt));
				  strncpy(alt,search(lwlan,"wtp_id_illegal1"),sizeof(alt)-1);
				  memset(max_wtp_num,0,sizeof(max_wtp_num));
				  snprintf(max_wtp_num,sizeof(max_wtp_num)-1,"%d",WTP_NUM-1);
				  strncat(alt,max_wtp_num,sizeof(alt)-strlen(alt)-1);
				  strncat(alt,search(lwlan,"wtp_id_illegal2"),sizeof(alt)-strlen(alt)-1);
				  ShowAlert(alt);
				  break;
			}
			case -7:
			{
				ShowAlert(search(lwlan,"bind_interface_part_fail"));
				break;
			}				
			case -8:
			{
				ShowAlert(search(lwlan,"ipv4_ipv6"));
				break;
			}
			case -9:
			{
				ShowAlert(search(lwlan,"ipv4_interface_not_exist"));
				break;
			}
			case -10:
			{
				 ShowAlert(search(lwlan,"ipv4_interface_failed"));
				 break; 
			 }
			case -11:
			{
				 ShowAlert(search(lwlan,"ipv4_interface_dele"));
				 break; 
			 }
			case -14:
			{
				 ShowAlert(search(lwlan,"no_local_interface"));
				 break; 
			 }
			default:
			{
				ShowAlert(search(lpublic,"inter_error"));
				break; 
			}
		}
		
	}
	else if(type == 3) /* type == 3:set_wtp_wtpname_group*/
	{
		switch(ret)
		{
			case SNMPD_CONNECTION_ERROR:
			case 0:
			{
				ShowAlert(search(lwlan,"mod_ap_name_fail"));
				break;
			}
			case 1:break;
			case -1:
			{
				memset(temp,0,sizeof(temp));
				strncpy(temp,search(lwlan,"most1"),sizeof(temp)-1);
				memset(default_len,0,sizeof(default_len));
				snprintf(default_len,sizeof(default_len)-1,"%d",DEFAULT_LEN-1);
				strncat(temp,default_len,sizeof(temp)-strlen(temp)-1);
				strncat(temp,search(lwlan,"most2"),sizeof(temp)-strlen(temp)-1);
				ShowAlert(temp);
				break;
			}
			case -2:
			case -3:
			{
				ShowAlert(search(lwlan,"wtp_not_exist"));
				break;
			}
			case -5:
			{
				ShowAlert(search(lwlan,"mod_ap_name_part_fail"));
				break;
			}
			case -4:
			case -6:
			{
				ShowAlert(search(lwlan,"wtp_group_not_exist"));
				break;
			}
		}
	}
	else if(type == 4) /*type == 4:config_wtp_max_sta_num_group*/
	{
		switch (ret)
		{
			case SNMPD_CONNECTION_ERROR:
			case 0:
			{
				memset(temp,0,sizeof(temp));
				strncpy(temp,search(lwlan,"con_bss_max_sta_num_fail1"),sizeof(temp)-1);
				strncat(temp,"WTP",sizeof(temp)-strlen(temp)-1);
				strncat(temp,search(lwlan,"con_bss_max_sta_num_fail2"),sizeof(temp)-strlen(temp)-1);
				ShowAlert(temp);
				break;
				}
			case 1:break;
			case -1:
			{
				ShowAlert(search(lwlan,"wtp_not_exist"));		/*wtp not exist*/
				break; 
			}
			case -2:
			{
				ShowAlert(search(lwlan,"more_sta_has_access")); 	/*more sta(s) has accessed before you set max sta num*/
				break; 
				}
			case -3:
			{
				ShowAlert(search(lpublic,"oper_fail")); 		/*operation fail*/
				break; 
			}
			case -4:
			{
				  ShowAlert(search(lpublic,"error"));				 /*error*/
				  break;  
			}
			case -5:
			{													 /*WTP ID非法*/
				  memset(temp,0,sizeof(temp));
				  strncpy(temp,search(lwlan,"wtp_id_illegal1"),sizeof(temp)-1);
				  memset(max_wtp_num,0,sizeof(max_wtp_num));
				  snprintf(max_wtp_num,sizeof(max_wtp_num)-1,"%d",WTP_NUM-1);
				  strncat(temp,max_wtp_num,sizeof(temp)-strlen(temp)-1);
				  strncat(temp,search(lwlan,"wtp_id_illegal2"),sizeof(temp)-strlen(temp)-1);
				  ShowAlert(temp);
				  break;
			}
			case -7:
			{
				  ShowAlert(search(lwlan,"wtp_group_not_exist"));				 /*error*/
				  break;  
			}
			case -8:
			{
				  ShowAlert(search(lwlan,"con_bss_max_sta_num_part_fail"));				 /*error*/
				  break;  
			}
			case -9:
			{
				  ShowAlert(search(lpublic,"wtp_group_not_exist"));				 /*error*/
				  break;  
			}
			case -10:
			{													 /*input num should be 0-64*/
				memset(temp,0,sizeof(temp));
				strncpy(temp,search(lwlan,"max_sta_num_illegal1"),sizeof(temp)-1);
				strncat(temp,"32767",sizeof(temp)-strlen(temp)-1);
				strncat(temp,search(lwlan,"max_sta_num_illegal2"),sizeof(temp)-strlen(temp)-1);
				ShowAlert(temp);
				break;	
			}
		}
	}
	else if(type == 5) /*type == 5:config_wtp_triger_num_group; */
	{
		switch(ret)
		{
			case SNMPD_CONNECTION_ERROR:
			case 0:
			{
				ShowAlert(search(lwlan,"con_triger_num_fail"));
				break;
			} 
			case 1:break;
			case -1:
			{
				ShowAlert(search(lwlan,"wtp_not_exist")); 
				break; 
			}
			case -2:
			{
				ShowAlert(search(lpublic,"oper_fail")); 
				break; 
			}
			case -3:
			{
				ShowAlert(search(lwlan,"triger_num_little_sta_num")); 
				break; 
			}
			case -4:
			{
				ShowAlert(search(lpublic,"error")); 
				break;	
			}
			case -5:
			{														  /*WTP ID非法*/
				memset(alt,0,sizeof(alt));
				strncpy(alt,search(lwlan,"wtp_id_illegal1"),sizeof(alt)-1);
				memset(max_wtp_num,0,sizeof(max_wtp_num));
				snprintf(max_wtp_num,sizeof(max_wtp_num)-1,"%d",WTP_NUM-1);
				strncat(alt,max_wtp_num,sizeof(alt)-strlen(alt)-1);
				strncat(alt,search(lwlan,"wtp_id_illegal2"),sizeof(alt)-strlen(alt)-1);
				ShowAlert(alt);
				break;
			}
			case -6:
			case -8:
			{
				ShowAlert(search(lpublic,"wtp_group_not_exist")); 
				break;	
			}
			case -7:
			{
				ShowAlert(search(lwlan,"con_triger_num_part_fail")); 
				break;	
			}
			case -9:
			{
				ShowAlert(search(lpublic,"unknown_id_format")); 	 /*unknown id format*/
				break; 
			}
			case -10:
			{
				ShowAlert(search(lwlan,"triger_num_illegal"));		 /*input triger num should be 1~64*/
				break; 
			}
		}
	}
	else if(type == 6)/* type == 6:set_wtp_flow_trige_group*/
	{
		switch(ret)
		{
			case SNMPD_CONNECTION_ERROR:
			case 0:
			{
				ShowAlert(search(lwlan,"con_flow_triger_fail"));
				break;
			}
			case 1:break;
			case -1:
			{
				ShowAlert(search(lwlan,"wtp_not_exist"));              
				break; 
			}
			case -2:
			{
				ShowAlert(search(lpublic,"oper_fail"));                 
				break; 
			}
			case -3:
			{
				ShowAlert(search(lwlan,"flow_triger_num_illegal"));    
				break; 
			}
			case -4:
			{
				ShowAlert(search(lpublic,"error"));                    
				break;  
			}
			case -5:
			{														  /*WTP ID非法*/
				memset(alt,0,sizeof(alt));
				strncpy(alt,search(lwlan,"wtp_id_illegal1"),sizeof(alt)-1);
				memset(max_wtp_num,0,sizeof(max_wtp_num));
				snprintf(max_wtp_num,sizeof(max_wtp_num)-1,"%d",WTP_NUM-1);
				strncat(alt,max_wtp_num,sizeof(alt)-strlen(alt)-1);
				strncat(alt,search(lwlan,"wtp_id_illegal2"),sizeof(alt)-strlen(alt)-1);
				ShowAlert(alt);
				break;
			}
			case -6:
			case -8:
			{
				ShowAlert(search(lpublic,"wtp_group_not_exist"));                    
				break;  
			}
			case -7:
			{
				ShowAlert(search(lwlan,"con_flow_triger_part_fail"));                    
				break;  
			}
			case -9:
			{
				ShowAlert(search(lpublic,"unknown_id_format"));   	
				break; 
			}
			case -10:
			{
				ShowAlert(search(lwlan,"flow_triger_num_illegal"));   
				break; 
			}
		}
	}
	else if(type == 7)/* type == 7:set_ac_ap_ntp_func_group*/
	{
		switch(ret)
		{
			case SNMPD_CONNECTION_ERROR:
			case 0:
			{
				ShowAlert(search(lpublic,"ntp_syn_fail"));
				break;
			}
			case 1:break;
			case -2:
			{
				ShowAlert(search(lwlan,"wtp_not_exist"));
				break;
			}
			case -3:
			{
				ShowAlert(search(lpublic,"error"));
				break;
			}
			case -4:
			{
				memset(alt,0,sizeof(alt));
				strncpy(alt,search(lwlan,"wtp_id_illegal1"),sizeof(alt)-1);
				memset(max_wtp_num,0,sizeof(max_wtp_num));
				snprintf(max_wtp_num,sizeof(max_wtp_num)-1,"%d",WTP_NUM-1);
				strncat(alt,max_wtp_num,sizeof(alt)-strlen(alt)-1);
				strncat(alt,search(lwlan,"wtp_id_illegal2"),sizeof(alt)-strlen(alt)-1);
				ShowAlert(alt);
				break;
			}
			case -5:
			{
				ShowAlert(search(lpublic,"input_para_error"));
				break;
			}
			case -6:
			{
				ShowAlert(search(lwlan,"invalid_ntp_interval"));
				break;
			}
			case -7:
			case -9:
			{
				ShowAlert(search(lwlan,"wtp_group_not_exist"));
				break;
			}
			case -8:
			{
				ShowAlert(search(lwlan,"ntp_syn_part_fail"));
				break;
			}
		}
	}
	else if(type == 8)/* type == 8:set_ap_sta_infomation_report_enable_func_group*/
	{
		switch(ret)
		{
			case SNMPD_CONNECTION_ERROR:
			case 0:
			{
				ShowAlert(search(lwlan,"con_ap_sta_info_report_switch_fail"));
				break;
			}
			case 1:break;
			case -1:
			{
				ShowAlert(search(lpublic,"input_para_illegal"));
				break;
			}
			case -2:
			{
				ShowAlert(search(lwlan,"wtp_not_exist"));
				break;
			}
			case -3:
			{
				ShowAlert(search(lwlan,"wtp_not_run"));
				break;
			}	  
			case -4:
			{
				ShowAlert(search(lpublic,"error"));
				break;
			}
			case -5:
			{
				memset(alt,0,sizeof(alt));
				strncpy(alt,search(lwlan,"wtp_id_illegal1"),sizeof(alt)-1);
				memset(max_wtp_num,0,sizeof(max_wtp_num));
				snprintf(max_wtp_num,sizeof(max_wtp_num)-1,"%d",WTP_NUM-1);
				strncat(alt,max_wtp_num,sizeof(alt)-strlen(alt)-1);
				strncat(alt,search(lwlan,"wtp_id_illegal2"),sizeof(alt)-strlen(alt)-1);
				ShowAlert(alt);
				break;
			}
			case -6:
			case -8:
			{
				ShowAlert(search(lpublic,"wtp_group_not_exist"));
				break;
			}
			case -7:
			{
				ShowAlert(search(lwlan,"con_ap_sta_info_report_switch_part_fail"));
				break;
			}
		}
	}
	else if(type == 9)/* type == 9:set_ap_extension_infomation_enable_group*/
	{
		switch(ret)										  
		{
			case SNMPD_CONNECTION_ERROR:
			case 0:
			{
				ShowAlert(search(lwlan,"con_ap_sta_info_report_extension_switch_fail"));
				break;
			}
			case 1:break;
			case -1:
			{
				ShowAlert(search(lpublic,"input_para_illegal"));
				break;
			}
			case -2:
			{
				ShowAlert(search(lwlan,"wtp_not_exist"));
				break;
			}
			case -3:
			{
				ShowAlert(search(lwlan,"wtp_not_run"));
				break;
			}	  
			case -4:
			{
				ShowAlert(search(lpublic,"error"));
				break;
			}
			case -5:
			{
				memset(alt,0,sizeof(alt));
				strncpy(alt,search(lwlan,"wtp_id_illegal1"),sizeof(alt)-1);
				memset(max_wtp_num,0,sizeof(max_wtp_num));
				snprintf(max_wtp_num,sizeof(max_wtp_num)-1,"%d",WTP_NUM-1);
				strncat(alt,max_wtp_num,sizeof(alt)-strlen(alt)-1);
				strncat(alt,search(lwlan,"wtp_id_illegal2"),sizeof(alt)-strlen(alt)-1);
				ShowAlert(alt);
				break;
			}
			case -6:
			case -8:
			{
				ShowAlert(search(lpublic,"wtp_group_not_exist"));
				break;
			}
			case -7:
			{
				ShowAlert(search(lwlan,"con_ap_sta_info_report_extension_switch_part_fail"));
				break;
			}
		}
	}
	else if(type == 10)/* type == 10:set_ap_if_rate_cmd_group*/
	{
		switch(ret)										  
		{
			case SNMPD_CONNECTION_ERROR:
			case 0:
			{
				ShowAlert(search(lwlan,"con_ap_eth_rate_fail"));
				break;
			}
			case 1:break;
			case -1:
			{
				ShowAlert(search(lpublic,"unknown_id_format"));
				break;
			}
			case -2:
			{
				ShowAlert(search(lpublic,"input_para_illegal"));
				break;
			}
			case -3:
			{
				ShowAlert(search(lwlan,"wtp_not_run"));
				break;
			}	  
			case -4:
			{
				ShowAlert(search(lwlan,"wtp_not_exist"));
				break;
			}
			case -5:
			{
				ShowAlert(search(lwlan,"eth_ifindex_not_exist"));
				break;
			}
			case -6:
			{
				ShowAlert(search(lpublic,"error"));
				break;
			}
			case -7:
			{
				memset(alt,0,sizeof(alt));
				strncpy(alt,search(lwlan,"wtp_id_illegal1"),sizeof(alt)-1);
				memset(max_wtp_num,0,sizeof(max_wtp_num));
				snprintf(max_wtp_num,sizeof(max_wtp_num)-1,"%d",WTP_NUM-1);
				strncat(alt,max_wtp_num,sizeof(alt)-strlen(alt)-1);
				strncat(alt,search(lwlan,"wtp_id_illegal2"),sizeof(alt)-strlen(alt)-1);
				ShowAlert(alt);
				break;
			}
			case -8:
			case -10:
			{
				ShowAlert(search(lwlan,"wtp_group_not_exist"));
				break;
			}	  
			case -9:
			{
				ShowAlert(search(lwlan,"con_ap_eth_rate_part_fail"));
				break;
			}	  
		}
	}

}

