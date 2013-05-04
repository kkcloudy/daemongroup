/* rdc_main.c */

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <signal.h>
#include <arpa/inet.h>
#include "rdc_interface.h"
#include "eag_time.h"
#include "eag_log.h"
#include <errno.h>
#define _GNU_SOURCE 			/* See feature_test_macros(7) */
#include <sched.h>


#include "rdc_ins.h"
#include "eag_util.h"

#define SLOT_ID_FILE			"/dbm/local_board/slot_id"

static void
print_usage(const char *file)
{
	fprintf(stderr, "usage: %s [<type> <id>]\n", file);
	fprintf(stderr, "type=0, remote hansi, id 1-16\n");
	fprintf(stderr, "type=1, none or local hansi, id 0-16\n");
	exit(1);
}

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

static int keep_going = 1;
static void signal_handler(int sig) 
{
	switch (sig) {
		case SIGTERM:
		case SIGINT:
		case SIGUSR2:
			eag_log_info("Received terminate signal %d\n", sig);
			keep_going = 0;
			break;
		default:
			break;
	}

}

static int set_rdc_core(void)
{
	unsigned int sys_core_mask = 1;
	unsigned long core_mask = 0;
	int corenum = 0;
	unsigned int set_core_mask = 1;
	unsigned long mask_tmp = 0; /* let rdc proc active on every core at random.  previous:1 */
	int num = 0;
	FILE *fp = NULL;
	
	fp = fopen( "/proc/coremask", "r" );
	if ( NULL == fp )
	{
		eag_log_err("set_rdc_core open coremask file failed: %s", 
			safe_strerror(errno));
		return -1;
	}
	
	num = fscanf( fp, "%x", &sys_core_mask );
	if (num < 0) {
		eag_log_err("set_rdc_core fscanf  failed: %s", 
			safe_strerror(errno));
		fclose( fp );
		return -1;
	}
	if (num != 1) {
		eag_log_err("set_rdc_core fscanf  failed num = %d", num );
		fclose( fp );
		return -1;
	}
	fclose( fp );
	
	eag_log_info("set_rdc_core sys_core_mask=%x", sys_core_mask);
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
			eag_log_err("set_rdc_core set_core_mask=%x, corenum=%d, sys_core_mask=%x :%s ",
				set_core_mask, corenum, sys_core_mask, safe_strerror(errno));
			return -1;
		}
	}
	
	return 0;
}

int main(int argc, char *argv[])
{
	int slot_id = 0;
	char buf[64] = "";
	int hansitype = 0;
	int insid = 0;
	char pidfile[128] = "";
	char daemon_name[32] = "";
	struct sigaction act;
	rdc_ins_t *rdcins = NULL;

	if (1 == argc) {
		hansitype = HANSI_LOCAL;
		insid = 0;
	}
	else if (argc >= 3) {
		hansitype = atoi(argv[1]);
		insid = atoi(argv[2]);
		if (HANSI_REMOTE == hansitype) {
			if (insid < 1 || insid > MAX_HANSI_ID) {
				print_usage(argv[0]);
			}
		}
		else if (HANSI_LOCAL == hansitype) {
			if (insid < 0 || insid > MAX_HANSI_ID) {
				print_usage(argv[0]);
			}
		}
		else {
			print_usage(argv[0]);
		}
	}
	else {
		print_usage(argv[0]);
	}
	eag_time_init();
#if 1	
	if (daemon(0, 0)){
		eag_log_err("daemon() failed!");
		exit(1);
	}
#endif

	snprintf(pidfile, sizeof(pidfile)-1,
			"/var/run/rdc_%d_%d.pid", hansitype, insid);
	log_pid(pidfile);
	
	read_file(SLOT_ID_FILE, buf, sizeof(buf));
	slot_id = atoi(buf);

	snprintf(daemon_name, sizeof(daemon_name)-1,
			"rdc[%d_%d]", slot_id, insid);
	eag_log_init(daemon_name);
	set_rdc_core();
	eag_log_add_filter("rdcins:server:client:pktconn");
	
	act.sa_handler = signal_handler;
	sigemptyset(&act.sa_mask);
	act.sa_flags = 0;
	sigaction(SIGTERM, &act, NULL);
	sigaction(SIGINT, &act, NULL);
	sigaction(SIGUSR2, &act, NULL);
	
	rdcins = rdc_ins_new(hansitype, insid);
	if (NULL == rdcins) {
		eag_log_err("rdc_ins_new failed");
		exit(1);
	}
	eag_log_del_filter("rdcins:server:client:pktconn");
	while (keep_going) {
		rdc_ins_dispatch(rdcins);
	}

	if (rdc_ins_is_running(rdcins)) {
		rdc_ins_stop(rdcins);
	}
	rdc_ins_free(rdcins);
	eag_log_uninit();
	
	return 0;
}

