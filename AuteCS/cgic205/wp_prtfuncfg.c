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
* wp_prtfuncfg.c
*
*
* CREATOR:
* autelan.software.Network Dep. team
* zhouym@autelan.com
*
* DESCRIPTION:
* system contrl for port function config
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
#include <sys/wait.h>


char *port_mode[] = {   /*port mode*/
	"switch",
	"route",
	"promiscuous",
};

void ShowPrtFunCfgPage(char *m,char *n,char *t,char *s,struct list *lpublic,struct list *lcon);     /*m代表加密后的字符串*/
void PortFunConfig(char *Pname,char *Pmode,struct list *lpublic,struct list *lcon);


int cgiMain()
{
  char *encry=(char *)malloc(BUF_LEN);			 /*存储从wp_topFrame.cgi带入的加密字符串*/    
  char *port_mode=(char *)malloc(15);
  char *port_name=(char *)malloc(10);
  char *default_port=(char *)malloc(20);
  char *str;	         /*存储解密后的当前登陆用户名*/  
  struct list *lpublic;   /*解析public.txt文件的链表头*/
  struct list *lcon;      /*解析control.txt文件的链表头*/  
  ETH_SLOT_LIST  head,*p;
  ETH_PORT_LIST *pp;
  int num,ret;
  lpublic=get_chain_head("../htdocs/text/public.txt");
  lcon=get_chain_head("../htdocs/text/control.txt");
  memset(encry,0,BUF_LEN);  
  memset(port_mode,0,15);
  memset(port_name,0,10);
  if(cgiFormStringNoNewlines("UN", encry, BUF_LEN)!=cgiFormNotFound )  /*首次进入该页*/
  {
	str=dcryption(encry);
    if(str==NULL)
	  ShowErrorPage(search(lpublic,"ill_user")); 	  /*用户非法*/
    else
    {	  
      if(cgiFormStringNoNewlines("ID", port_name, 10)!=cgiFormNotFound )
	    ShowPrtFunCfgPage(encry,str,"switch",port_name,lpublic,lcon);
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
        ShowPrtFunCfgPage(encry,str,"switch",default_port,lpublic,lcon);
		 if((ret==0)&&(num>0))
          {
        	Free_ethslot_head(&head);
          }

		 //////////////////////释放地点
	  }
    }
  }
  else
  {
    cgiFormStringNoNewlines("encry_PrtFunCfg",encry,BUF_LEN);
    cgiFormStringNoNewlines("port_mode",port_mode,15);  	
	cgiFormStringNoNewlines("port_num",port_name,10);   
	str=dcryption(encry);
    if(str==NULL)
	  ShowErrorPage(search(lpublic,"ill_user")); 	  /*用户非法*/
    else
      ShowPrtFunCfgPage(encry,str,port_mode,port_name,lpublic,lcon);
  }
  free(encry);  
  free(port_mode);
  free(port_name);
  free(default_port);
  release(lpublic);  
  release(lcon);
 
  return 0;
}

void ShowPrtFunCfgPage(char *m,char *n,char *t,char *s,struct list *lpublic,struct list *lcon)
{ 
  char *slot_port=(char *)malloc(10);
  int i,modeChoice,num,ret;
  char * url_temp =(char *)malloc(1024);
  memset(url_temp,0,1024);
  struct eth_port_s pr;
  struct slot sr;
  ETH_SLOT_LIST  head,*p;
  ETH_PORT_LIST *pp;
  pr.attr_bitmap=0;
  pr.mtu=0;
  pr.port_type=0;
  sr.module_status=0;     
  sr.modname=(char *)malloc(20);     //为结构体成员申请空间，假设该字段的最大长度为20
  sr.sn=(char *)malloc(20);          //为结构体成员申请空间，假设该字段的最大长度为20
  sr.hw_ver=0;
  sr.ext_slot_num=0;  
  cgiHeaderContentType("text/html");
  fprintf(cgiOut,"<html xmlns=\"http://www.w3.org/1999/xhtml\"><head>");
  fprintf(cgiOut,"<meta http-equiv=Content-Type content=text/html; charset=gb2312>");
  fprintf(cgiOut,"<title>%s</title>",search(lcon,"prt_manage"));
  fprintf(cgiOut,"<link rel=stylesheet href=/style.css type=text/css>"\
  	"<style type=text/css>"\
  	".a3{width:30;border:0; text-align:center}"\
  	"</style>"\
  "</head>"\
  "<script src=/ip.js>"\
  "</script>"\
  "<body>");

  
  if(cgiFormSubmitClicked("PrtFunCfg_apply") == cgiFormSuccess)
  {
    PortFunConfig(s,t,lpublic,lcon);
  }
  fprintf(cgiOut,"<form>"\
  "<div align=center>"\
  "<table width=976 border=0 cellpadding=0 cellspacing=0>"\
  "<tr>"\
      "<td width=8 align=left valign=top background=/images/di22.jpg><img src=/images/youce4.jpg width=8 height=30/></td>"\
    "<td width=51 align=left valign=bottom background=/images/di22.jpg><img src=/images/youce33.jpg width=37 height=24/></td>"\
    "<td width=153 align=left valign=bottom id=%s background=/images/di22.jpg>%s</td>",search(lpublic,"title_style"),search(lcon,"prt_manage"));
    fprintf(cgiOut,"<td width=692 align=right valign=bottom background=/images/di22.jpg>");
		{	
		  fprintf(cgiOut,"<table width=130 border=0 cellspacing=0 cellpadding=0>"\
		  "<tr>"\
		  "<td width=62 align=center><input id=but type=submit name=PrtFunCfg_apply style=background-image:url(/images/%s) value=""></td>"\
		  "<td width=62 align=left><a href=wp_contrl.cgi?UN=%s target=mainFrame><img src=/images/%s border=0 width=62 height=20/></a></td>",search(lpublic,"img_ok"),m,search(lpublic,"img_cancel"));
		  fprintf(cgiOut,"</tr>"\
		  "</table>");
		}
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
					"<tr height=25>"\
  					    "<td align=left id=tdleft><a href=wp_prtarp.cgi?UN=%s target=mainFrame class=top><font id=yingwen_san>ARP<font><font id=%s> %s<font></a></td></tr>",m,search(lpublic,"menu_san"),search(lpublic,"survey"));                       
					fprintf(cgiOut,"<tr height=25>"\
					  "<td align=left id=tdleft><a href=wp_static_arp.cgi?UN=%s target=mainFrame class=top><font id=%s>%s<font></a></td></tr>",m,search(lpublic,"menu_san"),search(lcon,"prt_static_arp"));						 
					
                    fprintf(cgiOut,"<tr height=25>"\
  					  "<td align=left id=tdleft><a href=wp_prtcfg.cgi?UN=%s target=mainFrame class=top><font id=%s>%s<font></a></td>",m,search(lpublic,"menu_san"),search(lcon,"prt_cfg"));                       
                    fprintf(cgiOut,"</tr>"\
                    "<tr height=26>"\
                      "<td align=left id=tdleft background=/images/bottom_bg.gif style=\"border-right:0\"><font id=%s>%s<font></td>",search(lpublic,"menu_san"),search(lcon,"func_cfg"));     /*突出显示*/
                    fprintf(cgiOut,"</tr>");
					//add by sjw  2008-10-9 17:14:37  for  subinterface
					fprintf(cgiOut,"<tr height=25>"\
  					    "<td align=left id=tdleft><a href=wp_subintf.cgi?UN=%s target=mainFrame class=top><font id=%s>%s<font></a></td></tr>",m,search(lpublic,"menu_san"),search(lcon,"title_subintf"));  	                    
										fprintf(cgiOut,"<tr height=25>"\
						  "<td align=left id=tdleft><a href=wp_interface_bindip.cgi?UN=%s target=mainFrame class=top><font id=%s>%s<font></a></td></tr>",m,search(lpublic,"menu_san"),search(lpublic,"config_interface"));
					//////INTF ////////
					fprintf(cgiOut,"<tr height=25>"\
  					    "<td align=left id=tdleft><a href=wp_all_interface.cgi?UN=%s target=mainFrame class=top><font id=%s>%s<font></a></td></tr>",m,search(lpublic,"menu_san"),search(lcon,"interface"));		

                fprintf(cgiOut,"</table>"\
              "</td>"\
              "<td align=left style=\"background-color:#ffffff; border-right:1px solid #707070; padding-left:30px\">"\
				"<table width=390 border=0 cellspacing=0 cellpadding=0>"\
				   "<tr height=30>"\
					  "<td width=100px>%s:</td>",search(lcon,"port_no"));
					  fprintf(cgiOut,"<td colspan=3><select name=port_num style=width:50px>");  
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
								  memset(slot_port,0,10);							
							      sprintf(slot_port,"%d-%d",p->slot_no,pp->port_no);
							      if(strcmp(slot_port,s)==0)
								    fprintf(cgiOut,"<option value=%s selected=selected>%s",slot_port,slot_port);
							      else
							        fprintf(cgiOut,"<option value=%s>%s",slot_port,slot_port);
								  pp=pp->next;
							  }
							  p=p->next;
						  }
					  }
			          fprintf(cgiOut,"</select></td>");  //端口号的下拉框显示		
			          
					  /*---------------------------------------------------------------*/
					  
					fprintf(cgiOut,"</tr>"\
					"<tr height=30>"\
					  "<td >%s:</td>",search(lpublic,"mode"));
					  fprintf(cgiOut,"<td width=140 colspan=2><select name=port_mode style=width:95px onchange=\"javascript:this.form.submit();\">");
					    for(i=0;i<3;i++)
			            if(strcmp(port_mode[i],t)==0)              /*显示上次选中的port mode*/
       	                  fprintf(cgiOut,"<option value=%s selected=selected>%s",port_mode[i],port_mode[i]);
			            else			  	
			              fprintf(cgiOut,"<option value=%s>%s",port_mode[i],port_mode[i]);
					  fprintf(cgiOut,"</select></td>");
			        fprintf(cgiOut,"<td width=200><font color=red>%s</font></td>", search( lcon,"prt_mod_default"));
					fprintf(cgiOut,"</tr>");
					cgiFormSelectSingle("port_mode", port_mode, 3, &modeChoice, 0);
					#if 0
					if(modeChoice!=0)
					{
					  fprintf(cgiOut,"<tr height=30>"\
					    "<td>IP:</td>"\
					    "<td>" );
					      fprintf( cgiOut, "<div style=\"border-width:1;border-color:#a5acb2;border-style:solid;width:140;font-size:9pt\">" );
              	          fprintf( cgiOut, "<input type=text name='port_ip1' maxlength=3 class=a3 onKeyUp=\"mask(this,%s)\" onbeforepaste=mask_c()>.",search(lpublic,"ip_error"));
              	          fprintf(cgiOut,"<input type=text name='port_ip2' maxlength=3 class=a3 onKeyUp=\"mask(this,%s)\" onbeforepaste=mask_c()>.",search(lpublic,"ip_error"));
              	          fprintf(cgiOut,"<input type=text name='port_ip3' maxlength=3 class=a3 onKeyUp=\"mask(this,%s)\" onbeforepaste=mask_c()>.",search(lpublic,"ip_error"));
              	          fprintf(cgiOut,"<input type=text name='port_ip4' maxlength=3 class=a3 onKeyUp=\"mask(this,%s)\" onbeforepaste=mask_c()>",search(lpublic,"ip_error"));
              	          fprintf(cgiOut,"</div>"\
					    "</td>"\
					    "<td>/</td>" );
					      fprintf( cgiOut, "<td><div style=\"border-width:1;border-color:#a5acb2;border-style:solid;width:140;font-size:9pt\">" );
              	          fprintf( cgiOut, "<input type=text name='port_mask1' maxlength=3 class=a3 onKeyUp=\"mask(this,%s)\" onbeforepaste=mask_c()>.",search(lpublic,"ip_error"));
              	          fprintf(cgiOut,"<input type=text name='port_mask2' maxlength=3 class=a3 onKeyUp=\"mask(this,%s)\" onbeforepaste=mask_c()>.",search(lpublic,"ip_error"));
              	          fprintf(cgiOut,"<input type=text name='port_mask3' maxlength=3 class=a3 onKeyUp=\"mask(this,%s)\" onbeforepaste=mask_c()>.",search(lpublic,"ip_error"));
              	          fprintf(cgiOut,"<input type=text name='port_mask4' maxlength=3 class=a3 onKeyUp=\"mask(this,%s)\" onbeforepaste=mask_c()>",search(lpublic,"ip_error"));
					    fprintf( cgiOut, "</div></td>"\
					  "</tr>");
					}
					#endif
					if(modeChoice==1)
					{
						sprintf( url_temp, "wp_interface_bindip.cgi?UN=%s&PORT=%s&MODE=%s",m,s,"route");
						fprintf( cgiOut, "<script type='text/javascript'>\n" );
						fprintf( cgiOut, "window.location.href='%s';\n", url_temp );
						fprintf( cgiOut, "</script>\n" );
					}
					else if(modeChoice==2)
					{
						sprintf( url_temp, "wp_interface_bindip.cgi?UN=%s&PORT=%s&MODE=%s",m,s,"promiscuous");
						fprintf( cgiOut, "<script type='text/javascript'>\n" );
						fprintf( cgiOut, "window.location.href='%s';\n", url_temp );
						fprintf( cgiOut, "</script>\n" );
					}
					fprintf(cgiOut,"<tr>"\
                      "<td colspan=4><input type=hidden name=encry_PrtFunCfg value=%s></td>",m);
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
free(url_temp);
if((ret==0)&&(num>0))
{
	Free_ethslot_head(&head);
}
}        

int maskstr2int( char *mask )
{
	unsigned int iMask, m0, m1, m2, m3;
	char binarystr[64]="";
	int i, iRet;
	
	sscanf( mask, "%u.%u.%u.%u", &m3,&m2,&m1,&m0 );
	iMask = m3*256*256*256 + m2*256*256 + m1*256 + m0;
//	printf( "m3=%u  m2=%u m1=%u  m0=%u  iMask = %u", m3, m2, m1, m0, iMask );
	
	iRet = 0;
	for( i=0; i < 32 ;i++ )
	{
		if( ( iMask & 1 ) == 1 )
		{
			binarystr[31-i] = '1';
			iRet ++;
		}
		else
		{
			binarystr[31-i] = '0';	
		}
		iMask = iMask >> 1;
	}
	
//	printf( "binarystr = %s", binarystr );
	if( strstr( binarystr, "01" ) )
	{
		return -1;	
	}
	
	return iRet;
}

void PortFunConfig(char *Pname,char *Pmode,struct list *lpublic,struct list *lcon)
{
  int ret,flag=1;    
  char *port_ip=(char *)malloc(20);  
  char *port_mask=(char *)malloc(20);
  #if 0
  int status,retu;
  char ip1[4],ip2[4],ip3[4],ip4[4];
  char mask1[4],mask2[4],mask3[4],mask4[4];
  char *mask=(char *)malloc(5);  
  char *command=(char *)malloc(50);
  
  if(strcmp(Pmode,"switch")!=0)
  {
    memset(port_ip,0,20);                                 /*security auth ip*/
	memset(ip1,0,4);
	cgiFormStringNoNewlines("port_ip1",ip1,4);	
	port_ip=strcat(port_ip,ip1);
	port_ip=strcat(port_ip,".");
	memset(ip2,0,4);
	cgiFormStringNoNewlines("port_ip2",ip2,4); 
	port_ip=strcat(port_ip,ip2);	
	port_ip=strcat(port_ip,".");
    memset(ip3,0,4);
    cgiFormStringNoNewlines("port_ip3",ip3,4); 
    port_ip=strcat(port_ip,ip3);	
    port_ip=strcat(port_ip,".");
    memset(ip4,0,4);
    cgiFormStringNoNewlines("port_ip4",ip4,4);
    port_ip=strcat(port_ip,ip4);
	
	memset(port_mask,0,20); 
	memset(mask1,0,4);
	cgiFormStringNoNewlines("port_mask1",mask1,4);	
	port_mask=strcat(port_mask,mask1);
	port_mask=strcat(port_mask,".");
	memset(mask2,0,4);
	cgiFormStringNoNewlines("port_mask2",mask2,4); 
	port_mask=strcat(port_mask,mask2);	
	port_mask=strcat(port_mask,".");
    memset(mask3,0,4);
    cgiFormStringNoNewlines("port_mask3",mask3,4); 
    port_mask=strcat(port_mask,mask3);	
    port_mask=strcat(port_mask,".");
    memset(mask4,0,4);
    cgiFormStringNoNewlines("port_mask4",mask4,4);	   
    port_mask=strcat(port_mask,mask4);
	
	
//	memset(mask,0,5);
//    cgiFormStringNoNewlines("ip_mask",mask,5);
	
//	printf( "ip1=%s   ip2=%s   ip3=%s   ip4=%s<br />", ip1, ip2, ip3, ip4 );
//	printf( "mask1=%s   mask2=%s   mask3=%s   mask4=%s<br />", mask1, mask2, mask3, mask4 );	
	if((strcmp(ip1,"")==0)||(strcmp(ip2,"")==0)||(strcmp(ip3,"")==0)||(strcmp(ip4,"")==0))
	{
      ShowAlert(search(lpublic,"ip_not_null"));
	  return;
	}
	if((strcmp(mask1,"")==0)||(strcmp(mask2,"")==0)||(strcmp(mask3,"")==0)||(strcmp(mask4,"")==0) )
	{
      ShowAlert(search(lpublic,"mask_not_null"));
	  return;
	}
	//ret=strtoul(mask,&endptr,10);					   /*char转成int，10代表十进制*/		
	ret = maskstr2int( port_mask );
	sprintf( mask, "%d", ret );
//	printf( "<br />mask = %s<br />", mask );
	if((ret<=0)||(ret>32))           /*最多WLAN_NUM个wlan*/   
	{
      ShowAlert(search(lpublic,"mask_illegal"));
	  return;
	}
  }
  #endif
  ccgi_dbus_init();
  ret=ccgi_port_mode_conf(Pname,Pmode);   /*返回0表示失败，返回1表示成功，返回-1表示no such port，返回-2表示it is already this mode*/
										  /*返回-3表示unsupport this command，返回-4表示execute command failed*/
  switch(ret)
  {
    case 0:{
		     ShowAlert(search(lcon,"con_port_mode_fail"));
			 flag=0;
             break;
    	   }
    case 1:break;
    case -1:{
		      ShowAlert(search(lcon,"no_port"));
			  flag=0;
              break;
    	    }
    case -2:{
		      ShowAlert(search(lcon,"already_mode"));
			  flag=0;
              break;
    	    }
    case -3:{
		      ShowAlert(search(lcon,"not_support"));
			  flag=0;
              break;
    	    }
	case -4:{
		      ShowAlert(search(lcon,"execute_fail"));
			  flag=0;
              break;
    	    }
  }   
  #if 0
  if(strcmp(Pmode,"switch")!=0)
  {
    	memset(command,0,50);
	strcat(command,"set_intf_ip.sh");
	strcat(command," ");
	if(strcmp(Pmode,"route")==0)   /*route*/
	  strcat(command,"eth.r.");
	else if(strcmp(Pmode,"promiscuous")==0)   /*promiscuous*/
	  strcat(command,"eth.p.");
	strcat(command,Pname);
	strcat(command," ");
	strcat(command,port_ip);
	strcat(command,"/");
	strcat(command,mask);

	status = system(command); 	 
	retu = WEXITSTATUS(status);
	if(0!=retu)    /*command fail*/
	{
	  flag=0;
	  ShowAlert(search(lcon,"con_ip_fail"));
	}
  }
  #endif
  
  if(flag)
  {
    if(strcmp(Pmode,"switch")==0)
	  ShowAlert(search(lcon,"con_port_mode_succ"));
	else
  	  ShowAlert(search(lpublic,"oper_succ"));
  }
  free(port_ip);
  free(port_mask);
}

