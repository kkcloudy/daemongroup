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
* wp_port_flow.c
*
*
* CREATOR:
* autelan.software.Network Dep. team
* zhouym@autelan.com
*
* DESCRIPTION:
* system infos 
*
*
*******************************************************************************/
#ifdef __cplusplus
extern "C"{
#endif

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <sys/types.h>
#include <unistd.h>
#include "cgic.h"
//#include "ws_usrinfo.h"
//#include "ws_err.h"
//#include "ws_ec.h"
//#include "ws_nm_status.h"
#include "ws_dcli_portconf.h"

#define MAX_LEN 1024

// show eth-port 0-1 statistics 命令
static void get_xml_mem_new(char *str_xml)
{
		DBusMessage *query, *reply;
		DBusError err;
		DBusMessageIter  iter;
		DBusMessageIter  iter_array;
		unsigned char slot_count,type=0;
		int i,value=0;

	   port_flow  pf;
	   memset(&pf,0,sizeof(pf));	
	 
	   char  aaa[64];	 
	   ccgi_dbus_init();     //初始化才可以取值    
	 
	
		query = dbus_message_new_method_call(
									NPD_DBUS_BUSNAME,		\
									NPD_DBUS_ETHPORTS_OBJPATH,		\
									NPD_DBUS_ETHPORTS_INTERFACE,		\
									NPD_DBUS_ETHPORTS_INTERFACE_METHOD_SHOW_ETHPORT_LIST);	
		dbus_error_init(&err);
		reply = dbus_connection_send_with_reply_and_block (ccgi_dbus_connection,query,-1, &err);
		
		dbus_message_unref(query);
		if (NULL == reply) {			
			if (dbus_error_is_set(&err)) {				
				dbus_error_free(&err);
			}			
		}
	
		dbus_message_iter_init(reply,&iter);
		dbus_message_iter_get_basic(&iter,&slot_count);	
		
		dbus_message_iter_next(&iter);
		
		dbus_message_iter_recurse(&iter,&iter_array);
	
		for (i = 0; i < slot_count; i++) {
			DBusMessageIter iter_struct;
			DBusMessageIter iter_sub_array;
			unsigned char slotno;
			unsigned char local_port_count;
			int j;
			
			
			dbus_message_iter_recurse(&iter_array,&iter_struct);
			
			dbus_message_iter_get_basic(&iter_struct,&slotno);		
			
			dbus_message_iter_next(&iter_struct);
			dbus_message_iter_get_basic(&iter_struct,&local_port_count);			
			
			dbus_message_iter_next(&iter_struct);
			dbus_message_iter_recurse(&iter_struct,&iter_sub_array);
	
			
			for (j = 0; j < local_port_count; j++) {
				DBusMessageIter iter_sub_struct;
				unsigned char portno;
								
				dbus_message_iter_recurse(&iter_sub_array,&iter_sub_struct);
				
				dbus_message_iter_get_basic(&iter_sub_struct,&portno);
				dbus_message_iter_next(&iter_sub_struct);
				
	//判断条件，是否默认?			
				if (1 == slot_count) {			
					 slotno=1;
	                 ccgi_get_port_flow(value,type,&pf,slotno,portno);	
				} else {				
	                 ccgi_get_port_flow(value,type,&pf,slotno,portno);	
				}
	fprintf(cgiOut,"%s",str_xml);
	memset(str_xml,0,sizeof(str_xml));
		
	sprintf(aaa,"%u-%u",slotno, portno);   //把plot  保存在  aaa中 	
    strcat(str_xml, "<slot name=\"");	
	strcat(str_xml, aaa);
	strcat(str_xml, "\">" );    
	
	strcat(str_xml,"<RX>");
	strcat(str_xml, "<rx_goodbytes>");	
    sprintf(str_xml+strlen(str_xml), "%lld", pf.rx_goodbytes);
    strcat(str_xml, "</rx_goodbytes>"); 

	strcat(str_xml, "<rx_badbytes>");	
    sprintf(str_xml+strlen(str_xml), "%lld", pf.rx_badbytes);
    strcat(str_xml, "</rx_badbytes>"); 
	
    strcat(str_xml, "<rx_uncastpkts>");	
    sprintf(str_xml+strlen(str_xml), "%lld", pf.rx_uncastpkts);
    strcat(str_xml, "</rx_uncastpkts>"); 

	strcat(str_xml, "<rx_bcastpkts>");	
    sprintf(str_xml+strlen(str_xml), "%lld", pf.rx_bcastpkts);
    strcat(str_xml, "</rx_bcastpkts>");

	strcat(str_xml, "<rx_mcastpkts>");	
    sprintf(str_xml+strlen(str_xml), "%lld", pf.rx_mcastpkts);
    strcat(str_xml, "</rx_mcastpkts>");

	strcat(str_xml, "<rx_fcframe>");	
    sprintf(str_xml+strlen(str_xml), "%lld", pf.rx_fcframe);
    strcat(str_xml, "</rx_fcframe>");

	strcat(str_xml, "<rx_fifooverruns>");	
    sprintf(str_xml+strlen(str_xml), "%lld", pf.rx_fifooverruns);
    strcat(str_xml, "</rx_fifooverruns>");

	strcat(str_xml, "<rx_underSizeframe>");	
    sprintf(str_xml+strlen(str_xml), "%lld", pf.rx_underSizeframe);
    strcat(str_xml, "</rx_underSizeframe>");

	strcat(str_xml, "<rx_fragments>");	
    sprintf(str_xml+strlen(str_xml), "%lld", pf.rx_fragments);
    strcat(str_xml, "</rx_fragments>");

	strcat(str_xml, "<rx_overSizeframe>");	
    sprintf(str_xml+strlen(str_xml), "%lld", pf.rx_overSizeframe);
    strcat(str_xml, "</rx_overSizeframe>");

	strcat(str_xml, "<rx_jabber>");	
    sprintf(str_xml+strlen(str_xml), "%lld", pf.rx_jabber);
    strcat(str_xml, "</rx_jabber>");

	strcat(str_xml, "<rx_errorframe>");	
    sprintf(str_xml+strlen(str_xml), "%lld", pf.rx_errorframe);
    strcat(str_xml, "</rx_errorframe>");

	strcat(str_xml, "<rx_BadCrc>");	
    sprintf(str_xml+strlen(str_xml), "%lld", pf.rx_BadCrc);
    strcat(str_xml, "</rx_BadCrc>");

	strcat(str_xml, "<rx_collision>");	
    sprintf(str_xml+strlen(str_xml), "%lld", pf.rx_collision);
    strcat(str_xml, "</rx_collision>");

	strcat(str_xml, "<rx_late_collision>");	
    sprintf(str_xml+strlen(str_xml), "%lld", pf.rx_late_collision);
    strcat(str_xml, "</rx_late_collision>");
	
	strcat(str_xml,"</RX>");

	strcat(str_xml,"<TX>");

	strcat(str_xml, "<tx_sent_deferred>");	
	sprintf(str_xml+strlen(str_xml), "%lld", pf.tx_sent_deferred);
	strcat(str_xml, "</tx_sent_deferred>"); 	
	
	strcat(str_xml, "<tx_goodbytes>");	
    sprintf(str_xml+strlen(str_xml), "%lld", pf.tx_goodbytes);
    strcat(str_xml, "</tx_goodbytes>"); 	

	strcat(str_xml, "<tx_uncastframe>");	
    sprintf(str_xml+strlen(str_xml), "%lld", pf.tx_uncastframe);
    strcat(str_xml, "</tx_uncastframe>"); 

	strcat(str_xml, "<tx_excessiveCollision>");	
    sprintf(str_xml+strlen(str_xml), "%lld", pf.tx_excessiveCollision);
    strcat(str_xml, "</tx_excessiveCollision>"); 	
	
	strcat(str_xml, "<tx_bcastframe>");	
    sprintf(str_xml+strlen(str_xml), "%lld", pf.tx_bcastframe);
    strcat(str_xml, "</tx_bcastframe>");

	strcat(str_xml, "<tx_mcastframe>");	
    sprintf(str_xml+strlen(str_xml), "%lld", pf.tx_mcastframe);
    strcat(str_xml, "</tx_mcastframe>");

	strcat(str_xml, "<tx_sentMutiple>");	
    sprintf(str_xml+strlen(str_xml), "%lld", pf.tx_sentMutiple);
    strcat(str_xml, "</tx_sentMutiple>");

	strcat(str_xml, "<tx_fcframe>");	
    sprintf(str_xml+strlen(str_xml), "%lld", pf.tx_fcframe);
    strcat(str_xml, "</tx_fcframe>");

    strcat(str_xml, "<tx_crcerror_fifooverrun>");	
    sprintf(str_xml+strlen(str_xml), "%lld", pf.tx_crcerror_fifooverrun);
    strcat(str_xml, "</tx_crcerror_fifooverrun>");
	
	
    strcat(str_xml,"</TX>");
	
    strcat(str_xml, "</slot>");	
				
				dbus_message_iter_next(&iter_sub_array);			
			}			
			dbus_message_iter_next(&iter_array);			
		}			
		dbus_message_unref(reply);			
}

static void create_xml(char *str_xml)   //创建xml文件
{
    if(NULL == str_xml)
    {
        return;
    }
    strcpy(str_xml, "<?xml version=\"1.0\" encoding=\"utf-8\"?>"\
                    "<root>");
                    
    get_xml_mem_new(str_xml);
   
    strcat(str_xml, "</root>");
    
}

int cgiMain()
{
  char *str_xml = NULL;
  if(NULL == (str_xml = (char *)malloc(sizeof(char)*MAX_LEN)))
  {
      return 1;
  }
  memset(str_xml, 0, sizeof(char)*MAX_LEN);
  cgiHeaderContentType("text/xml");

  create_xml(str_xml);  

  fprintf(cgiOut,"%s",str_xml);
  
  free(str_xml);
  return 0;
}

#ifdef __cplusplus
}
#endif

