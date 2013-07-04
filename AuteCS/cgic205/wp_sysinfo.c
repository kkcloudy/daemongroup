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
* wp_sysinfo.c
*
*
* CREATOR:
* autelan.software.Network Dep. team
* zhouym@autelan.com
*
* DESCRIPTION:
* system function for system infos
*
*
*******************************************************************************/
#include <stdio.h>
#include "cgic.h"
#include <string.h>
#include <stdlib.h>
#include "ws_err.h"
#include "ws_usrinfo.h"
#include "ws_ec.h"

#include "ws_sysinfo.h"

#include <sys/wait.h>
#include <sys/resource.h>
#include <sys/stat.h>
#include "memory.h"

#include <shadow.h>

#include <pwd.h>
#include <grp.h>
#include <sys/types.h>
#include <stdarg.h>
#include <errno.h>


int ShowSystemInformationPage(struct list *lpublic, struct list *lsystem, struct list *lcontrol);   /*n代表加密后的字符串*/

void saveconfig();
void reboot();
int cgiMain()
{
	struct list *lpublic;	/*解析public.txt文件的链表头*/
	struct list *lsystem;	  /*解析system.txt文件的链表头*/	
	struct list *lcontrol;	  /*解析system.txt文件的链表头*/	
	lpublic=get_chain_head("../htdocs/text/public.txt");
	lsystem=get_chain_head("../htdocs/text/system.txt"); 
	lcontrol=get_chain_head("../htdocs/text/control.txt"); 
	
	ShowSystemInformationPage(lpublic,lsystem,lcontrol);
  	release(lpublic);  
  	release(lsystem);

	return 0;
}

int ShowSystemInformationPage(struct list *lpublic, struct list *lsystem, struct list *lcontrol)
{

	int i, ret,status;
	int retu;
	char *encry=(char *)malloc(BUF_LEN);                /*存储从wp_usrmag.cgi带入的加密字符串*/
	char save_encry[BUF_LEN];
	char *str;  
	
	struct sys_ver ptrsysver;/*产品系统信息*/
	ptrsysver.product_name=(char *)malloc(50);
	ptrsysver.base_mac=(char *)malloc(50);
	ptrsysver.serial_no=(char *)malloc(50);
	ptrsysver.swname=(char *)malloc(50); 
    char * procductId=readproductID();

	struct sys_img sysimg;
	sysimg.boot_img=(char *)malloc(50);

	
	if((cgiFormSubmitClicked("save_config") != cgiFormSuccess)&&(cgiFormSubmitClicked("del_config") != cgiFormSuccess)&&(cgiFormSubmitClicked("reboot") != cgiFormSuccess))
	{
		memset(encry,0,BUF_LEN);
		cgiFormStringNoNewlines("UN", encry, BUF_LEN); 
		str=dcryption(encry);
		if(str==NULL)
		{
		  ShowErrorPage(search(lpublic,"ill_user"));          //用户非法
		  return 0;
		}
		memset(save_encry,0,BUF_LEN);                   //清空临时变量
	}
   else
   {
	   cgiFormStringNoNewlines("encry_save",save_encry,BUF_LEN);
	   str=dcryption(save_encry);
   }
	  cgiHeaderContentType("text/html");
	fprintf(cgiOut,"<html xmlns=\"http://www.w3.org/1999/xhtml\"><head>");
	fprintf(cgiOut,"<meta http-equiv=Content-Type content=text/html; charset=gb2312>");
	fprintf(cgiOut,"<title>%s</title>",search(lsystem,"sys_function"));
	fprintf(cgiOut,"<link rel=stylesheet href=/style.css type=text/css>"\
		"<style type=text/css>"\
		".a3{width:30;border:0; text-align:center}"\
		"</style>"\
	"</head>"\
	"<script src=/ip.js>"\
	"</script>"\
	"<body>");
  if(cgiFormSubmitClicked("save_config") == cgiFormSuccess)
  {
    ret=1;
#if 0
		;
#else
   // int status_m =system("srvsave.sh");
	status = system("save_config.sh > /dev/null");

	ret = WEXITSTATUS(status);
	//int ret = WEXITSTATUS(status_m);
	//fprintf(cgiOut,"service save result = %d",status_m);

#endif
  
    if(ret==0)
    {
  	ShowAlert(search(lpublic,"oper_succ"));
    }
    else
    {
  	ShowAlert(search(lpublic,"oper_fail"));
    }
  }

    if(cgiFormSubmitClicked("del_config") == cgiFormSuccess)
	{
		ret = 1;
    int status_d =system("sudo earse.sh > /dev/null");
		if(status_d==0)
		{
			ShowAlert(search(lpublic,"oper_succ"));
		}
		else
		{
			ShowAlert(search(lpublic,"oper_fail"));
		}
	}
  
  if(cgiFormSubmitClicked("reboot") == cgiFormSuccess)
  {
  
  	status = system("dcli_reboot.sh > /dev/null");		  
//  	ret = WEXITSTATUS(status);

//  	if(ret==0)
//  	{
  	  ShowAlert(search(lpublic,"oper_succ"));
//  	}
//  	else
//  	{
//  	  ShowAlert(search(lpublic,"oper_fail"));
//  	}
  }
  
  fprintf(cgiOut,"<form method=post>"\
  "<div align=center>"\
  "<table width=976 border=0 cellpadding=0 cellspacing=0>"\
  "<tr>"\
    "<td width=8 align=left valign=top background=/images/di22.jpg><img src=/images/youce4.jpg width=8 height=30/></td>"\
    "<td width=51 align=left valign=bottom background=/images/di22.jpg><img src=/images/youce33.jpg width=37 height=24/></td>"\
	"<td width=153 align=left valign=bottom id=%s background=/images/di22.jpg>%s</td>",search(lpublic,"title_style"),search(lsystem,"sys_function"));
    fprintf(cgiOut,"<td width=690 align=right valign=bottom background=/images/di22.jpg>");
	
	
				  
	  if((cgiFormSubmitClicked("save_config") != cgiFormSuccess)&&(cgiFormSubmitClicked("del_config") != cgiFormSuccess)&&(cgiFormSubmitClicked("reboot") != cgiFormSuccess))
	  {
    	  fprintf(cgiOut,"<table width=130 border=0 cellspacing=0 cellpadding=0>"\
		"<tr>");		

		fprintf(cgiOut,"<td width=62 align=left><a href=wp_sysmagic.cgi?UN=%s target=mainFrame><img src=/images/%s border=0 width=62 height=20/></a></td>",encry,search(lpublic,"img_ok"));
		fprintf(cgiOut,"<td width=62 align=left><a href=wp_sysmagic.cgi?UN=%s target=mainFrame><img src=/images/%s border=0 width=62 height=20/></a></td>",encry,search(lpublic,"img_cancel"));

		  
        fprintf(cgiOut,"</tr>"\
		"</table>");
	  }
	  else	
	  {
	  	fprintf(cgiOut,"<table width=130 border=0 cellspacing=0 cellpadding=0>"\
		"<tr>");        
	    {
	      fprintf(cgiOut,"<td width=62 align=left><a href=wp_sysmagic.cgi?UN=%s target=mainFrame><img src=/images/%s border=0 width=62 height=20/></a></td>",save_encry,search(lpublic,"img_ok"));
		  fprintf(cgiOut,"<td width=62 align=left><a href=wp_sysmagic.cgi?UN=%s target=mainFrame><img src=/images/%s border=0 width=62 height=20/></a></td>",save_encry,search(lpublic,"img_cancel"));
		}
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
				  retu=checkuser_group(str);
				  if((cgiFormSubmitClicked("save_config") != cgiFormSuccess)&&(cgiFormSubmitClicked("del_config") != cgiFormSuccess)&&(cgiFormSubmitClicked("reboot") != cgiFormSuccess))
				  {					   
						fprintf(cgiOut,"<tr height=26>"\
						"<td align=left id=tdleft background=/images/bottom_bg.gif style=\"border-right:0\"><font id=%s>%s</font></td>",search(lpublic,"menu_san"),search(lsystem,"sys_infor"));  /*突出显示*/
						fprintf(cgiOut,"</tr>");
						if(retu==0)
						{
							fprintf(cgiOut,"<tr height=25>"\
							"<td align=left id=tdleft><a href=wp_sysconfig.cgi?UN=%s target=mainFrame class=top><font id=%s>%s</font></a></td>",encry,search(lpublic,"menu_san"),search(lsystem,"sys_config"));
							fprintf(cgiOut,"</tr>"\
							"<tr height=25>"\
							"<td align=left id=tdleft><a href=wp_impconf.cgi?UN=%s target=mainFrame class=top><font id=%s>%s</font></a></td>",encry,search(lpublic,"menu_san"),search(lsystem,"import_config"));
							fprintf(cgiOut,"</tr>"\
							"<tr height=25>"\
							"<td align=left id=tdleft><a href=wp_export.cgi?UN=%s target=mainFrame class=top><font id=%s>%s</font></a></td>",encry,search(lpublic,"menu_san"),search(lsystem,"export_config"));
							fprintf(cgiOut,"</tr>");
							//新增条目
							fprintf(cgiOut,"<tr height=25>"\
							"<td align=left id=tdleft><a href=wp_version_upgrade.cgi?UN=%s target=mainFrame class=top><font id=%s>%s</font></a></td>",encry,search(lpublic,"menu_san"),search(lpublic,"version_up"));
							fprintf(cgiOut,"</tr>");

							//boot upgrade 
							fprintf(cgiOut,"<tr height=25>"\
							"<td align=left id=tdleft><a href=wp_boot_upgrade.cgi?UN=%s target=mainFrame class=top><font id=%s>%s</font></a></td>",encry,search(lpublic,"menu_san"),search(lsystem,"boot_item"));
							fprintf(cgiOut,"</tr>");


							fprintf(cgiOut,"<tr height=25>"\
							"<td align=left id=tdleft><a href=wp_log_info.cgi?UN=%s target=mainFrame class=top><font id=%s>%s</font></a></td>",encry,search(lpublic,"menu_san"),search(lpublic,"log_info"));
							fprintf(cgiOut,"</tr>");   	            

							fprintf(cgiOut,"<tr height=25>"\
							"<td align=left id=tdleft><a href=wp_login_limit.cgi?UN=%s target=mainFrame class=top><font id=%s>%s</font></a></td>",encry,search(lpublic,"menu_san"),search(lpublic,"l_user"));
							fprintf(cgiOut,"</tr>");

						}


						//新增时间条目
						fprintf(cgiOut,"<tr height=26>"\
						"<td align=left id=tdleft><a href=wp_showtime.cgi?UN=%s target=mainFrame class=top><font id=%s>%s</font></a></td>",encry,search(lpublic,"menu_san"),search(lsystem,"systime"));
						fprintf(cgiOut,"</tr>");

						if(retu==0)
						{
							//新增条目
							fprintf(cgiOut,"<tr height=25>"\
							"<td align=left id=tdleft><a href=wp_ntp.cgi?UN=%s target=mainFrame class=top><font id=%s>%s</font></a></td>",encry,search(lpublic,"menu_san"),search(lpublic,"ntp_s"));
							fprintf(cgiOut,"</tr>");

							//新增pppoe条目
							fprintf(cgiOut,"<tr height=25>"\
							"<td align=left id=tdleft><a href=wp_pppoe_server.cgi?UN=%s target=mainFrame class=top><font id=%s>%s</font></a></td>",encry,search(lpublic,"menu_san"),"PPPOE");
							fprintf(cgiOut,"</tr>");
						}
						//新增时间条目
						fprintf(cgiOut,"<tr height=26>"\
						"<td align=left id=tdleft><a href=wp_webservice.cgi?UN=%s target=mainFrame class=top><font id=%s>%s</font></a></td>",encry,search(lpublic,"menu_san"),search(lpublic,"web_service"));
						fprintf(cgiOut,"</tr>");

				  }
				  else			  
				  {
					fprintf(cgiOut,"<tr height=26>"\
					"<td align=left id=tdleft background=/images/bottom_bg.gif style=\"border-right:0\">%s</td>",search(lsystem,"sys_infor"));  /*突出显示*/
					fprintf(cgiOut,"</tr>");
					if(retu==0)
					{
						fprintf(cgiOut,"<tr height=25>"\
						"<td align=left id=tdleft><a href=wp_impconf.cgi?UN=%s target=mainFrame class=top><font id=%s>%s</font></a></td>",save_encry,search(lpublic,"menu_san"),search(lsystem,"import_config"));
						fprintf(cgiOut,"</tr>"\
						"<tr height=25>"\
						"<td align=left id=tdleft><a href=wp_export.cgi?UN=%s target=mainFrame class=top><font id=%s>%s</font></a></td>",save_encry,search(lpublic,"menu_san"),search(lsystem,"export_config"));
						fprintf(cgiOut,"</tr>");
						//新增条目
						fprintf(cgiOut,"<tr height=25>"\
						"<td align=left id=tdleft><a href=wp_version_upgrade.cgi?UN=%s target=mainFrame class=top><font id=%s>%s</font></a></td>",save_encry,search(lpublic,"menu_san"),search(lpublic,"version_up"));
						fprintf(cgiOut,"</tr>");

						//boot upgrade 
						fprintf(cgiOut,"<tr height=25>"\
						"<td align=left id=tdleft><a href=wp_boot_upgrade.cgi?UN=%s target=mainFrame class=top><font id=%s>%s</font></a></td>",save_encry,search(lpublic,"menu_san"),search(lsystem,"boot_item"));
						fprintf(cgiOut,"</tr>");


						fprintf(cgiOut,"<tr height=25>"\
						"<td align=left id=tdleft><a href=wp_log_info.cgi?UN=%s target=mainFrame class=top><font id=%s>%s</font></a></td>",save_encry,search(lpublic,"menu_san"),search(lpublic,"log_info"));
						fprintf(cgiOut,"</tr>");

						fprintf(cgiOut,"<tr height=25>"\
						"<td align=left id=tdleft><a href=wp_login_limit.cgi?UN=%s target=mainFrame class=top><font id=%s>%s</font></a></td>",save_encry,search(lpublic,"menu_san"),search(lpublic,"l_user"));
						fprintf(cgiOut,"</tr>");

					}



					//新增时间条目
					fprintf(cgiOut,"<tr height=26>"\
					"<td align=left id=tdleft><a href=wp_showtime.cgi?UN=%s target=mainFrame class=top><font id=%s>%s</font></a></td>",save_encry,search(lpublic,"menu_san"),search(lsystem,"systime"));
					fprintf(cgiOut,"</tr>");


					if(retu==0)
					{
						//新增条目
						fprintf(cgiOut,"<tr height=25>"\
						"<td align=left id=tdleft><a href=wp_ntp.cgi?UN=%s target=mainFrame class=top><font id=%s>%s</font></a></td>",save_encry,search(lpublic,"menu_san"),search(lpublic,"ntp_s"));
						fprintf(cgiOut,"</tr>");
						//新增pppoe条目
						fprintf(cgiOut,"<tr height=25>"\
						"<td align=left id=tdleft><a href=wp_pppoe_server.cgi?UN=%s target=mainFrame class=top><font id=%s>%s</font></a></td>",save_encry,search(lpublic,"menu_san"),"PPPOE");
						fprintf(cgiOut,"</tr>");
					}
					
					//新增时间条目
					fprintf(cgiOut,"<tr height=26>"\
					"<td align=left id=tdleft><a href=wp_webservice.cgi?UN=%s target=mainFrame class=top><font id=%s>%s</font></a></td>",encry,search(lpublic,"menu_san"),search(lpublic,"web_service"));
					fprintf(cgiOut,"</tr>");

				 }
				  for(i=0;i<8;i++)
				  {
					fprintf(cgiOut,"<tr height=25>"\
					  "<td id=tdleft>&nbsp;</td>"\
					"</tr>");
				  }
				fprintf(cgiOut,"</table>"\
			  "</td>"\
			  "<td align=left style=\"background-color:#ffffff; border-right:1px solid #707070; padding-left:30px\">"\
    			"<table style=\"padding-left:50px; padding-bottom:10px\" width=594 border=0 cellspacing=0 cellpadding=0>"\
    			  "<tr style=\"padding-left:380px; padding-bottom:10px\">"\
    				"<td><table width=150 border=0 cellspacing=0 cellpadding=0>"\
    				  "<tr>");
			if(retu==0)
			{
     			fprintf(cgiOut,"<td><input type=submit width=60 name=save_config value=%s /></td>"\
				"<td><input type=submit name=del_config value=%s /></td>"\
				"<td><input type=submit name=reboot value=%s onclick=\"return confirm('%s')\" /></td>",search(lsystem,"save_config"),search(lsystem,"reset"),search(lsystem,"reboot"),search(lcontrol,"confirm_reboot"));

			}
			else
			{
				fprintf(cgiOut,"<td><input type=submit width=60 name=save_config value=%s disabled></td>"\
				"<td><input type=submit name=del_config value=%s disabled></td>"\
         	    "<td><input type=submit name=reboot value=%s onclick=\"return confirm('%s')\" disabled></td>",search(lsystem,"save_config"),search(lsystem,"reset"),search(lsystem,"reboot"),search(lcontrol,"confirm_reboot"));
			}
			fprintf(cgiOut,"</table></td>"\
				  "</tr>"\
				  "<tr>"\
					"<td id=thead1 style=\"padding-left:200px; padding-bottom:0px\"><font size=4px>%s</font></td>",search(lsystem,"product_info"));
				  fprintf(cgiOut,"</tr>"\
				  "<tr>"\
					"<td><table frame=below rules=rows width=500 border=1 cellspacing=0 cellpadding=0>");
			
           

			    ccgi_dbus_init();		  /*初始化dbus*/
				if(show_sys_ver(&ptrsysver)==0)   /*获取系统信息*/
				{
					fprintf(cgiOut,"<tr>"\
								"<td id=td1 width=240>Software Name</td>"\
								"<td id=td2 width=260>%s</td>",ptrsysver.sw_name);
					fprintf(cgiOut,"</tr>"\
							  "<tr>"\
								"<td id=td1>Product Name</td>"\
								"<td id=td2>%s</td>",ptrsysver.sw_product_name);
#if 0
					fprintf(cgiOut,"</tr>"\
							  "<tr>"\
								"<td id=td1>Version</td>"\
								"<td id=td2>V%d.%d.%d(Build%03d)</td>",SW_MAJOR_VER(ptrsysver.sw_version),SW_MINOR_VER(ptrsysver.sw_version),SW_COMPATIBLE_VER(ptrsysver.sw_version),SW_BUILD_VER(ptrsysver.sw_version));
#endif
{
					fprintf(cgiOut,"</tr>"\
							  "<tr>"\
								"<td id=td1>Version</td>"\
								"<td id=td2>%s</td>",ptrsysver.sw_version_str);

}					

					fprintf(cgiOut,"</tr>"\
							  "<tr>"\
								"<td id=td1>MAC Address</td>"\
								"<td id=td2>%s</td>",ptrsysver.sw_mac);
					fprintf(cgiOut,"</tr>");

                    #if 0
					fprintf(cgiOut,"<tr>"\
								"<td id=td1>Serial Number</td>"\
								"<td id=td2>%s</td>",ptrsysver.sw_serial_num);
					fprintf(cgiOut,"</tr>");
					
					ret=show_system_bootimg(&sysimg);
					//ShowAlert(sysimg.boot_img);
					//fprintf(cgiOut,"ret is: %d",ret);
					fprintf(cgiOut,"<tr>");
					fprintf(cgiOut,"<td id=td1>Boot Version</td>"\
								"<td id=td2>%s</td>",sysimg.boot_img);
					fprintf(cgiOut,"</tr>");
					#endif
					FILE *fp = NULL;
					char acDeviceSn[256] = {0};
					memset(acDeviceSn,0,256);			
				
					fp=fopen("/devinfo/sn","r");
					if(fp != NULL)
					{
						fgets(acDeviceSn,256,fp);
						if(strcmp(acDeviceSn,"") == 0)
						{
							memset(acDeviceSn,0,256);			
							strncpy(acDeviceSn,"01010106C14009900001",sizeof(acDeviceSn)-1);
						}
						fclose(fp);
					}
					else
					{
						memset(acDeviceSn,0,256);			
						strncpy(acDeviceSn,"01010106C14009900001",sizeof(acDeviceSn)-1);
					}	
					
					fprintf(cgiOut,"<tr>"\
								"<td id=td1>SN</td>"\
								"<td id=td2>%s</td>",acDeviceSn);
							  fprintf(cgiOut,"</tr>"\
							  "<tr>");
					if((cgiFormSubmitClicked("save_config") != cgiFormSuccess)&&(cgiFormSubmitClicked("del_config") != cgiFormSuccess)&&(cgiFormSubmitClicked("reboot") != cgiFormSuccess))
					{
					  fprintf(cgiOut,"<td colspan=3><input type=hidden name=encry_save value=%s></td>",encry);
					}
					else
					{
					  fprintf(cgiOut,"<td colspan=3><input type=hidden name=encry_save value=%s></td>",save_encry);
					}
							
					fprintf(cgiOut,"</tr>");
				}
				else
				{
					fprintf(cgiOut,"%s",search(lpublic,"contact_adm"));  
				}
			//else
			//{	  
				//fprintf(cgiOut,"%s",search(lpublic,"contact_adm"));	
			//}
			fprintf(cgiOut,"</table></td>"\
			"</tr>"\
			"<tr>"\
			  "<td><table frame=below rules=rows width=500 border=1 cellspacing=0 cellpadding=0>");

              #if 0
				if(show_hardware_configuration()!=0)
				{
					fprintf(cgiOut,"%s",search(lpublic,"contact_adm"));  
				}
              #endif
			
			fprintf(cgiOut,"</table>"\
			  "</td>"\
			"</tr>");
			fprintf(cgiOut,"<tr>"\
			"<td id=sec1 style=\"border-bottom:2px solid #53868b;font-size:14px\">%s</td>",search(lpublic,"description"));
			fprintf(cgiOut,"</tr>");
			
    			fprintf(cgiOut,"<tr height=25 style=padding-top:2px>"\
    			  "<td style=font-size:14px;color:#FF0000>%s </td>"\
    			"</tr>"\
    			"<tr height=25>"\
    			  "<td style=font-size:14px;color:#FF0000> %s</td>"\
    			"</tr>",search(lcontrol,"save_des"),search(lcontrol,"reboot_des"));
			

			fprintf(cgiOut,"</table>"\
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
  free(sysimg.boot_img);
  free(procductId); 
  free(ptrsysver.product_name);
  free(ptrsysver.base_mac);
  free(ptrsysver.serial_no);
  free(ptrsysver.swname);

  return 0;
}





