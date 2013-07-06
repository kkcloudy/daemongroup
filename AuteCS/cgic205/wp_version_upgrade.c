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
* wp_version_upgrade.c
*
*
* CREATOR:
* autelan.software.Network Dep. team
* zhouym@autelan.com
*
* DESCRIPTION:
* system function for version file upgrade
*
*
*******************************************************************************/
#include <stdio.h>
#include "cgic.h"
#include <string.h>
#include <fcntl.h> 
#include <sys/stat.h> 
#include <stdlib.h>
#include "ws_err.h"
#include "ws_usrinfo.h"
#include "ws_ec.h"
#include "ws_version_param.h"
#include <sys/wait.h>
#include "bsd_dbus.h"

#include "ws_init_dbus.h"
#include <dbus/dbus.h>
#include "ws_dbus_list.h"
#include "ws_webservice_conf.h"


//#include "BsdDbusPath.h"
#define CMD_SUCCESS 0
#define PATH_LEN (64)
#define MAX_SLOT_NUM 16
int HostSlotId = 0;

#define PATH_LEN 64
#define PFM_DBUS_BUSNAME				"pfm.daemon"
#define PFM_DBUS_OBJPATH				"/pfm/daemon"
#define PFM_DBUS_INTERFACE				"pfm.daemon"
#define PFM_DBUS_METHOD_PFM_TABLE 	"pfm_maintain_table"
#define SEM_ACTIVE_MASTER_SLOT_ID_PATH "/dbm/product/active_master_slot_id"
#define SEM_LOCAL_SLOT_ID_PATH    "/dbm/local_board/slot_id"

char BSD_DBUS_BUSNAME[PATH_LEN]=	"aw.bsd";
char BSD_DBUS_OBJPATH[PATH_LEN]=	"/aw/bsd";
char BSD_DBUS_INTERFACE[PATH_LEN]=  "aw.bsd";
char BSD_COPY_FILES_BETEWEEN_BORADS[PATH_LEN]= "bsd_copy_files_between_boards";
char BSD_GET_ALIVE_SLOT_IDS[PATH_LEN] = "bsd_get_alive_slot_ids";

struct dbus_connection{
	DBusConnection *ccgi_dbus_connection;
	int slot_id;
	int board_type;
	int	board_state;
};

typedef struct dbus_connection dbus_connection;

DBusConnection *ccgi_dbus_connection = NULL;
dbus_connection *dbus_connection_dcli[MAX_SLOT_NUM];

DBusConnection *connection = NULL;

//DBusConnection *ccgi_dbus_connection= NULL;
typedef enum bsd_file_type {
    BSD_TYPE_NORMAL = 0,
   	BSD_TYPE_BOOT_IMG = 1,	  	//ac img file
    BSD_TYPE_WTP = 2,			//ap img or configuration files
    BSD_TYPE_PATCH = 3,			//patch
    BSD_TYPE_CORE = 4,			//core file in /proc
	BSD_TYPE_FOLDER = 5,		//normal folder
	BSD_TYPE_SINGLE = 6,		//normal single file
	BSD_TYPE_CMD = 7,			//command
	BSD_TYPE_BLK = 8,
	BSD_TYPE_COMPRESS = 9
}bsd_file_type_t;
static unsigned int master_slot_id;
static unsigned int local_slot_id;

int 
dcli_communicate_pfm_by_dbus(int opt, 
							int opt_para, 
							unsigned short protocol, 
							char* ifname, 
							unsigned int src_port,
							unsigned int dest_port, 
							int slot,
							char* src_ipaddr,
							char* dest_ipaddr,
							unsigned int send_to);


dbus_connection_list snmpd_dbus_connection_list = { 0 };


#define LENG 512
DBusConnection*
 init_slot_dbus_connection(unsigned int slot_id);

int ShowVersionUpgradePage(struct list *lpublic, struct list *lsystem);     /*m代表加密后的字符串*/
int getFileName(char *fpath);     /*获得上传图片的名字fname及全名fpath*/
int freespace();  /*获取磁盘剩余空间大小*/
int webup(struct list *lpublic,char *address,char *cmd,char *encry,char *usrname,char *passwd) ;



//*********
static void ShowAlert_zhou(struct list *lpublic,char * encry){
fprintf(cgiOut,"<script language=javascript>");
fprintf(cgiOut,"if(confirm(\"%s\")){",search(lpublic,"space_cue"));
fprintf(cgiOut,"location.href=\"wp_version_del.cgi?UN=%s\";",encry);
fprintf(cgiOut,"}else{}");
fprintf(cgiOut,"</script>");
}


//**********
int cgiMain()
{
	struct list *lpublic;	/*解析public.txt文件的链表头*/
	struct list *lsystem;	  /*解析system.txt文件的链表头*/	
	lpublic=get_chain_head("../htdocs/text/public.txt");
	lsystem=get_chain_head("../htdocs/text/system.txt");
	//add by wangyan
	ccgi_dbus_init();
	//add by wangyan
	ShowVersionUpgradePage(lpublic,lsystem);
  	release(lpublic);  
  	release(lsystem);

	return 0;
}
static void dcli_web_slot_init()
{
    master_slot_id = get_product_info(SEM_ACTIVE_MASTER_SLOT_ID_PATH);
    local_slot_id = get_product_info(SEM_LOCAL_SLOT_ID_PATH);
	fprintf(stderr,"master_slot_id========%d\n",master_slot_id);
	fprintf(stderr,"local_slot_id========%d\n",local_slot_id);
}
    

static int dcli_web_vhost_show(struct webInfoHead *infohead)
{
    dcli_web_slot_init();

	struct webHostHead vhead;
	webInfoPtr winfo = NULL;

	unsigned int stat, flag;
    unsigned int sum = 0;
	int i , ret;

	for(i = 0; i< 16; i++)
	{
		if(NULL != (connection = init_slot_dbus_connection(i)))
		{
			fprintf(stderr,"connection=================%p\n",connection);
            LINK_INIT(&vhead);
            ret = ac_manage_web_show(connection, &vhead, &stat, &flag);
            if(ret != WEB_FAILURE)
            {
                winfo = (webInfoPtr)malloc(WEB_WEBINFO_SIZE);
                winfo->slotid = i;
                winfo->server_stat = stat;
                winfo->portal_flag = flag;
                winfo->head = vhead;
                LINK_INSERT_HEAD(infohead, winfo, entries);
            }
            if(i == master_slot_id)
            {
                sum  = ret;
            }
        }
    }
    return sum;
}

int ShowVersionUpgradePage(struct list *lpublic, struct list *lsystem)
{ 
 
  char *encry=(char *)malloc(BUF_LEN);      		
  char *str;

  ///结构体存储值
  ST_VERSION_PARAM ver_param,iss_param;
  memset(&ver_param,0,sizeof(ST_VERSION_PARAM));  //存储取到的值
  memset(&iss_param,0,sizeof(ST_VERSION_PARAM));
  
  int i;

  char address1[128];
  char usrname[30];
  char passwd[30];
  memset(passwd,0,30);
  memset(usrname,0,30);
  memset(address1,0,128);
  
  cgiFormStringNoNewlines("url",address1,128);
  cgiFormStringNoNewlines("usrname",usrname,30);
  cgiFormStringNoNewlines("passwd",passwd,30);


	struct webInfoHead infohead;
    webInfoPtr info;
    webHostPtr vh;
    webIfPtr in;
	    LINK_INIT(&infohead);
    char buf[32] = {0};
    int ret, dslot;
    int sum = dcli_web_vhost_show(&infohead);
	if(strncmp(address1,"ftp://",3)==0)
					{
					dcli_communicate_pfm_by_dbus(1, 0, 6, 
                                    in->ifname, 0, 0, info->slotid, buf, "all", in->slot);
					}
				else
					{
					dcli_communicate_pfm_by_dbus(1, 0, 6, 
                                    in->ifname,80, 0, info->slotid, "all", buf, in->slot);
					}
    if(sum < 1)
    {
        //vty_out(vty, "none configuration. falied\n");
        return -1;
    }
    LINK_FOREACH(info, &infohead, entries)
    {
        if(info->slotid == master_slot_id)
        {
			  fprintf(stderr, "info->slotid=====%d\n",info->slotid);
            //if(info->server_stat&WEB_SERVICE_ENABLE)
           // {
            //    web_list_flush(&infohead);
               // vty_out(vty, "is running. failed\n");
           //     return -1;
          //  }

            dslot = info->slotid;

           // ret = ac_manage_web_conf(dbus_connection_dcli[dslot]->dcli_dbus_connection, WEB_START);

            LINK_FOREACH(vh, &(info->head), entries)
            {
                if(vh->type == HTTP_SERVICE || vh->type == HTTPS_SERVICE)
                {
                    if(!LINK_EMPTY(&vh->head))
                    {
                        LINK_FOREACH(in, &(vh->head), entries)
                        {
                            memset(buf, 0, sizeof(buf));
                            sprintf(buf, "%s/32", vh->address);
							if(strncmp(address1,"ftp://",3)==0)
								{ 
									fprintf(stderr,"strncmp(address1,"")====%d\n",strncmp(address1,"ftp://",3));
									dcli_communicate_pfm_by_dbus(0, 0, 6, 
                                    in->ifname, 0, 0, info->slotid, "all", buf, in->slot);
									//dcli_communicate_pfm_by_dbus(1, 0, 6, 
                                   // in->ifname, 0, 0, info->slotid, buf, "all", in->slot);
								}
							
                           	else
                           		{
                           		 fprintf(stderr,"898989898989898989898\n");
                           			dcli_communicate_pfm_by_dbus(0, 0, 6, 
                                    in->ifname,80, 0, info->slotid, "all", buf, in->slot);
								//	dcli_communicate_pfm_by_dbus(1, 0, 6, 
                                  //  in->ifname,80, 0, info->slotid, "all", buf, in->slot);
                           		}
							   
							//fprintf(stderr, "in->ifname=====%s\n",in->ifname);
							//fprintf(stderr, "vh->port=====%u\n",vh->port);
							//fprintf(stderr, "info->slotid=====%d\n",info->slotid);
							//fprintf(stderr, "buf=====%s\n",buf);
							//fprintf(stderr, "in->slot=====%d\n",in->slot);
                        }
                    }
                }
            }
        }
    }

    //web_list_flush(&infohead);  
	//dcli_communicate_pfm_by_dbus(0, 0, 6,"eth2-1", 80, 0, 1, "all","192.168.122.190/32",2);
	
  char cmd[128];
  memset(cmd,0,sizeof(cmd));
  sprintf(cmd,"wget -N -P /mnt --user=%s --password=%s %s > /dev/null",usrname,passwd,address1);  //下载到指定目录，并保持最新版本
 fprintf(stderr,"cmd========================%s\n",cmd);
    memset(encry,0,BUF_LEN);
    cgiFormStringNoNewlines("UN", encry, BUF_LEN); 
    str=dcryption(encry);
    if(str==NULL)
    {
      ShowErrorPage(search(lpublic,"ill_user"));          /*用户非法*/
      return 0;
    }

  //cgiFormStringNoNewlines("UN",encry,BUF_LEN);


  
  /***********************2008.5.26*********************/
  cgiHeaderContentType("text/html");

  fprintf(cgiOut,"<html xmlns=\"http://www.w3.org/1999/xhtml\"><head>");
  fprintf(cgiOut,"<meta http-equiv=Content-Type content=text/html; charset=gb2312>");
  fprintf(cgiOut,"<title>%s</title>",search(lpublic,"version_up"));
  fprintf(cgiOut,"<link rel=stylesheet href=/style.css type=text/css>\n"\
  	"<style type=text/css>\n"\
  	".a3{width:30;border:0; text-align:center}\n"\
  	"</style>\n"\
    "</head>\n");
   fprintf(cgiOut,"<script src=/probar.js></script>");
   fprintf(cgiOut,"<script type=text/javascript>"\
     "function check_sysinfo_div_pos()\n"\
     "{"\
     "	    var   oRect   =   document.all.only.getBoundingClientRect();\n"\
     "  var obj_div=document.getElementById('only');\n"\
     "  obj_div.style.top = oRect.top+3;\n"\
     "  obj_div.style.left = oRect.left+204;\n"\
     "}\n"\
     "</script>"\
     "<script type=\"text/javascript\">\n"\
     "function HandleFileButtonClick(obj)\n"\
     "{\n" \
     "   with(obj){\n"\
     "	    var   oRect   =   document.all.fakeButton.getBoundingClientRect();\n"\
     "     style.posTop=oRect.top+3;\n"\
     "     style.posLeft=oRect.left+2;\n"\
     "	  }\n"\
     "}\n"\
     "</script>\n");  
  fprintf(cgiOut,"<body onResize=check_sysinfo_div_pos()>\n");

	//if(cgiFormSubmitClicked("upload_file") != cgiFormSuccess)
	//{   
		//本地上传按钮
		
//		if(cgiFormSubmitClicked("local_upload") == cgiFormSuccess && cgiFormSubmitClicked("web_upload") != cgiFormSuccess )
//		{	  
			//locup(lpublic,lsystem,encry);//已交由up.cgi处理
//		} 
		//网络资源上传
		if((cgiFormSubmitClicked("local_upload") != cgiFormSuccess && (cgiFormSubmitClicked("web_upload") == cgiFormSuccess))|| ((strcmp(address1,"")!=0)&&(strchr(address1,' ')==NULL)))
		{ 
		    if((strcmp(address1,"")!=0) && (strcmp(usrname,"")!=0) && (strcmp(passwd,"")!=0))
		    {
				webup(lpublic,address1,cmd,encry,usrname,passwd);
				#if 0
				if(strncmp(address1,"ftp://",3)==0)
					{
					dcli_communicate_pfm_by_dbus(1, 0, 6, 
                                    in->ifname, 0, 0, info->slotid, buf, "all", in->slot);
					}
				else
					{
					dcli_communicate_pfm_by_dbus(1, 0, 6, 
                                    in->ifname,80, 0, info->slotid, "all", buf, in->slot);
					}
				#endif
		    }
			else
			{
               if(strcmp(address1,"")==0)
			   	  ShowAlert(search(lpublic,"url_not_empty"));
			   else if(strcmp(usrname,"")==0)
			   	  ShowAlert(search(lsystem,"userna_err"));
			   else if(strcmp(passwd,"")==0)
			   	  ShowAlert(search(lsystem,"pass_err"));
			}
		}
		
	//}             
	#if 0
	else
	{  //可进行条件判断了   

		//本地上传按钮
		if(cgiFormSubmitClicked("local_upload") == cgiFormSuccess)
		{   
			locup(lpublic,lsystem,encry);
		}
		else
		{

			char *fpath = (char *)malloc(PATH_LENG);
			int ret_fpath;
			memset(fpath,0,PATH_LENG);
			ret_fpath=getFileName(fpath);  //获得本地文件名
			if(ret_fpath==1)
			{
				int ret_upfile;
				ret_upfile=upfile_v(fpath,lpublic,lsystem);
				if(ret_upfile==1)
				ShowAlert(search(lpublic,"lupload_succ"));
				else if(ret_upfile==0)
				ShowAlert(search(lpublic,"lupload_fail"));
			}

		}


	//* * * * * * * * * * * * * * * *  * * * * * * * * *  

	//网络资源上传
	if(cgiFormSubmitClicked("web_upload") == cgiFormSuccess|| ((strcmp(address,"")!=0)&&(strchr(address,' ')==NULL)))
	{ 
		webup(lpublic,address,cmd,encry);
	}
	else
	{
		if(strcmp(address,"")!=0)
		{
				webup(lpublic,address,cmd,imp_encry);		
		}

	}   	
} 
 #endif
  fprintf(cgiOut,"<form method=post encType=multipart/form-data name=frmUpload action=\"up.cgi?UN=%s\">", encry);
 fprintf(cgiOut,
  "<div align=center>"\
  "<table width=976 border=0 cellpadding=0 cellspacing=0>"\
  "<tr>"\
  "<td width=8 align=left valign=top background=/images/di22.jpg><img src=/images/youce4.jpg width=8 height=30/></td>"\
  "<td width=51 align=left valign=bottom background=/images/di22.jpg><img src=/images/youce33.jpg width=37 height=24/></td>"\
  "<td width=153 align=left valign=bottom id=%s background=/images/di22.jpg>%s</td>",search(lpublic,"title_style"),search(lsystem,"sys_function"));
    fprintf(cgiOut,"<td width=690 align=right valign=bottom background=/images/di22.jpg>");
    fprintf(cgiOut,"<input type=hidden name=UN value=%s />",encry);
 
    fprintf(cgiOut,"<table width=130 border=0 cellspacing=0 cellpadding=0>"\
	"<tr>");
    fprintf(cgiOut,"<td width=62 align=left><a href=wp_version_upgrade.cgi?UN=%s target=mainFrame><img src=/images/%s border=0 width=62 height=20/></a></td>",encry,search(lpublic,"img_ok"));
    fprintf(cgiOut,"<td width=62 align=left><a href=wp_sysmagic.cgi?UN=%s target=mainFrame><img src=/images/%s border=0 width=62 height=20/></a></td>",encry,search(lpublic,"img_cancel"));
	fprintf(cgiOut,"</tr>"\
	"</table>");
 
 
  
//  以上是关于中英文的 不同页面了

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

						fprintf(cgiOut,"<tr height=25>"\
						"<td align=left id=tdleft><a href=wp_impconf.cgi?UN=%s target=mainFrame class=top><font id=%s>%s</font></a></td>",encry,search(lpublic,"menu_san"),search(lsystem,"import_config"));
						fprintf(cgiOut,"</tr>");

						fprintf(cgiOut,"<tr height=25>"\
						"<td align=left id=tdleft><a href=wp_export.cgi?UN=%s target=mainFrame class=top><font id=%s>%s</font></a></td>",encry,search(lpublic,"menu_san"),search(lsystem,"export_config"));
						fprintf(cgiOut,"</tr>");

						fprintf(cgiOut,"<tr height=26>"\
						"<td align=left id=tdleft background=/images/bottom_bg.gif style=\"border-right:0\"><font id=%s>%s</font></td>",search(lpublic,"menu_san"),search(lpublic,"version_up"));  /*突出显示*/
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
						//新增时间条目
						fprintf(cgiOut,"<tr height=25>"\
						"<td align=left id=tdleft><a href=wp_showtime.cgi?UN=%s target=mainFrame class=top><font id=%s>%s</font></a></td>",encry,search(lpublic,"menu_san"),search(lsystem,"systime"));
						fprintf(cgiOut,"</tr>");

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
					
				for(i=0;i<4;i++)
				{
				  fprintf(cgiOut,"<tr height=25>"\
					"<td id=tdleft>&nbsp;</td>"\
				  "</tr>");
				}
			  fprintf(cgiOut,"</table>"\
			  "</td>");
			  
            get_version_param(&ver_param,V_XML);  //从xml文件中获得消息			  
			fprintf(cgiOut,"<td align=left style=\"background-color:#ffffff; border-right:1px solid #707070; padding-left:30px\">"\
                "<table width=700 border=0 cellspacing=0 cellpadding=0>");
			 ////local upload
			fprintf(cgiOut,"<tr height=20>"\
				"<td align=left width=70>"\
				"%s:</td><td align=left><input type=\"file\" size=\"30\" name=\"myFile\" id=only style=\"position:absolute;filter:alpha(opacity=0);width:30px;\" onchange='document.frmUpload.txtFakeText.value = this.value;' value=\"\" />",search(lpublic,"local_upload"));
			if(strcmp(ver_param.sendtype,UP_TYPE_L)==0)				 
				  fprintf(cgiOut,"<input type=text size=\"32\" name=txtFakeText value=\"%s\" disabled/>",ver_param.routeip);
			else
				  fprintf(cgiOut,"<input type=text size=\"32\" name=txtFakeText value=\"\" disabled/>");

              fprintf(cgiOut,"<input type=button style=\"width:70px;\"   name=fakeButton onmouseover=\"HandleFileButtonClick(document.frmUpload.myFile);\" value=\"%s\"></p>",search(lpublic,"browse"));
			 fprintf(cgiOut,"<td width=300><input type=submit value=\"%s\" onclick=\"test('%s')\"></td>",search(lpublic,"local_upload"),search(lpublic,"locup_warn"));	
			  //fprintf(cgiOut,"<td width=300><input type=submit value=\"%s\" ></td>",search(lpublic,"local_upload"),search(lpublic,"locup_warn"));	
              fprintf(cgiOut,"</tr>");			  
			  fprintf(cgiOut,"</div></form>");
			  //有问题


			  
			  fprintf(cgiOut,"<form method=post encType=multipart/form-data>");
			  fprintf(cgiOut,"<input type=hidden name=UN value=%s />",encry);

			  ////////web upload  url
			  get_version_param(&iss_param,ISS_XML);  //从xml文件中获得消息		
			   fprintf(cgiOut,"<tr height=25>");				
			   fprintf(cgiOut,"<td align=left width=70>");
			   if(strcmp(iss_param.sendtype,UP_TYPE_W)==0)
				   fprintf(cgiOut,"%s:</td><td align=left><input type=text size=\"40\" name=url value=\"%s\"></td>",search(lpublic,"web_source"),iss_param.routeip);	
			   else
				   fprintf(cgiOut,"%s:</td><td align=left><input type=text size=\"40\" name=url value=\"%s\"></td>",search(lpublic,"web_source"),address1);	
			   fprintf(cgiOut,"<td width=300><input type=submit name=web_upload value=\"%s\" onclick=\"test('%s')\"></td>",search(lpublic,"web_up"),search(lpublic,"webup_warn"));	
			   fprintf(cgiOut,"</tr>");

               ///usrname
			   fprintf(cgiOut,"<tr height=25>\n");
			   fprintf(cgiOut,"<td align=left width=70>%s:</td><td colspan=2>",search(lsystem,"user_na"));
			   fprintf(cgiOut,"<input type=text name=usrname size=22 style=width:200px;></td>\n");
			   fprintf(cgiOut,"</tr>\n");

               //password
			   fprintf(cgiOut,"<tr height=25>\n");
			   fprintf(cgiOut,"<td align=left width=70>%s:</td><td colspan=2>",search(lsystem,"password"));
			   fprintf(cgiOut,"<input type=password name=passwd size=22 style=width:200px;></td>\n");
			   fprintf(cgiOut,"</tr>\n");

			   fprintf(cgiOut,"<tr height=10><td colspan=3></td></tr>");		
			   fprintf(cgiOut,"<tr height=10><td colspan=3><a href=wp_version_del.cgi?UN=%s><font color=blue size=2>&nbsp;&nbsp;&nbsp;%s</font></a></td></tr>",encry,search(lpublic,"view_version"));	
			   fprintf(cgiOut,"<tr><td colspan=3><input type=hidden name=encry_import value=%s></td></tr>",encry);
			   fprintf(cgiOut,"<tr>"\
			   "<td colspan=3 id=sec1 style=\"padding-left:23px;width=600; border-bottom:2px solid #53868b;font-size:14px\">%s</td>",search(lpublic,"description"));
			   fprintf(cgiOut,"</tr>");
			
				   fprintf(cgiOut,"<tr height=25 style=\"padding-left:23px; padding-top:2px\">"\
					 "<td colspan=3 style=font-size:14px;color:#FF0000>%s</td>"\
					 "</tr>"\
					 "<tr height=25 style=\"padding-left:23px; padding-top:2px\">"\
					 "<td colspan=3 style=font-size:14px;color:#FF0000>%s</td>"\
				   "</tr>",search(lpublic,"loc_des"),search(lpublic,"web_des"));
				   fprintf(cgiOut,"<tr height=25 style=\"padding-left:23px; padding-top:2px\">"\
					 "<td colspan=3 style=font-size:14px;color:#FF0000>%s</td>"\
				   "</tr>",search(lpublic,"view_des"));
              
			   
fprintf(cgiOut,"<tr height=50></tr>"\
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
"<script type=text/javascript>"\
"check_sysinfo_div_pos()"\
"</script>"\
"</html>");
free(encry); 
return 0;
}          



int getFileName(char *fpath)        /*返回0表示失败，返回1表示成功*/
{
  cgiFilePtr file;
  char name[1024];    /*存放本地路径名*/ 
  char *tmpStr=NULL;   

  int t;
  if (cgiFormFileName("myFile", name, sizeof(name)) != cgiFormSuccess) 
    return 0;
  
  if (cgiFormFileOpen("myFile", &file) != cgiFormSuccess) 
  {
	fprintf(stderr, "Could not open the file.<p>\n");
    return 0;
  }
  t=-1; 
  //从路径名解析出用户文件名 
  while(1){ 
  tmpStr=strstr(name+t+1,"\\");   /*在name+t+1中寻找"\\"*/
  if(NULL==tmpStr)                /*成功返回位置，否则返回NULL*/
  tmpStr=strstr(name+t+1,"/");    /*  if "\\" is not path separator, try "/"   */
  if(NULL!=tmpStr) 
    t=(int)(tmpStr-name);         /*如果找到，t存储偏移地址*/       
  else 
    break;  
  } 
  strcpy(fpath,name+t+1);     /*将文件全名赋给fpath*/ 

  #if 0
  end=strstr(fpath,".");		 /*在fpath中寻找"." */
  if(end != NULL)				 /*成功返回位置，否则返回NULL*/
  {
	t=(int)(end-fpath); /*如果找到，t存储偏移地址*/
	t=t+25;
	for(i=0;i<t;i++)
	  fname[i]=fpath[i];
	fname[i]='\0';
  }   
#endif
  
  return 1;
}


 //关于查看剩余空间
int  freespace()  
{

FILE *fp;
char buff[128];

fp=popen("df /mnt|grep /mnt","r");
if(fp != NULL)
{
	fgets( buff, sizeof(buff), fp );  //很重要 ，不然与条目不匹配
}


char input[128];
memset(input,0,128);
strcpy(input,buff);

char *p;
int i,j,m;

p=strtok(input," ");
if(p)

p=strtok(NULL," ");
i=atoi(p);

p=strtok(NULL," ");
j=atoi(p);

m=(i-j)/1000;
///fprintf(cgiOut,"剩余空间:%dM",m);   //中英文的对照

if(m<36){
	return 1;
}

 if(fp != NULL)
 {
	 pclose(fp);  //别忘记关闭了
 }
 
 return 0;	

}
 int dbus_connection_remote_init(dbus_connection** connection)
{
	if(NULL != (*connection))
	{
	//	free(*connection);
		(*connection) = NULL;
	}

	(*connection) = (dbus_connection*)malloc(sizeof(dbus_connection));
	
	if(NULL == (*connection))
	{
		//syslog(LOG_INFO,"malloc error\n");
		return -1;
	}

	(*connection) -> ccgi_dbus_connection	= NULL;
	(*connection) -> slot_id				= -1;
	(*connection) -> board_type				= -1;
	(*connection) -> board_state			= -1;

}
 int dbus_connection_init_all(dbus_connection** dbus_connection)
{
	int i = 0;
	if(NULL == dbus_connection)
	{
		//syslog(LOG_INFO,"ERROR:dbus_connection_init_all:dbus_connection = NULL\n");
		return -1;
	}

	for(i = 0;i < 4;i++)
	{
		if(0 == dbus_connection_remote_init(&dbus_connection[i]))
		{
			//syslog(LOG_INFO,"ERROR:dbus_connection_init_all:init connection %d error\n",i);
			return -1;
		}
	}
	return 0;
}
 
 int dbus_connection_register(int slot_id,dbus_connection** connection)
 {
	 
	 DBusError dbus_error;	 
	 dbus_error_init (&dbus_error);
	 
	 if(slot_id > 4)
	 {
		// syslog(LOG_INFO,"ERROR:dbus_connection_register:error slot_id\n");
		 return -1;
	 }
	 
	 if((*connection) == NULL)
	 {
		 //syslog(LOG_INFO,"ERROR:dbus_connection_register:connection is NULL\n");
		 return -1;
	 }
 
	 (*connection) -> slot_id			 = slot_id;
	 (*connection) -> board_type		 = -1;
	 (*connection) -> board_state			 = -1;
	 (*connection) -> ccgi_dbus_connection	 = dbus_bus_get_remote(DBUS_BUS_SYSTEM,slot_id,&dbus_error);
 
	 if((*connection) -> ccgi_dbus_connection == NULL)
	 {
		 //syslog(LOG_INFO,"dbus_bus_get(): %s", dbus_error.message);
		 return -1;
		 
	 }
	 
	 return 0;
	 
	 
 }
	int dbus_connection_register_all(dbus_connection** dbus_connection)
{
	int i = 0;	
	int fd;	
	int product_serial = 0;	
	int max_number = 0;

	
	fd = fopen("/dbm/product/product_serial", "r");
	fscanf(fd, "%d", &product_serial);
	fclose(fd);

	if(8 == product_serial)
	{
		max_number = 16;
	}
	else if(7 == product_serial)
	{
		max_number = 3;
	}

	
	if(NULL == dbus_connection)
	{
		//syslog(LOG_INFO,"ERROR:dbus_connection_register_all:dbus_connection = NULL\n");
		return -1;
	}
	for(i = 1;i <= max_number;i++)
	{
		
		//syslog(LOG_INFO,"\n===============connect slot %d ===================\n",i);
		if(-1 == dbus_connection_register(i,&dbus_connection[i]))
		{
			//syslog(LOG_INFO,"ERROR:dbus-connection_register_all:connect slot %d error\n",i);
			continue;
		}
	}
	return 0;
}
 int dcli_dbus_init_remote(void) {
	 if(dbus_connection_init_all(dbus_connection_dcli) == -1)
	 {
		 return FALSE;
	 }
 
	 if(dbus_connection_register_all(dbus_connection_dcli) == -1)
	 {
		 return FALSE;
	 }
	 return TRUE;
 }
	 
 
 
 void
 dbus_error_free_for_dcli(DBusError *error)
 {
	 if (dbus_error_is_set(error)) {
		// syslog(LOG_NOTICE,"dbus connection of dcli error ,reinit it\n");
		 
		 //syslog(LOG_NOTICE,"%s raised: %s\n",error->name,error->message);
		 dcli_dbus_init_remote();
		 
	 }
	 dbus_error_free(error);
	 
 
 
	 
	 }
 
 DBusConnection*
 init_slot_dbus_connection(unsigned int slot_id) {
 
	 if(slot_id == snmpd_dbus_connection_list.local_slot_num) {
		 //syslog(LOG_DEBUG, "This connection is local , don`t need init it\n");
		 return ccgi_dbus_connection;
	 }
 
	 struct list_head *pos = NULL;
	 list_for_each(pos, &snmpd_dbus_list_head) {
		 netsnmp_dbus_connection *snmpd_dbus_node = dbus_node(pos);
		 if(slot_id == snmpd_dbus_node->slot_id) {
 
			 if(snmpd_dbus_node->connection) {
				 snmpd_close_dbus_connection(&(snmpd_dbus_node->connection));
			 }
 
			 snmpd_dbus_node->connection = dbus_get_tipc_connection(slot_id);
 
			 return snmpd_dbus_node->connection;
		 }
	 }
 
	 return NULL;
 }
 int 
 dcli_communicate_pfm_by_dbus(int opt, 
							 int opt_para, 
							 unsigned short protocol, 
							 char* ifname, 
							 unsigned int src_port,
							 unsigned int dest_port, 
							 int slot,
							 char* src_ipaddr,
							 char* dest_ipaddr,
							 unsigned int send_to)
 
 {
	 DBusMessage *query, *reply;
	 DBusError err;
	 unsigned int op_ret;
	 /*
	 long src_ipaddr1,src_ipaddr2,dest_ipaddr1,dest_ipaddr2;
	 memcpy(&src_ipaddr1,&src_ipaddr,sizeof(src_ipaddr1));
	 memcpy(&src_ipaddr2,(&src_ipaddr) + sizeof(src_ipaddr1),sizeof(src_ipaddr2));
	 memcpy(&dest_ipaddr1,(&dest_ipaddr),sizeof(dest_ipaddr1));
	 memcpy(&dest_ipaddr2,(&dest_ipaddr) + sizeof(dest_ipaddr1),sizeof(dest_ipaddr2));
	 */
#if 0
	 fprintf(stderr,"DCLI send data to PFM are :\n");
			 fprintf(stderr," opt is %d ....\n",opt);
			 fprintf(stderr," protocol is %u ....\n",protocol);
			 fprintf(stderr," ifindex is %s ....\n",ifname);
			 fprintf(stderr," port is %d ....\n",dest_port);
			 fprintf(stderr," send to is %d ....\n",send_to);
			 fprintf(stderr," slot is %d ....\n",slot);
			 fprintf(stderr," ipaddr is %u ....\n",dest_ipaddr);
	#endif
	 if(slot == send_to)
	 {
		 return -1;
		 				
	 }
	 
	 query = dbus_message_new_method_call(
								 PFM_DBUS_BUSNAME,			 \
								 PFM_DBUS_OBJPATH,		 \
								 PFM_DBUS_INTERFACE, \
								 PFM_DBUS_METHOD_PFM_TABLE);
	 
	 dbus_error_init(&err);
 
	 dbus_message_append_args(query,
							 DBUS_TYPE_INT32, &opt,
							 DBUS_TYPE_INT32, &opt_para,
							 DBUS_TYPE_UINT16, &protocol,
							 DBUS_TYPE_STRING, &ifname,
							 DBUS_TYPE_UINT32, &src_port,
							 DBUS_TYPE_UINT32, &dest_port,
							 DBUS_TYPE_STRING, &src_ipaddr,
							 DBUS_TYPE_STRING, &dest_ipaddr,	 
							 DBUS_TYPE_INT32,  &slot,
							 DBUS_TYPE_INVALID);
 	fprintf(stderr,"~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~\n");
	 if(-1 == send_to)
	 {
		 int i;
		 for(i = 0;i < 16 ; i++)
		 {
			 if( NULL != (connection = init_slot_dbus_connection(i)))
			 	{
				 reply = dbus_connection_send_with_reply_and_block (connection, query, -1, &err);
				 if (NULL == reply) {
					 fprintf(stderr,"[%d]failed get args.\n",i);
					 if (dbus_error_is_set(&err)) {
						 fprintf(stderr,"%s raised: %s\n",err.name,err.message);
						 dbus_message_unref(query);
						 dbus_error_free_for_dcli(&err);
						 return -1;
					 }
				 }
				 
				 if (dbus_message_get_args ( reply, &err,
											 DBUS_TYPE_UINT32,&op_ret,
											 DBUS_TYPE_INVALID)) 
				 {
					 //vty_out(vty,"DCLI recv [%d] reply is :%s\n",i,op_ret == 0?"OK":"ERROR");
				 } 
				 else {
					 fprintf(stderr,"Failed get args.\n");
					 if (dbus_error_is_set(&err)) {
						 fprintf(stderr,"%s raised: %s",err.name,err.message);
						 dbus_error_free_for_dcli(&err);
					 }
				 }
				 
 // 			 dbus_message_unref(reply);
			 }
 
		 }
		 dbus_message_unref(query);
		 
		 dbus_message_unref(reply);
 
		 dbus_error_free_for_dcli(&err);
		 
	 }else{
	 // fprintf(stderr,"dbus_connection_dcli ====%p\n",dbus_connection_dcli);
	  /// fprintf(stderr,"dbus_connection_dcli[i] ====%p\n",dbus_connection_dcli[0]);
	 //	 fprintf(stderr,"dbus_connection_dcli[i] ====%p\n",dbus_connection_dcli[send_to]);
		 if(NULL != (connection = init_slot_dbus_connection(send_to)))
		 {
			// fprintf(stderr,"dbus_connection_dcli[i] -> ccgi_dbus_connection====%p\n",dbus_connection_dcli[send_to] -> ccgi_dbus_connection);
			 reply = dbus_connection_send_with_reply_and_block (connection, query, -1, &err);
			 if (NULL == reply){
				 fprintf(stderr,"failed get args. \n");
				 if (dbus_error_is_set(&err)){
					 fprintf(stderr,"%s raised: %s\n",err.name,err.message);
					 dbus_message_unref(query);
					 dbus_error_free_for_dcli(&err);
					 return -1;
				 }
			 }
			 
			 dbus_message_unref(query);
		 }else{
			 fprintf(stderr,"connection of board %d is not exist\n",send_to);
			 return -1;
		 }
	 
		 
			 
 //  #if 0
	 if (dbus_message_get_args ( reply, &err,
								 DBUS_TYPE_UINT32,&op_ret,
								 DBUS_TYPE_INVALID)) 
	 {
		// vty_out(vty,"DCLI recv reply is :%s\n",op_ret == 0?"OK":"ERROR");
	 } 
	 else {
		 fprintf(stderr,"Failed get args.\n");
		 if (dbus_error_is_set(&err)) {
			 fprintf(stderr,"%s raised: %s",err.name,err.message);
			 dbus_error_free_for_dcli(&err);
		 }
	 }
	 
	 dbus_message_unref(reply);
 //  #endif
	 }
	 return 0;
 }




//网络上传函数
int webup(struct list *lpublic,char *address,char *cmd,char *encry,char *usrname,char *passwd) 
{
	
	char sys_cmd[128];
	ST_VERSION_PARAM new_param;
	memset(&new_param,0,sizeof(ST_VERSION_PARAM));  //存储得到的值

	char *tempf =(char *)malloc(128);
	char* filename=strrchr(address,'/');
	//20,21
	
	
	//dcli_communicate_pfm_by_dbus(0, 0, 6,"eth2-1", 20, 0, 1, "all","192.168.122.190/32",2);
	
	
	int ret,op_ret,status=0;
	fprintf(stderr,"ssssssssssssssssssssstttttttttttttyy\n");
	
	ret=system(cmd);
	fprintf(stderr,"ret===========1========!!!!!!!!!!!!!!!%d\n",ret);
	//操作写到xml文件中去
	strcpy(new_param.routeip,address);   //远程url
	strcpy(new_param.routeport,"80");  //远程端口
	strcpy(new_param.protocal,T_HTTP);  //协议类型
	strcpy(new_param.sendtype,UP_TYPE_W); //操作类型
	strcpy(new_param.filetype,T_IMG); //版本文件
	strcpy(new_param.filename,filename+1);
	strcpy(new_param.username,usrname);
	strcpy(new_param.password,passwd);
	fprintf(stderr,"ret==========2=========!!!!!!!!!!!!!!!%d\n",ret);

	if(ret==0)	 
	{
		fprintf(stderr,"ret=======3============!!!!!!!!!!!!!!!%d\n",ret);
		memset(tempf,0,128);
		if(filename)
		{
			sprintf(tempf,"sudo sor.sh cp %s %d > /dev/null 2>&1",filename+1,LONG_SORT);
			status = system(tempf);
			op_ret = WEXITSTATUS(status);
			fprintf(stderr,"usrname========%s\n",usrname);
			fprintf(stderr,"usrname========%s\n",passwd);
			//set boot_img version
			if(op_ret==0)
			{

				memset(sys_cmd,0,128);
				sprintf(sys_cmd,	
					"source vtysh_start.sh >/dev/null 2>&1\n"
					"vtysh -c \"show boot_img\" |grep '^%s$' >/dev/null"
					,filename+1);
				status = system(sys_cmd);
				ret = WEXITSTATUS(status);
				fprintf(stderr, "fname======tttttttttttttt================%s\n",filename+1);
				fprintf(stderr, "ret======================%d\n",ret);
				if(ret==0)
					
					{
						fprintf(stderr, "fname======jjjjjjjjjj================%s\n",filename+1);
						char src_path[PATH_LEN] = {0};
						char des_path[PATH_LEN] = {0};
						sprintf(src_path, "/blk/%s", filename+1);
						sprintf(des_path, "/blk/%s", filename+1);
	       				int ID[MAX_SLOT_NUM] = {0};
						int i =0;
						//printf("count = %d\n",board_count);
						int board_count = -1;
						 board_count =ccgi_dcli_bsd_get_slot_ids_web(ccgi_dbus_connection,ID,BSD_TYPE_BOOT_IMG);
						  fprintf(stderr, "board_count======================%d\n",board_count);
						for(i = 0; i < board_count; i++)
						{
						
						 ccgi_dcli_bsd_copy_file_to_board_web(ccgi_dbus_connection,ID[i],src_path,des_path,0,BSD_TYPE_BLK);
						memset(tempf,0,128);
						sprintf(tempf,"sudo boot.sh  %s >/dev/null 2>&1",filename+1);		
						system(tempf); 
						}
					}
				
				//memset(tempf,0,128);
				//sprintf(tempf,"sudo boot.sh  %s >/dev/null 2>&1",filename+1);		
				//system(tempf);  
				fprintf(stderr,"ShowAlertShowAlertShowAlertShowAlert//ShowAlert//ShowAlert\n");
				ShowAlert(search(lpublic,"wupload_succ"));
			}
			else if(op_ret==5)
				ShowAlert_zhou(lpublic,encry);	
			else
			{
				strcpy(new_param.failcause,"address cannot be reached"); 
				ShowAlert(search(lpublic,"wupload_fail"));
			}	

		}  
		else 
		{
			ShowAlert(search(lpublic,"wupload_fail"));
		}				
	}
	else
	{
		strcpy(new_param.failcause,"address cannot be reached"); 
		ShowAlert(search(lpublic,"wupload_fail"));
	}
	set_version_param(new_param,ISS_XML);
	free(tempf);
	return 0;

}





int ccgi_dcli_bsd_get_slot_ids_web(DBusConnection *connection, int *ID, const int op)
{
    int ret = 0;
    DBusMessageIter  iter;
    DBusMessage *query = NULL;
    DBusMessage *reply = NULL;
    DBusError err = {0};
	int i = 0;
	int slot_id = 0;
   
	fprintf(stderr, "****=====******ccgi_dcli_bsd_get_slot_ids*****======*********");
    query = dbus_message_new_method_call(BSD_DBUS_BUSNAME, BSD_DBUS_OBJPATH, \
        BSD_DBUS_INTERFACE, BSD_GET_ALIVE_SLOT_IDS);

    dbus_message_append_args(query,
    					DBUS_TYPE_UINT32,&op,
    					DBUS_TYPE_INVALID);

    reply = dbus_connection_send_with_reply_and_block (connection,query,-1, &err);

    dbus_message_unref(query);

    if (NULL == reply) {
		if (dbus_error_is_set(&err)) {
			dbus_error_free(&err);
		}
		return -1;
	}
	
	dbus_message_iter_init(reply,&iter);
	dbus_message_iter_get_basic(&iter,&ret);
	//printf("ret = %d\n",ret);

	if(ret != 0){
		for(i = 0; i < ret; i++)
		{
		    dbus_message_iter_next(&iter);
	        dbus_message_iter_get_basic(&iter,&(ID[i]));
		}
	}

    return ret;
}

int ccgi_dcli_bsd_copy_file_to_board_web(DBusConnection *connection, const int slot_id, const char *src_path, const char *des_path, const int flag, const int op)
{   
    int ret = 0;
    DBusMessage *query = NULL;
    DBusMessage *reply = NULL;
    DBusError err = {0};
	int retu = 0;
    fprintf(stderr, "*****----------*****ccgi_dcli_bsd_copy_file_to_board********--------------******");
    char *tmp_src_path = src_path;
    char *tmp_des_path = des_path;
    
    //printf("dbus_name = %s\ndbus_objpath = %s\ndbus_interface = %s\n",BSD_DBUS_BUSNAME,BSD_DBUS_OBJPATH,BSD_DBUS_INTERFACE);
    query = dbus_message_new_method_call(BSD_DBUS_BUSNAME,BSD_DBUS_OBJPATH,\
						BSD_DBUS_INTERFACE,BSD_COPY_FILES_BETEWEEN_BORADS);
    
	dbus_error_init(&err);
    //printf("11111\n");
	dbus_message_append_args(query,
							 DBUS_TYPE_UINT32,&slot_id,
							 DBUS_TYPE_STRING,&tmp_src_path,
							 DBUS_TYPE_STRING,&tmp_des_path,
							 DBUS_TYPE_UINT32,&flag,
							 DBUS_TYPE_UINT32,&op,
							 DBUS_TYPE_INVALID);
    
   // printf("Copying file, please wait ...\n");
    //printf("0000\n");
	reply = dbus_connection_send_with_reply_and_block (connection,query,120000, &err);
	//printf("0001\n");
	dbus_message_unref(query);
	//printf("2222\n");
	if (NULL == reply) {
		printf("<error> failed get reply.\n");
		if (dbus_error_is_set(&err)) {
			printf("%s raised: %s",err.name,err.message);
			dbus_error_free(&err);
		}
		return CMD_SUCCESS;
	}
	//printf("3333\n");
	if (dbus_message_get_args ( reply, &err,
					DBUS_TYPE_UINT32,&ret,
					DBUS_TYPE_INVALID)) {
		
	}
	//printf("4444\n");
	dbus_message_unref(reply);
    
    return ret;
}


