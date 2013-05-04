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
* wp_addmap.c
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
#include "ws_ec.h"
#include "ws_usrinfo.h"
#include "ws_init_dbus.h"
#include "ws_sysinfo.h"
#include "ws_dcli_qos.h"

#define AMOUNT 512
int ShowAddMapPage(); 
int addmap_hand(struct list * lcontrol,struct list *lpublic);

int cgiMain()
{
 ShowAddMapPage();
 return 0;
}

int ShowAddMapPage()
{
	ccgi_dbus_init();
	struct list *lpublic;	/*解析public.txt文件的链表头*/
	struct list *lcontrol; 	/*解析help.txt文件的链表头*/
	lpublic=get_chain_head("../htdocs/text/public.txt");
    lcontrol=get_chain_head("../htdocs/text/control.txt"); 
	//FILE *fp;
	//char lan[3];
	char *encry=(char *)malloc(BUF_LEN);			  
	char *str;
	int i;
	char * mode=(char *)malloc(AMOUNT);
	memset(mode,0,AMOUNT);
	char addmap_encry[BUF_LEN];  
	if(cgiFormSubmitClicked("submit_addmap") != cgiFormSuccess)
	{
	  memset(encry,0,BUF_LEN);
	  cgiFormStringNoNewlines("UN", encry, BUF_LEN); 
	  str=dcryption(encry);
	  if(str==NULL)
	  {
		ShowErrorPage(search(lpublic,"ill_user")); 		 /*用户非法*/
		return 0;
	  }
	  memset(addmap_encry,0,BUF_LEN);	/*清空临时变量*/		

	}
	
  cgiFormStringNoNewlines("encry_addmap",addmap_encry,BUF_LEN);
   cgiHeaderContentType("text/html");
	fprintf(cgiOut,"<html xmlns=\"http://www.w3.org/1999/xhtml\"><head>");
  fprintf(cgiOut,"<meta http-equiv=Content-Type content=text/html; charset=gb2312>");
  fprintf(cgiOut,"<title>%s</title>",search(lcontrol,"qos_manage"));
  fprintf(cgiOut,"<link rel=stylesheet href=/style.css type=text/css>");
	fprintf(cgiOut,"<script type=\"text/javascript\">");
	fprintf(cgiOut,"function isNum1(str)"\
		"{"\
			"if(str!=null&&str!=\"\")"\
			"{"\
				"var a = str.match(/^[0-9]*$/);"\
				"if (a == null)"\
				"{"\
					"alert(\"%s\");",search(lcontrol,"notnum"));
					fprintf(cgiOut,"document.all.flag.value = \"\";"\
					"document.all.flag.focus();"\
				"}"\
				"return true;"\
			"}"\
	"}");

	fprintf(cgiOut,"function isNum2(str)"\
		"{"\
			"if(str!=null&&str!=\"\")"\
			"{"\
				"var a = str.match(/^[0-9]*$/);"\
				"if (a == null)"\
				"{"\
					"alert(\"%s\");",search(lcontrol,"notnum"));
					fprintf(cgiOut,"document.all.qosprofile.value = \"\";"\
					"document.all.qosprofile.focus();"\
				"}"\
				"return true;"\
			"}"\
	"}");
	fprintf(cgiOut,"</script>");

  
  fprintf(cgiOut,"</head>"\
  "<body>");
  if(cgiFormSubmitClicked("submit_addmap") == cgiFormSuccess)
  {
    	addmap_hand(lcontrol,lpublic);
  }
	show_qos_mode(mode);
  fprintf(cgiOut,"<form method=post>"\
  "<div align=center>"\
  "<table width=976 border=0 cellpadding=0 cellspacing=0>"\
  "<tr>"\
    "<td width=8 align=left valign=top background=/images/di22.jpg><img src=/images/youce4.jpg width=8 height=30/></td>"\
    "<td width=51 align=left valign=bottom background=/images/di22.jpg><img src=/images/youce33.jpg width=37 height=24/></td>"\
    "<td width=153 align=left valign=bottom background=/images/di22.jpg><font id=titleen>QOS</font><font id=%s> %s</font></td>",search(lpublic,"title_style"),search(lpublic,"management"));
    fprintf(cgiOut,"<td width=690 align=right valign=bottom background=/images/di22.jpg>");
	    //if((fp=fopen("../htdocs/text/public.txt","r"))==NULL)		 /*以只读方式打开资源文件*/
	   ///   ShowAlert(search(lpublic,"error_open"));
	   // fseek(fp,4,0); 						/*将文件指针移到离文件首4个字节处，即lan=之后*/
	   // fgets(lan,3,fp);	   
		//fclose(fp);
	   // if(strcmp(lan,"ch")==0)
    	//{	
    	  fprintf(cgiOut,"<table width=130 border=0 cellspacing=0 cellpadding=0>"\
          "<tr>"\
          "<td width=62 align=center><input id=but type=submit name=submit_addmap style=background-image:url(/images/%s) value=""></td>",search(lpublic,"img_ok"));
		  if(cgiFormSubmitClicked("submit_addmap") != cgiFormSuccess)
            fprintf(cgiOut,"<td width=62 align=left><a href=wp_qosmap.cgi?UN=%s target=mainFrame><img src=/images/%s border=0 width=62 height=20/></a></td>",encry,search(lpublic,"img_cancel"));
		  else                                         
     		fprintf(cgiOut,"<td width=62 align=left><a href=wp_qosmap.cgi?UN=%s target=mainFrame><img src=/images/%s border=0 width=62 height=20/></a></td>",addmap_encry,search(lpublic,"img_cancel"));
		  fprintf(cgiOut,"</tr>"\
          "</table>");
		/*}		
		else			
		{	
		  fprintf(cgiOut,"<table width=130 border=0 cellspacing=0 cellpadding=0>"\
		  "<tr>"\
		  "<td width=62 align=center><input id=but type=submit name=submit_addmap style=background-image:url(/images/ok-en.jpg) value=""></td>");		  
		  if(cgiFormSubmitClicked("submit_addmap") != cgiFormSuccess)
		    fprintf(cgiOut,"<td width=62 align=left><a href=wp_qosmap.cgi?UN=%s target=mainFrame><img src=/images/cancel-en.jpg border=0 width=62 height=20/></a></td>",encry);
		  else
		    fprintf(cgiOut,"<td width=62 align=left><a href=wp_qosmap.cgi?UN=%s target=mainFrame><img src=/images/cancel-en.jpg border=0 width=62 height=20/></a></td>",addmap_encry);
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
            		if(cgiFormSubmitClicked("submit_addmap") != cgiFormSuccess)
            		{
            		  fprintf(cgiOut,"<tr height=25>"\
            			"<td align=left id=tdleft><a href=wp_qosmap.cgi?UN=%s target=mainFrame class=top><font id=yingwen_san>QOS </font><font id=%s>%s</font></a></td>",encry,search(lpublic,"menu_san"),search(lcontrol,"list"));					   
            		  fprintf(cgiOut,"</tr>"\
            		  "<tr height=25>"\
            			"<td align=left id=tdleft><a href=wp_addqos.cgi?UN=%s target=mainFrame class=top><font id=%s>%s</font><font id=yingwen_san>QOS Profile</font></a></td>",encry,search(lpublic,"menu_san"),search(lcontrol,"add"));
            		  fprintf(cgiOut,"</tr>"\
            		    "<tr height=26>"\
            			"<td align=left id=tdleft background=/images/bottom_bg.gif style=\"border-right:0\"><font id=%s>%s</font></td>",search(lpublic,"menu_san"),search(lcontrol,"add_map"));	 /*突出显示*/
            		  fprintf(cgiOut,"</tr>");
			 
			   fprintf(cgiOut,"<tr height=25>"\
            			"<td align=left id=tdleft><a href=wp_qosmapinfo.cgi?UN=%s target=mainFrame class=top><font id=%s>QOS %s</font></a></td>",encry,search(lpublic,"menu_san"),search(lcontrol,"map_detail"));
            		  fprintf(cgiOut,"</tr>");
					  
					  
            		  fprintf(cgiOut,"<tr height=25>"\
            			"<td align=left id=tdleft><a href=wp_policymaplist.cgi?UN=%s target=mainFrame class=top><font id=%s>%s</font></a></td>",encry,search(lpublic,"menu_san"),search(lcontrol,"policy_map"));
            		  fprintf(cgiOut,"</tr>");
            		  fprintf(cgiOut,"<tr height=25>"\
            			"<td align=left id=tdleft><a href=wp_createpolicy.cgi?UN=%s target=mainFrame class=top><font id=%s>%s</font></a></td>",encry,search(lpublic,"menu_san"),search(lcontrol,"add_policy"));
            		  fprintf(cgiOut,"</tr>");
            		}
            		else if(cgiFormSubmitClicked("submit_addmap") == cgiFormSuccess)				
            		{
            		  fprintf(cgiOut,"<tr height=25>"\
            			"<td align=left id=tdleft><a href=wp_qosmap.cgi?UN=%s target=mainFrame style=color:#000000><font id=yingwen_san>QOS </font><font id=%s>%s</font></a></td>",addmap_encry,search(lpublic,"menu_san"),search(lcontrol,"list"));						 
            		  fprintf(cgiOut,"</tr>"\
            		  "<tr height=25>"\
            			"<td align=left id=tdleft><a href=wp_addqos.cgi?UN=%s target=mainFrame style=color:#000000><font id=%s>%s</font><font id=yingwen_san>QOS Profile</font></a></td>",addmap_encry,search(lpublic,"menu_san"),search(lcontrol,"add"));
            		  fprintf(cgiOut,"</tr>"\
            		  "<tr height=26>"\
            			"<td align=left id=tdleft background=/images/bottom_bg.gif style=\"border-right:0\"><font id=%s>%s</font></td>",search(lpublic,"menu_san"),search(lcontrol,"add_map"));	 /*突出显示*/
            		  fprintf(cgiOut,"</tr>");

			 fprintf(cgiOut,"<tr height=25>"\
            			"<td align=left id=tdleft><a href=wp_qosmapinfo.cgi?UN=%s target=mainFrame class=top><font id=%s>QOS %s</font></a></td>",addmap_encry,search(lpublic,"menu_san"),search(lcontrol,"map_detail"));
            		  fprintf(cgiOut,"</tr>");
					  
            		  fprintf(cgiOut,"<tr height=25>"\
            			"<td align=left id=tdleft><a href=wp_policymaplist.cgi?UN=%s target=mainFrame class=top><font id=%s>%s</font></a></td>",addmap_encry,search(lpublic,"menu_san"),search(lcontrol,"policy_map"));
            		  fprintf(cgiOut,"</tr>");
            		  fprintf(cgiOut,"<tr height=25>"\
            			"<td align=left id=tdleft><a href=wp_createpolicy.cgi?UN=%s target=mainFrame class=top><font id=%s>%s</font></a></td>",addmap_encry,search(lpublic,"menu_san"),search(lcontrol,"add_policy"));
            		  fprintf(cgiOut,"</tr>");
            		}
					for(i=0;i<1;i++)
					  {
						fprintf(cgiOut,"<tr height=25>"\
						  "<td id=tdleft>&nbsp;</td>"\
						"</tr>");
					  }

				  fprintf(cgiOut,"</table>"\
              "</td>"\
              "<td align=left valign=top style=\"background-color:#ffffff; border-right:1px solid #707070; padding-left:30px; padding-top:10px\">"\
					  "<table width=640 height=115 border=0 cellspacing=0 cellpadding=0>"\
						"<tr>"\
						  "<td align=left valign=top>"\
						  "<table width=350 border=0 cellspacing=0 cellpadding=0  style=padding-top:18px>"\
						  	"<tr height=30>");
					fprintf(cgiOut,"<td style=\"font-size:14px\"><font color='red'><b>%s</b></font></td>",search(lcontrol,mode));
				fprintf(cgiOut,"</tr>"\
							 "<tr height=30>"\
								"<td align=left id=tdprompt style=font-size:12px>%s:</td>",search(lcontrol,"Map_Type"));
								fprintf(cgiOut,"<td>");
								if(get_product_id()!=PRODUCT_ID_AU3K_BCM)
								{
									fprintf(cgiOut,"<select name=map_type>");
										fprintf(cgiOut,"<option value=%s>%s","up-to-profile","up-to-profile");
										fprintf(cgiOut,"<option value=%s>%s","dscp-to-profile","dscp-to-profile");
										fprintf(cgiOut,"<option value=%s>%s","dscp-to-dscp","dscp-to-dscp");
									fprintf(cgiOut,"</select>");
								}
								else
								{
									fprintf(cgiOut,"<input type=hidden name=map_type value=dscp-to-profile><b>dscp-to-profile</b>");
								}
								fprintf(cgiOut,"</td>");
							  fprintf(cgiOut,"</tr>"\
							  "<tr height=30>"\
								"<td align=left id=tdprompt style=font-size:12px>%s:</td>",search(lcontrol,"Map_src"));
								fprintf(cgiOut,"<td><input type=text name=flag id=flag size=21 onblur=\"isNum1(this.value);\"></td>"\
							  "</tr>"\
							  "<tr height=30>"\
								"<td align=left id=tdprompt style=font-size:12px>%s:</td>",search(lcontrol,"Map_dst"));
								fprintf(cgiOut,"<td><input type=text name=qosprofile id=qosprofile size=21 onblur=\"isNum2(this.value);\"></td>"\
							  "</tr>"\
							  "<tr>");
							  if(cgiFormSubmitClicked("submit_addmap") != cgiFormSuccess)
							  {
								fprintf(cgiOut,"<td colspan=2><input type=hidden name=encry_addmap value=%s></td>",encry);
							  }
							  else if(cgiFormSubmitClicked("submit_addmap") == cgiFormSuccess)
							  {
								fprintf(cgiOut,"<td colspan=2><input type=hidden name=encry_addmap value=%s></td>",addmap_encry);
							  }
							  fprintf(cgiOut,"</tr>"\
							"</table>"\
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
release(lpublic);  
release(lcontrol);
free(mode);
return 0;
}

															 
int addmap_hand(struct list * lcontrol,struct list *lpublic)
{
    int retz;
	char * map_type=(char *)malloc(20);
	char * flag=(char *)malloc(10);
	char * qosprofile=(char *)malloc(10);
	cgiFormStringNoNewlines("map_type",map_type,20);
	cgiFormStringNoNewlines("flag",flag,10);
	cgiFormStringNoNewlines("qosprofile",qosprofile,10);
	if(strcmp(flag,"")!=0 || strcmp(qosprofile,"")!=0)
	{
     	if(strcmp(map_type,"up-to-profile")==0)
     	{
     		int temp1=0,temp2=0;
     		temp1=atoi(flag);
     		temp2=atoi(qosprofile);
     		if(temp1<0 || temp1>7)
  				ShowAlert(search(lcontrol,"illegal_up_input"));
  			else if(temp2<1 || temp2>127)
  				ShowAlert(search(lcontrol,"illegal_index_input"));
  			else 
			{
     			retz=set_up_to_profile(flag,qosprofile,lcontrol);
				switch(retz)
				{
					case 0:
						ShowAlert(search(lcontrol,"map_up_qos_suc"));
						break;
					case -2:
						ShowAlert(search(lcontrol,"qos_profile_not_exist"));
						break;					
					case -5:
						ShowAlert(search(lcontrol,"map_up_qos_fail"));
						break;
					default:
						ShowAlert(search(lpublic,"oper_fail"));
						break;
				}
			}
     	}
     	else if(strcmp(map_type,"dscp-to-profile")==0)
     	{
     		int temp1=0,temp2=0;
     		temp1=atoi(flag);
     		temp2=atoi(qosprofile);
     		if(temp1<0 || temp1>63)
  				ShowAlert(search(lcontrol,"illegal_dscp_input"));
  			else if(temp2<1 || temp2>127)
  				ShowAlert(search(lcontrol,"illegal_index_input"));
  			else
			{
		 		 retz=set_dscp_to_profile(flag,qosprofile,lcontrol);
				 switch(retz)
				{
					case 0:
						ShowAlert(search(lcontrol,"map_dscp_qos_suc"));
						break;
					case -3:
						ShowAlert(search(lcontrol,"map_dscp_qos_fail"));
						break;
					case -2:
						ShowAlert(search(lcontrol,"qos_profile_not_exist"));
						break;
					default:
						ShowAlert(search(lpublic,"oper_fail"));
						break;
				}
			}
     	}
     	else if(strcmp(map_type,"dscp-to-dscp")==0)
     	{
     		int temp1=0,temp2=0;
     		temp1=atoi(flag);
     		temp2=atoi(qosprofile);
     		if(temp1<0 || temp1>63)
  				ShowAlert(search(lcontrol,"illegal_dscp_input"));
  			else if(temp2<0 || temp2>63)
  				ShowAlert(search(lcontrol,"illegal_dscp_input"));
  			else
     		{
     			retz=set_dscp_to_dscp(flag,qosprofile,lcontrol);
				switch(retz)
				{
					case 0:
						ShowAlert(search(lcontrol,"map_dscp_dscp_suc"));
						break;
					case -3:
						ShowAlert(search(lcontrol,"map_dscp_dscp_fail"));
						break;
					default:
						ShowAlert(search(lpublic,"oper_fail"));
						break;
				}
			}
     	}
     }
     else ShowAlert(search(lcontrol,"blank_input"));
     
	free(map_type);
	free(flag);
	free(qosprofile);
	return 1;
}

