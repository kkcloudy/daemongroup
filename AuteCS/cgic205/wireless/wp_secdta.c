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
* wp_secdta.c
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
#include "wcpss/asd/asd.h"
#include "dbus/asd/ASDDbusDef1.h"
#include "ws_security.h"
#include "ws_usrinfo.h"
#include "ws_err.h"
#include "ws_ec.h"
#include "ws_init_dbus.h"
#include "ws_dbus_list_interface.h"

void ShowSecdtaPage(char *m,char *n,char *ins_id,struct list *lpublic,struct list *lsecu);  

int cgiMain()
{
  char encry[BUF_LEN] = { 0 };    
  char instance_id[10] = { 0 };
  char *str = NULL;               
  struct list *lpublic = NULL;   /*解析public.txt文件的链表头*/
  struct list *lsecu = NULL;     /*解析security.txt文件的链表头*/  
  lpublic=get_chain_head("../htdocs/text/public.txt");
  lsecu=get_chain_head("../htdocs/text/security.txt");
  
  DcliWInit();
  ccgi_dbus_init();
  memset(encry,0,sizeof(encry));
  memset(instance_id,0,sizeof(instance_id));
  cgiFormStringNoNewlines("UN", encry, BUF_LEN); 
  cgiFormStringNoNewlines("INSTANCE_ID", instance_id, 10);
  str=dcryption(encry);
  if(str==NULL)
    ShowErrorPage(search(lpublic,"ill_user"));		 /*用户非法*/
  else
    ShowSecdtaPage(encry,str,instance_id,lpublic,lsecu);
  release(lpublic);  
  release(lsecu);
  destroy_ccgi_dbus();
  return 0;
}

void ShowSecdtaPage(char *m,char *n,char *ins_id,struct list *lpublic,struct list *lsecu)
{  
  char pno[10] = { 0 };
  struct dcli_security *security = NULL; 
  char SecurityType[20] = { 0 }; 
  char EncryptionType[20] = { 0 }; 
  char RekeyMethod[30] = { 0 };	
  char ID[10] = { 0 };
  char sec_name[20] = { 0 };
  char *endptr = NULL;  
  int i = 0,result = 0,sec_id = 0,key_length = 0,limit = 0,retu = 1;             /*颜色初值为#f9fafe*/
  char alt[100] = { 0 };
  char max_sec_num[10] = { 0 };
  instance_parameter *paraHead1 = NULL;
  dbus_parameter ins_para;
  cgiHeaderContentType("text/html");
  fprintf(cgiOut,"<html xmlns=\"http://www.w3.org/1999/xhtml\"><head>");
  fprintf(cgiOut,"<meta http-equiv=Content-Type content=text/html; charset=gb2312>");
  fprintf(cgiOut,"<title>%s</title>",search(lsecu,"wlan_sec"));
  fprintf(cgiOut,"<link rel=stylesheet href=/style.css type=text/css>");
  fprintf(cgiOut,"<style>"\
  	"th{ font-family:Arial, Helvetica, sans-serif; font-weight:bold; font-size:12px; color:#0a4e70}"\
  	"#div1{ width:82px; height:18px; border:1px solid #666666; background-color:#f9f8f7;}"\
    "#div2{ width:80px; height:15px; padding-left:5px; padding-top:3px}"\
    "#link{ text-decoration:none; font-size: 12px}"\
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

  "<body>"\
  "<form>"\
  "<div align=center>"\
  "<table width=976 border=0 cellpadding=0 cellspacing=0>"\
  "<tr>"\
    "<td width=8 align=left valign=top background=/images/di22.jpg><img src=/images/youce4.jpg width=8 height=30/></td>"\
    "<td width=51 align=left valign=bottom background=/images/di22.jpg><img src=/images/youce33.jpg width=37 height=24/></td>"\
    "<td width=153 align=left valign=bottom id=%s background=/images/di22.jpg>%s</td>",search(lpublic,"title_style"),search(lsecu,"wlan_sec"));
    fprintf(cgiOut,"<td width=690 align=right valign=bottom background=/images/di22.jpg>");
		  memset(pno,0,sizeof(pno));
		  cgiFormStringNoNewlines("PN",pno,10);
    	  fprintf(cgiOut,"<table width=155 border=0 cellspacing=0 cellpadding=0>"\
          "<tr>"\
          "<td width=62 align=center><a href=wp_seculis.cgi?UN=%s&PN=%s&INSTANCE_ID=%s target=mainFrame><img src=/images/%s border=0 width=62 height=20/></a></td>",m,pno,ins_id,search(lpublic,"img_ok"));
		  fprintf(cgiOut,"<td width=62 align=center><a href=wp_seculis.cgi?UN=%s&PN=%s&INSTANCE_ID=%s target=mainFrame><img src=/images/%s border=0 width=62 height=20/></a></td>",m,pno,ins_id,search(lpublic,"img_cancel"));
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
                    "<td align=left id=tdleft background=/images/bottom_bg.gif style=\"border-right:0\"><font id=%s>%s<font></td>",search(lpublic,"menu_san"),search(lsecu,"secur_det"));   /*突出显示*/
                  fprintf(cgiOut,"</tr>");
				  retu=checkuser_group(n);
				  if(retu==0)
				  {
				    fprintf(cgiOut,
                    "<tr height=25>"\
					  "<td align=left id=tdleft><a href=wp_secre.cgi?UN=%s target=mainFrame class=top><font id=%s>%s<font></a></td>",m,search(lpublic,"menu_san"),search(lsecu,"create_sec"));                       
                    fprintf(cgiOut,"</tr>"\
				    "<tr height=25>"\
					  "<td align=left id=tdleft><a href=wp_secon.cgi?UN=%s target=mainFrame class=top><font id=%s>%s<font></a></td>",m,search(lpublic,"menu_san"),search(lsecu,"config_sec"));                       
                    fprintf(cgiOut,"</tr>");
					fprintf(cgiOut,"<tr height=25>"\
					  "<td align=left id=tdleft><a href=wp_secscon.cgi?UN=%s target=mainFrame class=top><font id=%s>%s</font></a></td>",m,search(lpublic,"menu_san"),search(lsecu,"config_secon_sec"));                       
                    fprintf(cgiOut,"</tr>");
					 fprintf(cgiOut,"<tr height=25>"\
					  "<td align=left id=tdleft><a href=wp_cer_dload.cgi?UN=%s target=mainFrame class=top><font id=%s>%s</font></a></td>",m,search(lpublic,"menu_san"),search(lpublic,"cer_up"));                       
                    fprintf(cgiOut,"</tr>");
				  }
				  cgiFormStringNoNewlines("ID", ID, 10);
				  cgiFormStringNoNewlines("Na", sec_name, 20);
				  sec_id= strtoul(ID,&endptr,10);	/*char转成int，10代表十进制*/		  
				  get_slotID_localID_instanceID(ins_id,&ins_para);	
				  get_instance_dbus_connection(ins_para, &paraHead1, INSTANCE_STATE_WEB);
				  if(paraHead1)
 				  {
					  result=show_security_one(paraHead1->parameter,paraHead1->connection,sec_id,&security);			  
 				  }

				  limit=19;
				  if(result == 1)
				  {
					if(security->SecurityType==WAPI_AUTH||security->SecurityType==WAPI_PSK)
					{
						limit+=2;
						if((security->wapi_ucast_rekey_method==1)||(security->wapi_ucast_rekey_method==2))
						  limit+=1;
						if(security->wapi_ucast_rekey_method==3)
						  limit+=2;
						if((security->wapi_mcast_rekey_method==1)||(security->wapi_mcast_rekey_method==2))
						  limit+=1;
						if(security->wapi_mcast_rekey_method==3)
						  limit+=2;
					}
				  }
				  if(retu==1)  /*普通用户*/
				  	limit+=4;
				  for(i=0;i<limit;i++)
	              {
  				    fprintf(cgiOut,"<tr height=25>"\
                      "<td id=tdleft>&nbsp;</td>"\
                    "</tr>");
	              }
                fprintf(cgiOut,"</table>"\
              "</td>"\
              "<td align=left valign=middle style=\"background-color:#ffffff; border-right:1px solid #707070; padding-left:30px\">"\
 "<table width=570 border=0 bgcolor=#ffffff cellspacing=0 cellpadding=0>"\
  "<tr valign=middle>"\
    "<td>");
			if(result == 1)
			{
				memset(SecurityType, 0, sizeof(SecurityType));
				memset(EncryptionType, 0, sizeof(EncryptionType));
				CheckSecurityType(SecurityType, security->SecurityType);
				CheckEncryptionType(EncryptionType, security->EncryptionType);
				if(security->SecurityKey!=NULL)
				{
					key_length=strlen(security->SecurityKey);	/*密钥长度*/
				}
			   fprintf(cgiOut,"<table width=570 border=0 cellspacing=0 cellpadding=0>"\
			   "<tr>"\
		         "<td id=ins_style>%s:%s</td>",search(lpublic,"instance"),ins_id);
		       fprintf(cgiOut,"</tr>"\
	           "<tr align=left height=10 valign=top>");
				 fprintf(cgiOut,"<td id=thead1>%s %s</td>",sec_name,search(lsecu,"detail"));
	           fprintf(cgiOut,"</tr>"\
               "<tr>"\
               "<td align=left style=\"padding-left:20px\">");
			   if(key_length>30)
			     fprintf(cgiOut,"<table frame=below rules=rows width=570 border=1>");
			   else
			     fprintf(cgiOut,"<table frame=below rules=rows width=350 border=1>");
			   fprintf(cgiOut,"<tr align=left>"\
			   "<td id=td1 width=200>%s</td>",search(lpublic,"name"));
			   if(security->name)
			   {
				   if(key_length>30)
					 fprintf(cgiOut,"<td id=td2 width=370>%s</td>",security->name);
				   else
					 fprintf(cgiOut,"<td id=td2 width=150>%s</td>",security->name);
			   }
			  fprintf(cgiOut,"</tr>"\
				"<tr align=left>"\
				  "<td id=td1>ID</td>"\
				  "<td id=td2>%d</td>",security->SecurityID);
			  fprintf(cgiOut,"</tr>"\
				"<tr align=left>"\
				  "<td id=td1>%s IP</td>",search(lpublic,"host"));
			  	  if(security->host_ip)
			  	  {
					  fprintf(cgiOut,"<td id=td2>%s</td>",security->host_ip);
			  	  }
			  fprintf(cgiOut,"</tr>"\
				"<tr align=left>"\
				  "<td id=td1>%s</td>",search(lsecu,"secur_type"));
				  fprintf(cgiOut,"<td id=td2>%s</td>",SecurityType);
			  fprintf(cgiOut,"</tr>"\
				"<tr align=left>"\
				"<td id=td1>%s</td>",search(lsecu,"encry_type"));
			    fprintf(cgiOut,"<td id=td2>%s</td>",EncryptionType);			  
			  fprintf(cgiOut,"</tr>"\
			  	"<tr align=left>"\
				  "<td id=td1>Index</td>"\
				  "<td id=td2>%d</td>",security->security_index);
			  fprintf(cgiOut,"</tr>"\
				"<tr align=left>"\
				"<td id=td1>%s%s</td>",search(lsecu,"security"),search(lsecu,"key"));			  
			    if(security->SecurityKey)
			  	{
					fprintf(cgiOut,"<td id=td2>%s</td>",security->SecurityKey);
			  	}
			  fprintf(cgiOut,"</tr>"\
			  	"<tr align=left>"\
				"<td id=td1>%s</td>",search(lsecu,"inputype"));			  
				if(security->keyInputType==1)
					fprintf(cgiOut,"<td id=td2>%s</td>","ASCII");
				else if(security->keyInputType==2)
					fprintf(cgiOut,"<td id=td2>%s</td>","HEX");
				else
						fprintf(cgiOut,"<td id=td2>%s</td>","UNKNOWN");
			  fprintf(cgiOut,"</tr>"\
			    "<tr align=left>"\
				"<td id=td1>%s</td>",search(lsecu,"exten_auth_state"));
			    if(security->extensible_auth==1)
			      fprintf(cgiOut,"<td id=td2>open</td>");
				else
				  fprintf(cgiOut,"<td id=td2>close</td>");
			  fprintf(cgiOut,"</tr>"\
			  	"<tr align=left>"\
				"<td id=td1>Radius%s</td>",search(lsecu,"server_type"));
			    if(security->wired_radius==1)
			      fprintf(cgiOut,"<td id=td2>wired</td>");
				else
				  fprintf(cgiOut,"<td id=td2>wireless</td>");
			  fprintf(cgiOut,"</tr>"\
			  	"<tr align=left>"\
				"<td id=td1>%s</td>",search(lsecu,"eap"));			  
			    fprintf(cgiOut,"<td id=td2>%d</td>",security->eap_reauth_period);
			  fprintf(cgiOut,"</tr>"\
			  	"<tr align=left>"\
				"<td id=td1>%s</td>",search(lsecu,"acct_interim_interval"));			  
			    fprintf(cgiOut,"<td id=td2>%d</td>",security->acct_interim_interval);
			  fprintf(cgiOut,"</tr>"\
			  	"<tr align=left>"\
				"<td id=td1>%s</td>",search(lsecu,"1x_quiet_period"));			  
			    fprintf(cgiOut,"<td id=td2>%d</td>",security->quiet_period);
			  fprintf(cgiOut,"</tr>"\
				"<tr align=left>"\
				  "<td id=td1>%s</td>",search(lsecu,"auth_ip"));
			      if(security->auth.auth_ip)
			  	  {
					  fprintf(cgiOut,"<td id=td2>%s:%d</td>",security->auth.auth_ip,security->auth.auth_port);
			  	  }
				  else
				  {
					  fprintf(cgiOut,"<td id=td2>%s:%d</td>","0.0.0.0",security->auth.auth_port);
			  	  }
			  fprintf(cgiOut,"</tr>"\
				"<tr align=left>"\
				  "<td id=td1>%s</td>",search(lsecu,"auth_share_secret"));
				  if(security->auth.auth_shared_secret)
			  	  {
					  fprintf(cgiOut,"<td id=td2>%s</td>",security->auth.auth_shared_secret);
			  	  }
			  fprintf(cgiOut,"</tr>"\
				"<tr align=left>"\
				  "<td id=td1>%s</td>",search(lsecu,"acct_ip"));
			   	  if(security->acct.acct_ip)
			  	  {
					  fprintf(cgiOut,"<td id=td2>%s:%d</td>",security->acct.acct_ip,security->acct.acct_port);
			  	  }
				  else
				  {
					  fprintf(cgiOut,"<td id=td2>%s:%d</td>","0.0.0.0",security->acct.acct_port);
			  	  }
			  fprintf(cgiOut,"</tr>"\
			    "<tr align=left>"\
				  "<td id=td1>%s</td>",search(lsecu,"acct_share_secret"));
			  	  if(security->acct.acct_shared_secret)
			  	  {
					  fprintf(cgiOut,"<td id=td2>%s</td>",security->acct.acct_shared_secret);
			  	  }
			  fprintf(cgiOut,"</tr>"\
			  	
			  	"<tr align=left>"\
				  "<td id=td1>%s</td>",search(lsecu,"secon_auth_ip"));
			  	  if(security->auth.secondary_auth_ip)
			  	  {
					  fprintf(cgiOut,"<td id=td2>%s:%d</td>",security->auth.secondary_auth_ip,security->auth.secondary_auth_port);
			  	  }
				  else
				  {
					  fprintf(cgiOut,"<td id=td2>%s:%d</td>","0.0.0.0",security->auth.secondary_auth_port);
			  	  }
			  fprintf(cgiOut,"</tr>"\
			  		"<tr align=left>"\
				  "<td id=td1>%s</td>",search(lsecu,"secon_auth_share_secret"));
			  	  if(security->auth.secondary_auth_shared_secret)
			  	  {
					  fprintf(cgiOut,"<td id=td2>%s</td>",security->auth.secondary_auth_shared_secret);
			  	  }
			  fprintf(cgiOut,"</tr>"\
			  		"<tr align=left>"\
				  "<td id=td1>%s</td>",search(lsecu,"secon_acct_ip"));
			  	  if(security->acct.secondary_acct_ip)
			  	  {
					  fprintf(cgiOut,"<td id=td2>%s:%d</td>",security->acct.secondary_acct_ip,security->acct.secondary_acct_port);
			  	  }
				  else
				  {
					  fprintf(cgiOut,"<td id=td2>%s:%d</td>","0.0.0.0",security->acct.secondary_acct_port);
			  	  }
			  fprintf(cgiOut,"</tr>"\
			  		"<tr align=left>"\
				  "<td id=td1>%s</td>",search(lsecu,"secon_acct_share_secret"));
			  	  if(security->acct.secondary_acct_shared_secret)
			  	  {
					  fprintf(cgiOut,"<td id=td2>%s</td>",security->acct.secondary_acct_shared_secret);
			  	  }
			  fprintf(cgiOut,"</tr>"\
			  		"<tr align=left>"\
				  "<td id=td1>WAPI%sIP</td>",search(lsecu,"wapi_auth_ip"));
			  	  if(security->wapi_as.as_ip)
			  	  {
					  fprintf(cgiOut,"<td id=td2>%s:3810</td>",security->wapi_as.as_ip);
			  	  }
			  fprintf(cgiOut,"</tr>"\
			  		"<tr align=left>"\
				  "<td id=td1>WAPI%s</td>",search(lsecu,"wapi_cer_type"));
			      if(security->wapi_as.certification_type==WAPI_X509)
				  	fprintf(cgiOut,"<td id=td2>X.509</td>");
				  else if(security->wapi_as.certification_type==WAPI_GBW)
				  	fprintf(cgiOut,"<td id=td2>GBW</td>");
				  else
				  	fprintf(cgiOut,"<td id=td2> </td>");
			  fprintf(cgiOut,"</tr>"\
			  		"<tr align=left>"\
				  "<td id=td1>WAPI AS%s</td>",search(lsecu,"wapi_cer_path"));
			  	  if(security->wapi_as.certification_path)
			  	  {
					  fprintf(cgiOut,"<td id=td2>%s</td>",security->wapi_as.certification_path);
			  	  }
			  fprintf(cgiOut,"</tr>"\
			  		"<tr align=left>"\
				  "<td id=td1>WAPI AE%s</td>",search(lsecu,"wapi_cer_path"));
			  	  if(security->wapi_as.ae_cert_path)
			  	  {
					  fprintf(cgiOut,"<td id=td2>%s</td>",security->wapi_as.ae_cert_path);
			  	  }
			  fprintf(cgiOut,"</tr>"\
			  	    "<tr align=left>"\
				  "<td id=td1>WAPI %s</td>",search(lsecu,"wapi_multi_cert_switch"));
			  	  if(security->wapi_as.multi_cert==1)
				  	fprintf(cgiOut,"<td id=td2>enable</td>");
				  else
				  	fprintf(cgiOut,"<td id=td2>disable</td>");
			  fprintf(cgiOut,"</tr>"\
			  	  "<tr align=left>"\
				  "<td id=td1>WAPI %s</td>",search(lsecu,"wapi_ca_cer_path"));
			  	  if(security->wapi_as.ca_cert_path)
			  	  {
					  fprintf(cgiOut,"<td id=td2>%s</td>",security->wapi_as.ca_cert_path);
			  	  }
			  fprintf(cgiOut,"</tr>");
			  if(security->SecurityType==WAPI_AUTH||security->SecurityType==WAPI_PSK)
			  {
			  	  memset(RekeyMethod,0,sizeof(RekeyMethod));
			  	  CheckRekeyMethod(RekeyMethod,security->wapi_ucast_rekey_method);
				  fprintf(cgiOut,"<tr align=left>"\
					  "<td id=td1>WAPI %s</td>",search(lsecu,"unicast_rekey_method"));
					  fprintf(cgiOut,"<td id=td2>%s</td>",RekeyMethod);
				  fprintf(cgiOut,"</tr>");
				  if((security->wapi_ucast_rekey_method==1)||(security->wapi_ucast_rekey_method==3))
				  {
				  	fprintf(cgiOut,"<tr align=left>"\
					  "<td id=td1>WAPI %s</td>",search(lsecu,"unicast_rekey_time_based_para"));
					  fprintf(cgiOut,"<td id=td2>%5u</td>",security->wapi_ucast_rekey_para_t);
				    fprintf(cgiOut,"</tr>");
				  }
				  if((security->wapi_ucast_rekey_method==2)||(security->wapi_ucast_rekey_method==3))
				  {
				  	fprintf(cgiOut,"<tr align=left>"\
					  "<td id=td1>WAPI %s</td>",search(lsecu,"unicast_rekey_packet_based_para"));
					  fprintf(cgiOut,"<td id=td2>%5u</td>",security->wapi_ucast_rekey_para_p);
				    fprintf(cgiOut,"</tr>");
				  }
				  
				  memset(RekeyMethod,0,sizeof(RekeyMethod));
			  	  CheckRekeyMethod(RekeyMethod,security->wapi_mcast_rekey_method);
				  fprintf(cgiOut,"<tr align=left>"\
					  "<td id=td1>WAPI %s</td>",search(lsecu,"multicast_rekey_method"));
					  fprintf(cgiOut,"<td id=td2>%s</td>",RekeyMethod);
				  fprintf(cgiOut,"</tr>");
				  if((security->wapi_mcast_rekey_method==1)||(security->wapi_mcast_rekey_method==3))
				  {
				  	fprintf(cgiOut,"<tr align=left>"\
					  "<td id=td1>WAPI %s</td>",search(lsecu,"multicast_rekey_time_based_para"));
					  fprintf(cgiOut,"<td id=td2>%5u</td>",security->wapi_mcast_rekey_para_t);
				    fprintf(cgiOut,"</tr>");
				  }
				  if((security->wapi_mcast_rekey_method==2)||(security->wapi_mcast_rekey_method==3))
				  {
				  	fprintf(cgiOut,"<tr align=left>"\
					  "<td id=td1>WAPI %s</td>",search(lsecu,"multicast_rekey_packet_based_para"));
					  fprintf(cgiOut,"<td id=td2>%5u</td>",security->wapi_mcast_rekey_para_p);
				    fprintf(cgiOut,"</tr>");
				  }
			  }
			  fprintf(cgiOut,"</table></td>"\
				"</tr>"\
			  "</table>");
			}
            else if((result==0)||(result == SNMPD_CONNECTION_ERROR))
		      fprintf(cgiOut,"%s",search(lpublic,"contact_adm")); 	 
			else if(result==-1)
			{
				memset(alt,0,sizeof(alt));
				strncpy(alt,search(lpublic,"secur_id_illegal1"),sizeof(alt)-1);
				memset(max_sec_num,0,sizeof(max_sec_num));
				snprintf(max_sec_num,sizeof(max_sec_num)-1,"%d",WLAN_NUM-1);
				strncat(alt,max_sec_num,sizeof(alt)-strlen(alt)-1);
				strncat(alt,search(lpublic,"secur_id_illegal2"),sizeof(alt)-strlen(alt)-1);
				fprintf(cgiOut,"%s",alt);   
			}
			else if(result==-2)
			  fprintf(cgiOut,"%s",search(lsecu,"secur_not_exist"));  
			else
			  fprintf(cgiOut,"%s",search(lpublic,"error"));	
	fprintf(cgiOut,"</td>"\
 " </tr>"\
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
	Free_security_one(security);
}
free_instance_parameter_list(&paraHead1);
}


