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
* wp_dhcp_optmod.c
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
#include <stdio.h>
#include "cgic.h"
#include <string.h>
#include <stdlib.h>
#include "ws_usrinfo.h"
#include "ws_err.h"
#include "ws_ec.h"
#include <fcntl.h>
#include <sys/wait.h>

#include "ws_dhcp_conf.h"



int ShowOptModPage(char *m,char *id,struct list *lpublic,struct list *lcontrol);    
int ShowOptClearPage(char *m,char *id,struct list *lpublic,struct list *lcontrol);

int cgiMain()
{  
  char *encry=(char *)malloc(BUF_LEN);  
  char *str;   
  struct list *lpublic;   /*解析public.txt文件的链表头*/
  struct list *lcontrol;     /*解析wlan.txt文件的链表头*/  
  lpublic=get_chain_head("../htdocs/text/public.txt");
  lcontrol=get_chain_head("../htdocs/text/control.txt");

  char hsid[30];
  memset(hsid,0,30);

  char hstype[10];
  memset(hstype,0,10);


  cgiFormStringNoNewlines("ID",hsid,30);
  cgiFormStringNoNewlines("TYPE",hstype,10);


  if(cgiFormStringNoNewlines("UN", encry, BUF_LEN)!=cgiFormNotFound )  /*首次进入该页*/
  {
    str=dcryption(encry);
    if(str==NULL)
      ShowErrorPage(search(lpublic,"ill_user"));		 /*用户非法*/
    else 
    {
	  if(strcmp(hstype,"2")==0)
	    ShowOptClearPage(encry,hsid,lpublic,lcontrol);
	  else
	  	ShowOptModPage(encry,hsid,lpublic,lcontrol);
    }

  }
  else                    
  {      
   cgiFormStringNoNewlines("encry_newvrrp",encry,BUF_LEN);
	str=dcryption(encry);	
    if(str==NULL)
      ShowErrorPage(search(lpublic,"ill_user"));		 /*用户非法*/
    else 
    {
      
	  if(strcmp(hstype,"2")==0)
	    ShowOptClearPage(encry,hsid,lpublic,lcontrol);
	  else
	  	ShowOptModPage(encry,hsid,lpublic,lcontrol);
    }

  } 
  
  
  free(encry);
  release(lpublic);  
  release(lcontrol);
  return 0;
}

int ShowOptModPage(char *m,char *id,struct list *lpublic,struct list *lcontrol)
{  
  int i;  

  char hexip[30];
  memset(hexip,0,30);

  char optname[20];
  memset(optname,0,20);

  char strip[30];
  memset(strip,0,30);

  char uip1[4],uip2[4],uip3[4],uip4[4];
  memset(uip1,0,4);
  memset(uip2,0,4);
  memset(uip3,0,4);
  memset(uip4,0,4);

  char ip1[4],ip2[4],ip3[4],ip4[4];

  char key[30];
  memset(key,0,30);

  char idenr[30];
  memset(idenr,0,30);
  
  char *tmp;

  
  cgiHeaderContentType("text/html");
  fprintf(cgiOut,"<html xmlns=\"http://www.w3.org/1999/xhtml\"><head>");
  fprintf(cgiOut,"<meta http-equiv=Content-Type content=text/html; charset=gb2312>");
  fprintf(cgiOut,"<title></title>");
  fprintf(cgiOut,"<link rel=stylesheet href=/style.css type=text/css>"\
  	"<style type=text/css>"\
  	".a3{width:30;border:0; text-align:center}"\
  	"</style>"\
  	"<script language=javascript src=/ip.js>"\
  	"</script>"\
  "</head>"\
  "<body>");
  
 if(cgiFormSubmitClicked("dhcpadd") == cgiFormSuccess)
 	{
	 
	  memset(optname,0,20);
	  memset(strip,0,30);
	  memset(ip1,0,4);
	  memset(ip2,0,4);
	  memset(ip3,0,4);
	  memset(ip4,0,4);

	  cgiFormStringNoNewlines("iden",optname,20);
	  cgiFormStringNoNewlines("ip1",ip1,4);
	  cgiFormStringNoNewlines("ip2",ip2,4);
	  cgiFormStringNoNewlines("ip3",ip3,4);
	  cgiFormStringNoNewlines("ip4",ip4,4);

	  int tip1,tip2,tip3,tip4;
	  tip1=strtoul(ip1,0,10);
	  tip2=strtoul(ip2,0,10);
	  tip3=strtoul(ip3,0,10);
	  tip4=strtoul(ip4,0,10);


	  if((strcmp(optname,"")!=0) &&(strcmp(ip1,"")!=0) &&(strcmp(ip2,"")!=0) &&(strcmp(ip3,"")!=0) &&(strcmp(ip4,"")!=0))
	  {
	    sprintf(strip,"%d:%d:%d:%d",tip1,tip2,tip3,tip4);
		sprintf(hexip,"%X:%X:%X:%X",tip1,tip2,tip3,tip4);
	    add_dhcp_node_attr(DHCP_XML, DHCP_OPT_NAME, DHCP_NODE_ATTR, id);
		mod_dhcp_node(DHCP_XML, DHCP_OPT_NAME, DHCP_NODE_ATTR, id, DHCP_OPT_USRN, id);
		mod_dhcp_node(DHCP_XML, DHCP_OPT_NAME, DHCP_NODE_ATTR, id, DHCP_OPT_IDEN, optname);
		mod_dhcp_node(DHCP_XML, DHCP_OPT_NAME, DHCP_NODE_ATTR, id, DHCP_OPT_VENDER, strip);
		mod_dhcp_node(DHCP_XML, DHCP_OPT_NAME, DHCP_NODE_ATTR, id, DHCP_OPT_HEX, hexip);
		///////////write to conf
		int ret=-1;

		int bflag=-1,cflag=-1,oflag=-1,gflag=-1;

		struct bindipmac b_head;
		int bnum;

		struct optionsix o_head;
		int onum;

		struct confinfo c_head;
		int cnum;

		struct gbparam g_head;
		int gnum;

		bflag=read_bindinfo_xml(&b_head, &bnum);
		oflag=read_option_info(&o_head, &onum);
		cflag=read_confinfo_xml(&c_head, &cnum);
		gflag=read_gbparam_xml(&g_head,&gnum);
		
		if((bflag==0)&&(oflag==0)&&(cflag==0)&&(gflag==0))
		{
		ret=write_dhcp_conf(DHCP_CONF_PATH, &o_head, &c_head, &b_head,&g_head);
		}
		if((bflag==0 )&& (bnum > 0))
		Free_bindipmac_info(&b_head);

		if((cflag==0 )&& (cnum > 0))
		Free_confinfo_info(&c_head);

		if((oflag==0 )&& (onum > 0))
		Free_optionsix_info(&o_head);
		
		if((gflag==0 )&& (gnum > 0))
		Free_gbparam_info(&g_head);
		
		if(ret==0)
		   ShowAlert(search(lpublic,"oper_succ"));
		else
			ShowAlert(search(lpublic,"oper_fail"));
			
	  }
	  else
	  	ShowAlert(search(lpublic,"param_null"));
	  
  }
  fprintf(cgiOut,"<form method=post >"\
  "<div align=center>"\
  "<table width=976 border=0 cellpadding=0 cellspacing=0>"\
  "<tr>"\
    "<td width=8 align=left valign=top background=/images/di22.jpg><img src=/images/youce4.jpg width=8 height=30/></td>"\
    "<td width=51 align=left valign=bottom background=/images/di22.jpg><img src=/images/youce33.jpg width=37 height=24/></td>"\
    "<td width=153 align=left valign=bottom id=titleen background=/images/di22.jpg>DHCP</td>"\
    "<td width=690 align=right valign=bottom background=/images/di22.jpg>");
	  
    	  fprintf(cgiOut,"<table width=155 border=0 cellspacing=0 cellpadding=0>"\
          "<tr>"\
          "<td width=62 align=center><input  id=but type=submit name=dhcpadd style=background-image:url(/images/%s) value=""></td>",search(lpublic,"img_ok"));			  
          fprintf(cgiOut,"<td width=62 align=center><a href=wp_dhcp_opt.cgi?UN=%s target=mainFrame><img src=/images/%s border=0 width=62 height=20/></a></td>",m,search(lpublic,"img_cancel"));
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
                      "<td align=left id=tdleft background=/images/bottom_bg.gif style=\"border-right:0\"><font id=%s> %s</font></td>",search(lpublic,"menu_san"),"option");   /*突出显示*/
					fprintf(cgiOut,"</tr>");
						
                  for(i=0;i<7;i++)
	              {
  				    fprintf(cgiOut,"<tr height=25>"\
                      "<td id=tdleft>&nbsp;</td>"\
                     "</tr>");
	              }
			fprintf(cgiOut,"</table>"\
			  "</td>"\
			 "<td align=left valign=top style=\"background-color:#ffffff; border-right:1px solid #707070; padding-left:30px; padding-top:10px\">");
				fprintf(cgiOut,"<table width=700 border=0 cellspacing=0 cellpadding=0>");					
				fprintf(cgiOut,"<tr>");
				
                ///////////////////////////add new content  identifier 
                get_dhcp_node_attr(DHCP_XML, DHCP_OPT_NAME, DHCP_NODE_ATTR, id, DHCP_OPT_IDEN, &key);
				get_dhcp_node_attr(DHCP_XML, DHCP_OPT_NAME, DHCP_NODE_ATTR, id, DHCP_OPT_VENDER, &idenr);

				memset(ip1,0,4);
			    memset(ip2,0,4);
			    memset(ip3,0,4);
			    memset(ip4,0,4);

			    tmp = strtok(idenr,":");
			    i=0;
			    while(tmp != NULL)
			  	{
			  		i++;
					if(i==1)
						strcpy(uip1,tmp);
					else if(i ==2 )
						strcpy(uip2,tmp);
					else if(i==3)
						strcpy(uip3,tmp);
					else if(i==4)
						strcpy(uip4,tmp);
					tmp = strtok(NULL,":");	
			  	}
				

				fprintf(cgiOut,"<tr>\n");
				fprintf(cgiOut,"<td width=80>\n");
				fprintf(cgiOut,"%s:",search(lpublic,"dhcp_rname"));
				fprintf(cgiOut,"</td>\n");
				fprintf(cgiOut,"<td>\n");
				fprintf(cgiOut,"<input type=text name=optclass value=\"%s\" disabled=disabled>",id);
				fprintf(cgiOut,"</td>\n");
				fprintf(cgiOut,"</tr>\n");

				fprintf(cgiOut,"<tr height=7><td></td></tr>\n");

				fprintf(cgiOut,"<tr>\n");
				fprintf(cgiOut,"<td width=80>\n");
				fprintf(cgiOut,"option60:");
				fprintf(cgiOut,"</td>\n");
				fprintf(cgiOut,"<td>\n");
				fprintf(cgiOut,"<input type=text name=iden value=\"%s\">",key);
				fprintf(cgiOut,"</td>\n");
				fprintf(cgiOut,"</tr>\n");

				fprintf(cgiOut,"<tr height=7><td></td></tr>\n");

				fprintf(cgiOut,"<tr>\n");
				fprintf(cgiOut,"<td width=80>\n");
				fprintf(cgiOut,"option43:");
				fprintf(cgiOut,"</td>\n");
				fprintf(cgiOut,"<td width=150 >"\
						"<div style=\"border-width:1;border-color:#a5acb2;border-style:solid;width:140;font-size:9pt\">");
	  		    fprintf(cgiOut,"<input type=text  name=ip1 value=\"%s\" maxlength=3 class=a3 onKeyUp=\"mask(this,%s)\" onbeforepaste=mask_c()>.",uip1,search(lpublic,"ip_error")); 
	  	        fprintf(cgiOut,"<input type=text  name=ip2 value=\"%s\" maxlength=3 class=a3 onKeyUp=\"mask(this,%s)\" onbeforepaste=mask_c()>.",uip2,search(lpublic,"ip_error"));
 		        fprintf(cgiOut,"<input type=text  name=ip3 value=\"%s\" maxlength=3 class=a3 onKeyUp=\"mask(this,%s)\" onbeforepaste=mask_c()>.",uip3,search(lpublic,"ip_error"));
		        fprintf(cgiOut,"<input type=text  name=ip4 value=\"%s\" maxlength=3 class=a3 onKeyUp=\"mask(this,%s)\" onbeforepaste=mask_c()>",uip4,search(lpublic,"ip_error"));
	  	        fprintf(cgiOut,"</div></td>");
				fprintf(cgiOut,"</tr>\n");

  fprintf(cgiOut,"<tr>"\
    "<td><input type=hidden name=encry_newvrrp value=\"%s\"></td>",m);
  fprintf(cgiOut,"<td><input type=hidden name=ID value=\"%s\"></td>",id);
  fprintf(cgiOut,"</tr>"\
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
return 0;
}



int ShowOptClearPage(char *m,char *id,struct list *lpublic,struct list *lcontrol)
{

	cgiHeaderContentType("text/html");	
	fprintf( cgiOut, "<html xmlns=\"http://www.w3.org/1999/xhtml\"> \n" );
	fprintf( cgiOut, "<head> \n" );
	fprintf( cgiOut, "<meta http-equiv=Content-Type content=text/html; charset=gb2312> \n" );
	//下面三句话用于禁止页面缓存
	fprintf( cgiOut, "<META   HTTP-EQUIV=\"pragma\"   CONTENT=\"no-cache\"> \n");
	fprintf( cgiOut, "<META   HTTP-EQUIV=\"Cache-Control\"   CONTENT=\"no-cache,   must-revalidate\"> \n" );
	fprintf( cgiOut, "<META   HTTP-EQUIV=\"expires\"   CONTENT=\"Wed,   26   Feb   1997   08:21:57   GMT\">	\n");

	fprintf( cgiOut, "<title>%s</title> \n", search( lcontrol, "del" ) );
	fprintf( cgiOut, "<link rel=stylesheet href=/style.css type=text/css> \n" );
	fprintf( cgiOut, "<style type=text/css> \n" );
	fprintf( cgiOut, ".usrlis {overflow-x:hidden; overflow:auto; width: 416px; height: 270px; clip: rect( ); padding-top: 0px; padding-right: 0px; padding-bottom: 0px; padding-left: 0px} \n" );
	fprintf( cgiOut, "</style> \n" );
	fprintf( cgiOut, "<style type=text/css> \n" );
	fprintf( cgiOut, "tr.even td { \n" );
	fprintf( cgiOut, "background-color: #eee; \n" );
	fprintf( cgiOut, "} \n" );
	fprintf( cgiOut, "tr.odd td { \n" );
	fprintf( cgiOut, "background-color: #fff; \n" );
	fprintf( cgiOut, "} \n" );
	fprintf( cgiOut, "tr.changed td { \n" );
	fprintf( cgiOut, "background-color: #ffd; \n" );
	fprintf( cgiOut, "} \n" );
	fprintf( cgiOut, " \n" ); 
	fprintf( cgiOut, "tr.new td { \n" );  
	fprintf( cgiOut, "background-color: #dfd; \n" );
	fprintf( cgiOut, "} \n" );
	fprintf( cgiOut, "</style> \n" );
	fprintf( cgiOut, "</head> \n" );		
	fprintf( cgiOut, "<body> \n" );


		int ret=-1;
		ret=delete_dhcp_onelevel(DHCP_XML, DHCP_OPT_NAME, DHCP_NODE_ATTR, id);
				///////////write to conf
		int bflag=-1,cflag=-1,oflag=-1,gflag=-1;

		struct bindipmac b_head;
		int bnum;

		struct optionsix o_head;
		int onum;

		struct confinfo c_head;
		int cnum;

		struct gbparam g_head;
		int gnum;

		bflag=read_bindinfo_xml(&b_head, &bnum);
		oflag=read_option_info(&o_head, &onum);
		cflag=read_confinfo_xml(&c_head, &cnum);
		gflag=read_gbparam_xml(&g_head,&gnum);
		
		if((bflag==0)&&(oflag==0)&&(cflag==0)&&(gflag==0))
		{
		ret=write_dhcp_conf(DHCP_CONF_PATH, &o_head, &c_head, &b_head,&g_head);
		}
		if((bflag==0 )&& (bnum > 0))
		Free_bindipmac_info(&b_head);

		if((cflag==0 )&& (cnum > 0))
		Free_confinfo_info(&c_head);

		if((oflag==0 )&& (onum > 0))
		Free_optionsix_info(&o_head);
		
		if((gflag==0 )&& (gnum > 0))
		Free_gbparam_info(&g_head);

		if(ret==0)
		   ShowAlert(search(lpublic,"oper_succ"));
		else
			ShowAlert(search(lpublic,"oper_fail"));

		fprintf( cgiOut, "<script type='text/javascript'>\n" );
		fprintf( cgiOut, "window.location.href='wp_dhcp_opt.cgi?UN=%s';\n", m);
		fprintf( cgiOut, "</script>\n" );	

		fprintf( cgiOut, "</body>\n" );
		fprintf( cgiOut, "</html>\n" );
	
}

