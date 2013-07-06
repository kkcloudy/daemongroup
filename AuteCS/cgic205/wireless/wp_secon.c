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
* wp_secon.c
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
#include "dbus/asd/ASDDbusDef1.h"
#include "ws_security.h"
#include "ws_dcli_vrrp.h"
#include "ws_init_dbus.h"
#include "wcpss/wid/WID.h"
#include "dbus/wcpss/dcli_wid_wtp.h"
#include "dbus/wcpss/dcli_wid_wlan.h"
#include "ws_dcli_wlans.h"
#include "ws_dbus_list_interface.h"

#define BUF_LENZ 128
#define AE_CER_PATH "/var/run/ae_cer_path.txt"

char *secur_type[] = {   /*security type*/
	"open",
	"shared",
	"802.1x",
	"WPA_E",
	"WPA_P",
	"WPA2_E",
	"WPA2_P",
	"MD5",
	"WAPI_PSK",
	"WAPI_AUTH"
};

char *rekey_method[] = {
	"disable",
	"time_based",
	"packet_based",
	"both_based"
};

char *cer_type[2]={
	".cer",
	".p12"
};


int ShowSecurityConfigPage(char *m,char *n,char *t,char *r,char *exten,char *pn,struct list *lpublic,struct list *lsecu);    /*m代表加密字符串，n代表security type，t代表选中的security id，r代表选中的encryption type*/
void Config_Security(instance_parameter *ins_para,char *id,char *secu_type,struct list *lpublic,struct list *lsecu);

int cgiMain()
{
  char encry[BUF_LEN] = { 0 };              
  char *str = NULL;    
  char secChoice[10] = { 0 };  
  char secID[5] = { 0 };
  char enc[5] = { 0 };
  char ext[5] = { 0 };
  char pno[10] = { 0 };  
  struct list *lpublic = NULL;   /*解析public.txt文件的链表头*/
  struct list *lsecu = NULL;     /*解析security.txt文件的链表头*/  
  lpublic=get_chain_head("../htdocs/text/public.txt");
  lsecu=get_chain_head("../htdocs/text/security.txt");
  
  DcliWInit();
  ccgi_dbus_init();
  memset(secChoice,0,sizeof(secChoice));
  memset(secID,0,sizeof(secID));
  memset(enc,0,sizeof(enc));
  memset(ext,0,sizeof(ext));
  memset(encry,0,sizeof(encry));  
  memset(pno,0,sizeof(pno));  
  if(cgiFormStringNoNewlines("UN", encry, BUF_LEN)!=cgiFormNotFound )  /*首次进入该页*/
  {
    cgiFormStringNoNewlines("PN",pno,10);
    str=dcryption(encry);
    if(str==NULL)
     ShowErrorPage(search(lpublic,"ill_user"));		 /*用户非法*/
    else if(cgiFormStringNoNewlines("SecuID", secID, 5)!=cgiFormNotFound )   /*从wp_seculis.cgi进入该页*/
	  ShowSecurityConfigPage(encry,"802.1x",secID,"WEP","0",pno,lpublic,lsecu); 
	else
      ShowSecurityConfigPage(encry,"802.1x","1","WEP","0",pno,lpublic,lsecu); 
  }
  else                    
  {   
    cgiFormStringNoNewlines("page_no",pno,10);
    cgiFormStringNoNewlines("secur_id",secID,5);
    cgiFormStringNoNewlines("secur_type",secChoice,10);	
    cgiFormStringNoNewlines("encry_type",enc,5);
	cgiFormStringNoNewlines("set_exten_authen",ext,5);
	cgiFormStringNoNewlines("encry_secon",encry,BUF_LEN);	
	if(strcmp(ext,"")==0)
      strncpy(ext,"0",sizeof(ext)-1);
	str=dcryption(encry);
    if(str==NULL)
     ShowErrorPage(search(lpublic,"ill_user"));		 /*用户非法*/
    else
     ShowSecurityConfigPage(encry,secChoice,secID,enc,ext,pno,lpublic,lsecu); 
  }
  release(lpublic);  
  release(lsecu);
  destroy_ccgi_dbus();
  return 0;
}

int ShowSecurityConfigPage(char *m,char *n,char *t,char *r,char *exten,char *pn,struct list *lpublic,struct list *lsecu)
{ 
  char select_insid[10] = { 0 };
  int i = 0,sectypeChoice = 0,security_id = 0,j = 0;         /*sectypeChoice存储选择的security type值*/
  char *endptr = NULL;  
  char sID[5] = { 0 };
  struct dcli_security *head = NULL,*q = NULL;
  struct dcli_security *security = NULL;          /*存放security信息的链表头*/       
  int sec_num = 0;             /*存放security的个数*/
  int result1 = 0,result2 = 0,limit = 0;
  FILE *pp = NULL;
  char buff[BUF_LEN] = { 0 };
  char cmd[BUF_LEN] = { 0 };
  char ip1[5] = { 0 };
  char ip2[5] = { 0 };
  char ip3[5] = { 0 };
  char ip4[5] = { 0 };
  char *temp = NULL;
  char temp_name[DEFAULT_LEN] = { 0 };
  char cer_path[DEFAULT_LEN] = { 0 };
  char tempz[512] = { 0 };
  char tz[BUF_LENZ] = { 0 };
  int flag = 0;
  instance_parameter *paraHead1 = NULL,*paraHead2 = NULL;
  instance_parameter *pq = NULL;
  char temp1[10] = { 0 };
  dbus_parameter ins_para;
  cgiHeaderContentType("text/html");
  fprintf(cgiOut,"<html xmlns=\"http://www.w3.org/1999/xhtml\"><head>");
  fprintf(cgiOut,"<meta http-equiv=Content-Type content=text/html; charset=gb2312>");
  fprintf(cgiOut,"<title>%s</title>",search(lsecu,"wlan_sec"));
  fprintf(cgiOut,"<link rel=stylesheet href=/style.css type=text/css>"\
  	"<style type=text/css>"\
  	".a3{width:30;border:0; text-align:center}"\
  	"</style>"\
  	"<script type=\"text/javascript\">"\
  		"function showna(obj)"\
					"{"\
  						"if(obj.value==\"enable\")"\
  						"{"\
  							"document.all.mytr_ca.style.display = \"\";"\
						"}"\
  						"else"\
  						"{"\
  							"document.all.mytr_ca.style.display = \"none\";"\
						"}"\
					"}"\
					"</script>"\
  "</head>"\
  "<script src=/ip.js>"\
  "</script>"\
  "<script src=/instanceid_onchange.js>"\
  "</script>");
  if(strcmp(n,"open")==0)
    fprintf(cgiOut,"<body onload=\"showTR(mytr,mytr2)\">");  
  else
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
					"<td align=left id=tdleft><a href=wp_seculis.cgi?UN=%s&INSTANCE_ID=%s target=mainFrame class=top><font id=%s>%s</font></a></td>",m,select_insid,search(lpublic,"menu_san"),search(lsecu,"secur_list"));                       
                  fprintf(cgiOut,"</tr>"\
				  "<tr height=25>"\
					"<td align=left id=tdleft><a href=wp_secre.cgi?UN=%s target=mainFrame class=top><font id=%s>%s</font></a></td>",m,search(lpublic,"menu_san"),search(lsecu,"create_sec"));                       
                  fprintf(cgiOut,"</tr>"\
				  "<tr height=26>"\
                    "<td align=left id=tdleft background=/images/bottom_bg.gif style=\"border-right:0\"><font id=%s>%s</font></td>",search(lpublic,"menu_san"),search(lsecu,"config_sec"));   /*突出显示*/
                  fprintf(cgiOut,"</tr>");
					 fprintf(cgiOut,"<tr height=25>"\
					  "<td align=left id=tdleft><a href=wp_secscon.cgi?UN=%s target=mainFrame class=top><font id=%s>%s</font></a></td>",m,search(lpublic,"menu_san"),search(lsecu,"config_secon_sec"));                       
                    fprintf(cgiOut,"</tr>");

					 fprintf(cgiOut,"<tr height=25>"\
					  "<td align=left id=tdleft><a href=wp_cer_dload.cgi?UN=%s target=mainFrame class=top><font id=%s>%s</font></a></td>",m,search(lpublic,"menu_san"),search(lpublic,"cer_up"));                       
                    fprintf(cgiOut,"</tr>");
					
				  if(strcmp(n,"802.1x")==0)
				  	limit=29;
				  else if(strcmp(n,"MD5")==0)
				  	limit=24;
				  else if (strcmp(n,"WPA_E")==0)
				  	limit=30;
				  else if(strcmp(n,"WPA2_E")==0)
				  	limit=32;
				  else if((strcmp(n,"WPA_P")==0)||(strcmp(n,"WPA2_P")==0))
				  	limit=6;
				  else if(strcmp(n,"open")==0)
				  {
					  limit=4;
					  if(strcmp(r,"WEP")==0)
				  		limit+=5;
				  }
				  else if(strcmp(n,"WAPI_PSK")==0)
				  	limit=7;
				  else if(strcmp(n,"WAPI_AUTH")==0)
				  	limit=16;
				  else
				  	limit=9;
				  
				  if(((strcmp(n,"open")==0)||(strcmp(n,"shared")==0))&&(strcmp(exten,"1")==0))
					limit+=21;	
				  //if(strcmp(n,"WAPI_PSK")==0)
				  	//limit+=1;
				  //if(strcmp(n,"WAPI_AUTH")==0)
				  	//limit+=11;
                  for(i=0;i<limit;i++)
	              {
  				    fprintf(cgiOut,"<tr height=25>"\
                      "<td id=tdleft>&nbsp;</td>"\
                    "</tr>");
	              }
                fprintf(cgiOut,"</table>"\
              "</td>"\
              "<td align=left valign=top style=\"background-color:#ffffff; border-right:1px solid #707070; padding-left:30px; padding-top:10px\">"\
          "<table width=570 border=0 cellspacing=0 cellpadding=0>"\
			 "<tr height=30>"\
			  "<td width=110 id=sec2>%s ID:</td>",search(lpublic,"instance"));
			  fprintf(cgiOut,"<td align=left width=460>"\
				  "<select name=instance_id id=instance_id style=width:85px onchange=instanceid_change(this,\"wp_secon.cgi\",\"%s\")>",m);  
				  list_instance_parameter(&paraHead2, INSTANCE_STATE_WEB);	 
				  for(pq=paraHead2;(NULL != pq);pq=pq->next)
				  {					  
					  memset(temp1,0,sizeof(temp1));
					  snprintf(temp1,sizeof(temp1)-1,"%d-%d-%d",pq->parameter.slot_id,pq->parameter.local_id,pq->parameter.instance_id);					  
					   
					  if(strcmp(select_insid,temp1) == 0)
						fprintf(cgiOut,"<option value='%s' selected=selected>%s",temp1,temp1);
					  else
						fprintf(cgiOut,"<option value='%s'>%s",temp1,temp1);
				  } 		  
				  free_instance_parameter_list(&paraHead2);
				  fprintf(cgiOut,"</select>"\
			  "</td>"\
			"<tr>");		 
		  if(paraHead1)
		  {
			  result1=show_security_list(paraHead1->parameter,paraHead1->connection,&head,&sec_num);		  
		  }
		  security_id=strtoul(t,&endptr,10);				 /*char转成int，10代表十进制*/	 
		  if(paraHead1)
		  {
			  result2=show_security_one(paraHead1->parameter,paraHead1->connection,security_id,&security);
		  }
		  if(result1 == 1)	/*显示所有security的信息，head返回security信息链表的链表头*/
		  {
            if(sec_num>0)    /*如果security存在*/
            {          
              /**********************  security ID && security type  ************************************/
              fprintf(cgiOut,"<tr>"\
                "<td colspan=2><table width=300 border=0 cellspacing=0 cellpadding=0>"\
                    "<tr height=30>"\
                      "<td width=110 id=sec2>%s ID:</td>",search(lpublic,"security"));
                      fprintf(cgiOut,"<td width=190><select name=secur_id style=width:85px>");        
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
                    "<tr height=30>"\
                      "<td id=sec2>%s:</td>",search(lsecu,"secur_type"));
                      fprintf(cgiOut,"<td><select name=secur_type style=width:85px onchange=\"javascript:this.form.submit();\">");
                      for(i=0;i<10;i++)
			          if(strcmp(secur_type[i],n)==0)              /*显示上次选中的security type*/
       	                fprintf(cgiOut,"<option value=%s selected=selected>%s",secur_type[i],secur_type[i]);
			          else			  	
			            fprintf(cgiOut,"<option value=%s>%s",secur_type[i],secur_type[i]);
			          fprintf(cgiOut,"</select>"\
          	          "</td>"\
                    "</tr>"\
                    "</table>"\
               "</td>"\
            "</tr>");
		    cgiFormSelectSingle("secur_type", secur_type, 10, &sectypeChoice, 2);
            /**********************************  encryptin type  **********************************/              
            fprintf(cgiOut,"<tr style=padding-top:20px>"\
              "<td colspan=2><table width=300 border=0 cellspacing=0 cellpadding=0>"\
                  "<tr>"\
                    "<td id=sec1 colspan=2 style=\"border-bottom:2px solid #53868b\">%s %s</td>",secur_type[sectypeChoice],search(lsecu,"para"));
                  fprintf(cgiOut,"</tr>"\
                  "<tr style=padding-top:10px>"\
                    "<td width=130 id=sec2>%s:</td>",search(lsecu,"encry_type"));
				    switch(sectypeChoice)
				    {
                      case 0:{                                    /*security type=open*/
					  	       fprintf(cgiOut,"<td width=170 id=sec3>");					  	
         					   if(strcmp(r,"WEP")==0)
							   { 
         					     fprintf(cgiOut,"<input type=radio name=encry_type value=none onclick=\"showTR(mytr,mytr2);javascript:this.form.submit();\">none"\
         						 "<input type=radio name=encry_type value=WEP checked=checked onclick=\"showTR(mytr,mytr2);javascript:this.form.submit();\">WEP");
         					   }          					   
         					   else	  /*默认是none*/    			      
         					   {
         					     fprintf(cgiOut,"<input type=radio name=encry_type value=none checked=checked onclick=\"showTR(mytr,mytr2);javascript:this.form.submit();\">none"\
         						 "<input type=radio name=encry_type value=WEP onclick=\"showTR(mytr,mytr2);javascript:this.form.submit();\">WEP");
         					   }     			      
               			       fprintf(cgiOut,"</td>"\
							   "</tr>"\
							    "<tr id=mytr height=25 style=\"display:none\">"\
							    "<td id=sec2>%s:</td>",search(lsecu,"inputype"));
	             			   	fprintf(cgiOut,"<td><input type=radio name=input_type value=ascii checked>ASCII"\
	         						"<input type=radio name=input_type value=hex>HEX</td>"\
             				   "</tr>"\
             				   "<tr id=mytr2 height=25 style=\"display:none\">"\
                                 "<td id=sec2>%s:</td>",search(lsecu,"key"));
								 if((result2 == 1)&&(security)&&(security->SecurityKey))
             				       fprintf(cgiOut,"<td id=sec3><input type=text name=key size=21 onkeypress=\"return event.keyCode!=32\" value=%s></td>",security->SecurityKey);
								 else
								   fprintf(cgiOut,"<td id=sec3><input type=text name=key size=21 onkeypress=\"return event.keyCode!=32\"></td>");
             				   fprintf(cgiOut,"</tr>");
					         }
					  	     break; 
					  case 1:{                                    /*security type=shared*/
                               fprintf(cgiOut,"<td width=170 id=sec3>WEP</td>"\
							   "</tr>"\
							    "<tr height=25>"\
							    "<td id=sec2>%s:</td>",search(lsecu,"inputype"));
	             			   fprintf(cgiOut,"<td><input type=radio name=input_type value=ascii checked>ASCII"\
	         						"<input type=radio name=input_type value=hex>HEX</td>"\
             				   "</tr>"\
							   "<tr height=25>"\
                                 "<td id=sec2>%s:</td>",search(lsecu,"key"));
							     if((result2 == 1)&&(security)&&(security->SecurityKey))
             				       fprintf(cgiOut,"<td id=sec3><input type=text name=key size=21 onkeypress=\"return event.keyCode!=32\" value=%s></td>",security->SecurityKey);
								 else
								   fprintf(cgiOut,"<td id=sec3><input type=text name=key size=21 onkeypress=\"return event.keyCode!=32\"></td>");
             				   fprintf(cgiOut,"</tr>");
					  	     }           
					         break; 
					  case 2:{                                    /*security type=802.1x*/
                               fprintf(cgiOut,"<td width=170 id=sec3>WEP</td>"\
             				   "</tr>");
             				  /* "<tr height=25>"\
							    "<td id=sec2>%s:</td>",search(lsecu,"inputype"));
	             			   	fprintf(cgiOut,"<td><input type=radio name=input_type value=ascii checked>ASCII"\
	         						"<input type=radio name=input_type value=hex>HEX</td>"\
             				   "</tr>");*/
					  	     }
					  	     break; 
					  case 3:{                                    /*security type=WPA Enterprise*/
					  	       fprintf(cgiOut,"<td width=170 id=sec3>");					  	
         					   if((strcmp(r,"AES")==0)&&(cgiFormSubmitClicked("submit_secon") == cgiFormSuccess))
							   { 
         					     fprintf(cgiOut,"<input type=radio name=encry_type value=AES checked=checked>AES"\
         						 "<input type=radio name=encry_type value=TKIP>TKIP");
         						 
         					   }          					   
         					   else	  /*默认是TKIP*/    	 			      
         					   {
         					     fprintf(cgiOut,"<input type=radio name=encry_type value=AES>AES"\
         						 "<input type=radio name=encry_type value=TKIP checked=checked>TKIP");
         					   }     			      
               			       fprintf(cgiOut,"</td>"\
							   "</tr>");
							 /*  "<tr height=25>"\
							    "<td id=sec2>%s:</td>",search(lsecu,"inputype"));
	             			   	fprintf(cgiOut,"<td><input type=radio name=input_type value=ascii checked>ASCII"\
	         						"<input type=radio name=input_type value=hex>HEX</td>"\
             				   "</tr>");*/
					         }
					  	      break;                                     
					  case 4:{                                    /*security type=WPA Personal*/
					  	       fprintf(cgiOut,"<td width=170 id=sec3>");					  	
         					   if((strcmp(r,"AES")==0)&&(cgiFormSubmitClicked("submit_secon") == cgiFormSuccess))
							   { 
         					     fprintf(cgiOut,"<input type=radio name=encry_type value=AES checked=checked>AES"\
         						 "<input type=radio name=encry_type value=TKIP>TKIP");
         					   }          					   
         					   else	  /*默认是TKIP*/    	 			      
         					   {
         					     fprintf(cgiOut,"<input type=radio name=encry_type value=AES>AES"\
         						 "<input type=radio name=encry_type value=TKIP checked=checked>TKIP");
         					   }     			      
               			       fprintf(cgiOut,"</td>"\
							   "</tr>"\
							    "<tr height=25>"\
							    "<td id=sec2>%s:</td>",search(lsecu,"inputype"));
	             			   fprintf(cgiOut,"<td><input type=radio name=input_type value=ascii checked>ASCII"\
	         						"<input type=radio name=input_type value=hex>HEX</td>"\
             				   "</tr>"\
							   "<tr height=25>"\
                                 "<td id=sec2>%s:</td>",search(lsecu,"key"));
							     if((result2 == 1)&&(security)&&(security->SecurityKey))
             				       fprintf(cgiOut,"<td id=sec3><input type=text name=key size=21 onkeypress=\"return event.keyCode!=32\" value=%s></td>",security->SecurityKey);
								 else
								   fprintf(cgiOut,"<td id=sec3><input type=text name=key size=21 onkeypress=\"return event.keyCode!=32\"></td>");
							   fprintf(cgiOut,"</tr>");
					         }
					  	      break;
					  case 5:{                                    /*security type=WPA2 Enterprise*/
					  	       fprintf(cgiOut,"<td width=170 id=sec3>");					  	
         					   if((strcmp(r,"TKIP")==0)&&(cgiFormSubmitClicked("submit_secon") == cgiFormSuccess))
							   { 
         					     fprintf(cgiOut,"<input type=radio name=encry_type value=AES>AES"\
         						 "<input type=radio name=encry_type value=TKIP checked=checked>TKIP");
         					   }          					   
         					   else	  /*默认是AES*/    	 			      
         					   {
         					     fprintf(cgiOut,"<input type=radio name=encry_type value=AES checked=checked>AES"\
         						 "<input type=radio name=encry_type value=TKIP>TKIP");
         					   }     			      
               			       fprintf(cgiOut,"</td>"\
							   "</tr>");
							 /*  "<tr height=25>"\
							    "<td id=sec2>%s:</td>",search(lsecu,"inputype"));
	             			   	fprintf(cgiOut,"<td><input type=radio name=input_type value=ascii checked>ASCII"\
	         						"<input type=radio name=input_type value=hex>HEX</td>"\
             				   "</tr>");*/
					         }
					  	      break;                                     
					  case 6:{                                    /*security type=WPA2 Personal*/
					  	       fprintf(cgiOut,"<td width=170 id=sec3>");					  	
         					   if((strcmp(r,"TKIP")==0)&&(cgiFormSubmitClicked("submit_secon") == cgiFormSuccess))
							   { 
         					     fprintf(cgiOut,"<input type=radio name=encry_type value=AES>AES"\
         						 "<input type=radio name=encry_type value=TKIP checked=checked>TKIP");
         					   }          					   
         					   else	  /*默认是AES*/    	 			      
         					   {
         					     fprintf(cgiOut,"<input type=radio name=encry_type value=AES checked=checked>AES"\
         						 "<input type=radio name=encry_type value=TKIP>TKIP");
         					   }     			      
               			       fprintf(cgiOut,"</td>"\
							   "</tr>"\
							    "<tr height=25>"\
							    "<td id=sec2>%s:</td>",search(lsecu,"inputype"));
	             			   	fprintf(cgiOut,"<td><input type=radio name=input_type value=ascii checked>ASCII"\
	         						"<input type=radio name=input_type value=hex>HEX</td>"\
             				   "</tr>"\
							   "<tr height=25>"\
                                 "<td id=sec2>%s:</td>",search(lsecu,"key"));
								 if((result2 == 1)&&(security)&&(security->SecurityKey))
             				       fprintf(cgiOut,"<td id=sec3><input type=text name=key size=21 onkeypress=\"return event.keyCode!=32\" value=%s></td>",security->SecurityKey);
								 else
								   fprintf(cgiOut,"<td id=sec3><input type=text name=key size=21 onkeypress=\"return event.keyCode!=32\"></td>");
							   fprintf(cgiOut,"</tr>");
					         }
					  	      break;
					  case 7:{                                    /*security type=MD5*/
                               fprintf(cgiOut,"<td width=170 id=sec3>none</td>"\
             				   "</tr>");
					  	     }
					  	     break;
					  case 8:{                                    /*security type=WAPI_PSK*/
                               fprintf(cgiOut,"<td width=170 id=sec3>SMS4</td>"\
							   "</tr>"\
							    "<tr height=25>"\
							    "<td id=sec2>%s:</td>",search(lsecu,"inputype"));
	             			   fprintf(cgiOut,"<td><input type=radio name=input_type value=ascii checked>ASCII"\
	         						"<input type=radio name=input_type value=hex>HEX"\
	         					"</td>"\
             				   "</tr>"\
							   "<tr height=25>"\
                                 "<td id=sec2>%s:</td>",search(lsecu,"key"));
							     if((result2 == 1)&&(security)&&(security->SecurityKey))
             				       fprintf(cgiOut,"<td id=sec3><input type=text name=key size=21 onkeypress=\"return event.keyCode!=32\" value=%s></td>",security->SecurityKey);
								 else
								   fprintf(cgiOut,"<td id=sec3><input type=text name=key size=21 onkeypress=\"return event.keyCode!=32\"></td>");
             				   fprintf(cgiOut,"</tr>");
					  	     }
					  		  break;
					  case 9:{                                    /*security type=WAPI_AUTH*/
					  	       fprintf(cgiOut,"<td width=170 id=sec3>SMS4</td>"\
							   "</tr>");
					         }
					  		   break;
					}
                  fprintf(cgiOut,"</table>"\
            "</td>"\
           "</tr>");
		   /************************************  security index  ******************************/ 
		   if(((0 == sectypeChoice)&&(strcmp(r,"WEP")==0))||(1 == sectypeChoice)||(2 == sectypeChoice))
		   {
			    fprintf(cgiOut,"<tr style=padding-top:20px>"\
					"<td colspan=2><table width=300 border=0 cellspacing=0 cellpadding=0>"\
				  "<tr>"\
					"<td id=sec1 colspan=3 style=\"border-bottom:2px solid #53868b\">%s</td>","security index");
					fprintf(cgiOut,"</tr>"\
				  "<tr height=20 style=padding-top:10px>"\
					"<td width=130 id=sec2>Security Index:</td>"\
					"<td width=100><input type=text name=security_index size=11 maxLength=1 onkeypress=\"return event.keyCode>=48&&event.keyCode<=57\"></td>");
					fprintf(cgiOut,"<td width=70 align=left><font color=red>(1--4)</font></td>"\
				  "</tr>"\
				  "</table>"\
				   "</td>"\
				"</tr>");
		   }
			if((strcmp(n,"WPA_E")==0)||(strcmp(n,"WPA_P")==0)||(strcmp(n,"WPA2_E")==0)||(strcmp(n,"WPA2_P")==0))
			{
				fprintf(cgiOut,"<tr style=padding-top:20px>"\
	                "<td colspan=2><table width=300 border=0 cellspacing=0 cellpadding=0>"\
	              "<tr>"\
	                "<td id=sec1 colspan=3 style=\"border-bottom:2px solid #53868b\">%s</td>",search(lsecu,"wpa_group_rekey_period"));
	                fprintf(cgiOut,"</tr>"\
	              "<tr height=20 style=padding-top:10px>"\
	                "<td width=130 id=sec2>%s:</td>",search(lsecu,"wpa_group_rekey_period"));
					fprintf(cgiOut,"<td width=100><input type=text name=wpa_group_rekey_period size=11 maxLength=5 onkeypress=\"return event.keyCode>=48&&event.keyCode<=57\"></td>"\
					"<td width=70 align=left><font color=red>(0--86400)</font></td>"\
	              "</tr>"\
	              "</table>"\
	               "</td>"\
	            "</tr>");
			}
		   /********************************  extensible authentication  *************************/ 
			  if((strcmp(n,"open")==0)||(strcmp(n,"shared")==0))
			  {
			    fprintf(cgiOut,"<tr style=padding-top:20px>"\
                  "<td colspan=2>"\
            	  "<table width=300 border=0 cellspacing=0 cellpadding=0>"\
                "<tr>"\
                  "<td id=sec1 colspan=2 style=\"border-bottom:2px solid #53868b\">%s</td>","extensible authentication");
                fprintf(cgiOut,"</tr>"\
                "<tr height=20 style=padding-top:10px>"\
                  "<td width=160 id=sec2>Extensible Authentication:</td>"\
                  "<td width=140><select name=set_exten_authen id=set_exten_authen  style=width:100px onchange=\"javascript:this.form.submit();\">");
				  if(strcmp(exten,"1")==0)
				  {
					fprintf(cgiOut,"<option value=1 selected=selected>enable"\
  				    "<option value=0>disable");
				  }
				  else
				  {
					fprintf(cgiOut,"<option value=0 selected=selected>disable"\
  				    "<option value=1>enable");
				  }
		          fprintf(cgiOut,"</select></td>"\
                "</tr>"\
                "</table>"\
            	"</td>"\
               "</tr>");
			  }
           /**********************************  security auth  **************************************/   
		    if((strcmp(n,"WPA_P")!=0)&&(strcmp(n,"WPA2_P")!=0)&&(strcmp(n,"WAPI_PSK")!=0)&&(strcmp(n,"WAPI_AUTH")!=0))
		    {
              if((strcmp(n,"open")==0)||(strcmp(n,"shared")==0))
              {
		        if(strcmp(exten,"1")==0)
		          fprintf(cgiOut,"<tr><td colspan=2><table width=300 border=0 cellspacing=0 cellpadding=0>");
			    else
			  	  fprintf(cgiOut,"<tr style=\"display:none\"><td colspan=2><table width=300 border=0 cellspacing=0 cellpadding=0>");
              }
              fprintf(cgiOut,"<tr style=padding-top:20px>"\
                "<td colspan=2>"\
            	"<table width=300 border=0 cellspacing=0 cellpadding=0>"\
              "<tr>"\
                "<td id=sec1 colspan=2 style=\"border-bottom:2px solid #53868b\">%s</td>",search(lsecu,"secur_auth_para"));
                fprintf(cgiOut,"</tr>"\
              "<tr height=20 style=padding-top:10px>"\
                "<td width=130 id=sec2>IP:</td>"\
                "<td width=170>"\
                    "<div style=\"border-width:1;border-color:#a5acb2;border-style:solid;width:140;font-size:9pt\">");
				memset(ip1,0,sizeof(ip1));
				memset(ip2,0,sizeof(ip2));
				memset(ip3,0,sizeof(ip3));
				memset(ip4,0,sizeof(ip4));
				if((result2 == 1)&&(security)&&(security->auth.auth_ip))
				{
					if(strcmp(security->auth.auth_ip,"0.0.0.0")!=0)
					{
						j=0;
						temp = strtok(security->auth.auth_ip,".");
						while(temp != NULL)
						{
						  j++;
						  if(j==1)
							  strncpy(ip1,temp,sizeof(ip1)-1);
						  else if(j ==2 )
							  strncpy(ip2,temp,sizeof(ip2)-1);
						  else if(j==3)
							  strncpy(ip3,temp,sizeof(ip3)-1);
						  else if(j==4)
							  strncpy(ip4,temp,sizeof(ip4)-1);
						  temp = strtok(NULL,".");	  
						}				   
					}
				}
      			  fprintf(cgiOut,"<input type=text name=auth_ip1 maxlength=3 class=a3 onKeyUp=\"mask(this,%s)\" onbeforepaste=mask_c() value=%s>.",search(lpublic,"ip_error"),ip1);
      			  fprintf(cgiOut,"<input type=text name=auth_ip2 maxlength=3 class=a3 onKeyUp=\"mask(this,%s)\" onbeforepaste=mask_c() value=%s>.",search(lpublic,"ip_error"),ip2);
      			  fprintf(cgiOut,"<input type=text name=auth_ip3 maxlength=3 class=a3 onKeyUp=\"mask(this,%s)\" onbeforepaste=mask_c() value=%s>.",search(lpublic,"ip_error"),ip3);
      			  fprintf(cgiOut,"<input type=text name=auth_ip4 maxlength=3 class=a3 onKeyUp=\"mask(this,%s)\" onbeforepaste=mask_c() value=%s>",search(lpublic,"ip_error"),ip4);
      			  fprintf(cgiOut,"</div>"\
                "</td>"\
              "</tr>"\
              "<tr height=20>"\
                "<td id=sec2>%s:</td>",search(lpublic,"port"));
				if(result2 == 1)
                  fprintf(cgiOut,"<td><input type=text name=auth_port size=21 maxLength=9 onkeypress=\"return event.keyCode>=48&&event.keyCode<=57\" value=%d></td>",security->auth.auth_port);
				else
				  fprintf(cgiOut,"<td><input type=text name=auth_port size=21 maxLength=9 onkeypress=\"return event.keyCode>=48&&event.keyCode<=57\"></td>");
              fprintf(cgiOut,"</tr>"\
              "<tr height=20>"\
                "<td id=sec2>%s:</td>",search(lsecu,"share_secret"));
			    if((result2 == 1)&&(security)&&(security->auth.auth_shared_secret))
                  fprintf(cgiOut,"<td><input type=text name=auth_secret size=21 maxLength=19 onkeypress=\"return event.keyCode!=32\" value=%s></td>",security->auth.auth_shared_secret);
				else
				  fprintf(cgiOut,"<td><input type=text name=auth_secret size=21 maxLength=19 onkeypress=\"return event.keyCode!=32\"></td>");
              fprintf(cgiOut,"</tr>"\
            "</table>"\
            	"</td>"\
              "</tr>"\
            /********************************  security acct  ***************************************/              
              "<tr style=padding-top:20px>"\
                "<td colspan=2>"\
            	"<table width=300 border=0 cellspacing=0 cellpadding=0>"\
              "<tr>"\
                "<td id=sec1 colspan=2 style=\"border-bottom:2px solid #53868b\">%s</td>",search(lsecu,"secur_acct_para"));
                fprintf(cgiOut,"</tr>"\
              "<tr height=20 style=padding-top:10px>"\
                "<td width=130 id=sec2>IP:</td>"\
                "<td width=170>"\
                   "<div style=\"border-width:1;border-color:#a5acb2;border-style:solid;width:140;font-size:9pt\">");
				  memset(ip1,0,sizeof(ip1));
				  memset(ip2,0,sizeof(ip2));
				  memset(ip3,0,sizeof(ip3));
				  memset(ip4,0,sizeof(ip4));		
				  if((result2 == 1)&&(security)&&(security->acct.acct_ip))
				  {
					  if(strcmp(security->acct.acct_ip,"0.0.0.0")!=0)
					  {
						  j=0;
						  temp = strtok(security->acct.acct_ip,".");
						  while(temp != NULL)
						  {
							j++;
							if(j==1)
								strncpy(ip1,temp,sizeof(ip1)-1);
							else if(j ==2 )
								strncpy(ip2,temp,sizeof(ip2)-1);
							else if(j==3)
								strncpy(ip3,temp,sizeof(ip3)-1);
							else if(j==4)
								strncpy(ip4,temp,sizeof(ip4)-1);
							temp = strtok(NULL,".");	
						  } 	
					  }
				  }
              	 fprintf(cgiOut,"<input type=text name=acct_ip1 maxlength=3 class=a3 onKeyUp=\"mask(this,%s)\" onbeforepaste=mask_c() value=%s>.",search(lpublic,"ip_error"),ip1);
              	 fprintf(cgiOut,"<input type=text name=acct_ip2 maxlength=3 class=a3 onKeyUp=\"mask(this,%s)\" onbeforepaste=mask_c() value=%s>.",search(lpublic,"ip_error"),ip2);
              	 fprintf(cgiOut,"<input type=text name=acct_ip3 maxlength=3 class=a3 onKeyUp=\"mask(this,%s)\" onbeforepaste=mask_c() value=%s>.",search(lpublic,"ip_error"),ip3);
              	 fprintf(cgiOut,"<input type=text name=acct_ip4 maxlength=3 class=a3 onKeyUp=\"mask(this,%s)\" onbeforepaste=mask_c() value=%s>",search(lpublic,"ip_error"),ip4);
              	 fprintf(cgiOut,"</div>"\
                "</td>"\
              "</tr>"\
              "<tr height=20>"\
                "<td id=sec2>%s:</td>",search(lpublic,"port"));
				if(result2 == 1)
                  fprintf(cgiOut,"<td><input type=text name=acct_port size=21 maxLength=9 onkeypress=\"return event.keyCode>=48&&event.keyCode<=57\" value=%d></td>",security->acct.acct_port);
				else
				  fprintf(cgiOut,"<td><input type=text name=acct_port size=21 maxLength=9 onkeypress=\"return event.keyCode>=48&&event.keyCode<=57\"></td>");
              fprintf(cgiOut,"</tr>"\
              "<tr height=20>"\
                "<td id=sec2>%s:</td>",search(lsecu,"share_secret"));
			    if((result2 == 1)&&(security)&&(security->acct.acct_shared_secret))
                  fprintf(cgiOut,"<td><input type=text name=acct_secret size=21 maxLength=19 onkeypress=\"return event.keyCode!=32\" value=%s></td>",security->acct.acct_shared_secret);
				else
				  fprintf(cgiOut,"<td><input type=text name=acct_secret size=21 maxLength=19 onkeypress=\"return event.keyCode!=32\"></td>");
                fprintf(cgiOut,"</tr>"\
              "</table>"\
            	  "</td>"\
               "</tr>");
			  /********************************  security host ip  ***************************************/   
			fprintf(cgiOut,"<tr style=padding-top:20px>"\
                "<td colspan=2>"\
            	"<table width=300 border=0 cellspacing=0 cellpadding=0>"\
              "<tr>"\
                "<td id=sec1 colspan=2 style=\"border-bottom:2px solid #53868b\">%s</td>","host ip");
                fprintf(cgiOut,"</tr>"\
              "<tr height=20 style=padding-top:10px>"\
                "<td width=130 id=sec2>Host IP:</td>"\
                "<td width=170>"\
                   "<div style=\"border-width:1;border-color:#a5acb2;border-style:solid;width:140;font-size:9pt\">");
				  memset(ip1,0,sizeof(ip1));
				  memset(ip2,0,sizeof(ip2));
				  memset(ip3,0,sizeof(ip3));
				  memset(ip4,0,sizeof(ip4));	
				  if((result2 == 1)&&(security)&&(security->host_ip))
				  {
					  if(strcmp(security->host_ip,"0.0.0.0")!=0)
					  {
						  j=0;
						  temp = strtok(security->host_ip,".");
						  while(temp != NULL)
						  {
							j++;
							if(j==1)
								strncpy(ip1,temp,sizeof(ip1)-1);
							else if(j ==2 )
								strncpy(ip2,temp,sizeof(ip2)-1);
							else if(j==3)
								strncpy(ip3,temp,sizeof(ip3)-1);
							else if(j==4)
								strncpy(ip4,temp,sizeof(ip4)-1);
							temp = strtok(NULL,".");	
						  } 
					  }
				  }
              	 fprintf(cgiOut,"<input type=text name=host_ip1 maxlength=3 class=a3 onKeyUp=\"mask(this,%s)\" onbeforepaste=mask_c() value=%s>.",search(lpublic,"ip_error"),ip1);
              	 fprintf(cgiOut,"<input type=text name=host_ip2 maxlength=3 class=a3 onKeyUp=\"mask(this,%s)\" onbeforepaste=mask_c() value=%s>.",search(lpublic,"ip_error"),ip2);
              	 fprintf(cgiOut,"<input type=text name=host_ip3 maxlength=3 class=a3 onKeyUp=\"mask(this,%s)\" onbeforepaste=mask_c() value=%s>.",search(lpublic,"ip_error"),ip3);
              	 fprintf(cgiOut,"<input type=text name=host_ip4 maxlength=3 class=a3 onKeyUp=\"mask(this,%s)\" onbeforepaste=mask_c() value=%s>",search(lpublic,"ip_error"),ip4);
              	 fprintf(cgiOut,"</div>"\
                "</td>"\
              "</tr>"\
              "</table>"\
            	  "</td>"\
               "</tr>");
			  if((strcmp(n,"open")==0)||(strcmp(n,"shared")==0))
			   fprintf(cgiOut,"</table></td></tr>");
		      }
			  /**********************  connect with (wired|wireless) radius server  *******************/ 
			if((strcmp(n,"WPA_P")!=0)&&(strcmp(n,"WPA2_P")!=0)&&(strcmp(n,"WAPI_PSK")!=0)&&(strcmp(n,"WAPI_AUTH")!=0))
			{
			  if((strcmp(n,"open")==0)||(strcmp(n,"shared")==0))
              {
		        if(strcmp(exten,"1")==0)
		          fprintf(cgiOut,"<tr><td colspan=2><table width=300 border=0 cellspacing=0 cellpadding=0>");
			    else
			  	  fprintf(cgiOut,"<tr style=\"display:none\"><td colspan=2><table width=300 border=0 cellspacing=0 cellpadding=0>");
              }
			  fprintf(cgiOut,"<tr style=padding-top:20px>"\
                "<td colspan=2>"\
            	"<table width=300 border=0 cellspacing=0 cellpadding=0>"\
              "<tr>"\
                "<td id=sec1 colspan=2 style=\"border-bottom:2px solid #53868b\">%s</td>","radius server");
                fprintf(cgiOut,"</tr>"\
              "<tr height=20 style=padding-top:10px>"\
                "<td width=130 id=sec2>Radius Server:</td>"\
                "<td width=170><select name=set_radius_server id=set_radius_server style=width:100px>");
				if((result2 == 1)&&(security->wired_radius==1))
				{
				  fprintf(cgiOut,"<option value=1>wired"\
  				  "<option value=0>wireless");
				}
				else
				{
				  fprintf(cgiOut,"<option value=0>wireless"\
  				  "<option value=1>wired");
				}
		          fprintf(cgiOut,"</select></td>"\
              "</tr>"\
              "</table>"\
            	  "</td>"\
               "</tr>");
			  /*****************************  set  acct interim interval  ****************************/ 
			  fprintf(cgiOut,"<tr style=padding-top:20px>"\
                "<td colspan=2>"\
            	"<table width=300 border=0 cellspacing=0 cellpadding=0>"\
              "<tr>"\
                "<td id=sec1 colspan=3 style=\"border-bottom:2px solid #53868b\">%s</td>",search(lsecu,"acct_interim_interval"));
                fprintf(cgiOut,"</tr>"\
              "<tr height=20 style=padding-top:10px>"\
                "<td width=130 id=sec2>%s:</td>",search(lsecu,"acct_interim_interval"));
                fprintf(cgiOut,"<td width=100><input type=text name=acct_interim_interval size=11 maxLength=5 onkeypress=\"return event.keyCode>=48&&event.keyCode<=57\"></td>"\
				"<td width=70 align=left><font color=red>(0--32767)s</font></td>"\
              "</tr>"\
              "</table>"\
            	  "</td>"\
               "</tr>");
			  /*****************************  set eap reauth period  ****************************/ 
			  fprintf(cgiOut,"<tr style=padding-top:20px>"\
                "<td colspan=2>"\
            	"<table width=300 border=0 cellspacing=0 cellpadding=0>"\
              "<tr>"\
                "<td id=sec1 colspan=3 style=\"border-bottom:2px solid #53868b\">%s</td>",search(lsecu,"eap_time"));
                fprintf(cgiOut,"</tr>"\
              "<tr height=20 style=padding-top:10px>"\
                "<td width=130 id=sec2>%s:</td>",search(lsecu,"eap_time"));
                fprintf(cgiOut,"<td width=100><input type=text name=eap size=11 maxLength=5 onkeypress=\"return event.keyCode>=48&&event.keyCode<=57\"></td>"\
				"<td width=70 align=left><font color=red>(0--32767)s</font></td>"\
              "</tr>"\
              "</table>"\
            	  "</td>"\
               "</tr>");
			  /*****************************  set quite period  ****************************/ 
			  fprintf(cgiOut,"<tr style=padding-top:20px>"\
                "<td colspan=2>"\
            	"<table width=300 border=0 cellspacing=0 cellpadding=0>"\
              "<tr>"\
                "<td id=sec1 colspan=3 style=\"border-bottom:2px solid #53868b\">%s</td>",search(lsecu,"1x_quiet_period"));
                fprintf(cgiOut,"</tr>"\
              "<tr height=20 style=padding-top:10px>"\
                "<td width=130 id=sec2>%s:</td>",search(lsecu,"1x_quiet_period"));
                fprintf(cgiOut,"<td width=100><input type=text name=quiet_period size=11 maxLength=5 onkeypress=\"return event.keyCode>=48&&event.keyCode<=57\"></td>"\
				"<td width=70 align=left><font color=red>(0--65535)s</font></td>"\
              "</tr>"\
              "</table>"\
            	  "</td>"\
               "</tr>");
			  if((strcmp(n,"open")==0)||(strcmp(n,"shared")==0))
			   fprintf(cgiOut,"</table></td></tr>");
			 }
			 /*****************************  set asd rdc  ****************************/ 
			 if((strcmp(n,"802.1x")==0)||(strcmp(n,"WPA_E")==0)||(strcmp(n,"WPA2_E")==0))
			 {
				fprintf(cgiOut,"<tr style=padding-top:20px>"\
                "<td colspan=2>"\
                "<table width=300 border=0 cellspacing=0 cellpadding=0>"\
				"<tr>"\
				  "<td id=sec1 colspan=2 style=\"border-bottom:2px solid #53868b\">%s</td>",search(lsecu,"asd_rdc"));
				  fprintf(cgiOut,"</tr>"\
				"<tr height=20 style=padding-top:10px>"\
				  "<td width=130 id=sec2>%s:</td>",search(lsecu,"asd_rdc"));
				  fprintf(cgiOut,"<td width=170><select name=asd_rdc_slot id=asd_rdc_slot style=width:40px>"\
				  		  "<option value=>");
					  for(i=1; i<SLOT_MAX_NUM+1; i++)
					  {
						  fprintf(cgiOut,"<option value=%d>%d",i,i);
					  }
				  fprintf(cgiOut,"</select>-"\
				  "<select name=asd_rdc_ins id=asd_rdc_ins style=width:40px>"\
				  		  "<option value=>");
					  for(i=1; i<INSTANCE_NUM+1; i++)
					  {
						  fprintf(cgiOut,"<option value=%d>%d",i,i);
					  }
				  fprintf(cgiOut,"</select></td>"\
				"</tr>"\
				"</table>"\
            	  "</td>"\
                "</tr>");
			 }
			 /*****************************  config wapi as ****************************/ 
			 if(strcmp(n,"WAPI_AUTH")==0)
			 {
			  fprintf(cgiOut,"<tr style=padding-top:20px>"\
                "<td colspan=2>"\
            	"<table width=300 border=0 cellspacing=0 cellpadding=0>"\
              "<tr>"\
                "<td id=sec1 colspan=2 style=\"border-bottom:2px solid #53868b\">WAPI%s%s</td>",search(lsecu,"wapi_as"),search(lsecu,"para"));
                fprintf(cgiOut,"</tr>"\
              "<tr height=20 style=padding-top:10px>"\
                "<td width=130 id=sec2>IP:</td>"\
                "<td width=170>"\
                "<div style=\"border-width:1;border-color:#a5acb2;border-style:solid;width:140;font-size:9pt\">");
				  memset(ip1,0,sizeof(ip1));
				  memset(ip2,0,sizeof(ip2));
				  memset(ip3,0,sizeof(ip3));
				  memset(ip4,0,sizeof(ip4));
				  if((result2 == 1)&&(security)&&(security->wapi_as.as_ip))
				  {
					  if(strcmp(security->wapi_as.as_ip,"0.0.0.0")!=0)
					  {
						  j=0;
						  temp = strtok(security->wapi_as.as_ip,".");
						  while(temp != NULL)
						  {
							j++;
							if(j==1)
								strncpy(ip1,temp,sizeof(ip1)-1);
							else if(j ==2 )
								strncpy(ip2,temp,sizeof(ip2)-1);
							else if(j==3)
								strncpy(ip3,temp,sizeof(ip3)-1);
							else if(j==4)
								strncpy(ip4,temp,sizeof(ip4)-1);
							temp = strtok(NULL,".");	
						  }
					  }
				  }
              	 fprintf(cgiOut,"<input type=text name=wapi_as_ip1 maxlength=3 class=a3 onKeyUp=\"mask(this,%s)\" onbeforepaste=mask_c() value=%s>.",search(lpublic,"ip_error"),ip1);
              	 fprintf(cgiOut,"<input type=text name=wapi_as_ip2 maxlength=3 class=a3 onKeyUp=\"mask(this,%s)\" onbeforepaste=mask_c() value=%s>.",search(lpublic,"ip_error"),ip2);
              	 fprintf(cgiOut,"<input type=text name=wapi_as_ip3 maxlength=3 class=a3 onKeyUp=\"mask(this,%s)\" onbeforepaste=mask_c() value=%s>.",search(lpublic,"ip_error"),ip3);
              	 fprintf(cgiOut,"<input type=text name=wapi_as_ip4 maxlength=3 class=a3 onKeyUp=\"mask(this,%s)\" onbeforepaste=mask_c() value=%s>",search(lpublic,"ip_error"),ip4);
              	 fprintf(cgiOut,"</div>"\
                "</td>"\
              "</tr>"\
              "<tr height=20 style=padding-top:10px>"\
                "<td id=sec2>%s:</td>",search(lpublic,"type"));
                fprintf(cgiOut,"<td><select name=as_type id=as_type style=width:100px>");
				if((result2 == 1)&&(security->wapi_as.certification_type==WAPI_GBW))
				{
					fprintf(cgiOut,"<option value=X.509>X.509"\
					"<option value=GBW selected=selected>GBW");
				}
				else
				{
					fprintf(cgiOut,"<option value=X.509 selected=selected>X.509"\
					"<option value=GBW>GBW");
				}				
		        fprintf(cgiOut,"</select></td>"\
              "</tr>"\
              "<tr height=20 style=padding-top:10px>"\
                "<td id=sec2>%s:</td>",search(lsecu,"cer_path"));
                fprintf(cgiOut,"<td><select name=as_path id=as_path style=width:100px>"\
				  "<option value=>");
				  memset(cmd,0,sizeof(cmd));
			      snprintf(cmd,sizeof(cmd)-1,"ls  /mnt/wtp |grep -i '.cer$' ");   
			      pp=popen(cmd,"r"); 
				  if(pp!=NULL)
				  {
				  	  memset(buff,0,sizeof(buff));
					  fgets( buff, sizeof(buff), pp );
					  do
					  {
					  	  if(strcmp(buff,"")!=0)
					  	  {
					  	  	memset(temp_name,0,sizeof(temp_name));
							strncpy(temp_name,"/mnt/wtp/",sizeof(temp_name)-1);
							strncat(temp_name,buff,sizeof(temp_name)-strlen(temp_name)-1);
							memset(cer_path,0,sizeof(cer_path));
							memcpy(cer_path,temp_name,strlen(temp_name)-1);/*fgets()函数以NULL结尾，因此长度减1才是实际长度*/
					  	  	if((result2 == 1)&&(security)&&(security->wapi_as.certification_path)&&(strcmp(cer_path,security->wapi_as.certification_path)==0))
						  		fprintf(cgiOut,"<option value=%s selected=selected>%s",buff,buff);
							else
								fprintf(cgiOut,"<option value=%s>%s",buff,buff);
					  	  }
						  else
						  	fprintf(cgiOut,"<option value=>");
						  
						  memset(buff,0,sizeof(buff));
						  fgets( buff, sizeof(buff), pp ); 						  	
					  }while(!feof(pp));
					  pclose(pp); 
				  }
				fprintf(cgiOut,"</select></td>"\
              "</tr>"\
              "</table>"\
            	  "</td>"\
               "</tr>");
			  /*****************************  config wapi ae ****************************/ 
			  fprintf(cgiOut,"<tr style=padding-top:20px>"\
                "<td colspan=2>"\
            	"<table width=300 border=0 cellspacing=0 cellpadding=0>"\
              "<tr>"\
                "<td id=sec1 colspan=2 style=\"border-bottom:2px solid #53868b\">WAPI%s%s</td>",search(lsecu,"wapi_ae"),search(lsecu,"para"));
                fprintf(cgiOut,"</tr>"\
              "<tr height=20 style=padding-top:10px>"\
                "<td width=130 id=sec2>%s:</td>",search(lsecu,"cer_path"));
                fprintf(cgiOut,"<td width=170><select name=ae_path id=ae_path style=width:135px onchange=\"show_ae_passwd_tr(this)\">"\
				  "<option value=>");
				  memset(tempz,0,sizeof(tempz));
				  for(i=0;i<2;i++)
				  {
				    memset(tz,0,sizeof(tz));
					snprintf(tz,sizeof(tz)-1,"ls  /mnt/wtp |grep -i '%s$'  >> %s\n",cer_type[i],AE_CER_PATH);
					strncat(tempz,tz,sizeof(tempz)-strlen(tempz)-1);
				  }
				  system(tempz);

				  memset(cmd,0,sizeof(cmd));
				  snprintf(cmd,sizeof(cmd)-1,"cat %s",AE_CER_PATH);
				  pp=popen(cmd,"r");  
				  if(pp!=NULL)
				  {
				  	memset(buff,0,sizeof(buff));
					fgets( buff, sizeof(buff), pp );	
					do
					{										   
						if(strcmp(buff,"")!=0)
						{
							memset(temp_name,0,sizeof(temp_name));
							strncpy(temp_name,"/mnt/wtp/",sizeof(temp_name)-1);
							strncat(temp_name,buff,sizeof(temp_name)-strlen(temp_name)-1);
							memset(cer_path,0,sizeof(cer_path));
							memcpy(cer_path,temp_name,strlen(temp_name)-1);/*fgets()函数以NULL结尾，因此长度减1才是实际长度*/
							if((result2 == 1)&&(security)&&(security->wapi_as.ae_cert_path)&&(strcmp(cer_path,security->wapi_as.ae_cert_path)==0))
						  		fprintf(cgiOut,"<option value=%s selected=selected>%s",buff,buff);
							else
								fprintf(cgiOut,"<option value=%s>%s",buff,buff);			
						}
						else
						{  
							fprintf(cgiOut,"<option value=>");
						}
						fgets( buff, sizeof(buff), pp ); 	
					}while( !feof(pp) ); 	
					pclose(pp); 
				  }
									
				  memset(cmd,0,sizeof(cmd));
				  snprintf(cmd,sizeof(cmd)-1,"sudo rm %s",AE_CER_PATH);
				  system(cmd);
				fprintf(cgiOut,"</select></td>"\
              "</tr>"\
			  "<tr id=ae_passwd_tr height=20 style=\"padding-top:10px; display:none\">"\
              	"<td id=sec2>%s:</td>",search(lsecu,"key"));
                fprintf(cgiOut,"<td><input type=text name=ae_passwd size=21 onkeypress=\"return event.keyCode!=32\"></td>"\
			  "</tr>"
              "</table>"\
            	  "</td>"\
               "</tr>");
			  /*****************************  config wapi multi cert switch****************************/ 
			  fprintf(cgiOut,"<tr style=padding-top:20px>"\
                "<td colspan=2>"\
            	"<table width=300 border=0 cellspacing=0 cellpadding=0>"\
              "<tr>"\
                "<td id=sec1 colspan=2 style=\"border-bottom:2px solid #53868b\">WAPI%s%s</td>",search(lsecu,"wapi_multi_cert"),search(lsecu,"para"));
                fprintf(cgiOut,"</tr>"\
              "<tr height=20 style=padding-top:10px>"\
                "<td width=130 id=sec2>%s:</td>",search(lpublic,"switch"));
                fprintf(cgiOut,"<td><select name=multi_cert_switch id=multi_cert_switch style=width:100px onchange=\"showna(this)\">");
				if((result2 == 1)&&(security->wapi_as.multi_cert==1))
				{
					fprintf(cgiOut,"<option value=enable selected=selected>enable"\
					"<option value=disable>disable");
				}
				else
				{
					fprintf(cgiOut,"<option value=enable>enable"\
					"<option value=disable selected=selected>disable");
				}				
		        fprintf(cgiOut,"</select></td>"\
              "</tr>"\
              "</table>"\
            	  "</td>"\
               "</tr>");
			  /*****************************  config wapi ca ****************************/ 
			  if((result2 == 1)&&(security->wapi_as.multi_cert==1))
			    fprintf(cgiOut,"<tr style=\"padding-top:20px\" id=mytr_ca>");
			  else
			  	fprintf(cgiOut,"<tr style=\"padding-top:20px; display:none\" id=mytr_ca style>");
                fprintf(cgiOut,"<td colspan=2>"\
            	"<table width=300 border=0 cellspacing=0 cellpadding=0>"\
              "<tr>"\
                "<td id=sec1 colspan=2 style=\"border-bottom:2px solid #53868b\">WAPI%s%s</td>",search(lsecu,"wapi_ca"),search(lsecu,"para"));
                fprintf(cgiOut,"</tr>"\
              "<tr height=20 style=padding-top:10px>"\
                "<td width=130 id=sec2>%s:</td>",search(lsecu,"cer_path"));
                fprintf(cgiOut,"<td width=170><select name=ca_path id=ca_path style=width:100px>"\
				  "<option value=>");
				  memset(cmd,0,sizeof(cmd));
			      snprintf(cmd,sizeof(cmd)-1,"ls  /mnt/wtp |grep -i '.cer' ");   
			      pp=popen(cmd,"r"); 
				  if(pp!=NULL)
				  {
				  	  memset(buff,0,sizeof(buff));
					  fgets( buff, sizeof(buff), pp );
					  do
					  {
					  	  if(strcmp(buff,"")!=0)
					  	  {
					  	  	memset(temp_name,0,sizeof(temp_name));
							strncpy(temp_name,"/mnt/wtp/",sizeof(temp_name)-1);
							strncat(temp_name,buff,sizeof(temp_name)-strlen(temp_name)-1);
							memset(cer_path,0,sizeof(cer_path));
							memcpy(cer_path,temp_name,strlen(temp_name)-1);/*fgets()函数以NULL结尾，因此长度减1才是实际长度*/
					  	  	if((result2 == 1)&&(security)&&(security->wapi_as.ca_cert_path)&&(strcmp(cer_path,security->wapi_as.ca_cert_path)==0))
						  		fprintf(cgiOut,"<option value=%s selected=selected>%s",buff,buff);
							else
								fprintf(cgiOut,"<option value=%s>%s",buff,buff);
					  	  }
						  else
						  	fprintf(cgiOut,"<option value=>");
						  
						  memset(buff,0,sizeof(buff));
						  fgets( buff, sizeof(buff), pp ); 						  	
					  }while(!feof(pp));
					  pclose(pp); 
				  }
				fprintf(cgiOut,"</select></td>"\
              "</tr>"\
              "</table>"\
            	  "</td>"\
               "</tr>");
			 }
			 /*****************************  config wapi rekey para ****************************/ 
			 if((strcmp(n,"WAPI_PSK")==0)||(strcmp(n,"WAPI_AUTH")==0))
			 {
			 	if((result2 == 1)&&(security->SecurityType==WAPI_AUTH||security->SecurityType==WAPI_PSK))
					flag = 1;
			 	fprintf(cgiOut,"<tr style=padding-top:20px>"\
                "<td colspan=2>"\
            	"<table width=570 border=0 cellspacing=0 cellpadding=0>"\
              "<tr>"\
                "<td id=sec1 colspan=5 style=\"border-bottom:2px solid #53868b\">WAPI%s%s</td>",search(lsecu,"rekey"),search(lsecu,"para"));
                fprintf(cgiOut,"</tr>"\
              "<tr height=20 style=padding-top:10px>"\
                "<td id=sec2>%s:</td>",search(lsecu,"rekey_method"));
                fprintf(cgiOut,"<td><select name=method_uorm id=method_uorm style=width:100px>");
				  if(flag == 0)
				  {
					  fprintf(cgiOut,"<option value=\"\" selected=selected>"\
					  "<option value=unicast>unicast"\
					  "<option value=multicast>multicast");
				  }
				  else
				  {
					  fprintf(cgiOut,"<option value=>"\
					  "<option value=unicast selected=selected>unicast"\
					  "<option value=multicast>multicast");
				  }
				fprintf(cgiOut,"</select></td>"\
				"<td colspan=3><select name=rekey_method id=rekey_method style=width:100px>");
				  if(flag == 0)
				  {
					  fprintf(cgiOut,"<option value=\"\" selected=selected>"\
					  "<option value=disable>disable"\
					  "<option value=time_based>time_based"\
					  "<option value=packet_based>packet_based"\
					  "<option value=both_based>both_based");
				  }
				  else
				  {
				  	  for(i=0;i<4;i++)
				  	  {
				  	  	if((result2 == 1)&&(security->wapi_ucast_rekey_method == i))
				  	  	  fprintf(cgiOut,"<option value=\"%s\" selected=selected>%s",rekey_method[i],rekey_method[i]);
						else
						  fprintf(cgiOut,"<option value=\"%s\">%s",rekey_method[i],rekey_method[i]);
				  	  }
				  }
				fprintf(cgiOut,"</select></td>"\
              "</tr>"\
              "<tr height=20 style=padding-top:10px>"\
                "<td width=130 id=sec2>%s:</td>",search(lsecu,"rekey_para"));
                fprintf(cgiOut,"<td width=130><select name=para_uorm id=para_uorm style=width:100px>");
				  if(flag == 0)
				  {
					  fprintf(cgiOut,"<option value=\"\" selected=selected>"\
					  "<option value=unicast>unicast"\
					  "<option value=multicast>multicast");
				  }
				  else
				  {
					  fprintf(cgiOut,"<option value=\"\">"\
					  "<option value=unicast selected=selected>unicast"\
					  "<option value=multicast>multicast");
				  }
				fprintf(cgiOut,"</select></td>"\
				"<td width=130><select name=rekey_torp id=rekey_torp style=width:100px>");
				  if((flag == 0)||((flag == 1)&&(result2 == 1)&&(security->wapi_ucast_rekey_method == 0)))
				  {
					  fprintf(cgiOut,"<option value=\"\" selected=selected>"\
					  "<option value=time>time"\
					  "<option value=packet>packet");
				  }
				  else
				  {
					  if((result2 == 1)&&((security->wapi_ucast_rekey_method == 1)||(security->wapi_ucast_rekey_method == 3)))
					  {
						  fprintf(cgiOut,"<option value=\"\">"\
						  "<option value=time selected=selected>time"\
						  "<option value=packet>packet");
					  }
					  else if((result2 == 1)&&(security->wapi_ucast_rekey_method == 2))
					  {
						  fprintf(cgiOut,"<option value=\"\">"\
						  "<option value=time>time"\
						  "<option value=packet selected=selected>packet");
					  }
				  }
				fprintf(cgiOut,"</select></td>"\
				"<td width=80>");
				  if((flag == 0)||((flag == 1)&&(result2 == 1)&&(security->wapi_ucast_rekey_method == 0)))
				    fprintf(cgiOut,"<input type=text name=rekey_para size=11 maxLength=9 onkeypress=\"return event.keyCode>=48&&event.keyCode<=57&&event.keyCode!=32\" value=\"\">");
				  else
				  {
				  	if((result2 == 1)&&((security->wapi_ucast_rekey_method == 1)||(security->wapi_ucast_rekey_method == 3)))
					  fprintf(cgiOut,"<input type=text name=rekey_para size=11 maxLength=9 onkeypress=\"return event.keyCode>=48&&event.keyCode<=57&&event.keyCode!=32\" value=%d>",security->wapi_ucast_rekey_para_t);
					else if((result2 == 1)&&(security->wapi_ucast_rekey_method == 2))
					  fprintf(cgiOut,"<input type=text name=rekey_para size=11 maxLength=9 onkeypress=\"return event.keyCode>=48&&event.keyCode<=57&&event.keyCode!=32\" value=%d>",security->wapi_ucast_rekey_para_p);
				  }
				fprintf(cgiOut,"</td>"\
				"<td width=100 align=left><font color=red>(0--400000000)</font></td>"\
              "</tr>"\
              "</table>"\
            	  "</td>"\
               "</tr>");
			 }
			 /***********************config pre_authentication**************************/
			 if(strcmp(n,"WPA2_E")==0)
			 {
			  fprintf(cgiOut,"<tr style=padding-top:20px>"\
                "<td colspan=2>"\
            	"<table width=300 border=0 cellspacing=0 cellpadding=0>"\
              "<tr>"\
                "<td id=sec1 colspan=3 style=\"border-bottom:2px solid #53868b\">%s</td>",search(lsecu,"pre_auth"));
                fprintf(cgiOut,"</tr>"\
              "<tr height=20 style=padding-top:10px>"\
                "<td width=130 id=sec2>%s:</td>",search(lsecu,"pre_auth"));
                fprintf(cgiOut,"<td width=170>"\
					"<select name=set_radius_server id=set_radius_server style=width:100px>");
				  	fprintf(cgiOut,"<option value=>"\
					"<option value=disable>disable"\
  				  	"<option value=enable>enable"\
                "</td>"\
              "</tr>"\
              "</table>"\
            	  "</td>"\
               "</tr>");
			 }
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
		  	   fprintf(cgiOut,"<td><input type=hidden name=page_no value=%s></td>",pn);
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
if(result1 == 1)
{
  Free_security_head(head);
}
if(result2 == 2)
{
  Free_security_one(security);
}
free_instance_parameter_list(&paraHead1);
return 0;
}


void Config_Security(instance_parameter *ins_para,char *id,char *secu_type,struct list *lpublic,struct list *lsecu)
{
	int sid = 0,ret = 0,flag = 1,flag1 = 1,flag2 = 1,flag3 = 1,flag4 = 1,flag5 = 1,flag6 = 1;  /*sid表示security id，flag=0表示操作失败，flag=1表示操作成功*/
	char *endptr = NULL;   
	char ins_id[5] = { 0 };
	char encr_type[20] = { 0 };          /*encryption type*/
	char inputype[20] = { 0 };
	char ip1[4] = { 0 };
	char ip2[4] = { 0 };
	char ip3[4] = { 0 };
	char ip4[4] = { 0 };
	char secu_ip[20] = { 0 };            /*security auth|acct IP*/
	char host_ip[20] = { 0 };
	char secu_port[10] = { 0 }; 		 /*security auth|acct  port*/  
	int port = 0;
	char secu_secret[20] = { 0 };        /*security auth|acct shared secret*/  
	char secu_key[70] = { 0 };           /*security key*/
	char exten_type[5] = { 0 };          /*extensible authentication type*/
	char security_index[5] = { 0 };				 /*security index*/
	char wpa_group_rekey_period[10] = { 0 };
	char alt[100] = { 0 };
    char max_sec_num[10] = { 0 };
	char radius_type[5] = { 0 };         /*radius server type*/
	int exType = 0,raType = 0;
	char acct_interim[10] = { 0 };       /*acct interim interval*/
	char eap[20] = { 0 };               /*eap reauth period*/
	char quiet_period[10] = { 0 };      /*quiet period*/
	char asd_rdc_slot[5] = { 0 };
	char asd_rdc_ins[5] = { 0 };
	int acct_Time = 0;
	long int eap_time = 0;
	char as_ip[20] = { 0 };
	char cer_type[10] = { 0 };
	char as_cer_path[DEFAULT_LEN] = { 0 };
	char ae_cer_path[DEFAULT_LEN] = { 0 };
	char ae_cer_passwd[DEFAULT_LEN] = { 0 };	
	char *tmpStr=NULL; 
	char temp[10] = { 0 };
	int t = 0;
	char multi_cert_switch[10] = { 0 };
	char ca_cer_path[DEFAULT_LEN] = { 0 };
	char cer_name[DEFAULT_LEN+15] = { 0 };
	char pre_auth[10] = { 0 };
	char uorm[10] = { 0 };
	char method[15] = { 0 };
	char torp[10] = { 0 };
	char para[10] = { 0 };
	memset(ins_id,0,sizeof(ins_id));
	sid=strtoul(id,&endptr,10); 				 /*char转成int，10代表十进制*/		
	memset(encr_type,0,sizeof(encr_type));
	memset(inputype,0,sizeof(inputype));
	memset(acct_interim,0,sizeof(acct_interim));
	memset(eap,0,sizeof(eap));
	memset(quiet_period,0,sizeof(quiet_period));
	if((strcmp(secu_type,"shared")==0)||(strcmp(secu_type,"802.1x")==0))		   
	  strncpy(encr_type,"WEP",sizeof(encr_type)-1);
	else if(strcmp(secu_type,"MD5")==0)
	  strncpy(encr_type,"none",sizeof(encr_type)-1);
	else if((strcmp(secu_type,"WAPI_PSK")==0)||(strcmp(secu_type,"WAPI_AUTH")==0))
	  strncpy(encr_type,"SMS4",sizeof(encr_type)-1);
	else		 
	  cgiFormStringNoNewlines("encry_type",encr_type,20);	 

    cgiFormStringNoNewlines("acct_interim_interval",acct_interim,10);	
    acct_Time=strtoul(acct_interim,&endptr,10);			   /*char转成int，10代表十进制*/

	cgiFormStringNoNewlines("eap",eap,20);
	eap_time= strtoul(eap,&endptr,10);

	cgiFormStringNoNewlines("quiet_period",quiet_period,10);
	
    cgiFormStringNoNewlines("input_type",inputype,20);	
	/***********************security type**************************/
	ret=security_type(ins_para->parameter,ins_para->connection,sid,secu_type);                    
	switch(ret)
    {
      case SNMPD_CONNECTION_ERROR:
      case 0:{
	  	       ShowAlert(search(lsecu,"secur_type_fail"));       /*如果失败*/
			   flag=0;
			   break;
          	 }
      case 1:break;                                            /*如果成功，继续执行下一函数encryption_type()*/
	  case -1:{
	  	        ShowAlert(search(lsecu,"secur_type_unknown"));   /*如果unknown security type*/
	            flag=0;
			    break;
	  	      }
	  case -2:{
	  	        ShowAlert(search(lsecu,"secur_not_exist"));      /*如果ecurity profile not exist*/
	            flag=0;
			    break;
	  	      }
	  case -3:{
	  	        ShowAlert(search(lsecu,"secur_be_used"));      /*如果This Security Profile be used by some Wlans,please disable these Wlans first*/
	            flag=0;
			    break;
	  	      }
	  case -4:{
	  	        ShowAlert(search(lpublic,"error"));              /*如果error*/
	            flag=0;
			    break; 
	  	      }
	  case -5:{
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
      case -6:{
		        ShowAlert(search(lsecu,"radius_heart_is_on"));
		        flag=0;
		        break;
		      }
    }   
    /**********************encryption type*************************/
	if(flag==1)
	{
  	  ret=encryption_type(ins_para->parameter,ins_para->connection,sid,encr_type);	
  	  switch(ret)
      {
        case SNMPD_CONNECTION_ERROR:
        case 0:{
			     ShowAlert(search(lsecu,"encry_type_fail"));       /*如果失败*/
  	             flag=0;
			     break;
        	   }
        case 1:break;                                            /*如果成功，继续执行下一函数security_key()*/
  	    case -1:{
			      ShowAlert(search(lsecu,"encry_type_unknown"));   /*如果unknown encryption type*/
  	              flag=0;
			      break;
  	    	    }
  	    case -2:{
			      ShowAlert(search(lpublic,"encry_type_not_match")); /*如果encryption type dosen't match with security type，直接返回*/
  	              flag=0;
			      break;
  	    	    }
  	    case -3:{
			      ShowAlert(search(lsecu,"secur_not_exist"));      /*如果security id not exist*/
  	              flag=0;
			      break; 
  	    	    }
		case -4:{
	  	          ShowAlert(search(lsecu,"secur_be_used"));      /*如果This Security Profile be used by some Wlans,please disable these Wlans first*/
	              flag=0;
			      break;
	  	        }
  	    case -5:{
			      ShowAlert(search(lpublic,"error"));              /*如果error*/
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
      }   
	}

	if(flag==1)
	{
  	  if(((strcmp(secu_type,"open")==0)&&(strcmp(encr_type,"WEP")==0))||(strcmp(secu_type,"shared")==0)||(strcmp(secu_type,"WPA_P")==0)||(strcmp(secu_type,"WPA2_P")==0)||(strcmp(secu_type,"WAPI_PSK")==0))
  	  {
  	
        /**********************security key*************************/
  	    memset(secu_key,0,sizeof(secu_key));	                            /*security auth shared secret*/
  	    cgiFormStringNoNewlines("key",secu_key,70);	
  	    if(strcmp(secu_key,"")!=0)
  	    {
  	    	if(strchr(secu_key,' ')==NULL)/*不包含空格*/
  	    	{
				ret=security_key(ins_para->parameter,ins_para->connection,sid,secu_key,inputype);
				switch(ret)
				{
				  case SNMPD_CONNECTION_ERROR:
				  case 0:{
						   ShowAlert(search(lsecu,"secur_key_fail"));	  /*如果失败*/
						   flag=0;
						   break;
						 }
				  case 1:break; 										  /*如果成功，继续执行下一函数security_auth_acct(1)*/
				  case -1:{
							ShowAlert(search(lsecu,"secur_not_exist"));   /*如果security not exist*/
							flag=0;
							break;
						  }
				  case -2:{
							ShowAlert(search(lsecu,"secur_key_not_per")); /*如果security key not permit set*/
							flag=0;
							break;
						  }
				  case -3:{
							if(strcmp(encr_type,"WEP")==0)
							  ShowAlert(search(lsecu,"wep_key_length"));   /*如果key length error*/
							else if(strcmp(secu_type,"WAPI_PSK")==0)
							  ShowAlert(search(lsecu,"wapi_key_length8")); /*如果key length error*/
							else
							  ShowAlert(search(lsecu,"wpa_key_length"));   /*如果key length error*/
							flag=0;
							break;
						  }
				  case -4:{
							ShowAlert(search(lsecu,"key_has_set")); 	   /*如果Key has been set up*/
							flag=0;
							break; 
						  }
				  case -5:{
							ShowAlert(search(lsecu,"secur_be_used"));	   /*如果This Security Profile be used by some Wlans,please disable these Wlans first*/
							flag=0;
							break;
						  }
				  case -6:{
							ShowAlert(search(lpublic,"error")); 		   /*如果error*/
							flag=0;
							break; 
						  }
				  case -7:{
						   if(strcmp(encr_type,"WEP")==0)
							  ShowAlert(search(lsecu,"wep_key_length_hex"));   /*如果key length error*/
						   else if(strcmp(secu_type,"WAPI_PSK")==0)
							  ShowAlert(search(lsecu,"wapi_key_length16"));    /*如果key length error*/
						   else
							  ShowAlert(search(lsecu,"wpa_key_length_hex"));   /*如果key length error*/
							flag=0;
							break;
							  
						  }
				  case -8:{
							ShowAlert(search(lsecu,"key_format_err"));		   /*如果error*/
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
  	      ShowAlert(search(lsecu,"key_not_null"));
		  flag=0;
  	    }
  	  }
	}
  /***************************security index*****************************/
  memset(security_index,0,sizeof(security_index));
  cgiFormStringNoNewlines("security_index",security_index,5);	
  if(strcmp(security_index,"") != 0)
  {
  	ret=set_security_wep_index_cmd(ins_para->parameter,ins_para->connection,sid,security_index); /*返回0表示失败，返回1表示成功，返回-1表示unknown id format，返回-2表示input security index should be 1 to 4*/
																								 /*返回-3表示security profile does not exist，返回-4表示This Security Profile be used by some Wlans,please disable them first*/
																								 /*返回-5表示the encryption type of the security should be wep，返回-6表示error，返回-7表示Security ID非法*/
																								 /*返回-8表示illegal input:Input exceeds the maximum value of the parameter type*/
																								 /*返回SNMPD_CONNECTION_ERROR表示connection error*/
	switch(ret)
	{
		case SNMPD_CONNECTION_ERROR:
		case 0:{
				 ShowAlert(search(lsecu,"con_security_index_fail"));
				 flag=0;
	       	     break;
			   }
		case 1:break;
		case -1:{
				  ShowAlert(search(lpublic,"unknown_id_format"));
				  flag=0;
	       	      break;
			    }
		case -2:{
				  ShowAlert(search(lsecu,"security_index_illegal"));
				  flag=0;
	       	      break;
			    }
		case -3:{
				  ShowAlert(search(lsecu,"secur_not_exist"));
				  flag=0;
	       	      break;
			    }
		case -4:{
				  ShowAlert(search(lsecu,"secur_be_used"));
				  flag=0;
	       	      break;
			    }
		case -5:{
				  ShowAlert(search(lsecu,"encry_type_be_wep"));
				  flag=0;
	       	      break;
			    }
		case -6:{
				  ShowAlert(search(lpublic,"error"));
				  flag=0;
	       	      break;
			    }
		case -7:{
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
		case -8:{
				  ShowAlert(search(lpublic,"input_exceed_max_value"));
				  flag=0;
	       	      break;
			    }
	}
  }


  /***************************wpa group rekey period*****************************/
  memset(wpa_group_rekey_period,0,sizeof(wpa_group_rekey_period));
  cgiFormStringNoNewlines("wpa_group_rekey_period",wpa_group_rekey_period,10);	
  if(strcmp(wpa_group_rekey_period,"") != 0)
  {
  	ret=set_wpa_group_rekey_period_cmd(ins_para->parameter,ins_para->connection,sid,wpa_group_rekey_period); 
																				 /*返回0表示失败，返回1表示成功*/
																				 /*返回-1表示input period value should be 0 to 86400*/
																				 /*返回-2表示Security ID非法*/
																				 /*返回-3表示security profile does not exist*/
																				 /*返回-4表示This Security Profile be used by some Wlans,please disable them first*/
																				 /*返回-5表示Can't set wpa group rekey period under current security type*/
																				 /*返回-6表示error*/
																				 /*返回SNMPD_CONNECTION_ERROR表示connection error*/
	switch(ret)
	{
		case SNMPD_CONNECTION_ERROR:
		case 0:{
				 ShowAlert(search(lsecu,"config_wpa_group_rekey_period_fail"));
				 flag=0;
	       	     break;
			   }
		case 1:break;
		case -1:{
				  memset(alt,0,sizeof(alt));
                  strncpy(alt,search(lpublic,"input_para_0to"),sizeof(alt)-1);
                  strncat(alt,"86400",sizeof(alt)-strlen(alt)-1);
                  strncat(alt,search(lsecu,"secur_id_2"),sizeof(alt)-strlen(alt)-1);
                  ShowAlert(alt);
				  flag=0;
	       	      break;
			    }
		case -2:{
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
		case -3:{
				  ShowAlert(search(lsecu,"secur_not_exist"));
				  flag=0;
	       	      break;
			    }
		case -4:{
				  ShowAlert(search(lsecu,"secur_be_used"));
				  flag=0;
	       	      break;
			    }
		case -5:{
				  ShowAlert(search(lsecu,"can_not_set_wpa_group_rekey_period_under"));
				  flag=0;
	       	      break;
			    }
		case -6:{
				  ShowAlert(search(lpublic,"error"));
				  flag=0;
	       	      break;
			    }
	}
  }

  /**********************extensible authentication *************************/
if(flag==1)
{
  cgiFormStringNoNewlines("set_exten_authen",exten_type,5);
  if(strcmp(exten_type,"")!=0)
  {
    exType=strtoul(exten_type,&endptr,10); 				 /*char转成int，10代表十进制*/
    ret=extensible_authentication(ins_para->parameter,ins_para->connection,sid,exType);
	switch(ret)
    {
      case SNMPD_CONNECTION_ERROR:
      case 0:{
	           ShowAlert(search(lsecu,"exten_authen_fail"));       /*如果失败*/
  	           flag1=0;
			   break;
        	 }
      case 1:break;                                                /*如果成功*/
      case -1:{
		        ShowAlert(search(lpublic,"encry_type_not_match")); /*如果encryption type dosen't match with security type*/
  	            flag1=0;
			    break;
        	  }
  	  case -2:{
			    ShowAlert(search(lsecu,"secur_not_exist"));        /*如果security id not exist*/
  	            flag1=0;
			    break; 
  	    	  }
	  case -3:{
	  	        ShowAlert(search(lsecu,"secur_be_used"));          /*如果This Security Profile be used by some Wlans,please disable these Wlans first*/
	            flag1=0;
			    break;
	  	      }
  	  case -4:{
			    ShowAlert(search(lsecu,"exten_authen_error"));     /*如果error*/
  	            flag1=0;
			    break;
  	    	  }
	  case -5:{													   /*如果Security ID非法*/
				memset(alt,0,sizeof(alt));
				strncpy(alt,search(lpublic,"secur_id_illegal1"),sizeof(alt)-1);
				memset(max_sec_num,0,sizeof(max_sec_num));
				snprintf(max_sec_num,sizeof(max_sec_num)-1,"%d",WLAN_NUM-1);
				strncat(alt,max_sec_num,sizeof(alt)-strlen(alt)-1);
				strncat(alt,search(lpublic,"secur_id_illegal2"),sizeof(alt)-strlen(alt)-1);
				ShowAlert(alt);
				flag1=0;
				break;
			  }
	  case -6:{														/*如果extensible auth is supported open or shared*/
			    ShowAlert(search(lsecu,"exten_authen_is_support_open_or_shared"));
  	            flag1=0;
			    break;
  	    	  }
	  case -7:{
	            ShowAlert(search(lsecu,"radius_heart_is_on"));
	            flag1=0;
	            break;
	          }
    }
  }   
}
	if(flag==1)
	{
      if((strcmp(secu_type,"WPA_P")!=0)&&(strcmp(secu_type,"WPA2_P")!=0)&&(strcmp(secu_type,"WAPI_PSK")!=0)&&(strcmp(secu_type,"WAPI_AUTH")!=0))
      {
        if(!(((strcmp(secu_type,"open")==0)||(strcmp(secu_type,"shared")==0))&&(strcmp(exten_type,"0")==0)))
	    {	
	   
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
					ret=security_auth_acct(ins_para->parameter,ins_para->connection,1,sid,secu_ip,port,secu_secret);   /*security auth*/   
					switch(ret)
					{
					  case SNMPD_CONNECTION_ERROR:
					  case 0:{
							   ShowAlert(search(lsecu,"secur_auth_fail"));		/*如果失败*/
							   flag=0;
							   break;
							 }
					  case 1:break; 										/*如果成功，继续执行下一函数security_auth_acct(0)*/
					  case -1:{
								ShowAlert(search(lsecu,"unknown_port"));	/*如果unknown Port*/
								flag=0;
								break;
							  } 
					  case -2:{
								ShowAlert(search(lsecu,"secur_type_not_sup"));	/*如果security type which you choose not supported 802.1X*/
								flag=0;
								break;
							  } 
					  case -3:{
								ShowAlert(search(lsecu,"change_radius_not_per"));/*如果Change radius info not permited*/
								flag=0;
								break;
							  }
					  case -4:{
								ShowAlert(search(lsecu,"secur_be_used"));	   /*如果This Security Profile be used by some Wlans,please disable these Wlans first*/
								flag=0;
								break;
							  }
					  case -5:{
								ShowAlert(search(lpublic,"error")); 			 /*如果error*/
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
		  flag=0;
	    }

		if(flag==1)
		{
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
						ret=security_auth_acct(ins_para->parameter,ins_para->connection,0,sid,secu_ip,port,secu_secret);    /*security acct*/		
						switch(ret)
						{
						  case SNMPD_CONNECTION_ERROR:
						  case 0:{
								   ShowAlert(search(lsecu,"secur_acct_fail"));		  /*如果失败*/
								   flag=0;
								   break;
								 }
						  case 1:break; 										  /*如果成功*/
						  case -1:{
									ShowAlert(search(lsecu,"unknown_port"));		  /*如果unknown Port*/
									flag=0;
									break;
								  } 
						  case -2:{
									ShowAlert(search(lsecu,"secur_type_not_sup"));	  /*如果security type which you choose not supported 802.1X*/
									flag=0;
									break;
								  }
						  case -3:{
									ShowAlert(search(lsecu,"change_radius_not_per"));/*如果Change radius info not permited*/
									flag=0;
									break;
								  }
						  case -4:{
									ShowAlert(search(lsecu,"secur_be_used"));	   /*如果This Security Profile be used by some Wlans,please disable these Wlans first*/
									flag=0;
									break;
								  }
						  case -5:{
									ShowAlert(search(lpublic,"error")); 			  /*如果error*/
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
      	}
	    }
	  }
	}
	 /****************config host ip***************/  
	if(flag==1)
	{
		  if((strcmp(secu_type,"WPA_P")!=0)&&(strcmp(secu_type,"WPA2_P")!=0)&&(strcmp(secu_type,"WAPI_PSK")!=0)&&(strcmp(secu_type,"WAPI_AUTH")!=0))
		  { 
		     memset(host_ip,0,sizeof(host_ip));                                 
			 memset(ip1,0,sizeof(ip1));
			 cgiFormStringNoNewlines("host_ip1",ip1,4);	
			 strncat(host_ip,ip1,sizeof(host_ip)-strlen(host_ip)-1);
			 strncat(host_ip,".",sizeof(host_ip)-strlen(host_ip)-1);
			 memset(ip2,0,sizeof(ip2));
			 cgiFormStringNoNewlines("host_ip2",ip2,4); 
			 strncat(host_ip,ip2,sizeof(host_ip)-strlen(host_ip)-1);	
			 strncat(host_ip,".",sizeof(host_ip)-strlen(host_ip)-1);
		     memset(ip3,0,sizeof(ip3));
		     cgiFormStringNoNewlines("host_ip3",ip3,4); 
		     strncat(host_ip,ip3,sizeof(host_ip)-strlen(host_ip)-1);	
		     strncat(host_ip,".",sizeof(host_ip)-strlen(host_ip)-1);
		     memset(ip4,0,sizeof(ip4));
		     cgiFormStringNoNewlines("host_ip4",ip4,4);	   
		     strncat(host_ip,ip4,sizeof(host_ip)-strlen(host_ip)-1);
			 if((strcmp(ip1,"")!=0)&&(strcmp(ip2,"")!=0)&&(strcmp(ip3,"")!=0)&&(strcmp(ip4,"")!=0))	
			 {
				ret =  security_host_ip(ins_para->parameter,ins_para->connection,sid,host_ip);/*返回0表示失败，返回1表示成功，返回-1表示unknown ip format*/
													                                         /*返回-2表示check_local_ip error，返回-3表示not local ip，返回-4表示security profile does not exist*/
												                                             /*返回-5表示this security profile is used by some wlans,please disable them first，返回-6表示error，返回-7表示Security ID非法*/
																							 /*返回-8表示The radius heart test is on,turn it off first!*/
																							 /*返回SNMPD_CONNECTION_ERROR表示connection error*/
				 switch(ret)
	    	     {
	    	        case SNMPD_CONNECTION_ERROR:
	    		    case 0:{
						     ShowAlert(search(lsecu,"con_host_ip_fail"));		
							 flag=0;
				             break;
						   }
					case 1:break;
					case -1:{
						      ShowAlert(search(lpublic,"err_ip_format"));		/*unknown ip format*/
	    			          flag=0;
				              break;
	    		    	    }
	    		    case -2:{
						      ShowAlert(search(lsecu,"check_local_ip_error"));	/*如果check_local_ip error*/
	    			   	      flag=0;
				              break;
	    		    	    }
					case -3:{
						      ShowAlert(search(lsecu,"not_local_ip"));	        /*如果not local ip*/
	    			   	      flag=0;
				              break;
	    		    	    }
					case -4:{
						      ShowAlert(search(lsecu,"secur_not_exist"));	   /*如果security profile does not exist*/
	    			   	      flag=0;
				              break;
	    		    	    }
	    		    case -5:{
						      ShowAlert(search(lsecu,"secur_be_used"));        /*如果this security profile is used by some wlans,please disable them first*/
	    			  	      flag=0;
				              break;
	    		    	    }
					case -6:{
		  	                  ShowAlert(search(lpublic,"error"));              /*如果error*/
		                      flag=0;
				              break;
		  	                }
					case -7:{
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
				    case -8:{
						      ShowAlert(search(lsecu,"radius_heart_is_on"));
						      flag=0;
						      break;
						    }
	    	     }   
			 }		
		  }
  }  

  /****************connect with (wired|wireless) radius server ***************/  
if(flag==1)
{
  cgiFormStringNoNewlines("set_radius_server",radius_type,5);
  if((!(((strcmp(secu_type,"open")==0)||(strcmp(secu_type,"shared")==0))&&(strcmp(exten_type,"0")==0)))&&(strcmp(secu_type,"WPA_P")!=0)&&(strcmp(secu_type,"WPA2_P")!=0)&&(strcmp(secu_type,"WAPI_PSK")!=0)&&(strcmp(secu_type,"WAPI_AUTH")!=0))
  {
    raType=strtoul(radius_type,&endptr,10);				   /*char转成int，10代表十进制*/
	ret=radius_server(ins_para->parameter,ins_para->connection,sid,raType);
    switch(ret)
    {
      case SNMPD_CONNECTION_ERROR:
      case 0:{
	           ShowAlert(search(lsecu,"radius_server_fail"));      /*如果失败*/
               flag2=0;
	           break;
             }
      case 1:break;                                                /*如果成功*/
      case -1:{
		        ShowAlert(search(lpublic,"encry_type_not_match")); /*如果encryption type dosen't match with security type*/
  	            flag2=0;
		        break;
        	  }
  	  case -2:{
		        ShowAlert(search(lsecu,"secur_not_exist"));        /*如果security id not exist*/
  	            flag2=0;
			    break; 
  	    	  }
	  case -3:{
		        ShowAlert(search(lsecu,"secur_be_used"));          /*如果This Security Profile be used by some Wlans,please disable these Wlans first*/
	            flag2=0;
			    break;
	  	      }
  	  case -4:{
			    ShowAlert(search(lsecu,"radius_server_error"));     /*如果error*/
  	            flag2=0;
			    break;
  	    	  }
    }   
  }
}
  /****************set acct interim interval***************/  
  if((strcmp(acct_interim,"")!=0)&&(flag==1))
  {
    ret=set_acct_interim_interval(ins_para->parameter,ins_para->connection,sid,acct_Time);
    switch(ret)
    {
      case SNMPD_CONNECTION_ERROR:
      case 0:{
	           ShowAlert(search(lsecu,"acct_interim_interval_fail"));    /*如果失败*/
               flag3=0;
	           break;
             }
      case 1:break;                                                      /*如果成功*/
      case -1:{
		        ShowAlert(search(lsecu,"interim_interval_time"));        /*如果input time value should be 0 to 32767*/
  	            flag3=0;
		        break;
        	  }
  	  case -2:{
		        ShowAlert(search(lsecu,"secur_not_exist"));              /*如果security profile does not exist*/
  	            flag3=0;
			    break; 
  	    	  }
	  case -3:{
		        ShowAlert(search(lsecu,"secur_be_used"));                /*如果This Security Profile be used by some Wlans,please disable these Wlans first*/
	            flag3=0;
			    break;
	  	      }
  	  case -4:{
			    ShowAlert(search(lsecu,"can_not_set_acct_interim"));     /*如果Can't set acct interim interval under current security type*/
  	            flag3=0;
			    break;
  	    	  }
    }   
  }

  /*****************************  set eap reauth period  ****************************/ 
  if((strcmp(eap,"")!=0)&&(flag==1))
  {
    ret=set_eap_reauth_period_cmd(ins_para->parameter,ins_para->connection,sid,eap_time);
    switch(ret)
    {
      case SNMPD_CONNECTION_ERROR:
      case 0:{
		       ShowAlert(search(lsecu,"con_eap_reauth_period_fail"));	 /*如果失败*/
  	           flag4=0;
		       break;
        	 } 
      case 1:break;                                                      /*如果成功*/
	  case -1:{
		       ShowAlert(search(lpublic,"input_para_illegal"));          /*如果input period value should be 0 to 32767*/
  	           flag4=0;
		       break;
        	 }  
      case -2:{
		       ShowAlert(search(lsecu,"secur_not_exist"));               /*如果security profile not exist.*/
  	           flag4=0;
		       break;
        	 }                                                          
      case -3:{
		       ShowAlert(search(lsecu,"secur_be_used"));                 /*如果This Security Profile be used by some Wlans,please disable these Wlans first*/
  	           flag4=0;
		       break;
        	 }
  	  case -4:{
		        ShowAlert(search(lsecu,"can_not_set_eap_reauth_period"));/*如果Can't set eap reauth period,under current security type*/
  	            flag4=0;
			    break; 
  	    	  }
	  case -5:{
		        ShowAlert(search(lpublic,"error"));                      /*如果发生错误*/
	            flag4=0;
			    break;
	  	      }
    }   
  }

  
  /*****************************  set quiet period  ****************************/ 
	if((strcmp(quiet_period,"")!=0)&&(flag==1))
	{
	  ret=set_security_quiet_period_cmd_func(ins_para->parameter,ins_para->connection,sid,quiet_period);
	  switch(ret)
	  {
	  	case SNMPD_CONNECTION_ERROR:
		case 0:{
				 ShowAlert(search(lsecu,"con_quiet_period_fail"));   	   /*如果失败*/
				 flag6=0;
				 break;
			   } 
		case 1:break;													   /*如果成功*/
		case -1:{
				  ShowAlert(search(lsecu,"quiet_time_err")); 		   	   /*如果input time value should be 0 to 65535*/
				  flag6=0;
				  break;
			    }	
		case -2:{
				  ShowAlert(search(lsecu,"secur_not_exist")); 		   	   /*如果security profile not exist.*/
				  flag6=0;
				  break;
			    }
		case -3:{
				  ShowAlert(search(lsecu,"secur_be_used"));				   /*如果This Security Profile be used by some Wlans,please disable these Wlans first*/
				  flag6=0;
				  break;
			    }
		case -4:{
				  ShowAlert(search(lsecu,"can_not_set_quiet_period"));	   /*如果Can't set 1x quiet period under current security type*/
				  flag6=0;
				  break; 
				}
		case -5:{
				   ShowAlert(search(lpublic,"error"));					   /*如果发生错误*/
				   flag6=0;
				   break;
				 }
	  }   
	}

  /*****************************  set asd rdc para cmd  ****************************/ 
  memset(asd_rdc_slot,0,sizeof(asd_rdc_slot));
  cgiFormStringNoNewlines("asd_rdc_slot",asd_rdc_slot,5);
  memset(asd_rdc_ins,0,sizeof(asd_rdc_ins));
  cgiFormStringNoNewlines("asd_rdc_ins",asd_rdc_ins,5);  
  if((strcmp(asd_rdc_slot,"")!=0)&&(strcmp(asd_rdc_ins,"")!=0))
  {
    ret=set_asd_rdc_para_cmd(ins_para->parameter,ins_para->connection,sid,asd_rdc_slot,asd_rdc_ins);
																		 /*返回0表示失败，返回1表示成功*/
																		 /*返回-1表示slotid should be 0 to 16*/
																		 /*返回-2表示Security ID非法*/
																		 /*返回-3表示security type should be 802.1X, wpa_e or wpa2_e*/
																		 /*返回-4表示security profile does not exist*/
																		 /*返回-5表示this security profile is used by some wlans,please disable them first*/
																		 /*返回-6表示The radius heart test is on,turn it off first*/
																		 /*返回-7表示error*/
																		 /*返回SNMPD_CONNECTION_ERROR表示connection error*/
    switch(ret)
    {
      case SNMPD_CONNECTION_ERROR:
      case 0:{
		       ShowAlert(search(lsecu,"config_asd_rdc_fail"));
  	           flag=0;
		       break;
        	 } 
      case 1:break;                                                     
	  case -1:{
		        ShowAlert(search(lpublic,"input_para_illegal"));
  	            flag=0;
		        break;
        	  }  
      case -2:{
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
      case -3:{
		       ShowAlert(search(lsecu,"secur_type_1x_wape"));       
  	           flag=0;
		       break;
        	 }
  	  case -4:{
		        ShowAlert(search(lsecu,"secur_not_exist"));
  	            flag=0;
			    break; 
  	    	  }
	  case -5:{
		        ShowAlert(search(lsecu,"secur_be_used"));
  	            flag=0;
			    break; 
  	    	  }
	  case -6:{
		        ShowAlert(search(lsecu,"radius_heart_is_on"));
  	            flag=0;
			    break; 
  	    	  }
	  case -7:{
		        ShowAlert(search(lpublic,"error"));                      
	            flag=0;
			    break;
	  	      }
    }   
  }

  /*****************************  config wapi AS && AE ****************************/ 
  if((strcmp(secu_type,"WAPI_AUTH")==0)&&(flag==1))
  {
  	 memset(cer_type,0,sizeof(cer_type));
	 cgiFormStringNoNewlines("as_type",cer_type,10);
	 memset(as_ip,0,sizeof(as_ip));                                 
	 memset(ip1,0,sizeof(ip1));
	 cgiFormStringNoNewlines("wapi_as_ip1",ip1,4);	
	 strncat(as_ip,ip1,sizeof(as_ip)-strlen(as_ip)-1);
	 strncat(as_ip,".",sizeof(as_ip)-strlen(as_ip)-1);
	 memset(ip2,0,sizeof(ip2));
	 cgiFormStringNoNewlines("wapi_as_ip2",ip2,4); 
	 strncat(as_ip,ip2,sizeof(as_ip)-strlen(as_ip)-1);	
	 strncat(as_ip,".",sizeof(as_ip)-strlen(as_ip)-1);
     memset(ip3,0,sizeof(ip3));
     cgiFormStringNoNewlines("wapi_as_ip3",ip3,4); 
     strncat(as_ip,ip3,sizeof(as_ip)-strlen(as_ip)-1);	
     strncat(as_ip,".",sizeof(as_ip)-strlen(as_ip)-1);
     memset(ip4,0,sizeof(ip4));
     cgiFormStringNoNewlines("wapi_as_ip4",ip4,4);	   
     strncat(as_ip,ip4,sizeof(as_ip)-strlen(as_ip)-1);

	 memset(as_cer_path,0,sizeof(as_cer_path));
	 cgiFormStringNoNewlines("as_path",as_cer_path,DEFAULT_LEN);	

	 memset(ae_cer_path,0,sizeof(ae_cer_path));
	 cgiFormStringNoNewlines("ae_path",ae_cer_path,DEFAULT_LEN);
	 
	 memset(ae_cer_passwd,0,sizeof(ae_cer_passwd));
	 cgiFormStringNoNewlines("ae_passwd",ae_cer_passwd,DEFAULT_LEN);

	 memset(multi_cert_switch,0,sizeof(multi_cert_switch));	 
	 cgiFormStringNoNewlines("multi_cert_switch",multi_cert_switch,10);

	 memset(ca_cer_path,0,sizeof(ca_cer_path));
	 cgiFormStringNoNewlines("ca_path",ca_cer_path,DEFAULT_LEN);	
	 
	 if((strcmp(ip1,"")!=0)&&(strcmp(ip2,"")!=0)&&(strcmp(ip3,"")!=0)&&(strcmp(ip4,"")!=0))	
	 {
	 	if((strcmp(as_cer_path,"")!=0)&&(strcmp(ae_cer_path,"")!=0))
	 	{
			ret=config_wapi_auth(ins_para->parameter,ins_para->connection,sid,as_ip,cer_type);
			switch(ret)
			{
				case SNMPD_CONNECTION_ERROR:
				case 0:{
						 ShowAlert(search(lsecu,"config_wapi_auth_fail"));	  /*如果失败*/
						 flag5=0;
						 break;
					   }
				case 1:break;
				case -1:{
						  ShowAlert(search(lpublic,"unknown_certificate_format")); /*如果unknown certification type*/
						  flag5=0;
						  break;
						}
				case -2:{
						  ShowAlert(search(lpublic,"unknown_ip_format"));	  /*如果unknown ip format*/
						  flag5=0;
						  break;
						}
				case -3:{
						  ShowAlert(search(lsecu,"not_support_wapi_auth"));  /*如果security type which you chose does not support wapi authentication*/
						  flag5=0;
						  break;
						}
				case -4:{
						  ShowAlert(search(lsecu,"secur_be_used")); 		 /*如果this security profile be used by some wlans,please disable them first*/
						  flag5=0;
						  break;
						}
				case -5:{
						  ShowAlert(search(lpublic,"error"));				 /*如果发生错误*/
						  flag5=0;
						  break;
						}
			}

			
			if((flag=1)&&(flag5==1))
			{
			   memset(cer_name,0,sizeof(cer_name));
			   strncpy(cer_name,"/mnt/wtp/",sizeof(cer_name)-1);
			   strncat(cer_name,as_cer_path,sizeof(cer_name)-strlen(cer_name)-1);
			   ret=config_wapi_auth_path(ins_para->parameter,ins_para->connection,sid,"as",cer_name);
			   switch(ret)
			   {
			   	   case SNMPD_CONNECTION_ERROR:
				   case 0:{
							ShowAlert(search(lsecu,"config_wapi_as_path_fail"));	/*如果失败*/
							flag5=0;
							break;
						  }
				   case 1:break;
				   case -1:{
							 ShowAlert(search(lsecu,"cer_not_exit"));			/*如果certification isn't exit or can't be read*/
							 flag5=0;
							 break;
						   }
				   case -2:{
							 ShowAlert(search(lsecu,"not_support_wapi_auth"));	/*如果security type which you chose does not support wapi authentication*/
							 flag5=0;
							 break;
						   }
				   case -3:{
							 ShowAlert(search(lsecu,"secur_be_used"));			/*如果this security profile be used by some wlans,please disable them first*/
							 flag5=0;
							 break;
						   }
				   case -4:{
							 ShowAlert(search(lsecu,"secur_prof_not_intg"));	/*如果this security profile isn't integrity*/
							 flag5=0;
							 break;
						   }
				   case -5:{
							 ShowAlert(search(lpublic,"error"));				/*如果发生错误*/
							 flag5=0;
							 break;
						   }
			   }
			}

			
			if((flag=1)&&(flag5==1))
			{
			   memset(cer_name,0,sizeof(cer_name));
			   strncpy(cer_name,"/mnt/wtp/",sizeof(cer_name)-1);
			   strncat(cer_name,ae_cer_path,sizeof(cer_name)-strlen(cer_name)-1);
			   t=-1;
			   while(1)
			   { 
				  tmpStr=strstr(ae_cer_path+t+1,".");   /*在ae_cer_path+t+1中寻找"."*/
				  if(NULL!=tmpStr) 
				    t=(int)(tmpStr-ae_cer_path);         /*如果找到，t存储偏移地址*/       
				  else 				  	
				    break;  
			   } 
			   memset(temp,0,sizeof(temp));
			   strncpy(temp,ae_cer_path+t+1,sizeof(temp)-1);     /*将证书后缀赋值给temp*/ 

			   if(strcmp(temp,"p12")==0)/*如果证书是p12类型*/ 
			   {
			     if(strcmp(ae_cer_passwd,""))
			     {
				   ret=config_wapi_p12_cert_auth_path_cmd(ins_para->parameter,ins_para->connection,sid,"ae",cer_name,ae_cer_passwd);
				   switch(ret)
				   {
					   case 0:{
								ShowAlert(search(lsecu,"config_wapi_ae_path_fail")); /*如果失败*/
								flag5=0;
								break;
							  }
					   case 1:break;
					   case -1:{
								 ShowAlert(search(lsecu,"cer_not_exit"));			/*如果certification isn't exit or can't be read*/
								 flag5=0;
								 break;
							   }
					   case -2:{
								 ShowAlert(search(lsecu,"not_support_wapi_auth"));	/*如果security type which you chose does not support wapi authentication*/
								 flag5=0;
								 break;
							   }
					   case -3:{
								 ShowAlert(search(lsecu,"secur_be_used"));			/*如果this security profile be used by some wlans,please disable them first*/
								 flag5=0;
								 break;
							   }
					   case -4:{
								 ShowAlert(search(lsecu,"secur_prof_not_intg"));	/*如果this security profile isn't integrity*/
								 flag5=0;
								 break;
							   }
					   case -5:{
								 ShowAlert(search(lsecu,"p12_cert_passwd_err"));	/*如果p12 cert password error*/
								 flag5=0;
								 break;
							   }
					   case -6:{
								 ShowAlert(search(lpublic,"error"));				/*如果发生错误*/
								 flag5=0;
								 break;
							   }
				   }
			   	 }	
				 else
				 {
				   ShowAlert(search(lsecu,"key_not_null"));
				   flag=0;
				 }
			   }
			   else if(strcmp(ae_cer_path,""))
			   {
				   ret=config_wapi_auth_path(ins_para->parameter,ins_para->connection,sid,"ae",cer_name);
				   switch(ret)
				   {
				   	   case SNMPD_CONNECTION_ERROR:
					   case 0:{
								ShowAlert(search(lsecu,"config_wapi_ae_path_fail"));/*如果失败*/
								flag5=0;
								break;
							  }
					   case 1:break;
					   case -1:{
								 ShowAlert(search(lsecu,"cer_not_exit"));			/*如果certification isn't exit or can't be read*/
								 flag5=0;
								 break;
							   }
					   case -2:{
								 ShowAlert(search(lsecu,"not_support_wapi_auth"));	/*如果security type which you chose does not support wapi authentication*/
								 flag5=0;
								 break;
							   }
					   case -3:{
								 ShowAlert(search(lsecu,"secur_be_used"));			/*如果this security profile be used by some wlans,please disable them first*/
								 flag5=0;
								 break;
							   }
					   case -4:{
								 ShowAlert(search(lsecu,"secur_prof_not_intg"));	/*如果this security profile isn't integrity*/
								 flag5=0;
								 break;
							   }
					   case -5:{
								 ShowAlert(search(lpublic,"error"));				/*如果发生错误*/
								 flag5=0;
								 break;
							   }
				   }
			   }			   
			}
	 	}
		else
		{
			ShowAlert(search(lsecu,"cer_name_not_null"));
		    flag5=0;
		}			
	 }
	 else
     {
  	   ShowAlert(search(lpublic,"ip_not_null"));
	   flag=0;
     }

	 /**********  config wapi multi cert switch************/
	 ret=config_wapi_multi_cert_cmd(ins_para->parameter,ins_para->connection,sid,multi_cert_switch);
	 switch(ret)
     {
       case SNMPD_CONNECTION_ERROR:
	   case 0:{
				ShowAlert(search(lsecu,"config_wapi_multi_cert_switch_fail"));	/*如果失败*/
				flag5=0;
				break;
			  }
	   case 1:break;
	   case -1:{
				 ShowAlert(search(lpublic,"input_para_illegal"));	/*如果parameter illegal*/
				 flag5=0;
				 break;
			   }
	   case -2:{
				 ShowAlert(search(lsecu,"not_support_wapi_auth"));	/*如果security type which you chose does not support wapi authentication*/
				 flag5=0;
				 break;
			   }
	   case -3:{
				 ShowAlert(search(lsecu,"secur_be_used"));			/*如果this security profile be used by some wlans,please disable them first*/
				 flag5=0;
				 break;
			   }
	   case -4:{
				 ShowAlert(search(lpublic,"error"));				/*如果发生错误*/
				 flag5=0;
				 break;
			   }
     }

	 /***************  config wapi CA *****************/
	 if(strcmp(ca_cer_path,""))
	 {
	 	memset(cer_name,0,sizeof(cer_name));
	    strncpy(cer_name,"/mnt/wtp/",sizeof(cer_name)-1);
	    strncat(cer_name,ca_cer_path,sizeof(cer_name)-strlen(cer_name)-1);
	    ret=config_wapi_auth_path(ins_para->parameter,ins_para->connection,sid,"ca",cer_name);
	    switch(ret)
	    {
	       case SNMPD_CONNECTION_ERROR:
		   case 0:{
					ShowAlert(search(lsecu,"config_wapi_ae_path_fail"));	/*如果失败*/
					flag5=0;
					break;
				  }
		   case 1:break;
		   case -1:{
					 ShowAlert(search(lsecu,"cer_not_exit"));			/*如果certification isn't exit or can't be read*/
					 flag5=0;
					 break;
				   }
		   case -2:{
					 ShowAlert(search(lsecu,"not_support_wapi_auth"));	/*如果security type which you chose does not support wapi authentication*/
					 flag5=0;
					 break;
				   }
		   case -3:{
					 ShowAlert(search(lsecu,"secur_be_used"));			/*如果this security profile be used by some wlans,please disable them first*/
					 flag5=0;
					 break;
				   }
		   case -4:{
					 ShowAlert(search(lsecu,"secur_prof_not_intg"));	/*如果this security profile isn't integrity*/
					 flag5=0;
					 break;
				   }
		   case -5:{
					 ShowAlert(search(lpublic,"error"));				/*如果发生错误*/
					 flag5=0;
					 break;
				   }
	    }
	  }

  }


  /*****************************  config pre_auth *****************************/
  memset(pre_auth,0,sizeof(pre_auth));
  cgiFormStringNoNewlines("pre_auth",pre_auth,10);	
  if(strcmp(pre_auth,"") != 0)
  {
  	ret=config_pre_auth_cmd_func(ins_para->parameter,ins_para->connection,sid,pre_auth); /*返回0表示失败，返回1表示 成功，返回-1表示unknown encryption type*/
																						/*返回-2表示encryption type does not match security type，返回-3表示security profile does not exist*/
																						/*返回-4表示this security profile is used by some wlans,please disable them first，返回-5表示error*/
	switch(ret)
	{
		case SNMPD_CONNECTION_ERROR:
		case 0:{
				 ShowAlert(search(lsecu,"con_host_ip_fail"));     		/*如果失败*/
				 flag=0;
	       	     break;
			   }
		case 1:break;
		case -1:{
				  ShowAlert(search(lsecu,"encry_type_unknown"));  		/*如果unknown encryption type*/
				  flag=0;
	       	      break;
				}
		case -2:{
				  ShowAlert(search(lpublic,"encry_type_not_match"));    /*如果encryption type does not match security type*/
				  flag=0;
	       	      break;
			    }
		case -3:{
				  ShowAlert(search(lsecu,"secur_not_exist"));  			/*如果security profile does not exist*/
				  flag=0;
	       	      break;
			    }
		case -4:{
				  ShowAlert(search(lsecu,"secur_be_used"));  			/*如果this security profile is used by some wlans,please disable them first*/
				  flag=0;
	       	      break;
			    }
		case -5:{
				  ShowAlert(search(lpublic,"error"));                	/*如果发生错误*/
				  flag=0;
	       	      break;
			    }
	}
  }

  /*****************************  config rekey para *****************************/
  if((strcmp(secu_type,"WAPI_PSK")==0)||(strcmp(secu_type,"WAPI_AUTH")==0))
  {
  	memset(uorm,0,sizeof(uorm));
	cgiFormStringNoNewlines("method_uorm",uorm,10);	
	memset(method,0,sizeof(method));
	cgiFormStringNoNewlines("rekey_method",method,15);	
	if((strcmp(uorm,"") != 0)&&(strcmp(method,"") != 0))
	{
		ret=set_wapi_ucast_rekey_method_cmd_func(ins_para->parameter,ins_para->connection,sid,uorm,method);
		switch(ret)
		{
			case SNMPD_CONNECTION_ERROR:
			case 0:{
					 ShowAlert(search(lsecu,"con_wapi_rekey_method_fail"));    		/*如果失败*/
					 flag=0;
		       	     break;
				   }
			case 1:break;
			case -1:{
					  ShowAlert(search(lpublic,"unknown_command_format")); 		 	/*如果unknown command format*/
					  flag=0;
		       	      break;
					}
			case -2:{
					  ShowAlert(search(lsecu,"secur_not_exist"));  					/*如果security profile does not exist*/
					  flag=0;
		       	      break;
				    }
			case -3:{
					  ShowAlert(search(lsecu,"secur_be_used"));  					/*如果this security profile is used by some wlans,please disable them first*/
					  flag=0;
		       	      break;
				    }
			case -4:{
					  ShowAlert(search(lsecu,"can_not_set_wapi_rekey_method")); 	/*如果Can't set wapi rekey method under current security type*/
					  flag=0;
		       	      break;
				    }
			case -5:{
					  ShowAlert(search(lpublic,"error"));                			/*如果发生错误*/
					  flag=0;
		       	      break;
				    }
		}
	}

	memset(uorm,0,sizeof(uorm));
	cgiFormStringNoNewlines("para_uorm",uorm,10);	
	memset(torp,0,sizeof(torp));
	cgiFormStringNoNewlines("rekey_torp",torp,10);	
	memset(para,0,sizeof(para));
	cgiFormStringNoNewlines("rekey_para",para,10);	
	if((strcmp(uorm,"") != 0)&&(strcmp(torp,"") != 0)&&(strcmp(para,"") != 0))
	{
		ret=set_wapi_rekey_para_cmd_func(ins_para->parameter,ins_para->connection,sid,uorm,torp,para);
		switch(ret)
		{
			case SNMPD_CONNECTION_ERROR:
			case 0:{
					 ShowAlert(search(lsecu,"con_wapi_rekey_para_fail"));    			/*如果失败*/
					 flag=0;
		       	     break;
				   }
			case 1:break;
			case -1:{
					  ShowAlert(search(lpublic,"unknown_command_format")); 			 	/*如果unknown command format*/
					  flag=0;
		       	      break;
					}
			case -2:{
					  ShowAlert(search(lpublic,"input_para_illegal"));  				/*如果input value should be 0 to 400000000*/
					  flag=0;
		       	      break;
				    }
			case -3:{
					  ShowAlert(search(lsecu,"secur_not_exist"));  						/*如果security profile does not exist*/
					  flag=0;
		       	      break;
				    }
			case -4:{
					  ShowAlert(search(lsecu,"secur_be_used"));  						/*如果this security profile is used by some wlans,please disable them first*/
					  flag=0;
		       	      break;
				    }
			case -5:{
					  ShowAlert(search(lsecu,"can_not_set_wapi_rekey_method_under")); 	/*如果Can't set wapi rekey parameter under current config*/
					  flag=0;
		       	      break;
				    }
			case -6:{
					  ShowAlert(search(lpublic,"error"));                				/*如果发生错误*/
					  flag=0;
		       	      break;
				    }
		}
	}
  }
  
  if((flag==1)&&(flag1==1)&&(flag2==1)&&(flag3==1)&&(flag4==1)&&(flag5==1)&&(flag6==1))
    ShowAlert(search(lpublic,"oper_succ"));                /*所有操作都成功，提示操作成功*/
}



