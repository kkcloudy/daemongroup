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
* wp_stasumary.c
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
*/


#include <stdio.h>
#include "cgic.h"
#include <string.h>
#include <stdlib.h>
#include "wcpss/asd/asd.h"
#include "ws_sta.h"
#include "ws_usrinfo.h"
#include "ws_err.h"
#include "ws_ec.h"
#include "ws_dcli_vrrp.h"
#include "ws_init_dbus.h"
#include "ws_dbus_list_interface.h"

void ShowStationPage(char *m,int n,struct list *lpublic,struct list *lwlan);    


int cgiMain()
{
  char encry[BUF_LEN] = { 0 }; 
  char page_no[10] = { 0 };  
  char *str = NULL;        
  char *endptr = NULL;  
  int pno = 0;
  struct list *lpublic = NULL;   /*解析public.txt文件的链表头*/
  struct list *lwlan = NULL;     /*解析wlan.txt文件的链表头*/  
  lpublic=get_chain_head("../htdocs/text/public.txt");
  lwlan=get_chain_head("../htdocs/text/wlan.txt");
  
  DcliWInit();
  ccgi_dbus_init();
  memset(encry,0,sizeof(encry));
  memset(page_no,0,sizeof(page_no));
  cgiFormStringNoNewlines("UN", encry, BUF_LEN); 
  str=dcryption(encry);
  if(str==NULL)
    ShowErrorPage(search(lpublic,"ill_user"));		 /*用户非法*/
  else
  {
    if(cgiFormStringNoNewlines("PN", page_no, 10)!=cgiFormNotFound )  /*点击翻页进入该页面*/
    {
      pno= strtoul(page_no,&endptr,10);	/*char转成int，10代表十进制*/ 
      ShowStationPage(encry,pno,lpublic,lwlan);
	}
	else
      ShowStationPage(encry,0,lpublic,lwlan);
  }
  release(lpublic);  
  release(lwlan);
  destroy_ccgi_dbus();
  return 0;
}

void ShowStationPage(char *m,int n,struct list *lpublic,struct list *lwlan)
{   
  int i = 0,result = 0;
  char select_insid[10] = { 0 };
  struct dcli_ac_info *ac = NULL;  
  struct dcli_wtp_info 	*wtp = NULL;
  struct dcli_wlan_info *wlan = NULL;
  instance_parameter *paraHead1 = NULL,*paraHead2 = NULL;
  instance_parameter *pq = NULL;
  char temp[10] = { 0 };
  dbus_parameter ins_para;
  cgiHeaderContentType("text/html");
  fprintf(cgiOut,"<html xmlns=\"http://www.w3.org/1999/xhtml\"><head>");
  fprintf(cgiOut,"<meta http-equiv=Content-Type content=text/html; charset=gb2312>");
  fprintf(cgiOut,"<title>Station</title>");
  fprintf(cgiOut,"<link rel=stylesheet href=/style.css type=text/css>"\
  "<style type=text/css>"\
  ".stanumlis {overflow-x:hidden; overflow:auto; width: 600; height: 100px; clip: rect( ); padding-top: 0px; padding-right: 0px; padding-bottom: 0px; padding-left: 0px} "\
  "</style>"\
  "</head>"\
	  "<script type=\"text/javascript\">"\
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
  "<script src=/instanceid_onchange.js>"\
  "</script>"\
  "<body>");  	
  memset(select_insid,0,sizeof(select_insid));
	cgiFormStringNoNewlines( "INSTANCE_ID", select_insid, 10 );
	if(strcmp(select_insid,"")==0)
	{ 
	  list_instance_parameter(&paraHead1, INSTANCE_STATE_WEB); 
	  if(paraHead1)
	  {
		  snprintf(select_insid,sizeof(select_insid)-1,"%d-%d-%d",paraHead1->parameter.slot_id,paraHead1->parameter.local_id,paraHead1->parameter.instance_id);
	  }
	}  
	else
	{
	  get_slotID_localID_instanceID(select_insid,&ins_para);  
	  get_instance_dbus_connection(ins_para, &paraHead1, INSTANCE_STATE_WEB);
	}
	fprintf(cgiOut,"<form>"\
		"<div align=center>"\
		"<table width=976 border=0 cellpadding=0 cellspacing=0>"\
        "<tr>"\
          "<td width=8 align=left valign=top background=/images/di22.jpg><img src=/images/youce4.jpg width=8 height=30/></td>"\
          "<td width=51 align=left valign=bottom background=/images/di22.jpg><img src=/images/youce33.jpg width=37 height=24/></td>"\
		  "<td width=153 align=left valign=bottom id=%s background=/images/di22.jpg>%s</td>",search(lpublic,"title_style"),search(lwlan,"station"));
		  fprintf(cgiOut,"<td width=690 align=right valign=bottom background=/images/di22.jpg>");  
				fprintf(cgiOut,"<table width=155 border=0 cellspacing=0 cellpadding=0>"\
				"<tr>"\
				"<td width=62 align=center><a href=wp_wlan.cgi?UN=%s target=mainFrame><img src=/images/%s border=0 width=62 height=20/></a></td>",m,search(lpublic,"img_ok"));
				fprintf(cgiOut,"<td width=62 align=center><a href=wp_wlan.cgi?UN=%s target=mainFrame><img src=/images/%s border=0 width=62 height=20/></a></td>",m,search(lpublic,"img_cancel"));
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
                  "</tr>"\
						"<tr height=25>"\
  					      "<td align=left id=tdleft><a href=wp_stalis.cgi?UN=%s target=mainFrame class=top><font id=%s>%s</font></a></td>",m,search(lpublic,"menu_san"),search(lwlan,"sta_list"));                       
                        fprintf(cgiOut,"</tr>"\
						"<tr height=25>"\
  					      "<td align=left id=tdleft><a href=wp_seachsta.cgi?UN=%s target=mainFrame class=top><font id=%s>%s</font></a></td>",m,search(lpublic,"menu_san"),search(lwlan,"search_sta"));                       
                        fprintf(cgiOut,"</tr>");
						if(paraHead1)
						{
							result = show_sta_summary(paraHead1->parameter,paraHead1->connection,&ac,NULL);
						}
						for(i=0;i<20;i++)
						{
						  fprintf(cgiOut,"<tr height=25>"\
							"<td id=tdleft>&nbsp;</td>"\
						  "</tr>");
						}
					  fprintf(cgiOut,"</table>"\
              "</td>"\
              "<td align=left valign=top style=\"background-color:#ffffff; border-right:1px solid #707070; padding-left:30px; padding-top:10px\">"\
			 "<table width=760 border=0 bgcolor=#ffffff cellspacing=0 cellpadding=0>"\
			  "<tr>"\
			    "<td>");			
              fprintf(cgiOut,"<table width=766 height=380 align=center border=0 bgcolor=#ffffff cellpadding=0 cellspacing=0>"\
			    "<tr>"\
			      "<td width=766 align=left valign=top style=\"padding-left:10px; padding-top:10px\">"\
			      "<table width=700 border=0 cellspacing=0 cellpadding=0>"\
			      "<tr height=30>"
			       "<td style=\"border-bottom:2px solid #163871\"><font id=%s1>%s</font></td>",search(lpublic,"menu_summary"),search(lpublic,"summary"));
					fprintf(cgiOut,"</tr>");
					fprintf(cgiOut,"<tr style=\"padding-top:5px\">\n");
					fprintf(cgiOut,"<td>%s ID:&nbsp;&nbsp;",search(lpublic,"instance"));
					fprintf(cgiOut,"<select name=instance_id id=instance_id style=width:72px onchange=instanceid_change(this,\"wp_stasumary.cgi\",\"%s\")>",m);
					list_instance_parameter(&paraHead2, INSTANCE_STATE_WEB);    
					for(pq=paraHead2;(NULL != pq);pq=pq->next)
					{
					   memset(temp,0,sizeof(temp));
					   snprintf(temp,sizeof(temp)-1,"%d-%d-%d",pq->parameter.slot_id,pq->parameter.local_id,pq->parameter.instance_id);
					
					   if(strcmp(select_insid,temp) == 0)
						 fprintf(cgiOut,"<option value='%s' selected=selected>%s",temp,temp);
					   else
						 fprintf(cgiOut,"<option value='%s'>%s",temp,temp);
					}			
					free_instance_parameter_list(&paraHead2);
					fprintf(cgiOut,"</select>");
					fprintf(cgiOut,"</td>\n");
					fprintf(cgiOut,"</tr>\n");

			      fprintf(cgiOut,"<tr>"\
                    "<td style=\"padding-left:30px; padding-top:20px\"><table width=600 border=0 cellspacing=0 cellpadding=0>"\
                  	"<tr align=left style=\"padding-top:10px\">"\
                      "<td width=300><font id=%s2>%s</font></td>",search(lpublic,"menu_summary"),search(lwlan,"totle_num"));
                      fprintf(cgiOut,"<td width=150><font id=%s4>%s</font></td>",search(lpublic,"menu_summary"),search(lpublic,"total"));	
					  if((result == 1)&&(ac))
                  	    fprintf(cgiOut,"<td width=150>%d</td>",ac->num_sta);
					  else
					  	fprintf(cgiOut,"<td width=150>0</td>");
                    fprintf(cgiOut,"</tr>"\
                    "<tr align=left style=\"padding-top:10px\">"\
                      "<td width=300><font id=%s2>%s</font></td>",search(lpublic,"menu_summary"),"Wlan");
                      fprintf(cgiOut,"<td width=150><font id=%s4>%s</font></td>",search(lpublic,"menu_summary"),search(lpublic,"total"));	
					  if((result == 1)&&(ac))
						fprintf(cgiOut,"<td width=150>%d</td>",ac->num_wlan);
					  else
					  	fprintf(cgiOut,"<td width=150>0</td>");
                    fprintf(cgiOut,"</tr>"\
						 "<tr align=left style=\"padding-top:10px\">"\
                      "<td width=300><font id=%s2>%s</font></td>",search(lpublic,"menu_summary"),"Wtp");
                      fprintf(cgiOut,"<td width=150><font id=%s4>%s</font></td>",search(lpublic,"menu_summary"),search(lpublic,"total"));	
					  if((result == 1)&&(ac))
					    fprintf(cgiOut,"<td width=150>%d</td>",ac->num_wtp);
					  else
					  	fprintf(cgiOut,"<td width=150>0</td>");
                    fprintf(cgiOut,"</tr>");
					 if((result == 1)&&(ac))
					 {
						 fprintf(cgiOut,"<tr>"\
						 	"<td colspan=3 style=\"border-bottom:1px solid black; padding-top:15px\"><font id=%s2>%s</font></td>",search(lpublic,"menu_summary"),search(lwlan,"wlan_info"));
						 fprintf(cgiOut,"</tr>"\
						 "<tr>"\
							"<td colspan=3>"\
								"<div class=stanumlis>"\
								"<table width=600 border=0 cellspacing=0 cellpadding=0>");
						 for(i = 0,wlan = ac->wlan_list;
						 	 ((i<ac->num_wlan)&&(NULL != wlan));
							 i++,wlan = wlan->next)
						 {
							 
							 fprintf(cgiOut,"<tr align=left style=\"padding-top:10px; padding-left:10px\">"\
							 "<td width=300><font id=%s3>%s %d:</font></td>",search(lpublic,"menu_summary"),"Sta under Wlan",wlan->WlanID);
							 fprintf(cgiOut,"<td width=150><font id=%s4>%s</font></td>",search(lpublic,"menu_summary"),search(lpublic,"total")); 
							 fprintf(cgiOut,"<td width=150>%d</td>",wlan->num_sta);
							 fprintf(cgiOut,"</tr>");
						 
						 }
						 fprintf(cgiOut,"</table>"\
								"</div>"\
							"</td>"\
						 "</tr>");
					 }						
					 if((result == 1)&&(ac))
					 {
						 fprintf(cgiOut,"<tr>"\
							"<td colspan=3 style=\"border-bottom:1px solid black; padding-top:15px\"><font id=%s2>%s</font></td>",search(lpublic,"menu_summary"),search(lwlan,"wtp_info"));
						 fprintf(cgiOut,"</tr>"\
						 "<tr>"\
							"<td colspan=3>"\
								"<div class=stanumlis>"\
								"<table width=600 border=0 cellspacing=0 cellpadding=0>");
						 for(i = 0, wtp = ac->wtp_list;
						 	 ((i<ac->num_wtp)&&(NULL != wtp));
							 i++,wtp = wtp->next)
						 {
							 fprintf(cgiOut,"<tr align=left style=\"padding-top:10px; padding-left:10px\">"\
							   "<td width=300><font id=%s3>%s %d:</font></td>",search(lpublic,"menu_summary"),"Sta under Wtp",wtp->WtpID);
							   fprintf(cgiOut,"<td width=150><font id=%s4>%s</font></td>",search(lpublic,"menu_summary"),search(lpublic,"total"));	 
							fprintf(cgiOut,"<td width=150>%d</td>",wtp->num_sta);
							 fprintf(cgiOut,"</tr>");
					 
						 }
						 fprintf(cgiOut,"</table>"\
								"</div>"\
							"</td>"\
						 "</tr>");
					 }	
            fprintf(cgiOut,"</table></td>"\
                   "</tr>"\
                  "</table>"\
			      "</td>"\
				"</tr>"\
		  "</table>");
	fprintf(cgiOut,"</td>"\
  "</tr>"\
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
	Free_sta_summary(ac);
}
free_instance_parameter_list(&paraHead1);
}


