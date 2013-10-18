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
* wp_rogapkind.c
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
#include "ws_dcli_ac.h"
#include "ws_usrinfo.h"
#include "ws_err.h"
#include "ws_ec.h"
#include "ws_dbus_list_interface.h"

#define MAX_PAGE_NUM 25     /*每页显示的最多非法AP个数*/  

void ShowRogueApKindPage(char *m,char *n,int t,char *ins_id,struct list *lpublic,struct list *lwlan,struct list *lwcontrol);  

int cgiMain()
{
  char encry[BUF_LEN] = { 0 };       
  char page_no[10] = { 0 }; 
  char instance_id[10] = { 0 };
  char *str = NULL; 
  char *endptr = NULL; 
  int pno = 0;
  struct list *lpublic = NULL;   /*解析public.txt文件的链表头*/
  struct list *lwlan = NULL;     /*解析wlan.txt文件的链表头*/  
  struct list *lwcontrol = NULL;     /*解析wcontrol.txt文件的链表头*/  
  lpublic=get_chain_head("../htdocs/text/public.txt");
  lwlan=get_chain_head("../htdocs/text/wlan.txt");
  lwcontrol=get_chain_head("../htdocs/text/wcontrol.txt");
  
  DcliWInit();
  ccgi_dbus_init();
  memset(encry,0,sizeof(encry));
  memset(page_no,0,sizeof(page_no));
  memset(instance_id,0,sizeof(instance_id));
  cgiFormStringNoNewlines("INSTANCE_ID", instance_id, 10);
  cgiFormStringNoNewlines("UN", encry, BUF_LEN); 
  str=dcryption(encry);
  if(str==NULL)
    ShowErrorPage(search(lpublic,"ill_user"));		 /*用户非法*/
  else
  {
    if(cgiFormStringNoNewlines("SPN", page_no, 10)!=cgiFormNotFound )  /*点击翻页进入该页面*/
    {
      pno= strtoul(page_no,&endptr,10);	/*char转成int，10代表十进制*/ 
      ShowRogueApKindPage(encry,str,pno,instance_id,lpublic,lwlan,lwcontrol);
	}
	else
      ShowRogueApKindPage(encry,str,0,instance_id,lpublic,lwlan,lwcontrol);
  }  
  release(lpublic);  
  release(lwlan);
  release(lwcontrol);
  destroy_ccgi_dbus();
  return 0;
}

void ShowRogueApKindPage(char *m,char *n,int t,char *ins_id,struct list *lpublic,struct list *lwlan,struct list *lwcontrol)
{  
  char pno[10] = { 0 }; 
  char ID[10] = { 0 };
  DCLI_AC_API_GROUP_TWO *LIST = NULL;  
  int len = 0;
  int j = 0;
  struct Neighbor_AP_ELE *head = NULL;
  char *endptr = NULL;  
  int rogAp_num = 0,i = 0,wid = 0,cl = 1,result = 0,retu = 1;                 /*颜色初值为#f9fafe*/  
  int limit = 0,start_illapno = 0,end_illapno = 0,illapno_page = 0,total_pnum = 0;       /*start_illapno表示要显示的起始非法AP，end_illapno表示要显示的结束非法AP id，illapno_page表示本页要显示的非法AP数，total_pnum表示总页数*/
  char alt[100] = { 0 };
  char max_wtp_num[10] = { 0 };
  instance_parameter *paraHead1 = NULL;
  dbus_parameter ins_para;

  cgiHeaderContentType("text/html");
  fprintf(cgiOut,"<html xmlns=\"http://www.w3.org/1999/xhtml\"><head>");
  fprintf(cgiOut,"<meta http-equiv=Content-Type content=text/html; charset=gb2312>");
  fprintf(cgiOut,"<title>Rogue AP list</title>");
  fprintf(cgiOut,"<link rel=stylesheet href=/style.css type=text/css>"\
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
  "<body>"\
  "<form>"\
  "<div align=center>"\
  "<table width=976 border=0 cellpadding=0 cellspacing=0>"\
  "<tr>"\
    "<td width=8 align=left valign=top background=/images/di22.jpg><img src=/images/youce4.jpg width=8 height=30/></td>"\
    "<td width=51 align=left valign=bottom background=/images/di22.jpg><img src=/images/youce33.jpg width=37 height=24/></td>"\
    "<td width=153 align=left valign=bottom id=%s background=/images/di22.jpg>%s</td>",search(lpublic,"title_style"),search(lwlan,"ap"));
    fprintf(cgiOut,"<td width=690 align=right valign=bottom background=/images/di22.jpg>");	
		memset(pno,0,sizeof(pno));
		cgiFormStringNoNewlines("PN",pno,10);
    	  fprintf(cgiOut,"<table width=155 border=0 cellspacing=0 cellpadding=0>"\
          "<tr>"\
          "<td width=62 align=center><a href=wp_wtplis.cgi?UN=%s&PN=%s&INSTANCE_ID=%s target=mainFrame><img src=/images/%s border=0 width=62 height=20/></a></td>",m,pno,ins_id,search(lpublic,"img_ok"));
		  fprintf(cgiOut,"<td width=62 align=center><a href=wp_wtplis.cgi?UN=%s&PN=%s&INSTANCE_ID=%s target=mainFrame><img src=/images/%s border=0 width=62 height=20/></a></td>",m,pno,ins_id,search(lpublic,"img_cancel"));
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
                    "<td align=left id=tdleft background=/images/bottom_bg.gif style=\"border-right:0\"><font id=%s>%s</font><font id=yingwen_san>AP</font></td>",search(lpublic,"menu_san"),search(lwlan,"rogue"));   /*突出显示*/
                  fprintf(cgiOut,"</tr>"\
				  "<tr height=25>"\
  					"<td align=left id=tdleft><a href=wp_wtpsearch.cgi?UN=%s target=mainFrame class=top><font id=%s>%s</font><font id=yingwen_san> AP</font></a></td>",m,search(lpublic,"menu_san"),search(lpublic,"Search"));                       
                  fprintf(cgiOut,"</tr>");
				  retu=checkuser_group(n);
				  if(retu==0)  /*管理员*/
				  {
                    fprintf(cgiOut,"<tr height=25>"\
  					  "<td align=left id=tdleft><a href=wp_wtpnew.cgi?UN=%s target=mainFrame class=top><font id=%s>%s</font><font id=yingwen_san> AP</font></a></td>",m,search(lpublic,"menu_san"),search(lpublic,"create"));                       
                    fprintf(cgiOut,"</tr>");
				  }
			fprintf(cgiOut,"<tr height=25>"\
					"<td align=left id=tdleft><a href=wp_wtpgrouplist.cgi?UN=%s target=mainFrame class=top><font id=%s>%s</font></a></td>",m,search(lpublic,"menu_san"),search(lwlan,"ap_group_list"));			  
		  fprintf(cgiOut,"</tr>");
		  if(retu==0)  /*管理员*/
		  {
		fprintf(cgiOut,"<tr height=25>"\
					"<td align=left id=tdleft><a href=wp_wtpgroupnew.cgi?UN=%s target=mainFrame class=top><font id=%s>%s</font></a></td>",m,search(lpublic,"menu_san"),search(lwlan,"create_apgroup"));			  
		  fprintf(cgiOut,"</tr>");
		  	}
				  fprintf(cgiOut,"<tr height=25>"\
  					"<td align=left id=tdleft><a href=wp_wtpver.cgi?UN=%s target=mainFrame class=top><font id=yingwen_san>AP</font><font id=%s> %s</font></a></td>",m,search(lpublic,"menu_san"),search(lpublic,"mode"));                       
                  fprintf(cgiOut,"</tr>");
				  if(retu==0)  /*管理员*/
				  {
					fprintf(cgiOut,"<tr height=25>"\
  					  "<td align=left id=tdleft><a href=wp_wtpdown.cgi?UN=%s target=mainFrame class=top><font id=%s>%s</font></a></td>",m,search(lpublic,"menu_san"),search(lpublic,"version_upload"));                       
                    fprintf(cgiOut,"</tr>");
				  }
				  fprintf(cgiOut,"<tr height=25>"\
				  	"<td align=left id=tdleft><a href=wp_verbind.cgi?UN=%s target=mainFrame class=top><font id=%s>%s</font></a></td>",m,search(lpublic,"menu_san"),search(lwlan,"version_bind"));                       
                  fprintf(cgiOut,"</tr>"\
				  "<tr height=25>"\
  					"<td align=left id=tdleft><a href=wp_wtpupgrade.cgi?UN=%s target=mainFrame class=top><font id=%s>%s</font></a></td>",m,search(lpublic,"menu_san"),search(lwlan,"batch_update"));                       
                  fprintf(cgiOut,"</tr>");
				  if(retu==0)  /*管理员*/
				  {
					fprintf(cgiOut,"<tr height=25>"\
  					  "<td align=left id=tdleft><a href=wp_wtpbw.cgi?UN=%s target=mainFrame class=top><font id=yingwen_san>MAC </font><font id=%s>%s</font></a></td>",m,search(lpublic,"menu_san"),search(lwlan,"mac_filter"));                       
                    fprintf(cgiOut,"</tr>");
				  }
				  fprintf(cgiOut,"<tr height=25>"\
  					"<td align=left id=tdleft><a href=wp_showAutoAp.cgi?UN=%s target=mainFrame class=top><font id=%s>%s</font><font id=yingwen_san>AP</font><font id=%s>%s</font></a></td>",m,search(lpublic,"menu_san"),search(lpublic,"show_auto"),search(lpublic,"menu_san"),search(lpublic,"policy"));                       
                  fprintf(cgiOut,"</tr>");
				  if(retu==0) /*管理员*/
				  {
                    fprintf(cgiOut,"<tr height=25>"\
  					"<td align=left id=tdleft><a href=wp_conAutoAp.cgi?UN=%s target=mainFrame class=top><font id=%s>%s</font><font id=yingwen_san>AP</font><font id=%s>%s</font></a></td>",m,search(lpublic,"menu_san"),search(lpublic,"config_auto"),search(lpublic,"menu_san"),search(lpublic,"policy"));                       
                    fprintf(cgiOut,"</tr>");
				  }
				  cgiFormStringNoNewlines("ID", ID, 10);
				  wid= strtoul(ID,&endptr,10);   /*char转成int，10代表十进制*/
				  fprintf(cgiOut,"<script type=\"text/javascript\">"\
				  "function page_change(obj)"\
				  "{"\
				     "var page_num = obj.options[obj.selectedIndex].value;"\
				   	 "var url = 'wp_rogapkind.cgi?UN=%s&SPN='+page_num+'&ID=%s&PN=%s&INSTANCE_ID=%s';"\
				   	 "window.location.href = url;"\
				   	"}", m , ID , pno , ins_id);
				  fprintf(cgiOut,"</script>");
				  
				  get_slotID_localID_instanceID(ins_id,&ins_para);	
				  get_instance_dbus_connection(ins_para, &paraHead1, INSTANCE_STATE_WEB);

				  if(paraHead1)
				  {
					  result=show_rogue_aplist_bywtpid(paraHead1->parameter,paraHead1->connection,wid,&LIST);  /*返回0表示失败，返回1表示成功，返回2表示there is no rouge ap*/
																											   /*返回-1表示input wtp id should be 1 to 1024，返回-2表示wtp does not exist*/
																											   /*返回-3表示radio resource managment is disable please enable first*/
				  }
				  if(result == 1)
				  {
				  	if((LIST)&&(LIST->rouge_ap_list))
					{
						len = LIST->rouge_ap_list->neighborapInfosCount;
						rogAp_num = len;
					}
				  }

				  total_pnum=((rogAp_num%MAX_PAGE_NUM)==0)?(rogAp_num/MAX_PAGE_NUM):((rogAp_num/MAX_PAGE_NUM)+1);
				  start_illapno=t*MAX_PAGE_NUM;   
				  end_illapno=(((t+1)*MAX_PAGE_NUM)>rogAp_num)?rogAp_num:((t+1)*MAX_PAGE_NUM);
				  illapno_page=end_illapno-start_illapno;
				  if((illapno_page<MAX_PAGE_NUM/2)||(rogAp_num==MAX_PAGE_NUM/2))   /*该页显示1--9个或者一共有10个非法AP*/
				  	limit=2;
				  else if((illapno_page<MAX_PAGE_NUM)||(rogAp_num==MAX_PAGE_NUM))  /*该页显示10--19个或者一共有20个非法AP*/
			  	    limit=12;
			      else         /*大于20个翻页*/
				   	limit=14;
				  if(retu==1)  /*普通用户*/
				  	limit+=4;
				  for(i=0;i<limit;i++)
	              {
  				    fprintf(cgiOut,"<tr height=25>"\
                      "<td id=tdleft>&nbsp;</td>"\
                    "</tr>");
	              }				  
                fprintf(cgiOut,"</table>"\
              "</td>"\
              "<td align=left valign=top style=\"background-color:#ffffff; border-right:1px solid #707070; padding-left:30px; padding-top:10px\">"\
              "<table width=790 border=0 bgcolor=#ffffff cellspacing=0 cellpadding=0>"\
   "<tr>"\
     "<td id=ins_style>%s:%s</td>",search(lpublic,"instance"),ins_id);
   fprintf(cgiOut,"</tr>"\
   "<tr>"\
    "<td valign=top align=center style=\"padding-bottom:10px\">");     
	if(result == 1)    /*显示所有非法AP的信息，head返回非法AP信息链表的链表头*/
	{   
	  fprintf(cgiOut,"<table width=790 border=0 cellspacing=0 cellpadding=0>"\
      "<tr>"\
      "<td align=left colspan=3>");
	  if(rogAp_num>0)           /*如果非法AP存在*/
	  {		   
   	    fprintf(cgiOut,"<table frame=below rules=rows width=790 border=1>");
		fprintf(cgiOut,"<tr align=left>"\
		"<th width=130><font id=yingwen_thead>MAC</font></th>"\
        "<th width=65><font id=%s>%s</font></th>",search(lpublic,"menu_thead"),search(lwlan,"rate"));
		fprintf(cgiOut,"<th width=80><font id=%s>%s</font></th>",search(lpublic,"menu_thead"),search(lwlan,"channel"));
        fprintf(cgiOut,"<th width=130><font id=%s>%s</font></th>",search(lpublic,"menu_thead"),search(lwcontrol,"rssi"));
		fprintf(cgiOut,"<th width=90><font id=%s>%s</font></th>",search(lpublic,"menu_thead"),search(lwcontrol,"noise"));
		fprintf(cgiOut,"<th width=100><font id=%s>%s</font></th>",search(lpublic,"menu_thead"),search(lwcontrol,"bea_int"));
		fprintf(cgiOut,"<th width=100><font id=%s>%s</font></th>",search(lpublic,"menu_thead"),search(lwcontrol,"capab"));
		fprintf(cgiOut,"<th width=110><font id=yingwen_thead>ESSID</font></th>"\
		"<th width=85><font id=%s>%s</font></th>",search(lpublic,"menu_thead"),search(lwcontrol,"ie_info"));
        fprintf(cgiOut,"</tr>");
		if((LIST)&&(LIST->rouge_ap_list)&&(LIST->rouge_ap_list->neighborapInfos != NULL))
		{
			head = LIST->rouge_ap_list->neighborapInfos;
			for(i=0;i<start_illapno;i++)
			{
				if(head)
				{
					head = head->next;
				}
			}
		}		
		for(i=start_illapno;i<end_illapno;i++)
		{ 
			if(head)
			{
				fprintf(cgiOut,"<tr align=left bgcolor=%s>",setclour(cl));
				fprintf(cgiOut,"<td>%02X:%02X:%02X:%02X:%02X:%02X</td>",head->BSSID[0],head->BSSID[1],head->BSSID[2],head->BSSID[3],head->BSSID[4],head->BSSID[5]);
				fprintf(cgiOut,"<td>%d</td>",head->Rate);
				fprintf(cgiOut,"<td>%d</td>",head->Channel);
				fprintf(cgiOut,"<td>%d</td>",head->RSSI);
				fprintf(cgiOut,"<td>%d</td>",head->NOISE);
				fprintf(cgiOut,"<td>%d</td>",head->BEACON_INT);
				fprintf(cgiOut,"<td>%d</td>",head->capabilityinfo);
				if(head->ESSID)
				{
					fprintf(cgiOut,"<td>%s</td>",head->ESSID);
				}
				if(head->IEs_INFO)
				{
					fprintf(cgiOut,"<td>%s</td>",head->IEs_INFO);
				}
				fprintf(cgiOut,"</tr>");
				cl=!cl;
				head = head->next;	
			}
		}		  
		fprintf(cgiOut,"</table>");
	  }
	  else				 /*no rogue ap exist*/
		fprintf(cgiOut,"%s",search(lwcontrol,"no_rogue_ap"));
	  fprintf(cgiOut,"</td></tr>");
	  if(rogAp_num>MAX_PAGE_NUM)               /*大于30个radio时，显示翻页的链接*/
	  {
	    fprintf(cgiOut,"<tr style=\"padding-top:20px\">");
		if(t!=0)
		  fprintf(cgiOut,"<td align=left width=100><a href=wp_rogapkind.cgi?UN=%s&SPN=%d&ID=%s&PN=%s&INSTANCE_ID=%s target=mainFrame>%s</a></td>",m,t-1,ID,pno,ins_id,search(lpublic,"up_page"));
		else
		  fprintf(cgiOut,"<td width=100>&nbsp;</td>");
		fprintf(cgiOut,"<td align=center width=590>%s",search(lpublic,"jump_to_page1"));
										 fprintf(cgiOut,"<select name=page_num id=page_num style=width:50px onchange=page_change(this)>");
										 for(i=0;i<total_pnum;i++)
										 {
										   if(i==t)
											 fprintf(cgiOut,"<option value=%d selected=selected>%d",i,i+1);
										   else
											 fprintf(cgiOut,"<option value=%d>%d",i,i+1);
										 }
										 fprintf(cgiOut,"</select>"\
										 "%s</td>",search(lpublic,"jump_to_page2"));
		if(t!=((rogAp_num-1)/MAX_PAGE_NUM))
		  fprintf(cgiOut,"<td align=right width=100><a href=wp_rogapkind.cgi?UN=%s&SPN=%d&ID=%s&PN=%s&INSTANCE_ID=%s target=mainFrame>%s</a></td>",m,t+1,ID,pno,ins_id,search(lpublic,"down_page"));
		else
		  fprintf(cgiOut,"<td width=100>&nbsp;</td>");
	    fprintf(cgiOut,"</tr>");
	  }  
      fprintf(cgiOut,"</table>");
	}
	else if(result == 2)
	  fprintf(cgiOut,"%s",search(lwcontrol,"no_rogue_ap"));	
	else if(result == -1)
	{
		memset(alt,0,sizeof(alt));
	    strncpy(alt,search(lwlan,"wtp_id_illegal1"),sizeof(alt)-1);
		memset(max_wtp_num,0,sizeof(max_wtp_num));
		snprintf(max_wtp_num,sizeof(max_wtp_num)-1,"%d",WTP_NUM-1);
		strncat(alt,max_wtp_num,sizeof(alt)-strlen(alt)-1);
		strncat(alt,search(lwlan,"wtp_id_illegal2"),sizeof(alt)-strlen(alt)-1);
		fprintf(cgiOut,"%s",alt);  
	}
	else if(result == -2)
	  fprintf(cgiOut,"%s",search(lwlan,"wtp_not_exist"));
	else if(result == -3)
	  fprintf(cgiOut,"%s",search(lwcontrol,"enable_radio_resource"));
	else
      fprintf(cgiOut,"%s",search(lpublic,"contact_adm"));  
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
  Free_rogue_aplist_bywtpid(LIST);
}
free_instance_parameter_list(&paraHead1);
}



