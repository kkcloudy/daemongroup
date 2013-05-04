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

#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/select.h>
#include <arpa/inet.h>
#include <errno.h>
#include <stdio.h>
#include <syslog.h>
#include <signal.h>
#include <dbus/dbus.h>
#include <unistd.h>
#include <fcntl.h>
#include <signal.h>
#include <sys/types.h>
#include <sys/stat.h>

#include "nm_list.h"
#include "drp_def.h"
#include "drp_log.h"
#include "drp_mem.h"
#include "drp_dbus.h"
#include "drp_opxml.h"
#include "drp_handle.h"


static int keep_going = 1;
static void termination_handler(int signum) 
{
	drp_log_info("SIGTERM received!\n");
	keep_going = 0;
}

/*log pid and protect one process each OS fileclock*/
static void drp_log_pid(char *pidfile) 
{
	int fd,ret;
	char pid_buf[10]={0};
	struct flock file_lock;
	memset (&file_lock,0,sizeof(file_lock));
	
	if ((fd = open(pidfile, O_RDWR |O_CREAT, S_IRUSR |S_IWUSR)) < 0){
		drp_log_err("open file %s error!program exit\n",pidfile);
		exit(-1);
	}

	file_lock.l_type = F_WRLCK;
	if((ret=fcntl(fd, F_GETLK, &file_lock))<0){
		drp_log_err("fcntl error:can't get file lock state!program exit ret=%d error:%s\n",ret, safe_strerror(errno));
		exit(-1);
	}
	if (F_UNLCK!=file_lock.l_type)
	{
		drp_log_info("program already exist!\n");
		exit(0);
	}	
	file_lock.l_type=F_WRLCK;
	if((ret=fcntl(fd, F_SETLK, &file_lock))<0){
		drp_log_info("set lock error! program exit! ret=%d error:%s\n",ret,safe_strerror(errno));
		exit(-2);
	}

	snprintf(pid_buf,sizeof(pid_buf),"%u",getpid());
	drp_log_debug("sizeof(pid_buf)=%d,pid=%u,pid_buf=%s\n",sizeof(pid_buf),getpid(),pid_buf);
	write(fd, pid_buf, strlen(pid_buf));
	//close(fp); should not closed because of the file lock !
	
	return;
}

static void
drp_register_all_dbus_method( drp_dbus_t *drp_dbus)
{
	/*base conf*/
	drp_dbus_register_method(drp_dbus,
				DRP_DBUS_INTERFACE,drp_dbus_method_add_domain,NULL);
	drp_dbus_register_method(drp_dbus,
				DRP_DBUS_INTERFACE,drp_dbus_method_del_domain,NULL);
	drp_dbus_register_method(drp_dbus,
				DRP_DBUS_INTERFACE,drp_dbus_method_get_domain_ip,NULL);
	drp_dbus_register_method(drp_dbus,
				DRP_DBUS_INTERFACE,drp_dbus_method_add_domain_ip,NULL);
	drp_dbus_register_method(drp_dbus,
				DRP_DBUS_INTERFACE,drp_dbus_method_del_domain_ip,NULL);
	drp_dbus_register_method(drp_dbus,
				DRP_DBUS_INTERFACE,drp_dbus_method_show_domain_ip,NULL);

	drp_dbus_register_method(drp_dbus,
				DRP_DBUS_INTERFACE,drp_dbus_method_log_debug,NULL);

	return ;
}


int 
main( int argc, char *argv[] )
{
	struct sigaction act;
	char daemon_name[32];
	char pidfile[128];

	drp_dbus_t *drp_dbus = NULL;

	act.sa_handler = termination_handler;
	sigaction(SIGTERM, &act, NULL);
	sigaction(SIGINT, &act, NULL);
	sigaction(SIGUSR2, &act, NULL);

#if 1	
	if (daemon(1, 1)){
		drp_log_err("daemon() failed!");
		exit(1);
	}
#endif

	snprintf(pidfile,sizeof(pidfile)-1,"/var/run/drp.pid");
	drp_log_pid(pidfile);

	snprintf(daemon_name,sizeof(daemon_name)-1,"drp");
	drp_log_init(daemon_name);
	
	drp_dbus = drp_dbus_new(DRP_DBUS_BUSNAME, DRP_DBUS_OBJPATH);
	if ( NULL == drp_dbus ){
		drp_log_err("drp_dbus_new error!");
		exit(2);
	}
	drp_register_all_dbus_method(drp_dbus);
	
	while (keep_going) {	
		drp_dbus_dispach( drp_dbus, 500 );
	}

	drp_dbus_free(drp_dbus);

	drp_log_err("this daemon should not exit!!!");
	drp_log_uninit();
	return 0;
}


