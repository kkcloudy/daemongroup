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
* wp_prtcfg.c
*
*
* CREATOR:
* autelan.software.Network Dep. team
* zhouym@autelan.com
*
* DESCRIPTION:
* system contrl for port config
*
*
*******************************************************************************/
#include <stdio.h>
#include "cgic.h"
#include <string.h>
#include <stdlib.h>
#include "ws_usrinfo.h"
#include "ws_err.h"
#include <sys/types.h>
#include <unistd.h>
#include "ws_ec.h"
#include "ws_dcli_portconf.h"
#include "ws_dbus_list_interface.h"

#define MTU_MIN 64
#define MTU_MAX 8192
#define MTU_DEFAULT 1522

static char* LANG_PATH_GL = "../htdocs/text/public.txt";
static char* LANG_PATH_LO = "../htdocs/text/control.txt";
static char* DECV_7000 = "Switch7000";

static struct list *lpublic = NULL;
static struct list *llocal = NULL;

typedef enum DECEIVE_NAME{
    UNKNOWN,
    SWITCH_5000,
    SWITCH_7000
}DECV;

//----------------------------------

static void ShowAlert_plus(char *str_option, char *message)
{
  fprintf(cgiOut,"<SCRIPT  LANGUAGE=JavaScript>"\
  "alert(\"%s:\"+\" \"+\"%s\");",str_option,message);
  fprintf(cgiOut,"</SCRIPT>");
}

static void ShowRes(char *str_option, int res)
{
	switch(res)
	{
		
		case WS_SUCCESS:
	    {
		    ShowAlert_plus(str_option,search(lpublic,"config_sucess"));
			break;
		}
		case WS_NOT_SUPPORT:
		{
			ShowAlert_plus(str_option,search(llocal,"not_support"));
			break;
		}
		case WS_NO_SUCH_PORT:
		{
			ShowAlert_plus(str_option,search(lpublic,"interface_not_exist"));
			break;
		}
		case WS_EXEC_COMM_FAIL:
		{
		    ShowAlert_plus(str_option,search(llocal,"execute_fail")); 
			break;
		}
		case WS_ADMIN_DIS:
		{
		    ShowAlert_plus(str_option,search(llocal,"admin_dis")); 
			break;
		}
		case WS_ERR_UNKNOW:
		{
		    ShowAlert_plus(str_option,search(llocal,"interfac_err_know")); 
			break;
		}
		case WS_DEL_FROM_VLAN_FIRST:
		{
		    ShowAlert_plus(str_option,search(llocal,"interfac_err_del_from_vlan")); 
			break;
		}
		/*
		case WS_BAD_VALUE:
		{
		    ShowAlert_plus(str_option,search(llocal,"execute_fail"));//必须为偶数
			break;
		}
		case WS_OUT_RANGE:
		{
		    ShowAlert_plus(str_option,search(llocal,"mtu_value_err")); //超出范围
			break;
		}
       */
			
		default:
		{
			ShowAlert_plus(str_option,search(lpublic,"oper_fail"));			
			break;
		}
	}
}

//----------------------------------

static void redirection(char *str_port_no)
{

   fprintf(cgiOut,
   	"<script language=javascript>\n"\
       "var str_clean = document.location.href;\n"\
       "var index = str_clean.indexOf(\"&\");\n"\
       "str_clean = str_clean.substr(0,index);\n"\
       "str_clean += \"&\" + \"port_no=\" + \"%s\";\n"\
       "document.location.href = str_clean;\n"\
   "</script>",str_port_no);
}

int ShowPortConfPage(char *prtno); 
void port_config(char *def_portno); 
int config_buffer_module();	
int cgiMain()
{
  char *init_port=(char *)malloc(20);
  char *prt_no=(char *)malloc(10);
  ETH_SLOT_LIST  head,*p;
  ETH_PORT_LIST *pp;
  int num,ret=-1;
  memset(init_port,0,20);
  memset(prt_no,0,10);
  if(cgiFormStringNoNewlines("ID", prt_no, 10)!=cgiFormNotFound )  
    ShowPortConfPage(prt_no);
  else
  {
    ccgi_dbus_init();
  	ret=show_ethport_list(&head,&num);
	p=head.next;
	if(p!=NULL)
	{
		while(p!=NULL)
		{
			pp=p->port.next;
			while(pp!=NULL)
			{
				memset(init_port,0,10);							
				sprintf(init_port,"%d-%d",p->slot_no,pp->port_no);		 /*int转成char*/
				pp=pp->next;
				break;
			}
			p=p->next;
       }
	}
	ShowPortConfPage(init_port);
  }
  free(init_port);
  free(prt_no);
  if((ret==0)&&(num>0))
  {
	  Free_ethslot_head(&head);
  }
  return 0;
}

int ShowPortConfPage(char *prtno)
{ 
  unsigned int un_portseq = 0;
  char *str_encry =(char *)malloc(BUF_LEN);				
  char *str_uid = NULL; 
  char str_port_no[32];
  char str_decv_name[32];
  unsigned int un_media_flag = 0;
  memset(str_encry,0,BUF_LEN);
  memset(str_port_no,0,sizeof(str_port_no));
  memset(str_decv_name, 0, sizeof(str_decv_name));

  lpublic = get_chain_head(LANG_PATH_GL);
  llocal = get_chain_head(LANG_PATH_LO);
  if((NULL == lpublic) || (NULL == llocal))
  {
      ShowAlert(search(lpublic, "error_open"));
  }

  struct eth_port_s pr;
  struct global_ethport_s PortV;
  struct slot sr;
  ETH_SLOT_LIST  head,*p;
  ETH_PORT_LIST *pp;
  int num,ret;
  unsigned int value = 0;

  PortV.attr_bitmap=0;
  PortV.mtu=0;
  PortV.port_type=0;
  sr.module_status=0; 	
  sr.modname=(char *)malloc(20);	   
  sr.sn=(char *)malloc(20);		   
  sr.hw_ver=0;
  sr.ext_slot_num=0; 
  
  ccgi_dbus_init();        //初始化dbus
  
  if(cgiFormSubmitClicked("port_conf") != cgiFormSuccess)
  {
    memset(str_encry,0,BUF_LEN);
    cgiFormStringNoNewlines("UN", str_encry, BUF_LEN); 
    
    str_uid = dcryption(str_encry);
    if(NULL == str_uid)
    {   
      ShowErrorPage(search(lpublic,"ill_user")); 	     
      return 0;
	}
  }

  if(cgiFormStringNoNewlines("port_no", str_port_no, BUF_LEN)!= cgiFormSuccess)
  {
      strcpy(str_port_no,prtno);
  }

  cgiHeaderContentType("text/html");
  fprintf(cgiOut,"<html xmlns=\"http://www.w3.org/1999/xhtml\"><head>");
  fprintf(cgiOut,"<meta http-equiv=Content-Type content=text/html; charset=gb2312>");
  fprintf(cgiOut,"<title>%s</title>",search(llocal,"prt_manage"));
  fprintf(cgiOut,"<link rel=stylesheet href=/style.css type=text/css>\n"\
  	"<style type=text/css>\n"\
  	".a3{width:30;border:0; text-align:center}\n"\
  	"</style>\n"\
  "</head>\n"\
  "<body onload=oninitialize()>"); 
  if(cgiFormSubmitClicked("port_conf") == cgiFormSuccess)
  {
  	  //cgiFormStringNoNewlines("port_no", str_port_no, 32);
	  port_config(prtno);
	  release(llocal);
      release(lpublic);
      free(str_encry);
      return 0;
  }
  fprintf(cgiOut,"<form method=post name=port_config encType=multipart/form-data onsubmit=\"return mysubmit()\">\n"\
  "<div align=center>\n"\
  "<table width=976 border=0 cellpadding=0 cellspacing=0>\n"\
  "<tr>\n"\
    "<td width=8 align=left valign=top background=/images/di22.jpg><img src=/images/youce4.jpg width=8 height=30/></td>\n"\
    "<td width=51 align=left valign=bottom background=/images/di22.jpg><img src=/images/youce33.jpg width=37 height=24/></td>\n"\
    "<td width=153 align=left valign=bottom id=%s background=/images/di22.jpg>%s</td>",search(lpublic,"title_style"),search(llocal,"prt_manage"));
    fprintf(cgiOut,"<td width=690 align=right valign=bottom background=/images/di22.jpg>");

  {   
	fprintf(cgiOut,"<table width=131 border=0 cellspacing=0 cellpadding=0><tr>\n"\
	"<td width=62 align=center><input id=but type=submit name=port_conf style=background-image:url(/images/%s) value=""></td>", search(lpublic,"img_ok")); 		
	  fprintf(cgiOut,"<td width=62 align=left><a href=wp_contrl.cgi?UN=%s target=mainFrame><img src=/images/%s border=0 width=62 height=20/></a></td>",str_encry, search(lpublic,"img_cancel"));
	fprintf(cgiOut,"</tr>\n"\
	"</table>");
  } 	  
	fprintf(cgiOut,"</td>\n"\
    "<td width=74 align=right valign=top background=/images/di22.jpg><img src=/images/youce3.jpg width=31 height=30/></td>\n"\
  "</tr>\n"\
  "<tr>\n"\
    "<td colspan=5 align=center valign=middle><table width=976 border=0 cellpadding=0 cellspacing=0 bgcolor=#f0eff0>\n"\
      "<tr>\n"\
        "<td width=12 align=left valign=top background=/images/di888.jpg>&nbsp;</td>\n"\
        "<td width=948><table width=947 border=0 cellspacing=0 cellpadding=0>\n"\
            "<tr height=4 valign=bottom>\n"\
              "<td width=120>&nbsp;</td>\n"\
              "<td width=827 valign=bottom><img src=/images/bottom_05.gif width=827 height=4/></td>\n"\
            "</tr>\n"\
            "<tr>\n"\
              "<td><table width=120 border=0 cellspacing=0 cellpadding=0>\n"\
                   "<tr height=25>\n"\
                    "<td id=tdleft>&nbsp;</td>\n"\
                  "</tr>");
  fprintf(cgiOut,"<tr height=25>\n"\
 					"<td align=left id=tdleft>\n"\
 					    "<a href=wp_prtsur.cgi?UN=%s\n"\
 					      " target=mainFrame\n"\
 					      " class=top\n"\
 					      "><font id=%s>%s<font></a>\n"\
 					"</td>\n"\
  			     "</tr>",str_encry,search(lpublic,"menu_san"),search(llocal,"prt_sur"));
  
  fprintf(cgiOut,"<tr height=25>\n"\
	  "<td align=left id=tdleft><a href=wp_prtarp.cgi?UN=%s target=mainFrame class=top><font id=yingwen_san>ARP<font><font id=%s> %s<font></a></td></tr>",str_encry,search(lpublic,"menu_san"),search(lpublic,"survey")); 					  
  fprintf(cgiOut,"<tr height=25>"\
	"<td align=left id=tdleft><a href=wp_static_arp.cgi?UN=%s target=mainFrame class=top><font id=%s>%s<font></a></td></tr>",str_encry,search(lpublic,"menu_san"),search(llocal,"prt_static_arp"));					   
  fprintf(cgiOut,"\n"\
  "<tr height=26>\n"\
	  "<td align=left id=tdleft background=/images/bottom_bg.gif style=\"border-right:0\"><font id=%s>%s<font></td>",search(lpublic,"menu_san"),search(llocal,"prt_cfg"));    
  fprintf(cgiOut,"</tr>\n"\
  "<tr height=25>\n"\
      "<td align=left id=tdleft><a href=wp_prtfuncfg.cgi?UN=%s target=mainFrame class=top><font id=%s>%s<font></a></td>",str_encry,search(lpublic,"menu_san"),search(llocal,"func_cfg"));                       
  fprintf(cgiOut,"</tr>");                    
					//add by sjw  2008-10-9 17:14:37  for  subinterface
					fprintf(cgiOut,"<tr height=25>\n"\
  					    "<td align=left id=tdleft><a href=wp_subintf.cgi?UN=%s target=mainFrame class=top><font id=%s>%s<font></a></td></tr>",str_encry,search(lpublic,"menu_san"),search(llocal,"title_subintf"));  	
					fprintf(cgiOut,"<tr height=25>"\
						  "<td align=left id=tdleft><a href=wp_interface_bindip.cgi?UN=%s target=mainFrame class=top><font id=%s>%s<font></a></td></tr>",str_encry,search(lpublic,"menu_san"),search(lpublic,"config_interface"));
					fprintf(cgiOut,"<tr height=25>"\
  					    "<td align=left id=tdleft><a href=wp_all_interface.cgi?UN=%s target=mainFrame class=top><font id=%s>%s<font></a></td></tr>",str_encry,search(lpublic,"menu_san"),search(llocal,"interface")); 						
				

   int i=0;
   for(;i<9;i++)
   {
     fprintf(cgiOut,"<tr height=25>\n"\
   	"<td id=tdleft>&nbsp;</td>\n"\
     "</tr>");
   }
   fprintf(cgiOut, "</table></td>");

   fprintf(cgiOut, 
    "<script language=javascript>\n"\
    "var eth_port_type_str = new Array(\n"\
		"\"ETH_INVALID\",\n"\
		"\"ETH_FE_TX\",\n"\
		"\"ETH_FE_FIBER\",\n"\
		"\"ETH_GTX\",\n"\
		"\"ETH_GE_FIBER\",\n"\
		"\"ETH_GE_SFP\",\n"\
		"\"ETH_XGE_XFP\",\n"\
		"\"ETH_XGTX\",\n"\
		"\"ETH_XGE_FIBER\",\n"\
		"null\n"\
	");\n"\
	"var onoff_status_str = new Array(\n"\
		"\"OFF\",\n"\
		"\"ON\",\n"\
		"null\n"\
	");\n"\
	"var link_status_str = new Array(\n"\
		"\"DOWN\",\n"\
		"\"UP\",\n"\
		"\"AUTO\",\n"\
		"null\n"\
	");\n"\
	"var duplex_status_str = new Array(\n"\
		"\"HALF\",\n"\
		"\"FULL\",\n"\
		"null\n"\
	");\n"\
	"var port_media_str = new Array(\n"\
		"\"NONE\",\n"\
		"\"FIBER\",\n"\
		"\"COPPER\",\n"\
		"null\n"\
	");\n"\
	"var port_speed_str = new Array(\n"\
		"\"10M\",\n"\
		"\"100M\",\n"\
		"\"1000M\",\n"\
		/*"10G,"    not support now
		"12G,"
		"2.5G,"
		"5G"*/
		"null\n"\
	");\n"\
	"var str_sUrl = document.location.href;\n"\
	"var str_oUrl = \"\";\n"\
	"var bl_SendFlag = 0;\n"\
	"function PortInfo()\n"\
	"{\n"\
		"this.port_name;\n"\
		"this.port_type;\n"\
		"this.admin_status;\n"\
		"this.link_status;\n"\
		"this.auto_nage;\n"\
		"this.auto_speed;\n"\
		"this.auto_dup;\n"\
		"this.auto_flowctl;\n"\
		"this.port_dup;\n"\
		"this.flow_ctl;\n"\
		"this.back_pre;\n"\
		"this.port_speed;\n"\
		"this.port_media;\n"\
		"this.port_mtu;\n"\
	"};\n"\
	"var port_list = new Array();");


 	ret=show_ethport_list(&head,&num);
	p=head.next;
	if(p!=NULL)
	{
		while(p!=NULL)
		{
			pp=p->port.next;
			while(pp!=NULL)
			{
				unsigned char type = 0;
				if ( 1 != distributed_flag) {
					value = p->slot_no; 			  
					value =  (value << 8) |pp->port_no; 
					type = 0;
				}
				else {
					value = (p->slot_no - 1)* 64 + pp->port_no - 1;
					type = 1;
				}
				if(show_eth_port_atrr(value,type,&PortV)==CCGI_SUCCESS)	  //读取端口信息成功
				{
					fprintf(cgiOut,"port_list[%d] = new PortInfo;",un_portseq);
					fprintf(cgiOut,"port_list[%d].port_name = \"%d-%d\";",un_portseq,p->slot_no,pp->port_no);
					fprintf(cgiOut,"port_list[%d].port_type = \"%s\";",un_portseq,eth_port_type_str[PortV.port_type]);
					fprintf(stderr,"port_name = %d-%d,pr.port_type=%d,port_type =%s\n",p->slot_no,pp->port_no,PortV.port_type,eth_port_type_str[PortV.port_type]);
					fprintf(cgiOut,"port_list[%d].admin_statue = \"%s\";",un_portseq,strcmp(onoff_status_str[(PortV.attr_bitmap & ETH_ATTR_ADMIN_STATUS) >> ETH_ADMIN_STATUS_BIT],"ON")==0?"ON":"OFF");
					fprintf(cgiOut,"port_list[%d].link_status = \"%s\";",un_portseq,link_status_str[(PortV.attr_bitmap & ETH_ATTR_LINK_STATUS) >> ETH_LINK_STATUS_BIT]);
					fprintf(cgiOut,"port_list[%d].auto_nage = \"%s\";",un_portseq,doneOrnot_status_str[(PortV.attr_bitmap & ETH_ATTR_AUTONEG) >> ETH_AUTONEG_BIT]);
					fprintf(cgiOut,"port_list[%d].auto_speed = \"%s\";",un_portseq,strcmp(onoff_status_str[(PortV.attr_bitmap & ETH_ATTR_AUTONEG_SPEED) >> ETH_AUTONEG_SPEED_BIT],"ON")==0?"ON":"OFF");
					fprintf(cgiOut,"port_list[%d].auto_dup = \"%s\";",un_portseq,strcmp(onoff_status_str[(PortV.attr_bitmap & ETH_ATTR_AUTONEG_DUPLEX) >> ETH_AUTONEG_DUPLEX_BIT],"ON")==0?"ON":"OFF");
					fprintf(cgiOut,"port_list[%d].auto_flowctl = \"%s\";",un_portseq,strcmp(onoff_status_str[(PortV.attr_bitmap & ETH_ATTR_AUTONEG_FLOWCTRL) >> ETH_AUTONEG_FLOWCTRL_BIT],"ON")==0?"ON":"OFF");
					fprintf(cgiOut,"port_list[%d].port_dup = \"%s\";",un_portseq,duplex_status_str[(PortV.attr_bitmap & ETH_ATTR_DUPLEX) >> ETH_DUPLEX_BIT]);
					fprintf(cgiOut,"port_list[%d].flow_ctl = \"%s\";",un_portseq,strcmp(onoff_status_str[(PortV.attr_bitmap & ETH_ATTR_FLOWCTRL) >> ETH_FLOWCTRL_BIT],"ON")==0?"ON":"OFF");
					fprintf(cgiOut,"port_list[%d].back_pre = \"%s\";",un_portseq,strcmp(onoff_status_str[(PortV.attr_bitmap & ETH_ATTR_BACKPRESSURE) >> ETH_BACKPRESSURE_BIT],"ON")==0?"ON":"OFF");
					fprintf(cgiOut,"port_list[%d].port_speed = \"%s\";",un_portseq,eth_speed_str[(PortV.attr_bitmap & ETH_ATTR_SPEED_MASK) >> ETH_SPEED_BIT]);
				
					un_media_flag = 0;
					un_media_flag |= (((PortV.attr_bitmap & ETH_ATTR_PREFERRED_COPPER_MEDIA) >> ETH_PREFERRED_COPPER_MEDIA_BIT)<< 1);
					un_media_flag |= (PortV.attr_bitmap & ETH_ATTR_PREFERRED_FIBER_MEDIA) >> ETH_PREFERRED_FIBER_MEDIA_BIT;				 
					fprintf(cgiOut,"port_list[%d].port_media = \"%s\";",un_portseq,eth_media_str[un_media_flag]);
				
					fprintf(cgiOut,"port_list[%d].port_mtu = %d;",un_portseq++,PortV.mtu);
				}			 
				else
				{
					fprintf(cgiOut,"port_list[%d] = new PortInfo;",un_portseq);
					fprintf(cgiOut,"port_list[%d].port_name = \"%d-%d\";",un_portseq,p->slot_no,pp->port_no);
					fprintf(cgiOut,"port_list[%d].port_type = \"%s\";",un_portseq,"");
					fprintf(cgiOut,"port_list[%d].admin_statue = \"%s\";",un_portseq,"");
					fprintf(cgiOut,"port_list[%d].link_status = \"%s\";",un_portseq,"");
					fprintf(cgiOut,"port_list[%d].auto_nage = \"%s\";",un_portseq,"");
					fprintf(cgiOut,"port_list[%d].auto_speed = \"%s\";",un_portseq,"");
					fprintf(cgiOut,"port_list[%d].auto_dup = \"%s\";",un_portseq,"");
					fprintf(cgiOut,"port_list[%d].auto_flowctl = \"%s\";",un_portseq,"");
					fprintf(cgiOut,"port_list[%d].port_dup = \"%s\";",un_portseq,"");
					fprintf(cgiOut,"port_list[%d].flow_ctl = \"%s\";",un_portseq,"");
					fprintf(cgiOut,"port_list[%d].back_pre = \"%s\";",un_portseq,"");
					fprintf(cgiOut,"port_list[%d].port_speed = \"%s\";",un_portseq,"");
					fprintf(cgiOut,"port_list[%d].port_media = \"%s\";",un_portseq,"");
					fprintf(cgiOut,"port_list[%d].port_mtu = %d;",un_portseq++,0);
				}
				pp=pp->next;
			}
			p=p->next;
		}
	 }
 
   if(str_decv_name[0] == 0)
   {
       strcpy(str_decv_name, DECV_7000);
   }
   fprintf(cgiOut,"port_list[%d] = null;\n"\
   	              "function port_select(obj)\n"\
   	              "{\n"\
                     "var str_clean = str_sUrl;\n"\
                     "var length = str_clean.indexOf(\"&\");\n"\
                     "if(length > 0)\n"\
                     "{\n"\
                        "str_sUrl = str_clean.substr(0,length);\n"\
                     "}\n"\
   	                 "obj.selectedIndex = obj.selectedIndex>=0?obj.selectedIndex:0;\n"\
   	                 "var index = obj.selectedIndex;\n"\
   	                 "document.port_config.port_type.value = port_list[index].port_type;\n"\
   	                 "document.port_config.admin_status.value = port_list[index].admin_statue;\n"\
   	                 "document.port_config.link_status.value = port_list[index].link_status;\n"\
   	                 "document.port_config.an_spe.value = port_list[index].auto_speed;\n"\
   	                 "document.port_config.an_dup.value = port_list[index].auto_dup;\n"\
   	                 "document.port_config.an_flowctrl.value = port_list[index].auto_flowctl;\n"\
   	                 "document.port_config.duplex.value = port_list[index].port_dup;\n"\
   	                 "document.port_config.flow_ctrl.value = port_list[index].flow_ctl;\n"\
   	                 "document.port_config.back_pres.value = port_list[index].back_pre;\n"\
   	                 "document.port_config.speed.value = port_list[index].port_speed;\n"\
   	                 "document.port_config.media.value = port_list[index].port_media;\n"\
   	                 "document.port_config.mtu.value = port_list[index].port_mtu;\n"\
                     "bl_SendFlag = -1;\n"\
                     "port_conf_change(obj);\n"\
                     "str_oUrl = str_sUrl;\n"\
   	              "}\n"\
   	              "function port_default()\n"\
   	              "{\n"\
   	                 "var str_clean = str_oUrl;\n"\
                     "str_clean += \"&default=\"+\"\";\n"\
                     "str_clean += \"&port_conf=\"+\"\";\n"\
                     "document.location.replace(str_clean);\n"\
   	              "}\n"\
   	              "function remove_attr(str_aname)\n"\
   	              "{\n"\
   	                 "if(str_aname == null)\n"\
   	                 "{\n"\
   	                    "return;\n"\
   	                 "}\n"\
   	                 "var rex_para = eval(\"/&\"+str_aname+\"=[\\\\w-]+/\");\n"\
   	                 "var str_temp = str_sUrl.replace(rex_para,\"\");\n"\
   	                 "str_sUrl = str_temp;\n"\
   	              "}\n"\
   	              "function mysubmit()\n"\
   	              "{\n"\
	                 "var selObj_portmtu = document.port_config.mtu;\n"\
	                 "if(!check_mtu(selObj_portmtu.value))\n"\
	                 "{\n"\
	                    "return false;\n"\
	                 "}\n"\
	                 "if(bl_SendFlag > 0)\n"\
	                 "{\n"\
	                     "if(str_sUrl.indexOf(\"an_spe=ON\") != -1)\n"\
	                     "{\n"\
	                        "remove_attr(\"speed\");\n"\
	                     "}\n"\
	                     "if(str_sUrl.indexOf(\"an_dup=ON\") != -1)\n"\
	                     "{\n"\
	                        "remove_attr(\"duplex\");\n"\
	                     "}\n"\
	                     "if(str_sUrl.indexOf(\"an_flowctrl=ON\") != -1)\n"\
	                     "{\n"\
	                        "remove_attr(\"flow_ctrl\");\n"\
	                     "}\n"\
	                     "if(str_sUrl.indexOf(\"duplex=HALF\") != -1)\n"\
	                     "{\n"\
	                        "remove_attr(\"flow_ctrl\");\n"\
	                     "}\n"\
	                     "if(str_sUrl.indexOf(\"duplex=FULL\") != -1)\n"\
	                     "{\n"\
	                        "remove_attr(\"back_pres\");\n"\
	                     "}\n"\
	                     "if(str_sUrl.indexOf(\"speed=1000M\") != -1)\n"\
	                     "{\n"\
	                        "str_sUrl += \"&duplex=FULL\";\n"\
	                     "}\n"\
    	                 "document.port_config.action = str_sUrl;\n"\
    	                 "str_sUrl = str_sUrl + \"&port_conf=\"+\"\";\n"\
    	                 "str_sUrl = str_sUrl + \"&buffer_mode_value=\"+document.getElementsByName('buffer_mode_value')[0].value;\n"\
    	                 "document.location.replace(str_sUrl);\n"\
    	             "}\n"\
	                 "return false;\n"\
   	              "}\n"\
   	              "function check_mtu(value)\n"\
   	              "{\n"\
   	                 "if(value == \"\")\n"\
   	                 "{\n"\
   	                    "value = \"%d\";\n"\
   	                    "return true;\n"\
   	                 "}\n"\
   	                 "var pattern = /^[1-9][0-9]*$/;\n"\
                     "if( !pattern.test(value) )\n"\
					 "{\n"\
					 	"alert(\"%s\");\n"\
					    "return false;\n"\
                     "}\n"\
					 "if( parseInt(value)<%d || parseInt(value)>%d)\n"\
                     "{\n"\
                     	"alert(\"%s\" + \"%d - %d\");\n"\
					    "return false;\n"\
                     "}\n"\
                     "if( parseInt(value)%%2 != 0)\n"\
                     "{\n"\
                     	"alert(\"%s\");\n"\
					    "return false;\n"\
                     "}\n"\
					 "return true;\n"\
   	              "}\n"\
   	              "function check_autoneg(obj)\n"\
   	              "{\n"\
   	                 "if(obj.name==\"an_spe\")\n"\
   	                 "{\n"\
                        "document.port_config.speed.disabled = (obj.value==\"ON\")?true:false;\n"\
   	                 "}\n"\
   	                 "if(obj.name==\"an_dup\")\n"\
   	                 "{\n"\
                        "document.port_config.duplex.disabled = (obj.value==\"ON\")?true:false;\n"\
   	                 "}\n"\
   	                 "if(obj.name==\"an_flowctrl\")\n"\
   	                 "{\n"\
   	                    "if(obj.value==\"OFF\" && document.port_config.duplex.value ==\"FULL\")\n"\
   	                    "{\n"\
                           "document.port_config.flow_ctrl.disabled = false;\n"\
                        "}\n"\
                        "else\n"\
                        "{\n"\
                           "document.port_config.flow_ctrl.disabled = true;\n"\
                        "}\n"\
   	                 "}\n"\
   	              "}\n"\
				  "function port_conf_change(obj)\n"\
   	              "{\n"\
     				 "var str_temp1 = str_sUrl;\n"\
     				 "var str_temp2 = \"\";\n"\
     				 "var str_ObjPara = obj.name+\"=\"+obj.value;\n"\
     				 "var rex_para_mid = eval(\"/\"+obj.name+\"=[\\\\w-]+&/\");\n"\
     				 "var rex_para_end = eval(\"/\"+obj.name+\"=[\\\\w-]+$/\");\n"\
     				 "if(str_temp1.indexOf(obj.name) != -1)\n"\
     				 "{\n"\
     					"str_temp2 = str_temp1.replace(rex_para_mid,str_ObjPara+\"&\");\n"\
     					"str_temp1 = str_temp2.replace(rex_para_end,str_ObjPara);\n"\
     				 "}\n"\
     				 "else\n"\
     				 "{\n"\
     				    "str_temp1 = str_temp1 +\"&\"+str_ObjPara;\n"\
                     "}\n"\
     				 "str_sUrl = str_temp1;\n"\
     				 "bl_SendFlag++;\n"\
     				 /*
   				 	"document.port_config.speed.disabled = (document.port_config.an_spe.value=='ON')?true:false;\n"
					"document.port_config.duplex.disabled = ((document.port_config.an_dup.value=='ON')||(document.port_config.speed.value=='1000M'))?true:false;\n"\
					"document.port_config.flow_ctrl.disabled = ((document.port_config.an_flowctrl.value=='ON')||(document.port_config.duplex.value=='HALF'))?true:false;\n"\
					"document.port_config.back_pres.disabled = ((document.port_config.an_flowctrl.value=='ON')||(document.port_config.duplex.value=='FULL'))?true:false;\n"\
					*/
				  "document.port_config.speed.disabled = false;\n"
					"document.port_config.duplex.disabled = false;\n"\
					"document.port_config.flow_ctrl.disabled = false;\n"\
					"document.port_config.back_pres.disabled = false;\n"\
				"}\n"\
   	              "function oninitialize()\n"\
                  "{\n"\
                     "var i=0;\n"\
                     "var selObj_portno = document.port_config.port_no;\n"\
                     "var selObj_porttype = document.port_config.port_type;\n"\
                     "selObj_porttype.disabled = true;\n"\
                     "var selObj_portas = document.port_config.admin_status;\n"\
                     "var selObj_portls = document.port_config.link_status;\n"\
                     "var selObj_portasp = document.port_config.an_spe;\n"\
                     "var selObj_portadu = document.port_config.an_dup;\n"\
                     "var selObj_portafc = document.port_config.an_flowctrl;\n"\
                     "var selObj_portdu = document.port_config.duplex;\n"\
                     "var selObj_portfc = document.port_config.flow_ctrl;\n"\
                     "var selObj_portbp = document.port_config.back_pres;\n"\
                     "var selObj_portsp = document.port_config.speed;\n"\
                     "var selObj_portmtu = document.port_config.mtu;\n"\
                     "var selObj_portmedia = document.port_config.media;\n"\
                     "while(port_list[i] != null)\n"\
				     "{\n"\
				        "selObj_portno.options[selObj_portno.length] = new Option(port_list[i].port_name, port_list[i++].port_name);\n"\
                     "}\n"\
                     "var i=0;\n"\
                     "while(eth_port_type_str[i] != null)\n"\
                     "{\n"\
                        "selObj_porttype.options[selObj_porttype.length] = new Option(eth_port_type_str[i], eth_port_type_str[i++]);\n"\
                     "}\n"\
  				     "i=0;\n"\
  				     "while(onoff_status_str[i] != null)\n"\
  				     "{\n"\
  					    "selObj_portas.options[selObj_portas.length] = new Option(onoff_status_str[i], onoff_status_str[i]);\n"\
  					    "selObj_portasp.options[selObj_portasp.length] = new Option(onoff_status_str[i], onoff_status_str[i]);\n"\
  					    "selObj_portadu.options[selObj_portadu.length] = new Option(onoff_status_str[i], onoff_status_str[i]);\n"\
  					    "selObj_portafc.options[selObj_portafc.length] = new Option(onoff_status_str[i], onoff_status_str[i]);\n"\
  					    "selObj_portbp.options[selObj_portbp.length] = new Option(onoff_status_str[i], onoff_status_str[i]);\n"\
  					    "selObj_portfc.options[selObj_portfc.length] = new Option(onoff_status_str[i], onoff_status_str[i++]);\n"\
  				     "}\n"\
	           	     "i=0;\n"\
	           	     "while(link_status_str[i] != null)\n"\
	           	     "{\n"\
	           		    "selObj_portls.options[selObj_portls.length] = new Option(link_status_str[i], link_status_str[i++]);\n"\
	           	     "}\n"\
	           	     "i=0;\n"\
	           	     "while(duplex_status_str[i] != null)\n"\
	           	     "{\n"\
	           		    "selObj_portdu.options[selObj_portdu.length] = new Option(duplex_status_str[i], duplex_status_str[i++]);\n"\
	           	     "}\n"\
	           	     "i=0;\n"\
	           	     "while(port_speed_str[i] != null)\n"\
	           	     "{\n"\
	           		    "selObj_portsp.options[selObj_portsp.length] = new Option(port_speed_str[i], port_speed_str[i++]);\n"\
	           	     "}\n"\
	           	     "i=0;\n"\
	           	     "while(port_media_str[i] != null)\n"\
	           	     "{\n"\
	           	        "selObj_portmedia.options[selObj_portmedia.length] = new Option(port_media_str[i], port_media_str[i++]);\n"\
	           	     "}\n"\
	           	     "selObj_portno.value = \"%s\";\n"\
	           	     "port_select(selObj_portno);\n"\
                  "}\n"\
   	              "</script>",un_portseq,MTU_DEFAULT,search(llocal,"mtu_need_int"),MTU_MIN,MTU_MAX,search(llocal,"mtu_value_err"),MTU_MIN,MTU_MAX,search(llocal,"mtu_not_even"),str_port_no);//错误字符串必须改为ser_var()

   free(sr.modname);
   free(sr.sn); 

 //display port config option 
 fprintf(cgiOut, "<td align=left style=\"background-color:#ffffff; border-right:1px solid #707070; padding-left:30px\">\n"\
	  	           "<table border=0 cellspacing=0 cellpadding=0>");

        //port number
        fprintf(cgiOut, "<tr height=30>\n"\
	  			       "<td align=left id=tdprompt>%s:</td>",search(llocal,"port_no"));
		fprintf(cgiOut, "<td colspan=2 width=150>\n"\
		 	               "<select name=\"port_no\" style=\"width:100px\" onchange='port_select(this);'>\n"\
	  				       "</select>\n"\
	  				     "</td>\n"\
	  		         "");
		
		 //interface_state    add by liuyu 2011-5-9

		fprintf(cgiOut,"<tr height=30>\n"\
  			           "<td align=left id=tdprompt>%s:</td>",search(llocal,"port_inf_state"));
		fprintf(cgiOut, "<td colspan=2 width=150>\n"\
						  "<select name=\"interface_state\" style=\"width:100px\"  onchange='port_conf_change(this);'>\n"\
						    "<option value="">%s"\
						     "<option value=%s>%s"\
						     "<option value=%s>%s"\
						  "<option value=%s>%s"\
						  "</select>\n"\	
						"</td>\n"\
					   "","","UP","UP","DOWN","DOWN","NO_INTERFACE",search(llocal,"port_no_interface"));		

	     //default
		fprintf(cgiOut, "<td colspan=2>\n"\
						  "<input type=button value=%s onclick=port_default()>\n"\
						"</td>\n"\
					   "</tr>",search(llocal,"default"));

		 //port type
        fprintf(cgiOut,"<tr height=30>\n"\
  			           "<td align=left id=tdprompt>%s:</td>",search(llocal,"port_type"));
		fprintf(cgiOut, "<td colspan=2>\n"\
						  "<select name=\"port_type\" style=\"width:100px\" onchange='port_conf_change(this);'>\n"\
						  "</select>\n"\
						"</td>\n"\
					   "</tr>");
         //port admin status

        fprintf(cgiOut,"<tr height=30>\n"\
  			           "<td align=left id=tdprompt>%s:</td>",search(llocal,"admin_status"));
		fprintf(cgiOut, "<td colspan=2 width=150>\n"\
						  "<select name=\"admin_status\" style=\"width:100px\" onchange='port_conf_change(this);'>\n"\
						  "</select>\n"\
						"</td>\n"\
					   "");
		
		//自协商总开关
		fprintf(cgiOut,"<tr height=30>\n"\
  			           "<td align=left id=tdprompt>%s:</td>",search(llocal,"auto_status"));
		fprintf(cgiOut, "<td colspan=2 width=150>\n"\
						  "<select name=\"auto_status\" style=\"width:100px\"  onchange='port_conf_change(this);'>\n"\
						    "<option value="">%s"\
						     "<option value=%s>%s"\
						  "<option value=%s>%s"\
						  "</select>\n"\	
						"</td>\n"\
					   "",search(llocal,"auto_switch"),"ON","ON","OFF","OFF");
		
	     //port link status
        fprintf(cgiOut,"\n"\
  			           "<td align=left id=tdprompt>%s:</td>",search(llocal,"link_status"));
		fprintf(cgiOut, "<td colspan=2>\n"\
						  "<select name=\"link_status\" style=\"width:60px\" disabled onchange='port_conf_change(this);'>\n"\
						  "</select>\n"\
						"</td>\n"\
					   "</tr>");
		
		 //port  auto speed
        fprintf(cgiOut,"<tr height=30>\n"\
  			           "<td align=left id=tdprompt>%s:</td>",search(llocal,"an_spe"));
		fprintf(cgiOut, "<td colspan=2>\n"\
						  "<select name=\"an_spe\" disabled style=\"width:100px\" onchange='port_conf_change(this);'>\n"\
						  "</select>\n"\
						"</td>\n"\
					   "");
		
        search(llocal,"an_dup");
			
		 //port speed
		 fprintf(cgiOut,"\n"\
						"<td align=left id=tdprompt>%s:</td>",search(llocal,"speed"));
		 fprintf(cgiOut, "<td colspan=2 width=150>\n"\
						   "<select name=\"speed\" style=\"width:60px\" onchange='port_conf_change(this);'>\n"\
						   "</select>\n"\
						 "</td>\n"\
						"</tr>");

		 //port auto duplex
        fprintf(cgiOut,"<tr height=30>\n"\
  			           "<td align=left id=tdprompt>%s:</td>",search(llocal,"an_dup"));
		fprintf(cgiOut, "<td colspan=2>\n"\
						  "<select name=\"an_dup\" disabled style=\"width:100px\" onchange='port_conf_change(this);'>\n"\
						  "</select>\n"\
						"</td>\n"\
					   "");

	     //port duplex
        fprintf(cgiOut,"\n"\
  			           "<td align=left id=tdprompt>%s:</td>",search(llocal,"duplex"));
		fprintf(cgiOut, "<td colspan=2 width=150>\n"\
						  "<select name=\"duplex\" style=\"width:60px\" onchange='port_conf_change(this);'>\n"\
						  "</select>\n"\
						"</td>\n"\
					   "</tr>");

		 //port auto flow contrl
        fprintf(cgiOut,"<tr height=30>\n"\
  			           "<td align=left id=tdprompt>%s:</td>",search(llocal,"an_flowctrl"));
		fprintf(cgiOut, "<td colspan=2>\n"\
						  "<select name=\"an_flowctrl\" disabled style=\"width:100px\" onchange='port_conf_change(this);'>\n"\
						  "</select>\n"\
						"</td>\n"\
					   "");

	     //port flow contrl
        fprintf(cgiOut,"\n"\
  			           "<td align=left id=tdprompt>%s:</td>",search(llocal,"flow_ctrl"));
		fprintf(cgiOut, "<td colspan=2 width=150>\n"\
						  "<select name=\"flow_ctrl\" style=\"width:60px\" onchange='port_conf_change(this);'>\n"\
						  "</select>\n"\
						"</td>\n"\
					   "</tr>");

	    //port back press
        fprintf(cgiOut,"<tr height=30>\n"\
  			           "<td align=left id=tdprompt>%s:</td>",search(llocal,"back_pres"));
		fprintf(cgiOut, "<td colspan=2>\n"\
						  "<select name=\"back_pres\" style=\"width:100px\" onchange='port_conf_change(this);'>\n"\
						  "</select>\n"\
						"</td>\n"\
					   "</tr>");

	   //port media preferred
        fprintf(cgiOut,"<tr height=30>\n"\
  			           "<td align=left id=tdprompt>%s:</td>",search(llocal,"media"));
		fprintf(cgiOut, "<td colspan=2>\n"\
						  "<select name=\"media\" style=\"width:100px\" onchange='port_conf_change(this);'>\n"\
						  "</select>\n"\
						"</td>\n"\
					   "</tr>");
	  //port mtu
	  fprintf(cgiOut,"<tr height=30>\n"\
					 "<td align=left id=tdprompt>%s:</td>",search(llocal,"mtu"));
	  fprintf(cgiOut, "<td colspan=2>\n"\
						"<input type=text name=mtu size=15 onblur='port_conf_change(this);'>\n"\
					  "</td>\n"\
					 "</tr>"); 
{// add by shaojunwu 2008-11-7 14:50:01
		  //buffer mode
		  char get_buffer_mode[12] = "";
			if( 0 != get_port_buffer_mode( get_buffer_mode ) )
			{
				strcpy( get_buffer_mode, "shared" );
			}
	  fprintf(cgiOut,"<tr height=30>\n"\
					 "<td align=left id=tdprompt>%s:</td>",search(llocal,"buffer_mode"));
		
	  fprintf(cgiOut, "<td td colspan=2>\n"\
						"<select name='buffer_mode' style=\"width:100px\" onchange=check_buffer_mode(this)>\n" );
		if( strcmp( get_buffer_mode, "shared" ) == 0 )
		{
			fprintf( cgiOut, "<option value='shared' selected='selected'>shared</option>\n"\
							"<option value='divided'>divided</option>" );
			fprintf( cgiOut, "<input type=hidden name=buffer_mode_value value='shared'>\n" );
		}
		else
		{
			fprintf( cgiOut, "<option value='shared' >shared</option>" );
			fprintf( cgiOut, "<option value='divided' selected='selected'>divided</option>" );
			fprintf( cgiOut, "<input type=hidden name=buffer_mode_value value='divided'>\n" );
		}
			fprintf( cgiOut, "</select>\n"\
					  "</td>\n"\
					  "<td colspan=2><font color=red>%s</font></td>\n"\
					 "</tr>",search(llocal,"buffer_help"));
	
	fprintf( cgiOut, "<script type=text/javascript>\n"\
		"function check_buffer_mode(obj){\n"\
			"document.getElementsByName('buffer_mode_value')[0].value=obj.options[obj.selectedIndex].value;\n"\
			"bl_SendFlag++;\n"\
			"//alert( document.getElementsByName('buffer_mode_value')[0].value );\n"\
		"}\n"\
	"</script>\n");
}
      //end of table
	  fprintf(cgiOut,"</table>\n"\
             "</td>\n"\
            "</tr>");
	//page bottom		
      fprintf(cgiOut,"<tr height=4 valign=top>\n"\
              "<td width=120 height=4 align=right valign=top><img src=/images/bottom_07.gif width=1 height=10/></td>\n"\
              "<td width=827 height=4 valign=top bgcolor=#FFFFFF><img src=/images/bottom_06.gif width=827 height=15/></td>\n"\
            "</tr>\n"\
          "</table>\n"\
        "</td>\n"\
        "<td width=15 background=/images/di999.jpg>&nbsp;</td>\n"\
      "</tr>\n"\
    "</table></td>\n"\
  "</tr>\n"\
  "<tr>\n"\
    "<td colspan=3 align=left valign=top background=/images/di777.jpg><img src=/images/di555.jpg width=61 height=62/></td>\n"\
    "<td align=left valign=top background=/images/di777.jpg>&nbsp;</td>\n"\
    "<td align=left valign=top background=/images/di777.jpg><img src=/images/di666.jpg width=74 height=62/></td>\n"\
  "</tr>\n"\
"</table>\n"\
"</div>\n"\
"</form>\n"\
"</body>\n"\
"</html>"); 

release(llocal);
release(lpublic);
free(str_encry);
if((ret==0)&&(num>0))
{
	Free_ethslot_head(&head);
}

return 0;
}

//对于端口的配置问题
void port_config(char *def_portno)
{  
	char str_portno[32];
	char str_decv_name[32];
	char str_para_1[32];
	int op_ret = WS_SUCCESS;
    unsigned short mtu_arg;

	memset(str_portno, 0, sizeof(str_portno));
	memset(str_decv_name, 0, sizeof(str_decv_name));
	memset(str_para_1, 0, sizeof(str_para_1));
	
    int result = cgiFormStringNoNewlines("port_no", str_portno, 16);
	if( cgiFormSuccess != result )
	{
	    ShowAlert(search(llocal,"no_port_name"));
		return;
	}

    
    result = cgiFormSubmitClicked("default");
    if( cgiFormSuccess == result )
    {   
		op_ret = ccgi_port_default(str_portno);
		ShowRes( search(llocal,"default"),op_ret );
		if(WS_SUCCESS == op_ret)
		{
		    redirection( def_portno);
		}
    }

    result = cgiFormStringNoNewlines("admin_status", str_para_1, 16);
    if( cgiFormSuccess == result )
    {
		op_ret = ccgi_port_admin_state(str_portno, str_para_1);
		ShowRes(search(llocal,"admin_status"),op_ret);
    }
	 result = cgiFormStringNoNewlines("auto_status", str_para_1, 16);
    if( cgiFormSuccess == result )
    {
		op_ret = ccgi_port_auto_state(str_portno, str_para_1);
		ShowRes(search(llocal,"auto_status"),op_ret);
    }	
    result = cgiFormStringNoNewlines("link_status", str_para_1, 16);
    if( cgiFormSuccess == result )
    {
		op_ret = ccgi_port_link_state(str_portno, str_para_1);
		ShowRes(search(llocal,"link_status"),op_ret);
    }
    result = cgiFormStringNoNewlines("an_spe", str_para_1, 16);
    if( cgiFormSuccess == result )
    {
   		op_ret = ccgi_port_auto_speed(str_portno, str_para_1);
   		ShowRes(search(llocal,"an_spe"),op_ret );
   		search(llocal,"an_dup");
    }
    result = cgiFormStringNoNewlines("an_dup", str_para_1, 16);
    if( cgiFormSuccess == result )
    {
   		op_ret = ccgi_port_auto_dup(str_portno, str_para_1);
   		ShowRes(search(llocal,"an_dup"),op_ret );
    }
    result = cgiFormStringNoNewlines("an_flowctrl", str_para_1, 16);
    if( cgiFormSuccess == result )
    {
   		op_ret = ccgi_port_auto_flowctl(str_portno, str_para_1);
   		ShowRes(search(llocal,"an_flowctrl"),op_ret );
    }

	result = cgiFormStringNoNewlines("duplex", str_para_1, 16);
    if( cgiFormSuccess == result )
    {                
		op_ret = ccgi_port_dupmode_conf(str_portno, str_para_1);
		ShowRes(search(llocal,"duplex"),op_ret );
    }
	
    result = cgiFormStringNoNewlines("flow_ctrl", str_para_1, 16);
    if( cgiFormSuccess == result )
    {
		op_ret = ccgi_port_flowctl_conf(str_portno, str_para_1);
		ShowRes( search(llocal,"flow_ctrl"),op_ret );
    }
	
    result = cgiFormStringNoNewlines("back_pres", str_para_1, 16);
    if( cgiFormSuccess == result )
    {
		op_ret = ccgi_port_backpre_conf(str_portno, str_para_1);
		ShowRes( search(llocal,"back_pres"),op_ret );
    }

    result = cgiFormStringNoNewlines("speed", str_para_1, 16);
    if( cgiFormSuccess == result )
    {
	    op_ret = ccgi_port_speed_conf(str_portno,str_para_1);
        ShowRes(search(llocal,"speed"),op_ret);
    }

    result = cgiFormStringNoNewlines("media", str_para_1, 16);
        if( cgiFormSuccess == result )
        {
            op_ret = ccgi_port_medai_conf(str_portno, str_para_1);
            ShowRes( search(llocal,"media"),op_ret );
        }

	  //interface_state    add by liuyu 2011-5-9
	  result = cgiFormStringNoNewlines("interface_state", str_para_1, 16);
	    if( cgiFormSuccess == result )
	    {
	    	//fprintf(stderr,"test_str_para_1=%s,str_portno=%s\n",str_para_1,str_portno);
	        op_ret = ccgi_port_interface_state_conf(str_portno, str_para_1);
			ShowRes( search(llocal,"port_inf_state"),op_ret );
	    }
	
    result = cgiFormStringNoNewlines("mtu", str_para_1, 16);
	if (strcmp(str_para_1,"") != 0)
	{
		mtu_arg=strtoul(str_para_1,0,10);
		if((mtu_arg<64) || (mtu_arg>8192))
		{
			op_ret=WS_OUT_RANGE;
			ShowRes( search(llocal,"mtu"),op_ret );
		}
		else
		{
			if( cgiFormSuccess == result )
			{
				op_ret = ccgi_port_mtu_conf(str_portno, str_para_1);
				ShowRes( search(llocal,"mtu"),op_ret );
			}
		}
	}
   
    config_buffer_module();
    
	redirection( def_portno);


	
}


int config_buffer_module()
{
	char buffer_mode[16];
	cgiFormStringNoNewlines("buffer_mode_value", buffer_mode, sizeof(buffer_mode));
	return set_port_buffer_mode( buffer_mode );
}

