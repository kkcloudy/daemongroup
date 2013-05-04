#include <stdio.h>
#include <syslog.h>
#include <string.h>
#include <stdlib.h>
#include <time.h>
#include <unistd.h>
#include <linux/kernel.h>
#include <dbus/dbus.h>

#include "m_dbus.h"
#include "m_log.h"
#include "m_data.h"

static DBusConnection *m_dbus_connection = NULL;

int m_dbus_init(void)
{
	DBusError dbus_error;
	int ret = 0;
	dbus_error_init (&dbus_error);
	m_dbus_connection = dbus_bus_get (DBUS_BUS_SYSTEM, &dbus_error);
	if (m_dbus_connection == NULL) 
	{
		printf("dbus_bus_get error!\n");
		dbus_error_free(&dbus_error);
		return M_DBUS_ERR;
	}

	return M_DBUS_OK;
}

int m_dbus_send_signal( const char *signal_name, int first_arg_type,...)

{
	DBusConnection *conn = m_dbus_connection;
	const char *obj_path = M_DBUS_OBJPATH;
	const char *interface_name = M_DBUS_INTERFACE;
	DBusMessage *msg=NULL;
	unsigned int serial = 0;

	va_list var_args;
	int iret = M_DBUS_OK;
	va_start ( var_args, first_arg_type );

	msg = dbus_message_new_signal(obj_path, 	/* object name of the signal */
			interface_name,			/* interface name of the signal */
			signal_name);		    	/* name of the signal */

	if (NULL == msg) 
	{
		return M_DBUS_ERR;
	}

	dbus_message_append_args_valist ( msg,
			first_arg_type,
			var_args );

	if (!dbus_connection_send(conn, msg, &serial)) 
	{
		dbus_message_unref(msg);
		return M_DBUS_ERR;
	}

	dbus_connection_flush(conn);
	dbus_message_unref(msg);

	va_end (var_args);

	return iret;
}

int m_dbus_send_vsignal ( SIGLIST *sig )
{
	int type=55;
	char *rcv_ip="192.168.7.192";
	int rcv_port= 162;
	char byte= 'm';


	if(0 == sig->func_param)
	{
	
		type=sig->flag;
	
		if(m_dbus_send_signal(sig->signal_name,
					DBUS_TYPE_UINT32,	&type,
					DBUS_TYPE_STRING,	&rcv_ip,
					DBUS_TYPE_UINT16,	&rcv_port,
					DBUS_TYPE_INVALID))
		{
			printf("( signal number %d trap OID %s param %d )%s:m_dbus_send_signal error!\n",sig->num,sig->trap_oid, sig->flag, sig->signal_name);
			return M_DBUS_ERR;
		}
		return M_DBUS_OK;

	}else if(1 == sig->func_param)
	{
	
		type=sig->flag;
			
		if(m_dbus_send_signal(sig->signal_name,
					DBUS_TYPE_UINT32,	&type,
					DBUS_TYPE_INVALID))
		{
			printf("( signal number %d trap OID %s param %d )%s:m_dbus_send_signal error!\n",sig->num,sig->trap_oid, sig->flag, sig->signal_name);
			return M_DBUS_ERR;
		}
		return M_DBUS_OK;

	}else if(2 == sig->func_param)
	{
	
		type=sig->flag;
	
		if(m_dbus_send_signal(sig->signal_name,
					DBUS_TYPE_INT32,	&type,
					DBUS_TYPE_UINT32,	&type,
					DBUS_TYPE_INVALID))
		{
			printf("( signal number %d trap OID %s param %d )%s:m_dbus_send_signal error!\n",sig->num,sig->trap_oid, sig->flag, sig->signal_name);
			return M_DBUS_ERR;
		}
		return M_DBUS_OK;

	}else if(3 == sig->func_param)
	{
	
		type=sig->flag;
	
		if(m_dbus_send_signal(sig->signal_name,
					DBUS_TYPE_INT32,	&type,
					DBUS_TYPE_UINT32,	&type,
					DBUS_TYPE_UINT32,	&type,
					DBUS_TYPE_INVALID))
		{
			printf("( signal number %d trap OID %s param %d )%s:m_dbus_send_signal error!\n",sig->num,sig->trap_oid, sig->flag, sig->signal_name);
			return M_DBUS_ERR;
		}
		return M_DBUS_OK;

	}else if(301 == sig->func_param)
	{
	
		type=sig->flag;
	
		if(m_dbus_send_signal(sig->signal_name,
					DBUS_TYPE_UINT32,	&type,
					DBUS_TYPE_STRING,	&rcv_ip,
					DBUS_TYPE_UINT16,	&rcv_port,
					DBUS_TYPE_INVALID))
		{
			printf("( signal number %d trap OID %s param %d )%s:m_dbus_send_signal error!\n",sig->num,sig->trap_oid, sig->flag, sig->signal_name);
			return M_DBUS_ERR;
		}
		return M_DBUS_OK;

	}else if(302 == sig->func_param)
	{
	
		type=sig->flag;
	
		if(m_dbus_send_signal(sig->signal_name,
					DBUS_TYPE_UINT32,	&type,
					DBUS_TYPE_BYTE,		&byte,
					DBUS_TYPE_STRING,	&rcv_ip,
					DBUS_TYPE_INVALID))
		{
			printf("( signal number %d trap OID %s param %d )%s:m_dbus_send_signal error!\n",sig->num,sig->trap_oid, sig->flag, sig->signal_name);
			return M_DBUS_ERR;
		}
		return M_DBUS_OK;

	}else if(303 == sig->func_param)
	{
	
		type=sig->flag;
	
		if(m_dbus_send_signal(sig->signal_name,
					DBUS_TYPE_BYTE,		&byte,
					DBUS_TYPE_BYTE,		&byte,
					DBUS_TYPE_BYTE,		&byte,
					DBUS_TYPE_INVALID))
		{
			printf("( signal number %d trap OID %s param %d )%s:m_dbus_send_signal error!\n",sig->num,sig->trap_oid, sig->flag, sig->signal_name);
			return M_DBUS_ERR;
		}
		return M_DBUS_OK;

	}else if(4 == sig->func_param)
	{
	
		type=sig->flag;
	
		if(m_dbus_send_signal(sig->signal_name,
					DBUS_TYPE_UINT32,	&type,
					DBUS_TYPE_UINT32,	&type,
					DBUS_TYPE_UINT32,	&type,
					DBUS_TYPE_STRING,	&rcv_ip,
					DBUS_TYPE_INVALID))
		{
			printf("( signal number %d trap OID %s param %d )%s:m_dbus_send_signal error!\n",sig->num,sig->trap_oid, sig->flag, sig->signal_name);
			return M_DBUS_ERR;
		}
		return M_DBUS_OK;

	}else if(7 == sig->func_param)
	{
	
		type=sig->flag;
	
		if(m_dbus_send_signal(sig->signal_name,
					DBUS_TYPE_UINT32,	&type,
					DBUS_TYPE_BYTE,		&byte,
					DBUS_TYPE_BYTE,		&byte,
					DBUS_TYPE_BYTE,		&byte,
					DBUS_TYPE_BYTE, 	&byte,
					DBUS_TYPE_BYTE, 	&byte,
					DBUS_TYPE_BYTE, 	&byte,
					DBUS_TYPE_INVALID))
		{
			printf("( signal number %d trap OID %s param %d )%s:m_dbus_send_signal error!\n",sig->num,sig->trap_oid, sig->flag, sig->signal_name);
			return M_DBUS_ERR;
		}
		return M_DBUS_OK;

	}else if(8 == sig->func_param)
	{
	
		type=sig->flag;
	
		if(m_dbus_send_signal(sig->signal_name,
					DBUS_TYPE_UINT32,	&type,
					DBUS_TYPE_STRING,	&rcv_ip,
					DBUS_TYPE_BYTE, 	&byte,
					DBUS_TYPE_BYTE, 	&byte,
					DBUS_TYPE_BYTE, 	&byte,
					DBUS_TYPE_BYTE, 	&byte,
					DBUS_TYPE_BYTE, 	&byte,
					DBUS_TYPE_BYTE, 	&byte,
					DBUS_TYPE_INVALID))
		{
			printf("( signal number %d trap OID %s param %d )%s:m_dbus_send_signal error!\n",sig->num,sig->trap_oid, sig->flag, sig->signal_name);
			return M_DBUS_ERR;
		}
		return M_DBUS_OK;

	}else if(9 == sig->func_param)
	{
	
		type=sig->flag;
	
		if(m_dbus_send_signal(sig->signal_name,
					DBUS_TYPE_UINT32,	&type,
					DBUS_TYPE_UINT32,	&type,
					DBUS_TYPE_STRING,	&rcv_ip,
					DBUS_TYPE_BYTE, 	&byte,
					DBUS_TYPE_BYTE, 	&byte,
					DBUS_TYPE_BYTE, 	&byte,
					DBUS_TYPE_BYTE, 	&byte,
					DBUS_TYPE_BYTE, 	&byte,
					DBUS_TYPE_BYTE, 	&byte,
					DBUS_TYPE_INVALID))
		{
			printf("( signal number %d trap OID %s param %d )%s:m_dbus_send_signal error!\n",sig->num,sig->trap_oid, sig->flag, sig->signal_name);
			return M_DBUS_ERR;
		}
		return M_DBUS_OK;

	}else if(901 == sig->func_param)
	{
	
		type=sig->flag;
	
		if(m_dbus_send_signal(sig->signal_name,
					DBUS_TYPE_UINT32,	&type,
					DBUS_TYPE_STRING,	&rcv_ip,
					DBUS_TYPE_BYTE, 	&byte,
					DBUS_TYPE_BYTE, 	&byte,
					DBUS_TYPE_BYTE, 	&byte,
					DBUS_TYPE_BYTE, 	&byte,
					DBUS_TYPE_BYTE, 	&byte,
					DBUS_TYPE_BYTE, 	&byte,
					DBUS_TYPE_UINT32,	&type,
					DBUS_TYPE_INVALID))
		{
			printf("( signal number %d trap OID %s param %d )%s:m_dbus_send_signal error!\n",sig->num,sig->trap_oid, sig->flag, sig->signal_name);
			return M_DBUS_ERR;
		}
		return M_DBUS_OK;

	}else if(902 == sig->func_param)
	{
	
		type=sig->flag;
	
		if(m_dbus_send_signal(sig->signal_name,
					DBUS_TYPE_UINT32,	&type,
					DBUS_TYPE_STRING,	&rcv_ip,
					DBUS_TYPE_BYTE, 	&type,
					DBUS_TYPE_BYTE, 	&byte,
					DBUS_TYPE_BYTE, 	&byte,
					DBUS_TYPE_BYTE, 	&byte,
					DBUS_TYPE_BYTE, 	&byte,
					DBUS_TYPE_BYTE, 	&byte,
					DBUS_TYPE_BYTE, 	&byte,
					DBUS_TYPE_INVALID))
		{
			printf("( signal number %d trap OID %s param %d )%s:m_dbus_send_signal error!\n",sig->num,sig->trap_oid, sig->flag, sig->signal_name);
			return M_DBUS_ERR;
		}
		return M_DBUS_OK;

	}else if(903 == sig->func_param)
	{
	
		type=sig->flag;
	
		if(m_dbus_send_signal(sig->signal_name,
					DBUS_TYPE_UINT32,	&type,
					DBUS_TYPE_BYTE, 	&byte,
					DBUS_TYPE_STRING,	&rcv_ip,
					DBUS_TYPE_BYTE, 	&byte,
					DBUS_TYPE_BYTE, 	&byte,
					DBUS_TYPE_BYTE, 	&byte,
					DBUS_TYPE_BYTE, 	&byte,
					DBUS_TYPE_BYTE, 	&byte,
					DBUS_TYPE_BYTE, 	&byte,
					DBUS_TYPE_INVALID))
		{
			printf("( signal number %d trap OID %s param %d )%s:m_dbus_send_signal error!\n",sig->num,sig->trap_oid, sig->flag, sig->signal_name);
			return M_DBUS_ERR;
		}
		return M_DBUS_OK;

	}else if(904 == sig->func_param)
	{
	
		type=sig->flag;
	
		if(m_dbus_send_signal(sig->signal_name,
					DBUS_TYPE_UINT32,	&type,
					DBUS_TYPE_STRING,	&rcv_ip,
					DBUS_TYPE_BYTE, 	&byte,
					DBUS_TYPE_BYTE, 	&byte,
					DBUS_TYPE_BYTE, 	&byte,
					DBUS_TYPE_BYTE, 	&byte,
					DBUS_TYPE_BYTE, 	&byte,
					DBUS_TYPE_BYTE, 	&byte,
					DBUS_TYPE_STRING,	&rcv_ip,
					DBUS_TYPE_INVALID))
		{
			printf("( signal number %d trap OID %s param %d )%s:m_dbus_send_signal error!\n",sig->num,sig->trap_oid, sig->flag, sig->signal_name);
			return M_DBUS_ERR;
		}
		return M_DBUS_OK;

	
	}else if(905 == sig->func_param)
		{
		
			type=sig->flag;
		
			if(m_dbus_send_signal(sig->signal_name,
						DBUS_TYPE_UINT32,	&type,
						DBUS_TYPE_STRING,	&rcv_ip,
						DBUS_TYPE_BYTE, 	&byte,
						DBUS_TYPE_BYTE, 	&byte,
						DBUS_TYPE_BYTE, 	&byte,
						DBUS_TYPE_BYTE, 	&byte,
						DBUS_TYPE_BYTE, 	&byte,
						DBUS_TYPE_BYTE, 	&byte,
						DBUS_TYPE_BYTE, 	&byte,
						DBUS_TYPE_INVALID))
			{
				printf("( signal number %d trap OID %s param %d )%s:m_dbus_send_signal error!\n",sig->num,sig->trap_oid, sig->flag, sig->signal_name);
				return M_DBUS_ERR;
			}
			return M_DBUS_OK;
	
	}else if(11 == sig->func_param)
	{
	
		type=sig->flag;
	
		if(m_dbus_send_signal(sig->signal_name,
					DBUS_TYPE_UINT32,	&type,
					DBUS_TYPE_BYTE, 	&byte,
					DBUS_TYPE_BYTE, 	&byte,
					DBUS_TYPE_UINT32,	&type,
					DBUS_TYPE_STRING,	&rcv_ip,
					DBUS_TYPE_BYTE, 	&byte,
					DBUS_TYPE_BYTE, 	&byte,
					DBUS_TYPE_BYTE, 	&byte,
					DBUS_TYPE_BYTE, 	&byte,
					DBUS_TYPE_BYTE, 	&byte,
					DBUS_TYPE_BYTE, 	&byte,
					DBUS_TYPE_INVALID))
		{
			printf("( signal number %d trap OID %s param %d )%s:m_dbus_send_signal error!\n",sig->num,sig->trap_oid, sig->flag, sig->signal_name);
			return M_DBUS_ERR;
		}
		return M_DBUS_OK;


	}else if(1101 == sig->func_param)
	{
	
		type=sig->flag;

		//char test=sig->flag;
	
		if(m_dbus_send_signal(sig->signal_name,
					DBUS_TYPE_UINT32,	&type,
					DBUS_TYPE_STRING,	&rcv_ip,
					DBUS_TYPE_UINT32,	&type,
					DBUS_TYPE_UINT32,	&type,
					DBUS_TYPE_BYTE, 	&type,
					DBUS_TYPE_BYTE, 	&byte,
					DBUS_TYPE_BYTE, 	&byte,
					DBUS_TYPE_BYTE, 	&byte,
					DBUS_TYPE_BYTE, 	&byte,
					DBUS_TYPE_BYTE, 	&byte,
					DBUS_TYPE_BYTE, 	&byte,
					DBUS_TYPE_INVALID))
		{
			printf("( signal number %d trap OID %s param %d )%s:m_dbus_send_signal error!\n",sig->num,sig->trap_oid, sig->flag, sig->signal_name);
			return M_DBUS_ERR;
		}
		return M_DBUS_OK;

	}else if(1102 == sig->func_param)
	{
	
		type=sig->flag;
	
		if(m_dbus_send_signal(sig->signal_name,
					DBUS_TYPE_UINT32,	&type,
					DBUS_TYPE_STRING,	&rcv_ip,
					DBUS_TYPE_BYTE, 	&byte,
					DBUS_TYPE_UINT32,	&type,
					DBUS_TYPE_BYTE, 	&type,
					DBUS_TYPE_BYTE, 	&byte,
					DBUS_TYPE_BYTE, 	&byte,
					DBUS_TYPE_BYTE, 	&byte,
					DBUS_TYPE_BYTE, 	&byte,
					DBUS_TYPE_BYTE, 	&byte,
					DBUS_TYPE_BYTE, 	&byte,
					DBUS_TYPE_INVALID))
		{
			printf("( signal number %d trap OID %s param %d )%s:m_dbus_send_signal error!\n",sig->num,sig->trap_oid, sig->flag, sig->signal_name);
			return M_DBUS_ERR;
		}
		return M_DBUS_OK;

	}else if(1103 == sig->func_param)
	{
	
		type=sig->flag;
	
		if(m_dbus_send_signal(sig->signal_name,
					DBUS_TYPE_UINT32,	&type,
					DBUS_TYPE_STRING,	&rcv_ip,
					DBUS_TYPE_BYTE, 	&byte,
					DBUS_TYPE_BYTE, 	&byte,
					DBUS_TYPE_BYTE, 	&type,
					DBUS_TYPE_BYTE, 	&byte,
					DBUS_TYPE_BYTE, 	&byte,
					DBUS_TYPE_BYTE, 	&byte,
					DBUS_TYPE_BYTE, 	&byte,
					DBUS_TYPE_BYTE, 	&byte,
					DBUS_TYPE_BYTE, 	&byte,
					DBUS_TYPE_INVALID))
		{
			printf("( signal number %d trap OID %s param %d )%s:m_dbus_send_signal error!\n",sig->num,sig->trap_oid, sig->flag, sig->signal_name);
			return M_DBUS_ERR;
		}
		return M_DBUS_OK;

	}else if(12 == sig->func_param)
	{
	
		type=sig->flag;
	
		if(m_dbus_send_signal(sig->signal_name,
					DBUS_TYPE_UINT32,	&type,
					DBUS_TYPE_STRING,	&rcv_ip,
					DBUS_TYPE_BYTE, 	&byte,
					DBUS_TYPE_BYTE, 	&byte,
					DBUS_TYPE_BYTE, 	&byte,
					DBUS_TYPE_BYTE, 	&byte,
					DBUS_TYPE_BYTE, 	&type,
					DBUS_TYPE_BYTE, 	&byte,
					DBUS_TYPE_BYTE, 	&byte,
					DBUS_TYPE_BYTE, 	&byte,
					DBUS_TYPE_BYTE, 	&byte,
					DBUS_TYPE_BYTE, 	&byte,
					DBUS_TYPE_INVALID))
		{
			printf("( signal number %d trap OID %s param %d )%s:m_dbus_send_signal error!\n",sig->num,sig->trap_oid, sig->flag, sig->signal_name);
			return M_DBUS_ERR;
		}
		return M_DBUS_OK;

	}else if(14 == sig->func_param)
	{
	
		type=sig->flag;
	
		if(m_dbus_send_signal(sig->signal_name,
					DBUS_TYPE_BYTE, 	&byte,
					DBUS_TYPE_BYTE, 	&byte,
					DBUS_TYPE_BYTE, 	&byte,
					DBUS_TYPE_BYTE, 	&byte,
					DBUS_TYPE_BYTE, 	&byte,
					DBUS_TYPE_BYTE, 	&byte,
					DBUS_TYPE_INT32,	&type,
					DBUS_TYPE_STRING,	&rcv_ip,
					DBUS_TYPE_BYTE, 	&byte,
					DBUS_TYPE_BYTE, 	&byte,
					DBUS_TYPE_BYTE, 	&byte,
					DBUS_TYPE_BYTE, 	&byte,
					DBUS_TYPE_BYTE, 	&byte,
					DBUS_TYPE_BYTE, 	&byte,
					DBUS_TYPE_INVALID))
		{
			printf("( signal number %d trap OID %s param %d )%s:m_dbus_send_signal error!\n",sig->num,sig->trap_oid, sig->flag, sig->signal_name);
			return M_DBUS_ERR;
		}
		return M_DBUS_OK;

	}else if(1401 == sig->func_param)
	{
	
		type=sig->flag;
	
		if(m_dbus_send_signal(sig->signal_name,
					DBUS_TYPE_UINT32,	&type,
					DBUS_TYPE_STRING,	&rcv_ip,
					DBUS_TYPE_BYTE, 	&byte,
					DBUS_TYPE_BYTE, 	&byte,
					DBUS_TYPE_BYTE, 	&byte,
					DBUS_TYPE_BYTE, 	&byte,
					DBUS_TYPE_BYTE, 	&byte,
					DBUS_TYPE_BYTE, 	&byte,
					DBUS_TYPE_BYTE, 	&byte,
					DBUS_TYPE_BYTE, 	&byte,
					DBUS_TYPE_BYTE, 	&byte,
					DBUS_TYPE_BYTE, 	&byte,
					DBUS_TYPE_BYTE, 	&byte,
					DBUS_TYPE_BYTE, 	&byte,
					DBUS_TYPE_INVALID))
		{
			printf("( signal number %d trap OID %s param %d )%s:m_dbus_send_signal error!\n",sig->num,sig->trap_oid, sig->flag, sig->signal_name);
			return M_DBUS_ERR;
		}
		return M_DBUS_OK;

	}else if(15 == sig->func_param)
	{
	
		type=sig->flag;
	
		if(m_dbus_send_signal(sig->signal_name,
					DBUS_TYPE_BYTE, 	&byte,
					DBUS_TYPE_BYTE, 	&byte,
					DBUS_TYPE_BYTE, 	&byte,
					DBUS_TYPE_BYTE, 	&byte,
					DBUS_TYPE_BYTE, 	&byte,
					DBUS_TYPE_BYTE, 	&byte,
					DBUS_TYPE_INT32,	&type,
					DBUS_TYPE_UINT16,	&rcv_port,
					DBUS_TYPE_STRING,	&rcv_ip,
					DBUS_TYPE_BYTE, 	&byte,
					DBUS_TYPE_BYTE, 	&byte,
					DBUS_TYPE_BYTE, 	&byte,
					DBUS_TYPE_BYTE, 	&byte,
					DBUS_TYPE_BYTE, 	&byte,
					DBUS_TYPE_BYTE, 	&byte,
					DBUS_TYPE_INVALID))
		{
			printf("( signal number %d trap OID %s param %d )%s:m_dbus_send_signal error!\n",sig->num,sig->trap_oid, sig->flag, sig->signal_name);
			return M_DBUS_ERR;
		}
		return M_DBUS_OK;

	}else if(1501 == sig->func_param)
	{
	
		type=sig->flag;
	
		if(m_dbus_send_signal(sig->signal_name,
					DBUS_TYPE_BYTE, 	&byte,
					DBUS_TYPE_BYTE, 	&byte,
					DBUS_TYPE_BYTE, 	&byte,
					DBUS_TYPE_BYTE, 	&byte,
					DBUS_TYPE_BYTE, 	&type,
					DBUS_TYPE_BYTE, 	&type,
					DBUS_TYPE_BYTE, 	&type,
					DBUS_TYPE_INT32,	&rcv_port,
					DBUS_TYPE_STRING,	&rcv_ip,
					DBUS_TYPE_BYTE, 	&type,
					DBUS_TYPE_BYTE, 	&type,
					DBUS_TYPE_BYTE, 	&byte,
					DBUS_TYPE_BYTE, 	&byte,
					DBUS_TYPE_BYTE, 	&byte,
					DBUS_TYPE_BYTE, 	&byte,
					DBUS_TYPE_INVALID))
		{
			printf("( signal number %d trap OID %s param %d )%s:m_dbus_send_signal error!\n",sig->num,sig->trap_oid, sig->flag, sig->signal_name);
			return M_DBUS_ERR;
		}
		return M_DBUS_OK;

	}else if(1502 == sig->func_param)
	{
	
		type=sig->flag;
	
		if(m_dbus_send_signal(sig->signal_name,
					DBUS_TYPE_UINT32,	&type,
					DBUS_TYPE_BYTE, 	&byte,
					DBUS_TYPE_STRING,	&rcv_ip,
					DBUS_TYPE_BYTE, 	&byte,
					DBUS_TYPE_BYTE, 	&byte,
					DBUS_TYPE_BYTE, 	&byte,
					DBUS_TYPE_BYTE, 	&byte,
					DBUS_TYPE_BYTE, 	&byte,
					DBUS_TYPE_BYTE, 	&byte,
					DBUS_TYPE_BYTE, 	&byte,
					DBUS_TYPE_BYTE, 	&byte,
					DBUS_TYPE_BYTE, 	&byte,
					DBUS_TYPE_BYTE, 	&byte,
					DBUS_TYPE_BYTE, 	&byte,
					DBUS_TYPE_BYTE, 	&byte,
					DBUS_TYPE_INVALID))
		{
			printf("( signal number %d trap OID %s param %d )%s:m_dbus_send_signal error!\n",sig->num,sig->trap_oid, sig->flag, sig->signal_name);
			return M_DBUS_ERR;
		}
		return M_DBUS_OK;

	}else if(16 == sig->func_param)
	{
	
		type=sig->flag;
	
		if(m_dbus_send_signal(sig->signal_name,
					DBUS_TYPE_BYTE, 	&byte,
					DBUS_TYPE_BYTE, 	&byte,
					DBUS_TYPE_BYTE, 	&byte,
					DBUS_TYPE_BYTE, 	&byte,
					DBUS_TYPE_BYTE, 	&byte,
					DBUS_TYPE_BYTE, 	&byte,
					DBUS_TYPE_UINT32,	&type,
					DBUS_TYPE_UINT32,	&type,
					DBUS_TYPE_BYTE, 	&byte,
					DBUS_TYPE_STRING,	&rcv_ip,
					DBUS_TYPE_BYTE, 	&byte,
					DBUS_TYPE_BYTE, 	&byte,
					DBUS_TYPE_BYTE, 	&byte,
					DBUS_TYPE_BYTE, 	&byte,
					DBUS_TYPE_BYTE, 	&byte,
					DBUS_TYPE_BYTE, 	&byte,
					DBUS_TYPE_INVALID))
		{
			printf("( signal number %d trap OID %s param %d )%s:m_dbus_send_signal error!\n",sig->num,sig->trap_oid, sig->flag, sig->signal_name);
			return M_DBUS_ERR;
		}
		return M_DBUS_OK;

	}

	return M_DBUS_OK;
}


