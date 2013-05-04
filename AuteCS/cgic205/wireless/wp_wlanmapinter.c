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
* wp_wlanmapinter.c
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
#include "wcpss/wid/WID.h"
#include "dbus/wcpss/dcli_wid_wtp.h"
#include "dbus/wcpss/dcli_wid_wlan.h"
#include "ws_dcli_wlans.h"
#include "ws_init_dbus.h"
#include <sys/wait.h>
#include "ws_dbus_list_interface.h"


int ShowWlanMapInterPage(char *m,char *n,char *pn,char *ins_id,instance_parameter *ins_para,struct list *lpublic,struct list *lwlan);               /*m代表加密字符串，n代表wlan id，t代表接口类型*/
void MapInterface(instance_parameter *ins_para,char *id,struct list *lpublic,struct list *lwlan);                         
void UnmapInterface(instance_parameter *ins_para,char *id,struct list *lpublic,struct list *lwlan);                         



int cgiMain()
{
  char encry[BUF_LEN] = { 0 };              
  char *str = NULL;                
  char ID[5] = { 0 };
  char pno[10] = { 0 }; 
  char instance_id[10] = { 0 };
  struct list *lpublic = NULL;   /*解析public.txt文件的链表头*/
  struct list *lwlan = NULL;     /*解析wlan.txt文件的链表头*/  
  instance_parameter *paraHead1 = NULL;
  dbus_parameter ins_para;
  lpublic=get_chain_head("../htdocs/text/public.txt");
  lwlan=get_chain_head("../htdocs/text/wlan.txt");
  
  DcliWInit();
  ccgi_dbus_init();
  memset(encry,0,sizeof(encry));
  memset(ID,0,sizeof(ID));
  memset(pno,0,sizeof(pno)); 
  memset(instance_id,0,sizeof(instance_id));
  if(cgiFormStringNoNewlines("UN", encry, BUF_LEN)!=cgiFormNotFound )  /*首次进入该页*/
  {
	cgiFormStringNoNewlines("ID", ID, 5);	
	cgiFormStringNoNewlines("PN",pno,10);
	cgiFormStringNoNewlines("INSTANCE_ID", instance_id, 10);
  }  
  else
  {  
    cgiFormStringNoNewlines("encry_mapwlaninter",encry,BUF_LEN);
    cgiFormStringNoNewlines("wlan_id",ID,5);  
	cgiFormStringNoNewlines("page_no",pno,10);
	cgiFormStringNoNewlines("instance_id",instance_id,10);  
  }  
  
  if(strcmp(instance_id,"")==0)
  {	
	list_instance_parameter(&paraHead1, INSTANCE_STATE_WEB);	
	if(paraHead1)
	{
		snprintf(instance_id,sizeof(instance_id)-1,"%d-%d-%d",paraHead1->parameter.slot_id,paraHead1->parameter.local_id,paraHead1->parameter.instance_id); 
	} 
  }
  else
  {
	get_slotID_localID_instanceID(instance_id,&ins_para);	
	get_instance_dbus_connection(ins_para, &paraHead1, INSTANCE_STATE_WEB);
  }
  
  str=dcryption(encry);
  if(str==NULL)
	ShowErrorPage(search(lpublic,"ill_user"));			  /*用户非法*/
  else
	ShowWlanMapInterPage(encry,ID,pno,instance_id,paraHead1,lpublic,lwlan);
  
  release(lpublic);  
  release(lwlan);
  free_instance_parameter_list(&paraHead1);
  destroy_ccgi_dbus();
  return 0;
}

int ShowWlanMapInterPage(char *m,char *n,char *pn,char *ins_id,instance_parameter *ins_para,struct list *lpublic,struct list *lwlan)
{  
  int i = 0;  
  cgiHeaderContentType("text/html");
  fprintf(cgiOut,"<html xmlns=\"http://www.w3.org/1999/xhtml\"><head>");
  fprintf(cgiOut,"<meta http-equiv=Content-Type content=text/html; charset=gb2312>");
  fprintf(cgiOut,"<title>Wlan</title>");
  fprintf(cgiOut,"<link rel=stylesheet href=/style.css type=text/css>"\
  	"<style type=text/css>"\
  	".a3{width:30;border:0; text-align:center}"\
  	"</style>"\
  "</head>"\
  "<script src=/ip.js>"\
  "</script>"\
  "<body>");	
  if(cgiFormSubmitClicked("MapInter_apply") == cgiFormSuccess)
  {
  	if(ins_para)
	{
		MapInterface(ins_para,n,lpublic,lwlan);
	}
  }
  if(cgiFormSubmitClicked("UnmapInter_apply") == cgiFormSuccess)
  {
  	if(ins_para)
	{
		UnmapInterface(ins_para,n,lpublic,lwlan);
	}
  }
  fprintf(cgiOut,"<form method=post>"\
  "<div align=center>"\
  "<table width=976 border=0 cellpadding=0 cellspacing=0>"\
  "<tr>"\
    "<td width=8 align=left valign=top background=/images/di22.jpg><img src=/images/youce4.jpg width=8 height=30/></td>"\
    "<td width=51 align=left valign=bottom background=/images/di22.jpg><img src=/images/youce33.jpg width=37 height=24/></td>"\
    "<td width=153 align=left valign=bottom id=titleen background=/images/di22.jpg>WLAN</td>"\
    "<td width=690 align=right valign=bottom background=/images/di22.jpg>");
	      	  fprintf(cgiOut,"<table width=155 border=0 cellspacing=0 cellpadding=0>"\
          "<tr>"\
		  "<td width=62 align=center><a href=wp_wlanlis.cgi?UN=%s&PN=%s&INSTANCE_ID=%s target=mainFrame><img src=/images/%s border=0 width=62 height=20/></a></td>",m,pn,ins_id,search(lpublic,"img_ok"));
          fprintf(cgiOut,"<td width=62 align=center><a href=wp_wlanlis.cgi?UN=%s&PN=%s&INSTANCE_ID=%s target=mainFrame><img src=/images/%s border=0 width=62 height=20/></a></td>",m,pn,ins_id,search(lpublic,"img_cancel"));
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
                    "<td align=left id=tdleft background=/images/bottom_bg.gif style=\"border-right:0\"><font id=yingwen_san>L3</font><font id=%s>%s</font></td>",search(lpublic,"menu_san"),search(lpublic,"interface"));   /*突出显示*/
                  fprintf(cgiOut,"</tr>"\
                  "<tr height=25>"\
					"<td align=left id=tdleft><a href=wp_wlannew.cgi?UN=%s target=mainFrame class=top><font id=%s>%s</font><font id=yingwen_san> WLAN</font></a></td>",m,search(lpublic,"menu_san"),search(lpublic,"create"));                       
                  fprintf(cgiOut,"</tr>");
				  fprintf(cgiOut,"<tr height=25>"\
					"<td align=left id=tdleft><a href=wp_wlanbw.cgi?UN=%s target=mainFrame class=top><font id=yingwen_san>MAC</font><font id=%s> %s</font></a></td>",m,search(lpublic,"menu_san"),search(lwlan,"mac_filter"));                       
                  fprintf(cgiOut,"</tr>");
                  for(i=0;i<2;i++)
	              {
  				    fprintf(cgiOut,"<tr height=25>"\
                      "<td id=tdleft>&nbsp;</td>"\
                    "</tr>");
	              }
                fprintf(cgiOut,"</table>"\
              "</td>"\
              "<td align=left style=\"background-color:#ffffff; border-right:1px solid #707070; padding-left:30px; padding-top:15px\">");
			    fprintf(cgiOut,"<table width=240 border=0 cellspacing=0 cellpadding=0>"\
				"<tr>"\
		          "<td colspan=2 id=ins_style>%s:%s</td>",search(lpublic,"instance"),ins_id);
		        fprintf(cgiOut,"</tr>"\
			    "<tr height=30>"\
                 "<td>IP:</td>"\
                 "<td align=left>"\
				   "<div style=\"border-width:1;border-color:#a5acb2;border-style:solid;width:140;font-size:9pt\">"\
				   "<input type=text name='port_ip1' maxlength=3 class=a3 onKeyUp=\"mask(this,%s)\" onbeforepaste=mask_c()>.",search(lpublic,"ip_error"));
				   fprintf(cgiOut,"<input type=text name='port_ip2' maxlength=3 class=a3 onKeyUp=\"mask(this,%s)\" onbeforepaste=mask_c()>.",search(lpublic,"ip_error"));
				   fprintf(cgiOut,"<input type=text name='port_ip3' maxlength=3 class=a3 onKeyUp=\"mask(this,%s)\" onbeforepaste=mask_c()>.",search(lpublic,"ip_error"));
				   fprintf(cgiOut,"<input type=text name='port_ip4' maxlength=3 class=a3 onKeyUp=\"mask(this,%s)\" onbeforepaste=mask_c()>",search(lpublic,"ip_error"));
				   fprintf(cgiOut,"</div>"\
				 "</td>"\
                "</tr>"\
				"<tr height=30>"\
                 "<td>%s:</td>",search(lpublic,"netmask"));
                 fprintf(cgiOut,"<td align=left>"\
				   "<div style=\"border-width:1;border-color:#a5acb2;border-style:solid;width:140;font-size:9pt\">"\
              	   "<input type=text name='port_mask1' maxlength=3 class=a3 onKeyUp=\"mask(this,%s)\" onbeforepaste=mask_c()>.",search(lpublic,"ip_error"));
              	   fprintf(cgiOut,"<input type=text name='port_mask2' maxlength=3 class=a3 onKeyUp=\"mask(this,%s)\" onbeforepaste=mask_c()>.",search(lpublic,"ip_error"));
              	   fprintf(cgiOut,"<input type=text name='port_mask3' maxlength=3 class=a3 onKeyUp=\"mask(this,%s)\" onbeforepaste=mask_c()>.",search(lpublic,"ip_error"));
              	   fprintf(cgiOut,"<input type=text name='port_mask4' maxlength=3 class=a3 onKeyUp=\"mask(this,%s)\" onbeforepaste=mask_c()>",search(lpublic,"ip_error"));
				   fprintf( cgiOut,"</div>"\
				 "</td>"\
                "</tr>"\
				"<tr height=30>"\
				"<td><input type=submit style=\"width:40px\" border=0 name=MapInter_apply style=background-image:url(/images/SubBackGif.gif) value=\"%s\"></td>",search(lwlan,"wlan_map"));
                fprintf(cgiOut,"<td align=left><input type=submit style=\"width:60px\" border=0 name=UnmapInter_apply style=background-image:url(/images/SubBackGif.gif) value=\"%s\"></td>",search(lwlan,"wlan_unmap"));
                fprintf(cgiOut,"</tr>"\
                "<tr>"\
                "<td><input type=hidden name=encry_mapwlaninter value=%s></td>",m);
                fprintf(cgiOut,"<td><input type=hidden name=wlan_id value=%s></td>",n);	
	          fprintf(cgiOut,"</tr>"\
			    "<tr>"\
                "<td><input type=hidden name=page_no value=%s></td>",pn);			    
				fprintf(cgiOut,"<td><input type=hidden name=instance_id value=%s></td>",ins_id);
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
return 0;
}


int maskstr2int( char *mask )
{
	unsigned int iMask, m0, m1, m2, m3;
	char binarystr[64]="";
	int i, iRet;
	
	sscanf( mask, "%u.%u.%u.%u", &m3,&m2,&m1,&m0 );
	iMask = m3*256*256*256 + m2*256*256 + m1*256 + m0;
	
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
	
	if( strstr( binarystr, "01" ) )
	{
		return -1;	
	}
	
	return iRet;
}

void MapInterface(instance_parameter *ins_para,char *id,struct list *lpublic,struct list *lwlan)
{
	int ret = 0,status = 0,retu = 1,status1 = 0,retu1 = 1;  
	char ip1[4] = { 0 };
	char ip2[4] = { 0 };
	char ip3[4] = { 0 };
	char ip4[4] = { 0 };
	char mask1[4] = { 0 };
	char mask2[4] = { 0 };
	char mask3[4] = { 0 };
	char mask4[4] = { 0 };
	char inter_ip[20] = { 0 };  
	char inter_mask[20] = { 0 };
	char mask[5] = { 0 };  
	memset(mask,0,sizeof(mask));
	char command[100] = { 0 };
	char alt[100] = { 0 };
	char max_wlan_num[10] = { 0 };
	char tmp[10] = { 0 };
	
    memset(inter_ip,0,sizeof(inter_ip));                                 
    memset(ip1,0,sizeof(ip1));
    cgiFormStringNoNewlines("port_ip1",ip1,4);	
    strncat(inter_ip,ip1,sizeof(inter_ip)-strlen(inter_ip)-1);
    strncat(inter_ip,".",sizeof(inter_ip)-strlen(inter_ip)-1);
    memset(ip2,0,sizeof(ip2));
    cgiFormStringNoNewlines("port_ip2",ip2,4); 
    strncat(inter_ip,ip2,sizeof(inter_ip)-strlen(inter_ip)-1);	
    strncat(inter_ip,".",sizeof(inter_ip)-strlen(inter_ip)-1);
    memset(ip3,0,sizeof(ip3));
    cgiFormStringNoNewlines("port_ip3",ip3,4); 
    strncat(inter_ip,ip3,sizeof(inter_ip)-strlen(inter_ip)-1);	
    strncat(inter_ip,".",sizeof(inter_ip)-strlen(inter_ip)-1);
    memset(ip4,0,sizeof(ip4));
    cgiFormStringNoNewlines("port_ip4",ip4,4);
    strncat(inter_ip,ip4,sizeof(inter_ip)-strlen(inter_ip)-1);
	
    memset(inter_mask,0,sizeof(inter_mask)); 
    memset(mask1,0,sizeof(mask1));
    cgiFormStringNoNewlines("port_mask1",mask1,4);	
    strncat(inter_mask,mask1,sizeof(inter_mask)-strlen(inter_mask)-1);
    strncat(inter_mask,".",sizeof(inter_mask)-strlen(inter_mask)-1);
    memset(mask2,0,sizeof(mask2));
    cgiFormStringNoNewlines("port_mask2",mask2,4); 
    strncat(inter_mask,mask2,sizeof(inter_mask)-strlen(inter_mask)-1);	
    strncat(inter_mask,".",sizeof(inter_mask)-strlen(inter_mask)-1);
    memset(mask3,0,sizeof(mask3));
    cgiFormStringNoNewlines("port_mask3",mask3,4); 
    strncat(inter_mask,mask3,sizeof(inter_mask)-strlen(inter_mask)-1);	
    strncat(inter_mask,".",sizeof(inter_mask)-strlen(inter_mask)-1);
    memset(mask4,0,sizeof(mask4));
    cgiFormStringNoNewlines("port_mask4",mask4,4);	   
    strncat(inter_mask,mask4,sizeof(inter_mask)-strlen(inter_mask)-1);

    if(!(((strcmp(ip1,"")==0)&&(strcmp(ip2,"")==0)&&(strcmp(ip3,"")==0)&&(strcmp(ip4,"")==0))||
			((strcmp(ip1,""))&&(strcmp(ip2,""))&&(strcmp(ip3,""))&&(strcmp(ip4,"")))))
    {
      ShowAlert(search(lpublic,"ip_not_null"));
	  return;
    }

	if(strcmp(inter_ip,"...")!=0)/*输入IP*/
	{
		if((strcmp(mask1,"")==0)&&(strcmp(mask2,"")==0)&&(strcmp(mask3,"")==0)&&(strcmp(mask4,"")==0))/*没有输入掩码时*/
		{
		  ShowAlert(search(lpublic,"mask_not_null"));
		  return;
		}
	}

    if(!(((strcmp(mask1,"")==0)&&(strcmp(mask2,"")==0)&&(strcmp(mask3,"")==0)&&(strcmp(mask4,"")==0))||
			((strcmp(mask1,""))&&(strcmp(mask2,""))&&(strcmp(mask3,""))&&(strcmp(mask4,"")))))
    {
      ShowAlert(search(lpublic,"mask_not_null"));
	  return;
    }

	if(strcmp(inter_mask,"...")!=0)/*输入掩码时，将掩码由xxx.xxx.xxx.xxx的格式变成整数*/
	{
		ret = maskstr2int( inter_mask );
		snprintf( mask, sizeof(mask)-1, "%d", ret );
		
		if((ret<=0)||(ret>32))			 
		{
		  ShowAlert(search(lpublic,"mask_illegal"));
		  return;
		}
	}

      
	memset(command,0,sizeof(command));
	strncat(command,"set_intf_ins.sh ",sizeof(command)-strlen(command)-1);
	
	memset(tmp,0,sizeof(tmp));
	snprintf(tmp,sizeof(tmp)-1,"%d",ins_para->parameter.slot_id);
	strncat(command,tmp,sizeof(command)-strlen(command)-1);
	
	strncat(command," ",sizeof(command)-strlen(command)-1);
	memset(tmp,0,sizeof(tmp));
	snprintf(tmp,sizeof(tmp)-1,"%d",ins_para->parameter.local_id);
	strncat(command,tmp,sizeof(command)-strlen(command)-1);
	
	strncat(command," ",sizeof(command)-strlen(command)-1);
	memset(tmp,0,sizeof(tmp));
	snprintf(tmp,sizeof(tmp)-1,"%d",ins_para->parameter.instance_id);
	strncat(command,tmp,sizeof(command)-strlen(command)-1);
	
	strncat(command," ",sizeof(command)-strlen(command)-1);
	strncat(command,"wlan",sizeof(command)-strlen(command)-1);
	strncat(command,id,sizeof(command)-strlen(command)-1);
	if((strcmp(inter_ip,""))&&(strcmp(inter_ip,"..."))&&(strcmp(mask,""))&&(strcmp(inter_mask,"...")))
	{
	   strncat(command," ",sizeof(command)-strlen(command)-1);
	   strncat(command,inter_ip,sizeof(command)-strlen(command)-1);
	   strncat(command,"/",sizeof(command)-strlen(command)-1);
	   strncat(command,mask,sizeof(command)-strlen(command)-1);
	}
	strncat(command," >/var/run/apache2/wlanmapinter.txt",sizeof(command)-strlen(command)-1);
	status = system(command); 	 
	//retu = WEXITSTATUS(status);
	//if(0!=retu)    /*command fail*/
	//{
	 status1 = system("mapinter.sh"); 	 
	 retu1 = WEXITSTATUS(status1);
	 switch(retu1)
	 {
	 	case 1: ShowAlert(search(lpublic,"malloc_error"));
				break;
		case 2: {
				  memset(alt,0,sizeof(alt));
				  strncpy(alt,search(lwlan,"wlan_id_illegal1"),sizeof(alt)-1);
				  memset(max_wlan_num,0,sizeof(max_wlan_num));
				  snprintf(max_wlan_num,sizeof(max_wlan_num)-1,"%d",WLAN_NUM-1);
				  strncat(alt,max_wlan_num,sizeof(alt)-strlen(alt)-1);
				  strncat(alt,search(lwlan,"wlan_id_illegal2"),sizeof(alt)-strlen(alt)-1);
		  	      ShowAlert(alt);
			  	  break;
				}
		case 3: ShowAlert(search(lwlan,"wlan_not_exist"));
				break;
		case 4: ShowAlert(search(lwlan,"dis_wlan"));
				break;
		case 5: ShowAlert(search(lwlan,"wlan_creat_l3_fail"));
				break;
		case 6: ShowAlert(search(lwlan,"wlan_del_l3_fail"));
				break;
		case 7: ShowAlert(search(lwlan,"undo_wlan_vlan"));
				break;
		case 8: ShowAlert(search(lwlan,"wlan_create_br_fail"));
				break;
		case 9: ShowAlert(search(lwlan,"wlan_del_br_fail"));
				break;
		case 10: ShowAlert(search(lwlan,"add_bss_inter_to_wlanbr_fail"));
				 break;
		case 11: ShowAlert(search(lwlan,"del_bss_inter_to_wlanbr_fail"));
				 break;
		case 12: ShowAlert(search(lpublic,"error"));
				 break;
		default: ShowAlert(search(lwlan,"wlan_map_l3_succ"));
				 break;
	 }
	//}
	//else
	 //ShowAlert(search(lwlan,"wlan_map_l3_succ"));           
}

void UnmapInterface(instance_parameter *ins_para,char *id,struct list *lpublic,struct list *lwlan)
{
	int ret = 1,status = 0;  
	char wlaname[20] = { 0 };
	char command[50] = { 0 };
	char tmp[10] = { 0 };
	memset(command,0,sizeof(command));
	memset(wlaname,0,sizeof(wlaname));
	strncpy(wlaname,"wlan",sizeof(wlaname)-1);
	strncat(wlaname,id,sizeof(wlaname)-strlen(wlaname)-1);
 
	strncpy(command,"no_inter_ins.sh ",sizeof(command)-1);
	memset(tmp,0,sizeof(tmp));
	snprintf(tmp,sizeof(tmp)-1,"%d",ins_para->parameter.slot_id);
	strncat(command,tmp,sizeof(command)-strlen(command)-1);
	
	strncat(command," ",sizeof(command)-strlen(command)-1);
	memset(tmp,0,sizeof(tmp));
	snprintf(tmp,sizeof(tmp)-1,"%d",ins_para->parameter.local_id);
	strncat(command,tmp,sizeof(command)-strlen(command)-1);
	
	strncat(command," ",sizeof(command)-strlen(command)-1);
	memset(tmp,0,sizeof(tmp));
	snprintf(tmp,sizeof(tmp)-1,"%d",ins_para->parameter.instance_id);
	strncat(command,tmp,sizeof(command)-strlen(command)-1);
	
	strncat(command," ",sizeof(command)-strlen(command)-1);
	strncat(command,wlaname,sizeof(command)-strlen(command)-1);
    strncat(command,">/dev/null 2>&1",sizeof(command)-strlen(command)-1);
	status = system(command); 	 
	ret = WEXITSTATUS(status);
	if(0!=ret)    /*command fail*/
		ShowAlert(search(lwlan,"wlan_unmap_l3_fail"));
	else
		ShowAlert(search(lwlan,"wlan_unmap_l3_succ"));  
}


