#include <stdio.h>
#include <syslog.h>
#include <string.h>
#include <stdlib.h>
#include <time.h>
#include <unistd.h>
#include <linux/kernel.h>
#include <dbus/dbus.h>

#include "m_data.h"
#include "m_dbus.h"
#include "m_log.h"


extern  SIGLIST gsiglist;
extern int len;
//extern m_dbus_send_signal;
//TrapSignalSendFunc;


int main (int argc, char **argv)
{
	int interval=1;
	int ret;
	int i;
	int j;
	int array[]={0};//{7,8,64,

	int arraylen=sizeof(array)/sizeof(array[0]);
	
	SIGLIST *signal;
	//	int type=0;
	//	char *rcv_ip="192.168.7.192";
	//	int rcv_port= 162;
	//	TrapSignalSendFunc p_func; 

	//	siglist->sendfunc=m_dbus_send_signal;

	//	p_func= m_dbus_send_signal;

	if (m_dbus_init())
	{
		printf ("m_dbus_init error!\n");
		return M_DBUS_ERR;
	}

	i=0;
	j=0;

	while (1)
	{
		if (0 != array[0])
		{
						
			if (j<0||j>=arraylen)
			{
				j=0;
			}
			
			i=array[j];
			printf("array not empty!\n i=%d\n",i);
			j++;
		
		}else if(0 == array[0])
		{
			printf("array is null!\n i=%d\n",i);
			i++;
		}
		
		
		
		if ((signal = get_gsiglist_member(i, 0)) == NULL)
		{
			if (i<=0||i>len)
			{
				i=1;
				continue;
			}
			printf("get_gsiglist_member(%d) error!\n",i);

		}

		if( m_dbus_send_vsignal( signal))
		{
			printf("( signal number %d trap OID %s param %d )%s:m_dbus_send_signal error!\n",signal->num,signal->trap_oid, signal->flag, signal->signal_name);
			return M_DBUS_ERR;
		}

		printf("( signal number %d trap OID %s param %d )%s:m_dbus_send_signal succeed!\n",signal->num,signal->trap_oid, signal->flag, signal->signal_name);
		sleep(interval);
		
	}
	return 0;
}

/*		siglist->sendfunc("ac_sample_portal_server_reach_over_threshold",
		DBUS_TYPE_UINT32,  &type,
		DBUS_TYPE_STRING,  &rcv_ip,
		DBUS_TYPE_UINT16,  &rcv_port,
		DBUS_TYPE_INVALID);
		*/

/*		if(siglist->sendfunc("ac_sample_portal_server_reach_over_threshold",
		DBUS_TYPE_UINT32,  &type,
		DBUS_TYPE_STRING,  &rcv_ip,
		DBUS_TYPE_UINT16,  &rcv_port,
		DBUS_TYPE_INVALID))*/
/*		if(m_dbus_send_signal("ac_sample_portal_server_reach_over_threshold",
		DBUS_TYPE_UINT32,  NULL,
		DBUS_TYPE_STRING,  NULL,
		DBUS_TYPE_UINT16,  NULL,
		DBUS_TYPE_UINT32,  &type,
		DBUS_TYPE_STRING,  &rcv_ip,
		DBUS_TYPE_UINT16,  &rcv_port,
		DBUS_TYPE_INVALID )
		)
		*/	

