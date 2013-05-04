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
* wp_log_mod.c
*
*
* CREATOR:
* autelan.software.Network Dep. team
* zhouym@autelan.com
*
* DESCRIPTION:
* system function for syslog config
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
#include <fcntl.h>
#include <sys/wait.h>
#include "ws_log_conf.h"

int ShowLogconfPage(struct list *lcontrol,struct list *lpublic);

int cgiMain()
{
	
	
	struct list *lcontrol;
	struct list *lpublic;
	lcontrol = get_chain_head("../htdocs/text/control.txt");
	lpublic= get_chain_head("../htdocs/text/public.txt");
	
	ShowLogconfPage(lcontrol,lpublic);
	release(lcontrol);
	release(lpublic); 
	return 0;
}



int ShowLogconfPage(struct list *lcontrol,struct list *lpublic)
{ 
  char *encry=(char *)malloc(BUF_LEN);
 
  char *str;
  
  char deb[128];
  memset(deb,0,128);
  


  char file_name[128];  //读取文件
  memset(file_name,0,128);

  char zstring[128];  //父串
  memset(zstring,0,128);

  char subs[128];   //子串
  memset(subs,0,128);
 
  char log_encry[BUF_LEN]; 
 
  int ret ;  /*命令及命令执行结果*/
  int op_ret;

  int j = 0; 
  int i=-1;
  int ir=0;
  
  int flag=1;  /*标志这状态*/
  
  char showtype[N];
  memset(showtype,0,N);  
   
  char dstring[128];    /*插入的规则行内容*/
  memset(dstring,0,128); 

  char dstring1[128];    /*插入的规则行内容*/
  memset(dstring1,0,128); 

  char dstring2[128];    /*插入的规则行内容*/
  memset(dstring2,0,128); 

  char dstring3[128];    /*插入的规则行内容*/
  memset(dstring3,0,128); 

  char tcpname[N];    /*下拉框内容*/
  memset(tcpname,0,N); 

  ST_SYS_ALL sysall;  /*总的结构体内容*/
  memset(&sysall,0,sizeof(sysall));

  ST_LOG_KEY logkey;
  memset(&logkey,0,sizeof(logkey));

  char *fpath;
  fpath=XML_FPATH;
  
  /*要修改的内容*/
  char content[128];
  memset(content,0,128);
  
  cgiFormStringNoNewlines("Nb", file_name, 128);

 
  if(cgiFormSubmitClicked("version") != cgiFormSuccess)
  	{
	memset(encry,0,BUF_LEN);
    cgiFormStringNoNewlines("UN", encry, BUF_LEN); 
    str=dcryption(encry);
    if(str==NULL)
    {
      ShowErrorPage(search(lpublic,"ill_user")); 	 /*用户非法*/
	}
	
	memset(log_encry,0,BUF_LEN);                   /*清空临时变量*/
  }
  else
  	{
  	cgiFormStringNoNewlines("encry_version", log_encry, BUF_LEN); 
    str=dcryption(log_encry);
    if(str==NULL)
    {
      ShowErrorPage(search(lpublic,"ill_user")); 	 /*用户非法*/
	}
	
	memset(log_encry,0,BUF_LEN);                   /*清空临时变量*/
	
  	}
 
  cgiFormStringNoNewlines("encry_version",log_encry,BUF_LEN);
  
  cgiHeaderContentType("text/html");
  fprintf(cgiOut,"<html xmlns=\"http://www.w3.org/1999/xhtml\"><head>");
  fprintf(cgiOut,"<meta http-equiv=Content-Type content=text/html; charset=gb2312>");
 
  fprintf(cgiOut,"<link rel=stylesheet href=/style.css type=text/css>"\
  		"<style type=text/css>"\
  	".a3{width:30;border:0; text-align:center}"\
  	"</style>"\
  "</head>"\
  "<script language=javascript src=/ip.js>"\
  "</script>"\
  	"<script type='text/javascript'>"\
		"function changestate(){"\
		"var fl = document.getElementsByName('showtype')[0];"\
		"var tcd = document.getElementsByName('showtype')[1];"\
		"var user = document.getElementsByName('showtype')[2];"\
		"}"\
		"</script>"\
  	"<body>");   
   
  /*----------------------------begin-----------------------------------------------*/
  /*
   $1 : 传过来的目的规则名
   $2 : syslog 文件
   $3 : 临时文件名
   $4 : 添加的新规则名   
  */
     /* 重启服务*/
    if(cgiFormSubmitClicked("reboot") == cgiFormSuccess)
    	{
    	int ret;
    	ret=restart_syslog();
		if(ret==0)
		ShowAlert(search(lpublic,"oper_succ"));
	    else
		ShowAlert(search(lpublic,"oper_fail"));
    	}

   
	/*恢复默认,删除旧有文件并重新生成新文件,先删除xml文件，读取信息是从xml来的*/	 
	if(cgiFormSubmitClicked("default") == cgiFormSuccess)
		{
		
		sprintf(deb,"sudo rm %s",XML_FPATH);
        ret=system(deb);

        memset(deb,0,128);
		sprintf(deb,"sudo rm %s",CONF_FPATH);
		op_ret=system(deb);

		restart_syslog();
		
        if(ret==0 && op_ret==0)
		ShowAlert(search(lpublic,"oper_succ"));
	    else
		ShowAlert(search(lpublic,"oper_fail"));
       
		
		
		}
	

	/*保存更改 此页面主要是修改log 信息的，其他配置另有页面*/
	if(cgiFormSubmitClicked("submit_mod") == cgiFormSuccess)
    {
         	  cgiFormStringNoNewlines("showtype", showtype, N); 
         	 
			  int result;   
			  char **responses; 
			  
			  result = cgiFormStringMultiple("showtype", &responses);  //多选框的操作
	  

			if((strcmp(showtype,"1")!=0)&&(strcmp(showtype,"3")!=0)&&(strcmp(showtype,"2")!=0))
				{
					 ShowAlert(search(lpublic,"log_no_opt"));
				}
			    else{

					 i = 0;
                    while(responses[i])
                    {
                        i++;
					}

          
		     /*只选 1，3 的双选情况
		       修改的是log的 content 信息，这样相对conf比较稳定，要找到value的关键字
		     */
			 if(i==2 && (strcmp(responses[0],"1")==0)&&(strcmp(responses[1],"3")==0))
			 	{
			  char temp[60];
			  memset(temp,0,60);            

    		  ir=find_log_node(fpath,NODE_DES,NODE_ATT,file_name,NODE_VALUE,&logkey);
              sprintf(dstring1,"%s;",logkey.key);	

			  memset(&logkey,0,sizeof(logkey)); 
    		  ir=find_log_node(fpath,NODE_DES,NODE_ATT,L_ALL,NODE_VALUE,&logkey);	
    		  sprintf(temp,"%s;",logkey.key);	
    
    		  strcat(dstring1,temp);
			  //fprintf(cgiOut,"zone:          %s",dstring1);
			  
    		  mod_log_node(fpath, NODE_LOG, NODE_ATT,file_name, CH_DEST,dstring1);
			  read_filter(fpath, NODE_LOG, &sysall);
			  ret=write_config(&sysall,CONF_FPATH);


				
	          if(ret==0)
		      ShowAlert(search(lpublic,"oper_succ"));
	          else
		      ShowAlert(search(lpublic,"oper_fail"));
			 	}
              
			 //只选 1，2 的双选情况
			 if(i==2 && (strcmp(responses[0],"1")==0)&&(strcmp(responses[1],"2")==0))
			 	{
                      		
                   		  char temp[60];
            			  memset(temp,0,60); 
            
                		  ir=find_log_node(fpath,NODE_DES,NODE_ATT,file_name,NODE_VALUE,&logkey);	
                          sprintf(dstring1,"%s;",logkey.key);	

						  //fprintf(cgiOut,"fen:         %s<br>",dstring1);
						  
                		  memset(&logkey,0,sizeof(logkey)); 
                		  ir=find_log_node(fpath,NODE_DES,NODE_ATT,L_IP,NODE_VALUE,&logkey);	
                		  sprintf(temp,"%s;",logkey.key);							  
              
                		  strcat(dstring1,temp);
            			  //fprintf(cgiOut,"zone:          %s<br>",dstring1);
            			  
                		  mod_log_node(fpath, NODE_LOG, NODE_ATT, file_name, CH_DEST,dstring1);
            			  read_filter(fpath, NODE_LOG, &sysall);
            			  ret=write_config(&sysall,CONF_FPATH);
			  
				          if(ret==0)
					      ShowAlert(search(lpublic,"oper_succ"));
				          else
					      ShowAlert(search(lpublic,"oper_fail"));  
			 }
  
			 //只选 2，3 的双选情况
			 if(i==2 && (strcmp(responses[0],"2")==0)&&(strcmp(responses[1],"3")==0))
			 	{
         		          char temp[60];
            			  memset(temp,0,60); 
            
                		  ir=find_log_node(fpath,NODE_DES,NODE_ATT,L_IP,NODE_VALUE,&logkey);	
                          sprintf(dstring1,"%s;",logkey.key);	

						  //fprintf(cgiOut,"fen:         %s<br>",dstring1);
						  
                		  memset(&logkey,0,sizeof(logkey)); 
                		  ir=find_log_node(fpath,NODE_DES,NODE_ATT,L_ALL,NODE_VALUE,&logkey);	
                		  sprintf(temp,"%s;",logkey.key);							  
              
                		  strcat(dstring1,temp);
            			  //fprintf(cgiOut,"zone:          %s<br>",dstring1);
            			  
                		  mod_log_node(fpath, NODE_LOG, NODE_ATT, file_name, CH_DEST,dstring1);
            			  read_filter(fpath, NODE_LOG, &sysall);
            			  ret=write_config(&sysall,CONF_FPATH);
			  
				          if(ret==0)
					      ShowAlert(search(lpublic,"oper_succ"));
				          else
					      ShowAlert(search(lpublic,"oper_fail"));
                   	
			 	}

			

             //三个框都选的情况
			 if(i==3 && (strcmp(responses[0],"1")==0)&&(strcmp(responses[1],"2")==0)&&(strcmp(responses[2],"3")==0))
			 	{
                
			  char temp[60];
			  memset(temp,0,60);            

    		  ir=find_log_node(fpath,NODE_DES,NODE_ATT,file_name,NODE_VALUE,&logkey);	
              sprintf(dstring1,"%s;",logkey.key);   
    		 // fprintf(cgiOut,"fen:         %s<br>",dstring1);	

   			  memset(&logkey,0,sizeof(logkey)); 
			  ir=find_log_node(fpath,NODE_DES,NODE_ATT,L_IP,NODE_VALUE,&logkey);	
              sprintf(temp,"%s;",logkey.key);	
			  strcat(dstring1,temp);


			  memset(&logkey,0,sizeof(logkey)); 
			  memset(temp,0,60);   
			  ir=find_log_node(fpath,NODE_DES,NODE_ATT,L_ALL,NODE_VALUE,&logkey);	
    		  sprintf(temp,"%s;",logkey.key);	
    
    		  strcat(dstring1,temp);
			  //fprintf(cgiOut,"zone:          %s",dstring1);
			  
    		  mod_log_node(fpath, NODE_LOG, NODE_ATT,file_name, CH_DEST,dstring1);
			  read_filter(fpath, NODE_LOG, &sysall);
			  ret=write_config(&sysall,CONF_FPATH);


				
	          if(ret==0)
		      ShowAlert(search(lpublic,"oper_succ"));
	          else
		      ShowAlert(search(lpublic,"oper_fail"));
			 	}

			  
            /*单选一 
            */
     
			 if(i==1 && (strcmp(responses[0],"1")==0))
         	 { 
				 
                  memset(dstring1,0,128); 

        		  ir=find_log_node(fpath,NODE_DES,NODE_ATT,file_name,NODE_VALUE,&logkey);	
                  sprintf(dstring1,"%s;",logkey.key);	
    
    			//  fprintf(cgiOut,"fen:         %s<br>",dstring1);	      
    			  
        		  mod_log_node(fpath, NODE_LOG, NODE_ATT, file_name, CH_DEST,dstring1);
    			  read_filter(fpath, NODE_LOG, &sysall);
				  
				 // fprintf(cgiOut,"test:  %d<br>",sysall.des_num);
				  
    			  ret=write_config(&sysall,CONF_FPATH);
      
    	          if(ret==0)
    		      ShowAlert(search(lpublic,"oper_succ"));
    	          else
    		      ShowAlert(search(lpublic,"oper_fail"));

              	 }

			 //单选二 
         	  if(i==1 && (strcmp(responses[0],"2")==0))
         	 {
         	  		
        		  ir=find_log_node(fpath,NODE_DES,NODE_ATT,L_IP,NODE_VALUE,&logkey);	
                  sprintf(dstring1,"%s;",logkey.key);	
    
    			 // fprintf(cgiOut,"fen:         %s<br>",dstring1);	      
    			  
        		  mod_log_node(fpath, NODE_LOG, NODE_ATT, file_name, CH_DEST,dstring1);
    			  read_filter(fpath, NODE_LOG, &sysall);
    			  ret=write_config(&sysall,CONF_FPATH);
      
    	          if(ret==0)
    		      ShowAlert(search(lpublic,"oper_succ"));
    	          else
    		      ShowAlert(search(lpublic,"oper_fail"));

			   
         	 }

			 //单选三
         	 if(i==1 && (strcmp(responses[0],"3")==0))
         	  {
               				 
        		  ir=find_log_node(fpath,NODE_DES,NODE_ATT,L_ALL,NODE_VALUE,&logkey);	
                  sprintf(dstring1,"%s;",logkey.key);	
    
    			 // fprintf(cgiOut,"fen:         %s<br>",dstring1);	      
    			  
        		  mod_log_node(fpath, NODE_LOG, NODE_ATT, file_name, CH_DEST,dstring1);
    			  read_filter(fpath, NODE_LOG, &sysall);				  
    			  ret=write_config(&sysall,CONF_FPATH);
				 // fprintf(cgiOut,"<br>ir:   %d",ret);
     
    	          if(ret==0)
    		      ShowAlert(search(lpublic,"oper_succ"));
    	          else
    		      ShowAlert(search(lpublic,"oper_fail"));

				
         	  }
     	
		}

		cgiStringArrayFree(responses);	 
 }


   
   
					  /*----------------------------- end ----------------------------------------------*/
		   
  fprintf(cgiOut,"<form method=post encType=multipart/form-data>"\
  "<div align=center>"\
  "<table width=976 border=0 cellpadding=0 cellspacing=0>");  //111111111111111111111
  fprintf(cgiOut,"<tr>"\
    "<td width=8 align=left valign=top background=/images/di22.jpg><img src=/images/youce4.jpg width=8 height=30/></td>"\
    "<td width=51 align=left valign=bottom background=/images/di22.jpg><img src=/images/youce33.jpg width=37 height=24/></td>"\
    "<td width=153 align=left valign=bottom id=%s background=/images/di22.jpg>%s</td>",search(lpublic,"title_style"),search(lpublic,"log_info"));
    fprintf(cgiOut,"<td width=690 align=right valign=bottom background=/images/di22.jpg>");
	   
        // 鉴权
      fprintf(cgiOut,"<input type=hidden name=UN value=%s />",encry);
	  fprintf(cgiOut,"<input type=hidden name=Nb value=%s />",file_name);  //取到传送的值 
	   	
		  fprintf(cgiOut,"<table width=130 border=0 cellspacing=0 cellpadding=0>"\
		  "<tr>"\
		  "<td width=62 align=center><input id=but type=submit name=submit_mod style=background-image:url(/images/%s) value=""></td>",search(lpublic,"img_ok"));		  
		  if(cgiFormSubmitClicked("submit_mod") != cgiFormSuccess)
			fprintf(cgiOut,"<td width=62 align=left><a href=wp_log_info.cgi?UN=%s target=mainFrame><img src=/images/%s border=0 width=62 height=20/></a></td>",encry,search(lpublic,"img_cancel"));
		  else										   
			fprintf(cgiOut,"<td width=62 align=left><a href=wp_log_info.cgi?UN=%s target=mainFrame><img src=/images/%s border=0 width=62 height=20/></a></td>",log_encry,search(lpublic,"img_cancel"));
		  fprintf(cgiOut,"</tr>"\
		  "</table>");			
		
	fprintf(cgiOut,"</td>"\
    "<td width=74 align=right valign=top background=/images/di22.jpg><img src=/images/youce3.jpg width=31 height=30/></td>"\
  "</tr>"\
  "<tr>");
    fprintf(cgiOut,"<td colspan=5 align=center valign=middle><table width=976 border=0 cellpadding=0 cellspacing=0 bgcolor=#f0eff0>");
                            //333333333333333333333
	  fprintf(cgiOut,"<tr>");
        fprintf(cgiOut,"<td width=12 align=left valign=top background=/images/di888.jpg>&nbsp;</td>"\
        "<td width=948><table width=947 border=0 cellspacing=0 cellpadding=0>"); //44444444444444444444		
            fprintf(cgiOut,"<tr height=4 valign=bottom>"\
              "<td width=120>&nbsp;</td>"\
              "<td width=827 valign=bottom><img src=/images/bottom_05.gif width=827 height=4/></td>"\
            "</tr>");
	fprintf(cgiOut,"<tr>");  //次内
              fprintf(cgiOut,"<td><table width=120 border=0 cellspacing=0 cellpadding=0>"); //555555555555555
                   fprintf(cgiOut,"<tr height=25>"\
                    "<td id=tdleft>&nbsp;</td>"\
                  "</tr>");

			     
					
					fprintf(cgiOut,"<tr height=26>"\
							"<td align=left id=tdleft background=/images/bottom_bg.gif style=\"border-right:0\"><font id=%s>%s</font></td>",search(lpublic,"menu_san"),search(lpublic,"log_modrule"));    /*突出显示*/
					fprintf(cgiOut,"</tr>");

                       fprintf(cgiOut,"<tr height=25>"\
					  "<td align=left id=tdleft><a href=wp_log_add.cgi?UN=%s&Nb=%s target=mainFrame class=top><font id=%s>%s</font></a></td>",encry,file_name,search(lpublic,"menu_san"),search(lpublic,"log_addip"));
					fprintf(cgiOut,"</tr>");
				
						 for(j=0;j<13;j++)
								{
									fprintf(cgiOut,"<tr height=25>"\
									  "<td id=tdleft>&nbsp;</td>"\
									"</tr>");
								}
								
								   fprintf(cgiOut,"</table>"); //555555555555555555555
							   fprintf(cgiOut,"</td>"\
							   "<td align=left valign=top style=\"background-color:#ffffff; border-right:1px solid #707070; padding-left:30px; padding-top:10px\">");
								 
				         fprintf(cgiOut,"<table width=460 border=0 cellspacing=0 cellpadding=0>");	 //66666666666666666				 
					  
							 fprintf(cgiOut,"<tr>");   //内
							  fprintf(cgiOut,"<td colspan=2 style=\"padding-top:20px\">");
								 fprintf(cgiOut,"<table width=460 border=0 bordercolor=#cccccc cellspacing=0 cellpadding=0>");
								 

                                      //777777777777777777777

					
									  
                     memset(showtype,0,N);
					 cgiFormStringNoNewlines("showtype", showtype, N);
                    
					 fprintf(cgiOut,"<tr><td>");
                     fprintf(cgiOut,"<input type=submit name=reboot value=\"%s\">",search(lpublic,"log_reboot"));				
					 fprintf(cgiOut,"<input type=submit name=default value=\"%s\"></td>",search(lpublic,"log_def"));
					 fprintf(cgiOut,"</tr>");

					 fprintf(cgiOut,"<tr height=12><td></td></tr>");

					 
					 fprintf(cgiOut,"<tr><td>");
                     fprintf(cgiOut, "<div class=\"col1\"> \n" );
                     fprintf(cgiOut, "<label class=\"col1\"> \n" );
                     fprintf(cgiOut, "<h4>%s</h4> \n", search(lpublic,"log_dest") );
                     fprintf(cgiOut, "</label> \n" );
                     fprintf(cgiOut, "</div> \n" );
                     fprintf(cgiOut,"</td></tr>");

  				  fprintf(cgiOut, "<tr><td><hr width=100%% size=1 color=#fff align=center noshade /></td><td>"\
  				  "<hr width=100%% size=1 color=#fff align=center noshade /></td><td>"\
  				  "<hr width=100%% size=1 color=#fff align=center noshade /></td>"\
  				  "</tr>" );
				  
					 fprintf(cgiOut,"<tr height=12><td></td></tr>");


                     fprintf(cgiOut,"<tr ><td>");

                  find_log_node(fpath,NODE_LOG,NODE_ATT,file_name,CH_DEST,&logkey);	 //log的dest属性
                  strcpy(zstring,logkey.key);	

				  
				  
	              find_log_node(fpath,NODE_DES,NODE_ATT,file_name,NODE_VALUE,&logkey);	
                  strcpy(subs,logkey.key);	

			      flag=if_subs(zstring,subs);
									
					 if(flag==2)
                     fprintf(cgiOut, "<input type=\"checkbox\" name=\"showtype\" value=\"1\" onclick=\"changestate()\" checked ></td><td>\n" );
                     else 
					 fprintf(cgiOut, "<input type=\"checkbox\" name=\"showtype\" value=\"1\" onclick=\"changestate()\" ></td><td>\n" );
					 
                     fprintf(cgiOut, "<label class=\"col1\" for=\"Package.DestinationAddress.Single\">%s</label></td><td> \n", search(lpublic,"log_file"));
                     fprintf(cgiOut,"</td></tr>");  

					 

					 fprintf(cgiOut,"<tr height=12><td></td></tr>");
                      //分割线
                      fprintf(cgiOut, "<tr><td><hr width=100%% size=1 color=#fff align=center noshade /></td><td>"\
                      "<hr width=100%% size=1 color=#fff align=center noshade /></td><td>"\
                      "<hr width=100%% size=1 color=#fff align=center noshade /></td>"\
                      "</tr>" );
					 fprintf(cgiOut,"<tr height=12><td></td></tr>");


                     fprintf(cgiOut,"<tr ><td>");
					 
					 
					find_log_node(fpath,NODE_DES,NODE_ATT,L_IP,NODE_VALUE,&logkey);	
                    strcpy(subs,logkey.key);	

			        flag=if_subs(zstring,subs);
					
					 if(flag==2)
                     fprintf(cgiOut, "<input type=\"checkbox\" name=\"showtype\" value=\"2\" onclick=\"changestate()\" checked ></td><td>\n" );
                     else 
					 fprintf(cgiOut, "<input type=\"checkbox\" name=\"showtype\" value=\"2\" onclick=\"changestate()\" ></td><td>\n" );
					 
                     fprintf(cgiOut, "<label class=\"col1\" for=\"Package.DestinationAddress.Single\">%s</label></td><td> \n", search(lpublic,"log_port"));
                     fprintf(cgiOut,"</td>");
					 fprintf(cgiOut,"</tr>");

					 fprintf(cgiOut,"<tr height=7><td></td></tr>");

					 fprintf(cgiOut,"<tr><td></td>");
					 fprintf(cgiOut,"<td><a href=wp_log_add.cgi?UN=%s&Nb=%s><font color=blue size=2>%s</font></a></td></tr>",encry,file_name,search(lpublic,"log_addip"));	
					  
     				  //分割线
     				  fprintf(cgiOut, "<tr><td><hr width=100%% size=1 color=#fff align=center noshade /></td><td>"\
     				  "<hr width=100%% size=1 color=#fff align=center noshade /></td><td>"\
     				  "<hr width=100%% size=1 color=#fff align=center noshade /></td>"\
     				  "</tr>" );
     				 fprintf(cgiOut,"<tr height=12><td></td></tr>");
     
     
     				 
     				 fprintf(cgiOut,"<tr ><td bgcolor=#FFFFFF>");

                    find_log_node(fpath,NODE_DES,NODE_ATT,L_ALL,NODE_VALUE,&logkey);	
                    strcpy(subs,logkey.key);	

			        flag=if_subs(zstring,subs);
					
					 if(flag==2)
     				 fprintf(cgiOut, "<input type=\"checkbox\" name=\"showtype\" value=\"3\" onclick=\"changestate()\" checked></td><td>\n" );
                     else
					 fprintf(cgiOut, "<input type=\"checkbox\" name=\"showtype\" value=\"3\" onclick=\"changestate()\" ></td><td>\n" );

						
					 fprintf(cgiOut, "<label class=\"col1\" for=\"Package.DestinationAddress.Single\">%s</label></td> \n", search(lpublic,"log_user") );
     				 fprintf(cgiOut,"</tr>");	  

					 fprintf(cgiOut,"<tr height=12><td></td></tr>");
					  //分割线
					  fprintf(cgiOut, "<tr><td><hr width=100%% size=1 color=#fff align=center noshade /></td><td>"\
					  "<hr width=100%% size=1 color=#fff align=center noshade /></td><td>"\
					  "<hr width=100%% size=1 color=#fff align=center noshade /></td>"\
					  "</tr>" );
					 fprintf(cgiOut,"<tr height=12><td></td></tr>");
	 
					 if(cgiFormSubmitClicked("submit_mod") != cgiFormSuccess)
								  {
									fprintf(cgiOut,"<td><input type=hidden name=fdb_encry value=%s></td>",encry);									
								  }
								  else if(cgiFormSubmitClicked("submit_mod") == cgiFormSuccess)
									  { 			 
										fprintf(cgiOut,"<td><input type=hidden name=fdb_encry value=%s></td>",log_encry);
										
									  }
					 
				
						fprintf(cgiOut,"</table>");		//7777777777777777777
						fprintf(cgiOut,"</td></tr>");
				fprintf(cgiOut,"<tr>");										
				if(cgiFormSubmitClicked("version") != cgiFormSuccess)
				{
				  fprintf(cgiOut,"<td><input type=hidden name=encry_version value=%s></td>",encry);
				}
				else if(cgiFormSubmitClicked("version") == cgiFormSuccess)
					 {
					   fprintf(cgiOut,"<td><input type=hidden name=encry_version value=%s></td>",log_encry);
					   
					 }		
				fprintf(cgiOut,"</tr>"\
		  "</table>"); //6666666666666666666
		  fprintf(cgiOut,"</td>");
            fprintf(cgiOut,"</tr>");  //内
            fprintf(cgiOut,"<tr height=4 valign=top>"\
              "<td width=120 height=4 align=right valign=top><img src=/images/bottom_07.gif width=1 height=10/></td>"\
              "<td width=827 height=4 valign=top bgcolor=#FFFFFF><img src=/images/bottom_06.gif width=827 height=15/></td>"\
            "</tr>"\
          "</table>");  //444444444444444444444
        fprintf(cgiOut,"</td>"\
        "<td width=15 background=/images/di999.jpg>&nbsp;</td>");
      fprintf(cgiOut,"</tr>");  //次内
    fprintf(cgiOut,"</table></td>"); //33333333333333333333
  fprintf(cgiOut,"</tr>"\
  "<tr>"\
    "<td colspan=3 align=left valign=top background=/images/di777.jpg><img src=/images/di555.jpg width=61 height=62/></td>"\
    "<td align=left valign=top background=/images/di777.jpg>&nbsp;</td>"\
    "<td align=left valign=top background=/images/di777.jpg><img src=/images/di666.jpg width=74 height=62/></td>"\
  "</tr>"\
"</table>"); //1111111111111111111
fprintf(cgiOut,"</div>"\
"</form>"\
"</body>");
fprintf(cgiOut,"</html>");  

free(encry);
return 0;
}

