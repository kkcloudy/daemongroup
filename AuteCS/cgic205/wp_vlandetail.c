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
* wp_vlandetail.c
*
*
* CREATOR:
* autelan.software.Network Dep. team
* tangsq@autelan.com
*
* DESCRIPTION:
* system contrl for vlan detail config
*
*
*******************************************************************************/
#include <stdio.h>
#include "cgic.h"
#include <string.h>
#include <stdlib.h>
#include <math.h>
#include "ws_usrinfo.h"
#include "ws_init_dbus.h"
#include "util/npd_list.h"
#include "sysdef/npd_sysdef.h"
#include "npd/nam/npd_amapi.h"
#include "ws_err.h"
#include "ws_ec.h"
#include "ws_public.h"
#include "ws_trunk.h"
#include <sys/types.h>
#include <sys/wait.h>
#include "ws_dcli_vlan.h"
#include "ws_returncode.h"
#include "ws_dbus_def.h"

int show_vlan_detail();
int setvlanID_hand(char* vidOld,char* vidnew);
int modifyVlanIp(char *vid,struct list *lcontrol);
double MaskChange();
int checkMark(unsigned long mark);
void addordel_trunk_alert(int retu,struct list *lcontrol);

int cgiMain()
{
    show_vlan_detail();
    return 0;
}

int show_vlan_detail()
{
    struct list *lpublic;   /*解析public.txt文件的链表头*/
    struct list *lcontrol;  /*解析help.txt文件的链表头*/
    lpublic=get_chain_head("../htdocs/text/public.txt");
    lcontrol=get_chain_head("../htdocs/text/control.txt"); 
	int retu = 0;
    char *encry=(char *)malloc(BUF_LEN);    			
    char *str;
    char *endptr = NULL;  
    int i;
    char detail_encry[BUF_LEN]; 
    char *IDForSearch=(char *)malloc(10);
    memset(IDForSearch,0,10);
    char *IDForSelfTurn=(char *)malloc(10);
    memset(IDForSelfTurn,0,10);
    char *IDnew=(char *)malloc(10);
    memset(IDnew,0,10);
    
    char * advanced_routing=(char *)malloc(20);
    memset(advanced_routing,0,20);
    
    char * Enable_mode=(char *)malloc(10);
    memset(Enable_mode,0,10);
    
    char * filter_type=(char *)malloc(10);
    memset(filter_type,0,10);

    char * trunkID=(char *)malloc(10);
    memset(trunkID,0,10);
    
    char * trunkTag=(char *)malloc(10);
    memset(trunkTag,0,10);
/////////////////////////////////////////
    char * flag_href=(char *)malloc(10);
    memset(flag_href,0,10);
    char * SER_TYPE=(char *)malloc(10);
    memset(SER_TYPE,0,10);
    char * SER_TEXT=(char *)malloc(30);
    memset(SER_TEXT,0,30);
    int flag_for_vlan_show = 0;
    
  struct vlan_info v_InfoByID;
    int tagNum,untagNum;
    unsigned short IDTemp;
  int k,result=0,resultlater;
  char **responses;
  int IDflag=0;
  struct trunk_profile head,*q;
  int result1=0,Tnum=0;
  ccgi_dbus_init();
  char *product=readproductID();

	PRODUCT_PAGE_INFO Product_select_info;
	memset(&Product_select_info, 0 ,sizeof(PRODUCT_PAGE_INFO));

	Product_Adapter_for_page(&Product_select_info,product);
    for(i=0;i<Product_select_info.port_total_num;i++)
    {
        v_InfoByID.slot_port_tag[i]=(char *)malloc(6);
        v_InfoByID.slot_port_untag[i]=(char *)malloc(6);
        memset(v_InfoByID.slot_port_tag[i],0,6);
        memset(v_InfoByID.slot_port_untag[i],0,6);	
		
		v_InfoByID.promisProt_tag[i] = 0;
		v_InfoByID.promisProt_untag[i] = 0;
    }
  v_InfoByID.vlanName=(char *)malloc(21);
  if((cgiFormSubmitClicked("submit_vlandetail") != cgiFormSuccess) && (cgiFormSubmitClicked("delIP") != cgiFormSuccess) 
  	&& (cgiFormSubmitClicked("addIP") != cgiFormSuccess) && (cgiFormSubmitClicked("modID") != cgiFormSuccess)
  	&&(cgiFormSubmitClicked("submit_igmp") != cgiFormSuccess)&&(cgiFormSubmitClicked("vlan_filter") != cgiFormSuccess) 
  	&&(cgiFormSubmitClicked("addTrunk") != cgiFormSuccess) && (cgiFormSubmitClicked("delTrunk") != cgiFormSuccess)
  	&&(cgiFormSubmitClicked("Map_intf") != cgiFormSuccess) && (cgiFormSubmitClicked("vlan_bond_slot") != cgiFormSuccess)
  	&& (cgiFormSubmitClicked("vlan_unbond_slot") != cgiFormSuccess))
  {
    memset(encry,0,BUF_LEN);
    cgiFormStringNoNewlines("UN", encry, BUF_LEN); 
    cgiFormStringNoNewlines("VID", IDForSearch, 10);
    cgiFormStringNoNewlines("FLAG", flag_href, 10);
    cgiFormStringNoNewlines("SER_TYPE", SER_TYPE, 10);
    cgiFormStringNoNewlines("SER_TEXT", SER_TEXT, 30);
    //fprintf(stderr,"SER_TYPE=%s-SER_TEXT=%s",SER_TYPE,SER_TEXT);
    if( strcmp(flag_href,"") != 0)
        flag_for_vlan_show = atoi(flag_href);
    str=dcryption(encry);
    if(str==NULL)
    {
      ShowErrorPage(search(lpublic,"ill_user")); /*用户非法*/
      return 0;
    }
    memset(detail_encry,0,BUF_LEN);                   /*清空临时变量*/
  }

  cgiFormStringNoNewlines("encry_detail",detail_encry,BUF_LEN);
  cgiHeaderContentType("text/html");
  fprintf(cgiOut,"<html xmlns=\"http://www.w3.org/1999/xhtml\"><head>");
  fprintf(cgiOut,"<meta http-equiv=Content-Type content=text/html; charset=gb2312>");
  fprintf(cgiOut,"<title>%s</title>",search(lcontrol,"vlan_manage"));
  fprintf(cgiOut,"<link rel=stylesheet href=/style.css type=text/css>"\
    "<style type=text/css>"\
      ".a3{width:30;border:0; text-align:center}"\
      "#div1{ width:58px; height:18px; border:1px solid #666666; background-color:#f9f8f7;}"\
      "#div2{ width:56px; height:15px; padding-left:3px; padding-top:3px}"\
      "#link{ text-decoration:none; font-size: 12px}"\
      "</style>"\
  
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
      "<script src=/ip.js>"\
      "</script>"\
  "</head>"\
  "<body>");
  cgiFormStringNoNewlines("encry_IDOld",IDForSelfTurn,10);
  if(strcmp(IDForSelfTurn,"1")==0)
        strcpy(IDnew,IDForSelfTurn);
    else 
      cgiFormStringNoNewlines("V_ID",IDnew,10);  //默认vlan抓不到值。
      unsigned int temp=atoi(IDnew); 
      cgiFormStringNoNewlines("advanced_routing",advanced_routing,20);
    cgiFormStringNoNewlines("Enable_mode",Enable_mode,10);	
    cgiFormStringNoNewlines("filter_type",filter_type,10);
    cgiFormStringNoNewlines("trunklist",trunkID,10);
    cgiFormStringNoNewlines("trunkTag",trunkTag,10);

    	
    if(cgiFormSubmitClicked("submit_vlandetail") == cgiFormSuccess)
    {
        /*result=setvlanID_hand(IDForSelfTurn,IDnew);
        resultlater = cgiFormStringMultiple("filter_type", &responses);
        if (result == cgiFormNotFound) 
    	{	
    	} 
        else 
    	{
        	i=0;
            while (responses[i])
    		{
                config_vlan_filter(temp,Enable_mode,responses[i]);
        		i++;
    		}
    	}
        //config_vlan_filter(temp,Enable_mode,filter_type);
         if(strcmp(advanced_routing,"normal")==0)
            createIntfForVlan(temp);
         else if(strcmp(advanced_routing,"advanced")==0)
            config_vlan_advanced_routing(temp);
         modifyVlanIp(IDnew,lcontrol);*/

        cgiFormStringNoNewlines("FLAG", flag_href, 10);
        cgiFormStringNoNewlines("SER_TYPE", SER_TYPE, 10);
        cgiFormStringNoNewlines("SER_TEXT", SER_TEXT, 30);
        if( strcmp(flag_href,"") != 0)
        flag_for_vlan_show = atoi(flag_href);

        //fprintf(stderr,"flag_href=%s-SER_TYPE=%s-SER_TEXT=%s",flag_href,SER_TYPE,SER_TEXT);
    	
        fprintf( cgiOut, "<script type='text/javascript'>\n" );
        fprintf( cgiOut, "window.location.href='wp_configvlan.cgi?UN=%s&FLAG=%d&SER_TYPE=%s&SER_TEXT=%s';\n", detail_encry,flag_for_vlan_show,SER_TYPE,SER_TEXT);
        fprintf( cgiOut, "</script>\n" );
    }
    
    if(cgiFormSubmitClicked("vlan_filter") == cgiFormSuccess)
      {
            resultlater = cgiFormStringMultiple("filter_type", &responses);
            if (result == cgiFormNotFound) 
        	{	
        	} 
            else 
        	{
            	i=0;
                while (responses[i])
        		{
                    config_vlan_filter(temp,Enable_mode,responses[i]);
            		i++;
        		}
        	}
      }
    if(cgiFormSubmitClicked("modID") == cgiFormSuccess)
	{
		result=setvlanID_hand(IDForSelfTurn,IDnew);
		switch(result)
		{
			case -2:
				ShowAlert(search(lcontrol,"illegal_vID"));
			break;
			case -3:
				ShowAlert(search(lcontrol,"same_vID"));	 
			break;
			case -4:
				ShowAlert(search(lcontrol,"exist_vID"));
			break;
			case -1:
				ShowAlert(search(lpublic,"oper_fail"));
			break;
			case -5:
				ShowAlert(search(lcontrol,"HW_error"));	
			break;
			case 1:
				ShowAlert(search(lcontrol,"modify_success"));
			break;
			case -6:
				ShowAlert(search(lcontrol,"L3_modify_fail"));
			break;
			default:
				ShowAlert(search(lpublic,"oper_fail"));
			break;
		}
	}
    if(cgiFormSubmitClicked("addIP") == cgiFormSuccess)
      {
          modifyVlanIp(IDnew,lcontrol);
      }
    if(cgiFormSubmitClicked("delIP") == cgiFormSuccess)
      {
            retu = deleteIntfForVlan(temp);
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
     if(cgiFormSubmitClicked("addTrunk") == cgiFormSuccess)
      {
          unsigned short  IDt=atoi(IDnew);
          unsigned short  IDtr;
    	  
          if(strcmp(trunkID,"")!=0)
          {
            IDtr=atoi(trunkID);
            retu = addordel_trunk("add",IDt,trunkID,trunkTag,lcontrol);
			addordel_trunk_alert(retu,lcontrol);
          }
      }
    if(cgiFormSubmitClicked("delTrunk") == cgiFormSuccess)
      {
          unsigned short  IDt=atoi(IDnew);
          unsigned short  IDtr;
          if(strcmp(trunkID,"")!=0)
          {
            //fprintf(stderr,"1111");
            IDtr=atoi(trunkID);
            retu = addordel_trunk("delete",IDt,trunkID,trunkTag,lcontrol);
			addordel_trunk_alert(retu,lcontrol);
          }
  
      }
	 int ad_ret = 0;
     if(cgiFormSubmitClicked("Map_intf") == cgiFormSuccess)
     {
         if(strcmp(advanced_routing,"normal")==0)
         {
            createIntfForVlan(temp);
         }
         else if(strcmp(advanced_routing,"advanced")==0)
         {
           ad_ret = config_vlan_advanced_routing(temp);
		   if( ad_ret == -1 )
		   		ShowAlert(search(lcontrol,"dbus_conn_null"));
		   else if( ad_ret == -2 )
		   		ShowAlert(search(lcontrol,"already_no_advanced_routing"));
		   else if( ad_ret == -3 )
		   		ShowAlert(search(lcontrol,"already_advanced_routing"));
		   else if( ad_ret == -4 )
		   		ShowAlert(search(lcontrol,"opt_fail"));
		   else if( ad_ret == 0 )
		   		ShowAlert(search(lcontrol,"Operation_Success"));
         }
     }

	 if(cgiFormSubmitClicked("vlan_bond_slot") == cgiFormSuccess)
	 {
	 	int ret = 0;
		char slotid[5] = { 0 };
		char cpu[5] = { 0 };
		char cpu_port[5] = { 0 };

		memset(slotid,0,sizeof(slotid));
  		cgiFormStringNoNewlines("slot_id",slotid,5);
		memset(cpu,0,sizeof(cpu));
  		cgiFormStringNoNewlines("cpu_no",cpu,5);
		memset(cpu_port,0,sizeof(cpu_port));
  		cgiFormStringNoNewlines("cpu_port",cpu_port,5);

		if((strcmp(slotid,"")!=0) && (strcmp(cpu,"")!=0) && (strcmp(cpu_port,"")!=0))
		{
			ret = bond_vlan_to_ms_cpu_port_cmd("bond",IDnew,slotid,cpu,cpu_port);
			switch(ret)
			{
				case 0:ShowAlert(search(lcontrol,"vlan_bind_slot_fail"));
					   break;
				case 1:ShowAlert(search(lcontrol,"vlan_bind_slot_succ"));
					   break;
				case -1:ShowAlert(search(lcontrol,"open_file_error"));
					    break;
				case -2:ShowAlert(search(lcontrol,"s_arp_bad_param"));
					    break;
				case -3:
				case -4:
				case -5:
				case -6:
				case -7:ShowAlert(search(lpublic,"input_para_illegal"));
					    break;
				case -8:ShowAlert(search(lpublic,"slot_not_ac_board"));
					    break;
				case -9:ShowAlert(search(lcontrol,"ve_if_is_exist"));
					    break;
				case -10:ShowAlert(search(lcontrol,"vlan_not_bond_to_slot"));
					     break;
				case -11:ShowAlert(search(lcontrol,"vlan_not_exist_on_slot"));
					     break;
				case -12:ShowAlert(search(lcontrol,"vlan_bond_to_slot"));
					     break;
				case -13:ShowAlert(search(lcontrol,"add_port_fail"));
					     break;
				case -14:ShowAlert(search(lcontrol,"delete_port_fail"));
					     break;
				case -15:ShowAlert(search(lcontrol,"slot_sync_vlan_err"));
					     break;
				case -16:ShowAlert(search(lcontrol,"slot_have_not_cpu_port"));
					     break;
				case -17:ShowAlert(search(lpublic,"error"));
					    break;
			}
		}
	 }

	 if(cgiFormSubmitClicked("vlan_unbond_slot") == cgiFormSuccess)
	 {
	 	int ret = 0;
		char slotid[5] = { 0 };
		char cpu[5] = { 0 };
		char cpu_port[5] = { 0 };

		memset(slotid,0,sizeof(slotid));
  		cgiFormStringNoNewlines("slot_id",slotid,5);
		memset(cpu,0,sizeof(cpu));
  		cgiFormStringNoNewlines("cpu_no",cpu,5);
		memset(cpu_port,0,sizeof(cpu_port));
  		cgiFormStringNoNewlines("cpu_port",cpu_port,5);

		if((strcmp(slotid,"")!=0) && (strcmp(cpu,"")!=0) && (strcmp(cpu_port,"")!=0))
		{
			ret = bond_vlan_to_ms_cpu_port_cmd("unbond",IDnew,slotid,cpu,cpu_port);
			switch(ret)
			{
				case 0:ShowAlert(search(lcontrol,"vlan_unbind_slot_fail"));
					   break;
				case 1:ShowAlert(search(lcontrol,"vlan_unbind_slot_succ"));
					   break;
				case -1:ShowAlert(search(lcontrol,"open_file_error"));
					    break;
				case -2:ShowAlert(search(lcontrol,"s_arp_bad_param"));
					    break;
				case -3:
				case -4:
				case -5:
				case -6:
				case -7:ShowAlert(search(lpublic,"input_para_illegal"));
					    break;
				case -8:ShowAlert(search(lpublic,"slot_not_ac_board"));
					    break;
				case -9:ShowAlert(search(lcontrol,"ve_if_is_exist"));
					    break;
				case -10:ShowAlert(search(lcontrol,"vlan_not_bond_to_slot"));
					     break;
				case -11:ShowAlert(search(lcontrol,"vlan_not_exist_on_slot"));
					     break;
				case -12:ShowAlert(search(lcontrol,"vlan_bond_to_slot"));
					     break;
				case -13:ShowAlert(search(lcontrol,"add_port_fail"));
					     break;
				case -14:ShowAlert(search(lcontrol,"delete_port_fail"));
					     break;
				case -15:ShowAlert(search(lcontrol,"slot_sync_vlan_err"));
					     break;
				case -16:ShowAlert(search(lcontrol,"slot_have_not_cpu_port"));
					     break;
				case -17:ShowAlert(search(lpublic,"error"));
					    break;
			}
		}
	 }

    //end

  fprintf(cgiOut,"<form method=post>"\
  "<div align=center>"\
  "<table width=976 border=0 cellpadding=0 cellspacing=0>"\
  "<tr>"\
    "<td width=8 align=left valign=top background=/images/di22.jpg><img src=/images/youce4.jpg width=8 height=30/></td>"\
    "<td width=51 align=left valign=bottom background=/images/di22.jpg><img src=/images/youce33.jpg width=37 height=24/></td>"\
    "<td width=153 align=left valign=bottom background=/images/di22.jpg><font id=titleen>VLAN</font><font id=%s> %s</font></td>",search(lpublic,"title_style"),search(lpublic,"management"));
    fprintf(cgiOut,"<td width=690 align=right valign=bottom background=/images/di22.jpg>");

      fprintf(cgiOut,"<table width=130 border=0 cellspacing=0 cellpadding=0>"\
      "<tr>"\
      "<td width=62 align=center><input id=but type=submit name=submit_vlandetail style=background-image:url(/images/%s) value=""></td>",search(lpublic,"img_ok"));       
     if((cgiFormSubmitClicked("submit_vlandetail") != cgiFormSuccess) && (cgiFormSubmitClicked("delIP") != cgiFormSuccess)
	 	&& (cgiFormSubmitClicked("addIP") != cgiFormSuccess) && (cgiFormSubmitClicked("modID") != cgiFormSuccess)
	 	&&(cgiFormSubmitClicked("submit_igmp") != cgiFormSuccess)&&(cgiFormSubmitClicked("vlan_filter") != cgiFormSuccess)
	 	&&(cgiFormSubmitClicked("addTrunk") != cgiFormSuccess) && (cgiFormSubmitClicked("delTrunk") != cgiFormSuccess)
	 	&&(cgiFormSubmitClicked("Map_intf") != cgiFormSuccess) && (cgiFormSubmitClicked("vlan_bond_slot") != cgiFormSuccess)
  		&& (cgiFormSubmitClicked("vlan_unbond_slot") != cgiFormSuccess))
        fprintf(cgiOut,"<td width=62 align=left><a href=wp_configvlan.cgi?UN=%s&FLAG=%d&SER_TYPE=%s&SER_TEXT=%s target=mainFrame><img src=/images/%s border=0 width=62 height=20/></a></td>",encry,flag_for_vlan_show,SER_TYPE,SER_TEXT,search(lpublic,"img_cancel"));
      else                                         
        fprintf(cgiOut,"<td width=62 align=left><a href=wp_configvlan.cgi?UN=%s&FLAG=%d&SER_TYPE=%s&SER_TEXT=%s target=mainFrame><img src=/images/%s border=0 width=62 height=20/></a></td>",detail_encry,flag_for_vlan_show,SER_TYPE,SER_TEXT,search(lpublic,"img_cancel"));
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
                        "<td align=left id=tdleft background=/images/bottom_bg.gif style=\"border-right:0\"><font id=%s>%s<font></td>",search(lpublic,"menu_san"),search(lcontrol,"vlan_detail"));  /*突出显示*/
                      fprintf(cgiOut,"</tr>");

                    //modify by qiandawei 2008.08.7
                    if(strcmp(IDnew,"1")==0 || strcmp(IDForSearch,"1")==0 || strcmp(IDForSelfTurn,"1")==0)
        			{
                        for(i=0;i<21;i++)
        				{
                            fprintf(cgiOut,"<tr height=25>"\
                              "<td id=tdleft>&nbsp;</td>"\
                			"</tr>");
        				}

        			}
            		else
        			{
                        for(i=0;i<19;i++)
        				{
                            fprintf(cgiOut,"<tr height=25>"\
                              "<td id=tdleft>&nbsp;</td>"\
                			"</tr>");
        				}
            		}//end
                  fprintf(cgiOut,"</table>"\
              "</td>"\
              "<td align=left valign=top style=\"background-color:#ffffff; border-right:1px solid #707070; padding-left:30px; padding-top:10px\">"\
                      "<table width=640 height=295 border=0 cellspacing=0 cellpadding=0>"\
                	  	"<tr>"\
                        "<td id=sec1 style=\"border-bottom:2px solid #53868b;font-size:14px;padding-left:0px;padding-top:0px\">%s</td>",search(lcontrol,"Vlan_info"));
                        fprintf(cgiOut,"</tr>"\
                		"<tr>"\
                          "<td align=center valign=top  style=padding-top:18px>"\
                          "<table align=left width=400 border=0 cellspacing=0 cellpadding=0>");
                            untagNum=0;tagNum=0;
                            //fprintf(stderr,"0225-IDForSelfTurn=%s-IDnew=%s",IDForSelfTurn,IDnew);
                            if((cgiFormSubmitClicked("submit_vlandetail") != cgiFormSuccess) && (cgiFormSubmitClicked("delIP") != cgiFormSuccess) 
								&& (cgiFormSubmitClicked("addIP") != cgiFormSuccess) && (cgiFormSubmitClicked("modID") != cgiFormSuccess)
								&&(cgiFormSubmitClicked("submit_igmp") != cgiFormSuccess)&&(cgiFormSubmitClicked("vlan_filter") != cgiFormSuccess)
								&&(cgiFormSubmitClicked("addTrunk") != cgiFormSuccess) && (cgiFormSubmitClicked("delTrunk") != cgiFormSuccess)
								&&(cgiFormSubmitClicked("Map_intf") != cgiFormSuccess) && (cgiFormSubmitClicked("vlan_bond_slot") != cgiFormSuccess)
								&&(cgiFormSubmitClicked("vlan_unbond_slot") != cgiFormSuccess))
            					{
                                    IDTemp= strtoul(IDForSearch,&endptr,10);
                                    memset(v_InfoByID.vlanName,0,21);

                                    k=show_vlan_ByID(&v_InfoByID,IDTemp,&untagNum,&tagNum);

            					}
            				else
            					{
                                	if( result == 0 )
                					{	
                                        IDTemp= strtoul(IDForSelfTurn,&endptr,10);
                					}
                    				else
                					{
                                        IDTemp= strtoul(IDnew,&endptr,10);
                					}
                					
                					
                                    if(-1==parse_vlan_no(IDnew,&IDTemp) || result== -1)
                						{
                                            IDTemp= strtoul(IDForSelfTurn,&endptr,10);
                        					IDflag=1;
                						}
                                    //fprintf(stderr,"222result=%dIDnew=%s-IDForSelfTurn=%s-IDTemp=%d-IDflag=%d222",result,IDnew,IDForSelfTurn,IDTemp,IDflag);
                                    memset(v_InfoByID.vlanName,0,21);
									
                                    k=show_vlan_ByID(&v_InfoByID,IDTemp,&untagNum,&tagNum);
									
            					}
                            if(CMD_SUCCESS==k)
            					{
                                fprintf(cgiOut,"<tr height=30>"\
                                "<td align=left id=tdprompt width=80>%s: </td>","VLAN ID");

        						
                        		if( IDTemp == 1)
            					{
                                if((cgiFormSubmitClicked("submit_vlandetail") != cgiFormSuccess) && (cgiFormSubmitClicked("delIP") != cgiFormSuccess) 
									&& (cgiFormSubmitClicked("addIP") != cgiFormSuccess) && (cgiFormSubmitClicked("modID") != cgiFormSuccess)
									&&(cgiFormSubmitClicked("submit_igmp") != cgiFormSuccess)&&(cgiFormSubmitClicked("vlan_filter") != cgiFormSuccess)
									&&(cgiFormSubmitClicked("addTrunk") != cgiFormSuccess) && (cgiFormSubmitClicked("delTrunk") != cgiFormSuccess)
									&&(cgiFormSubmitClicked("Map_intf") != cgiFormSuccess) && (cgiFormSubmitClicked("vlan_bond_slot") != cgiFormSuccess)
  									&&(cgiFormSubmitClicked("vlan_unbond_slot") != cgiFormSuccess))
            						{
                                        fprintf(cgiOut,"<td align=left width=145 colspan=2><input type=text name=V_ID size=12 value=%s disabled=true style=background-color:#CCCCCC ></td>",IDForSearch);
            						}
                            	else if(IDflag==0)
            						{
                                        fprintf(cgiOut,"<td align=left  width=145 colspan=2><input type=text name=V_ID size=12 value=%s disabled=true style=background-color:#CCCCCC></td>",IDnew);
            						}
                            	else if(IDflag==1)
            						{
                                        fprintf(cgiOut,"<td align=left  width=145 colspan=2><input type=text name=V_ID size=12 value=%s disabled=true style=background-color:#CCCCCC></td>",IDForSelfTurn);
            						}
            					}
            					else
            					{
                                if((cgiFormSubmitClicked("submit_vlandetail") != cgiFormSuccess) && (cgiFormSubmitClicked("delIP") != cgiFormSuccess) 
									&& (cgiFormSubmitClicked("addIP") != cgiFormSuccess) && (cgiFormSubmitClicked("modID") != cgiFormSuccess) 
									&& (cgiFormSubmitClicked("submit_igmp") != cgiFormSuccess)&&(cgiFormSubmitClicked("vlan_filter") != cgiFormSuccess)
									&&(cgiFormSubmitClicked("addTrunk") != cgiFormSuccess) && (cgiFormSubmitClicked("delTrunk") != cgiFormSuccess)
									&&(cgiFormSubmitClicked("Map_intf") != cgiFormSuccess) && (cgiFormSubmitClicked("vlan_bond_slot") != cgiFormSuccess)
  									&&(cgiFormSubmitClicked("vlan_unbond_slot") != cgiFormSuccess))
            						{
                                        fprintf(cgiOut,"<td align=left  width=145 colspan=2><input type=text name=V_ID size=12 value=%s></td>",IDForSearch);
            						}
                            	else if(IDflag==0)
            						{
                                        fprintf(cgiOut,"<td align=left  width=145 colspan=2><input type=text name=V_ID size=12 value=%s></td>",IDnew);
            						}
                            	else if(IDflag==1)
            						{
                                        fprintf(cgiOut,"<td align=left  width=145 colspan=2><input type=text name=V_ID size=12 value=%s></td>",IDForSelfTurn);
            						}
            					}

        						
                                fprintf(cgiOut,"<td align=right width=80><input type=submit style=width:70px; height:36px  border=0 name=modID style=background-image:url(/images/SubBackGif.gif) value=\"%s\"></td>",search(lcontrol,"modify_VID"));
                                fprintf(cgiOut,"<td>&nbsp;</td>");
                               fprintf(cgiOut,"</tr>"\
                              "<tr height=30>"\
                                "<td align=left id=tdprompt width=80>%s: </td>",search(lcontrol,"vlan_name"));
                                fprintf(cgiOut,"<td align=left colspan=4 width=270>%s</td>",v_InfoByID.vlanName);
                              fprintf(cgiOut,"</tr>");
                              fprintf(cgiOut,"<tr height=30>"\
                                "<td align=left id=tdprompt width=80>%s: </td>",search(lcontrol,"trunk_list"));
                                fprintf(cgiOut,"<td align=left>");
                                fprintf(cgiOut, "<select name=\"trunklist\">");

                                result1=show_trunk_list(&head,&Tnum);
                        		q=head.next;
                            	while(Tnum>0)
                          		{
                                    fprintf(cgiOut,"<option value1=%d>%d",q->trunkId,q->trunkId);
                                    Tnum=Tnum-1;
                                	q=q->next;
            					}
                                fprintf(cgiOut, "</select>\n");
                                fprintf(cgiOut,"</td>");
                                fprintf(cgiOut,"<td>");
                                fprintf(cgiOut, "<select name=\"trunkTag\">"\
                                "<option value=untag>untag"\
                                "<option value=tag>tag"\
                        		"</select>\n"\
                    			"</td>");
        						
                                fprintf(cgiOut,"<td align=right><input type=submit style=width:70px; height:36px  border=0 name=addTrunk style=background-image:url(/images/SubBackGif.gif) value=\"%s\"></td>",search(lcontrol,"add"));
                                fprintf(cgiOut,"<td align=right><input type=submit style=width:70px; height:36px  border=0 name=delTrunk style=background-image:url(/images/SubBackGif.gif) value=\"%s\"></td>",search(lcontrol,"delete"));
                                fprintf(cgiOut,"</tr>");
                              fprintf(cgiOut,"</table>"\
                    		  "</td>"\
                    		  "</tr>"\
                			  "<tr>"\
                                "<td align=center valign=top  style=padding-top:8px>"\
                              "<table align=left width=400 border=0 cellspacing=0 cellpadding=0>");
                              fprintf(cgiOut,"<tr height=30>"\
                              "<td width=70>%s:</td>",search(lcontrol,"vlan_filter"));
                              fprintf(cgiOut,"<td align=left width=50px>"\
                              "<select name=Enable_mode>"\
                              "<option value=enable>Enable"\
                              "<option value=disable>Disable"\
                        	  "</select>"\
                    		  "</td>");
            				  
                              //"<select name=filter_type>");
            				  
                              fprintf(cgiOut,"<td align=left width=160px style=padding-left:5px>");
                              fprintf(cgiOut, "<input type=\"checkbox\" name=\"filter_type\" value=unkown-unicast>%s",search(lcontrol,"unkown_unicast"));
                              fprintf(cgiOut,"</td>");
                              fprintf(cgiOut,"<td align=right style=padding-left:10px width=90><input type=submit style=width:70px; height:36px  border=0 name=vlan_filter style=background-image:url(/images/SubBackGif.gif) value=\"%s\"></td>",search(lcontrol,"filter"));
                              fprintf(cgiOut,"</tr>");

                              fprintf(cgiOut,"<tr>");
                              fprintf(cgiOut,"<td colspan=2 width=195>&nbsp;</td>");
                              fprintf(cgiOut,"<td align=left width=160px style=padding-left:5px colspan=2>");
                              fprintf(cgiOut, "<input type=\"checkbox\" name=\"filter_type\" value=unreg-ipv4-multicast>%s",search(lcontrol,"unreg_ipv4_multicast"));
                              fprintf(cgiOut,"</td>");
                              fprintf(cgiOut,"</tr>");

                              fprintf(cgiOut,"<tr>");
                              fprintf(cgiOut,"<td colspan=2 width=195>&nbsp;</td>");
                              fprintf(cgiOut,"<td align=left width=160px style=padding-left:5px colspan=2>");
                              fprintf(cgiOut, "<input type=\"checkbox\" name=\"filter_type\" value=unreg-ipv6-multicast>%s",search(lcontrol,"unreg_ipv6_multicast"));
                              fprintf(cgiOut,"</td>");
                              fprintf(cgiOut,"</tr>");

                              fprintf(cgiOut,"<tr>");
                              fprintf(cgiOut,"<td colspan=2 width=195>&nbsp;</td>");
                              fprintf(cgiOut,"<td align=left width=160px style=padding-left:5px colspan=2>");
                              fprintf(cgiOut, "<input type=\"checkbox\" name=\"filter_type\" value=unreg-nonIp-multicast>%s",search(lcontrol,"unreg_nonIp_multicast"));
                              fprintf(cgiOut,"</td>");
                              fprintf(cgiOut,"</tr>");

                              fprintf(cgiOut,"<tr>");
                              fprintf(cgiOut,"<td colspan=2 width=195>&nbsp;</td>");
                              fprintf(cgiOut,"<td align=left width=160px style=padding-left:5px colspan=2>");
                              fprintf(cgiOut, "<input type=\"checkbox\" name=\"filter_type\" value=unreg-ipv4-broadcast>%s",search(lcontrol,"unreg_ipv4_broadcast"));
                              fprintf(cgiOut,"</td>");
                              fprintf(cgiOut,"</tr>");

                              fprintf(cgiOut,"<tr>");
                              fprintf(cgiOut,"<td colspan=2 width=195>&nbsp;</td>");
                              fprintf(cgiOut,"<td align=left width=160px style=padding-left:5px colspan=2>");
                              fprintf(cgiOut, "<input type=\"checkbox\" name=\"filter_type\" value=unreg-nonIpv4-broadcast>%s",search(lcontrol,"unreg_nonIpv4_broadcast"));
                              fprintf(cgiOut,"</td>");
                              fprintf(cgiOut,"</tr>");
            				  
                              /*fprintf(cgiOut, "<input type=\"checkbox\" name=\"filter_type\" value=unreg-ipv4-multicast>%s",search(lcontrol,"unreg_ipv4_multicast"));
                              fprintf(cgiOut, "<input type=\"checkbox\" name=\"filter_type\" value=unreg-ipv6-multicast>%s",search(lcontrol,"unreg_ipv6_multicast"));
                              fprintf(cgiOut, "<input type=\"checkbox\" name=\"filter_type\" value=unreg-nonIp-multicast>%s",search(lcontrol,"unreg_nonIp_multicast"));
                              fprintf(cgiOut, "<input type=\"checkbox\" name=\"filter_type\" value=unreg-ipv4-broadcast>%s",search(lcontrol,"unreg_ipv4_broadcast"));
                              fprintf(cgiOut, "<input type=\"checkbox\" name=\"filter_type\" value=unreg-nonIpv4-broadcast>%s",search(lcontrol,"unreg_nonIpv4_broadcast"));
                              fprintf(cgiOut,"</td>");*/
            				  
                              fprintf(cgiOut,"</table>"\
                    		  "</td>"\
                    		  "</tr>"\
                			  "<tr>"\
                                "<td id=sec1 style=\"border-bottom:2px solid #53868b;font-size:14px;padding-left:0px;padding-top:0px\">%s</td>",search(lcontrol,"Vlan_L3_info"));
                                fprintf(cgiOut,"</tr>"\
                				"<tr>"\
                                "<td align=center valign=top  style=padding-top:8px>"\
                              "<table align=left width=400 border=0 cellspacing=0 cellpadding=0>");
                              fprintf(cgiOut,"<tr height=30>"\
                              "<td align=left id=tdprompt width=130>%s: </td>",search(lcontrol,"Vlan_L3_mode"));
                              fprintf(cgiOut,"<td align=left>"\
                              "<select name=advanced_routing >"\
                              "<option value=normal>Normal Mode"\
                              "<option value=advanced>Advanced Routing Mode"\
                        	  "</select>"\
                    		  "</td>"\
                              "<td align=left style=padding-left:10px><input type=submit style=width:80px; height:36px border=0 name=Map_intf style=background-image:url(/images/SubBackGif.gif) value=\"%s\"></td>",search(lcontrol,"map_intf"));
                                fprintf(cgiOut,"</td>"\
                    		  "</tr>");
                                fprintf(cgiOut,"<tr height=30>"\
                                "<td align=left id=tdprompt width=130>VLAN IP: </td>"\
                                "<td align=left  style=padding-left:3px width=170>"\
                                 "<div style=\"border-width:1;border-color:#a5acb2;border-style:solid;width:140;font-size:9pt\">");
                                  fprintf(cgiOut,"<input type=text name=vlan_ip1 maxlength=3 class=a3 onKeyUp=\"mask(this,%s)\" onbeforepaste=mask_c()>.",search(lpublic,"ip_error"));
                                  fprintf(cgiOut,"<input type=text name=vlan_ip2 maxlength=3 class=a3 onKeyUp=\"mask(this,%s)\" onbeforepaste=mask_c()>.",search(lpublic,"ip_error"));
                                  fprintf(cgiOut,"<input type=text name=vlan_ip3 maxlength=3 class=a3 onKeyUp=\"mask(this,%s)\" onbeforepaste=mask_c()>.",search(lpublic,"ip_error"));
                                  fprintf(cgiOut,"<input type=text name=vlan_ip4 maxlength=3 class=a3 onKeyUp=\"mask(this,%s)\" onbeforepaste=mask_c()>",search(lpublic,"ip_error"));
                                fprintf(cgiOut,"</div>"\
                				"</td>"\
                                "<td align=left style=padding-left:10px><input type=submit style=width:80px; height:36px border=0 name=addIP style=background-image:url(/images/SubBackGif.gif) value=\"%s\"></td>",search(lcontrol,"add_L3_IP"));
                                fprintf(cgiOut,"</tr>");
            				  
                                fprintf(cgiOut,"<tr height=30>"\
                                "<td align=left id=tdprompt>%s: </td>",search(lcontrol,"mask"));
                                fprintf(cgiOut,"<td align=left style=padding-left:3px>"\
                                "<div style=\"border-width:1;border-color:#a5acb2;border-style:solid;width:140;font-size:9pt\">");
                                    fprintf(cgiOut,"<input type=text name=vlan_mask1 maxlength=3 class=a3 onKeyUp=\"mask(this,%s)\" onbeforepaste=mask_c()>.",search(lpublic,"ip_error"));
                                    fprintf(cgiOut,"<input type=text name=vlan_mask2 maxlength=3 class=a3 onKeyUp=\"mask(this,%s)\" onbeforepaste=mask_c()>.",search(lpublic,"ip_error"));
                                    fprintf(cgiOut,"<input type=text name=vlan_mask3 maxlength=3 class=a3 onKeyUp=\"mask(this,%s)\" onbeforepaste=mask_c()>.",search(lpublic,"ip_error"));
                                    fprintf(cgiOut,"<input type=text name=vlan_mask4 maxlength=3 class=a3 onKeyUp=\"mask(this,%s)\" onbeforepaste=mask_c()>",search(lpublic,"ip_error"));
                                    fprintf(cgiOut,"</div>"\
                    				"</td>"\
                                    "<td align=left style=padding-left:10px><input type=submit style=width:80px; height:36px  border=0 name=delIP style=background-image:url(/images/SubBackGif.gif) value=\"%s\"></td>",search(lcontrol,"delete_L3_IP"));
            						
                                fprintf(cgiOut,"</tr>");
                                fprintf(cgiOut,"</table>"\
                    		    "</td>"\
                    		    "</tr>");

								fprintf(cgiOut,"<tr>"\
                                "<td id=sec1 style=\"border-bottom:2px solid #53868b;font-size:14px;padding-left:0px;padding-top:0px\">VLAN %s SLOT</td>",search(lcontrol,"bind"));
                                fprintf(cgiOut,"</tr>"\
                				"<tr>"\
                                "<td align=center valign=top  style=padding-top:8px>"\
	                              "<table align=left width=400 border=0 cellspacing=0 cellpadding=0>"\
	                     		  "<tr height=30>"\
		                              "<td align=left id=tdprompt>SLOT:</td>"\
		                              "<td align=left>"\
			                              "<select name=slot_id style=width:100px>"\
			                              "<option value=></option>");
										  for(i=1; i<=SLOT_MAX_NUM; i++)
										  {
											  fprintf(cgiOut,"<option value=%d>%d</option>",i,i);
										  }
			                        	  fprintf(cgiOut,"</select>"\
		                    		  "</td>"\
		                    		  "<td align=left style=padding-left:10px><input type=submit style=width:80px; height:36px  border=0 name=vlan_bond_slot style=background-image:url(/images/SubBackGif.gif) value=\"%s\"></td>",search(lcontrol,"bind"));
	                     		  fprintf(cgiOut,"</tr>"\
	                     		  "</tr>"\
	                     		  "<tr height=30>"\
		                              "<td align=left id=tdprompt>CPU NO:</td>"\
		                              "<td align=left>"\
			                              "<select name=cpu_no style=width:100px>"\
			                              "<option value=></option>");
										  for(i=1; i<=2; i++)
										  {
											  fprintf(cgiOut,"<option value=%d>%d</option>",i,i);
										  }
			                        	  fprintf(cgiOut,"</select>"\
		                    		  "</td>"\
		                    		  "<td align=left style=padding-left:10px><input type=submit style=width:80px; height:36px  border=0 name=vlan_unbond_slot style=background-image:url(/images/SubBackGif.gif) value=\"%s\"></td>",search(lcontrol,"unbind"));
	                     		  fprintf(cgiOut,"</tr>"\
	                     		  "<tr height=30>"\
		                              "<td align=left id=tdprompt>CPU PORT:</td>"\
		                              "<td align=left>"\
			                              "<select name=cpu_port style=width:100px>"\
			                              "<option value=></option>");
										  for(i=1; i<=8; i++)
										  {
											  fprintf(cgiOut,"<option value=%d>%d</option>",i,i);
										  }
			                        	  fprintf(cgiOut,"</select>"\
		                    		  "</td>"\
	                     		  "</tr>"\
                                  "</table>"\
                    		    "</td>"\
                    		    "</tr>");
                                     						
                                fprintf(cgiOut,"<tr>"\
                                "<td align=center valign=top  style=padding-top:8px>"\
                                "<table align=left width=400 border=0 cellspacing=0 cellpadding=0>");
        					}        					
                              if((cgiFormSubmitClicked("submit_vlandetail") != cgiFormSuccess) && (cgiFormSubmitClicked("delIP") != cgiFormSuccess)
							  	&& (cgiFormSubmitClicked("addIP") != cgiFormSuccess) && (cgiFormSubmitClicked("modID") != cgiFormSuccess)
							  	&&(cgiFormSubmitClicked("submit_igmp") != cgiFormSuccess)&&(cgiFormSubmitClicked("vlan_filter") != cgiFormSuccess)
							  	&&(cgiFormSubmitClicked("addTrunk") != cgiFormSuccess) && (cgiFormSubmitClicked("delTrunk") != cgiFormSuccess)
							  	&&(cgiFormSubmitClicked("Map_intf") != cgiFormSuccess) && (cgiFormSubmitClicked("vlan_bond_slot") != cgiFormSuccess)
								&&(cgiFormSubmitClicked("vlan_unbond_slot") != cgiFormSuccess))
            				  {
                                fprintf(cgiOut,"<td colspan=2><input type=hidden name=encry_detail value=%s></td>",encry);
                                fprintf(cgiOut,"<td colspan=2><input type=hidden name=encry_IDOld value=%s></td>",IDForSearch);
                                fprintf(cgiOut,"<td colspan=2><input type=hidden name=FLAG value=%s></td>",flag_href);
                                fprintf(cgiOut,"<td colspan=2><input type=hidden name=SER_TYPE value=%s></td>",SER_TYPE);
                                fprintf(cgiOut,"<td colspan=2><input type=hidden name=SER_TEXT value=%s></td>",SER_TEXT);
            				  }
                			  else
            				  {
                                fprintf(cgiOut,"<td><input type=hidden name=encry_detail value=%s></td>",detail_encry);
                            	if( result == 0 )
                                    fprintf(cgiOut,"<td><input type=hidden name=encry_IDOld value=%s></td>",IDnew);
            					else
                                    fprintf(cgiOut,"<td><input type=hidden name=encry_IDOld value=%s></td>",IDForSearch);

                                fprintf(cgiOut,"<td colspan=2><input type=hidden name=FLAG value=%s></td>",flag_href);
                                fprintf(cgiOut,"<td colspan=2><input type=hidden name=SER_TYPE value=%s></td>",SER_TYPE);
                                fprintf(cgiOut,"<td colspan=2><input type=hidden name=SER_TEXT value=%s></td>",SER_TEXT);
            				  }
                              fprintf(cgiOut,"</tr>"\
                    		"</table>");
                          fprintf(cgiOut,"</td>"\
                		"</tr>");
        				  
                fprintf(cgiOut,"</table>"\

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
free(product);
for(i=0;i<24;i++)
    {
        free(v_InfoByID.slot_port_tag[i]);
        free(v_InfoByID.slot_port_untag[i]);
    }
free(SER_TYPE);
free(SER_TEXT);
free(flag_href);
free(v_InfoByID.vlanName);
free(encry);
free(IDnew);
free(IDForSelfTurn);
free(IDForSearch);
free(advanced_routing);
free(Enable_mode);
free(filter_type);
free(trunkID);
free(trunkTag);
release(lpublic);  
release(lcontrol);

return 0;
}

            				  
int setvlanID_hand(char* vidOld,char* vidnew)
{
    unsigned short vidOldTemp;
    vidOldTemp=atoi(vidOld);
    
    //vidnewTemp=atoi(vidnew);
    return setVID(vidOldTemp, vidnew);
}

            				  
double MaskChange(char * masksub)
{
    int tempMask=atoi(masksub);
    double ret;
    ret=8-(log10(256-(double)tempMask)/log10(2));
            				  
    return ret; 
}
            				  
int modifyVlanIp(char *vid,struct list *lcontrol)
{
    struct list *lpublic;   /*解析public.txt文件的链表头*/
    lpublic=get_chain_head("../htdocs/text/public.txt");

    char ip1[4],ip2[4],ip3[4],ip4[4];
    char mask1[4],mask2[4],mask3[4],mask4[4];
    unsigned long mask[4];
    unsigned long vlan_mask;
    char vlan_ip[20];
    memset(vlan_ip,0,20);
    char vlan_ipMask[40];
    memset(vlan_ipMask,0,40);
    char command[100];
    memset(command,0,100);
            						  
    memset(ip1,0,4);
    cgiFormStringNoNewlines("vlan_ip1",ip1,4);	 
    strcat(vlan_ip,ip1);
    strcat(vlan_ip,".");
    memset(ip2,0,4);
    cgiFormStringNoNewlines("vlan_ip2",ip2,4); 
    strcat(vlan_ip,ip2);  
    strcat(vlan_ip,".");
    memset(ip3,0,4);
    cgiFormStringNoNewlines("vlan_ip3",ip3,4); 
    strcat(vlan_ip,ip3);  
    strcat(vlan_ip,".");
    memset(ip4,0,4);
    cgiFormStringNoNewlines("vlan_ip4",ip4,4);		
    strcat(vlan_ip,ip4);
    //fprintf(stderr"vlan_ip=%s",vlan_ip);      				  
    double maskLen,k;
    int maskLenTemp;
    memset(mask1,0,4);
    cgiFormStringNoNewlines("vlan_mask1",mask1,4);
    mask[0]=atoi(mask1);
    k=MaskChange(mask1);
    if(k!=-1)
        maskLen=k;
    else return -1;
            						  
    memset(mask2,0,4);
    cgiFormStringNoNewlines("vlan_mask2",mask2,4);
    mask[1]=atoi(mask2);
    k=MaskChange(mask2);
    if(k!=-1)
        maskLen=maskLen+k;
    else return -1;
            						  
    memset(mask3,0,4);
    cgiFormStringNoNewlines("vlan_mask3",mask3,4);
    mask[2]=atoi(mask3);
    k=MaskChange(mask3);
    if(k!=-1)
        maskLen=maskLen+k;
    else return -1;
            						  
    memset(mask4,0,4);
    cgiFormStringNoNewlines("vlan_mask4",mask4,4);
    mask[3]=atoi(mask4);
    k=MaskChange(mask4);
    if(k!=-1)
        maskLen=maskLen+k;
    else return -1;
            				  
    vlan_mask=(mask[0] << 24) + (mask[1] << 16) + (mask[2] << 8) + mask[3];
    if(0==checkMark(vlan_mask))
    	{
            ShowAlert(search(lcontrol,"mask_incorrect"));
            return 0;
    	}
     if( !(strcmp(ip1,"")&&strcmp(ip2,"")&&strcmp(ip3,"")&&strcmp(ip4,"")
        &&strcmp(mask1,"")&&strcmp(mask2,"")&&strcmp(mask3,"")&&strcmp(mask4,"")) )
     	{
        ShowAlert(search(lcontrol,"ip_null"));
        return 0;
     	}
    maskLenTemp=(int)maskLen;
    unsigned short IDTemp;
    IDTemp=atoi(vid);
    char intername[20];
    memset(intername,0,20);
    sprintf(intername,"vlan%d",IDTemp);
    sprintf(vlan_ipMask,"%s/%d",vlan_ip,maskLenTemp);
            						  	
    strcat(command,"set_intf_ip.sh");
    strcat(command," ");
    strcat(command,intername);
    strcat(command," ");
    strcat(command,vlan_ipMask);
    strcat(command," ");
    strcat(command,">/var/run/apache2/vlan_ip_error_output.txt");
    int status = system(command); 	 
    int ret = WEXITSTATUS(status);
    FILE *fd;
    char  temp[60];
    memset(temp,0,60);
        					 
    if((fd=fopen("/var/run/apache2/vlan_ip_error_output.txt","r"))==NULL)
    	{
            ShowAlert(search(lpublic,"error_open"));
            return 0;
    	}
//  int i=0;
    while((fgets(temp,60,fd)) != NULL)
    	{
            if(strstr(temp,"there already has the same network")!=NULL)
        		{
                    ShowAlert(search(lcontrol,"there_already_network"));
                    release(lpublic);
                	return -1;
        		}
    	}
	fclose(fd);
    if(0==ret)
        ShowAlert(search(lcontrol,"IP_SUCCESS"));
    else ShowAlert(search(lcontrol,"IP_fail"));  

    release(lpublic);
    return 0;
}
        		  
            				  
// 输入已经转换为按主机字节序的 unsigned long
int checkMark(unsigned long mark)
{
    return ((((mark ^ (mark - 1)) >> 1) ^ mark) == -1);
}

void addordel_trunk_alert(int retu,struct list *lcontrol)
{
	switch(retu)
	{
		case -2:
			ShowAlert(search(lcontrol,"illegal_input"));
		break;    
		case -3:
			ShowAlert(search(lcontrol,"VLAN_NOT_EXITSTS"));
		break;
		case -4:
			ShowAlert(search(lcontrol,"trunk_not_exist"));
		break;
		case -5:
			ShowAlert(search(lcontrol,"vlan_Already_trunk"));
		break;
		case -6:
			ShowAlert(search(lcontrol,"port_NotExist"));
		break;
		case -7:
			ShowAlert(search(lcontrol,"vlan_had_other_trunk"));
		break;
		case -8:
			ShowAlert(search(lcontrol,"trunk_no_member"));
		break;
		case -9:
			ShowAlert(search(lcontrol,"HW_error"));	
		break;
		case 1:
			ShowAlert(search(lcontrol,"add_trunk_suc"));
		break;
		case 2:
			ShowAlert(search(lcontrol,"delete_trunk_succ"));
		break;
		case -10:
			ShowAlert(search(lcontrol,"trunk_conflict"));	
		break;
		default:
			ShowAlert(search(lcontrol,"opt_fail"));
		break;
	}
}
