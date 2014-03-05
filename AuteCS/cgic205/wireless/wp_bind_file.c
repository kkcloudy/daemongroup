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
* wp_wtpcon.c
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
#include "ws_dbus_list_interface.h"
#include "ws_init_dbus.h"


int ShowBindFilePage(char *m,char *n,char *pn,char *ins_id,instance_parameter *ins_para,struct list *lpublic,struct list *lwlan); 
void bind_file(instance_parameter *ins_para,struct list *lpublic,struct list *lwlan);

#define TEMP_FILE_BIND "/mnt/wtp/version_bind.txt"
#define BUF_LENZ 128

int cgiMain()
{
  char encry[BUF_LEN] = { 0 };              
  char Model[256] = { 0 };
  char pno[10] = { 0 };  
  char instance_id[10] = { 0 };
  char *str = NULL; 
  struct list *lpublic = NULL;   /*解析public.txt文件的链表头*/
  struct list *lwlan = NULL;     /*解析wlan.txt文件的链表头*/  
  instance_parameter *paraHead1 = NULL;
  dbus_parameter ins_para;
  
  lpublic=get_chain_head("../htdocs/text/public.txt");
  lwlan=get_chain_head("../htdocs/text/wlan.txt");

  ccgi_dbus_init();
  memset(encry,0,sizeof(encry));
  memset(Model,0,sizeof(Model));
  memset(pno,0,sizeof(pno));  
  memset(instance_id,0,sizeof(instance_id));
  if(cgiFormStringNoNewlines("UN", encry, BUF_LEN)!=cgiFormNotFound )  /*首次进入该页*/
  {    
	cgiFormStringNoNewlines("Model", Model, 256);		
	cgiFormStringNoNewlines("PN",pno,10);
	cgiFormStringNoNewlines("INSTANCE_ID", instance_id, 10);
  }
  else
  {    
	cgiFormStringNoNewlines("encry_conwtp",encry,BUF_LEN);
	cgiFormStringNoNewlines("ap_model",Model,256);	  
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
	ShowErrorPage(search(lpublic,"ill_user"));			/*用户非法*/
  else
	ShowBindFilePage(encry,Model,pno,instance_id,paraHead1,lpublic,lwlan);

  release(lpublic);  
  release(lwlan);
  free_instance_parameter_list(&paraHead1);
  destroy_ccgi_dbus();
  return 0;
}

int ShowBindFilePage(char *m,char *n,char *pn,char *ins_id,instance_parameter *ins_para,struct list *lpublic,struct list *lwlan)
{  
  char IsSubmit[5] = { 0 };
  DCLI_WTP_API_GROUP_ONE *wtp = NULL;
  char tmp[128] = { 0 };
  char cmd[128] = { 0 };
  FILE *pp = NULL;
  char buff[128] = { 0 };
  int i = 0,result = 0;

  cgiHeaderContentType("text/html");
  fprintf(cgiOut,"<html xmlns=\"http://www.w3.org/1999/xhtml\"><head>");
  fprintf(cgiOut,"<meta http-equiv=Content-Type content=text/html; charset=gb2312>");
  fprintf(cgiOut,"<title>Wtp</title>");
  fprintf(cgiOut,"<link rel=stylesheet href=/style.css type=text/css>"\
  "</head>");     
  memset(IsSubmit,0,sizeof(IsSubmit));  
  cgiFormStringNoNewlines("SubmitFlag", IsSubmit, 5);
  if((cgiFormSubmitClicked("bindfile_apply") == cgiFormSuccess)&&(strcmp(IsSubmit,"")))
  {
    bind_file(ins_para,lpublic,lwlan);    
  }  
  fprintf(cgiOut,"<body>"\
  "<form method=post>"\
  "<div align=center>"\
  "<table width=976 border=0 cellpadding=0 cellspacing=0>"\
  "<tr>"\
    "<td width=8 align=left valign=top background=/images/di22.jpg><img src=/images/youce4.jpg width=8 height=30/></td>"\
    "<td width=51 align=left valign=bottom background=/images/di22.jpg><img src=/images/youce33.jpg width=37 height=24/></td>"\
    "<td width=153 align=left valign=bottom id=%s background=/images/di22.jpg>%s</td>",search(lpublic,"title_style"),search(lwlan,"ap"));
    fprintf(cgiOut,"<td width=690 align=right valign=bottom background=/images/di22.jpg>");
    	  fprintf(cgiOut,"<table width=155 border=0 cellspacing=0 cellpadding=0>"\
          "<tr>"\
          "<td width=62 align=center><input id=but type=submit name=bindfile_apply style=background-image:url(/images/%s) value=""></td>",search(lpublic,"img_ok"));
   fprintf(cgiOut,"<td width=62 align=center><a href=wp_verbind.cgi?UN=%s&PN=%s&INSTANCE_ID=%s target=mainFrame><img src=/images/%s border=0 width=62 height=20/></a></td>",m,pn,ins_id,search(lpublic,"img_cancel"));
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
                    "<td align=left id=tdleft background=/images/bottom_bg.gif style=\"border-right:0\"><font id=%s>%s</font></td>",search(lpublic,"menu_san"),search(lwlan,"version_bind"));   /*突出显示*/
                  fprintf(cgiOut,"</tr>");
				  for(i=0;i<2;i++)
	              {
  				    fprintf(cgiOut,"<tr height=25>"\
                      "<td id=tdleft>&nbsp;</td>"\
                    "</tr>");
	              }
                fprintf(cgiOut,"</table>"\
              "</td>"\
              "<td align=left style=\"background-color:#ffffff; border-right:1px solid #707070; padding-left:30px\">"\
                      "<table width=550 border=0 cellspacing=0 cellpadding=0>"\
						"<tr>"\
						  "<td id=ins_style>%s:%s</td>",search(lpublic,"instance"),ins_id);
						fprintf(cgiOut,"</tr>"\
						"<tr><td align=center style=\"padding-left:20px\">");
				fprintf(cgiOut,"<table width=550 border=0 cellspacing=0 cellpadding=0>");
                fprintf(cgiOut,"<tr height=30>"\
                 "<td width=60>%s:</td>",search(lpublic,"mode"));
                 fprintf(cgiOut,"<td align=left width=490><select name=mode id=mode style=width:480px>");
				 result=show_version(ins_para->parameter,ins_para->connection,&wtp);
				 if((result == 1)&&(wtp))
				 {
				 	for(i=0; i<wtp->num; i++)
				 	{
				 		if((wtp->AP_VERSION[i])&&(wtp->AP_VERSION[i]->apmodel))
				 		{
				 			if(strcmp(wtp->AP_VERSION[i]->apmodel,n) == 0)
								fprintf(cgiOut,"<option value=%s selected=selected>%s",wtp->AP_VERSION[i]->apmodel,wtp->AP_VERSION[i]->apmodel);
							else
								fprintf(cgiOut,"<option value=%s>%s",wtp->AP_VERSION[i]->apmodel,wtp->AP_VERSION[i]->apmodel);
				 		}
				 	}
				 }
	             fprintf(cgiOut,"</select></td>"\
                "</tr>"\
				"<tr height=30>"\
				  "<td>%s:</td>",search(lpublic,"version_file"));
				  fprintf(cgiOut,"<td align=left><select name=version_file id=version_file style=width:480px>");
				  fprintf(cgiOut,"<option value=none></option>");
				  char *version_name[5]={".bin",".tar",".img",".gni",".tar.bz2"};
				  char tempz[512] = { 0 };
				  char tz[BUF_LENZ]  = { 0 };
				  memset(tempz,0,sizeof(tempz));
				      for(i=0;i<5;i++)
				      {
					  memset(tz,0,sizeof(tz));
					      snprintf(tz,sizeof(tz)-1,"ls  /mnt/wtp |grep -i '%s$'  >> %s\n",version_name[i],TEMP_FILE_BIND);
					      strncat(tempz,tz,sizeof(tempz)-strlen(tempz)-1);
				      }
				      system(tempz);

				  memset(cmd,0,sizeof(cmd));
				  snprintf(cmd,sizeof(cmd)-1,"cat %s",TEMP_FILE_BIND);
				  pp=popen(cmd,"r");
				  if(pp)
				  {
				  memset(buff,0,sizeof(buff));
				fgets( buff, sizeof(buff), pp );						 
				do
				{										   
					if(strcmp(buff,"")!=0){
						fprintf(cgiOut,"<option value=%s>%s",buff,buff);
					}					
					memset(buff,0,sizeof(buff));
					fgets( buff, sizeof(buff), pp ); 	
				}while( !feof(pp) );
				
				pclose(pp); 
				memset(cmd,0,sizeof(cmd));
				snprintf(cmd,sizeof(cmd)-1,"sudo rm %s",TEMP_FILE_BIND);
				system(cmd);
				  }
	              fprintf(cgiOut,"</select></td>"\
                "</tr>"\
				"<tr>"\
				  "<td><input type=hidden name=encry_conwtp value=%s></td>",m);
				  fprintf(cgiOut,"<td><input type=hidden name=ap_model value=%s></td>",n); 
				  fprintf(cgiOut,"<td><input type=hidden name=page_no value=%s></td>",pn);
				  fprintf(cgiOut,"<td><input type=hidden name=SubmitFlag value=%d></td>",1);
				fprintf(cgiOut,"</tr>"\
				"<tr>"\
				  "<td colspan=4><input type=hidden name=instance_id value=%s></td>",ins_id);
				fprintf(cgiOut,"</tr>"\
			    "</table>");
				fprintf(cgiOut,"</td></tr>"\
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
if(result==1)
{
  Free_wtp_model(wtp);
}
return 0;
}


void bind_file(instance_parameter *ins_para,struct list *lpublic,struct list *lwlan)   /*返回0表示失败，返回1表示成功*/
{
  int ret = 0;
  char ap_model[256] = { 0 };
  char version_file[256] = { 0 };  

  memset(ap_model,0,sizeof(ap_model));
  cgiFormStringNoNewlines("mode",ap_model,256);
  memset(version_file,0,sizeof(version_file));
  cgiFormStringNoNewlines("version_file",version_file,256);
  if((strcmp(ap_model,"")!=0)&&(strcmp(version_file,"")!=0))
  {
	ret=bind_ap_model_with_file_config(ins_para->parameter,ins_para->connection,ap_model,version_file);
	switch(ret)
	{
		case SNMPD_CONNECTION_ERROR:
		case 0:ShowAlert(search(lwlan,"version_bind_fail"));
			   break;
		case 1:ShowAlert(search(lwlan,"version_bind_succ"));
			   break;
		case -1:ShowAlert(search(lpublic,"malloc_error"));
				break;
		case -2:ShowAlert(search(lwlan,"sys_cant_find_bind_file"));
			    break;
		case -3:ShowAlert(search(lwlan,"does_not_surport_model"));
			    break;
		case -4:ShowAlert(search(lpublic,"update_is_process"));
			    break;
		case -5:ShowAlert(search(lpublic,"free_memory_is_not_enough"));
			    break;
		case -6:ShowAlert(search(lwlan,"model_has_been_bound"));
			    break;
		case -7:ShowAlert(search(lpublic,"error"));
			    break;
	}
  }
}


