/*
 * Zebra connect library for OSPFd
 * Copyright (C) 1997, 98, 99, 2000 Kunihiro Ishiguro, Toshiaki Takada
 *
 * This file is part of GNU Zebra.
 *
 * GNU Zebra is free software; you can redistribute it and/or modify it
 * under the terms of the GNU General Public License as published by the
 * Free Software Foundation; either version 2, or (at your option) any
 * later version.
 *
 * GNU Zebra is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with GNU Zebra; see the file COPYING.  If not, write to the
 * Free Software Foundation, Inc., 59 Temple Place - Suite 330,
 * Boston, MA 02111-1307, USA. 
 */

#include <zebra.h>

#include "thread.h"
#include "command.h"
#include "network.h"
#include "prefix.h"
#include "routemap.h"
#include "table.h"
#include "stream.h"
#include "memory.h"
#include "zclient.h"
#include "filter.h"
#include "plist.h"
#include "log.h"

#include "ospfd/ospfd.h"
#include "ospfd/ospf_interface.h"
#include "ospfd/ospf_ism.h"
#include "ospfd/ospf_asbr.h"
#include "ospfd/ospf_asbr.h"
#include "ospfd/ospf_abr.h"
#include "ospfd/ospf_lsa.h"
#include "ospfd/ospf_dump.h"
#include "ospfd/ospf_route.h"
#include "ospfd/ospf_zebra.h"
#ifdef HAVE_SNMP
#include "ospfd/ospf_snmp.h"
#include <dbus/dbus.h>
#include "stdio.h" 

#define PFM_DBUS_BUSNAME				"pfm.daemon"
#define PFM_DBUS_OBJPATH				"/pfm/daemon"
#define PFM_DBUS_INTERFACE				"pfm.daemon"
#define PFM_DBUS_METHOD_PFM_TABLE 	"pfm_maintain_table"
#if 1 
#define N_BUS_TYPES 3
#endif
#define _DBUS_FUNCTION_NAME "unknown function"
/** String used in _dbus_return_if_fail macro */
const char *_dbus_return_if_fail_warning_format =
"arguments to %s() were incorrect, assertion \"%s\" failed in file %s line %d.\n"
"This is normally a bug in some application using the D-Bus library.\n";
#endif /* HAVE_SNMP */

/* Zebra structure to hold current status. */
struct zclient *zclient = NULL;

/* For registering threads. */
extern struct thread_master *master;
struct in_addr router_id_zebra;
struct dbus_connection{
DBusConnection *dcli_dbus_connection;
int 			slot_id;
int 			board_type;
int				board_state;
};
typedef struct dbus_connection dbus_connection;

dbus_connection *dbus_connection_dcli[MAX_SLOT];


#ifdef USE_DBUS_LIB



#if defined(__GNUC__) && (__GNUC__ > 2) && defined(__OPTIMIZE__)
#define _DBUS_BOOLEAN_EXPR(expr)                \
 __extension__ ({                               \
   int _dbus_boolean_var_;                      \
   if (expr)                                    \
      _dbus_boolean_var_ = 1;                   \
   else                                         \
      _dbus_boolean_var_ = 0;                   \
   _dbus_boolean_var_;                          \
})
#define _DBUS_LIKELY(expr) (__builtin_expect (_DBUS_BOOLEAN_EXPR(expr), 1))
#define _DBUS_UNLIKELY(expr) (__builtin_expect (_DBUS_BOOLEAN_EXPR(expr), 0))
#else
#define _DBUS_LIKELY(expr) (expr)
#define _DBUS_UNLIKELY(expr) (expr)
#endif





extern char *bus_connection_addresses[N_BUS_TYPES] ;
extern DBusConnection *
dbus_bus_get_remote (DBusBusType  	type,
			  		int			slot_num,
	      	  		DBusError   	*error);
#define _dbus_return_val_if_error_is_set(error, val) _dbus_return_val_if_fail ((error) == NULL || !dbus_error_is_set ((error)), (val))

#if 0
static DBusBusType activation_bus_type = DBUS_BUS_STARTER;
static dbus_bool_t initialized = FALSE;
static dbus_bool_t fatal_warnings_on_check_failed = TRUE;
static dbus_bool_t warn_initted = FALSE;
static dbus_bool_t fatal_warnings = FALSE;
#endif


extern dbus_bool_t warn_initted = FALSE;
extern dbus_bool_t fatal_warnings = FALSE;
extern dbus_bool_t fatal_warnings_on_check_failed = TRUE;


/*test*/

unsigned long
_dbus_pid_for_log (void)
{
  return getpid ();
}




void
_dbus_real_assert (dbus_bool_t  condition,
                   const char  *condition_text,
                   const char  *file,
                   int          line,
                   const char  *func)
{
  if (_DBUS_UNLIKELY (!condition))
    {
      /*_dbus_warn ("%lu: assertion failed \"%s\" file \"%s\" line %d function %s\n",
                  _dbus_pid_for_log (), condition_text, file, line, func);
                  20110111*/
      _dbus_abort ();
    }
}



#define _dbus_assert(condition)                                         \
  _dbus_real_assert ((condition) != 0, #condition, __FILE__, __LINE__, _DBUS_FUNCTION_NAME)


#define _dbus_return_val_if_fail(condition, val) do {                                   \
   _dbus_assert ((*(const char*)_DBUS_FUNCTION_NAME) != '_');                           \
  if (!(condition)) {                                                                   \
    _dbus_warn_check_failed (_dbus_return_if_fail_warning_format,                       \
                             _DBUS_FUNCTION_NAME, #condition, __FILE__, __LINE__);      \
    return (val);                                                                       \
  } } while (0)


const char*
_dbus_getenv (const char *varname)
{  
  return getenv (varname);
}


static void
init_warnings(void)

{
  if (!warn_initted)
    {
      const char *s;
      s = _dbus_getenv ("DBUS_FATAL_WARNINGS");
      if (s && *s)
        {
          if (*s == '0')
            {
              fatal_warnings = FALSE;
              fatal_warnings_on_check_failed = FALSE;
            }
          else if (*s == '1')
            {
              fatal_warnings = TRUE;
              fatal_warnings_on_check_failed = TRUE;
            }
          else
            {
              fprintf(stderr, "DBUS_FATAL_WARNINGS should be set to 0 or 1 if set, not '%s'",
                      s);
            }
        }

      warn_initted = TRUE;
    }
}


void
_dbus_sleep_milliseconds (int milliseconds)
{
#ifdef HAVE_NANOSLEEP
  struct timespec req;
  struct timespec rem;

  req.tv_sec = milliseconds / MILLISECONDS_PER_SECOND;
  req.tv_nsec = (milliseconds % MILLISECONDS_PER_SECOND) * NANOSECONDS_PER_MILLISECOND;
  rem.tv_sec = 0;
  rem.tv_nsec = 0;

  while (nanosleep (&req, &rem) < 0 && errno == EINTR)
    req = rem;
#elif defined (HAVE_USLEEP)
  usleep (milliseconds * MICROSECONDS_PER_MILLISECOND);
#else /* ! HAVE_USLEEP */
  sleep (MAX (milliseconds / 1000, 1));
#endif
}

void
_dbus_print_backtrace (void)
{
#if defined (HAVE_BACKTRACE) && defined (DBUS_BUILT_R_DYNAMIC)
  void *bt[500];
  int bt_size;
  int i;
  char **syms;

  bt_size = backtrace (bt, 500);

  syms = backtrace_symbols (bt, bt_size);

  i = 0;
  while (i < bt_size)
    {
      /* don't use dbus_warn since it can _dbus_abort() */
      fprintf (stderr, "  %s\n", syms[i]);
      ++i;
    }
  fflush (stderr);

  free (syms);
#elif defined (HAVE_BACKTRACE) && ! defined (DBUS_BUILT_R_DYNAMIC)
  fprintf (stderr, "  D-Bus not built with -rdynamic so unable to print a backtrace\n");
#else
  fprintf (stderr, "  D-Bus not compiled with backtrace support so unable to print a backtrace\n");
#endif
}



void
_dbus_abort (void)
{
  const char *s;
  
  _dbus_print_backtrace ();
  
  s = _dbus_getenv ("DBUS_BLOCK_ON_ABORT");
  if (s && *s)
    {
      /* don't use _dbus_warn here since it can _dbus_abort() */
      fprintf (stderr, "  Process %lu sleeping for gdb attach\n", _dbus_pid_for_log ());
      _dbus_sleep_milliseconds (1000 * 180);
    }
  
  abort ();
  _dbus_exit (1); /* in case someone manages to ignore SIGABRT ? */
}


void
_dbus_warn_check_failed(const char *format,
                        ...)
{
  va_list args;
  
  if (!warn_initted)
    init_warnings ();

  fprintf (stderr, "process %lu: ", _dbus_pid_for_log ());
  
  va_start (args, format);
  vfprintf (stderr, format, args);
  va_end (args);

  if (fatal_warnings_on_check_failed)
    {
      fflush (stderr);
      _dbus_abort ();
    }
}



#endif

int dbus_connection_register(int slot_id,dbus_connection** connection)
{
	
	zlog_debug("(sync_test %d),dbus_connection_register in \n",__LINE__);
	DBusError dbus_error;	
	dbus_error_init (&dbus_error);
	
	if(slot_id > MAX_SLOT)
	{
		syslog(LOG_INFO,"ERROR:dbus_connection_register:error slot_id\n");
		zlog_debug("(sync_test %d)ERROR:dbus_connection_register:error slot_id\n",__LINE__);
		return -1;
	}
	
	if((*connection) == NULL)
	{
		syslog(LOG_INFO,"ERROR:dbus_connection_register:connection is NULL\n");
		
		zlog_debug("(sync_test %d)ERROR:dbus_connection_register:connection is NULL\n",__LINE__);
		return -1;
	}

	(*connection) -> slot_id 			= slot_id;
	(*connection) -> board_type			= -1;
	(*connection) -> board_state			= -1;
	(*connection) -> dcli_dbus_connection 	= dbus_bus_get_remote(DBUS_BUS_SYSTEM,slot_id,&dbus_error);

	if((*connection) -> dcli_dbus_connection == NULL)
	{
		syslog(LOG_INFO,"dbus_bus_get(): %s", dbus_error.message);
		zlog_debug("(sync_test %d) dbus_bus_get(): %s \n",__LINE__, dbus_error.message);
		return -1;
		
	}
	
	return 0;
	
	
}




int dbus_connection_remote_init(dbus_connection** connection)
{
	zlog_debug("(sync_test %d),dbus_connection_remote_init in \n",__LINE__);


	if(NULL != (*connection))
	{
	//	free(*connection);
		(*connection) = NULL;
	}

	(*connection) = (dbus_connection*)malloc(sizeof(dbus_connection));
	
	if(NULL == (*connection))
	{
		syslog(LOG_INFO,"malloc error\n");
		
		zlog_debug("(sync_test %d) malloc error\n",__LINE__);
		return -1;
	}

	(*connection) -> dcli_dbus_connection	= NULL;
	(*connection) -> slot_id				= -1;
	(*connection) -> board_type				= -1;
	(*connection) -> board_state			= -1;

	/* CID 11178 (#1 of 3): Missing return statement, fix in */
	return 0;
	/* CID 11178 (#1 of 3) end */

}



int dbus_connection_register_all(dbus_connection** dbus_connection)
{
	int i = 0;
	if(NULL == dbus_connection)
	{
		syslog(LOG_INFO,"ERROR:dbus_connection_register_all:dbus_connection = NULL\n");
		zlog_debug("(sync_test %dERROR:dbus_connection_register_all:dbus_connection = NULL\n",__LINE__);

		return -1;
	}
	for(i = 0;i < MAX_SLOT;i++)
	{
		syslog(LOG_INFO,"\n===============connect slot %d ===================\n",i);
		zlog_debug("(sync_test %d)===============connect slot %d ===================\n",__LINE__,i);
		if(-1 == dbus_connection_register(i,&dbus_connection[i]))
		{
			syslog(LOG_INFO,"ERROR:dbus-connection_register_all:connect slot %d error\n",i);
			zlog_debug("(sync_test %d)ERROR:dbus-connection_register_all:connect slot %d error\n",__LINE__,i);

			continue;
		}
	}
	return 0;
}


int dbus_connection_init_all(dbus_connection** dbus_connection)
{
	zlog_debug("(sync_test %d),dbus_connection_init_all in \n",__LINE__);

	int i = 0;
	if(NULL == dbus_connection)
	{
		syslog(LOG_INFO,"ERROR:dbus_connection_init_all:dbus_connection = NULL\n");
		zlog_debug("(sync_test %d)ERROR:dbus_connection_init_all:dbus_connection = NULL\n",__LINE__);

		return -1;
	}

	for(i = 0;i < MAX_SLOT;i++)
	{
		/*fix CID 11178 (#1 of 3)*/
		//if(0 == dbus_connection_remote_init(&dbus_connection[i]))
		if(dbus_connection_remote_init(&dbus_connection[i]))
		/*fix CID 11178 (#1 of 3) end*/
		{
			syslog(LOG_INFO,"ERROR:dbus_connection_init_all:init connection %d error\n",i);
			zlog_debug("(sync_test %d)ERROR:dbus_connection_init_all:init connection %d error \n",__LINE__,i);

			return -1;
		}
	}
	
	zlog_debug("(sync_test %d),dbus_connection_init_all ok \n",__LINE__);
	return 0;
}



int dcli_dbus_init_remote(void) {
	zlog_debug("(sync_test %d),dcli_dbus_init_remote \n",__LINE__);
	if(dbus_connection_init_all(dbus_connection_dcli) == -1)
	{
	zlog_debug("(sync_test %d),dcli_dbus_init_remote1 \n",__LINE__);
		return FALSE;
	}
	else
		zlog_debug("(sync_test %d),dcli_dbus_init_remote2 \n",__LINE__);

	if(dbus_connection_register_all(dbus_connection_dcli) == -1)
	{
	
	zlog_debug("(sync_test %d),dcli_dbus_init_remote3 \n",__LINE__);
		return FALSE;
	}
	else		
	zlog_debug("(sync_test %d),dcli_dbus_init_remote4 \n",__LINE__);
	
	zlog_debug("(sync_test %d),dcli_dbus_init_remote(void) ok \n",__LINE__);
	return TRUE;
	
}

///////////////////////////////////////////
///////////////#endif

int 
dcli_communicate_pfm_by_dbus(int opt, 
							int opt_para, 
							unsigned short protocol, 
							char* ifname, 
							unsigned int src_port,
							unsigned int dest_port, 
							int slot,
							char* src_ipaddr,
							char* dest_ipaddr,
							unsigned int send_to)

{
zlog_debug("(sync_test %d),dcli_communicate_pfm_by_dbus start ifname is %s slot is %d  send_to is :%d\n",__LINE__,ifname,slot,send_to);
	DBusMessage *query, *reply;
	DBusError err;
	unsigned int op_ret;
	/*
	long src_ipaddr1,src_ipaddr2,dest_ipaddr1,dest_ipaddr2;
	memcpy(&src_ipaddr1,&src_ipaddr,sizeof(src_ipaddr1));
	memcpy(&src_ipaddr2,(&src_ipaddr) + sizeof(src_ipaddr1),sizeof(src_ipaddr2));
	memcpy(&dest_ipaddr1,(&dest_ipaddr),sizeof(dest_ipaddr1));
	memcpy(&dest_ipaddr2,(&dest_ipaddr) + sizeof(dest_ipaddr1),sizeof(dest_ipaddr2));
	*/
#if 0
	fprintf(stderr,"DCLI send data to PFM are :\n");
			fprintf(stderr," opt is %d ....\n",opt);
			fprintf(stderr," protocol is %u ....\n",protocol);
			fprintf(stderr," ifindex is %s ....\n",ifname);
			fprintf(stderr," port is %d ....\n",dest_port);
			fprintf(stderr," send to is %d ....\n",send_to);
			fprintf(stderr," slot is %d ....\n",slot);
			fprintf(stderr," ipaddr is %u ....\n",dest_ipaddr);		
	#endif

			zlog_debug("DCLI send data to PFM are :\n");
			zlog_debug(" opt is %d ....\n",opt);
			zlog_debug(" protocol is %u ....\n",protocol);
			zlog_debug(" ifindex is %s ....\n",ifname);
			zlog_debug(" port is %d ....\n",dest_port);
			zlog_debug(" send to is %d ....\n",send_to);
			zlog_debug(" slot is %d ....\n",slot);
			zlog_debug(" ipaddr is %u ....\n",dest_ipaddr);

			
	if(slot == send_to)
	{
		return -1;
	}
	query = dbus_message_new_method_call(
								PFM_DBUS_BUSNAME,			\
								PFM_DBUS_OBJPATH,		\
								PFM_DBUS_INTERFACE,	\
								PFM_DBUS_METHOD_PFM_TABLE);
	
	dbus_error_init(&err);

	dbus_message_append_args(query,
							DBUS_TYPE_INT32, &opt,
							DBUS_TYPE_INT32, &opt_para,
							DBUS_TYPE_UINT16, &protocol,
							DBUS_TYPE_STRING, &ifname,
							DBUS_TYPE_UINT32, &src_port,
							DBUS_TYPE_UINT32, &dest_port,
							DBUS_TYPE_STRING,  &src_ipaddr,
							DBUS_TYPE_STRING,  &dest_ipaddr,	
							DBUS_TYPE_INT32,  &slot,
							DBUS_TYPE_INVALID);

	if(-1 == send_to)
	{
		int i;
		for(i = 0;i < MAX_SLOT ; i++)
		{
			if(NULL != (dbus_connection_dcli[i] -> dcli_dbus_connection))
			{
				
				reply = dbus_connection_send_with_reply_and_block (dbus_connection_dcli[i] -> dcli_dbus_connection,query,-1, &err);
				if (NULL == reply) {
					fprintf(stderr,"[%d]failed get args.\n",i);
					zlog_debug("(sync_test %d),[%d]failed get args.\n",__LINE__,i);
					if (dbus_error_is_set(&err)) {
						fprintf(stderr,"%s raised: %s\n",err.name,err.message);				
						zlog_debug("(sync_test %d),%s raised: %s\n",__LINE__,err.name,err.message);
						dbus_message_unref(query);
						dbus_error_free(&err);
						return -1;
					}
				}
				
				if (dbus_message_get_args ( reply, &err,
											DBUS_TYPE_UINT32,&op_ret,
											DBUS_TYPE_INVALID)) 
				{
					//vty_out(vty,"DCLI recv [%d] reply is :%s\n",i,op_ret == 0?"OK":"ERROR");
				} 
				else {
					fprintf(stderr,"Failed get args.\n");
					
					zlog_debug("(sync_test %d),Failed get args.\n",__LINE__);
					if (dbus_error_is_set(&err)) {
						fprintf(stderr,"%s raised: %s",err.name,err.message);
						zlog_debug("(sync_test %d),%s raised: %s",__LINE__,err.name,err.message);
						dbus_error_free(&err);
					}
				}
				
//				dbus_message_unref(reply);
			}

		}
		dbus_message_unref(query);
		
		dbus_message_unref(reply);

		dbus_error_free(&err);
	}else{
	
		if(NULL != dbus_connection_dcli[send_to] -> dcli_dbus_connection)
		{
zlog_debug("(sync_test %d),NULL != dbus_connection_dcli[send_to] -> dcli_dbus_connection \n",__LINE__);
zlog_debug("(sync_test %d),send_to:%d \n",__LINE__,send_to);
zlog_debug("(syna_test %d),dbus_connection_dcli[send_to]->slot_id:%d \n",__LINE__,dbus_connection_dcli[send_to]->slot_id);
zlog_debug("(syna_test %d),dbus_connection_dcli[send_to]->board_type:%d \n",__LINE__,dbus_connection_dcli[send_to]->board_type);
zlog_debug("(syna_test %d),dbus_connection_dcli[send_to]->board_state:%d \n",__LINE__,dbus_connection_dcli[send_to]->board_state);
			


			reply = dbus_connection_send_with_reply_and_block (dbus_connection_dcli[send_to] -> dcli_dbus_connection,query,-1, &err);
			if (NULL == reply){
				fprintf(stderr,"failed get args. \n");
				
				zlog_debug("(sync_test %d),Failed get args.\n",__LINE__);
				if (dbus_error_is_set(&err)){
					fprintf(stderr,"%s raised: %s\n",err.name,err.message);
					zlog_debug("(sync_test %d),%s raised: %s\n",__LINE__,err.name,err.message);
					
					dbus_message_unref(query);
					dbus_error_free(&err);
					return -1;
				}
			}
			zlog_debug("(sync_test %d),NULL != dbus_connection_dcli[send_to] -> dcli_dbus_connection SSSSS \n",__LINE__);
			dbus_message_unref(query);
		}else{
			fprintf(stderr,"connection of board %d is not exist\n",send_to);
			
			zlog_debug("(sync_test %d),connection of board %d is not exist\n",__LINE__,send_to);
			return -1;
		}
	
		
			
//	#if 0
	if (dbus_message_get_args ( reply, &err,
								DBUS_TYPE_UINT32,&op_ret,
								DBUS_TYPE_INVALID)) 
	{
		//vty_out(vty,"DCLI recv reply is :%s\n",op_ret == 0?"OK":"ERROR");
		zlog_debug("(sync_test %d),op_ret=%d \n",__LINE__,op_ret);
		zlog_debug("(sync_test %d)DCLI recv reply is :%s\n",__LINE__,op_ret == 0?"OK":"ERROR");
	} 
	else {
		fprintf(stderr,"Failed get args.\n");
		zlog_debug("(sync_test %d),Failed get args.\n",__LINE__);
		if (dbus_error_is_set(&err)) {
			fprintf(stderr,"%s raised: %s",err.name,err.message);
			zlog_debug("(sync_test %d),%s raised: %s",__LINE__,err.name,err.message);
			dbus_error_free(&err);
		}
	}
	
	dbus_message_unref(reply);
//	#endif
	}
	
	zlog_debug("(sync_test %d),dcli_communicate_pfm_by_dbus end \n",__LINE__);
	return 0;
}
void 
ospf_pfm_start()
{
	int temp_ret;	
	int slot,back_slot=2;
	FILE *fd = NULL;
	int product_type = -1;	
	int is_active_master = -1;
	int board_id = -1;
	int i;

	fd = fopen(PRODUCT_TYPE_FILE,"r");
	if(NULL == fd)
	{
		zlog_notice("open file /dbm/product/product_serial failed\n");
		return -1;
	}
	/* CID 10320 (#1 of 3): Unchecked return value from library  */
	fscanf(fd,"%d",&product_type);
	/* CID 10320 (#1 of 3) end */
	fclose(fd);

	fd = fopen(IS_ACTIVE_MASTER_FILE,"r");
	if(NULL == fd)
	{
		zlog_notice("open file /dbm/local_board/is_active_master failed\n");
		return -1;
	}
	/* CID 10320 (#2 of 3): Unchecked return value from library  */
	fscanf(fd,"%d",&is_active_master);
	/* CID 10320 (#2 of 3) end */
	fclose(fd);

	fd = fopen(SLOT_ID_FILE,"r");
	if(fd == NULL)
	{
		zlog_notice("open file /dbm/local_board/slot_id failed\n");
		return -1;	
	}
	/* CID 10320 (#3 of 3): Unchecked return value from library  */
	fscanf(fd, "%d", &board_id);
	/* CID 10320 (#3 of 3) end */
	fclose(fd);
	
	zlog_debug("(sync_test %d)%s, start \n",__LINE__,__func__);
	zlog_debug("(sync_test %d),product->board_id:%d,product->product_type:%d",__LINE__,board_id,product_type);
	
	  /*7600*/
	  
  
		if(product_type==7&&is_active_master==1)
		  {				
			slot=board_id;
			if(slot == 1)
				back_slot = 2;
			else if(slot == 2)
				back_slot = 1;
			else
			{
				back_slot=-1;
			}	
				
			  for(i = 0;i < MAX_SLOT ; i++)
				{
					temp_ret=0;
					/* CID 14333 (#1 of 2): Uninitialized scalar variable , no problem */
					 if(NULL != (dbus_connection_dcli[i] -> dcli_dbus_connection)&&i != back_slot &&i != slot)
						  
						 temp_ret=dcli_communicate_pfm_by_dbus(0, 0, 89, "all", 0,0,slot, "all", "all",i);/*use dbus send message to pfm*/
						  
					 
					 if(temp_ret != 0)

						 zlog_debug("(sync_test %d) from slot(%d)to slot(%d) dbus send message error_table\n",__LINE__,slot,i);
					 else
					 	
					 	 zlog_debug("(sync_test %d) from slot(%d)to slot(%d) dbus send message success! \n",__LINE__,slot,i);
			   }	  
		}

	  /*8600*/
 
	  if(product_type == 8 && is_active_master == 1)
		{
					
		slot=board_id;
		zlog_debug("(ospf_syn %d),%s slot(%d) \n",__LINE__,__func__,slot);
		if(slot == 5)
				back_slot = 6;
			if(slot == 6)
				back_slot = 5;
	
	zlog_debug("(ospf_syn %d),%s back_slot(%d) \n",__LINE__,__func__,back_slot);
	    for(i = 0;i < MAX_SLOT ; i++)
			{
				/* CID 14333 (#2 of 2): Uninitialized scalar variable , no problem */
				if(NULL != (dbus_connection_dcli[i] -> dcli_dbus_connection)&& i != back_slot && i!=slot)
					{
					zlog_debug("(sync_test %d) %s i= %d \n",__LINE__,__func__,i);
					//if(i!=slot)
						temp_ret=dcli_communicate_pfm_by_dbus(0, 0, 89, "all", 0,0,slot, "all", "all",i);/*use dbus send message to pfm*/

					if(temp_ret != 0)
					
						zlog_debug("(sync_test %d) from slot(%d)to slot(%d) dbus send message error_table\n",__LINE__,slot,i);
					else
								
						zlog_debug("(sync_test %d) from slot(%d)to slot(%d) dbus send message success! \n",__LINE__,slot,i);

				}
						 
			}
		
	
		}
  

}

/* Router-id update message from zebra. */
static int
ospf_router_id_update_zebra (int command, struct zclient *zclient,
			     zebra_size_t length)
{
  struct ospf *ospf;
  struct prefix router_id;
  zebra_router_id_update_read(zclient->ibuf,&router_id);

  router_id_zebra = router_id.u.prefix4;

  ospf = ospf_lookup ();
  
  if (ospf != NULL)
    ospf_router_id_update (ospf);
  
  return 0;
}

/* Inteface addition message from zebra. */
static int
ospf_interface_add (int command, struct zclient *zclient, zebra_size_t length)
{
  struct interface *ifp;
  struct ospf *ospf;

  ifp = zebra_interface_add_read (zclient->ibuf);

  if (IS_DEBUG_OSPF (zebra, ZEBRA_INTERFACE))
    zlog_debug ("Rtm: interface add %s index %d flags %ld metric %d mtu %d",
               ifp->name, ifp->ifindex, ifp->flags, ifp->metric, ifp->mtu);

  assert (ifp->info);

  if (!OSPF_IF_PARAM_CONFIGURED (IF_DEF_PARAMS (ifp), type))
    {
      SET_IF_PARAM (IF_DEF_PARAMS (ifp), type);
      IF_DEF_PARAMS (ifp)->type = ospf_default_iftype(ifp);
    }

  ospf = ospf_lookup ();
  if (ospf != NULL)
    ospf_if_update (ospf);

#ifdef HAVE_SNMP
  ospf_snmp_if_update (ifp);
#endif /* HAVE_SNMP */

  return 0;
}

static int
ospf_interface_delete (int command, struct zclient *zclient,
                       zebra_size_t length)
{
  struct interface *ifp;
  struct stream *s;
  struct route_node *rn;

  s = zclient->ibuf;
  /* zebra_interface_state_read() updates interface structure in iflist */
  ifp = zebra_interface_state_read (s);

  if (ifp == NULL)
    return 0;

  if (IS_DEBUG_OSPF (zebra, ZEBRA_INTERFACE)){
	 if (if_is_up (ifp))
	   zlog_warn ("Rtm: got delete of %s, but interface is still up",
	               ifp->name);
	   zlog_debug
	      ("Rtm: interface delete %s index %d flags %ld metric %d mtu %d",
	       ifp->name, ifp->ifindex, ifp->flags, ifp->metric, ifp->mtu);
  	}
#ifdef HAVE_SNMP
  ospf_snmp_if_delete (ifp);
#endif /* HAVE_SNMP */

  for (rn = route_top (IF_OIFS (ifp)); rn; rn = route_next (rn))
    if (rn->info)
      ospf_if_free ((struct ospf_interface *) rn->info);
/*changged by scx*/
#if 0
  ifp->ifindex = IFINDEX_INTERNAL;
#else
	if_delete(ifp);
#endif
  return 0;
}

static struct interface *
zebra_interface_if_lookup (struct stream *s)
{
  char ifname_tmp[INTERFACE_NAMSIZ];

  /* Read interface name. */
  stream_get (ifname_tmp, s, INTERFACE_NAMSIZ);

  /* And look it up. */
  return if_lookup_by_name_len(ifname_tmp,
			       strnlen(ifname_tmp, INTERFACE_NAMSIZ));
}

static int
ospf_interface_state_up (int command, struct zclient *zclient,
                         zebra_size_t length)
{
  struct interface *ifp;
  struct ospf_interface *oi;
  struct route_node *rn;

  ifp = zebra_interface_if_lookup (zclient->ibuf);

  if (ifp == NULL)
    return 0;

  /* Interface is already up. */
  if (if_is_operative (ifp))
    {
      /* Temporarily keep ifp values. */
      struct interface if_tmp;
      memcpy (&if_tmp, ifp, sizeof (struct interface));

      zebra_interface_if_set_value (zclient->ibuf, ifp);

      if (IS_DEBUG_OSPF (zebra, ZEBRA_INTERFACE))
        zlog_debug ("Rtm: Interface[%s] state update.", ifp->name);

      if (if_tmp.bandwidth != ifp->bandwidth)
        {
          if (IS_DEBUG_OSPF (zebra, ZEBRA_INTERFACE))
            zlog_debug ("Rtm: Interface[%s] bandwidth change %d -> %d.",
                       ifp->name, if_tmp.bandwidth, ifp->bandwidth);

          ospf_if_recalculate_output_cost (ifp);
        }

      if (if_tmp.mtu != ifp->mtu)
        {
          if (IS_DEBUG_OSPF (zebra, ZEBRA_INTERFACE))
            zlog_debug ("Rtm: Interface[%s] MTU change %u -> %u.",
                       ifp->name, if_tmp.mtu, ifp->mtu);

	  /* Must reset the interface (simulate down/up) when MTU changes. */
          ospf_if_reset(ifp);
	}
      return 0;
    }

  zebra_interface_if_set_value (zclient->ibuf, ifp);

  if (IS_DEBUG_OSPF (zebra, ZEBRA_INTERFACE))
    zlog_debug ("Rtm: Interface[%s] state change to up.", ifp->name);

  for (rn = route_top (IF_OIFS (ifp)); rn; rn = route_next (rn))
    {
      if ((oi = rn->info) == NULL)
        continue;

      ospf_if_up (oi);
    }

  return 0;
}

static int
ospf_interface_state_down (int command, struct zclient *zclient,
                           zebra_size_t length)
{
  struct interface *ifp;
  struct ospf_interface *oi;
  struct route_node *node;

  ifp = zebra_interface_state_read (zclient->ibuf);

  if (ifp == NULL)
    return 0;

  if (IS_DEBUG_OSPF (zebra, ZEBRA_INTERFACE))
    zlog_debug ("Rtm: Interface[%s] state change to down.", ifp->name);

  for (node = route_top (IF_OIFS (ifp)); node; node = route_next (node))
    {
      if ((oi = node->info) == NULL)
        continue;
      ospf_if_down (oi);
    }

  return 0;
}

static int
ospf_interface_address_add (int command, struct zclient *zclient,
                            zebra_size_t length)
{
  struct ospf *ospf;
  struct connected *c;

  c = zebra_interface_address_read (command, zclient->ibuf);

  if (c == NULL)
    return 0;

  ospf = ospf_lookup ();
  if (ospf != NULL)
    ospf_if_update (ospf);

#ifdef HAVE_SNMP
  ospf_snmp_if_update (c->ifp);
#endif /* HAVE_SNMP */

  return 0;
}

static int
ospf_interface_address_delete (int command, struct zclient *zclient,
                               zebra_size_t length)
{
  struct ospf *ospf;
  struct connected *c;
  struct interface *ifp;
  struct ospf_interface *oi;
  struct route_node *rn;
  struct prefix p;

  c = zebra_interface_address_read (command, zclient->ibuf);

  if (c == NULL)
    return 0;

  ifp = c->ifp;
  p = *c->address;
  p.prefixlen = IPV4_MAX_PREFIXLEN;

  rn = route_node_lookup (IF_OIFS (ifp), &p);
  if (!rn)
    {
      connected_free (c);
      return 0;
    }

  assert (rn->info);
  oi = rn->info;

  /* Call interface hook functions to clean up */
  ospf_if_free (oi);

#ifdef HAVE_SNMP
  ospf_snmp_if_update (c->ifp);
#endif /* HAVE_SNMP */

  connected_free (c);

  ospf = ospf_lookup ();
  if (ospf != NULL)
    ospf_if_update (ospf);

  return 0;
}

void
ospf_zebra_add (struct prefix_ipv4 *p, struct ospf_route *or)
{
  u_char message;
  u_char distance;
  u_char flags;
  int psize;
  struct stream *s;
  struct ospf_path *path;
  struct listnode *node;

  if (zclient->redist[ZEBRA_ROUTE_OSPF])
    {
      message = 0;
      flags = 0;

      /* OSPF pass nexthop and metric */
      SET_FLAG (message, ZAPI_MESSAGE_NEXTHOP);
      SET_FLAG (message, ZAPI_MESSAGE_METRIC);

      /* Distance value. */
      distance = ospf_distance_apply (p, or);
      if (distance)
        SET_FLAG (message, ZAPI_MESSAGE_DISTANCE);

      /* Make packet. */
      s = zclient->obuf;
      stream_reset (s);

      /* Put command, type, flags, message. */
      zclient_create_header (s, ZEBRA_IPV4_ROUTE_ADD);
      stream_putc (s, ZEBRA_ROUTE_OSPF);
      stream_putc (s, flags);
      stream_putc (s, message);

      /* Put prefix information. */
      psize = PSIZE (p->prefixlen);
      stream_putc (s, p->prefixlen);
      stream_write (s, (u_char *) & p->prefix, psize);

      /* Nexthop count. */
      stream_putc (s, or->paths->count);

      /* Nexthop, ifindex, distance and metric information. */
      for (ALL_LIST_ELEMENTS_RO (or->paths, node, path))
        {
          if (path->nexthop.s_addr != INADDR_ANY)
            {
              stream_putc (s, ZEBRA_NEXTHOP_IPV4);
              stream_put_in_addr (s, &path->nexthop);
            }
          else
            {
              stream_putc (s, ZEBRA_NEXTHOP_IFINDEX);
              if (path->oi)
                stream_putl (s, path->oi->ifp->ifindex);
              else
                stream_putl (s, 0);
            }

          if (IS_DEBUG_OSPF (zebra, ZEBRA_REDISTRIBUTE))
            {
              zlog_debug ("Rtm: Route add %s/%d nexthop %s",
                         inet_ntoa (p->prefix),
                         p->prefixlen, inet_ntoa (path->nexthop));
            }
        }

      if (CHECK_FLAG (message, ZAPI_MESSAGE_DISTANCE))
        stream_putc (s, distance);
      if (CHECK_FLAG (message, ZAPI_MESSAGE_METRIC))
        {
          if (or->path_type == OSPF_PATH_TYPE1_EXTERNAL)
            stream_putl (s, or->cost + or->u.ext.type2_cost);
          else if (or->path_type == OSPF_PATH_TYPE2_EXTERNAL)
            stream_putl (s, or->u.ext.type2_cost);
          else
            stream_putl (s, or->cost);
        }

      stream_putw_at (s, 0, stream_get_endp (s));

      zclient_send_message(zclient);
    }
}

void
ospf_zebra_delete (struct prefix_ipv4 *p, struct ospf_route *or)
{
  struct zapi_ipv4 api;
  struct ospf_path *path;
  struct in_addr *nexthop;
  struct listnode *node, *nnode;

  if (zclient->redist[ZEBRA_ROUTE_OSPF])
    {
      api.type = ZEBRA_ROUTE_OSPF;
      api.flags = 0;
      api.message = 0;
      api.ifindex_num = 0;
      api.nexthop_num = 0;

      for (ALL_LIST_ELEMENTS (or->paths, node, nnode, path))
        {
          if (path->nexthop.s_addr != INADDR_ANY)
            {
              SET_FLAG (api.message, ZAPI_MESSAGE_NEXTHOP);
              api.nexthop_num = 1;
              nexthop = &path->nexthop;
              api.nexthop = &nexthop;
            }
          else if (ospf_if_exists(path->oi) && (path->oi->ifp))
            {
              SET_FLAG (api.message, ZAPI_MESSAGE_NEXTHOP);
              api.ifindex_num = 1;
              api.ifindex = &path->oi->ifp->ifindex;
            }
          else if ( IS_DEBUG_OSPF(zebra,ZEBRA_REDISTRIBUTE) )
            {
              zlog_debug("Rtm: no ifp %s %d",
                         inet_ntoa(p->prefix),
                         p->prefixlen);
            }

          zapi_ipv4_route (ZEBRA_IPV4_ROUTE_DELETE, zclient, p, &api);

          if (IS_DEBUG_OSPF (zebra, ZEBRA_REDISTRIBUTE) && api.nexthop_num)
            {
              zlog_debug ("Rtm: Route delete %s/%d nexthop %s",
                         inet_ntoa (p->prefix),
                         p->prefixlen, inet_ntoa (**api.nexthop));
            }
          if (IS_DEBUG_OSPF (zebra, ZEBRA_REDISTRIBUTE) && api.ifindex_num)
            {
              zlog_debug ("Rtm: Route delete %s/%d ifindex %d",
                         inet_ntoa (p->prefix),
                         p->prefixlen, *api.ifindex);
            }
        }
    }
}

void
ospf_zebra_add_discard (struct prefix_ipv4 *p)
{
  struct zapi_ipv4 api;

  if (zclient->redist[ZEBRA_ROUTE_OSPF])
    {
      api.type = ZEBRA_ROUTE_OSPF;
      api.flags = ZEBRA_FLAG_BLACKHOLE;
      api.message = 0;
      SET_FLAG (api.message, ZAPI_MESSAGE_NEXTHOP);
      api.nexthop_num = 0;
      api.ifindex_num = 0;

      zapi_ipv4_route (ZEBRA_IPV4_ROUTE_ADD, zclient, p, &api);
    }
}

void
ospf_zebra_delete_discard (struct prefix_ipv4 *p)
{
  struct zapi_ipv4 api;

  if (zclient->redist[ZEBRA_ROUTE_OSPF])
    {
      api.type = ZEBRA_ROUTE_OSPF;
      api.flags = ZEBRA_FLAG_BLACKHOLE;
      api.message = 0;
      SET_FLAG (api.message, ZAPI_MESSAGE_NEXTHOP);
      api.nexthop_num = 0;
      api.ifindex_num = 0;

      zapi_ipv4_route (ZEBRA_IPV4_ROUTE_DELETE, zclient, p, &api);

      if (IS_DEBUG_OSPF (zebra, ZEBRA_REDISTRIBUTE))
        zlog_debug ("Rtm: Route delete discard %s/%d",
                   inet_ntoa (p->prefix), p->prefixlen);

    }
}

int
ospf_is_type_redistributed (int type)
{
  return (DEFAULT_ROUTE_TYPE (type)) ?
    zclient->default_information : zclient->redist[type];
}

int
ospf_redistribute_set (struct ospf *ospf, int type, int mtype, int mvalue)
{
  int force = 0;

  if (ospf_is_type_redistributed (type))
    {
      if (mtype != ospf->dmetric[type].type)
        {
          ospf->dmetric[type].type = mtype;
          force = LSA_REFRESH_FORCE;
        }
      if (mvalue != ospf->dmetric[type].value)
        {
          ospf->dmetric[type].value = mvalue;
          force = LSA_REFRESH_FORCE;
        }

      ospf_external_lsa_refresh_type (ospf, type, force);

      if (IS_DEBUG_OSPF (zebra, ZEBRA_REDISTRIBUTE))
        zlog_debug ("Redistribute[%s]: Refresh  Type[%d], Metric[%d]",
                   ospf_redist_string(type),
                   metric_type (ospf, type), metric_value (ospf, type));

      return CMD_SUCCESS;
    }

  ospf->dmetric[type].type = mtype;
  ospf->dmetric[type].value = mvalue;

  zclient_redistribute (ZEBRA_REDISTRIBUTE_ADD, zclient, type);

  if (IS_DEBUG_OSPF (zebra, ZEBRA_REDISTRIBUTE))
    zlog_debug ("Redistribute[%s]: Start  Type[%d], Metric[%d]",
               ospf_redist_string(type),
               metric_type (ospf, type), metric_value (ospf, type));

  ospf_asbr_status_update (ospf, ++ospf->redistribute);

  return CMD_SUCCESS;
}

int
ospf_redistribute_unset (struct ospf *ospf, int type)
{
  if (type == zclient->redist_default)
    return CMD_SUCCESS;

  if (!ospf_is_type_redistributed (type))
    return CMD_SUCCESS;

  zclient_redistribute (ZEBRA_REDISTRIBUTE_DELETE, zclient, type);

  if (IS_DEBUG_OSPF (zebra, ZEBRA_REDISTRIBUTE))
    zlog_debug ("Redistribute[%s]: Stop",
               ospf_redist_string(type));

  ospf->dmetric[type].type = -1;
  ospf->dmetric[type].value = -1;

  /* Remove the routes from OSPF table. */
  ospf_redistribute_withdraw (ospf, type);

  ospf_asbr_status_update (ospf, --ospf->redistribute);

  return CMD_SUCCESS;
}

int
ospf_redistribute_default_set (struct ospf *ospf, int originate,
                               int mtype, int mvalue)
{
  int force = 0;

  if (ospf_is_type_redistributed (DEFAULT_ROUTE))
    {
      if (mtype != ospf->dmetric[DEFAULT_ROUTE].type)
        {
          ospf->dmetric[DEFAULT_ROUTE].type = mtype;
          force = 1;
        }
      if (mvalue != ospf->dmetric[DEFAULT_ROUTE].value)
        {
          force = 1;
          ospf->dmetric[DEFAULT_ROUTE].value = mvalue;
        }

      ospf_external_lsa_refresh_default (ospf);

      if (IS_DEBUG_OSPF (zebra, ZEBRA_REDISTRIBUTE))
        zlog_debug ("Redistribute[%s]: Refresh  Type[%d], Metric[%d]",
                   ospf_redist_string(DEFAULT_ROUTE),
                   metric_type (ospf, DEFAULT_ROUTE),
                   metric_value (ospf, DEFAULT_ROUTE));
      return CMD_SUCCESS;
    }

  ospf->default_originate = originate;
  ospf->dmetric[DEFAULT_ROUTE].type = mtype;
  ospf->dmetric[DEFAULT_ROUTE].value = mvalue;

  zclient_redistribute_default (ZEBRA_REDISTRIBUTE_DEFAULT_ADD, zclient);

  if (IS_DEBUG_OSPF (zebra, ZEBRA_REDISTRIBUTE))
    zlog_debug ("Redistribute[DEFAULT]: Start  Type[%d], Metric[%d]",
               metric_type (ospf, DEFAULT_ROUTE),
               metric_value (ospf, DEFAULT_ROUTE));

  if (ospf->router_id.s_addr == 0)
    ospf->external_origin |= (1 << DEFAULT_ROUTE);
  else
    thread_add_timer (master, ospf_default_originate_timer, ospf, 1);

  ospf_asbr_status_update (ospf, ++ospf->redistribute);

  return CMD_SUCCESS;
}

int
ospf_redistribute_default_unset (struct ospf *ospf)
{
  if (!ospf_is_type_redistributed (DEFAULT_ROUTE))
    return CMD_SUCCESS;

  ospf->default_originate = DEFAULT_ORIGINATE_NONE;
  ospf->dmetric[DEFAULT_ROUTE].type = -1;
  ospf->dmetric[DEFAULT_ROUTE].value = -1;

  zclient_redistribute_default (ZEBRA_REDISTRIBUTE_DEFAULT_DELETE, zclient);

  if (IS_DEBUG_OSPF (zebra, ZEBRA_REDISTRIBUTE))
    zlog_debug ("Redistribute[DEFAULT]: Stop");

  ospf_asbr_status_update (ospf, --ospf->redistribute);

  return CMD_SUCCESS;
}

static int
ospf_external_lsa_originate_check (struct ospf *ospf,
                                   struct external_info *ei)
{
  /* If prefix is multicast, then do not originate LSA. */
  if (IN_MULTICAST (htonl (ei->p.prefix.s_addr)))
    {
	  if (IS_DEBUG_OSPF_LSA)
	  	zlog_info ("LSA[Type5:%s]: Not originate AS-external-LSA, "
                 "Prefix belongs multicast", inet_ntoa (ei->p.prefix));
      return 0;
    }

  /* Take care of default-originate. */
  if (is_prefix_default (&ei->p))
    if (ospf->default_originate == DEFAULT_ORIGINATE_NONE)
      {
		if (IS_DEBUG_OSPF_LSA)
	        zlog_info ("LSA[Type5:0.0.0.0]: Not originate AS-exntenal-LSA "
                   "for default");
        return 0;
      }

  return 1;
}

/* If connected prefix is OSPF enable interface, then do not announce. */
int
ospf_distribute_check_connected (struct ospf *ospf, struct external_info *ei)
{
  struct route_node *rn;

  for (rn = route_top (ospf->networks); rn; rn = route_next (rn))
    if (rn->info != NULL)
      if (prefix_match (&rn->p, (struct prefix *) &ei->p))
        {
          route_unlock_node (rn);
          return 0;
        }

  return 1;
}

/* return 1 if external LSA must be originated, 0 otherwise */
int
ospf_redistribute_check (struct ospf *ospf,
                         struct external_info *ei, int *changed)
{
  struct route_map_set_values save_values;
  struct prefix_ipv4 *p = &ei->p;
  u_char type = is_prefix_default (&ei->p) ? DEFAULT_ROUTE : ei->type;

  if (changed)
    *changed = 0;

  if (!ospf_external_lsa_originate_check (ospf, ei))
    return 0;

  /* Take care connected route. */
  if (type == ZEBRA_ROUTE_CONNECT &&
      !ospf_distribute_check_connected (ospf, ei))
    return 0;

  if (!DEFAULT_ROUTE_TYPE (type) && DISTRIBUTE_NAME (ospf, type))
    /* distirbute-list exists, but access-list may not? */
    if (DISTRIBUTE_LIST (ospf, type))
      if (access_list_apply (DISTRIBUTE_LIST (ospf, type), p) == FILTER_DENY)
        {
          if (IS_DEBUG_OSPF (zebra, ZEBRA_REDISTRIBUTE))
            zlog_debug ("Redistribute[%s]: %s/%d filtered by ditribute-list.",
                       ospf_redist_string(type),
                       inet_ntoa (p->prefix), p->prefixlen);
          return 0;
        }

  save_values = ei->route_map_set;
  ospf_reset_route_map_set_values (&ei->route_map_set);

  /* apply route-map if needed */
  if (ROUTEMAP_NAME (ospf, type))
    {
      int ret;

      ret = route_map_apply (ROUTEMAP (ospf, type), (struct prefix *) p,
                             RMAP_OSPF, ei);

      if (ret == RMAP_DENYMATCH)
        {
          ei->route_map_set = save_values;
          if (IS_DEBUG_OSPF (zebra, ZEBRA_REDISTRIBUTE))
            zlog_debug ("Redistribute[%s]: %s/%d filtered by route-map.",
                       ospf_redist_string(type),
                       inet_ntoa (p->prefix), p->prefixlen);
          return 0;
        }

      /* check if 'route-map set' changed something */
      if (changed)
        *changed = !ospf_route_map_set_compare (&ei->route_map_set,
                                                &save_values);
    }

  return 1;
}

/* OSPF route-map set for redistribution */
void
ospf_routemap_set (struct ospf *ospf, int type, const char *name)
{
  if (ROUTEMAP_NAME (ospf, type))
    free (ROUTEMAP_NAME (ospf, type));

  ROUTEMAP_NAME (ospf, type) = strdup (name);
  ROUTEMAP (ospf, type) = route_map_lookup_by_name (name);
}

void
ospf_routemap_unset (struct ospf *ospf, int type)
{
  if (ROUTEMAP_NAME (ospf, type))
    free (ROUTEMAP_NAME (ospf, type));

  ROUTEMAP_NAME (ospf, type) = NULL;
  ROUTEMAP (ospf, type) = NULL;
}

/* Zebra route add and delete treatment. */
static int
ospf_zebra_read_ipv4 (int command, struct zclient *zclient,
                      zebra_size_t length)
{
  struct stream *s;
  struct zapi_ipv4 api;
  unsigned long ifindex;
  struct in_addr nexthop;
  struct prefix_ipv4 p;
  struct external_info *ei;
  struct ospf *ospf;

  s = zclient->ibuf;
  ifindex = 0;
  nexthop.s_addr = 0;

  /* Type, flags, message. */
  api.type = stream_getc (s);
  api.flags = stream_getc (s);
  api.message = stream_getc (s);

  /* IPv4 prefix. */
  memset (&p, 0, sizeof (struct prefix_ipv4));
  p.family = AF_INET;
  p.prefixlen = stream_getc (s);
  stream_get (&p.prefix, s, PSIZE (p.prefixlen));

  if (IPV4_NET127(ntohl(p.prefix.s_addr)))
    return 0;

  /* Nexthop, ifindex, distance, metric. */
  if (CHECK_FLAG (api.message, ZAPI_MESSAGE_NEXTHOP))
    {
      api.nexthop_num = stream_getc (s);
      nexthop.s_addr = stream_get_ipv4 (s);
    }
  if (CHECK_FLAG (api.message, ZAPI_MESSAGE_IFINDEX))
    {
      api.ifindex_num = stream_getc (s);
      /* XXX assert(api.ifindex_num == 1); */
      ifindex = stream_getl (s);
    }
  if (CHECK_FLAG (api.message, ZAPI_MESSAGE_DISTANCE))
    api.distance = stream_getc (s);
  if (CHECK_FLAG (api.message, ZAPI_MESSAGE_METRIC))
    api.metric = stream_getl (s);

  ospf = ospf_lookup ();
  if (ospf == NULL)
    return 0;

  if (command == ZEBRA_IPV4_ROUTE_ADD)
    {
      /* XXX|HACK|TODO|FIXME:
       * Maybe we should ignore reject/blackhole routes? Testing shows that
       * there is no problems though and this is only way to "summarize"
       * routes in ASBR at the moment. Maybe we need just a better generalised
       * solution for these types?
       *
       * if ( CHECK_FLAG (api.flags, ZEBRA_FLAG_BLACKHOLE)
       *     || CHECK_FLAG (api.flags, ZEBRA_FLAG_REJECT))
       * return 0;
       */
        
      ei = ospf_external_info_add (api.type, p, ifindex, nexthop);

      if (ospf->router_id.s_addr == 0)
        /* Set flags to generate AS-external-LSA originate event
           for each redistributed protocols later. */
        ospf->external_origin |= (1 << api.type);
      else
        {
          if (ei)
            {
              if (is_prefix_default (&p))
                ospf_external_lsa_refresh_default (ospf);
              else
                {
                  struct ospf_lsa *current;

                  current = ospf_external_info_find_lsa (ospf, &ei->p);
                  if (!current)
                    ospf_external_lsa_originate (ospf, ei);
                  else if (IS_LSA_MAXAGE (current))
                    ospf_external_lsa_refresh (ospf, current,
                                               ei, LSA_REFRESH_FORCE);
                  else
                    zlog_warn ("ospf_zebra_read_ipv4() : %s already exists",
                               inet_ntoa (p.prefix));
                }
            }
        }
    }
  else                          /* if (command == ZEBRA_IPV4_ROUTE_DELETE) */
    {
      ospf_external_info_delete (api.type, p);
      if (is_prefix_default (&p))
        ospf_external_lsa_refresh_default (ospf);
      else
        ospf_external_lsa_flush (ospf, api.type, &p, ifindex /*, nexthop */);
    }

  return 0;
}


int
ospf_distribute_list_out_set (struct ospf *ospf, int type, const char *name)
{
  /* Lookup access-list for distribute-list. */
  DISTRIBUTE_LIST (ospf, type) = access_list_lookup (AFI_IP, name);

  /* Clear previous distribute-name. */
  if (DISTRIBUTE_NAME (ospf, type))
    free (DISTRIBUTE_NAME (ospf, type));

  /* Set distribute-name. */
  DISTRIBUTE_NAME (ospf, type) = strdup (name);

  /* If access-list have been set, schedule update timer. */
  if (DISTRIBUTE_LIST (ospf, type))
    ospf_distribute_list_update (ospf, type);

  return CMD_SUCCESS;
}

int
ospf_distribute_list_out_unset (struct ospf *ospf, int type, const char *name)
{
  /* Schedule update timer. */
  if (DISTRIBUTE_LIST (ospf, type))
    ospf_distribute_list_update (ospf, type);

  /* Unset distribute-list. */
  DISTRIBUTE_LIST (ospf, type) = NULL;

  /* Clear distribute-name. */
  if (DISTRIBUTE_NAME (ospf, type))
    free (DISTRIBUTE_NAME (ospf, type));

  DISTRIBUTE_NAME (ospf, type) = NULL;

  return CMD_SUCCESS;
}

/* distribute-list update timer. */
static int
ospf_distribute_list_update_timer (struct thread *thread)
{
  struct route_node *rn;
  struct external_info *ei;
  struct route_table *rt;
  struct ospf_lsa *lsa;
  intptr_t type;
  struct ospf *ospf;

  type = (intptr_t)THREAD_ARG (thread);
  assert (type <= ZEBRA_ROUTE_MAX);
  
  rt = EXTERNAL_INFO (type);

  ospf = ospf_lookup ();
  if (ospf == NULL)
    return 0;

  ospf->t_distribute_update = NULL;
  if (IS_DEBUG_OSPF_EVENT)
  	zlog_info ("Rtm[Redistribute]: distribute-list update timer fired!");

  /* foreach all external info. */
  if (rt)
    for (rn = route_top (rt); rn; rn = route_next (rn))
      if ((ei = rn->info) != NULL)
        {
          if (is_prefix_default (&ei->p))
            ospf_external_lsa_refresh_default (ospf);
          else if ((lsa = ospf_external_info_find_lsa (ospf, &ei->p)))
            ospf_external_lsa_refresh (ospf, lsa, ei, LSA_REFRESH_IF_CHANGED);
          else
            ospf_external_lsa_originate (ospf, ei);
        }
  return 0;
}

#define OSPF_DISTRIBUTE_UPDATE_DELAY 5

/* Update distribute-list and set timer to apply access-list. */
void
ospf_distribute_list_update (struct ospf *ospf, int type)
{
  struct route_table *rt;

  /* External info does not exist. */
  if (!(rt = EXTERNAL_INFO (type)))
    return;

  /* If exists previously invoked thread, then cancel it. */
  if (ospf->t_distribute_update)
    OSPF_TIMER_OFF (ospf->t_distribute_update);

  /* Set timer. */
  ospf->t_distribute_update =
    thread_add_timer (master, ospf_distribute_list_update_timer,
                      (void *) type, OSPF_DISTRIBUTE_UPDATE_DELAY);
}

/* If access-list is updated, apply some check. */
static void
ospf_filter_update (struct access_list *access)
{
  struct ospf *ospf;
  int type;
  int abr_inv = 0;
  struct ospf_area *area;
  struct listnode *node;

  /* If OSPF instatnce does not exist, return right now. */
  ospf = ospf_lookup ();
  if (ospf == NULL)
    return;

  /* Update distribute-list, and apply filter. */
  for (type = 0; type <= ZEBRA_ROUTE_MAX; type++)
    {
      if (ROUTEMAP (ospf, type) != NULL)
        {
          /* if route-map is not NULL it may be using this access list */
          ospf_distribute_list_update (ospf, type);
          continue;
        }

      /* There is place for route-map for default-information (ZEBRA_ROUTE_MAX),
       * but no distribute list. */
      if (type == ZEBRA_ROUTE_MAX)
	break;

      if (DISTRIBUTE_NAME (ospf, type))
        {
          /* Keep old access-list for distribute-list. */
          struct access_list *old = DISTRIBUTE_LIST (ospf, type);

          /* Update access-list for distribute-list. */
          DISTRIBUTE_LIST (ospf, type) =
            access_list_lookup (AFI_IP, DISTRIBUTE_NAME (ospf, type));

          /* No update for this distribute type. */
          if (old == NULL && DISTRIBUTE_LIST (ospf, type) == NULL)
            continue;

          /* Schedule distribute-list update timer. */
          if (DISTRIBUTE_LIST (ospf, type) == NULL ||
              strcmp (DISTRIBUTE_NAME (ospf, type), access->name) == 0)
            ospf_distribute_list_update (ospf, type);
        }
    }

  /* Update Area access-list. */
  for (ALL_LIST_ELEMENTS_RO (ospf->areas, node, area))
    {
      if (EXPORT_NAME (area))
        {
          EXPORT_LIST (area) = NULL;
          abr_inv++;
        }

      if (IMPORT_NAME (area))
        {
          IMPORT_LIST (area) = NULL;
          abr_inv++;
        }
    }

  /* Schedule ABR tasks -- this will be changed -- takada. */
  if (IS_OSPF_ABR (ospf) && abr_inv)
    ospf_schedule_abr_task (ospf);
}

/* If prefix-list is updated, do some updates. */
void
ospf_prefix_list_update (struct prefix_list *plist)
{
  struct ospf *ospf;
  int type;
  int abr_inv = 0;
  struct ospf_area *area;
  struct listnode *node;

  /* If OSPF instatnce does not exist, return right now. */
  ospf = ospf_lookup ();
  if (ospf == NULL)
    return;

  /* Update all route-maps which are used as redistribution filters.
   * They might use prefix-list.
   */
  for (type = 0; type <= ZEBRA_ROUTE_MAX; type++)
    {
      if (ROUTEMAP (ospf, type) != NULL)
        {
          /* If route-map is not NULL it may be using this prefix list */
          ospf_distribute_list_update (ospf, type);
          continue;
        }
    }

  /* Update area filter-lists. */
  for (ALL_LIST_ELEMENTS_RO (ospf->areas, node, area))
    {
      /* Update filter-list in. */
      if (PREFIX_NAME_IN (area))
        if (strcmp (PREFIX_NAME_IN (area), plist->name) == 0)
          {
            PREFIX_LIST_IN (area) =
              prefix_list_lookup (AFI_IP, PREFIX_NAME_IN (area));
            abr_inv++;
          }

      /* Update filter-list out. */
      if (PREFIX_NAME_OUT (area))
        if (strcmp (PREFIX_NAME_OUT (area), plist->name) == 0)
          {
            PREFIX_LIST_IN (area) =
              prefix_list_lookup (AFI_IP, PREFIX_NAME_OUT (area));
            abr_inv++;
          }
    }

  /* Schedule ABR task. */
  if (IS_OSPF_ABR (ospf) && abr_inv)
    ospf_schedule_abr_task (ospf);
}

static struct ospf_distance *
ospf_distance_new (void)
{
  struct ospf_distance *new;
  new = XMALLOC (MTYPE_OSPF_DISTANCE, sizeof (struct ospf_distance));
  memset (new, 0, sizeof (struct ospf_distance));
  return new;
}

static void
ospf_distance_free (struct ospf_distance *odistance)
{
  XFREE (MTYPE_OSPF_DISTANCE, odistance);
}

int
ospf_distance_set (struct vty *vty, struct ospf *ospf, 
                   const char *distance_str,
                   const char *ip_str, 
                   const char *access_list_str)
{
  int ret;
  struct prefix_ipv4 p;
  u_char distance;
  struct route_node *rn;
  struct ospf_distance *odistance;

  ret = str2prefix_ipv4 (ip_str, &p);
  if (ret == 0)
    {
      vty_out (vty, "Malformed prefix%s", VTY_NEWLINE);
      return CMD_WARNING;
    }

  distance = atoi (distance_str);

  /* Get OSPF distance node. */
  rn = route_node_get (ospf->distance_table, (struct prefix *) &p);
  if (rn->info)
    {
      odistance = rn->info;
      route_unlock_node (rn);
    }
  else
    {
      odistance = ospf_distance_new ();
      rn->info = odistance;
    }

  /* Set distance value. */
  odistance->distance = distance;

  /* Reset access-list configuration. */
  if (odistance->access_list)
    {
      free (odistance->access_list);
      odistance->access_list = NULL;
    }
  if (access_list_str)
    odistance->access_list = strdup (access_list_str);

  return CMD_SUCCESS;
}

int
ospf_distance_unset (struct vty *vty, struct ospf *ospf, 
                     const char *distance_str,
                     const char *ip_str, char 
                     const *access_list_str)
{
  int ret;
  struct prefix_ipv4 p;
  u_char distance;
  struct route_node *rn;
  struct ospf_distance *odistance;

  ret = str2prefix_ipv4 (ip_str, &p);
  if (ret == 0)
    {
      vty_out (vty, "Malformed prefix%s", VTY_NEWLINE);
      return CMD_WARNING;
    }

  distance = atoi (distance_str);

  rn = route_node_lookup (ospf->distance_table, (struct prefix *) &p);
  if (!rn)
    {
      vty_out (vty, "Can't find specified prefix%s", VTY_NEWLINE);
      return CMD_WARNING;
    }

  odistance = rn->info;

  if (odistance->access_list)
    free (odistance->access_list);
  ospf_distance_free (odistance);

  rn->info = NULL;
  route_unlock_node (rn);
  route_unlock_node (rn);

  return CMD_SUCCESS;
}

void
ospf_distance_reset (struct ospf *ospf)
{
  struct route_node *rn;
  struct ospf_distance *odistance;

  for (rn = route_top (ospf->distance_table); rn; rn = route_next (rn))
    if ((odistance = rn->info) != NULL)
      {
        if (odistance->access_list)
          free (odistance->access_list);
        ospf_distance_free (odistance);
        rn->info = NULL;
        route_unlock_node (rn);
      }
}

u_char
ospf_distance_apply (struct prefix_ipv4 *p, struct ospf_route *or)
{
  struct ospf *ospf;

  ospf = ospf_lookup ();
  if (ospf == NULL)
    return 0;

  if (ospf->distance_intra)
    if (or->path_type == OSPF_PATH_INTRA_AREA)
      return ospf->distance_intra;

  if (ospf->distance_inter)
    if (or->path_type == OSPF_PATH_INTER_AREA)
      return ospf->distance_inter;

  if (ospf->distance_external)
    if (or->path_type == OSPF_PATH_TYPE1_EXTERNAL
        || or->path_type == OSPF_PATH_TYPE2_EXTERNAL)
      return ospf->distance_external;

  if (ospf->distance_all)
    return ospf->distance_all;

  return 0;
}

void
ospf_zebra_init ()
{
  /* Allocate zebra structure. */
  zclient = zclient_new ();
  zclient_init (zclient, ZEBRA_ROUTE_OSPF);
  zclient->router_id_update = ospf_router_id_update_zebra;
  zclient->interface_add = ospf_interface_add;
  zclient->interface_delete = ospf_interface_delete;
  zclient->interface_up = ospf_interface_state_up;
  zclient->interface_down = ospf_interface_state_down;
  zclient->interface_address_add = ospf_interface_address_add;
  zclient->interface_address_delete = ospf_interface_address_delete;
  zclient->ipv4_route_add = ospf_zebra_read_ipv4;
  zclient->ipv4_route_delete = ospf_zebra_read_ipv4;

  access_list_add_hook (ospf_filter_update);
  access_list_delete_hook (ospf_filter_update);
  prefix_list_add_hook (ospf_prefix_list_update);
  prefix_list_delete_hook (ospf_prefix_list_update);
 dcli_dbus_init_remote();
}
