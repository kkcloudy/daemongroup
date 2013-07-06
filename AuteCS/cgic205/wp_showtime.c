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
* wp_showtime.c
*
*
* CREATOR:
* autelan.software.Network Dep. team
* zhouym@autelan.com
*
* DESCRIPTION:
* system function for system time show
*
*
*******************************************************************************/
#include <stdio.h>
#include "ws_ec.h"
#include "cgic.h"
#include <string.h>
#include <stdlib.h>
#include "ws_usrinfo.h"
#include "ws_err.h"
#include "ws_nm_status.h"
#include "ws_init_dbus.h"
#include "ws_dbus_list_interface.h"
#include "ws_dbus_list.h"
#include "ac_manage_def.h"
#include "ac_manage_extend_interface.h"
#include "ac_manage_ntpsyslog_interface.h"
#include <time.h>
#include <sys/time.h>

#define PATH_LENG 512  
#define SHOW_AMOUNT 64  //转换数组的容量

int ShowTime(struct list *lpublic, struct list *llocal);   /*n代表加密后的字符串*/

int cgiMain()
{
	struct list *lpublic;
	struct list *lsystem;
	lpublic=get_chain_head("../htdocs/text/public.txt");
	lsystem=get_chain_head("../htdocs/text/system.txt");

	ShowTime(lpublic, lsystem);
	return 0;
}

int ShowTime(struct list *lpublic, struct list *lsystem)
{ 
  
	char addn[N]="";
	int i;
	char encry[BUF_LEN] = {0}; 
	char *str = NULL;        
	cgiFormStringNoNewlines("UN", encry, BUF_LEN); 
	str=dcryption(encry);
	if(str==NULL)
	{
		ShowErrorPage(search(lpublic, "ill_user")); 
		return 0;
	}
	strcpy(addn,str);
	char timebuf[128] = {0};
	char timecst[128] = {0};

	 FILE *start_time,*cur_time;	
	 char buff_start[1024]="";
	 char buff_cur[1024]="";
	 //***************转换变量*****************
	 char month[SHOW_AMOUNT]="";
	 char week[SHOW_AMOUNT]="";
	 char day[SHOW_AMOUNT]="";
	 char time_now[SHOW_AMOUNT]="";
	 char year[SHOW_AMOUNT]="";
	 char area[SHOW_AMOUNT]="";
	 char out_put[128]="";
	 char temp[SHOW_AMOUNT]="";
	 
	 char pass_time[SHOW_AMOUNT]="";
	 char pass_time_day[SHOW_AMOUNT]="";
	 char hour[SHOW_AMOUNT]="";
	 char minute[SHOW_AMOUNT]="";
	 //*************************************

	 char time[PATH_LENG]="";
	 char date[PATH_LENG]="";
	 char days[PATH_LENG] = "";
	 memset(time,0,PATH_LENG); //清空临时变量
	char *buf;
	char cmdbuf[256] = {0};
	char zonestr[20] = {0};
	struct tm tm;
	time_t timez;
	int timediff = 0;
	struct tm *ptr;
	int p_masterid = 0;	
	p_masterid = get_product_info(PRODUCT_ACTIVE_MASTER);
	 	
//****************预处理执行命令************************
	start_time = popen("uptime","r");
	 cur_time = popen("date","r");
	 if(start_time != NULL)
	 {
		 fgets(buff_start,sizeof(buff_start),start_time);
	 }
	 if(cur_time != NULL)
	 {
		 fgets(buff_cur,sizeof(buff_cur),cur_time);
	 }
	//**********获取具体时间信息************
	strcpy(week,strtok(buff_cur," "));
	strcpy(month,strtok(NULL," "));
	strcpy(day,strtok(NULL," "));
	strcpy(time_now,strtok(NULL," "));
	strcpy(area,strtok(NULL," "));
	strcpy(year,strtok(NULL," "));
		//去掉最后的空格
		year[4]='\0';

		
	strcat(out_put,year);
	strcat(out_put,search(lsystem,"year"));
	strcat(out_put,search(lsystem,month));
	strcat(out_put,search(lsystem,"month"));
	strcat(out_put,search(lsystem,day));
	strcat(out_put,search(lsystem,"day"));
	strcat(out_put,search(lsystem,week));
	strcat(out_put," ");
	strcat(out_put,time_now);
	strcat(out_put," ");
	strcat(out_put,area);
	//***********获取系统信息***********
	//strcpy(buff_start,"17:22:48  up 1 day,  11:08,  5 users,  load average: 2.20, 2.05, 2.02");
	strcpy(temp,strtok(buff_start," "));
	strcpy(temp,strtok(NULL," "));
	strcpy(pass_time,strtok(NULL,","));
	if(strchr(pass_time,'d')!=NULL)
	{
		strcpy(pass_time_day,strtok(NULL,","));
		strcpy(days,strtok(pass_time," "));
		if( strchr(pass_time_day,':') != NULL )//strcspn(pass_time,":"))
		{
			strcpy(hour,strtok(pass_time_day,":"));
			strcpy(minute,strtok(NULL," "));
		}
		else
		{
			strcpy(hour,"0");
			strcpy(minute,strtok(pass_time_day," "));
		}

	}
	else
	{
		strcpy(days,"0");
		if( strchr(pass_time,':') != NULL )//strcspn(pass_time,":"))
		{
			strcpy(hour,strtok(pass_time,":"));
			strcpy(minute,strtok(NULL," "));
		}
		else
		{
			strcpy(hour,"0");
			strcpy(minute,strtok(pass_time," "));
		}
	}

	//*********************************************
	if(start_time != NULL)
	{
		pclose(start_time);
	}
	if(cur_time != NULL)
	{
		pclose(cur_time);
	}
//**************************************************

  cgiHeaderContentType("text/html");
  fprintf(cgiOut,"<html xmlns=\"http://www.w3.org/1999/xhtml\"><head>");
  fprintf(cgiOut,"<meta http-equiv=Content-Type content=text/html; charset=gb2312>");
  fprintf(cgiOut,"<title>Time</title>");
  fprintf(cgiOut,"<link rel=stylesheet href=/style.css type=text/css>"\
  "<style type=text/css>"\
  	".a3{width:30;border:0; text-align:center}"\
  	"</style>"\
  "<script type=\"text/javascript\">");
	fprintf(cgiOut,"function isTime(str)"\
		"{"\
			"if(str!=null&&str!=\"\")"\
			"{"\
				"var a = str.match(/^(\\d{1,2})(:)?(\\d{1,2})\\2(\\d{1,2})$/);"\
				"if (a == null)"\
				"{"\
					"alert(\"%s\");",search(lsystem,"wrongtime"));
					fprintf(cgiOut,"document.all.retime.value = \"\";"\
					"document.all.retime.focus();"\
				"}"\
				"if (a[1]>24 || a[3]>60 || a[4]>60)"\
				"{"\
					"alert(\"%s\");"\
					"document.all.retime.value = \"\";"\
					"document.all.retime.focus();"\
				"}"\
				"return true;"\
			"}"\
	"}",search(lsystem,"timewrong"));

	fprintf(cgiOut,"function isDate(str)\n"\
		"{\n"\
		"	if(str!=null&&str!=\"\")\n"\
		"	{\n"\
		"		var a = str.match(/^((((1[6-9]|[2-9]\\d)\\d{2})-(0[13578]|1[02])-(0[1-9]|[12]\\d|3[01]))|(((1[6-9]|[2-9]\\d)\\d{2})-(0[13456789]|1[012])-(0[1-9]|[12]\\d|30))|(((1[6-9]|[2-9]\\d)\\d{2})-02-(0[1-9]|1\\d|2[0-8]))|(((1[6-9]|[2-9]\\d)(0[48]|[2468][048]|[13579][26])|((16|[2468][048]|[3579][26])00))-02-29-))$/);\n"\
		"		if (a == null)\n"\
		"		{\n"\
		"			alert(\"%s\");\n",search(lsystem,"wrongtime"));
	fprintf(cgiOut,"			document.all.redate.value = \"\";\n"\
				"			document.all.retime.focus();\n"\
		"		}\n"\
		"	return true;\n"\
		"	}\n"\
		"}\n");

  fprintf(cgiOut,"</script>");
  fprintf(cgiOut,"</head>");



  //**************************************************************
  	ccgi_dbus_init();
	instance_parameter *paraHead2 = NULL;
	instance_parameter *p_q = NULL;
	list_instance_parameter(&paraHead2, SNMPD_SLOT_CONNECT);
	if(checkuser_group(addn)==0)/*管理员*/
	{
		cgiFormStringNoNewlines("redate",date,sizeof(date));
		cgiFormStringNoNewlines("retime",time,sizeof(time));
		cgiFormStringNoNewlines("zonestr",zonestr,sizeof(zonestr));
		if(cgiFormSubmitClicked("set_time")==cgiFormSuccess)
		{
			if(strcmp(time,"")==0||strcmp(date,"")==0)
			{
				ShowAlert(search(lsystem,"inserttime"));
			}
			else
			{
				memset(timebuf,0,sizeof(timebuf));
				sprintf(timebuf,"%s %s",date,time);
				
				memset (&tm, 0, sizeof (struct tm));
				buf = strptime(timebuf,"%Y-%m-%d %H:%M:%S",&tm);
				timez = mktime (&tm);
				timediff = timez - 28800;
				ptr=localtime(&timediff);
				snprintf(timecst,sizeof(timecst)-1,"%d/%d/%d %d:%d:%d",ptr->tm_year+1900,ptr->tm_mon+1,ptr->tm_mday,ptr->tm_hour,ptr->tm_min,ptr->tm_sec);
				if(buf)
				{
					memset(cmdbuf,0,sizeof(cmdbuf));
					if(0 == strncmp(zonestr,"utc",3))
			    	{
						sprintf(cmdbuf,"sudo hwclock --set --date '%s'",timebuf);
			    	}
					else if(0 == strncmp(zonestr,"cst",3))
					{
						sprintf(cmdbuf,"sudo hwclock --set --date '%s'",timecst);
			    	}
					for(p_q=paraHead2;(NULL != p_q);p_q=p_q->next)
					{

						if(0 == strncmp(zonestr,"utc",3))
						{
							ac_manage_set_time(p_q->connection,timebuf);
						}
						else if(0 == strncmp(zonestr,"cst",3))
						{
							ac_manage_set_time(p_q->connection,timecst);
						}
						if(p_q->parameter.slot_id == p_masterid)
						{
							system(cmdbuf);
						}
					}
					update_usrtime(encry);
					ShowAlert(search(lsystem,"success"));			
				}
				else
				{
					ShowAlert(search(lpublic,"oper_fail"));	
				}
			}
			cgiFormStringNoNewlines("UN", encry, BUF_LEN); 
			char url_temp[128];
			memset(url_temp,0,128);
			sprintf( url_temp, "wp_showtime.cgi?UN=%s",  encry );
			fprintf( cgiOut, "<script type='text/javascript'>\n" );
			fprintf( cgiOut, "window.location.href='%s';\n", url_temp );
			fprintf( cgiOut, "</script>\n" );
		}
	}
		//**************************************************************
	free_instance_parameter_list(&paraHead2);
  fprintf(cgiOut,"<body>"\
  "<form id=Form1>"\
  "<div align=center>"\
  "<table width=976 border=0 cellpadding=0 cellspacing=0>"\
  "<tr>"\
    "<td width=8 align=left valign=top background=/images/di22.jpg><img src=/images/youce4.jpg width=8 height=30/></td>"\
    "<td width=51 align=left valign=bottom background=/images/di22.jpg><img src=/images/youce33.jpg width=37 height=24/></td>"\
    "<td width=153 align=left valign=bottom id=%s background=/images/di22.jpg>%s</td>",search(lpublic,"title_style"),search(lsystem,"sys_function"));
    fprintf(cgiOut,"<td width=690 align=right valign=bottom background=/images/di22.jpg>");
	   
          fprintf(cgiOut,"<input type=hidden name=UN value=\"%s\">",encry);
    	  fprintf(cgiOut,"<table width=155 border=0 cellspacing=0 cellpadding=0>"\
          "<tr>"\
          "<td width=62 align=center><input id=but type=submit name=set_time style=background-image:url(/images/%s) value=""></td>",search(lpublic,"img_ok"));		  
          fprintf(cgiOut,"<td width=62 align=center><a href=wp_sysmagic.cgi?UN=%s target=mainFrame><img src=/images/%s border=0 width=62 height=20/></a></td>",encry,search(lpublic,"img_cancel"));
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
					  "<td align=left id=tdleft><a href=wp_sysinfo.cgi?UN=%s target=mainFrame class=top><font id=%s>%s</font></a></td>",encry,search(lpublic,"menu_san"),search(lsystem,"sys_infor"));
					fprintf(cgiOut,"</tr>");
					if(checkuser_group(addn)==0)/*管理员*/
					{
					fprintf(cgiOut,"<tr height=25>"\
					  "<td align=left id=tdleft><a href=wp_sysconfig.cgi?UN=%s target=mainFrame class=top><font id=%s>%s</font></a></td>",encry,search(lpublic,"menu_san"),search(lsystem,"sys_config"));
					fprintf(cgiOut,"</tr>"\
					"<tr height=26>"\
					  "<td align=left id=tdleft><a href=wp_impconf.cgi?UN=%s target=mainFrame class=top><font id=%s>%s</font></a></td>",encry,search(lpublic,"menu_san"),search(lsystem,"import_config"));  /*突出显示*/
					fprintf(cgiOut,"</tr>");
					fprintf(cgiOut,"<tr height=25>"\
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


					/*日志信息*/
					fprintf(cgiOut,"<tr height=25>"\
					  "<td align=left id=tdleft><a href=wp_log_info.cgi?UN=%s target=mainFrame class=top><font id=%s>%s</font></a></td>",encry,search(lpublic,"menu_san"),search(lpublic,"log_info"));
					fprintf(cgiOut,"</tr>");

					fprintf(cgiOut,"<tr height=25>"\
                      "<td align=left id=tdleft><a href=wp_login_limit.cgi?UN=%s target=mainFrame class=top><font id=%s>%s</font></a></td>",encry,search(lpublic,"menu_san"),search(lpublic,"l_user"));
                      fprintf(cgiOut,"</tr>");
                      
					}

					
					
					//新增时间条目
					fprintf(cgiOut,"<tr height=26>"\
					  "<td align=left id=tdleft background=/images/bottom_bg.gif style=\"border-right:0\"><font id=%s>%s</font></td>",search(lpublic,"menu_san"),search(lsystem,"systime"));
					fprintf(cgiOut,"</tr>");

					if(checkuser_group(addn)==0)/*管理员*/
						{
			               	//新增NTP条目
						    fprintf(cgiOut,"<tr height=25>"\
						       "<td align=left id=tdleft><a href=wp_ntp.cgi?UN=%s target=mainFrame class=top><font id=%s>%s</font></a></td>",encry,search(lpublic,"menu_san"),search(lpublic,"ntp_s"));
						    fprintf(cgiOut,"</tr>");
							//新增pppoe条目
        					fprintf(cgiOut,"<tr height=25>"\
        					  "<td align=left id=tdleft><a href=wp_pppoe_server.cgi?UN=%s target=mainFrame class=top><font id=%s>%s</font></a></td>",encry,search(lpublic,"menu_san"),"PPPOE");
        					fprintf(cgiOut,"</tr>");
							fprintf(cgiOut,"<tr height=25>"\
        					  "<td align=left id=tdleft><a href=wp_pppoe_snp.cgi?UN=%s target=mainFrame class=top><font id=%s>%s</font></a></td>",encry,search(lpublic,"menu_san"),"PPPOE SNP");
        					fprintf(cgiOut,"</tr>");
						}
					
					//新增时间条目
					fprintf(cgiOut,"<tr height=26>"\
					"<td align=left id=tdleft><a href=wp_webservice.cgi?UN=%s target=mainFrame class=top><font id=%s>%s</font></a></td>",encry,search(lpublic,"menu_san"),search(lpublic,"web_service"));
					fprintf(cgiOut,"</tr>");
					
					 for(i=0;i<5;i++) 
	              {
  				    fprintf(cgiOut,"<tr height=25>"\
                      "<td id=tdleft>&nbsp;</td>"\
                    "</tr>");
	              }
                fprintf(cgiOut,"</table>"\
              "</td>"\
              "<td align=left valign=top style=\"background-color:#ffffff; border-right:1px solid #707070; padding-left:30px; padding-top:10px\">"\
      "<table width=%s border=0 cellspacing=0 cellpadding=0>","80%");
	    fprintf(cgiOut,"<tr>"\
          "<td align='left'>");
	 //MY CODE********************************************************************************************************	
	 		fprintf(cgiOut,"<input type=hidden name=UN value=\"%s\">",encry);//鉴权
			 fprintf(cgiOut,"<table align='left' width='%%70'>"); 
			
			 fprintf(cgiOut,"<tr><td>&nbsp;</td></tr>");
			  fprintf(cgiOut,"<tr>");
			   	 fprintf(cgiOut,"<td align='center'>&nbsp;%s:&nbsp;</td>",search(lsystem,"sys_currenttime"));
			   	 fprintf(cgiOut,"<td align='center' colspan='2'>%s</td>",out_put);
			   fprintf(cgiOut,"</tr>");
			            #if 0
						fprintf(cgiOut,"<tr>"\
							"<td align='center'>%s:</td>",search(lsystem,"loadtime"));
							fprintf(cgiOut,"<td align='center'>");
							fprintf(cgiOut,"%s",days);
							if((strcmp(days,"0")==0)||(strcmp(days,"1")==0))
							{
								fprintf(cgiOut," %s",search(lsystem,"day_one"));
							}
							else
							{
								fprintf(cgiOut," %s",search(lsystem,"day_several"));
							}
							
							fprintf(cgiOut,"%s",hour);
							if((strcmp(hour,"0")==0)||(strcmp(hour," 1")==0))
							{
								fprintf(cgiOut,"%s",search(lsystem,"hour"));
							}
							else
							{
								fprintf(cgiOut,"%s",search(lsystem,"hours"));
							}
							
							if(minute[0]=='0')
							{
								fprintf(cgiOut,"%c",minute[1]);
								if((minute[1]=='1')||(minute[1]=='0'))
								{
									fprintf(cgiOut,"%s</td>",search(lsystem,"minute"));
								}
								else
								{
									fprintf(cgiOut,"%s</td>",search(lsystem,"minutes"));
								}
							}
							else
							{
								fprintf(cgiOut,"%s",minute);
								fprintf(cgiOut,"%s</td>",search(lsystem,"minutes"));
							}
							
						fprintf(cgiOut,	"</tr>");
						#endif
			   	//****************没有权限则隐藏**********************
			   	if(checkuser_group(addn)==0)/*管理员*/
			   	{
			
			   fprintf(cgiOut,"<tr>"\
			   	 "<td align='center'>%s:</td>",search(lsystem,"sys_rebootdate"));
			   	 fprintf(cgiOut,"<td align='center'><input type='text' align='center' id='redate'  name='redate' style='width=120px' onblur=\"isDate(this.value);\"></td>"\
				 	"<td align='left'><font color='red' clospan='3'>YYYY-MM-DD</font></td>"\
			   "</tr>"\
			   "<tr>");
			   	fprintf(cgiOut,"<td align='center'>%s:</td>",search(lsystem,"sys_reboottime"));
				fprintf(cgiOut,"<td align='center'><input type='text' align='center' id='retime' name='retime'  style='width=120px' onblur=\"isTime(this.value);\"></td>"\
					"<td center='center'><font color='red' colspan='3'>HH:MM:SS</font></td>"\
			   "</tr>");
			   fprintf(cgiOut,"<tr>");
			   	fprintf(cgiOut,"<td align='center'></td>");
				fprintf(cgiOut,"<td align='center'><select name=zonestr style='width=120px'><option value=utc>%s</option>"\
					"<option value=cst>%s</option></td>"\
					"<td center='center'></td>"\
			   "</tr>",search(lsystem,"UTC"),search(lsystem,"CST"));
			   fprintf(cgiOut,"</table>");
			   }
			   else
			   {
				fprintf(cgiOut,"</table>");
			   }
		//***************************************************************	
		  
              
			  fprintf(cgiOut,"<tr>"\
			   "<td  id=sec1 style=\"padding-left:23px;width=600; border-bottom:2px solid #53868b;font-size:14px\">%s</td>",search(lpublic,"description"));
			   fprintf(cgiOut,"</tr>");
			

                       fprintf(cgiOut,"<tr height=25 style=\"padding-left:23px; padding-top:2px\">"\
					 "<td style=font-size:14px;color:#FF0000>%s</td>"\
					 "</tr>"\
					  ,search(lsystem,"nowtime"));

                                 
				   fprintf(cgiOut,"<tr height=25 style=\"padding-left:23px; padding-top:2px\">"\
					 "<td style=font-size:14px;color:#FF0000>%s</td>"\
				   "</tr>",search(lsystem,"resettime"));
			 
		
//*********************************************************END


fprintf(cgiOut,"</td>"\
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

release(lpublic);
release(lsystem);
return 0;
}

