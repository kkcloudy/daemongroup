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
* wp_qoscounter.c
*
*
* CREATOR:
* autelan.software.Network Dep. team
* zhouym@autelan.com
*
* DESCRIPTION:
* system contrl for qos config
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
#include "ws_dcli_qos.h"
#include "ws_init_dbus.h"

#define PATH_LENG 512  
#define INPUT_AMOUNT 32
#define SHOW_AMOUNT 15  //转换数组的容量
#define AMOUNT 512

int CounterInfo(struct list *lpublic, struct list *lcontrol);   /*n代表加密后的字符串*/

int cgiMain()
{
 struct list *lpublic;
 struct list *lcontrol;

 	lpublic=get_chain_head("../htdocs/text/public.txt");
	lcontrol=get_chain_head("../htdocs/text/control.txt");


 	CounterInfo(lpublic, lcontrol);
	 return 0;
}

int CounterInfo(struct list *lpublic, struct list *lcontrol)
{ 
	//FILE *fp;
	//char lan[3]; 
	ccgi_dbus_init();				/*初始化*/
	
	int i,back,count_flag,color_flag,sub_back;
	char addn[N]="";
	char * mode=(char *)malloc(AMOUNT);
	memset(mode,0,AMOUNT);
	
	char * inNum[SHOW_AMOUNT]={"1","2","3","4","5","6","7","8","9","10","11","12","13","14","15"};
	char * counterIndex = malloc(INPUT_AMOUNT);
	char * inprofile = malloc(INPUT_AMOUNT);
	char * outprofile = malloc(INPUT_AMOUNT);
	memset(counterIndex,0,INPUT_AMOUNT);
	memset(inprofile,0,INPUT_AMOUNT);
	memset(outprofile,0,INPUT_AMOUNT);

	count_flag=0;
	color_flag=0;
	
	struct counter_info * counter[SHOW_AMOUNT];
	for(i=0;i<SHOW_AMOUNT;i++)
	{
  		counter[i] = (struct counter_info *)malloc(sizeof(struct counter_info));	
		
		if(counter[i]==NULL)
	  	{
	  		ShowErrorPage("no space");
			return 0;
	  	}
		memset(counter[i],0,sizeof(struct counter_info));
 	}
  	
	
	for(i=0;i<15;i++)
	{
		back=show_counter(inNum[i],counter[i]);
		if(back==0)
		{
			count_flag++;
		}
	}

	
	 
  
  
  	char *encry=malloc(BUF_LEN);                /*存储从wp_usrmag.cgi带入的加密字符串*/
  	char *str = NULL;        
  	memset(encry,0,BUF_LEN);
  	cgiFormStringNoNewlines("UN", encry, BUF_LEN); 
  	str=dcryption(encry);
  	if(str==NULL)
  	{
    		ShowErrorPage(search(lpublic, "ill_user")); 	       /*用户非法*/
    		return 0;
  	}
	strcpy(addn,str);
  cgiHeaderContentType("text/html");
  show_qos_mode(mode);
  fprintf(cgiOut,"<html xmlns=\"http://www.w3.org/1999/xhtml\"><head>\n");
  fprintf(cgiOut,"<meta http-equiv=Content-Type content=text/html; charset=gb2312>\n");
  fprintf(cgiOut,"<title>%s</title>\n",search(lcontrol,"counter"));
  fprintf(cgiOut,"<link rel=stylesheet href=/style.css type=text/css>\n"\
  "<style type=text/css>\n"\
  	".a3{width:30;border:0; text-align:center}"\
  	"</style>\n"\
  "<script type=\"text/javascript\">\n");  
  
fprintf(cgiOut,"	function checkInNum(strNum)\n"\
	  		"	{\n"\
	  		"		var re = /^(0|[1-9][0-9]*)$/;\n"\
			"		if(strNum!=\"\"&&strNum!=null)\n"\
			"		{\n"\
			"			if(re.test(strNum)==true)\n"\
			"			{\n"\
			"				if(((document.all.inprofile.value)>4294967296)||((document.all.inprofile.value)<1))\n"\
			"				{\n"\
			"					alert(\"%s\");\n"\
			"					document.all.inprofile.value = \"\";\n"\
	  		"					document.all.inprofile.focus();\n"\
			"				}\n"\
			"			}\n"\
			"			else\n"\
			"			{\n"\
			"				alert(\"%s\");\n"\
			"				document.all.inprofile.value = \"\";	\n"\
			"				document.all.inprofile.focus();\n"\
			"			}\n"\
			"		}\n"\
			"	}\n",search(lcontrol,"counter_range"),search(lcontrol,"counter_num"));
fprintf(cgiOut,"	function checkOutNum(strNum)\n"\
	  		"	{\n"\
	  		"		var re = /^(0|[1-9][0-9]*)$/;\n"\
			"		if(strNum!=\"\"&&strNum!=null)\n"\
			"		{\n"\
			"			if(re.test(strNum)==true)\n"\
			"			{\n"\
			"				if(((document.all.inprofile.value)>4294967296)||((document.all.inprofile.value)<1))\n"\
			"				{\n"\
			"					alert(\"%s\");\n"\
			"					document.all.outprofile.value = \"\";\n"\
	  		"						document.all.outprofile.focus();\n"\
			"				}\n"\
			"			}\n"\
			"			else\n"\
			"			{\n"\
			"				alert(\"%s\");\n"\
			"				document.all.outprofile.value = \"\";	\n"\
			"				document.all.outprofile.focus();\n"\
			"			}\n"\
			"		}\n"\
			"	}\n",search(lcontrol,"counter_range"),search(lcontrol,"counter_num"));
  
	  fprintf(cgiOut,"</script>\n");
  fprintf(cgiOut,"</head>\n");
		  if(checkuser_group(addn)==0)/*administrator*/
		  {

				 if(cgiFormSubmitClicked("set_counter")==cgiFormSuccess)
				{
					char url_temp[128];
					cgiFormStringNoNewlines("counter", counterIndex,INPUT_AMOUNT);
					cgiFormStringNoNewlines("inprofile", inprofile,INPUT_AMOUNT);
					cgiFormStringNoNewlines("outprofile", outprofile,INPUT_AMOUNT);
					if(strcmp(counterIndex,"")==0)
					{
						ShowAlert(search(lcontrol,"counter_notnull"));
						sprintf( url_temp, "wp_qoscounter.cgi?UN=%s", encry);
						fprintf( cgiOut, "<script type='text/javascript'>\n" );
						fprintf( cgiOut, "window.location.href='%s';\n", url_temp );
						fprintf( cgiOut, "</script>\n" );
					}
					else if(strcmp(inprofile,"")==0)
					{
						ShowAlert(search(lcontrol,"inprofile_notnull"));
						sprintf( url_temp, "wp_qoscounter.cgi?UN=%s", encry);
						fprintf( cgiOut, "<script type='text/javascript'>\n" );
						fprintf( cgiOut, "window.location.href='%s';\n", url_temp );
						fprintf( cgiOut, "</script>\n" );
					}
					else if(strcmp(outprofile,"")==0)
					{
						ShowAlert(search(lcontrol,"outprofile_notnull"));
						sprintf( url_temp, "wp_qoscounter.cgi?UN=%s", encry);
						fprintf( cgiOut, "<script type='text/javascript'>\n" );
						fprintf( cgiOut, "window.location.href='%s';\n", url_temp );
						fprintf( cgiOut, "</script>\n" );
					}
					else
					{
						sub_back = set_counter_attr(counterIndex,inprofile,outprofile);
						switch(sub_back)
						{
							case 0:
							{
								ShowAlert(search(lcontrol,"counterconfigsucc"));
								sprintf( url_temp, "wp_qoscounter.cgi?UN=%s",encry);
								fprintf( cgiOut, "<script type='text/javascript'>\n" );
								fprintf( cgiOut, "window.location.href='%s';\n", url_temp );
								fprintf( cgiOut, "</script>\n" );
								break;
							}
							case -2:
								ShowAlert(search(lcontrol,"counterconfigfail"));
								break;
							default:
								ShowAlert(search(lcontrol,"counterconfigfail"));
								break;
						}
					}
				}
		  	}
		  	else
		  	{
		  		 if(cgiFormSubmitClicked("set_counter")==cgiFormSuccess)
				{
					char url_temp[128];
					sprintf( url_temp, "wp_qoscounter.cgi?UN=%s",encry);
					fprintf( cgiOut, "<script type='text/javascript'>\n" );
					fprintf( cgiOut, "window.location.href='%s';\n", url_temp );
					fprintf( cgiOut, "</script>\n" );			
				}
		  	}


  

  fprintf(cgiOut,"<body>\n"\
  "<form id=Form1>\n"\
  "<div align=center>\n"\
  "<table width=976 border=0 cellpadding=0 cellspacing=0>\n"\
  "<tr>\n"\
    "<td width=8 align=left valign=top background=/images/di22.jpg><img src=/images/youce4.jpg width=8 height=30/></td>\n"\
    "<td width=51 align=left valign=bottom background=/images/di22.jpg><img src=/images/youce33.jpg width=37 height=24/></td>\n"\
    "<td width=153 align=left valign=bottom background=/images/di22.jpg><font id=titleen>QOS</font><font id=%s> %s</font></td>",search(lpublic,"title_style"),search(lpublic,"management"));
    fprintf(cgiOut,"<td width=690 align=right valign=bottom background=/images/di22.jpg>\n");
	   // if((fp=fopen("../htdocs/text/public.txt","r"))==NULL)		 /*以只读方式打开资源文件*/
	   //   ShowAlert(search(lpublic,"error_open"));
	 //   fseek(fp,4,0); 						/*将文件指针移到离文件首4个字节处，即lan=之后*/
	  //  fgets(lan,3,fp);
	//	fclose(fp);
		
	    //if(strcmp(lan,"ch")==0)
    //	{	
    	  fprintf(cgiOut,"<table width=155 border=0 cellspacing=0 cellpadding=0>\n"\
          "<tr>\n"\
          "<td width=62 align=center><input id=but type=submit name=set_counter style=background-image:url(/images/%s) value=""></td>\n",search(lpublic,"img_ok"));		  
          fprintf(cgiOut,"<td width=62 align=center><a href=wp_policer.cgi?UN=%s target=mainFrame><img src=/images/%s border=0 width=62 height=20/></a></td>\n",encry,search(lpublic,"img_cancel"));
		  fprintf(cgiOut,"</tr>\n"\
          "</table>\n");
		/*}		
		else			
		{	
		  fprintf(cgiOut,"<table width=155 border=0 cellspacing=0 cellpadding=0>\n"\
		  "<tr>\n"\*/
		  //"<td width=62 align=center><input id=but type=submit name=/*此处添加submit名字*/ style=background-image:url(/images/ok-en.jpg) value=""></td>\n");		  
          //fprintf(cgiOut,"<td width=62 align=center><a href=/*此处添加返回页面*/?UN=%s target=mainFrame><img src=/images/cancel-en.jpg border=0 width=62 height=20/></a></td>\n",encry);
		  /*fprintf(cgiOut,"</tr>\n"\
		  "</table>\n");
		}*/		
      fprintf(cgiOut,"</td>\n"\
    "<td width=74 align=right valign=top background=/images/di22.jpg><img src=/images/youce3.jpg width=31 height=30/></td>\n"\
  "</tr>\n"\
  "<tr>\n"\
    "<td colspan=5 align=center valign=middle><table width=976 border=0 cellpadding=0 cellspacing=0 bgcolor=#f0eff0>\n"\
      "<tr>\n"\
        "<td width=12 align=left valign=top background=/images/di888.jpg>&nbsp;</td>\n"\
        "<td width=948><table width=947 border=0 cellspacing=0 cellpadding=0>\n"\
            "<tr height=4 valign=bottom>\n"\
              "<td width=120>&nbsp;</td>\n"\
              "<td width=827 valign=bottom><img src=/images/bottom_05.gif width=827 height=4/></td>\n"\
            "</tr>\n"\
            "<tr>\n"\
              "<td><table width=120 border=0 cellspacing=0 cellpadding=0>\n"\
                   "<tr height=25>\n"\
                    "<td id=tdleft>&nbsp;</td>\n"\
                  "</tr>\n");
					
					//显示左侧功能表
					fprintf(cgiOut,"<tr height=25>"\
		            			"<td align=left id=tdleft><a href=wp_policer.cgi?UN=%s target=mainFrame class=top><font id=%s>%s</font></a></td>",encry,search(lpublic,"menu_san"),search(lcontrol,"policer"));						   
		            		fprintf(cgiOut,"</tr>");
					
					if(checkuser_group(addn)==0)/*administrator*/
	         			{
		            			fprintf(cgiOut,"<tr height=25>"\
		            			"<td align=left id=tdleft><a href=wp_addpolicer.cgi?UN=%s target=mainFrame class=top><font id=%s>%s</font></a></td>",encry,search(lpublic,"menu_san"),search(lcontrol,"add_policer"));					   
		            			fprintf(cgiOut,"</tr>");
	         			}
					
					fprintf(cgiOut,"<tr height=26>\n"\
					  "<td align=left id=tdleft background=/images/bottom_bg.gif style=\"border-right:0\"><font id=%s>%s</font></td>\n",search(lpublic,"menu_san"),search(lcontrol,"counter"));
					fprintf(cgiOut,"</tr>\n");
					//调整页面纵向长度
					for(i=0;i<count_flag+5;i++) 
	              {
  				    fprintf(cgiOut,"<tr height=25>\n"\
                      "<td id=tdleft>&nbsp;</td>\n"\
                    "</tr>\n");
	              }
                fprintf(cgiOut,"</table>\n"\
				"</td>\n"\
				"<td align=left valign=top style=\"background-color:#ffffff; border-right:1px solid #707070; padding-left:30px; padding-top:10px\">\n"\
				"<table  border=0 cellspacing=0 cellpadding=0>\n"\
				"<tr>\n"\
				"<td align='left'>\n");
fprintf(cgiOut,"<input type='hidden' name=UN value=%s>",encry);//鉴权
	 //MY CODE********************************************************************************************************	
		fprintf(cgiOut,"<table align='left'>\n");
		 	fprintf(cgiOut,"<tr height='25'>\n"\
							"<td style=\"font-size:14px\"><font color='red'><b>%s</b></font></td>",search(lcontrol,mode));
			fprintf(cgiOut,"</tr>\n");
	 if(checkuser_group(addn)==0)/*administrator*/
	 {
		fprintf(cgiOut,"	<tr>\n"\
					"		<td>\n"\
					"			<table>\n"\
					"				<tr>\n"\
					"					<td>Index:</td>\n"\
					"					<td>\n"\
					"						<select name=counter>\n"\
					"							<option value=''>--%s--</option>\n",search(lcontrol,"select"));
	 for(i=1;i<=15;i++)
	 {
	 	fprintf(cgiOut,"							<option value=%d>%d</option>\n",i,i);
	 }
		fprintf(cgiOut,"						</select>\n"\
					"					</td>\n"\
					"					<td><font color='red'>(1-15)</font></td>\n"\
					"				</tr>\n"\
					"				<tr>\n"\
					"					<td>In-Profile:</td>\n"\
					"					<td><input type='text' id='inprofile' name='inprofile' onBlur=\"checkInNum(this.value);\"></td>\n"\
					"					<td><font color='red'>(1-4G %s)</font></td>\n",search(lcontrol,"byte"));
		fprintf(cgiOut,"				</tr>\n"\
					"				<tr>\n"\
					"					<td>Out-Profile:</td>\n"\
					"					<td><input type='text' id='outprofile' name='outprofile' onBlur=\"checkOutNum(this.value);\"></td>\n"\
					"					<td><font color='red'>(1-4G %s)</font></td>\n"\
					"				</tr>\n"\
					"			</table>\n"\
					"		</td>\n"\
					"	</tr>\n",search(lcontrol,"byte"));
	 }
	if(count_flag==0)
	{
		fprintf(cgiOut,"	<tr>\n"\
					"		<td id=sec1 style=\"border-bottom:2px solid #53868b;font-size:14px\" colspan=4>%s</td>\n"\
					"	</tr>",search(lcontrol,"nocount"));
	}
	else
	{
		fprintf(cgiOut,"	<tr>\n"\
					"		<td id=sec1 style=\"border-bottom:2px solid #53868b;font-size:14px\" colspan=4>%s</td>\n"\
					"	</tr>\n",search(lcontrol,"countmsg"));
		/*
		fprintf(cgiOut,"	<tr>\n"\
					"		<td>\n"\
					"			<table>\n"\
					"				<tr>\n"\
					"					<td>\n"\
					"					</td>\n"\
					"				</tr>\n"\
					"			</table>\n"\
					"		</td>\n"\
					"	</tr>\n");
		*/
		fprintf(cgiOut,"	<tr>\n"\
					"		<td align=left valign=top style=padding-top:18px colspan=4>\n"\
					"			<table width=503 border=1 frame=below rules=rows bordercolor=#cccccc cellspacing=0 cellpadding=0>\n"\
					"				<tr height=30 bgcolor=#eaeff9 id=td1 align=left>\n");
		fprintf(cgiOut,"					<th width=80 style=font-size:12px><font id=%s>INDEX</font></th>\n", search(lpublic,"menu_thead"));
		fprintf(cgiOut,"					<th width=80 style=font-size:12px><font id=%s>INPROFILE</font></th>\n", search(lpublic,"menu_thead"));
		fprintf(cgiOut,"					<th width=80 style=font-size:12px><font id=%s>OUTPROFILE</font></th>\n", search(lpublic,"menu_thead"));
		fprintf(cgiOut,"				</tr>\n");
		for(i=0;i<15;i++)
		{
			if(counter[i]->index!=0)
			{
				if(color_flag%2==0)
				{
		fprintf(cgiOut,"				<tr height=25 bgcolor=#f9fafe align=left>\n"\
					"					<td style=font-size:12px align=left>%d</td>\n",counter[i]->index);
		fprintf(cgiOut,"					<td style=font-size:12px align=left>%lu</td>\n",counter[i]->inprofile);
		fprintf(cgiOut,"					<td style=font-size:12px align=left>%lu</td>\n",counter[i]->outprofile);
		fprintf(cgiOut,"				</tr>\n");
				}
				else
				{
		fprintf(cgiOut,"				<tr height=25 bgcolor=#ffffff align=left>\n"\
					"					<td style=font-size:12px align=left>%d</td>\n",counter[i]->index);
		fprintf(cgiOut,"					<td style=font-size:12px align=left>%lu</td>\n",counter[i]->inprofile);
		fprintf(cgiOut,"					<td style=font-size:12px align=left>%lu</td>\n",counter[i]->outprofile);
		fprintf(cgiOut,"				</tr>\n");	
				}
				color_flag++;
			}
		}
		fprintf(cgiOut,"			</table>\n"\
					"		</td>\n"\
					"	</tr>\n");
	}
		fprintf(cgiOut,"</table>\n");
//*********************************************************END
fprintf(cgiOut,"</td>\n"\
		  	  "</tr>\n"\
      "</table>\n"\
      "	</td>\n"\
            "</tr>\n"\
            "<tr height=4 valign=top>\n"\
              "<td width=120 height=4 align=right valign=top><img src=/images/bottom_07.gif width=1 height=10/></td>\n"\
              "<td width=827 height=4 valign=top bgcolor=#FFFFFF><img src=/images/bottom_06.gif width=827 height=15/></td>\n"\
            "</tr>\n"\
          "</table>\n"\
        "</td>\n"\
        "<td width=15 background=/images/di999.jpg>&nbsp;</td>\n"\
      "</tr>\n"\
    "</table></td>\n"\
  "</tr>\n"\
  "<tr>\n"\
    "<td colspan=3 align=left valign=top background=/images/di777.jpg><img src=/images/di555.jpg width=61 height=62/></td>\n"\
    "<td align=left valign=top background=/images/di777.jpg>&nbsp;</td>\n"\
    "<td align=left valign=top background=/images/di777.jpg><img src=/images/di666.jpg width=74 height=62/></td>\n"\
  "</tr>\n"\
"</table>\n"\
"</div>\n"\
"</form>\n"\
"</body>\n"\
"</html>\n");
for(i=0;i<15;i++)
{
	free(counter[i]);
}
release(lpublic);
release(lcontrol);
free(encry);
free(counterIndex);
free(inprofile);
free(outprofile);
free(mode);
return 0;
}

