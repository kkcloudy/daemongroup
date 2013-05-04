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
* wp_con_aclgrp.c
*
*
* CREATOR:
* autelan.software.Network Dep. team
* zhouym@autelan.com
*
* DESCRIPTION:
* system contrl for acl group config
*
*
*******************************************************************************/
#include <stdio.h>
#include "cgic.h"
#include <string.h>
#include <stdlib.h>
#include "ws_usrinfo.h"
#include "ws_init_dbus.h"
#include "ws_dcli_vlan.h"
#include "ws_dcli_acl.h"
#include "ws_dcli_portconf.h"
#define ACL_NUM 40960
#define MAX_ACL_NUM 4096

char * bind_mode[] = {
	"VlanId",
	"PortNo"
	
};


int ShowConfigPolicyPage(); 
int bind_hand(struct list *lcontrol,char * grpindex,char * grptype);
int unbind_hand(struct list *lcontrol,char * grpindex,char * grptype); 

int cgiMain()
{
 ShowConfigPolicyPage();
 return 0;
}



int ShowConfigPolicyPage()
{
	ccgi_dbus_init();
	struct list *lpublic;	/*解析public.txt文件的链表头*/
	struct list *lcontrol; 	/*解析help.txt文件的链表头*/
	lpublic=get_chain_head("../htdocs/text/public.txt");
    	lcontrol=get_chain_head("../htdocs/text/control.txt"); 
	char* i_char=(char *)malloc(10);
	char *encry=(char *)malloc(BUF_LEN);			  
	char *str;
	char * index=(char *)malloc(10);
	memset(index,0,10);
	unsigned int indextemp=0;
	char *endptr = NULL;  
	int i;
	char configpolicy_encry[BUF_LEN];
	memset(encry,0,BUF_LEN);
	//*********add show port*************
	ETH_SLOT_LIST  head,*p;
    	ETH_PORT_LIST *pp;
	int ret = 0;
	int slot_mun = 0;
	//********************************

	//**********new***************
	struct vlan_info_simple receive_vlan[MAX_VLAN_NUM];
	int port_num[MAX_VLAN_NUM],vlanNum=0;

		for(i=0;i<4095;i++)
	  	{
	  		receive_vlan[i].vlanId=0;
			receive_vlan[i].vlanStat=0;
			receive_vlan[i].vlanName=(char*)malloc(21);
			memset(receive_vlan[i].vlanName,0,21);
	  		port_num[i]=0;
	  		//locat[i]=0;
	  	}
		struct acl_info receive_acl[MAX_ACL_NUM];
		 for(i=0;i<MAX_ACL_NUM;i++)
	  	{
		     	receive_acl[i].ruleIndex=0;
		     	receive_acl[i].groupIndex=0;	
		     	receive_acl[i].ruleType=(char *)malloc(20);
		     	memset(receive_acl[i].ruleType,0,20);
		     	receive_acl[i].protype=(char *)malloc(20);
		     	memset(receive_acl[i].protype,0,20);
		     	receive_acl[i].dip=(char *)malloc(20);
		     	memset(receive_acl[i].dip,0,20);
		     	receive_acl[i].sip=(char *)malloc(20);
		     	memset(receive_acl[i].sip,0,20);
		     	receive_acl[i].srcport=0; 
		     	receive_acl[i].dstport=0;
		     	receive_acl[i].icmp_code=0;
		     	receive_acl[i].icmp_type=0;
		     	receive_acl[i].actype=(char *)malloc(50);
		     	memset(receive_acl[i].actype,0,50);
		     	receive_acl[i].dmac=(char *)malloc(30);
		     	memset(receive_acl[i].dmac,0,30);
		     	receive_acl[i].smac=(char *)malloc(30);
		     	memset(receive_acl[i].smac,0,30);
		     	receive_acl[i].vlanid=0; 
		     	receive_acl[i].source_port=(char *)malloc(30);
		     	memset(receive_acl[i].source_port,0,30);
		     	receive_acl[i].redirect_port=(char *)malloc(30);
		     	memset(receive_acl[i].redirect_port,0,30);
			receive_acl[i].analyzer_port=(char *)malloc(30);
			memset(receive_acl[i].analyzer_port,0,30);
			receive_acl[i].policerId=0;
			receive_acl[i].up=0;
			receive_acl[i].dscp=0;
			receive_acl[i].egrUP=0;
			receive_acl[i].egrDSCP=0;
			receive_acl[i].modifyDSCP=0;
			receive_acl[i].modifyUP=0;
			receive_acl[i].precedence=0;
			receive_acl[i].SubQosMakers=0;
			receive_acl[i].appendIndex=0;
			receive_acl[i].qosprofileindex=0;
			receive_acl[i].upmm=(char *)malloc(30);
			memset(receive_acl[i].upmm,0,30);
			receive_acl[i].dscpmm=(char *)malloc(30);
			memset(receive_acl[i].dscpmm,0,30);
	  	}

		int aclNum=0;
		char * iotype=(char *)malloc(20);
		memset(iotype,0,20);

		struct acl_groupone_info * p_oneinfo;
  		p_oneinfo = (struct acl_groupone_info *)malloc(sizeof(struct acl_groupone_info));
		if(p_oneinfo==NULL)
	  	{
	  		ShowErrorPage("no space");
			return 0;
	  	}

		
	//*******************************
	
	char * grpType=(char * )malloc(10);
	memset(grpType,0,10);
	char * grpTypeLA=(char * )malloc(10);
	memset(grpTypeLA,0,10);
	char * ACL_RULE=(char * )malloc(10);
	memset(ACL_RULE,0,10);
	char * typeBind=(char * )malloc(10);
	memset(typeBind,0,10);
	//***********NEW**********************
	char * typeUnBind=(char *)malloc(10);
	memset(typeUnBind,0,10);
	//**********************************
	char * aclRuleIndex=(char * )malloc(10);
	memset(aclRuleIndex,0,10);
	if(cgiFormStringNoNewlines("UN", encry, BUF_LEN)!=cgiFormNotFound )  /*首次进入该页*/
	{

		cgiFormStringNoNewlines("INDEX",index,10);
		cgiFormStringNoNewlines("TYPE",grpType,10);
	  str=dcryption(encry);
	  if(str==NULL)
	  {
		ShowErrorPage(search(lpublic,"ill_user")); 		 /*用户非法*/
		return 0;
	  }
	  memset(configpolicy_encry,0,BUF_LEN);					 /*清空临时变量*/
	}
	else
	{
		
       	cgiFormStringNoNewlines("encry_configaclgrp",encry,BUF_LEN);
       	cgiFormStringNoNewlines("INDEXLA",index,10);
      		 cgiFormStringNoNewlines("TYPELA",grpType,10);

       indextemp=strtoul(index,&endptr,10);
   	}

		if(strcmp(grpType,"0")==0)
		{
			strcpy(iotype,"ingress");
		}
		else if(strcmp(grpType,"1")==0)
		{
			strcpy(iotype,"egress");
		}
	/////////////add new code here/////////////////////
	int disableFlag = 0;
	struct acl_groupone_info * dis_or_en;
	dis_or_en = (struct acl_groupone_info *)malloc(sizeof(struct acl_groupone_info));
	if(dis_or_en==NULL)
  	{
  		ShowErrorPage("no space");
		return 0;
  	}
	show_acl_group_one(iotype,index,dis_or_en);
	if((dis_or_en->bind_by_port_count!=0)||(dis_or_en->vlan_count!=0))
	{
		disableFlag = 1;
	}
	else
	{
		disableFlag = 0;
	}
	////////////////////////////////////////////////

	
   	    cgiFormStringNoNewlines("acl_index",aclRuleIndex,10);
   cgiHeaderContentType("text/html");
	fprintf(cgiOut,"<html xmlns=\"http://www.w3.org/1999/xhtml\"><head>\n");
  fprintf(cgiOut,"<meta http-equiv=Content-Type content=text/html; charset=gb2312>\n");
  fprintf(cgiOut,"<title>%s</title>\n",search(lcontrol,"acl_conf"));
  fprintf(cgiOut,"<link rel=stylesheet href=/style.css type=text/css>\n"\
  "<style type=text/css>\n"\
	  "#div1{ width:42px; height:18px; border:1px solid #666666; background-color:#f9f8f7;}\n"\
	  "#div2{ width:40px; height:15px; padding-left:5px; padding-top:3px}\n"\
	  "#link{ text-decoration:none; font-size: 12px}\n"\
	 "</style>\n"\
  "</head>\n"\
  			"<script type=\"text/javascript\">\n"\
			  "function popMenu(objId)\n"\
			  "{\n"\
				 "var obj = document.getElementById(objId);\n"\
				 "if (obj.style.display == 'none')\n"\
				 "{\n"\
				   	"obj.style.display = 'block';\n"\
				 "}\n"\
				 	"else\n"\
				 "{\n"\
				   "obj.style.display = 'none';\n"\
				 "}\n"\
			 "}\n"\
			 "</script>\n"\
  "<body>\n");
  if(cgiFormSubmitClicked("submit_configaclgrp") == cgiFormSuccess)
  {

     fprintf( cgiOut, "<script type='text/javascript'>\n" );
 	 fprintf( cgiOut, "window.location.href='wp_aclgrplist.cgi?UN=%s';\n", encry);
 	 fprintf( cgiOut, "</script>\n" );
  }

		  if(cgiFormSubmitClicked("submit_delacl")==cgiFormSuccess)
		  {
		  	unsigned int temp1=0,temp2=0;
	  	
		  	if(strcmp(index,"")!=0)
		  	   temp1=atoi(index);
		  	if(strcmp(grpType,"")!=0)
		  	   temp2=atoi(grpType);
		  	if(strcmp(aclRuleIndex,"")!=0)
		  	{
		  		add_rule_group("delete",aclRuleIndex,temp1,temp2,lcontrol);
		  	}
			else
			{
				ShowAlert(search(lcontrol,"select_acl"));
			}
		  }
    if(strcmp(grpType,"1")==0)
   	{
   		memset(grpTypeLA,0,10);
   		strcpy(grpTypeLA,"egress");
   	}
   	else if(strcmp(grpType,"0")==0)
   	{
   		memset(grpTypeLA,0,10);
   		strcpy(grpTypeLA,"ingress");
   	}
   	int dir=atoi(grpType);

	 char url_temp[512];
	 if(cgiFormSubmitClicked("submit_bind") == cgiFormSuccess)
	 {
		bind_hand(lcontrol,index,grpTypeLA);
		sprintf( url_temp, "wp_con_aclgrp.cgi?UN=%s&INDEX=%s&TYPE=%s",encry,index,grpType);
		fprintf( cgiOut, "<script type='text/javascript'>\n" );
		fprintf( cgiOut, "window.location.href='%s';\n", url_temp );
		fprintf( cgiOut, "</script>\n" );

	 }
	 if(cgiFormSubmitClicked("submit_unbind") == cgiFormSuccess)
	 {
	   	unbind_hand(lcontrol,index,grpTypeLA);
		sprintf( url_temp, "wp_con_aclgrp.cgi?UN=%s&INDEX=%s&TYPE=%s",encry,index,grpType);
		fprintf( cgiOut, "<script type='text/javascript'>\n" );
		fprintf( cgiOut, "window.location.href='%s';\n", url_temp );
		fprintf( cgiOut, "</script>\n" );
	 }
	 if(cgiFormSubmitClicked("submit_addrule") == cgiFormSuccess)
	 {
	   cgiFormStringNoNewlines("ACL_RULE",ACL_RULE,10);
	   if(strcmp(ACL_RULE,"")!=0)
	   {
	   	add_rule_group("add",ACL_RULE,indextemp,dir,lcontrol);
	   }
	   else
	   {
	   	ShowAlert(search(lcontrol,"acl_index_not_null"));
	   }
	 }
  fprintf(cgiOut,"<form method=post>\n"\
  "<div align=center>\n"\
  "<table width=976 border=0 cellpadding=0 cellspacing=0>\n"\
  "<tr>\n"\
    "<td width=8 align=left valign=top background=/images/di22.jpg><img src=/images/youce4.jpg width=8 height=30/></td>\n"\
    "<td width=51 align=left valign=bottom background=/images/di22.jpg><img src=/images/youce33.jpg width=37 height=24/></td>\n"\
    "<td width=153 align=left valign=bottom background=/images/di22.jpg><font id=titleen>ACL</font><font id=%s> %s</font></td>\n",search(lpublic,"title_style"),search(lpublic,"management"));
    fprintf(cgiOut,"<td width=690 align=right valign=bottom background=/images/di22.jpg>\n");
    	  fprintf(cgiOut,"<table width=130 border=0 cellspacing=0 cellpadding=0>\n"\
          "<tr>\n"\
          "<td width=62 align=center><input id=but type=submit name=submit_configaclgrp style=background-image:url(/images/%s) value=""></td>\n",search(lpublic,"img_ok"));		  
		  if(cgiFormSubmitClicked("submit_configaclgrp") != cgiFormSuccess)
            fprintf(cgiOut,"<td width=62 align=left><a href=wp_aclgrplist.cgi?UN=%s target=mainFrame><img src=/images/%s border=0 width=62 height=20/></a></td>\n",encry,search(lpublic,"img_cancel"));
		  else                                         
     		fprintf(cgiOut,"<td width=62 align=left><a href=wp_aclgrplist.cgi?UN=%s target=mainFrame><img src=/images/%s border=0 width=62 height=20/></a></td>\n",encry,search(lpublic,"img_cancel"));
		  fprintf(cgiOut,"</tr>\n"\
          "</table>\n");
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
                  "</tr>\n");

 				fprintf(cgiOut,"<tr height=25>\n"\
				  "<td align=left id=tdleft><a href=wp_aclall.cgi?UN=%s target=mainFrame class=top><font id=%s>%s</font></a></td>\n",encry,search(lpublic,"menu_san"),search(lcontrol,"acl_list")); 					 
				  fprintf(cgiOut,"</tr>\n");
				  fprintf(cgiOut,"<tr height=25>\n"\
				  "<td align=left id=tdleft><a href=wp_addaclrule.cgi?UN=%s target=mainFrame class=top><font id=%s>%s</font></a></td>\n",encry,search(lpublic,"menu_san"),search(lcontrol,"add_acl_rule")); 					 
				  fprintf(cgiOut,"</tr>\n");
				  fprintf(cgiOut,"<tr height=26>\n"\
        			"<td align=left id=tdleft background=/images/bottom_bg.gif style=\"border-right:0\"><font id=%s>%s</font></td>\n",search(lpublic,"menu_san"),search(lcontrol,"config_acl_grp"));   /*突出显示*/
        			fprintf(cgiOut,"</tr>\n");
				  fprintf(cgiOut,"<tr height=25>\n"\
    				"<td align=left id=tdleft><a href=wp_addaclgroup.cgi?UN=%s target=mainFrame class=top><font id=%s>%s</font></a></td>\n",encry,search(lpublic,"menu_san"),search(lcontrol,"add_acl_group"));

					for(i=0;i<4;i++)
					  {
						fprintf(cgiOut,"<tr height=25>\n"\
						  "<td id=tdleft>&nbsp;</td>\n"\
						"</tr>\n");
					  }
				cgiFormStringNoNewlines("bind_type", typeBind, 10);
				  fprintf(cgiOut,"</table>\n"\
              "</td>\n"\
              "<td align=left valign=top style=\"background-color:#ffffff; border-right:1px solid #707070; padding-left:30px; padding-top:10px\">\n"\
					  "<table width=640 border=0 cellspacing=0 cellpadding=0>\n"\
						"<tr align=left>\n"\
						  "<td align=left valign=top colspan=3>\n");
						fprintf(cgiOut,"<table frame=below rules=rows width=390 border=1>\n");	
     					fprintf(cgiOut,"<tr height=25 padding-top:10px  align=left>\n");
					fprintf(cgiOut,"<td style='width:60px;height:auto' align='center'><b>%s:</b></td>\n",search(lcontrol,"bind"));
					if(get_product_id()!=PRODUCT_ID_AU3K_BCM)
					{
	     					fprintf(cgiOut,"<td width=80 align=left><select name=bind_type onchange=\"javascript:this.form.submit();\" style='width:84px;height:auto'>\n");
						    for(i=0;i<2;i++)
	 							if(strcmp(bind_mode[i],typeBind)==0)				/*显示上次选中的*/
	 								fprintf(cgiOut,"<option value=%s selected=selected>%s</option>\n",bind_mode[i],bind_mode[i]);
	 							else				
	 								fprintf(cgiOut,"<option value=%s>%s</option>\n",bind_mode[i],bind_mode[i]);
	 								
						    fprintf(cgiOut,"</select>\n"\
						    "</td>\n");
					}
					else
					{
						fprintf(cgiOut,"<td width=80 align=left><input type=hidden name=bind_type value=PortNo><b>PortNo</b></td>\n");
					}
					    if(get_product_id()!=PRODUCT_ID_AU3K_BCM)
					    {
						    if(strcmp(typeBind,"PortNo")==0)
						    {
		           							fprintf(cgiOut,"<td  align=left><select name=bind_text style='width:84px;height:auto'>\n");
											fprintf(cgiOut,"<option value=''>--%s--</option>\n",search(lcontrol,"select"));
											ret = show_ethport_list(&head,&slot_mun);
											p = head.next;
											while(p!=NULL)
											{
												pp = p->port.next;
												while(pp!=NULL)
												{
													if(p->slot_no!=0)
													{
														fprintf(cgiOut,"<option value = %d/%d>%d/%d</option>\n",p->slot_no,pp->port_no,p->slot_no,pp->port_no);
													}
													pp = pp->next;
												}
												p = p->next;
											}
		           						    fprintf(cgiOut,"</select>\n");
		           						    fprintf(cgiOut,"</td>\n");
	      					     }
						    else
						    {
						    		show_vlan_list(receive_vlan,port_num,&vlanNum);
	             						fprintf(cgiOut,"<td>\n");
									fprintf(cgiOut,"<select name='bind_text' style='width:84px;height:auto'>\n");
										fprintf(cgiOut,"<option value=''>--%s--</option>\n",search(lcontrol,"select"));
									for(i=0;i<vlanNum;i++)
									{	
										if(receive_vlan[i].vlanId!=4095)
										{
											fprintf(cgiOut,"<option vlaue=%d>%d</option>\n",receive_vlan[i].vlanId,receive_vlan[i].vlanId);
										}
									}
									fprintf(cgiOut,"</select>\n");
								fprintf(cgiOut,"</td>\n");
						    }
					    }
					else
					{
						fprintf(cgiOut,"<td  align=left><select name=bind_text style='width:84px;height:auto'>\n");
						fprintf(cgiOut,"<option value=''>--%s--</option>\n",search(lcontrol,"select"));
						ret = show_ethport_list(&head,&slot_mun);
						p = head.next;
						while(p!=NULL)
						{
							pp = p->port.next;
							while(pp!=NULL)
							{
								if(p->slot_no!=0)
								{
									fprintf(cgiOut,"<option value = %d/%d>%d/%d</option>\n",p->slot_no,pp->port_no,p->slot_no,pp->port_no);
								}
								pp = pp->next;
							}
							p = p->next;
						}
						fprintf(cgiOut,"</select>\n");
						fprintf(cgiOut,"</td>\n");
					}
						
					    fprintf(cgiOut,"<td width=60 style=padding-left:5px><input type=submit name=submit_bind style=width:60px; height:36px border=0  style=background-image:url(/images/SubBackGif.gif) value=%s></td>\n",search(lcontrol,"bind"));
					   
					    fprintf(cgiOut,"</tr>\n");
				//**********************add unbind function********************************
				cgiFormStringNoNewlines("unbind_type", typeUnBind, 10);
				fprintf(cgiOut,"<tr>\n");
					//***********choose vlan or port***************
					fprintf(cgiOut,"<td  style='width:60px;height:auto' align='center'><b>%s:</b></td>\n",search(lcontrol,"unbind"));
					if(get_product_id()!=PRODUCT_ID_AU3K_BCM)
					{
						fprintf(cgiOut,"<td>\n");
							fprintf(cgiOut,"<select name=unbind_type onchange=\"javascript:this.form.submit();\" style='width:84px;height:auto'>\n");
							for(i=0;i<2;i++)
							{
								if(strcmp(bind_mode[i],typeUnBind)==0)				/*显示上次选中的*/
								{
									fprintf(cgiOut,"<option value=%s selected=selected>%s</option>\n",bind_mode[i],bind_mode[i]);
								}
								else
								{
									fprintf(cgiOut,"<option value=%s>%s</option>\n",bind_mode[i],bind_mode[i]);
								}
							}
							
						    fprintf(cgiOut,"</select>\n");
						fprintf(cgiOut,"</td>\n");
					}
					else
					{
						fprintf(cgiOut,"<td><input type=hidden name=unbind_type value=PortNo><b>PortNo</b></td>\n");
					}
					//************choose vlan number or port number while choose vlan or port*****************************
					//return 0;
					show_acl_group_one(iotype,index,p_oneinfo);
					if(get_product_id()!=PRODUCT_ID_AU3K_BCM)
					{
						if(strcmp(typeUnBind,"PortNo")==0)
						{
							if((p_oneinfo->bind_by_port_count)!=0)
							{
								fprintf(cgiOut,"<td  align=left><select name=unbind_text style='width:84px;height:auto'>\n");
								fprintf(cgiOut,"<option value=''>--%s--</option>\n",search(lcontrol,"select"));
								//****add lop here*******
								for(i=0;i<(p_oneinfo->bind_by_port_count);i++)
								{
									fprintf(cgiOut,"<option value='%u/%u'>%u/%u</option>\n",p_oneinfo->bind_by_slot[i],p_oneinfo->bind_by_port[i],p_oneinfo->bind_by_slot[i],p_oneinfo->bind_by_port[i]);
								}
								fprintf(cgiOut,"</select>\n");
								fprintf(cgiOut,"</td>\n");

								fprintf(cgiOut,"<td width=190 style=padding-left:5px><input type=submit name=submit_unbind style=width:60px; height:36px border=0 style=background-image:url(/images/SubBackGif.gif)  value=%s></td>",search(lcontrol,"unbind"));
							}
							else
							{
								fprintf(cgiOut,"<td align='center'><font color='red'>%s</font></td>",search(lcontrol,"no_bind_port"));
								fprintf(cgiOut,"<td width=190 style=padding-left:5px><input type=submit name=submit_unbind style=width:60px; height:36px border=0 style=background-image:url(/images/SubBackGif.gif)  value=%s disabled='disabled'></td>",search(lcontrol,"unbind"));
							}

						}
						else
						{
							if(p_oneinfo->vlan_count!=0)
							{
								fprintf(cgiOut,"<td>\n");
								fprintf(cgiOut,"<select name='unbind_text' style='width:84px;height:auto'>\n");
								fprintf(cgiOut,"<option value=''>--%s--</option>\n",search(lcontrol,"select"));
								//**********add lop here**********
								for(i=0;i<(p_oneinfo->vlan_count);i++)
								{
									fprintf(cgiOut,"<option value=%u>%u</option>\n",p_oneinfo->bind_by_vlan[i],p_oneinfo->bind_by_vlan[i]);
								}
								//******************************

								fprintf(cgiOut,"</select>\n");
								fprintf(cgiOut,"</td>\n");

								fprintf(cgiOut,"<td width=190 style=padding-left:5px><input type=submit name=submit_unbind style=width:60px; height:36px border=0 style=background-image:url(/images/SubBackGif.gif)  value=%s></td>",search(lcontrol,"unbind"));
							}
							else
							{
								fprintf(cgiOut,"<td align='center'><font color='red'>%s</font></td>\n",search(lcontrol,"no_bind_vlan"));
								fprintf(cgiOut,"<td width=190 style=padding-left:5px><input type=submit name=submit_unbind style=width:60px; height:36px border=0 style=background-image:url(/images/SubBackGif.gif)  value=%s disabled='disabled'></td>",search(lcontrol,"unbind"));
							}
						}
					}
					else
					{
						if((p_oneinfo->bind_by_port_count)!=0)
						{
							fprintf(cgiOut,"<td  align=left><select name=unbind_text style='width:84px;height:auto'>\n");
							fprintf(cgiOut,"<option value=''>--%s--</option>\n",search(lcontrol,"select"));
							//****add lop here*******
							for(i=0;i<(p_oneinfo->bind_by_port_count);i++)
							{
								fprintf(cgiOut,"<option value='%u/%u'>%u/%u</option>\n",p_oneinfo->bind_by_slot[i],p_oneinfo->bind_by_port[i],p_oneinfo->bind_by_slot[i],p_oneinfo->bind_by_port[i]);
							}
							fprintf(cgiOut,"</select>\n");
							fprintf(cgiOut,"</td>\n");

							fprintf(cgiOut,"<td width=190 style=padding-left:5px><input type=submit name=submit_unbind style=width:60px; height:36px border=0 style=background-image:url(/images/SubBackGif.gif)  value=%s></td>",search(lcontrol,"unbind"));
						}
						else
						{
							fprintf(cgiOut,"<td align='center'><font color='red'>%s</font></td>",search(lcontrol,"no_bind_port"));
							fprintf(cgiOut,"<td width=190 style=padding-left:5px><input type=submit name=submit_unbind style=width:60px; height:36px border=0 style=background-image:url(/images/SubBackGif.gif)  value=%s disabled='disabled'></td>",search(lcontrol,"unbind"));
						}

					}
						//********************************
					
					 
					
				fprintf(cgiOut,"</tr>\n");
				//***************************************************************
					 if(disableFlag==1)
					 {
						fprintf(cgiOut,"<tr align=left>\n");
				fprintf(cgiOut,"<td style='width:60px;height:auto' align='center' disabled='disabled'><b>ACL:</b></td>\n");
			fprintf(cgiOut,"<td width=80 disabled='disabled'>ACL %s:</td>",search(lcontrol,"rule"));
						show_acl_allinfo(receive_acl,&aclNum);
						 
						if(aclNum!=0)
						{
							fprintf(cgiOut,"<td width=60 disabled='disabled'>\n");
							
								fprintf(cgiOut,"<select name='ACL_RULE' style='width:84px;height:auto'>\n");
									fprintf(cgiOut,"<option value="">--%s--</option>\n",search(lcontrol,"select"));
									for(i=0;i<aclNum;i++)
									{
										fprintf(cgiOut,"<option value=%d>%d</option>\n",receive_acl[i].ruleIndex,receive_acl[i].ruleIndex);
									}
								fprintf(cgiOut,"</select>\n");
							fprintf(cgiOut,"</td>\n");
							fprintf(cgiOut,"<td colspan=3 width=280 style=padding-left:5px disabled='disabled'><input type=submit name=submit_addrule style=width:60px; height:36px border=0 style=background-image:url(/images/SubBackGif.gif)  value=%s></td>\n",search(lcontrol,"add_rule"));
						}
						else
						{
							fprintf(cgiOut,"<td align='center' disabled='disabled'><font color='red'>%s</font></d>\n",search(lcontrol,"no_acl"));
							fprintf(cgiOut,"<td colspan=3 width=280 style=padding-left:5px><input type=submit name=submit_addrule style=width:60px; height:36px border=0 style=background-image:url(/images/SubBackGif.gif)  value=%s  disabled='disabled'></td>\n",search(lcontrol,"add_rule"));
						}
						
						fprintf(cgiOut,"</tr>\n");

						fprintf(cgiOut,"<tr align='left'>"\
										        "<td align='center' style='width:120px;height:auto' colspan='2' disabled='disabled'><b>%s:</b></td>\n",search(lcontrol,"del_acl"));
							if((p_oneinfo->acl_count)!=0)
							{
								fprintf(cgiOut,"<td align='center' disabled='disabled'>\n"\
													"<select name='acl_index'>\n");
											fprintf(cgiOut,"<option value=''>--%s--</option>\n",search(lcontrol,"select"));
											for(i=0;i<p_oneinfo->acl_count;i++)
											{
												fprintf(cgiOut,"<option value='%d'>%d</option>\n",p_oneinfo->index[i],p_oneinfo->index[i]);
											}
										fprintf(cgiOut,"</select>\n"\
											 "</td>\n"\
										        "<td width='280' style='padding-left:5px' colspan='3' disabled='disabled'><input type=submit name=submit_delacl style=width:60px; height:36px border=0 style=background-image:url(/images/SubBackGif.gif)  value=%s></td>\n",search(lcontrol,"del_acl_rule"));
							}
							else
							{
								fprintf(cgiOut,"<td align='center' disabled='disabled'><font color='red'>%s</font></td>",search(lcontrol,"no_add_acl"));
								fprintf(cgiOut, "<td width='280' style='padding-left:5px' colspan='3'><input type=submit name=submit_delacl style=width:60px; height:36px border=0 style=background-image:url(/images/SubBackGif.gif)  value=%s  disabled='disabled'></td>\n",search(lcontrol,"del_acl_rule"));
							}
							fprintf(cgiOut,"</tr>\n");
							
					    fprintf(cgiOut,"</table>\n"\
						"</td>\n"\
						"</tr>\n");
						fprintf(cgiOut,"<tr>"\
										"<td style=font-size:14px;color:#FF0000 height='45'>%s</td>"\
									"</tr>",search(lcontrol,"bind_not_add_acl"));
					 	}
					 	else
						{
						fprintf(cgiOut,"<tr align=left>\n");
				fprintf(cgiOut,"<td style='width:60px;height:auto' align='center'><b>ACL:</b></td>\n");
			fprintf(cgiOut,"<td width=80>ACL %s:</td>",search(lcontrol,"rule"));
						show_acl_allinfo(receive_acl,&aclNum);
						 
						if(aclNum!=0)
						{
							fprintf(cgiOut,"<td width=60>\n");
							
								fprintf(cgiOut,"<select name='ACL_RULE' style='width:84px;height:auto'>\n");
									fprintf(cgiOut,"<option value="">--%s--</option>\n",search(lcontrol,"select"));
									for(i=0;i<aclNum;i++)
									{
										fprintf(cgiOut,"<option value=%d>%d</option>\n",receive_acl[i].ruleIndex,receive_acl[i].ruleIndex);
									}
								fprintf(cgiOut,"</select>\n");
							fprintf(cgiOut,"</td>\n");
							fprintf(cgiOut,"<td colspan=3 width=280 style=padding-left:5px><input type=submit name=submit_addrule style=width:60px; height:36px border=0 style=background-image:url(/images/SubBackGif.gif)  value=%s></td>\n",search(lcontrol,"add_rule"));
						}
						else
						{
							fprintf(cgiOut,"<td align='center'><font color='red'>%s</font></d>\n",search(lcontrol,"no_acl"));
							fprintf(cgiOut,"<td colspan=3 width=280 style=padding-left:5px><input type=submit name=submit_addrule style=width:60px; height:36px border=0 style=background-image:url(/images/SubBackGif.gif)  value=%s  disabled='disabled'></td>\n",search(lcontrol,"add_rule"));
						}
						
						fprintf(cgiOut,"</tr>\n");

						fprintf(cgiOut,"<tr align='left'>"\
										        "<td align='center' style='width:120px;height:auto' colspan='2'><b>%s:</b></td>\n",search(lcontrol,"del_acl"));
							if((p_oneinfo->acl_count)!=0)
							{
								fprintf(cgiOut,"<td align='center'>\n"\
													"<select name='acl_index'>\n");
											fprintf(cgiOut,"<option value=''>--%s--</option>\n",search(lcontrol,"select"));
											for(i=0;i<p_oneinfo->acl_count;i++)
											{
												fprintf(cgiOut,"<option value='%d'>%d</option>\n",p_oneinfo->index[i],p_oneinfo->index[i]);
											}
										fprintf(cgiOut,"</select>\n"\
											 "</td>\n"\
										        "<td width='280' style='padding-left:5px' colspan='3'><input type=submit name=submit_delacl style=width:60px; height:36px border=0 style=background-image:url(/images/SubBackGif.gif)  value=%s></td>\n",search(lcontrol,"del_acl_rule"));
							}
							else
							{
								fprintf(cgiOut,"<td align='center'><font color='red'>%s</font></td>",search(lcontrol,"no_add_acl"));
								fprintf(cgiOut, "<td width='280' style='padding-left:5px' colspan='3'><input type=submit name=submit_delacl style=width:60px; height:36px border=0 style=background-image:url(/images/SubBackGif.gif)  value=%s  disabled='disabled'></td>\n",search(lcontrol,"del_acl_rule"));
							}
							fprintf(cgiOut,"</tr>\n");
							
					    fprintf(cgiOut,"</table>\n"\
						"</td>\n"\
						"</tr>\n");
					 	}
										

						fprintf(cgiOut,"<tr>\n");
							fprintf(cgiOut,"<td>\n");
							fprintf(cgiOut,"</td>\n");
						fprintf(cgiOut,"</tr>\n");

						
					  	
						fprintf(cgiOut,"<tr>");

						fprintf(cgiOut,"<td><input type=hidden name=encry_configaclgrp value=%s></td>\n",encry);
						fprintf(cgiOut,"<td><input type=hidden name=INDEXLA value=%s></td>\n",index);
						fprintf(cgiOut,"<td><input type=hidden name=TYPELA value=%s></td>\n",grpType);
						fprintf(cgiOut,"<td><input type=hidden name=iotype value=%s></td>\n",iotype);

					  fprintf(cgiOut,"</tr>\n"\
								  "</table>\n"\




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
if((ret==0)&&(slot_mun>0))
{
	Free_ethslot_head(&head);
}
for(i=0;i<MAX_ACL_NUM;i++)
{
	free(receive_acl[i].ruleType);
	free(receive_acl[i].protype);
	free(receive_acl[i].dip);	
	free(receive_acl[i].sip);	

	free(receive_acl[i].actype);	
	free(receive_acl[i].dmac);
	free(receive_acl[i].smac);

	free(receive_acl[i].source_port);
	free(receive_acl[i].redirect_port);
	free(receive_acl[i].analyzer_port);

	free(receive_acl[i].upmm);
	free(receive_acl[i].dscpmm);
}
free(encry);
free(typeBind);
free(typeUnBind);
free(i_char);
free(grpType);
free(grpTypeLA);
free(ACL_RULE);
free(index);
free(aclRuleIndex);
free(p_oneinfo);
free(dis_or_en);
free(iotype);
release(lpublic);  
release(lcontrol);
return 0;
}

int bind_hand(struct list *lcontrol,char * grpindex,char * grptype)
{
	ETH_SLOT_LIST  head,*p;
    	ETH_PORT_LIST *pp;
	int ret = 0;
	int slot_mun = 0;
	char * bind_port = (char *)malloc(10);
	unsigned int nodetype=0,gu_index=0;
	char * bind_type=(char *)malloc(20);
	memset(bind_type,0,20);
	char * bind_text=(char *)malloc(30);
	memset(bind_text,0,30);
	char * unbind_type=(char *)malloc(20);
	memset(bind_type,0,20);
	char * unbind_text=(char *)malloc(30);
	memset(bind_text,0,30);
	int index_temp;
	index_temp=atoi(grpindex);
	int dir=0;
	char * product=(char *)malloc(10);
	memset(product,0,10);
	product=readproductID();

	if(strcmp(grptype,"ingress")==0)
		dir=0;
	else if(strcmp(grptype,"egress")==0)
		dir=1;
	cgiFormStringNoNewlines("bind_type", bind_type, 10);
	cgiFormStringNoNewlines("bind_text",bind_text,30);
	cgiFormStringNoNewlines("unbind_type",unbind_type, 10);
	cgiFormStringNoNewlines("unbind_text",unbind_text,30);
	if(strcmp(bind_type,"VlanId")==0)
	{
		
		nodetype=1;
		if(strcmp(bind_text,"")!=0)
		{
			if(strcmp(bind_text,"1")==0)
	     		{
	     			ShowAlert(search(lcontrol,"default_vlan_not_bind"));
	     		}
	     		else
	     		{
	    			gu_index=atoi(bind_text);
	    			enable_aclgrp(grptype,"enable",gu_index,1,lcontrol);
	    			bind_acl_group(dir,nodetype,gu_index,index_temp);
			}
		}
		else
		{	
			ShowAlert(search(lcontrol,"vlanId_null"));
		}
	}
	else if(strcmp(bind_type,"PortNo")==0)
	{
		nodetype=0;
		int flag=0;
		if(strcmp(bind_text,"")!=0)
		{
			ret = show_ethport_list(&head,&slot_mun);
			p = head.next;
			while(p!=NULL)
			{
				pp = p->port.next;
				while(pp!=NULL)
				{
					if(p->slot_no!=0)
					{
						memset(bind_port,0,10);
						sprintf(bind_port,"%d/%d",p->slot_no,pp->port_no);
						if(strcmp(bind_port,bind_text)==0)
						{
							flag = 1;
						}
					}
					pp = pp->next;
				}
				p = p->next;
			}		
			int reta=get_one_port_index(bind_text,&gu_index);
			if(reta==5 || flag==0)
				ShowAlert(search(lcontrol,"unknown_portno_format"));
			else
			{
	    			enable_aclgrp(grptype,"enable",gu_index,0,lcontrol);
	        		bind_acl_group(dir,nodetype,gu_index,index_temp);
    			}
		}
		else
		{
			ShowAlert(search(lcontrol,"port_null"));
		}
		
	}
	//free(product);
	free(bind_type);
	free(bind_text);
	if((ret==0)&&(slot_mun>0))
	{
		Free_ethslot_head(&head);
	}
	return 0;
}

int unbind_hand(struct list *lcontrol,char * grpindex,char * grptype)
{
	unsigned int gu_index=0;
	char * unbind_type=(char *)malloc(20);
	memset(unbind_type,0,20);
	char * unbind_text=(char *)malloc(30);
	memset(unbind_text,0,30);
	cgiFormStringNoNewlines("unbind_type", unbind_type, 10);
	cgiFormStringNoNewlines("unbind_text",unbind_text,30);
	
	if(strcmp(unbind_type,"VlanId")==0)
	{
		if(strcmp(unbind_text,"")!=0)
		{
			gu_index=atoi(unbind_text);
			unbind_aclgrp_port(grptype,grpindex,gu_index,1,lcontrol);
		}
		else
		{
			ShowAlert(search(lcontrol,"vlanId_null"));
		}
	}
	else if(strcmp(unbind_type,"PortNo")==0)
	{
		if(strcmp(unbind_text,"")!=0)
		{
			get_one_port_index(unbind_text,&gu_index);
			unbind_aclgrp_port(grptype,grpindex,gu_index,0,lcontrol);
		}
		else
		{
			ShowAlert(search(lcontrol,"port_null"));
		}
	}
	free(unbind_type);
	free(unbind_text);
	return 0;
}


