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
* wp_pppoe_server.c
*
*
* CREATOR:
* autelan.software.Network Dep. team
* zhouym@autelan.com
*
* DESCRIPTION:
* system function for system pppoe
*
*
*******************************************************************************/
#include <stdio.h>
#include "cgic.h"
#include <string.h>
#include <stdlib.h>
#include <sys/wait.h>
#include "ws_usrinfo.h"
#include "ws_err.h"
#include "ws_ec.h"
#include "ws_p3_server.h"

int ShowExportConfPage(struct list *lpublic, struct list *lsystem);     /*m代表加密后的字符串*/

int cgiMain()
{
	struct list *lpublic;	/*解析public.txt文件的链表头*/
	struct list *lsystem;	  /*解析system.txt文件的链表头*/	
	lpublic=get_chain_head("../htdocs/text/public.txt");
	lsystem=get_chain_head("../htdocs/text/system.txt");

	ShowExportConfPage(lpublic,lsystem);
  	release(lpublic);  
  	release(lsystem);

	return 0;
}

int ShowExportConfPage(struct list *lpublic, struct list *lsystem)
{ 
	 
	  char *encry=(char *)malloc(BUF_LEN);				
	  char *str;
	  int ret;	
      char addn[N]="";	  
	  int j;
	  int flag = 1;

      ///////////////////////////

	 ST_P3_ALL sysall;  /*总的结构体内容*/
     memset(&sysall,0,sizeof(sysall));
  
     char *fpath;
     fpath=P3_XML_FPATH;
   
  //   char showtype[N];
   
     char ip_add[20];  //获取ip地址的，三个条要分别显示
     memset(ip_add,0,20);

	 char pstr[N];   //密码，端口，数字等
	 memset(pstr,0,N);

     char *temp=(char *)malloc(N);
	 memset(temp,0,N);

	int i = 0;		

	char *tmp;
	//char *tmp=(char*)malloc(30);
	//memset(tmp,0,30);
	//fprintf(stderr,"1-----tmp=%p",tmp);

	char serverip1[4] = {0};
	char serverip2[4] = {0};
	char serverip3[4] = {0};
	char serverip4[4] = {0};

	char authip1[4] = {0};
	char authip2[4] = {0};
	char authip3[4] = {0};
	char authip4[4] = {0};

	char acctip1[4] = {0};
	char acctip2[4] = {0};
	char acctip3[4] = {0};
	char acctip4[4] = {0};	

	 char baseip1[4] = {0};
	 char baseip2[4] = {0};
	 char baseip3[4] = {0};
	 char baseip4[4] = {0};

	 char localip1[4] = {0};
	 char localip2[4] = {0};
	 char localip3[4] = {0};
	 char localip4[4] = {0};

	 char hostip1[4] = {0};
	 char hostip2[4] = {0};
	 char hostip3[4] = {0};
	 char hostip4[4] = {0};

	 char backupip1[4] = {0};
	 char backupip2[4] = {0};
	 char backupip3[4] = {0};
	 char backupip4[4] = {0};

	 char ip1[4] = {0};
	 char ip2[4] = {0};
	 char ip3[4] = {0};
	 char ip4[4] = {0};	 
     ///////////////////////////
  
 	memset(encry,0,BUF_LEN);
 	cgiFormStringNoNewlines("UN", encry, BUF_LEN); 
 	str=dcryption(encry);
 	if(str==NULL)
 	{
 	  ShowErrorPage(search(lpublic,"ill_user"));		   /*用户非法*/
 	  return 0;
 	}
 	strcpy(addn,str);
	  	
	  cgiHeaderContentType("text/html");
	  fprintf(cgiOut,"<html xmlns=\"http://www.w3.org/1999/xhtml\"><head>");
	  fprintf(cgiOut,"<meta http-equiv=Content-Type content=text/html; charset=gb2312>");
	  fprintf(cgiOut,"<title>%s</title>",search(lpublic,"pppoe_s"));
	 fprintf(cgiOut,"<link rel=stylesheet href=/style.css type=text/css>"\
	"<style type=text/css>"\
	".a3{width:30;border:0; text-align:center}"\
	"#div1{ width:62px; height:18px; border:1px solid #666666; background-color:#f9f8f7;}"\
	"#div2{ width:60px; height:15px; padding-left:5px; padding-top:3px}"\
	"#link{ text-decoration:none; font-size: 12px}"\
	".usrlis {overflow-x:hidden;	overflow:auto; width: 300px; height: 100px; clip: rect( ); padding-top: 0px; padding-right: 0px; padding-bottom: 0px; padding-left: 0px} "\
	"</style>"\

	"<script language=javascript src=/ip.js>\n"\
 	"</script>\n"\
	"<script type='text/javascript'>"\
	"function changestate(){"\
	"var a1 = document.getElementsByName('showtype')[0];"\
	"var a2 = document.getElementsByName('showtype')[1];"\
	"var a3 = document.getElementsByName('showtype')[2];"\
	"}"\
	"</script>"\
	"</head>"\
	"<body>");
		

	 ////////////读取xml文件并保存到conf中去	
	 
      if(cgiFormSubmitClicked("ppp_config") == cgiFormSuccess)
      {
            memset(ip_add,0,20);
    		cgiFormStringNoNewlines("serverip1", serverip1, 4); 
			cgiFormStringNoNewlines("serverip2", serverip2, 4); 
			cgiFormStringNoNewlines("serverip3", serverip3, 4); 
			cgiFormStringNoNewlines("serverip4", serverip4, 4); 
			//fprintf(stderr,"6 %s",serverip1); 
			//fprintf(stderr,"7 %s",serverip2); 
			//fprintf(stderr,"8 %s",serverip3); 
			//fprintf(stderr,"9 %s",serverip4); 
			sprintf(ip_add,"%s.%s.%s.%s",serverip1,serverip2,serverip3,serverip4);				  
            //fprintf(stderr,"ip_add is %s",ip_add);
			memset(pstr,0,N);
            cgiFormStringNoNewlines("pwd", pstr, N);

			if(strcmp(ip_add,"")!=0 && strcmp(pstr,"")!=0)
			{
			  mod_p3_node(P3_XML_FPATH, P3_RADIUS, P3_SERIP,ip_add);
			  mod_p3_node(P3_XML_FPATH, P3_RADIUS, P3_PWD,pstr);
			}
    
			memset(ip_add,0,20);
			cgiFormStringNoNewlines("authip1", authip1, 4); 
			cgiFormStringNoNewlines("authip2", authip2, 4); 
			cgiFormStringNoNewlines("authip3", authip3, 4); 
			cgiFormStringNoNewlines("authip4", authip4, 4); 
			sprintf(ip_add,"%s.%s.%s.%s",authip1,authip2,authip3,authip4); 
			//fprintf(stderr,"authip_add is %s",ip_add);
			if(strcmp(ip_add,"")!=0)
    		  {
    			  mod_p3_node(P3_XML_FPATH, P3_RADIUS, P3_AUTH,ip_add);
    		  }


			memset(pstr,0,N);
			cgiFormStringNoNewlines("authp", pstr, N); 
			if(strcmp(pstr,"")!=0)
			{
			  mod_p3_node(P3_XML_FPATH, P3_RADIUS, P3_AUTH_PORT,pstr);
			}

			memset(ip_add,0,20);
			cgiFormStringNoNewlines("acctip1", acctip1, 4); 
			cgiFormStringNoNewlines("acctip2", acctip2, 4); 
			cgiFormStringNoNewlines("acctip3", acctip3, 4); 
			cgiFormStringNoNewlines("acctip4", acctip4, 4); 
			sprintf(ip_add,"%s.%s.%s.%s",acctip1,acctip2,acctip3,acctip4);
			if(strcmp(ip_add,"")!=0)
    		  {
    			  mod_p3_node(P3_XML_FPATH, P3_RADIUS, P3_ACCT,ip_add);
    		  }

			memset(pstr,0,N);
			cgiFormStringNoNewlines("acctp", pstr, N); 
			if(strcmp(pstr,"")!=0)
			{
			  mod_p3_node(P3_XML_FPATH, P3_RADIUS, P3_ACCT_PORT,pstr);
			}
//            }


			//interface 
//			if(strcmp(showtype,"4")==0)
//            {
           // fprintf(stderr,"interface");
              memset(pstr,0,N);
			  cgiFormStringNoNewlines("max", pstr, N);
			 
			  int max = atoi(pstr);
			  if(max>1 && max <65536){
					mod_p3_node(P3_XML_FPATH, P3_INF, P3_MAX,pstr);
			  }
			  else{
					ShowAlert("2~65535");
					flag = 0;
			  }

			//base
//			if(strcmp(showtype,"5")==0)
//            {
              //fprintf(stderr,"base");
              memset(ip_add,0,20);
			  cgiFormStringNoNewlines("baseip1", baseip1, 4); 
			  cgiFormStringNoNewlines("baseip2", baseip2, 4); 
			  cgiFormStringNoNewlines("baseip3", baseip3, 4); 
			  cgiFormStringNoNewlines("baseip4", baseip4, 4); 
			  sprintf(ip_add,"%s.%s.%s.%s",baseip1,baseip2,baseip3,baseip4);
			  if(strcmp(ip_add,"")!=0)
    		  {
    			  mod_p3_node(P3_XML_FPATH, P3_INF, P3_BASE,ip_add);
    		  }
//            }

			  
			//localip
//			if(strcmp(showtype,"6")==0)
//            {
              //fprintf(stderr,"localip");
              memset(ip_add,0,20);
			  cgiFormStringNoNewlines("localip1", localip1, 4); 
			  cgiFormStringNoNewlines("localip2", localip2, 4); 
			  cgiFormStringNoNewlines("localip3", localip3, 4); 
			  cgiFormStringNoNewlines("localip4", localip4, 4); 
			  sprintf(ip_add,"%s.%s.%s.%s",localip1,localip2,localip3,localip4);
			  if(strcmp(ip_add,"")!=0)
    		  {
    			  mod_p3_node(P3_XML_FPATH, P3_INF, P3_MYIP,ip_add);
    		  }
//            }

			//port
//			if(strcmp(showtype,"7")==0)
//            {
           // fprintf(stderr,"port");
              memset(pstr,0,N);
			  cgiFormStringNoNewlines("port", pstr, N);
			  if(strcmp(pstr,"")!=0)
    		  {
    			  mod_p3_node(P3_XML_FPATH, P3_INF, P3_PORT,pstr);
    		  }
//            } 

			//hostip	
//			if(strcmp(showtype,"8")==0)
//            {
              //fprintf(stderr,"hostip");
              memset(ip_add,0,20);
			  cgiFormStringNoNewlines("hostip1", hostip1, 4); 
			  cgiFormStringNoNewlines("hostip2", hostip2, 4); 
			  cgiFormStringNoNewlines("hostip3", hostip3, 4); 
			  cgiFormStringNoNewlines("hostip4", hostip4, 4); 
			  sprintf(ip_add,"%s.%s.%s.%s",hostip1,hostip2,hostip3,hostip4);
			  if(strcmp(ip_add,"")!=0)
    		  {
    			  mod_p3_node(P3_XML_FPATH, P3_DNS, P3_HIP,ip_add);
    		  }
//            }

			//backip
//			 if(strcmp(showtype,"9")==0)
//            {
              //fprintf(stderr,"backip");
              memset(ip_add,0,20);
			  cgiFormStringNoNewlines("backupip1", backupip1, 4); 
			  cgiFormStringNoNewlines("backupip2", backupip2, 4); 
			  cgiFormStringNoNewlines("backupip3", backupip3, 4); 
			  cgiFormStringNoNewlines("backupip4", backupip4, 4); 
			  sprintf(ip_add,"%s.%s.%s.%s",backupip1,backupip2,backupip3,backupip4);
			  if(strcmp(ip_add,"")!=0)
    		  {
    			  mod_p3_node(P3_XML_FPATH, P3_DNS, P3_BIP,ip_add);
    		  }
//            }
      
            //host ip and password
//            if(strcmp(showtype,"10")==0)
//            {
            	//fprintf(stderr,"host ip and password");
                memset(ip_add,0,20);
				cgiFormStringNoNewlines("ip1", ip1, 4); 
				cgiFormStringNoNewlines("ip2", ip2, 4); 
				cgiFormStringNoNewlines("ip3", ip3, 4); 
				cgiFormStringNoNewlines("ip4", ip4, 4); 
				sprintf(ip_add,"%s.%s.%s.%s",ip1,ip2,ip3,ip4);
    			memset(pstr,0,N);
                cgiFormStringNoNewlines("hostname", pstr, N);
    
   			if(strcmp(ip_add,"")!=0 && strcmp(pstr,"")!=0)
    			{
    			  mod_p3_node(P3_XML_FPATH, P3_HOST, P3_HOSTIP,ip_add);
				  mod_p3_node(P3_XML_FPATH, P3_HOST, P3_HOSTNAME,pstr);
    			}
				else
				{
					//ShowAlert(search(lsystem,"p_null"));
  	 			}
//            }
			 read_p3_xml(fpath, &sysall);	  
     		 ret=write_p3_conf(&sysall);	 
	
	  	     if(ret==0 && flag){
	  			ShowAlert(search(lpublic,"oper_succ"));
	  	     }
      }
  //////////////////启用 关闭pppoe服务
  
  char status[10] = {0};
  int ret_val;

  find_p3_node(P3_XML_FPATH, P3_INF, "lstatus", status);
  
  if(cgiFormSubmitClicked("boot") == cgiFormSuccess)
  {
  
  	 if(!strcmp(status,"1")){
		ShowAlert(search(lsystem,"p_enable"));	
	 }
	 else{
  		ret_val = system("sudo /opt/services/init/pppoe_init start");
			if(WIFEXITED(ret_val)){
				if(WEXITSTATUS(ret_val) > 1 ){
				mod_p3_node(P3_XML_FPATH, P3_INF, P3_STATUS,"1");
				ShowAlert(search(lpublic,"oper_succ"));
				}
				else{
					ShowAlert(search(lpublic,"oper_fail"));
				}
			}
			else{
				ShowAlert(search(lpublic,"oper_fail"));
			}	
 	 }
  }
  
  if(cgiFormSubmitClicked("halt") == cgiFormSuccess)
  {
	  if(!strcmp(status,"0")){
		 ShowAlert(search(lsystem,"p_disable"));	
	  }
	  else{
		 ret_val = system("sudo /opt/services/init/pppoe_init stop");
		 if(WIFEXITED(ret_val)){
			 mod_p3_node(P3_XML_FPATH, P3_INF, P3_STATUS,"0");
			 ShowAlert(search(lpublic,"oper_succ"));
		 }
		 else{
			 ShowAlert(search(lpublic,"oper_fail"));
		 }
	  }
  }

	
  /////////////////////end submit


	  fprintf(cgiOut,"<form method=post encType=multipart/form-data>"\
	  "<div align=center>"\
	  "<table width=976 border=0 cellpadding=0 cellspacing=0>"); 
	  fprintf(cgiOut,"<tr>"\
		"<td width=8 align=left valign=top background=/images/di22.jpg><img src=/images/youce4.jpg width=8 height=30/></td>"\
		"<td width=51 align=left valign=bottom background=/images/di22.jpg><img src=/images/youce33.jpg width=37 height=24/></td>"\
	  "<td width=153 align=left valign=bottom id=%s background=/images/di22.jpg>%s</td>",search(lpublic,"title_style"),search(lsystem,"sys_function"));
		fprintf(cgiOut,"<td width=690 align=right valign=bottom background=/images/di22.jpg>");
	  
	    fprintf(cgiOut,"<input type=hidden name=UN value=%s />",encry);
	 
		fprintf(cgiOut,"<table width=130 border=0 cellspacing=0 cellpadding=0>"); 
		fprintf(cgiOut,"<tr>"\
		"<td width=62 align=center><input id=but type=submit name=ppp_config style=background-image:url(/images/%s) value=""></td>",search(lpublic,"img_ok"));
	
		fprintf(cgiOut,"<td width=62 align=left><a href=wp_sysmagic.cgi?UN=%s target=mainFrame><img src=/images/%s border=0 width=62 height=20/></a></td>",encry,search(lpublic,"img_cancel"));
		
     	
		fprintf(cgiOut,"</tr>"\
		"</table>"); 
	 
	 
	  
	
		fprintf(cgiOut,"</td>"\
		"<td width=74 align=right valign=top background=/images/di22.jpg><img src=/images/youce3.jpg width=31 height=30/></td>"\
	"</tr>"\
	"<tr>"\
		"<td colspan=5 align=center valign=middle><table width=976 border=0 cellpadding=0 cellspacing=0 bgcolor=#f0eff0>"\
		"<tr>"); 
		
			fprintf(cgiOut,"<td width=12 align=left valign=top background=/images/di888.jpg>&nbsp;</td>"\
			"<td width=948><table width=947 border=0 cellspacing=0 cellpadding=0>"); 
			  fprintf(cgiOut,"<tr height=4 valign=bottom>"\
				  "<td width=120>&nbsp;</td>"\
				  "<td width=827 valign=bottom><img src=/images/bottom_05.gif width=827 height=4/></td>"\
			  "</tr>"\
			  "<tr>"\
				  "<td>");
			  fprintf(cgiOut,"<table width=120 border=0 cellspacing=0 cellpadding=0>"); 						

			  fprintf(cgiOut,"<tr height=25>"\
				  "<td id=tdleft>&nbsp;</td>"\
				"</tr>"); 
                     /*管理员*/
                     if(checkuser_group(addn)==0)
                     {
    								 
    						 fprintf(cgiOut,"<tr height=25>"\
    						  "<td align=left id=tdleft><a href=wp_sysinfo.cgi?UN=%s target=mainFrame class=top><font id=%s>%s</font></a></td>",encry,search(lpublic,"menu_san"),search(lsystem,"sys_infor"));
    						 fprintf(cgiOut,"</tr>"\
							 "<tr height=25>"\
							 "<td align=left id=tdleft><a href=wp_sysconfig.cgi?UN=%s target=mainFrame class=top><font id=%s>%s</font></a></td>",encry,search(lpublic,"menu_san"),search(lsystem,"sys_config"));
							 fprintf(cgiOut,"</tr>"\
    						 "<tr height=25>"\
    						  "<td align=left id=tdleft><a href=wp_impconf.cgi?UN=%s target=mainFrame class=top><font id=%s>%s</font></a></td>",encry,search(lpublic,"menu_san"),search(lsystem,"import_config"));
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
        
                            fprintf(cgiOut,"<tr height=25>"\
        					  "<td align=left id=tdleft><a href=wp_ntp.cgi?UN=%s target=mainFrame class=top><font id=%s>%s</font></a></td>",encry,search(lpublic,"menu_san"),search(lpublic,"ntp_s"));
        					fprintf(cgiOut,"</tr>");
        					
        					fprintf(cgiOut,"<tr height=26>"\
        							"<td align=left id=tdleft background=/images/bottom_bg.gif style=\"border-right:0\"><font id=%s>%s</font></td>",search(lpublic,"menu_san"),"PPPOE");  /*突出显示*/
        					fprintf(cgiOut,"</tr>");

							fprintf(cgiOut,"<tr height=25>"\
									"<td align=left id=tdleft><a href=wp_pppoe_snp.cgi?UN=%s target=mainFrame class=top><font id=%s>%s</font></a></td>",encry,search(lpublic,"menu_san"),"PPPOE SNP");
							fprintf(cgiOut,"</tr>");
							
							//新增时间条目
							fprintf(cgiOut,"<tr height=26>"\
							"<td align=left id=tdleft><a href=wp_webservice.cgi?UN=%s target=mainFrame class=top><font id=%s>%s</font></a></td>",encry,search(lpublic,"menu_san"),search(lpublic,"web_service"));
							fprintf(cgiOut,"</tr>");
        					
    					
                    }
					else
    				{
    						fprintf(cgiOut,"<tr height=25>"\
    						  "<td align=left id=tdleft><a href=wp_sysinfo.cgi?UN=%s target=mainFrame class=top><font id=%s>%s</font></a></td>",encry,search(lpublic,"menu_san"),search(lsystem,"sys_infor"));
    						fprintf(cgiOut,"</tr>");						
    					
        					//新增时间条目
        					fprintf(cgiOut,"<tr height=25>"\
        					  "<td align=left id=tdleft><a href=wp_showtime.cgi?UN=%s target=mainFrame class=top><font id=%s>%s</font></a></td>",encry,search(lpublic,"menu_san"),search(lsystem,"systime"));
        					fprintf(cgiOut,"</tr>");
    				}
                 
					for(j=0;j<11;j++)
					{
					    fprintf(cgiOut,"<tr height=25>"\
						"<td id=tdleft>&nbsp;</td>"\
					  "</tr>");
					}

                 
				       fprintf(cgiOut,"</table>"); 
				        fprintf(cgiOut,"</td>"\
				        "<td  align=left style=\"background-color:#ffffff; border-right:1px solid #707070; padding-left:30px\">");

						fprintf(cgiOut,"<table width=400 border=0 cellspacing=0 cellpadding=0>"); 


                       //---------------------------------->>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
                       fprintf(cgiOut,"<tr>\n");
                       fprintf(cgiOut,"<td style=\"font-size:16px\">\n");
							   
                       fprintf(cgiOut,"<font id=yingwen_san>%s</font>",search(lpublic,"pppoe_s"));
	
					   fprintf(cgiOut,"</td>\n");

					   fprintf( cgiOut, "<td>\n" );
		
					   memset(status,0,10);
					   find_p3_node(P3_XML_FPATH, P3_INF, "lstatus", status);
					   if(!strcmp(status,"1")){
					   		fprintf(cgiOut,"<font id=yingwen_san>%s</font>",search(lpublic,"log_start_n"));
					   }
					   else{
							fprintf(cgiOut,"<font id=yingwen_san>%s</font>",search(lpublic,"stop"));
					   }
					   fprintf(cgiOut,"</td>\n");		
					 
                       fprintf(cgiOut,"</tr>\n");
                       
                       fprintf(cgiOut,"<tr height=10><td></td></tr>");

     //////////////////////// Radius Client
     
                       fprintf(cgiOut,"<tr>\n");
                       fprintf(cgiOut,"<td style=\"font-weight:bold;font-size:12px\">\n");
                       fprintf(cgiOut,"<font id=yingwen_san>%s</font>","Radius Client");
                       fprintf(cgiOut,"</td>\n");
                       fprintf(cgiOut,"</tr>\n");
                        //分割线
                       fprintf(cgiOut, "<tr><td><hr width=100%% size=1 color=#fff align=center noshade /></td><td>"\
                       "<hr width=100%% size=1 color=#fff align=center noshade /></td><td>"\
                       "<hr width=100%% size=1 color=#fff align=center noshade /></td>"\
                       "</tr>" );


//serverip and pwd
					   memset(temp,0,N);
					   ret=find_p3_node(P3_XML_FPATH, P3_RADIUS, P3_SERIP, temp);//查找指定节点的值
					   
                       fprintf(cgiOut,"<tr height=5><td></td></tr>");
                       fprintf(cgiOut,"<tr>\n");
                       fprintf(cgiOut,"<td width=140>\n");
					   
					   fprintf(cgiOut,"%s:",search(lsystem,"p_radius"));
                
					   fprintf(cgiOut,"</td>\n");

					   #if 0
					   fprintf(cgiOut,"<td>\n");

					   if(strcmp(temp,"")!=0)
					   {	
					   
					   //fprintf(stderr,"1 %s",temp);
						   i=0;
						   tmp = strtok(temp,".");
						   //fprintf(stderr,"2-----tmp=%p",tmp);
						   while(tmp!= NULL)
						   {
							   i++;
							   if(i==1)
								   strcpy(serverip1,tmp);
							   else if(i ==2 )
								   strcpy(serverip2,tmp);
							   else if(i==3)
								   strcpy(serverip3,tmp);
							   else if(i==4)
								   strcpy(serverip4,tmp);
							   tmp = strtok(NULL,"."); 
							  
						   }

					   }
					   
						fprintf( cgiOut, "<div style='border-width:1px;border-color:#a5acb2;border-style:solid;width:140px;font-size:9pt'>");
						fprintf( cgiOut, "<input type=text name='serverip1' value=\"%s\" maxlength=3 class=a3 onKeyUp=\"mask(this,%s)\" onbeforepaste=mask_c() />.",serverip1,search(lpublic,"ip_error"));
						fprintf( cgiOut, "<input type=text name='serverip2' value=\"%s\" maxlength=3 class=a3 onKeyUp=\"mask(this,%s)\" onbeforepaste=mask_c() />.",serverip2,search(lpublic,"ip_error"));
						fprintf( cgiOut, "<input type=text name='serverip3' value=\"%s\" maxlength=3 class=a3 onKeyUp=\"mask(this,%s)\" onbeforepaste=mask_c() />.",serverip3,search(lpublic,"ip_error"));
						fprintf( cgiOut, "<input type=text name='serverip4' value=\"%s\" maxlength=3 class=a3 onKeyUp=\"mask(this,%s)\" onbeforepaste=mask_c() />",serverip4,search(lpublic,"ip_error"));
						fprintf( cgiOut, "</div>\n" );
						fprintf( cgiOut, "</td>\n" );

					#endif

					   fprintf( cgiOut, "<td>\n" );

					   memset(temp,0,N);
					   ret=find_p3_node(P3_XML_FPATH, P3_RADIUS, P3_PWD, temp);
					   fprintf(cgiOut,"<input type=text name=pwd value=\"%s\" style=\"width:70px\">",temp);
                       fprintf(cgiOut,"</td>\n");
                       fprintf(cgiOut,"</tr>\n");
//auth
					   memset(temp,0,N);
					   ret=find_p3_node(P3_XML_FPATH, P3_RADIUS, P3_AUTH, temp);
                       fprintf(cgiOut,"<tr height=5><td></td></tr>");
                       fprintf(cgiOut,"<tr>\n");
                       fprintf(cgiOut,"<td width=140>\n");
				// 	   fprintf(cgiOut,"<input type=\"radio\" name=\"showtype\" value=\"2\" >");
					   fprintf(cgiOut,"%s:",search(lsystem,"p_auth"));
					   fprintf(cgiOut,"</td>\n");
                       fprintf(cgiOut,"<td>\n");
					   
					   if(strcmp(temp,"")!=0)
					   {
							 //fprintf(stderr,"temp is %s",temp);
							 i=0;
							 tmp = strtok(temp,".");
							 //fprintf(stderr,"2-----tmp=%p",tmp);
							 while(tmp!= NULL)
							 {
								 i++;
								 if(i==1)
									 strcpy(authip1,tmp);
								 else if(i ==2 )
									 strcpy(authip2,tmp);
								 else if(i==3)
									 strcpy(authip3,tmp);
								 else if(i==4)
									 strcpy(authip4,tmp);
								 tmp = strtok(NULL,".");
								// fprintf(stderr,"3 4 5 6-----tmp=%p",tmp);
							 }
					   }

						 fprintf( cgiOut, "<div style='border-width:1px;border-color:#a5acb2;border-style:solid;width:140px;font-size:9pt'>");
						 fprintf( cgiOut, "<input type=text name='authip1' value=\"%s\" maxlength=3 class=a3 onKeyUp=\"mask(this,%s)\" onbeforepaste=mask_c() />.",authip1,search(lpublic,"ip_error"));
						 fprintf( cgiOut, "<input type=text name='authip2' value=\"%s\" maxlength=3 class=a3 onKeyUp=\"mask(this,%s)\" onbeforepaste=mask_c() />.",authip2,search(lpublic,"ip_error"));
						 fprintf( cgiOut, "<input type=text name='authip3' value=\"%s\" maxlength=3 class=a3 onKeyUp=\"mask(this,%s)\" onbeforepaste=mask_c() />.",authip3,search(lpublic,"ip_error"));
						 fprintf( cgiOut, "<input type=text name='authip4' value=\"%s\" maxlength=3 class=a3 onKeyUp=\"mask(this,%s)\" onbeforepaste=mask_c() />",authip4,search(lpublic,"ip_error"));
						 fprintf( cgiOut, "</div>\n" );
						 fprintf( cgiOut, "</td>\n" );

			/* add	 RadiusAuth_Port   Config  */
						fprintf( cgiOut, "<td>\n" );
						//int aport = 1812;
						memset(temp,0,N);
						ret=find_p3_node(P3_XML_FPATH, P3_RADIUS, P3_AUTH_PORT, temp);
						fprintf(cgiOut,"&nbsp;&nbsp;<input type=text name=authp value=\"%s\" style=\"width:50px\">",temp);
						fprintf(cgiOut,"</td>\n");
						fprintf(cgiOut,"</tr>\n");

//acct
                       memset(temp,0,N);
					   ret=find_p3_node(P3_XML_FPATH, P3_RADIUS, P3_ACCT, temp);
					   fprintf(cgiOut,"<tr height=5><td></td></tr>");
                       fprintf(cgiOut,"<tr>\n");
                       fprintf(cgiOut,"<td width=140>\n");
					//   fprintf(cgiOut,"<input type=\"radio\" name=\"showtype\" value=\"3\" >");
					   fprintf(cgiOut,"%s:",search(lsystem,"p_acct"));
					   fprintf(cgiOut,"</td>\n");
                       fprintf(cgiOut,"<td>\n");

						 if(strcmp(temp,"")!=0)
						 {	  
						 
							 //fprintf(stderr,"%s",temp);
							 i=0;
							 tmp = strtok(temp,".");
							 while(tmp!= NULL)
							 {
								 i++;
								 if(i==1)
									 strcpy(acctip1,tmp);
								 else if(i ==2 )
									 strcpy(acctip2,tmp);
								 else if(i==3)
									 strcpy(acctip3,tmp);
								 else if(i==4)
									 strcpy(acctip4,tmp);
								 tmp = strtok(NULL,"."); 
							 }				  
						 }

						 fprintf( cgiOut, "<div style='border-width:1px;border-color:#a5acb2;border-style:solid;width:140px;font-size:9pt'>");
						 fprintf( cgiOut, "<input type=text name='acctip1' value=\"%s\" maxlength=3 class=a3 onKeyUp=\"mask(this,%s)\" onbeforepaste=mask_c() />.",acctip1,search(lpublic,"ip_error"));
						 fprintf( cgiOut, "<input type=text name='acctip2' value=\"%s\" maxlength=3 class=a3 onKeyUp=\"mask(this,%s)\" onbeforepaste=mask_c() />.",acctip2,search(lpublic,"ip_error"));
						 fprintf( cgiOut, "<input type=text name='acctip3' value=\"%s\" maxlength=3 class=a3 onKeyUp=\"mask(this,%s)\" onbeforepaste=mask_c() />.",acctip3,search(lpublic,"ip_error"));
						 fprintf( cgiOut, "<input type=text name='acctip4' value=\"%s\" maxlength=3 class=a3 onKeyUp=\"mask(this,%s)\" onbeforepaste=mask_c() />",acctip4,search(lpublic,"ip_error"));
						 fprintf( cgiOut, "</div>\n" );
						 fprintf( cgiOut, "</td>\n" );
						 
			/* add   RadiusAcct_Port	Config	*/
						 fprintf( cgiOut, "<td>\n" );
						 //int bport = 1813;
						 memset(temp,0,N);
						 ret=find_p3_node(P3_XML_FPATH, P3_RADIUS, P3_ACCT_PORT, temp);
						 fprintf(cgiOut,"&nbsp;&nbsp;<input type=text name=acctp value=\"%s\" style=\"width:50px\">",temp);
						 fprintf(cgiOut,"</td>\n");
						 fprintf(cgiOut,"</tr>\n");

                       
                       fprintf(cgiOut,"<tr height=5><td></td></tr>");
					   
 ///////////////////Interface and IP
 
                       fprintf(cgiOut,"<tr>\n");
                       fprintf(cgiOut,"<td style=\"font-weight:bold;font-size:12px\">\n");
                       fprintf(cgiOut,"<font id=yingwen_san>%s</font>","Interface and IP");
                       fprintf(cgiOut,"</td>\n");
                       fprintf(cgiOut,"</tr>\n");
                       
                        //分割线
                       fprintf(cgiOut, "<tr><td><hr width=100%% size=1 color=#fff align=center noshade /></td><td>"\
                       "<hr width=100%% size=1 color=#fff align=center noshade /></td><td>"\
                       "<hr width=100%% size=1 color=#fff align=center noshade /></td>"\
                       "</tr>" );
                       
                       fprintf(cgiOut,"<tr height=7><td></td></tr>");
//max
					   memset(temp,0,N);
					   ret=find_p3_node(P3_XML_FPATH, P3_INF, P3_MAX, temp);
                       fprintf(cgiOut,"<tr height=5><td></td></tr>");
                       fprintf(cgiOut,"<tr>\n");
                       fprintf(cgiOut,"<td width=140>\n");
				//	   fprintf(cgiOut,"<input type=\"radio\" name=\"showtype\" value=\"4\" >");
                       fprintf(cgiOut,"%s:",search(lsystem,"p_range"));
					   fprintf(cgiOut,"</td>\n");
                       fprintf(cgiOut,"<td>\n");
                       fprintf(cgiOut,"<input type=text name=max value=\"%s\" size=21 />",temp);
                       fprintf(cgiOut,"</td>\n");
                       fprintf(cgiOut,"</tr>\n");
//base
                       memset(temp,0,N);
					   ret=find_p3_node(P3_XML_FPATH, P3_INF, P3_BASE, temp);
					   fprintf(cgiOut,"<tr height=5><td></td></tr>");
                       fprintf(cgiOut,"<tr>\n");
                       fprintf(cgiOut,"<td width=140>\n");
				//	   fprintf(cgiOut,"<input type=\"radio\" name=\"showtype\" value=\"5\" >");
                       fprintf(cgiOut,"%s:",search(lsystem,"p_base"));
					   fprintf(cgiOut,"</td>\n");
                       fprintf(cgiOut,"<td>\n");

						 if(strcmp(temp,"")!=0)
						 {	  
						 
							//fprintf(stderr,"%s",temp);
							 i=0;
							 tmp = strtok(temp,".");
							 while(tmp!= NULL)
							 {
								 i++;
								 if(i==1)
									 strcpy(baseip1,tmp);
								 else if(i ==2 )
									 strcpy(baseip2,tmp);
								 else if(i==3)
									 strcpy(baseip3,tmp);
								 else if(i==4)
									 strcpy(baseip4,tmp);
								 tmp = strtok(NULL,"."); 
							//fprintf(stderr,"%s",temp);
							 }				  
						 }

					 fprintf( cgiOut, "<div style='border-width:1px;border-color:#a5acb2;border-style:solid;width:140px;font-size:9pt'>");
					 fprintf( cgiOut, "<input type=text name='baseip1' value=\"%s\" maxlength=3 class=a3 onKeyUp=\"mask(this,%s)\" onbeforepaste=mask_c() />.",baseip1,search(lpublic,"ip_error"));
					 fprintf( cgiOut, "<input type=text name='baseip2' value=\"%s\" maxlength=3 class=a3 onKeyUp=\"mask(this,%s)\" onbeforepaste=mask_c() />.",baseip2,search(lpublic,"ip_error"));
					 fprintf( cgiOut, "<input type=text name='baseip3' value=\"%s\" maxlength=3 class=a3 onKeyUp=\"mask(this,%s)\" onbeforepaste=mask_c() />.",baseip3,search(lpublic,"ip_error"));
					 fprintf( cgiOut, "<input type=text name='baseip4' value=\"%s\" maxlength=3 class=a3 onKeyUp=\"mask(this,%s)\" onbeforepaste=mask_c() />",baseip4,search(lpublic,"ip_error"));
					 fprintf( cgiOut, "</div>\n" );
					 fprintf( cgiOut, "</td>\n" );
                     fprintf(cgiOut,"</tr>\n");

//localip
                       memset(temp,0,N);
					   ret=find_p3_node(P3_XML_FPATH, P3_INF, P3_MYIP, temp);
					   fprintf(cgiOut,"<tr height=5><td></td></tr>");
                       fprintf(cgiOut,"<tr>\n");
                       fprintf(cgiOut,"<td width=140>\n");
					 //  fprintf(cgiOut,"<input type=\"radio\" name=\"showtype\" value=\"6\" >");
                       fprintf(cgiOut,"%s:",search(lsystem,"p_serip"));
					   fprintf(cgiOut,"</td>\n");
                       fprintf(cgiOut,"<td>\n");
                       
						 if(strcmp(temp,"")!=0)
						 {	  
						 
							//fprintf(stderr,"%s",temp);
						 i=0;
						 tmp = strtok(temp,".");
						 while(tmp!= NULL)
						 {
							 i++;
							 if(i==1)
								 strcpy(localip1,tmp);
							 else if(i ==2 )
								 strcpy(localip2,tmp);
							 else if(i==3)
								 strcpy(localip3,tmp);
							 else if(i==4)
								 strcpy(localip4,tmp);
							 tmp = strtok(NULL,"."); 
							//fprintf(stderr,"%s",temp);
						 }				  
					 }

					 fprintf( cgiOut, "<div style='border-width:1px;border-color:#a5acb2;border-style:solid;width:140px;font-size:9pt'>");
					 fprintf( cgiOut, "<input type=text name='localip1' value=\"%s\" maxlength=3 class=a3 onKeyUp=\"mask(this,%s)\" onbeforepaste=mask_c() />.",localip1,search(lpublic,"ip_error"));
					 fprintf( cgiOut, "<input type=text name='localip2' value=\"%s\" maxlength=3 class=a3 onKeyUp=\"mask(this,%s)\" onbeforepaste=mask_c() />.",localip2,search(lpublic,"ip_error"));
					 fprintf( cgiOut, "<input type=text name='localip3' value=\"%s\" maxlength=3 class=a3 onKeyUp=\"mask(this,%s)\" onbeforepaste=mask_c() />.",localip3,search(lpublic,"ip_error"));
					 fprintf( cgiOut, "<input type=text name='localip4' value=\"%s\" maxlength=3 class=a3 onKeyUp=\"mask(this,%s)\" onbeforepaste=mask_c() />",localip4,search(lpublic,"ip_error"));
					 fprintf( cgiOut, "</div>\n" );
					 fprintf( cgiOut, "</td>\n" );
                     fprintf(cgiOut,"</tr>\n");                   
//port
                       memset(temp,0,N);
					   ret=find_p3_node(P3_XML_FPATH, P3_INF, P3_PORT, temp);
					   fprintf(cgiOut,"<tr height=5><td></td></tr>");
                       fprintf(cgiOut,"<tr>\n");
                       fprintf(cgiOut,"<td width=140>\n");
					//   fprintf(cgiOut,"<input type=\"radio\" name=\"showtype\" value=\"7\" >");
                       fprintf(cgiOut,"%s:",search(lsystem,"p_listen"));
					   fprintf(cgiOut,"</td>");
					   fprintf(cgiOut,"<td>");
                       fprintf(cgiOut,"<input type=text name=port value=\"%s\" size=21>",temp);	
                       fprintf(cgiOut,"</td>\n");
                       fprintf(cgiOut,"</tr>\n");
                       
                       fprintf(cgiOut,"<tr height=13><td></td></tr>");
					   
 /////////////////DNS and IP
 
                       fprintf(cgiOut,"<tr>\n");
                       fprintf(cgiOut,"<td style=\"font-weight:bold;font-size:12px\">\n");
                       fprintf(cgiOut,"<font id=yingwen_san>%s</font>","DNS Server IP");
                       fprintf(cgiOut,"</td>\n");
                       fprintf(cgiOut,"</tr>\n");
                       
                       
                        //分割线
                       fprintf(cgiOut, "<tr><td><hr width=100%% size=1 color=#fff align=center noshade /></td><td>"\
                       "<hr width=100%% size=1 color=#fff align=center noshade /></td><td>"\
                       "<hr width=100%% size=1 color=#fff align=center noshade /></td>"\
                       "</tr>" );
                        
                       fprintf(cgiOut,"<tr height=7><td></td></tr>");    
//hostip					   
                       memset(temp,0,N);
					   ret=find_p3_node(P3_XML_FPATH, P3_DNS, P3_HIP, temp);
					   
                       fprintf(cgiOut,"<tr>\n");
                       fprintf(cgiOut,"<td width=140>\n");
					//   fprintf(cgiOut,"<input type=\"radio\" name=\"showtype\" value=\"8\" >");
                       fprintf(cgiOut,"%s:","DNS1");
					   fprintf(cgiOut,"</td>\n");
                       fprintf(cgiOut,"<td>\n");

						 if(strcmp(temp,"")!=0)
						 {	  
						 
							//	 fprintf(stderr,"%s",temp);
							 i=0;
							 tmp = strtok(temp,".");
							 while(tmp!= NULL)
							 {
								 i++;
								 if(i==1)
									 strcpy(hostip1,tmp);
								 else if(i ==2 )
									 strcpy(hostip2,tmp);
								 else if(i==3)
									 strcpy(hostip3,tmp);
								 else if(i==4)
									 strcpy(hostip4,tmp);
								 tmp = strtok(NULL,"."); 
								//fprintf(stderr,"%s",temp);
							 }				  
						 }

						 fprintf( cgiOut, "<div style='border-width:1px;border-color:#a5acb2;border-style:solid;width:140px;font-size:9pt'>");
						 fprintf( cgiOut, "<input type=text name='hostip1' value=\"%s\" maxlength=3 class=a3 onKeyUp=\"mask(this,%s)\" onbeforepaste=mask_c() />.",hostip1,search(lpublic,"ip_error"));
						 fprintf( cgiOut, "<input type=text name='hostip2' value=\"%s\" maxlength=3 class=a3 onKeyUp=\"mask(this,%s)\" onbeforepaste=mask_c() />.",hostip2,search(lpublic,"ip_error"));
						 fprintf( cgiOut, "<input type=text name='hostip3' value=\"%s\" maxlength=3 class=a3 onKeyUp=\"mask(this,%s)\" onbeforepaste=mask_c() />.",hostip3,search(lpublic,"ip_error"));
						 fprintf( cgiOut, "<input type=text name='hostip4' value=\"%s\" maxlength=3 class=a3 onKeyUp=\"mask(this,%s)\" onbeforepaste=mask_c() />",hostip4,search(lpublic,"ip_error"));
						 fprintf( cgiOut, "</div>\n" );
						 fprintf( cgiOut, "</td>\n" );
                       	 fprintf(cgiOut,"</tr>\n");


					   fprintf(cgiOut,"<tr height=7><td></td></tr>");                       
//backupip
                       memset(temp,0,N);
					   ret=find_p3_node(P3_XML_FPATH, P3_DNS, P3_BIP, temp);
					   
					   fprintf(cgiOut,"<tr>\n");
                       fprintf(cgiOut,"<td width=140>\n");
					//   fprintf(cgiOut,"<input type=\"radio\" name=\"showtype\" value=\"9\" >");
                       fprintf(cgiOut,"%s:","DNS2");
					   fprintf(cgiOut,"</td>\n");
                       fprintf(cgiOut,"<td>\n");

						 if(strcmp(temp,"")!=0)
						 {	  
							 i=0;
							 tmp = strtok(temp,".");
							 while(tmp!= NULL)
							 {
								 i++;
								 if(i==1)
									 strcpy(backupip1,tmp);
								 else if(i ==2 )
									 strcpy(backupip2,tmp);
								 else if(i==3)
									 strcpy(backupip3,tmp);
								 else if(i==4)
									 strcpy(backupip4,tmp);
								 tmp = strtok(NULL,"."); 
								 //fprintf(stderr,"%s",temp);
							 }				  
						 }

						 fprintf( cgiOut, "<div style='border-width:1px;border-color:#a5acb2;border-style:solid;width:140px;font-size:9pt'>");
						 fprintf( cgiOut, "<input type=text name='backupip1' value=\"%s\" maxlength=3 class=a3 onKeyUp=\"mask(this,%s)\" onbeforepaste=mask_c() />.",backupip1,search(lpublic,"ip_error"));
						 fprintf( cgiOut, "<input type=text name='backupip2' value=\"%s\" maxlength=3 class=a3 onKeyUp=\"mask(this,%s)\" onbeforepaste=mask_c() />.",backupip2,search(lpublic,"ip_error"));
						 fprintf( cgiOut, "<input type=text name='backupip3' value=\"%s\" maxlength=3 class=a3 onKeyUp=\"mask(this,%s)\" onbeforepaste=mask_c() />.",backupip3,search(lpublic,"ip_error"));
						 fprintf( cgiOut, "<input type=text name='backupip4' value=\"%s\" maxlength=3 class=a3 onKeyUp=\"mask(this,%s)\" onbeforepaste=mask_c() />",backupip4,search(lpublic,"ip_error"));
						 fprintf( cgiOut, "</div>\n" );
						 fprintf( cgiOut, "</td>\n" );                     
                       	 fprintf(cgiOut,"</tr>\n");
                       	 

					   fprintf(cgiOut,"<tr height=7><td></td></tr>");  
                      
                       fprintf(cgiOut,"<tr>\n");
                       fprintf(cgiOut,"<td style=\"font-weight:bold;font-size:12px\">\n");
                       fprintf(cgiOut,"<font id=yingwen_san>%s</font>","HOSTS");
                       fprintf(cgiOut,"</td>\n");
                       fprintf(cgiOut,"</tr>\n");					   
//host 					   
                        //分割线
                       fprintf(cgiOut, "<tr><td><hr width=100%% size=1 color=#fff align=center noshade /></td><td>"\
                       "<hr width=100%% size=1 color=#fff align=center noshade /></td><td>"\
                       "<hr width=100%% size=1 color=#fff align=center noshade /></td>"\
                       "</tr>" );

					   fprintf(cgiOut,"<tr height=5><td></td></tr>");  

					   memset(temp,0,N);
					   ret=find_p3_node(P3_XML_FPATH, P3_HOST, P3_HOSTIP, temp);					   
                       
                       fprintf(cgiOut,"<tr>\n");
                       fprintf(cgiOut,"<td width=140>\n");
               //        fprintf(cgiOut,"<input type=\"radio\" name=\"showtype\" value=\"10\">");
					   fprintf(cgiOut,"%s:",search(lsystem,"p_host"));
					   fprintf(cgiOut,"</td>\n");
                       fprintf(cgiOut,"<td>\n");

						if(strcmp(temp,"")!=0)
						 {	  
						 
							 //fprintf(stderr,"%s",temp);
							 i=0;
							 tmp = strtok(temp,".");
							 while(tmp!= NULL)
							 {
								 i++;
								 if(i==1)
									 strcpy(ip1,tmp);
								 else if(i ==2 )
									 strcpy(ip2,tmp);
								 else if(i==3)
									 strcpy(ip3,tmp);
								 else if(i==4)
									 strcpy(ip4,tmp);
								 tmp = strtok(NULL,"."); 
								 //fprintf(stderr,"%s",temp);
							 }				  
						 }

						 fprintf( cgiOut, "<div style='border-width:1px;border-color:#a5acb2;border-style:solid;width:140px;font-size:9pt'>");
						 fprintf( cgiOut, "<input type=text name='ip1' value=\"%s\" maxlength=3 class=a3 onKeyUp=\"mask(this,%s)\" onbeforepaste=mask_c() />.",ip1,search(lpublic,"ip_error"));
						 fprintf( cgiOut, "<input type=text name='ip2' value=\"%s\" maxlength=3 class=a3 onKeyUp=\"mask(this,%s)\" onbeforepaste=mask_c() />.",ip2,search(lpublic,"ip_error"));
						 fprintf( cgiOut, "<input type=text name='ip3' value=\"%s\" maxlength=3 class=a3 onKeyUp=\"mask(this,%s)\" onbeforepaste=mask_c() />.",ip3,search(lpublic,"ip_error"));
						 fprintf( cgiOut, "<input type=text name='ip4' value=\"%s\" maxlength=3 class=a3 onKeyUp=\"mask(this,%s)\" onbeforepaste=mask_c() />",ip4,search(lpublic,"ip_error"));
						 fprintf( cgiOut, "</div>\n" );
						 fprintf( cgiOut, "</td>\n" );
						 fprintf( cgiOut, "<td>\n" );

//hostname
					   memset(temp,0,N);
					   ret=find_p3_node(P3_XML_FPATH, P3_HOST, P3_HOSTNAME, temp);
					   fprintf(cgiOut,"&nbsp;&nbsp;<input type=text name=hostname value=\"%s\" style=\"width:70px\">",temp);
                       fprintf(cgiOut,"</td>\n");                      
                       fprintf(cgiOut,"</tr>\n");
						
  /////////////Boot service
    //分割线
                       fprintf(cgiOut, "<tr><td><hr width=100%% size=1 color=#fff align=center noshade /></td><td>"\
                       "<hr width=100%% size=1 color=#fff align=center noshade /></td><td>"\
                       "<hr width=100%% size=1 color=#fff align=center noshade /></td>"\
                       "</tr>" );

  
                       fprintf(cgiOut,"<tr height=7><td></td></tr>");
                       
                       fprintf(cgiOut,"<tr height=5><td></td></tr>");
                       fprintf(cgiOut,"<tr>\n");
                       fprintf(cgiOut,"<td>\n");
                       fprintf(cgiOut,"<input type=submit name=boot value=\"%s\">",search(lsystem,"p_boot"));
					   fprintf(cgiOut,"<td>\n");
                       fprintf(cgiOut,"<input type=submit name=halt value=\"%s\">",search(lsystem,"p_halt"));
                       fprintf(cgiOut,"</td>\n");
                       fprintf(cgiOut,"</td>\n");
                       fprintf(cgiOut,"</tr>\n");
                       
                       
                       
                       
//------------------------------------>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
			 								
   fprintf(cgiOut,"</table>"); 
			  fprintf(cgiOut,"</td>"\
			  "</tr>"\
			  "<tr height=4 valign=top>"\
				  "<td width=120 height=4 align=right valign=top><img src=/images/bottom_07.gif width=1 height=10/></td>"\
				  "<td width=827 height=4 valign=top bgcolor=#FFFFFF><img src=/images/bottom_06.gif width=827 height=15/></td>"\
			  "</tr>"\
			"</table>");
		  fprintf(cgiOut,"</td>"\
		  "<td width=15 background=/images/di999.jpg>&nbsp;</td>"\
		"</tr>"\
	  "</table></td>"); 
	fprintf(cgiOut,"</tr>"\
	"<tr>"\
		"<td colspan=3 align=left valign=top background=/images/di777.jpg><img src=/images/di555.jpg width=61 height=62/></td>"\
	  "<td align=left valign=top background=/images/di777.jpg>&nbsp;</td>"\
		"<td align=left valign=top background=/images/di777.jpg><img src=/images/di666.jpg width=74 height=62/></td>"\
	"</tr>"\
	"</table>");
	fprintf(cgiOut,"</div>"\
	"</form>"\
	"</body>"\
"</html>");
free(encry);
free(temp);
//free(tmp);
return 0;
}  

