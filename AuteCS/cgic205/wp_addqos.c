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
* wp_addqos.c
*
*
* CREATOR:
* autelan.software.Network Dep. team
* zhouym@autelan.com
*
* DESCRIPTION:
* system contrl for qos config
*
*
*******************************************************************************/
#include <stdio.h>
#include "cgic.h"
#include <string.h>
#include <stdlib.h>
#include "ws_usrinfo.h"
#include "ws_init_dbus.h"

#include "ws_dcli_qos.h"
#define AMOUNT 512

int ShowAddQosPage(); 

int cgiMain()
{
 ShowAddQosPage();
 return 0;
}

int ShowAddQosPage()
{
	struct list *lpublic;	/*解析public.txt文件的链表头*/
	struct list *lcontrol; 	/*解析help.txt文件的链表头*/
	lpublic=get_chain_head("../htdocs/text/public.txt");
    lcontrol=get_chain_head("../htdocs/text/control.txt"); 
	//FILE *fp;
	//char lan[3];
	int retu;
	char *encry=(char *)malloc(BUF_LEN);			  
	char *str;
	char * mode=(char *)malloc(AMOUNT);
	memset(mode,0,AMOUNT);
	int i;
	char addqos_encry[BUF_LEN];  
	char * qos_index=(char *)malloc(10);
	char * dp=(char *)malloc(10);
	char * up=(char *)malloc(10);
	char * tc=(char *)malloc(10);
	char * dscp=(char *)malloc(10);
	if(cgiFormSubmitClicked("submit_addqos") != cgiFormSuccess)
	{
	  memset(encry,0,BUF_LEN);
	  cgiFormStringNoNewlines("UN", encry, BUF_LEN); 
	  str=dcryption(encry);
	  if(str==NULL)
	  {
		ShowErrorPage(search(lpublic,"ill_user")); 		 /*用户非法*/
		return 0;
	  }
	  memset(addqos_encry,0,BUF_LEN);					 /*清空临时变量*/
	}
	ccgi_dbus_init();
	show_qos_mode(mode);
  cgiFormStringNoNewlines("encry_addvlan",addqos_encry,BUF_LEN);
  cgiFormStringNoNewlines("qos_index",qos_index,10);
  cgiFormStringNoNewlines("DP",dp,10);
  cgiFormStringNoNewlines("UP",up,10);
  cgiFormStringNoNewlines("TC",tc,10);
  cgiFormStringNoNewlines("DSCP",dscp,10);
   cgiHeaderContentType("text/html");
	fprintf(cgiOut,"<html xmlns=\"http://www.w3.org/1999/xhtml\"><head>");
  fprintf(cgiOut,"<meta http-equiv=Content-Type content=text/html; charset=gb2312>");
  fprintf(cgiOut,"<title>%s</title>","QOS Profile");
  fprintf(cgiOut,"<link rel=stylesheet href=/style.css type=text/css>"\
  "</head>"\
  "<body>");
  if(cgiFormSubmitClicked("submit_addqos") == cgiFormSuccess)
  {
  	if(strcmp(qos_index,"")!=0 && strcmp(dp,"")!=0 && strcmp(up,"")!=0 && strcmp(tc,"")!=0 && strcmp(dscp,"")!=0)
  	{
  		int temp1=0,temp2=0,temp3=0,temp4=0,temp5=0;
  		temp1=atoi(qos_index);
  		temp2=atoi(dp);
  		temp3=atoi(up);
  		temp4=atoi(tc);
  		temp5=atoi(dscp);

  		if(temp1<1 || temp1>127)
  			ShowAlert(search(lcontrol,"illegal_index_input"));
  		else if(temp2<0 || temp2>1)
  			ShowAlert(search(lcontrol,"illegal_dp_input"));
  		else if(temp3<0 || temp3>7)
  			ShowAlert(search(lcontrol,"illegal_up_input"));
  		else if(temp4<0 || temp4>7)
  			ShowAlert(search(lcontrol,"illegal_tc_input"));
  		else if(temp5<0 || temp5>63)
  			ShowAlert(search(lcontrol,"illegal_dscp_input"));
  		else
  		{
 	  		retu=set_qos_profile(qos_index,lcontrol);
			if(retu==-3)
			{
				ShowAlert(search(lcontrol,"set_qos_profile_fail"));
			}
			
    		retu=set_qos_profile_atrribute(qos_index,dp,up,tc,dscp,lcontrol);
			switch(retu)
			{
			  case 0:
			  	ShowAlert(search(lpublic,"oper_succ"));
				break;
			  case -1:
			  	ShowAlert(search(lpublic,"oper_fail"));
			  	break;
			  case -2:
			  	ShowAlert(search(lcontrol,"illegal_input"));
				break;
			  default:
			  	ShowAlert(search(lpublic,"oper_fail"));
			  	break;
			   
			}
    	}
    }
    else ShowAlert(search(lcontrol,"blank_input"));
  }

	
  fprintf(cgiOut,"<form method=post>"\
  "<div align=center>"\
  "<table width=976 border=0 cellpadding=0 cellspacing=0>"\
  "<tr>"\
    "<td width=8 align=left valign=top background=/images/di22.jpg><img src=/images/youce4.jpg width=8 height=30/></td>"\
    "<td width=51 align=left valign=bottom background=/images/di22.jpg><img src=/images/youce33.jpg width=37 height=24/></td>"\
    "<td width=153 align=left valign=bottom background=/images/di22.jpg><font id=titleen>QOS</font><font id=%s> %s</font></td>",search(lpublic,"title_style"),search(lpublic,"management"));
    fprintf(cgiOut,"<td width=690 align=right valign=bottom background=/images/di22.jpg>");
	  //  if((fp=fopen("../htdocs/text/public.txt","r"))==NULL)		 /*以只读方式打开资源文件*/
	  //    ShowAlert(search(lpublic,"error_open"));
	   // fseek(fp,4,0); 						/*将文件指针移到离文件首4个字节处，即lan=之后*/
	   // fgets(lan,3,fp);	   
	//	fclose(fp);
	  //  if(strcmp(lan,"ch")==0)
    	//{	
    	  fprintf(cgiOut,"<table width=130 border=0 cellspacing=0 cellpadding=0>"\
          "<tr>"\
          "<td width=62 align=center><input id=but type=submit name=submit_addqos style=background-image:url(/images/%s) value=""></td>",search(lpublic,"img_ok"));		  
		  if(cgiFormSubmitClicked("submit_addqos") != cgiFormSuccess)
            fprintf(cgiOut,"<td width=62 align=left><a href=wp_qosmap.cgi?UN=%s target=mainFrame><img src=/images/%s border=0 width=62 height=20/></a></td>",encry,search(lpublic,"img_cancel"));
		  else                                         
     		fprintf(cgiOut,"<td width=62 align=left><a href=wp_qosmap.cgi?UN=%s target=mainFrame><img src=/images/%s border=0 width=62 height=20/></a></td>",addqos_encry,search(lpublic,"img_cancel"));
		  fprintf(cgiOut,"</tr>"\
          "</table>");
		/*}		
		else			
		{	
		  fprintf(cgiOut,"<table width=130 border=0 cellspacing=0 cellpadding=0>"\
		  "<tr>"\
		  "<td width=62 align=center><input id=but type=submit name=submit_addqos style=background-image:url(/images/ok-en.jpg) value=""></td>");		  
		  if(cgiFormSubmitClicked("submit_addqos") != cgiFormSuccess)
		    fprintf(cgiOut,"<td width=62 align=left><a href=wp_qosmap.cgi?UN=%s target=mainFrame><img src=/images/cancel-en.jpg border=0 width=62 height=20/></a></td>",encry);
		  else
		    fprintf(cgiOut,"<td width=62 align=left><a href=wp_qosmap.cgi?UN=%s target=mainFrame><img src=/images/cancel-en.jpg border=0 width=62 height=20/></a></td>",addqos_encry);
		  fprintf(cgiOut,"</tr>"\
		  "</table>");
		}*/
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
            		if(cgiFormSubmitClicked("submit_addqos") != cgiFormSuccess)
            		{
            		  fprintf(cgiOut,"<tr height=25>"\
            			"<td align=left id=tdleft><a href=wp_qosmap.cgi?UN=%s target=mainFrame class=top><font id=yingwen_san>QOS </font><font id=%s>%s</font></a></td>",encry,search(lpublic,"menu_san"),search(lcontrol,"list"));		   
            		  fprintf(cgiOut,"</tr>"\
            		  "<tr height=26>"\
            			"<td align=left id=tdleft background=/images/bottom_bg.gif style=\"border-right:0\"><font id=%s>%s</font><font id=yingwen_san> QOS Profile</font></td>",search(lpublic,"menu_san"),search(lcontrol,"add"));	 /*突出显示*/
            		  fprintf(cgiOut,"</tr>");
            		  fprintf(cgiOut,"<tr height=25>"\
            			"<td align=left id=tdleft><a href=wp_addmap.cgi?UN=%s target=mainFrame class=top><font id=%s>%s</font></a></td>",encry,search(lpublic,"menu_san"),search(lcontrol,"add_map"));
            		  fprintf(cgiOut,"</tr>");
					  
			  fprintf(cgiOut,"<tr height=25>"\
            			"<td align=left id=tdleft><a href=wp_qosmapinfo.cgi?UN=%s target=mainFrame class=top><font id=%s>QOS %s</font></a></td>",encry,search(lpublic,"menu_san"),search(lcontrol,"map_detail"));
            		  fprintf(cgiOut,"</tr>");
					  
            		  fprintf(cgiOut,"<tr height=25>"\
            			"<td align=left id=tdleft><a href=wp_policymaplist.cgi?UN=%s target=mainFrame class=top><font id=%s>%s</font></a></td>",encry,search(lpublic,"menu_san"),search(lcontrol,"policy_map"));
            		  fprintf(cgiOut,"</tr>"\
            		  "<tr height=25>"\
            			"<td align=left id=tdleft><a href=wp_createpolicy.cgi?UN=%s target=mainFrame class=top><font id=%s>%s</font></a></td>",encry,search(lpublic,"menu_san"),search(lcontrol,"add_policy"));
            		  fprintf(cgiOut,"</tr>");
            		}
            		else if(cgiFormSubmitClicked("submit_addqos") == cgiFormSuccess)				
            		{
            		  fprintf(cgiOut,"<tr height=25>"\
            			"<td align=left id=tdleft><a href=wp_qosmap.cgi?UN=%s target=mainFrame style=color:#000000><font id=yingwen_san>QOS </font><font id=%s>%s</font></a></td>",addqos_encry,search(lpublic,"menu_san"),search(lcontrol,"list"));			 
            		  fprintf(cgiOut,"</tr>"\
            		  "<tr height=26>"\
            			"<td align=left id=tdleft background=/images/bottom_bg.gif style=\"border-right:0\"><font id=%s>%s</font><font id=yingwen_san> QOS Profile</font></td>",search(lpublic,"menu_san"),search(lcontrol,"add"));	 /*突出显示*/
            		  fprintf(cgiOut,"</tr>");
            		  fprintf(cgiOut,"<tr height=25>"\
            			"<td align=left id=tdleft><a href=wp_addmap.cgi?UN=%s target=mainFrame style=color:#000000><font id=%s>%s</font></a></td>",addqos_encry,search(lpublic,"menu_san"),search(lcontrol,"add_map"));
            		  fprintf(cgiOut,"</tr>");

			   fprintf(cgiOut,"<tr height=25>"\
            			"<td align=left id=tdleft><a href=wp_qosmapinfo.cgi?UN=%s target=mainFrame style=color:#000000><font id=%s>QOS %s</font></a></td>",addqos_encry,search(lpublic,"menu_san"),search(lcontrol,"map_detail"));
            		  fprintf(cgiOut,"</tr>");
					  
            		  fprintf(cgiOut,"<tr height=25>"\
            			"<td align=left id=tdleft><a href=wp_policymaplist.cgi?UN=%s target=mainFrame style=color:#000000><font id=%s>%s</font></a></td>",addqos_encry,search(lpublic,"menu_san"),search(lcontrol,"policy_map"));
            		  fprintf(cgiOut,"</tr>"\
            		  "<tr height=25>"\
            			"<td align=left id=tdleft><a href=wp_createpolicy.cgi?UN=%s target=mainFrame style=color:#000000><font id=%s>%s</font></a></td>",addqos_encry,search(lpublic,"menu_san"),search(lcontrol,"add_policy"));
            		  fprintf(cgiOut,"</tr>");
            		  
            		}
					for(i=0;i<10;i++)
						{
							fprintf(cgiOut,"<tr height=25>"\
							  "<td id=tdleft>&nbsp;</td>"\
							"</tr>");
						}

				  fprintf(cgiOut,"</table>"\
              "</td>"\
              "<td align=left valign=top style=\"background-color:#ffffff; border-right:1px solid #707070; padding-left:30px; padding-top:10px\">"\
					  "<table width=640 height=190 border=0 cellspacing=0 cellpadding=0>"\
						"<tr>"\
						  "<td align=left valign=top>"\
						  //****************************************************************************
						  "<table width=%s>\n","100%");
						  	fprintf(cgiOut,"<tr>\n"\
								  "<table width=330 border=0 cellspacing=0 cellpadding=0  style=padding-top:18px>"\
								  	"<tr height=25>");
							fprintf(cgiOut,"<td align=left style=color:#FF0000;font-size:14px;padding-left:2px colspan='3'><b>%s</b></td>",search(lcontrol,mode));
						fprintf(cgiOut,"</tr>"\
									 "<tr height=25>"\
										"<td align=left id=tdprompt style=font-size:12px>%s:</td>","QOS Profile Index");
										fprintf(cgiOut,"<td><input type=text name=qos_index size=21></td>"\
										"<td align=left style=color:#FF0000;font-size:12px;padding-left:2px>(1-127)</td>"\
									  "</tr>"\
									  "<tr height=25>"\
										"<td align=left id=tdprompt style=font-size:12px>%s:</td>","DP");
										fprintf(cgiOut,"<td><input type=text name=DP size=21></td>"\
										"<td align=left style=color:#FF0000;font-size:12px;padding-left:2px>(0-1)</td>"\
									  "</tr>"\
									  "<tr height=25>"\
										"<td align=left id=tdprompt style=font-size:12px>%s:</td>","UP");
										fprintf(cgiOut,"<td><input type=text name=UP size=21></td>"\
										"<td align=left style=color:#FF0000;font-size:12px;padding-left:2px>(0-7)</td>"\
									  "</tr>"\
									  "<tr height=25>"\
										"<td align=left id=tdprompt style=font-size:12px>%s:</td>","TC");
										fprintf(cgiOut,"<td><input type=text name=TC size=21></td>"\
										"<td align=left style=color:#FF0000;font-size:12px;padding-left:2px>(0-7)</td>"\
									  "</tr>"\
									  "<tr height=25>"\
										"<td align=left id=tdprompt style=font-size:12px>%s:</td>","DSCP");
								fprintf(cgiOut,"<td><input type=text name=DSCP size=21></td>"\
								"<td align=left style=color:#FF0000;font-size:12px;padding-left:2px>(0-63)</td>"\
							  "</tr>"\
									  "<tr>");
									  if(cgiFormSubmitClicked("submit_addqos") != cgiFormSuccess)
									  {
										fprintf(cgiOut,"<td colspan=2><input type=hidden name=encry_addvlan value=%s></td>",encry);
									  }
									  else if(cgiFormSubmitClicked("submit_addqos") == cgiFormSuccess)
									  {
										fprintf(cgiOut,"<td colspan=2><input type=hidden name=encry_addvlan value=%s></td>",addqos_encry);
									  }
									  fprintf(cgiOut,"</tr>"\
									"</table>"\
							"</tr>\n"\
							"<tr>\n"\
								"<td>\n"\
									"<table>\n");
							fprintf(cgiOut,"<tr>\n"\
											"<td  id=sec1 style=\"padding-left:23px;width=600; border-bottom:2px solid #53868b;font-size:14px\">%s</td>\n",search(lpublic,"description"));
							fprintf(cgiOut,"</tr>\n"\
										"<tr height=25 style=\"padding-left:23px; padding-top:2px\">\n"\
										      "<td style=font-size:14px;color:#FF0000 height='45'>%s</td>",search(lcontrol,"qos_mode_desc_one"));
							fprintf(cgiOut,"</tr>"\
										"<tr height=25 style=\"padding-left:23px; padding-top:2px\">"\
										      "<td style=font-size:14px;color:#FF0000 height='45'>%s</td>",search(lcontrol,"qos_mode_desc_two"));
							fprintf(cgiOut,"</tr>"\
										"<tr height=25 style=\"padding-left:23px; padding-top:2px\">"\
										      "<td style=font-size:14px;color:#FF0000 height='45'>%s</td>",search(lcontrol,"qos_mode_desc_three"));
							fprintf(cgiOut,"</tr>"\
								"</td>\n"\
							"</tr>\n"\
							"</table>\n"\
						//**********************************************************************************
						  "</td>"\
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
free(encry);
free(qos_index);
free(dp);
free(up);
free(tc);
free(dscp);
free(mode);
release(lpublic);  
release(lcontrol);
return 0;
}

