#include <stdio.h>
#include "cgic.h"
#include <string.h>
#include <stdlib.h>
#include "wcpss/asd/asd.h"
#include "wcpss/wid/WID.h"
#include "dbus/wcpss/dcli_wid_wtp.h"
#include "dbus/wcpss/dcli_wid_wlan.h"
#include "ws_dcli_wlans.h"
#include "ws_usrinfo.h"
#include "ws_err.h"
#include "ws_ec.h"
#include "ws_dcli_vrrp.h"
#include "ws_init_dbus.h"
#include "ws_dbus_list_interface.h"

int cgiMain(void)
{
    int result=0,i=0;
    int wnum=0;
    char jsonstr[500] = { 0 };
    char start_global[10] = { 0 };
    char end_global[10] = { 0 };
    char select_insid[10] = { 0 };
    char wtp_state[20] = { 0 };
    unsigned int start_wtpno=0,end_wtpno=0;
    void *connection = NULL;
    dbus_parameter ins_para;
    WID_WTP *q = NULL;
    char *endptr = NULL;  
    DCLI_WTP_API_GROUP_ONE *head = NULL;

    DcliWInit();
    ccgi_dbus_init();

    memset(start_global,0,sizeof(start_global));
    memset(end_global,0,sizeof(end_global));  
    memset(select_insid,0,sizeof(select_insid));  
    cgiFormStringNoNewlines("start_global", start_global, 10);
    cgiFormStringNoNewlines("end_global", end_global,10);
    cgiFormStringNoNewlines("select_insid", select_insid,10);



    end_wtpno = strtoul(end_global,0,10);
    start_wtpno= strtoul(start_global,0,10);
    get_slotID_localID_instanceID(select_insid,&ins_para);  
    get_slot_dbus_connection(ins_para.slot_id, &connection, SNMPD_INSTANCE_MASTER_V3);

    if(connection)
    {
	    result=show_wtp_list_new_cmd_func(ins_para,connection,&head);
    } 
    if(result == 1)
    {
	  if((head)&&(head->WTP_INFO))
	  {
	    wnum = head->WTP_INFO->list_len;
	    fprintf(stderr,"--------wnum=%d-----\n",wnum);
	  }
    }
    if((wnum>0)&&((wnum)>start_wtpno)&&((wnum+1)>end_wtpno))
    {
	    q=head->WTP_INFO->WTP_LIST;
	    for(i=0;i<start_wtpno;i++)
	    {
		    if(q)
		    {
			    q=q->next;
		    }
	    }
	    for(i=start_wtpno;i<end_wtpno;i++)
	    {
		    if(q)
		    {
			    CheckWTPState(wtp_state,q->WTPStat);
		    }
		    if(strcmp(wtp_state,"imagedata")==0)
		    {
			    memset(wtp_state, 0,sizeof(wtp_state));
			    if(q->image_data_percent > 100)
			    {
				  strcpy(wtp_state,"image-mismatch");
			    }
			    else
			    {
				    sprintf(wtp_state, "image-%u%%", q->image_data_percent);
			    }
		    }
		    strcat(jsonstr,wtp_state);
		    if(i != (end_wtpno-1))
		    	strcat(jsonstr,"^");
		    if(q)
		    {
			    q=q->next;
		    }

	    }
    }
    fprintf(stderr,"--------jsonstr=%s-----\n",jsonstr);
    destroy_ccgi_dbus();
    if(result==1)
    {
      Free_wtp_list_new_head(head);
    }
    
    //这一句一定要加，否则异步访问会出现页面异常
    printf("Content type: text/html\n\n");
    printf("%s", jsonstr);
    
}








