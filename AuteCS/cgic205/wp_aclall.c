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
* wp_aclall.c
*
*
* CREATOR:
* autelan.software.Network Dep. team
* zhouym@autelan.com
*
* DESCRIPTION:
* system contrl for acl list
*
*
*******************************************************************************/
#include <stdio.h>
#include "cgic.h"
#include <string.h>
#include <stdlib.h>
#include "ws_usrinfo.h"
#include <sys/wait.h>
#include "ws_dcli_acl.h"
#include "ws_sysinfo.h"
#include "ws_init_dbus.h"


#define PageNum  10
#define MAX_ACL_NUM 4096



int ShowACLListPage();

int cgiMain()
{
 ShowACLListPage();
 return 0;
}

int ShowACLListPage()
{
	struct list *lpublic;	/*解析public.txt文件的链表头*/
	struct list *lcontrol; 	/*解析help.txt文件的链表头*/
	lpublic=get_chain_head("../htdocs/text/public.txt");
       lcontrol=get_chain_head("../htdocs/text/control.txt"); 	
	  char *encry=(char *)malloc(BUF_LEN);
	  char *PNtemp=(char *)malloc(10);
	  char *SNtemp=(char *)malloc(10);
	  char *str=NULL;
	  //FILE *fp;
	 // char lan[3];
	  char acllist_encry[BUF_LEN]; 
	  char addn[N];         
	  int i;
	  char menu[21]="menulist";
	  char* i_char=(char *)malloc(10);
	  int cl=1;
  	int pageNum=0;
	char * pageNumCA=(char *)malloc(10);
	memset(pageNumCA,0,10);
	char * pageNumCD=(char *)malloc(10);
	memset(pageNumCD,0,10);

	char * DelAcl=(char *)malloc(10);
	memset(DelAcl,0,10);
	char * ruleindex=(char *)malloc(10);
	memset(ruleindex,0,10);
	char * Gindex=(char *)malloc(10);
	memset(Gindex,0,10);
	char * Director=(char *)malloc(10);
	memset(Director,0,10);
	char * ACL_Enable=(char *)malloc(10);
	memset(ACL_Enable,0,10);

	char * CheckUsr=(char *)malloc(10);
	memset(CheckUsr,0,10);

	int retu=0;
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
  ccgi_dbus_init();
	
  if(cgiFormSubmitClicked("submit_acllist") != cgiFormSuccess && cgiFormSubmitClicked("enableACL") != cgiFormSuccess)
  {
	memset(encry,0,BUF_LEN);
    cgiFormStringNoNewlines("UN", encry, BUF_LEN); 
    str=dcryption(encry);
    if(str==NULL)
    {
      ShowErrorPage(search(lpublic,"ill_user")); 	 /*用户非法*/
      return 0;
	}
	strcpy(addn,str);
	memset(acllist_encry,0,BUF_LEN);                   /*清空临时变量*/
  }
  
    memset(PNtemp,0,10);
	cgiFormStringNoNewlines("PN",PNtemp,10);
	pageNum=atoi(PNtemp);
	memset(SNtemp,0,10);
	cgiFormStringNoNewlines("SN",SNtemp,10);
 	cgiFormStringNoNewlines("encry_acllist",acllist_encry,BUF_LEN);
  	cgiFormStringNoNewlines("DELRULE", DelAcl, 10);
	cgiFormStringNoNewlines("INDEX", ruleindex, 10);
	cgiFormStringNoNewlines("GINDEX", Gindex, 10);
	cgiFormStringNoNewlines("DIR", Director, 10);
	cgiFormStringNoNewlines("ACL_Enable", ACL_Enable, 10);
	cgiFormStringNoNewlines("CheckUsr", CheckUsr, 10);

	if(strcmp(CheckUsr,"")!=0)
		retu=atoi(CheckUsr);	
	

  cgiHeaderContentType("text/html");
  fprintf(cgiOut,"<html xmlns=\"http://www.w3.org/1999/xhtml\"><head>");
  fprintf(cgiOut,"<meta http-equiv=Content-Type content=text/html; charset=gb2312>");
  fprintf(cgiOut,"<title>%s</title>",search(lcontrol,"acl_conf"));
  fprintf(cgiOut,"<link rel=stylesheet href=/style.css type=text/css>"\
  	"<style type=text/css>"\
  	"#div1{ width:62px; height:18px; border:1px solid #666666; background-color:#f9f8f7;}"\
	  "#div2{ width:60px; height:15px; padding-left:5px; padding-top:3px}"\
	  "#link{ text-decoration:none; font-size: 12px}"\
  	".ShowACL {overflow-x:hidden;  overflow:auto; width: 600px; height: 330px; clip: rect( ); padding-top: 2px; padding-right: 0px; padding-bottom: 0px; padding-left: 0px} "\
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

  if(cgiFormSubmitClicked("submit_acllist") != cgiFormSuccess && cgiFormSubmitClicked("enableACL") != cgiFormSuccess)
  {
  	 retu=checkuser_group(str);
  }
  else if(cgiFormSubmitClicked("submit_acllist") == cgiFormSuccess)
	{
		 fprintf( cgiOut, "<script type='text/javascript'>\n" );
     	 fprintf( cgiOut, "window.location.href='wp_contrl.cgi?UN=%s';\n", acllist_encry);
     	 fprintf( cgiOut, "</script>\n" );
	}
	//unsigned int gIndexTemp=0,dirtemp=0;
  if(strcmp(DelAcl,"delete")==0)
  {
  	//if(Gindex!=0)
  	//{
  	//	gIndexTemp=atoi(Gindex);
  	//	dirtemp=atoi(Director);
  	//	add_rule_group("delete",ruleindex,gIndexTemp,dirtemp);
  	//}
  	delete_acl_rule(ruleindex);
  }
  if(cgiFormSubmitClicked("enableACL") == cgiFormSuccess)
	{
		//if(strcmp(ACL_Enable,"enable")==0)
		//fprintf(stderr,"ACL_Enable=%s",ACL_Enable);	
		
			acl_service_glabol_enable(ACL_Enable,lcontrol);
		//else if(strcmp(ACL_Enable,"disable")==0)
			//acl_service_glabol_enable();
	}

  fprintf(cgiOut,"<form method=post>"\
  "<div align=center>"\
  "<table width=976 border=0 cellpadding=0 cellspacing=0>"\
  "<tr>"\
    "<td width=8 align=left valign=top background=/images/di22.jpg><img src=/images/youce4.jpg width=8 height=30/></td>"\
    "<td width=51 align=left valign=bottom background=/images/di22.jpg><img src=/images/youce33.jpg width=37 height=24/></td>"\
    "<td width=153 align=left valign=bottom background=/images/di22.jpg><font id=titleen>ACL</font><font id=%s> %s</font></td>",search(lpublic,"title_style"),search(lpublic,"management"));
    fprintf(cgiOut,"<td width=690 align=right valign=bottom background=/images/di22.jpg>");
	   // if((fp=fopen("../htdocs/text/public.txt","r"))==NULL)		 /*以只读方式打开资源文件*/
	    //  ShowAlert(search(lpublic,"error_open"));
	   // fseek(fp,4,0); 						/*将文件指针移到离文件首4个字节处，即lan=之后*/
	   // fgets(lan,3,fp);	   
		//fclose(fp);
	   // if(strcmp(lan,"ch")==0)
    	//{	
    	  fprintf(cgiOut,"<table width=130 border=0 cellspacing=0 cellpadding=0>"\
          "<tr>"\
          "<td width=62 align=center><input id=but type=submit name=submit_acllist style=background-image:url(/images/%s) value=""></td>",search(lpublic,"img_ok"));		  
		  if(cgiFormSubmitClicked("submit_acllist") != cgiFormSuccess && cgiFormSubmitClicked("enableACL") != cgiFormSuccess)
            fprintf(cgiOut,"<td width=62 align=left><a href=wp_contrl.cgi?UN=%s target=mainFrame><img src=/images/%s border=0 width=62 height=20/></a></td>",encry,search(lpublic,"img_cancel"));
		  else                                         
     		fprintf(cgiOut,"<td width=62 align=left><a href=wp_contrl.cgi?UN=%s target=mainFrame><img src=/images/%s border=0 width=62 height=20/></a></td>",acllist_encry,search(lpublic,"img_cancel"));
		  fprintf(cgiOut,"</tr>"\
          "</table>");
		//}	
		/*
		else			
		{	
		  fprintf(cgiOut,"<table width=130 border=0 cellspacing=0 cellpadding=0>"\
		  "<tr>"\
		  "<td width=62 align=center><input id=but type=submit name=submit_acllist style=background-image:url(/images/ok-en.jpg) value=""></td>");		  
		  if(cgiFormSubmitClicked("submit_acllist") != cgiFormSuccess && cgiFormSubmitClicked("enableACL") != cgiFormSuccess)
		    fprintf(cgiOut,"<td width=62 align=left><a href=wp_contrl.cgi?UN=%s target=mainFrame><img src=/images/cancel-en.jpg border=0 width=62 height=20/></a></td>",encry);
		  else
		    fprintf(cgiOut,"<td width=62 align=left><a href=wp_contrl.cgi?UN=%s target=mainFrame><img src=/images/cancel-en.jpg border=0 width=62 height=20/></a></td>",acllist_encry);
		  fprintf(cgiOut,"</tr>"\
		  "</table>");
		}
		*/
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
         			"<td align=left id=tdleft background=/images/bottom_bg.gif style=\"border-right:0\"><font id=%s>%s</font></td>",search(lpublic,"menu_san"),search(lcontrol,"acl_list"));   /*突出显示*/
         			fprintf(cgiOut,"</tr>");
         		if(cgiFormSubmitClicked("submit_acllist") != cgiFormSuccess && cgiFormSubmitClicked("enableACL") != cgiFormSuccess)
         		{
         			if(retu==0)
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
         			else
         			{
         				fprintf(cgiOut,"<tr height=25>"\
             			"<td align=left id=tdleft><a href=wp_aclgrplist.cgi?UN=%s target=mainFrame class=top><font id=%s>%s</font></a></td>",encry,search(lpublic,"menu_san"),search(lcontrol,"acl_grp_lsit"));					   
             			fprintf(cgiOut,"</tr>");
         			}
         		}
         		else	  
         		{
         			if(retu==0)
					{
            			fprintf(cgiOut,"<tr height=25>"\
             			"<td align=left id=tdleft><a href=wp_addaclrule.cgi?UN=%s target=mainFrame style=color:#000000><font id=%s>%s</font></a></td>",acllist_encry,search(lpublic,"menu_san"),search(lcontrol,"add_acl_rule"));						
             			fprintf(cgiOut,"</tr>");
             			fprintf(cgiOut,"<tr height=25>"\
             			"<td align=left id=tdleft><a href=wp_aclgrplist.cgi?UN=%s target=mainFrame style=color:#000000><font id=%s>%s</font></a></td>",acllist_encry,search(lpublic,"menu_san"),search(lcontrol,"acl_grp_lsit"));						
             			fprintf(cgiOut,"</tr>"\
             			"<tr height=25>"\
             			"<td align=left id=tdleft><a href=wp_addaclgroup.cgi?UN=%s target=mainFrame style=color:#000000><font id=%s>%s</font></a></td>",acllist_encry,search(lpublic,"menu_san"),search(lcontrol,"add_acl_group"));
             			fprintf(cgiOut,"</tr>");
         			}
         			else
         			{
         				fprintf(cgiOut,"<tr height=25>"\
            			"<td align=left id=tdleft><a href=wp_aclgrplist.cgi?UN=%s target=mainFrame style=color:#000000><font id=%s>%s</font></a></td>",acllist_encry,search(lpublic,"menu_san"),search(lcontrol,"acl_grp_lsit"));						
            			fprintf(cgiOut,"</tr>");
         			}
         		}
         		int rowsCount=0;
         		if(retu==0)
         			rowsCount=13;
         		else rowsCount=15;
				for(i=0;i<rowsCount;i++)
					{
						fprintf(cgiOut,"<tr height=25>"\
						  "<td id=tdleft>&nbsp;</td>"\
						"</tr>");
					}

				  fprintf(cgiOut,"</table>"\
              "</td>"\
              "<td align=left valign=top style=\"background-color:#ffffff; border-right:1px solid #707070; padding-left:30px; padding-top:10px\">"\
						"<table width=640 height=385 border=0 cellspacing=0 cellpadding=0>"\
						"<tr>"\
						"<td id=sec1 style=\"border-bottom:2px solid #53868b;font-size:14px\" colspan=4>%s</td>",search(lcontrol,"ACL_Global_Enable"));
						fprintf(cgiOut,"</tr>"\
						"<tr style=padding-top:8px>");
						fprintf(cgiOut,"<td align=left width=100>%s: </td>",search(lcontrol,"ACL_enable"));
						fprintf(cgiOut,"<td align=left width=100>");
						if(retu==0)
							fprintf(cgiOut, "<select name=\"ACL_Enable\">\n");
						else
							fprintf(cgiOut, "<select name=\"ACL_Enable\" disabled>\n");
						fprintf(cgiOut, "<option value=enable>Enable\n");
	                  	fprintf(cgiOut, "<option value=disable>Disable\n");
	                  	fprintf(cgiOut, "</select>\n");				
						fprintf(cgiOut,"</td>");
						if(retu==0)
							fprintf(cgiOut,"<td align=left width=80><input type=submit style=width:70px; height:36px  border=0 name=enableACL style=background-image:url(/images/SubBackGif.gif) value=\"%s\"></td>",search(lcontrol,"enable"));
						else
							fprintf(cgiOut,"<td align=left width=80><input type=submit style=width:70px; height:36px  border=0 name=enableACL disabled style=background-image:url(/images/SubBackGif.gif) value=\"%s\"></td>",search(lcontrol,"enable"));
						char * acl_service=show_acl_service();
						if(strcmp(acl_service,"enable")==0)
							fprintf(cgiOut,"<td align=left  width=360 style=color:red>%s</td>",search(lcontrol,"Now_enable"));
						else if(strcmp(acl_service,"disable")==0)
							fprintf(cgiOut,"<td align=left  width=360 style=color:red>%s</td>",search(lcontrol,"Now_disable"));
						else fprintf(cgiOut,"<td align=left  width=360 style=color:red>%s</td>",search(lcontrol,"Now_disable"));
							
						fprintf(cgiOut,"</tr>"\
						"<tr>"\
						"<td id=sec1 style=\"border-bottom:2px solid #53868b;font-size:14px\" colspan=4>%s</td>",search(lcontrol,"ACL_RULE"));
						fprintf(cgiOut,"</tr>"\
						 "<tr>"\
						   "<td align=left valign=top style=padding-top:18px colspan=4>"\
						   "<div class=ShowACL><table width=503 border=1 frame=below rules=rows bordercolor=#cccccc cellspacing=0 cellpadding=0>");
							 fprintf(cgiOut,"<tr height=30 bgcolor=#eaeff9 id=td1 align=left>");
							 fprintf(cgiOut,"<th width=80 style=font-size:12px><font id=%s>%s</font></th>", search(lpublic,"menu_thead"),search(lcontrol,"acl_index"));
							 fprintf(cgiOut,"<th width=80 style=font-size:12px><font id=%s>%s</font></th>", search(lpublic,"menu_thead"),search(lcontrol,"rule_group"));
							 fprintf(cgiOut,"<th width=120 style=font-size:12px><font id=%s>%s</font></th>", search(lpublic,"menu_thead"),search(lcontrol,"rule"));
							 fprintf(cgiOut,"<th width=70 style=font-size:12px><font id=%s>%s</font></th>", search(lpublic,"menu_thead"),search(lcontrol,"action"));
							 fprintf(cgiOut,"<th width=70 style=font-size:12px><font id=%s>%s</font></th>", search(lpublic,"menu_thead"),search(lcontrol,"Protocol"));
							 fprintf(cgiOut,"<th width=13 style=font-size:12px>&nbsp;</th>");
							 fprintf(cgiOut,"</tr>");
							 int aclNum=0;
							 int q=0;
							 int xnt=0,head=0,tail=0;
							unsigned int groupindex=0,groupType=0;
							 int k=show_acl_allinfo(receive_acl,&aclNum);
							 if(k==0)
							 {
									 if(aclNum==0)
										 {
											 head=0;
											 tail=0;
										 }
     									else if(0==strcmp(SNtemp,"PageDown") || 0==strcmp(SNtemp,""))
     										{
     											if(aclNum-pageNum*PageNum<=0)
     												{
     													pageNum=pageNum-1;
     													ShowAlert(search(lcontrol,"Page_end")); 
     												}
     											if(aclNum-pageNum*PageNum<PageNum)
     													xnt=aclNum;
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
     												if(aclNum-pageNum*PageNum<PageNum)
     														xnt=aclNum;
     												else	  xnt=(pageNum+1)*PageNum;
     												
     												head=pageNum*PageNum;
     												tail=xnt;
     											}
									 for(i=head;i<tail;i++)
										 {
										 	groupindex=0;
										 	memset(menu,0,21);
										  	strcpy(menu,"menulist");
										  	sprintf(i_char,"%d",i+1);
										  	strcat(menu,i_char);
										  	q=show_group_ByRuleIndex(receive_acl[i].ruleIndex,0,&groupindex,&groupType);
										  	if(groupindex==0)
										  		show_group_ByRuleIndex(receive_acl[i].ruleIndex,1,&groupindex,&groupType);
    											 fprintf(cgiOut,"<tr height=25 bgcolor=%s align=left>",setclour(cl));
    											 fprintf(cgiOut,"<td style=font-size:12px align=left>%d</td>",receive_acl[i].ruleIndex);
    											if(groupindex==0)
    											{
    											 	fprintf(cgiOut,"<td style=font-size:12px align=left>%s</td>",search(lcontrol,"not_add_aclgrp"));
    											}
    											else
    											{
    											 	fprintf(cgiOut,"<td style=font-size:12px align=left>%d</td>",groupindex);
    											}
    											 fprintf(cgiOut,"<td style=font-size:12px align=left>%s</td>",receive_acl[i].ruleType);
    											 fprintf(cgiOut,"<td style=font-size:12px align=left>%s</td>",receive_acl[i].actype);
    											 fprintf(cgiOut,"<td style=font-size:12px align=left>%s</td>",receive_acl[i].protype);
    											 fprintf(cgiOut,"<td align=center>");
    											 fprintf(cgiOut,"<div style=\"position:relative; z-index:%d\" onmouseover=\"popMenu('%s');\" onmouseout=\"popMenu('%s');\">",(aclNum-i),menu,menu);
    																   fprintf(cgiOut,"<img src=/images/detail.gif>"\
    																   "<div id=%s style=\"display:none; position:absolute; top:5px; left:0;\">",menu);
    																   fprintf(cgiOut,"<div id=div1>");
    																   	if(cgiFormSubmitClicked("submit_acllist") != cgiFormSuccess && cgiFormSubmitClicked("enableACL") != cgiFormSuccess)
    																   	{
    																   		fprintf(cgiOut,"<div id=div2 onmouseover=\"this.style.backgroundColor='#b6bdd2'\" onmouseout=\"this.style.backgroundColor='#f9f8f7'\"><a id=link href=wp_aclruledta.cgi?UN=%s&INDEX=%d target=mainFrame>%s</a></div>",encry,receive_acl[i].ruleIndex,search(lpublic,"details"));
    																   		if(retu==0)
    																   		{
    																   			fprintf(cgiOut,"<div id=div2 onmouseover=\"this.style.backgroundColor='#b6bdd2'\" onmouseout=\"this.style.backgroundColor='#f9f8f7'\"><a id=link href=wp_aclall.cgi?UN=%s&INDEX=%u&GINDEX=%u&DIR=%u&DELRULE=%s target=mainFrame onclick=\"return confirm('%s')\">%s</a></div>",encry,receive_acl[i].ruleIndex,groupindex,groupType,"delete",search(lcontrol,"confirm_delete"),search(lcontrol,"Delete_acl"));
																			if(strcmp(receive_acl[i].ruleType,"extended")==0)
																			{
																				if(get_product_id()!=PRODUCT_ID_AU3K_BCM)
																				{
																					fprintf(cgiOut,"<div id=div2 onmouseover=\"this.style.backgroundColor='#b6bdd2'\" onmouseout=\"this.style.backgroundColor='#f9f8f7'\"><a id=link href=wp_qosappend_acl.cgi?UN=%s&INDEX=%d target=mainFrame>%s</a></div>",encry,receive_acl[i].ruleIndex,search(lcontrol,"addQos"));		
																				}
																			}
    																   		}
    																   	}
    																   	else
    																   	{
    																   		fprintf(cgiOut,"<div id=div2 onmouseover=\"this.style.backgroundColor='#b6bdd2'\" onmouseout=\"this.style.backgroundColor='#f9f8f7'\"><a id=link href=wp_aclruledta.cgi?UN=%s&INDEX=%d target=mainFrame>%s</a></div>",acllist_encry,receive_acl[i].ruleIndex,search(lpublic,"details"));
    																   		if(retu==0)
    																   		{
    																   			fprintf(cgiOut,"<div id=div2 onmouseover=\"this.style.backgroundColor='#b6bdd2'\" onmouseout=\"this.style.backgroundColor='#f9f8f7'\"><a id=link href=wp_aclall.cgi?UN=%s&INDEX=%u&GINDEX=%u&DIR=%u&DELRULE=%s target=mainFrame onclick=\"return confirm('%s')\">%s</a></div>",acllist_encry,receive_acl[i].ruleIndex,groupindex,groupType,"delete",search(lcontrol,"confirm_delete"),search(lcontrol,"Delete_acl"));
																			if(strcmp(receive_acl[i].ruleType,"extended")==0)
																			{
																				if(get_product_id()!=PRODUCT_ID_AU3K_BCM)
																				{
																					fprintf(cgiOut,"<div id=div2 onmouseover=\"this.style.backgroundColor='#b6bdd2'\" onmouseout=\"this.style.backgroundColor='#f9f8f7'\"><a id=link href=wp_qosappend_acl.cgi?UN=%s&INDEX=%d target=mainFrame>%s</a></div>",acllist_encry,receive_acl[i].ruleIndex,search(lcontrol,"addQos"));		
																				}
																			}
    																   		}
    																   	}
    																   fprintf(cgiOut,"</div>"\
    																   "</div>"\
    																   "</div>");
    										 	fprintf(cgiOut,"</td>");
    											 fprintf(cgiOut,"</tr>");
    									
    											 cl=!cl;
											//}
										 }
								 
							 }
						   fprintf(cgiOut,"</table></div>"\
							 "</td>"\
						   "</tr>"\
			   			
						 "<tr>"\
						"<td  colspan=4>"\
						"<table width=430 style=padding-top:2px>"\
						"<tr>");
						sprintf(pageNumCA,"%d",pageNum+1);
						sprintf(pageNumCD,"%d",pageNum-1);
						if(cgiFormSubmitClicked("submit_acllist") != cgiFormSuccess && cgiFormSubmitClicked("enableACL") != cgiFormSuccess)
							{
								fprintf(cgiOut,"<td align=center style=padding-top:2px><a href=wp_aclall.cgi?UN=%s&PN=%s&SN=%s>%s</td>",encry,pageNumCA,"PageDown",search(lcontrol,"page_down"));
								fprintf(cgiOut,"<td align=center style=padding-top:2px><a href=wp_aclall.cgi?UN=%s&PN=%s&SN=%s>%s</td>",encry,pageNumCD,"PageUp",search(lcontrol,"page_up"));
							}
						else
							{
								fprintf(cgiOut,"<td align=center style=padding-top:2px><a href=wp_aclall.cgi?UN=%s&PN=%s&SN=%s>%s</td>",acllist_encry,pageNumCA,"PageDown",search(lcontrol,"page_down"));
								fprintf(cgiOut,"<td align=center style=padding-top:2px><a href=wp_aclall.cgi?UN=%s&PN=%s&SN=%s>%s</td>",acllist_encry,pageNumCD,"PageUp",search(lcontrol,"page_up"));  
							}
						fprintf(cgiOut,"</tr></table></td>"\
						"</tr>"\
						 "<tr>");
						 if(cgiFormSubmitClicked("submit_acllist") != cgiFormSuccess && cgiFormSubmitClicked("enableACL") != cgiFormSuccess)
						 {
						   fprintf(cgiOut,"<td><input type=hidden name=encry_acllist value=%s></td>",encry);
						   fprintf(cgiOut,"<td><input type=hidden name=CheckUsr value=%d></td>",retu);
						 }
						 else
						 {
						   fprintf(cgiOut,"<td><input type=hidden name=encry_acllist value=%s></td>",acllist_encry);
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
free(encry);
free(CheckUsr);
free(ACL_Enable);

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
free(i_char);
free(PNtemp);
free(SNtemp);
free(pageNumCA);
free(pageNumCD);
free(DelAcl);
free(ruleindex);
free(Gindex);
free(Director);
release(lpublic); 
release(lcontrol);

return 0;
}

