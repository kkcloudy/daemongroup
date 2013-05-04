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
* wp_mirror_tool.c
*
*
* CREATOR:
* autelan.software.Network Dep. team
* zhouym@autelan.com
*
* DESCRIPTION:
* system contrl for tools mirror
*
*
*******************************************************************************/
#include <stdio.h>
#include "cgic.h"
#include <string.h>
#include <stdlib.h>
#include "ws_usrinfo.h"
#include "ws_init_dbus.h"

#include "ws_dcli_mirror.h"


int ShowMirrorPage(); 


int cgiMain()
{
 ShowMirrorPage();
 return 0;
}

int ShowMirrorPage()
{
	struct list *lpublic;	/*解析public.txt文件的链表头*/
	struct list *lcontrol; 	/*解析help.txt文件的链表头*/
	lpublic=get_chain_head("../htdocs/text/public.txt");
    lcontrol=get_chain_head("../htdocs/text/control.txt"); 
	FILE *fp;
	char lan[3];
	char *encry=(char *)malloc(BUF_LEN);			  
	char *str;
	int i;
	char show_mirror[BUF_LEN];
	struct mirror_info * rev_mirror_info;
	rev_mirror_info=(struct mirror_info * )malloc(sizeof(struct mirror_info));
	if(cgiFormSubmitClicked("submit_mirror") != cgiFormSuccess)
	{
	  memset(encry,0,BUF_LEN);
	  cgiFormStringNoNewlines("UN", encry, BUF_LEN); 
	  str=dcryption(encry);
	  if(str==NULL)
	  {
		ShowErrorPage(search(lpublic,"ill_user")); 		 /*用户非法*/
		return 0;
	  }
	  memset(show_mirror,0,BUF_LEN);					 /*清空临时变量

*/
	}
  ccgi_dbus_init();
  cgiFormStringNoNewlines("encry_mirror",show_mirror,BUF_LEN);
  cgiHeaderContentType("text/html");
  fprintf(cgiOut,"<html xmlns=\"http://www.w3.org/1999/xhtml\"><head>");
  fprintf(cgiOut,"<meta http-equiv=Content-Type content=text/html; charset=gb2312>");
  fprintf(cgiOut,"<title>%s</title>",search(lcontrol,"vlan_manage"));
  fprintf(cgiOut,"<link rel=stylesheet href=/style.css type=text/css>"\
  	"<style>"\
  	".mirror_table {overflow:auto;  overflow-x:hidden; border:1; width:300px;height:190px;clip: rect( ); padding-top: 0px; padding-right: 0px; padding-bottom: 0px; padding-left: 0px}"\
  	"</style>"\
  "</head>"\
  "<body>");
  if(cgiFormSubmitClicked("submit_mirror") == cgiFormSuccess)
  {
   
  }

  fprintf(cgiOut,"<form method=post>"\
  "<div align=center>"\
  "<table width=976 border=0 cellpadding=0 cellspacing=0>"\
  "<tr>"\
    "<td width=8 align=left valign=top background=/images/di22.jpg><img src=/images/youce4.jpg width=8 height=30/></td>"\
    "<td width=51 align=left valign=bottom background=/images/di22.jpg><img src=/images/youce33.jpg width=37 height=24/></td>"\
    "<td width=153 align=left valign=bottom background=/images/di22.jpg><font id=titleen>MIRROR</font><font id=%s> %s</font></td>",search(lpublic,"title_style"),search(lpublic,"management"));
    fprintf(cgiOut,"<td width=690 align=right valign=bottom background=/images/di22.jpg>");
	    if((fp=fopen("../htdocs/text/public.txt","r"))==NULL)		 /*以只读方式打开资源文件*/
		{
			ShowAlert(search(lpublic,"error_open"));
	    }
		else
		{
			fseek(fp,4,0);						/*将文件指针移到离文件首4个字节处，即lan=之后*/
			fgets(lan,3,fp);	   
			fclose(fp);
		}
	
    	  fprintf(cgiOut,"<table width=130 border=0 cellspacing=0 cellpadding=0>"\
          "<tr>"\
          "<td width=62 align=center><input id=but type=submit name=submit_mirror style=background-image:url(/images/%s) value=""></td>",search(lpublic,"img_ok"));	  
		  if(cgiFormSubmitClicked("submit_mirror") != cgiFormSuccess)
            fprintf(cgiOut,"<td width=62 align=left><a href=wp_command.cgi?UN=%s target=mainFrame><img src=/images/%s border=0 width=62 height=20/></a></td>",encry,search(lpublic,"img_cancel"));
		  else                                         
     		fprintf(cgiOut,"<td width=62 align=left><a href=wp_command.cgi?UN=%s target=mainFrame><img src=/images/%s border=0 width=62 height=20/></a></td>",show_mirror,search(lpublic,"img_cancel"));
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
            		if(cgiFormSubmitClicked("submit_mirror") != cgiFormSuccess)
            		{
            		  
					  //突出条幅
                      fprintf(cgiOut,"<tr height=26>"\
                             "<td align=left id=tdleft background=/images/bottom_bg.gif style=\"border-right:0\"><font id=%s>%s%s</font></td>",search(lpublic,"menu_san"),search(lcontrol,"show"),search(lcontrol,"mirror"));   //突出显示 
                      fprintf(cgiOut,"</tr>");
					  fprintf(cgiOut,"<tr height=25>"\
            		  "<td align=left id=tdleft><a href=wp_config_mirror.cgi?UN=%s target=mainFrame class=top><font id=%s>%s%s</font></a></td>",encry,search(lpublic,"menu_san"),search(lcontrol,"config"),search(lcontrol,"mirror")); 
            		  fprintf(cgiOut,"</tr>");
            		}
            		else if(cgiFormSubmitClicked("submit_mirror") == cgiFormSuccess)				
            		{
					  //突出条幅
                      fprintf(cgiOut,"<tr height=26>"\
                             "<td align=left id=tdleft background=/images/bottom_bg.gif style=\"border-right:0\"><font id=%s>%s%s</font></td>",search(lpublic,"menu_san"),search(lcontrol,"show"),search(lcontrol,"mirror"));   //突出显示 
                      fprintf(cgiOut,"</tr>");
					  fprintf(cgiOut,"<tr height=25>"\
            		  "<td align=left id=tdleft><a href=wp_config_mirror.cgi?UN=%s target=mainFrame class=top><font id=%s>%s%s</font></a></td>",show_mirror,search(lpublic,"menu_san"),search(lcontrol,"config"),search(lcontrol,"mirror"));
            		  fprintf(cgiOut,"</tr>");
            		}
					for(i=0;i<6;i++)
					  {
						fprintf(cgiOut,"<tr height=25>"\
						  "<td id=tdleft>&nbsp;</td>"\
						"</tr>");
					  }

				  fprintf(cgiOut,"</table>"\
              "</td>"\
              "<td align=left valign=top style=\"background-color:#ffffff; border-right:1px solid #707070; padding-left:30px; padding-top:10px\">"\
					  "<table width=640 height=200 border=0 cellspacing=0 cellpadding=0>"\
						"<tr>"\
						  "<td align=left valign=top>"\
						  "<div class=mirror_table><table width=260 border=0 cellspacing=0 cellpadding=0  style=padding-top:18px>");
				  			//fprintf(stderr,"111111");
				  			show_mirror_configure(rev_mirror_info);
							//fprintf(stderr,"policyindex=%u-num=%d",rev_mirror_info->policyindex[0],rev_mirror_info->policyindexNum);
							if( strcmp(rev_mirror_info->destPort_bid,"")==0 && strcmp(rev_mirror_info->destPort_in,"")==0 && strcmp(rev_mirror_info->destPort_eg,"")==0)
								{
									fprintf(cgiOut,"<tr height=50><td><font color=red>%s</font></td></tr>",search(lcontrol,"no_config_mirror_port"));

								}
							else
								{
								  if(rev_mirror_info->bid_flag)
								  	{
        							  fprintf(cgiOut,"<tr height=30>"\
        							  "<td align=left id=tdprompt style=font-size:12px>%s:</td>","Bidirection dest Port");
        							  fprintf(cgiOut,"<td align=left>%s</td>",rev_mirror_info->destPort_bid);
        							  fprintf(cgiOut,"</tr>");
								  	}
								   if(rev_mirror_info->in_flag)
								  	{
        							  fprintf(cgiOut,"<tr height=30>"\
        							  "<td align=left id=tdprompt style=font-size:12px>%s:</td>","ingress dest Port");
        							  fprintf(cgiOut,"<td align=left>%s</td>",rev_mirror_info->destPort_in);
        							  fprintf(cgiOut,"</tr>");
								  	}
								    if(rev_mirror_info->eg_flag)
								  	{
        							  fprintf(cgiOut,"<tr height=30>"\
        							  "<td align=left id=tdprompt style=font-size:12px>%s:</td>","egress dest Port");
        							  fprintf(cgiOut,"<td align=left>%s</td>",rev_mirror_info->destPort_eg);
        							  fprintf(cgiOut,"</tr>");
								  	}
								  
								}
							if(rev_mirror_info->VlanID[0]!=0)
							  	{
							  		for(i=0;i<rev_mirror_info->VlanNum;i++)
							  			{
             							   fprintf(cgiOut,"<tr height=30>"\
             							   "<td align=left id=tdprompt style=font-size:12px>%s:</td>","mirror vlan");
             							   fprintf(cgiOut,"<td align=left>%d</td>",rev_mirror_info->VlanID[i]);
             							   fprintf(cgiOut,"</tr>");
							  			}
							  	}
							if(rev_mirror_info->policyindex[0]!=0)
							  	{
      							   for(i=0;i<rev_mirror_info->policyindexNum;i++)
							  			{
             							   fprintf(cgiOut,"<tr height=30>"\
             							   "<td align=left id=tdprompt style=font-size:12px>%s:</td>","mirror policy");
             							   fprintf(cgiOut,"<td align=left>%u</td>",rev_mirror_info->policyindex[i]);
             							   fprintf(cgiOut,"</tr>");
							  			}
							  	}
							if(strcmp(rev_mirror_info->PortNo[0],"")!=0)
							  	{
							  		for(i=0;i<rev_mirror_info->PortNum;i++)
							  			{
             							   fprintf(cgiOut,"<tr height=30>"\
             							   "<td align=left id=tdprompt style=font-size:12px>%s:</td>","mirror port");
             							   fprintf(cgiOut,"<td align=left>%s(%s)</td>",rev_mirror_info->PortNo[i],rev_mirror_info->Port_mode[i]);
             							   fprintf(cgiOut,"</tr>");
							  			}
							  	}
							if(strcmp(rev_mirror_info->Mac[0],"")!=0)
							  	{
							  		fprintf(cgiOut,"<tr height=30>"\
             							   "<td align=left id=tdprompt style=font-size:12px colspan=2>%s:</td>","FDB mirror");
										   fprintf(cgiOut,"</tr>");
							  		for(i=0;i<rev_mirror_info->fdbNum;i++)
							  			{ 
										   fprintf(cgiOut,"<tr height=30>"\
										   "<td align=left id=tdprompt style=font-size:12px colspan=2>%s:</td>","Mac");
             							   fprintf(cgiOut,"<td align=left style=padding-left:5px>%s</td>",rev_mirror_info->Mac[i]);
             							   fprintf(cgiOut,"</tr>");
										   fprintf(cgiOut,"<tr height=30>"\
										   "<td align=left id=tdprompt style=font-size:12px colspan=2>%s:</td>","Vlan Id");
             							   fprintf(cgiOut,"<td align=left style=padding-left:5px>%d</td>",rev_mirror_info->fdb_vlan[i]);
             							   fprintf(cgiOut,"</tr>");
										   fprintf(cgiOut,"<tr height=30>"\
										   "<td align=left id=tdprompt style=font-size:12px colspan=2>%s:</td>","Port");
             							   fprintf(cgiOut,"<td align=left style=padding-left:5px>%s</td>",rev_mirror_info->fdb_port[i]);
             							   fprintf(cgiOut,"</tr>");
							  			}
							  	}
							fprintf(cgiOut,"<tr>");
							if(cgiFormSubmitClicked("submit_mirror") != cgiFormSuccess)
							  {
								fprintf(cgiOut,"<td colspan=2><input type=hidden name=encry_mirror value=%s></td>",encry);
							  }
							else if(cgiFormSubmitClicked("submit_mirror") == cgiFormSuccess)
							  {
								fprintf(cgiOut,"<td colspan=2><input type=hidden name=encry_mirror value=%s></td>",show_mirror);
							  }
							  fprintf(cgiOut,"</tr>"\
							"</table></div>"\
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
free(rev_mirror_info);
release(lpublic);  
release(lcontrol);
return 0;
}

															 


