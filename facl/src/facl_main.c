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
* facl_main.c
*
*
* CREATOR:
* autelan.software.Network Dep. team
*
* DESCRIPTION:
* facl main
*
*
*******************************************************************************/

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <signal.h>
#include <arpa/inet.h>
#include <errno.h>
#define _GNU_SOURCE 			/* See feature_test_macros(7) */
#include <sched.h>

#include "nm_time.h"
#include "nm_log.h"
#include "nm_process.h"
#include "facl_ins.h"

#define IPTABLES_LOCK_FILE				"/var/run/eag_iptables_lock"

nmp_mutex_t iptables_lock = {-1, ""};

static int keep_going = 1;
facl_ins_t *faclins = NULL;

static void
log_pid(const char *pidfile) 
{
	FILE *file = NULL;
	mode_t oldmask;

	oldmask = umask(022);
	file = fopen(pidfile, "w");
	umask(oldmask);
	if(NULL == file) return;
	fprintf(file, "%d\n", getpid());
	fclose(file);
}

static void signal_handler(int sig) 
{
	switch (sig) {
		case SIGTERM:
		case SIGINT:
		case SIGUSR2:
			nm_log_info("Received terminate signal %d\n", sig);
			keep_going = 0;
			break;
		default:
			break;
	}
}

static int set_facl_core(void)
{
	unsigned int sys_core_mask = 1;
	unsigned long core_mask = 0;
	int corenum = 0;
	unsigned int set_core_mask = 1;
	unsigned long mask_tmp = 0; /* let eag proc active on every core at random.  previous:1 */
	int num = 0;
	FILE *fp = NULL;
	
	fp = fopen( "/proc/coremask", "r" );
	if ( NULL == fp )
	{
		nm_log_err("set_facl_core open coremask file failed: %s", 
			safe_strerror(errno));
		return -1;
	}
	
	num = fscanf( fp, "%x", &sys_core_mask );
	if (num < 0) {
		nm_log_err("set_facl_core fscanf  failed: %s", 
			safe_strerror(errno));
		fclose( fp );
		return -1;
	}
	if (num != 1) {
		nm_log_err("set_facl_core fscanf  failed num = %d", num );
		fclose( fp );
		return -1;
	}
	fclose( fp );
	
	nm_log_info("set_facl_core sys_core_mask=%x", sys_core_mask);
	core_mask = sys_core_mask;
	
	while( core_mask>0 )
	{
		corenum++;
		core_mask>>=1;
	}
	
	if (corenum > 1)
	{
		set_core_mask = sys_core_mask & (~mask_tmp);
		if (sched_setaffinity(getpid(), sizeof(set_core_mask),  &set_core_mask) < 0)
		{	
			nm_log_err("set_facl_core set_core_mask=%x, corenum=%d, sys_core_mask=%x :%s ",
				set_core_mask, corenum, sys_core_mask, safe_strerror(errno));
			return -1;
		}
	}
	
	return 0;
}

int facl_main(int argc, char *argv[])
{
	char pidfile[128] = "";
	char daemon_name[32] = "";
	struct sigaction act;

	if (daemon(0, 0)){
		nm_log_err("daemon() failed!");
		return 1;
	}
	nm_time_init();
	snprintf(pidfile, sizeof(pidfile)-1, "/var/run/facl.pid");
	log_pid(pidfile);
	
	snprintf(daemon_name, sizeof(daemon_name)-1, "facl");
	nm_log_init(daemon_name);
	set_facl_core();
	
	act.sa_handler = signal_handler;
	sigemptyset(&act.sa_mask);
	act.sa_flags = 0;
	sigaction(SIGTERM, &act, NULL);
	sigaction(SIGINT, &act, NULL);
	sigaction(SIGUSR2, &act, NULL);
	
	faclins = facl_ins_new();
	if (NULL == faclins) {
		nm_log_err("facl_ins_new failed");
		return 1;
	}
	nmp_mutex_init(&iptables_lock, IPTABLES_LOCK_FILE);

	nm_log_add_filter("acl_shell");
	nm_log_add_filter("acl_db");

	while (keep_going) {
		facl_ins_dispatch(faclins);
	}	
	facl_ins_free(faclins);
	
	nm_log_uninit();
	nmp_mutex_destroy(&iptables_lock);
	
	return 0;
}

int main(int argc, char *argv[])
{
	int ret = 0;
	
	#if 1
	#include <mcheck.h>
	mtrace();
	#endif

	ret = facl_main(argc, argv);
	
	return ret; 
}

