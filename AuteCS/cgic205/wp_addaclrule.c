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
* wp_addaclrule.c
*
*
* CREATOR:
* autelan.software.Network Dep. team
* zhouym@autelan.com
*
* DESCRIPTION:
* system contrl for add acl ruler
*
*
*******************************************************************************/
#include <stdio.h>
#include "cgic.h"
#include <string.h>
#include <stdlib.h>
#include "ws_usrinfo.h"
#include "ws_dcli_acl.h"
#include "ws_dcli_qos.h"
#include <sys/wait.h>
char *flavors[] = {
	"ip",
	"tcp",
	"udp",
	"icmp",
	"arp",
	"ethernet"
};
char *ActionType[] = {
	"permit",
	"deny",
	"trap_to_cpu",
	"redirect",
	"ingress-qos",
	"egress-qos"
};
char *RuleType[] = {
	"standard",
	"extended"
};

char *flavorsLater[] = {
	"ip",
	"tcp",
	"udp",
	"icmp",
	"arp",
	"ethernet"
};

#define AMOUNT 512

int ShowAddRulePage(); 
int checkvalid(char * slotport);

int cgiMain()
{
 ShowAddRulePage();
 return 0;
}

int ShowAddRulePage()
{
	struct list *lpublic;	/*解析public.txt文件的链表头*/
	struct list *lcontrol; 	/*解析help.txt文件的链表头*/
	struct list *lsecu;     /*解析security.txt文件的链表头*/  
	lpublic=get_chain_head("../htdocs/text/public.txt");
    lcontrol=get_chain_head("../htdocs/text/control.txt"); 
    lsecu=get_chain_head("../htdocs/text/security.txt");
	//FILE *fp;
	//char lan[3];
	char *encry=(char *)malloc(BUF_LEN);			  
	char *str;
	int i;
	char * select_a=(char *)malloc(10);
	char * Exselect_a=(char *)malloc(10);
	char * select_b=(char *)malloc(20);
	char * select_c=(char *)malloc(10);
	char * text_index=(char *)malloc(30);
	char * smasklength=(char *)malloc(10);
	char * dmasklength=(char *)malloc(10);
	memset(smasklength,0,10);
	memset(dmasklength,0,10);
	memset(encry,0,BUF_LEN);
	memset(select_a,0,10);
	memset(Exselect_a,0,10);
	memset(select_b,0,20);
	memset(select_c,0,10);
	memset(text_index,0,30);
	char slot[5],port[5];
	memset(slot,0,5);
	memset(port,0,5);
	char * slotportNo=(char *)malloc(10);
	memset(slotportNo,0,10);
	struct policer_info receive_policer[Policer_Num];
	int policer_num=0;
	char dip1[4],dip2[4],dip3[4],dip4[4];
	char * dstip=(char *)malloc(20);
	char sip1[4],sip2[4],sip3[4],sip4[4];
	char * srcip=(char *)malloc(20);
	memset(dstip,0,20);
	memset(srcip,0,20);
	char * srcPort=(char *)malloc(10);
	char * DstPort=(char *)malloc(10);
	memset(srcPort,0,10);
	memset(DstPort,0,10);
	char * icmp_type=(char *)malloc(10);
	char * icmp_code=(char *)malloc(10);
	memset(icmp_type,0,10);
	memset(icmp_code,0,10);
	char * srcmac=(char *)malloc(20);
	memset(srcmac,0,20);
	char * dstmac=(char *)malloc(20);
	memset(dstmac,0,20);
	char * vlanID=(char *)malloc(10);
	memset(vlanID,0,10);

	char * policyindex=(char *)malloc(10);
	memset(policyindex,0,10);
//***************new*****************
	struct qos_info receive_qos[MAX_QOS_PROFILE];
	for(i=0;i<MAX_QOS_PROFILE;i++)
  	{
  		receive_qos[i].profileindex=0;
  		receive_qos[i].dp=0;
  		receive_qos[i].up=0;
  		receive_qos[i].tc=0;
  		receive_qos[i].dscp=0;
  	}
	int qos_num=0;
	char * mode=(char *)malloc(AMOUNT);
	memset(mode,0,AMOUNT);
//***********************************
	for(i=0;i<Policer_Num;i++)
	{
	 	receive_policer[i].policer_index=0;
	  	receive_policer[i].policer_state=(char *)malloc(20);
	    	memset(receive_policer[i].policer_state,0,20);
	    	 
	    	receive_policer[i].CounterState=(char *)malloc(20);
	    	memset(receive_policer[i].CounterState,0,20);
	    	 
	    	receive_policer[i].Out_Profile_Action=(char *)malloc(20);
	    	memset(receive_policer[i].Out_Profile_Action,0,20);
	    	 
	    	receive_policer[i].Policer_Mode=(char *)malloc(20);
	    	memset(receive_policer[i].Policer_Mode,0,20);
	    	 
	    	receive_policer[i].Policing_Packet_Size=(char *)malloc(20);
	    	memset(receive_policer[i].Policing_Packet_Size,0,20);
	 }



	int showFlagL=0;
///////////////////any//////////////////////	
	char * sourceIP=(char *)malloc(10);
	memset(sourceIP,0,10);
	char * dstIP=(char *)malloc(10);
	memset(dstIP,0,10);
	char * sourcePort=(char *)malloc(10);
	memset(sourcePort,0,10);
	char * dstPort=(char *)malloc(10);
	memset(dstPort,0,10);
	char * srcMac=(char *)malloc(10);
	memset(srcMac,0,10);
	char * dstMac=(char *)malloc(10);
	memset(dstMac,0,10);
	
/////////////////////////////////////////	
	char * sourceslot=(char *)malloc(10);
	memset(sourceslot,0,10);
	char * sourceport=(char *)malloc(10);
	memset(sourceport,0,10);
	char * sourceslotport=(char *)malloc(10);
	memset(sourceslotport,0,10);
/////////////////////////add to group////////////	
	char * groupindex=(char *)malloc(20);
	memset(groupindex,0,20);
	
	char * indexG=(char *)malloc(10);
	memset(indexG,0,10);
	char * typeG=(char *)malloc(10);
	memset(typeG,0,10);

/////////////////////////QOS////////////////////////////	
	char * qosindex=(char *)malloc(10);
	memset(qosindex,0,10);
	char * sub_qos=(char *)malloc(10);
	memset(sub_qos,0,10);
	char * srcUP=(char *)malloc(10);
	memset(srcUP,0,10);
	char * srcDSCP=(char *)malloc(10);
	memset(srcDSCP,0,10);
	char * egressUP=(char *)malloc(10);
	memset(egressUP,0,10);
	char * egressDSCP=(char *)malloc(10);
	memset(egressDSCP,0,10);
/////////////////////////////////////////////////////////	
	
	int showFlag=0;
	//int showExFlag=0;
	unsigned int in_groupNum[1024];
	unsigned int e_groupNum[1024];
	char * in_groupNumTemp=(char *)malloc(20);
	memset(in_groupNumTemp,0,20);
	char * e_groupNumTemp=(char *)malloc(20);
	memset(e_groupNumTemp,0,20);
	//int tempflag=0;
	for(i=0;i<1024;i++)
		{
			in_groupNum[i]=0;
			e_groupNum[i]=0;
		}
	ccgi_dbus_init();
	if(cgiFormStringNoNewlines("UN", encry, BUF_LEN)!=cgiFormNotFound )  /*首次进入该页*/
	{
	  str=dcryption(encry);
	  if(str==NULL)
	  {
		ShowErrorPage(search(lpublic,"ill_user")); 		 /*用户非法*/
		return 0;
	  }
	}
    else
    {
  	 ////////////////////////////////////////////////////抓 any
      cgiFormStringNoNewlines("sourceIP",sourceIP,10);
      cgiFormStringNoNewlines("dstIP",dstIP,10);
      cgiFormStringNoNewlines("sourcePort",sourcePort,10);
      cgiFormStringNoNewlines("dstPort",dstPort,10);
      cgiFormStringNoNewlines("srcMac",srcMac,10);
      cgiFormStringNoNewlines("dstMac",dstMac,10);
	//////////////////////////////////////////////////////////////////
      cgiFormStringNoNewlines("encry_addrule",encry,BUF_LEN);
      cgiFormStringNoNewlines("Protocol",select_a,10);
      cgiFormStringNoNewlines("ExProtocol",Exselect_a,10);
      cgiFormStringNoNewlines("Action",select_b,20);
      cgiFormStringNoNewlines("RuleType",select_c,10);
      cgiFormStringNoNewlines("ruleindex",text_index,30);
      cgiFormStringNoNewlines("policyindex",policyindex,10);
      cgiFormStringNoNewlines("smasklen",smasklength,10);
      cgiFormStringNoNewlines("dmasklen",dmasklength,10);
      cgiFormStringNoNewlines("slot",slot,10);
      cgiFormStringNoNewlines("port",port,10);
	  cgiFormStringNoNewlines("groupindex",groupindex,20);
	  cgiFormStringNoNewlines("qosindex",qosindex,10);
	  cgiFormStringNoNewlines("sub_qos",sub_qos,10);
	  cgiFormStringNoNewlines("srcUP",srcUP,10);
	  cgiFormStringNoNewlines("srcDSCP",srcDSCP,10);
	  cgiFormStringNoNewlines("egressUP",egressUP,10);
	  cgiFormStringNoNewlines("egressDSCP",egressDSCP,10);

	show_qos_profile(receive_qos,&qos_num,lcontrol);

	if(strcmp(policyindex,"")==0)
	{
      		strcpy(policyindex,"0");
	}
      
      if(strcmp(sourcePort,"any")==0)
      		strcat(srcPort,"any");
      else 
     	cgiFormStringNoNewlines("srcPort",srcPort,10);
     	
      if(strcmp(dstPort,"any")==0)
      		strcat(DstPort,"any");
      else 
     	 cgiFormStringNoNewlines("DstPort",DstPort,10);

      cgiFormStringNoNewlines("icmp_type",icmp_type,10);
      cgiFormStringNoNewlines("icmp_code",icmp_code,10);
      
      if(strcmp(srcMac,"any")==0)
      		strcat(srcmac,"any");
      else 
      	cgiFormStringNoNewlines("srcmac",srcmac,20);
      if(strcmp(dstMac,"any")==0)
      		strcat(dstmac,"any");
      else 
      	cgiFormStringNoNewlines("dstmac",dstmac,20);
      cgiFormStringNoNewlines("vlanID",vlanID,10);
	if(strcmp(vlanID,"")==0)
	{
		strcpy(vlanID,"any");
	}
      cgiFormStringNoNewlines("sourceslot",sourceslot,10);
      cgiFormStringNoNewlines("sourceport",sourceport,10);
      sprintf(sourceslotport,"%s/%s",sourceslot,sourceport);
      sprintf(slotportNo,"%s/%s",slot,port);
      if(strcmp(sourceslotport,"/")==0)
      {
      		strcpy(sourceslotport,"any");
      }
     if(strcmp(dstIP,"any")==0)
      		strcat(dstip,"any");
      else
      {
        memset(dip1,0,4);
    	cgiFormStringNoNewlines("dst_ip1",dip1,4);	 
    	strcat(dstip,dip1);
    	strcat(dstip,".");
    	memset(dip2,0,4);
    	cgiFormStringNoNewlines("dst_ip2",dip2,4); 
    	strcat(dstip,dip2);  
    	strcat(dstip,".");
    	memset(dip3,0,4);
    	cgiFormStringNoNewlines("dst_ip3",dip3,4); 
    	strcat(dstip,dip3);  
    	strcat(dstip,".");
    	memset(dip4,0,4);
    	cgiFormStringNoNewlines("dst_ip4",dip4,4);
    	strcat(dstip,dip4);
    	sprintf(dstip,"%s/%s",dstip,dmasklength);
	}
	if(strcmp(sourceIP,"any")==0)
      		strcat(srcip,"any");
      else
      {
        memset(sip1,0,4);
    	cgiFormStringNoNewlines("src_ip1",sip1,4);	 
    	strcat(srcip,sip1);
    	strcat(srcip,".");
    	memset(sip2,0,4);
    	cgiFormStringNoNewlines("src_ip2",sip2,4); 
    	strcat(srcip,sip2);  
    	strcat(srcip,".");
    	memset(sip3,0,4);
    	cgiFormStringNoNewlines("src_ip3",sip3,4); 
    	strcat(srcip,sip3);  
    	strcat(srcip,".");
    	memset(sip4,0,4);
    	cgiFormStringNoNewlines("src_ip4",sip4,4);
    	strcat(srcip,sip4);
    	sprintf(srcip,"%s/%s",srcip,smasklength);
    	}
    }
	show_qos_mode(mode);
   cgiHeaderContentType("text/html");
	fprintf(cgiOut,"<html xmlns=\"http://www.w3.org/1999/xhtml\"><head>\n");
  fprintf(cgiOut,"<meta http-equiv=Content-Type content=text/html; charset=gb2312>\n");
  fprintf(cgiOut,"<title>%s</title>\n",search(lcontrol,"route_manage"));
  fprintf(cgiOut,"<link rel=stylesheet href=/style.css type=text/css>\n"\
  	  	"<style type=text/css>\n"\
	  ".a3{width:30;border:0; text-align:center}\n"\
	  "</style>\n"\
  "</head>\n"\
   "<script src=/ip.js>\n"\
  "</script>\n"\
  "<script language=javascript>\n"\
   "function changestate()\n"\
   "{\n"\

    "if(document.addacl.RuleType.value==\"extended\")\n"\
    "{\n"\
    		 "if(document.addacl.ExProtocol.value==\"ip\" || document.addacl.ExProtocol.value==\"icmp\")\n"\
    		 	"{\n"\
    				"if(document.addacl.sourceIP.value==\"any\")\n"\
            		"{\n"\
            			"document.addacl.src_ip1.disabled=true;\n"\
            			"document.addacl.src_ip2.disabled=true;\n"\
            			"document.addacl.src_ip3.disabled=true;\n"\
            			"document.addacl.src_ip4.disabled=true;\n"\
            			"document.addacl.smasklen.disabled=true;\n"\
            			"document.addacl.smasklen.style.backgroundColor = '#ccc';\n"\
        
            		"}\n"\
            		"else if(document.addacl.sourceIP.value==\"Single\")\n"\
            		"{\n"\
            			"document.addacl.src_ip1.disabled=false;"\
            			"document.addacl.src_ip1.style.backgroundColor = '#fff';\n"\
            			"document.addacl.src_ip2.disabled=false;"\
            			"document.addacl.src_ip2.style.backgroundColor = '#fff';\n"\
            			"document.addacl.src_ip3.disabled=false;"\
            			"document.addacl.src_ip3.style.backgroundColor = '#fff';\n"\
            			"document.addacl.src_ip4.disabled=false;"\
            			"document.addacl.src_ip4.style.backgroundColor = '#fff';\n"\
            			"document.addacl.smasklen.disabled=false;"\
            			"document.addacl.smasklen.style.backgroundColor = '#fff';\n"\

            		"}\n"\
            		"if(document.addacl.dstIP.value==\"any\")\n"\
            		"{\n"\
            			"document.addacl.dst_ip1.disabled=true;\n"\
            			"document.addacl.dst_ip2.disabled=true;\n"\
            			"document.addacl.dst_ip3.disabled=true;\n"\
            			"document.addacl.dst_ip4.disabled=true;\n"\
            			"document.addacl.dmasklen.disabled=true;\n"\
            			"document.addacl.dmasklen.style.backgroundColor = '#ccc';\n"\
        
            		"}\n"\
            		"else if(document.addacl.dstIP.value==\"Single\")\n"\
            		"{\n"\
            			"document.addacl.dst_ip1.disabled=false;\n"\
            			"document.addacl.dst_ip1.style.backgroundColor = '#fff';\n"\
            			"document.addacl.dst_ip2.disabled=false;\n"\
            			"document.addacl.dst_ip2.style.backgroundColor = '#fff';\n"\
            			"document.addacl.dst_ip3.disabled=false;\n"\
            			"document.addacl.dst_ip3.style.backgroundColor = '#fff';\n"\
            			"document.addacl.dst_ip4.disabled=false;\n"\
            			"document.addacl.dst_ip4.style.backgroundColor = '#fff';\n"\
            			"document.addacl.dmasklen.disabled=false;\n"\
            			"document.addacl.dmasklen.style.backgroundColor = '#fff';\n"\
            		"}\n"\
    			"}\n"\
    		"else if(document.addacl.ExProtocol.value==\"tcp\" || document.addacl.ExProtocol.value==\"udp\")\n"\
    			"{\n"\
    				 "if(document.addacl.sourceIP.value==\"any\")\n"\
                		"{\n"\
                			"document.addacl.src_ip1.disabled=true;\n"\
                			"document.addacl.src_ip2.disabled=true;\n"\
                			"document.addacl.src_ip3.disabled=true;\n"\
                			"document.addacl.src_ip4.disabled=true;\n"\
                			"document.addacl.smasklen.disabled=true;\n"\
            				"document.addacl.smasklen.style.backgroundColor = '#ccc';\n"\
            
                		"}\n"\
                	"else if(document.addacl.sourceIP.value==\"Single\")\n"\
                		"{\n"\
            
                			"document.addacl.src_ip1.disabled=false;\n"\
                			"document.addacl.src_ip2.disabled=false;\n"\
                			"document.addacl.src_ip3.disabled=false;\n"\
                			"document.addacl.src_ip4.disabled=false;\n"\
                			"document.addacl.smasklen.disabled=false;\n"\
                 			"document.addacl.smasklen.style.backgroundColor = '#fff';\n"\
                		"}\n"\
                	"if(document.addacl.dstIP.value==\"any\")\n"\
                		"{\n"\
                			"document.addacl.dst_ip1.disabled=true;\n"\
                			"document.addacl.dst_ip2.disabled=true;\n"\
                			"document.addacl.dst_ip3.disabled=true;\n"\
                			"document.addacl.dst_ip4.disabled=true;\n"\
            				"document.addacl.dmasklen.disabled=true;\n"\
            				"document.addacl.dmasklen.style.backgroundColor = '#ccc';\n"\
                		"}\n"\
                	"else if(document.addacl.dstIP.value==\"Single\")\n"\
                		"{\n"\
                			"document.addacl.dst_ip1.disabled=false;\n"\
                			"document.addacl.dst_ip2.disabled=false;\n"\
                			"document.addacl.dst_ip3.disabled=false;\n"\
                			"document.addacl.dst_ip4.disabled=false;\n"\
                 			"document.addacl.dmasklen.disabled=false;\n"\
                 			"document.addacl.dmasklen.style.backgroundColor = '#fff';\n"\
                		"}\n"\
             
                	"if(document.addacl.sourcePort.value==\"any\")\n"\
                		"{\n"\
                			"document.addacl.srcPort.disabled=true;\n"\
                			"document.addacl.srcPort.style.backgroundColor = '#ccc';\n"\
                		"}\n"\
                	"else if(document.addacl.sourcePort.value==\"Single\")\n"\
                		"{\n"\
                			"document.addacl.srcPort.disabled=false;\n"\
                			"document.addacl.srcPort.style.backgroundColor = '#fff';\n"\
                		"}\n"\
                	"if(document.addacl.dstPort.value==\"any\")\n"\
                		"{\n"\
                			"document.addacl.DstPort.disabled=true;\n"\
                			"document.addacl.DstPort.style.backgroundColor = '#ccc';\n"\
                		"}\n"\
                	"else if(document.addacl.dstPort.value==\"Single\")\n"\
                		"{\n"\
                			"document.addacl.DstPort.disabled=false;\n"\
                			"document.addacl.DstPort.style.backgroundColor = '#fff';\n"\
                		"}\n"\
    			"}\n"\
    		"else if(document.addacl.ExProtocol.value==\"arp\")\n"\
    			"{\n"\
    				"if(document.addacl.srcMac.value==\"any\")\n"\
                		"{\n"\
                			"document.addacl.srcmac.disabled=true;\n"\
                			"document.addacl.srcmac.style.backgroundColor = '#ccc';\n"\
                		"}\n"\
                	"else if(document.addacl.srcMac.value==\"Single\")\n"\
                		"{\n"\
                			"document.addacl.srcmac.disabled=false;\n"\
                			"document.addacl.srcmac.style.backgroundColor = '#fff';\n"\
                		"}\n"\
    			"}\n"\
    		"else if(document.addacl.ExProtocol.value==\"ethernet\")\n"\
    			"{\n"\
    				"if(document.addacl.srcMac.value==\"any\")\n"\
                		"{\n"\
                			"document.addacl.srcmac.disabled=true;\n"\
                			"document.addacl.srcmac.style.backgroundColor = '#ccc';\n"\
                		"}\n"\
                	"else if(document.addacl.srcMac.value==\"Single\")\n"\
                		"{\n"\
                			"document.addacl.srcmac.disabled=false;\n"\
                			"document.addacl.srcmac.style.backgroundColor = '#fff';\n"\
                		"}\n"\
                	"if(document.addacl.dstMac.value==\"any\")\n"\
                		"{\n"\
                			"document.addacl.dstmac.disabled=true;\n"\
                			"document.addacl.dstmac.style.backgroundColor = '#ccc';\n"\
                		"}\n"\
                	"else if(document.addacl.dstMac.value==\"Single\")\n"\
                		"{\n"\
                			"document.addacl.dstmac.disabled=false;\n"\
                			"document.addacl.dstmac.style.backgroundColor = '#fff';\n"\
                		"}\n"\
    			"}\n"\
    	"}\n"\
	"else if(document.addacl.RuleType.value==\"standard\")\n"\
    	"{\n"\
    		 "if(document.addacl.Protocol.value==\"ip\" || document.addacl.Protocol.value==\"icmp\")\n"\
    		 	"{\n"\
    				"if(document.addacl.sourceIP.value==\"any\")\n"\
            		"{\n"\
            			"document.addacl.src_ip1.disabled=true;\n"\
            			"document.addacl.src_ip2.disabled=true;\n"\
            			"document.addacl.src_ip3.disabled=true;\n"\
            			"document.addacl.src_ip4.disabled=true;\n"\
            			"document.addacl.smasklen.disabled=true;\n"\
            			"document.addacl.smasklen.style.backgroundColor = '#ccc';\n"\
        
            		"}\n"\
            		"else if(document.addacl.sourceIP.value==\"Single\")\n"\
            		"{\n"\
            			"document.addacl.src_ip1.disabled=false;"\
            			"document.addacl.src_ip1.style.backgroundColor = '#fff';\n"\
            			"document.addacl.src_ip2.disabled=false;"\
            			"document.addacl.src_ip2.style.backgroundColor = '#fff';\n"\
            			"document.addacl.src_ip3.disabled=false;"\
            			"document.addacl.src_ip3.style.backgroundColor = '#fff';\n"\
            			"document.addacl.src_ip4.disabled=false;"\
            			"document.addacl.src_ip4.style.backgroundColor = '#fff';\n"\
            			"document.addacl.smasklen.disabled=false;"\
            			"document.addacl.smasklen.style.backgroundColor = '#fff';\n"\

            		"}\n"\
            		"if(document.addacl.dstIP.value==\"any\")\n"\
            		"{\n"\
            			"document.addacl.dst_ip1.disabled=true;\n"\
            			"document.addacl.dst_ip2.disabled=true;\n"\
            			"document.addacl.dst_ip3.disabled=true;\n"\
            			"document.addacl.dst_ip4.disabled=true;\n"\
            			"document.addacl.dmasklen.disabled=true;\n"\
            			"document.addacl.dmasklen.style.backgroundColor = '#ccc';\n"\
        
            		"}\n"\
            		"else if(document.addacl.dstIP.value==\"Single\")\n"\
            		"{\n"\
            			"document.addacl.dst_ip1.disabled=false;\n"\
            			"document.addacl.dst_ip1.style.backgroundColor = '#fff';\n"\
            			"document.addacl.dst_ip2.disabled=false;\n"\
            			"document.addacl.dst_ip2.style.backgroundColor = '#fff';\n"\
            			"document.addacl.dst_ip3.disabled=false;\n"\
            			"document.addacl.dst_ip3.style.backgroundColor = '#fff';\n"\
            			"document.addacl.dst_ip4.disabled=false;\n"\
            			"document.addacl.dst_ip4.style.backgroundColor = '#fff';\n"\
            			"document.addacl.dmasklen.disabled=false;\n"\
            			"document.addacl.dmasklen.style.backgroundColor = '#fff';\n"\
            		"}\n"\
    			"}\n"\
    		"else if(document.addacl.Protocol.value==\"tcp\" || document.addacl.Protocol.value==\"udp\")\n"\
    			"{\n"\
    				 "if(document.addacl.sourceIP.value==\"any\")\n"\
                		"{\n"\
                			"document.addacl.src_ip1.disabled=true;\n"\
                			"document.addacl.src_ip2.disabled=true;\n"\
                			"document.addacl.src_ip3.disabled=true;\n"\
                			"document.addacl.src_ip4.disabled=true;\n"\
                			"document.addacl.smasklen.disabled=true;\n"\
            				"document.addacl.smasklen.style.backgroundColor = '#ccc';\n"\
            
                		"}\n"\
                	"else if(document.addacl.sourceIP.value==\"Single\")\n"\
                		"{\n"\
            
                			"document.addacl.src_ip1.disabled=false;\n"\
                			"document.addacl.src_ip2.disabled=false;\n"\
                			"document.addacl.src_ip3.disabled=false;\n"\
                			"document.addacl.src_ip4.disabled=false;\n"\
                			"document.addacl.smasklen.disabled=false;\n"\
                 			"document.addacl.smasklen.style.backgroundColor = '#fff';\n"\
                		"}\n"\
                	"if(document.addacl.dstIP.value==\"any\")\n"\
                		"{\n"\
                			"document.addacl.dst_ip1.disabled=true;\n"\
                			"document.addacl.dst_ip2.disabled=true;\n"\
                			"document.addacl.dst_ip3.disabled=true;\n"\
                			"document.addacl.dst_ip4.disabled=true;\n"\
            				"document.addacl.dmasklen.disabled=true;\n"\
            				"document.addacl.dmasklen.style.backgroundColor = '#ccc';\n"\
                		"}\n"\
                	"else if(document.addacl.dstIP.value==\"Single\")\n"\
                		"{\n"\
                			"document.addacl.dst_ip1.disabled=false;\n"\
                			"document.addacl.dst_ip2.disabled=false;\n"\
                			"document.addacl.dst_ip3.disabled=false;\n"\
                			"document.addacl.dst_ip4.disabled=false;\n"\
                 			"document.addacl.dmasklen.disabled=false;\n"\
                 			"document.addacl.dmasklen.style.backgroundColor = '#fff';\n"\
                		"}\n"\
             
                	"if(document.addacl.sourcePort.value==\"any\")\n"\
                		"{\n"\
                			"document.addacl.srcPort.disabled=true;\n"\
                			"document.addacl.srcPort.style.backgroundColor = '#ccc';\n"\
                		"}\n"\
                	"else if(document.addacl.sourcePort.value==\"Single\")\n"\
                		"{\n"\
                			"document.addacl.srcPort.disabled=false;\n"\
                			"document.addacl.srcPort.style.backgroundColor = '#fff';\n"\
                		"}\n"\
                	"if(document.addacl.dstPort.value==\"any\")\n"\
                		"{\n"\
                			"document.addacl.DstPort.disabled=true;\n"\
                			"document.addacl.DstPort.style.backgroundColor = '#ccc';\n"\
                		"}\n"\
                	"else if(document.addacl.dstPort.value==\"Single\")\n"\
                		"{\n"\
                			"document.addacl.DstPort.disabled=false;\n"\
                			"document.addacl.DstPort.style.backgroundColor = '#fff';\n"\
                		"}\n"\
    			"}\n"\
    		"else if(document.addacl.Protocol.value==\"arp\")\n"\
    			"{\n"\
    				"if(document.addacl.srcMac.value==\"any\")\n"\
                		"{\n"\
                			"document.addacl.srcmac.disabled=true;\n"\
                			"document.addacl.srcmac.style.backgroundColor = '#ccc';\n"\
                		"}\n"\
                	"else if(document.addacl.srcMac.value==\"Single\")\n"\
                		"{\n"\
                			"document.addacl.srcmac.disabled=false;\n"\
                			"document.addacl.srcmac.style.backgroundColor = '#fff';\n"\
                		"}\n"\
    			"}\n"\
    		"else if(document.addacl.Protocol.value==\"ethernet\")\n"\
    			"{\n"\
    				"if(document.addacl.srcMac.value==\"any\")\n"\
                		"{\n"\
                			"document.addacl.srcmac.disabled=true;\n"\
                			"document.addacl.srcmac.style.backgroundColor = '#ccc';\n"\
                		"}\n"\
                	"else if(document.addacl.srcMac.value==\"Single\")\n"\
                		"{\n"\
                			"document.addacl.srcmac.disabled=false;\n"\
                			"document.addacl.srcmac.style.backgroundColor = '#fff';\n"\
                		"}\n"\
                	"if(document.addacl.dstMac.value==\"any\")\n"\
                		"{\n"\
                			"document.addacl.dstmac.disabled=true;\n"\
                			"document.addacl.dstmac.style.backgroundColor = '#ccc';\n"\
                		"}\n"\
                	"else if(document.addacl.dstMac.value==\"Single\")\n"\
                		"{\n"\
                			"document.addacl.dstmac.disabled=false;\n"\
                			"document.addacl.dstmac.style.backgroundColor = '#fff';\n"\
                		"}\n"\
    			"}\n"\
    	"}\n"\
   	"}\n"\
  "</script>\n"\
  "<body onload=changestate()>\n");
  if(cgiFormSubmitClicked("submit_addrule") == cgiFormSuccess)
  {
  	int IPCheckTemp=0,IPCheckTempSrc=0,IPCheckTempDst=0;
  	unsigned long temprule = 0;
	unsigned long tempvlanid = 0;
	dcli_str2ulong(text_index,&temprule);
	dcli_str2ulong(vlanID,&tempvlanid);
	
  	if(strcmp(text_index,"")==0)
  	{
  		ShowAlert(search(lcontrol,"acl_index_not_null"));
  	}
  	else if((temprule<1 || temprule>1000)&&(strcmp(select_c,"standard")==0))
  	{
  			ShowAlert(search(lcontrol,"standard_acl_outrange"));		
  	}
	else if((temprule<1 || temprule>500)&&(strcmp(select_c,"extended")==0))
	{	
			ShowAlert(search(lcontrol,"extended_acl_outrange"));			
	}
  	else if(tempvlanid<0 || tempvlanid>4095)
  	{
  		ShowAlert(search(lcontrol,"acl_illegal_input"));
  	}
  	else
  	{
		///////////////////case ////////////////
		if(strcmp(select_c,"standard")==0)
		{
			if(strcmp(select_a,"ip")==0)
			{
				if(strcmp(dstIP,"any")!=0)
				{
					if(!(strcmp(dip1,"")&&strcmp(dip2,"")&&strcmp(dip3,"")&&strcmp(dip4,"")))
					{
						ShowAlert(search(lcontrol,"ip_null"));
					}
					else
					{
						IPCheckTempDst=1;
					}

				}
				else
				{
					IPCheckTempDst=1;
				}
				
				if(strcmp(sourceIP,"any")!=0)
				{
					if(!(strcmp(sip1,"")&&strcmp(sip2,"")&&strcmp(sip3,"")&&strcmp(sip4,"")))
						ShowAlert(search(lcontrol,"ip_null"));
					else IPCheckTempSrc=1;

				}
				else IPCheckTempSrc=1;

				if(IPCheckTempSrc==1 && IPCheckTempDst==1)
					IPCheckTemp=1;
				
				if(IPCheckTemp==1)
				{
    				if(strcmp(select_b,"permit")==0 || strcmp(select_b,"deny")==0)
    				{
    					addrule_PermitOrDeny_ip(0,text_index,select_b,dstip,srcip,policyindex);
    				}
    				else if(strcmp(select_b,"trap_to_cpu")==0)
    						addrule_trap_IP(0,text_index,dstip,srcip);
    				else if(strcmp(select_b,"mirror")==0|| strcmp(select_b,"redirect")==0)
    						addrule_MirrorOrRedir_ip(0,text_index,select_b,slotportNo,dstip,srcip,policyindex);
				}
				
			}
			else if(strcmp(select_a,"tcp")==0 || strcmp(select_a,"udp")==0)
			{
				if(strcmp(dstIP,"any")!=0)
				{
					if(!(strcmp(dip1,"")&&strcmp(dip2,"")&&strcmp(dip3,"")&&strcmp(dip4,"")))
						ShowAlert(search(lcontrol,"ip_null"));
					else IPCheckTempDst=1;

				}
				else IPCheckTempDst=1;
				
				if(strcmp(sourceIP,"any")!=0)
				{
					if(!(strcmp(sip1,"")&&strcmp(sip2,"")&&strcmp(sip3,"")&&strcmp(sip4,"")))
						ShowAlert(search(lcontrol,"ip_null"));
					else IPCheckTempSrc=1;

				}
				else IPCheckTempSrc=1;

				if(IPCheckTempSrc==1 && IPCheckTempDst==1)
					IPCheckTemp=1;
				
				if(IPCheckTemp==1)
				{
	     				if(strcmp(select_b,"permit")==0 || strcmp(select_b,"deny")==0)
	     				{
	     					addrule_PermitOrDeny_TcpOrUdp(0,text_index,select_b,select_a,dstip,DstPort,srcip,srcPort,policyindex);
	     				}
	     				else if(strcmp(select_b,"trap_to_cpu")==0)
	     				{
	     					addrule_trap_UdpOrTcp(0,select_a,text_index,dstip,DstPort,srcip,srcPort);
	     				}
	     				else if(strcmp(select_b,"mirror")==0|| strcmp(select_b,"redirect")==0)
	     				{
	     					addrule_MirrorOrRedir_TcpOrUdp(0,text_index,select_b,slotportNo,select_a,dstip,DstPort,srcip,srcPort,policyindex);
	     				}
				}
			}
			else if(strcmp(select_a,"icmp")==0)
			{
				if(strcmp(select_b,"permit")==0 || strcmp(select_b,"deny")==0)
					addrule_PermitOrDeny_icmp(0,text_index,select_b,dstip,srcip,icmp_type,icmp_code,policyindex);
				else if(strcmp(select_b,"trap_to_cpu")==0)
					addrule_trap_Icmp(0,text_index,dstip,srcip,icmp_type,icmp_code);
				else if(strcmp(select_b,"mirror")==0|| strcmp(select_b,"redirect")==0)
					addrule_MirrorOrRedir_icmp(0,text_index,select_b,slotportNo,dstip,srcip,icmp_type,icmp_code,policyindex);
			}
			else if(strcmp(select_a,"arp")==0)
			{
				int check=checkvalid(sourceslotport);
				
				if(check==1)
					ShowAlert(search(lcontrol,"slot_port_illegal"));
				else if(check==0)
				{
     				if(strcmp(select_b,"permit")==0 || strcmp(select_b,"deny")==0)
     					addrule_PermitOrDeny_arp(0,text_index,select_b,srcmac,vlanID,sourceslotport,policyindex);
     				else if(strcmp(select_b,"trap_to_cpu")==0)
     					addrule_trap_arp(0,text_index,srcmac,vlanID,sourceslotport);
     				else if(strcmp(select_b,"mirror")==0|| strcmp(select_b,"redirect")==0)
     					addrule_MirrorOrRedir_arp(0,text_index,select_b,slotportNo,srcmac,vlanID,sourceslotport,policyindex);
				}
				else if(check == -1)
				{
     				if(strcmp(select_b,"permit")==0 || strcmp(select_b,"deny")==0)
     					addrule_PermitOrDeny_arp(0,text_index,select_b,srcmac,vlanID,"any",policyindex);
     				else if(strcmp(select_b,"trap_to_cpu")==0)
     					addrule_trap_arp(0,text_index,srcmac,vlanID,"any");
     				else if(strcmp(select_b,"mirror")==0|| strcmp(select_b,"redirect")==0)
     					addrule_MirrorOrRedir_arp(0,text_index,select_b,slotportNo,srcmac,vlanID,"any",policyindex);
				}
			}
			else if(strcmp(select_a,"ethernet")==0)
			{
				if(strcmp(select_b,"permit")==0 || strcmp(select_b,"deny")==0)
					addrule_PermitOrDeny_ethernet(0,text_index,select_b,dstmac,srcmac,policyindex);
				else if(strcmp(select_b,"trap_to_cpu")==0)
					addrule_trap_ethernet(0,text_index,dstmac,srcmac);
				else if(strcmp(select_b,"mirror")==0|| strcmp(select_b,"redirect")==0)
					addrule_MirrorOrRedir_ethernet(0,text_index,select_b,slotportNo,dstmac,srcmac,policyindex);
			}
			//*****************not support ingress-qos and egress-qos*************************************
			#if 0
			else if(strcmp(select_b,"ingress-qos")==0)
			{
				
				if(strcmp(srcUP,"")==0)
					strcpy(srcUP,"none");
				
				if(strcmp(srcDSCP,"")==0)
					strcpy(srcDSCP,"none");

				addrule_ingress_qos(text_index,qosindex,sub_qos,srcUP,srcDSCP,policyindex,lcontrol);
			}
			else if(strcmp(select_b,"egress-qos")==0)
			{
				//fprintf(cgiOut,"222egressUP=%s-egressDSCP=%s-srcUP=%s-srcDSCP=%s22",egressUP,egressDSCP,srcUP,srcDSCP);
				addrule_egress_qos(text_index,egressUP,egressDSCP,srcUP,srcDSCP,lcontrol);
			}
			#endif
			//********************************************************************************************
		}
		else if(strcmp(select_c,"extended")==0)
		{
			if(strcmp(Exselect_a,"ip")==0)
			{
				if(strcmp(dstIP,"any")!=0)
				{
					if(!(strcmp(dip1,"")&&strcmp(dip2,"")&&strcmp(dip3,"")&&strcmp(dip4,"")))
					{
						ShowAlert(search(lcontrol,"ip_null"));
					}
					else
					{
						IPCheckTempDst=1;
					}

				}
				else
				{
					IPCheckTempDst=1;
				}
				
				if(strcmp(sourceIP,"any")!=0)
				{
					if(!(strcmp(sip1,"")&&strcmp(sip2,"")&&strcmp(sip3,"")&&strcmp(sip4,"")))
					{
						ShowAlert(search(lcontrol,"ip_null"));
					}
					else
					{
						IPCheckTempSrc=1;
					}

				}
				else 
				{
					IPCheckTempSrc=1;
				}

				if(IPCheckTempSrc==1 && IPCheckTempDst==1)
				{
					IPCheckTemp=1;
				}
				
				if(IPCheckTemp==1)
				{
	    				if(strcmp(select_b,"permit")==0 || strcmp(select_b,"deny")==0)
	    				{
	    					//ShowAlert("addrule_PermitOrDeny_ip");
	    					addrule_PermitOrDeny_ip(1,text_index,select_b,dstip,srcip,policyindex);
	    				}
	    				else if(strcmp(select_b,"trap_to_cpu")==0)
	    				{
	    						addrule_trap_IP(1,text_index,dstip,srcip);
	    				}
	    				else if(strcmp(select_b,"mirror")==0|| strcmp(select_b,"redirect")==0)
	    				{
	    						//ShowAlert("addrule_MirrorOrRedir_ip");
	    						addrule_MirrorOrRedir_ip(1,text_index,select_b,slotportNo,dstip,srcip,policyindex);
					}
				}
				
			}
			else if(strcmp(Exselect_a,"tcp")==0 || strcmp(Exselect_a,"udp")==0)
			{
				if(strcmp(dstIP,"any")!=0)
				{
					if(!(strcmp(dip1,"")&&strcmp(dip2,"")&&strcmp(dip3,"")&&strcmp(dip4,"")))
					{
						//ShowAlert(search(lcontrol,"ip_null"));
					}
					else
					{
						IPCheckTempDst=1;
					}

				}
				else 
				{
					IPCheckTempDst=1;
				}
				
				if(strcmp(sourceIP,"any")!=0)
				{
					if(!(strcmp(sip1,"")&&strcmp(sip2,"")&&strcmp(sip3,"")&&strcmp(sip4,"")))
					{
						ShowAlert(search(lcontrol,"ip_null"));
					}
					else
					{
						IPCheckTempSrc=1;
					}

				}
				else 
				{
					IPCheckTempSrc=1;
				}

				if(IPCheckTempSrc==1 && IPCheckTempDst==1)
				{
					IPCheckTemp=1;
				}
				
				if(IPCheckTemp==1)
				{
	     				if(strcmp(select_b,"permit")==0 || strcmp(select_b,"deny")==0)
	     				{
	     					addrule_extend(text_index,select_b,Exselect_a,dstip,DstPort,srcip,srcPort,dstmac,srcmac,vlanID,sourceslotport,policyindex);
	     				}
	     				else if(strcmp(select_b,"trap_to_cpu")==0)
	     				{
	     					addrule_extend(text_index,select_b,Exselect_a,dstip,DstPort,srcip,srcPort,dstmac,srcmac,vlanID,sourceslotport,policyindex);
	     				}
	     				else if(strcmp(select_b,"mirror")==0|| strcmp(select_b,"redirect")==0)
	     				{
						if(strcmp(sourceslot,"")==0)
						{
							strcpy(sourceslot,"any");
						}
	     					addrule_MirrorOrRedir_TcpOrUdp_extended(text_index,Exselect_a,slotportNo,dstip,DstPort,srcip,srcPort,dstmac,srcmac,vlanID,sourceslotport,policyindex);
	     				}
				}
			}
			else if(strcmp(Exselect_a,"icmp")==0)
			{
				if(strcmp(select_b,"permit")==0 || strcmp(select_b,"deny")==0)
				{
					addrule_PermitOrDeny_icmp(1,text_index,select_b,dstip,srcip,icmp_type,icmp_code,policyindex);
				}
				else if(strcmp(select_b,"trap_to_cpu")==0)
				{
					addrule_trap_Icmp(1,text_index,dstip,srcip,icmp_type,icmp_code);
				}
				else if(strcmp(select_b,"mirror")==0|| strcmp(select_b,"redirect")==0)
				{
					addrule_MirrorOrRedir_icmp(1,text_index,select_b,slotportNo,dstip,srcip,icmp_type,icmp_code,policyindex);
				}
			}
			else if(strcmp(Exselect_a,"arp")==0)
			{
				if(strcmp(sourceslotport,"any")==0)
				{
					strcpy(sourceslotport,"/");
				}
				int check=checkvalid(sourceslotport);
				
				if(check==1)
				{
					ShowAlert(search(lcontrol,"slot_port_illegal"));
				}
				else if(check==0)
				{
	     				if(strcmp(select_b,"permit")==0 || strcmp(select_b,"deny")==0)
	     				{
	     					addrule_PermitOrDeny_arp(1,text_index,select_b,srcmac,vlanID,sourceslotport,policyindex);
	     				}
	     				else if(strcmp(select_b,"trap_to_cpu")==0)
	     				{
	     					addrule_trap_arp(1,text_index,srcmac,vlanID,sourceslotport);
	     				}
	     				else if(strcmp(select_b,"mirror")==0|| strcmp(select_b,"redirect")==0)
	     				{
	     					
	     					addrule_MirrorOrRedir_arp(1,text_index,select_b,slotportNo,srcmac,vlanID,sourceslotport,policyindex);
	     				}
				}
				else if(check == -1)
				{
	     				if(strcmp(select_b,"permit")==0 || strcmp(select_b,"deny")==0)
	     				{
	     					addrule_PermitOrDeny_arp(1,text_index,select_b,srcmac,vlanID,"any",policyindex);
	     				}
	     				else if(strcmp(select_b,"trap_to_cpu")==0)
	     				{
	     					addrule_trap_arp(1,text_index,srcmac,vlanID,"any");
	     				}
	     				else if(strcmp(select_b,"mirror")==0|| strcmp(select_b,"redirect")==0)
	     				{
	     					addrule_MirrorOrRedir_arp(1,text_index,select_b,slotportNo,srcmac,vlanID,"any",policyindex);
	     				}
				}
			}
			else if(strcmp(Exselect_a,"ethernet")==0)
			{
				if(strcmp(select_b,"permit")==0 || strcmp(select_b,"deny")==0)
				{
					addrule_PermitOrDeny_ethernet(1,text_index,select_b,dstmac,srcmac,policyindex);
				}
				else if(strcmp(select_b,"trap_to_cpu")==0)
				{
					addrule_trap_ethernet(1,text_index,dstmac,srcmac);
				}
				else if(strcmp(select_b,"mirror")==0|| strcmp(select_b,"redirect")==0)
				{
					addrule_MirrorOrRedir_ethernet(1,text_index,select_b,slotportNo,dstmac,srcmac,policyindex);
				}
			}
			//*****************extended support ingress-qos and egress-qos*************************************
			else if(strcmp(select_b,"ingress-qos")==0)
			{
				if(qos_num!=0)
				{
					if(strcmp(srcUP,"")==0)
						strcpy(srcUP,"none");
					
					if(strcmp(srcDSCP,"")==0)
						strcpy(srcDSCP,"none");
					if(strcmp(mode,"flow")==0)
					{
						//configQosMode("flow");
						addrule_ingress_qos(text_index,qosindex,sub_qos,srcUP,srcDSCP,policyindex,lcontrol);
					}
					else
					{
						ShowAlert(search(lcontrol,"notflow"));
					}
					
				}
				else
				{
					ShowAlert(search(lcontrol,"qos_warnning"));
				}
			}
			else if(strcmp(select_b,"egress-qos")==0)
			{
				//configQosMode("flow");
				if(strcmp(mode,"flow")==0)
				{
					addrule_egress_qos(text_index,egressUP,egressDSCP,srcUP,srcDSCP,lcontrol);
				}
				else
				{
					ShowAlert(search(lcontrol,"notflow"));
				}
			}

			
		}

//////////////////////////////////////add to group/////////////////////////////////////////////////////////////
		if(strcmp(groupindex,"NONE")!=0)
		{
			int tempdir=0;
    		indexG=strtok(groupindex,"-");
    		typeG=strtok(NULL,"-");
    		int temp=atoi(indexG);
    		if(strcmp(typeG,"ingress")==0)
    			tempdir=0;
    		else if(strcmp(typeG,"egress")==0)
    			tempdir=1;
    		add_rule_group("add",text_index,temp,tempdir,lcontrol);
		}

	}
  }
	  ccgi_dbus_init();
  fprintf(cgiOut,"<form name=addacl method=post>\n"\
  "<div align=center>\n"\
  "<table width=976 border=0 cellpadding=0 cellspacing=0>\n"\
  "<tr>\n"\
    "<td width=8 align=left valign=top background=/images/di22.jpg><img src=/images/youce4.jpg width=8 height=30/></td>\n"\
    "<td width=51 align=left valign=bottom background=/images/di22.jpg><img src=/images/youce33.jpg width=37 height=24/></td>\n"\
    "<td width=153 align=left valign=bottom background=/images/di22.jpg><font id=titleen>ACL</font><font id=%s> %s</font></td>\n",search(lpublic,"title_style"),search(lpublic,"management"));
    fprintf(cgiOut,"<td width=690 align=right valign=bottom background=/images/di22.jpg>");
	  //  if((fp=fopen("../htdocs/text/public.txt","r"))==NULL)		 /*以只读方式打开资源文件*/
	  //    ShowAlert(search(lpublic,"error_open"));
	  //  fseek(fp,4,0); 						/*将文件指针移到离文件首4个字节处，即lan=之后*/
	    //fgets(lan,3,fp);	   
		//fclose(fp);
	   // if(strcmp(lan,"ch")==0)
    //	{	
    	  fprintf(cgiOut,"<table width=130 border=0 cellspacing=0 cellpadding=0>\n"\
          "<tr>\n"\
          "<td width=62 align=center><input id=but type=submit name=submit_addrule style=background-image:url(/images/%s) value=\"\"></td>\n",search(lpublic,"img_ok"));		  
		  if(cgiFormSubmitClicked("submit_addrule") != cgiFormSuccess)
            fprintf(cgiOut,"<td width=62 align=left><a href=wp_aclall.cgi?UN=%s target=mainFrame><img src=/images/%s border=0 width=62 height=20/></a></td>\n",encry,search(lpublic,"img_cancel"));
		  else                                         
     		fprintf(cgiOut,"<td width=62 align=left><a href=wp_aclall.cgi?UN=%s target=mainFrame><img src=/images/%s border=0 width=62 height=20/></a></td>\n",encry,search(lpublic,"img_cancel"));
		  fprintf(cgiOut,"</tr>\n"\
          "</table>\n");
		/*}		
		else			
		{	
		  fprintf(cgiOut,"<table width=130 border=0 cellspacing=0 cellpadding=0>"\
		  "<tr>"\
		  "<td width=62 align=center><input id=but type=submit name=submit_addrule style=background-image:url(/images/ok-en.jpg) value=""></td>");		  
		  if(cgiFormSubmitClicked("submit_addrule") != cgiFormSuccess)
		    fprintf(cgiOut,"<td width=62 align=left><a href=wp_aclall.cgi?UN=%s target=mainFrame><img src=/images/cancel-en.jpg border=0 width=62 height=20/></a></td>",encry);
		  else
		    fprintf(cgiOut,"<td width=62 align=left><a href=wp_aclall.cgi?UN=%s target=mainFrame><img src=/images/cancel-en.jpg border=0 width=62 height=20/></a></td>",encry);
		  fprintf(cgiOut,"</tr>"\
		  "</table>");
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
                  
				fprintf(cgiOut,"<tr height=25>"\
				"<td align=left id=tdleft><a href=wp_aclall.cgi?UN=%s target=mainFrame class=top><font id=%s>%s</font></a></td>",encry,search(lpublic,"menu_san"),search(lcontrol,"acl_list")); 				   
				fprintf(cgiOut,"</tr>");
				
				fprintf(cgiOut,"<tr height=26>"\
	   			"<td align=left id=tdleft background=/images/bottom_bg.gif style=\"border-right:0\"><font id=%s>%s</font></td>",search(lpublic,"menu_san"),search(lcontrol,"add_acl_rule"));   /*突出显示*/
	   			fprintf(cgiOut,"</tr>");
	   			fprintf(cgiOut,"<tr height=25>"\
	  			"<td align=left id=tdleft><a href=wp_aclgrplist.cgi?UN=%s target=mainFrame class=top><font id=%s>%s</font></a></td>",encry,search(lpublic,"menu_san"),search(lcontrol,"acl_grp_lsit"));					   
	  			fprintf(cgiOut,"</tr>"\
				"<tr height=25>"\
				"<td align=left id=tdleft><a href=wp_addaclgroup.cgi?UN=%s target=mainFrame class=top><font id=%s>%s</font></a></td>",encry,search(lpublic,"menu_san"),search(lcontrol,"add_acl_group"));
				fprintf(cgiOut,"</tr>");



				for(i=0;i<13;i++)
					  {
						fprintf(cgiOut,"<tr height=25>"\
						  "<td id=tdleft>&nbsp;</td>"\
						"</tr>");
					  }
					showFlag=0;
				  fprintf(cgiOut,"</table>"\
              "</td>"\
              "<td align=left valign=top style=\"background-color:#ffffff; border-right:1px solid #707070; padding-left:30px; padding-top:10px\">"\
					  "<table width=640 border=0 cellspacing=0 cellpadding=0>"\
						"<tr>"\
						  "<td align=left valign=top style=padding-top:10px>"\
						  	"<table width=640 border=0 cellspacing=0 cellpadding=0>"\
						  	"<tr height=35>\n"\
						  		"<td colspan='3' style=\"font-size:14px\"><font color='red'><b>%s</b></font></td>",search(lcontrol,mode));
				fprintf(cgiOut,"</tr>\n"\
						  	"<tr align=left>");
							fprintf(cgiOut,"<td  align=left width=90>%s:</td>",search(lcontrol,"ruleType"));
							fprintf(cgiOut,"<td align=left colspan=2>");
							fprintf(cgiOut, "<select name=\"RuleType\" onchange=\"javascript:this.form.submit();\" style='width:110;height:auto'>");
						  	for(i=0;i<2;i++)
							if(strcmp(RuleType[i],select_c)==0)				/*显示上次选中的*/
								fprintf(cgiOut,"<option value=%s selected=selected>%s",RuleType[i],RuleType[i]);
							else				
								fprintf(cgiOut,"<option value=%s>%s",RuleType[i],RuleType[i]);
                         	fprintf(cgiOut, "</select>\n"\
                         	"</td>"\
                         	"</tr>");
							int RuleTypechoice=0;
                         	cgiFormSelectSingle("RuleType", RuleType, 2, &RuleTypechoice, 0);
                         	switch(RuleTypechoice)
                         	{
                         		//*********change here*********
                         		case 0: showFlagL=0 ;  break;//standard
					//****************************
                         		case 1: showFlagL=1;//extended
                         	}

							fprintf(cgiOut,"<tr align=left>");
							fprintf(cgiOut,"<td align=left>%s:</td>",search(lcontrol,"action"));
							fprintf(cgiOut,"<td align=left colspan=2>");
							fprintf(cgiOut, "<select name=\"Action\" onchange=\"javascript:this.form.submit();\" style='width:110;height:auto'>");
							if(RuleTypechoice==1)
							{
								for(i=0;i<6;i++)
								if(strcmp(ActionType[i],select_b)==0)				/*显示上次选中的*/
									fprintf(cgiOut,"<option value=%s selected=selected>%s",ActionType[i],ActionType[i]);
								else				
									fprintf(cgiOut,"<option value=%s>%s",ActionType[i],ActionType[i]);
							}
							else if(RuleTypechoice==0)
							{
								for(i=0;i<4;i++)
								if(strcmp(ActionType[i],select_b)==0)				/*显示上次选中的*/
									fprintf(cgiOut,"<option value=%s selected=selected>%s",ActionType[i],ActionType[i]);
								else				
									fprintf(cgiOut,"<option value=%s>%s",ActionType[i],ActionType[i]);
							}

                         	fprintf(cgiOut, "</select>\n"\
                         	"</td>"\
                         	"</tr>");
                         	
							int ActionTypechoice=0;
							if(showFlagL==0)
							{
								cgiFormSelectSingle("Action", ActionType, 4, &ActionTypechoice, 0);
							}
							else
							{
								cgiFormSelectSingle("Action", ActionType, 6, &ActionTypechoice, 0);
							}
							switch(ActionTypechoice)
							{
								case 0 : showFlag=4;break; //showflag=4  permit operat
								case 1 :break;//deny
								case 2 :break;//trap to cpu
								case 3 : showFlag=5; break;//redirect
								case 4 : showFlag=2; break;//ingress-qos
								case 5 : showFlag=3; break;//egress-qos
								default : break;
							}
							                       	
							if( (showFlagL!=1)  && (showFlag!=2) &&  (showFlag!=3))
							{
								fprintf(cgiOut,"<tr align=left>");
                         						fprintf(cgiOut,"<td align=left>%s:</td>",search(lcontrol,"Protocol"));
									fprintf(cgiOut,"<td align=left colspan=2>");
    										fprintf(cgiOut,"<select name=\"Protocol\" onchange=\"javascript:this.form.submit();\" style='width:110;height:auto'>");
    						  		for(i=0;i<4;i++)
    						  		{
	    			          				if(strcmp(flavors[i],select_a)==0)              /*显示上次选中的*/
	    			          				{
	           	                						fprintf(cgiOut,"<option value=%s selected=selected>%s",flavors[i],flavors[i]);
									}
	    			          				else			
	    			          				{
	    			            						fprintf(cgiOut,"<option value=%s>%s",flavors[i],flavors[i]);
	    			          				}
    						  		}
                             	fprintf(cgiOut, "</select>\n");
                             	fprintf(cgiOut,"</td>"\
								
                         		"</tr>");
								
                         	}
                         	else if(showFlagL==1)
                         	{
                         		if(showFlag!=2&&showFlag!=3)
                         		{
                         		fprintf(cgiOut,"<tr align=left>");
                         		fprintf(cgiOut,"<td align=left>%s:</td>",search(lcontrol,"Protocol"));
								fprintf(cgiOut,"<td align=left colspan=2>");
    							fprintf(cgiOut, "<select name=\"ExProtocol\" onchange=\"javascript:this.form.submit();\" style='width:110;height:auto'>");
    						  	for(i=0;i<6;i++)
    						  	{
    			          		if(strcmp(flavorsLater[i],Exselect_a)==0)              /*显示上次选中的*/
	    			          			{
           	                		fprintf(cgiOut,"<option value=%s selected=selected>%s",flavorsLater[i],flavorsLater[i]);
	    			          			}
    			          		else			  	
	    			          			{
    			            		fprintf(cgiOut,"<option value=%s>%s",flavorsLater[i],flavorsLater[i]);
	    			          			}
    						  	}
                             	fprintf(cgiOut, "</select>\n");
                             	fprintf(cgiOut,"</td>"\
                         		"</tr>");
                         		}
                         	}
                         	else if(showFlag==2 || showFlag==3)
                         	{
                         		
                         	}
							

                         	fprintf(cgiOut,"<tr align=left>");
                         	fprintf(cgiOut,"<td align=left width=90>%s:</td>",search(lcontrol,"acl_index"));
							fprintf(cgiOut,"<td align=left width=110><input type=text name=ruleindex size=13 style='width:110;height:auto'></td>");
							if(showFlagL==1)
							{
								fprintf(cgiOut,"<td align=left style=color:red>&nbsp;&nbsp;&nbsp;%s</td>","(1-500)");
							}
							else
							{
								fprintf(cgiOut,"<td align=left style=color:red>&nbsp;&nbsp;&nbsp;%s</td>","(1-1000)");
							}
							fprintf(cgiOut,"</tr>");
							
							show_group_index(0,in_groupNum);
							show_group_index(1,e_groupNum);
							
                         	fprintf(cgiOut,"<tr align=left>");
                         	fprintf(cgiOut,"<td align=left>%s:</td>",search(lcontrol,"rule_group"));
							fprintf(cgiOut,"<td align=left colspan=2>");
							fprintf(cgiOut, "<select name=\"groupindex\" style='width:110;height:auto'>");
							fprintf(cgiOut,"<option value=%s>%s","NONE","NONE");
							for(i=0;i<1024;i++)
							{
								if(in_groupNum[i]!=0)
									{
										memset(in_groupNumTemp,0,20);
										sprintf(in_groupNumTemp,"%u-ingress",in_groupNum[i]);
			            				fprintf(cgiOut,"<option value=%s>%u",in_groupNumTemp,in_groupNum[i]);
			            			}
							}
			            	for(i=0;i<1024;i++)
			            			{
								if(e_groupNum[i]!=0)
			            				{
										memset(e_groupNumTemp,0,20);
										sprintf(e_groupNumTemp,"%u-egress",e_groupNum[i]);
			            				fprintf(cgiOut,"<option value=%s>%u",e_groupNumTemp,e_groupNum[i]);
			            				}
			            			}
	                     	fprintf(cgiOut, "</select>\n");
                         	fprintf(cgiOut,"</td>"\
                         	"</tr>");
                         	if(showFlag==2 || showFlag==4)
                         	{
                         		show_policer(receive_policer,&policer_num);
    					fprintf(cgiOut,"<tr align=left>");
                             		fprintf(cgiOut,"<td align=left>%s:</td>",search(lcontrol,"policy_index"));
						if(policer_num!=0)
						{
							fprintf(cgiOut,"<td>");
								fprintf(cgiOut,"<select name=policyindex style='width:%s;height:auto'>","100%");
								fprintf(cgiOut,"<option value=''>NONE</option>");		
								for(i=0;i<policer_num;i++)
								{
									fprintf(cgiOut,"<option value=%d>%d</option>",receive_policer[i].policer_index,receive_policer[i].policer_index);		
								}
								fprintf(cgiOut,"</select>");
	    						fprintf(cgiOut,"</td>");
						}
						else
						{
							fprintf(cgiOut,"<td height='35' align='center''><font color='red'>%s</font></td>",search(lcontrol,"no_policer"));
						}
						fprintf(cgiOut,"<td><font color='red'>&nbsp;&nbsp;&nbsp;(%s)</font></td>",search(lcontrol,"can_select"));
    					fprintf(cgiOut,"</tr>");
				}

                         	fprintf(cgiOut,"</table>"\
                         	"</td>"\
                         	"</tr>");


                      			switch(ActionTypechoice)
                      			{
                      				case 0:break;
                      				case 1:break;
                      				case 2:break;
                      				case 3:
                      				{
                                        fprintf(cgiOut,"<tr style=padding-top:20px>"\
                                		"<td><table width=620 border=0 cellspacing=0 cellpadding=0>"\
                                     	"<tr>"\
                                       "<td id=sec1 colspan=6 style=\"border-bottom:2px solid #53868b\">%s %s</td>",ActionType[ActionTypechoice],search(lsecu,"para"));
                                     	fprintf(cgiOut,"</tr>");
    
    									fprintf(cgiOut,"<tr style=padding-top:10px align=left>"\
    									"<td width=70 id=sec2>%s:</td>","PORTNO");
    									fprintf(cgiOut,"<td width=15><input type=text size=3 name=slot></td>");
    									fprintf(cgiOut,"<td style=padding-left:4px width=10 align=center>%s</td>","/");
    									fprintf(cgiOut,"<td style=padding-left:4px  align=left width=15><input type=text name=port size=3></td>"\
    									"<td width=510 style=padding-left:4px;color=red>(SLOT/PORT)</td>"\
    									"</tr>");
    									 fprintf(cgiOut,"</table>"\
               						 	"</td>"\
               							"</tr>");
                                 	}
                                 	break;
                                 	case 4:
                                 	{
						
                                 		fprintf(cgiOut,"<tr style=padding-top:20px>"\
                                		"<td><table width=620 border=0 cellspacing=0 cellpadding=0>"\
                                     	"<tr>"\
                                       "<td id=sec1 colspan=6 style=\"border-bottom:2px solid #53868b\">%s %s</td>",ActionType[ActionTypechoice],search(lsecu,"para"));
                                     	fprintf(cgiOut,"</tr>");

                                     	fprintf(cgiOut,"<tr style=padding-top:10px align=left>"\
    									"<td width=90 id=sec2>%s:</td>","Ingress-QOS");
    									
										if(qos_num!=0)
										{
											fprintf(cgiOut,"<td width=90 align=left>");
											fprintf(cgiOut,"<select name='qosindex' style='width:88;height:auto'>'");
											for(i=0;i<qos_num;i++)
											{
												fprintf(cgiOut,"<option value='%u'>%u</option>",receive_qos[i].profileindex,receive_qos[i].profileindex);
											}
											
											fprintf(cgiOut,"</select>");
											fprintf(cgiOut,"</td>");
    											fprintf(cgiOut,"<td align=left style=color:red width=110>%s</td>","(1-127)");
										}
										else
										{
											fprintf(cgiOut,"<td width=120 align='center' colspan='2'>");
												fprintf(cgiOut,"<font color=red>%s</font>",search(lcontrol,"no_qos_profile"));
											fprintf(cgiOut,"</td>");
    											//fprintf(cgiOut,"<td align=left style=color:red width=110>%s</td>","(1-127)");	
										}
									//fprintf(cgiOut,"</td>");
    									//fprintf(cgiOut,"<td align=left style=color:red width=110>%s</td>","(1-127)");
										fprintf(cgiOut,"<td width=120 id=sec2>%s:</td>","Sub-QOS-Markers");
    									fprintf(cgiOut,"<td width=210 align=left colspan=2><select name=sub_qos style='width:88;height:auto'>"\
    									"<option value=%s>%s","enable","enable");
    									fprintf(cgiOut,"<option value=%s>%s","disable","disable");
    									fprintf(cgiOut,"</select>"\
    									"</td></tr>");
    									
    									fprintf(cgiOut,"<tr style=padding-top:10px align=left>"\
    									"<td width=90 id=sec2>%s:</td>","Source-UP");
    									fprintf(cgiOut,"<td width=90 align=left><input type=text size=12 name=srcUP></td>");
    									fprintf(cgiOut,"<td align=left style=color:red width=120>%s</td>","(0-7)");
    									fprintf(cgiOut,"<td width=100 id=sec2>%s:</td>","Source-DSCP");
    									fprintf(cgiOut,"<td width=100 align=left><input type=text size=12 name=srcDSCP></td>");
    									fprintf(cgiOut,"<td align=left style=color:red width=120>%s</td>","(0-63)");

    									fprintf(cgiOut,"</tr></table>"\
               						 	"</td>"\
               							"</tr>");
                                 	}
                                 	break;
                                 	case 5:
                                 	{
                                 		fprintf(cgiOut,"<tr style=padding-top:20px>"\
                                		"<td><table width=620 border=0 cellspacing=0 cellpadding=0>"\
                                     	"<tr>"\
                                       "<td id=sec1 colspan=6 style=\"border-bottom:2px solid #53868b\">%s %s</td>",ActionType[ActionTypechoice],search(lsecu,"para"));
                                     	fprintf(cgiOut,"</tr>");

                                     	fprintf(cgiOut,"<tr style=padding-top:10px align=left>"\
    									"<td width=90 id=sec2>%s:</td>","Egress-UP");
    									fprintf(cgiOut,"<td width=90 align=left><input type=text size=12 name=egressUP></td>");
    									fprintf(cgiOut,"<td align=left style=color:red width=120>%s</td>","(0-7)");
    									fprintf(cgiOut,"<td width=100 id=sec2>%s:</td>","Egress-DSCP");
    									fprintf(cgiOut,"<td width=100 align=left><input type=text size=12 name=egressDSCP></td>");
    									fprintf(cgiOut,"<td align=left style=color:red width=120>%s</td>","(0-63)");
    									fprintf(cgiOut,"</tr>");
    									
    									fprintf(cgiOut,"<tr style=padding-top:10px align=left>"\
    									"<td width=90 id=sec2>%s:</td>","Source-UP");
    									fprintf(cgiOut,"<td width=90 align=left><input type=text size=12 name=srcUP></td>");
    									fprintf(cgiOut,"<td align=left style=color:red width=120>%s</td>","(0-7)");
    									fprintf(cgiOut,"<td width=100 id=sec2>%s:</td>","Source-DSCP");
    									fprintf(cgiOut,"<td width=100 align=left><input type=text size=12 name=srcDSCP></td>");
    									fprintf(cgiOut,"<td align=left style=color:red width=120>%s</td>","(0-63)");

    									fprintf(cgiOut,"</tr></table>"\
               						 	"</td>"\
               							"</tr>");
                                 	}
                                 	break;
                                 	
								}
                  ///////////////////////////////////////////Protocol//////////////////////////////////////////////
                  
                  if(showFlagL!=1 )
                  {
                  	int flavorChoices=0; 
                  	int invalid=0;	   
                  	cgiFormSelectSingle("Protocol", flavors, 6, &flavorChoices, invalid);
                    fprintf(cgiOut,"<tr style=padding-top:20px>"\
             		"<td><table width=620 border=0 cellspacing=0 cellpadding=0>"\
                  	"<tr>"\
                    "<td id=sec1 colspan=10 style=\"border-bottom:2px solid #53868b\">%s %s</td>",flavors[flavorChoices],search(lsecu,"para"));
                  fprintf(cgiOut,"</tr>");
				    switch(flavorChoices)
				    {
                      case 0:{                                    /*Protocol=ip*/
                      			fprintf(cgiOut,"<tr style=padding-top:10px>"\
                      			"<td width=200 id=sec2>%s:</td>",search(lcontrol,"source_IP"));
					  	       fprintf(cgiOut,"<td id=sec3>");				  	
         					   fprintf(cgiOut, "<select name=\"sourceIP\" onchange=changestate();>"\
						  		"<option value=\"any\">Any\n");
                         		fprintf(cgiOut, "<option value=\"Single\">Single IP\n");	      
               			       	fprintf(cgiOut,"</td>");
							    fprintf(cgiOut,"<td align=left>"\
							  "<div style=\"border-width:1;border-color:#a5acb2;border-style:solid;width:140;font-size:9pt\">");
							  fprintf(cgiOut,"<input type=text name=src_ip1 maxlength=3 class=a3 onKeyUp=\"mask(this,%s)\" onbeforepaste=mask_c()>.",search(lpublic,"ip_error"));
							  fprintf(cgiOut,"<input type=text name=src_ip2 maxlength=3 class=a3 onKeyUp=\"mask(this,%s)\" onbeforepaste=mask_c()>.",search(lpublic,"ip_error"));
							  fprintf(cgiOut,"<input type=text name=src_ip3 maxlength=3 class=a3 onKeyUp=\"mask(this,%s)\" onbeforepaste=mask_c()>.",search(lpublic,"ip_error"));
							  fprintf(cgiOut,"<input type=text name=src_ip4 maxlength=3 class=a3 onKeyUp=\"mask(this,%s)\" onbeforepaste=mask_c()>",search(lpublic,"ip_error"));
							  fprintf(cgiOut,"</div>");
							  fprintf(cgiOut,"</td>");
							  fprintf(cgiOut,"<td width=10 style=padding-left:2px align=center>%s</td>","/");
    						fprintf(cgiOut,"<td  align=left style=padding-left:2px  width=15><input type=text name=smasklen size=3></td>");

							fprintf(cgiOut,"<td width=200 id=sec2 style=padding-left:10px>%s:</td>",search(lcontrol,"destination_IP"));
							fprintf(cgiOut,"<td id=sec3>");				  	
         					   fprintf(cgiOut, "<select name=\"dstIP\" onchange=changestate();>"\
						  		"<option value=\"any\">Any\n");
                         		fprintf(cgiOut, "<option value=\"Single\">Single IP\n");	      
               			       	fprintf(cgiOut,"</td>");
							    fprintf(cgiOut,"<td align=left>"\
							  "<div style=\"border-width:1;border-color:#a5acb2;border-style:solid;width:140;font-size:9pt\">");
							  fprintf(cgiOut,"<input type=text name=dst_ip1 maxlength=3 class=a3 onKeyUp=\"mask(this,%s)\" onbeforepaste=mask_c()>.",search(lpublic,"ip_error"));
							  fprintf(cgiOut,"<input type=text name=dst_ip2 maxlength=3 class=a3 onKeyUp=\"mask(this,%s)\" onbeforepaste=mask_c()>.",search(lpublic,"ip_error"));
							  fprintf(cgiOut,"<input type=text name=dst_ip3 maxlength=3 class=a3 onKeyUp=\"mask(this,%s)\" onbeforepaste=mask_c()>.",search(lpublic,"ip_error"));
							  fprintf(cgiOut,"<input type=text name=dst_ip4 maxlength=3 class=a3 onKeyUp=\"mask(this,%s)\" onbeforepaste=mask_c()>",search(lpublic,"ip_error"));
							  fprintf(cgiOut,"</div>");
							  fprintf(cgiOut,"</td>");
							  fprintf(cgiOut,"<td width=10 style=padding-left:2px align=center>%s</td>","/");
    						fprintf(cgiOut,"<td  align=left style=padding-left:2px  width=15><input type=text name=dmasklen size=3></td>"\
							  "</tr>");
					         }
					  	     break; 
					  case 1:
					  case 2:
					  {                                    /*Protocol=tcp   or  Protocol=udp*/
					  			fprintf(cgiOut,"<tr style=padding-top:10px>"\
                               "<td width=200 id=sec2>%s:</td>",search(lcontrol,"source_IP"));
					  	       fprintf(cgiOut,"<td id=sec3  align=left>");				  	
         					   fprintf(cgiOut, "<select name=\"sourceIP\" onchange=changestate();>"\
						  		"<option value=\"any\">Any\n");
                         		fprintf(cgiOut, "<option value=\"Single\">Single IP\n");	      
               			       	fprintf(cgiOut,"</td>");
							    fprintf(cgiOut,"<td align=left>"\
							  "<div style=\"border-width:1;border-color:#a5acb2;border-style:solid;width:140;font-size:9pt\">");
							  fprintf(cgiOut,"<input type=text name=src_ip1 maxlength=3 class=a3 onKeyUp=\"mask(this,%s)\" onbeforepaste=mask_c()>.",search(lpublic,"ip_error"));
							  fprintf(cgiOut,"<input type=text name=src_ip2 maxlength=3 class=a3 onKeyUp=\"mask(this,%s)\" onbeforepaste=mask_c()>.",search(lpublic,"ip_error"));
							  fprintf(cgiOut,"<input type=text name=src_ip3 maxlength=3 class=a3 onKeyUp=\"mask(this,%s)\" onbeforepaste=mask_c()>.",search(lpublic,"ip_error"));
							  fprintf(cgiOut,"<input type=text name=src_ip4 maxlength=3 class=a3 onKeyUp=\"mask(this,%s)\" onbeforepaste=mask_c()>",search(lpublic,"ip_error"));
							  fprintf(cgiOut,"</div>");
							  fprintf(cgiOut,"</td>");
							  fprintf(cgiOut,"<td width=10 style=padding-left:2px align=center>%s</td>","/");
    							fprintf(cgiOut,"<td  align=left style=padding-left:2px  width=15><input type=text name=smasklen size=3></td>");

							fprintf(cgiOut,"<td width=200 id=sec2 style=padding-left:10px>%s:</td>",search(lcontrol,"destination_IP"));
							fprintf(cgiOut,"<td id=sec3 align=left>");				  	
         					   fprintf(cgiOut, "<select name=\"dstIP\" onchange=changestate();>"\
						  		"<option value=\"any\">Any\n");
                         		fprintf(cgiOut, "<option value=\"Single\">Single IP\n");	      
               			       	fprintf(cgiOut,"</td>");
							    fprintf(cgiOut,"<td align=left>"\
							  "<div style=\"border-width:1;border-color:#a5acb2;border-style:solid;width:140;font-size:9pt\">");
							  fprintf(cgiOut,"<input type=text name=dst_ip1 maxlength=3 class=a3 onKeyUp=\"mask(this,%s)\" onbeforepaste=mask_c()>.",search(lpublic,"ip_error"));
							  fprintf(cgiOut,"<input type=text name=dst_ip2 maxlength=3 class=a3 onKeyUp=\"mask(this,%s)\" onbeforepaste=mask_c()>.",search(lpublic,"ip_error"));
							  fprintf(cgiOut,"<input type=text name=dst_ip3 maxlength=3 class=a3 onKeyUp=\"mask(this,%s)\" onbeforepaste=mask_c()>.",search(lpublic,"ip_error"));
							  fprintf(cgiOut,"<input type=text name=dst_ip4 maxlength=3 class=a3 onKeyUp=\"mask(this,%s)\" onbeforepaste=mask_c()>",search(lpublic,"ip_error"));
							  fprintf(cgiOut,"</div>");
							  fprintf(cgiOut,"</td>");
							  fprintf(cgiOut,"<td width=10 style=padding-left:2px align=center>%s</td>","/");
    						fprintf(cgiOut,"<td  align=left style=padding-left:2px  width=15><input type=text name=dmasklen size=3></td>"\
							  "</tr>");

							  fprintf(cgiOut,"<tr style=padding-top:10px>"\
							  "<td width=200 id=sec2>%s:</td>",search(lcontrol,"source_port"));
					  	       fprintf(cgiOut,"<td id=sec3 align=left>");				  	
         					   fprintf(cgiOut, "<select name=\"sourcePort\" onchange=changestate();>"\
						  		"<option value=\"any\">Any\n");
                         		fprintf(cgiOut, "<option value=\"Single\">Single Port\n");	      
               			       	fprintf(cgiOut,"</td>");
							    fprintf(cgiOut,"<td colspan=3><input type=text name=srcPort size=12></td>");
							fprintf(cgiOut,"<td width=200 id=sec2 style=padding-left:10px>%s:</td>",search(lcontrol,"destination_port"));
							fprintf(cgiOut,"<td id=sec3 align=left>");				  	
         					   fprintf(cgiOut, "<select name=\"dstPort\" onchange=changestate();>"\
						  		"<option value=\"any\">Any\n");
                         		fprintf(cgiOut, "<option value=\"Single\">Single Port\n");	      
               			       	fprintf(cgiOut,"</td>");
							    fprintf(cgiOut,"<td colspan=3><input type=text name=DstPort size=12></td>"\
							  "</tr>");
					  	     }           
					         break; 
					  case 3:{                                    /*Protocol=icmp*/
					  	       fprintf(cgiOut,"<tr style=padding-top:10px>"\
                               "<td width=200 id=sec2>%s:</td>",search(lcontrol,"source_IP"));
					  	       fprintf(cgiOut,"<td id=sec3  align=left>");				  	
         					   fprintf(cgiOut, "<select name=\"sourceIP\" onchange=changestate();>"\
						  		"<option value=\"any\">Any\n");
                         		fprintf(cgiOut, "<option value=\"Single\">Single IP\n");	      
               			       	fprintf(cgiOut,"</td>");
							    fprintf(cgiOut,"<td align=left>"\
							  "<div style=\"border-width:1;border-color:#a5acb2;border-style:solid;width:140;font-size:9pt\">");
							  fprintf(cgiOut,"<input type=text name=src_ip1 maxlength=3 class=a3 onKeyUp=\"mask(this,%s)\" onbeforepaste=mask_c()>.",search(lpublic,"ip_error"));
							  fprintf(cgiOut,"<input type=text name=src_ip2 maxlength=3 class=a3 onKeyUp=\"mask(this,%s)\" onbeforepaste=mask_c()>.",search(lpublic,"ip_error"));
							  fprintf(cgiOut,"<input type=text name=src_ip3 maxlength=3 class=a3 onKeyUp=\"mask(this,%s)\" onbeforepaste=mask_c()>.",search(lpublic,"ip_error"));
							  fprintf(cgiOut,"<input type=text name=src_ip4 maxlength=3 class=a3 onKeyUp=\"mask(this,%s)\" onbeforepaste=mask_c()>",search(lpublic,"ip_error"));
							  fprintf(cgiOut,"</div>");
							  fprintf(cgiOut,"</td>");
							  fprintf(cgiOut,"<td width=10 style=padding-left:2px align=center>%s</td>","/");
    						fprintf(cgiOut,"<td  align=left style=padding-left:2px  width=15><input type=text name=smasklen size=3></td>");

							fprintf(cgiOut,"<td width=200 id=sec2 style=padding-left:10px>%s:</td>",search(lcontrol,"destination_IP"));
							fprintf(cgiOut,"<td id=sec3 align=left>");				  	
         					   fprintf(cgiOut, "<select name=\"dstIP\" onchange=changestate();>"\
						  		"<option value=\"any\">Any\n");
                         		fprintf(cgiOut, "<option value=\"Single\">Single IP\n");	      
               			       	fprintf(cgiOut,"</td>");
							    fprintf(cgiOut,"<td align=left>"\
							  "<div style=\"border-width:1;border-color:#a5acb2;border-style:solid;width:140;font-size:9pt\">");
							  fprintf(cgiOut,"<input type=text name=dst_ip1 maxlength=3 class=a3 onKeyUp=\"mask(this,%s)\" onbeforepaste=mask_c()>.",search(lpublic,"ip_error"));
							  fprintf(cgiOut,"<input type=text name=dst_ip2 maxlength=3 class=a3 onKeyUp=\"mask(this,%s)\" onbeforepaste=mask_c()>.",search(lpublic,"ip_error"));
							  fprintf(cgiOut,"<input type=text name=dst_ip3 maxlength=3 class=a3 onKeyUp=\"mask(this,%s)\" onbeforepaste=mask_c()>.",search(lpublic,"ip_error"));
							  fprintf(cgiOut,"<input type=text name=dst_ip4 maxlength=3 class=a3 onKeyUp=\"mask(this,%s)\" onbeforepaste=mask_c()>",search(lpublic,"ip_error"));
							  fprintf(cgiOut,"</div>");
							  fprintf(cgiOut,"</td>");
							  fprintf(cgiOut,"<td width=10 style=padding-left:2px align=center>%s</td>","/");
    						fprintf(cgiOut,"<td  align=left style=padding-left:2px  width=15><input type=text name=dmasklen size=3></td>"\
							  "</tr>");

								fprintf(cgiOut,"<tr style=padding-top:10px align=left>"\
							  "<td width=70 id=sec2>%s:</td>",search(lcontrol,"icmp_type"));
							    fprintf(cgiOut,"<td width=20 colspan=4><input type=text name=icmp_type size=6></td>");
								fprintf(cgiOut,"<td width=70 id=sec2 style=padding-left:10px align=right>%s:</td>",search(lcontrol,"icmp_code"));
							    fprintf(cgiOut,"<td width=460 colspan=2><input type=text name=icmp_code size=6></td>"\
							  "</tr>");
							  
					         }
					  	      break;                                     
					  case 4:{                                   /*Protocol=arp*/
					  	       fprintf(cgiOut,"<tr style=padding-top:10px>"\
							  "<td width=70 id=sec2>%s:</td>",search(lcontrol,"acl_smac"));
					  	       fprintf(cgiOut,"<td id=sec3 align=left>");				  	
         					   fprintf(cgiOut, "<select name=\"srcMac\" onchange=changestate();>"\
						  		"<option value=\"any\">Any\n");
                         		fprintf(cgiOut, "<option value=\"Single\">Single Mac\n");	      
               			       	fprintf(cgiOut,"</td>");
							    fprintf(cgiOut,"<td><input type=text name=srcmac size=21></td>");
    							fprintf(cgiOut,"<td width=70 id=sec2 style=padding-left:10px>%s:</td>",search(lcontrol,"vID"));
    							fprintf(cgiOut,"<td><input type=text name=vlanID size=4></td>");
    							fprintf(cgiOut,"<td width=70 id=sec2 style=padding-left:10px>%s:</td>",search(lcontrol,"source_slot_port"));
    				    		fprintf(cgiOut,"<td width=15><input type=text size=3 name=sourceslot></td>");
    							fprintf(cgiOut,"<td style=padding-left:2px width=10 align=center>%s</td>","/");
    							fprintf(cgiOut,"<td style=padding-left:2px  align=left width=15><input type=text name=sourceport size=3></td>"\
    							"</tr>");
    							fprintf(cgiOut,"<tr style=padding-top:10px>"\
                        		"<td id=sec1 colspan=10 style=\"border-bottom:2px solid #53868b\">%s</td>",search(lpublic,"description"));
                      			fprintf(cgiOut,"</tr>");
                      			fprintf(cgiOut,"<tr>"\
                      			"<td colspan=10>Mac Address Format (For Example 00:FF:23:44:55:AA)</td>"\
                      			"</tr>");
					         }
					  	      break;
					  case 5:{                                    /*Protocol=ethernet*/
					  	       fprintf(cgiOut,"<tr style=padding-top:10px>"\
							  "<td width=70 id=sec2>%s:</td>",search(lcontrol,"acl_smac"));
					  	       fprintf(cgiOut,"<td id=sec3 align=left>");				  	
         					   fprintf(cgiOut, "<select name=\"srcMac\" onchange=changestate();>"\
						  		"<option value=\"any\">Any\n");
                         		fprintf(cgiOut, "<option value=\"Single\">Single Mac\n");	      
               			       	fprintf(cgiOut,"</td>");
							    fprintf(cgiOut,"<td><input type=text name=srcmac size=21></td>");
     							fprintf(cgiOut,"<td width=70 id=sec2 style=padding-left:10px>%s:</td>",search(lcontrol,"acl_dmac"));
     							fprintf(cgiOut,"<td id=sec3 align=left>");				  	
         					    fprintf(cgiOut, "<select name=\"dstMac\" onchange=changestate();>"\
						  		"<option value=\"any\">Any\n");
                         		fprintf(cgiOut, "<option value=\"Single\">Single Mac\n");	      
               			       	fprintf(cgiOut,"</td>");
							    fprintf(cgiOut,"<td><input type=text name=dstmac size=21></td>"\
							  	"</tr>");
							    fprintf(cgiOut,"<tr style=padding-top:10px>"\
                        		"<td id=sec1 colspan=10 style=\"border-bottom:2px solid #53868b\">%s</td>",search(lpublic,"description"));
                      			fprintf(cgiOut,"</tr>");
                      			fprintf(cgiOut,"<tr>"\
                      			"<td colspan=10>Mac Address Format (For Example 00:FF:23:44:55:AA)</td>"\
                      			"</tr>");
					         }
					  	      break;
					}
					      fprintf(cgiOut,"</table>"\
                       "</td>"\
                      "</tr>");
				}



			

			//*****************************new code***********************************************
			  ///////////////////////////////////////////Protocol//////////////////////////////////////////////
                  if(showFlagL==1 && showFlag!=2 && showFlag!=3)
                  {
                  	int ExActionTypechoice=0; 
                  	int invalid=0;	   
                  	cgiFormSelectSingle("ExProtocol", flavorsLater, 6, &ExActionTypechoice, invalid);
			
                    fprintf(cgiOut,"<tr style=padding-top:20px>"\
             		"<td><table width=620 border=0 cellspacing=0 cellpadding=0>"\
                  	"<tr>");
			if(ExActionTypechoice!=1&&ExActionTypechoice!=2)
			{
                    		fprintf(cgiOut,"<td id=sec1 colspan=10 style=\"border-bottom:2px solid #53868b\">%s %s</td>",flavors[ExActionTypechoice],search(lsecu,"para"));
			}
                  fprintf(cgiOut,"</tr>");
		
				    switch(ExActionTypechoice)
				    {
                      case 0:{                                    /*Protocol=ip*/
                      			fprintf(cgiOut,"<tr style=padding-top:10px>"\
                      			"<td width=200 id=sec2>%s:</td>",search(lcontrol,"source_IP"));
					  	       fprintf(cgiOut,"<td id=sec3>");				  	
         					   fprintf(cgiOut, "<select name=\"sourceIP\" onchange=changestate();>"\
						  		"<option value=\"any\">Any\n");
                         		fprintf(cgiOut, "<option value=\"Single\">Single IP\n");	      
               			       	fprintf(cgiOut,"</td>");
							    fprintf(cgiOut,"<td align=left>"\
							  "<div style=\"border-width:1;border-color:#a5acb2;border-style:solid;width:140;font-size:9pt\">");
							  fprintf(cgiOut,"<input type=text name=src_ip1 maxlength=3 class=a3 onKeyUp=\"mask(this,%s)\" onbeforepaste=mask_c()>.",search(lpublic,"ip_error"));
							  fprintf(cgiOut,"<input type=text name=src_ip2 maxlength=3 class=a3 onKeyUp=\"mask(this,%s)\" onbeforepaste=mask_c()>.",search(lpublic,"ip_error"));
							  fprintf(cgiOut,"<input type=text name=src_ip3 maxlength=3 class=a3 onKeyUp=\"mask(this,%s)\" onbeforepaste=mask_c()>.",search(lpublic,"ip_error"));
							  fprintf(cgiOut,"<input type=text name=src_ip4 maxlength=3 class=a3 onKeyUp=\"mask(this,%s)\" onbeforepaste=mask_c()>",search(lpublic,"ip_error"));
							  fprintf(cgiOut,"</div>");
							  fprintf(cgiOut,"</td>");
							  fprintf(cgiOut,"<td width=10 style=padding-left:2px align=center>%s</td>","/");
    						fprintf(cgiOut,"<td  align=left style=padding-left:2px  width=15><input type=text name=smasklen size=3></td>");

							fprintf(cgiOut,"<td width=200 id=sec2 style=padding-left:10px>%s:</td>",search(lcontrol,"destination_IP"));
							fprintf(cgiOut,"<td id=sec3>");				  	
         					   fprintf(cgiOut, "<select name=\"dstIP\" onchange=changestate();>"\
						  		"<option value=\"any\">Any\n");
                         		fprintf(cgiOut, "<option value=\"Single\">Single IP\n");	      
               			       	fprintf(cgiOut,"</td>");
							    fprintf(cgiOut,"<td align=left>"\
							  "<div style=\"border-width:1;border-color:#a5acb2;border-style:solid;width:140;font-size:9pt\">");
							  fprintf(cgiOut,"<input type=text name=dst_ip1 maxlength=3 class=a3 onKeyUp=\"mask(this,%s)\" onbeforepaste=mask_c()>.",search(lpublic,"ip_error"));
							  fprintf(cgiOut,"<input type=text name=dst_ip2 maxlength=3 class=a3 onKeyUp=\"mask(this,%s)\" onbeforepaste=mask_c()>.",search(lpublic,"ip_error"));
							  fprintf(cgiOut,"<input type=text name=dst_ip3 maxlength=3 class=a3 onKeyUp=\"mask(this,%s)\" onbeforepaste=mask_c()>.",search(lpublic,"ip_error"));
							  fprintf(cgiOut,"<input type=text name=dst_ip4 maxlength=3 class=a3 onKeyUp=\"mask(this,%s)\" onbeforepaste=mask_c()>",search(lpublic,"ip_error"));
							  fprintf(cgiOut,"</div>");
							  fprintf(cgiOut,"</td>");
							  fprintf(cgiOut,"<td width=10 style=padding-left:2px align=center>%s</td>","/");
    						fprintf(cgiOut,"<td  align=left style=padding-left:2px  width=15><input type=text name=dmasklen size=3></td>"\
							  "</tr>");
					         }
					  	     break; 
					  case 1:
					  case 2:
					  {
                         			showFlagL=1;//此时下面的显示无效
                         			fprintf(cgiOut,"<tr style=padding-top:20px>"\
                            		"<td><table width=620 border=0 cellspacing=0 cellpadding=0>"\
                                 	"<tr>"\
                                   "<td id=sec1 colspan=12 style=\"border-bottom:2px solid #53868b\">%s %s</td>",RuleType[RuleTypechoice],search(lsecu,"para"));
                                 	fprintf(cgiOut,"</tr>");

                                      fprintf(cgiOut,"<tr style=padding-top:10px>"\
                                    "<td width=200 id=sec2>%s:</td>",search(lcontrol,"source_IP"));
     					  	       fprintf(cgiOut,"<td id=sec3  align=left>");				  	
              					   fprintf(cgiOut, "<select name=\"sourceIP\" onchange=changestate();>"\
     						  		"<option value=\"any\">Any\n");
                              		fprintf(cgiOut, "<option value=\"Single\">Single IP\n");	      
                    			       	fprintf(cgiOut,"</td>");
     							    fprintf(cgiOut,"<td align=left>"\
     							  "<div id=sip style=\"border-width:1;border-color:#a5acb2;border-style:solid;width:140;font-size:9pt\">");
     							  fprintf(cgiOut,"<input type=text name=src_ip1 maxlength=3 class=a3 onKeyUp=\"mask(this,%s)\" onbeforepaste=mask_c()>.",search(lpublic,"ip_error"));
     							  fprintf(cgiOut,"<input type=text name=src_ip2 maxlength=3 class=a3 onKeyUp=\"mask(this,%s)\" onbeforepaste=mask_c()>.",search(lpublic,"ip_error"));
     							  fprintf(cgiOut,"<input type=text name=src_ip3 maxlength=3 class=a3 onKeyUp=\"mask(this,%s)\" onbeforepaste=mask_c()>.",search(lpublic,"ip_error"));
     							  fprintf(cgiOut,"<input type=text name=src_ip4 maxlength=3 class=a3 onKeyUp=\"mask(this,%s)\" onbeforepaste=mask_c()>",search(lpublic,"ip_error"));
     							  fprintf(cgiOut,"</div>");
     							  fprintf(cgiOut,"</td>");
     							  fprintf(cgiOut,"<td width=10 style=padding-left:2px align=center>%s</td>","/");
    								fprintf(cgiOut,"<td  align=left style=padding-left:2px  width=15><input type=text name=smasklen size=3></td>");
     
     							fprintf(cgiOut,"<td width=200 id=sec2 style=padding-left:10px colspan=2>%s:</td>",search(lcontrol,"destination_IP"));
     							fprintf(cgiOut,"<td id=sec3 align=left>");				  	
              					   fprintf(cgiOut, "<select name=\"dstIP\" onchange=changestate();>"\
     						  		"<option value=\"any\">Any\n");
                              		fprintf(cgiOut, "<option value=\"Single\">Single IP\n");	      
                    			       	fprintf(cgiOut,"</td>");
     							    fprintf(cgiOut,"<td align=left>"\
     							  "<div style=\"border-width:1;border-color:#a5acb2;border-style:solid;width:140;font-size:9pt\">");
     							  fprintf(cgiOut,"<input type=text name=dst_ip1 maxlength=3 class=a3 onKeyUp=\"mask(this,%s)\" onbeforepaste=mask_c()>.",search(lpublic,"ip_error"));
     							  fprintf(cgiOut,"<input type=text name=dst_ip2 maxlength=3 class=a3 onKeyUp=\"mask(this,%s)\" onbeforepaste=mask_c()>.",search(lpublic,"ip_error"));
     							  fprintf(cgiOut,"<input type=text name=dst_ip3 maxlength=3 class=a3 onKeyUp=\"mask(this,%s)\" onbeforepaste=mask_c()>.",search(lpublic,"ip_error"));
     							  fprintf(cgiOut,"<input type=text name=dst_ip4 maxlength=3 class=a3 onKeyUp=\"mask(this,%s)\" onbeforepaste=mask_c()>",search(lpublic,"ip_error"));
     							  fprintf(cgiOut,"</div>");
     							  fprintf(cgiOut,"</td>");
     							  fprintf(cgiOut,"<td width=10 align=center>%s</td>","/");
    							fprintf(cgiOut,"<td  align=left width=15><input type=text name=dmasklen size=3></td>"\
     							  "</tr>");
     
     							  fprintf(cgiOut,"<tr style=padding-top:10px>"\
     							  "<td width=200 id=sec2>%s:</td>",search(lcontrol,"source_port"));
     					  	       fprintf(cgiOut,"<td id=sec3 align=left>");				  	
              					   fprintf(cgiOut, "<select name=\"sourcePort\" onchange=changestate();>"\
     						  		"<option value=\"any\">Any\n");
                              		fprintf(cgiOut, "<option value=\"Single\">Single Port\n");	      
                    			       	fprintf(cgiOut,"</td>");
     							    fprintf(cgiOut,"<td colspan=3><input type=text name=srcPort size=12></td>");
     							fprintf(cgiOut,"<td width=200 id=sec2 style=padding-left:10px colspan=2>%s:</td>",search(lcontrol,"destination_port"));
     							fprintf(cgiOut,"<td id=sec3 align=left>");				  	
              					   fprintf(cgiOut, "<select name=\"dstPort\" onchange=changestate();>"\
     						  		"<option value=\"any\">Any\n");
                              		fprintf(cgiOut, "<option value=\"Single\">Single Port\n");	      
                    			       	fprintf(cgiOut,"</td>");
     							    fprintf(cgiOut,"<td colspan=3><input type=text name=DstPort size=12></td>"\
     							  "</tr>");

     							  fprintf(cgiOut,"<tr style=padding-top:10px>"\
    							  "<td width=200 id=sec2>%s:</td>",search(lcontrol,"acl_smac"));
    					  	       fprintf(cgiOut,"<td id=sec3 align=left>");				  	
             					   fprintf(cgiOut, "<select name=\"srcMac\" onchange=changestate();>"\
    						  		"<option value=\"any\">Any\n");
                             		fprintf(cgiOut, "<option value=\"Single\">Single Mac\n");	      
                   			       	fprintf(cgiOut,"</td>");
    							    fprintf(cgiOut,"<td colspan=3><input type=text name=srcmac size=21></td>");
									fprintf(cgiOut,"<td width=70 id=sec2 style=padding-left:10px colspan=2>%s:</td>",search(lcontrol,"acl_dmac"));
         							fprintf(cgiOut,"<td id=sec3 align=left>");				  	
             					    fprintf(cgiOut, "<select name=\"dstMac\" onchange=changestate();>"\
    						  		"<option value=\"any\">Any\n");
                             		fprintf(cgiOut, "<option value=\"Single\">Single Mac\n");	      
                   			       	fprintf(cgiOut,"</td>");
    							    fprintf(cgiOut,"<td colspan=3><input type=text name=dstmac size=21></td>"\
    							  	"</tr>");

    							    fprintf(cgiOut,"<tr>");
        							fprintf(cgiOut,"<td width=70 id=sec2>%s:</td>",search(lcontrol,"vID"));
        							fprintf(cgiOut,"<td align=left><input type=text name=vlanID size=4></td>");
        							fprintf(cgiOut,"<td width=70 id=sec2 style=padding-left:50px colspan=2>%s:</td>",search(lcontrol,"source_slot_port"));
        				    		fprintf(cgiOut,"<td width=15><input type=text size=3 name=sourceslot></td>");
        							fprintf(cgiOut,"<td width=2 align=left>%s</td>","/");
        							fprintf(cgiOut,"<td align=left width=15><input type=text name=sourceport size=3></td>"\
        							"<td colspan=4>&nbsp;</td>"\
        							  "</tr>");
     							  fprintf(cgiOut,"</table>"\
                       				"</td>"\
                      				"</tr>");
                         		}           
					         break; 
					  case 3:{                                    /*Protocol=icmp*/
					  	       fprintf(cgiOut,"<tr style=padding-top:10px>"\
                               "<td width=200 id=sec2>%s:</td>",search(lcontrol,"source_IP"));
					  	       fprintf(cgiOut,"<td id=sec3  align=left>");				  	
         					   fprintf(cgiOut, "<select name=\"sourceIP\" onchange=changestate();>"\
						  		"<option value=\"any\">Any\n");
                         		fprintf(cgiOut, "<option value=\"Single\">Single IP\n");	      
               			       	fprintf(cgiOut,"</td>");
							    fprintf(cgiOut,"<td align=left>"\
							  "<div style=\"border-width:1;border-color:#a5acb2;border-style:solid;width:140;font-size:9pt\">");
							  fprintf(cgiOut,"<input type=text name=src_ip1 maxlength=3 class=a3 onKeyUp=\"mask(this,%s)\" onbeforepaste=mask_c()>.",search(lpublic,"ip_error"));
							  fprintf(cgiOut,"<input type=text name=src_ip2 maxlength=3 class=a3 onKeyUp=\"mask(this,%s)\" onbeforepaste=mask_c()>.",search(lpublic,"ip_error"));
							  fprintf(cgiOut,"<input type=text name=src_ip3 maxlength=3 class=a3 onKeyUp=\"mask(this,%s)\" onbeforepaste=mask_c()>.",search(lpublic,"ip_error"));
							  fprintf(cgiOut,"<input type=text name=src_ip4 maxlength=3 class=a3 onKeyUp=\"mask(this,%s)\" onbeforepaste=mask_c()>",search(lpublic,"ip_error"));
							  fprintf(cgiOut,"</div>");
							  fprintf(cgiOut,"</td>");
							  fprintf(cgiOut,"<td width=10 style=padding-left:2px align=center>%s</td>","/");
    						fprintf(cgiOut,"<td  align=left style=padding-left:2px  width=15><input type=text name=smasklen size=3></td>");

							fprintf(cgiOut,"<td width=200 id=sec2 style=padding-left:10px>%s:</td>",search(lcontrol,"destination_IP"));
							fprintf(cgiOut,"<td id=sec3 align=left>");				  	
         					   fprintf(cgiOut, "<select name=\"dstIP\" onchange=changestate();>"\
						  		"<option value=\"any\">Any\n");
                         		fprintf(cgiOut, "<option value=\"Single\">Single IP\n");	      
               			       	fprintf(cgiOut,"</td>");
							    fprintf(cgiOut,"<td align=left>"\
							  "<div style=\"border-width:1;border-color:#a5acb2;border-style:solid;width:140;font-size:9pt\">");
							  fprintf(cgiOut,"<input type=text name=dst_ip1 maxlength=3 class=a3 onKeyUp=\"mask(this,%s)\" onbeforepaste=mask_c()>.",search(lpublic,"ip_error"));
							  fprintf(cgiOut,"<input type=text name=dst_ip2 maxlength=3 class=a3 onKeyUp=\"mask(this,%s)\" onbeforepaste=mask_c()>.",search(lpublic,"ip_error"));
							  fprintf(cgiOut,"<input type=text name=dst_ip3 maxlength=3 class=a3 onKeyUp=\"mask(this,%s)\" onbeforepaste=mask_c()>.",search(lpublic,"ip_error"));
							  fprintf(cgiOut,"<input type=text name=dst_ip4 maxlength=3 class=a3 onKeyUp=\"mask(this,%s)\" onbeforepaste=mask_c()>",search(lpublic,"ip_error"));
							  fprintf(cgiOut,"</div>");
							  fprintf(cgiOut,"</td>");
							  fprintf(cgiOut,"<td width=10 style=padding-left:2px align=center>%s</td>","/");
    						fprintf(cgiOut,"<td  align=left style=padding-left:2px  width=15><input type=text name=dmasklen size=3></td>"\
							  "</tr>");

								fprintf(cgiOut,"<tr style=padding-top:10px align=left>"\
							  "<td width=70 id=sec2>%s:</td>",search(lcontrol,"icmp_type"));
							    fprintf(cgiOut,"<td width=20 colspan=4><input type=text name=icmp_type size=6></td>");
								fprintf(cgiOut,"<td width=70 id=sec2 style=padding-left:10px align=right>%s:</td>",search(lcontrol,"icmp_code"));
							    fprintf(cgiOut,"<td width=460 colspan=2><input type=text name=icmp_code size=6></td>"\
							  "</tr>");
							  
					         }
					  	      break;                                     
					  case 4:{                                   /*Protocol=arp*/
					  	       fprintf(cgiOut,"<tr style=padding-top:10px>"\
							  "<td width=70 id=sec2>%s:</td>",search(lcontrol,"acl_smac"));
					  	       fprintf(cgiOut,"<td id=sec3 align=left>");				  	
         					   fprintf(cgiOut, "<select name=\"srcMac\" onchange=changestate();>"\
						  		"<option value=\"any\">Any\n");
                         		fprintf(cgiOut, "<option value=\"Single\">Single Mac\n");	      
               			       	fprintf(cgiOut,"</td>");
							    fprintf(cgiOut,"<td><input type=text name=srcmac size=21></td>");
    							fprintf(cgiOut,"<td width=70 id=sec2 style=padding-left:10px>%s:</td>",search(lcontrol,"vID"));
    							fprintf(cgiOut,"<td><input type=text name=vlanID size=4></td>");
    							fprintf(cgiOut,"<td width=70 id=sec2 style=padding-left:10px>%s:</td>",search(lcontrol,"source_slot_port"));
    				    		fprintf(cgiOut,"<td width=15><input type=text size=3 name=sourceslot></td>");
    							fprintf(cgiOut,"<td style=padding-left:2px width=10 align=center>%s</td>","/");
    							fprintf(cgiOut,"<td style=padding-left:2px  align=left width=15><input type=text name=sourceport size=3></td>"\
    							"</tr>");
    							fprintf(cgiOut,"<tr style=padding-top:10px>"\
                        		"<td id=sec1 colspan=10 style=\"border-bottom:2px solid #53868b\">%s</td>",search(lpublic,"description"));
                      			fprintf(cgiOut,"</tr>");
                      			fprintf(cgiOut,"<tr>"\
                      			"<td colspan=10>Mac Address Format (For Example 00:FF:23:44:55:AA)</td>"\
                      			"</tr>");
					         }
					  	      break;
					  case 5:{                                    /*Protocol=ethernet*/
					  	       fprintf(cgiOut,"<tr style=padding-top:10px>"\
							  "<td width=70 id=sec2>%s:</td>",search(lcontrol,"acl_smac"));
					  	       fprintf(cgiOut,"<td id=sec3 align=left>");				  	
         					   fprintf(cgiOut, "<select name=\"srcMac\" onchange=changestate();>"\
						  		"<option value=\"any\">Any\n");
                         		fprintf(cgiOut, "<option value=\"Single\">Single Mac\n");	      
               			       	fprintf(cgiOut,"</td>");
							    fprintf(cgiOut,"<td><input type=text name=srcmac size=21></td>");
     							fprintf(cgiOut,"<td width=70 id=sec2 style=padding-left:10px>%s:</td>",search(lcontrol,"acl_dmac"));
     							fprintf(cgiOut,"<td id=sec3 align=left>");				  	
         					    fprintf(cgiOut, "<select name=\"dstMac\" onchange=changestate();>"\
						  		"<option value=\"any\">Any\n");
                         		fprintf(cgiOut, "<option value=\"Single\">Single Mac\n");	      
               			       	fprintf(cgiOut,"</td>");
							    fprintf(cgiOut,"<td><input type=text name=dstmac size=21></td>"\
							  	"</tr>");
							    fprintf(cgiOut,"<tr style=padding-top:10px>"\
                        		"<td id=sec1 colspan=10 style=\"border-bottom:2px solid #53868b\">%s</td>",search(lpublic,"description"));
                      			fprintf(cgiOut,"</tr>");
                      			fprintf(cgiOut,"<tr>"\
                      			"<td colspan=10>Mac Address Format (For Example 00:FF:23:44:55:AA)</td>"\
                      			"</tr>");
					         }
					  	      break;
					}
					      fprintf(cgiOut,"</table>"\
                       "</td>"\
                      "</tr>");
				}
			//************************************************************************************





















			
       //////////////////////////////////////////////////////////////////////////////////////////
       						fprintf(cgiOut,"<tr>");
       						if(cgiFormSubmitClicked("submit_addrule") != cgiFormSuccess)
							  {
								fprintf(cgiOut,"<td><input type=hidden name=encry_addrule value=%s></td>",encry);
							  }
							  else if(cgiFormSubmitClicked("submit_addrule") == cgiFormSuccess)
							  {
								fprintf(cgiOut,"<td><input type=hidden name=encry_addrule value=%s></td>",encry);
							  }
							  fprintf(cgiOut,"</tr>");
								  fprintf(cgiOut,"</table>"\

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
free(encry);
free(text_index);
free(dstip);
free(srcip);
free(select_a);
free(Exselect_a);
free(select_b);
free(select_c);
free(smasklength);
free(dmasklength);
free(srcPort);
free(DstPort);
free(dstmac);
free(srcmac);
free(icmp_type);
free(icmp_code);
free(vlanID);
free(sourceslot);
free(sourceport);
free(sourceslotport);
free(sourceIP);
free(sourcePort);
free(dstIP);
free(dstPort);
free(srcMac);
free(dstMac);
free(groupindex);
free(in_groupNumTemp);
free(e_groupNumTemp);
free(policyindex);
free(qosindex);
free(sub_qos);
free(srcUP);
free(srcDSCP);
free(egressUP);
free(egressDSCP);
for(i=0;i<Policer_Num;i++)
  {

	  	free(receive_policer[i].policer_state);
	  	free(receive_policer[i].CounterState);
	  	free(receive_policer[i].Out_Profile_Action);
	  	free(receive_policer[i].Policer_Mode);
	  	free(receive_policer[i].Policing_Packet_Size);
 }

release(lpublic);  
release(lcontrol);
release(lsecu);
free(mode);
return 0;

}

int checkvalid(char * slotport)
{
	int i;
	char * switch7000[] = {
			"1/1","1/2","1/3","1/4","1/5","1/6","2/1","2/2","2/3","2/4","2/5","2/6","3/1","3/2","3/3","3/4","3/5","3/6","4/1","4/2","4/3","4/4","4/5","4/6"
			
	};
	
	char * switch5000[] = {
			"1/1","1/2","1/3","1/4","1/5","1/6","1/7","1/8","1/9","1/10","1/11","1/12","1/13","1/14","1/15","1/16","1/17","1/18","1/19","1/20","1/21","1/22","1/23","1/24",
			
	};
	if(0==strcmp(slotport,"/"))
		{
			return -1;
		}

 	char * productId;
 	productId=readproductID();
 	if(0==strcmp(productId,"Switch7000"))
 	{
 		for(i=0;i<24;i++)
 		{
 			if(strcmp(slotport,switch7000[i])==0)
 				return 0;
 		}
 		return 1;
 	}
 	else if(0==strcmp(productId,"Switch5000"))
 	{
 		for(i=0;i<24;i++)
 		{
 			if(strcmp(slotport,switch5000[i])==0)
 				return 0;
 		}
 		return 1;
 	}
	return 1;
}

