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
* wp_prtarp.c
*
*
* CREATOR:
* autelan.software.Network Dep. team
* zhouym@autelan.com
*
* DESCRIPTION:
* system contrl for port arp
*
*
*******************************************************************************/
#include <stdio.h>
#include "cgic.h"
#include <string.h>
#include <stdlib.h>
#include "ws_dcli_portconf.h"
#include "ws_err.h"
#include "ws_usrinfo.h"
#include "ws_ec.h"
#include "ws_static_arp.h"
#include "ws_returncode.h"

void ShowPrtArpPage(char *m,char *n,char *s,char *t,int is_arp,int is_nexthop,struct list *lpublic,struct list *lcon);     /*m代表加密后的字符串*/
static int delete_static_arp();
static void show_static_arp_delete_err( int errorno,   struct list *lcon );


int cgiMain()
{
  char *encry=(char *)malloc(BUF_LEN);			 /*存储从wp_topFrame.cgi带入的加密字符串*/    
  char *arp_port=(char *)malloc(10);
  char *nexthop_port=(char *)malloc(10);
  char *default_port=(char *)malloc(20);
  char *isArp=(char *)malloc(5);
  char *isNexthop=(char *)malloc(5);
  char *str;	         /*存储解密后的当前登陆用户名*/  
  char *endptr = NULL;
  int is_arp,is_nexthop,num,ret=-1;
  ETH_SLOT_LIST  head,*p;
  ETH_PORT_LIST *pp;
  struct list *lpublic;   /*解析public.txt文件的链表头*/
  struct list *lcon;      /*解析control.txt文件的链表头*/  

  
  lpublic=get_chain_head("../htdocs/text/public.txt");
  lcon=get_chain_head("../htdocs/text/control.txt");
  memset(encry,0,BUF_LEN);  
  memset(arp_port,0,10);
  memset(nexthop_port,0,10);
  memset(isArp,0,5);
  memset(isNexthop,0,5);
  if(cgiFormStringNoNewlines("UN", encry, BUF_LEN)!=cgiFormNotFound )  /*首次进入该页*/
  {
	str=dcryption(encry);
    if(str==NULL)
	  ShowErrorPage(search(lpublic,"ill_user")); 	  /*用户非法*/
    else
    {
      if(cgiFormStringNoNewlines("ID", arp_port, 10)!=cgiFormNotFound )
	    ShowPrtArpPage(encry,str,arp_port,arp_port,0,0,lpublic,lcon);
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
					memset(default_port,0,20);
					sprintf(default_port,"%d-%d",p->slot_no,pp->port_no);
					pp=pp->next;
					break;
				}
				p=p->next;
			}
		}	
        ShowPrtArpPage(encry,str,default_port,default_port,0,0,lpublic,lcon);
		 if((ret==0)&&(num>0))
          {
        	Free_ethslot_head(&head);
          }   
	  }
    }
  }
  else  /*非首次进入到该页*/
  {
    cgiFormStringNoNewlines("encry_PrtArp",encry,BUF_LEN);
	cgiFormStringNoNewlines("arp_num",arp_port,10);   
	cgiFormStringNoNewlines("nexthop_num",nexthop_port,10); 
	cgiFormStringNoNewlines("is_arp",isArp,BUF_LEN);
	cgiFormStringNoNewlines("is_nexthop",isNexthop,BUF_LEN);
	is_arp=strtoul(isArp,&endptr,10);  /*char转成int*/ 
	is_nexthop=strtoul(isNexthop,&endptr,10);  /*char转成int*/ 
	str=dcryption(encry);
    if(str==NULL)
	  ShowErrorPage(search(lpublic,"ill_user")); 	  /*用户非法*/
    else
      ShowPrtArpPage(encry,str,arp_port,nexthop_port,is_arp,is_nexthop,lpublic,lcon);
  }
  free(encry);  
  free(arp_port);
  free(nexthop_port);
  free(default_port);
  free(isArp);
  free(isNexthop);
  release(lpublic);  
  release(lcon);
 
  return 0;
}

void ShowPrtArpPage(char *m,char *n,char *s,char *t,int is_arp,int is_nexthop,struct list *lpublic,struct list *lcon)
{ 
  char *slot_port=(char *)malloc(10);
  int i,cl=1,limit=2,arpNum=0,nexthopNum=0,retu,num,ret;
  int result1 = 0;
  int result2 = 0;
  struct slot sr;
  ETH_SLOT_LIST  head,*p;
  ETH_PORT_LIST *pp;
  sr.module_status=0;     
  sr.modname=(char *)malloc(20);     //为结构体成员申请空间，假设该字段的最大长度为20
  sr.sn=(char *)malloc(20);          //为结构体成员申请空间，假设该字段的最大长度为20
  sr.hw_ver=0;
  sr.ext_slot_num=0;  
  struct arp_nexthop_profile arp_head,*aq,nexthop_head,*nq=NULL;
  int del_static_arp_err = 0;
  
  int temp=1;
 
  cgiHeaderContentType("text/html");
  fprintf(cgiOut,"<html xmlns=\"http://www.w3.org/1999/xhtml\"><head>");
  fprintf(cgiOut,"<meta http-equiv=Content-Type content=text/html; charset=gb2312>");
  fprintf(cgiOut,"<title>%s</title>",search(lcon,"prt_manage"));
  fprintf(cgiOut,"<link rel=stylesheet href=/style.css type=text/css>"\
  "</head>" );
  fprintf( cgiOut, "<script src=\"/fw.js\"></script> \n" );

  ccgi_dbus_init();
  
  del_static_arp_err = delete_static_arp();  
  show_static_arp_delete_err(del_static_arp_err, lcon);
  fprintf( cgiOut, "<body>");

  if(cgiFormSubmitClicked("show_arp_apply") == cgiFormSuccess)
  {
     is_arp=temp;
  }
  if(cgiFormSubmitClicked("clear_arp_apply") == cgiFormSuccess)
  {
     is_arp=0;
	 ccgi_clear_ethport_arp(s);
  }
  if(cgiFormSubmitClicked("show_nexthop_apply") == cgiFormSuccess)
  {
     is_nexthop=1;
  }
  /*
  if(cgiFormSubmitClicked("clear_nexthop_apply") == cgiFormSuccess)
  {
     is_nexthop=0;
	 ccgi_clear_ethport_arp(t);
  }
  */
  fprintf(cgiOut,"<form method=post encType=multipart/form-data>"\
  "<div align=center>"\
  "<table width=976 border=0 cellpadding=0 cellspacing=0>"\
  "<tr>"\
      "<td width=8 align=left valign=top background=/images/di22.jpg><img src='/images/youce4.jpg' width=8 height=30/></td>"\
    "<td width=51 align=left valign=bottom background=/images/di22.jpg><img alt='' src=/images/youce33.jpg width=37 height=24/></td>"\
    "<td width=153 align=left valign=bottom id=%s background=/images/di22.jpg>%s</td>",search(lpublic,"title_style"),search(lcon,"prt_manage"));
    fprintf(cgiOut,"<td width=692 align=right valign=bottom background=/images/di22.jpg>");
	  fprintf(cgiOut,"<table width=130 border=0 cellspacing=0 cellpadding=0>"\
	  "<tr>"\
	  "<td width=62 align=center><a href=wp_contrl.cgi?UN=%s target=mainFrame><img src=/images/%s border=0 width=62 height=20/></a></td>",m,search(lpublic,"img_ok"));
	  fprintf(cgiOut,"<td width=62 align=left><a href=wp_contrl.cgi?UN=%s target=mainFrame><img src=/images/%s border=0 width=62 height=20/></a></td>",m,search(lpublic,"img_cancel"));
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
					fprintf(cgiOut,"<tr height=25>"\
					  "<td align=left id=tdleft><a href=wp_prtsur.cgi?UN=%s target=mainFrame class=top><font id=%s>%s<font></a></td>",m,search(lpublic,"menu_san"),search(lcon,"prt_sur"));						
					fprintf(cgiOut,"</tr>"		
					"<tr height=26>"\
                      "<td align=left id=tdleft background=/images/bottom_bg.gif style=\"border-right:0\"><font id=yingwen_san>ARP<font><font id=%s> %s<font></td>",search(lpublic,"menu_san"),search(lpublic,"survey"));     /*突出显示*/
                    fprintf(cgiOut,"</tr>");
					retu=checkuser_group(n);
					if(retu==0)  /*管理员*/
					{
						fprintf(cgiOut,"<tr height=25>"\
						  "<td align=left id=tdleft><a href=wp_static_arp.cgi?UN=%s target=mainFrame class=top><font id=%s>%s<font></a></td></tr>",m,search(lpublic,"menu_san"),search(lcon,"prt_static_arp"));						 
					  fprintf(cgiOut,"<tr height=25>"\
  					    "<td align=left id=tdleft><a href=wp_prtcfg.cgi?UN=%s target=mainFrame class=top><font id=%s>%s<font></a></td>",m,search(lpublic,"menu_san"),search(lcon,"prt_cfg"));                       
                      fprintf(cgiOut,"</tr>"\
                      "<tr height=25>"\
  					      "<td align=left id=tdleft><a href=wp_prtfuncfg.cgi?UN=%s target=mainFrame class=top><font id=%s>%s<font></a></td>",m,search(lpublic,"menu_san"),search(lcon,"func_cfg"));                       
                      fprintf(cgiOut,"</tr>");				  
					}
					//add by sjw  2008-10-9 17:14:37  for  subinterface
					fprintf(cgiOut,"<tr height=25>"\
  					    "<td align=left id=tdleft><a href=wp_subintf.cgi?UN=%s target=mainFrame class=top><font id=%s>%s<font></a></td></tr>",m,search(lpublic,"menu_san"),search(lcon,"title_subintf"));
					if(retu==0)
					{
					fprintf(cgiOut,"<tr height=25>"\
						  "<td align=left id=tdleft><a href=wp_interface_bindip.cgi?UN=%s target=mainFrame class=top><font id=%s>%s<font></a></td></tr>",m,search(lpublic,"menu_san"),search(lcon,"bind_ip"));	
					}
					fprintf(cgiOut,"<tr height=25>"\
  					    "<td align=left id=tdleft><a href=wp_all_interface.cgi?UN=%s target=mainFrame class=top><font id=%s>%s<font></a></td></tr>",m,search(lpublic,"menu_san"),search(lcon,"interface")); 					
				
  					if( 1 == is_arp )
  					{  			
				  		result1=ccgi_show_ethport_arp(s,&arp_head,&arpNum);				  
				  		//aq=arp_head.next;                       
                       
				  	}

				  	if( 1 == is_nexthop )
				  	{				  	
				  		result2=ccgi_show_ethport_nexthop(t,&nexthop_head,&nexthopNum);
				  		//nq = nexthop_head.next;              
				  	}
				  limit+=is_arp;
				  if(is_arp==1)
				    limit+=arpNum;
				  limit+=is_nexthop;
				  if(is_nexthop==1)
				    limit+=nexthopNum;
				  if(retu==1)  /*普通用户*/
				  	limit+=2;
				  for(i=0;i<limit;i++)
	              {
  				    fprintf(cgiOut,"<tr height=20>"\
                      "<td id=tdleft>&nbsp;</td>"\
                    "</tr>");
	              }
                fprintf(cgiOut,"</table>"\
              "</td>"\
              "<td align=left style=\"background-color:#ffffff; border-right:1px solid #707070; padding-left:30px\">"\
              "<table width=700 border=0 cellspacing=0 cellpadding=0>"\
              "<tr style=\" padding-top:20px; padding-bottom:20px\">"\
                "<td width=50>%s:</td>",search(lcon,"port_no"));
                fprintf(cgiOut,"<td width=70><select name=arp_num style=width:50px>");
				ret=show_ethport_list(&head,&num);
				p=head.next;
				if(p!=NULL)
				{
					while(p!=NULL)
					{
						pp=p->port.next;
						while(pp!=NULL)
						{
							memset(slot_port,0,10);							
							sprintf(slot_port,"%d-%d",p->slot_no,pp->port_no);		 /*int转成char*/
							if(strcmp(slot_port,s)==0)
			 				  fprintf(cgiOut,"<option value=%s selected=selected>%s",slot_port,slot_port);
							else
							  fprintf(cgiOut,"<option value=%s>%s",slot_port,slot_port);
							pp=pp->next;
						}
						p=p->next;
					}
				}	
				if((ret==0)&&(num>0))
					Free_ethslot_head(&head);
                  fprintf(cgiOut,"</select></td>"\
                "<td width=110><input type=submit style=\"width:80px\" border=0 name=show_arp_apply style=background-image:url(/images/SubBackGif.gif) value=\"%s\"></td>"\
                "<td width=470><input type=submit style=\"width:80px\" border=0 name=clear_arp_apply style=background-image:url(/images/SubBackGif.gif) value=\"%s\"></td>"\
              "</tr>"\
              "<tr>"\
                "<td colspan=4>", search(lcon,"prt_show_arp"), search(lcon,"prt_clear_arp") );	

			  
				if(is_arp == 1)
				{


					if(result1==1)
					{		


                  		fprintf(cgiOut,"<table frame=below rules=rows width=700 border=1>"\
		            			"<tr align=left>"\
                      			"<th width=110><font id=yingwen_thead> IP</font></th>"\
					  			"<th width=160><font id=yingwen_thead> MAC</font></th>"\
					  			"<th width=70><font id=%s>%s</font></th>",search(lpublic,"menu_thead"),search(lcon,"port_no"));
				      	fprintf(cgiOut,"<th width=80><font id=yingwen_thead>Trunk ID</font></th>"\
					  			"<th width=45><font id=yingwen_thead>VID</font></th>"\
					  			"<th width=45><font id=yingwen_thead>VIDX</font></th>"\
					  			"<th width=60><font id=yingwen_thead>ISTAG</font></th>"\
					  			"<th width=60><font id=yingwen_thead>STATIC</font></th>"\
					  			"<th width=60><font id=yingwen_thead>ISVALID</font></th>"\
					  			"<th width=20></th>"\
                    			"</tr>");
						if(arpNum>0)
						{


							char ipaddr[32]="";
							char mac[64]="";	
							aq=arp_head.next;
					  		for(i=0;i<arpNum;i++)
					  		{

								 
                        		fprintf(cgiOut,"<tr align=left bgcolor=%s>",setclour(cl));
								sprintf( ipaddr, "%d.%d.%d.%d", ((aq->ipaddr & 0xff000000) >> 24),((aq->ipaddr & 0xff0000) >> 16),((aq->ipaddr & 0xff00) >> 8),(aq->ipaddr & 0xff) );
                          		fprintf(cgiOut,"<td>%s</td>", ipaddr );
						  		sprintf( mac, "%02x:%02x:%02x:%02x:%02x:%02x", aq->mac[0],aq->mac[1],aq->mac[2],aq->mac[3],aq->mac[4],aq->mac[5] );
                          		fprintf(cgiOut,"<td>%s</td>", mac );
                          		fprintf(cgiOut,"<td>%d-%d</td>",aq->slot_no,aq->port_no);
						  		if(aq->isTrunk)
                            		fprintf(cgiOut,"<td>%d</td>",aq->trunkId);
						  		else
						  			fprintf(cgiOut,"<td>-</td>");
                          		fprintf(cgiOut,"<td>%d</td>",aq->vid);
                          		fprintf(cgiOut,"<td>-</td>");
						  		if(aq->isTagged==1)
                            		fprintf(cgiOut,"<td>TRUE</td>");
						  		else
						  			fprintf(cgiOut,"<td>FALSE</td>");
						  
							  	if(aq->isStatic==1)
                            		fprintf(cgiOut,"<td>TRUE</td>");
						  		else
						  			fprintf(cgiOut,"<td>FALSE</td>");

								if(aq->isValidz==1)
                            		fprintf(cgiOut,"<td>TRUE</td>");
						  		else
						  			fprintf(cgiOut,"<td>FALSE</td>");

								

								if( 1 == aq->isStatic )
								{
								//输出删除按钮

																
									fprintf( cgiOut, "<td>" );
									fprintf( cgiOut, "<script type=text/javascript>\n" );
									fprintf( cgiOut, "var popMenu%d = new popMenu('popMenu%d','%d');\n", i, i, arpNum-i );
									fprintf( cgiOut, "popMenu%d.addItem( new popMenuItem( '%s', 'wp_prtarp.cgi?encry_PrtArp=%s"\
													"&is_arp=%d&is_nexthop=%d&arp_num=%d-%d&nexthop_num=%s"\
													"&editType=delete&ipaddr=%s/32&static_arp_vlanid=%d&static_arp_mac=%s&static_arp_portnum=%d/%d' ) );\n",
												i,"delete", m, is_arp,is_nexthop,aq->slot_no,aq->port_no,t,
												ipaddr,aq->vid, mac,aq->slot_no, aq->port_no );
									fprintf( cgiOut, "popMenu%d.show();\n", i );
								
									fprintf( cgiOut, "</script>" );

									fprintf( cgiOut, "</td>" );
								}
								else
								{
									fprintf( cgiOut, "<td></td>" );
								}
                        		fprintf(cgiOut,"</tr>");

								              

					    		cl=!cl;
								aq=aq->next;
				      		}
				    	}				

						
				  		fprintf(cgiOut,"</table>");
                	}
                	else if(result1==-1)
				  		fprintf(cgiOut,"%s",search(lcon,"no_port"));
					else if(result1==-2)
				  		fprintf(cgiOut,"%s",search(lcon,"execute_fail"));
					else if(result1==0)
				  		fprintf(cgiOut,"%s",search(lpublic,"contact_adm"));
				}
                fprintf(cgiOut,"</td>"\
              "</tr>"\
              "<tr style=\"padding-top:40px; padding-bottom:20px\">"\
                "<td>%s:</td>",search(lcon,"port_no"));
                fprintf(cgiOut,"<td><select name=nexthop_num style=width:50px>");
				
				ret=show_ethport_list(&head,&num);
				p=head.next;
				if(p!=NULL)
				{
					while(p!=NULL)
					{
						pp=p->port.next;
						while(pp!=NULL)
						{
							memset(slot_port,0,10);							
							sprintf(slot_port,"%d-%d",p->slot_no,pp->port_no);		 /*int转成char*/
							if(strcmp(slot_port,t)==0)
			 				  fprintf(cgiOut,"<option value=%s selected=selected>%s",slot_port,slot_port);
							else
							  fprintf(cgiOut,"<option value=%s>%s",slot_port,slot_port);
							pp=pp->next;
						}
						p=p->next;
					}
				}
			   if((ret==0)&&(num>0))
					Free_ethslot_head(&head);

				  fprintf(cgiOut,"</select></td>"\
                "<td><input type=submit style=\"width:100px\" border=0 name=show_nexthop_apply style=background-image:url(/images/SubBackGif.gif) value=\"%s\"></td>", search(lcon,"prt_show_nexthop"));
				  

			  fprintf(cgiOut,"</tr>"\
              "<tr style=\"padding-bottom:10px\">");
              //fprintf(cgiOut,"<td colspan=4>", search(lcon,"prt_show_nexthop"), search(lcon,"prt_clear_nexthop"));
			  fprintf(cgiOut,"<td colspan=4>");

				if(is_nexthop == 1)
				{
					if(result2==1)
					{				  
                  		fprintf(cgiOut,"<table frame=below rules=rows width=620 border=1>"\
		            			"<tr align=left>"\
                      			"<th width=110><font id=yingwen_thead> IP</font></th>"\
								  "<th width=120><font id=yingwen_thead> MAC</font></th>"\
								  "<th width=70><font id=%s>%s</font></th>",search(lpublic,"menu_thead"),search(lcon,"port_no"));
				      	fprintf(cgiOut,"<th width=80><font id=yingwen_thead>Trunk ID</font></th>"\
							  "<th width=45><font id=yingwen_thead>VID</font></th>"\
							  "<th width=45><font id=yingwen_thead>VIDX</font></th>"\
							  "<th width=60><font id=yingwen_thead>ISTAG</font></th>"\
							  "<th width=60><font id=yingwen_thead>STATIC</font></th>"\
							  "<th width=30><font id=yingwen_thead>REF</font></th>"\
		                    "</tr>");
						if(nexthopNum > 0)
						{
					  		cl=1;
					  		nq=nexthop_head.next;
					  		for(i=0;i<nexthopNum;i++)
					  		{
                        		fprintf(cgiOut,"<tr align=left bgcolor=%s>",setclour(cl));
                          		fprintf(cgiOut,"<td>%d.%d.%d.%d</td>",((nq->ipaddr & 0xff000000) >> 24),((nq->ipaddr & 0xff0000) >> 16),((nq->ipaddr & 0xff00) >> 8),(nq->ipaddr & 0xff));
                          		fprintf(cgiOut,"<td>%02x:%02x:%02x:%02x:%02x:%02x</td>",nq->mac[0],nq->mac[1],nq->mac[2],nq->mac[3],nq->mac[4],nq->mac[5]);
                          		fprintf(cgiOut,"<td>%d/%d</td>",nq->slot_no,nq->port_no);
						  		if(nq->isTrunk)
                            		fprintf(cgiOut,"<td>%d</td>",nq->trunkId);
						  		else
						  			fprintf(cgiOut,"<td>-</td>");
                          		fprintf(cgiOut,"<td>%d</td>",nq->vid);
                          		fprintf(cgiOut,"<td>-</td>");
						  		if(nq->isTagged==1)
                            		fprintf(cgiOut,"<td>TRUE</td>");
						  		else
						  			fprintf(cgiOut,"<td>FALSE</td>");
						  		if(nq->isStatic==1)
                            		fprintf(cgiOut,"<td>TRUE</td>");
						  		else
						  			fprintf(cgiOut,"<td>FALSE</td>");
                          		fprintf(cgiOut,"<td>%d</td>",nq->refCnt);
                        		fprintf(cgiOut,"</tr>");
					    		cl=!cl;
								nq=nq->next;
				      		}
				  		}
				  		fprintf(cgiOut,"</table>");
					}
					else if(result2==-1)
				  		fprintf(cgiOut,"%s",search(lcon,"no_port"));
					else if(result2==-2)
				  		fprintf(cgiOut,"%s",search(lcon,"execute_fail"));
					else if(result2==0)
				  		fprintf(cgiOut,"%s",search(lpublic,"contact_adm"));
				}
                fprintf(cgiOut,"</td>"\
              "</tr>"\
              "<tr>"\
                "<td><input type=hidden name=encry_PrtArp value=%s></td>",m);
				fprintf(cgiOut,"<td><input type=hidden name=is_arp value=%d></td>",is_arp);
				fprintf(cgiOut,"<td colspan=2><input type=hidden name=is_nexthop value=%d></td>",is_nexthop);
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
free(slot_port);
free(sr.modname);
free(sr.sn); 

if((result1==1)&&(arpNum>0))
  Free_arp_nexthop_head(&arp_head);

if((result2==1)&&(nexthopNum>0))
  Free_arp_nexthop_head(&nexthop_head);

}        



static int delete_static_arp()
{
	char sz_edit_type[16] = "";
	int error = 0;
	
//编辑类型，如果没有该参数，编辑类型为add，如果为delete,表示删除一个静态arp
//delete是从wp_prtarp.cgi连接过来的，处理完了要再连接回去
	
	cgiFormStringNoNewlines( "editType", sz_edit_type, sizeof(sz_edit_type) );
	
	if( strcmp( sz_edit_type, "delete" ) == 0 )
	{
		char port_num[16] = "";
		char mac[64] = "";
		char ipaddr[32] = "";
		char vlanID[16] = "";
		
		//端口号
		cgiFormStringNoNewlines( "static_arp_portnum", port_num, sizeof(port_num) );
		
		//mac
		cgiFormStringNoNewlines( "static_arp_mac", mac, sizeof(mac) );
	
		cgiFormStringNoNewlines( "ipaddr", ipaddr, sizeof(ipaddr) );
		
		//vlanid
		cgiFormStringNoNewlines( "static_arp_vlanid", vlanID, sizeof(vlanID) );
		
		error = delete_ip_static_arp( port_num, mac, ipaddr, vlanID );
	}

	return error;
}

static void show_static_arp_delete_err( int errorno,   struct list *lcon )
{
    char *error_message=NULL;
    
    if( 0 == errorno )
	{
		return;
	}
	
	switch(errorno)
	{
 	    case WP_ERR_NULL_POINTER:
 	        error_message = search( lcon, "static_arp_out_mem" );
 	        break;
 	    case WP_ERR_IPADDR_LEN:
 	        error_message = search( lcon, "s_arp_ip_err" );
 	        break;
 	    case WS_ERR_MAC_FORMAT:
 	        error_message = search( lcon, "s_arp_mac_err_format" );
 	        break;
 	    case WS_ERR_MAC_BROADCAST:
 	        error_message = search( lcon, "s_arp_mac_broadcast" );
 	        break;
 	    case WS_ERR_VLANID_OUTRANGE:
 	        error_message = search( lcon, "s_arp_vlan_out_range" );
 	        break;
		case NPD_DBUS_ERROR:
			error_message = search( lcon, "s_arp_intf_create_err" );
			break;
		case DCLI_VLAN_BADPARAM:
			error_message = search( lcon, "s_arp_bad_param" );
			break;
		case DCLI_VLAN_NOTEXISTS:
			 error_message = search( lcon, "s_arp_vlan_not_l3" );
			break;
		case DCLI_INTF_NOTEXISTED:
			error_message = search( lcon, "s_arp_l3_not_active" );
			break;
		case DCLI_DBUS_PORT_NOT_IN_VLAN:
			error_message = search( lcon, "s_arp_port_not_in_vlan" );
			break;
		case DCLI_ARP_SNOOPING_ERR_STATIC_NOTEXIST:
		case DCLI_ARP_SNOOPING_ERR_PORT_NOTMATCH:
		case NPD_DBUS_ERROR_NO_SUCH_PORT:	
		default:
			error_message = search( lcon, "s_arp_unknow_err" );
			break;
	}
	
	return;	
}


