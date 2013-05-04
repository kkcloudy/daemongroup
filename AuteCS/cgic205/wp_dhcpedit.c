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
* wp_dhcpedit.c
*
*
* CREATOR:
* autelan.software.Network Dep. team
* zhouym@autelan.com
*
* DESCRIPTION:
* system cotrl for dhcp edit
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

#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>



#include "ws_dcli_dhcp.h"
#include "ws_init_dbus.h"
#include "ws_dbus_list_interface.h"

int ShowDhcpEditPage(char *encry,char *eid,struct list *lcontrol,struct list *lpublic);
void ConfIPAdd(struct list *lcontrol,struct list *lpublic,char *poolname);
int ShowConfClearPage(char *m,char *id,struct list *lcontrol,struct list *lpublic);

int cgiMain()
{
	char *encry=(char *)malloc(BUF_LEN);  
	char *str;   

	struct list *lpublic;   /*解析public.txt文件的链表头*/
    struct list *lcontrol;     /*解析control.txt文件的链表头*/  
    lpublic=get_chain_head("../htdocs/text/public.txt");
    lcontrol=get_chain_head("../htdocs/text/control.txt");

    ccgi_dbus_init();
	///////////////
	char eid[30];
	memset(eid,0,30);

	char etype[10];
	memset(etype,0,10);
	cgiFormStringNoNewlines("NAME",eid,30);
	cgiFormStringNoNewlines("TYPE",etype,10);
	cgiFormStringNoNewlines("UN",encry,BUF_LEN);
	
	str=dcryption(encry);
	if(str==NULL)
	ShowErrorPage(search(lpublic,"ill_user"));		 /*用户非法*/
	else 
	{
		if(strcmp(etype,"2")==0)
		   ShowConfClearPage(encry,eid,lcontrol,lpublic);
		else
		   ShowDhcpEditPage(encry,eid,lcontrol,lpublic);
	}  
	free(encry);
	release(lpublic);  
    release(lcontrol);

	return 0;
}


int ShowDhcpEditPage(char *encry,char *eid,struct list *lcontrol,struct list *lpublic)
{ 

	int i = 0;
	char *tmp=(char*)malloc(30);
	memset(tmp,0,30);

	char *startip=(char *)malloc(32);
	memset(startip,0,32);
	char st_ip1[4];
	char st_ip2[4];
	char st_ip3[4];
	char st_ip4[4];

	char *endip=(char *)malloc(32);
	memset(endip,0,32);
	char end_ip1[4];
	char end_ip2[4];
	char end_ip3[4];
	char end_ip4[4];


	char *dhcpmask=(char *)malloc(32);
	memset(dhcpmask,0,32);
	char mask_ip1[4];
	char mask_ip2[4];
	char mask_ip3[4];
	char mask_ip4[4];

	char *optypez = (char *)malloc(512);
	memset(optypez,0,512);
	
	char *tmp1=(char *)malloc(50);
	memset(tmp1,0,50);
	char *tmp2=(char *)malloc(32);
	memset(tmp2,0,32);
	char *tmp3=(char *)malloc(32);
	memset(tmp3,0,32);

    char *routeip=(char *)malloc(32);
	memset(routeip,0,32);
	char route1[4];
	char route2[4];
	char route3[4];
	char route4[4];

    char *winip=(char *)malloc(32);
	memset(winip,0,32);
	char win1[4];
	char win2[4];
	char win3[4];
	char win4[4];

	struct dhcp_global_show_st global_show;
	memset(&global_show,0,sizeof(struct dhcp_global_show));
	global_show.domainname = (char *)malloc(30);
	memset(global_show.domainname,0,30);
	global_show.option43 = (char *)malloc(256);	
	memset(global_show.option43,0,256);


	char *domname=(char *)malloc(50);
	memset(domname,0,50);

	char *ip_mask=(char *)malloc(32);
	memset(ip_mask,0,32);

	int lease_t = 0;
	char day[5];
	char hour[5];
	char min[5];
	int ttmp;

	char *sipdef = (char *)malloc(32);
	memset(sipdef,0,32);
	char *eipdef = (char *)malloc(32);
	memset(eipdef,0,32);

	char *riptmp = (char *)malloc(32);
	memset(riptmp,0,32);
	char *wiptmp = (char *)malloc(32);
	memset(wiptmp,0,32);

	char *siptmp=(char *)malloc(32);
	memset(siptmp,0,32);
	char *eiptmp=(char *)malloc(32);
	memset(eiptmp,0,32);

	char *gdom = (char *)malloc(32);
    char *groute = (char *)malloc(32);
	char *gwin = (char *)malloc(32);
	char *glease = (char *)malloc(32);
	char *gdns1 =(char *)malloc(32);
	char *gdns2 = (char *)malloc(32);
	char *gdns3 = (char *)malloc(32);
	char *gdns = (char *)malloc(128);
	memset(gdns,0,128);
	char grip1[4] = {0};
	char grip2[4] = {0};
	char grip3[4] = {0};
	char grip4[4] = {0};
	char gwip1[4] = {0};
	char gwip2[4] = {0};
	char gwip3[4] = {0};
	char gwip4[4] = {0};
    

	
 ///////////////add new 
	struct dhcp_pool_show_st  head,*q;
    memset(&head,0,sizeof(struct dhcp_pool_show_st));
	struct dhcp_sub_show_st *pq;
     
	unsigned int mode = 0, index = 0, count = 0;	
	int cflag=-1;

    char *tip1=(char *)malloc(32);
    char *tip2=(char *)malloc(32);
    char *tip3=(char *)malloc(32);
 //////////////end add new 
    char *dns = (char*)malloc(128);
 	memset(dns,0,128);

	instance_parameter *paraHead2 = NULL;
	instance_parameter *p_q = NULL;
	list_instance_parameter(&paraHead2, SNMPD_SLOT_CONNECT);
	char allslotid[10] = {0};
	int allslot_id = 0;
	cgiFormStringNoNewlines("allslotid",allslotid,sizeof(allslotid));
	allslot_id = atoi(allslotid);
	if(0 == allslot_id)
	{
		for(p_q=paraHead2;(NULL != p_q);p_q=p_q->next)
		{
			allslot_id = p_q->parameter.slot_id;
			break;
		}
	}	

  cgiHeaderContentType("text/html");
  fprintf(cgiOut,"<html xmlns=\"http://www.w3.org/1999/xhtml\"><head>");
  fprintf(cgiOut,"<meta http-equiv=Content-Type content=text/html; charset=gb2312>");
  fprintf(cgiOut,"<link rel=stylesheet href=/style.css type=text/css>\n"\
  	"<style type=text/css>\n"\
  	".a3{width:30;border:0; text-align:center}\n"\
  	"</style>\n"\
    "<script type=\"text/javascript\">");
	 fprintf(cgiOut, "function get_ip_addr(str_name)\n"\
								"{\n"\
								   "var str_para = \"\";\n"\
								   "var obj_temp;\n"\
								   "var str_temp = \"\";\n"\
								   "var iter = 1;\n"\
								   "for(; iter<=4; iter++)\n"\
								   "{\n"\
									  "str_temp = \"document.getElementById(\\\"\" +  str_name + iter + \"\\\")\";\n"\
									  "obj_temp = eval(str_temp);\n"\
									  "if(\"\" == obj_temp.value)\n"\
									  "{\n"\
										 "return null;\n"\
									  "}\n"\
									  "str_para += obj_temp.value + \".\";\n"\
								   "}\n"\
								   "str_temp = str_para.replace(/\\.$/,\"\");\n"\
								   "return str_temp;\n"\
								"}");
	 fprintf(cgiOut, "function get_sub_net(str_ip_addr, mask)"
                            "{\n"\
                               "var tmp = \"\";\n"\
                               "var ip_tmp = ip2val(str_ip_addr);\n"\
                               "var mask_tmp = ip2val(mask);\n"\
                               "var str_ip_addr = \"\";\n"\
                               "var tmp_ip = ip_tmp >>> 24;\n"\
                               "var tmp_mask = mask_tmp >>>24;\n"\
                               "tmp = tmp_ip&tmp_mask;\n"\
                               "str_ip_addr = tmp.toString(10);\n"\
	 						   "tmp_ip = ip_tmp << 8;\n"\
                               "tmp_ip = tmp_ip >>> 24;\n"\
                               "tmp_mask = mask_tmp << 8;\n"\
                               "tmp_mask = tmp_mask >>> 24;\n"\
							   "tmp = tmp_ip&tmp_mask;\n"\
							   "str_ip_addr += \".\" + tmp.toString(10);\n"\
							   "tmp_ip = ip_tmp << 16;\n"\
                               "tmp_ip = tmp_ip >>> 24;\n"\
							   "tmp_mask = mask_tmp << 16;\n"\
							   "tmp_mask = tmp_mask >>> 24;\n"\
	 							"tmp = tmp_ip&tmp_mask;\n"\
								"str_ip_addr += \".\" + tmp.toString(10);\n"\
								"tmp_ip = ip_tmp << 24;\n"\
                               "tmp_ip = tmp_ip >>> 24;\n"\
							   "tmp_mask = mask_tmp << 24;\n"\
							   "tmp_mask = tmp_mask >>> 24;\n"\
								"tmp = tmp_ip&tmp_mask;\n"\
								"str_ip_addr += \".\" + tmp.toString(10);\n"\
								"return str_ip_addr;\n"\
                            "}");	 
	  fprintf(cgiOut, "function ip2val(str_ip_addr)\n"\
                            "{\n"\
                               "var arr_ip_seg = str_ip_addr.split(\".\");\n"\
                               "var un_val = 0;\n"\
                               "for(var i=0; i<4; i++)\n"\
                               "{\n"\
                                  "un_val = (un_val << 8) + parseInt(arr_ip_seg[i]);\n"\
                               "}\n"\
                               "return un_val;\n"\
                            "}");
	   fprintf(cgiOut, "function clear_ip_addr(str_name)\n"\
                            "{\n"\
                               "var obj_temp;\n"\
                               "var str_temp = \"\";"\
                               "var iter = 1;\n"\
                               "for(; iter<=4; iter++)\n"\
                               "{\n"\
                                  "str_temp = \"document.getElementById(\\\"\" +  str_name + iter + \"\\\")\";\n"\
                                  "obj_temp = eval(str_temp);\n"\
                                  "obj_temp.value = \"\";\n"\
                               "}\n"\
                            "}");
	  fprintf(cgiOut,"function rm_dns(){\n"\
	  	"for(var i=0; i<document.all.dnselect.options.length; i++){\n"\
		  "if(document.all.dnselect.options[i].selected==true){\n"\
		  "var optionIndex ;\n"\
		  "optionIndex = i;\n"\
		  "}\n"\
		  "if(optionIndex!=null){\n"\
		  "document.all.dnselect.remove(optionIndex);\n"\
		  "}\n"\
		  "}\n"\
          "}");	
	  fprintf(cgiOut,"function grm_dns(){\n"\
	  	"for(var i=0; i<document.all.gdnselect.options.length; i++){\n"\
		  "if(document.all.gdnselect.options[i].selected==true){\n"\
		  "var optionIndex ;\n"\
		  "optionIndex = i;\n"\
		  "}\n"\
		  "if(optionIndex!=null){\n"\
		  "document.all.gdnselect.remove(optionIndex);\n"\
		  "}\n"\
		  "}\n"\
          "}");	
  	  fprintf(cgiOut,"function rm_opt(){\n"\
  	"for(var i=0; i<document.all.optselect.options.length; i++){\n"\
	  "if(document.all.optselect.options[i].selected==true){\n"\
	  "var optionIndex ;\n"\
	  "optionIndex = i;\n"\
	  "}\n"\
	  "if(optionIndex!=null){\n"\
	  "document.all.optselect.remove(optionIndex);\n"\
	  "}\n"\
	  "}\n"\
      "}");
	fprintf(cgiOut,"function add_dns()\n"\
			"{\n"\
				"var dnsip = get_ip_addr(\"dns_ip\");\n"\
				"if(dnsip != null)\n"\
					"{\n"\
					"document.all.dnselect.options.add(\n"\
					 "new Option(dnsip,dnsip,false,false)\n"\
					 ");\n"\
					"}\n"\
					"else"\
				"{\n"\
				"alert(\"%s\");\n"\
				"window.event.returnValue = false;\n"\
				"}\n"\
				"clear_ip_addr(\"dns_ip\");\n"\
			"}",search(lpublic,"ip_not_null"));
	fprintf(cgiOut,"function gadd_dns()\n"\
			"{\n"\
				"var gdnsip = get_ip_addr(\"gdns_ip\");\n"\
				"if(gdnsip != null)\n"\
					"{\n"\
					"document.all.gdnselect.options.add(\n"\
					 "new Option(gdnsip,gdnsip,false,false)\n"\
					 ");\n"\
					"}\n"\
					"else"\
				"{\n"\
				"alert(\"%s\");\n"\
				"window.event.returnValue = false;\n"\
				"}\n"\
				"clear_ip_addr(\"gdns_ip\");\n"\
			"}",search(lpublic,"ip_not_null"));
				fprintf(cgiOut,"function add_opt()\n"\
			"{\n"\
				"var optip = get_ip_addr(\"opt_ip\");\n"\
				"for(var k=0; k<document.all.optselect.options.length; k++)\n"\
				"{\n"\
                  "if(optip == document.all.optselect.options[k].value)\n"\
                  "{\n"\
	                  "alert(\"%s\");\n"\
	                  "clear_ip_addr(\"opt_ip\");\n"\
	                  "return;\n"\
                  "}\n"\
				"}\n"\
				"if(optip != null)\n"\
				"{\n"\
					"document.all.optselect.options.add(\n"\
					 "new Option(optip,optip,false,false)\n"\
					 ");\n"\
				"}\n"\
				"else\n"\
				"{\n"\
					"alert(\"%s\");\n"\
					"window.event.returnValue = false;\n"\
				"}\n"\
				"clear_ip_addr(\"opt_ip\");\n"\
			"}",search(lpublic,"dhcp_exist"),search(lpublic,"ip_not_null"));
       fprintf(cgiOut,"function hexen()\n"\
	   		"{\n"\
		   		"document.getElementById(\"hex1\").style.display = 'block';\n"\
		   		"document.getElementById(\"hex2\").style.display = 'none';\n"\
		   		"document.getElementById(\"hex3\").style.display = 'none';\n"\
	   		"}");
       fprintf(cgiOut,"function opten()\n"\
	   		"{\n"\
		   		"document.getElementById(\"hex1\").style.display = 'none';\n"\
		   		"document.getElementById(\"hex2\").style.display = 'block';\n"\
		   		"document.getElementById(\"hex3\").style.display = 'block';\n"\
	   		"}\n");
	   fprintf(cgiOut, "function mysubmit()\n"\
					 "{\n"\
					    "var un_temp = 0;\n"\
					    "var j = 0;"\
		   		 		"var flagz = \"\";\n"\
		   		 		"var sflag = 0;\n"\
		   		 		"var eflag =0;\n"\
		   		 		"var tt = \"\";"\
		   		 		"var dnsnum = document.all.dnselect.options.length;\n"\
		   		 		"var gdnsnum = document.all.gdnselect.options.length;\n"\
		   		 		"if(dnsnum>3)\n"\
		   		 		"{\n"\
		   		 			"alert(\"%s\");\n"\
		   		 			"return false;\n"\
		   		 		"}\n"\
		   		 		"if(gdnsnum>3)\n"\
		   		 		"{\n"\
		   		 			"alert(\"%s\");\n"\
		   		 			"return false;\n"\
		   		 		"}\n"\
		   		 		"tt = \"\";"\
						"for(j=0; j<document.all.dnselect.options.length; j++)\n"\
						"{\n"\
							"tt = document.all.dnselect.options[j].value;\n"\
   							"if(j==0)\n"\
				   				"{\n"\
				   					"document.all.dns_ip.value = tt;\n"\
				   				"}\n"\
				   				"else{\n"\
				   					"document.all.dns_ip.value = document.all.dns_ip.value + \",\" + tt;\n"\
				   				"}\n"\
					   "}\n"\
					   "tt = \"\";"\
					   "for(j=0; j<document.all.gdnselect.options.length; j++)\n"\
						"{\n"\
							"tt = document.all.gdnselect.options[j].value;\n"\
   							"if(j==0)\n"\
				   				"{\n"\
				   					"document.all.gdns_ip.value = tt;\n"\
				   				"}\n"\
				   				"else{\n"\
				   					"document.all.gdns_ip.value = document.all.gdns_ip.value + \",\" + tt;\n"\
				   				"}\n"\
					   "}\n"\
					   "tt = \"\";"\
					   "for(var k=0; k<document.all.optselect.options.length; k++)\n"\
							"{\n"\
								"tt = document.all.optselect.options[k].value;\n"\
	   							"if(k==0)\n"\
					   				"{\n"\
					   					"document.all.opt_ip.value = tt;\n"\
					   				"}\n"\
					   				"else{\n"\
					   					"document.all.opt_ip.value = document.all.opt_ip.value + \" \" + tt;\n"\
					   				"}\n"\
					   "}\n"\
					   "var v1=document.all.day.value;\n"\
		   				"if(v1>365)\n"\
	   	    			 "{\n"\
	   	     				"alert(\"%s\");\n"\
	   	      				"return false;\n"\
		   		 		"}\n"\
		   		 		"var v2=document.all.hour.value;\n"\
		   				"if(v2>23)\n"\
	   	    			 "{\n"\
	   	     				"alert(\"%s\");\n"\
	   	      				"return false;\n"\
		   		 		"}\n"\
		   		 		"var v3=document.all.min.value;\n"\
		   				"if(v3>59)\n"\
	   	    			 "{\n"\
	   	     				"alert(\"%s\");\n"\
	   	      				"return false;\n"\
		   		 		"}\n"\
		   		 		"var v4=v1 + v2 + v3;\n"\
		   				"if(v4 == \"\")\n"\
		   	    		"{\n"\
	   	     				"alert(\"%s\");\n"\
	   	      				"return false;\n"\
		   		 		"}\n"\
		   		 		"var leaseall = v1*86400 + v2*3600 + v3*60;"\
		   		 		"if(leaseall>31536000)"\
		   		 		"{"\
		   		 		"alert(\"%s\");"\
		   		 		"return false;"\
		   		 		"}"\
		   		 		"var gleasez = document.all.glease.value;"\
		   		 		"if(gleasez != \"\")\n"\
		   		 		"{"\
			   		 		"if((gleasez>31536000)||(gleasez<60))"\
			   		 		"{"\
				   		 		"alert(\"%s\");"\
				   		 		"return false;"\
			   		 		"}"\
		   		 		"}"\
						"var startipz = get_ip_addr(\"beg_ip\");\n"\
						"var endipz = get_ip_addr(\"end_ip\");\n"\
						"if((startipz == null)&&(endipz == null))\n"\
						"{\n"\
							"alert(\"%s\");\n"\
							"return false;\n"\
						"}\n"\
						"var maskz = get_ip_addr(\"mask_ip\");\n"\
						"document.all.ip_mask.value = maskz;"\
						"if((startipz != null)&&(endipz != null))\n"\
						"{\n"\
							"if(get_sub_net(startipz, maskz)!= get_sub_net(endipz, maskz))\n"\
							"{\n"\
								"alert(\"%s\");\n"\
								"return false;\n"\
							"}\n"\
							"else"\
							"{\n"\
							    "un_temp = 0;\n"\
							    "sflag = 0;\n"\
								"un_temp = ip2val(startipz)&ip2val(maskz);\n"\
								"un_temp = ip2val(startipz)^un_temp;\n"\
								"sflag = un_temp;\n"\
								"un_temp = 0;\n"\
								"eflag = 0;\n"\
								"un_temp = ip2val(endipz)&ip2val(maskz);\n"\
								"un_temp = ip2val(endipz)^un_temp;\n"\
								"eflag = un_temp;\n"\
								"if(eflag < sflag)\n"\
	                            "{\n"\
	                            	"alert(\"%s\");\n"\
	                            	"return false;\n"\
	                            "}\n"\
	                            "else"\
	                            "{\n"\
									"document.all.startip.value=startipz;\n"\
									"document.all.endip.value=endipz;\n"\
								"}\n"\
							"}\n"\
						"}\n"\
						"else"\
						"{\n"\
							"alert(\"%s\");\n"\
							"return false;\n"\
						"}\n"\
	   					"var routipz=get_ip_addr(\"route_ip\");\n"\
	   					"if(routipz != null)\n"\
						"{\n"\
							"document.all.routeip.value = routipz;\n"\
						"}\n"\
	   					"var winipz=get_ip_addr(\"win_ip\");\n"\
	   					"if(winipz != null)\n"\
						"{\n"\
							"document.all.winip.value = winipz;\n"\
						"}\n"\
						"var groutipz=get_ip_addr(\"groute_ip\");\n"\
						"if(groutipz != null)\n"\
						"{\n"\
							"document.all.grouteip.value = groutipz;\n"\
						"}\n"\
						"var gwinipz=get_ip_addr(\"gwin_ip\");\n"\
						"if(gwinipz != null)\n"\
						"{\n"\
							"document.all.gwinip.value = gwinipz;\n"\
						"}\n"\
		   		"}",search(lcontrol,"dns_max_num"),search(lcontrol,"dns_max_num"),search(lcontrol,"lease_d_error"),search(lcontrol,"lease_h_error"),search(lcontrol,"lease_m_error"),search(lcontrol,"lease_empty"),search(lcontrol,"lease_max"),search(lpublic,"input_overflow"),search(lcontrol,"dhcp_ranges_empty"),search(lcontrol,"dhcp_subnet_fail"),search(lcontrol,"dhcp_endbeg_start"),search(lcontrol,"dhcp_format_err"));
	  fprintf(cgiOut,"</script>\n"\
	  "<script language=javascript src=/ip.js>\n"\
	  "</script>\n"\
	  "</head>\n"\
	  "<body onLoad=\"hexen();\">");
  

	  if(cgiFormSubmitClicked("dhcp_add") == cgiFormSuccess)
	  {	  	    
	    ConfIPAdd(lcontrol,lpublic,eid);		
	  }
	  /////////////////dcli_dhcp
	  cflag=ccgi_show_ip_pool(mode, index, &head, &count,allslot_id);
	  ///////////////////////////
		if(cflag==1)
		{
			q = head.next;
			while( q != NULL )		
			{
				pq = q->sub_show.next;
				while( pq != NULL )
				{
					if(strcmp(eid,q->poolname)!=0)
					{
						break;
					}
					else
					{

						if (q->domainname) 
						{
							memset(domname,0,50);
							strcpy(domname,q->domainname); 	
						}
						if (q->option43) 
						{
							//vty_out(vty, "dhcp server option43 is : %s\n", poolshow[j].option_show.option43); 	
							memset(optypez,0,512);
							strcpy(optypez,q->option43);
						}
						lease_t = (q->defaulttime ? q->defaulttime : 86400);

						memset(tip1,0,32);
						memset(tip2,0,32);
						memset(tip3,0,32);
						if(q->dns[0]!=0)
						ip_long2string(q->dns[0], tip1);
						if(q->dns[1]!=0)
						ip_long2string(q->dns[1], tip2);
						if(q->dns[2]!=0)
						ip_long2string(q->dns[2], tip3);

						memset(tmp2, 0, 32);
						strcpy(routeip, q->routers ? ip_long2string(q->routers, tmp2) : "0.0.0.0");	
						memset(tmp2, 0, 32);
						strcpy(winip, q->wins? ip_long2string(q->wins, tmp2) : "0.0.0.0");

						memset(dhcpmask,0,32);
						memset(tmp2,0,32);
						strcpy(dhcpmask,pq->mask ? ip_long2string(pq->mask, tmp2) : "0.0.0.0");
						memset(startip,0,32);
						memset(tmp2,0,32);
						memset(siptmp,0,32);
						strcpy(startip,pq->iplow ? ip_long2string(pq->iplow, tmp2) : "0.0.0.0");
						strcpy(siptmp,startip);
						memset(endip,0,32);
						memset(tmp2,0,32);
						memset(eiptmp,0,32);
						strcpy(endip, pq->iphigh ? ip_long2string(pq->iphigh, tmp2) : "0.0.0.0");
						strcpy(eiptmp,endip);
					}
					pq = pq->next;
				}
				q = q->next;
			}	

		}
		    if( cflag == 1 )
			{
				Free_show_dhcp_pool(&head);
			}
/////////// startip 
memset(st_ip1,0,4);
memset(st_ip2,0,4);
memset(st_ip3,0,4);
memset(st_ip4,0,4);
if(strcmp(startip,"")!=0)
{
  i=0;
  tmp = NULL;
  tmp = strtok(startip,".");
  while(tmp != NULL)
  	{
  		i++;
		if(i==1)
			strcpy(st_ip1,tmp);
		else if(i ==2 )
			strcpy(st_ip2,tmp);
		else if(i==3)
			strcpy(st_ip3,tmp);
		else if(i==4)
			strncpy(st_ip4,tmp,sizeof(st_ip4)-1);
		tmp = strtok(NULL,".");	
  	}
 
}
/////////// endip 
memset(end_ip1,0,4);
memset(end_ip2,0,4);
memset(end_ip3,0,4);
memset(end_ip4,0,4);
if(strcmp(endip,"")!=0)
{
  i=0;
  tmp = NULL;
  tmp = strtok(endip,".");
  while(tmp != NULL)
  	{
  		i++;
		if(i==1)
			strcpy(end_ip1,tmp);
		else if(i ==2 )
			strcpy(end_ip2,tmp);
		else if(i==3)
			strcpy(end_ip3,tmp);
		else if(i==4)
			strncpy(end_ip4,tmp,sizeof(end_ip4)-1);
		tmp = strtok(NULL,".");	
		
  	}
 
}
//////netmask
memset(mask_ip1,0,4);
memset(mask_ip2,0,4);
memset(mask_ip3,0,4);
memset(mask_ip4,0,4);
if(strcmp(dhcpmask,"")!=0)
{
  i = 0;
  tmp = NULL;
  tmp = strtok(dhcpmask,".");
  while(tmp != NULL)
  	{
  		i++;
		if(i==1)
			strcpy(mask_ip1,tmp);
		else if(i ==2 )
			strcpy(mask_ip2,tmp);
		else if(i==3)
			strcpy(mask_ip3,tmp);
		else if(i==4)
			strncpy(mask_ip4,tmp,sizeof(mask_ip4)-1);
		tmp = strtok(NULL,".");	
  	}
}
////lease  
		ttmp = lease_t/86400;
		sprintf(day,"%d",ttmp);
		ttmp = lease_t%86400;
		ttmp = ttmp/3600;
		sprintf(hour,"%d",ttmp);
		ttmp = lease_t%3600;
		ttmp = ttmp/60;
		sprintf(min,"%d",ttmp);
        

///routeip
	memset(route1,0,4);
	memset(route2,0,4);
	memset(route3,0,4);
	memset(route4,0,4);
   if(strcmp(routeip,""))
	{
	    i = 0;
  		tmp = NULL;
		tmp = strtok(routeip,".");
		while(tmp != NULL)
		{
			i++;
			if(i==1)
			{
				strcpy(route1,tmp);
			}
			else if(i==2)
			{
				strcpy(route2,tmp);
			}
			else if(i==3)
			{
				strcpy(route3,tmp);
			}
			else if(i==4)
			{
				strncpy(route4,tmp,sizeof(route4)-1);
				
			}
			tmp = strtok(NULL,".");	
		}
	}
	
///winip
	memset(win1,0,4);
	memset(win2,0,4);
	memset(win3,0,4);
	memset(win4,0,4);
   if(strcmp(winip,""))
	{
	    i = 0;
	  	tmp = NULL;
		tmp = strtok(winip,".");
		while(tmp != NULL)
		{
			i++;
			if(i==1)
			{
				strcpy(win1,tmp);
			}
			else if(i==2)
			{
				strcpy(win2,tmp);
			}
			else if(i==3)
			{
				strcpy(win3,tmp);
			}
			else if(i==4)
			{
				strncpy(win4,tmp,sizeof(win4)-1);
			}
			tmp = strtok(NULL,".");	
		}
	}

  fprintf(cgiOut,"<form method=post onsubmit=\"return mysubmit()\">"\
  "<div align=center>"\
  "<table width=976 border=0 cellpadding=0 cellspacing=0>"\
  "<tr>"\
    "<td width=8 align=left valign=top background=/images/di22.jpg><img src=/images/youce4.jpg width=8 height=30/></td>"\
    "<td width=51 align=left valign=bottom background=/images/di22.jpg><img src=/images/youce33.jpg width=37 height=24/></td>"\
    "<td width=153 align=left valign=bottom id=%s background=/images/di22.jpg>%s</td>",search(lpublic,"title_style"),"DHCP");
    fprintf(cgiOut,"<td width=690 align=right valign=bottom background=/images/di22.jpg>");
    fprintf(cgiOut,"<table width=130 border=0 cellspacing=0 cellpadding=0>"\
    "<tr>");
    fprintf(cgiOut,"<td width=62 align=center><input id=but type=submit name=dhcp_add style=background-image:url(/images/%s) value=""></td>",search(lpublic,"img_ok"));
    fprintf(cgiOut,"<td width=62 align=center><a href=wp_dhcpcon.cgi?UN=%s target=mainFrame><img src=/images/%s border=0 width=62 height=20/></a></td>",encry,search(lpublic,"img_cancel"));
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
         		if(cgiFormSubmitClicked("dhcp_add") != cgiFormSuccess)
         		{ 		  
					fprintf(cgiOut,"<tr height=26>"\
         			"<td align=left id=tdleft background=/images/bottom_bg.gif style=\"border-right:0\"><font id=%s>%s<font></td>",search(lpublic,"menu_san"),search(lcontrol,"edit"));   /*突出显示*/
         			fprintf(cgiOut,"</tr>");	
         		}
         		else if(cgiFormSubmitClicked("dhcp_add") == cgiFormSuccess) 			  
         		{					   
					fprintf(cgiOut,"<tr height=26>"\
         			"<td align=left id=tdleft background=/images/bottom_bg.gif style=\"border-right:0\"><font id=%s>%s<font></td>",search(lpublic,"menu_san"),search(lcontrol,"edit"));   /*突出显示*/
					fprintf(cgiOut,"</tr>");
         		}
				for(i=0;i<32;i++)
				{
					fprintf(cgiOut,"<tr height=25>"\
					  "<td id=tdleft>&nbsp;</td>"\
					"</tr>");
				}
				fprintf(cgiOut,"</table>"\
				"</td>"\
				"<td align=left valign=top style=\"background-color:#ffffff; border-right:1px solid #707070; padding-left:30px; padding-top:10px\">");

				fprintf(cgiOut,"<table width=500 border=0 cellspacing=0 cellpadding=0>");
				fprintf(cgiOut,"<tr>");
				fprintf(cgiOut,"<td>%s&nbsp;&nbsp;</td>","Slot ID:");
				fprintf(cgiOut,"<td colspan=4>%d</td>",allslot_id);
				fprintf(cgiOut,"</tr>");
				free_instance_parameter_list(&paraHead2);	
				fprintf(cgiOut,"<input type=hidden name=allslotid value=\"%d\">",allslot_id);
				
				fprintf( cgiOut,"<tr><td colspan=5>DHCP %s</td></tr>\n",search(lpublic,"dhcp_global"));
				fprintf( cgiOut, "<tr><td colspan=5><hr width=100%% size=1 color=#fff align=center noshade /></td></tr>");
				///////////////////////////////////////////////
				ccgi_show_ip_dhcp_server(&global_show,allslot_id);
				fprintf( cgiOut,"<tr><td>%s</td>",search(lpublic,"dhcp_change"));
				fprintf( cgiOut,"<td colspan=2><input type=radio name=globalc value=\"1\">Yes</td>");
				fprintf( cgiOut,"<td colspan=2><input type=radio name=globalc value=\"2\" checked>No</td>");
				fprintf( cgiOut,"</tr>\n");

	            fprintf( cgiOut,"<tr><td>Static-arp %s</td>",search(lcontrol,"Status"));
				if(global_show.staticarp==1)
				{
					fprintf( cgiOut,"<td colspan=2><input type=radio name=showarp value=\"1\" checked>Enable</td>");
					fprintf( cgiOut,"<td colspan=2><input type=radio name=showarp value=\"2\" >Disable</td>\n");
				}
				else
				{
					fprintf( cgiOut,"<td colspan=2><input type=radio name=showarp value=\"1\">Enable</td>");
					fprintf( cgiOut,"<td colspan=2><input type=radio name=showarp value=\"2\" checked>Disable</td>\n");
				}
				fprintf( cgiOut,"</tr>\n");

				fprintf( cgiOut,"<tr>\n");
				fprintf( cgiOut,"<td width=100 height=40 >%s:</td>","Global domain");
				fprintf( cgiOut,"<td colspan=4><input type=text name=gdomain value=\"%s\"></td>\n",global_show.domainname);
				fprintf( cgiOut,"</tr>\n");
				
				fprintf(cgiOut,"<tr>"\
 					"<td width=100 height=40>Global %s:</td>",search(lcontrol,"dnserver"));
	 			fprintf(cgiOut,"<td width=140>"\
	 						"<div style=\"border-width:1;border-color:#a5acb2;border-style:solid;width:140;font-size:9pt\">");
		  		fprintf(cgiOut,"<input type=text  name=gdns_ip1 value=\"\" maxlength=3 class=a3 onKeyUp=\"mask(this,%s)\" onbeforepaste=mask_c()>.",search(lpublic,"ip_error")); 
		  	    fprintf(cgiOut,"<input type=text  name=gdns_ip2 value=\"\" maxlength=3 class=a3 onKeyUp=\"mask(this,%s)\" onbeforepaste=mask_c()>.",search(lpublic,"ip_error"));
	 		    fprintf(cgiOut,"<input type=text  name=gdns_ip3 value=\"\" maxlength=3 class=a3 onKeyUp=\"mask(this,%s)\" onbeforepaste=mask_c()>.",search(lpublic,"ip_error"));
			    fprintf(cgiOut,"<input type=text  name=gdns_ip4 value=\"\" maxlength=3 class=a3 onKeyUp=\"mask(this,%s)\" onbeforepaste=mask_c()>",search(lpublic,"ip_error"));
		  	    fprintf(cgiOut,"</div></td>"\
					"<td>&nbsp;</td>"\
					"<td colspan=2 width=50 ><input type=button name=gdns_add id=ranges_add onclick=gadd_dns() style=\"width:50px\" value=\"%s\"></input></td>", search(lcontrol, "dhcp_add"));
	  			fprintf(cgiOut,"</tr>");
				
				fprintf(cgiOut,"<tr height=60>"\
							"<td>&nbsp;</td>"\
							"<td width=140 colspan=2 ><select name=gdnselect  valus=\"\" size=\"3\" style=\"width:140px\">");

                memset(gdns1,0,32);
                memset(gdns2,0,32);
                memset(gdns3,0,32);
				ip_long2string(global_show.dns[0], gdns1);
				ip_long2string(global_show.dns[1], gdns2);
				ip_long2string(global_show.dns[2], gdns3);

				if((strcmp(gdns1,"")!=0)&&(global_show.dns[0] !=0))
	   			{						
					fprintf(cgiOut,"<option value=\"%s\">%s</option>",gdns1,gdns1);						
				}
				if((strcmp(gdns2,"")!=0)&&(global_show.dns[1]!=0))
	   			{						
					fprintf(cgiOut,"<option value=\"%s\">%s</option>",gdns2,gdns2);						
				}

				if((strcmp(gdns3,"")!=0)&&(global_show.dns[2]!=0))
	   			{						
					fprintf(cgiOut,"<option value=\"%s\">%s</option>",gdns3,gdns3);						
				}
				fprintf(cgiOut,"</select></td>"\
							"<td width=50 colspan=2 valign=top><input type=button name=gdns_rm id=ranges_rm onclick=grm_dns() style=\"width:50px\" value=\"%s\"></input></td>", search(lcontrol, "dhcp_rm"));
				fprintf(cgiOut,"</tr>"); 		
				/////////router ip 				
				memset(groute,0,32);
				ip_long2string(global_show.routers, groute);
				if(strcmp(groute,"")!=0)
				{
					i=0;
					tmp = NULL;
					tmp = strtok(groute,".");
					while(tmp != NULL)
					{
						i++;
						if(i==1)
							strcpy(grip1,tmp);
						else if(i ==2 )
							strcpy(grip2,tmp);
						else if(i==3)
							strcpy(grip3,tmp);
						else if(i==4)
							strcpy(grip4,tmp);
						tmp = strtok(NULL,".");	
					}				 
				}				
				fprintf(cgiOut,"<tr>"\
	 					"<td width=100 height=40>%s:</td>","Global Router IP");
	 			fprintf(cgiOut,"<td  width=140>"\
	 						"<div style=\"border-width:1;border-color:#a5acb2;border-style:solid;width:140;font-size:9pt\">");
		  		fprintf(cgiOut,"<input type=text  name=groute_ip1 value=\"%s\" maxlength=3 class=a3 onKeyUp=\"mask(this,%s)\" onbeforepaste=mask_c()>.",grip1,search(lpublic,"ip_error")); 
		  	    fprintf(cgiOut,"<input type=text  name=groute_ip2 value=\"%s\" maxlength=3 class=a3 onKeyUp=\"mask(this,%s)\" onbeforepaste=mask_c()>.",grip2,search(lpublic,"ip_error"));
	 		    fprintf(cgiOut,"<input type=text  name=groute_ip3 value=\"%s\" maxlength=3 class=a3 onKeyUp=\"mask(this,%s)\" onbeforepaste=mask_c()>.",grip3,search(lpublic,"ip_error"));
			    fprintf(cgiOut,"<input type=text  name=groute_ip4 value=\"%s\" maxlength=3 class=a3 onKeyUp=\"mask(this,%s)\" onbeforepaste=mask_c()>",grip4,search(lpublic,"ip_error"));
		  	    fprintf(cgiOut,"</div></td>"\
							"<td>&nbsp;</td>"\
							"<td colspan=2 width=50></td>");
	  			fprintf(cgiOut,"</tr>");
				////win ip 
				memset(gwin,0,32);
				ip_long2string(global_show.wins, gwin);
				if(strcmp(gwin,"")!=0)
				{
					i=0;
					tmp = NULL;
					tmp = strtok(gwin,".");
					while(tmp != NULL)
					{
						i++;
						if(i==1)
							strcpy(gwip1,tmp);
						else if(i ==2 )
							strcpy(gwip2,tmp);
						else if(i==3)
							strcpy(gwip3,tmp);
						else if(i==4)
							strcpy(gwip4,tmp);
						tmp = strtok(NULL,".");	
					}				 
				}				
				fprintf(cgiOut,"<tr>"\
	 					"<td width=100 height=40>%s:</td>","Global Winip IP");
	 			fprintf(cgiOut,"<td  width=140>"\
	 						"<div style=\"border-width:1;border-color:#a5acb2;border-style:solid;width:140;font-size:9pt\">");
		  		fprintf(cgiOut,"<input type=text  name=gwin_ip1 value=\"%s\" maxlength=3 class=a3 onKeyUp=\"mask(this,%s)\" onbeforepaste=mask_c()>.",gwip1,search(lpublic,"ip_error")); 
		  	    fprintf(cgiOut,"<input type=text  name=gwin_ip2 value=\"%s\" maxlength=3 class=a3 onKeyUp=\"mask(this,%s)\" onbeforepaste=mask_c()>.",gwip2,search(lpublic,"ip_error"));
	 		    fprintf(cgiOut,"<input type=text  name=gwin_ip3 value=\"%s\" maxlength=3 class=a3 onKeyUp=\"mask(this,%s)\" onbeforepaste=mask_c()>.",gwip3,search(lpublic,"ip_error"));
			    fprintf(cgiOut,"<input type=text  name=gwin_ip4 value=\"%s\" maxlength=3 class=a3 onKeyUp=\"mask(this,%s)\" onbeforepaste=mask_c()>",gwip4,search(lpublic,"ip_error"));
		  	    fprintf(cgiOut,"</div></td>"\
							"<td>&nbsp;</td>"\
							"<td colspan=2 width=50></td>");
	  			fprintf(cgiOut,"</tr>");

				fprintf( cgiOut,"<tr>\n");
				fprintf( cgiOut,"<td width=100 height=40 >%s:</td>","Global lease");
				fprintf( cgiOut,"<td colspan=4><input type=text name=glease value=\"%d\"><font color=red>(60-31536000)</font></td>\n",global_show.defaulttime ? global_show.defaulttime : 86400);
				fprintf( cgiOut,"</tr>\n");
				/////////////////////////////////////////////////
				fprintf( cgiOut, "<tr height=10><td colspan=5></td></tr>");

				fprintf( cgiOut,"<tr><td colspan=5>DHCP %s</td></tr>\n",search(lpublic,"dhcp_pconfig"));
				fprintf( cgiOut, "<tr><td colspan=5><hr width=100%% size=1 color=#fff align=center noshade /></td></tr>");

				////////////////pool name
	            fprintf(cgiOut,"<tr>\n");
				fprintf(cgiOut,"<td width=100 height=40 >%s:</td>","POOL");
				fprintf(cgiOut,"<td>%s</td></tr>\n",eid);

				//domain name
	            fprintf(cgiOut,"<tr>\n");
				fprintf(cgiOut,"<td width=100 height=40 >%s:</td>","Domain");
				fprintf(cgiOut,"<td><input type=text name=domname value=\"%s\" size=21 maxLength=20></td></tr>\n",domname);

				fprintf(cgiOut,"<tr>"\
				 					"<td width=100 height=40 >%s:</td>",search(lcontrol,"lease duration"));
				fprintf(cgiOut,"<td colspan=4 width=350 align=left><input type=text name=day onkeypress=\"return event.keyCode>=48&&event.keyCode<=57\"\
									onpaste=\"var s=clipboardData.getData('text');   if(!/\\D/.test(s)) value=s.replace(/^0*/,'');   return   false;\"\
									ondragenter=\"return  false;\"\
									style=\"ime-mode:disabled\" onkeyup=\"if(/(^0+)/.test(value))value=value.replace(/^0*/,'')\" value=\"%s\" size=5/><font color=red>(%s)</font>",day,search(lcontrol,"day"));
				fprintf(cgiOut,"<input type=text name=hour onkeypress=\"return event.keyCode>=48&&event.keyCode<=57\"\
									onpaste=\"var s=clipboardData.getData('text');   if(!/\\D/.test(s)) value=s.replace(/^0*/,'');   return   false;\"\
									ondragenter=\"return  false;\"\
									style=\"ime-mode:disabled\" onkeyup=\"if(/(^0+)/.test(value))value=value.replace(/^0*/,'')\" value=\"%s\" size=5/><font color=red>(%s)</font>",hour,search(lcontrol,"hour"));
				fprintf(cgiOut,"<input type=text name=min onkeypress=\"return event.keyCode>=48&&event.keyCode<=57\"\
									onpaste=\"var s=clipboardData.getData('text');   if(!/\\D/.test(s)) value=s.replace(/^0*/,'');   return   false;\"\
									ondragenter=\"return  false;\"\
									style=\"ime-mode:disabled\" onkeyup=\"if(/(^0+)/.test(value))value=value.replace(/^0*/,'')\" value=\"%s\" size=5/><font color=red>(%s)</font></td>",min,search(lcontrol,"time_min"));
	 			fprintf(cgiOut,"</tr>");				
				
	 			fprintf(cgiOut,"<tr>"\
 					"<td width=100 height=40>%s:</td>",search(lcontrol,"dnserver"));
	 			fprintf(cgiOut,"<td width=140>"\
	 						"<div style=\"border-width:1;border-color:#a5acb2;border-style:solid;width:140;font-size:9pt\">");
		  		fprintf(cgiOut,"<input type=text  name=dns_ip1 value=\"\" maxlength=3 class=a3 onKeyUp=\"mask(this,%s)\" onbeforepaste=mask_c()>.",search(lpublic,"ip_error")); 
		  	    fprintf(cgiOut,"<input type=text  name=dns_ip2 value=\"\" maxlength=3 class=a3 onKeyUp=\"mask(this,%s)\" onbeforepaste=mask_c()>.",search(lpublic,"ip_error"));
	 		    fprintf(cgiOut,"<input type=text  name=dns_ip3 value=\"\" maxlength=3 class=a3 onKeyUp=\"mask(this,%s)\" onbeforepaste=mask_c()>.",search(lpublic,"ip_error"));
			    fprintf(cgiOut,"<input type=text  name=dns_ip4 value=\"\" maxlength=3 class=a3 onKeyUp=\"mask(this,%s)\" onbeforepaste=mask_c()>",search(lpublic,"ip_error"));
		  	    fprintf(cgiOut,"</div></td>"\
					"<td>&nbsp;</td>"\
					"<td colspan=2 width=50 ><input type=button name=dns_add id=ranges_add onclick=add_dns() style=\"width:50px\" value=\"%s\"></input></td>", search(lcontrol, "dhcp_add"));
	  			fprintf(cgiOut,"</tr>");
				
				fprintf(cgiOut,"<tr height=60>"\
							"<td>&nbsp;</td>"\
							"<td width=140 colspan=2 ><select name=dnselect  valus=\"\" size=\"3\" style=\"width:140px\">");
				if(strcmp(tip1,"")!=0)
	   			{						
					fprintf(cgiOut,"<option value=\"%s\">%s</option>",tip1,tip1);						
				}
				if(strcmp(tip2,"")!=0)
	   			{						
					fprintf(cgiOut,"<option value=\"%s\">%s</option>",tip2,tip2);						
				}

				if(strcmp(tip3,"")!=0)
	   			{						
					fprintf(cgiOut,"<option value=\"%s\">%s</option>",tip3,tip3);						
				}
				fprintf(cgiOut,"</select></td>"\
							"<td width=50 colspan=2 valign=top><input type=button name=dns_rm id=ranges_rm onclick=rm_dns() style=\"width:50px\" value=\"%s\"></input></td>", search(lcontrol, "dhcp_rm"));
				fprintf(cgiOut,"</tr>"); 		
				/////////router ip 
				fprintf(cgiOut,"<tr>"\
	 					"<td width=100 height=40>%s:</td>","Router IP");
	 			fprintf(cgiOut,"<td  width=140>"\
	 						"<div style=\"border-width:1;border-color:#a5acb2;border-style:solid;width:140;font-size:9pt\">");
		  		fprintf(cgiOut,"<input type=text  name=route_ip1 value=\"%s\" maxlength=3 class=a3 onKeyUp=\"mask(this,%s)\" onbeforepaste=mask_c()>.",route1,search(lpublic,"ip_error")); 
		  	    fprintf(cgiOut,"<input type=text  name=route_ip2 value=\"%s\" maxlength=3 class=a3 onKeyUp=\"mask(this,%s)\" onbeforepaste=mask_c()>.",route2,search(lpublic,"ip_error"));
	 		    fprintf(cgiOut,"<input type=text  name=route_ip3 value=\"%s\" maxlength=3 class=a3 onKeyUp=\"mask(this,%s)\" onbeforepaste=mask_c()>.",route3,search(lpublic,"ip_error"));
			    fprintf(cgiOut,"<input type=text  name=route_ip4 value=\"%s\" maxlength=3 class=a3 onKeyUp=\"mask(this,%s)\" onbeforepaste=mask_c()>",route4,search(lpublic,"ip_error"));
		  	    fprintf(cgiOut,"</div></td>"\
							"<td>&nbsp;</td>"\
							"<td colspan=2 width=50></td>");
	  			fprintf(cgiOut,"</tr>");
				////win ip 
				fprintf(cgiOut,"<tr>"\
	 					"<td width=100 height=40>%s:</td>","Winip IP");
	 			fprintf(cgiOut,"<td  width=140>"\
	 						"<div style=\"border-width:1;border-color:#a5acb2;border-style:solid;width:140;font-size:9pt\">");
		  		fprintf(cgiOut,"<input type=text  name=win_ip1 value=\"%s\" maxlength=3 class=a3 onKeyUp=\"mask(this,%s)\" onbeforepaste=mask_c()>.",win1,search(lpublic,"ip_error")); 
		  	    fprintf(cgiOut,"<input type=text  name=win_ip2 value=\"%s\" maxlength=3 class=a3 onKeyUp=\"mask(this,%s)\" onbeforepaste=mask_c()>.",win2,search(lpublic,"ip_error"));
	 		    fprintf(cgiOut,"<input type=text  name=win_ip3 value=\"%s\" maxlength=3 class=a3 onKeyUp=\"mask(this,%s)\" onbeforepaste=mask_c()>.",win3,search(lpublic,"ip_error"));
			    fprintf(cgiOut,"<input type=text  name=win_ip4 value=\"%s\" maxlength=3 class=a3 onKeyUp=\"mask(this,%s)\" onbeforepaste=mask_c()>",win4,search(lpublic,"ip_error"));
		  	    fprintf(cgiOut,"</div></td>"\
							"<td>&nbsp;</td>"\
							"<td colspan=2 width=50></td>");
	  			fprintf(cgiOut,"</tr>");
				//////////option43 			
				fprintf(cgiOut,"<tr height=30><td width=80>option43 select:</td>\n");
				fprintf(cgiOut,"<td><input type=radio name=showtype value=\"1\" checked onclick=\"hexen();\">\n");
				fprintf(cgiOut,"<input type=radio name=showtype value=\"2\" onclick=\"opten();\"></td>");
				fprintf(cgiOut,"</tr>\n");

				fprintf(cgiOut,"<tr height=30 id=hex1>\n");
				fprintf(cgiOut,"<td width=80>\n");
				fprintf(cgiOut,"option43:");
				fprintf(cgiOut,"</td>\n");
				fprintf(cgiOut,"<td colspan=4>\n");
				fprintf(cgiOut,"<input type=text name=optype value=\"%s\" style=\"width:140;\"/><font color=red>(xx-%sxx-Numberxxxx-IP|120401010101)</font>",optypez,search(lpublic,"l_type"));
				fprintf(cgiOut,"</td>\n");
				fprintf(cgiOut,"</tr>\n");	
				
			    fprintf(cgiOut,"<tr height=30 id=hex2>\n");
				fprintf(cgiOut,"<td width=80>\n");
				fprintf(cgiOut,"option43:");
				fprintf(cgiOut,"</td>\n");
				fprintf(cgiOut,"<td width=140>\n"\
						"<div style=\"border-width:1;border-color:#a5acb2;border-style:solid;width:140;font-size:9pt\">");
				fprintf(cgiOut,"<input type=text  name=opt_ip1 value=\"\" maxlength=3 class=a3 onKeyUp=\"mask(this,%s)\" onbeforepaste=mask_c()>.",search(lpublic,"ip_error")); 
	  	        fprintf(cgiOut,"<input type=text  name=opt_ip2 value=\"\" maxlength=3 class=a3 onKeyUp=\"mask(this,%s)\" onbeforepaste=mask_c()>.",search(lpublic,"ip_error"));
 		        fprintf(cgiOut,"<input type=text  name=opt_ip3 value=\"\" maxlength=3 class=a3 onKeyUp=\"mask(this,%s)\" onbeforepaste=mask_c()>.",search(lpublic,"ip_error"));
		        fprintf(cgiOut,"<input type=text  name=opt_ip4 value=\"\" maxlength=3 class=a3 onKeyUp=\"mask(this,%s)\" onbeforepaste=mask_c()>",search(lpublic,"ip_error"));
	  	        fprintf(cgiOut,"</div></td>");
				fprintf(cgiOut,"<td>&nbsp;</td>");
				fprintf(cgiOut,"<td colspan=2 width=50 ><input type=button name=optid_add id=ranges_add onclick=add_opt() style=\"width:50px\" value=\"%s\"></input></td>", search(lcontrol, "dhcp_add"));
				fprintf(cgiOut,"</tr>\n");
				
				fprintf(cgiOut,"<tr height=60 id=hex3>\n"\
					"<td>&nbsp;</td>\n"\
					"<td width=140 colspan=2 ><select name=optselect  valus=\"\" size=\"6\" style=\"width:140px\">\n");
				///////////////				
				char len[3] = {0}, optio43_list[16] = {0};
				unsigned char ip[9], ip_list[16] = {0};
				int j = 0; 
				int ipnum = 0;
				char* endstr = NULL;
				unsigned int ip43 = 0;
				if (0 != strcmp(optypez,"")) 
				{
					memcpy(len, &(optypez[2]), 2);
					ipnum = (strtoul(len, &endstr, 16)) / 4;			

					for(j = 0; j < ipnum; ++j)
					{
						memset(ip, 0, 9);				
						memcpy(ip, &(optypez[4 + j*8]), 8);
						ip43 = strtoul(ip, &endstr, 16);

						ip_list[0] = ip43>>24 & 0xff;
						ip_list[1] = ip43>>16 & 0xff;
						ip_list[2] = ip43>>8 & 0xff;
						ip_list[3] = ip43 & 0xff;
						inet_ntop(AF_INET, ip_list, optio43_list, sizeof(optio43_list));
						fprintf(cgiOut,"<option value=\"%s\">%s</option>",optio43_list,optio43_list);						
						memset(optio43_list, 0, 16);
					}
				}
				//////////////
				fprintf(cgiOut,"</select></td>"\
					"<td width=50 colspan=2 valign=top><input type=button name=opt_rm id=ranges_rm onclick=rm_opt() style=\"width:50px\" value=\"%s\"></input></td>", search(lcontrol, "dhcp_rm"));
				fprintf(cgiOut,"</tr>");
				

				fprintf(cgiOut,"<tr>"\
	    				"<td width=100 height=40>%s:</td>",search(lcontrol,"dhcp_sub_mask"));
				fprintf(cgiOut,"<td width=140>"\
								"<div style=\"border-width:1;border-color:#a5acb2;border-style:solid;width:140;font-size:9pt\">");
		  		fprintf(cgiOut,"<input type=text  name=mask_ip1 value=\"%s\" maxlength=3 class=a3 disabled onKeyUp=\"mask(this,%s)\" onbeforepaste=mask_c()>.",mask_ip1,search(lpublic,"ip_error")); 
		  	    fprintf(cgiOut,"<input type=text  name=mask_ip2 value=\"%s\" maxlength=3 class=a3 disabled onKeyUp=\"mask(this,%s)\" onbeforepaste=mask_c()>.",mask_ip2,search(lpublic,"ip_error"));
	 		    fprintf(cgiOut,"<input type=text  name=mask_ip3 value=\"%s\" maxlength=3 class=a3 disabled onKeyUp=\"mask(this,%s)\" onbeforepaste=mask_c()>.",mask_ip3,search(lpublic,"ip_error"));
			    fprintf(cgiOut,"<input type=text  name=mask_ip4 value=\"%s\" maxlength=3 class=a3 disabled onKeyUp=\"mask(this,%s)\" onbeforepaste=mask_c()>",mask_ip4,search(lpublic,"ip_error"));
		  	    fprintf(cgiOut,"</div></td>"\
					"<td width=10>&nbsp;</td>"\
					"<td width=140>&nbsp;</td>"\
					"<td width=50>&nbsp;</td>"\
					"</tr>"\
					"<tr>"\
					"<td width=100 height=40>%s:</td>",search(lcontrol,"dhcp_pool"));
	 			fprintf(cgiOut,"<td width=140>"\
	 							"<div style=\"border-width:1;border-color:#a5acb2;border-style:solid;width:140;font-size:9pt\">");
		  		fprintf(cgiOut,"<input type=text  name=beg_ip1 value=\"%s\" maxlength=3 class=a3 onKeyUp=\"mask(this,%s)\" onbeforepaste=mask_c()>.",st_ip1,search(lpublic,"ip_error")); 
		  	    fprintf(cgiOut,"<input type=text  name=beg_ip2 value=\"%s\" maxlength=3 class=a3 onKeyUp=\"mask(this,%s)\" onbeforepaste=mask_c()>.",st_ip2,search(lpublic,"ip_error"));
	 		    fprintf(cgiOut,"<input type=text  name=beg_ip3 value=\"%s\" maxlength=3 class=a3 onKeyUp=\"mask(this,%s)\" onbeforepaste=mask_c()>.",st_ip3,search(lpublic,"ip_error"));
				int poolstart = 0;
				poolstart = strtoul(st_ip4,0,10);
				fprintf(cgiOut,"<input type=text  name=beg_ip4 value=%d maxlength=3 class=a3 onKeyUp=\"mask(this,%s)\" onbeforepaste=mask_c()>",poolstart,search(lpublic,"ip_error"));
		  	    fprintf(cgiOut,"</div></td>"\
	 					"<td align=center>-</td>"\
						"<td width=150 colspan=2>"\
						"<div style=\"border-width:1;border-color:#a5acb2;border-style:solid;width:140;font-size:9pt\">");
		  		fprintf(cgiOut,"<input type=text  name=end_ip1 value=\"%s\" maxlength=3 class=a3 onKeyUp=\"mask(this,%s)\" onbeforepaste=mask_c()>.",end_ip1,search(lpublic,"ip_error")); 
		  	    fprintf(cgiOut,"<input type=text  name=end_ip2 value=\"%s\" maxlength=3 class=a3 onKeyUp=\"mask(this,%s)\" onbeforepaste=mask_c()>.",end_ip2,search(lpublic,"ip_error"));
				fprintf(cgiOut,"<input type=text  name=end_ip3 value=\"%s\" maxlength=3 class=a3 onKeyUp=\"mask(this,%s)\" onbeforepaste=mask_c()>.",end_ip3,search(lpublic,"ip_error"));
				int testnum = strtoul(end_ip4,0,10);
			    fprintf(cgiOut,"<input type=text  name=end_ip4 value=%d maxlength=3 class=a3 onKeyUp=\"mask(this,%s)\" onbeforepaste=mask_c()>",testnum,search(lpublic,"ip_error"));
		  	    fprintf(cgiOut,"</div></td>");			
				
				fprintf(cgiOut,"</tr>");	
				fprintf(cgiOut,"<tr>\n");
				fprintf(cgiOut,"<td><input type=hidden name=UN value=%s></td>",encry);	
				fprintf(cgiOut,"<td><input type=hidden name=NAME value=%s></td>",eid);
				fprintf(cgiOut,"<td><input type=hidden name=dns_ip value=%s></td>",dns);
				fprintf(cgiOut,"<td><input type=hidden name=routeip value=%s></td>",riptmp);
				fprintf(cgiOut,"<td><input type=hidden name=winip value=%s></td>",wiptmp);
				fprintf(cgiOut,"</tr>"\
					"<tr>"\
					"<td><input type=hidden name=ip_mask value=%s></td>",ip_mask);
				fprintf(cgiOut,"<td><input type=hidden name=startip value=%s></td>",sipdef);
				fprintf(cgiOut,"<td><input type=hidden name=endip value=%s></td>",eipdef);
				fprintf(cgiOut,"<td><input type=hidden name=defaultsip value=%s></td>",siptmp);
				fprintf(cgiOut,"<td><input type=hidden name=defaulteip value=%s></td>",eiptmp);
				fprintf(cgiOut,"</tr>\n");
				fprintf(cgiOut,"<tr>\n");
				memset(groute,0,32);
				memset(gwin,0,32);
				fprintf(cgiOut,"<td><input type=hidden name=grouteip value=%s></td>",groute);
				fprintf(cgiOut,"<td><input type=hidden name=gwinip value=%s></td>",gwin);
				fprintf(cgiOut,"<td><input type=hidden name=gdns_ip value=%s></td>",gdns);
				fprintf(cgiOut,"<input type=hidden name=opt_ip>");
				fprintf(cgiOut,"</tr>\n");
				fprintf(cgiOut,"</table>");		
				
fprintf(cgiOut,"</td>"\
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
free(dns);
free(tmp);
free(dhcpmask);
free(tmp1);
free(tmp2);
free(tmp3);
free(startip);
free(endip);
free(routeip);
free(domname);
free(tip1);
free(tip2);
free(tip3);
free(winip);
free(sipdef);
free(eipdef);
free(siptmp);
free(eiptmp);
free(ip_mask);
free(riptmp);
free(wiptmp);
free(optypez);
free(global_show.domainname);
free(global_show.option43);
free(glease);
free(gwin);
free(groute);
free(gdom);
free(gdns1);
free(gdns2);
free(gdns3);
free(gdns);

return 0;
}
						 
void  ConfIPAdd(struct list *lcontrol,struct list *lpublic,char *poolname)
{
	char lease[32] = {0};
	char day[10] = {0};
	char hour[10] = {0};
	char min[10] = {0};
	char dns_ip[128] = {0};
	char ip_mask[32] = {0};
	char startip[32] = {0};
	char endip[32] = {0};
	char domname[32] = {0};
	char routeip[32] = {0};
	char winip[32] = {0};
	char opthexip[512] = {0};
	char defsip[32] = {0};
	char defeip[32] = {0};
	
	char glease[32] = {0};
	char groute[32] = {0};
	char gdns[128] = {0};
	char gwin[32] = {0};
	char garp[10] = {0};
	char gdom[32] = {0};
	char globalc[10] = {0};
	char opt_ip[512] = {0};
	char showtype[10] = {0};
	int flag = 0;	
	char allslotid[10] = {0};
	int allslot_id = 0;
	cgiFormStringNoNewlines("allslotid",allslotid,sizeof(allslotid));
	allslot_id = atoi(allslotid);
	
	int ret1=-1,mode = 1,ret2 = 0,gmode = 0;
	unsigned int index = 0,gindex = 0;
	unsigned int isenable = 0;

	cgiFormStringNoNewlines("domname",domname,32);
	cgiFormStringNoNewlines("day",day,10);
	cgiFormStringNoNewlines("hour",hour,10);
	cgiFormStringNoNewlines("min",min,10);
	cgiFormStringNoNewlines("dns_ip",dns_ip,128);
	cgiFormStringNoNewlines("ip_mask",ip_mask,32);
	cgiFormStringNoNewlines("startip",startip,32);
	cgiFormStringNoNewlines("endip",endip,32);	
	cgiFormStringNoNewlines("defaultsip",defsip,32);
	cgiFormStringNoNewlines("defaulteip",defeip,32);	
	cgiFormStringNoNewlines("optype", opthexip, 512);	
	cgiFormStringNoNewlines("routeip",routeip,32);
	cgiFormStringNoNewlines("winip",winip,32);
	cgiFormStringNoNewlines("gwinip",gwin,32);
	cgiFormStringNoNewlines("grouteip",groute,32);
	cgiFormStringNoNewlines("gdns_ip",gdns,128);	
	cgiFormStringNoNewlines("glease",glease,32);
	cgiFormStringNoNewlines("gdomain",gdom,32);
	cgiFormStringNoNewlines("showarp",garp,10);
	cgiFormStringNoNewlines("globalc",globalc,10);
	cgiFormStringNoNewlines("opt_ip", opt_ip, 512);	
	cgiFormStringNoNewlines("showtype",showtype,10);
	
	int lease_t = 0;
	int lease_d = atoi(day);
	int lease_h = atoi(hour);
	int lease_m = atoi(min);
	lease_t = lease_d*24*3600+lease_h*3600+lease_m*60;
	sprintf(lease,"%d",lease_t);	

	ccgi_dbus_init();
	/*global dhcp config*/
	if(strcmp(globalc,"1")==0)
	{
		if(strcmp(gdom,"")!=0)
		{
			ip_dhcp_server_domain_name(gdom,gmode,gindex,allslot_id);
		}
		else
		{
			no_ip_dhcp_server_domain_name(gdom, gindex,gmode,allslot_id);
		}
		
		if(strcmp(gdns,"")!=0)
		{
			ip_dhcp_server_dns(gmode, gindex, gdns,allslot_id);
		}
		else
		{
			no_ip_dhcp_server_dns(gmode,gindex,allslot_id);
		}
		
		if(strcmp(groute,"")!=0)
		{
			ip_dhcp_server_routers_ip(groute, gindex,gmode,allslot_id);
		}
		else
		{
			no_ip_dhcp_server_routers_ip(gmode,gindex,allslot_id);
		}
		
		if(strcmp(gwin,"")!=0)
		{
			ip_dhcp_server_wins_ip(gwin,gmode, gindex,allslot_id);
		}
		else
		{
			no_ip_dhcp_server_wins_ip(gmode,gindex,allslot_id);
		}
		
		if(strcmp(glease,"")!=0)
		{
			ip_dhcp_server_lease_default(glease, gmode,gindex,allslot_id);
		}
		else
		{
			no_ip_dhcp_server_lease_default(gmode,gindex,allslot_id);
		}
		
		if(strcmp(garp,"1")==0)
		{
			isenable = 1;
		}
		else if(strcmp(garp,"2")==0)
		{
			isenable = 0;
		}
		ccgi_set_server_static_arp_enable(isenable,allslot_id);
	}
	
	/*ip pool name*/
	ret1=config_ip_pool_name(poolname, &index,allslot_id);
	if (1 == ret1)
	{
		/*modify ip pool range*/	
		if ((strcmp(defsip,startip) == 0) && (strcmp(defeip,endip) == 0))
		{
		}
		else
		{
			add_dhcp_pool_ip_range("delete", defsip,defeip, ip_mask,index,allslot_id);
			ret2=add_dhcp_pool_ip_range("add", startip,endip, ip_mask,index,allslot_id);
			if(ret2==1)
			{
				flag = 1;
				ShowAlert(search(lcontrol,"add_suc"));
			}
			else
			{
				ShowAlert(search(lcontrol,"oper_fail"));
			}
		}
		
		/*modify domain name*/	
		int domret = 0;
		if (strcmp(domname,"") != 0)
		{
			ip_dhcp_server_domain_name(domname,mode,index,allslot_id);
		}
		else
		{
			domret = no_ip_dhcp_server_domain_name(domname, index,mode,allslot_id);
		}
		/*modify dns*/	
		if(strcmp(dns_ip,"")!=0)
		{
			ip_dhcp_server_dns(mode, index, dns_ip,allslot_id);
		}
		else
		{
			no_ip_dhcp_server_dns(mode,index,allslot_id);
		}
		/*modify lease*/
		if (0 != lease_t)
		{
			ip_dhcp_server_lease_default(lease, mode,index,allslot_id);
		}
		else
		{
			no_ip_dhcp_server_lease_default(mode,index,allslot_id);
		}

		/*option 43*/		
		if (strcmp(showtype,"1") == 0)
		{
			if(strcmp(opthexip,"")!=0)
			{
				ccgi_set_server_no_option43(index,allslot_id);
				ip_dhcp_server_option43(opthexip, mode, index,allslot_id);
			}
			else
			{
				ccgi_set_server_no_option43(index,allslot_id);
			}
		}
		else
		{
			if(strcmp(opt_ip,"") != 0)
			{
				ccgi_set_server_no_option43(index,allslot_id);
				char *pl = NULL;
				pl = strtok(opt_ip," ");
				while (NULL != pl)
				{
					ccgi_set_server_option43_veo(pl, mode, index, 0,allslot_id);
					pl = strtok(NULL," ");
				}
			}
			else
			{
				ccgi_set_server_no_option43(index,allslot_id);
			}
		}
		/*modify route ip*/
		if(strcmp(routeip,"")!=0)
		{
			ip_dhcp_server_routers_ip(routeip, index,mode,allslot_id);
		}
		else
		{
			no_ip_dhcp_server_routers_ip(mode,index,allslot_id);
		}
		
		/*modify win ip*/
		if(strcmp(winip,"")!=0)
		{
			ip_dhcp_server_wins_ip(winip,mode, index,allslot_id);
		}
		else
		{
			no_ip_dhcp_server_wins_ip(mode,index,allslot_id);
		}
		
		if (0 == flag)
		{
			ShowAlert(search(lpublic,"oper_succ"));
		}
	}

}






int ShowConfClearPage(char *m,char *id,struct list *lcontrol,struct list *lpublic)
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
	char allslotid[10] = {0};
	int allslot_id = 0;
	cgiFormStringNoNewlines("allslotid",allslotid,sizeof(allslotid));
	allslot_id = atoi(allslotid);

		int ret=-1;		
		unsigned int index = 0;
		///////////write to conf
		ret = delete_ip_pool_name(id,&index,allslot_id);

		if( ret == 1 )
		   ShowAlert(search(lpublic,"oper_succ"));
		else
			ShowAlert(search(lpublic,"oper_fail"));

	    fprintf(cgiOut,"<input type=hidden name=poolname value=%s>",m);

		fprintf( cgiOut, "<script type='text/javascript'>\n" );
		fprintf( cgiOut, "window.location.href='wp_dhcpcon.cgi?UN=%s';\n", m);
		fprintf( cgiOut, "</script>\n" );	

		fprintf( cgiOut, "</body>\n" );
		fprintf( cgiOut, "</html>\n" );

	return 0;
}


