/* cgicTempDir is the only setting you are likely to need
	to change in this file. */

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
* wp_secscon.c
*
*
* CREATOR:
* autelan.software.Network Dep. team
* qiaojie@autelan.com
*
* DESCRIPTION:
*
*
*
*******************************************************************************/


#include <stdio.h>
#include "cgic.h"
#include <string.h>
#include <stdlib.h>
#include "ws_usrinfo.h"
#include "ws_err.h"
#include "ws_ec.h"
#include "wcpss/asd/asd.h"
#include "ws_security.h"
#include "ws_dcli_vrrp.h"
#include "ws_init_dbus.h"
#include "ws_dbus_list_interface.h"

char *secur_type[] = {   /*security type*/
	"802.1x",
	"WPA_E",
	"WPA2_E",
};


int ShowSecurityConfigPage(char *m,char *n,char *t,char *r,struct list *lpublic,struct list *lsecu);    /*m代表加密字符串，n代表security type，t代表选中的security id，r代表选中的encryption type*/
void Config_Security(instance_parameter *ins_para,char *id,char *secu_type,struct list *lpublic,struct list *lsecu);

int cgiMain()
{
  char encry[BUF_LEN] = { 0 };              
  char *str = NULL;    
  char secChoice[10] = { 0 };  
  char secID[5] = { 0 };
  char enc[5] = { 0 };
  struct list *lpublic = NULL;   /*解析public.txt文件的链表头*/
  struct list *lsecu = NULL;     /*解析security.txt文件的链表头*/  
  lpublic=get_chain_head("../htdocs/text/public.txt");
  lsecu=get_chain_head("../htdocs/text/security.txt");
  
  DcliWInit();
  ccgi_dbus_init();
  memset(secChoice,0,sizeof(secChoice));
  memset(secID,0,sizeof(secID));
  memset(enc,0,sizeof(enc));
  memset(encry,0,sizeof(encry));  
  if(cgiFormStringNoNewlines("UN", encry, BUF_LEN)!=cgiFormNotFound )  /*首次进入该页*/
  {
    str=dcryption(encry);
    if(str==NULL)
     ShowErrorPage(search(lpublic,"ill_user"));		 /*用户非法*/
    else if(cgiFormStringNoNewlines("SecuID", secID, 5)!=cgiFormNotFound )   /*从wp_seculis.cgi进入该页*/
	  ShowSecurityConfigPage(encry,"802.1x",secID,"WEP",lpublic,lsecu); 
	else
      ShowSecurityConfigPage(encry,"802.1x","1","WEP",lpublic,lsecu); 
  }
  else                    
  {      
    cgiFormStringNoNewlines("secur_id",secID,5);
  //  cgiFormStringNoNewlines("secur_type",secChoice,10);	
    cgiFormStringNoNewlines("encry_type",enc,5);
	cgiFormStringNoNewlines("encry_secon",encry,BUF_LEN);		
	str=dcryption(encry);
    if(str==NULL)
     ShowErrorPage(search(lpublic,"ill_user"));		 /*用户非法*/
    else
     ShowSecurityConfigPage(encry,secChoice,secID,enc,lpublic,lsecu); 
  }
  release(lpublic);  
  release(lsecu);
  destroy_ccgi_dbus();
  return 0;
}

int ShowSecurityConfigPage(char *m,char *n,char *t,char *r,struct list *lpublic,struct list *lsecu)
{ 
  char select_insid[10] = { 0 };
  int i = 0;         /*sectypeChoice存储选择的security type值*/
  char sID[5] = { 0 };
  struct dcli_security *head = NULL,*q = NULL;          /*存放security信息的链表头*/       
  int sec_num = 0;             /*存放security的个数*/
  int result = 0;
  instance_parameter *paraHead1 = NULL,*paraHead2 = NULL;
  instance_parameter *pq = NULL;
  char temp[10] = { 0 };
  dbus_parameter ins_para;
  cgiHeaderContentType("text/html");
  fprintf(cgiOut,"<html xmlns=\"http://www.w3.org/1999/xhtml\"><head>");
  fprintf(cgiOut,"<meta http-equiv=Content-Type content=text/html; charset=gb2312>");
  fprintf(cgiOut,"<title>%s</title>",search(lsecu,"wlan_sec"));
  fprintf(cgiOut,"<link rel=stylesheet href=/style.css type=text/css>"\
  	"<style type=text/css>"\
  	".a3{width:30;border:0; text-align:center}"\
  	"</style>"\
  "</head>"\
  "<script src=/instanceid_onchange.js>"\
  "</script>"\
  "<script src=/ip.js>"\
  "</script>");
  	fprintf(cgiOut,"<body>");   
  memset(select_insid,0,sizeof(select_insid));
  cgiFormStringNoNewlines( "INSTANCE_ID", select_insid, 10 );
  if(strcmp(select_insid,"")==0)
  {	
	list_instance_parameter(&paraHead1, INSTANCE_STATE_WEB);	
	if(paraHead1)
	{
		snprintf(select_insid,sizeof(select_insid)-1,"%d-%d-%d",paraHead1->parameter.slot_id,paraHead1->parameter.local_id,paraHead1->parameter.instance_id);
	}
  } 
  else
  {
	get_slotID_localID_instanceID(select_insid,&ins_para);	
	get_instance_dbus_connection(ins_para, &paraHead1, INSTANCE_STATE_WEB);
  }
  if(cgiFormSubmitClicked("submit_secon") == cgiFormSuccess)
  {
  	if(paraHead1)
	{
		Config_Security(paraHead1,t,n,lpublic,lsecu);
	}
  }
  fprintf(cgiOut,"<form method=post >"\
  "<div align=center>"\
  "<table width=976 border=0 cellpadding=0 cellspacing=0>"\
  "<tr>"\
    "<td width=8 align=left valign=top background=/images/di22.jpg><img src=/images/youce4.jpg width=8 height=30/></td>"\
    "<td width=51 align=left valign=bottom background=/images/di22.jpg><img src=/images/youce33.jpg width=37 height=24/></td>"\
    "<td width=153 align=left valign=bottom id=%s background=/images/di22.jpg>%s</td>",search(lpublic,"title_style"),search(lsecu,"wlan_sec"));
    fprintf(cgiOut,"<td width=690 align=right valign=bottom background=/images/di22.jpg>");
	  
    	  fprintf(cgiOut,"<table width=130 border=0 cellspacing=0 cellpadding=0>"\
          "<tr>"\
          "<td width=62 align=center><input id=but type=submit name=submit_secon style=background-image:url(/images/%s) value=""></td>",search(lpublic,"img_ok"));		  
          fprintf(cgiOut,"<td width=62 align=center><a href=wp_wlan.cgi?UN=%s target=mainFrame><img src=/images/%s border=0 width=62 height=20/></a></td>",m,search(lpublic,"img_cancel"));
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
					"<td align=left id=tdleft><a href=wp_seculis.cgi?UN=%s target=mainFrame class=top><font id=%s>%s</font></a></td>",m,search(lpublic,"menu_san"),search(lsecu,"secur_list"));                       
                  fprintf(cgiOut,"</tr>"\
				  "<tr height=25>"\
					"<td align=left id=tdleft><a href=wp_secre.cgi?UN=%s target=mainFrame class=top><font id=%s>%s</font></a></td>",m,search(lpublic,"menu_san"),search(lsecu,"create_sec"));                       
                  fprintf(cgiOut,"</tr>"\
				  "<tr height=25>"\
                    "<td align=left id=tdleft><a href=wp_secon.cgi?UN=%s target=mainFrame class=top><font id=%s>%s</font></a></td>",m,search(lpublic,"menu_san"),search(lsecu,"config_sec"));   
                  fprintf(cgiOut,"</tr>");
				   fprintf(cgiOut,"<tr height=26>"\
					  "<td align=left id=tdleft background=/images/bottom_bg.gif style=\"border-right:0\"><font id=%s>%s</font></td>",search(lpublic,"menu_san"),search(lsecu,"config_secon_sec"));    /*突出显示*/                   
                    fprintf(cgiOut,"</tr>");

					 fprintf(cgiOut,"<tr height=25>"\
					  "<td align=left id=tdleft><a href=wp_cer_dload.cgi?UN=%s target=mainFrame class=top><font id=%s>%s</font></a></td>",m,search(lpublic,"menu_san"),search(lpublic,"cer_up"));                       
                    fprintf(cgiOut,"</tr>");
                  for(i=0;i<6;i++)
	              {
  				    fprintf(cgiOut,"<tr height=25>"\
                      "<td id=tdleft>&nbsp;</td>"\
                    "</tr>");
	              }
                fprintf(cgiOut,"</table>"\
              "</td>"\
              "<td align=left valign=top style=\"background-color:#ffffff; border-right:1px solid #707070; padding-left:30px; padding-top:10px\">"\
          "<table width=300 border=0 cellspacing=0 cellpadding=0>"\
			 "<tr height=30>"\
			  "<td width=110 id=sec2>%s ID:</td>",search(lpublic,"instance"));
			  fprintf(cgiOut,"<td align=left width=190>"\
				  "<select name=instance_id id=instance_id style=width:65px onchange=instanceid_change(this,\"wp_secscon.cgi\",\"%s\")>",m);	
				  list_instance_parameter(&paraHead2, INSTANCE_STATE_WEB);	 
				  for(pq=paraHead2;(NULL != pq);pq=pq->next)				  
				  {
					  memset(temp,0,sizeof(temp));
					  snprintf(temp,sizeof(temp)-1,"%d-%d-%d",pq->parameter.slot_id,pq->parameter.local_id,pq->parameter.instance_id);
					   
					  if(strcmp(select_insid,temp) == 0)					  
						fprintf(cgiOut,"<option value='%s' selected=selected>%s",temp,temp);
					  else
						fprintf(cgiOut,"<option value='%s'>%s",temp,temp);
				  } 	
				  free_instance_parameter_list(&paraHead2);
				  fprintf(cgiOut,"</select>"\
			  "</td>"\
			"<tr>");		  
		  if(paraHead1)
		  {
			  result=show_security_list(paraHead1->parameter,paraHead1->connection,&head,&sec_num);
		  }
		  if(result == 1)	/*显示所有security的信息，head返回security信息链表的链表头*/
		  {
            if(sec_num>0)    /*如果security存在*/
            {          
              /**********************  security ID && security type  ************************************/
              fprintf(cgiOut,"<tr>"\
                "<td colspan=2><table width=300 border=0 cellspacing=0 cellpadding=0>"\
                    "<tr height=30>"\
                      "<td width=110 id=sec2>%s ID:</td>",search(lpublic,"security"));
                      fprintf(cgiOut,"<td width=190><select name=secur_id style=width:65px>");        
       		   	      for(i=0,q=head;((i<sec_num)&&(NULL != q));i++,q=q->next)
			          {
				        snprintf(sID,sizeof(sID)-1,"%d",q->SecurityID);	  /*int转成char*/		
        				if(strcmp(t,sID)==0)
                    	  fprintf(cgiOut,"<option value=%d selected=selected>%d",q->SecurityID,q->SecurityID);
        				else				  	
        				  fprintf(cgiOut,"<option value=%d>%d",q->SecurityID,q->SecurityID);
				      }	
			          fprintf(cgiOut,"</select>"\
          	          "</td>"\
                    "</tr>"\
                    "</table>"\
               "</td>"\
            "</tr>");
		//    cgiFormSelectSingle("secur_type", secur_type, 3, &sectypeChoice, 0);
			/*
                       
            fprintf(cgiOut,"<tr style=padding-top:20px>"\
              "<td colspan=2><table width=300 border=0 cellspacing=0 cellpadding=0>"\
                  "<tr>"\
                    "<td id=sec1 colspan=2 style=\"border-bottom:2px solid #53868b\">%s %s</td>",secur_type[sectypeChoice],search(lsecu,"para"));
                  fprintf(cgiOut,"</tr>"\
                  "<tr style=padding-top:10px>"\
                    "<td width=130 id=sec2>%s:</td>",search(lsecu,"encry_type"));
				    switch(sectypeChoice)
				    {
                      
					  case 0:{                                   
                               fprintf(cgiOut,"<td width=170 id=sec3>WEP</td>"\
             				   "</tr>");
					  	     }
					  	     break; 
					  case 1:{                                  
					  	       fprintf(cgiOut,"<td width=170 id=sec3>");					  	
         					   if((strcmp(r,"AES")==0)&&(cgiFormSubmitClicked("submit_secon") == cgiFormSuccess))
							   { 
         					     fprintf(cgiOut,"<input type=radio name=encry_type value=AES checked=checked>AES"\
         						 "<input type=radio name=encry_type value=TKIP>TKIP");
         					   }          					   
         					   else	 	 			      
         					   {
         					     fprintf(cgiOut,"<input type=radio name=encry_type value=AES>AES"\
         						 "<input type=radio name=encry_type value=TKIP checked=checked>TKIP");
         					   }     			      
               			       fprintf(cgiOut,"</td>"\
							   "</tr>");
					         }
					  	      break;                                     

					  case 2:{                                   
					  	       fprintf(cgiOut,"<td width=170 id=sec3>");					  	
         					   if((strcmp(r,"TKIP")==0)&&(cgiFormSubmitClicked("submit_secon") == cgiFormSuccess))
							   { 
         					     fprintf(cgiOut,"<input type=radio name=encry_type value=AES>AES"\
         						 "<input type=radio name=encry_type value=TKIP checked=checked>TKIP");
         					   }          					   
         					   else	     	 			      
         					   {
         					     fprintf(cgiOut,"<input type=radio name=encry_type value=AES checked=checked>AES"\
         						 "<input type=radio name=encry_type value=TKIP>TKIP");
         					   }     			      
               			       fprintf(cgiOut,"</td>"\
							   "</tr>");
					         }
					  	      break;                                     
					}
                  fprintf(cgiOut,"</table>"\
            "</td>"\
           "</tr>");

			*/
           /**********************************  security auth  **************************************/   
		    
              fprintf(cgiOut,"<tr style=padding-top:20px; display:none>"\
                "<td colspan=2>"\
            	"<table width=300 border=0 cellspacing=0 cellpadding=0>"\
              "<tr>"\
                "<td id=sec1 colspan=2 style=\"border-bottom:2px solid #53868b\">%s</td>",search(lsecu,"secur_auth_sec_para"));
                fprintf(cgiOut,"</tr>"\
              "<tr height=20 style=padding-top:10px>"\
                "<td width=130 id=sec2>IP:</td>"\
                "<td width=170>"\
                    "<div style=\"border-width:1;border-color:#a5acb2;border-style:solid;width:140;font-size:9pt\">"\
      			  "<input type=text name=auth_ip1 maxlength=3 class=a3 onKeyUp=\"mask(this,%s)\" onbeforepaste=mask_c()>.",search(lpublic,"ip_error"));
      			  fprintf(cgiOut,"<input type=text name=auth_ip2 maxlength=3 class=a3 onKeyUp=\"mask(this,%s)\" onbeforepaste=mask_c()>.",search(lpublic,"ip_error"));
      			  fprintf(cgiOut,"<input type=text name=auth_ip3 maxlength=3 class=a3 onKeyUp=\"mask(this,%s)\" onbeforepaste=mask_c()>.",search(lpublic,"ip_error"));
      			  fprintf(cgiOut,"<input type=text name=auth_ip4 maxlength=3 class=a3 onKeyUp=\"mask(this,%s)\" onbeforepaste=mask_c()>",search(lpublic,"ip_error"));
      			  fprintf(cgiOut,"</div>"\
                "</td>"\
              "</tr>"\
              "<tr height=20>"\
                "<td id=sec2>%s:</td>",search(lpublic,"port"));
                fprintf(cgiOut,"<td><input type=text name=auth_port size=21 maxLength=9 onkeypress=\"return event.keyCode>=48&&event.keyCode<=57\"></td>"\
              "</tr>"\
              "<tr height=20>"\
                "<td id=sec2>%s:</td>",search(lsecu,"share_secret"));
                fprintf(cgiOut,"<td><input type=text name=auth_secret size=21 maxLength=19 onkeypress=\"return event.keyCode!=32\"></td>"\
              "</tr>"\
            "</table>"\
            	"</td>"\
              "</tr>"\
            /******************************** secondary  security acct  ***************************************/              
              "<tr style=padding-top:20px>"\
                "<td colspan=2>"\
            	"<table width=300 border=0 cellspacing=0 cellpadding=0>"\
              "<tr>"\
                "<td id=sec1 colspan=2 style=\"border-bottom:2px solid #53868b\">%s</td>",search(lsecu,"secur_acct_sec_para"));
                fprintf(cgiOut,"</tr>"\
              "<tr height=20 style=padding-top:10px>"\
                "<td width=130 id=sec2>IP:</td>"\
                "<td width=170>"\
                   "<div style=\"border-width:1;border-color:#a5acb2;border-style:solid;width:140;font-size:9pt\">"\
              	 "<input type=text name=acct_ip1 maxlength=3 class=a3 onKeyUp=\"mask(this,%s)\" onbeforepaste=mask_c()>.",search(lpublic,"ip_error"));
              	 fprintf(cgiOut,"<input type=text name=acct_ip2 maxlength=3 class=a3 onKeyUp=\"mask(this,%s)\" onbeforepaste=mask_c()>.",search(lpublic,"ip_error"));
              	 fprintf(cgiOut,"<input type=text name=acct_ip3 maxlength=3 class=a3 onKeyUp=\"mask(this,%s)\" onbeforepaste=mask_c()>.",search(lpublic,"ip_error"));
              	 fprintf(cgiOut,"<input type=text name=acct_ip4 maxlength=3 class=a3 onKeyUp=\"mask(this,%s)\" onbeforepaste=mask_c()>",search(lpublic,"ip_error"));
              	 fprintf(cgiOut,"</div>"\
                "</td>"\
              "</tr>"\
              "<tr height=20>"\
                "<td id=sec2>%s:</td>",search(lpublic,"port"));
                fprintf(cgiOut,"<td><input type=text name=acct_port size=21 maxLength=9 onkeypress=\"return event.keyCode>=48&&event.keyCode<=57\"></td>"\
              "</tr>"\
              "<tr height=20>"\
                "<td id=sec2>%s:</td>",search(lsecu,"share_secret"));
                fprintf(cgiOut,"<td><input type=text name=acct_secret size=21 maxLength=19 onkeypress=\"return event.keyCode!=32\"></td>"\
                "</tr>"\
              "</table>"\
            	  "</td>"\
               "</tr>");
				
		      }
		    
		    else
		      fprintf(cgiOut,"<tr><td colspan=2>%s</td></tr>",search(lpublic,"no_security"));	
		  }
		  else
            fprintf(cgiOut,"<tr><td colspan=2>%s</td></tr>",search(lpublic,"contact_adm"));
		  /************************************  hidden  ***************************************/   
            fprintf(cgiOut,"<tr>"\
              "<td colspan=2>"\
          	"<table width=300 border=0 cellspacing=0 cellpadding=0>"\
            "<tr>"\
               "<td><input type=hidden name=encry_secon value=%s></td>",m);				  
			   fprintf(cgiOut,"<td><input type=hidden name=INSTANCE_ID value=%s></td>",select_insid);
            fprintf(cgiOut,"</tr>"\
          "</table>"\
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
if(result == 1)
{
  Free_security_head(head);
}
free_instance_parameter_list(&paraHead1);
return 0;
}


void Config_Security(instance_parameter *ins_para,char *id,char *secu_type,struct list *lpublic,struct list *lsecu)  /*id表示security id，schoice表示security type*/
{
	int sid = 0,ret = 0,flag = 1;                          /*sid表示security id，flag=0表示操作失败，flag=1表示操作成功*/
	char *endptr = NULL;   
	char ip1[4] = { 0 };
	char ip2[4] = { 0 };
	char ip3[4] = { 0 };
	char ip4[4] = { 0 };
	char secu_ip[20] = { 0 };            /*security auth|acct IP*/
	char secu_port[10] = { 0 }; 		 /*security auth|acct  port*/  
	int port = 0;
	char secu_secret[20] = { 0 };        /*security auth|acct shared secret*/  
	sid= strtoul(id,&endptr,10); 				 /*char转成int，10代表十进制*/		
	char alt[100] = { 0 };
    char max_sec_num[10] = { 0 };
	   
    /**********************security auth*************************/
    memset(secu_ip,0,sizeof(secu_ip));                                 /*security auth ip*/
    memset(ip1,0,sizeof(ip1));
    cgiFormStringNoNewlines("auth_ip1",ip1,4);	
    strncat(secu_ip,ip1,sizeof(secu_ip)-strlen(secu_ip)-1);
    strncat(secu_ip,".",sizeof(secu_ip)-strlen(secu_ip)-1);
    memset(ip2,0,sizeof(ip2));
    cgiFormStringNoNewlines("auth_ip2",ip2,4); 
    strncat(secu_ip,ip2,sizeof(secu_ip)-strlen(secu_ip)-1);	
    strncat(secu_ip,".",sizeof(secu_ip)-strlen(secu_ip)-1);
	memset(ip3,0,sizeof(ip3));
	cgiFormStringNoNewlines("auth_ip3",ip3,4); 
	strncat(secu_ip,ip3,sizeof(secu_ip)-strlen(secu_ip)-1);	
	strncat(secu_ip,".",sizeof(secu_ip)-strlen(secu_ip)-1);
	memset(ip4,0,sizeof(ip4));
	cgiFormStringNoNewlines("auth_ip4",ip4,4);	   
	strncat(secu_ip,ip4,sizeof(secu_ip)-strlen(secu_ip)-1);		 

	memset(secu_port,0,sizeof(secu_port));	                            	/*security auth port*/
	cgiFormStringNoNewlines("auth_port",secu_port,10);	
	  
	memset(secu_secret,0,sizeof(secu_secret));	                            /*security auth shared secret*/
	cgiFormStringNoNewlines("auth_secret",secu_secret,20);	

    if((strcmp(ip1,"")!=0)&&(strcmp(ip2,"")!=0)&&(strcmp(ip3,"")!=0)&&(strcmp(ip4,"")!=0))
    {
      if(strcmp(secu_port,""))
      {
      	  port=strtoul(secu_port,&endptr,10); 				 /*char转成int，10代表十进制*/
		  if(strcmp(secu_secret,"")!=0) 
		  {
		  	if(strchr(secu_secret,' ')==NULL)   /*不包含空格*/
		  	{
				ret=secondary_radius_auth(ins_para->parameter,ins_para->connection,sid,secu_ip,port,secu_secret);   /*security auth*/	  
				switch(ret)
				{
				  case SNMPD_CONNECTION_ERROR:
				  case 0:{
							ShowAlert(search(lsecu,"secur_auth_fail")); /*如果失败*/
							flag=0;
							break;
						 } 
				  case 1:break; 											/*如果成功，继续执行下一函数security_auth_acct(0)*/
				  case -1:{
							ShowAlert(search(lsecu,"secur_type_not_sup"));	/*如果security type which you choose not supported 802.1X*/
							flag=0;
							break;
						  } 
				  case -2:{
							ShowAlert(search(lsecu,"change_radius_not_per"));/*如果Change radius info not permited*/
							flag=0;
							break;
						  }
				  case -3:{
							ShowAlert(search(lsecu,"secur_be_used"));		/*如果This Security Profile be used by some Wlans,please disable these Wlans first*/
							flag=0;
							break;
						  }
				  case -4:{
							ShowAlert(search(lsecu,"main_not"));			/*如果Please use radius auth ip port shared_secret first,command failed*/
							flag=0;
							break;
						  } 
				  case -5:{
							ShowAlert(search(lpublic,"error")); 			/*如果error*/
							flag=0;
							break;
						  }
				  case -6:{
						    memset(alt,0,sizeof(alt));
						    strncpy(alt,search(lpublic,"secur_id_illegal1"),sizeof(alt)-1);
						    memset(max_sec_num,0,sizeof(max_sec_num));
						    snprintf(max_sec_num,sizeof(max_sec_num)-1,"%d",WLAN_NUM-1);
						    strncat(alt,max_sec_num,sizeof(alt)-strlen(alt)-1);
						    strncat(alt,search(lpublic,"secur_id_illegal2"),sizeof(alt)-strlen(alt)-1);
						    ShowAlert(alt);
						    flag=0;
						    break;
					      }
				  case -7:{
						    ShowAlert(search(lsecu,"radius_heart_is_on"));
						    flag=0;
						    break;
						  }
				  case -8:{
						    ShowAlert(search(lsecu,"unknown_port"));
						    flag=0;
						    break;
						  }
				}  
		  	}
			else
			{
				ShowAlert(search(lpublic,"input_para_dont_contain_spaces"));
				flag=0;
		    }	
		  }
		  else
		  {
			ShowAlert(search(lsecu,"share_secret_not_null"));
			flag=0;
		  }
      }
	  else
	  {
		ShowAlert(search(lsecu,"port_not_null"));
		flag=0;
	  }
    }
    else
    {
      ShowAlert(search(lsecu,"auth_ip_not_null"));
	  flag =0;
	 
    }
	if(flag == 0)
	{
		return;
	}
	
	/**********************security acct*************************/
	memset(secu_ip,0,sizeof(secu_ip));                                 /*security acct ip*/
	memset(ip1,0,sizeof(ip1));
	cgiFormStringNoNewlines("acct_ip1",ip1,4);	
	strncat(secu_ip,ip1,sizeof(secu_ip)-strlen(secu_ip)-1);
	strncat(secu_ip,".",sizeof(secu_ip)-strlen(secu_ip)-1);
	memset(ip2,0,sizeof(ip2));
	cgiFormStringNoNewlines("acct_ip2",ip2,4); 
	strncat(secu_ip,ip2,sizeof(secu_ip)-strlen(secu_ip)-1);	
	strncat(secu_ip,".",sizeof(secu_ip)-strlen(secu_ip)-1);
	memset(ip3,0,sizeof(ip3));
	cgiFormStringNoNewlines("acct_ip3",ip3,4); 
	strncat(secu_ip,ip3,sizeof(secu_ip)-strlen(secu_ip)-1);	
	strncat(secu_ip,".",sizeof(secu_ip)-strlen(secu_ip)-1);
	memset(ip4,0,sizeof(ip4));
	cgiFormStringNoNewlines("acct_ip4",ip4,4);	   
	strncat(secu_ip,ip4,sizeof(secu_ip)-strlen(secu_ip)-1);		  

	memset(secu_port,0,sizeof(secu_port));	                           	/*security acct port*/
	cgiFormStringNoNewlines("acct_port",secu_port,10);

	memset(secu_secret,0,sizeof(secu_secret));	                            /*security acct shared secret*/
	cgiFormStringNoNewlines("acct_secret",secu_secret,20);	

	if((strcmp(ip1,"")!=0)&&(strcmp(ip2,"")!=0)&&(strcmp(ip3,"")!=0)&&(strcmp(ip4,"")!=0))
	{
        if(strcmp(secu_port,""))
        {
        	port=strtoul(secu_port,&endptr,10); 				 /*char转成int，10代表十进制*/
			if(strcmp(secu_secret,"")!=0) 
			{
				if(strchr(secu_secret,' ')==NULL)   /*不包含空格*/
				{
					ret=secondary_radius_acct(ins_para->parameter,ins_para->connection,sid,secu_ip,port,secu_secret);	/*security acct*/		
					switch(ret)
					{
					    case SNMPD_CONNECTION_ERROR:
						case 0:{
								  ShowAlert(search(lsecu,"secur_acct_fail"));		/*如果失败*/
								  flag=0;
								  break;
							   }
						case 1:break;												/*如果成功*/
						case -1:{
								  ShowAlert(search(lsecu,"secur_type_not_sup"));	/*如果security type which you choose not supported 802.1X*/
								  flag=0;
								  break;
								}
						case -2:{
								  ShowAlert(search(lsecu,"change_radius_not_per"));/*如果Change radius info not permited*/
								  flag=0;
								  break;
								}
						case -3:{
								  ShowAlert(search(lsecu,"secur_be_used")); 	  /*如果This Security Profile be used by some Wlans,please disable these Wlans first*/
								  flag=0;
								  break;
								}
						case -4:{
								  ShowAlert(search(lsecu,"main_not"));			  /*如果Please use radius acct ip port shared_secret first,command failed*/
								  flag=0;
								  break;
								} 
						case -5:{
								  ShowAlert(search(lpublic,"error"));			  /*如果error*/
								  flag=0;
								  break;
								}
						case -6:{
								  memset(alt,0,sizeof(alt));
								  strncpy(alt,search(lpublic,"secur_id_illegal1"),sizeof(alt)-1);
								  memset(max_sec_num,0,sizeof(max_sec_num));
								  snprintf(max_sec_num,sizeof(max_sec_num)-1,"%d",WLAN_NUM-1);
								  strncat(alt,max_sec_num,sizeof(alt)-strlen(alt)-1);
								  strncat(alt,search(lpublic,"secur_id_illegal2"),sizeof(alt)-strlen(alt)-1);
								  ShowAlert(alt);
								  flag=0;
								  break;
							    }
						case -7:{
								  ShowAlert(search(lsecu,"radius_heart_is_on"));
								  flag=0;
								  break;
								}
						case -8:{
								  ShowAlert(search(lsecu,"unknown_port"));
								  flag=0;
								  break;
								}
					  }   
			  }
			  else
			  {
			    ShowAlert(search(lpublic,"input_para_dont_contain_spaces"));
			    flag=0;
			  }
			}
			else
			{
			  ShowAlert(search(lsecu,"share_secret_not_null"));
			  flag=0;
			}
        }
		else
	    {
		  ShowAlert(search(lsecu,"port_not_null"));
		  flag=0;
	    }
	}
	else
	{
	    ShowAlert(search(lsecu,"acct_ip_not_null"));
		flag=0;
	}

	if(flag==1)
      ShowAlert(search(lpublic,"oper_succ"));                /*所有操作都成功，提示操作成功*/
}


