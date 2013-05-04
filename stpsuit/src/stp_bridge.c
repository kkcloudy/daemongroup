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
* stp_bridge.c
*
* CREATOR:
*       zhubo@autelan.com
*
* DESCRIPTION:
*       stp module main routine
*
* DATE:
*       04/18/2008
*
*  FILE REVISION NUMBER:
*       $Revision: 1.2 $
*******************************************************************************/
#ifdef __cplusplus
extern "C" {
#endif
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <string.h>
#include <sys/wait.h>
#include <sys/time.h>
#include <sys/types.h>
#include <signal.h>
#include <unistd.h>
#include <errno.h>
#include <readline/readline.h>
#include <sys/select.h>
#include <pthread.h>

#include "stp_cli.h"
#include "stp_mngr.h"
#include "stp_npd_cmd.h"
#include "stp_dcli.h"

#include "stp_base.h"
#include "stp_bitmap.h"
#include "stp_bpdu.h"
#include "stp_uid.h"
#include "stp_stpm.h"
#include "stp_in.h"
#include "stp_dbus.h"
#include "stp_log.h"

/*
* # main
* @$ init port bridge
* 
*
*/

long        my_pid = 0;
unsigned int  record_port = 0;
BITMAP_T    enabled_ports;
unsigned int pktsck,npdsck,netlinkSck,lnkstsck;
Bool  g_flag = False;
char shutdown_flag = 0;
unsigned int productId = 0;

/* For distributed system  2011-09-09  zhangchunlong */
unsigned int stp_slot_id = 0;
unsigned int stp_is_distributed = 0;
static int get_num_from_file(const char *filename)
{
    FILE *fp = NULL;
    int data = -1;
    
    fp = fopen(filename, "r");
    if (fp)
    {
        fscanf(fp, "%d", &data);
		fclose(fp);
    }
	else
        stp_syslog_dbg("%s open error!\n", filename);
	
    stp_syslog_dbg("read %s return:%d \n",filename,data);
    return data;
}



extern void stp_mgmt_stpm_delete_all ();

int stp_bridge_main_loop (void)
{	
	fd_set        readfds;
  	struct timeval    tv,ct; 
  	struct timeval    now, earliest;
  	int           rc, tmpNum, numfds,  kkk;

	ct.tv_sec = 0;
	ct.tv_usec = 0;
#if 1	/* for stp bug:AXSSZFI-291 on distributed system */
	netlinkSck = stp_netlink_socket_init();
	if (netlinkSck < 0) {
		return -1;
	}
#endif
  	pktsck = stp_uid_bpduPktSocketInit();
  
	if(pktsck < 0)
	{
		return -1;
	}
 	npdsck = stp_npd_socket_init();
	if(npdsck < 0)
	{
		return -1;
	}
#if 1	/* for stp bug:AXSSZFI-291 on distributed system */
	lnkstsck = stp_wan_lnk_socket_init();
	stp_syslog_error("lnksck %d\n",lnkstsck);
	if(lnkstsck < 0)
	{
		return -1;
	}
#endif
	
	/* send msg to request sys mac*/
	stp_npd_cmd_send_stp_mac();
	
  	gettimeofday(&earliest, NULL);
  	earliest.tv_sec++;

  	do 
	{
    	numfds = -1;
    	FD_ZERO(&readfds);
#if 0
    	kkk = 0; /* stdin for commands */
    	FD_SET(kkk, &readfds);
    	if (kkk > numfds) numfds = kkk;
#endif
    	FD_SET(pktsck, &readfds);
		FD_SET(npdsck,&readfds);
#if 1   /* for stp bug:AXSSZFI-291 on distributed system */
		FD_SET(netlinkSck, &readfds);
		FD_SET(lnkstsck,&readfds);

		tmpNum = (netlinkSck > tmpNum) ? netlinkSck : tmpNum;
		tmpNum = (lnkstsck > tmpNum) ? lnkstsck : tmpNum;
#endif
		tmpNum = (pktsck > npdsck) ? pktsck : npdsck;
		
		#if 1
		if (tmpNum > numfds) numfds = tmpNum;

    	if (numfds < 0)
      		numfds = 0;
    	else
      		numfds++;
        #endif
		
    	gettimeofday (&now, 0);
    	tv.tv_usec = 0;
    	tv.tv_sec = 0;

    	if (now.tv_sec < earliest.tv_sec) 
		{ 
			/* we must wait more than 1 sec. */
      		tv.tv_sec = 1;
      		tv.tv_usec = 2000;
    	}
		else if (now.tv_sec == earliest.tv_sec) 
		{
		   /* canceled by zhengcs@autelan.com
	      		if (now.tv_usec < earliest.tv_usec)
				{
	        		if (earliest.tv_usec < now.tv_usec)
	          			tv.tv_usec = 0;
	       	 		else
	          			tv.tv_usec = earliest.tv_usec - now.tv_usec;
	      		}
	      	  */
	      	 if (earliest.tv_usec < now.tv_usec)
	          	tv.tv_usec = 0;
	       	 else
	          	tv.tv_usec = earliest.tv_usec - now.tv_usec;
    	}

    	//printf ("wait %ld-%ld\n", (long) tv.tv_sec, (long) tv.tv_usec);
    	rc = select(numfds, &readfds, NULL, NULL, &tv);
    	if(rc < 0)
		{             
			// Error
      		if (EINTR == errno) continue; // don't break 
      		stp_syslog_error("RSTP>>## FATAL_MODE:select failed: %s\n", strerror(errno));
      		return -2;
    	}

    	if(!rc) 
		{   
			// Timeout expired
			stp_in_one_second ();
    		
      		gettimeofday (&earliest, NULL);
      		//printf ("tick %ld-%ld\n", (long) earliest.tv_sec - 1005042800L, (long) earliest.tv_usec);

      		earliest.tv_sec++;
			//ct.tv_usec = ((earliest.tv_usec - now.tv_usec-tv.tv_usec)>0)? (earliest.tv_usec - now.tv_usec-tv.tv_usec):(now.tv_usec +tv.tv_usec- earliest.tv_usec);
      		continue;
    	}

    	if(FD_ISSET(0, &readfds))
		{
			#ifdef STP_WITH_CLI
     		rl_callback_read_char();
			#endif
    	}

    	if(FD_ISSET(pktsck, &readfds))
		{
      		shutdown_flag |= stp_uid_read_socket();
    	}

		if(FD_ISSET(npdsck,&readfds))
		{
			shutdown_flag |= stp_npd_read_socket();
		}
#if 1		/* for stp bug:AXSSZFI-291 on distributed system */
		if(FD_ISSET(netlinkSck,&readfds))
		{
			shutdown_flag |= stp_netlink_read_socket();
		}

		if(FD_ISSET(lnkstsck,&readfds))
		{
			shutdown_flag |= stp_lnkst_read_socket();
		}
#endif		
	} while(!shutdown_flag);
	return 0;
}


int stp_bridge_start (void)
{
	
	register int  iii;
#ifdef STP_WITH_CLI
	stp_dcli_init (); 
#endif
	stp_in_init (NUMBER_OF_PORTS);

	stp_bitmap_clear(&enabled_ports);

	iii = stp_mgmt_stpm_create_cist();
	if (STP_OK != iii) {
	 stp_trace("FATAL: can't enable:%s\n",
	             stp_in_get_error_explanation (iii));
	  return (-1);
	}

	return STP_OK;
}


void stp_bridge_shutdown (void)
{
  
  /* send SHUTDOWN */
#if 0
  int       rc;
  rc = STP_IN_stpm_delete (0);
#endif
  stp_mgmt_stpm_delete_all();

 /* if (STP_OK != rc) {
    printf ("FATAL: can't delete:%s\n",
               stp_in_get_error_explanation (rc));
    exit (1);
  }*/
}

#if 0
char *get_prompt (void)
{
  static char prompt[MAX_CLI_PROMT];
  snprintf (prompt, MAX_CLI_PROMT - 1, "%s B%ld > ", UT_sprint_time_stamp(), my_pid);
  return prompt;
}
#endif

int main (int argc, char** argv)
{
	int				ret = 0;
	pthread_t 		*dbus_thread;
	pthread_attr_t 	dbus_thread_attr;


	stp_slot_id = get_num_from_file("/proc/product_info/board_slot_id");
	stp_is_distributed = get_num_from_file("/proc/product_info/distributed_product");

	//stp_set_log_level(STP_LOG_DEBUG);
	#ifdef STP_WITH_CLI
	stp_cli_rl_init ();
	#endif
  /*add for do br ioctl*/
	stp_npd_ioctl_init();
/********************************/
	my_pid = getpid();
	stp_syslog_dbg("STP>>PID[%ld]\n", my_pid);
	
	ret = stp_dbus_init();
	if(0 != ret)
	{
		stp_syslog_warning("STP>>stp dbus init error!\n");
	}
	dbus_thread = (pthread_t *)malloc(sizeof(pthread_t));
	pthread_attr_init(&dbus_thread_attr);
	pthread_create(dbus_thread,&dbus_thread_attr,stp_dbus_thread_main,NULL);

	
	if(STP_OK == stp_bridge_start()) {
		stp_bridge_main_loop();
	}
  	stp_bridge_shutdown ();
		
		stp_syslog_dbg("bridge shutdown end\n");
  	#ifdef STP_WITH_CLI
  	stp_cli_rl_shutdown ();
	#endif
    close (pktsck);
	close(npdsck);
  	return 0;
}

char* stp_bridge_sprint_timestamp (void)
{
  time_t      clock;
  struct      tm *local_tm;
  static char time_str[20];

  time(&clock);
  local_tm = localtime (&clock);
  strftime(time_str, 20 - 1, "%H:%M:%S", local_tm);
  return time_str;
}

#ifdef __cplusplus
}
#endif

