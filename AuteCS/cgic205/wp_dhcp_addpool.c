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
* wp_dhcp_addpool.c
*
*
* CREATOR:
* autelan.software.Network Dep. team
* zhouym@autelan.com
*
* DESCRIPTION:
* system cotrl for dhcp  
*
*
*******************************************************************************/
#ifdef __cplusplus
extern "C"{
#endif

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include "cgic.h"

#include "ws_usrinfo.h"
#include "ws_err.h"
#include "ws_ec.h"

//buffer length
#define CMD_LEN 512
#define MSG_LEN 256
#define INTF_LEN 64

#define BITE_LEN 32

//return value
#define CMD_SUCCESS 0
#define NO_PARA 1
#define NO_CONF_FILE 2

//static declare of used const value
static char* LANG_PATH_GL = "../htdocs/text/public.txt";
static char* LANG_PATH_LO = "../htdocs/text/control.txt";
static char* DHCP_SUB_READ = "dhcp_sub_read.sh";
static char* DHCP_SUB_WRITE = "dhcp_sub_write.sh";
static char* DHCP_SUB_INFO = "/var/run/apache2/dhcp_sub.tmp";
static char* DEST_PAGE = "wp_dhcpcon.cgi";

static const char* NO_MESSAGE = " > /dev/null";
static const char* SPACE_MARK = " ";
static const char* QUOTA_MARK = "\"";

static struct list *lpublic = NULL;
static struct list *llocal = NULL;

//preview declaration
void ShowDhcpPoolPage(); 
void dhcp_pool_config(char* str_uid); 
static int get_first_token(char *str_msg);


int cgiMain()
{
    ShowDhcpPoolPage();
    return 0;
}

static void redirection(char *str_dest)
{
    if(NULL == str_dest)
    {
        return;
    }
    cgiHeaderContentType("text/html");
    fprintf(cgiOut, "<html>"\
                       "<body>"\
                          "<script language=javascript>"\
                             "var str_url = document.location.href;"\
                             "var index = str_url.indexOf(\"&\");"\
                             "var str_clean = str_url.substr(0,index);"\
                             "str_clean = str_clean.replace(/\\w+\\.\\w+\\?/,\"%s?\");"\
                             "document.location.replace(str_clean);"\
                          "</script>"\
                       "</body>"\
                    "</html>", str_dest);
    return;
}

void dhcp_pool_config(char*str_uid)
{   
    if(checkuser_group(str_uid)==0)
    	{
    char str_sys_cmd[CMD_LEN];
    char str_sys_cmd2[CMD_LEN];
    char str_para_buf[MSG_LEN];
    char str_para_tmp[INTF_LEN];
    int res_para = cgiFormNotFound;
    int res_sys = CMD_SUCCESS;
    FILE *fd = NULL;
    char *pc_temp = NULL;
    int pos = 0;
    memset(str_sys_cmd2, 0, CMD_LEN);
    memset(str_sys_cmd, 0, CMD_LEN);
    memset(str_para_buf, 0, MSG_LEN);
    memset(str_para_tmp, 0, INTF_LEN);

    strcpy(str_sys_cmd, DHCP_SUB_WRITE);
    strcat(str_sys_cmd, SPACE_MARK);
    strcpy(str_sys_cmd2, DHCP_SUB_READ);
    strcat(str_sys_cmd2, SPACE_MARK);

    res_para = cgiFormStringNoNewlines("intf_name", str_para_buf, MSG_LEN);
    if(cgiFormSuccess != res_para)
    {
        ShowErrorPage(search(llocal,"Get_args_error"));
        return;
    }
    strcat(str_sys_cmd, str_para_buf);
    strcat(str_sys_cmd, SPACE_MARK);
    strcat(str_sys_cmd2, str_para_buf);
    strcat(str_sys_cmd2, SPACE_MARK);

    res_para = cgiFormStringNoNewlines("subnet", str_para_buf, MSG_LEN);
    if(cgiFormSuccess != res_para)
    {
        ShowErrorPage(search(llocal,"Get_args_error"));
        return;
    }
    strcat(str_sys_cmd, str_para_buf);
    strcat(str_sys_cmd, SPACE_MARK);

    res_para = cgiFormStringNoNewlines("mask", str_para_buf, MSG_LEN);
    if(cgiFormSuccess != res_para)
    {
        ShowErrorPage(search(llocal,"Get_args_error"));
        return;
    }
    strcat(str_sys_cmd, str_para_buf);
    strcat(str_sys_cmd, SPACE_MARK);

    res_para = cgiFormStringNoNewlines("ipr_list", str_para_buf, MSG_LEN);
    if(cgiFormSuccess != res_para)
    {
        ShowErrorPage(search(llocal,"Get_args_error"));
        return;
    }

    system(str_sys_cmd2);
  
    strcat(str_sys_cmd, QUOTA_MARK);
    strcat(str_sys_cmd, str_para_buf);
    strcat(str_sys_cmd, QUOTA_MARK);
    strcat(str_sys_cmd, SPACE_MARK);
    
    memset(str_para_buf, 0, MSG_LEN);
    
    if( NULL == (fd = fopen(DHCP_SUB_INFO, "r")))  
    {
        ShowErrorPage(search(lpublic, "error_open"));
        exit(1);
    }
    while(NULL != fgets(str_para_tmp, MSG_LEN, fd))
    {
       pos = get_first_token(str_para_tmp);
       if(pos > 0)
       {
           if('r' != str_para_tmp[pos] && 'R' != str_para_tmp[pos])
           {
               pc_temp = strtok(str_para_tmp, ";");
               strcat(str_para_buf, pc_temp);
               strcat(str_para_buf, "^"); 
           }
       }
    }
    if((pos = strlen(str_para_buf)) > 0 )
    {
       str_para_buf[strlen(str_para_buf)-1] = '\0';
       strcat(str_sys_cmd, " '");
       strcat(str_sys_cmd, str_para_buf);
       strcat(str_sys_cmd, "'");
    }

    strcat(str_sys_cmd, NO_MESSAGE);
    fprintf(stderr,"%s",str_sys_cmd);
    res_sys = system(str_sys_cmd);
    res_sys = WEXITSTATUS(res_sys);
    if(CMD_SUCCESS != res_sys)
    {
        ShowErrorPage(search(llocal, "exec_serv_fail"));
        return;
    }
    fclose(fd);
    	}
    redirection( DEST_PAGE );
    return;
}

static int get_first_token(char *str_msg)
{
   int pos = 0;
   int flag = 0;
   while('\0' != str_msg[pos])
   {
       if(' ' != str_msg[pos] && 0x0A != str_msg[pos])
       {  
           flag = 1;
           break;
       }
       pos++;
   }
   return (1 == flag) ? pos : -1;
}

void ShowDhcpPoolPage(void)
{
    //page parameter
    FILE *fp = NULL;
    FILE *fd = NULL;
    char str_lan[3];
    char str_img_ok[MSG_LEN];
    char str_img_cancel[MSG_LEN];
    char *str_encry = NULL; 
    char *str_uid = NULL; 
    unsigned int un_padding = 6;

    //module pararmeter(syscmd and so on)
    char str_intf_name[INTF_LEN];
    char str_intf_addr[INTF_LEN];
    char str_mask[INTF_LEN]; 
    char str_sys_cmd[CMD_LEN];
    char str_msg_buf[MSG_LEN];
    char *pc_temp = NULL;
    char str_temp[INTF_LEN];
                           
    int res = 0;
    int pos =0;
	str_encry =(char *)malloc(MSG_LEN);
	memset(str_encry, 0, MSG_LEN);

    //language initialize
    lpublic = get_chain_head(LANG_PATH_GL);
    llocal = get_chain_head(LANG_PATH_LO);

    //config process
    
    cgiFormStringNoNewlines("UN", str_encry, MSG_LEN); 
    str_uid = dcryption(str_encry);
    if(NULL == str_uid)
    {
        ShowErrorPage(search(lpublic, "ill_user")); 	     
        exit(1);
	}
    if(cgiFormSubmitClicked("dhcp_pool_cfg") == cgiFormSuccess)
    {
        dhcp_pool_config(str_uid);
        release(llocal);
        release(lpublic);
        return;
    }

    //parameter initialize
 
    memset(str_lan, 0, sizeof(str_lan));
    
    memset(str_img_ok, 0, MSG_LEN);
    memset(str_img_cancel, 0, MSG_LEN);
    memset(str_intf_name, 0, INTF_LEN);
    memset(str_intf_addr, 0, INTF_LEN);
    memset(str_mask, 0, INTF_LEN);
    memset(str_sys_cmd, 0, CMD_LEN);
    memset(str_msg_buf, 0, MSG_LEN);
    memset(str_temp, 0, INTF_LEN);

    //get language type(not suitable)
  

    if( NULL == (fp = fopen(LANG_PATH_GL, "r")))  
    {
        ShowErrorPage(search(lpublic, "error_open"));
        exit(1);
    }
    fseek(fp, 4, 0);                      
    fgets(str_lan, 3, fp);
    fclose(fp);
    if(0 == strcmp(str_lan, "ch"))
    {
        strcpy(str_img_ok, "/images/ok-ch.jpg");
        strcpy(str_img_cancel, "/images/cancel-ch.jpg");
    }
    else if(0 == strcmp(str_lan, "en"))
    {
        strcpy(str_img_ok, "/images/ok-en.jpg");
        strcpy(str_img_cancel, "/images/cancel-en.jpg");
    }
    else
    {
        ShowErrorPage(search(llocal, "lang_err"));
        exit(1);
    }

    //make data (displayed in the page)
    cgiFormStringNoNewlines("Na", str_intf_name, INTF_LEN);
    cgiFormStringNoNewlines("Nb", str_intf_addr, INTF_LEN);
    cgiFormStringNoNewlines("Nm", str_mask, MSG_LEN);   
    
    strcpy(str_sys_cmd, DHCP_SUB_READ);
    strcat(str_sys_cmd, SPACE_MARK);
    strcat(str_sys_cmd, str_intf_name);
    strcat(str_sys_cmd, NO_MESSAGE);
   
    res = system(str_sys_cmd);
    res = WEXITSTATUS(res);

    if( NULL == (fd = fopen(DHCP_SUB_INFO, "r")))  
    {
        ShowErrorPage(search(lpublic, "error_open"));
        exit(1);
    }
    
   //page head
   cgiHeaderContentType("text/html");
   fprintf(cgiOut, "<html xmlns=\"http://www.w3.org/1999/xhtml\">");
      fprintf(cgiOut, "<head>"\
                         "<meta http-equiv=Content-Type content=text/html; charset=gb2312>"\
                         "<title>%s</title>"\
                         "<link rel=stylesheet href=/style.css type=text/css>"\
  	                     "<style type=text/css>"\
  	                        ".a3{width:30;border:0; text-align:center}"\
  	                     "</style>"\
  	                     "<script language=javascript src=/ip.js>"\
  	                     "</script>","DHCP");
  	     //add script
         fprintf(cgiOut, "<script languange=javascript>");

           //get global variable
           fprintf(cgiOut, "var str_sUrl = document.location.href;"\
                           "var str_oUrl = document.location.href;"\
                           "var arr_ipr_list = new Array();"\
                           "var arr_other_opt = new Array();"\
                           "var iter_r = 0;"\
                           "var iter_o = 0;"\
                           "var str_port_addr = \"%s\";"\
                           "var str_port_name = \"%s\";"\
                           "var un_mask = %s;"\
                           "var str_port_subnet = get_sub_net(str_port_addr, un_mask);", str_intf_addr, str_intf_name, str_mask);
           //get ip range array
           while(NULL != fgets(str_msg_buf, MSG_LEN, fd))
           {
               pos = get_first_token(str_msg_buf);
               if(pos > 0)
               {
                   if('r' == str_msg_buf[pos] || 'R' == str_msg_buf[pos])
                   {
                       pc_temp = strtok(str_msg_buf, " ;");
           fprintf(cgiOut, "var arr_temp = new Array();");
                       if(NULL != pc_temp)
                       {
                           pc_temp = strtok(NULL, " ;");
                           if(0x0A != pc_temp[0])
                           {
           fprintf(cgiOut, "arr_temp[\"sip\"] = \"%s\";"\
                           "arr_temp[\"sval\"] = ip2val(arr_temp[\"sip\"]) << un_mask;"\
                           "arr_temp[\"sval\"] = arr_temp[\"sval\"] >>> un_mask;", pc_temp);
                           pc_temp = strtok(NULL, " ;");
           fprintf(cgiOut, "arr_temp[\"eip\"] = \"%s\";"\
                           "arr_temp[\"eval\"] = ip2val(arr_temp[\"eip\"]) << un_mask;"\
                           "arr_temp[\"eval\"] = arr_temp[\"eval\"] >>> un_mask;", pc_temp);
           fprintf(cgiOut, "arr_ipr_list[iter_r++] = arr_temp;");  
                           }
                       }
                   }
                   else
                   {
                       pc_temp = strtok(str_msg_buf, ";");
                       if(NULL != strchr(pc_temp, '"'))
                       {
                           strcpy(str_temp, strtok(pc_temp, QUOTA_MARK));
                           strcat(str_temp, "\\\"");
                           strcat(str_temp, strtok(NULL, QUOTA_MARK));
                           strcat(str_temp, "\\\"");
                           pc_temp = str_temp;
                       }
           fprintf(cgiOut, "var str_temp = \"%s\";"\
                           "arr_other_opt[iter_o++] = str_temp.replace(/(^\\s+)|(\\s+$)/g,\"\");", pc_temp); 
                   }
               }
           }
           fprintf(cgiOut, "var arr_temp = new Array();"\
                           "arr_temp[\"sip\"] = null;"\
                           "arr_temp[\"eip\"] = null;"\
                           "arr_temp[\"sval\"] =(0xFFFFFFFF << un_mask)>>> un_mask;"\
                           "arr_temp[\"eval\"] =(0xFFFFFFFF << un_mask)>>> un_mask;"\
                           "arr_ipr_list[iter_r] = arr_temp;");
           //function
           fprintf(cgiOut, "function oninitialize()"\
                            "{"\
                               "document.getElementById(\"intf_name\").innerHTML = \"<b>\" + str_port_name + \"</b>\";"\
                               "document.getElementById(\"subnet_addr\").innerHTML = \"<b>\" + get_sub_net(str_port_addr, un_mask) + \"</b>\";"\
                               "document.getElementById(\"subnet_mask\").innerHTML = \"<b>\" + bite2addr(un_mask) + \"</b>\";"\
                               "set_select(arr_ipr_list);"\
                               "change_href();"\
                            "}");
            fprintf(cgiOut, "function change_href()"\
                            "{"\
                               "var hrefObj = document.getElementById(\"sub_para\");"\
                               "var str_temp = str_oUrl + \"&\";"\
                               "var iter = 0;"\
                               "if(arr_ipr_list.length > 1)"\
                               "{"\
                                  "str_temp += \"pool=\";"\
                               "}"\
                               "for(; iter<arr_ipr_list.length-1; iter++)"\
                               "{"\
                                  "str_temp += arr_ipr_list[iter][\"sip\"] + \" \" + arr_ipr_list[iter][\"eip\"];"\
                                  "str_temp += \"^\";"\
                               "}"\
                               "str_temp = str_temp.substr(0, str_temp.length-1);"\
                               "str_temp = str_temp.replace(/\\w+\\.cgi/,\"%s\");"\
                               "str_temp = str_temp.replace(/Nb=[0-9.]+/, \"Nb=\"+str_port_subnet);"\
                               "str_temp += \"&Nm1=\" + bite2addr(un_mask);"\
                               "hrefObj.href = str_temp;"\
                            "}", "wp_dhcp_sub_para.cgi");
				 if(checkuser_group(str_uid)==0)
				 	{
                 	fprintf(cgiOut, "function mysubmit()"\
                            "{"\
                               "str_sUrl = str_oUrl;"\
                               "var index = str_sUrl.indexOf(\"&\");"\
                               "var iter = 0;"\
                               "var str_temp = \"\";"\
                               "var un_temp = 0;"\
                               "str_sUrl = str_sUrl.substr(0,index);"\
                               "str_sUrl += \"&dhcp_pool_cfg=\";"\
                               "str_sUrl += \"&ipr_list=\";"\
                               "if(arr_ipr_list.length <= 1)"\
                               "{"\
                               		"str_temp = \"0.0.0.0\" + \" \" + \"0.0.0.0\";"\
                               		"str_sUrl += str_temp;"\
                               		"str_sUrl = str_sUrl.substr(0, str_sUrl.length);"\
                               "}"\
                              	"else"\
                              	"{"\
                               		"for(; iter<arr_ipr_list.length-1; iter++)"\
                               		"{"\
                                  		"if(str_port_subnet != get_sub_net(arr_ipr_list[iter][\"sip\"], un_mask))"\
                                  			"{"\
                                     			"alert(arr_ipr_list[iter][\"sip\"]+\"-\"+arr_ipr_list[iter][\"eip\"]+\"%s\");"\
                                     			"return false;"\
                                  			"}"\
                                  		"str_temp = arr_ipr_list[iter][\"sip\"] + \" \" + arr_ipr_list[iter][\"eip\"];"\
                                  		"str_sUrl += str_temp + \"^\";"\
                               		"}"\
                               		"str_sUrl = str_sUrl.substr(0, str_sUrl.length-1);"\
                               	"}"\
                               "str_sUrl += \"&intf_name=\" + str_port_name;"\
                               "str_sUrl += \"&subnet=\" + str_port_subnet;"\
                               "str_sUrl += \"&mask=\" + bite2addr(un_mask);"\
                               "document.location.replace(str_sUrl);"\
                               "return false;"\
                            "}", search(llocal, "dhcp_subnet_fail"));
				 	}
				 else{
				 	       fprintf(cgiOut, "function mysubmit()"\
											 "{"\
												"str_sUrl = str_oUrl;"\
												"var index = str_sUrl.indexOf(\"&\");"\
												"var iter = 0;"\
												"var str_temp = \"\";"\
												"var un_temp = 0;"\
												"str_sUrl = str_sUrl.substr(0,index);"\
												"str_sUrl += \"&dhcp_pool_cfg=\";"\
												"str_sUrl += \"&ipr_list=\";"\
												"for(; iter<arr_ipr_list.length-1; iter++)"\
												"{"\
												   "if(str_port_subnet != get_sub_net(arr_ipr_list[iter][\"sip\"], un_mask))"\
												   "{"\
													  "alert(arr_ipr_list[iter][\"sip\"]+\"-\"+arr_ipr_list[iter][\"eip\"]+\"%s\");"\
													  "return false;"\
												   "}"\
												   "str_temp = arr_ipr_list[iter][\"sip\"] + \" \" + arr_ipr_list[iter][\"eip\"];"\
												   "str_sUrl += str_temp + \"^\";"\
												"}"\
												"str_sUrl = str_sUrl.substr(0, str_sUrl.length-1);"\
												"str_sUrl += \"&intf_name=\" + str_port_name;"\
												"str_sUrl += \"&subnet=\" + str_port_subnet;"\
												"str_sUrl += \"&mask=\" + bite2addr(un_mask);"\
												"document.location.replace(str_sUrl);"\
												"return false;"\
											 "}", search(llocal, "dhcp_subnet_fail"));

				 }
            fprintf(cgiOut, "function set_select(arr_list)"\
                            "{"\
                               "var obj_temp = document.getElementById(\"ipr_pool\");"\
                               "var iter = obj_temp.length;"\
                               "var length = obj_temp.options.length;"\
                               "while(iter >= 0)"\
                               "{"\
                                  "obj_temp.remove(iter);"\
                                  "iter--;"\
                               "}"\
                               "iter = 0;"\
                               "while(null != arr_list[iter][\"sip\"])"\
                               "{"\
                                  "var str_temp = arr_list[iter][\"sip\"] + \"-\" + arr_list[iter][\"eip\"];"\
                                  "obj_temp.options[obj_temp.length] = new Option(str_temp, str_temp);"\
                                  "iter++;"\
                               "}"\
                            "}"); 
            fprintf(cgiOut, "function add_ip_ranges()"\
                            "{"\
                               "var str_start_ip = get_ip_addr(\"lower_addr\");"\
                               "var un_temp = 0;"\
							   "var tt = \"\";"\
							   "tt = document.getElementById(\"upper_addr4\").value;"\
							   "if(tt==255)"\
							   "{"\
							   		"alert(\"%s\");"\
							   		"return;"\
							   "}"\
                               "var str_end_ip = get_ip_addr(\"upper_addr\");"\
                               "if(null == str_start_ip || null == str_end_ip)"\
                               "{"\
                                  "alert(\"%s\");"\
                                  "return;"\
                               "}"\
                               "if(str_port_subnet != get_sub_net(str_start_ip, un_mask))"\
                               "{"\
                                  "alert(\"%s\");"\
                                  "return;"\
                               "}"\
                               "if(str_port_subnet != get_sub_net(str_end_ip, un_mask))"\
                               "{"\
                                  "alert(\"%s\");"\
                                  "return;"\
                               "}"\
                               "var arr_ins_ipr = new Array();"\
                               "arr_ins_ipr[\"sip\"] = str_start_ip;"\
                               "arr_ins_ipr[\"eip\"] = str_end_ip;"\
                               "un_temp = ip2val(str_start_ip) << un_mask;"\
                               "arr_ins_ipr[\"sval\"] = un_temp >>> un_mask;"\
                               "un_temp = ip2val(str_end_ip) << un_mask;"\
                               "arr_ins_ipr[\"eval\"] = un_temp >>> un_mask;"\
                               "if(arr_ins_ipr[\"sval\"] > arr_ins_ipr[\"eval\"])"\
                               "{"\
                                  "alert(\"%s\");"\
                                  "return;"\
                               "}"\
                               "arr_ipr_list = check_bound(arr_ipr_list, arr_ins_ipr);"\
                               "set_select(arr_ipr_list);"\
                               "clear_ip_addr(\"lower_addr\");"\
                               "clear_ip_addr(\"upper_addr\");"\
                               "document.getElementById(\"ranges_rm\").disabled = false;"\
                               "if(arr_ipr_list.length >= 6)"\
                               "{"\
                                  "document.getElementById(\"ranges_add\").disabled = true;"\
                               "}"\
                               "change_href();"\
                            "}", search(lpublic,"ip_err"),search(llocal, "ip_null"), search(llocal, "dhcp_start_ip"), search(llocal, "dhcp_end_ip"), search(llocal, "dhcp_format_err"));//search  
            fprintf(cgiOut, "function rm_ip_ranges()"\
                            "{"\
                               "var obj_temp = document.getElementById(\"ipr_pool\");"\
                               "var iter = 0;"\
                               "var str_temp = \"\";"\
                               "while(iter < obj_temp.options.length)"\
                               "{"\
                                  "if(true == obj_temp.options[iter].selected)"\
                                  "{"\
                                     "str_temp += iter.toString(10);"\
                                  "}"\
                                  "iter++;"\
                                  "change_href();"\
                               "}"\
                               "arr_ipr_list = arr_del(arr_ipr_list, str_temp);"\
                               "set_select(arr_ipr_list);"\
                               "document.getElementById(\"ranges_add\").disabled = false;"\
                               "if(arr_ipr_list.length <= 1)"\
                               "{"\
                                  "document.getElementById(\"ranges_rm\").disabled = true;"\
                               "}"\
                               "change_href();"\
                            "}");     
            fprintf(cgiOut, "function change_ip_ranges()"\
                            "{"\
                            "}");  
            //reuseable function
            fprintf(cgiOut, "function get_ip_addr(str_name)"\
                            "{"\
                               "var str_para = \"\";"\
                               "var obj_temp;"\
                               "var str_temp = \"\";"\
                               "var iter = 1;"\
                               "for(; iter<=4; iter++)"\
                               "{"\
                                  "str_temp = \"document.getElementById(\\\"\" +  str_name + iter + \"\\\")\";"\
                                  "obj_temp = eval(str_temp);"\
                                  "if(\"\" == obj_temp.value)"\
                                  "{"\
                                     "return null;"\
                                  "}"\
                                  "str_para += obj_temp.value + \".\";"\
                               "}"\
                               "str_temp = str_para.replace(/\\.$/,\"\");"\
                               "return str_temp;"\
                            "}");
            fprintf(cgiOut, "function arr_del(arr, str)"
                            "{"\
                               "var arr_temp = new Array();"\
                               "var index = 0;"\
                               "var iter = 0;"\
                               "for(; iter<arr.length; iter++)"\
                               "{"\
                                  "if(-1 == str.indexOf(iter.toString(10)))"\
                                  "{"\
                                     "arr_temp[index++] = arr[iter];"\
                                  "}"\
                               "}"\
                               "return arr_temp;"\
                            "}");
            fprintf(cgiOut, "function clear_ip_addr(str_name)"\
                            "{"\
                               "var obj_temp;"\
                               "var str_temp = \"\";"\
                               "var iter = 1;"\
                               "for(; iter<=4; iter++)"\
                               "{"\
                                  "str_temp = \"document.getElementById(\\\"\" +  str_name + iter + \"\\\")\";"\
                                  "obj_temp = eval(str_temp);"\
                                  "obj_temp.value = \"\";"\
                               "}"\
                            "}");
                            //assume this function always get valid ip address string 
            fprintf(cgiOut, "function ip2val(str_ip_addr)"\
                            "{"\
                               "var arr_ip_seg = str_ip_addr.split(\".\");"\
                               "var un_val = 0;"\
                               "for(var i=0; i<4; i++)"\
                               "{"\
                                  "un_val = (un_val << 8) + parseInt(arr_ip_seg[i]);"\
                               "}"\
                               "return un_val;"\
                            "}");
            fprintf(cgiOut, "function val2ip(un_val)"\
                            "{"\
                               "var str_ip_addr = \"\";"\
                               "var un_temp = un_val >>> 24;"\
                               "str_ip_addr = un_temp.toString(10);"\
                               "un_temp = un_val << 8;"\
                               "un_temp = un_temp >>> 24;"\
                               "str_ip_addr += \".\" + un_temp.toString(10);"\
                               "un_temp = un_val << 16;"\
                               "un_temp = un_temp >>> 24;"\
                               "str_ip_addr += \".\" + un_temp.toString(10);"\
                               "un_temp = un_val << 24;"\
                               "un_temp = un_temp >>> 24;"\
                               "str_ip_addr += \".\" + un_temp.toString(10);"\
                               "return str_ip_addr;"\
                            "}");
            fprintf(cgiOut, "function get_sub_net(str_ip_addr, mask)"
                            "{"\
                               "var un_temp = ip2val(str_ip_addr);"\
                               "un_temp = un_temp >>> (32 - mask);"\
                               "un_temp = un_temp << (32 - mask);"\
                               "return val2ip(un_temp);"\
                            "}");
            fprintf(cgiOut, "function bite2addr(un_mask_bite)"
                            "{"\
                               "if(un_mask_bite < 1 || un_mask_bite > 31)"\
                               "{"\
                                  "return \"%s\";"\
                               "}"\
                               "var un_temp = parseInt(\"ffffffff\", 16);"\
                               "un_temp = un_temp >>> (32 - un_mask_bite);"\
                               "un_temp = un_temp << (32 - un_mask_bite);"\
                               "return val2ip(un_temp);"\
                            "}", search(llocal, "dhcp_mask_err"));
            fprintf(cgiOut, "function check_bound(arr_ipr_list, arr_ins_ipr)"\
                            "{"\
                               "var si = 0;"\
                               "var ei = 0;"\
                               "var un_stemp = arr_ins_ipr[\"sval\"];"\
                               "var un_etemp = arr_ins_ipr[\"eval\"];"\
                               "var arr_temp = new Array();"\
                               "for(; si<arr_ipr_list.length; si++)"\
                               "{"\
                                  "if(un_stemp <= arr_ipr_list[si][\"sval\"]||"\
                                     "un_stemp <= arr_ipr_list[si][\"eval\"])"\
                                  "{"\
                                    "break;"\
                                  "}"\
                               "}"\
                               "for(ei=si; ei<arr_ipr_list.length; ei++)"\
                               "{"\
                                  "if(un_etemp <= arr_ipr_list[ei][\"sval\"]||"\
                                     "un_etemp <= arr_ipr_list[ei][\"eval\"])"\
                                  "{"\
                                    "break;"\
                                  "}"\
                               "}"\
                              "var arr_ins_temp = new Array();"\
                               "if(un_etemp < arr_ipr_list[si][\"sval\"]-1)"\
                               "{"\
                                  "if(ei == 0 || un_stemp > arr_ipr_list[ei-1][\"eval\"]+1)"\
                                  "{"\
                                     "arr_ins_temp[\"sip\"] = arr_ins_ipr[\"sip\"];"\
                                     "arr_ins_temp[\"eip\"] = arr_ins_ipr[\"eip\"];"\
                                     "arr_ins_temp[\"sval\"] = arr_ins_ipr[\"sval\"];"\
                                     "arr_ins_temp[\"eval\"] = arr_ins_ipr[\"eval\"];"\
                                  "}"\
                                  "else"\
                                  "{"\
                                     "si--;"\
                                     "arr_ins_temp[\"sip\"] = arr_ipr_list[si][\"sip\"];"\
                                     "arr_ins_temp[\"sval\"] = arr_ipr_list[si][\"sval\"];"\
                                     "arr_ins_temp[\"eip\"] = arr_ins_ipr[\"eip\"];"\
                                     "arr_ins_temp[\"eval\"] = arr_ins_ipr[\"eval\"];"\
                                  "}"\
                               "}"\
                               "else"\
                               "{"\
                                  "if(un_stemp < arr_ipr_list[si][\"sval\"])"\
                                  "{"\
                                     "if(si>0 && (un_stemp==arr_ipr_list[si-1][\"eval\"]+1))"\
                                     "{"\
                                        "si--;"\
                                        "arr_ins_temp[\"sval\"] = arr_ipr_list[si][\"sval\"];"\
                                        "arr_ins_temp[\"sip\"] = arr_ipr_list[si][\"sip\"];"\
                                     "}"\
                                     "else"\
                                     "{"\
                                        "arr_ins_temp[\"sval\"] = un_stemp;"\
                                        "arr_ins_temp[\"sip\"] = arr_ins_ipr[\"sip\"];"\
                                      "}"\
                                  "}"\
                                  "else"\
                                  "{"\
                                     "arr_ins_temp[\"sval\"] = arr_ipr_list[si][\"sval\"];"\
                                     "arr_ins_temp[\"sip\"] = arr_ipr_list[si][\"sip\"];"\
                                  "}"\
                                  "if(un_etemp < arr_ipr_list[ei][\"sval\"])"\
                                  "{"\
                                     "arr_ins_temp[\"eval\"] = un_etemp;"\
                                     "arr_ins_temp[\"eip\"] = arr_ins_ipr[\"eip\"];"\
                                  "}"\
                                  "else"\
                                  "{"\
                                     "arr_ins_temp[\"eval\"] = arr_ipr_list[ei][\"eval\"];"\
                                     "arr_ins_temp[\"eip\"] = arr_ipr_list[ei][\"eip\"];"\
                                     "ei++;"\
                                  "}"\
                               "}"\
                               "arr_temp = arr_ipr_list.slice(0, si);"\
                               "arr_temp[si] = arr_ins_temp;"\
                               "arr_temp = arr_temp.concat(arr_ipr_list.slice(ei));"\
                               "return arr_temp;"\
                            "}");
         fprintf(cgiOut, "</script>");
  	  fprintf(cgiOut, "</head>");

       //page body
       fprintf(cgiOut, "<body onload=oninitialize()>"\
                          "<form method=post onsubmit=\"return mysubmit();\">"\
                             "<div align=center>"\
                                "<table width=976 border=0 cellpadding=0 cellspacing=0>");
                    //module title
                    fprintf(cgiOut,"<tr>"\
                                      "<td width=8 "\
                                           "align=left "\
                                           "valign=top "\
                                           "background=/images/di22.jpg>"\
                                         "<img src=/images/youce4.jpg width=8 height=30/>"\
                                      "</td>"\
                                      "<td width=51 "\
                                           "align=left "\
                                           "valign=bottom "\
                                           "background=/images/di22.jpg>"\
                                         "<img src=/images/youce33.jpg width=37 height=24/>"\
                                      "</td>"\
                                      "<td width=153 "\
                                           "align=left "\
                                           "valign=bottom "\
                                           "id=%s "\
                                           "background=/images/di22.jpg>%s</td>",search(lpublic, "title_style"), search(llocal, "dhcp_pool_cfg"));//search(llocal, "dhcp_relay")
                       //submit button
                       fprintf(cgiOut,"<td width=690 "\
                                          "align=right "\
                                          "valign=bottom "\
                                          "background=/images/di22.jpg>");
                             fprintf(cgiOut, "<table width=131 border=0 cellspacing=0 cellpadding=0>"\
                                                "<tr>"\
	                                               "<td width=62 align=center>"\
	                                                  "<input id=but "\
	                                                         "type=submit "\
	                                                         "name=dhcp_relay "\
	                                                         "style=background-image:url(%s) "\
	                                                         "value=\"\" >"\
	                                                  "</input>"\
	                                               "</td>"\
	                                               "<td width=62 align=left>"\
	                                                  "<a href=wp_dhcpcon.cgi?UN=%s target=mainFrame>"\
	                                                     "<img src=%s border=0 width=62 height=20/>"\
	                                                  "</a>"\
	                                               "</td>"\
	                                            "</tr>"\
	                                         "</table>", str_img_ok, str_encry, str_img_cancel);
                       fprintf(cgiOut,"</td>");
                       
                       fprintf(cgiOut,"<td width=74 align=right valign=top background=/images/di22.jpg>"\
                                         "<img src=/images/youce3.jpg width=31 height=30/>"\
                                      "</td>"\
                                   "</tr>");
                    //main table
                    fprintf(cgiOut,"<tr>"\
                                      "<td colspan=5 align=center valign=middle>"\
                                         "<table width=976 border=0 cellpadding=0 cellspacing=0 bgcolor=#f0eff0>"\
                                            "<tr>");
                                fprintf(cgiOut,"<td width=12 align=left valign=top background=/images/di888.jpg>&nbsp;</td>"\
                                               "<td width=948>"\
                                                  "<table width=947 border=0 cellspacing=0 cellpadding=0>"\
                                                     "<tr height=4 valign=bottom>"\
                                                        "<td width=120>&nbsp;</td>"\
                                                        "<td width=827 valign=bottom>"\
                                                           "<img src=/images/bottom_05.gif width=827 height=4/>"\
                                                        "</td>"\
                                                     "</tr>");

                                      //main config area 
                                      fprintf(cgiOut,"<tr>");
                                         //link title table   
                                         fprintf(cgiOut,"<td>"\
                                                           "<table width=120 border=0 cellspacing=0 cellpadding=0>"\
                                                              "<tr height=25>"\
                                                                 "<td id=tdleft>&nbsp;</td>"\
                                                              "</tr>");
                                              fprintf(cgiOut, "<tr height=26>"\
                                                                 "<td align=left "\
                                                                     "id=tdleft background=/images/bottom_bg.gif "\
                                                                     "style=\"border-right:0\"><font id=yingwen_san>DHCP</font><font id=%s>%s</font>"\
                                                                 "</td>",search(lpublic,"menu_san"), search(llocal, "dhcp_pool"));//search(llocal, "dhcp_relay")   
                                              fprintf(cgiOut, "</tr>");

                                              fprintf(cgiOut,"<tr height=25>"\
                                                                "<td align=left id=tdleft>"\
                                                                   "<a href=wp_dhcpcon.cgi?UN=%s "\
                                                                     " id=sub_para"\
                                                                     " target=mainFrame "\
                                                                     " class=top><font id=%s>%s</font>"\
                                                                   "</a>"\
                                                                "</td>"\
                                                             "</tr>", str_encry,search(lpublic,"menu_san"), search(llocal, "dhcp_sub_para"));
										
                                             //padding
                                             int i=0;
                                             for(; i<un_padding; i++)
                                             {
                                               fprintf(cgiOut,"<tr height=25>"\
                                                                 "<td id=tdleft>&nbsp;</td>"\
                                                              "</tr>");
                                             }
                                           fprintf(cgiOut, "</table>"\
                                                        "</td>");
                                                        
                                        //web console config area
                                        fprintf(cgiOut, "<td align=left style=\"background-color:#ffffff; border-right:1px solid #707070; padding-left:30px\">"\
                                                           "<table border=0 cellspacing=0 cellpadding=0 width=760>");

                                              //interface name
                                              fprintf(cgiOut, "<tr height=35>"\
                                                                 "<td align=left id=tdprompt width=20%%>%s:</td>"\
                                                                 "<td align=left id=intf_name name=intf_name></td>"\
                                                              "</tr>", search(llocal, "intf_name"));
                                              //subnet 
                                              fprintf(cgiOut, "<tr height=30>"\
                                                                 "<td align=left id=tdprompt width=20%%>%s:</td>"\
                                                                 "<td align=left id=subnet_addr name=subnet_addr></td>"\
                                                              "</tr>", search(llocal, "dhcp_sub_addr"));
                                              //mask
                                              fprintf(cgiOut, "<tr height=30>"\
                                                                 "<td align=left id=tdprompt width=20%%>%s:</td>"\
                                                                 "<td align=left id=subnet_mask name=subnet_mask></td>"\
                                                              "</tr>", search(llocal, "dhcp_sub_mask"));//search(llocal, "subnet_mask");  
                                              //IP ranges
                                              fprintf(cgiOut, "<tr height=30 colspan=5>"\
                                                                 "<td  align=left id=tdprompt width=20%%>%s:</td>", search(llocal, "dhcp_ip_range"));//search
                                                 fprintf(cgiOut, "<td width=15%%>"\
                                                                    "<div style=\"border-width:1;border-color:#a5acb2;border-style:solid;width:140;font-size:9pt\">"\
                                                                       "<input type=text name=lower_addr1 id=lower_addr1 maxlength=3 class=a3 onKeyUp=\"mask(this,%s)\" onbeforepaste=mask_c()>.</input>"\
                                                                       "<input type=text name=lower_addr2 id=lower_addr2 maxlength=3 class=a3 onKeyUp=\"mask(this,%s)\" onbeforepaste=mask_c()>.</input>"\
                                                                       "<input type=text name=lower_addr3 id=lower_addr3 maxlength=3 class=a3 onKeyUp=\"mask(this,%s)\" onbeforepaste=mask_c()>.</input>"\
                                                                       "<input type=text name=lower_addr4 id=lower_addr4 maxlength=3 class=a3 onKeyUp=\"mask(this,%s)\" onbeforepaste=mask_c()></input>"\
                                                                    "</div>"\
                                                                 "</td>", search(lpublic,"ip_error"), search(lpublic,"ip_error"), search(lpublic,"ip_error"), search(lpublic,"ip_error"));
                                                 fprintf(cgiOut, "<td align=center width=2%%>-</td>");
                                                 fprintf(cgiOut, "<td width=15%%>"\
                                                                    "<div style=\"border-width:1;border-color:#a5acb2;border-style:solid;width:140;font-size:9pt\">"\
                                                                       "<input type=text name=upper_addr1 id=upper_addr1 maxlength=3 class=a3 onKeyUp=\"mask(this,%s)\" onbeforepaste=mask_c()>.</input>"\
                                                                       "<input type=text name=upper_addr2 id=upper_addr2 maxlength=3 class=a3 onKeyUp=\"mask(this,%s)\" onbeforepaste=mask_c()>.</input>"\
                                                                       "<input type=text name=upper_addr3 id=upper_addr3 maxlength=3 class=a3 onKeyUp=\"mask(this,%s)\" onbeforepaste=mask_c()>.</input>"\
                                                                       "<input type=text name=upper_addr4 id=upper_addr4 maxlength=3 class=a3 onKeyUp=\"mask(this,%s)\" onbeforepaste=mask_c()></input>"\
                                                                    "</div>"\
                                                                 "</td>",search(lpublic,"ip_error"),search(lpublic,"ip_error"),search(lpublic,"ip_error"),search(lpublic,"ip_error"));
                                                 fprintf(cgiOut, "<td align=center width=10%%>");
												          if(checkuser_group(str_uid)==0)  /*管理员*/
												          	{
                                                                fprintf(cgiOut,"<input type=button name=ranges_add id=ranges_add onclick=add_ip_ranges() style=\"width:50px\" height:36px style=background-image:url(/images/SubBackGif.gif) value=%s></input>"\
                                                                 "</td>", search(llocal, "dhcp_add")); 
												          	}
														  else
														  	{
																 fprintf(cgiOut,"<input type=button name=ranges_add id=ranges_add onclick=add_ip_ranges() style=\"width:50px\" height:36px disabled style=background-image:url(/images/SubBackGif.gif) value=%s></input>"\
                                                                 "</td>", search(llocal, "dhcp_add")); 			
														  	}
                                                 fprintf(cgiOut, "<td>"\
                                                                    "<font color=red>&nbsp;</font>"
                                                                 "</td>");//search
                                              fprintf(cgiOut, "</tr>");
                                              //IP ranges pool                                                                                 
                                              fprintf(cgiOut, "<tr>"\
                                                                 "<td width=20%%>&nbsp;"\
                                                                 "</td>"\
                                                                 "<td align=left colspan=3>"\
                                                                    "<select id=ipr_pool"\
                                                                           " name=ip_pool"\
                                                                           " id=ip_pool"\
                                                                           " multiple"\
                                                                           " size=5"\
                                                                           " style=\"width:300px\""\
                                                                           " onchange=change_ip_ranges()"\
                                                                           ">"\
                                                                    "</select>"\
                                                                 "</td>"\
                                                                 "<td valign=top align=center width=60>"\
                                                                    "<div width=20 valign=top>");
											                       if(checkuser_group(str_uid)==0)  /*管理员*/
											                       	{
                                                                       fprintf(cgiOut,"<input type=button name=ranges_rm id=ranges_rm onclick=rm_ip_ranges() style=\"width:50px\" height:36px style=background-image:url(/images/SubBackGif.gif) value=%s></input>", search(llocal, "dhcp_rm"));
											                       	}
																   else
																   	{
																	  fprintf(cgiOut,"<input type=button name=ranges_rm id=ranges_rm onclick=rm_ip_ranges() style=\"width:50px\" height:36px style=background-image:url(/images/SubBackGif.gif) disabled value=%s></input>", search(llocal, "dhcp_rm"));
																   	}
														fprintf(cgiOut,"</div>"\
                                                               			"</tr>"\
                                                               			"</table>");     
                                      fprintf(cgiOut,"</tr>");
                                      //bottom
                                      fprintf(cgiOut,"<tr height=4 valign=top>"\
                                                        "<td width=120 height=4 align=right valign=top>"\
                                                           "<img src=/images/bottom_07.gif width=1 height=10/>"
                                                        "</td>"\
                                                        "<td width=827 height=4 valign=top bgcolor=#FFFFFF>"\
                                                           "<img src=/images/bottom_06.gif width=827 height=15/>"\
                                                        "</td>"\
                                                     "</tr>"\
                                                  "</table>"\
                                               "</td>"\
                                               "<td width=15 background=/images/di999.jpg>&nbsp;</td>"\
                                            "</tr>"\
                                         "</table>"\
                                      "</td>"\
                                   "</tr>"\
                                   "<tr>"\
                                      "<td colspan=3 align=left valign=top background=/images/di777.jpg>"\
                                         "<img src=/images/di555.jpg width=61 height=62/>"\
                                      "</td>"\
                                      "<td align=left valign=top background=/images/di777.jpg>&nbsp;</td>"\
                                      "<td align=left valign=top background=/images/di777.jpg>"\
                                         "<img src=/images/di666.jpg width=74 height=62/>"\
                                      "</td>"\
                                   "</tr>");

	             fprintf(cgiOut,"</table>"\
	                         "</div>"\
	                      "</form>"\
	                   "</body>"); 
    fprintf(cgiOut,"</html>");

    release(llocal);
    release(lpublic);
    fclose(fd);
    free(str_encry);
}

#ifdef __cplusplus
}
#endif

