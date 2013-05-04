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
* wp_deletevlan.c
*
*
* CREATOR:
* autelan.software.Network Dep. team
* zhouym@autelan.com
*
* DESCRIPTION:
* 
*
*
*******************************************************************************/
#include <stdio.h>
#include "cgic.h"
#include <stdio.h>
#include "cgic.h"
#include <string.h>
#include <stdlib.h>
#include "ws_usrinfo.h"
#include "ws_init_dbus.h"

#include "ws_dcli_vlan.h"
#define PageNum  10


int DeleteVlanPage();
void  delete_vlan_byID(struct list *lcontrol);

int cgiMain()
{
	DeleteVlanPage();
	return 0;
}


int DeleteVlanPage()
{
	struct list *lpublic;	/*解析public.txt文件的链表头*/
	struct list *lcontrol; 	/*解析help.txt文件的链表头*/
	lpublic=get_chain_head("../htdocs/text/public.txt");
    lcontrol=get_chain_head("../htdocs/text/control.txt"); 
	char *encry=(char *)malloc(BUF_LEN);			   /*存储从wp_usrmag.cgi带入的加密字符串*/
    char *PNtemp=(char *)malloc(10);
    char *SNtemp=(char *)malloc(10);
	 char *str;
	 FILE *fp1;
	 char lan[3];
	 char delvlan_encry[BUF_LEN]; 
	 //struct user_infor *p;
	 struct vlan_info_simple  receive_vlan[MAX_VLAN_NUM];
	 int i,port_num[MAX_VLAN_NUM],vlanNum=0;
	 int cl=1;				   /*cl标识表格的底色，1为#f9fafe，0为#ffffff*/
	int pageNum=0;
	char * pageNumCA=(char *)malloc(10);
	memset(pageNumCA,0,10);
	char * pageNumCD=(char *)malloc(10);
	memset(pageNumCD,0,10);
	 for(i=0;i<4095;i++)
	   {
		   receive_vlan[i].vlanName=(char *)malloc(21);
		   memset(receive_vlan[i].vlanName,0,21);
		   port_num[i]=0;
	   }
	 
	 ccgi_dbus_init();
	  if(cgiFormSubmitClicked("submit_delvlan") != cgiFormSuccess)
	 {
	   memset(encry,0,BUF_LEN);
	   cgiFormStringNoNewlines("UN", encry, BUF_LEN); 
	   str=dcryption(encry);
	   if(str==NULL)
	   {
		 ShowErrorPage(search(lpublic,"ill_user"));	/*用户非法*/
		 return 0;
	   }
	   memset(delvlan_encry,0,BUF_LEN); 				  /*清空临时变量*/
	 }
  memset(PNtemp,0,10);
  cgiFormStringNoNewlines("PN",PNtemp,10);
  pageNum=atoi(PNtemp);
  memset(SNtemp,0,10);
  cgiFormStringNoNewlines("SN",SNtemp,10);
  cgiFormStringNoNewlines("encry_del",delvlan_encry,BUF_LEN);
  cgiHeaderContentType("text/html");
  fprintf(cgiOut,"<html xmlns=\"http://www.w3.org/1999/xhtml\"><head>");
  fprintf(cgiOut,"<meta http-equiv=Content-Type content=text/html; charset=gb2312>");
  fprintf(cgiOut,"<title>%s</title>",search(lcontrol,"vlan_manage"));
  fprintf(cgiOut,"<link rel=stylesheet href=/style.css type=text/css>"\
  	"<style type=text/css>"\
		  "#div1{ width:82px; height:18px; border:1px solid #666666; background-color:#f9f8f7;}"\
		  "#div2{ width:80px; height:15px; padding-left:5px; padding-top:3px}"\
		  "#link{ text-decoration:none; font-size: 12px}"\
		  ".deletevlan {overflow-x:hidden;	overflow:auto; width: 546px; height: 196px; clip: rect( ); padding-top: 0px; padding-right: 0px; padding-bottom: 0px; padding-left: 0px} "\
	"</style>"\

  "</head>"\
  "<body>");
  if(cgiFormSubmitClicked("submit_delvlan") == cgiFormSuccess)
	{
		delete_vlan_byID(lcontrol);
	}

  fprintf(cgiOut,"<form method=post>"\
  "<div align=center>"\
  "<table width=976 border=0 cellpadding=0 cellspacing=0>"\
  "<tr>"\
    "<td width=8 align=left valign=top background=/images/di22.jpg><img src=/images/youce4.jpg width=8 height=30/></td>"\
    "<td width=51 align=left valign=bottom background=/images/di22.jpg><img src=/images/youce33.jpg width=37 height=24/></td>"\
    "<td width=153 align=left valign=bottom background=/images/di22.jpg><font id=titleen>VLAN</font><font id=%s> %s</font></td>",search(lpublic,"title_style"),search(lpublic,"management"));
    fprintf(cgiOut,"<td width=690 align=right valign=bottom background=/images/di22.jpg>");
	    if((fp1=fopen("../htdocs/text/public.txt","r"))==NULL)		 /*以只读方式打开资源文件*/
		{
			ShowAlert(search(lpublic,"error_open"));
	    }
	    else
	    {
			fseek(fp1,4,0); 						/*将文件指针移到离文件首4个字节处，即lan=之后*/
			fgets(lan,3,fp1);	   
			fclose(fp1);
	    }
	    if(strcmp(lan,"ch")==0)
    	{	
    	  fprintf(cgiOut,"<table width=130 border=0 cellspacing=0 cellpadding=0>"\
          "<tr>"\
          "<td width=62 align=center><input id=but type=submit name=submit_delvlan style=background-image:url(/images/%s) value=""></td>",search(lpublic,"img_ok"));		  
		  if(cgiFormSubmitClicked("submit_delvlan") != cgiFormSuccess)
            fprintf(cgiOut,"<td width=62 align=left><a href=wp_configvlan.cgi?UN=%s target=mainFrame><img src=/images/%s border=0 width=62 height=20/></a></td>",encry,search(lpublic,"img_cancel"));
		  else                                         
     		fprintf(cgiOut,"<td width=62 align=left><a href=wp_configvlan.cgi?UN=%s target=mainFrame><img src=/images/%s border=0 width=62 height=20/></a></td>",delvlan_encry,search(lpublic,"img_cancel"));
		  fprintf(cgiOut,"</tr>"\
          "</table>");
		}		
		else			
		{	
		  fprintf(cgiOut,"<table width=130 border=0 cellspacing=0 cellpadding=0>"\
		  "<tr>"\
		  "<td width=62 align=center><input id=but type=submit name=submit_delvlan style=background-image:url(/images/ok-en.jpg) value=""></td>");		  
		  if(cgiFormSubmitClicked("submit_delvlan") != cgiFormSuccess)
		    fprintf(cgiOut,"<td width=62 align=left><a href=wp_configvlan.cgi?UN=%s target=mainFrame><img src=/images/cancel-en.jpg border=0 width=62 height=20/></a></td>",encry);
		  else
		    fprintf(cgiOut,"<td width=62 align=left><a href=wp_configvlan.cgi?UN=%s target=mainFrame><img src=/images/cancel-en.jpg border=0 width=62 height=20/></a></td>",delvlan_encry);
		  fprintf(cgiOut,"</tr>"\
		  "</table>");
		}
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
             		if(cgiFormSubmitClicked("submit_delvlan") != cgiFormSuccess)
             		{
             		  fprintf(cgiOut,"<tr height=25>"\
             			"<td align=left id=tdleft><a href=wp_configvlan.cgi?UN=%s target=mainFrame class=top><font id=%s>%s<font></a></td>",encry,search(lpublic,"menu_san"),search(lcontrol,"vlan_config"));					   
             		  fprintf(cgiOut,"</tr>"\
             		  "<tr height=25>"\
             			"<td align=left id=tdleft><a href=wp_addvlan.cgi?UN=%s target=mainFrame class=top><font id=%s>%s<font></a></td>",encry,search(lpublic,"menu_san"),search(lcontrol,"add_vlan"));
             		  fprintf(cgiOut,"</tr>"\
             		  "<tr height=26>"\
             			"<td align=left id=tdleft background=/images/bottom_bg.gif style=\"border-right:0\"><font id=%s>%s<font></td>",search(lpublic,"menu_san"),search(lcontrol,"delete_vlan"));	/*突出显示*/
             		  fprintf(cgiOut,"</tr>");
             		}
             		else if(cgiFormSubmitClicked("submit_delvlan") == cgiFormSuccess)				
             		{
             		  fprintf(cgiOut,"<tr height=25>"\
             			"<td align=left id=tdleft><a href=wp_configvlan.cgi?UN=%s target=mainFrame style=color:#000000><font id=%s>%s<font></a></td>",delvlan_encry,search(lpublic,"menu_san"),search(lcontrol,"vlan_config"));						 
             		  fprintf(cgiOut,"</tr>"\
             		  "<tr height=25>"\
             			"<td align=left id=tdleft><a href=wp_addvlan.cgi?UN=%s target=mainFrame style=color:#000000><font id=%s>%s<font></a></td>",delvlan_encry,search(lpublic,"menu_san"),search(lcontrol,"add_vlan"));
             		  fprintf(cgiOut,"</tr>"\
             		  "<tr height=26>"\
             			"<td align=left id=tdleft background=/images/bottom_bg.gif style=\"border-right:0\"><font id=%s>%s<font></td>",search(lpublic,"menu_san"),search(lcontrol,"delete_vlan"));	/*突出显示*/
             		  fprintf(cgiOut,"</tr>");
             		}
					for(i=0;i<11;i++)
					  {
						fprintf(cgiOut,"<tr height=25>"\
						  "<td id=tdleft>&nbsp;</td>"\
						"</tr>");
					  }

				  fprintf(cgiOut,"</table>"\
              "</td>"\
              "<td align=left valign=top style=\"background-color:#ffffff; border-right:1px solid #707070; padding-left:30px; padding-top:10px\">"\
					  "<table width=640 height=340 border=0 cellspacing=0 cellpadding=0>"\
													 "<tr>"\
													  "<td id=sec1 style=\"border-bottom:2px solid #53868b;font-size:14px;padding-left:0px;padding-top:0px\">%s</td>",search(lcontrol,"Vlan_info"));
													  fprintf(cgiOut,"</tr>"\
													"<tr>"\
													  "<td align=left valign=top  style=\"padding-top:18px\">"\
													  "<div class=configvlan><table width=438 border=1 frame=below rules=rows bordercolor=#cccccc cellspacing=0 cellpadding=0>"\
														   "<tr height=25 bgcolor=#eaeff9 style=font-size:14px>"\
															"<th width=30 >&nbsp;</th>");
                                							fprintf(cgiOut,"<th width=124 style=font-size:14px align=left>%s</th>","VLAN ID");
                                							fprintf(cgiOut,"<th width=194 style=font-size:14px align=left>%s</th>",search(lcontrol,"vlan_name"));
                                							fprintf(cgiOut,"<th width=90  style=font-size:14px align=left>%s</th>",search(lcontrol,"port_num"));
                                							fprintf(cgiOut,"</tr>");
															int k=show_vlan_list(receive_vlan,port_num,&vlanNum);
															int xnt=0,head=0,tail=0;					  
															  
															//fprintf(cgiOut,"pageNum=%d--xnt=%d",pageNum,xnt);
															if(k==CMD_SUCCESS)
															  {
															  if(0==strcmp(SNtemp,"PageDown") || 0==strcmp(SNtemp,""))
																  {
																	  if(vlanNum-pageNum*PageNum<0)
																	  {
																		  pageNum=pageNum-1;
																		  ShowAlert(search(lcontrol,"Page_end")); 
																	  }
																	  if(vlanNum-pageNum*PageNum<PageNum)
																			  xnt=vlanNum;
																	  else	  xnt=(pageNum+1)*PageNum;
					  
																	  head=pageNum*PageNum;
																	  tail=xnt;
																  }
															  else if(0==strcmp(SNtemp,"PageUp"))
																  {
																	  if(pageNum<0)
																	  {
																		  pageNum=pageNum+1;
																		  ShowAlert(search(lcontrol,"Page_Begin"));
																	  }
																	  if(vlanNum-pageNum*PageNum<PageNum)
																			  xnt=vlanNum;
																	  else	  xnt=(pageNum+1)*PageNum;
																	  head=pageNum*PageNum;
																	  tail=xnt;
																  }
															   for(i=head;i<tail;i++)
																  {
																   fprintf(cgiOut,"<tr height=25 bgcolor=%s>",setclour(cl));
																   if(receive_vlan[i].vlanId!=4095)
																   {
    																   if(receive_vlan[i].vlanId==1)
    																  	{
    																   		fprintf(cgiOut,"<td>&nbsp;</td>");
    																  	}
    																   else
    																   	{
    																   		fprintf(cgiOut,"<td><input type=radio name=radiobutton value=\"%d\"></td>",receive_vlan[i].vlanId);
    																   	}
    																   fprintf(cgiOut,"<td style=font-size:12px align=left>%d</td>",receive_vlan[i].vlanId);
    																   fprintf(cgiOut,"<td style=font-size:12px align=left>%s</td>",receive_vlan[i].vlanName);
    																   fprintf(cgiOut,"<td style=font-size:12px align=left>%d</td>",port_num[i]);
    																   fprintf(cgiOut,"</tr>");
    																   cl=!cl;
																   }
																 }
														  }
													  fprintf(cgiOut,"</table></div></td>"\
													  "</tr>"\
													  "<tr>"\
													  "<td>"\
													  "<table width=430 style=padding-top:2px>"\
													  "<tr>");
													  sprintf(pageNumCA,"%d",pageNum+1);
													  sprintf(pageNumCD,"%d",pageNum-1);
													  if(cgiFormSubmitClicked("submit_delvlan") != cgiFormSuccess)
														  {
															  fprintf(cgiOut,"<td align=center style=padding-top:2px><a href=wp_deletevlan.cgi?UN=%s&PN=%s&SN=%s>%s</td>",encry,pageNumCA,"PageDown",search(lcontrol,"page_down"));
															  fprintf(cgiOut,"<td align=center style=padding-top:2px><a href=wp_deletevlan.cgi?UN=%s&PN=%s&SN=%s>%s</td>",encry,pageNumCD,"PageUp",search(lcontrol,"page_up"));
														  }
													  else if(cgiFormSubmitClicked("submit_delvlan") == cgiFormSuccess)
														  {
															  fprintf(cgiOut,"<td align=center style=padding-top:2px><a href=wp_deletevlan.cgi?UN=%s&PN=%s&SN=%s>%s</td>",delvlan_encry,pageNumCA,"PageDown",search(lcontrol,"page_down"));
															  fprintf(cgiOut,"<td align=center style=padding-top:2px><a href=wp_deletevlan.cgi?UN=%s&PN=%s&SN=%s>%s</td>",delvlan_encry,pageNumCD,"PageUp",search(lcontrol,"page_up"));  
														  }
													  fprintf(cgiOut,"</tr></table></td>"\
														  "</tr>"\
												  "<tr>");
												  if(cgiFormSubmitClicked("submit_delvlan") != cgiFormSuccess)
												  {
													fprintf(cgiOut,"<td><input type=hidden name=encry_del value=%s></td>",encry);
												  }
												  else if(cgiFormSubmitClicked("submit_delvlan") == cgiFormSuccess)
													  { 			 
														fprintf(cgiOut,"<td><input type=hidden name=encry_del value=%s></td>",delvlan_encry);
													  }
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
for(i=0;i<4095;i++)
{
   free(receive_vlan[i].vlanName);
}
free(PNtemp);
free(SNtemp);
free(pageNumCA);
free(pageNumCD);
free(encry);
release(lpublic);  
release(lcontrol);															 
return 0;
}

void  delete_vlan_byID(struct list *lcontrol)
 {
	unsigned short vID;
	char * IDText=(char *)malloc(10);	
 	if(cgiFormString("radiobutton", IDText, 10)==cgiFormNotFound)
 		{
 			ShowAlert(search(lcontrol,"NO_CHOICE"));
 		}
	else
		{
			vID=atoi(IDText);
			//unsigned int temp=vID;
			fprintf(stderr,"VLANID=%s",IDText);
			//deleteIntfForVlanNoShow(temp);
			//delete_vlan(vID);			
		}
	free(IDText);
//	cgiStringArrayFree(valuesTextTemp);
 }


