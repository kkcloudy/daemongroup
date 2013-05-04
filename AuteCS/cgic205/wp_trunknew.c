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
* wp_trunknew.c
*
*
* CREATOR:
* autelan.software.Network Dep. team
* zhouym@autelan.com
*
* DESCRIPTION:
* system contrl for create trunk 
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
#include "util/npd_list.h"
#include "sysdef/npd_sysdef.h"
#include "npd/nam/npd_amapi.h"
#include "ws_public.h"
#include "ws_trunk.h"

int ShowTrunknewPage(char *m,struct list *lpublic,struct list *lcon);    
void NewTrunk(struct list *lpublic,struct list *lcon);

int cgiMain()
{
  char *encry=(char *)malloc(BUF_LEN); 
  char *str;
  struct list *lpublic;   /*解析public.txt文件的链表头*/
  struct list *lcon;     /*解析wlan.txt文件的链表头*/  
  lpublic=get_chain_head("../htdocs/text/public.txt");
  lcon=get_chain_head("../htdocs/text/control.txt");
  memset(encry,0,BUF_LEN);
  if(cgiFormStringNoNewlines("UN", encry, BUF_LEN)!=cgiFormNotFound )  /*首次进入该页*/
  {
	;
  }
  else
  {
    cgiFormStringNoNewlines("encry_newtrunk",encry,BUF_LEN);
  }
  str=dcryption(encry);
  if(str==NULL)
    ShowErrorPage(search(lpublic,"ill_user"));		 /*用户非法*/
  else
	ShowTrunknewPage(encry,lpublic,lcon);
  free(encry);
  release(lpublic);  
  release(lcon);
  return 0;
}

int ShowTrunknewPage(char *m,struct list *lpublic,struct list *lcon)
{  
  FILE *fp;
  char lan[3];
  int i;
  cgiHeaderContentType("text/html");
  fprintf(cgiOut,"<html xmlns=\"http://www.w3.org/1999/xhtml\"><head>");
  fprintf(cgiOut,"<meta http-equiv=Content-Type content=text/html; charset=gb2312>");
  fprintf(cgiOut,"<title>Wlan</title>");
  fprintf(cgiOut,"<link rel=stylesheet href=/style.css type=text/css>"\
  "</head>"\
  "<body>");
  if(cgiFormSubmitClicked("trunknew_apply") == cgiFormSuccess)
  {
    NewTrunk(lpublic,lcon);
  }
  fprintf(cgiOut,"<form>"\
  "<div align=center>"\
  "<table width=976 border=0 cellpadding=0 cellspacing=0>"\
  "<tr>"\
    "<td width=8 align=left valign=top background=/images/di22.jpg><img src=/images/youce4.jpg width=8 height=30/></td>"\
    "<td width=51 align=left valign=bottom background=/images/di22.jpg><img src=/images/youce33.jpg width=37 height=24/></td>"\
    "<td width=153 align=left valign=bottom id=%s background=/images/di22.jpg>%s</td>",search(lpublic,"title_style"),search(lcon,"trunk_manage"));
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
	
	  fprintf(cgiOut,"<table width=155 border=0 cellspacing=0 cellpadding=0>"\
      "<tr>"\
      "<td width=62 align=center><input id=but type=submit name=trunknew_apply style=background-image:url(/images/%s) value=""></td>",search(lpublic,"img_ok"));
      fprintf(cgiOut,"<td width=62 align=center><a href=wp_contrl.cgi?UN=%s target=mainFrame><img src=/images/%s border=0 width=62 height=20/></a></td>",m,search(lpublic,"img_cancel"));
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
  					"<td align=left id=tdleft><a href=wp_trunklis.cgi?UN=%s target=mainFrame class=top><font id=%s>%s<font></a></td>",m,search(lpublic,"menu_san"),search(lcon,"trunk_list"));                       
                  fprintf(cgiOut,"</tr>"\
  				  "<tr height=26>"\
                    "<td align=left id=tdleft background=/images/bottom_bg.gif style=\"border-right:0\"><font id=%s>%s<font></td>",search(lpublic,"menu_san"),search(lcon,"create_trunk"));   /*突出显示*/
				  fprintf(cgiOut,"</tr>");
                  for(i=0;i<1;i++)
	              {
  				    fprintf(cgiOut,"<tr height=25>"\
                      "<td id=tdleft>&nbsp;</td>"\
                    "</tr>");
	              }
                fprintf(cgiOut,"</table>"\
              "</td>"\
              "<td align=left style=\"background-color:#ffffff; border-right:1px solid #707070; padding-left:30px\">"\
              "<table width=700 border=0 cellspacing=0 cellpadding=0>"\
  "<tr height=30>"\
    "<td width=70>Trunk ID:</td>"\
    "<td width=260 align=left><input type=text name=trunk_id size=40></td>"\
    "<td width=370><font color=red>(1--%d)</font></td>",127);
  fprintf(cgiOut,"</tr>"\
  "<tr height=30>"\
    "<td>Trunk %s:</td>",search(lpublic,"name"));
    fprintf(cgiOut,"<td align=left><input type=text name=trunk_name size=40></td>");

  fprintf(cgiOut,"<td align=left><font color=red>(%s)</font></td>",search(lcon,"trunk_name_des"));
  fprintf(cgiOut,"</tr>"\
  "<tr>"\
    "<td colspan=3><input type=hidden name=encry_newtrunk value=%s></td>",m);
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
return 0;
}


void NewTrunk(struct list *lpublic,struct list *lcon)
{
  int id,ret;  
  char *endptr = NULL;  
  char *ID=(char *)malloc(20);
  char *trunk_name=(char *)malloc(25);
  memset(ID,0,20);
  cgiFormStringNoNewlines("trunk_id",ID,20);   
  memset(trunk_name,0,25);
  cgiFormStringNoNewlines("trunk_name",trunk_name,25);  
  if(strcmp(ID,"")!=0)                             /*trunk id不能为空*/
  {
	id= strtoul(ID,&endptr,10);					   /*char转成int，10代表十进制*/		
	if((id>0)&&(id<128))           /*最多WLAN_NUM个wlan*/   
	{
      if((strcmp(trunk_name,"")!=0)&&(strchr(trunk_name,' ')==NULL))            /*trunk name不能为空*/
      {
        if(strlen(trunk_name)>20)
	      ShowAlert(search(lpublic,"input_too_long"));
		else
		{
 	      ccgi_dbus_init();
          ret=create_trunk(ID,trunk_name);/*返回0表示失败，返回1表示成功，返回-1表示trunk name can  not be list*/
                                          /*返回-2表示trunk name too long，返回-3表示trunk name begins with an illegal char*/
                                          /*返回-4表示trunk name contains illegal char，返回-5表示trunk id illeagal*/
                                          /*返回-6表示trunkID Already Exists，返回-7表示trunkName Already Exists*/
                                          /*返回-8表示error*/
          switch(ret)
          {
            case 0:ShowAlert(search(lpublic,"create_fail"));
    	           break;
            case 1:ShowAlert(search(lpublic,"create_success"));
    	           break;
    		case -1:ShowAlert(search(lcon,"trunk_name_not_be_list"));
    	            break;
    	    case -2:ShowAlert(search(lpublic,"input_too_long"));
    	            break;			
    	    case -3:ShowAlert(search(lcon,"trunk_name_begin_illegal"));
    	            break;
			case -4:ShowAlert(search(lcon,"trunk_name_contain_illegal"));
    	            break;	
		    case -5:ShowAlert(search(lcon,"trunk_id_illegal"));
    	            break;	
			case -6:ShowAlert(search(lcon,"trunk_id_exist"));
    	            break;	
			case -7:ShowAlert(search(lcon,"trunk_name_exist"));
    	            break;	
			case -8:ShowAlert(search(lpublic,"error"));
    	            break;	
          }   
        }
      }
      else
		ShowAlert(search(lcon,"trunk_name_not_null"));
	}
	else
	  ShowAlert(search(lcon,"trunk_id_illegal"));
  }
  else
	ShowAlert(search(lcon,"trunk_id_not_null"));  
  free(ID);
  free(trunk_name);
}

