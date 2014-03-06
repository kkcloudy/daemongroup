/* cgicTempDir is the only setting you are likely to need
	to change in this file. */

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
* wp_add_faclrule.c
*
*
* CREATOR:
* autelan.software.Network Dep. team
* qiaojie@autelan.com
*
* DESCRIPTION:
*
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
#include "ws_init_dbus.h"
#include "facl/facl_db.h"
#include "facl/facl_errcode.h"
#include "drp/drp_interface.h"

int ShowAddFaclRulePage(char *m,struct list *lpublic,struct list *lfirewall);    
void showInterfaceSelect();
void AddFaclRule(struct list *lpublic,struct list *lfirewall);

int cgiMain()
{
  char encry[BUF_LEN] = { 0 };			
  char *str = NULL;
  struct list *lpublic = NULL;   /*解析public.txt文件的链表头*/
  struct list *lfirewall = NULL;     /*解析firewall.txt文件的链表头*/  
  lpublic=get_chain_head("../htdocs/text/public.txt");
  lfirewall=get_chain_head("../htdocs/text/firewall.txt");
  
  DcliWInit();
  ccgi_dbus_init();
  memset(encry,0,sizeof(encry));
  if(cgiFormStringNoNewlines("UN", encry, BUF_LEN)!=cgiFormNotFound )  /*首次进入该页*/
  {
  	;
  }
  else
  {
    cgiFormStringNoNewlines("encry_addfaclrule",encry,BUF_LEN);
  }
  str=dcryption(encry);
  if(str==NULL)
	ShowErrorPage(search(lpublic,"ill_user"));			  /*用户非法*/
  else
    ShowAddFaclRulePage(encry,lpublic,lfirewall);
  release(lpublic);  
  release(lfirewall);
  destroy_ccgi_dbus();
  return 0;
}

int ShowAddFaclRulePage(char *m,struct list *lpublic,struct list *lfirewall)
{  
  char IsSubmit[5] = { 0 };
  int result = 0;
  struct list_head policy_buf_head = { 0 };
  policy_rule_buf_t *policy_buf = NULL;
  int i = 0;
  cgiHeaderContentType("text/html");
  fprintf(cgiOut,"<html xmlns=\"http://www.w3.org/1999/xhtml\"><head>");
  fprintf(cgiOut,"<meta http-equiv=Content-Type content=text/html; charset=gb2312>");
  fprintf(cgiOut,"<title>AddFACLRule</title>");
  fprintf(cgiOut,"<link rel=stylesheet href=/style.css type=text/css>"\
  	"<style type=text/css>"\
  	".a3{width:30;border:0; text-align:center}"\
  	"</style>"\
  "</head>"\
  	"<script src=/ip.js>"\
    "</script>"\
    "<script type=\"text/javascript\">"\
	"function showRuleMode(s)"\
	"{"\
		"var tb = document.getElementById(\"add_rule\");"\
    	"if(s == \"ip_port\")"\
    	"{"\
    		"tb.rows[6].style.display = \"none\";"\
    		"tb.rows[7].style.display = \"block\";"\
    		"tb.rows[8].style.display = \"block\";"\
    		"tb.rows[9].style.display = \"block\";"\
    		"tb.rows[10].style.display = \"block\";"\
    		"tb.rows[11].style.display = \"block\";"\
    		"tb.rows[12].style.display = \"block\";"\
    		"tb.rows[13].style.display = \"block\";"\
    	"}"\
    	"else"\
    	"{"\
    		"tb.rows[6].style.display = \"block\";"\
    		"tb.rows[7].style.display = \"none\";"\
    		"tb.rows[8].style.display = \"none\";"\
    		"tb.rows[9].style.display = \"none\";"\
    		"tb.rows[10].style.display = \"none\";"\
    		"tb.rows[11].style.display = \"none\";"\
    		"tb.rows[12].style.display = \"none\";"\
    		"tb.rows[13].style.display = \"none\";"\
    	"}"\
	"}"\
	"</script>"\
  "<body>");  
  memset(IsSubmit,0,sizeof(IsSubmit));  
  cgiFormStringNoNewlines("SubmitFlag", IsSubmit, 5);
  if((cgiFormSubmitClicked("addfaclrule_apply") == cgiFormSuccess)&&(strcmp(IsSubmit,"")))
  {
	AddFaclRule(lpublic,lfirewall);
  }
  fprintf(cgiOut,"<form>"\
  "<div align=center>"\
  "<table width=976 border=0 cellpadding=0 cellspacing=0>"\
  "<tr>"\
    "<td width=8 align=left valign=top background=/images/di22.jpg><img src=/images/youce4.jpg width=8 height=30/></td>"\
    "<td width=51 align=left valign=bottom background=/images/di22.jpg><img src=/images/youce33.jpg width=37 height=24/></td>"\
    "<td width=153 align=left valign=bottom id=titleen background=/images/di22.jpg>FACL</td>"\
    "<td width=690 align=right valign=bottom background=/images/di22.jpg>");
	  
    	  fprintf(cgiOut,"<table width=155 border=0 cellspacing=0 cellpadding=0>"\
          "<tr>"\
          "<td width=62 align=center><input id=but type=submit name=addfaclrule_apply style=background-image:url(/images/%s) value=""></td>",search(lpublic,"img_ok"));			  
     	  fprintf(cgiOut,"<td width=62 align=center><a href=wp_firewall.cgi?UN=%s target=mainFrame><img src=/images/%s border=0 width=62 height=20/></a></td>",m,search(lpublic,"img_cancel"));
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
                  "</tr>"\
  				    "<tr height=25>"\
  					  "<td align=left id=tdleft><a href=wp_facl_list.cgi?UN=%s target=mainFrame class=top><font id=yingwen_san>FACL</font><font id=%s>%s</font></a></td>",m,search(lpublic,"menu_san"),search(lpublic,"policy"));                       
                    fprintf(cgiOut,"</tr>"\
					"<tr height=25>"\
					  "<td align=left id=tdleft><a href=wp_add_facl.cgi?UN=%s target=mainFrame class=top><font id=%s>%s</font></a></td>",m,search(lpublic,"menu_san"),search(lfirewall,"add_facl"));					   
					fprintf(cgiOut,"</tr>"\
  				    "<tr height=26>"\
                      "<td align=left id=tdleft background=/images/bottom_bg.gif style=\"border-right:0\"><font id=%s>%s</font></td>",search(lpublic,"menu_san"),search(lfirewall,"tc_addrule"));   /*突出显示*/
					fprintf(cgiOut,"</tr>");
                  for(i=0;i<12;i++)
	              {
  				    fprintf(cgiOut,"<tr height=25>"\
                      "<td id=tdleft>&nbsp;</td>"\
                    "</tr>");
	              }
                fprintf(cgiOut,"</table>"\
              "</td>"\
              "<td align=left valign=top style=\"background-color:#ffffff; border-right:1px solid #707070; padding-left:30px\">"\
              "<table id='add_rule' width=380 border=0 cellspacing=0 cellpadding=0>"\
  "<tr height=30>"\
    "<td>FACL Index:</td>"\
    "<td align=left colspan=2><select name=facl_index id=facl_index style=width:101px>"\
		"<option value=>");
		memset(&policy_buf_head, 0, sizeof(policy_buf_head));
	    INIT_LIST_HEAD(&policy_buf_head);
	    result=facl_interface_show_running(ccgi_dbus_connection, &policy_buf_head);
	    if(result == 0)
	    {
	  	  list_for_each_entry(policy_buf, &policy_buf_head, node)
		  {
		  	if(policy_buf)
		  	{
				fprintf(cgiOut,"<option value=%d>%d",policy_buf->facl_tag,policy_buf->facl_tag);
		  	}
		  }
	    }
		facl_interface_free_policy_buf(&policy_buf_head);
	fprintf(cgiOut,"</select></td>"\
  "</tr>"\
  "<tr height=30>"\
    "<td width=70>%s:</td>",search(lfirewall,"rule_index"));
    fprintf(cgiOut,"<td width=110 align=left><input type=text name=rule_index maxLength=3 onkeypress=\"return event.keyCode>=48&&event.keyCode<=57\" size=15></td>");
    fprintf(cgiOut,"<td width=200><font color=red>(1--128)</font></td>"\
  "</tr>"\
  "<tr height=30>"\
    "<td>%s:</td>",search(lfirewall,"action_type"));
    fprintf(cgiOut,"<td align=left colspan=2><select name=action_type id=action_type style=width:101px>"\
		 "<option value=>"\
		 "<option value=deny>deny"\
		 "<option value=permit>permit"\
	"</select></td>"\
  "</tr>"\
  "<tr height=30>"\
    "<td>%s:</td>",search(lfirewall,"input_intf"));
    fprintf(cgiOut,"<td align=left colspan=2><select name=input_intf id=input_intf style=width:101px>"\
		"<option value=any>any");
		showInterfaceSelect();
	fprintf(cgiOut,"</select></td>"\
  "</tr>"\
  "<tr height=30>"\
    "<td>%s:</td>",search(lfirewall,"output_intf"));
    fprintf(cgiOut,"<td align=left colspan=2><select name=output_intf id=output_intf style=width:101px>"\
		"<option value=any>any");
		showInterfaceSelect();
	fprintf(cgiOut,"</select></td>"\
  "</tr>"\
  "<tr height=30>"\
    "<td>%s:</td>",search(lpublic,"mode"));
    fprintf(cgiOut,"<td align=left colspan=2><select name=mode id=mode style=width:101px onchange='showRuleMode(this.options[this.options.selectedIndex].value);'>"\
		"<option value=ip_port>IP/%s",search(lpublic,"port"));
		fprintf(cgiOut,"<option value=domain>%s",search(lfirewall,"domain"));
	fprintf(cgiOut,"</select></td>"\
  "</tr>"\
  
  "<tr height=30 style='display:none'>"\
	"<td>%s:</td>",search(lfirewall,"domain"));
	fprintf(cgiOut,"<td align=left><input type=text name=domain maxLength=64 onkeypress=\"return event.keyCode!=32\" size=15></td>");
	fprintf(cgiOut,"<td><font color=red>(%s)</font></td>",search(lfirewall,"most_64_char"));
  fprintf(cgiOut,"</tr>"\
	
  "<tr height=30>"\
	"<td>%s:</td>",search(lfirewall,"source_ip"));
	fprintf(cgiOut,"<td colspan=2>"\
		 "<input type='radio' name='source_ip_type' value='any' checked='checked'>%s:",search(lfirewall,"ruleedit_sourceaddr_any"));
    fprintf(cgiOut,"</td>"\
  "</tr>"\
  "<tr height=30>"\
	"<td>&nbsp;</td>"\
	"<td>"\
		 "<input type='radio' name='source_ip_type' value='ip'>%s",search(lfirewall,"ruleedit_addr_single"));
    fprintf(cgiOut,"</td>"\
	"<td>"\
		 "<div style=\"border-width:1;border-color:#a5acb2;border-style:solid;width:140;font-size:9pt\">"\
		 "<input type=text name='source_ip1' maxlength=3 class=a3 onKeyUp=\"mask(this,%s)\" onbeforepaste=mask_c()>.",search(lpublic,"ip_error"));
		 fprintf(cgiOut,"<input type=text name='source_ip2' maxlength=3 class=a3 onKeyUp=\"mask(this,%s)\" onbeforepaste=mask_c()>.",search(lpublic,"ip_error"));
		 fprintf(cgiOut,"<input type=text name='source_ip3' maxlength=3 class=a3 onKeyUp=\"mask(this,%s)\" onbeforepaste=mask_c()>.",search(lpublic,"ip_error"));
		 fprintf(cgiOut,"<input type=text name='source_ip4' maxlength=3 class=a3 onKeyUp=\"mask(this,%s)\" onbeforepaste=mask_c()>",search(lpublic,"ip_error"));
		 fprintf(cgiOut,"</div>"\
    "</td>"\
  "</tr>"\
  "<tr height=30>"\
	"<td>%s:</td>",search(lfirewall,"dest_ip"));
	fprintf(cgiOut,"<td colspan=2>"\
		 "<input type='radio' name='dest_ip_type' value='any' checked='checked'>%s:",search(lfirewall,"ruleedit_desaddr_any"));
    fprintf(cgiOut,"</td>"\
  "</tr>"\
  "<tr height=30>"\
	"<td>&nbsp;</td>"\
	"<td>"\
		 "<input type='radio' name='dest_ip_type' value='ip'>%s",search(lfirewall,"ruleedit_addr_single"));
    fprintf(cgiOut,"</td>"\
	"<td>"\
		 "<div style=\"border-width:1;border-color:#a5acb2;border-style:solid;width:140;font-size:9pt\">"\
		 "<input type=text name='dest_ip1' maxlength=3 class=a3 onKeyUp=\"mask(this,%s)\" onbeforepaste=mask_c()>.",search(lpublic,"ip_error"));
		 fprintf(cgiOut,"<input type=text name='dest_ip2' maxlength=3 class=a3 onKeyUp=\"mask(this,%s)\" onbeforepaste=mask_c()>.",search(lpublic,"ip_error"));
		 fprintf(cgiOut,"<input type=text name='dest_ip3' maxlength=3 class=a3 onKeyUp=\"mask(this,%s)\" onbeforepaste=mask_c()>.",search(lpublic,"ip_error"));
		 fprintf(cgiOut,"<input type=text name='dest_ip4' maxlength=3 class=a3 onKeyUp=\"mask(this,%s)\" onbeforepaste=mask_c()>",search(lpublic,"ip_error"));
		 fprintf(cgiOut,"</div>"\
    "</td>"\
  "</tr>"\
  "<tr height=30>"\
    "<td>%s:</td>",search(lfirewall,"tc_protocol"));
    fprintf(cgiOut,"<td align=left colspan=2><select name=protocol id=protocol style=width:101px>"\
		 "<option value=any>any"\
		 "<option value=tcp>tcp"\
		 "<option value=udp>udp"\
		 "<option value=icmp>icmp"\
	"</select></td>"\
  "</tr>"\
  "<tr height=30>"\
    "<td>%s:</td>",search(lfirewall,"source_port"));
    fprintf(cgiOut,"<td align=left><input type=text name=source_port maxLength=5 value=any onkeypress=\"return event.keyCode>=48&&event.keyCode<=57\" size=15></td>");
    fprintf(cgiOut,"<td><font color=red>(0--65535)</font></td>"\
  "</tr>"\
  "<tr height=30>"\
    "<td>%s:</td>",search(lfirewall,"dest_port"));
    fprintf(cgiOut,"<td align=left><input type=text name=dest_port maxLength=5 value=any onkeypress=\"return event.keyCode>=48&&event.keyCode<=57\" size=15></td>");
    fprintf(cgiOut,"<td><font color=red>(0--65535)</font></td>"\
  "</tr>"\
  "<tr>"\
    "<td><input type=hidden name=encry_addfaclrule value=%s></td>",m);
    fprintf(cgiOut,"<td colspan=2><input type=hidden name=SubmitFlag value=%d></td>",1);
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

void showInterfaceSelect()
{
	FILE *ft = NULL;
	char  temp[20] = { 0 };
	int i = 0,j = 0,is_repeat = 0;
	char intfname[4095][20] = {{0}};
	char syscommand[300] = { 0 };
	
	memset(temp,0,sizeof(temp));	
	memset(syscommand,0,sizeof(syscommand));
	
	sprintf(syscommand,"ip addr | awk 'BEGIN{FS=\":\";RS=\"(\^|\\n)[0-9]+:[ ]\"}{print $1}' | awk 'NR==2,NR==0{print}' ");
	ft = popen(syscommand,"r"); 
	if(ft != NULL)
	{
		while((fgets(temp,sizeof(temp),ft)) != NULL)
		{
			 is_repeat = 0;
			 for(j=0;j<i;j++)
			 {
			 	if(0 == strncmp(temp,intfname[j],strlen(temp)-1))
			 	{
			 		is_repeat = 1;
			 	}
			 }

			 if(0 == is_repeat)
			 {			 	 
				 strncpy(intfname[i],temp,strlen(temp)-1);
				 i++;
				 if(0 == strncmp(temp, "ve", 2)) {
					char *temp_ve = NULL;

				 	temp_ve = strchr(temp,'@');
					if (temp_ve) {
						*temp_ve = '\0';
					}
				 }
				 fprintf(cgiOut, "<option value='%s'>%s</option>\n", temp, temp);
			 }
			 memset(temp,0,20);
		}
		pclose(ft);
	}
}

static char * 
ip2str(uint32_t ip, char *str, size_t size)
{
	if (NULL != str)
	{
		snprintf(str, size, "%u.%u.%u.%u", 
			(ip>>24)&0xff, (ip>>16)&0xff, (ip>>8)&0xff, ip&0xff);
	}

	return str;
}

void AddFaclRule(struct list *lpublic,struct list *lfirewall)
{
	int ret = -1;  
	char *endptr = NULL; 
	char facl_index[10] = { 0 };
	int fid = 0;
	char rule_index[10] = { 0 };
	int rid = 0;
	char action_type[10] = { 0 };
	int type = 0;
	char input_intf[20] = { 0 };
	char output_intf[20] = { 0 };
	char mode[10] = { 0 };
	char domain[256] = { 0 };
	char ip1[4] = { 0 };
	char ip2[4] = { 0 };
	char ip3[4] = { 0 };
	char ip4[4] = { 0 };
	char source_ip[20] = { 0 };
	char dest_ip[20] = { 0 };
	char protocol[10] = { 0 };
	int proto = 0;
	char source_port[10] = { 0 };
	char dest_port[10] = { 0 };
	domain_pt domain_conf;
	domain_ct domain_ctr;
	char ipstr[32] = {0};
	int i = 0;
	
	memset(facl_index,0,sizeof(facl_index));
	cgiFormStringNoNewlines("facl_index",facl_index,10);   
	memset(rule_index,0,sizeof(rule_index));
	cgiFormStringNoNewlines("rule_index",rule_index,10);  
	memset(action_type,0,sizeof(action_type));
	cgiFormStringNoNewlines("action_type",action_type,10);  
	memset(input_intf,0,sizeof(input_intf));
	cgiFormStringNoNewlines("input_intf",input_intf,20); 
	memset(output_intf,0,sizeof(output_intf));
	cgiFormStringNoNewlines("output_intf",output_intf,20); 
	memset(mode,0,sizeof(mode));
	cgiFormStringNoNewlines("mode",mode,10);

	if((strcmp(facl_index,""))&&(strcmp(rule_index,""))&&(strcmp(action_type,"")))
	{
		if(0 == (strcmp(mode,"ip_port")))
		{
			memset(source_ip,0,sizeof(source_ip));
			cgiFormStringNoNewlines("source_ip_type",source_ip,20);
			if(0 == (strcmp(source_ip,"ip")))
			{
			    memset(ip1,0,sizeof(ip1));
			    cgiFormStringNoNewlines("source_ip1",ip1,4);
			    memset(ip2,0,sizeof(ip2));
			    cgiFormStringNoNewlines("source_ip2",ip2,4);
			    memset(ip3,0,sizeof(ip3));
			    cgiFormStringNoNewlines("source_ip3",ip3,4);
			    memset(ip4,0,sizeof(ip4));
			    cgiFormStringNoNewlines("source_ip4",ip4,4);
				if((strcmp(ip1,""))&&(strcmp(ip2,""))&&(strcmp(ip3,""))&&(strcmp(ip4,"")))
				{
					memset(source_ip,0,sizeof(source_ip));
					snprintf(source_ip, sizeof(source_ip)-1, "%s.%s.%s.%s", ip1, ip2, ip3, ip4);					
				}
				else
				{
					memset(source_ip,0,sizeof(source_ip));
					ShowAlert(search(lpublic,"para_incom"));
				}
			}

			memset(dest_ip,0,sizeof(dest_ip));
			cgiFormStringNoNewlines("dest_ip_type",dest_ip,20);
			if(0 == (strcmp(dest_ip,"ip")))
			{
			    memset(ip1,0,sizeof(ip1));
			    cgiFormStringNoNewlines("dest_ip1",ip1,4);
			    memset(ip2,0,sizeof(ip2));
			    cgiFormStringNoNewlines("dest_ip2",ip2,4);
			    memset(ip3,0,sizeof(ip3));
			    cgiFormStringNoNewlines("dest_ip3",ip3,4);
			    memset(ip4,0,sizeof(ip4));
			    cgiFormStringNoNewlines("dest_ip4",ip4,4);
				if((strcmp(ip1,""))&&(strcmp(ip2,""))&&(strcmp(ip3,""))&&(strcmp(ip4,"")))
				{
					memset(dest_ip,0,sizeof(dest_ip));
					snprintf(dest_ip, sizeof(dest_ip)-1, "%s.%s.%s.%s", ip1, ip2, ip3, ip4);					
				}
				else
				{
					memset(dest_ip,0,sizeof(dest_ip));
					ShowAlert(search(lpublic,"para_incom"));
				}
			}

			memset(protocol,0,sizeof(protocol));
			cgiFormStringNoNewlines("protocol",protocol,10);

			memset(source_port,0,sizeof(source_port));
			cgiFormStringNoNewlines("source_port",source_port,10);

			memset(dest_port,0,sizeof(dest_port));
			cgiFormStringNoNewlines("dest_port",dest_port,10);

			if((strcmp(source_ip,""))&&(strcmp(dest_ip,""))&&(strcmp(protocol,""))&&(strcmp(source_port,""))&&(strcmp(dest_port,"")))
			{
				fid = strtoul(facl_index,&endptr,10);
				rid = strtoul(rule_index,&endptr,10);

				if(0 == strcmp(action_type, "deny")) 
				{
					type = 1;
				}
				
				if(0 == strcmp(protocol, "icmp")) 
				{
					proto = 1;
				}
				else if (0 == strcmp(protocol, "tcp")) 
				{
					proto = 6;
				}
				else if (0 == strcmp(protocol, "udp")) 
				{
					proto = 17;
				}
				ret = facl_interface_add_rule(ccgi_dbus_connection, fid, rid, type, input_intf, output_intf, source_ip, dest_ip, proto, source_port, dest_port, "");
				if (FACL_RETURN_OK == ret) 
				{
					ShowAlert(search(lfirewall,"add_success"));
				}
				else if (FACL_INDEX_ALREADY_EXIST == ret) 
				{
					ShowAlert(search(lfirewall,"facl_rule_exist"));
				}
				else if (FACL_TOTAL_RULE_NUM_OVER == ret) 
				{
					ShowAlert(search(lfirewall,"facl_rule_outsize"));
				} 
				else 
				{
					ShowAlert(search(lpublic,"error"));
				}
			}
			else
			{
				ShowAlert(search(lpublic,"para_incom"));
			}
		}
		else/*mode is domain*/
		{
			memset(domain,0,sizeof(domain));
			cgiFormStringNoNewlines("domain",domain,256);

			if(strcmp(domain,""))
			{
				fid = strtoul(facl_index,&endptr,10);
				rid = strtoul(rule_index,&endptr,10);

				if(0 == strcmp(action_type, "deny")) 
				{
					type = 1;
				}
				
				memset(&domain_conf,0,sizeof(domain_conf));
				strncpy((domain_conf.domain_name),domain,sizeof(domain_conf.domain_name)-1);
				memset(&domain_ctr,0,sizeof(domain_ctr));
				
				ret = conf_drp_get_domain_ip(ccgi_dbus_connection,	&domain_conf, &domain_ctr);
				if(0 == ret)
				{
					for (i = 0; i<domain_ctr.num; i++)
					{
						ip2str(domain_ctr.domain_ip[i].ipaddr, ipstr, sizeof(ipstr));
						
						ret = facl_interface_add_rule(ccgi_dbus_connection, fid, rid, type, input_intf, output_intf, ipstr, "any", 0, "any", "any", domain);
						if (FACL_RETURN_OK != ret) 
						{
							break;
						}
				
						ret = facl_interface_add_rule(ccgi_dbus_connection, fid, rid, type, input_intf, output_intf, "any", ipstr, 0, "any", "any", domain);
						if (FACL_RETURN_OK != ret) 
						{
							break;
						}
					}
				}
				
				if (FACL_RETURN_OK == ret) 
				{
					ShowAlert(search(lfirewall,"add_success"));
				} 
				else if (FACL_INDEX_ALREADY_EXIST == ret) 
				{
					ShowAlert(search(lfirewall,"facl_rule_exist"));
				} 
				else if (FACL_TOTAL_RULE_NUM_OVER == ret) 
				{
					ShowAlert(search(lfirewall,"facl_rule_outsize"));
				} 
				else 
				{
					ShowAlert(search(lpublic,"error"));
				}
			}
			else
			{
				ShowAlert(search(lpublic,"para_incom"));
			}
		}
	}
	else
	{
		ShowAlert(search(lpublic,"para_incom"));
	}
}

