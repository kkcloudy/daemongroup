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
* wp_cer_del.c
*
*
* CREATOR:
* autelan.software.Network Dep. team
* zhouym@autelan.com
*
* DESCRIPTION:
* system infos for delete file
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

#define TEMP_FILE_CER "/mnt/wtp/cerversion.txt"

int ShowVersionDelPage(struct list *lcontrol,struct list *lpublic,struct list *lwlan);

typedef struct{
	unsigned int 	ih_magic;	
	unsigned int	ih_hcrc;	
	unsigned int	ih_size;	
	unsigned int	ih_time;	
	unsigned int	ih_load;	
	unsigned int	ih_ep;		
	unsigned int	ih_dcrc;	
	unsigned int	ih_os;		
}image_header;


int cgiMain()
{	
	struct list *lcontrol = NULL;
	struct list *lpublic = NULL;
    struct list *lwlan = NULL;     /*解析wlan.txt文件的链表头*/  

	lcontrol = get_chain_head("../htdocs/text/control.txt");
	lpublic= get_chain_head("../htdocs/text/public.txt");
	lwlan=get_chain_head("../htdocs/text/security.txt");

	DcliWInit();
	ccgi_dbus_init();
	ShowVersionDelPage(lcontrol,lpublic,lwlan);
	release(lcontrol);
	release(lpublic); 
	release(lwlan);
	destroy_ccgi_dbus();
	return 0;
}



int ShowVersionDelPage(struct list *lcontrol,struct list *lpublic,struct list *lwlan)
{ 
  char encry[BUF_LEN] = { 0 }; 
  char *str = NULL;
  char *version_name[2]={".cer",".p12"};
  char tempz[512] = { 0 };
  char tz[128] = { 0 };
  char version_encry[BUF_LEN] = { 0 }; 
  char addn[N] = { 0 };     
  FILE *pp = NULL;
  char buff[128] = { 0 }; //读取popen中的块大小
  char file_name[128] = { 0 };  //读取文件
  memset(file_name,0,sizeof(file_name));
  char cmd[PATH_LENG] = { 0 };    
  int i = 0;   
  int cl = 1;
  	
  if(cgiFormSubmitClicked("version") != cgiFormSuccess)
  {
	memset(encry,0,sizeof(encry));
    cgiFormStringNoNewlines("UN", encry, BUF_LEN); 
    str=dcryption(encry);
    if(str==NULL)
    {
      ShowErrorPage(search(lpublic,"ill_user")); 	 /*用户非法*/
	}
	strncpy(addn,str,sizeof(addn)-1);
	memset(version_encry,0,sizeof(version_encry));                   /*清空临时变量*/
  }
  else
  {
  	cgiFormStringNoNewlines("encry_version", version_encry, BUF_LEN); 
    str=dcryption(version_encry);
    if(str==NULL)
    {
      ShowErrorPage(search(lpublic,"ill_user")); 	 /*用户非法*/
	}
	strncpy(addn,str,sizeof(addn)-1);
	memset(version_encry,0,sizeof(version_encry));                   /*清空临时变量*/	
  }
 
  cgiFormStringNoNewlines("encry_version",version_encry,BUF_LEN);
  
  cgiHeaderContentType("text/html");
  fprintf(cgiOut,"<html xmlns=\"http://www.w3.org/1999/xhtml\"><head>");
  fprintf(cgiOut,"<meta http-equiv=Content-Type content=text/html; charset=gb2312>");
 
  fprintf(cgiOut,"<link rel=stylesheet href=/style.css type=text/css>"\
  	 "<style type=text/css>"\
  	  "#div1{ width:62px; height:18px; border:1px solid #666666; background-color:#f9f8f7;}"\
	  "#div2{ width:60px; height:15px; padding-left:5px; padding-top:3px}"\
	  "#link{ text-decoration:none; font-size: 12px}"\
	  ".usrlis {overflow-x:hidden;	overflow:auto; width: 700px; height: 200px; clip: rect( ); padding-top: 0px; padding-right: 0px; padding-bottom: 0px; padding-left: 0px} "\
	  "</style>"\
  "</head>"\
  "<body>");   
    /*
  	  cgiFormStringNoNewlines("Nb",file_name,128);
	  if(strcmp(file_name,"")!=0)
	  	{
	  	int op_ret;
		memset(cmd,0,sizeof(cmd));
		snprintf(cmd,sizeof(cmd)-1,"rm /mnt/images/wtp/%s",file_name);
        op_ret=system(cmd);
        if(op_ret==0)
			ShowAlert(search(lpublic,"oper_succ"));
		else
			ShowAlert(search(lpublic,"oper_fail"));
	  	}	
	  */
  fprintf(cgiOut,"<form method=post encType=multipart/form-data>"\
  "<div align=center>"\
  "<table width=976 border=0 cellpadding=0 cellspacing=0>"\
  "<tr>"\
    "<td width=8 align=left valign=top background=/images/di22.jpg><img src=/images/youce4.jpg width=8 height=30/></td>"\
    "<td width=51 align=left valign=bottom background=/images/di22.jpg><img src=/images/youce33.jpg width=37 height=24/></td>"\
    "<td width=153 align=left valign=bottom id=%s background=/images/di22.jpg>%s</td>",search(lpublic,"title_style"),search(lwlan,"wlan_sec"));
    fprintf(cgiOut,"<td width=690 align=right valign=bottom background=/images/di22.jpg>");
	   
        // 鉴权
        fprintf(cgiOut,"<input type=hidden name=UN value=%s />",encry);
		
	   	
    	  fprintf(cgiOut,"<table width=130 border=0 cellspacing=0 cellpadding=0>"\
          "<tr>");
          if(checkuser_group(addn)==0)  /*管理员*/
          	{
				 fprintf(cgiOut,"<td width=62 align=center><input id=but type=submit name=version style=background-image:url(/images/%s) value=""></td>",search(lpublic,"img_ok"));
		  	}
		  else
		  	{
          	if(cgiFormSubmitClicked("version") != cgiFormSuccess)
            fprintf(cgiOut,"<td width=62 align=left><a href=wp_cer_dload.cgi?UN=%s target=mainFrame><img src=/images/%s border=0 width=62 height=20/></a></td>",encry,search(lpublic,"img_ok"));
		   else                                         
     		fprintf(cgiOut,"<td width=62 align=left><a href=wp_cer_dload.cgi?UN=%s target=mainFrame><img src=/images/%s border=0 width=62 height=20/></a></td>",version_encry,search(lpublic,"img_ok"));	  
		  	}
		  if(cgiFormSubmitClicked("version") != cgiFormSuccess)
            fprintf(cgiOut,"<td width=62 align=left><a href=wp_cer_dload.cgi?UN=%s target=mainFrame><img src=/images/%s border=0 width=62 height=20/></a></td>",encry,search(lpublic,"img_cancel"));
		  else                                         
     		fprintf(cgiOut,"<td width=62 align=left><a href=wp_cer_dload.cgi?UN=%s target=mainFrame><img src=/images/%s border=0 width=62 height=20/></a></td>",version_encry,search(lpublic,"img_cancel"));
		  fprintf(cgiOut,"</tr>"\
          "</table>");
				
		
	fprintf(cgiOut,"</td>"\
    "<td width=74 align=right valign=top background=/images/di22.jpg><img src=/images/youce3.jpg width=31 height=30/></td>"\
  "</tr>"\
  "<tr>");
    fprintf(cgiOut,"<td colspan=5 align=center valign=middle><table width=976 border=0 cellpadding=0 cellspacing=0 bgcolor=#f0eff0>");
      fprintf(cgiOut,"<tr>");
        fprintf(cgiOut,"<td width=12 align=left valign=top background=/images/di888.jpg>&nbsp;</td>"\
        "<td width=948><table width=947 border=0 cellspacing=0 cellpadding=0>");		
            fprintf(cgiOut,"<tr height=4 valign=bottom>"\
              "<td width=120>&nbsp;</td>"\
              "<td width=827 valign=bottom><img src=/images/bottom_05.gif width=827 height=4/></td>"\
            "</tr>");
	fprintf(cgiOut,"<tr>");  //次内
              fprintf(cgiOut,"<td><table width=120 border=0 cellspacing=0 cellpadding=0>"\
                   "<tr height=25>"\
                    "<td id=tdleft>&nbsp;</td>"\
                  "</tr>");
			  fprintf(cgiOut,"<tr height=25>"\
                    "<td id=tdleft>&nbsp;</td>"\
                  "</tr>");
			  fprintf(cgiOut,"<tr height=25>"\
                    "<td id=tdleft>&nbsp;</td>"\
                  "</tr>");
         		

								for(i=0;i<9;i++)
								{
									fprintf(cgiOut,"<tr height=25>"\
									  "<td id=tdleft>&nbsp;</td>"\
									"</tr>");
								}
								
								   fprintf(cgiOut,"</table>"\
							   "</td>"\
							   "<td align=left valign=top style=\"background-color:#ffffff; border-right:1px solid #707070; padding-left:30px; padding-top:10px\">");
								 
				 fprintf(cgiOut,"<table width=460 border=0 cellspacing=0 cellpadding=0>");	 
				 
					   fprintf(cgiOut,"<tr><th width=150 align=left style=font-size:14px>%s</th></tr>",search(lpublic,"cer_list"));
							   fprintf(cgiOut,"<tr>");
							   fprintf(cgiOut,"</tr><tr>");
							   fprintf(cgiOut,"<td colspan=2 id=sec style=\"border-bottom:2px solid #53868b\">&nbsp;</td>"\
							   "</tr>");
							 fprintf(cgiOut,"<tr>");   //内
							  fprintf(cgiOut,"<td colspan=2 style=\"padding-top:20px\">");
								// fprintf(cgiOut,"<table frame=below rules=rows width=460 border=0 cellspacing=0 cellpadding=0>");
								 fprintf(cgiOut,"<div class=usrlis><table frame=below rules=rows width=460 border=0 cellspacing=0 cellpadding=0>");
										  fprintf(cgiOut,"<tr >"\
														 "<th width=150 align=left style=font-size:14px>%s</th>",search(lpublic,"old_cer"));
										  fprintf(cgiOut,"<th width=75 align=left style=font-size:14px>%s</th>",search(lpublic,"version_opt"));	
										  fprintf(cgiOut,"<th width=75 align=left style=font-size:14px></th>");	
										  fprintf(cgiOut,"</tr>");
										 
										  fprintf(cgiOut,"<tr height=10><td></td></tr>");	
										  memset(tempz,0,sizeof(tempz));
										for(i=0;i<2;i++)
										{
										    memset(tz,0,sizeof(tz));
											snprintf(tz,sizeof(tz)-1,"ls  /mnt/wtp |grep -i '%s$'  >> %s\n",version_name[i],TEMP_FILE_CER);
											strncat(tempz,tz,sizeof(tempz)-strlen(tempz)-1);
										}
										system(tempz);
										
										memset(cmd,0,sizeof(cmd));
										snprintf(cmd,sizeof(cmd)-1,"cat %s",TEMP_FILE_CER);
										pp=popen(cmd,"r");  
										if(pp==NULL)
										fprintf(cgiOut,"error open the pipe");
										else
										{
											memset(buff,0,sizeof(buff));
											fgets( buff, sizeof(buff), pp );	//很重要 ，不然与条目不匹配 						 
											do
											{										   
												fprintf(cgiOut,"<tr bgcolor=%s><td>",setclour(cl));



												if(strcmp(buff,"")!=0)
												{
													if(replace_url(buff,"#","%23") != NULL)
														strncpy(file_name,replace_url(buff,"#","%23"),sizeof(file_name)-1);
													else
														strncpy(file_name,buff,sizeof(file_name)-1);	

													fprintf(cgiOut,"%s<br>",buff);									  

													if(checkuser_group(addn)==0)	/*管理员*/
													{
														if(cgiFormSubmitClicked("version") != cgiFormSuccess)
														{															
															fprintf(cgiOut,"<td align=left><a href=wp_version_rm.cgi?UN=%s&ID=%s&Nb=%s target=mainFrame><font color=blue>%s</font></a></td>",encry,"3",file_name,search(lcontrol,"del"));
														}
														else if(cgiFormSubmitClicked("version") == cgiFormSuccess)
														{															
															fprintf(cgiOut,"<td align=left><a href=wp_version_rm.cgi?UN=%s&ID=%s&Nb=%s  target=mainFrame><font color=blue>%s</font></a></td>",version_encry,"3",file_name,search(lcontrol,"del"));
														}
													}
													else
													{		
														if(cgiFormSubmitClicked("version") != cgiFormSuccess)
														{																		 
															fprintf(cgiOut,"<td align=left>%s</td>",search(lcontrol,"del"));
														}
														else if(cgiFormSubmitClicked("version") == cgiFormSuccess)
														{																			 
															fprintf(cgiOut,"<td  align=left>%s</td>",search(lcontrol,"del"));
														}															 
													}

												}
												else
												{  
													fprintf(cgiOut,"</td></tr>");  

												}
												fgets( buff, sizeof(buff), pp ); 	
												cl=!cl;
												}while( !feof(pp) ); 					   
												pclose(pp); 
											}
										memset(cmd,0,sizeof(cmd));
										snprintf(cmd,sizeof(cmd)-1,"sudo rm %s",TEMP_FILE_CER);
										system(cmd);
										fprintf(cgiOut,"</table></div>");		
										fprintf(cgiOut,"</td></tr>");
										fprintf(cgiOut,"<tr>");										
										if(cgiFormSubmitClicked("version") != cgiFormSuccess)
										{
										  fprintf(cgiOut,"<td><input type=hidden name=encry_version value=%s></td>",encry);
										}
										else if(cgiFormSubmitClicked("version") == cgiFormSuccess)
											 {
											   fprintf(cgiOut,"<td><input type=hidden name=encry_version value=%s></td>",version_encry);
											   
											 }		
										fprintf(cgiOut,"</tr>"\
								  "</table>");
		  fprintf(cgiOut,"</td>");
            fprintf(cgiOut,"</tr>");  //内
            fprintf(cgiOut,"<tr height=4 valign=top>"\
              "<td width=120 height=4 align=right valign=top><img src=/images/bottom_07.gif width=1 height=10/></td>"\
              "<td width=827 height=4 valign=top bgcolor=#FFFFFF><img src=/images/bottom_06.gif width=827 height=15/></td>"\
            "</tr>"\
          "</table>"\
        "</td>"\
        "<td width=15 background=/images/di999.jpg>&nbsp;</td>");
      fprintf(cgiOut,"</tr>");  //次内
    fprintf(cgiOut,"</table></td>"\
  "</tr>"\
  "<tr>"\
    "<td colspan=3 align=left valign=top background=/images/di777.jpg><img src=/images/di555.jpg width=61 height=62/></td>"\
    "<td align=left valign=top background=/images/di777.jpg>&nbsp;</td>"\
    "<td align=left valign=top background=/images/di777.jpg><img src=/images/di666.jpg width=74 height=62/></td>"\
  "</tr>"\
"</table>"\
"</div>"\
"</form>"\
"</body>");
fprintf(cgiOut,"</html>");  
return 0;
}


