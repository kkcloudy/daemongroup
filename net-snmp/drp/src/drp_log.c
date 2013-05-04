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
 *
 *
 * CREATOR:
 * autelan.software.xxx. team
 *
 * DESCRIPTION:
 * xxx module main routine
 *
 *
 *******************************************************************************/

/* drp_log.c */
#include <syslog.h>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <dbus/dbus.h>

#include "nm_list.h"
#include "drp_def.h"
#include "drp_log.h"


int forward = 0;

	const char *
safe_strerror(int errnum)
{
	const char *s = strerror(errnum);
	return (s != NULL) ? s : "Unknown error";
}


	int
drp_log_init( char *daemon )
{
	static int flag = 0;
	static char prefix[32];/*it must be static. because openlog not copy the str but save the pointer!!!*/
	strncpy( prefix, daemon, sizeof(prefix)-1 );

	if (1 != flag) {
		openlog(prefix, LOG_PID, LOG_DAEMON);
		setlogmask(LOG_AT_LEAST_INFO);
	}
	flag = 1;

	return DRP_RETURN_OK;
}

	int
drp_log_uninit()
{
	closelog();
	return DRP_RETURN_OK;
}

/*return prev log level mask*/
int
drp_log_set_level(int level)
{
	return setlogmask(level);
}

DBusMessage *
drp_dbus_method_log_debug ( DBusConnection *conn, 
					DBusMessage *msg, 
					void *user_data )
{
	DBusMessage *reply = NULL;
	int ret = DRP_RETURN_OK;
	DBusError		err = {0};	  
	unsigned int level = 0;

	dbus_message_get_args(	msg,
							&err,
							DBUS_TYPE_INT32, &level,
							DBUS_TYPE_INVALID );

	reply = dbus_message_new_method_return(msg);
	if (NULL == reply) 
	{
		drp_log_err("ac_sample_dbus_method_set_sample_state create reply failed" );
		return reply;
	}

	ret = drp_log_set_level(level);	/*return prev log level mask*/

	dbus_message_append_args( reply,
							DBUS_TYPE_INT32, &ret,
							DBUS_TYPE_INVALID );
	
	return reply;
}


