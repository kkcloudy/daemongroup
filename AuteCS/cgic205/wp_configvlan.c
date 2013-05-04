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
* wp_configvlan.c
*
*
* CREATOR:
* autelan.software.Network Dep. team
* tangsq@autelan.com
*
* DESCRIPTION:
* system contrl for vlan config
*
*
*******************************************************************************/
#include <stdio.h>
#include "cgic.h"
#include <string.h>
#include <stdlib.h>
#include "ws_usrinfo.h"
#include "ws_init_dbus.h"
#include "ws_ec.h"
#include "ws_dcli_vlan.h"
#include "ws_returncode.h"
#include "ws_dbus_list_interface.h"


#define PageNum  10

int ShowVlanlistPage();   
int show_vlan_IP(unsigned short vlanID,char * revIntfName[],int * Num ,struct list *lpublic);
int search_vlan(int ser_type,char * key,struct vlan_info_simple  vlan_inf[],int  location[],int * matchNum);


int cgiMain()
{
    ShowVlanlistPage();    
 return 0;
}

int ShowVlanlistPage()
{

	struct list *lpublic;   /*解析public.txt文件的链表头*/
	struct list *lcontrol;  /*解析help.txt文件的链表头*/
	lpublic=get_chain_head("../htdocs/text/public.txt");
	lcontrol=get_chain_head("../htdocs/text/control.txt");
	char *encry=(char *)malloc(BUF_LEN);                /*存储从wp_usrmag.cgi带入的加密字符串*/
	char *PNtemp=(char *)malloc(10);
	char *SNtemp=(char *)malloc(10);
	char *str=NULL;
	char configvlan_encry[BUF_LEN]; 
	struct vlan_info_simple receive_vlan[MAX_VLAN_NUM];

	int i,port_num[MAX_VLAN_NUM],vlanNum=0;
	int cl=1;                 /*cl标识表格的底色，1为#f9fafe，0为#ffffff*/
	char *vIDTemp=(char *)malloc(10);
	char* i_char=(char *)malloc(10);
	char menu[21]="menulist";
	int pageNum=0;
	char * pageNumCA=(char *)malloc(10);
	memset(pageNumCA,0,10);
	char * pageNumCD=(char *)malloc(10);
	memset(pageNumCD,0,10);
	char * pageNumCF=(char *)malloc(10);//first page
	memset(pageNumCF,0,10);
	char * pageNumCL=(char *)malloc(10);//last page
	memset(pageNumCL,0,10);

	char * search_text=(char *)malloc(30);
	memset(search_text,0,30);
	char * sub_text=(char *)malloc(10);
	memset(sub_text,0,10);
	char * deleteOP=(char *)malloc(10);
	memset(deleteOP,0,10);
	char * VIDLater=(char *)malloc(21);
	memset(VIDLater,0,21);
	int locat[4095];
	char * addn=(char *)malloc(N);;
	memset(addn,0,N);


	char * flag_href=(char *)malloc(10);
	memset(flag_href,0,10);
	char * SER_TYPE=(char *)malloc(10);
	memset(SER_TYPE,0,10);
	char * SER_TEXT=(char *)malloc(30);
	memset(SER_TEXT,0,30);

	int retu=0;
	int flag=0;
	for(i=0;i<4095;i++)
	{
		receive_vlan[i].vlanId=0;
		receive_vlan[i].vlanStat=0;
		receive_vlan[i].vlanName=(char*)malloc(21);
		memset(receive_vlan[i].vlanName,0,21);
		port_num[i]=0;
		locat[i]=0;
	}


	//////////////////////////added at 2009-2-26///////////////////////////
	char * revTemp[8];
	int revIPNum;
	for( i=0; i<8; i++ )
	{
		revTemp[i]=(char *)malloc(30);
		memset(revTemp[i], 0, 30);
	}
	ccgi_dbus_init();
	if(cgiFormSubmitClicked("submit_search") != cgiFormSuccess && cgiFormSubmitClicked("submit_ret") != cgiFormSuccess && cgiFormSubmitClicked("submit_egress_filter") != cgiFormSuccess)
	{
		memset(encry,0,BUF_LEN);
		cgiFormStringNoNewlines("UN", encry, BUF_LEN); 

		cgiFormStringNoNewlines("FLAG",flag_href,10);
		cgiFormStringNoNewlines("SER_TYPE", SER_TYPE, 10);
		cgiFormStringNoNewlines("SER_TEXT", SER_TEXT, 30);	 

		str=dcryption(encry);
		if(str==NULL)
		{
			ShowErrorPage(search(lpublic,"ill_user"));    /*用户非法*/
			return 0;
		}
		memset(configvlan_encry,0,BUF_LEN);                   /*清空临时变量*/
		strcpy(addn,str);
	}


	memset(PNtemp,0,10);
	cgiFormStringNoNewlines("PN",PNtemp,10);
	pageNum=atoi(PNtemp);
	memset(SNtemp,0,10);
	cgiFormStringNoNewlines("SN",SNtemp,10);
	memset(search_text,0,30);
	memset(sub_text,0,10);
	cgiFormStringNoNewlines("config_encry",configvlan_encry,BUF_LEN);
	cgiFormStringNoNewlines("ser_text",search_text,30);
	cgiFormStringNoNewlines("ser_select",sub_text,10);
	cgiFormStringNoNewlines("DELRULE",deleteOP,10);
	cgiFormStringNoNewlines("VLANID",VIDLater,21);
	cgiFormStringNoNewlines("CheckUsr",addn,N);


	if( 0 == strcmp(flag_href,"1") )
	{
		flag = 1;
	}
	fprintf(stderr,"flag_href=%s-flag=%d-SER_TEXT=%s",flag_href,flag,SER_TEXT);

	cgiHeaderContentType("text/html");
	fprintf(cgiOut,"<html xmlns=\"http://www.w3.org/1999/xhtml\"><head>");
	fprintf(cgiOut,"<meta http-equiv=Content-Type content=text/html; charset=gb2312>");
	fprintf(cgiOut,"<title>%s</title>",search(lcontrol,"vlan_manage"));
	fprintf(cgiOut,"<link rel=stylesheet href=/style.css type=text/css>"\
	"<style type=text/css>"\
	"#div1{ width:62px; height:18px; border:1px solid #666666; background-color:#f9f8f7;}"\
	"#div2{ width:60px; height:15px; padding-left:5px; padding-top:3px}"\
	"#link{ text-decoration:none; font-size: 12px}"\
	".configvlan {overflow-x:hidden;  overflow:auto; width: 560px; height=340;  clip: rect( ); padding-top: 0px; padding-right: 0px; padding-bottom: 0px; padding-left: 0px} "\
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
	"<body>");
	if(cgiFormSubmitClicked("submit_search") != cgiFormSuccess && cgiFormSubmitClicked("submit_ret") != cgiFormSuccess && cgiFormSubmitClicked("submit_egress_filter") != cgiFormSuccess)
	retu=checkuser_group(str);
	else retu=checkuser_group(addn);


	fprintf(stderr,"SER_TYPE=%s-SER_TEXT=%s",SER_TYPE,SER_TEXT);


	int type=0,matchN=0;
	unsigned short vID;
	vID=atoi(VIDLater);
	unsigned int temp=vID;

	if(flag == 1 && 0 != strcmp(SER_TEXT,""))
	{
		if(strcmp(SER_TYPE,"VlanID")==0)
		type=1;
		else if(strcmp(SER_TYPE,"VNAME")==0)
		type=2;
		fprintf(stderr,"type=%d-SER_TEXT=%saa",type,SER_TEXT);
		show_vlan_list(receive_vlan,port_num,&vlanNum);
		search_vlan(type,SER_TEXT,receive_vlan,locat,&matchN);
	}


	if(strcmp(deleteOP,"delete")==0)
	{
		deleteIntfForVlanNoShow(temp);
		instance_parameter *paraHead2 = NULL;
		instance_parameter *pq = NULL;
		list_instance_parameter(&paraHead2, SNMPD_SLOT_CONNECT);
		for(pq=paraHead2;(NULL != pq);pq=pq->next)
		{
			retu = delete_vlan(pq->connection,vID);
		}
		free_instance_parameter_list(&paraHead2);

		switch(retu)
		{
			case -2:
				ShowAlert(search(lcontrol,"illegal_vID"));
			break;
			case 0:
				ShowAlert(search(lcontrol,"opt_fail"));
			break;
			case -4:
				ShowAlert(search(lcontrol,"vID_NotExist"));
			break;
			case -5:
				ShowAlert(search(lcontrol,"Default_delete_error"));
			break;
			case -6:
				ShowAlert(search(lcontrol,"HW_error"));
			break;
			case 1:
				ShowAlert(search(lcontrol,"delete_vlan_success"));
			break;
			case -8:
				ShowAlert(search(lcontrol,"s_arp_unknow_err"));
			break;
			default:
				ShowAlert(search(lcontrol,"opt_fail"));
			break;
		}
	}
	else if(strcmp(deleteOP,"delete_l3")==0)
	{
		retu = deleteIntfForVlan(vID);
		switch(retu)
		{
			case 1:
				ShowAlert(search(lcontrol,"opt_succ"));
			break;
			case COMMON_RETURN_CODE_BADPARAM:
				ShowAlert(search(lcontrol,"INPUT_BADPARAM"));
			break;
			case ARP_RETURN_CODE_VLAN_NOTEXISTS :
				ShowAlert(search(lcontrol,"VLAN_NOT_EXITSTS"));
			break;
			case INTERFACE_RETURN_CODE_INTERFACE_NOTEXIST:  /*                     interface not existed*/
				ShowAlert(search(lcontrol,"intf_not_exist"));
			break;
			case INTERFACE_RETURN_CODE_CHECK_MAC_ERROR:
				ShowAlert(search(lcontrol,"macaddr_error"));
			break;
			case INTERFACE_RETURN_CODE_ERROR :
				ShowAlert(search(lcontrol,"intf_delete_error"));
			break;
			case INTERFACE_RETURN_CODE_ROUTE_CREATE_SUBIF:
				ShowAlert(search(lcontrol,"DISABLE_ROUTING_ERROR"));
			break;
			default:
				ShowAlert(search(lcontrol,"opt_fail"));
			break;
		}
	}
	if(cgiFormSubmitClicked("submit_search") == cgiFormSuccess) 
	{
		flag=1;
		memset(search_text,0,30);
		memset(sub_text,0,10);
		cgiFormStringNoNewlines("search_vlan",sub_text,10);
		cgiFormStringNoNewlines("search_text",search_text,30);

		if(strcmp(sub_text,"VID")==0)
		type=1;
		else if(strcmp(sub_text,"VNAME")==0)
		type=2;

		for(i=0;i<4095;i++)
		locat[i]=0;
		if(strcmp(sub_text,"VID")==0 && strcmp(search_text,"")==0)
		{
			ShowAlert(search(lcontrol,"search_not_null"));
			flag=0;
		}
		else if(strcmp(sub_text,"VNAME")==0 && strcmp(search_text,"")==0)
		{
			ShowAlert(search(lcontrol,"search_not_null"));
			flag=0;
		}
		else
		{
			show_vlan_list(receive_vlan,port_num,&vlanNum);
			search_vlan(type,search_text,receive_vlan,locat,&matchN);
		}

	}
	if(cgiFormSubmitClicked("submit_ret") == cgiFormSuccess)  
	{
		flag=0;
	}
	int egress_status = show_vlan_egress_filter();
	if(cgiFormSubmitClicked("submit_egress_filter") == cgiFormSuccess)  
	{
		if(egress_status == 1)
		{
			config_vlan_egress_filter("disable");
		}
		else if(egress_status == 2)
		{
			config_vlan_egress_filter("enable");
		}
	}
	fprintf(cgiOut,"<form method=post>"\
		"<div align=center>"\
		"<table width=976 border=0 cellpadding=0 cellspacing=0>"\
		"<tr>"\
		"<td width=8 align=left valign=top background=/images/di22.jpg><img src=/images/youce4.jpg width=8 height=30/></td>"\
		"<td width=51 align=left valign=bottom background=/images/di22.jpg><img src=/images/youce33.jpg width=37 height=24/></td>"\
		"<td width=153 align=left valign=bottom background=/images/di22.jpg><font id=titleen>VLAN</font><font id=%s> %s</font></td>",search(lpublic,"title_style"),search(lpublic,"management"));
	fprintf(cgiOut,"<td width=690 align=right valign=bottom background=/images/di22.jpg>");	
	fprintf(cgiOut,"<table width=130 border=0 cellspacing=0 cellpadding=0>"\
		"<tr>");
	if(cgiFormSubmitClicked("submit_ret") != cgiFormSuccess && cgiFormSubmitClicked("submit_search") != cgiFormSuccess)
	{
		fprintf(cgiOut,"<td width=62 align=left><a href=wp_contrl.cgi?UN=%s target=mainFrame><img src=/images/%s border=0 width=62 height=20/></a></td>",encry,search(lpublic,"img_ok"));
		fprintf(cgiOut,"<td width=62 align=left><a href=wp_contrl.cgi?UN=%s target=mainFrame><img src=/images/%s border=0 width=62 height=20/></a></td>",encry,search(lpublic,"img_cancel"));
	}
	else
	{
		fprintf(cgiOut,"<td width=62 align=left><a href=wp_contrl.cgi?UN=%s target=mainFrame><img src=/images/%s border=0 width=62 height=20/></a></td>",configvlan_encry,search(lpublic,"img_ok"));
		fprintf(cgiOut,"<td width=62 align=left><a href=wp_contrl.cgi?UN=%s target=mainFrame><img src=/images/%s border=0 width=62 height=20/></a></td>",configvlan_encry,search(lpublic,"img_cancel"));
	}
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
		"<tr height=26>"\
		"<td align=left id=tdleft background=/images/bottom_bg.gif style=\"border-right:0\"><font id=yingwen_san>VLAN </font><font id=%s>%s</font></td>",search(lpublic,"menu_san"),search(lcontrol,"config"));   /*突出显示*/
	fprintf(cgiOut,"</tr>");
	if(cgiFormSubmitClicked("submit_search") != cgiFormSuccess && cgiFormSubmitClicked("submit_ret") != cgiFormSuccess)
	{
		if(retu==0)  /*管理员*/
		{
			fprintf(cgiOut,"<tr height=25>"\
				"<td align=left id=tdleft><a href=wp_addvlan.cgi?UN=%s target=mainFrame class=top><font id=%s>%s</font><font id=yingwen_san> VLAN</font></a></td>",encry,search(lpublic,"menu_san"),search(lcontrol,"add"));			  
			fprintf(cgiOut,"</tr>");
		}
		fprintf(cgiOut,"<tr height=25>"\
			"<td align=left id=tdleft><a href=wp_show_pvlan.cgi?UN=%s target=mainFrame class=top><font id=yingwen_san>PVLAN </font><font id=%s>%s</font></a></td>",encry,search(lpublic,"menu_san"),search(lcontrol,"list"));		  
		fprintf(cgiOut,"</tr>");
		if(retu==0)  /*管理员*/
		{
			fprintf(cgiOut,"<tr height=25>"\
				"<td align=left id=tdleft><a href=wp_config_pvlan.cgi?UN=%s target=mainFrame class=top><font id=%s>%s</font><font id=yingwen_san> PVLAN</font></a></td>",encry,search(lpublic,"menu_san"),search(lcontrol,"pvlan_add"));			  
			fprintf(cgiOut,"</tr>");
		}
	}
	else		  
	{
		if(retu==0)  /*管理员*/
		{
			fprintf(cgiOut,"<tr height=25>"\
				"<td align=left id=tdleft><a href=wp_addvlan.cgi?UN=%s target=mainFrame style=color:#000000><font id=%s>%s</font><font id=yingwen_san> VLAN</font></a></td>",configvlan_encry,search(lpublic,"menu_san"),search(lcontrol,"add"));						 
			fprintf(cgiOut,"</tr>");
		}
		fprintf(cgiOut,"<tr height=25>"\
			"<td align=left id=tdleft><a href=wp_show_pvlan.cgi?UN=%s target=mainFrame style=color:#000000><font id=yingwen_san>PVLAN </font><font id=%s>%s</font></a></td>",configvlan_encry,search(lpublic,"menu_san"),search(lcontrol,"list"));						 
		fprintf(cgiOut,"</tr>");
		if(retu==0)  /*管理员*/
		{
			fprintf(cgiOut,"<tr height=25>"\
				"<td align=left id=tdleft><a href=wp_config_pvlan.cgi?UN=%s target=mainFrame style=color:#000000><font id=%s>%s</font><font id=yingwen_san> PVLAN</font></a></td>",configvlan_encry,search(lpublic,"menu_san"),search(lcontrol,"pvlan_add"));						 
			fprintf(cgiOut,"</tr>");
		}
	}
	for(i=0;i<13;i++)
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
		"<td id=sec1 colspan=4 style=\"border-bottom:2px solid #53868b;font-size:14px;padding-left:0px;padding-top:0px\">%s</td>",search(lcontrol,"Vlan_info"));
	fprintf(cgiOut,"</tr>"\
		"<tr height=25 padding-top:10px  align=left>"\
		"<td colspan=2>%s: </td>", search(lcontrol,"cur_filter_status"));
		if (egress_status == 2)
		{
			fprintf(cgiOut,"<td>enabled</td>");
		}
		else if (egress_status == 1)
		{
			fprintf(cgiOut,"<td>disabled</td>");
		}
		
		if (egress_status == 2)
		{
			fprintf(cgiOut,"<td><input type=submit name=submit_egress_filter style=width:60px; height:36px border=0 style=background-image:url(/images/SubBackGif.gif)  value=%s></td>", search(lcontrol,"disable"));
		}
		else if (egress_status == 1)
		{
			fprintf(cgiOut,"<td><input type=submit name=submit_egress_filter style=width:60px; height:36px border=0 style=background-image:url(/images/SubBackGif.gif)  value=%s></td>", search(lcontrol,"enable"));
		}
		fprintf(cgiOut,"</tr>"\
		"<tr height=25 padding-top:10px  align=left>");
	if(strcmp(sub_text,"VNAME")==0)
	{
		fprintf(cgiOut,"<td width=40 align=right><select name=search_vlan>"\
			"<option value=VNAME>Vlan Name</option>"\
			"<option value=VID>VlanID</option></select>"\
			"</td>");
	}
	else
	{
		fprintf(cgiOut,"<td width=40 align=right><select name=search_vlan>"\
			"<option value=VID>VlanID</option>"\
			"<option value=VNAME>Vlan Name</option></select>"\
			"</td>");
	}
	fprintf(cgiOut,"<td width=60><input type=text name=search_text size=12 value=%s></td>",search_text);
	fprintf(cgiOut,"<td width=60 style=padding-left:5px><input type=submit name=submit_search style=width:60px; height:36px border=0 name=addIP style=background-image:url(/images/SubBackGif.gif) value=%s></td>",search(lcontrol,"search_vlan"));
	fprintf(cgiOut,"<td width=470 style=padding-left:5px><input type=submit name=submit_ret style=width:60px; height:36px border=0 name=addIP style=background-image:url(/images/SubBackGif.gif)  value=%s></td>",search(lpublic,"return"));
	fprintf(cgiOut,"<td><input type=text name=no_submit style=display:none></td>"\
		"</tr>"\
		"<tr>"\

		"<td align=left valign=top  style=\"padding-top:2px\" colspan=5>"\
		"<div class=configvlan><table width=508 border=1 frame=below rules=rows bordercolor=#cccccc cellspacing=0 cellpadding=0>"\
		"<tr height=25 bgcolor=#eaeff9  padding-top:5px>"\
		"<th width=70 style=font-size:12px align=left><font id=%s>%s</font></th>", search(lpublic,"menu_thead"),"VLAN ID");
	fprintf(cgiOut,"<th width=100 style=font-size:12px align=left><font id=%s>%s</font></th>", search(lpublic,"menu_thead"),search(lcontrol,"vlan_name"));
	fprintf(cgiOut,"<th width=100 style=font-size:12px align=left><font id=%s>%s</font></th>", search(lpublic,"menu_thead"),search(lcontrol,"vlan_stat"));
	fprintf(cgiOut,"<th width=144 style=font-size:12px align=left><font id=%s>%s</font></th>", search(lpublic,"menu_thead"),"VLAN IP");
	fprintf(cgiOut,"<th width=70 style=font-size:12px align=left><font id=%s>%s</font></th>", search(lpublic,"menu_thead"),search(lcontrol,"port_num"));
	fprintf(cgiOut,"<th width=13>&nbsp;</th>");
	fprintf(cgiOut,"</tr>");
	int k=0,FirstPage=0,LastPage=0;
	if(flag!=1)
		k=show_vlan_list(receive_vlan,port_num,&vlanNum);

	int xnt=0,head=0,tail=0;
	if(flag==1)
		vlanNum=matchN;

	if((vlanNum%PageNum)==0)
		LastPage=(vlanNum/PageNum); //计算出最大页数
	else	
		LastPage=(vlanNum/PageNum)+1; //计算出最大页数
	if(k==5)
		ShowAlert(search(lpublic,"contact_adm"));
	if(k==CMD_SUCCESS)	
	{
		if(0==strcmp(SNtemp,"PageFirst"))
		{
			if(vlanNum-pageNum*PageNum<PageNum)
				xnt=vlanNum;
			else	  
				xnt=(pageNum+1)*PageNum;

			head=0;
			tail=xnt;
		}
		else if(0==strcmp(SNtemp,"PageDown") || 0==strcmp(SNtemp,""))
		{
			if(vlanNum-pageNum*PageNum<0)
			{
				pageNum=pageNum-1;
				ShowAlert(search(lcontrol,"Page_end")); 
			}
			if(vlanNum-pageNum*PageNum<PageNum)
				xnt=vlanNum;
			else   
				xnt=(pageNum+1)*PageNum;

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
			else   
				xnt=(pageNum+1)*PageNum;
			head=pageNum*PageNum;
			tail=xnt;
		}
		else if(0==strcmp(SNtemp,"PageLast"))
		{
			head = (pageNum-1)*PageNum;
			tail = vlanNum;
			pageNum--;
		}
		////////////////
		sprintf(pageNumCA,"%d",pageNum);

        //kehao modify  2011-04-21
		//for(i=head;i<(tail-1);i++)		
		for(i=head;i<tail;i++)
		//	
		{
			//kehao add for debug web program
			//fprintf(stderr,"#################################### head = %d\n",head);
			//fprintf(stderr,"$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$ tail = %d\n",tail);
			//
			memset(menu,0,21);   
			strcpy(menu,"menulist");
			sprintf(i_char,"%d",i+1);
			strcat(menu,i_char);
			for( k=0; k<8; k++ )
			memset(revTemp[k], 0, 30);

			sprintf( vIDTemp,"%d",receive_vlan[i].vlanId); /*int 转化成 char* */
			if(flag!=1)
			{
				show_vlan_IP(receive_vlan[i].vlanId,revTemp,&revIPNum,lpublic);

                //kehao modify  2011-04-21
                //if(receive_vlan[i].vlanId!=4095)
				if(receive_vlan[i].vlanId!=4095  && receive_vlan[i].vlanId!=0)
			    //		
				{
					fprintf(cgiOut,"<tr height=25 bgcolor=%s>",setclour(cl));
					fprintf(cgiOut,"<td style=font-size:12px align=left>%d</td>",receive_vlan[i].vlanId);
					fprintf(cgiOut,"<td style=font-size:12px align=left>%s</td>",receive_vlan[i].vlanName);
					if(receive_vlan[i].vlanStat==1)
						fprintf(cgiOut,"<td style=font-size:12px align=left>%s</td>","UP");
					else if(receive_vlan[i].vlanStat==0)
						fprintf(cgiOut,"<td style=font-size:12px align=left>%s</td>","DOWN");

					if( 0 == revIPNum )
						fprintf(cgiOut,"<td style=font-size:12px align=left>%s</td>",search(lcontrol,"Unallocated"));
					else if( 1 == revIPNum )
						fprintf(cgiOut,"<td style=font-size:12px align=left>%s</td>",revTemp[0]);
					else if( revIPNum >1 )
						fprintf(cgiOut,"<td style=\"font-size:12px;font-weight:bold\" align=left><a id=link href=wp_vlanInfo.cgi?UN=%s&VID=%s&VNAME=%s target=mainFrame title=\"%s\"><font >...</a></td>",encry,vIDTemp,receive_vlan[i].vlanName,search(lcontrol,"mutiple_ip_show_tips"));

					fprintf(cgiOut,"<td style=font-size:12px align=left>%d</td>",port_num[i]);

					fprintf(cgiOut,"<td align=left>");
				}

				if(receive_vlan[i].vlanId==4095)
				{
					fprintf(cgiOut,"&nbsp;");
				}
				//kehao modify 2011-04-21
				//else
				else if(receive_vlan[i].vlanId != 0)
				//
				{
					fprintf(cgiOut,"<div style=\"position:relative; z-index:%d\" onmouseover=\"popMenu('%s');\" onmouseout=\"popMenu('%s');\">",(vlanNum-i),menu,menu);
					fprintf(cgiOut,"<img src=/images/detail.gif>"\
						"<div id=%s style=\"display:none; position:absolute; top:5px; left:0;\">",menu);
					fprintf(cgiOut,"<div id=div1>");
					if(cgiFormSubmitClicked("submit_search") != cgiFormSuccess && cgiFormSubmitClicked("submit_ret") != cgiFormSuccess)
					{
						if(retu==0)  /*管理员*/
						{
							fprintf(cgiOut,"<div id=div2 onmouseover=\"this.style.backgroundColor='#b6bdd2'\" onmouseout=\"this.style.backgroundColor='#f9f8f7'\"><a id=link href=wp_vlandetail.cgi?UN=%s&VID=%s&SetVlan=%s target=mainFrame>%s</a></div>",encry,vIDTemp,"NoSet",search(lpublic,"configure"));
							fprintf(cgiOut,"<div id=div2 onmouseover=\"this.style.backgroundColor='#b6bdd2'\" onmouseout=\"this.style.backgroundColor='#f9f8f7'\"><a id=link href=wp_configvlan.cgi?UN=%s&VLANID=%u&DELRULE=%s&PN=%s target=mainFrame onclick=\"return confirm('%s')\">%s</a></div>",encry,receive_vlan[i].vlanId,"delete",pageNumCA,search(lcontrol,"confirm_delete"),search(lcontrol,"delete"));
						}
						fprintf(cgiOut,"<div id=div2 onmouseover=\"this.style.backgroundColor='#b6bdd2'\" onmouseout=\"this.style.backgroundColor='#f9f8f7'\"><a id=link href=wp_vlanInfo.cgi?UN=%s&VID=%s&VNAME=%s target=mainFrame>%s</a></div>",encry,vIDTemp,receive_vlan[i].vlanName,search(lpublic,"details"));
						if(retu==0)  /*管理员*/
						fprintf(cgiOut,"<div id=div2 onmouseover=\"this.style.backgroundColor='#b6bdd2'\" onmouseout=\"this.style.backgroundColor='#f9f8f7'\"><a id=link href=wp_configvlan.cgi?UN=%s&VLANID=%u&DELRULE=%s&PN=%s target=mainFrame onclick=\"return confirm('%s')\">%s</a></div>",encry,receive_vlan[i].vlanId,"delete_l3",pageNumCA,search(lcontrol,"confirm_delete"),search(lcontrol,"delete_l3"));
					}
					else
					{
						if(retu==0)  /*管理员*/
						{
							fprintf(cgiOut,"<div id=div2 onmouseover=\"this.style.backgroundColor='#b6bdd2'\" onmouseout=\"this.style.backgroundColor='#f9f8f7'\"><a id=link href=wp_vlandetail.cgi?UN=%s&VID=%s&SetVlan=%s target=mainFrame>%s</a></div>",configvlan_encry,vIDTemp,"NoSet",search(lpublic,"configure"));
							fprintf(cgiOut,"<div id=div2 onmouseover=\"this.style.backgroundColor='#b6bdd2'\" onmouseout=\"this.style.backgroundColor='#f9f8f7'\"><a id=link href=wp_configvlan.cgi?UN=%s&VLANID=%u&DELRULE=%s&PN=%s target=mainFrame onclick=\"return confirm('%s')\">%s</a></div>",configvlan_encry,receive_vlan[i].vlanId,"delete",pageNumCA,search(lcontrol,"confirm_delete"),search(lcontrol,"delete"));
						}
						fprintf(cgiOut,"<div id=div2 onmouseover=\"this.style.backgroundColor='#b6bdd2'\" onmouseout=\"this.style.backgroundColor='#f9f8f7'\"><a id=link href=wp_vlanInfo.cgi?UN=%s&VID=%s&VNAME=%s target=mainFrame>%s</a></div>",configvlan_encry,vIDTemp,receive_vlan[i].vlanName,search(lpublic,"details"));
						if(retu==0)  /*管理员*/
						fprintf(cgiOut,"<div id=div2 onmouseover=\"this.style.backgroundColor='#b6bdd2'\" onmouseout=\"this.style.backgroundColor='#f9f8f7'\"><a id=link href=wp_configvlan.cgi?UN=%s&VLANID=%u&DELRULE=%s&PN=%s target=mainFrame onclick=\"return confirm('%s')\">%s</a></div>",encry,receive_vlan[i].vlanId,"delete_l3",pageNumCA,search(lcontrol,"confirm_delete"),search(lcontrol,"delete_l3"));
					}
					fprintf(cgiOut,"</div>"\
					"</div>"\
					"</div>");
				}
				fprintf(cgiOut,"</td>");
				fprintf(cgiOut,"</tr>");
				cl=!cl;
			}
			else if(flag==1)//搜索列表
			{
				if(matchN>0)
				{
					show_vlan_IP(receive_vlan[locat[i]].vlanId,revTemp,&revIPNum,lpublic);
					fprintf(cgiOut,"<tr height=25 bgcolor=%s>",setclour(cl));

                    //kehao modify 2011-04-21
					//if(receive_vlan[locat[i]].vlanId!=4095)
					if(receive_vlan[locat[i]].vlanId!=4095  && receive_vlan[locat[i]].vlanId!=0)
					//	
					{
						fprintf(cgiOut,"<td style=font-size:12px align=left>%d</td>",receive_vlan[locat[i]].vlanId);
						fprintf(cgiOut,"<td style=font-size:12px align=left>%s</td>",receive_vlan[locat[i]].vlanName);
						if(receive_vlan[locat[i]].vlanStat==1)
							fprintf(cgiOut,"<td style=font-size:12px align=left>%s</td>","UP");
						else if(receive_vlan[locat[i]].vlanStat==0)
							fprintf(cgiOut,"<td style=font-size:12px align=left>%s</td>","DOWN");

						if( 0 == revIPNum )
							fprintf(cgiOut,"<td style=font-size:12px align=left>%s</td>",search(lcontrol,"Unallocated"));
						else if( 1 == revIPNum )
							fprintf(cgiOut,"<td style=font-size:12px align=left>%s</td>",revTemp[0]);
						else if( revIPNum >1 )
							fprintf(cgiOut,"<td style=font-size:12px align=left><a id=link href=wp_vlanInfo.cgi?UN=%s&VID=%s&VNAME=%s target=mainFrame>...</a></td>",configvlan_encry,vIDTemp,receive_vlan[locat[i]].vlanName);

						fprintf(cgiOut,"<td style=font-size:12px align=left>%d</td>",port_num[locat[i]]);
						sprintf( vIDTemp,"%d",receive_vlan[i].vlanId); /*int 转化成 char* */
						fprintf(cgiOut,"<td align=left>");
					}

					if(receive_vlan[locat[i]].vlanId==4095)
					{
						fprintf(cgiOut,"&nbsp;");
					}
					//kehao modify 2011-04-21
					//else
					else if(receive_vlan[locat[i]].vlanId!=0)
					//	
					{
						fprintf(cgiOut,"<div style=\"position:relative; z-index:%d\" onmouseover=\"popMenu('%s');\" onmouseout=\"popMenu('%s');\">",(vlanNum-i),menu,menu);
						fprintf(cgiOut,"<img src=/images/detail.gif>"\
						"<div id=%s style=\"display:none; position:absolute; top:5px; left:0;\">",menu);
						fprintf(cgiOut,"<div id=div1>");
						if(cgiFormSubmitClicked("submit_search") != cgiFormSuccess && cgiFormSubmitClicked("submit_ret") != cgiFormSuccess)
						{
							if(retu==0)  /*管理员*/
							{
								fprintf(cgiOut,"<div id=div2 onmouseover=\"this.style.backgroundColor='#b6bdd2'\" onmouseout=\"this.style.backgroundColor='#f9f8f7'\"><a id=link href=wp_vlandetail.cgi?UN=%s&VID=%s&SetVlan=%s&SER_TYPE=%s&SER_TEXT=%s&FLAG=%s target=mainFrame>%s</a></div>",encry,vIDTemp,"NoSet",sub_text,search_text,"1",search(lpublic,"configure"));
								fprintf(cgiOut,"<div id=div2 onmouseover=\"this.style.backgroundColor='#b6bdd2'\" onmouseout=\"this.style.backgroundColor='#f9f8f7'\"><a id=link href=wp_configvlan.cgi?UN=%s&VLANID=%u&DELRULE=%s&PN=%s target=mainFrame onclick=\"return confirm('%s')\">%s</a></div>",encry,receive_vlan[locat[i]].vlanId,"delete",pageNumCA,search(lcontrol,"confirm_delete"),search(lcontrol,"delete"));
							}
							fprintf(cgiOut,"<div id=div2 onmouseover=\"this.style.backgroundColor='#b6bdd2'\" onmouseout=\"this.style.backgroundColor='#f9f8f7'\"><a id=link href=wp_vlanInfo.cgi?UN=%s&VID=%s&VNAME=%s target=mainFrame>%s</a></div>",encry,vIDTemp,receive_vlan[locat[i]].vlanName,search(lpublic,"details"));
							if(retu==0)  /*管理员*/
							fprintf(cgiOut,"<div id=div2 onmouseover=\"this.style.backgroundColor='#b6bdd2'\" onmouseout=\"this.style.backgroundColor='#f9f8f7'\"><a id=link href=wp_configvlan.cgi?UN=%s&VLANID=%u&DELRULE=%s&PN=%s target=mainFrame onclick=\"return confirm('%s')\">%s</a></div>",encry,receive_vlan[locat[i]].vlanId,"delete_l3",pageNumCA,search(lcontrol,"confirm_delete"),search(lcontrol,"delete_l3"));
						}
						else
						{
							if(retu==0)  /*管理员*/
							{
								fprintf(cgiOut,"<div id=div2 onmouseover=\"this.style.backgroundColor='#b6bdd2'\" onmouseout=\"this.style.backgroundColor='#f9f8f7'\"><a id=link href=wp_vlandetail.cgi?UN=%s&VID=%u&SetVlan=%s&SER_TYPE=%s&SER_TEXT=%s&FLAG=%s target=mainFrame>%s</a></div>",configvlan_encry,receive_vlan[locat[i]].vlanId,"NoSet",sub_text,search_text,"1",search(lpublic,"configure"));
								fprintf(cgiOut,"<div id=div2 onmouseover=\"this.style.backgroundColor='#b6bdd2'\" onmouseout=\"this.style.backgroundColor='#f9f8f7'\"><a id=link href=wp_configvlan.cgi?UN=%s&VLANID=%u&DELRULE=%s&PN=%s target=mainFrame onclick=\"return confirm('%s')\">%s</a></div>",configvlan_encry,receive_vlan[locat[i]].vlanId,"delete",pageNumCA,search(lcontrol,"confirm_delete"),search(lcontrol,"delete"));
							}
							fprintf(cgiOut,"<div id=div2 onmouseover=\"this.style.backgroundColor='#b6bdd2'\" onmouseout=\"this.style.backgroundColor='#f9f8f7'\"><a id=link href=wp_vlanInfo.cgi?UN=%s&VID=%u&VNAME=%s target=mainFrame>%s</a></div>",configvlan_encry,receive_vlan[locat[i]].vlanId,receive_vlan[locat[i]].vlanName,search(lpublic,"details"));
							if(retu==0)  /*管理员*/
								fprintf(cgiOut,"<div id=div2 onmouseover=\"this.style.backgroundColor='#b6bdd2'\" onmouseout=\"this.style.backgroundColor='#f9f8f7'\"><a id=link href=wp_configvlan.cgi?UN=%s&VLANID=%u&DELRULE=%s&PN=%s target=mainFrame onclick=\"return confirm('%s')\">%s</a></div>",configvlan_encry,receive_vlan[locat[i]].vlanId,"delete_l3",pageNumCA,search(lcontrol,"confirm_delete"),search(lcontrol,"delete_l3"));
						}
						fprintf(cgiOut,"</div>"\
							"</div>"\
							"</div>");
					}
					fprintf(cgiOut,"</td>");
					fprintf(cgiOut,"</tr>");

					cl=!cl;
				}
			} 
		}					
	}
	fprintf(cgiOut,"</table></div></td>"\
		"</tr>"\
		"<tr>"\
		"<td colspan=4>"\
		"<table width=430 style=padding-top:2px>"\
		"<tr>");
	sprintf(pageNumCF,"%d",FirstPage);
	sprintf(pageNumCA,"%d",pageNum+1);
	sprintf(pageNumCD,"%d",pageNum-1);
	sprintf(pageNumCL,"%d",LastPage);
	if(cgiFormSubmitClicked("submit_search") != cgiFormSuccess && cgiFormSubmitClicked("submit_ret") != cgiFormSuccess)
	{
		fprintf(cgiOut,"<td align=center style=padding-top:2px><a href=wp_configvlan.cgi?UN=%s&PN=%s&SN=%s>%s</td>",encry,pageNumCF,"PageFirst",search(lcontrol,"Page_First"));
		fprintf(cgiOut,"<td align=center style=padding-top:2px><a href=wp_configvlan.cgi?UN=%s&PN=%s&SN=%s>%s</td>",encry,pageNumCD,"PageUp",search(lcontrol,"page_up"));
		fprintf(cgiOut,"<td align=center style=padding-top:2px><a href=wp_configvlan.cgi?UN=%s&PN=%s&SN=%s>%s</td>",encry,pageNumCA,"PageDown",search(lcontrol,"page_down"));
		fprintf(cgiOut,"<td align=center style=padding-top:2px><a href=wp_configvlan.cgi?UN=%s&PN=%s&SN=%s>%s</td>",encry,pageNumCL,"PageLast",search(lcontrol,"Page_Last"));
	}
	else
	{
		fprintf(cgiOut,"<td align=center style=padding-top:2px><a href=wp_configvlan.cgi?UN=%s&PN=%s&SN=%s>%s</td>",configvlan_encry,pageNumCF,"PageFirst",search(lcontrol,"Page_First"));
		fprintf(cgiOut,"<td align=center style=padding-top:2px><a href=wp_configvlan.cgi?UN=%s&PN=%s&SN=%s>%s</td>",configvlan_encry,pageNumCD,"PageUp",search(lcontrol,"page_up"));  
		fprintf(cgiOut,"<td align=center style=padding-top:2px><a href=wp_configvlan.cgi?UN=%s&PN=%s&SN=%s>%s</td>",configvlan_encry,pageNumCA,"PageDown",search(lcontrol,"page_down"));
		fprintf(cgiOut,"<td align=center style=padding-top:2px><a href=wp_configvlan.cgi?UN=%s&PN=%s&SN=%s>%s</td>",configvlan_encry,pageNumCL,"PageLast",search(lcontrol,"Page_Last"));
	}
	fprintf(cgiOut,"</tr>"\
		"<tr height=30  align=center valign=bottom>"\
		"<td colspan=4>%s%d%s(%s%d%s)</td>",search(lpublic,"current_sort"),pageNum+1,search(lpublic,"page"),search(lpublic,"total"),LastPage,search(lpublic,"page"));
	fprintf(cgiOut,"</tr>"\
		"</table></td>"\
		"</tr>"\
		"<tr>");

	if(cgiFormSubmitClicked("submit_search") != cgiFormSuccess && cgiFormSubmitClicked("submit_ret") != cgiFormSuccess && cgiFormSubmitClicked("submit_egress_filter") != cgiFormSuccess)
	{
		fprintf(cgiOut,"<td><input type=hidden name=config_encry value=%s></td>",encry);
		fprintf(cgiOut,"<td><input type=hidden name=ser_text value=%s></td>",search_text);
		fprintf(cgiOut,"<td><input type=hidden name=ser_select value=%s></td>",sub_text);
		fprintf(cgiOut,"<td><input type=hidden name=CheckUsr value=%s></td>",str);
	}
	else
	{ 			 
		fprintf(cgiOut,"<td><input type=hidden name=config_encry value=%s></td>",configvlan_encry);
		fprintf(cgiOut,"<td><input type=hidden name=ser_text value=%s></td>",search_text);
		fprintf(cgiOut,"<td><input type=hidden name=ser_select value=%s></td>",sub_text);
		fprintf(cgiOut,"<td><input type=hidden name=CheckUsr value=%s></td>",addn);
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
	if(strcmp(SNtemp,"")!=0 && strcmp(PNtemp,"")!=0)
	{
		if(cgiFormSubmitClicked("submit_delroute") != cgiFormSuccess)
		{
			fprintf( cgiOut, "<script type='text/javascript'>\n" );
			fprintf( cgiOut, "window.location.href='wp_configvlan.cgi?UN=%s&PN=%d';\n", encry,pageNum);
			fprintf( cgiOut, "</script>\n" );
		}
		else
		{
			fprintf( cgiOut, "<script type='text/javascript'>\n" );
			fprintf( cgiOut, "window.location.href='wp_configvlan.cgi?UN=%s&PN=%d';\n", configvlan_encry,pageNum);
			fprintf( cgiOut, "</script>\n" );
		}
	}

	for(i=0;i<4095;i++)
	{
		free(receive_vlan[i].vlanName);
	}
	for( i = 0 ;i < 8; i++)
	{
		free(revTemp[i]);
	}
	free(SER_TYPE);
	free(SER_TEXT);
	free(flag_href);

	free(revTemp);
	free(addn);
	free(i_char);
	free(PNtemp);
	free(SNtemp);
	free(vIDTemp);
	free(pageNumCA);
	free(pageNumCD);
	free(pageNumCL);
	free(pageNumCF);
	free(sub_text);
	free(search_text);
	free(encry);
	free(deleteOP);
	free(VIDLater);
	release(lpublic);  
	release(lcontrol);                  										 
	return 0;
}
                											 
int show_vlan_IP(unsigned short vlanID,char * revIntfName[],int * Num ,struct list *lpublic)
{
	FILE * ft;
	int tempID=vlanID;
	char * command=(char * )malloc(150);
	memset(command,0,150);
	char temp[30];
	memset(temp,0,30);
	sprintf(temp,"vlan%d",tempID);
	strcat(command,"show_intf_ip.sh");
	strcat(command," ");
	strcat(command,temp);
	strcat(command," ");
	strcat(command,"2>/dev/null | awk '{if($1==\"inet\") {print $2}}' >/var/run/apache2/vlan_intf_ip.txt");
	system(command);
	if((ft=fopen("/var/run/apache2/vlan_intf_ip.txt","r"))==NULL)
	{
		ShowAlert(search(lpublic,"error_open"));
		return 0;
	}
	memset(temp , 0, 30);
	int i = 0;
	while(fgets(temp,28,ft))
	{
		strcpy(revIntfName[i],temp);
		i++;
		memset(temp,0,30);
	}
	fclose(ft);
	*Num = i;
	free(command);
	return 0;
}


int search_vlan(int ser_type,char * key,struct vlan_info_simple  vlan_inf[],int  location[],int * matchNum)
{
	int i,k;

	/*vlan id 匹配*/
	k=0;
	if(ser_type==1)
	{
		unsigned short  temp=atoi(key);
		if(temp!=0)
		{
			for(i=0;i<4095;i++)
			{
				if(vlan_inf[i].vlanId==temp)
				{
					location[k]=i;
					k++;
					break;
				}
			}
			*matchNum=k;
		}
	}
	else if(ser_type==2)   /*vlan name 匹配*/
	{
		for(i=0;i<4095;i++)
		{
			if(strstr(vlan_inf[i].vlanName,key)!=NULL)
			{
				location[k]=i;
				k++;
			}
		}
		*matchNum=k;
	}
	return 0;
}
