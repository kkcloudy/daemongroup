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
* wp_policymaplist.c
*
*
* CREATOR:
* autelan.software.Network Dep. team
* zhouym@autelan.com
*
* DESCRIPTION:
* system contrl for qos config
*
*
*******************************************************************************/
#include <stdio.h>
#include "cgic.h"
#include <string.h>
#include <stdlib.h>
#include "ws_usrinfo.h"
#include <sys/wait.h>
#include "ws_init_dbus.h"
#include "ws_sysinfo.h"
#include "ws_dcli_qos.h"

#define AMOUNT 512

int ShowPolicyMapPage();

int cgiMain()
{
 ShowPolicyMapPage();
 return 0;
}

int ShowPolicyMapPage()
{
	ccgi_dbus_init();       /*调用底层初始化函数*/
	struct list *lpublic;	/*解析public.txt文件的链表头*/
	struct list *lcontrol; 	/*解析help.txt文件的链表头*/
	lpublic=get_chain_head("../htdocs/text/public.txt");
    lcontrol=get_chain_head("../htdocs/text/control.txt"); 	
  char *encry=(char *)malloc(BUF_LEN);
  char *str=NULL;
 // FILE *fp;
  //char lan[3];
  char policymap_encry[BUF_LEN]; 
  char addn[N];         
  int i=0;   
  int retu=0,retz;
  int cl=1;
  int policy_map_num=0;
  char menu[21]="menulist";
  char* i_char=(char *)malloc(10);
  char * deletepolicy=(char * )malloc(10);
  memset(deletepolicy,0,10);
  char * index=(char * )malloc(10);
  memset(index,0,10);

  char * mode=(char *)malloc(AMOUNT);
  memset(mode,0,AMOUNT);
  char * CheckUsr=(char * )malloc(10);
  memset(CheckUsr,0,10);

  
  struct policy_map_info receive_policy_map[Policy_Map_Num];
  
  for(i=0;i<Policy_Map_Num;i++)
	{
    	 receive_policy_map[i].policy_map_index=0;
    	 receive_policy_map[i].droppre=(char *)malloc(20);
    	 memset(receive_policy_map[i].droppre,0,20);
    	 
    	 receive_policy_map[i].trustMem=(char *)malloc(30);
    	 memset(receive_policy_map[i].trustMem,0,30);
    	 
    	 receive_policy_map[i].modiUp=(char *)malloc(20);
    	 memset(receive_policy_map[i].modiUp,0,20);
    	 
    	 receive_policy_map[i].modiDscp=(char *)malloc(20);
    	 memset(receive_policy_map[i].modiDscp,0,20);
    	 
    	 receive_policy_map[i].remaps=(char *)malloc(20);
    	 memset(receive_policy_map[i].remaps,0,20);
    }
  
  if(cgiFormSubmitClicked("submit_policymaplist") != cgiFormSuccess)
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
	memset(policymap_encry,0,BUF_LEN);                   /*清空临时变量*/
  }
  
  cgiFormStringNoNewlines("encry_routelist",policymap_encry,BUF_LEN);
  cgiFormStringNoNewlines("DELRULE",deletepolicy,10);
  cgiFormStringNoNewlines("INDEX",index,10);
  cgiFormStringNoNewlines("CheckUsr",CheckUsr,10);

  if(strcmp(CheckUsr,"")!=0)
  	retu=atoi(CheckUsr);
	show_qos_mode(mode);
  cgiHeaderContentType("text/html");
  fprintf(cgiOut,"<html xmlns=\"http://www.w3.org/1999/xhtml\"><head>");
  fprintf(cgiOut,"<meta http-equiv=Content-Type content=text/html; charset=gb2312>");
  fprintf(cgiOut,"<title>%s</title>",search(lcontrol,"route_manage"));
  fprintf(cgiOut,"<link rel=stylesheet href=/style.css type=text/css>"\
  	"<style type=text/css>"\
  	  "#div1{ width:42px; height:18px; border:1px solid #666666; background-color:#f9f8f7;}"\
	  "#div2{ width:40px; height:15px; padding-left:5px; padding-top:3px}"\
	  "#link{ text-decoration:none; font-size: 12px}"\
  	".ShowPolicy {overflow-x:hidden;  overflow:auto; width: 680px; height: 386px; clip: rect( ); padding-top: 2px; padding-right: 0px; padding-bottom: 0px; padding-left: 0px} "\
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
  if(strcmp(deletepolicy,"delete")==0)
  {
  	retz=del_policy_map(index,lcontrol);
	switch(retz)
	{
		case 0:
			ShowAlert(search(lpublic,"oper_succ"));
			break;
		case -2:
			ShowAlert(search(lcontrol,"policy_map_not_exist"));
			break;
		case -3:
			ShowAlert(search(lcontrol,"policy_map_bind_port"));
			break;
		case -4:
			ShowAlert(search(lcontrol,"del_policy_map_fail"));
			break;
		default:
			ShowAlert(search(lpublic,"oper_fail"));
			break;
		}
  }
  if(cgiFormSubmitClicked("submit_policymaplist") != cgiFormSuccess)
  {
  	 retu=checkuser_group(str);
  }
  if(cgiFormSubmitClicked("submit_policymaplist") == cgiFormSuccess)
  {
  	 fprintf( cgiOut, "<script type='text/javascript'>\n" );
 	 fprintf( cgiOut, "window.location.href='wp_qosModule.cgi?UN=%s';\n", policymap_encry);
 	 fprintf( cgiOut, "</script>\n" );
  }


  fprintf(cgiOut,"<form method=post>"\
  "<div align=center>"\
  "<table width=976 border=0 cellpadding=0 cellspacing=0>"\
  "<tr>"\
    "<td width=8 align=left valign=top background=/images/di22.jpg><img src=/images/youce4.jpg width=8 height=30/></td>"\
    "<td width=51 align=left valign=bottom background=/images/di22.jpg><img src=/images/youce33.jpg width=37 height=24/></td>"\
    "<td width=153 align=left valign=bottom background=/images/di22.jpg><font id=titleen>QOS</font><font id=%s> %s</font></td>",search(lpublic,"title_style"),search(lpublic,"management"));
    fprintf(cgiOut,"<td width=690 align=right valign=bottom background=/images/di22.jpg>");
    	  fprintf(cgiOut,"<table width=130 border=0 cellspacing=0 cellpadding=0>"\
          "<tr>"\
          "<td width=62 align=center><input id=but type=submit name=submit_policymaplist style=background-image:url(/images/%s) value=""></td>",search(lpublic,"img_ok"));		  
          
		  if(cgiFormSubmitClicked("submit_policymaplist") != cgiFormSuccess)
		  {
            fprintf(cgiOut,"<td width=62 align=left><a href=wp_qosmap.cgi?UN=%s target=mainFrame><img src=/images/%s border=0 width=62 height=20/></a></td>",encry,search(lpublic,"img_cancel"));
           
            }
		  else                                         
     		fprintf(cgiOut,"<td width=62 align=left><a href=wp_qosmap.cgi?UN=%s target=mainFrame><img src=/images/%s border=0 width=62 height=20/></a></td>",policymap_encry,search(lpublic,"img_cancel"));
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
         		if(cgiFormSubmitClicked("submit_policymaplist") != cgiFormSuccess)
         		{
         			if(retu==0)
         			{
             			fprintf(cgiOut,"<tr height=25>"\
            			"<td align=left id=tdleft><a href=wp_qosmap.cgi?UN=%s target=mainFrame class=top><font id=yingwen_san>QOS </font><font id=%s>%s</font></a></td>",encry,search(lpublic,"menu_san"),search(lcontrol,"list"));					   
            		  	fprintf(cgiOut,"</tr>"\
            		  	"<tr height=25>"\
            			"<td align=left id=tdleft><a href=wp_addqos.cgi?UN=%s target=mainFrame class=top><font id=%s>%s</font><font id=yingwen_san>QOS Profile</font></a></td>",encry,search(lpublic,"menu_san"),search(lcontrol,"add"));
            		  	fprintf(cgiOut,"</tr>");
             			fprintf(cgiOut,"<tr height=25>"\
             			"<td align=left id=tdleft><a href=wp_addmap.cgi?UN=%s target=mainFrame class=top><font id=%s>%s</font></a></td>",encry,search(lpublic,"menu_san"),search(lcontrol,"add_map"));					   
             			fprintf(cgiOut,"</tr>");

				fprintf(cgiOut,"<tr height=25>"\
             			"<td align=left id=tdleft><a href=wp_qosmapinfo.cgi?UN=%s target=mainFrame class=top><font id=%s>QOS %s</font></a></td>",encry,search(lpublic,"menu_san"),search(lcontrol,"map_detail"));					   
             			fprintf(cgiOut,"</tr>");
						
             			fprintf(cgiOut,"<tr height=26>"\
             			"<td align=left id=tdleft background=/images/bottom_bg.gif style=\"border-right:0\"><font id=%s>%s</font></td>",search(lpublic,"menu_san"),search(lcontrol,"policy_map"));   /*突出显示*/
             			fprintf(cgiOut,"</tr>");
             			fprintf(cgiOut,"<tr height=25>"\
             			"<td align=left id=tdleft><a href=wp_createpolicy.cgi?UN=%s target=mainFrame class=top><font id=%s>%s</font></a></td>",encry,search(lpublic,"menu_san"),search(lcontrol,"add_policy"));					   
             			fprintf(cgiOut,"</tr>");
         			}
         			else
         			{
	         			fprintf(cgiOut,"<tr height=25>"\
	            			"<td align=left id=tdleft><a href=wp_qosmap.cgi?UN=%s target=mainFrame class=top><font id=yingwen_san>QOS </font><font id=%s>%s</font></a></td>",encry,search(lpublic,"menu_san"),search(lcontrol,"list"));					   
	            		  	fprintf(cgiOut,"</tr>");

					fprintf(cgiOut,"<tr height=25>"\
	            			"<td align=left id=tdleft><a href=wp_qosmapinfo.cgi?UN=%s target=mainFrame class=top><font id=yingwen_san>QOS </font><font id=%s>%s</font></a></td>",encry,search(lpublic,"menu_san"),search(lcontrol,"map_detail"));					   
	            		  	fprintf(cgiOut,"</tr>");
					
	            		  	fprintf(cgiOut,"<tr height=26>"\
	             			"<td align=left id=tdleft background=/images/bottom_bg.gif style=\"border-right:0\"><font id=%s>%s</font></td>",search(lpublic,"menu_san"),search(lcontrol,"policy_map"));   /*突出显示*/
	             			fprintf(cgiOut,"</tr>");
         			}
         		}
         		else if(cgiFormSubmitClicked("submit_policymaplist") == cgiFormSuccess) 			  
         		{
         			if(retu==0)
	         		{
	             			fprintf(cgiOut,"<tr height=25>"\
	            			"<td align=left id=tdleft><a href=wp_qosmap.cgi?UN=%s target=mainFrame style=color:#000000><font id=yingwen_san>QOS </font><font id=%s>%s</font></a></td>",policymap_encry,search(lpublic,"menu_san"),search(lcontrol,"list"));						 
	            		  	fprintf(cgiOut,"</tr>"\
	            		  	"<tr height=25>"\
	            			"<td align=left id=tdleft><a href=wp_addqos.cgi?UN=%s target=mainFrame style=color:#000000><font id=%s>%s</font><font id=yingwen_san>QOS Profile</font></a></td>",policymap_encry,search(lpublic,"menu_san"),search(lcontrol,"add"));
	            		   	fprintf(cgiOut,"</tr>");
	             			fprintf(cgiOut,"<tr height=25>"\
	             			"<td align=left id=tdleft><a href=wp_addmap.cgi?UN=%s target=mainFrame class=top><font id=%s>%s</font></a></td>",policymap_encry,search(lpublic,"menu_san"),search(lcontrol,"add_map"));					   
	             			fprintf(cgiOut,"</tr>");

					fprintf(cgiOut,"<tr height=25>"\
	             			"<td align=left id=tdleft><a href=wp_qosmapinfo.cgi?UN=%s target=mainFrame class=top><font id=%s>QOS %s</font></a></td>",policymap_encry,search(lpublic,"menu_san"),search(lcontrol,"map_detail"));					   
	             			fprintf(cgiOut,"</tr>");
					
	             			fprintf(cgiOut,"<tr height=26>"\
	             			"<td align=left id=tdleft background=/images/bottom_bg.gif style=\"border-right:0\"><font id=%s>%s</font></td>",search(lpublic,"menu_san"),search(lcontrol,"policy_map"));   /*突出显示*/
	             			fprintf(cgiOut,"</tr>");
	             			fprintf(cgiOut,"<tr height=25>"\
	             			"<td align=left id=tdleft><a href=wp_createpolicy.cgi?UN=%s target=mainFrame class=top><font id=%s>%s</font></a></td>",policymap_encry,search(lpublic,"menu_san"),search(lcontrol,"add_policy"));					   
	             			fprintf(cgiOut,"</tr>");
	         		}
         			else
         			{
	         			fprintf(cgiOut,"<tr height=25>"\
	            			"<td align=left id=tdleft><a href=wp_qosmap.cgi?UN=%s target=mainFrame style=color:#000000><font id=yingwen_san>QOS </font><font id=%s>%s</font></a></td>",policymap_encry,search(lpublic,"menu_san"),search(lcontrol,"list"));						 
	            		  	fprintf(cgiOut,"</tr>");

					fprintf(cgiOut,"<tr height=25>"\
	            			"<td align=left id=tdleft><a href=wp_qosmapinfo.cgi?UN=%s target=mainFrame style=color:#000000><font id=yingwen_san>QOS </font><font id=%s>%s</font></a></td>",policymap_encry,search(lpublic,"menu_san"),search(lcontrol,"map_detail"));						 
	            		  	fprintf(cgiOut,"</tr>");
							
	            		  	fprintf(cgiOut,"<tr height=26>"\
	             			"<td align=left id=tdleft background=/images/bottom_bg.gif style=\"border-right:0\"><font id=%s>%s</font></td>",search(lpublic,"menu_san"),search(lcontrol,"policy_map"));   /*突出显示*/
	             			fprintf(cgiOut,"</tr>");
         			}
         		}
         		int rowsCount=0;
         		if(retu==0)
         			rowsCount=13;
         		else
         			rowsCount=17;
				for(i=0;i<rowsCount;i++)
					{
						fprintf(cgiOut,"<tr height=25>"\
						  "<td id=tdleft>&nbsp;</td>"\
						"</tr>");
					}
				  fprintf(cgiOut,"</table>"\
              "</td>"\
              "<td align=left valign=top style=\"background-color:#ffffff; border-right:1px solid #707070; padding-left:30px; padding-top:10px\">"\
						"<table width=640 height=310 border=0 cellspacing=0 cellpadding=0>"\
						"<tr>\n"\
							"<td id=sec1 style=\"border-bottom:2px solid #53868b;font-size:14px\">\n"\
								"<table width=%s>\n","100%");
									fprintf(cgiOut,"<tr>\n"\
										"<td id=sec1 style=\"font-size:14px\">%s</td>",search(lcontrol,"policy_map_info"));
				  			fprintf(cgiOut,"<td id=sec1 style=\"font-size:14px\" align='right'><font color='red'><b>%s</b></font></td>",search(lcontrol,mode)),
				  		fprintf(cgiOut,"</tr>\n"\
								"</table>\n"\
							"</td>");
						fprintf(cgiOut,"</tr>"\
						 "<tr>"\
						   "<td align=left valign=top style=padding-top:18px>"\
						   "<div class=ShowPolicy><table width=583 border=1 frame=below rules=rows bordercolor=#cccccc cellspacing=0 cellpadding=0>");
						   fprintf(cgiOut,"<tr height=30 bgcolor=#eaeff9 style=font-size:14px  id=td1 align=left>"\
							 "<th width=60><font id=%s>%s</font></th>", search(lpublic,"menu_thead"),"Index");
							 fprintf(cgiOut,"<th width=130><font id=%s>%s</font></th>", search(lpublic,"menu_thead"),"QoS sub-markers");
							 if(get_product_id()!=PRODUCT_ID_AU3K_BCM)
							 {
							 	fprintf(cgiOut,"<th width=100><font id=%s>%s</font></th>", search(lpublic,"menu_thead"),"TrustMode");
							 }
							 fprintf(cgiOut,"<th width=160 align=left><font id=%s>%s</font></th>", search(lpublic,"menu_thead"),"Modify UP");
							 fprintf(cgiOut,"<th width=160 align=left><font id=%s>%s</font></th>", search(lpublic,"menu_thead"),"Modify DSCP");
							 fprintf(cgiOut,"<th width=160 align=left><font id=%s>%s</font></th>", search(lpublic,"menu_thead"),"Remap DSCP");
							 
							 
							 //////////////////////         add by kehao 02/21/2011  15:24                   ////////////////////////////////////////
							 fprintf(cgiOut,"<th width=0 align=left><font id=%s>%s</font></th>", search(lpublic,"menu_thead"),"Binded PORT");
							 
							 /////////////////////////////////////////////////////////////////////////////////////////////////////////
							 
							 fprintf(cgiOut,"<th width=13>&nbsp;</th>");
							 fprintf(cgiOut,"</tr>");
							 show_policy_map(receive_policy_map,&policy_map_num,lcontrol); //显示策略内容
							//fprintf(stderr,"policy_map_num=%d",policy_map_num);
							for(i=0;i<policy_map_num;i++)
							{
								memset(menu,0,21);
							  	strcpy(menu,"menulist");
							  	sprintf(i_char,"%d",i+1);
							  	strcat(menu,i_char);

								 fprintf(cgiOut,"<tr height=25 bgcolor=%s  align=left>",setclour(cl));
								 fprintf(cgiOut,"<td style=font-size:12px  align=left>%u</td>",receive_policy_map[i].policy_map_index);
								 fprintf(cgiOut,"<td style=font-size:12px  align=left>%s</td>",receive_policy_map[i].droppre);
								 if(get_product_id()!=PRODUCT_ID_AU3K_BCM)
								 {
								 	fprintf(cgiOut,"<td style=font-size:12px align=left>%s</td>",receive_policy_map[i].trustMem);
								 }
								 fprintf(cgiOut,"<td style=font-size:12px align=left>%s</td>",receive_policy_map[i].modiUp);
								 fprintf(cgiOut,"<td style=font-size:12px align=left>%s</td>",receive_policy_map[i].modiDscp);
								 fprintf(cgiOut,"<td style=font-size:12px align=left>%s</td>",receive_policy_map[i].remaps);

								 ///////////////////////add by kehao 02/21/2011 17:12///////////////////


                                   fprintf(cgiOut,"<td style=font-size:12px align=left>%d/%d</td>",receive_policy_map[i].slot_no,receive_policy_map[i].port_no);

                                   //fprintf(cgiOut,"<td style=font-size:12px align=left>%d</td>",receive_policy_map[i].port_no);
								  

								 //////////////////////////////////////////////////////////////////////////////////////
								 if(retu==0)
								 {
     								 fprintf(cgiOut,"<td align=left>");
     								 fprintf(cgiOut,"<div style=\"position:relative; z-index:%d\" onmouseover=\"popMenu('%s');\" onmouseout=\"popMenu('%s');\">",(policy_map_num-i),menu,menu);
     																   fprintf(cgiOut,"<img src=/images/detail.gif>"\
     																   "<div id=%s style=\"display:none; position:absolute; top:5px; left:0;\">",menu);
     																   fprintf(cgiOut,"<div id=div1>");
     																   if(cgiFormSubmitClicked("submit_policymaplist") != cgiFormSuccess)
     																   {
     																   		fprintf(cgiOut,"<div id=div2 onmouseover=\"this.style.backgroundColor='#b6bdd2'\" onmouseout=\"this.style.backgroundColor='#f9f8f7'\"><a id=link href=wp_configpolicy.cgi?UN=%s&INDEX=%u target=mainFrame>%s</a></div>",encry,receive_policy_map[i].policy_map_index,search(lpublic,"configure"));
         																   	fprintf(cgiOut,"<div id=div2 onmouseover=\"this.style.backgroundColor='#b6bdd2'\" onmouseout=\"this.style.backgroundColor='#f9f8f7'\"><a id=link href=wp_policymaplist.cgi?UN=%s&INDEX=%u&DELRULE=%s target=mainFrame onclick=\"return confirm('%s')\">%s</a></div>",encry,receive_policy_map[i].policy_map_index,"delete",search(lcontrol,"confirm_delete"),search(lcontrol,"delete"));
     																   	}
     																   else
     																   {
     																		fprintf(cgiOut,"<div id=div2 onmouseover=\"this.style.backgroundColor='#b6bdd2'\" onmouseout=\"this.style.backgroundColor='#f9f8f7'\"><a id=link href=wp_configpolicy.cgi?UN=%s&INDEX=%u target=mainFrame>%s</a></div>",policymap_encry,receive_policy_map[i].policy_map_index,search(lpublic,"configure"));
         																   	fprintf(cgiOut,"<div id=div2 onmouseover=\"this.style.backgroundColor='#b6bdd2'\" onmouseout=\"this.style.backgroundColor='#f9f8f7'\"><a id=link href=wp_policymaplist.cgi?UN=%s&INDEX=%u&DELRULE=%s target=mainFrame onclick=\"return confirm('%s')\">%s</a></div>",policymap_encry,receive_policy_map[i].policy_map_index,"delete",search(lcontrol,"confirm_delete"),search(lcontrol,"delete"));
     																	}
     																   fprintf(cgiOut,"</div>"\
     																   "</div>"\
     																   "</div>");
     																   fprintf(cgiOut,"</td>");
								}
								else fprintf(cgiOut,"<td>&nbsp;</td>");
								 
							 fprintf(cgiOut,"</tr>");
								 cl=!cl;
							}
							 
						  	fprintf(cgiOut,"</table></div>"\
							 "</td>"\
						   "</tr>"\
						 
						 /*"<tr height=25 style=padding-top:2px>"\
			   			"<td style=font-size:14px;color:#FF0000> K - kernel route, C - connected, S - static, R - RIP, O - OSPF,I - ISIS, B - BGP, > - selected route, * - FIB route</td>"\
			   			"</tr>"\*/
			   			
						/*"<tr>"\
						"<td>"\
						"<table width=430 style=padding-top:2px>"\
						"<tr>");
						sprintf(pageNumCA,"%d",pageNum+1);
						sprintf(pageNumCD,"%d",pageNum-1);
						if(cgiFormSubmitClicked("submit_policymaplist") != cgiFormSuccess)
							{
								fprintf(cgiOut,"<td align=center style=padding-top:2px><a href=wp_srouter.cgi?UN=%s&PN=%s&SN=%s>%s</td>",encry,pageNumCA,"PageDown",search(lcontrol,"page_down"));
								fprintf(cgiOut,"<td align=center style=padding-top:2px><a href=wp_srouter.cgi?UN=%s&PN=%s&SN=%s>%s</td>",encry,pageNumCD,"PageUp",search(lcontrol,"page_up"));
							}
						else if(cgiFormSubmitClicked("submit_policymaplist") == cgiFormSuccess)
							{
								fprintf(cgiOut,"<td align=center style=padding-top:2px><a href=wp_srouter.cgi?UN=%s&PN=%s&SN=%s>%s</td>",policymap_encry,pageNumCA,"PageDown",search(lcontrol,"page_down"));
								fprintf(cgiOut,"<td align=center style=padding-top:2px><a href=wp_srouter.cgi?UN=%s&PN=%s&SN=%s>%s</td>",policymap_encry,pageNumCD,"PageUp",search(lcontrol,"page_up"));  
							}
						fprintf(cgiOut,"</tr></table></td>"\
						"</tr>"\*/
						 "<tr>");
						 if(cgiFormSubmitClicked("submit_policymaplist") != cgiFormSuccess)
						 {
						   fprintf(cgiOut,"<td><input type=hidden name=encry_routelist value=%s></td>",encry);
						   fprintf(cgiOut,"<td><input type=hidden name=CheckUsr value=%d></td>",retu);
						 }
						 else if(cgiFormSubmitClicked("submit_policymaplist") == cgiFormSuccess)
							 {
							   fprintf(cgiOut,"<td><input type=hidden name=encry_routelist value=%s></td>",policymap_encry);
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
free(i_char);
for(i=0;i<Policy_Map_Num;i++)
  {

	  	free(receive_policy_map[i].droppre);
	  	free(receive_policy_map[i].trustMem);
	  	free(receive_policy_map[i].modiUp);
	  	free(receive_policy_map[i].modiDscp);
	  	free(receive_policy_map[i].remaps);
 }
free(deletepolicy);
free(CheckUsr);
free(index);
release(lpublic);  
release(lcontrol);
free(mode);
return 0;
}

