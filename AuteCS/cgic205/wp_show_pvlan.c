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
* wp_show_pvlan.c
*
*
* CREATOR:
* autelan.software.Network Dep. team
* tangsq@autelan.com
*
* DESCRIPTION:
* system contrl for pvlan config
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
#include "ws_pvlan.h"



int ShowPvlanPage(struct list *lpublic,struct list *lcon);

int cgiMain()
{
	struct list *lpublic;	/*解析public.txt文件的链表头*/
	struct list *lcon;	   /*解析control.txt文件的链表头*/	
	lpublic=get_chain_head("../htdocs/text/public.txt");
	lcon=get_chain_head("../htdocs/text/control.txt");

    ShowPvlanPage(lpublic,lcon);
	release(lpublic);  
	release(lcon);

 	return 0;
}

int ShowPvlanPage(struct list *lpublic,struct list *lcon)
{
	char *encry=(char *)malloc(BUF_LEN);				/*存储从wp_usrmag.cgi带入的加密字符串*/
	char *str=NULL;
	FILE *fp1;
	char lan[3];
	char pvlan_encry[BUF_LEN]; 
	char *menu_id=(char *)malloc(10);
  	char *menu=(char *)malloc(15);
	struct Pvlan_Info head,*q;
	head.lport=0;
	head.lsort=0;
	head.mode=0;
	head.pport=0;
	head.pslot=0;
	head.trunk=0;
	int xt=0;
	int pvlanNum=0;
	//char showtype[N],vlantype[N],vlanid[N],vlanname[N],mac[N],port[N];
	int i,ret;
	char * del_port=(char * )malloc(10);
	memset(del_port,0,10);
	char * deleteOP=(char * )malloc(10);
	memset(deleteOP,0,10);
	char * PPORT=(char * )malloc(10);
	memset(PPORT,0,10);
	char * CheckUsr=(char * )malloc(10);
	memset(CheckUsr,0,10);
	int retu=0;
	if(cgiFormSubmitClicked("submit_pvlan") != cgiFormSuccess)
	{
		memset(encry,0,BUF_LEN);
		cgiFormStringNoNewlines("UN", encry, BUF_LEN); 
		str=dcryption(encry);
		if(str==NULL)
		{
			ShowErrorPage(search(lpublic,"ill_user"));	 /*用户非法*/
			return 0;
		}
	}
	
	cgiFormStringNoNewlines("CheckUsr",CheckUsr,10);
	cgiHeaderContentType("text/html");
	fprintf(cgiOut,"<html xmlns=\"http://www.w3.org/1999/xhtml\"><head>");
	fprintf(cgiOut,"<meta http-equiv=Content-Type content=text/html; charset=gb2312>");
	fprintf(cgiOut,"<title>%s</title>",search(lcon,"pvlan_man"));
	fprintf(cgiOut,"<link rel=stylesheet href=/style.css type=text/css>"\
	  "<style type=text/css>"\
	  ".a3{width:30;border:0; text-align:center}"\
	  "#div1{ width:42px; height:18px; border:1px solid #666666; background-color:#f9f8f7;}"\
	  "#div2{ width:40px; height:15px; padding-left:5px; padding-top:3px}"\
	  "#link{ text-decoration:none; font-size: 12px}"\
	  "</style>"\
	"</head>"\
	"<script src=/ip.js>"\
	"</script>"\
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
	
	ccgi_dbus_init();
	memset(pvlan_encry,0,BUF_LEN); 				  /*清空临时变量*/
	cgiFormStringNoNewlines("pvlan_encry",pvlan_encry,BUF_LEN);
	cgiFormStringNoNewlines("PORT",PPORT,10);
	cgiFormStringNoNewlines("DELRULE",deleteOP,10);
	fprintf(stderr,"deleteOP=%s-PPORT=%s",deleteOP,PPORT);
	if(strcmp(deleteOP,"delete")==0)
	{
		//fprintf(stderr,"xxxxPPORT=%sxxxxxx",PPORT);
		
		pvlan_delete(PPORT);
	}
	if(cgiFormSubmitClicked("submit_pvlan") != cgiFormSuccess)
	{
		retu=checkuser_group(str);
	}
	if(cgiFormSubmitClicked("submit_editIntf") == cgiFormSuccess)
    {
         fprintf( cgiOut, "<script type='text/javascript'>\n" );
    	 fprintf( cgiOut, "window.location.href='wp_contrl.cgi?UN=%s';\n", pvlan_encry);
    	 fprintf( cgiOut, "</script>\n" );
    }
	
  fprintf(cgiOut,"<form method=post>"\
  "<div align=center>"\
  "<table width=976 border=0 cellpadding=0 cellspacing=0 style=overflow:auto>"\
  "<tr>"\
	"<td width=8 align=left valign=top background=/images/di22.jpg><img src=/images/youce4.jpg width=8 height=30/></td>"\
	"<td width=51 align=left valign=bottom background=/images/di22.jpg><img src=/images/youce33.jpg width=37 height=24/></td>"\
	"<td width=153 align=left valign=bottom background=/images/di22.jpg><font id=titleen>PVLAN</font><font id=%s> %s</font></td>",search(lpublic,"title_style"),search(lpublic,"management"));
	fprintf(cgiOut,"<td width=690 align=right valign=bottom background=/images/di22.jpg>");
		if((fp1=fopen("../htdocs/text/public.txt","r"))==NULL)		 /*以只读方式打开资源文件*/
		{
			ShowAlert(search(lpublic,"error_open"));
	    }
		else
		{
			fseek(fp1,4,0); 						/*将文件指针移到离文件首4个字节处，即lan=之后*/
			fgets(lan,3,fp1);	   
			fclose(fp1);
		}

		fprintf(cgiOut,"<table width=130 border=0 cellspacing=0 cellpadding=0>"\
		"<tr>"\
		"<td width=62 align=center><input id=but type=submit name=submit_pvlan style=background-image:url(/images/%s) value=""></td>",search(lpublic,"img_ok"));	  
		if(cgiFormSubmitClicked("submit_pvlan") != cgiFormSuccess)
			fprintf(cgiOut,"<td width=62 align=left><a href=wp_configvlan.cgi?UN=%s target=mainFrame><img src=/images/%s border=0 width=62 height=20/></a></td>",encry,search(lpublic,"img_cancel"));
		else										   
			fprintf(cgiOut,"<td width=62 align=left><a href=wp_configvlan.cgi?UN=%s target=mainFrame><img src=/images/%s border=0 width=62 height=20/></a></td>",pvlan_encry,search(lpublic,"img_cancel"));
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
						if(cgiFormSubmitClicked("submit_pvlan") != cgiFormSuccess)
						{
							fprintf(cgiOut,"<tr height=25>"\
                 			"<td align=left id=tdleft><a href=wp_configvlan.cgi?UN=%s target=mainFrame style=color:#000000><font id=yingwen_san>VLAN </font><font id=%s>%s<font></a></td>",encry,search(lpublic,"menu_san"),search(lcon,"config"));						 
                 		  	fprintf(cgiOut,"</tr>");
                 		  	if(retu==0)  /*管理员*/
                 		  	{
                       		  	fprintf(cgiOut,"<tr height=25>"\
                  				"<td align=left id=tdleft><a href=wp_addvlan.cgi?UN=%s target=mainFrame style=color:#000000><font id=%s>%s<font><font id=yingwen_san> VLAN</font></a></td>",encry,search(lpublic,"menu_san"),search(lcon,"add"));						 
                  		  		fprintf(cgiOut,"</tr>");
            		  		}
							fprintf(cgiOut,"<tr height=26>"\
							  "<td align=left id=tdleft background=/images/bottom_bg.gif style=\"border-right:0\"><font id=yingwen_san>PVLAN </font><font id=%s>%s</font></td>",search(lpublic,"menu_san"),search(lcon,"list"));   /*突出显示*/
							fprintf(cgiOut,"</tr>");
							if(retu==0)  /*管理员*/
							{
     							fprintf(cgiOut,"<tr height=25>"\
     							  "<td align=left id=tdleft><a href=wp_config_pvlan.cgi?UN=%s target=mainFrame class=top><font id=%s>%s</font><font id=yingwen_san> PVLAN</font></a></td>",encry,search(lpublic,"menu_san"),search(lcon,"pvlan_add"));
     							fprintf(cgiOut,"</tr>");
							}

						}
						else if(cgiFormSubmitClicked("submit_pvlan") == cgiFormSuccess)
						{
							fprintf(cgiOut,"<tr height=25>"\
                 			"<td align=left id=tdleft><a href=wp_configvlan.cgi?UN=%s target=mainFrame style=color:#000000><font id=yingwen_san>VLAN </font><font id=%s>%s<font></a></td>",pvlan_encry,search(lpublic,"menu_san"),search(lcon,"config"));						 
                 		  	fprintf(cgiOut,"</tr>");
                 		  	if(retu==0)  /*管理员*/
                 		  	{
                     		  	fprintf(cgiOut,"<tr height=25>"\
                				"<td align=left id=tdleft><a href=wp_addvlan.cgi?UN=%s target=mainFrame style=color:#000000><font id=%s>%s<font><font id=yingwen_san> VLAN</font></a></td>",pvlan_encry,search(lpublic,"menu_san"),search(lcon,"add"));						 
                		  		fprintf(cgiOut,"</tr>");
							}
							fprintf(cgiOut,"<tr height=26>"\
							  "<td align=left id=tdleft background=/images/bottom_bg.gif style=\"border-right:0\"><font id=yingwen_san>PVLAN </font><font id=%s>%s</font></td>",search(lpublic,"menu_san"),search(lcon,"list"));   /*突出显示*/
							fprintf(cgiOut,"</tr>");
							if(retu==0)  /*管理员*/
							{
      							fprintf(cgiOut,"<tr height=25>"\
      							  "<td align=left id=tdleft><a href=wp_config_pvlan.cgi?UN=%s target=mainFrame class=top><font id=%s>%s</font><font id=yingwen_san> PVLAN</font></a></td>",pvlan_encry,search(lpublic,"menu_san"),search(lcon,"pvlan_add"));
      							fprintf(cgiOut,"</tr>");
							}

						}
						int rowsCount=0;
						ret = show_pvlan(&head,&pvlanNum);
						if(pvlanNum<7)
							rowsCount=10;
						else rowsCount=23;
						for(i=0;i<rowsCount;i++)
						{
							fprintf(cgiOut,"<tr height=25>"\
							  "<td id=tdleft>&nbsp;</td>"\
							"</tr>");
						}
	
					  fprintf(cgiOut,"</table>"\
				  "</td>"\
				  "<td align=left valign=top style=\"background-color:#ffffff; border-right:1px solid #707070; padding-left:30px; padding-top:10px\">");
				
			fprintf(cgiOut,"<table width=750 border=0 cellspacing=0 cellpadding=0>"\
								"<tr height=35>");
						fprintf(cgiOut,"<td align=center id=sec1 style=\"border-bottom:2px solid #53868b;font-size:14px;padding-left:0px;padding-top:0px\">PVLAN%s</td>",search(lcon,"list"));
							  fprintf(cgiOut,"</tr>"\
							"<tr>"\
							  "<td align=left valign=top  style=\"padding-top:0px\">");
							   fprintf(cgiOut,"<div id=\"list\"  style=overflow:auto><table  width=750 border=1 frame=below rules=rows bordercolor=#cccccc cellspacing=0 cellpadding=0>");
								fprintf(cgiOut,"<tr height=25 bgcolor=#eaeff9 style=font-size:14px align=left>");
                            	fprintf(cgiOut,"<th  style=font-size:12px>%s</th>","PSLOT");
                            	fprintf(cgiOut,"<th  style=font-size:12px>%s</th>","PPORT");
                            	fprintf(cgiOut,"<th  style=font-size:12px>%s</th>","USLOT");
                            	fprintf(cgiOut,"<th  style=font-size:12px>%s</th>","UPORT");
                            	fprintf(cgiOut,"<th  style=font-size:12px>%s</th>","MODE");
                            	fprintf(cgiOut,"<th  style=font-size:12px>%s</th>","UTRUNK");
                            	fprintf(cgiOut,"<th  style=font-size:12px>&nbsp;</th>");
                            	fprintf(cgiOut,"</tr>");
                            	int cl = 1;
								//int show_pvlan(struct Pvlan_Info * Pinfo, int * pNum)
									q=head.next;
									for(i=0;i<pvlanNum;i++)
 									{

	  									memset(menu,0,15);
	                      				strcat(menu,"menuLists");
	                      		        sprintf(menu_id,"%d",i+1); 
	                      		        strcat(menu,menu_id);

	                      		        sprintf(del_port,"%d/%d",q->pslot,q->pport);
	                      		        //fprintf(stderr,"del_port=%s",del_port);
	  									fprintf(cgiOut,"<tr height=25 bgcolor=%s>",setclour(cl));		
                         				fprintf(cgiOut,"<td align=left>%d</td>",q->pslot);
                         				fprintf(cgiOut,"<td align=left>%d</td>",q->pport);
                         				if(q->mode==1  || q->mode==2)
                         				{
                               				fprintf(cgiOut,"<td align=left>%d</td>",q->lsort);
                               				fprintf(cgiOut,"<td align=left>%d</td>",q->lport);
                               				if(q->mode==1)
                               					fprintf(cgiOut,"<td align=left>%s</td>", "single  ");
                               				else if(q->mode==2)
                               					fprintf(cgiOut,"<td align=left>%s</td>", "multiple  ");
                               				fprintf(cgiOut,"<td align=left>%s</td>","-");
										}
										else if(q->mode==3)
										{
											fprintf(cgiOut,"<td align=left>%s</td>","-");
                            				fprintf(cgiOut,"<td align=left>%s</td>","-");
                            				fprintf(cgiOut,"<td align=left>%s</td>","-");
                            				fprintf(cgiOut,"<td align=left>%d</td>",q->trunk);
										}
                         				fprintf(cgiOut,"<td align=left>");
                        				fprintf(cgiOut,"<div style=\"position:relative; z-index:%d\" onmouseover=\"popMenu('%s');\" onmouseout=\"popMenu('%s');\">",(pvlanNum-i),menu,menu);
                        			    fprintf(cgiOut,"<img src=/images/detail.gif>");
                        			    fprintf(cgiOut,"<div id=%s style=\"display:none; position:absolute; top:5px; left:0;\">",menu);
                        			    fprintf(cgiOut,"<div id=div1>");
                        			    if(cgiFormSubmitClicked("submit_pvlan") != cgiFormSuccess)
                        					fprintf(cgiOut,"<div id=div2 onmouseover=\"this.style.backgroundColor='#b6bdd2'\" onmouseout=\"this.style.backgroundColor='#f9f8f7'\"><a id=link href=wp_show_pvlan.cgi?UN=%s&PORT=%s&DELRULE=%s target=mainFrame onclick=\"return confirm('%s')\">%s</a></div>",encry,del_port,"delete",search(lcon,"confirm_delete"),search(lcon,"delete"));
                        				else if(cgiFormSubmitClicked("submit_pvlan") == cgiFormSuccess)
                        					fprintf(cgiOut,"<div id=div2 onmouseover=\"this.style.backgroundColor='#b6bdd2'\" onmouseout=\"this.style.backgroundColor='#f9f8f7'\"><a id=link href=wp_show_pvlan.cgi?UN=%s&PORT=%s&DELRULE=%s target=mainFrame onclick=\"return confirm('%s')\">%s</a></div>",pvlan_encry,del_port,"delete",search(lcon,"confirm_delete"),search(lcon,"delete"));
                        				fprintf(cgiOut,"</div>");
                        			   	fprintf(cgiOut,"</div>");
                        			   	fprintf(cgiOut,"</div>");
                        			   	fprintf(cgiOut,"</td>");
                         				fprintf(cgiOut,"</tr>");
                         				q=q->next;
                       				}
									switch(ret)
									{
										case -1:
											fprintf(cgiOut,"<div><tr height = 30><td colspan=6><font color=red>%s</font></td></tr></div>",search(lcon,"show_err"));
											break;
										case -2:
											fprintf(cgiOut,"<div><tr height = 30><td colspan=6><font color=red>%s</font></td></tr></div>",search(lcon,"pvlan_none"));
											break;
									}
							  fprintf(cgiOut,"</table></div>");



					fprintf(cgiOut,"</td>"\
							  "</tr>"\
								"<tr>");
								  if(cgiFormSubmitClicked("submit_pvlan") != cgiFormSuccess)
								  {
									fprintf(cgiOut,"<td><input type=hidden name=pvlan_encry value=%s></td>",encry);
									fprintf(cgiOut,"<td><input type=hidden name=CheckUsr value=%d></td>",retu);
								  }
								  else if(cgiFormSubmitClicked("submit_pvlan") == cgiFormSuccess)
								  { 			 
									fprintf(cgiOut,"<td><input type=hidden name=pvlan_encry value=%s></td>",pvlan_encry);
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

	free(CheckUsr);
	free(encry);
	free(menu_id);
	free(menu);
	free(del_port);
	free(deleteOP);
	free(PPORT);
	Free_list(&head,pvlanNum);
	return 0;

}


