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
* wp_qosModule.c
*
*
* CREATOR:
* autelan.software.Network Dep. team
* zhouym@autelan.com
*
* DESCRIPTION:
* system contrl for qos infos
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
//#include "ws_dcli_vlan.h"

#define AMOUNT 512

int ShowAddvlanPage(); 


int cgiMain()
{
 ShowAddvlanPage();
 return 0;
}

int ShowAddvlanPage()
{
	ccgi_dbus_init();	
	struct list *lpublic;	/*解析public.txt文件的链表头*/
	struct list *lcontrol; 	/*解析help.txt文件的链表头*/
	lpublic=get_chain_head("../htdocs/text/public.txt");
   	lcontrol=get_chain_head("../htdocs/text/control.txt"); 
	//FILE *fp;
	//char lan[3];
	char addn[N]="";
	char *encry=(char *)malloc(BUF_LEN);			  
	char *str;
	int i;
	char * mode=(char *)malloc(AMOUNT);
	memset(mode,0,AMOUNT);
	char *select_mode=(char *)malloc(AMOUNT);
	memset(select_mode,0,AMOUNT);

	memset(encry,0,BUF_LEN);
  	if(cgiFormStringNoNewlines("UN", encry, BUF_LEN)!=cgiFormNotFound )  /*首次进入该页*/
  	{
    	  str=dcryption(encry);
    	  if(str==NULL)
    	  {
    		ShowErrorPage(search(lpublic,"ill_user")); 		 /*用户非法*/
    		return 0;
    	  }

	}
  	else 
	{
	  	cgiFormStringNoNewlines("encry_qosmodule",encry,BUF_LEN);
		str=dcryption(encry);
  	}
   cgiHeaderContentType("text/html");

   show_qos_mode(mode);

   	if(cgiFormSubmitClicked("change_mode")==cgiFormSuccess)
	{
		
		cgiFormStringNoNewlines("qos_mode",select_mode,AMOUNT);
		if(strcmp(select_mode,"")!=0)
		{
			configQosMode(select_mode);
			show_qos_mode(mode);
		}
		else
		{
			ShowAlert(search(lcontrol,"select_mode"));
		}
	}
	strcpy(addn,str);
	fprintf(cgiOut,"<html xmlns=\"http://www.w3.org/1999/xhtml\"><head>");
  fprintf(cgiOut,"<meta http-equiv=Content-Type content=text/html; charset=gb2312>");
  fprintf(cgiOut,"<title>%s</title>",search(lcontrol,"vlan_manage"));
  fprintf(cgiOut,"<link rel=stylesheet href=/style.css type=text/css>"\
  "</head>"\
  "<body>");


  fprintf(cgiOut,"<form method=post>"\
  "<div align=center>"\
  "<table width=976 border=0 cellpadding=0 cellspacing=0>"\
  "<tr>"\
    "<td width=8 align=left valign=top background=/images/di22.jpg><img src=/images/youce4.jpg width=8 height=30/></td>"\
    "<td width=51 align=left valign=bottom background=/images/di22.jpg><img src=/images/youce33.jpg width=37 height=24/></td>"\
    "<td width=153 align=left valign=bottom background=/images/di22.jpg><font id=titleen>QOS</font><font id=%s> %s</font></td>",search(lpublic,"title_style"),search(lpublic,"management"));
    fprintf(cgiOut,"<td width=690 align=right valign=bottom background=/images/di22.jpg>");
	   // if((fp=fopen("../htdocs/text/public.txt","r"))==NULL)		 /*以只读方式打开资源文件*/
	   //   ShowAlert(search(lpublic,"error_open"));
	  //  fseek(fp,4,0); 						/*将文件指针移到离文件首4个字节处，即lan=之后*/
	  //  fgets(lan,3,fp);	   
	//	fclose(fp);
	  //  if(strcmp(lan,"ch")==0)
    	//{	
    	  fprintf(cgiOut,"<table width=130 border=0 cellspacing=0 cellpadding=0>"\
          "<tr>");
          //"<td width=62 align=center><input id=but type=submit name=submit_qosModule style=background-image:url(/images/ok-ch.jpg) value=""></td>");
            fprintf(cgiOut,"<td width=62 align=left><a href=wp_contrl.cgi?UN=%s target=mainFrame><img src=/images/%s border=0 width=62 height=20/></a></td>",encry,search(lpublic,"img_ok"));
            fprintf(cgiOut,"<td width=62 align=left><a href=wp_contrl.cgi?UN=%s target=mainFrame><img src=/images/%s border=0 width=62 height=20/></a></td>",encry,search(lpublic,"img_cancel"));
		  fprintf(cgiOut,"</tr>"\
          "</table>");
		/*}		
		else			
		{	
		  fprintf(cgiOut,"<table width=130 border=0 cellspacing=0 cellpadding=0>"\
		  "<tr>");
		  "<td width=62 align=center><input id=but type=submit name=submit_qosModule style=background-image:url(/images/ok-en.jpg) value=""></td>");
		    fprintf(cgiOut,"<td width=62 align=left><a href=wp_contrl.cgi?UN=%s target=mainFrame><img src=/images/ok-en.jpg border=0 width=62 height=20/></a></td>",encry);
		    fprintf(cgiOut,"<td width=62 align=left><a href=wp_contrl.cgi?UN=%s target=mainFrame><img src=/images/cancel-en.jpg border=0 width=62 height=20/></a></td>",encry);
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

        			fprintf(cgiOut,"<tr height=25>"\
        			"<td align=left id=tdleft><a href=wp_qosmap.cgi?UN=%s target=mainFrame class=top><font id=yingwen_san>QOS </font><font id=%s>%s</font></a></td>",encry,search(lpublic,"menu_san"),search(lcontrol,"mapping"));					   
        		  fprintf(cgiOut,"</tr>");
        		  
        		  fprintf(cgiOut,"<tr height=25>"\
        			"<td align=left id=tdleft><a href=wp_policer.cgi?UN=%s target=mainFrame class=top><font id=%s>%s</font></a></td>",encry,search(lpublic,"menu_san"),search(lcontrol,"policer"));					   
        		  fprintf(cgiOut,"</tr>"\
        		  
        		  "<tr height=25>"\
        			"<td align=left id=tdleft><a href=wp_config_queue.cgi?UN=%s target=mainFrame class=top><font id=%s>%s</font></a></td>",encry,search(lpublic,"menu_san"),search(lcontrol,"queue_scheduler"));
        		  fprintf(cgiOut,"</tr>"\

        		  "<tr height=25>"\
        			"<td align=left id=tdleft><a href=wp_TrafficShapping.cgi?UN=%s target=mainFrame class=top><font id=%s>%s</font></a></td>",encry,search(lpublic,"menu_san"),search(lcontrol,"Traffic_Shapping"));
        		  fprintf(cgiOut,"</tr>");

					for(i=0;i<11;i++)
					  {
						fprintf(cgiOut,"<tr height=25>"\
						  "<td id=tdleft>&nbsp;</td>"\
						"</tr>");
					  }

				  fprintf(cgiOut,"</table>"\
              "</td>"\
              "<td align=left valign=top style=\"background-color:#ffffff; border-right:1px solid #707070; padding-left:30px; padding-top:10px\">"\
					  "<table width=640 height=290 border=0 cellspacing=0 cellpadding=0>"\
						"<tr>"\
						  "<td align=left valign=top>"\
						   "<table width=600 border=0 cellspacing=0 cellpadding=0>"\
						   "<tr height=30>");
						if(checkuser_group(addn)==0)/*administrator*/
						{
         	fprintf(cgiOut,"<td style=\"border-bottom:2px solid #163871\">"\
         			       	"<table style=width:%s; height:auto>","100%");
         			       		fprintf(cgiOut,"<tr>");
         			      fprintf(cgiOut,"<td align=left width=%s><font id=%s1>%s</font></td>","20%",search(lpublic,"menu_summary"),search(lpublic,"summary"));
				      fprintf(cgiOut,"<td align=center id=tdprompt style=font-size:12px>QOS %s:</td>",search(lcontrol,"mode"));
				      fprintf(cgiOut,"<td>"\
							          "<select name='qos_mode'>");
				      		fprintf(cgiOut,"<option value=''>--%s--</option>",search(lcontrol,"select"));
						fprintf(cgiOut,"<option value='default'>%s</option>",search(lcontrol,"select_default"));
						fprintf(cgiOut,"<option value='hybird'>%s</option>",search(lcontrol,"select_hybird"));
						fprintf(cgiOut,"<option value='flow'>%s</option>",search(lcontrol,"select_flow"));
						fprintf(cgiOut,"<option value='port'>%s</option>",search(lcontrol,"select_port"));
					fprintf(cgiOut,"</select>"\
							    "</td>");
					fprintf(cgiOut,"<td align=center><input type='submit' name='change_mode' value='%s'></td>",search(lcontrol,"change_mode"));
         			      fprintf(cgiOut,"<td align='right' style=\"font-size:14px\"><font color='red'><b>%s</b></font></td>",search(lcontrol,mode));
				fprintf(cgiOut,"</tr>"\
						"</table>"\
					"</td>");
						}
						else
						{
         	fprintf(cgiOut,"<td style=\"border-bottom:2px solid #163871\"><font id=%s1>%s</font></td>",search(lpublic,"menu_summary"),search(lpublic,"summary"));
						}
                           fprintf(cgiOut,"</tr>"\
                           "<tr>"\
                          "<td style=\"padding-left:30px; padding-top:5px\"><table width=500 border=0 cellspacing=0 cellpadding=0>"\
                           "<tr>"\
                            "<td colspan=3 style=\"border-bottom:1px solid black; padding-top:25px\"><font id=%s2>%s</font></td>",search(lpublic,"menu_summary"),search(lcontrol,"qos_mapping"));
                           fprintf(cgiOut,"</tr>"\
                           "<tr align=left style=\"padding-top:10px; padding-left:10px\">");
                          // if(strcmp(lan,"ch")==0)
								//fprintf(cgiOut,"<td><font id=%s3>%s</font></td>",search(lpublic,"menu_summary"),"QOS映射包括QOS Profile显示和增删,QOS Profile映射关系,流处理策略的显示增删以及配置");
						   //else
						   		fprintf(cgiOut,"<td><font id=%s3>%s</font></td>",search(lpublic,"menu_summary"),search(lcontrol,"QOSmapping"));
                            fprintf(cgiOut,"</tr>"\

                           "<tr>"\
                            "<td colspan=3 style=\"border-bottom:1px solid black; padding-top:25px\"><font id=%s2>%s</font></td>",search(lpublic,"menu_summary"),search(lcontrol,"policer"));
                           fprintf(cgiOut,"</tr>"\
                           "<tr align=left style=\"padding-top:10px; padding-left:10px\">");
                          // if(strcmp(lan,"ch")==0)
							//	fprintf(cgiOut,"<td><font id=%s3>%s</font></td>",search(lpublic,"menu_summary"),"流量监管包括,增删流量监管,显示流量监管,设置监管属性CIR,CBS,溢出操作以及监管模式");
						//   else
						   		fprintf(cgiOut,"<td><font id=%s3>%s</font></td>",search(lpublic,"menu_summary"),search(lcontrol,"PolicerTrafficflow"));
                            fprintf(cgiOut,"</tr>"\

                            "<tr>"\
                            "<td colspan=3 style=\"border-bottom:1px solid black; padding-top:25px\"><font id=%s2>%s</font></td>",search(lpublic,"menu_summary"),search(lcontrol,"queue_scheduler"));
                           fprintf(cgiOut,"</tr>"\
                           "<tr align=left style=\"padding-top:10px; padding-left:10px\">");
                         //  if(strcmp(lan,"ch")==0)
							//	fprintf(cgiOut,"<td><font id=%s3>%s</font></td>",search(lpublic,"menu_summary"),"队列调度包括显示队列调度信息,配置队列调度模式,设置调度属性");
						  // else
						   		fprintf(cgiOut,"<td><font id=%s3>%s</font></td>",search(lpublic,"menu_summary"),search(lcontrol,"Queuescheduling"));
                            fprintf(cgiOut,"</tr>"\

                            "<tr>"\
                            "<td colspan=3 style=\"border-bottom:1px solid black; padding-top:25px\"><font id=%s2>%s</font></td>",search(lpublic,"menu_summary"),search(lcontrol,"Traffic_Shapping"));
                           fprintf(cgiOut,"</tr>"\
                           "<tr align=left style=\"padding-top:10px; padding-left:10px\">");
                           //if(strcmp(lan,"ch")==0)
							//	fprintf(cgiOut,"<td><font id=%s3>%s</font></td>",search(lpublic,"menu_summary"),"流量整形包括整形的模式,配置出端口,最大速率和突发量,整形的配置信息显示以及删除");
						//   else
						   		fprintf(cgiOut,"<td><font id=%s3>%s</font></td>",search(lpublic,"menu_summary"),search(lcontrol,"Trafficshaping"));
                            fprintf(cgiOut,"</tr>");
                            
				fprintf(cgiOut,"</table>"\
                           "</td>"\
                           "</tr>"\
						   "</table>"\
						  "</td>"\
						"</tr>"\
						"<tr>");
						fprintf(cgiOut,"<td ><input type=hidden name=encry_qosmodule value=%s></td>",encry);
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
free(encry);
release(lpublic);  
release(lcontrol);
free(mode);
free(select_mode);
return 0;
}


