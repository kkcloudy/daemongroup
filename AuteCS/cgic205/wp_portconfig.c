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
* wp_portconfig.c
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
#include "ws_dcli_wlans.h"
#include "ws_dcli_vlan.h"


int ShowPortconPage(char *m,char *vid,struct list *lpublic,struct list *lcontrol);
void ConPort(int slot_count,int vid,struct vlan_ports_collection *vlan_ports,struct list *lpublic,struct list *lcontrol);

int cgiMain()
{  
  char encry[BUF_LEN] = { 0 };              
  char *str = NULL;                
  char ID[10] = { 0 };
  struct list *lpublic = NULL;   /*解析public.txt文件的链表头*/
  struct list *lcontrol = NULL;     /*解析control.txt文件的链表头*/  

  lpublic=get_chain_head("../htdocs/text/public.txt");
  lcontrol=get_chain_head("../htdocs/text/control.txt");
  
  ccgi_dbus_init();
  memset(encry,0,sizeof(encry));
  memset(ID,0,sizeof(ID));
  if(cgiFormStringNoNewlines("UN", encry, BUF_LEN)!=cgiFormNotFound )  /*首次进入该页*/
  {
	cgiFormStringNoNewlines("VID", ID, 10);	
  }  
  else
  {  
    cgiFormStringNoNewlines("encry_conport",encry,BUF_LEN);
    cgiFormStringNoNewlines("vlan_id",ID,10);  
  }  
    
  str=dcryption(encry);
  if(str==NULL)
	ShowErrorPage(search(lpublic,"ill_user"));			  /*用户非法*/
  else
	ShowPortconPage(encry,ID,lpublic,lcontrol);
  
  release(lpublic);  
  release(lcontrol);
  destroy_ccgi_dbus();
  return 0;
}

int ShowPortconPage(char *m,char *vid,struct list *lpublic,struct list *lcontrol)
{    
  char IsSubmit[5] = { 0 };
  char *endptr = NULL;  
  int limit = 0,vlan_id = 0,i = 0,cl = 1;
  int slot_count = get_product_info(SLOT_COUNT_FILE);
  struct vlan_ports_collection vlan_ports[SLOT_MAX_NUM+1];
  unsigned int slot_id = 0, port_id = 0;
  char port[10] = { 0 };
  int tag_flag = 0;

  vlan_id=strtoul(vid,&endptr,10);	/*char转成int，10代表十进制*/	
  cgiHeaderContentType("text/html");
  fprintf(cgiOut,"<html xmlns=\"http://www.w3.org/1999/xhtml\"><head>");
  fprintf(cgiOut,"<meta http-equiv=Content-Type content=text/html; charset=gb2312>");
  fprintf(cgiOut,"<title>PortConfig</title>");
  fprintf(cgiOut,"<link rel=stylesheet href=/style.css type=text/css>"\
  "</head>"\
  "<body>");  
  get_vlan_ports_collection(vlan_ports);

  memset(IsSubmit,0,sizeof(IsSubmit));  
  cgiFormStringNoNewlines("SubmitFlag", IsSubmit, 5);  
  if((cgiFormSubmitClicked("portconser_apply") == cgiFormSuccess)&&(strcmp(IsSubmit,"")))
  {  	
		ConPort(slot_count,vlan_id,vlan_ports,lpublic,lcontrol);
  }
  fprintf(cgiOut,"<form method=post>"\
  "<div align=center>"\
  "<table width=976 border=0 cellpadding=0 cellspacing=0>"\
  "<tr>"\
    "<td width=8 align=left valign=top background=/images/di22.jpg><img src=/images/youce4.jpg width=8 height=30/></td>"\
    "<td width=51 align=left valign=bottom background=/images/di22.jpg><img src=/images/youce33.jpg width=37 height=24/></td>"\
    "<td width=153 align=left valign=bottom id=titleen background=/images/di22.jpg>WLAN</td>"\
    "<td width=690 align=right valign=bottom background=/images/di22.jpg>");	
	   
    	  fprintf(cgiOut,"<table width=155 border=0 cellspacing=0 cellpadding=0>"\
          "<tr>");
		  if(1 == vlan_id)
	          fprintf(cgiOut,"<td width=62 align=center><a href=wp_configvlan.cgi?UN=%s&VID=%s target=mainFrame><img src=/images/%s border=0 width=62 height=20/></a></td>",m,vid,search(lpublic,"img_ok"));
		  else
		  	  fprintf(cgiOut,"<td width=62 align=center><input id=but type=submit name=portconser_apply style=background-image:url(/images/%s) value=""></td>",search(lpublic,"img_ok"));		  
          fprintf(cgiOut,"<td width=62 align=center><a href=wp_configvlan.cgi?UN=%s&VID=%s target=mainFrame><img src=/images/%s border=0 width=62 height=20/></a></td>",m,vid,search(lpublic,"img_cancel"));
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
	              fprintf(cgiOut,"<tr height=26>");
				  	if(1 == vlan_id)
	                    fprintf(cgiOut,"<td align=left id=tdleft background=/images/bottom_bg.gif style=\"border-right:0\"><font id=%s>%s</font></td>",search(lpublic,"menu_san"),search(lcontrol,"port_show"));   /*突出显示*/
					else
						fprintf(cgiOut,"<td align=left id=tdleft background=/images/bottom_bg.gif style=\"border-right:0\"><font id=%s>%s</font></td>",search(lpublic,"menu_san"),search(lcontrol,"port_configure"));   /*突出显示*/
                  fprintf(cgiOut,"</tr>");
				  
				  for(i = 1; i <= slot_count; i++)
				  {
					if(vlan_ports[i].have_port)
					{
						limit += (vlan_ports[i].port_max - vlan_ports[i].port_min + 1);
					}
				  }

				  limit *= 0.85;
                  for(i=0;i<limit;i++)
	              {
  				    fprintf(cgiOut,"<tr height=25>"\
                      "<td id=tdleft>&nbsp;</td>"\
                    "</tr>");
	              }
                fprintf(cgiOut,"</table>"\
              "</td>"\
              "<td align=left valign=top style=\"background-color:#ffffff; border-right:1px solid #707070; padding-left:30px; padding-top:10px\">"\
       "<table width=550 border=0 cellspacing=0 cellpadding=0>"\
		"<tr>"\
		  "<td><table width=550 border=0 cellspacing=0 cellpadding=0>");
                  fprintf(cgiOut,"<tr height=30 align=left>");
				  	if(1 == vlan_id)
						fprintf(cgiOut,"<td id=thead5 align=left>%sVLAN%s%s</td>",search(lcontrol,"view_port1"),vid,search(lcontrol,"view_port2"));
					else
                    	fprintf(cgiOut,"<td id=thead5 align=left>VLAN%s %s</td>",vid,search(lcontrol,"port_configure"));
                  fprintf(cgiOut,"</tr>"\
               "</table>"\
          "</td>"\
        "</tr>"\
		"<tr><td align=left style=\"padding-left:20px\">");
			  	fprintf(cgiOut,"<table width=550 border=0 cellspacing=0 cellpadding=0>");
                fprintf(cgiOut,"<tr>"\
				"<th align=left><font id=%s>%s</font></td>",search(lpublic,"menu_thead"),search(lcontrol,"_port"));
				fprintf(cgiOut,"<th align=left><font id=%s>Untagged%s</font></td>",search(lpublic,"menu_thead"),search(lcontrol,"member_port"));
				fprintf(cgiOut,"<th align=left><font id=%s>Tagged%s</font></td>",search(lpublic,"menu_thead"),search(lcontrol,"member_port"));
				fprintf(cgiOut,"<th align=left><font id=%s>%s</font></td>",search(lpublic,"menu_thead"),search(lcontrol,"non_members"));
				fprintf(cgiOut,"</tr>");
				for(slot_id = 1; slot_id <= slot_count; slot_id++)
				{
					if(vlan_ports[slot_id].have_port)
					{
						for(port_id = vlan_ports[slot_id].port_min; port_id <= vlan_ports[slot_id].port_max; port_id++)
						{
							memset(port, 0, sizeof(port));
							snprintf(port, sizeof(port)-1, "%u/%u",slot_id,port_id);

							tag_flag = 0;
							get_vlan_port_member_tagflag(vlan_id, port, &tag_flag);
							fprintf(cgiOut,"<tr bgcolor=%s>",setclour(cl));
							fprintf(cgiOut,"<td>%s</td>",port);
							for(i=1;i<4;i++)
							{
								if(i == tag_flag)
									fprintf(cgiOut,"<td><input type=radio name=%s value=%d checked=checked></td>",port,i);
								else
									fprintf(cgiOut,"<td><input type=radio name=%s value=%d></td>",port,i);
							}
							fprintf(cgiOut,"</tr>");
							cl=!cl;
						}
					}
				}
                fprintf(cgiOut,"<tr>"\
                "<td><input type=hidden name=encry_conport value=%s></td>",m);
                fprintf(cgiOut,"<td><input type=hidden name=vlan_id value=%s></td>",vid);	
				fprintf(cgiOut,"<td colspan=2><input type=hidden name=SubmitFlag value=%d></td>",1);
	          fprintf(cgiOut,"</tr>");
  fprintf(cgiOut,"</table></td></tr>"\
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

void ConPort(int slot_count,int vid,struct vlan_ports_collection *vlan_ports,struct list *lpublic,struct list *lcontrol)
{
	void *connection = NULL;
	int ret = 0, flag = 1, flag1 = 0, flag2 = 0;
	unsigned int slot_id = 0, port_id = 0;
	char port_name[10] = { 0 };
	char port_value[5] = { 0 };
	char *endptr = NULL;  
	int old_port_type = 0, new_port_type = 0;
	char alert1[1024] = { 0 }, alert2[1024] = { 0 };
	
	for(slot_id = 1; slot_id <= slot_count; slot_id++)
	{
		if(vlan_ports[slot_id].have_port)
		{			
			connection = NULL;
			if(SNMPD_DBUS_SUCCESS != get_slot_dbus_connection(slot_id, &connection, SNMPD_INSTANCE_MASTER_V3))
			{
				continue;
			}
			
			for(port_id = vlan_ports[slot_id].port_min; port_id <= vlan_ports[slot_id].port_max; port_id++)
			{				
				memset(port_name, 0, sizeof(port_name));
				snprintf(port_name, sizeof(port_name)-1, "%u/%u", slot_id,port_id);
				
				get_vlan_port_member_tagflag(vid, port_name, &old_port_type);

				memset(port_value,0,sizeof(port_value));
				cgiFormStringNoNewlines(port_name,port_value,5); 
				new_port_type=strtoul(port_value,&endptr,10);

				switch(old_port_type)
				{
					case 1:{
								if(1 == new_port_type)
								{
									ret =1;
									break;
								}
								
								ret = addordelete_port(connection,"delete",port_name,"untag",vid,NULL);
								if(1 != ret)
								{
									flag = 0;
								}
								
								if(2 == new_port_type)
								{
									ret = addordelete_port(connection,"add",port_name,"tag",vid,NULL);
								}
						   }
						   break;
					case 2:{
								if(2 == new_port_type)
								{
									ret =1;
									break;
								}
								
								ret = addordelete_port(connection,"delete",port_name,"tag",vid,NULL);
								if(1 != ret)
								{
									flag = 0;
								}
								
								if(1 == new_port_type)
								{
									ret = addordelete_port(connection,"add",port_name,"untag",vid,NULL);
								}
						   }
					       break;
					case 3:{
								if(3 == new_port_type)
								{
									ret =1;
									break;
								}

								if(1 == new_port_type)
								{
									ret = addordelete_port(connection,"add",port_name,"untag",vid,NULL);
								}
								else if(2 == new_port_type)
								{
									ret = addordelete_port(connection,"add",port_name,"tag",vid,NULL);
								}
						   }
						   break;
				}

				if((-6 == ret)||(-8 == ret))
				{
					flag = 0;
					flag1 = 1;
					strncat(alert1, port_name ,sizeof(alert1)-strlen(alert1)-1);
					strncat(alert1, "," ,sizeof(alert1)-strlen(alert1)-1);
				}
				else if(1 != ret)
				{
					flag = 0;
					flag2 = 1;
					strncat(alert2, port_name ,sizeof(alert2)-strlen(alert2)-1);
					strncat(alert2, "," ,sizeof(alert2)-strlen(alert2)-1);
			    }
			}
		}
	}

	strncat(alert1, search(lcontrol,"port_is_other_vlan_member") ,sizeof(alert1)-strlen(alert1)-1);
	strncat(alert2, search(lpublic,"oper_fail") ,sizeof(alert2)-strlen(alert2)-1);

	if(flag==1)
	{
	  	ShowAlert(search(lpublic,"oper_succ"));
	}
	else
	{
		if(flag1==1)
		{
			ShowAlert(alert1);
		}

		if(flag2==1)
		{
			ShowAlert(alert2);
		}
	}
}



