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
* wp_aclruledta.c
*
*
* CREATOR:
* autelan.software.Network Dep. team
* zhouym@autelan.com
*
* DESCRIPTION:
* system contrl for acl ruler detail
*
*
*******************************************************************************/
#include <stdio.h>
#include "cgic.h"
#include <string.h>
#include <stdlib.h>
#include "ws_usrinfo.h"
#include "ws_init_dbus.h"
#include <arpa/inet.h> 

#include "ws_dcli_acl.h"


int ShowRuleDetailPage(); 
int addvlan_hand(struct list *lpublic); 

int cgiMain()
{
 ShowRuleDetailPage();
 return 0;
}

int ShowRuleDetailPage()
{
	struct list *lpublic;	/*解析public.txt文件的链表头*/
	struct list *lcontrol; 	/*解析help.txt文件的链表头*/
	lpublic=get_chain_head("../htdocs/text/public.txt");
    lcontrol=get_chain_head("../htdocs/text/control.txt"); 
	//FILE *fp;
	//char lan[3];
	char *encry=(char *)malloc(BUF_LEN);			  
	char *str;
	char index[10]={0};
	unsigned int indextemp=0;
	char *endptr = NULL;  
	int i;
	char ruledetail_encry[BUF_LEN];  
	struct acl_info  acl_info;
	
	/*acl_info.ruleIndex=0;
	acl_info.groupIndex=0;
	acl_info.ruleType=(char *)malloc(20);
	memset(acl_info.ruleType,0,20);
	acl_info.protype=(char *)malloc(20);
	memset(acl_info.protype,0,20);
	acl_info.dip=(char *)malloc(20);
	memset(acl_info.dip,0,20);
	acl_info.sip=(char *)malloc(20);
	memset(acl_info.sip,0,20);
	acl_info.srcport=0; 
	acl_info.dstport=0;
	acl_info.icmp_code=0;
	acl_info.icmp_type=0;
	acl_info.actype=(char *)malloc(50);
	memset(acl_info.actype,0,50);
	acl_info.dmac=(char *)malloc(30);
	memset(acl_info.dmac,0,30);
	acl_info.smac=(char *)malloc(30);
	memset(acl_info.smac,0,30);
	acl_info.vlanid=0; 
	acl_info.source_port=(char *)malloc(30);
	memset(acl_info.source_port,0,30);
	acl_info.redirect_port=(char *)malloc(30);
	memset(acl_info.redirect_port,0,30);
	acl_info.analyzer_port=(char *)malloc(30);
	memset(acl_info.analyzer_port,0,30);
	acl_info.policerId=0;
	acl_info.up=0;
	acl_info.dscp=0;
	acl_info.egrUP=0;
	acl_info.egrDSCP=0;
	acl_info.qosprofileindex=0;
	acl_info.upmm=(char *)malloc(30);
	memset(acl_info.upmm,0,30);
	acl_info.dscpmm=(char *)malloc(30);
	memset(acl_info.dscpmm,0,30);*/

	ccgi_dbus_init();
	if(cgiFormSubmitClicked("submit_acldta") != cgiFormSuccess)
	{
	  memset(encry,0,BUF_LEN);
	  cgiFormStringNoNewlines("UN", encry, BUF_LEN); 
	  str=dcryption(encry);
	  if(str==NULL)
	  {
		ShowErrorPage(search(lpublic,"ill_user")); 		 /*用户非法*/
		return 0;
	  }
	  memset(ruledetail_encry,0,BUF_LEN);	/*清空临时变量*/
	}

  cgiFormStringNoNewlines("encry_addvlan",ruledetail_encry,BUF_LEN);
  cgiFormStringNoNewlines("INDEX",index,10);
  indextemp=strtoul(index,&endptr,10);	   
   cgiHeaderContentType("text/html");
	fprintf(cgiOut,"<html xmlns=\"http://www.w3.org/1999/xhtml\"><head>");
  fprintf(cgiOut,"<meta http-equiv=Content-Type content=text/html; charset=gb2312>");
  fprintf(cgiOut,"<title>%s</title>",search(lcontrol,"acl_conf"));
  fprintf(cgiOut,"<link rel=stylesheet href=/style.css type=text/css>"\

  "</head>"\
  "<body>");
  if(cgiFormSubmitClicked("submit_acldta") == cgiFormSuccess)
  {
     fprintf( cgiOut, "<script type='text/javascript'>\n" );
   	 fprintf( cgiOut, "window.location.href='wp_aclall.cgi?UN=%s';\n", ruledetail_encry);
   	 fprintf( cgiOut, "</script>\n" );
  }

  fprintf(cgiOut,"<form method=post>"\
  "<div align=center>"\
  "<table width=976 border=0 cellpadding=0 cellspacing=0>"\
  "<tr>"\
    "<td width=8 align=left valign=top background=/images/di22.jpg><img src=/images/youce4.jpg width=8 height=30/></td>"\
    "<td width=51 align=left valign=bottom background=/images/di22.jpg><img src=/images/youce33.jpg width=37 height=24/></td>"\
    "<td width=153 align=left valign=bottom background=/images/di22.jpg><font id=titleen>ACL</font><font id=%s> %s</font></td>",search(lpublic,"title_style"),search(lpublic,"management"));
    fprintf(cgiOut,"<td width=690 align=right valign=bottom background=/images/di22.jpg>");
    	  fprintf(cgiOut,"<table width=130 border=0 cellspacing=0 cellpadding=0>"\
          "<tr>"\
          "<td width=62 align=center><input id=but type=submit name=submit_acldta style=background-image:url(/images/%s) value=""></td>",search(lpublic,"img_ok"));		  
		  if(cgiFormSubmitClicked("submit_acldta") != cgiFormSuccess)
            fprintf(cgiOut,"<td width=62 align=left><a href=wp_aclall.cgi?UN=%s target=mainFrame><img src=/images/%s border=0 width=62 height=20/></a></td>",encry,search(lpublic,"img_cancel"));
		  else                                         
     		fprintf(cgiOut,"<td width=62 align=left><a href=wp_aclall.cgi?UN=%s target=mainFrame><img src=/images/%s border=0 width=62 height=20/></a></td>",ruledetail_encry,search(lpublic,"img_cancel"));
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
         			"<td align=left id=tdleft background=/images/bottom_bg.gif style=\"border-right:0\"><font id=%s>%s</font></td>",search(lpublic,"menu_san"),search(lcontrol,"acl_detail"));   /*突出显示*/
         			fprintf(cgiOut,"</tr>");
            		if(cgiFormSubmitClicked("submit_acldta") != cgiFormSuccess)
            		{
						fprintf(cgiOut,"<tr height=25>"\
             			"<td align=left id=tdleft><a href=wp_addaclrule.cgi?UN=%s target=mainFrame class=top><font id=%s>%s</font></a></td>",encry,search(lpublic,"menu_san"),search(lcontrol,"add_acl_rule"));					   
             			fprintf(cgiOut,"</tr>");
             			fprintf(cgiOut,"<tr height=25>"\
             			"<td align=left id=tdleft><a href=wp_aclgrplist.cgi?UN=%s target=mainFrame class=top><font id=%s>%s</font></a></td>",encry,search(lpublic,"menu_san"),search(lcontrol,"acl_grp_lsit"));					   
             			fprintf(cgiOut,"</tr>");
             			
             			fprintf(cgiOut,"<tr height=25>"\
             			"<td align=left id=tdleft><a href=wp_addaclgroup.cgi?UN=%s target=mainFrame class=top><font id=%s>%s</font></a></td>",encry,search(lpublic,"menu_san"),search(lcontrol,"add_acl_group"));
             			fprintf(cgiOut,"</tr>");


            		}
            		else if(cgiFormSubmitClicked("submit_acldta") == cgiFormSuccess)				
            		{
					    fprintf(cgiOut,"<tr height=25>"\
             			"<td align=left id=tdleft><a href=wp_addaclrule.cgi?UN=%s target=mainFrame style=color:#000000><font id=%s>%s</font></a></td>",ruledetail_encry,search(lpublic,"menu_san"),search(lcontrol,"add_acl_rule"));						
             			fprintf(cgiOut,"</tr>");
             			fprintf(cgiOut,"<tr height=25>"\
             			"<td align=left id=tdleft><a href=wp_aclgrplist.cgi?UN=%s target=mainFrame style=color:#000000><font id=%s>%s</font></a></td>",ruledetail_encry,search(lpublic,"menu_san"),search(lcontrol,"acl_grp_lsit"));						
             			fprintf(cgiOut,"</tr>"\
             			"<tr height=25>"\
             			"<td align=left id=tdleft><a href=wp_addaclgroup.cgi?UN=%s target=mainFrame style=color:#000000><font id=%s>%s</font></a></td>",ruledetail_encry,search(lpublic,"menu_san"),search(lcontrol,"add_acl_group"));
             			fprintf(cgiOut,"</tr>");

            		}
					for(i=0;i<8;i++)
					  {
						fprintf(cgiOut,"<tr height=25>"\
						  "<td id=tdleft>&nbsp;</td>"\
						"</tr>");
					  }

				  fprintf(cgiOut,"</table>"\
              "</td>"\
              "<td align=left valign=top style=\"background-color:#ffffff; border-right:1px solid #707070; padding-left:30px; padding-top:10px\">"\
					  "<table width=640 border=0 cellspacing=0 cellpadding=0>"\
						"<tr>"\
						  "<td align=left valign=top>");
						//int k=show_aclinfo_Byindex(acl_info,indextemp);
						acl_info=show_aclinfo_Byindex(indextemp);
						
     					   	fprintf(cgiOut,"<div id=aclinfo><table frame=below rules=rows width=320 border=1>");	
     					   	fprintf(cgiOut,"<tr align=left>"\
             					   "<td id=td1 width=170>%s</td>",search(lcontrol,"Rule_index"));
             					   	fprintf(cgiOut,"<td id=td2 width=150>%d</td>",indextemp);
             					   	fprintf(cgiOut,"</tr>");
     					   	if(strcmp(acl_info.ruleType,"standard")==0)
     					   	{
     					   		fprintf(cgiOut,"<tr align=left>"\
             					   "<td id=td1 width=170>%s</td>",search(lcontrol,"ruleType"));
             					   	fprintf(cgiOut,"<td id=td2 width=150>%s</td>",acl_info.ruleType);
             					   	fprintf(cgiOut,"</tr>");
         					   	if((strcmp(acl_info.protype,"IP")==0) || (strcmp(acl_info.protype,"UDP")==0) || (strcmp(acl_info.protype,"TCP")==0) || (strcmp(acl_info.protype,"ICMP")==0))
         					   	{
         					   		fprintf(cgiOut,"<tr align=left>"\
             					   "<td id=td1 width=170>%s</td>",search(lcontrol,"action"));
             					   	fprintf(cgiOut,"<td id=td2 width=150>%s</td>",acl_info.actype);
             					   	fprintf(cgiOut,"</tr>");
             					   	fprintf(cgiOut,"<tr align=left>"\
             					   "<td id=td1 width=170>%s</td>","Protocol");
             					   	fprintf(cgiOut,"<td id=td2 width=150>%s</td>",acl_info.protype);
             					   	fprintf(cgiOut,"</tr>");
         					   		fprintf(cgiOut,"<tr align=left>"\
             					   "<td id=td1 width=170>%s</td>",search(lcontrol,"destination_IP"));
             					   	fprintf(cgiOut,"<td id=td2 width=150>%s</td>",acl_info.dip);
             					   	fprintf(cgiOut,"</tr>");
             					   	
         					   		fprintf(cgiOut,"<tr align=left>"\
             					   	"<td id=td1 width=170>%s</td>",search(lcontrol,"source_IP"));
             					   	fprintf(cgiOut,"<td id=td2 width=150>%s</td>",acl_info.sip);
             					   	fprintf(cgiOut,"</tr>");
             					   	if((strcmp(acl_info.protype,"TCP")==0)||(strcmp(acl_info.protype,"UDP")==0))
             					   	{
										fprintf(cgiOut,"<tr align=left>"\
										"<td id=td1 width=170>%s</td>",search(lcontrol,"destination_port"));
										if(acl_info.dstport == ACL_ANY_PORT)
										{
											fprintf(cgiOut,"<td id=td2 width=150>%s</td>","any");
										}
										else
										{
											fprintf(cgiOut,"<td id=td2 width=150>%ld</td>",acl_info.dstport);	
										}
										fprintf(cgiOut,"</tr>");

										fprintf(cgiOut,"<tr align=left>"\
										"<td id=td1 width=170>%s</td>",search(lcontrol,"source_port"));
										if(acl_info.srcport == ACL_ANY_PORT)
										{
											fprintf(cgiOut,"<td id=td2 width=150>%s</td>","any");
										}
										else
										{
											fprintf(cgiOut,"<td id=td2 width=150>%ld</td>",acl_info.srcport);
										}
										fprintf(cgiOut,"</tr>");
                    				}
                    				if(strcmp(acl_info.protype,"ICMP")==0)
                    				{
                    					fprintf(cgiOut,"<tr align=left>"\
										"<td id=td1 width=170>%s</td>","icmp code");
										if(acl_info.icmp_code!=ICMP_WARNING)
										{
											fprintf(cgiOut,"<td id=td2 width=150>%d</td>",acl_info.icmp_code);
										}
										else
										{
											fprintf(cgiOut,"<td id=td2 width=150>%s</td>","any");
										}
										fprintf(cgiOut,"</tr>");

										fprintf(cgiOut,"<tr align=left>"\
										"<td id=td1 width=170>%s</td>",search(lcontrol,"icmp_type"));
										if(acl_info.icmp_type!=ICMP_WARNING)
										{
											fprintf(cgiOut,"<td id=td2 width=150>%d</td>",acl_info.icmp_type);
										}
										else
										{
											fprintf(cgiOut,"<td id=td2 width=150>%s</td>","any");
										}
										fprintf(cgiOut,"</tr>");
                    				}
                    				if (strcmp(acl_info.actype,"Redirect")==0)
                    					{
			 		 						fprintf(cgiOut,"<tr align=left>"\
    										"<td id=td1 width=170>%s</td>",search(lcontrol,"redirect_port"));
    										fprintf(cgiOut,"<td id=td2 width=150>%s</td>",acl_info.redirect_port);
    										fprintf(cgiOut,"</tr>");
										}
									if (strcmp(acl_info.actype,"MirrorToAnalyzer")==0)
									{
			 								fprintf(cgiOut,"<tr align=left>"\
    										"<td id=td1 width=170>%s</td>",search(lcontrol,"analyzer_port"));
    										fprintf(cgiOut,"<td id=td2 width=150>%s</td>",acl_info.analyzer_port);
    										fprintf(cgiOut,"</tr>");
			 						}
			 						if(acl_info.policerId!=0)
			 						{
                        					fprintf(cgiOut,"<tr align=left>"\
    										"<td id=td1 width=170>%s</td>","policerId");
    										fprintf(cgiOut,"<td id=td2 width=150>%u</td>",acl_info.policerId);
    										fprintf(cgiOut,"</tr>");
									}
									if(32==acl_info.appendIndex)
									{
										fprintf(cgiOut,"<tr align=left>"\
    										"<td id=td1 width=170>%s</td>",search(lcontrol,"QoS_Table"));
    										fprintf(cgiOut,"<td id=td2 width=150>%u</td>",acl_info.qosprofileindex);
    										fprintf(cgiOut,"</tr>");
									}
         					   	}
         					   	else if((strcmp(acl_info.protype,"Ethernet")==0) || (strcmp(acl_info.protype,"ARP")==0))
         					   	{
            					   	fprintf(cgiOut,"<tr align=left>"\
                					   "<td id=td1 width=170>%s</td>",search(lcontrol,"action"));
                					   	fprintf(cgiOut,"<td id=td2 width=150>%s</td>",acl_info.actype);
                					   	fprintf(cgiOut,"</tr>");
                					fprintf(cgiOut,"<tr align=left>"\
                					   "<td id=td1 width=170>%s</td>","Protocol");
                					   	fprintf(cgiOut,"<td id=td2 width=150>%s</td>",acl_info.protype);
                					   	fprintf(cgiOut,"</tr>");
         					   		fprintf(cgiOut,"<tr align=left>"\
    										"<td id=td1 width=170>%s</td>",search(lcontrol,"acl_smac"));
    										fprintf(cgiOut,"<td id=td2 width=150>%s</td>",acl_info.smac);
    										fprintf(cgiOut,"</tr>");
    								if(strcmp(acl_info.protype,"Ethernet")==0)
    								{
					 					fprintf(cgiOut,"<tr align=left>"\
    										"<td id=td1 width=170>%s</td>",search(lcontrol,"acl_dmac"));
    										fprintf(cgiOut,"<td id=td2 width=150>%s</td>",acl_info.dmac);
    										fprintf(cgiOut,"</tr>");
				 					}
				 					if(strcmp(acl_info.protype,"ARP")==0)
			 						{
			 							fprintf(cgiOut,"<tr align=left>"\
    										"<td id=td1 width=170>%s</td>",search(lcontrol,"acl_dmac"));
    										fprintf(cgiOut,"<td id=td2 width=150>%s</td>","FF:FF:FF:FF:FF:FF");
    										fprintf(cgiOut,"</tr>");
    									fprintf(cgiOut,"<tr align=left>"\
    										"<td id=td1 width=170>%s</td>","VlanID");
										if(acl_info.vlanid!=0)
										{
    											fprintf(cgiOut,"<td id=td2 width=150>%u</td>",acl_info.vlanid);
										}
										else
										{
											fprintf(cgiOut,"<td id=td2 width=150>%s</td>","any");
										}
    										fprintf(cgiOut,"</tr>");
    									fprintf(cgiOut,"<tr align=left>"\
    										"<td id=td1 width=170>%s</td>",search(lcontrol,"source_slot_port"));
    										if(strcmp(acl_info.source_port,"255/255")!=0)
										{
    										fprintf(cgiOut,"<td id=td2 width=150>%s</td>",acl_info.source_port);
										}
										else
										{
    										fprintf(cgiOut,"<td id=td2 width=150>%s</td>","any");
										}
    										fprintf(cgiOut,"</tr>");
                    				}
                    
                    				 if (strcmp(acl_info.actype,"Redirect")==0)
                    				 {
                    				 	fprintf(cgiOut,"<tr align=left>"\
    										"<td id=td1 width=170>%s</td>",search(lcontrol,"redirect_port"));
    										fprintf(cgiOut,"<td id=td2 width=150>%s</td>",acl_info.redirect_port);
    										fprintf(cgiOut,"</tr>");
                    				 }
                    				 #if 0
                    				 if (strcmp(acl_info.actype,"MirrorToAnalyzer")==0)
                    				 {
                    				 	fprintf(cgiOut,"<tr align=left>"\
    										"<td id=td1 width=170>%s</td>",search(lcontrol,"analyzer_port"));
    										fprintf(cgiOut,"<td id=td2 width=150>%s</td>",acl_info.analyzer_port);
    										fprintf(cgiOut,"</tr>");
                    				 }
									 #endif
									 
                    				if(acl_info.policerId!=0)
			 						{
                        					fprintf(cgiOut,"<tr align=left>"\
    										"<td id=td1 width=170>%s</td>","policerId");
    										fprintf(cgiOut,"<td id=td2 width=150>%u</td>",acl_info.policerId);
    										fprintf(cgiOut,"</tr>");
									}
									if(32==acl_info.appendIndex)
									{
										fprintf(cgiOut,"<tr align=left>"\
    										"<td id=td1 width=170>%s</td>",search(lcontrol,"QoS_Table"));
    										fprintf(cgiOut,"<td id=td2 width=150>%u</td>",acl_info.qosprofileindex);
    										fprintf(cgiOut,"</tr>");
									}
									 
								}
							else if((strcmp(acl_info.protype,"All")==0))
								{
									char * uptemp=(char * )malloc(10);
                 					memset(uptemp,0,10);
                 					char * dscptemp=(char * )malloc(10);
                 					memset(dscptemp,0,10);
                 					if(acl_info.up==0)
                 						strcpy(uptemp,"None");
                 					if(acl_info.dscp==0)
                 						strcpy(dscptemp,"None");
									
									fprintf(cgiOut,"<tr align=left>"\
                					   "<td id=td1 width=170>%s</td>",search(lcontrol,"action"));
                					   	fprintf(cgiOut,"<td id=td2 width=150>%s</td>",acl_info.actype);
                					   	fprintf(cgiOut,"</tr>");	
							#if 0
                     				if(strcmp(acl_info.actype,"Ingress QoS Mark")==0)
                     				{
                     					
                     					fprintf(cgiOut,"<tr align=left>"\
    										"<td id=td1 width=170>%s</td>",search(lcontrol,"Source_UP"));
    										if(acl_info.modifyUP==0)
    											fprintf(cgiOut,"<td id=td2 width=150>%s</td>",uptemp);
    										else
    										fprintf(cgiOut,"<td id=td2 width=150>%u</td>",acl_info.up);
    										fprintf(cgiOut,"</tr>");
    									fprintf(cgiOut,"<tr align=left>"\
    										"<td id=td1 width=170>%s</td>",search(lcontrol,"Source_DSCP"));
    										if(acl_info.modifyDSCP==0)
    											fprintf(cgiOut,"<td id=td2 width=150>%s</td>",dscptemp);
    										else
    											fprintf(cgiOut,"<td id=td2 width=150>%u</td>",acl_info.dscp);
    										fprintf(cgiOut,"</tr>");
										fprintf(cgiOut,"<tr align=left>"\
    										"<td id=td1 width=170>%s</td>",search(lcontrol,"QoS_Table"));
    										fprintf(cgiOut,"<td id=td2 width=150>%u</td>",acl_info.qosprofileindex);
    										fprintf(cgiOut,"</tr>");
										fprintf(cgiOut,"<tr align=left>"\
    										"<td id=td1 width=170>%s</td>",search(lcontrol,"SUB_QOS"));
											if(acl_info.precedence == 0)
    											fprintf(cgiOut,"<td id=td2 width=150>%s</td>",search(lcontrol,"enable_gate"));
											else if(acl_info.precedence == 1)
												fprintf(cgiOut,"<td id=td2 width=150>%u</td>",search(lcontrol,"Disable"));
    										fprintf(cgiOut,"</tr>");
                     
                     				}
                     				if(strcmp(acl_info.actype,"Egress QoS Remark")==0)
                     				{											
                     					fprintf(cgiOut,"<tr align=left>"\
    										"<td id=td1 width=170>%s</td>",search(lcontrol,"Source_UP"));
    										if(acl_info.up==0)
    											fprintf(cgiOut,"<td id=td2 width=150>%s</td>",uptemp);
    										else
    										fprintf(cgiOut,"<td id=td2 width=150>%u</td>",acl_info.up);
    										fprintf(cgiOut,"</tr>");
    									fprintf(cgiOut,"<tr align=left>"\
    										"<td id=td1 width=170>%s</td>",search(lcontrol,"Source_DSCP"));
    										if(acl_info.dscp==0)
    											fprintf(cgiOut,"<td id=td2 width=150>%s</td>",dscptemp);
    										else
    											fprintf(cgiOut,"<td id=td2 width=150>%u</td>",acl_info.dscp);
    										fprintf(cgiOut,"</tr>");
    									fprintf(cgiOut,"<tr align=left>"\
    										"<td id=td1 width=170>%s</td>",search(lcontrol,"Egress_UP"));
    										fprintf(cgiOut,"<td id=td2 width=150>%u</td>",acl_info.egrUP);
    										fprintf(cgiOut,"</tr>");
    									fprintf(cgiOut,"<tr align=left>"\
    										"<td id=td1 width=170>%s</td>",search(lcontrol,"Egress_DSCP"));
    										fprintf(cgiOut,"<td id=td2 width=150>%u</td>",acl_info.egrDSCP);
    										fprintf(cgiOut,"</tr>");	
                     				}
							#endif
                     				fprintf(cgiOut,"<tr align=left>"\
    										"<td id=td1 width=170>%s</td>",search(lcontrol,"Modify_UP"));
    										fprintf(cgiOut,"<td id=td2 width=150>%s</td>",acl_info.upmm);
    										fprintf(cgiOut,"</tr>");
    									fprintf(cgiOut,"<tr align=left>"\
    										"<td id=td1 width=170>%s</td>",search(lcontrol,"Modify_DSCP"));
    										fprintf(cgiOut,"<td id=td2 width=150>%s</td>",acl_info.dscpmm);
    										fprintf(cgiOut,"</tr>");	
                     				
                     				if(acl_info.policerId!=0)
			 						{
                        					fprintf(cgiOut,"<tr align=left>"\
    										"<td id=td1 width=170>%s</td>","policerId");
    										fprintf(cgiOut,"<td id=td2 width=150>%u</td>",acl_info.policerId);
    										fprintf(cgiOut,"</tr>");
									}
									

                     				free(uptemp);
                     				free(dscptemp);
                     			}
                     		 }
                     		 else if(strcmp(acl_info.ruleType,"extended")==0)
							 {

     					   		fprintf(cgiOut,"<tr align=left>"\
             					   "<td id=td1 width=170>%s</td>",search(lcontrol,"ruleType"));
             					   	fprintf(cgiOut,"<td id=td2 width=150>%s</td>",acl_info.ruleType);
             					   	fprintf(cgiOut,"</tr>");
         					   	if((strcmp(acl_info.protype,"IP")==0) || (strcmp(acl_info.protype,"UDP")==0) || (strcmp(acl_info.protype,"TCP")==0) || (strcmp(acl_info.protype,"ICMP")==0))
         					   	{
         					   		fprintf(cgiOut,"<tr align=left>"\
             					   "<td id=td1 width=170>%s</td>",search(lcontrol,"action"));
             					   	fprintf(cgiOut,"<td id=td2 width=150>%s</td>",acl_info.actype);
             					   	fprintf(cgiOut,"</tr>");
             					   	fprintf(cgiOut,"<tr align=left>"\
             					   "<td id=td1 width=170>%s</td>","Protocol");
             					   	fprintf(cgiOut,"<td id=td2 width=150>%s</td>",acl_info.protype);
             					   	fprintf(cgiOut,"</tr>");
         					   		fprintf(cgiOut,"<tr align=left>"\
             					   "<td id=td1 width=170>%s</td>",search(lcontrol,"destination_IP"));
             					   	fprintf(cgiOut,"<td id=td2 width=150>%s</td>",acl_info.dip);
             					   	fprintf(cgiOut,"</tr>");
             					   	
         					   		fprintf(cgiOut,"<tr align=left>"\
             					   	"<td id=td1 width=170>%s</td>",search(lcontrol,"source_IP"));
             					   	fprintf(cgiOut,"<td id=td2 width=150>%s</td>",acl_info.sip);
             					   	fprintf(cgiOut,"</tr>");
             					   	if((strcmp(acl_info.protype,"TCP")==0)||(strcmp(acl_info.protype,"UDP")==0))
             					   	{
										fprintf(cgiOut,"<tr align=left>"\
										"<td id=td1 width=170>%s</td>",search(lcontrol,"destination_port"));
										if(acl_info.dstport == ACL_ANY_PORT)
										{
											fprintf(cgiOut,"<td id=td2 width=150>%s</td>","any");
										}
										else
										{
											fprintf(cgiOut,"<td id=td2 width=150>%ld</td>",acl_info.dstport);
										}	
										fprintf(cgiOut,"</tr>");

										fprintf(cgiOut,"<tr align=left>"\
										"<td id=td1 width=170>%s</td>",search(lcontrol,"source_port"));
										if(acl_info.srcport == ACL_ANY_PORT)
										{
											fprintf(cgiOut,"<td id=td2 width=150>%s</td>","any");
										}
										else
										{
											fprintf(cgiOut,"<td id=td2 width=150>%ld</td>",acl_info.srcport);
										}
										fprintf(cgiOut,"</tr>");

										fprintf(cgiOut,"<tr align=left>"\
										"<td id=td1 width=170>%s</td>",search(lcontrol,"acl_dmac"));
										fprintf(cgiOut,"<td id=td2 width=150>%s</td>",acl_info.dmac);
										fprintf(cgiOut,"</tr>");
									fprintf(cgiOut,"<tr align=left>"\
										"<td id=td1 width=170>%s</td>",search(lcontrol,"acl_smac"));
										fprintf(cgiOut,"<td id=td2 width=150>%s</td>",acl_info.smac);
										fprintf(cgiOut,"</tr>");
									fprintf(cgiOut,"<tr align=left>"\
										"<td id=td1 width=170>%s</td>","VlanID");
										if(acl_info.vlanid!=0)
										{
											fprintf(cgiOut,"<td id=td2 width=150>%u</td>",acl_info.vlanid);
										}
										else
										{
											fprintf(cgiOut,"<td id=td2 width=150>%s</td>","any");
										}
										fprintf(cgiOut,"</tr>");
									fprintf(cgiOut,"<tr align=left>"\
    										"<td id=td1 width=170>%s</td>",search(lcontrol,"source_slot_port"));
    										if(strcmp(acl_info.source_port,"255/255")!=0)
										{
    										fprintf(cgiOut,"<td id=td2 width=150>%s</td>",acl_info.source_port);
										}
										else
										{
    										fprintf(cgiOut,"<td id=td2 width=150>%s</td>","any");
										}
    										fprintf(cgiOut,"</tr>");
                    				}
                    				if(strcmp(acl_info.protype,"ICMP")==0)
                    				{
                    					fprintf(cgiOut,"<tr align=left>"\
										"<td id=td1 width=170>%s</td>","icmp code");
										if(acl_info.icmp_code!=ICMP_WARNING)
										{
											fprintf(cgiOut,"<td id=td2 width=150>%d</td>",acl_info.icmp_code);
										}
										else
										{
											fprintf(cgiOut,"<td id=td2 width=150>%s</td>","any");
										}
										fprintf(cgiOut,"</tr>");

										fprintf(cgiOut,"<tr align=left>"\
										"<td id=td1 width=170>%s</td>",search(lcontrol,"icmp_type"));
										if(acl_info.icmp_type!=ICMP_WARNING)
										{
											fprintf(cgiOut,"<td id=td2 width=150>%d</td>",acl_info.icmp_type);
										}
										else
										{
											fprintf(cgiOut,"<td id=td2 width=150>%s</td>","any");
										}
										fprintf(cgiOut,"</tr>");
                    				}
                    				if (strcmp(acl_info.actype,"Redirect")==0)
                    					{
			 		 						fprintf(cgiOut,"<tr align=left>"\
    										"<td id=td1 width=170>%s</td>",search(lcontrol,"redirect_port"));
    										fprintf(cgiOut,"<td id=td2 width=150>%s</td>",acl_info.redirect_port);
    										fprintf(cgiOut,"</tr>");
										}
									if (strcmp(acl_info.actype,"MirrorToAnalyzer")==0)
									{
			 								fprintf(cgiOut,"<tr align=left>"\
    										"<td id=td1 width=170>%s</td>",search(lcontrol,"analyzer_port"));
    										fprintf(cgiOut,"<td id=td2 width=150>%s</td>",acl_info.analyzer_port);
    										fprintf(cgiOut,"</tr>");
			 						}
			 						if(acl_info.policerId!=0)
			 						{
                        					fprintf(cgiOut,"<tr align=left>"\
    										"<td id=td1 width=170>%s</td>","policerId");
    										fprintf(cgiOut,"<td id=td2 width=150>%u</td>",acl_info.policerId);
    										fprintf(cgiOut,"</tr>");
									}
									if(32==acl_info.appendIndex)
									{
										fprintf(cgiOut,"<tr align=left>"\
    										"<td id=td1 width=170>%s</td>",search(lcontrol,"QoS_Table"));
    										fprintf(cgiOut,"<td id=td2 width=150>%u</td>",acl_info.qosprofileindex);
    										fprintf(cgiOut,"</tr>");
									}
         					   	}
         					   	else if((strcmp(acl_info.protype,"Ethernet")==0) || (strcmp(acl_info.protype,"ARP")==0))
         					   	{
            					   	fprintf(cgiOut,"<tr align=left>"\
                					   "<td id=td1 width=170>%s</td>",search(lcontrol,"action"));
                					   	fprintf(cgiOut,"<td id=td2 width=150>%s</td>",acl_info.actype);
                					   	fprintf(cgiOut,"</tr>");
                					fprintf(cgiOut,"<tr align=left>"\
                					   "<td id=td1 width=170>%s</td>","Protocol");
                					   	fprintf(cgiOut,"<td id=td2 width=150>%s</td>",acl_info.protype);
                					   	fprintf(cgiOut,"</tr>");
         					   		fprintf(cgiOut,"<tr align=left>"\
    										"<td id=td1 width=170>%s</td>",search(lcontrol,"acl_smac"));
    										fprintf(cgiOut,"<td id=td2 width=150>%s</td>",acl_info.smac);
    										fprintf(cgiOut,"</tr>");
    								if(strcmp(acl_info.protype,"Ethernet")==0)
    								{
					 					fprintf(cgiOut,"<tr align=left>"\
    										"<td id=td1 width=170>%s</td>",search(lcontrol,"acl_dmac"));
    										fprintf(cgiOut,"<td id=td2 width=150>%s</td>",acl_info.dmac);
    										fprintf(cgiOut,"</tr>");
				 				}
				 					if(strcmp(acl_info.protype,"ARP")==0)
			 						{
			 							fprintf(cgiOut,"<tr align=left>"\
    										"<td id=td1 width=170>%s</td>",search(lcontrol,"acl_dmac"));
    										fprintf(cgiOut,"<td id=td2 width=150>%s</td>","FF:FF:FF:FF:FF:FF");
    										fprintf(cgiOut,"</tr>");
    									fprintf(cgiOut,"<tr align=left>"\
    										"<td id=td1 width=170>%s</td>","VlanID");
										if(acl_info.vlanid!=0)
										{
    											fprintf(cgiOut,"<td id=td2 width=150>%u</td>",acl_info.vlanid);
										}
										else
										{
											fprintf(cgiOut,"<td id=td2 width=150>%s</td>","any");
										}
    										fprintf(cgiOut,"</tr>");
    									fprintf(cgiOut,"<tr align=left>"\
    										"<td id=td1 width=170>%s</td>",search(lcontrol,"source_slot_port"));
										if(strcmp(acl_info.source_port,"255/255")!=0)
										{
    										fprintf(cgiOut,"<td id=td2 width=150>%s</td>",acl_info.source_port);
										}
										else
										{
    										fprintf(cgiOut,"<td id=td2 width=150>%s</td>","any");
										}
    										fprintf(cgiOut,"</tr>");
                    							}
                    
                    				 if (strcmp(acl_info.actype,"Redirect")==0)
                    				 {
                    				 	fprintf(cgiOut,"<tr align=left>"\
    										"<td id=td1 width=170>%s</td>",search(lcontrol,"redirect_port"));
    										fprintf(cgiOut,"<td id=td2 width=150>%s</td>",acl_info.redirect_port);
    										fprintf(cgiOut,"</tr>");
                    				 }
                    				 #if 0
                    				 if (strcmp(acl_info.actype,"MirrorToAnalyzer")==0)
                    				 {
                    				 	fprintf(cgiOut,"<tr align=left>"\
    										"<td id=td1 width=170>%s</td>",search(lcontrol,"analyzer_port"));
    										fprintf(cgiOut,"<td id=td2 width=150>%s</td>",acl_info.analyzer_port);
    										fprintf(cgiOut,"</tr>");
                    				 }
									 #endif
									 
                    				if(acl_info.policerId!=0)
			 						{
                        					fprintf(cgiOut,"<tr align=left>"\
    										"<td id=td1 width=170>%s</td>","policerId");
    										fprintf(cgiOut,"<td id=td2 width=150>%u</td>",acl_info.policerId);
    										fprintf(cgiOut,"</tr>");
									}
									if(32==acl_info.appendIndex)
									{
										fprintf(cgiOut,"<tr align=left>"\
    										"<td id=td1 width=170>%s</td>",search(lcontrol,"QoS_Table"));
    										fprintf(cgiOut,"<td id=td2 width=150>%u</td>",acl_info.qosprofileindex);
    										fprintf(cgiOut,"</tr>");
									}
									 
								}
								else if((strcmp(acl_info.protype,"IPv6")==0))
									{
										fprintf(cgiOut,"<tr align=left>");

										fprintf(cgiOut,"<tr align=left>"\
                					   "<td id=td1 width=170>%s</td>",search(lcontrol,"action"));
                					   	fprintf(cgiOut,"<td id=td2 width=150>%s</td>",acl_info.actype);
                					   	fprintf(cgiOut,"</tr>");

										fprintf(cgiOut,"<tr align=left>"\
                					   "<td id=td1 width=170>%s</td>","Protocol");
                					   	fprintf(cgiOut,"<td id=td2 width=150>%s</td>",acl_info.protype);
                					   	fprintf(cgiOut,"</tr>");										

										fprintf(cgiOut,"<tr align=left>"\
										"<td id=td1 width=170>%s</td>","ipv6 next-header");
										fprintf(cgiOut,"<td id=td2 width=150>%d</td>",acl_info.nextheader);
										fprintf(cgiOut,"</tr>");


										char dbuf[sizeof("ffff:ffff:ffff:ffff:ffff:ffff:255.255.255.255")];
										
										fprintf(cgiOut,"<tr align=left>"\
	             					   "<td id=td1 width=170>%s</td>",search(lcontrol,"destination_IP"));
	             					   	fprintf(cgiOut,"<td id=td2 width=150>%s</td>",inet_ntop(AF_INET6,acl_info.dipv6, dbuf, sizeof(dbuf)));
	             					   	fprintf(cgiOut,"</tr>");

										char sbuf[sizeof("ffff:ffff:ffff:ffff:ffff:ffff:255.255.255.255")];
										fprintf(cgiOut,"<tr align=left>"\
										"<td id=td1 width=170>%s</td>",search(lcontrol,"source_IP"));
										fprintf(cgiOut,"<td id=td2 width=150>%s</td>",inet_ntop(AF_INET6, acl_info.sipv6, sbuf, sizeof(sbuf)));
										fprintf(cgiOut,"</tr>");			
										
    									fprintf(cgiOut,"</tr>");
                    				}
							else if((strcmp(acl_info.protype,"All")==0))
								{
									char * uptemp=(char * )malloc(10);
                 					memset(uptemp,0,10);
                 					char * dscptemp=(char * )malloc(10);
                 					memset(dscptemp,0,10);
                 					if(acl_info.up==0)
                 						strcpy(uptemp,"None");
                 					if(acl_info.dscp==0)
                 						strcpy(dscptemp,"None");
									
									fprintf(cgiOut,"<tr align=left>"\
                					   "<td id=td1 width=170>%s</td>",search(lcontrol,"action"));
                					   	fprintf(cgiOut,"<td id=td2 width=150>%s</td>",acl_info.actype);
                					   	fprintf(cgiOut,"</tr>");	
                     				if(strcmp(acl_info.actype,"Ingress QoS Mark")==0)
                     				{
                     					
                     					fprintf(cgiOut,"<tr align=left>"\
    										"<td id=td1 width=170>%s</td>",search(lcontrol,"Source_UP"));
    										if(acl_info.modifyUP==0)
    										{
    											fprintf(cgiOut,"<td id=td2 width=150>%s</td>",uptemp);    										}
    										else
    										{
	    										fprintf(cgiOut,"<td id=td2 width=150>%u</td>",acl_info.up);
	    									}
    										fprintf(cgiOut,"</tr>");
    									fprintf(cgiOut,"<tr align=left>"\
    										"<td id=td1 width=170>%s</td>",search(lcontrol,"Source_DSCP"));
    										if(acl_info.modifyDSCP==0)
    										{
    											fprintf(cgiOut,"<td id=td2 width=150>%s</td>",dscptemp);
												
    										}
    										else
    										{
    											fprintf(cgiOut,"<td id=td2 width=150>%u</td>",acl_info.dscp);
    										}
    										fprintf(cgiOut,"</tr>");
										fprintf(cgiOut,"<tr align=left>"\
    										"<td id=td1 width=170>%s</td>",search(lcontrol,"QoS_Table"));
    										fprintf(cgiOut,"<td id=td2 width=150>%u</td>",acl_info.qosprofileindex);
										
    										fprintf(cgiOut,"</tr>");
										fprintf(cgiOut,"<tr align=left>"\
    										"<td id=td1 width=170>%s</td>",search(lcontrol,"SUB_QOS"));
											if(acl_info.precedence == 0)
    											fprintf(cgiOut,"<td id=td2 width=150>%s</td>",search(lcontrol,"enable_gate"));
											else if(acl_info.precedence == 1)
												fprintf(cgiOut,"<td id=td2 width=150>%s</td>",search(lcontrol,"Disable"));
    										fprintf(cgiOut,"</tr>");
                     				}
                     				if(strcmp(acl_info.actype,"Egress QoS Remark")==0)
                     				{											
                     					fprintf(cgiOut,"<tr align=left>"\
    										"<td id=td1 width=170>%s</td>",search(lcontrol,"Source_UP"));
										#if 0
    										if(acl_info.up==0)
    											fprintf(cgiOut,"<td id=td2 width=150>%s</td>",uptemp);
    										else
										#endif
    										fprintf(cgiOut,"<td id=td2 width=150>%u</td>",acl_info.up);
    										fprintf(cgiOut,"</tr>");
    									fprintf(cgiOut,"<tr align=left>"\
    										"<td id=td1 width=170>%s</td>",search(lcontrol,"Source_DSCP"));
										#if 0
    										if(acl_info.dscp==0)
    										{
    											fprintf(cgiOut,"<td id=td2 width=150>%s</td>",dscptemp);
    										}
    										else
    										{
    										#endif
    											fprintf(cgiOut,"<td id=td2 width=150>%u</td>",acl_info.dscp);
    										//}
    										fprintf(cgiOut,"</tr>");
    									fprintf(cgiOut,"<tr align=left>"\
    										"<td id=td1 width=170>%s</td>",search(lcontrol,"Egress_UP"));
										
										
    										fprintf(cgiOut,"<td id=td2 width=150>%u</td>",acl_info.egrUP);
    										fprintf(cgiOut,"</tr>");
    									fprintf(cgiOut,"<tr align=left>"\
    										"<td id=td1 width=170>%s</td>",search(lcontrol,"Egress_DSCP"));
										
    										fprintf(cgiOut,"<td id=td2 width=150>%u</td>",acl_info.egrDSCP);
    										fprintf(cgiOut,"</tr>");	
                     				}
                     				fprintf(cgiOut,"<tr align=left>"\
    										"<td id=td1 width=170>%s</td>",search(lcontrol,"Modify_UP"));
    										fprintf(cgiOut,"<td id=td2 width=150>%s</td>",acl_info.upmm);
    										fprintf(cgiOut,"</tr>");
    									fprintf(cgiOut,"<tr align=left>"\
    										"<td id=td1 width=170>%s</td>",search(lcontrol,"Modify_DSCP"));
    										fprintf(cgiOut,"<td id=td2 width=150>%s</td>",acl_info.dscpmm);
    										fprintf(cgiOut,"</tr>");	
                     				
                     				if(acl_info.policerId!=0)
			 						{
                        					fprintf(cgiOut,"<tr align=left>"\
    										"<td id=td1 width=170>%s</td>","policerId");
    										fprintf(cgiOut,"<td id=td2 width=150>%u</td>",acl_info.policerId);
    										fprintf(cgiOut,"</tr>");
									}
									

                     				free(uptemp);
                     				free(dscptemp);
                     			}
                     		 }
					 #if 0		 	
                     		 {
                     		 	
                     		 		fprintf(cgiOut,"<tr align=left>"\
    										"<td id=td1 width=170>%s</td>",search(lcontrol,"ruleType"));
    										fprintf(cgiOut,"<td id=td2 width=150>%s</td>",acl_info.ruleType);
    										fprintf(cgiOut,"</tr>");
    								fprintf(cgiOut,"<tr align=left>"\
    										"<td id=td1 width=170>%s</td>","Action");
    										fprintf(cgiOut,"<td id=td2 width=150>%s</td>",acl_info.actype);
    										fprintf(cgiOut,"</tr>");
									fprintf(cgiOut,"<tr align=left>"\
										"<td id=td1 width=170>%s</td>","Protocol");
										fprintf(cgiOut,"<td id=td2 width=150>%s</td>",acl_info.protype);
										fprintf(cgiOut,"</tr>");
									fprintf(cgiOut,"<tr align=left>"\
										"<td id=td1 width=170>%s</td>",search(lcontrol,"destination_IP"));
										fprintf(cgiOut,"<td id=td2 width=150>%s</td>",acl_info.dip);
										fprintf(cgiOut,"</tr>");
									fprintf(cgiOut,"<tr align=left>"\
										"<td id=td1 width=170>%s</td>",search(lcontrol,"destination_port"));
										fprintf(cgiOut,"<td id=td2 width=150>%ld</td>",acl_info.dstport);
										fprintf(cgiOut,"</tr>");
    								fprintf(cgiOut,"<tr align=left>"\
    										"<td id=td1 width=170>%s</td>",search(lcontrol,"source_IP"));
    										fprintf(cgiOut,"<td id=td2 width=150>%s</td>",acl_info.sip);
    										fprintf(cgiOut,"</tr>");
    								fprintf(cgiOut,"<tr align=left>"\
    										"<td id=td1 width=170>%s</td>",search(lcontrol,"source_port"));
    										fprintf(cgiOut,"<td id=td2 width=150>%ld</td>",acl_info.srcport);
    										fprintf(cgiOut,"</tr>");
									fprintf(cgiOut,"<tr align=left>"\
										"<td id=td1 width=170>%s</td>",search(lcontrol,"acl_dmac"));
										fprintf(cgiOut,"<td id=td2 width=150>%s</td>",acl_info.dmac);
										fprintf(cgiOut,"</tr>");
									fprintf(cgiOut,"<tr align=left>"\
										"<td id=td1 width=170>%s</td>",search(lcontrol,"acl_smac"));
										fprintf(cgiOut,"<td id=td2 width=150>%s</td>",acl_info.smac);
										fprintf(cgiOut,"</tr>");
									fprintf(cgiOut,"<tr align=left>"\
										"<td id=td1 width=170>%s</td>","VlanID");
										fprintf(cgiOut,"<td id=td2 width=150>%u</td>",acl_info.vlanid);
										fprintf(cgiOut,"</tr>");
									fprintf(cgiOut,"<tr align=left>"\
    										"<td id=td1 width=170>%s</td>",search(lcontrol,"source_slot_port"));
    										fprintf(cgiOut,"<td id=td2 width=150>%s</td>",acl_info.source_port);
    										fprintf(cgiOut,"</tr>");
											
                         			 if (strcmp(acl_info.actype,"Redirect")==0)
                    				 {
                    				 	fprintf(cgiOut,"<tr align=left>"\
    										"<td id=td1 width=170>%s</td>",search(lcontrol,"redirect_port"));
    										fprintf(cgiOut,"<td id=td2 width=150>%s</td>",acl_info.redirect_port);
    										fprintf(cgiOut,"</tr>");
                    				 }
                    
                    				 if (strcmp(acl_info.actype,"MirrorToAnalyzer")==0)
                    				 {
                    				 	fprintf(cgiOut,"<tr align=left>"\
    										"<td id=td1 width=170>%s</td>",search(lcontrol,"analyzer_port"));
    										fprintf(cgiOut,"<td id=td2 width=150>%s</td>",acl_info.analyzer_port);
    										fprintf(cgiOut,"</tr>");
                    				 }
                                     
                         			if(acl_info.policerId!=0)
			 						{
                        					fprintf(cgiOut,"<tr align=left>"\
    										"<td id=td1 width=170>%s</td>","policerId");
    										fprintf(cgiOut,"<td id=td2 width=150>%u</td>",acl_info.policerId);
    										fprintf(cgiOut,"</tr>");
									}
									if(32==acl_info.appendIndex)
									{
										fprintf(cgiOut,"<tr align=left>"\
    										"<td id=td1 width=170>%s</td>",search(lcontrol,"QoS_Table"));
    										fprintf(cgiOut,"<td id=td2 width=150>%u</td>",acl_info.qosprofileindex);
    										fprintf(cgiOut,"</tr>");
									}
									

         				}
   					#endif
						//}
			  			fprintf(cgiOut,"<tr>");
					  if(cgiFormSubmitClicked("submit_acldta") != cgiFormSuccess)
					  {
						fprintf(cgiOut,"<td colspan=2><input type=hidden name=encry_addvlan value=%s></td>",encry);
					  }
					  else if(cgiFormSubmitClicked("submit_acldta") == cgiFormSuccess)
					  {
						fprintf(cgiOut,"<td colspan=2><input type=hidden name=encry_addvlan value=%s></td>",ruledetail_encry);
					  }
					  fprintf(cgiOut,"<input type=hidden name=INDEX value=%s>",index);
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
free(acl_info.ruleType);
free(acl_info.protype);
free(acl_info.dip);	
free(acl_info.sip);

free(acl_info.actype);	
free(acl_info.dmac);
free(acl_info.smac);

free(acl_info.source_port);
free(acl_info.redirect_port);
free(acl_info.analyzer_port);

free(acl_info.upmm);
free(acl_info.dscpmm);

release(lpublic);  
release(lcontrol);
return 0;
}

