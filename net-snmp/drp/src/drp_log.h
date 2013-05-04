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

/* drp_log.h */
#ifndef _DRP_LOG_H
#define _DRP_LOG_H
#include <syslog.h>

//#ifdef  __cplusplus
//extern "C" {
//#endif

/*syslog  level*/
#define LOG_EMERG_MASK			(LOG_MASK(LOG_EMERG))
#define LOG_ALERT_MASK			(LOG_MASK(LOG_ALERT))
#define LOG_CRIT_MASK			(LOG_MASK(LOG_CRIT))
#define LOG_ERR_MASK			(LOG_MASK(LOG_ERR))
#define LOG_WARNING_MASK		(LOG_MASK(LOG_WARNING))
#define LOG_NOTICE_MASK			(LOG_MASK(LOG_NOTICE))
#define LOG_INFO_MASK			(LOG_MASK(LOG_INFO))
#define LOG_DEBUG_MASK			(LOG_MASK(LOG_DEBUG))
#define LOG_USER_MASK			(LOG_MASK(LOG_USER))

#define LOG_AT_LEAST_ALERT		LOG_EMERG_MASK|LOG_ALERT_MASK
#define LOG_AT_LEAST_CRIT		LOG_AT_LEAST_ALERT|LOG_CRIT_MASK
#define LOG_AT_LEAST_ERR		LOG_AT_LEAST_CRIT|LOG_ERR_MASK
#define LOG_AT_LEAST_WARNING	LOG_AT_LEAST_ERR|LOG_WARNING_MASK
#define LOG_AT_LEAST_NOTICE		LOG_AT_LEAST_WARNING|LOG_NOTICE_MASK
#define LOG_AT_LEAST_INFO		LOG_AT_LEAST_NOTICE|LOG_INFO_MASK
#define LOG_AT_LEAST_DEBUG		LOG_AT_LEAST_INFO|LOG_DEBUG_MASK
#define LOG_AT_LEAST_USER		LOG_AT_LEAST_DEBUG|LOG_USER_MASK

int 
drp_log_init( char *daemon );
int
drp_log_uninit();
int 
drp_log_set_level(int level);
const char *
safe_strerror(int errnum);

extern int forward;

#if 0

#define SYSLOG(level,fmt,args...) if(forward){printf("file:%s line:%d   "fmt,__FILE__,__LINE__, ##args );printf("\n");}\
	else syslog(level,fmt, ##args )

#define eag_log_syslog( level, match, fmt, args... ) \
	if( EAG_TRUE == eag_log_is_filter_register(match) )\
{	\
	if(forward) {printf("file:%s line:%d   "fmt, __FILE__,__LINE__, ## args);printf("\n");}\
	else syslog( level, "file:%s line:%d   "fmt, __FILE__,__LINE__, ## args ); \
}

#else
#define SYSLOG(level,fmt,args...) if(forward){printf(fmt, ##args );printf("\n");}\
	else syslog(level,"%s:%d:"fmt,__FILE__,__LINE__,##args )

#define drp_log_syslog( level, match, fmt, args... ) \
{	\
	if(forward) {printf(fmt, ## args);printf("\n");}\
	else syslog( level, fmt, ## args ); \
}

#endif

#ifdef	log_test_use
#define drp_log_debug( fmt, args... ) 	\
	printf(fmt"\n",## args)

#define drp_trace_log( fmt, args... ) 	\
	printf("%s[%d]@%s:"fmt"\n",__FILE__,__LINE__,__func__,## args)

#define drp_log_info( fmt, args... ) 	printf(fmt"\n",## args)
#define drp_log_notice( fmt, args... ) 	printf(fmt"\n",## args)
#define drp_log_warning( fmt, args... ) 	printf(fmt"\n",## args)
#define drp_log_err( fmt, args... ) 		printf(fmt"\n",## args)
#define drp_log_crit( fmt, args... ) 		printf(fmt"\n",## args)

#define admin_log_notice( fmt, args...) 	printf(fmt"\n", ## args)
#else
#define drp_log_debug( fmt, args... ) 	\
	SYSLOG(LOG_DEBUG,fmt,## args)

#define drp_trace_log( fmt, args... ) 	\
	drp_log_syslog(LOG_DEBUG,"%s[%d]@%s:"fmt,__FILE__,__LINE__,__func__,## args)

#define drp_log_info( fmt, args... ) 	SYSLOG(LOG_INFO,fmt,## args)
#define drp_log_notice( fmt, args... ) 	SYSLOG(LOG_NOTICE,fmt,## args)
#define drp_log_warning( fmt, args... ) 	SYSLOG(LOG_WARNING,fmt,## args)
#define drp_log_err( fmt, args... ) 		SYSLOG(LOG_ERR,fmt,## args)
#define drp_log_crit( fmt, args... ) 		SYSLOG(LOG_CRIT,fmt,## args)

#define admin_log_notice( fmt, args...) 	SYSLOG(LOG_LOCAL7|LOG_NOTICE,fmt, ## args)
#endif

DBusMessage *
drp_dbus_method_log_debug ( DBusConnection *conn, 
					DBusMessage *msg, 
					void *user_data );


//#ifdef  __cplusplus
//}
//#endif
#endif				/* _DRP_LOG_H */
