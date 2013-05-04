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
* wp_aclgrplist.c
*
*
* CREATOR:
* autelan.software.Network Dep. team
* zhouym@autelan.com
*
* DESCRIPTION:
* system contrl for acl group list
*
*
*******************************************************************************/
#include <stdio.h>
#include "cgic.h"
#include <string.h>
#include <stdlib.h>
#include "ws_usrinfo.h"
#include "ws_init_dbus.h"

#include "ws_dcli_acl.h"

#define PageNum  10
#define MAX_GROUP_NUM  1024

int ShowACLGrplistPage();   


int cgiMain()
{
    ShowACLGrplistPage();    
 return 0;
}

int ShowACLGrplistPage()
{
	
	struct list *lpublic;	/*解析public.txt文件的链表头*/
	struct list *lcontrol; 	/*解析help.txt文件的链表头*/
	lpublic=get_chain_head("../htdocs/text/public.txt");
    lcontrol=get_chain_head("../htdocs/text/control.txt"); 
  char *encry=(char *)malloc(BUF_LEN);				/*存储从wp_usrmag.cgi带入的加密字符串*/
  char *aclGrplist_encry=(char *)malloc(BUF_LEN);
  char *PNtemp=(char *)malloc(10);
  char *SNtemp=(char *)malloc(10);
  char *str=NULL;
 // FILE *fp1;
  //char lan[3];
   struct group_info *max_groupnum = (struct group_info *)malloc(sizeof(struct group_info)*MAX_GROUP_NUM);
  
  int i,j;
  int retu=0;
  int cl=1;                 /*cl标识表格的底色，1为#f9fafe，0为#ffffff*/
	char *vIDTemp=(char *)malloc(10);
	char* i_char=(char *)malloc(10);
  	char menu[21]="menulist";
	int pageNum=0;
	char * pageNumCA=(char *)malloc(10);
	memset(pageNumCA,0,10);
	char * pageNumCD=(char *)malloc(10);
	memset(pageNumCD,0,10);

	char * DelAclG=(char *)malloc(10);
	memset(DelAclG,0,10);
	char * GrpIndex=(char *)malloc(10);
	memset(GrpIndex,0,10);
	char * GrpDir=(char *)malloc(10);
	memset(GrpDir,0,10);

	char * CheckUsr=(char *)malloc(10);
	memset(CheckUsr,0,10);
  for(i=0;i<MAX_GROUP_NUM;i++)
  	{
  		max_groupnum[i].groupIndex=0;
  		max_groupnum[i].ruleNumber=0;
		max_groupnum[i].groupType=0;
  		for(j=0;j<1024;j++)
			max_groupnum[i].ruleindex[j]=0;
  	}
  ccgi_dbus_init();
   if(cgiFormSubmitClicked("submit_GrpList") != cgiFormSuccess)
   {
  	memset(encry,0,BUF_LEN);
     cgiFormStringNoNewlines("UN", encry, BUF_LEN); 
     str=dcryption(encry);
     if(str==NULL)
     {
       ShowErrorPage(search(lpublic,"ill_user")); 	 /*用户非法*/
       return 0;
 	}
 		memset(aclGrplist_encry,0,BUF_LEN);                   /*清空临时变量*/
   }
   	memset(PNtemp,0,10);
	cgiFormStringNoNewlines("PN",PNtemp,10);
	pageNum=atoi(PNtemp);
	memset(SNtemp,0,10);
	cgiFormStringNoNewlines("SN",SNtemp,10);
	cgiFormStringNoNewlines("config_encry",aclGrplist_encry,BUF_LEN);
	cgiFormStringNoNewlines("DELACL",DelAclG,10);
	cgiFormStringNoNewlines("GrpIndex",GrpIndex,10);
	cgiFormStringNoNewlines("GrpDir",GrpDir,10);
	cgiFormStringNoNewlines("CheckUsr",CheckUsr,10);

	if(strcmp(CheckUsr,"")!=0)
		retu=atoi(CheckUsr);
	unsigned int dirtemp=0;

	
	cgiHeaderContentType("text/html");
	fprintf(cgiOut,"<html xmlns=\"http://www.w3.org/1999/xhtml\"><head>");
	fprintf(cgiOut,"<meta http-equiv=Content-Type content=text/html; charset=gb2312>");
	fprintf(cgiOut,"<title>%s</title>",search(lcontrol,"acl_conf"));
	fprintf(cgiOut,"<link rel=stylesheet href=/style.css type=text/css>"\
	  "<style type=text/css>"\
	  "#div1{ width:62px; height:18px; border:1px solid #666666; background-color:#f9f8f7;}"\
	  "#div2{ width:60px; height:15px; padding-left:5px; padding-top:3px}"\
	  "#link{ text-decoration:none; font-size: 12px}"\
	  ".configvlan {overflow-x:hidden;	overflow:auto; width: 480px; height=340;  clip: rect( ); padding-top: 0px; padding-right: 0px; padding-bottom: 0px; padding-left: 0px} "\
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

  if(strcmp(DelAclG,"delete")==0)
  {
	  dirtemp=atoi(GrpDir);
	  delete_acl_group(GrpIndex,dirtemp);
  }
  if(cgiFormSubmitClicked("submit_GrpList") != cgiFormSuccess)
  {
  	retu=checkuser_group(str);
  }
  else
  {
  		fprintf( cgiOut, "<script type='text/javascript'>\n" );
     	 fprintf( cgiOut, "window.location.href='wp_contrl.cgi?UN=%s';\n", aclGrplist_encry);
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
          "<td width=62 align=center><input id=but type=submit name=submit_GrpList style=background-image:url(/images/%s) value=""></td>",search(lpublic,"img_ok"));		  
		  if(cgiFormSubmitClicked("submit_GrpList") != cgiFormSuccess)
            fprintf(cgiOut,"<td width=62 align=left><a href=wp_aclall.cgi?UN=%s target=mainFrame><img src=/images/%s border=0 width=62 height=20/></a></td>",encry,search(lpublic,"img_cancel"));
		  else                                         
     		fprintf(cgiOut,"<td width=62 align=left><a href=wp_aclall.cgi?UN=%s target=mainFrame><img src=/images/%s border=0 width=62 height=20/></a></td>",aclGrplist_encry,search(lpublic,"img_cancel"));
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
					  if(cgiFormSubmitClicked("submit_GrpList") != cgiFormSuccess)
					  {
					  	  if(retu==0)
					  	  {
    					  	  fprintf(cgiOut,"<tr height=25>"\
    						  "<td align=left id=tdleft><a href=wp_aclall.cgi?UN=%s target=mainFrame class=top><font id=%s>%s</font></a></td>",encry,search(lpublic,"menu_san"),search(lcontrol,"acl_list")); 					 
    						  fprintf(cgiOut,"</tr>");
    						  fprintf(cgiOut,"<tr height=25>"\
    						  "<td align=left id=tdleft><a href=wp_addaclrule.cgi?UN=%s target=mainFrame class=top><font id=%s>%s</font></a></td>",encry,search(lpublic,"menu_san"),search(lcontrol,"add_acl_rule")); 					 
    						  fprintf(cgiOut,"</tr>");
    						  fprintf(cgiOut,"<tr height=26>"\
                    			"<td align=left id=tdleft background=/images/bottom_bg.gif style=\"border-right:0\"><font id=%s>%s</font></td>",search(lpublic,"menu_san"),search(lcontrol,"acl_grp_lsit"));   /*突出显示*/
                    			fprintf(cgiOut,"</tr>");
    						  fprintf(cgiOut,"<tr height=25>"\
                				"<td align=left id=tdleft><a href=wp_addaclgroup.cgi?UN=%s target=mainFrame class=top><font id=%s>%s</font></a></td>",encry,search(lpublic,"menu_san"),search(lcontrol,"add_acl_group"));
                				fprintf(cgiOut,"</tr>");
            			}
            			else
            			{
            				fprintf(cgiOut,"<tr height=25>"\
    						  "<td align=left id=tdleft><a href=wp_aclall.cgi?UN=%s target=mainFrame class=top><font id=%s>%s</font></a></td>",encry,search(lpublic,"menu_san"),search(lcontrol,"acl_list")); 					 
    						  fprintf(cgiOut,"</tr>");
						  	fprintf(cgiOut,"<tr height=26>"\
                			"<td align=left id=tdleft background=/images/bottom_bg.gif style=\"border-right:0\"><font id=%s>%s</font></td>",search(lpublic,"menu_san"),search(lcontrol,"acl_grp_lsit"));   /*突出显示*/
                			fprintf(cgiOut,"</tr>");
            			}

					  }
					  else if(cgiFormSubmitClicked("submit_GrpList") == cgiFormSuccess)				  
					  {
					      if(retu==0)
					      {
     							fprintf(cgiOut,"<tr height=25>"\
     						  "<td align=left id=tdleft><a href=wp_aclall.cgi?UN=%s target=mainFrame class=top><font id=%s>%s</font></a></td>",aclGrplist_encry,search(lpublic,"menu_san"),search(lcontrol,"acl_list")); 					 
     						  fprintf(cgiOut,"</tr>");
     						  fprintf(cgiOut,"<tr height=25>"\
     						  "<td align=left id=tdleft><a href=wp_addaclrule.cgi?UN=%s target=mainFrame class=top><font id=%s>%s</font></a></td>",aclGrplist_encry,search(lpublic,"menu_san"),search(lcontrol,"add_acl_rule")); 					 
     						  fprintf(cgiOut,"</tr>");
     						  fprintf(cgiOut,"<tr height=26>"\
                     			"<td align=left id=tdleft background=/images/bottom_bg.gif style=\"border-right:0\"><font id=%s>%s</font></td>",search(lpublic,"menu_san"),search(lcontrol,"acl_grp_lsit"));   /*突出显示*/
                     			fprintf(cgiOut,"</tr>");
     						  fprintf(cgiOut,"<tr height=25>"\
                 				"<td align=left id=tdleft><a href=wp_addaclgroup.cgi?UN=%s target=mainFrame class=top><font id=%s>%s</font></a></td>",aclGrplist_encry,search(lpublic,"menu_san"),search(lcontrol,"add_acl_group"));
                 				fprintf(cgiOut,"</tr>");
            			}
            			else
            				{
            					fprintf(cgiOut,"<tr height=25>"\
     						    "<td align=left id=tdleft><a href=wp_aclall.cgi?UN=%s target=mainFrame class=top><font id=%s>%s</font></a></td>",aclGrplist_encry,search(lpublic,"menu_san"),search(lcontrol,"acl_list")); 					 
     						    fprintf(cgiOut,"</tr>");
     						    fprintf(cgiOut,"<tr height=26>"\
                     			"<td align=left id=tdleft background=/images/bottom_bg.gif style=\"border-right:0\"><font id=%s>%s</font></td>",search(lpublic,"menu_san"),search(lcontrol,"acl_grp_lsit"));   /*突出显示*/
                     			fprintf(cgiOut,"</tr>");
            				}
					  }
					  int rowsCount=0;
					  if(retu==0)
					  	rowsCount=12;
					  else rowsCount=14;
					  for(i=0;i<rowsCount;i++)
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
													  "<td id=sec1 style=\"border-bottom:2px solid #53868b;font-size:14px;padding-left:0px;padding-top:0px\">%s</td>",search(lcontrol,"ACL_grp_info"));
													  fprintf(cgiOut,"</tr>"\
													"<tr>"\
													  "<td align=left valign=top  style=\"padding-top:18px\">"\
													  "<div class=configvlan><table width=280 border=1 frame=below rules=rows bordercolor=#cccccc cellspacing=0 cellpadding=0>"\
														   "<tr height=25 bgcolor=#eaeff9 style=font-size:12px align=left>"\
															"<th width=70 style=font-size:12px><font id=%s>%s</font></th>", search(lpublic,"menu_thead"),search(lcontrol,"group_index"));
															fprintf(cgiOut,"<th width=70 style=font-size:12px><font id=%s>%s</font></th>", search(lpublic,"menu_thead"),search(lcontrol,"group_type"));
															fprintf(cgiOut,"<th width=70 style=font-size:12px><font id=%s>%s</font></th>", search(lpublic,"menu_thead"),search(lcontrol,"group_rule_num"));
															fprintf(cgiOut,"<th width=13>&nbsp;</th>");
															fprintf(cgiOut,"</tr>");
															int GroupNum1=0,GroupNum2=0,GroupNum=0;
															show_group_list(0, max_groupnum,&GroupNum1,0);
															show_group_list(1, max_groupnum,&GroupNum2,GroupNum1);
															int xnt=0,head=0,tail=0;					  
															GroupNum=GroupNum1+GroupNum2;
															
															  if(0==strcmp(SNtemp,"PageDown") || 0==strcmp(SNtemp,""))
																  {
																	  if((GroupNum-(pageNum*PageNum)) < 0 )
																	  {
																		  pageNum=pageNum-1;
																		  ShowAlert(search(lcontrol,"Page_end")); 
																	  }
																	  if(GroupNum-pageNum*PageNum<PageNum)
																			  xnt=GroupNum;
																	  else	  xnt=(pageNum+1)*PageNum;
					  
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
																	  if(GroupNum-pageNum*PageNum<PageNum)
																			  xnt=GroupNum;
																	  else	  xnt=(pageNum+1)*PageNum;
																	  head=pageNum*PageNum;
																	  tail=xnt;
																  }
															   for(i=head;i<tail;i++)
																  {
																	  memset(menu,0,21);																		  
					  
																	   
																	  strcpy(menu,"menulist");
																	  sprintf(i_char,"%d",i+1);
																	  strcat(menu,i_char);
																	if(max_groupnum[i].groupType==0)
																	{
    																	fprintf(cgiOut,"<tr height=25 bgcolor=%s>",setclour(cl));
    																	fprintf(cgiOut,"<td style=font-size:12px align=left>%d</td>",max_groupnum[i].groupIndex);
    																	fprintf(cgiOut,"<td style=font-size:12px align=left>%s</td>","ingress");
    																	fprintf(cgiOut,"<td style=font-size:12px align=left>%d</td>",max_groupnum[i].ruleNumber);
																	}
																	else if(max_groupnum[i].groupType==1)
																	{											
    																	fprintf(cgiOut,"<tr height=25 bgcolor=%s>",setclour(cl));
    																	fprintf(cgiOut,"<td style=font-size:12px align=left>%d</td>",max_groupnum[i].groupIndex);
    																	fprintf(cgiOut,"<td style=font-size:12px align=left>%s</td>","egress");
    																	fprintf(cgiOut,"<td style=font-size:12px align=left>%d</td>",max_groupnum[i].ruleNumber);	
																	}
																	if(retu==0)
																	 {
    																	fprintf(cgiOut,"<td align=left>");
    																   fprintf(cgiOut,"<div style=\"position:relative; z-index:%d\" onmouseover=\"popMenu('%s');\" onmouseout=\"popMenu('%s');\">",(GroupNum-i),menu,menu);
    																   fprintf(cgiOut,"<img src=/images/detail.gif>"\
    																   "<div id=%s style=\"display:none; position:absolute; top:5px; left:0;\">",menu);
    																   fprintf(cgiOut,"<div id=div1>");
    																   if(cgiFormSubmitClicked("submit_GrpList") != cgiFormSuccess)
    																   {
    																   		fprintf(cgiOut,"<div id=div2 onmouseover=\"this.style.backgroundColor='#b6bdd2'\" onmouseout=\"this.style.backgroundColor='#f9f8f7'\"><a id=link href=wp_con_aclgrp.cgi?UN=%s&INDEX=%d&TYPE=%u target=mainFrame>%s</a></div>",encry,max_groupnum[i].groupIndex,max_groupnum[i].groupType,search(lpublic,"configure"));
    																   		fprintf(cgiOut,"<div id=div2 onmouseover=\"this.style.backgroundColor='#b6bdd2'\" onmouseout=\"this.style.backgroundColor='#f9f8f7'\"><a id=link href=wp_aclgrplist.cgi?UN=%s&DELACL=%s&GrpIndex=%u&GrpDir=%u target=mainFrame onclick=\"return confirm('%s')\">%s</a></div>",encry,"delete",max_groupnum[i].groupIndex,max_groupnum[i].groupType,search(lcontrol,"confirm_delete"),search(lcontrol,"del_group"));
    																   		fprintf(cgiOut,"<div id=div2 onmouseover=\"this.style.backgroundColor='#b6bdd2'\" onmouseout=\"this.style.backgroundColor='#f9f8f7'\"><a id=link href=wp_acl_port_info.cgi?UN=%s&INDEX=%d&TYPE=%u target=mainFrame>%s</a></div>",encry,max_groupnum[i].groupIndex,max_groupnum[i].groupType,search(lpublic, "portinfo"));
    																   }
    																   else if(cgiFormSubmitClicked("submit_GrpList") == cgiFormSuccess)
    																   {
    																   		fprintf(cgiOut,"<div id=div2 onmouseover=\"this.style.backgroundColor='#b6bdd2'\" onmouseout=\"this.style.backgroundColor='#f9f8f7'\"><a id=link href=wp_con_aclgrp.cgi?UN=%s&INDEX=%d&TYPE=%u target=mainFrame>%s</a></div>",aclGrplist_encry,max_groupnum[i].groupIndex,max_groupnum[i].groupType,search(lpublic,"configure"));
    																		fprintf(cgiOut,"<div id=div2 onmouseover=\"this.style.backgroundColor='#b6bdd2'\" onmouseout=\"this.style.backgroundColor='#f9f8f7'\"><a id=link href=wp_aclgrplist.cgi?UN=%s&DELACL=%s&GrpIndex=%u&GrpDir=%u target=mainFrame onclick=\"return confirm('%s')\">%s</a></div>",aclGrplist_encry,"delete",max_groupnum[i].groupIndex,max_groupnum[i].groupType,search(lcontrol,"confirm_delete"),search(lcontrol,"del_group"));
    																		fprintf(cgiOut,"<div id=div2 onmouseover=\"this.style.backgroundColor='#b6bdd2'\" onmouseout=\"this.style.backgroundColor='#f9f8f7'\"><a id=link href=wp_acl_port_info.cgi?UN=%s&INDEX=%u&TYPE=%u target=mainFrame>%s</a></div>",aclGrplist_encry,max_groupnum[i].groupIndex,max_groupnum[i].groupType,search(lpublic,"portinfo"));
    																   }
																  
    																   fprintf(cgiOut,"</div>"\
    																   "</div>"\
    																   "</div>");
    															
    																 fprintf(cgiOut,"</td>");
																  }
																  else
																  {
    																	fprintf(cgiOut,"<td align=left>");
    																   fprintf(cgiOut,"<div style=\"position:relative; z-index:%d\" onmouseover=\"popMenu('%s');\" onmouseout=\"popMenu('%s');\">",(GroupNum-i),menu,menu);
    																   fprintf(cgiOut,"<img src=/images/detail.gif>"\
    																   "<div id=%s style=\"display:none; position:absolute; top:5px; left:0;\">",menu);
    																   fprintf(cgiOut,"<div id=div1>");
    																   if(cgiFormSubmitClicked("submit_GrpList") != cgiFormSuccess)
    																   {
    																   		//fprintf(cgiOut,"<div id=div2 onmouseover=\"this.style.backgroundColor='#b6bdd2'\" onmouseout=\"this.style.backgroundColor='#f9f8f7'\"><a id=link href=wp_con_aclgrp.cgi?UN=%s&INDEX=%d&TYPE=%u target=mainFrame>%s</a></div>",encry,grpInfo[i].groupIndex,grpInfo[i].groupType,search(lpublic,"configure"));
    																   		//fprintf(cgiOut,"<div id=div2 onmouseover=\"this.style.backgroundColor='#b6bdd2'\" onmouseout=\"this.style.backgroundColor='#f9f8f7'\"><a id=link href=wp_aclgrplist.cgi?UN=%s&DELACL=%s&GrpIndex=%u&GrpDir=%u target=mainFrame onclick=\"return confirm('%s')\">%s</a></div>",encry,"delete",grpInfo[i].groupIndex,grpInfo[i].groupType,search(lcontrol,"confirm_delete"),search(lcontrol,"del_group"));
    																   		fprintf(cgiOut,"<div id=div2 onmouseover=\"this.style.backgroundColor='#b6bdd2'\" onmouseout=\"this.style.backgroundColor='#f9f8f7'\"><a id=link href=wp_acl_port_info.cgi?UN=%s&INDEX=%d&TYPE=%u target=mainFrame>%s</a></div>",encry,max_groupnum[i].groupIndex,max_groupnum[i].groupType,search(lpublic, "portinfo"));
    																   }
    																   else if(cgiFormSubmitClicked("submit_GrpList") == cgiFormSuccess)
    																   {
    																   		//fprintf(cgiOut,"<div id=div2 onmouseover=\"this.style.backgroundColor='#b6bdd2'\" onmouseout=\"this.style.backgroundColor='#f9f8f7'\"><a id=link href=wp_con_aclgrp.cgi?UN=%s&INDEX=%d&TYPE=%u target=mainFrame>%s</a></div>",aclGrplist_encry,grpInfo[i].groupIndex,grpInfo[i].groupType,search(lpublic,"configure"));
    																		//fprintf(cgiOut,"<div id=div2 onmouseover=\"this.style.backgroundColor='#b6bdd2'\" onmouseout=\"this.style.backgroundColor='#f9f8f7'\"><a id=link href=wp_aclgrplist.cgi?UN=%s&DELACL=%s&GrpIndex=%u&GrpDir=%u target=mainFrame onclick=\"return confirm('%s')\">%s</a></div>",aclGrplist_encry,"delete",grpInfo[i].groupIndex,grpInfo[i].groupType,search(lcontrol,"confirm_delete"),search(lcontrol,"del_group"));
    																		fprintf(cgiOut,"<div id=div2 onmouseover=\"this.style.backgroundColor='#b6bdd2'\" onmouseout=\"this.style.backgroundColor='#f9f8f7'\"><a id=link href=wp_acl_port_info.cgi?UN=%s&INDEX=%u&TYPE=%u target=mainFrame>%s</a></div>",aclGrplist_encry,max_groupnum[i].groupIndex,max_groupnum[i].groupType,search(lpublic,"portinfo"));
    																   }
																  
    																   fprintf(cgiOut,"</div>"\
    																   "</div>"\
    																   "</div>");
    															
    																 fprintf(cgiOut,"</td>");
																  }
																 fprintf(cgiOut,"</tr>");
																 cl=!cl;
																 }
													  fprintf(cgiOut,"</table></div></td>"\
													  "</tr>"\
													  "<tr>"\
													  "<td>"\
													  "<table width=430 style=padding-top:2px>"\
													  "<tr>");
													  sprintf(pageNumCA,"%d",pageNum+1);
													  sprintf(pageNumCD,"%d",pageNum-1);
													  if(cgiFormSubmitClicked("submit_GrpList") != cgiFormSuccess)
														  {
															  fprintf(cgiOut,"<td align=center style=padding-top:2px><a href=wp_aclgrplist.cgi?UN=%s&PN=%s&SN=%s>%s</td>",encry,pageNumCA,"PageDown",search(lcontrol,"page_down"));
															  fprintf(cgiOut,"<td align=center style=padding-top:2px><a href=wp_aclgrplist.cgi?UN=%s&PN=%s&SN=%s>%s</td>",encry,pageNumCD,"PageUp",search(lcontrol,"page_up"));
														  }
													  else if(cgiFormSubmitClicked("submit_GrpList") == cgiFormSuccess)
														  {
															  fprintf(cgiOut,"<td align=center style=padding-top:2px><a href=wp_aclgrplist.cgi?UN=%s&PN=%s&SN=%s>%s</td>",aclGrplist_encry,pageNumCA,"PageDown",search(lcontrol,"page_down"));
															  fprintf(cgiOut,"<td align=center style=padding-top:2px><a href=wp_aclgrplist.cgi?UN=%s&PN=%s&SN=%s>%s</td>",aclGrplist_encry,pageNumCD,"PageUp",search(lcontrol,"page_up"));  
														  }
													  fprintf(cgiOut,"</tr></table></td>"\
														  "</tr>"\
												  "<tr>");
												  if(cgiFormSubmitClicked("submit_GrpList") != cgiFormSuccess)
												  {
													fprintf(cgiOut,"<td><input type=hidden name=config_encry value=%s></td>",encry);
													fprintf(cgiOut,"<td><input type=hidden name=CheckUsr value=%d></td>",retu);
												  }
												  else if(cgiFormSubmitClicked("submit_GrpList") == cgiFormSuccess)
													  { 			 
														fprintf(cgiOut,"<td><input type=hidden name=config_encry value=%s></td>",aclGrplist_encry);
														fprintf(cgiOut,"<td><input type=hidden name=CheckUsr value=%d></td>",retu);
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
free(i_char);
free(PNtemp);
free(CheckUsr);
free(SNtemp);
free(vIDTemp);
free(pageNumCA);
free(pageNumCD);
free(DelAclG);
free(GrpIndex);
free(GrpDir);
free(encry);
release(lpublic);  
release(lcontrol);															 
return 0;
}

