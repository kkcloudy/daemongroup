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
* wp_version_del.c
*
*
* CREATOR:
* autelan.software.Network Dep. team
* zhouym@autelan.com
*
* DESCRIPTION:
* system function for version file deal
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


int ShowVersionDelPage(struct list *lcontrol,struct list *lpublic);

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
	
	
	struct list *lcontrol;
	struct list *lpublic;
	lcontrol = get_chain_head("../htdocs/text/control.txt");
	lpublic= get_chain_head("../htdocs/text/public.txt");
	
	ShowVersionDelPage(lcontrol,lpublic);
	release(lcontrol);
	release(lpublic); 
	return 0;
}



int ShowVersionDelPage(struct list *lcontrol,struct list *lpublic)
{ 
  char *encry=(char *)malloc(BUF_LEN);
 
  char *str;
 
 
  char version_encry[BUF_LEN]; 
  char addn[N]; 

  
  
  FILE *pp;
  char buff[128]; 
  memset(buff,0,128);

  char file_name[128]; 
  memset(file_name,0,128);
  
  char *cmd = (char *)malloc(PATH_LENG);   
  memset(cmd,0,PATH_LENG); 
  sprintf(cmd,"sudo sor.sh imgls xx %d",SHORT_SORT);
  
 
     int i = 0;   
     int cl=1;
  	
  if(cgiFormSubmitClicked("version") != cgiFormSuccess)
  {
	memset(encry,0,BUF_LEN);
    cgiFormStringNoNewlines("UN", encry, BUF_LEN); 
    str=dcryption(encry);
    if(str==NULL)
    {
      ShowErrorPage(search(lpublic,"ill_user")); 	
	}
	strcpy(addn,str);
	memset(version_encry,0,BUF_LEN);                 
  }
  else
  	{
  	cgiFormStringNoNewlines("encry_version", version_encry, BUF_LEN); 
    str=dcryption(version_encry);
    if(str==NULL)
    {
      ShowErrorPage(search(lpublic,"ill_user")); 	
	}
	strcpy(addn,str);
	memset(version_encry,0,BUF_LEN); 
	
  	}
 
  cgiFormStringNoNewlines("encry_version",version_encry,BUF_LEN);
  
  cgiHeaderContentType("text/html");
  fprintf(cgiOut,"<html xmlns=\"http://www.w3.org/1999/xhtml\"><head>");
  fprintf(cgiOut,"<meta http-equiv=Content-Type content=text/html; charset=gb2312>");
 
  fprintf(cgiOut,"<link rel=stylesheet href=/style.css type=text/css>"\
  	"<style type=text/css>"\
    "#div1{ width:42px; height:18px; border:1px solid #666666; background-color:#f9f8f7;}"\
    "#div2{ width:40px; height:15px; padding-left:5px; padding-top:3px}"\
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
  "<body>");   

  	 
  fprintf(cgiOut,"<form method=post encType=multipart/form-data>"\
  "<div align=center>"\
  "<table width=976 border=0 cellpadding=0 cellspacing=0>"\
  "<tr>"\
    "<td width=8 align=left valign=top background=/images/di22.jpg><img src=/images/youce4.jpg width=8 height=30/></td>"\
    "<td width=51 align=left valign=bottom background=/images/di22.jpg><img src=/images/youce33.jpg width=37 height=24/></td>"\
    "<td width=153 align=left valign=bottom id=%s background=/images/di22.jpg>%s</td>",search(lpublic,"title_style"),search(lpublic,"version_up"));
    fprintf(cgiOut,"<td width=690 align=right valign=bottom background=/images/di22.jpg>");
    fprintf(cgiOut,"<input type=hidden name=UN value=%s />",encry);
	fprintf(cgiOut,"<table width=130 border=0 cellspacing=0 cellpadding=0>"\
	"<tr>");
	if(checkuser_group(addn)==0) 
	{
		fprintf(cgiOut,"<td width=62 align=center><input id=but type=submit name=version style=background-image:url(/images/%s) value=""></td>",search(lpublic,"img_ok"));
	}
	else
	{
		if(cgiFormSubmitClicked("version") != cgiFormSuccess)
			fprintf(cgiOut,"<td width=62 align=left><a href=wp_version_upgrade.cgi?UN=%s target=mainFrame><img src=/images/%s border=0 width=62 height=20/></a></td>",encry,search(lpublic,"img_ok"));
		else                                         
			fprintf(cgiOut,"<td width=62 align=left><a href=wp_version_upgrade.cgi?UN=%s target=mainFrame><img src=/images/%s border=0 width=62 height=20/></a></td>",version_encry,search(lpublic,"img_ok"));	  
	}
	if(cgiFormSubmitClicked("version") != cgiFormSuccess)
		fprintf(cgiOut,"<td width=62 align=left><a href=wp_version_upgrade.cgi?UN=%s target=mainFrame><img src=/images/%s border=0 width=62 height=20/></a></td>",encry,search(lpublic,"img_cancel"));
	else                                         
		fprintf(cgiOut,"<td width=62 align=left><a href=wp_version_upgrade.cgi?UN=%s target=mainFrame><img src=/images/%s border=0 width=62 height=20/></a></td>",version_encry,search(lpublic,"img_cancel"));
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
	fprintf(cgiOut,"<tr>"); 
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
								for(i=0;i<8;i++)
								{
									fprintf(cgiOut,"<tr height=25>"\
									  "<td id=tdleft>&nbsp;</td>"\
									"</tr>");
								}
								
								   fprintf(cgiOut,"</table>"\
							   "</td>"\
							   "<td align=left valign=top style=\"background-color:#ffffff; border-right:1px solid #707070; padding-left:30px; padding-top:10px\">");
								 
				 fprintf(cgiOut,"<table width=460 border=0 cellspacing=0 cellpadding=0>");	 
				 
					   fprintf(cgiOut,"<tr><th width=150 align=left style=font-size:14px>%s</th></tr>",search(lpublic,"version_list"));
							   fprintf(cgiOut,"<tr>");
							   fprintf(cgiOut,"</tr><tr>");
							   fprintf(cgiOut,"<td colspan=2 id=sec style=\"border-bottom:2px solid #53868b\">&nbsp;</td>"\
							   "</tr>");
							 fprintf(cgiOut,"<tr>");   
						     fprintf(cgiOut,"<td colspan=2 style=\"padding-top:20px\">");
							 fprintf(cgiOut,"<table width=460 border=0 bordercolor=#cccccc cellspacing=0 cellpadding=0>");
						     fprintf(cgiOut,"<tr >"\
									 "<th width=150 align=left style=font-size:14px>%s</th>",search(lpublic,"old_version"));
							 fprintf(cgiOut,"<th width=75 align=left style=font-size:14px>%s</th>",search(lpublic,"version_opt"));	
						     fprintf(cgiOut,"<th width=75 align=left style=font-size:14px></th>");	
						     fprintf(cgiOut,"</tr>");										 
						     fprintf(cgiOut,"<tr height=10><td colspan=3></td></tr>");	

							int error = 0;
							pp=popen(cmd,"r");            
							if(pp==NULL)
								fprintf(cgiOut,"error open the pipe");
							else
							{
								fgets( buff, sizeof(buff), pp );				 
								do
								{										   
									    fprintf(cgiOut,"<tr bgcolor=%s>",setclour(cl));		
                                        if(strcmp(buff,"")!=0)
                                        {
											strcpy(file_name,buff);	
											fprintf(cgiOut,"<td>\n");
											fprintf(cgiOut,"%s",file_name);
											fprintf(cgiOut,"</td>\n");	 										  

											if(checkuser_group(addn)==0)	
											{
												if(cgiFormSubmitClicked("version") != cgiFormSuccess)
												{			
													fprintf(cgiOut,"<td align=right><a href=wp_md5_check.cgi?UN=%s&Nb=%s target=mainFrame><font color=black>%s</font></a></td>",encry,file_name,search(lpublic,"md5_check"));
													fprintf(cgiOut,"<td align=center><a href=wp_version_rm.cgi?UN=%s&ID=%s&Nb=%s target=mainFrame><font color=black>%s</font></a></td>",encry,"1",file_name,search(lcontrol,"del"));
													fprintf(cgiOut,"<td align=left><a href=wp_version_enable.cgi?UN=%s&Nb=%s target=mainFrame><font color=black>%s</font></a></td>",encry,file_name,search(lpublic,"version_enable"));
												}
												else if(cgiFormSubmitClicked("version") == cgiFormSuccess)
												{		
													fprintf(cgiOut,"<td align=right><a href=wp_md5_check.cgi?UN=%s&Nb=%s target=mainFrame><font color=black>%s</font></a></td>",version_encry,file_name,search(lpublic,"md5_check"));
													fprintf(cgiOut,"<td align=center><a href=wp_version_rm.cgi?UN=%s&ID=%s&Nb=%s target=mainFrame><font color=black>%s</font></a></td>",version_encry,"1",file_name,search(lcontrol,"del"));
													fprintf(cgiOut,"<td align=left><a href=wp_version_enable.cgi?UN=%s&Nb=%s target=mainFrame><font color=black>%s</font></a></td>",version_encry,file_name,search(lpublic,"version_enable"));
												}
											}
											else
											{		
												if(cgiFormSubmitClicked("version") != cgiFormSuccess)
												{																		 
													fprintf(cgiOut,"<td align=center>%s</td>",search(lcontrol,"del"));
													fprintf(cgiOut,"<td align=left>%s</td>",search(lpublic,"version_enable"));
												}
												else if(cgiFormSubmitClicked("version") == cgiFormSuccess)
												{																			 
													fprintf(cgiOut,"<td  align=center>%s</td>",search(lcontrol,"del"));
													fprintf(cgiOut,"<td  align=left>%s</td>",search(lpublic,"version_enable"));
												}															 
											}		
                                        }
										cl=!cl;	
									fprintf(cgiOut,"</tr>\n");
									fgets( buff, sizeof(buff), pp ); 									   
									}while( !feof(pp) ); 					   
									
									
									int ret = 0;
									ret=pclose(pp);
									switch (WEXITSTATUS(ret)) {
									case 0:
										break;
									case 1:
										ShowAlert("Sysetm internal error (1)");
										error = 1;
										break;
									case 2:
										ShowAlert("Sysetm internal error (2)");
										error = 1;
										break;
									case 3:
										ShowAlert("Storage media is busy");
										error = 1;
										break;
									case 4:
										ShowAlert("Storage operation time out");
										error = 1;
										break;
									case 5:
										ShowAlert("No left space on storage media");
										error = 1;
										break;
									default:
										ShowAlert("Sysetm internal error (3)");
										error = 1;
										break;
									}
								}
							if(1 == error)
								{
									fprintf( cgiOut, "<script type='text/javascript'>\n" );
							   		fprintf( cgiOut, "window.location.href='wp_version_upgrade.cgi?UN=%s';\n", encry);
									fprintf( cgiOut, "</script>\n" );
								}
							fprintf(cgiOut,"</table>\n");
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
            fprintf(cgiOut,"</tr>"); 
            fprintf(cgiOut,"<tr height=4 valign=top>"\
              "<td width=120 height=4 align=right valign=top><img src=/images/bottom_07.gif width=1 height=10/></td>"\
              "<td width=827 height=4 valign=top bgcolor=#FFFFFF><img src=/images/bottom_06.gif width=827 height=15/></td>"\
            "</tr>"\
          "</table>"\
        "</td>"\
        "<td width=15 background=/images/di999.jpg>&nbsp;</td>");
      fprintf(cgiOut,"</tr>");  
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

free(encry);
return 0;
}
