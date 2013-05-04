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
* wp_add_static_fdb.c
*
*
* CREATOR:
* autelan.software.Network Dep. team
* zhouym@autelan.com
*
* DESCRIPTION:
* system contrl for add static fdb
*
*
*******************************************************************************/
#include <stdio.h>
#include "cgic.h"
#include <string.h>
#include <stdlib.h>
#include "ws_usrinfo.h"
#include "ws_ec.h"
#include "ws_err.h"
#include "ws_init_dbus.h"
#include "ws_fdb.h"



int ShowAddStaticFDBPage(struct list *lpublic,struct list *lcon);

int cgiMain()
{
	struct list *lpublic;	/*解析public.txt文件的链表头*/
	struct list *lcon;	   /*解析control.txt文件的链表头*/	
	lpublic=get_chain_head("../htdocs/text/public.txt");
	lcon=get_chain_head("../htdocs/text/control.txt");
	ccgi_dbus_init();
    ShowAddStaticFDBPage(lpublic,lcon);
	release(lpublic);  
	release(lcon);

 	return 0;
}

int ShowAddStaticFDBPage(struct list *lpublic,struct list *lcon)
{
	char *encry=(char *)malloc(BUF_LEN);				/*存储从wp_usrmag.cgi带入的加密字符串*/
	char *str;
	
	char fdb_encry[BUF_LEN]; 
	char vlanid[N],vlanname[N],mac[N],portno[N],trunk[N];

	int i,ret;


	if((cgiFormSubmitClicked("submit_fdb") != cgiFormSuccess)&&(cgiFormSubmitClicked("submit_add") != cgiFormSuccess)&&(cgiFormSubmitClicked("submit_del") != cgiFormSuccess))
	{
			 memset(encry,0,BUF_LEN);
			 cgiFormStringNoNewlines("UN", encry, BUF_LEN); 
			 str=dcryption(encry);
			 if(str==NULL)
			 {
				   ShowErrorPage(search(lpublic,"ill_user"));	 /*用户非法*/
				   return 0;
			 }
			 //memset(fdb_encry,0,BUF_LEN); 				  /*清空临时变量*/
	}
	memset(fdb_encry,0,BUF_LEN); 				  /*清空临时变量*/
	cgiFormStringNoNewlines("fdb_encry",fdb_encry,BUF_LEN);
	cgiHeaderContentType("text/html");
	fprintf(cgiOut,"<html xmlns=\"http://www.w3.org/1999/xhtml\"><head>");
	fprintf(cgiOut,"<meta http-equiv=Content-Type content=text/html; charset=gb2312>");
	fprintf(cgiOut,"<title>%s</title>",search(lcon,"fdb_man"));
	fprintf(cgiOut,"<link rel=stylesheet href=/style.css type=text/css>"\
	  "<style type=text/css>"\
	  ".a3{width:30;border:0; text-align:center}"\
	  "</style>"\
	"</head>"\
	"<script src=/ip.js>"\
	"</script>"\
		"<script type='text/javascript'>"\
		"function changestate(){"\
			"var a = document.getElementsByName('vlantype')[0];"\
			"var b = document.getElementsByName('vlantype')[1];"\
			"var aa = document.getElementsByName('vlanid')[0];"\
			"var bb = document.getElementsByName('vlanname')[0];"\
			"var port = document.getElementsByName('addtype')[0];"\
			"var trunk = document.getElementsByName('addtype')[1];"\
			"var portno = document.getElementsByName('portno')[0];"\
			"var trunkid = document.getElementsByName('trunk')[0];"\
			
			"if( a.checked == true )"\
			"{"\
				"bb.disabled = true;"\
				"bb.style.backgroundColor = '#ccc';"\
				"aa.disabled = false;"\
				"aa.style.backgroundColor = '#fff';"\
			"}"\
			"else"\
			"{"\
				"aa.disabled = true;"\
				"aa.style.backgroundColor = '#ccc';"\
				"bb.disabled = false;"\
				"bb.style.backgroundColor = '#fff';"\
			"}"\

			"if( port.checked == true )"\
			"{"\
				"trunkid.disabled = true;"\
				"trunkid.style.backgroundColor = '#ccc';"\
				"portno.disabled = false;"\
				"portno.style.backgroundColor = '#fff';"\
			"}"\
			"else"\
			"{"\
				"portno.disabled = true;"\
				"portno.style.backgroundColor = '#ccc';"\
				"trunkid.disabled = false;"\
				"trunkid.style.backgroundColor = '#fff';"\
			"}"\
		"}"\
		"</script>"\
	"<body>");	

	
	if(cgiFormSubmitClicked("submit_add") == cgiFormSuccess)		//添加静态fdb
	{
		memset(vlanid,0,N);
		memset(vlanname,0,N);
		memset(mac,0,N);
		memset(portno,0,N);
		memset(trunk,0,N);
		cgiFormStringNoNewlines("vlanid",vlanid,N);
		cgiFormStringNoNewlines("vlanname",vlanname,N);
		cgiFormStringNoNewlines("mac",mac,N);
		cgiFormStringNoNewlines("portno",portno,N);
		cgiFormStringNoNewlines("trunk",trunk,N);

		if(strcmp(vlanid,"")!=0)
		{
			if(strcmp(mac,"")!=0)
			{
				if(strcmp(portno,"")!=0)
				{
						ret = -1;
						ret = create_fdb_static(mac, vlanid, portno);	//by vlanid
						switch(ret)
						{
							case -7:
								ShowAlert(search(lcon,"port_out_range"));
								break;
							case -6:
								ShowAlert(search(lcon,"port_format"));
								break;
							case -5:
								ShowAlert(search(lcon,"illegal_input"));
								break;
							case -4:
								ShowAlert(search(lcon,"vlan_form"));
								break;
							case -3:
								ShowAlert(search(lcon,"addr_br_mul"));
								break;
							case -2:
								ShowAlert(search(lcon,"mac_form"));
								break;
							case -1:
								ShowAlert(search(lpublic,"oper_fail"));
								break;
							case NPD_FDB_ERR_NONE :
								ShowAlert(search(lpublic,"oper_succ"));
								break;
							case NPD_FDB_ERR_GENERAL:
								ShowAlert(search(lpublic,"oper_fail"));
								break;
							case NPD_FDB_ERR_NODE_EXIST :
								ShowAlert(search(lcon,"fdb_exist"));
								break;
							case NPD_FDB_ERR_NODE_NOT_EXIST:
								ShowAlert(search(lcon,"fdb_not_exist"));
								break;
							case NPD_FDB_ERR_PORT_NOTIN_VLAN:
								ShowAlert(search(lcon,"port_not_vlan"));
								break;
							case NPD_FDB_ERR_VLAN_NONEXIST:
								ShowAlert(search(lcon,"vlan_not_exist"));
								break;
							case NPD_FDB_ERR_SYSTEM_MAC:
								ShowAlert(search(lcon,"mac_confilt"));
								break;
							case NPD_FDB_ERR_BADPARA:
								ShowAlert(search(lcon,"illegal_input"));
								break;
							case NPD_FDB_ERR_OCCUR_HW:
								ShowAlert(search(lcon,"HW_error"));
								break;

							default :
								ShowAlert(search(lpublic,"oper_fail"));
						}
				}
				else if(strcmp(trunk,"")!=0)
				{	
						ret = -1;
						ret = create_fdb_static_vlanid_trunk(mac, vlanid, trunk);	//by vlanid
						switch(ret)
						{
							case -7:
								ShowAlert(search(lcon,"trunk_id_illegal"));
								break;
							case -6:
								ShowAlert(search(lcon,"trunk_id_illegal"));
								break;
							case -5:
								ShowAlert(search(lcon,"vlan_out_range"));
								break;
							case -4:
								ShowAlert(search(lcon,"vlan_form"));
								break;
							case -3:
								ShowAlert(search(lcon,"addr_br_mul"));
								break;
							case -2:
								ShowAlert(search(lcon,"mac_form"));
								break;
							case -1:
								ShowAlert(search(lpublic,"oper_fail"));
								break;
							case NPD_FDB_ERR_NONE :
								ShowAlert(search(lpublic,"oper_succ"));
								break;
							case NPD_FDB_ERR_GENERAL:
								ShowAlert(search(lpublic,"oper_fail"));
								break;
							case NPD_FDB_ERR_NODE_EXIST :
								ShowAlert(search(lcon,"fdb_exist"));
								break;
							case NPD_FDB_ERR_NODE_NOT_EXIST:
								ShowAlert(search(lcon,"fdb_not_exist"));
								break;
							case NPD_FDB_ERR_PORT_NOTIN_VLAN:
								ShowAlert(search(lcon,"port_not_vlan"));
								break;
							case NPD_FDB_ERR_VLAN_NONEXIST:
								ShowAlert(search(lcon,"vlan_not_exist"));
								break;
							case NPD_FDB_ERR_SYSTEM_MAC:
								ShowAlert(search(lcon,"mac_confilt"));
								break;
							case NPD_FDB_ERR_BADPARA:
								ShowAlert(search(lcon,"illegal_input"));
								break;
							case NPD_FDB_ERR_OCCUR_HW:
								ShowAlert(search(lcon,"HW_error"));
								break;
							default :
								ShowAlert(search(lpublic,"oper_fail"));
						}

				}
				else
				{
					ShowAlert(search(lcon,"port_and_trunk_not_null"));
				}
			}
			else
			{
				ShowAlert(search(lcon,"mac_null"));
			}
		}
		else if(strcmp(vlanname,"")!=0)
		{
			if(strcmp(mac,"")!=0)
			{
				if(strcmp(portno,"")!=0)
				{
						ret = -1;
						ret = create_fdb_static_vlanname(mac, vlanname, portno);	//by vlanname
						switch(ret)
						{
							case -7:
								ShowAlert(search(lcon,"port_out_range"));
								break;
							case -6:
								ShowAlert(search(lcon,"port_format"));
								break;
							case -4:
								ShowAlert(search(lcon,"vlan_form"));
								break;
							case -3:
								ShowAlert(search(lcon,"addr_br_mul"));
								break;
							case -2:
								ShowAlert(search(lcon,"mac_form"));
								break;
							case -1:
								ShowAlert(search(lpublic,"oper_fail"));
								break;
							case NPD_FDB_ERR_NONE :
								ShowAlert(search(lpublic,"oper_succ"));
								break;
							case NPD_FDB_ERR_GENERAL:
								ShowAlert(search(lpublic,"oper_fail"));
								break;
							case NPD_FDB_ERR_NODE_EXIST :
								ShowAlert(search(lcon,"fdb_exist"));
								break;
							case NPD_FDB_ERR_NODE_NOT_EXIST:
								ShowAlert(search(lcon,"fdb_not_exist"));
								break;
							case NPD_FDB_ERR_PORT_NOTIN_VLAN:
								ShowAlert(search(lcon,"port_not_vlan"));
								break;
							case NPD_FDB_ERR_VLAN_NONEXIST:
								ShowAlert(search(lcon,"vlan_not_exist"));
								break;
							case NPD_FDB_ERR_SYSTEM_MAC:
								ShowAlert(search(lcon,"mac_confilt"));
								break;
							case NPD_FDB_ERR_BADPARA:
								ShowAlert(search(lcon,"illegal_input"));
								break;
							case NPD_FDB_ERR_OCCUR_HW:
								ShowAlert(search(lcon,"HW_error"));
								break;
							default :
								ShowAlert(search(lpublic,"oper_fail"));
						}
				}
				else if(strcmp(trunk,"")!=0)
				{	
						ret = -1;
						ret = create_fdb_static_vlanname_trunk(mac, vlanname, trunk);	//by vlanname
						switch(ret)
						{
							case -7:
								ShowAlert(search(lcon,"trunk_id_illegal"));
								break;
							case -6:
								ShowAlert(search(lcon,"trunk_id_illegal"));
								break;
							case -4:
								ShowAlert(search(lcon,"vlan_form"));
								break;
							case -3:
								ShowAlert(search(lcon,"addr_br_mul"));
								break;
							case -2:
								ShowAlert(search(lcon,"mac_form"));
								break;
							case -1:
								ShowAlert(search(lpublic,"oper_fail"));
								break;
							case NPD_FDB_ERR_NONE :
								ShowAlert(search(lpublic,"oper_succ"));
								break;
							case NPD_FDB_ERR_GENERAL:
								ShowAlert(search(lpublic,"oper_fail"));
								break;
							case NPD_FDB_ERR_NODE_EXIST :
								ShowAlert(search(lcon,"fdb_exist"));
								break;
							case NPD_FDB_ERR_NODE_NOT_EXIST:
								ShowAlert(search(lcon,"fdb_not_exist"));
								break;
							case NPD_FDB_ERR_PORT_NOTIN_VLAN:
								ShowAlert(search(lcon,"port_not_vlan"));
								break;
							case NPD_FDB_ERR_VLAN_NONEXIST:
								ShowAlert(search(lcon,"vlan_not_exist"));
								break;
							case NPD_FDB_ERR_SYSTEM_MAC:
								ShowAlert(search(lcon,"mac_confilt"));
								break;
							case NPD_FDB_ERR_BADPARA:
								ShowAlert(search(lcon,"illegal_input"));
								break;
							case NPD_FDB_ERR_OCCUR_HW:
								ShowAlert(search(lcon,"HW_error"));
								break;
							default :
								ShowAlert(search(lpublic,"oper_fail"));
						}

				}
				else
				{
					ShowAlert(search(lcon,"port_and_trunk_not_null"));
				}
			}
			else
			{
				ShowAlert(search(lcon,"mac_null"));
			}

		}
		else
		{
			ShowAlert(search(lcon,"vlanid_name_null"));
		}
	}


	if(cgiFormSubmitClicked("submit_del") == cgiFormSuccess)		//删除静态fdb
	{
		memset(vlanid,0,N);
		memset(vlanname,0,N);
		memset(mac,0,N);
		memset(portno,0,N);
		cgiFormStringNoNewlines("vlanid",vlanid,N);
		cgiFormStringNoNewlines("vlanname",vlanname,N);
		cgiFormStringNoNewlines("mac",mac,N);
		cgiFormStringNoNewlines("portno",portno,N);
		
		if(strcmp(vlanid,"")!=0)
		{
			if(strcmp(mac,"")!=0)
			{
					ret = -1;
					ret = delete_fdb_static_by_vid(mac, vlanid);		//by vlanid and mac
					switch(ret)
					{
						case -5:
							ShowAlert(search(lcon,"vlan_out_range"));
							break;
						case -4:
							ShowAlert(search(lcon,"vlan_form"));
							break;
						case -3:
							ShowAlert(search(lcon,"addr_br_mul"));
							break;
						case -2:
							ShowAlert(search(lcon,"mac_form"));
							break;
						case -1:
							ShowAlert(search(lpublic,"oper_fail"));
							break;
						case NPD_FDB_ERR_NONE :
							ShowAlert(search(lpublic,"oper_succ"));
							break;
						case NPD_FDB_ERR_GENERAL:
							ShowAlert(search(lpublic,"oper_fail"));
							break;
						case NPD_FDB_ERR_NODE_EXIST :
							ShowAlert(search(lcon,"fdb_exist"));
							break;
						case NPD_FDB_ERR_NODE_NOT_EXIST:
							ShowAlert(search(lcon,"fdb_not_exist"));
							break;
						case NPD_FDB_ERR_PORT_NOTIN_VLAN:
							ShowAlert(search(lcon,"port_not_vlan"));
							break;
						case NPD_FDB_ERR_VLAN_NONEXIST:
							ShowAlert(search(lcon,"vlan_not_exist"));
							break;
						case NPD_FDB_ERR_SYSTEM_MAC:
							ShowAlert(search(lcon,"mac_confilt"));
							break;
						case NPD_FDB_ERR_BADPARA:
							ShowAlert(search(lcon,"illegal_input"));
							break;
						case NPD_FDB_ERR_OCCUR_HW:
							ShowAlert(search(lcon,"HW_error"));
							break;
						default :
							ShowAlert(search(lpublic,"oper_fail"));
					}
			}
			else													//mac 为空时
			{
					ret = -1;
					ret = delete_fdb_static_by_vlanid(vlanid);		//by vlanid
					switch(ret)
					{
						case -3:
							ShowAlert(search(lcon,"vlan_out_range"));
							break;
						case -2:
							ShowAlert(search(lcon,"vlan_form"));
							break;
						case -1:
							ShowAlert(search(lpublic,"oper_fail"));
							break;
						case NPD_FDB_ERR_NONE :
							ShowAlert(search(lpublic,"oper_succ"));
							break;
						case NPD_FDB_ERR_GENERAL:
							ShowAlert(search(lpublic,"oper_fail"));
							break;
						case NPD_FDB_ERR_NODE_EXIST :
							ShowAlert(search(lcon,"fdb_exist"));
							break;
						case NPD_FDB_ERR_NODE_NOT_EXIST:
							ShowAlert(search(lcon,"fdb_not_exist"));
							break;
						case NPD_FDB_ERR_PORT_NOTIN_VLAN:
							ShowAlert(search(lcon,"port_not_vlan"));
							break;
						case NPD_FDB_ERR_VLAN_NONEXIST:
							ShowAlert(search(lcon,"vlan_not_exist"));
							break;
						case NPD_FDB_ERR_SYSTEM_MAC:
							ShowAlert(search(lcon,"mac_confilt"));
							break;
						case NPD_FDB_ERR_BADPARA:
							ShowAlert(search(lcon,"illegal_input"));
							break;
						case NPD_FDB_ERR_OCCUR_HW:
							ShowAlert(search(lcon,"HW_error"));
							break;
						default :
							ShowAlert(search(lpublic,"oper_fail"));
					}

			}
		}
		else if(strcmp(vlanname,"")!=0)
		{
			if(strcmp(mac,"")!=0)
			{
					ret = -1;
					ret = delete_fdb_static_by_vlanname(mac, vlanname);		//by vlanname
					switch(ret)
					{
						case -4:
							ShowAlert(search(lcon,"vlan_form"));
							break;
						case -3:
							ShowAlert(search(lcon,"addr_br_mul"));
							break;
						case -2:
							ShowAlert(search(lcon,"mac_form"));
							break;
						case -1:
							ShowAlert(search(lpublic,"oper_fail"));
							break;
						case NPD_FDB_ERR_NONE :
							ShowAlert(search(lpublic,"oper_succ"));
							break;
						case NPD_FDB_ERR_GENERAL:
							ShowAlert(search(lpublic,"oper_fail"));
							break;
						case NPD_FDB_ERR_NODE_EXIST :
							ShowAlert(search(lcon,"fdb_exist"));
							break;
						case NPD_FDB_ERR_NODE_NOT_EXIST:
							ShowAlert(search(lcon,"fdb_not_exist"));
							break;
						case NPD_FDB_ERR_PORT_NOTIN_VLAN:
							ShowAlert(search(lcon,"port_not_vlan"));
							break;
						case NPD_FDB_ERR_VLAN_NONEXIST:
							ShowAlert(search(lcon,"vlan_not_exist"));
							break;
						case NPD_FDB_ERR_SYSTEM_MAC:
							ShowAlert(search(lcon,"mac_confilt"));
							break;
						case NPD_FDB_ERR_BADPARA:
							ShowAlert(search(lcon,"illegal_input"));
							break;
						case NPD_FDB_ERR_OCCUR_HW:
							ShowAlert(search(lcon,"HW_error"));
							break;
						default :
							ShowAlert(search(lpublic,"oper_fail"));
					}
			}
			else
			{
				ShowAlert(search(lcon,"mac_null"));
			}	
		}
		else if(strcmp(portno,"")!=0)		//by port
		{
				ret = -1;
				ret = delete_fdb_static_by_port(portno);		//by port
				switch(ret)
				{
					case -3:
						ShowAlert(search(lcon,"port_out_range"));
						break;
					case -2:
						ShowAlert(search(lcon,"port_format"));
						break;
					case -1:
						ShowAlert(search(lpublic,"oper_fail"));
						break;
					case NPD_FDB_ERR_NONE :
						ShowAlert(search(lpublic,"oper_succ"));
						break;
					case NPD_FDB_ERR_GENERAL:
						ShowAlert(search(lpublic,"oper_fail"));
						break;
					case NPD_FDB_ERR_NODE_EXIST :
						ShowAlert(search(lcon,"fdb_exist"));
						break;
					case NPD_FDB_ERR_NODE_NOT_EXIST:
						ShowAlert(search(lcon,"fdb_not_exist"));
						break;
					case NPD_FDB_ERR_PORT_NOTIN_VLAN:
						ShowAlert(search(lcon,"port_not_vlan"));
						break;
					case NPD_FDB_ERR_VLAN_NONEXIST:
						ShowAlert(search(lcon,"vlan_not_exist"));
						break;
					case NPD_FDB_ERR_SYSTEM_MAC:
						ShowAlert(search(lcon,"mac_confilt"));
						break;
					case NPD_FDB_ERR_BADPARA:
						ShowAlert(search(lcon,"illegal_input"));
						break;
					case NPD_FDB_ERR_OCCUR_HW:
						ShowAlert(search(lcon,"HW_error"));
						break;
					default :
						ShowAlert(search(lpublic,"oper_fail"));
				}
		}
		else
		{
			ShowAlert(search(lcon,"vlanid_name_null"));
		}
	}

	fprintf(cgiOut,"<form method=post name=fdbform>"\
	"<div align=center>"\
	"<table width=976 border=0 cellpadding=0 cellspacing=0>"\
	"<tr>"\
	"<td width=8 align=left valign=top background=/images/di22.jpg><img src=/images/youce4.jpg width=8 height=30/></td>"\
	"<td width=51 align=left valign=bottom background=/images/di22.jpg><img src=/images/youce33.jpg width=37 height=24/></td>"\
	"<td width=153 align=left valign=bottom background=/images/di22.jpg><font id=titleen>FDB</font><font id=%s> %s</font></td>",search(lpublic,"title_style"),search(lpublic,"management"));
	fprintf(cgiOut,"<td width=690 align=right valign=bottom background=/images/di22.jpg>");
	
	  fprintf(cgiOut,"<table width=130 border=0 cellspacing=0 cellpadding=0>"\
	  "<tr>"\
	  "<td width=62 align=center><input id=but type=submit name=submit_fdb style=background-image:url(/images/%s) value=""></td>",search(lpublic,"img_ok"));		  
	  if((cgiFormSubmitClicked("submit_fdb") != cgiFormSuccess)&&(cgiFormSubmitClicked("submit_add") != cgiFormSuccess)&&(cgiFormSubmitClicked("submit_del") != cgiFormSuccess))
		fprintf(cgiOut,"<td width=62 align=left><a href=wp_contrl.cgi?UN=%s target=mainFrame><img src=/images/%s border=0 width=62 height=20/></a></td>",encry,search(lpublic,"img_cancel"));
	  else										   
		fprintf(cgiOut,"<td width=62 align=left><a href=wp_contrl.cgi?UN=%s target=mainFrame><img src=/images/%s border=0 width=62 height=20/></a></td>",fdb_encry,search(lpublic,"img_cancel"));
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
					if((cgiFormSubmitClicked("submit_fdb") != cgiFormSuccess)&&(cgiFormSubmitClicked("submit_add") != cgiFormSuccess)&&(cgiFormSubmitClicked("submit_del") != cgiFormSuccess))
					{
						fprintf(cgiOut,"<tr height=25>"\
						  "<td align=left id=tdleft><a href=wp_show_fdb.cgi?UN=%s target=mainFrame class=top><font id=yingwen_san>FDB </font><font id=%s>%s</font></a></td>",encry,search(lpublic,"menu_san"),search(lcon,"show_fdb"));   /*突出显示*/
						fprintf(cgiOut,"</tr>");
						fprintf(cgiOut,"<tr height=25>"\
						  "<td align=left id=tdleft><a href=wp_config_agingtime.cgi?UN=%s target=mainFrame class=top><font id=yingwen_san>FDB </font><font id=%s>%s</font></a></td>",encry,search(lpublic,"menu_san"),search(lcon,"fdb_age"));
						fprintf(cgiOut,"</tr>"\
						"<tr height=26>"\
						  "<td align=left id=tdleft background=/images/bottom_bg.gif style=\"border-right:0\"><font id=%s>%s</font><font id=yingwen_san> FDB</font></td>",search(lpublic,"menu_san"),search(lcon,"fdb_sta"));
						fprintf(cgiOut,"</tr>"\
						"<tr height=25>"\
						  "<td align=left id=tdleft><a href=wp_add_blacklist.cgi?UN=%s target=mainFrame class=top><font id=%s>%s</font></a></td>",encry,search(lpublic,"menu_san"),search(lcon,"fdb_bla"));
						fprintf(cgiOut,"</tr>");
						fprintf(cgiOut,"<tr height=25>"\
						  "<td align=left id=tdleft><a href=wp_config_fdb_limit.cgi?UN=%s target=mainFrame class=top><font id=yingwen_san>FDB </font><font id=%s>%s</font></a></td>",encry,search(lpublic,"menu_san"),search(lcon,"fdb_limit"));
						fprintf(cgiOut,"</tr>");
						 //add new web page for delete fdb
							fprintf(cgiOut,"<tr height=25>"\
    						"<td align=left id=tdleft><a href=wp_delete_fdb.cgi?UN=%s target=mainFrame class=top><font id=%s>%s FDB</font></a></td>",encry,search(lpublic,"menu_san"),search(lcon,"del"));  
    						fprintf(cgiOut,"</tr>");
					}
					else //if(cgiFormSubmitClicked("submit_fdb") == cgiFormSuccess)
					{
						fprintf(cgiOut,"<tr height=25>"\
						  "<td align=left id=tdleft><a href=wp_show_fdb.cgi?UN=%s target=mainFrame class=top><font id=yingwen_san>FDB </font><font id=%s>%s</font></a></td>",fdb_encry,search(lpublic,"menu_san"),search(lcon,"show_fdb"));   /*突出显示*/
						fprintf(cgiOut,"</tr>");
						fprintf(cgiOut,"<tr height=25>"\
						  "<td align=left id=tdleft><a href=wp_config_agingtime.cgi?UN=%s target=mainFrame class=top><font id=yingwen_san>FDB </font><font id=%s>%s</font></a></td>",fdb_encry,search(lpublic,"menu_san"),search(lcon,"fdb_age"));
						fprintf(cgiOut,"</tr>"\
						"<tr height=26>"\
						  "<td align=left id=tdleft background=/images/bottom_bg.gif style=\"border-right:0\"><font id=%s>%s</font><font id=yingwen_san> FDB</font></td>",search(lpublic,"menu_san"),search(lcon,"fdb_sta"));
						fprintf(cgiOut,"</tr>"\
						"<tr height=25>"\
						  "<td align=left id=tdleft><a href=wp_add_blacklist.cgi?UN=%s target=mainFrame class=top><font id=%s>%s</font></a></td>",fdb_encry,search(lpublic,"menu_san"),search(lcon,"fdb_bla"));
						
						fprintf(cgiOut,"<tr height=25>"\
						  "<td align=left id=tdleft><a href=wp_config_fdb_limit.cgi?UN=%s target=mainFrame class=top><font id=yingwen_san>FDB </font><font id=%s>%s</font></a></td>",fdb_encry,search(lpublic,"menu_san"),search(lcon,"fdb_limit"));
						fprintf(cgiOut,"</tr>");
						 //add new web page for delete fdb
							fprintf(cgiOut,"<tr height=25>"\
    						"<td align=left id=tdleft><a href=wp_delete_fdb.cgi?UN=%s target=mainFrame class=top><font id=%s>%s FDB</font></a></td>",fdb_encry,search(lpublic,"menu_san"),search(lcon,"del"));  
    						fprintf(cgiOut,"</tr>");

					}
					for(i=0;i<4;i++)
					{
						fprintf(cgiOut,"<tr height=25>"\
						  "<td id=tdleft>&nbsp;</td>"\
						"</tr>");
					}

				  fprintf(cgiOut,"</table>"\
			  "</td>"\
			  "<td align=left valign=top style=\"background-color:#ffffff; border-right:1px solid #707070; padding-left:30px; padding-top:10px\">"\
					  "<table width=700 border=0 cellspacing=0 cellpadding=0>"\
													"<tr>"\
													  "<td align=left valign=top  style=\"padding-top:18px\">");
												fprintf(cgiOut,"<table border=0 cellspacing=0 cellpadding=0>");
												//add static fdb
												fprintf(cgiOut,"<tr height=30>");
												  	fprintf(cgiOut,"<td><input type=\"radio\" name=\"vlantype\" value=\"1\" onclick=\"changestate()\" checked>%s:</td>",search(lcon,"vID"));
												  	fprintf(cgiOut,"<td width=140><input type=text name=vlanid size=21></td>"\
												  	"<td><font color=red>(1--4094)</font></td>");
												fprintf(cgiOut,"<tr height=30>");
													fprintf(cgiOut,"<td><input type=\"radio\" name=\"vlantype\" value=\"2\" onclick=\"changestate()\">%s:</td>",search(lcon,"vlan_name"));
													fprintf(cgiOut,"<td width=140><input type=text name=vlanname size=21></td>"\
													"<td><font color=red>(%s)</font></td>",search(lcon,"vlanname_form"));
												fprintf(cgiOut,"<tr height=30>");
													fprintf(cgiOut,"<td>%s:</td>","MAC");
													fprintf(cgiOut,"<td width=140><input type=text name=mac size=21></td>"\
													"<td><font color=red>%s</font></td>","(00:00:11:22:33:44)");
												fprintf(cgiOut,"<tr height=30>");
													fprintf(cgiOut,"<td><input type=\"radio\" name=\"addtype\" value=\"1\" onclick=\"changestate()\" checked>%s:</td>",search(lcon,"_port"));
													fprintf(cgiOut,"<td width=140><input type=text name=portno size=21></td>"\
													"<td><font color=red>(%s)</font></td>",search(lcon,"port_form"));
												fprintf(cgiOut,"<tr height=30>");
													fprintf(cgiOut,"<td><input type=\"radio\" name=\"addtype\" value=\"2\" onclick=\"changestate()\">%s:</td>","TrunkID");
													fprintf(cgiOut,"<td width=140><input type=text name=trunk size=21></td>"\
													"<td><font color=red>(1--127)</font></td>");

												fprintf(cgiOut,"<script type='text/javascript'>"\
																"changestate();"\
																"</script>");
												fprintf(cgiOut,"</table>");
												fprintf(cgiOut,"<table border=0 cellspacing=0 cellpadding=0 style=\"padding-top:18px\">");
													fprintf(cgiOut,"<tr height=30>");
														fprintf(cgiOut,"<td width=60><input type=submit style= height:22px;width=60  border=0 name=submit_add style=background-image:url(/images/SubBackGif.gif) value=\"%s\"></td>",search(lcon,"add"));
														fprintf(cgiOut,"<td width=20></td>");
														fprintf(cgiOut,"<td width=60><input type=submit style= height:22px;width=60  border=0 name=submit_del style=background-image:url(/images/SubBackGif.gif) value=\"%s\"></td>",search(lcon,"del"));
												fprintf(cgiOut,"</table>");

												/*
												memset(vlanid,0,N);
												memset(vlanname,0,N);
												memset(mac,0,N);
												memset(portno,0,N);
												cgiFormStringNoNewlines("vlanid",vlanid,N);
												cgiFormStringNoNewlines("vlanname",vlanname,N);
												cgiFormStringNoNewlines("mac",mac,N);
												cgiFormStringNoNewlines("portno",portno,N);
												if((strcmp(vlanid,"")==0)&&(strcmp(vlanname,"")==0)||(strcmp(mac,"")==0))
												{
													fprintf(cgiOut,"<table border=0 cellspacing=0 cellpadding=0 style=\"padding-top:18px\">");
														fprintf(cgiOut,"<tr height=30>");
														  	fprintf(cgiOut,"<td width=60><input type=submit style= height:22px;width=60  border=0 name=submit_add style=background-image:url(/images/SubBackGif.gif) value=\"%s\" disabled=true></td>","添加");
															fprintf(cgiOut,"<td width=20></td>");
														  	fprintf(cgiOut,"<td width=60><input type=submit style= height:22px;width=60  border=0 name=submit_del style=background-image:url(/images/SubBackGif.gif) value=\"%s\" disabled=true></td>","删除");
													fprintf(cgiOut,"</table>");
												}
												else
												{
													if(strcmp(portno,"")==0)
													{
														fprintf(cgiOut,"<table border=0 cellspacing=0 cellpadding=0 style=\"padding-top:18px\">");
															fprintf(cgiOut,"<tr height=30>");
															  	fprintf(cgiOut,"<td width=60><input type=submit style= height:22px;width=60  border=0 name=submit_add style=background-image:url(/images/SubBackGif.gif) value=\"%s\" disabled=true></td>","添加");
																fprintf(cgiOut,"<td width=20></td>");
															  	fprintf(cgiOut,"<td width=60><input type=submit style= height:22px;width=60  border=0 name=submit_del style=background-image:url(/images/SubBackGif.gif) value=\"%s\" disabled=false></td>","删除");
														fprintf(cgiOut,"</table>");
													}
													else
													{
														fprintf(cgiOut,"<table border=0 cellspacing=0 cellpadding=0 style=\"padding-top:18px\">");
															fprintf(cgiOut,"<tr height=30>");
															  	fprintf(cgiOut,"<td width=60><input type=submit style= height:22px;width=60  border=0 name=submit_add style=background-image:url(/images/SubBackGif.gif) value=\"%s\" disabled=false></td>","添加");
																fprintf(cgiOut,"<td width=20></td>");
															  	fprintf(cgiOut,"<td width=60><input type=submit style= height:22px;width=60  border=0 name=submit_del style=background-image:url(/images/SubBackGif.gif) value=\"%s\" disabled=true></td>","删除");
														fprintf(cgiOut,"</table>");

													}
												}
												*/

											fprintf(cgiOut,"</td>"\
													  "</tr>"\
														"<tr>");
														  if((cgiFormSubmitClicked("submit_fdb") != cgiFormSuccess)&&(cgiFormSubmitClicked("submit_add") != cgiFormSuccess)&&(cgiFormSubmitClicked("submit_del") != cgiFormSuccess))
														  {
															fprintf(cgiOut,"<td><input type=hidden name=fdb_encry value=%s></td>",encry);
														  }
														  else //if(cgiFormSubmitClicked("submit_fdb") == cgiFormSuccess)
														  { 			 
															fprintf(cgiOut,"<td><input type=hidden name=fdb_encry value=%s></td>",fdb_encry);
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

	free(encry);
																 
	return 0;

}



