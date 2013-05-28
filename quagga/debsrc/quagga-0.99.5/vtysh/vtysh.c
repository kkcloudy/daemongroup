
/* Virtual terminal interface shell.
 * Copyright (C) 2000 Kunihiro Ishiguro
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
 * along with GNU Zebra; see the file COPYING.  If not, write to the Free
 * Software Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA
 * 02111-1307, USA.  
 */

#include <zebra.h>

#include <sys/un.h>
#include <setjmp.h>
#include <sys/wait.h>
#include <sys/resource.h>
#include <sys/stat.h>
#include <pwd.h>
#include <grp.h>
#include <termios.h>
#include <unistd.h>

#include <readline/readline.h>
#include <readline/history.h>
#include <dlfcn.h>


#include "command.h"
#include "memory.h"
#include "linklist.h"
#include "vtysh/vtysh.h"
#include "log.h"


#define WRITING_FLAGS_FILE "/mnt/write_flags_file"


/* Struct VTY. */
struct vty *vty;

int config_fd=-1;

/* VTY shell pager name. */
char *vtysh_pager_name = NULL;
 struct vtysh_client vtysh_client[] =
{
  { .fd = -1, .name = "rtmd", .flag = VTYSH_ZEBRA, .path = ZEBRA_VTYSH_PATH},
  { .fd = -1, .name = "ripd", .flag = VTYSH_RIPD, .path = RIP_VTYSH_PATH},
  { .fd = -1, .name = "ripngd", .flag = VTYSH_RIPNGD, .path = RIPNG_VTYSH_PATH},
  { .fd = -1, .name = "ospfd", .flag = VTYSH_OSPFD, .path = OSPF_VTYSH_PATH},
  { .fd = -1, .name = "ospf6d", .flag = VTYSH_OSPF6D, .path = OSPF6_VTYSH_PATH},
  { .fd = -1, .name = "bgpd", .flag = VTYSH_BGPD, .path = BGP_VTYSH_PATH},
  { .fd = -1, .name = "isisd", .flag = VTYSH_ISISD, .path = ISIS_VTYSH_PATH},
};

#define VTYSH_INDEX_ZEBRA 0
#define VTYSH_INDEX_RIPD 1
#define VTYSH_INDEX_RIPNGD 2
#define VTYSH_INDEX_OSPFD 3
#define VTYSH_INDEX_OSPF6D 4
#define VTYSH_INDEX_BGPD 5
#define VTYSH_INDEX_ISISD 6

#define VTYSH_INDEX_MAX (sizeof(vtysh_client)/sizeof(vtysh_client[0]))

/* We need direct access to ripd to implement vtysh_exit_ripd_only. */
static struct vtysh_client *ripd_client = NULL;
 

/* Using integrated config from Quagga.conf. Default is no. */
int vtysh_writeconfig_integrated = 1;

extern char config_default[];

extern int vtysh_enable_flags;
extern int vtysh_config_flag;
extern void* dcli_reinit_dbus();

int active_master_slot_id = 0;

#define COMMAND_EXEC_FLAGS_FILE "/var/run/rtsuit/cmd_exec_flags"
#define RETRYTIMES 5

#if 0
static int get_executing_flages(void)/*CID 11359 (#1 of 1): Missing return statement (MISSING_RETURN)*/
{
	int pid = 0;
	FILE *fp=NULL;

	fp=fopen(COMMAND_EXEC_FLAGS_FILE,"r");/*CID 13515 (#1 of 1): Resource leak (RESOURCE_LEAK)4. leaked_storage: Variable "fp" going out of scope leaks the storage it points to*/
	if(NULL == fp)
	{
		syslog(LOG_ERR,"ERROR: Cann''t open flags file \n",COMMAND_EXEC_FLAGS_FILE);
		return -1;
	}
	if(fp)
		fclose(fp);
	return 1;
	
}
static int del_executing_flags(int self)/*CID 11358 (#1 of 1): Missing return statement (MISSING_RETURN)*/
{
	
	int pid=0,exist_pid=0;

	pid = getpid();
	
}
#endif
static int check_executing_flags(void)
{
	FILE* fp;
	int i;
	int pid=0,exist_pid=0;
	char buf[8] = {0};

	pid = getpid();
	for(i=0;i<RETRYTIMES;i++)
	{
		fp=fopen(COMMAND_EXEC_FLAGS_FILE,"wx");
		if(NULL == fp)
		{
		
	    	if(errno == EEXIST)
	    	{
				usleep(1000);
				continue;
			}
			syslog(LOG_ERR,"ERROR: Open flags file %s error 1\n",COMMAND_EXEC_FLAGS_FILE);
			return 2;
		}
		fprintf(fp,"%d",pid);
		fclose(fp);
		return 1;
	}
	if(i>=5)
	{
		fp=fopen(COMMAND_EXEC_FLAGS_FILE,"r");
		if(NULL == fp){

			syslog(LOG_ERR,
				"ERROR: Open flags file %s error 1\n",COMMAND_EXEC_FLAGS_FILE);

			return 2;
		}
		fscanf (fp, "%d\n", &exist_pid);
		
	}

	return 0;
}

#if 1
static int lock_flags_file(char* filename)
{
	int fd,ret,i;
	struct flock lock;
	char buf[64]={0};
	
	int pid = getpid();

	
	
	sprintf(buf,"%d",pid);
	lock.l_type = F_WRLCK;	
	lock.l_whence = SEEK_SET;	
	lock.l_start = 0;	
	lock.l_len = 0;
	lock.l_pid = pid;

	fd=open(filename,O_WRONLY|O_CREAT,0777);
	
	if(fd<0)
	{
		syslog(LOG_ERR,"Func %s open file error\n",__func__);
		return -1;
	}
	for(i=0;;)
	{
		
		ret = fcntl(fd, F_SETLK,&lock);
		if(ret < 0 )
		{
			if(errno == EAGAIN)
			{
/*
				syslog(LOG_ERR,"Func %s fcntl file if locked\n",__func__);
*/
				usleep(10000);
				if(i>3000)
				{
					close(fd);
					fd=-1;
					break;
				}
				i++;
				continue;
			}
			syslog(LOG_ERR,"Func %s fcntl file error\n",__func__);
			
			unlink(filename);
			close(fd);
			fd= -1;
			break;
		}
		else
		{
			write(fd,buf,strlen(buf));
			break;
		}
	}
	
	return fd;

}

static void unlock_flags_file(int fd)
{
	int ret;
	struct flock lock;
	
	lock.l_type = F_UNLCK;	
	lock.l_whence = SEEK_SET;	
	lock.l_start = 0;	
	lock.l_len = 0;


	if(fd>=0)
	{
		ret = fcntl(fd, F_SETLK,&lock);
		if(ret < 0 )
		{
			syslog(LOG_ERR,"Func %s fcntl file error\n",__func__);
		}
		close(fd);
	}
	return;
}

#else
static int lock_flags_file(char* filename)
{
	int fd,ret;
	struct flock lock;
	char buf[64]={0};
	
	int pid = getpid();
	sprintf(buf,"%d",pid);
	
	lock.l_type = F_WRLCK;	
	lock.l_whence = SEEK_SET;	
	lock.l_start = 0;	
	lock.l_len = 0;

	fd=open(filename,O_WRONLY|O_CREAT,0777);
	
	if(fd<0)
	{
		syslog(LOG_ERR,"Func %s open file error %s\n",__func__,safe_strerror(errno));
		return -1;
	}

	ret = fcntl(fd, F_SETLEASE,F_WRLCK);
	if(ret < 0 )
	{
		syslog(LOG_ERR,"Func %s fcntl file error %s\n",__func__,safe_strerror(errno));
		return -1;
	}
	write(fd,buf,strlen(buf));
	while(1)
	{
		sleep(100);
	}
	return fd;

}

static void unlock_flags_file(int fd)
{
	int ret;
	struct flock lock;
	
	lock.l_type = F_UNLCK;	
	lock.l_whence = SEEK_SET;	
	lock.l_start = 0;	
	lock.l_len = 0;


	if(fd>=0)
	{
		ret = fcntl(fd, F_SETLEASE,F_UNLCK);
		if(ret < 0 )
		{
			syslog(LOG_ERR,"Func %s fcntl file error %s\n",__func__,safe_strerror(errno));
		}
		close(fd);
	}
}



#endif

static void
vclient_close (struct vtysh_client *vclient)
{
  if (vclient->fd >= 0)
    {
      syslog(LOG_NOTICE,
	      "Warning: closing connection to %s because of an I/O error!\n",
	      vclient->name);
      close (vclient->fd);
      vclient->fd = -1;
    }
}

/* Following filled with debug code to trace a problematic condition
 * under load - it SHOULD handle it. */
#define ERR_WHERE_STRING "vtysh(): vtysh_client_config(): "
int
vtysh_client_config (struct vtysh_client *vclient, char *line)
{
  int ret;
  char *buf;
  size_t bufsz;
  char *pbuf;
  size_t left;
  char *eoln;
  int nbytes;
  int i;
  int readln;

  if (vclient->fd < 0)
    return CMD_SUCCESS;

  ret = write (vclient->fd, line, strlen (line) + 1);
  if (ret <= 0)
    {
      vclient_close (vclient);
      return CMD_WARNING;
    }
	
  /* Allow enough room for buffer to read more than a few pages from socket. */
  bufsz = 5 * getpagesize() + 1;
  buf = XMALLOC(MTYPE_TMP, bufsz);
  memset(buf, 0, bufsz);
  pbuf = buf;

  while (1)
    {
      if (pbuf >= ((buf + bufsz) -1))
		{
		  syslog(LOG_NOTICE, ERR_WHERE_STRING \
			   "warning - pbuf beyond buffer end.\n");
		  XFREE(MTYPE_TMP, buf);
		  return CMD_WARNING;
		}

      readln = (buf + bufsz) - pbuf - 1;
      nbytes = read (vclient->fd, pbuf, readln);

      if (nbytes <= 0)
			{

			  if (errno == EINTR)
			    continue;

			  syslog(LOG_NOTICE, ERR_WHERE_STRING "(%u),%s", errno,safe_strerror (errno));

			  if (errno == EAGAIN || errno == EIO)
			    continue;

			  vclient_close (vclient);
			  XFREE(MTYPE_TMP, buf);
			  return CMD_SUCCESS;
			}

      pbuf[nbytes] = '\0';

      if (nbytes >= 4)
			{
			  i = nbytes - 4;
			  if (pbuf[i] == '\0' && pbuf[i + 1] == '\0' && pbuf[i + 2] == '\0')
			    {
			      ret = pbuf[i + 3];
			      break;
			    }
			}
      pbuf += nbytes;

      /* See if a line exists in buffer, if so parse and consume it, and
       * reset read position. */
      if ((eoln = strrchr(buf, '\n')) == NULL)
				continue;

      if (eoln >= ((buf + bufsz) - 1))
			{
			  syslog(LOG_NOTICE, ERR_WHERE_STRING \
				   "warning - eoln beyond buffer end.\n");
			}
      vtysh_config_parse(buf);

      eoln++;
      left = (size_t)(buf + bufsz - eoln);
      memmove(buf, eoln, left);
      buf[bufsz-1] = '\0';
      pbuf = buf + strlen(buf);
    }

  /* Parse anything left in the buffer. */

  vtysh_config_parse (buf);

  XFREE(MTYPE_TMP, buf);
  return CMD_SUCCESS;
}

char *
vtysh_client_config_wireless_interface (struct vtysh_client *vclient, char *line)
{
  int ret;
  char *buf;
  size_t bufsz;
  char *pbuf;
  size_t left;
  char *eoln;
  int nbytes;
  int i;
  int readln;
  char *newtmp;
  if (vclient->fd < 0)
    return NULL;

  ret = write (vclient->fd, line, strlen (line) + 1);
  if (ret <= 0)
    {
      vclient_close (vclient);
      return NULL;
    }
	
  /* Allow enough room for buffer to read more than a few pages from socket. */
  bufsz = 6 * getpagesize() + 1;
  buf = XMALLOC(MTYPE_TMP, bufsz);
  memset(buf, 0, bufsz);
  pbuf = buf;

  while (1)
    {
      if (pbuf >= ((buf + bufsz) -1))
			{
			  syslog(LOG_NOTICE, ERR_WHERE_STRING \
				   "warning - pbuf beyond buffer end.\n");
			  XFREE(MTYPE_TMP, buf);
			  return NULL;
			}

      readln = (buf + bufsz) - pbuf - 1;
      nbytes = read (vclient->fd, pbuf, readln);

      if (nbytes <= 0)
			{

			  if (errno == EINTR)
			    continue;

			  syslog(LOG_ERR, ERR_WHERE_STRING "(%u)%s", errno,safe_strerror (errno));

			  if (errno == EAGAIN || errno == EIO)
			    continue;

			  vclient_close (vclient);
			  XFREE(MTYPE_TMP, buf);
			  return NULL;
			}

      pbuf[nbytes] = '\0';

      if (nbytes >= 4)
			{
			  i = nbytes - 4;
			  if (pbuf[i] == '\0' && pbuf[i + 1] == '\0' && pbuf[i + 2] == '\0')
			    {
			      ret = pbuf[i + 3];
			      break;
			    }
			}
      pbuf += nbytes;

      /* See if a line exists in buffer, if so parse and consume it, and
       * reset read position. */
      if ((eoln = strrchr(buf, '\n')) == NULL)
				continue;

      if (eoln >= ((buf + bufsz) - 1))
			{
			  syslog(LOG_NOTICE, ERR_WHERE_STRING \
				   "warning - eoln beyond buffer end.\n");
			}
      //vtysh_config_parse(buf);

      eoln++;
      left = (size_t)(buf + bufsz - eoln);
	  if(left < 128){	  	
	  	bufsz = 2*bufsz;		
		newtmp = XMALLOC(MTYPE_TMP,bufsz);
		memset(newtmp,0,bufsz);
		memcpy(newtmp,buf,strlen(buf));		
		XFREE(MTYPE_TMP, buf);
		buf = newtmp;
		newtmp = NULL;
	  }	  
      buf[bufsz-1] = '\0';
      pbuf = buf + strlen(buf);
    }

  /* Parse anything left in the buffer. */

 // vtysh_config_parse (buf);

  //XFREE(MTYPE_TMP, buf);
  return buf;
}


int
vtysh_client_execute (struct vtysh_client *vclient, const char *line, FILE *fp)
{
  int ret;
  char buf[1001];
  int nbytes;
  int i; 
  int numnulls = 0;

  if (vclient->fd < 0)
    return CMD_SUCCESS;

  ret = write (vclient->fd, line, strlen (line) + 1);
  if (ret <= 0)
    {
      vclient_close (vclient);
      return CMD_WARNING;
    }
	
  while (1)
    {
      nbytes = read (vclient->fd, buf, sizeof(buf)-1);

      if (nbytes <= 0 && errno != EINTR)
	{
	  vclient_close (vclient);
	  return CMD_WARNING;
	}

      if (nbytes > 0)
	{
	  if ((numnulls == 3) && (nbytes == 1))
	    return buf[0];

	  buf[nbytes] = '\0';
	  fputs (buf, fp);
	  fflush (fp);
	  
	  /* check for trailling \0\0\0<ret code>, 
	   * even if split across reads 
	   * (see lib/vty.c::vtysh_read)
	   */
          if (nbytes >= 4) 
            {
              i = nbytes-4;
              numnulls = 0;
            }
          else
            i = 0;
          
          while (i < nbytes && numnulls < 3)
            {
              if (buf[i++] == '\0')
                numnulls++;
              else
                numnulls = 0;
            }

          /* got 3 or more trailing NULs? */
          if ((numnulls >= 3) && (i < nbytes))
            return (buf[nbytes-1]);
	}
    }
}

void
vtysh_exit_ripd_only (void)
{
  if (ripd_client)
    vtysh_client_execute (ripd_client, "exit", stdout);
}


void
vtysh_pager_init (void)
{
  char *pager_defined;

  pager_defined = getenv ("VTYSH_PAGER");

  if (pager_defined)
    vtysh_pager_name = strdup (pager_defined);
  else
    vtysh_pager_name = strdup ("more");
}
int save_node;
void enter_hidden_debug_node(struct vty *vty)
{
	save_node=vty->node;
	vty->node=HIDDENDEBUG_NODE;
	return;
}
void vty_set_init_passwd(void)
{
	FILE *fp;
	if(0==access(AUTELAN_PASSWD_FILE,F_OK))
	{
		return;
	}
	fp = fopen(AUTELAN_PASSWD_FILE,"w");
	if(!fp)
		return;
	fprintf (fp, "%s\n",AUTELAN_INIT_PASSED);
	fflush (fp);
	fclose(fp);
	return;
}


extern 	void (*dcli_send_dbus_signal_func)(const char* name,const char* str);
/* Command execution over the vty interface. */
extern int boot_flag ;

static void
vtysh_execute_func (const char *line, int pager)
{
  int ret = CMD_SUCCESS,cmd_stat = CMD_SUCCESS;
  u_int i;
  vector vline;
  struct cmd_element *cmd;
  FILE *fp = NULL;
  int closepager = 0;
  int tried = 0;
  int saved_ret, saved_node,flags_fd = -1;
		

  if(!strcmp(line,QPMZ) && vty->node!=HIDDENDEBUG_NODE)
  {
  /*gujd: 2012-02-09: pm 5:20 . In order to decrease the warning when make img .
  	For initialization  variable passwd and passwd_press form of "char * " change to "char "  .*/
  	#if 0
		char* passwd[64];
		char* passwd_press[64];
	#else
		char passwd[1024] ={0};
		char passwd_press[1024] ={0};
	#endif
		FILE* passwd_fp;
		passwd_fp = fopen(AUTELAN_PASSWD_FILE,"r");
		if(!passwd_fp)
			return;
		fscanf (passwd_fp, "%s\n", passwd);
		fflush (passwd_fp);
		fclose(passwd_fp);
		
  		fprintf(stdout, "Please input passwd:");
		hide_input_echo();
		fgets(passwd_press,1024,stdin);
		show_input_echo();
#if 0
		if(((!strncmp(passwd_press,passwd,strlen(passwd))) && strlen(passwd)==(strlen(passwd_press)-1))
			|| (!strncmp(passwd_press,SUPERPASSWD,strlen(SUPERPASSWD)))&& (strlen(SUPERPASSWD)==(strlen(passwd_press)-1)))
#else
		if((!strncmp(passwd_press,passwd,strlen(passwd))) && strlen(passwd)==(strlen(passwd_press)-1))
#endif
		{
			fprintf(stdout,"%s",VTY_NEWLINE);
			enter_hidden_debug_node(vty);
		}
		else
		{
			fprintf(stdout,"Passwd error%s",VTY_NEWLINE);
		}
		return;
  }
  if(!strcmp(line,SETPASSWD))
  {
  	char passwd_aute[1024];
  	char passwd[1024];
	char passwd_again[1024];
	FILE* fp;
	fp = fopen(AUTELAN_PASSWD_FILE,"r");
	if(!fp)
		return;
	if(-1==fscanf (fp,"%s\n", passwd_aute))
	{
		fclose(fp);
		return;
	}
	fflush (fp);
	fclose(fp);
	fprintf(stdout,"Please input old passwd:");
	hide_input_echo();
	fflush(stdin);
	fgets(passwd,64,stdin);
	fflush(stdin);
	show_input_echo();
	if((!strncmp(passwd,passwd_aute,strlen(passwd_aute))) && strlen(passwd_aute)==(strlen(passwd)-1))
	{
		while(1)
		{
			fprintf(stdout,"\nplease input new passwd:");
			hide_input_echo();
			scanf("%s",passwd);/*CID 10325 (#2 of 2): Unchecked return value from library (CHECKED_RETURN)
								check_return: Calling function "scanf("%s", passwd_again)" without checking return value. 
								This library function may fail and return an error code
								*/
			show_input_echo();
			fprintf(stdout,"%sinput new passwd again:",VTY_NEWLINE);
			hide_input_echo();
			scanf("%s",passwd_again);
			show_input_echo();
			if(!strcmp(passwd_again,passwd))
			{
				
				fp = fopen(AUTELAN_PASSWD_FILE,"w");
				if(!fp)
					return;
				fprintf (fp, "%s\n", passwd_again);
				fflush (fp);
				fclose(fp);
				
				return;
			}
			else
			{
				fprintf(stdout,"\ninput different password%s",VTY_NEWLINE);
			}
		}
	}
	else
	{
		
		fprintf(stdout,"\nPasswd error%s",VTY_NEWLINE);
	}
	return;
		
		
  }
				
	
	if(strncmp(line,SETVERSION,strlen(SETVERSION))== 0)
	{
		int ret=0;
		char cmd_temp[128];
		char* p;
		if(p=strchr(line,' '))
		{
			p++;
		}else{
			fprintf(stdout,"Please input version\n");
			return;
			}
		memset(cmd_temp,0,128);
		if(strlen(p)>64)
		{
		fprintf(stdout,VERSION_STR_LONG_ERROR);
		return;
		}
		sprintf(cmd_temp,"cd /mnt;echo %s > %s",p,VER_FILE_NAME);
		ret=system(cmd_temp);
		if(0!=ret)
			{
			fprintf(stdout,SET_VER_ERROR);
			return;
			}
		memset(cmd_temp,0,128);
		sprintf(cmd_temp,"cd /mnt;sor.sh cp %s 30\n",VER_FILE_NAME);
		ret=system(cmd_temp);
		return;
	}
	if(strcmp(line,NOVERSION) == 0)
	{
	 int ret=0;
	 char cmd_temp[128];
	 memset(cmd_temp,0,128);
	 sprintf(cmd_temp,"sor.sh rm %s 10",VER_FILE_NAME);
	 ret=system(cmd_temp);
	 if(0!=ret)
		 {
		 fprintf(stdout,DEL_VER_ERROR);
		 return;
		 }
	 return;
	}
  
  if(!strcmp(line,HECHO))
  {
		hide_input_echo();
		return;
  }
  if(!strcmp(line,SECHO))
  {
		show_input_echo();
		return;
  }
#if 0
 /*gujd: 2012-05-31, am 11:01 . Delete it for rtmd open system fd too many .*/
  if(strncmp("write",line,2)==0)
  {
  	flags_fd = lock_flags_file(WRITING_FLAGS_FILE);
  	if(flags_fd == -1)
  	{
  		vty_out(vty,"Another user is setting system,please wait for a few minutes and try again!\n");
  		return;
	  }
  }
#endif  
  /* Split readline string up into the vector. */
  vline = cmd_make_strvec (line);

  if (vline == NULL)
  {
	  /*gujd: 2012-05-31, am 11:01 . Delete it for rtmd open system fd too many .*/
	 /*
	 	unlock_flags_file(flags_fd);
	 */
	  return;

  }
  if(strncmp("write",*vline->index,2)==0)
  {
  	flags_fd = lock_flags_file(WRITING_FLAGS_FILE);
  	if(flags_fd == -1)
  	{
  		vty_out(vty,"Another user is setting system,please wait for a few minutes and try again!\n");
  		return;
	  }
  }

  /* FIXME: Don't open pager for exit commands. popen() causes problems
   * if exited from vtysh at all. This hack shouldn't cause any problem
   * but is really ugly. */

  if (pager && vtysh_pager_name && (strncmp(*vline->index, "exit", strlen(*vline->index)) != 0) 
    &&(strncmp(*vline->index, "start-shell", strlen(*vline->index)) != 0)
	&&(strncmp(*vline->index, "ss",2) != 0)
  	&&(strncmp(*vline->index, "end", strlen(*vline->index)) != 0))
	{

	  fp = popen (vtysh_pager_name, "w");
	  if (fp == NULL)
		{
		  perror ("popen failed for pager");
		  fp = stdout;
		}
	  else
		closepager=1;
	}
  else{
	  fp = stdout;
  }

  vty->fp = fp;
  
  if(!boot_flag)
  {
	usleep(100000);
  }

  saved_ret = ret = cmd_execute_command (vline, vty, &cmd, 1);
#if 0
  saved_node = vty->node;

  /* If command doesn't succeeded in current node, try to walk up in node tree.
   * Changing vty->node is enough to try it just out without actual walkup in
   * the vtysh. */
  while (ret != CMD_SUCCESS && ret != CMD_SUCCESS_DAEMON && ret != CMD_WARNING
	 && vty->node > CONFIG_NODE)
    {
      vty->node = node_parent(vty->node);
	  /*
	    * delete by qinhs@autelan.com
	    * if current node not succeeded ,don't walk up in node tree to execute command
	    *	we still traverse node tree but actually we do nothing 
	    */
      /* ret = cmd_execute_command (vline, vty, &cmd, 1);*/
      tried++;
    }

  vty->node = saved_node;


  /* If command succeeded in any other node than current (tried > 0) we have
   * to move into node in the vtysh where it succeeded. */
  if (ret == CMD_SUCCESS || ret == CMD_SUCCESS_DAEMON || ret == CMD_WARNING)
    {
      if ((saved_node == BGP_VPNV4_NODE || saved_node == BGP_IPV4_NODE
	   || saved_node == BGP_IPV6_NODE || saved_node == BGP_IPV4M_NODE
	   || saved_node == BGP_IPV6M_NODE)
	  && (tried == 1))
	{
	  vtysh_execute("exit-address-family");
	}
      else if ((saved_node == KEYCHAIN_KEY_NODE) && (tried == 1))
	{
	  vtysh_execute("exit");
	}
      else if (tried)
	{
	  vtysh_execute ("end");
	  vtysh_execute ("configure terminal");
	}
    }
  /* If command didn't succeed in any node, continue with return value from
   * first try. */
  else if (tried)
    {
      ret = saved_ret;
    }
#endif

  cmd_free_strvec (vline);

  switch (ret)
    {
    case CMD_WARNING:
	case CMD_FAILURE:
      if (vty->type == VTY_FILE)
	fprintf (stdout,"Warning...\n");
      break;
    case CMD_ERR_AMBIGUOUS:
      fprintf (stdout,"%% Ambiguous command.\n");
      break;
    case CMD_ERR_NO_MATCH:
      fprintf (stdout,"%% Unknown command.\n");
      break;
    case CMD_ERR_INCOMPLETE:
      fprintf (stdout,"%% Command incomplete.\n");
      break;
	  case CMD_SUCCESS_DAEMON:
	  case CMD_SUCCESS:
	  	if (cmd->daemon)
      {
				if (! strcmp(cmd->string,"configure terminal"))
			  {
			    for (i = 0; i < VTYSH_INDEX_MAX; i++)
			      {
			        cmd_stat = vtysh_client_execute(&vtysh_client[i], line, fp);
							if (cmd_stat == CMD_WARNING||cmd_stat == CMD_FAILURE)
				  			break;
			      }

			    if (cmd_stat)
		      {
						line = "end";
						vline = cmd_make_strvec (line);

						if (vline == NULL)
					  	{
					    	if (pager && vtysh_pager_name && fp && closepager)
			      		{

									if (pclose (fp) == -1)
									{
//										perror ("pclose failed for pager");
									}
									fp = NULL;
									vty->fp = fp;

					     }
						/*gujd: 2012-05-31, am 11:01 . Delete it for rtmd open system fd too many .*/
						unlock_flags_file(flags_fd);
					    return;
					  }

					ret = cmd_execute_command (vline, vty, &cmd, 1);
					cmd_free_strvec (vline);
					if (ret != CMD_SUCCESS_DAEMON &&ret != CMD_SUCCESS)
					  break;
		     }
		  break;
#if 0
	    else
	      if (cmd->func)
		{
		  (*cmd->func) (cmd, vty, 0, NULL);
		  break;
		}
#endif
	  }

		cmd_stat = CMD_SUCCESS;
#if 0	
		if (cmd->func)
		{
			int tmp_ret = (*cmd->func)(cmd, vty, 0, NULL);
			if(CMD_SUCCESS != tmp_ret)
				break;
		}
#endif			
		for (i = 0; i < VTYSH_INDEX_MAX; i++)
		  {
		    if (cmd->daemon & vtysh_client[i].flag)
		      {
		        cmd_stat = vtysh_client_execute(&vtysh_client[i], line, fp);

			if (cmd_stat != CMD_SUCCESS)
			  break;
		      }
		  }

	}
    }
  if (pager && vtysh_pager_name && fp && closepager)
    {

      if (pclose (fp) == -1)
	{
//	  perror ("pclose failed for pager");
	}

	fp = NULL;
    }
#if 0  
  if(pager && ret == CMD_SUCCESS && cmd_stat == CMD_SUCCESS)
  {
		/*"show" "down" "debug"*/
	  if(strncmp(line,"sh",2)
	  	&&strncmp(line,"do",2)
	  	&&strncmp(line,"deb",3)
	  	&&strcmp(line,"enable"))
		  dcli_send_dbus_signal_func("configure",line);

  }
#endif  
  vty->fp = NULL;
  
  /*gujd: 2012-05-31, am 11:01 . Delete it for rtmd open system fd too many .*/
  unlock_flags_file(flags_fd);
  return;
}


/* Command execution over the vty interface. for vtysh -c*/
int
vtysh_execute_func_4ret (const char *line)
{
  int ret=CMD_SUCCESS, cmd_stat =CMD_SUCCESS;
  u_int i;
  vector vline;
  struct cmd_element *cmd;
  FILE *fp = NULL;
  int closepager = 0;
  int tried = 0;
  int saved_ret, saved_node,flags_fd = -1;
  char *cmd_line;
#if 0
  /*gujd: 2012-05-31, am 11:01 . Delete it for rtmd open system fd too many .*/
  if(strncmp("write",line,2)==0)
  {
  	flags_fd = lock_flags_file(WRITING_FLAGS_FILE);
	if(flags_fd == -1)
	{
		vty_out(vty,"Another user is setting system,please wait for a few minutes and try again!\n");
		return;
  	}
  }
#endif
  /* Split readline string up into the vector. */
  vline = cmd_make_strvec (line);

  if (vline == NULL){
  
	  /*gujd: 2012-05-31, am 11:01 . Delete it for rtmd open system fd too many .
	  unlock_flags_file(flags_fd);
	  */

	return CMD_SUCCESS;

  }
  if(strncmp("write",*vline->index,2)==0)
   {
	 flags_fd = lock_flags_file(WRITING_FLAGS_FILE);
	 if(flags_fd == -1)
	 {
		 vty_out(vty,"Another user is setting system,please wait for a few minutes and try again!\n");
		 return;
	   }
   }

  fp = stdout;

  vty->fp = fp;

  ret = cmd_execute_command (vline, vty, &cmd, 1);

  cmd_free_strvec (vline);

  switch (ret)
    {
    case CMD_WARNING:
	case CMD_FAILURE:
      if (vty->type == VTY_FILE)
	fprintf (stdout,"Warning...\n");
      break;
    case CMD_ERR_AMBIGUOUS:
      fprintf (stdout,"%% Ambiguous command.\n");
      break;
    case CMD_ERR_NO_MATCH:
      fprintf (stdout,"%% Unknown command.\n");
      break;
    case CMD_ERR_INCOMPLETE:
      fprintf (stdout,"%% Command incomplete.\n");
      break;
	  case CMD_SUCCESS_DAEMON:
	  case CMD_SUCCESS:
	  	if (cmd->daemon)
     		 {
			if (! strcmp(cmd->string,"configure terminal"))
		  	{
		    		for (i = 0; i < VTYSH_INDEX_MAX; i++)
		      		{
		       		 cmd_stat = vtysh_client_execute(&vtysh_client[i], line, fp);
					if (cmd_stat == CMD_WARNING||cmd_stat == CMD_FAILURE)
			  			break;
		     		 }
        			if (cmd_stat)
        		      {
        				cmd_line = "end";
        				vline = cmd_make_strvec (cmd_line);
        				if (vline == NULL)
        			  	{
        					fp = NULL;
        					vty->fp = fp;
							unlock_flags_file(flags_fd);
        				       return CMD_WARNING;
        			    }
                        ret = cmd_execute_command (vline, vty, &cmd, 1);
        			       cmd_free_strvec (vline);
        			       if (ret != CMD_SUCCESS_DAEMON &&ret != CMD_SUCCESS)
        			      		 break;
        		       }
        			 break;
		      }
               	cmd_stat = CMD_SUCCESS;
               	for (i = 0; i < VTYSH_INDEX_MAX; i++)
               	{
               		   if (cmd->daemon & vtysh_client[i].flag)
               		   {
               		        cmd_stat = vtysh_client_execute(&vtysh_client[i], line, fp);
                                  if (cmd_stat != CMD_SUCCESS)
               			  break;
               		    }
               	 }
	  	}
	}
    
  
  vty->fp = NULL;
if(ret!=CMD_SUCCESS_DAEMON && ret!=CMD_SUCCESS )
{
	/*gujd: 2012-05-31, am 11:01 . Delete it for rtmd open system fd too many .*/
	unlock_flags_file(flags_fd);
	return ret;
}
/*gujd: 2012-05-31, am 11:01 . Delete it for rtmd open system fd too many .*/
unlock_flags_file(flags_fd);
return cmd_stat;
}

void
vtysh_execute_no_pager (const char *line)
{
  vtysh_execute_func (line, 0);
}

void
vtysh_execute (const char *line)
{
	execute_flag = 0;
	execute_flag++;
	
  	vtysh_execute_func (line, 1);
  	execute_flag--;
}

void trap_loadconfig_send(char* str)
{
	struct timeval tv;
	char tmp[256] = {0};
	
	sprintf(tmp,"Loadconfig conf_xml.conf: %s\n",str);
	syslog(LOG_NOTICE,"dongshu Loadconfig config failed: (%s)\n", tmp);
	dcli_send_dbus_signal_func("loadconfigure",tmp);
	tv.tv_sec=3;
	tv.tv_usec=0;
	select(0,NULL,NULL,NULL,&tv);
}

void log_start_write(char* str)
{
	FILE* fp;
	fp = fopen(OSSTARTERRFILE,"a");
	if(!fp)
		return;
	fprintf (fp, "%s\n", str);
	fflush (fp);
	fclose(fp);
//	trap_loadconfig_send(str);
}

void trap_make_loadconfig_info(char *info, const char*str)
{
	char tmp[256] = {0};
	
	sprintf(tmp,"%%conf_xml.conf:%s\n",str);
	strcat(info,tmp);
}
int get_print_config_flags(void)
{
	FILE* fp;
	int config_flag=0;
	
	fp = fopen(PRINT_CONFIG_FLAGS_FILE,"r");
	if(fp==NULL){
		return -1;
	}
	else
	{
		
		if(1!= fscanf(fp,"%d",&config_flag))
		{
			fclose(fp);
			return -1;
		}
		fclose(fp);
	
		if(config_flag == 1){
			return 1;
		}
	}
	return -1;
	
}
int exec_config_tmp_file(char* filename)
{
	
	pid_t pid;
	int status;
	char cmd[256];
	
	if(filename == NULL)
	{
		return -1;
	}
	pid = fork ();
	
	if (pid < 0)
	{
		/* Failure of fork(). */
		zlog_err("Can't fork: %s\n", safe_strerror (errno));
		return -1;
	}
	if(pid==0)
	{
		sprintf(cmd,"vtysh -b -f %s",filename);
		system(cmd);
		exit(1);
	}
	return 1;
	
}



/* Configration make from file. */
int
vtysh_config_from_file (struct vty *vty, FILE *fp)
{
	int ret,tmp_file_num=0;
	FILE* conffp;
	FILE* tmpfp;
	char tmp_config_file_name[128];

	vector vline;
	struct cmd_element *cmd;
	char errbuf[VTY_BUFSIZ];
	int len,exec_cmd_num=0,module_num=0;
	unsigned int config_flag = 1;
	unsigned int print_config_flag = 0;
	char info[VTY_BUFSIZ]={0};
	char tmpbuff[64]= {0};
	int fail_flag = 0,is_split=0;
	char cmd_bak[VTY_BUFSIZ]= {0};

#if 0	
	conffp=fopen("/var/run/newconf","r");
	if(conffp)
	{
		fscanf(conffp,"%d",&config_flag);
		fclose(conffp);
	}
#endif
  print_config_flag=get_print_config_flags();
  while (fgets (vty->buf, VTY_BUFSIZ, fp))
    {

	  if(strncmp(vty->buf,BEGIN_MODULE,strlen(BEGIN_MODULE))==0)
	  {
		if(is_split==0)
		{
			is_split++;
			/*打开新文件，开始记录*/
			sprintf(tmp_config_file_name,"%s%d.conf",TMP_CONFIG_FILE,tmp_file_num++);
			tmpfp = fopen(tmp_config_file_name,"w");
			if(tmpfp==NULL)
			{
				zlog_err ("Create new temp file error\n");
				return -1;
			}
			continue;

		}
		else
		{
			zlog_err ("Config file format is error\n");
			fclose(tmpfp);
			return -1;

		}
	  }

	  
	  if(is_split==1)
	  {
		  if(strncmp(vty->buf,END_MODULE,strlen(END_MODULE))==0)
		  {
			  is_split--;
			  /*这里关闭临时文件，并起新进程处理*/
			  if(tmpfp)
			  {
			  	fclose(tmpfp);
			  }
			  exec_config_tmp_file(tmp_config_file_name);
/*   ????????????? 
			  continue;
*/
		  }
		  else
		  {
		  	/*这里将配置文件写到临时文件中*/
			fprintf(tmpfp,vty->buf);
			continue;
		  }

	  }
	  else if(is_split<0 || is_split>1)
	  {
		  zlog_err ("Config file format is error\n");
		  break;

	  }
/*??????????????????????????????????????*/
	  if ( vty->buf[1] == '#' || vty->buf[0] == '!' )
				continue;
/*
			if (vty->buf[0] == '!' )
			{
				fprintf(stdout,"Executing %s\n",vty->buf+1);
				continue;
			}	
*/
    /**gjd : add for Distribute System load config file**/
	 if(fail_flag == 1)
	 {
	 	if(cmd_bak[0] != ' ')
	 	{
	 		if(strncmp(vty->buf, " exit",5) == 0 || vty->buf[0] == ' ')
			continue ;
	 	}
		
	 }
	/**2011-05-24: pm 3:15**/
	
      vline = cmd_make_strvec (vty->buf);

      /* In case of comment line. */
      if (vline == NULL)
      {
				continue;
      }
	  if(print_config_flag==1)
	  {
	  	  
		  struct cmd_node *node;
		  
		  vector cmdvec = get_cmdvec();
	  	  char buf[VTY_BUFSIZ];
		  strcpy(buf,vty->buf);
		  buf[strlen(buf)-1]= '\0';
		  
		  node = vector_slot (cmdvec, vty->node);
		  fprintf(stdout,"PID %d EXEC:[%s] node is [%s]\n",getpid(),buf,node->node_name);

	  }
	  else
	  {
		if(exec_cmd_num>10)
		{
			exec_cmd_num=0;
			fprintf(stdout," .");
		}
		else
		{
			exec_cmd_num++;
		}
	  }

      /* Execute configuration command : this is strict match. */
      {
	  	int i=0;
	  	retry:
	  	
	      ret = cmd_execute_command_strict (vline, vty, &cmd);
		  if(CMD_DBUS_ERR_RETRY == ret)
		  {if(i>3)
		  	goto out;
		  	fprintf(stdout,"\ncommand [%s] run warning.\nreinit dbus and retry\n",vty->buf);
#ifdef DISTRIBUT
			dl_dcli_init(0);
#else
			dl_dcli_init();
#endif
			i++;
			goto retry;
		  }
		  out:
		  	;
      }
#if 0
      /* Try again with setting node to CONFIG_NODE. */
      if (ret != CMD_SUCCESS 
	  && ret != CMD_SUCCESS_DAEMON
	  && ret != CMD_WARNING)
	{
	  if (vty->node == KEYCHAIN_KEY_NODE)
	    {
	      vty->node = KEYCHAIN_NODE;
	      vtysh_exit_ripd_only ();
	      ret = cmd_execute_command_strict (vline, vty, &cmd);

	      if (ret != CMD_SUCCESS 
		  && ret != CMD_SUCCESS_DAEMON 
		  && ret != CMD_WARNING)
			{
			  vtysh_exit_ripd_only ();
			  vty->node = CONFIG_NODE;
			  ret = cmd_execute_command_strict (vline, vty, &cmd);
			}
	    }
	  else
	    {
	      vtysh_execute ("end");
	      vtysh_execute ("configure terminal");
	      vty->node = CONFIG_NODE;
	      ret = cmd_execute_command_strict (vline, vty, &cmd);
	    }
	}	  
#endif
      cmd_free_strvec (vline);
			memset(errbuf,0,512);
	if(config_flag && ret!=CMD_SUCCESS && ret!=CMD_SUCCESS_DAEMON)
		config_flag++;
		
      switch (ret)
	{
	case CMD_WARNING:
	case CMD_FAILURE:
		sprintf(errbuf,"%%  Warning ...... cmd %s . ",vty->buf);		
		log_start_write(errbuf);
		sprintf(tmpbuff,"%%ERROR #1051");
		trap_make_loadconfig_info(info,tmpbuff);
		fail_flag = 1;
		strcpy(cmd_bak,vty->buf);
	  break;
	case CMD_ERR_AMBIGUOUS:
		sprintf(errbuf,"%% Ambiguous command: %s .\n",vty->buf);		
		log_start_write(errbuf);
		sprintf(tmpbuff,"%%ERROR #1052");
		trap_make_loadconfig_info(info,tmpbuff);
	  break;
	case CMD_ERR_NO_MATCH:
		sprintf(errbuf,"%% Unknown command: %s .\n",vty->buf);		
		log_start_write(errbuf);
		sprintf(tmpbuff,"%%ERROR #1053");
		trap_make_loadconfig_info(info,tmpbuff);
		fail_flag = 1;
		strcpy(cmd_bak,vty->buf);

		break;
	case CMD_ERR_INCOMPLETE:
		sprintf(errbuf,"%% Command incomplete: %s .\n",vty->buf);		
		log_start_write(errbuf);
		sprintf(tmpbuff,"%%ERROR #1054");
		trap_make_loadconfig_info(info,tmpbuff);
		break;
  case CMD_SUCCESS_DAEMON:
  case CMD_SUCCESS:
	  {
			
	    u_int i;
	    int cmd_stat = CMD_SUCCESS;
			
	    for (i = 0; i < VTYSH_INDEX_MAX; i++)
	  {
	    if (cmd->daemon & vtysh_client[i].flag)
			  {
			    cmd_stat = vtysh_client_execute (&vtysh_client[i],
							     vty->buf, stdout);
			    if (cmd_stat != CMD_SUCCESS)
			      break;
			  }
	  }
		    if (cmd_stat != CMD_SUCCESS)
			{
			
				if(config_flag)
					config_flag++;
				sprintf(errbuf,"%% Command error: ...... %s .\n",vty->buf);		
				log_start_write(errbuf);
				sprintf(tmpbuff,"%%ERROR #1055");
				trap_make_loadconfig_info(info,tmpbuff);
		      break;

			}
			fail_flag = 0;
			memset(cmd_bak,0,VTY_BUFSIZ);
		}
		break;
	default:
		break;
	  }
    }
	
  sleep(5);
  if(config_flag>1)
  {
  	char tmp[64]={0};
		char *pstr=NULL;
		int i;

		
		sprintf(tmp,"There are %d command exec failure %%",config_flag-1);

		if(config_flag > 5)
		{
			strcat(tmp, "ERROR FILE: conf_xml.conf.");
			dcli_send_dbus_signal_func("update_config_load_failure",tmp);
			return CMD_SUCCESS;
		}

		pstr = strdup(info);
		i = strlen(tmp);
		memcpy(info+i, pstr, strlen(pstr)+1);
		free(pstr);
		
		memcpy(info, tmp, i);
	  dcli_send_dbus_signal_func("update_config_load_failure",info);
  }
  else if(config_flag == 1)
  {
	  dcli_send_dbus_signal_func("update_config_load_success","update config load success");
  }
  	
  return CMD_SUCCESS;
}

/* We don't care about the point of the cursor when '?' is typed. */
int
vtysh_rl_describe (void)
{
  int ret;
  unsigned int i;
  vector vline;
  vector describe;
  int width;
  struct desc *desc;

  vline = cmd_make_strvec (rl_line_buffer);

  /* In case of '> ?'. */
  if (vline == NULL)
    {
      vline = vector_init (1);
      vector_set (vline, '\0');
    }
  else 
    if (rl_end && isspace ((int) rl_line_buffer[rl_end - 1]))
      vector_set (vline, '\0');

  describe = cmd_describe_command (vline, vty, &ret);

  fprintf (stdout,"\n");

  /* Ambiguous and no match error. */
  switch (ret)
    {
    case CMD_ERR_AMBIGUOUS:
      cmd_free_strvec (vline);
      fprintf (stdout,"%% Ambiguous command.\n");
      rl_on_new_line ();
      return 0;
      break;
    case CMD_ERR_NO_MATCH:
      cmd_free_strvec (vline);
      fprintf (stdout,"%% There is no matched command.\n");
      rl_on_new_line ();
      return 0;
      break;
    }  

  /* Get width of command string. */
  width = 0;
  for (i = 0; i < vector_active (describe); i++)
    if ((desc = vector_slot (describe, i)) != NULL)
      {
	int len;

	if (desc->cmd[0] == '\0')
	  continue;

	len = strlen (desc->cmd);
	if (desc->cmd[0] == '.')
	  len--;

	if (width < len)
	  width = len;
      }

  for (i = 0; i < vector_active (describe); i++)
    if ((desc = vector_slot (describe, i)) != NULL)
      {
	if (desc->cmd[0] == '\0')
	  continue;

	if (! desc->str)
	  fprintf (stdout,"  %-s\n",
		   desc->cmd[0] == '.' ? desc->cmd + 1 : desc->cmd);
	else
	  fprintf (stdout,"  %-*s  %s\n",
		   width,
		   desc->cmd[0] == '.' ? desc->cmd + 1 : desc->cmd,
		   desc->str);
      }

  cmd_free_strvec (vline);
  vector_free (describe);

  rl_on_new_line();

  return 0;
}

/* Result of cmd_complete_command() call will be stored here
 * and used in new_completion() in order to put the space in
 * correct places only. */
int complete_status;

static char *
command_generator (const char *text, int state)
{
  vector vline;
  static char **matched = NULL;
  static int index = 0;

  /* First call. */
  if (! state)
    {
      index = 0;

      if (vty->node == AUTH_NODE || vty->node == AUTH_ENABLE_NODE)
	return NULL;

      vline = cmd_make_strvec (rl_line_buffer);
      if (vline == NULL)
	return NULL;

      if (rl_end && isspace ((int) rl_line_buffer[rl_end - 1]))
	vector_set (vline, '\0');

      matched = cmd_complete_command (vline, vty, &complete_status);
    }

  if (matched && matched[index])
    return matched[index++];

  return NULL;
}

static char **
new_completion (char *text, int start, int end)
{
  char **matches;

  matches = rl_completion_matches (text, command_generator);

  if (matches)
    {
      rl_point = rl_end;
      if (complete_status == CMD_COMPLETE_FULL_MATCH)
	rl_pending_input = ' ';
    }

  return matches;
}

#if 0
/* This function is not actually being used. */
static char **
vtysh_completion (char *text, int start, int end)
{
  int ret;
  vector vline;
  char **matched = NULL;

  if (vty->node == AUTH_NODE || vty->node == AUTH_ENABLE_NODE)
    return NULL;

  vline = cmd_make_strvec (rl_line_buffer);
  if (vline == NULL)
    return NULL;

  /* In case of 'help \t'. */
  if (rl_end && isspace ((int) rl_line_buffer[rl_end - 1]))
    vector_set (vline, '\0');

  matched = cmd_complete_command (vline, vty, &ret);

  cmd_free_strvec (vline);

  return (char **) matched;
}
#endif

/* Vty node structures. */
struct cmd_node bgp_node =
{
  BGP_NODE,
  "%s(config-router)# ",
};

struct cmd_node rip_node =
{
  RIP_NODE,
  "%s(config-router)# ",
};

struct cmd_node isis_node =
{
  ISIS_NODE,
  "%s(config-router)# ",
};

struct cmd_node interface_node =
{
  INTERFACE_NODE,
  "%s(config-if)# ",
};

struct cmd_node rmap_node =
{
  RMAP_NODE,
  "%s(config-route-map)# "
};

struct cmd_node zebra_node =
{
  ZEBRA_NODE,
  "%s(config-router)# "
};

struct cmd_node bgp_vpnv4_node =
{
  BGP_VPNV4_NODE,
  "%s(config-router-af)# "
};

struct cmd_node bgp_ipv4_node =
{
  BGP_IPV4_NODE,
  "%s(config-router-af)# "
};

struct cmd_node bgp_ipv4m_node =
{
  BGP_IPV4M_NODE,
  "%s(config-router-af)# "
};

struct cmd_node bgp_ipv6_node =
{
  BGP_IPV6_NODE,
  "%s(config-router-af)# "
};

struct cmd_node bgp_ipv6m_node =
{
  BGP_IPV6M_NODE,
  "%s(config-router-af)# "
};

struct cmd_node ospf_node =
{
  OSPF_NODE,
  "%s(config-router)# "
};

struct cmd_node ripng_node =
{
  RIPNG_NODE,
  "%s(config-router)# "
};

struct cmd_node ospf6_node =
{
  OSPF6_NODE,
  "%s(config-ospf6)# "
};

struct cmd_node keychain_node =
{
  KEYCHAIN_NODE,
  "%s(config-keychain)# "
};

struct cmd_node keychain_key_node =
{
  KEYCHAIN_KEY_NODE,
  "%s(config-keychain-key)# "
};
struct cmd_node hidden_debug_node =
{
  HIDDENDEBUG_NODE,
  "%s(hidden-debug)# ",
  1
};


/* Defined in lib/vty.c */
extern struct cmd_node vty_node;
extern struct cmd_node auth_node ;
extern struct cmd_node view_node;
extern struct cmd_node auth_enable_node ;
extern struct cmd_node enable_node ;
extern struct cmd_node config_node;

extern 	int	config_write_host (struct vty *vty);
extern 	void *dcli_dl_handle;

int (*dcli_show_runnig_config_func)(void)= NULL;
int(*dcli_wcpss_show_runnig_config_func)(void)=NULL;
int(*dcli_wcpss_show_runnig_config_end_func)(void)=NULL;
static int dl_dcli_show_running_config(void)
{
  char *error;
  dcli_dl_handle = dlopen("libdcli.so.0",RTLD_NOW);
  if (!dcli_dl_handle) {
      fputs (dlerror(),stderr);
      fprintf(stderr,"Run without libdcli.so.0\n");
	  return -1;      
  }
  dcli_show_runnig_config_func = dlsym(dcli_dl_handle,"dcli_show_running_config");
  if ((error = dlerror()) != NULL) {
      fprintf(stderr,"Run without dcli_init be called.\n");
      fputs(error,stderr);
	  return -1;	  

  }

  return 1;

}

static int dl_dcli_static_arp_show_running_config(void)
{
  char *error;
  dcli_dl_handle = dlopen("libdcli.so.0",RTLD_NOW);
  if (!dcli_dl_handle) {
      fputs (dlerror(),stderr);
      fprintf(stderr,"Run without libdcli.so.0\n");
	  return -1;      
  }
  dcli_show_runnig_config_func = dlsym(dcli_dl_handle,"dcli_static_arp_show_running_config");
  if ((error = dlerror()) != NULL) {
      fprintf(stderr,"Run without dcli_init be called.\n");
      fputs(error,stderr);
	  return -1;	  

  }

  return 1;

}

static int dl_dcli_wcpss_show_running_config(void)
{
  char *error;
  dcli_dl_handle = dlopen("libdcli.so.0",RTLD_NOW);
  if (!dcli_dl_handle) {
      fputs (dlerror(),stderr);
      fprintf(stderr,"Run without libdcli.so.0\n");
	  return -1;      
  }
  dcli_wcpss_show_runnig_config_func = dlsym(dcli_dl_handle,"dcli_wcpss_show_running_config_start");
  if ((error = dlerror()) != NULL) {
      fprintf(stderr,"Run without dcli_init be called.\n");
      fputs(error,stderr);
	  return -1;	  

  }

  return 1;

}

static int dl_dcli_wcpss_show_running_config_end(void)
{
  char *error;
  dcli_dl_handle = dlopen("libdcli.so.0",RTLD_NOW);
  if (!dcli_dl_handle) {
      fputs (dlerror(),stderr);
      fprintf(stderr,"Run without libdcli.so.0\n");
	  return -1;      
  }
  dcli_wcpss_show_runnig_config_end_func = dlsym(dcli_dl_handle,"dcli_wcpss_show_running_config_end");
  if ((error = dlerror()) != NULL) {
      fprintf(stderr,"Run without dcli_init be called.\n");
      fputs(error,stderr);
	  return -1;	  

  }

  return 1;

}


static int vtysh_show_running_config (void)
{
	int ret;
/*		
	ret = config_write_host(vty);
*/
	ret = dl_dcli_show_running_config();
	if(ret != 1)
	{
		fprintf(stderr,"Run without dcli_show_running_config be called.\n");
		return -1;
	}
  	 (*dcli_show_runnig_config_func)();
	 
	 dlclose(dcli_dl_handle);
	 return 1;

}

 static int vtysh_static_arp_show_running_config (void)
 {
	 int ret;
 /* 	 
	 ret = config_write_host(vty);
 */
	 ret = dl_dcli_static_arp_show_running_config();
	 if(ret != 1)
	 {
		 fprintf(stderr,"Run without dcli_show_running_config be called.\n");
		 return -1;
	 }
	  (*dcli_show_runnig_config_func)();
	  
	  dlclose(dcli_dl_handle);
	  return 1;
 
 }

 int vtysh_wcpss_show_running_config (void)
{
	int ret;
/*		
	ret = config_write_host(vty);
*/
	ret = dl_dcli_wcpss_show_running_config();
	if(ret != 1)
	{
		fprintf(stderr,"Run without dcli_show_running_config be called.\n");
		return -1;
	}
  	 (*dcli_wcpss_show_runnig_config_func)();
	 
	 dlclose(dcli_dl_handle);
	 return 1;

}

 int vtysh_wcpss_show_running_config_end (void)
{
	int ret;
/*		
	ret = config_write_host(vty);
*/
	ret = dl_dcli_wcpss_show_running_config_end();
	if(ret != 1)
	{
		fprintf(stderr,"Run without dcli_show_running_config be called.\n");
		return -1;
	}
  	 (*dcli_wcpss_show_runnig_config_end_func)();
	 
	 dlclose(dcli_dl_handle);
	 return 1;

}


void vtysh_add_entry(char *cmd)
{
	vtysh_config_parse_line(cmd);
	
/*	strcat(config_buf,cmd); */
if(config_fd>0)
	{
	fprintf(config_fd,"%s",cmd);
	}
	
}

void vtysh_add_show_string_parse(char *showStr) 
{
	vtysh_config_parse(showStr);


	

}

void vtysh_add_show_string(char *showStr) 
{
#if 0
	char *cursor = NULL,ch = 0,tmpBuf[SHOWRUN_PERLINE_SIZE] = {0};
	int count = 0,buffLen = 0,i = 0;

	buffLen = strlen(showStr);
	cursor = showStr;
	ch = cursor[0];
	count = i = 0;
	while((ch != '\0')&&(count <= buffLen)) {
		if(ch != '\n') {
			ch = cursor[++i];
		}
		else if((i > 0) && (i <= SHOWRUN_PERLINE_SIZE)) { // match one command line
			memcpy(tmpBuf,cursor,i);
			//printf("DBUG:%s\n",tmpBuf);
			vtysh_add_entry(tmpBuf);
			memset(tmpBuf,0,SHOWRUN_PERLINE_SIZE);
			count += i;
			cursor += i;
			i = 0;
			ch = cursor[0];
		}
		else if(0 == i) { // special line has only '\n'
			count++;
			cursor++;
			ch = cursor[0];
		}
		else { // ch == '\n' but line oversized
			// skip command line with too long size
			printf("skip command: %s,size %d too long\n",tmpBuf,i);
			memset(tmpBuf,0,SHOWRUN_PERLINE_SIZE);
			count += i;
			cursor += i;
			i = 0;
			ch = cursor[0]; 			
		}
	}
#else
	vtysh_add_entry(showStr);


#endif

}

/* When '^Z' is received from vty, move down to the enable mode. */


DEFUNSH (VTYSH_BGPD,
	 router_bgp,
	 router_bgp_cmd,
	 "router bgp <1-65535>",
	 ROUTER_STR
	 BGP_STR
	 AS_STR)
{
  vty->node = BGP_NODE;
  return CMD_SUCCESS;
}

DEFUNSH (VTYSH_BGPD,
	 address_family_vpnv4,
	 address_family_vpnv4_cmd,
	 "address-family vpnv4",
	 "Enter Address Family command mode\n"
	 "Address family\n")
{
  vty->node = BGP_VPNV4_NODE;
  return CMD_SUCCESS;
}

DEFUNSH (VTYSH_BGPD,
	 address_family_vpnv4_unicast,
	 address_family_vpnv4_unicast_cmd,
	 "address-family vpnv4 unicast",
	 "Enter Address Family command mode\n"
	 "Address family\n"
	 "Address Family Modifier\n")
{
  vty->node = BGP_VPNV4_NODE;
  return CMD_SUCCESS;
}

DEFUNSH (VTYSH_BGPD,
	 address_family_ipv4_unicast,
	 address_family_ipv4_unicast_cmd,
	 "address-family ipv4 unicast",
	 "Enter Address Family command mode\n"
	 "Address family\n"
	 "Address Family Modifier\n")
{
  vty->node = BGP_IPV4_NODE;
  return CMD_SUCCESS;
}

DEFUNSH (VTYSH_BGPD,
	 address_family_ipv4_multicast,
	 address_family_ipv4_multicast_cmd,
	 "address-family ipv4 multicast",
	 "Enter Address Family command mode\n"
	 "Address family\n"
	 "Address Family Modifier\n")
{
  vty->node = BGP_IPV4M_NODE;
  return CMD_SUCCESS;
}

DEFUNSH (VTYSH_BGPD,
	 address_family_ipv6,
	 address_family_ipv6_cmd,
	 "address-family ipv6",
	 "Enter Address Family command mode\n"
	 "Address family\n")
{
  vty->node = BGP_IPV6_NODE;
  return CMD_SUCCESS;
}

DEFUNSH (VTYSH_BGPD,
	 address_family_ipv6_unicast,
	 address_family_ipv6_unicast_cmd,
	 "address-family ipv6 unicast",
	 "Enter Address Family command mode\n"
	 "Address family\n"
	 "Address Family Modifier\n")
{
  vty->node = BGP_IPV6_NODE;
  return CMD_SUCCESS;
}

DEFUNSH (VTYSH_BGPD,
	 address_family_ipv6_multicast,
	 address_family_ipv6_multicast_cmd,
	 "address-family ipv6 multicast",
	 "Enter Address Family command mode\n"
	 "Address family\n"
	 "Address Family Modifier\n")
{
  vty->node = BGP_IPV6M_NODE;
  return CMD_SUCCESS;
}

DEFUNSH (VTYSH_RIPD,
	 key_chain,
	 key_chain_cmd,
	 "key chain WORD",
	 "Authentication key management\n"
	 "Key-chain management\n"
	 "Key-chain name\n")
{
  vty->node = KEYCHAIN_NODE;
  return CMD_SUCCESS;
}	 

DEFUNSH (VTYSH_RIPD,
	 key,
	 key_cmd,
	 "key <0-2147483647>",
	 "Configure a key\n"
	 "Key identifier number\n")
{
  vty->node = KEYCHAIN_KEY_NODE;
  return CMD_SUCCESS;
}

DEFUNSH (VTYSH_RIPD,
	 router_rip,
	 router_rip_cmd,
	 "router rip",
	 ROUTER_STR
	 "RIP")
{
  vty->node = RIP_NODE;
  return CMD_SUCCESS;
}
#if 0
DEFUNSH (VTYSH_RIPNGD,
	 router_ripng,
	 router_ripng_cmd,
	 "router ripng",
	 ROUTER_STR
	 "RIPng")
{
  vty->node = RIPNG_NODE;
  return CMD_SUCCESS;
}
#endif
DEFUNSH (VTYSH_OSPFD,
	 router_ospf,
	 router_ospf_cmd,
	 "router ospf",
	 "Enable a routing process\n"
	 "Start OSPF configuration\n")
{
  vty->node = OSPF_NODE;
  return CMD_SUCCESS;
}
#if 0
DEFUNSH (VTYSH_OSPF6D,
	 router_ospf6,
	 router_ospf6_cmd,
	 "router ospf6",
	 OSPF6_ROUTER_STR
	 OSPF6_STR)
{
  vty->node = OSPF6_NODE;
  return CMD_SUCCESS;
}

DEFUNSH (VTYSH_ISISD,
	 router_isis,
	 router_isis_cmd,
	 "router isis WORD",
	 ROUTER_STR
	 "ISO IS-IS\n"
	 "ISO Routing area tag")
{
  vty->node = ISIS_NODE;
  return CMD_SUCCESS;
}
#endif
DEFUNSH (VTYSH_RMAP,
	 route_map,
	 route_map_cmd,
	 "route-map WORD (deny|permit) <1-65535>",
	 "Create route-map or enter route-map command mode\n"
	 "Route map tag\n"
	 "Route map denies set operations\n"
	 "Route map permits set operations\n"
	 "Sequence to insert to/delete from existing route-map entry\n")
{
  vty->node = RMAP_NODE;
  return CMD_SUCCESS;
}

DEFUNSH (VTYSH_ALL,
	 vtysh_line_vty,
	 vtysh_line_vty_cmd,
	 "line vty",
	 "Configure a terminal line\n"
	 "Virtual terminal\n")
{
  vty->node = VTY_NODE;
  return CMD_SUCCESS;
}
#define VIEWGROUP "vtyview"
#define ADMINGROUP "vtyadmin"

int _get_self_role()
{
	int uid,i;
	struct passwd *passwd = NULL;
	struct group *group=NULL;
	char *name;

	passwd = getpwuid(getuid());
	if(passwd)
		name = passwd->pw_name;
	else
		return -1;

	if(default_admin_user)
	{
		int len = strlen(default_admin_user);
		char* str= default_admin_user;
		char* ptr = NULL;

		while(ptr = strchr(str,','))
		{
			if(!strncmp(str,name,ptr-str))
				return 2;
			str = ptr+1;
		}
		if(!strcmp(str,name))
			return 2;
	}
	
	group = getgrnam(ADMINGROUP);
	if(group && group->gr_mem )
	{
		for(i=0;group->gr_mem[i];i++)
			if(!strcmp(group->gr_mem[i],name))
				return 1;

	}
	
	group = getgrnam(VIEWGROUP);
	if(group && group->gr_mem )
	{
		for(i=0;group->gr_mem[i];i++)
			if(!strcmp(group->gr_mem[i],name))
				return 0;

	}
	return -1;
}
#if 0
DEFUNSH (VTYSH_ALL,
	 vtysh_enable, 
	 vtysh_enable_cmd,
	 "enable",
	 "Turn on privileged mode command\n")
{
	int ret;
	ret = _get_self_role();
	if(ret < 1)
	{
		vty_out(vty,"You aren't admin user,can't enter enable mode\n");
		return CMD_FAILURE;
	}
	vty->node = ENABLE_NODE;
	return CMD_SUCCESS;

}

#else
DEFUN (vtysh_enable, 
	 vtysh_enable_cmd,
	 "enable",
	 "Turn on privileged mode command\n")
{
	int ret;
	int i;
	char line[] = "enable";
	int is_active_master = 0;
	
	ret = _get_self_role();
	if(ret < 1)
	{
		vty_out(vty,"You aren't admin user,can't enter enable mode\n");
		return CMD_SUCCESS;
	}
	
#if 1
	is_active_master = judge_is_active_master(); 
	if(is_active_master > 0 || vtysh_enable_flags == 1 || vtysh_config_flag == 1)
	{
		for (i = 0; i < VTYSH_INDEX_MAX; i++)
		{
			if ( vtysh_client[i].fd >= 0 )
			  {
				ret = vtysh_client_execute (&vtysh_client[i], line, stdout);
			  }
		}

		vty->node = ENABLE_NODE;
		return CMD_SUCCESS;
	
	}
	else
	{
      vty_out (vty, "This board is not active master board ! You can't enter enable mode !\n");

	  return CMD_WARNING;
	  }
   	
#endif

#if 0
	
	for (i = 0; i < VTYSH_INDEX_MAX; i++)
	{
		if ( vtysh_client[i].fd >= 0 )
		  {
			ret = vtysh_client_execute (&vtysh_client[i], line, stdout);
		  }
	}
	
	vty->node = ENABLE_NODE;
	return CMD_SUCCESS;
#endif
}
#endif
DEFUNSH (VTYSH_ALL,
	 vtysh_disable, 
	 vtysh_disable_cmd,
	 "disable",
	 "Turn off privileged mode command\n")
{
  if (vty->node == ENABLE_NODE)
    vty->node = VIEW_NODE;
  return CMD_SUCCESS;
}

/* start - added by zhengbo 2011.09.19 */
/* check configure terminal passwd */
const char *ctpasswdfile = "/var/run/ctpasswd";
const char *ctpasswdfile_tmp = "/var/run/ctpasswd_tmp";
const char *ctpasswd_key = "Autelan";

static int
ct_passwd_write_enc(char *passwd, char *file)
{
	char passwd_cmd[256] = {0};
	
	sprintf(passwd_cmd, "openssl passwd -crypt -salt %s \"%s\" > %s", 
					ctpasswd_key, passwd, file);
	system(passwd_cmd);

	return CMD_SUCCESS;
}

static int
ct_passwd_write(char *passwd, char *file)
{
	int fd;
	int passwdlen = strlen(passwd);
	
	fd = open(file, O_RDWR | O_CREAT | O_TRUNC, 0644);
	if(fd < 0) {
		syslog(LOG_ERR, "open configure terminal passwd file failed at func %s\n", __func__);
		return CMD_FAILURE;
	}
	
	if(write(fd, passwd, passwdlen) != passwdlen) {
		syslog(LOG_ERR, "write configure terminal passwd to file failed at func %s\n", __func__);
		close(fd);
		unlink(file);
		return CMD_FAILURE;
	}

	close(fd);
	return CMD_SUCCESS;
}

static int
ct_passwd_read(char *passwd, int len, char *file)
{
	int fd;
	char *offset;
	
	fd = open(file, O_RDWR);
	if(fd < 0) {
		/* may be should check errno */
		syslog(LOG_ERR, "open configure terminal passwd file failed at func %s\n", __func__);
		return CMD_WARNING;
	}
	
	if(read(fd, passwd, len) < 0) {
		syslog(LOG_ERR, "read configure terminal passwd file failed at func %s\n", __func__);
		close(fd);
		return CMD_WARNING;
	}

	offset = strchr(passwd, '\r');
	if(offset)
		*offset = '\0';
	offset = strchr(passwd, '\n');
	if(offset)
		*offset = '\0';

	close(fd);
	return CMD_SUCCESS;
}

static int
ct_passwd_input()
{
	char *offset;
	char passwd[128] = {0};
	
	fprintf(stdout, "Please input passwd:");
	fflush(stdout);

	//hide_input_echo();
	fgets(passwd, sizeof(passwd), stdin);
	//show_input_echo();

	fprintf(stdout, "\n");
	
	offset = strchr(passwd, '\r');
	if(offset)
		*offset = '\0';
	offset = strchr(passwd, '\n');
	if(offset)
		*offset = '\0';

#if 0
	fprintf(stdout, "passwd inputed: [%s]\n", passwd);
#endif

	ct_passwd_write_enc(passwd, (char *)ctpasswdfile_tmp);

	return CMD_SUCCESS;
}

int
ct_passwd_isenabled()
{
	int fd;
	
	fd = open(ctpasswdfile, O_RDONLY);
	if(fd < 0) {
		/* may be should check errno */
		return 0;
	}
	
	if(!lseek(fd, 0, SEEK_END)) {
		close(fd);
		unlink(ctpasswdfile);
		return 0;
	}
	
	close(fd);
	return 1;
}

int 
ct_passwd_check()
{
	char passwd_src[128] = {0};
	char passwd_dst[128] = {0};
	int ret;

	ret = ct_passwd_read(passwd_src, sizeof(passwd_src), (char *)ctpasswdfile);
	if(ret == CMD_FAILURE)
		return CMD_FAILURE;
	else if(ret == CMD_WARNING)
		return CMD_SUCCESS;
	
	if(ct_passwd_input() == CMD_FAILURE)
		goto fail;
	
	ret = ct_passwd_read(passwd_dst, sizeof(passwd_src), (char *)ctpasswdfile_tmp);
	if(ret == CMD_FAILURE)
			goto fail;
#if 0	
	fprintf(stdout, "src Password: [%s], dst Password: [%s]\n", 
					passwd_src, passwd_dst);
#endif
	if(strcmp(passwd_src, passwd_dst))
		goto fail;
	
	ret = CMD_SUCCESS;

back:
	unlink(ctpasswdfile_tmp);
	return ret;

fail:
	fprintf(stdout, "Passwd error\n");
	ret = CMD_FAILURE;
	goto back;
}

int
ct_passwd_set(char *passwd, int enc)
{
	int ret;
	
	if(enc) {
		ret = ct_passwd_write_enc(passwd, (char *)ctpasswdfile);
	}
	else {
		ret = ct_passwd_write(passwd, (char *)ctpasswdfile);
	}
	
	if(ret == CMD_FAILURE) {
		return CMD_FAILURE;
	}
	
	return CMD_SUCCESS;
}

int 
ct_passwd_get(char *passwd, int len)
{
	int ret;
	ret = ct_passwd_read(passwd, len, (char *)ctpasswdfile);
	if(ret == CMD_FAILURE)
			return CMD_FAILURE;
	return CMD_SUCCESS;
}

void
ct_passwd_delete()
{
	unlink(ctpasswdfile);
	return ;
}

extern int boot_flag;
extern int cmd_flag;

DEFUNSH (VTYSH_ALL,
	 vtysh_config_terminal,
	 vtysh_config_terminal_cmd,
	 "configure terminal",
	 "Configuration from vty interface\n"
	 "Configuration terminal\n")
{
	if(!(boot_flag || cmd_flag) && ct_passwd_isenabled()) {
		if(ct_passwd_check() == CMD_FAILURE)
			return CMD_FAILURE;
	}
	
	vty->node = CONFIG_NODE;
	return CMD_SUCCESS;
}

DEFUN (config_terminal_passwd,
       config_terminal_passwd_cmd,
       "config passwd PASSWD",
       CONFIG_STR
       "Configure terminal passwd\n"
       "Set configuration terminal passwd\n")
{
	if(boot_flag) {
		if(ct_passwd_set((char *)argv[0], 0) != CMD_SUCCESS)
			goto fail;
	}
	else {
		if(ct_passwd_set((char *)argv[0], 1) != CMD_SUCCESS)
			goto fail;
	}

	return CMD_SUCCESS;

fail:
	vty_out(vty, "Set configure terminal passwd failed%s", VTY_NEWLINE);
	return CMD_FAILURE;
}

DEFUN (no_config_terminal_passwd,
       no_config_terminal_passwd_cmd,
       "no config passwd",
       NO_STR
       CONFIG_STR
       "Configure terminal passwd\n")
{
	if(ct_passwd_isenabled()) {
		ct_passwd_delete();
	}
	else {
		vty_out(vty, "Configure terminal password is not set%s", VTY_NEWLINE);
	}

	return CMD_SUCCESS;
}
/* end - added by zhengbo 2011.09.19 */

#undef VIEWGROUP 
#undef ADMINGROUP 

static int
vtysh_exit (struct vty *vty)
{
  switch (vty->node)
    {
    case VIEW_NODE:
    case ENABLE_NODE:
      exit (0);
      break;
    case CONFIG_NODE:
      vty->node = ENABLE_NODE;
      break;
    case INTERFACE_NODE:		
		if(vty->prenode == HANSI_NODE){			
			vty->node = HANSI_NODE;
			vty->index = vty->index_sub;
			vty->prenode = 0;
			break;
		}		
		if(vty->prenode == LOCAL_HANSI_NODE){			
			vty->node = LOCAL_HANSI_NODE;
			vty->index = vty->index_sub;
			vty->prenode = 0;
			break;
		}		
		vty->node = CONFIG_NODE;
		vty->index = NULL;
      	break;
    case ZEBRA_NODE:
    case BGP_NODE:
    case RIP_NODE:
    case RIPNG_NODE:
    case OSPF_NODE:
    case OSPF6_NODE:
    case ISIS_NODE:
    case MASC_NODE:
    case RMAP_NODE:
    case VTY_NODE:
    case KEYCHAIN_NODE:
/*

      vtysh_execute("end");
      vtysh_execute("configure terminal");

*/
      vty->node = CONFIG_NODE;
      break;
    case BGP_VPNV4_NODE:
    case BGP_IPV4_NODE:
    case BGP_IPV4M_NODE:
    case BGP_IPV6_NODE:
    case BGP_IPV6M_NODE:
      vty->node = BGP_NODE;
      break;
    case KEYCHAIN_KEY_NODE:
      vty->node = KEYCHAIN_NODE;
      break;
    default:
      break;
    }
  return CMD_SUCCESS;
}

DEFUNSH (VTYSH_ALL,
	 vtysh_exit_all,
	 vtysh_exit_all_cmd,
	 "exit",
	 "Exit current mode and down to previous mode\n")
{
  return vtysh_exit (vty);
}
#if 0
ALIAS (vtysh_exit_all,
       vtysh_quit_all_cmd,
       "quit",
       "Exit current mode and down to previous mode\n")
#else
DEFUNSH (VTYSH_ALL,
	 vtysh_quit_all,
	 vtysh_quit_all_cmd,
	 "quit",
	 "Exit current mode and down to previous mode\n")
{
  return vtysh_exit (vty);
}

#endif
DEFUNSH (VTYSH_BGPD,
	 exit_address_family,
	 exit_address_family_cmd,
	 "exit-address-family",
	 "Exit from Address Family configuration mode\n")
{
  if (vty->node == BGP_IPV4_NODE
      || vty->node == BGP_IPV4M_NODE
      || vty->node == BGP_VPNV4_NODE
      || vty->node == BGP_IPV6_NODE
      || vty->node == BGP_IPV6M_NODE)
    vty->node = BGP_NODE;
  return CMD_SUCCESS;
}

DEFUNSH (VTYSH_ZEBRA,
	 vtysh_exit_zebra,
	 vtysh_exit_zebra_cmd,
	 "exit",
	 "Exit current mode and down to previous mode\n")
{
  return vtysh_exit (vty);
}
#if 1
DEFUNSH (VTYSH_ZEBRA,
	 vtysh_quit_zebra,
	vtysh_quit_zebra_cmd,
	"quit",
	"Exit current mode and down to previous mode\n")
{
  return vtysh_exit (vty);
}

#else
ALIAS (vtysh_exit_zebra,
       vtysh_quit_zebra_cmd,
       "quit",
       "Exit current mode and down to previous mode\n")


#endif
DEFUNSH (VTYSH_RIPD,
	 vtysh_exit_ripd,
	 vtysh_exit_ripd_cmd,
	 "exit",
	 "Exit current mode and down to previous mode\n")
{
  return vtysh_exit (vty);
}
#if 0
ALIAS (vtysh_exit_ripd,
       vtysh_quit_ripd_cmd,
       "quit",
       "Exit current mode and down to previous mode\n")
#else

DEFUNSH (VTYSH_RIPD,
	 vtysh_quit_ripd,
	vtysh_quit_ripd_cmd,
	"quit",
	"Exit current mode and down to previous mode\n")
{
  return vtysh_exit (vty);
}

#endif
DEFUNSH (VTYSH_RIPNGD,
	 vtysh_exit_ripngd,
	 vtysh_exit_ripngd_cmd,
	 "exit",
	 "Exit current mode and down to previous mode\n")
{
  return vtysh_exit (vty);
}
#if 0
ALIAS (vtysh_exit_ripngd,
       vtysh_quit_ripngd_cmd,
       "quit",
       "Exit current mode and down to previous mode\n")
#else
DEFUNSH (VTYSH_RIPNGD,
	 vtysh_quit_ripngd,
	vtysh_quit_ripngd_cmd,
	"quit",
	"Exit current mode and down to previous mode\n")
{
  return vtysh_exit (vty);
}

#endif

DEFUNSH (VTYSH_RMAP,
	 vtysh_exit_rmap,
	 vtysh_exit_rmap_cmd,
	 "exit",
	 "Exit current mode and down to previous mode\n")
{
  return vtysh_exit (vty);
}
#if 0
ALIAS (vtysh_exit_rmap,
       vtysh_quit_rmap_cmd,
       "quit",
       "Exit current mode and down to previous mode\n")

#else
DEFUNSH (VTYSH_RMAP,
	 vtysh_quit_rmap,
	vtysh_quit_rmap_cmd,
	"quit",
	"Exit current mode and down to previous mode\n")
{
  return vtysh_exit (vty);
}


#endif

DEFUNSH (VTYSH_BGPD,
	 vtysh_exit_bgpd,
	 vtysh_exit_bgpd_cmd,
	 "exit",
	 "Exit current mode and down to previous mode\n")
{
  return vtysh_exit (vty);
}
#if 0
ALIAS (vtysh_exit_bgpd,
       vtysh_quit_bgpd_cmd,
       "quit",
       "Exit current mode and down to previous mode\n")

#else
DEFUNSH (VTYSH_BGPD,
	 vtysh_quit_bgpd,
	vtysh_quit_bgpd_cmd,
	"quit",
	"Exit current mode and down to previous mode\n")
{
  return vtysh_exit (vty);
}

#endif

DEFUNSH (VTYSH_OSPFD,
	 vtysh_exit_ospfd,
	 vtysh_exit_ospfd_cmd,
	 "exit",
	 "Exit current mode and down to previous mode\n")
{
  return vtysh_exit (vty);
}
#if 0
ALIAS (vtysh_exit_ospfd,
       vtysh_quit_ospfd_cmd,
       "quit",
       "Exit current mode and down to previous mode\n")
#else
DEFUNSH (VTYSH_OSPFD,
	 vtysh_quit_ospfd,
	vtysh_quit_ospfd_cmd,
	"quit",
	"Exit current mode and down to previous mode\n")
{
  return vtysh_exit (vty);
}

#endif
DEFUNSH (VTYSH_OSPF6D,
	 vtysh_exit_ospf6d,
	 vtysh_exit_ospf6d_cmd,
	 "exit",
	 "Exit current mode and down to previous mode\n")
{
  return vtysh_exit (vty);
}
#if 0
ALIAS (vtysh_exit_ospf6d,
       vtysh_quit_ospf6d_cmd,
       "quit",
       "Exit current mode and down to previous mode\n")
#else
DEFUNSH (VTYSH_OSPF6D,
	 vtysh_quit_ospf6d,
	vtysh_quit_ospf6d_cmd,
	"quit",
	"Exit current mode and down to previous mode\n")
{
  return vtysh_exit (vty);
}

#endif
DEFUNSH (VTYSH_ISISD,
	 vtysh_exit_isisd,
	 vtysh_exit_isisd_cmd,
	 "exit",
	 "Exit current mode and down to previous mode\n")
{
  return vtysh_exit (vty);
}
#if 0
ALIAS (vtysh_exit_isisd,
       vtysh_quit_isisd_cmd,
       "quit",
       "Exit current mode and down to previous mode\n")
#else
DEFUNSH (VTYSH_ISISD,
	 vtysh_quit_isisd,
	vtysh_quit_isisd_cmd,
	"quit",
	"Exit current mode and down to previous mode\n")
{
  return vtysh_exit (vty);
}

#endif
DEFUNSH (VTYSH_ALL,
         vtysh_exit_line_vty,
         vtysh_exit_line_vty_cmd,
         "exit",
         "Exit current mode and down to previous mode\n")
{
  return vtysh_exit (vty);
}
#if 0
ALIAS (vtysh_exit_line_vty,
       vtysh_quit_line_vty_cmd,
       "quit",
       "Exit current mode and down to previous mode\n")

#else

DEFUNSH (VTYSH_ALL,
         vtysh_quit_line_vty,
	vtysh_quit_line_vty_cmd,
	"quit",
	"Exit current mode and down to previous mode\n")
{
  return vtysh_exit (vty);
}

#endif

/* TODO Implement interface description commands in ripngd, ospf6d
 * and isisd. */
DEFSH (VTYSH_ZEBRA|VTYSH_RIPD|VTYSH_OSPFD,
       interface_desc_cmd,
	   "description .LINE",
       "Interface specific description\n"
       "Characters describing this interface\n")
       
       
DEFSH (VTYSH_ZEBRA|VTYSH_RIPD|VTYSH_OSPFD,
       no_interface_desc_cmd,
       "no description",
       NO_STR
       "Interface specific description\n")

DEFSH (VTYSH_ZEBRA|VTYSH_RIPD|VTYSH_OSPFD,
	   interface_desc_local_cmd,
	   "local description .LINE",
	   "The description scope in local board\n"
	   "Interface specific description\n"
	   "Characters describing this interface\n")


DEFUNSH (VTYSH_INTERFACE,
	 vtysh_exit_interface,
	 vtysh_exit_interface_cmd,
	 "exit",
	 "Exit current mode and down to previous mode\n")
{
  return vtysh_exit (vty);
}
#if 0
ALIAS (vtysh_exit_interface,
       vtysh_quit_interface_cmd,
       "quit",
       "Exit current mode and down to previous mode\n")
#else
DEFUNSH (VTYSH_INTERFACE,
	 vtysh_quit_interface,
	 vtysh_quit_interface_cmd,
	 "quit",
	 "Exit current mode and down to previous mode\n")
{
  return vtysh_exit (vty);
}


#endif

#if 0
/* Memory */
DEFUN (vtysh_show_memory,
       vtysh_show_memory_cmd,
       "show memory",
       SHOW_STR
       "Memory statistics\n")
{
  unsigned int i;
  int ret = CMD_SUCCESS;
  char line[] = "show memory\n";
  
  for (i = 0; i < VTYSH_INDEX_MAX; i++)
    if ( vtysh_client[i].fd >= 0 )
      {
        fprintf (stdout, "Memory statistics for %s:\n", 
                 vtysh_client[i].name);
        ret = vtysh_client_execute (&vtysh_client[i], line, stdout);
        fprintf (stdout,"\n");
      }
  
  return ret;
}
#else
/* Memory */
DEFUN (vtysh_show_memory,
       vtysh_show_memory_cmd,
       "show memory",
       SHOW_STR
       "Memory statistics\n")
{
	int ret=system("free");
	return WEXITSTATUS(ret);
}

#endif
/* Logging commands. */
DEFUN (vtysh_show_logging,
       vtysh_show_logging_cmd,
       "show logging",
       SHOW_STR
       "Show current logging configuration\n")
{
  unsigned int i;
  int ret = CMD_SUCCESS;
  char line[] = "show logging\n";
  
  for (i = 0; i < VTYSH_INDEX_MAX; i++)
    if ( vtysh_client[i].fd >= 0 )
      {
        fprintf (stdout,"Logging configuration for %s:\n", 
                 vtysh_client[i].name);
        ret = vtysh_client_execute (&vtysh_client[i], line, stdout);
        fprintf (stdout,"\n");
      }
  
  return ret;
}

DEFUNSH (VTYSH_ALL,
	 vtysh_log_stdout,
	 vtysh_log_stdout_cmd,
	 "log stdout",
	 "Logging control\n"
	 "Set stdout logging level\n")
{
  return CMD_SUCCESS;
}

DEFUNSH (VTYSH_ALL,
	 vtysh_log_stdout_level,
	 vtysh_log_stdout_level_cmd,
	 "log stdout "LOG_LEVELS,
	 "Logging control\n"
	 "Set stdout logging level\n"
	 LOG_LEVEL_DESC)
{
  return CMD_SUCCESS;
}

DEFUNSH (VTYSH_ALL,
	 no_vtysh_log_stdout,
	 no_vtysh_log_stdout_cmd,
	 "no log stdout [LEVEL]",
	 NO_STR
	 "Logging control\n"
	 "Cancel logging to stdout\n"
	 "Logging level\n")
{
  return CMD_SUCCESS;
}

DEFUNSH (VTYSH_ALL,
	 vtysh_log_file,
	 vtysh_log_file_cmd,
	 "log file FILENAME",
	 "Logging control\n"
	 "Logging to file\n"
	 "Logging filename\n")
{
  return CMD_SUCCESS;
}

DEFUNSH (VTYSH_ALL,
	 vtysh_log_file_level,
	 vtysh_log_file_level_cmd,
	 "log file FILENAME "LOG_LEVELS,
	 "Logging control\n"
	 "Logging to file\n"
	 "Logging filename\n"
	 LOG_LEVEL_DESC)
{
  return CMD_SUCCESS;
}

DEFUNSH (VTYSH_ALL,
	 no_vtysh_log_file,
	 no_vtysh_log_file_cmd,
	 "no log file [FILENAME]",
	 NO_STR
	 "Logging control\n"
	 "Cancel logging to file\n"
	 "Logging file name\n")
{
  return CMD_SUCCESS;
}

ALIAS_SH (VTYSH_ALL,
	  no_vtysh_log_file,
	  no_vtysh_log_file_level_cmd,
	  "no log file FILENAME LEVEL",
	  NO_STR
	  "Logging control\n"
	  "Cancel logging to file\n"
	  "Logging file name\n"
	  "Logging level\n")

DEFUNSH (VTYSH_ALL,
	 vtysh_log_monitor,
	 vtysh_log_monitor_cmd,
	 "log monitor",
	 "Logging control\n"
	 "Set terminal line (monitor) logging level\n")
{
  return CMD_SUCCESS;
}

DEFUNSH (VTYSH_ALL,
	 vtysh_log_monitor_level,
	 vtysh_log_monitor_level_cmd,
	 "log monitor "LOG_LEVELS,
	 "Logging control\n"
	 "Set terminal line (monitor) logging level\n"
	 LOG_LEVEL_DESC)
{
  return CMD_SUCCESS;
}

DEFUNSH (VTYSH_ALL,
	 no_vtysh_log_monitor,
	 no_vtysh_log_monitor_cmd,
	 "no log monitor [LEVEL]",
	 NO_STR
	 "Logging control\n"
	 "Disable terminal line (monitor) logging\n"
	 "Logging level\n")
{
  return CMD_SUCCESS;
}

DEFUNSH (VTYSH_ALL,
	 vtysh_log_syslog,
	 vtysh_log_syslog_cmd,
	 "log syslog",
	 "Logging control\n"
	 "Set syslog logging level\n")
{
  return CMD_SUCCESS;
}

DEFUNSH (VTYSH_ALL,
	 vtysh_log_syslog_level,
	 vtysh_log_syslog_level_cmd,
	 "log syslog "LOG_LEVELS,
	 "Logging control\n"
	 "Set syslog logging level\n"
	 LOG_LEVEL_DESC)
{
  return CMD_SUCCESS;
}

DEFUNSH (VTYSH_ALL,
	 no_vtysh_log_syslog,
	 no_vtysh_log_syslog_cmd,
	 "no log syslog [LEVEL]",
	 NO_STR
	 "Logging control\n"
	 "Cancel logging to syslog\n"
	 "Logging level\n")
{
  return CMD_SUCCESS;
}

DEFUNSH (VTYSH_ALL,
	 vtysh_log_facility,
	 vtysh_log_facility_cmd,
	 "log facility "LOG_FACILITIES,
	 "Logging control\n"
	 "Facility parameter for syslog messages\n"
	 LOG_FACILITY_DESC)

{
  return CMD_SUCCESS;
}

DEFUNSH (VTYSH_ALL,
	 no_vtysh_log_facility,
	 no_vtysh_log_facility_cmd,
	 "no log facility [FACILITY]",
	 NO_STR
	 "Logging control\n"
	 "Reset syslog facility to default (daemon)\n"
	 "Syslog facility\n")

{
  return CMD_SUCCESS;
}

DEFUNSH_DEPRECATED (VTYSH_ALL,
		    vtysh_log_trap,
		    vtysh_log_trap_cmd,
		    "log trap "LOG_LEVELS,
		    "Logging control\n"
		    "(Deprecated) Set logging level and default for all destinations\n"
		    LOG_LEVEL_DESC)

{
  return CMD_SUCCESS;
}

DEFUNSH_DEPRECATED (VTYSH_ALL,
		    no_vtysh_log_trap,
		    no_vtysh_log_trap_cmd,
		    "no log trap [LEVEL]",
		    NO_STR
		    "Logging control\n"
		    "Permit all logging information\n"
		    "Logging level\n")
{
  return CMD_SUCCESS;
}

DEFUNSH (VTYSH_ALL,
	 vtysh_log_record_priority,
	 vtysh_log_record_priority_cmd,
	 "log record-priority",
	 "Logging control\n"
	 "Log the priority of the message within the message\n")
{
  return CMD_SUCCESS;
}

DEFUNSH (VTYSH_ALL,
	 no_vtysh_log_record_priority,
	 no_vtysh_log_record_priority_cmd,
	 "no log record-priority",
	 NO_STR
	 "Logging control\n"
	 "Do not log the priority of the message within the message\n")
{
  return CMD_SUCCESS;
}

DEFUNSH (VTYSH_ALL,
	 vtysh_service_password_encrypt,
	 vtysh_service_password_encrypt_cmd,
	 "service password-encryption",
	 "Set up miscellaneous service\n"
	 "Enable encrypted passwords\n")
{
  return CMD_SUCCESS;
}

DEFUNSH (VTYSH_ALL,
	 no_vtysh_service_password_encrypt,
	 no_vtysh_service_password_encrypt_cmd,
	 "no service password-encryption",
	 NO_STR
	 "Set up miscellaneous service\n"
	 "Enable encrypted passwords\n")
{
  return CMD_SUCCESS;
}

DEFUNSH (VTYSH_ALL,
	 vtysh_config_password,
	 vtysh_password_cmd,
	 "password (8|) WORD",
	 "Assign the terminal connection password\n"
	 "Specifies a HIDDEN password will follow\n"
	 "dummy string \n"
	 "The HIDDEN line password string\n")
{
  return CMD_SUCCESS;
}

DEFUNSH (VTYSH_ALL,
	 vtysh_password_text,
	 vtysh_password_text_cmd,
	 "password LINE",
	 "Assign the terminal connection password\n"
	 "The UNENCRYPTED (cleartext) line password\n")
{
  return CMD_SUCCESS;
}

DEFUNSH (VTYSH_ALL,
	 vtysh_config_enable_password,
	 vtysh_enable_password_cmd,
	 "enable password (8|) WORD",
	 "Modify enable password parameters\n"
	 "Assign the privileged level password\n"
	 "Specifies a HIDDEN password will follow\n"
	 "dummy string \n"
	 "The HIDDEN 'enable' password string\n")
{
  return CMD_SUCCESS;
}

DEFUNSH (VTYSH_ALL,
	 vtysh_enable_password_text,
	 vtysh_enable_password_text_cmd,
	 "enable password LINE",
	 "Modify enable password parameters\n"
	 "Assign the privileged level password\n"
	 "The UNENCRYPTED (cleartext) 'enable' password\n")
{
  return CMD_SUCCESS;
}

DEFUNSH (VTYSH_ALL,
	 no_vtysh_config_enable_password,
	 no_vtysh_enable_password_cmd,
	 "no enable password",
	 NO_STR
	 "Modify enable password parameters\n"
	 "Assign the privileged level password\n")
{
  return CMD_SUCCESS;
}

DEFUN (vtysh_write_terminal,
       vtysh_write_terminal_cmd,
       "write terminal",
       "Write running configuration to memory, network, or terminal\n"
       "Write to terminal\n")
{
	u_int i;
	int ret;
	char line[] = "write terminal\n";
	FILE *fp = NULL;
  struct cmd_node *node;
	vector cmdvec = get_cmdvec();

	
	if(vty->fp)
		fp = vty->fp;
	else if (vtysh_pager_name)
	{
	  fp = popen (vtysh_pager_name, "w");
	  if (fp == NULL)
	{
	  perror ("popen");
	  exit (1);
	}
	}
	else
	fp = stdout;

	vty_out (vty, "Building configuration...%s", VTY_NEWLINE);
	vty_out (vty, "%sCurrent configuration:%s", VTY_NEWLINE,
	   VTY_NEWLINE);


	for (i = 0; i < HIDDENDEBUG_NODE; i++){
		if ((node = vector_slot (cmdvec, i)) && node->func )
			{
				if ((*node->func) (vty))
				{
					vty_out(vty,"Module %s[%d] error \n",node->node_name,node->node);
					return CMD_WARNING;
				}
			}
		}
		vtysh_config_dump (fp);
		
	for (i = 0; i < VTYSH_INDEX_MAX; i++)
	
		ret = vtysh_client_config (&vtysh_client[i], line);
	vtysh_config_dump (fp);
	
#ifdef _D_WCPSS_	
	for (i = HIDDENDEBUG_NODE; i < EBR_NODE1; i++)
		if ((node = vector_slot (cmdvec, i)) && node->func )
		{
			if ((*node->func) (vty))
			{
				vty_out(vty,"Module %s[%d] error \n",node->node_name,node->node);
				return CMD_WARNING;
			}
		
		
		}
		
		vtysh_config_dump (fp);

	for (i = EBR_NODE1; i < vector_active(cmdvec); i++)
		if ((node = vector_slot (cmdvec, i)) && node->func )
			{
				if ((*node->func) (vty))
				{
					vty_out(vty,"Module %s[%d] error \n",node->node_name,node->node);
					return CMD_WARNING;
				}
			}
		vtysh_config_dump (fp);

		
		
#endif
	
	ret = vtysh_client_config (&(vtysh_client[VTYSH_INDEX_ZEBRA]), "write terminal iproute");
	vtysh_config_dump (fp);


	if (!vty->fp && vtysh_pager_name && fp)
	{
	  fflush (fp);
	  if (pclose (fp) == -1)
		{
		  perror ("pclose");
		  exit (1);
		}
	  fp = NULL;
	}
	return CMD_SUCCESS;
	}
DEFUN (vtysh_write_terminal1,
       vtysh_write_terminal_cmd1,
       "show running-config (access-list|acl|arp|dhcpsnooping|dldp|ebr|eth|traffic-policer|fdb|igmpsnooping|interface|iproute|mirror|dhcp|dhcrelay|ospf|prefix-list|pvlan|qos|rip|stp|system|trunk|vlan|vrrp|wlan|wlansecurity|wtp|routepolicy|localinst)",
			 SHOW_STR
			 "Current operating configuration\n"
			 "Access list module\n"
			 "ACL module\n"
			 "Arp module\n"
			 "Dhcp snooping module\n"
			 "Dldp module\n"
			 "Ethernet bridge module\n"
			 "Ethernet port module\n"
			 "Traffic-policer module\n"
			 "Fdb module\n"
			 "Igmp snooping module\n"
			 "Interface module\n"
			 "IP route module\n"
			 "Mirror module\n"
			 "Ospf module\n"
			 "Prefix-list module\n"
			 "Pvlan module\n"
			 "Qos module\n"
			 "Rip module\n"
			 "Stp module\n"
			 "System module\n"
			 "Trunk module\n"
			 "Vlan module\n"
			 "Vrrp module\n"
			 "Wlan module\n"
			 "Wlan security module\n"
			 "Wtp module\n"
			 "Route policy module\n")

{
	u_int i,cmdnode;
	int ret,need_client=0,clent_node=0;
	char line[] = "write terminal\n";
	char cmd_clente_line[512];
	FILE *fp = NULL;
  struct cmd_node *node;
	vector cmdvec = get_cmdvec();

	
	if(vty->fp)
		fp = vty->fp;
	else if (vtysh_pager_name)
	{
	  fp = popen (vtysh_pager_name, "w");
	  if (fp == NULL)
	{
	  perror ("popen");
	  exit (1);
	}
	}
	else
	fp = stdout;


	vty_out (vty, "Building configuration...%s", VTY_NEWLINE);
	vty_out (vty, "%sCurrent configuration:%s", VTY_NEWLINE,
	   VTY_NEWLINE);
	if(argc==1)
	{
		if(!strncmp(argv[0],"acl",strlen(argv[0]))){
			cmdnode= ACL_GROUP_NODE;
		}
		else if(!strncmp(argv[0],"access-list",strlen(argv[0])))
		{
			cmdnode= ACL_GROUP_NODE;
			need_client=1;
			clent_node = VTYSH_INDEX_ZEBRA;
		}
		else if(!strncmp(argv[0],"arp",strlen(argv[0])))
			cmdnode= STATIC_ARP_NODE;
/*
		else if(!strncmp(argv[0],"bss",strlen(argv0)))
			cmdnode= ACL_GROUP_NODE;
*/
		else if(!strncmp(argv[0],"dhcp",strlen(argv[0])))
			cmdnode= POOL_NODE;
		else if(!strncmp(argv[0],"dhcrelay",strlen(argv[0])))
			cmdnode= DHCRELAY_NODE;
		else if(!strncmp(argv[0],"dhcpsnooping",strlen(argv[0])))
			cmdnode= DHCP_SNP_NODE;
		else if(!strncmp(argv[0],"dldp",strlen(argv[0])))
			cmdnode= DLDP_NODE;
#ifdef _D_WCPSS_
		else if(!strncmp(argv[0],"ebr",strlen(argv[0])))
			cmdnode= EBR_NODE;
#endif
		else if(!strncmp(argv[0],"eth",strlen(argv[0])))
			cmdnode= ETH_PORT_NODE;
		else if(!strncmp(argv[0],"traffic-policer",strlen(argv[0])))
			cmdnode= TRAFFIC_POLICER_NODE;
		else if(!strncmp(argv[0],"fdb",strlen(argv[0])))
			cmdnode= FDB_NODE;
		else if(!strncmp(argv[0],"igmpsnooping",strlen(argv[0])))
			cmdnode= IGMP_SNP_NODE;
		else if(!strncmp(argv[0],"mirror",strlen(argv[0])))
			cmdnode= MIRROR_NODE;
		else if(!strncmp(argv[0],"interface",strlen(argv[0]))){
			cmdnode= INTERFACE_NODE;
			need_client=1;
			clent_node = VTYSH_INDEX_ZEBRA;
		}
		else if(!strncmp(argv[0],"ospf",strlen(argv[0]))){
			cmdnode= OSPF_NODE;
			need_client=1;
			clent_node = VTYSH_INDEX_OSPFD;
		}
		else if(!strncmp(argv[0],"prefix-list",strlen(argv[0]))){
			cmdnode= PREFIX_NODE;
			need_client =1 ;
			clent_node = VTYSH_INDEX_ZEBRA;
			}
		else if(!strncmp(argv[0],"pvlan",strlen(argv[0])))
			cmdnode= PVLAN_NODE;
		else if(!strncmp(argv[0],"qos",strlen(argv[0])))
			cmdnode= QOS_NODE;
		else if(!strncmp(argv[0],"rip",strlen(argv[0]))){
			cmdnode= ACL_GROUP_NODE;
			need_client=1;
			clent_node = VTYSH_INDEX_RIPD;
		}
		else if(!strncmp(argv[0],"iproute",strlen(argv[0])))
		{
			cmdnode= IP_NODE;
			need_client=1;
			clent_node = VTYSH_INDEX_ZEBRA;
		}
		else if(!strncmp(argv[0],"stp",strlen(argv[0])))
			cmdnode= STP_NODE;
		else if(!strncmp(argv[0],"system",strlen(argv[0])))
			cmdnode= SYS_MNG_NODE;
		else if(!strncmp(argv[0],"trunk",strlen(argv[0])))
			cmdnode= TRUNK_NODE;
		else if(!strncmp(argv[0],"vlan",strlen(argv[0])))
			cmdnode= VLAN_NODE;
		
		else if(!strncmp(argv[0],"vrrp",strlen(argv[0])))
			cmdnode= HANSI_NODE;
		else if(!strncmp(argv[0],"localinst",strlen(argv[0])))
			cmdnode= LOCAL_HANSI_NODE;
		#ifdef _D_WCPSS_
		else if(!strncmp(argv[0],"wlan",strlen(argv[0])))
			cmdnode= WLAN_NODE;
		else if(!strncmp(argv[0],"wlansecurity",strlen(argv[0])))
			cmdnode= SECURITY_NODE;
		else if(!strncmp(argv[0],"wtp",strlen(argv[0])))
			cmdnode= WTP_NODE;
		else if(!strncmp(argv[0],"routepolicy",strlen(argv[0])))
			cmdnode= ROUTE_POLICY_NODE;
		#endif
		else
		{
			vty_out(vty,"The module %s is error\n",argv[0]);
			return CMD_WARNING;
		}
		
		if(need_client)
		{
			sprintf(cmd_clente_line,"write terminal %s",argv[0]);
			ret = vtysh_client_config (&vtysh_client[clent_node], cmd_clente_line);
		}
		else
		{
			for (i = 0; i < vector_active (cmdvec); i++){
				if ((node = vector_slot (cmdvec, i)) &&node->node == cmdnode && node->func )
				{
					if ((*node->func) (vty)){
					}
				}		
			}
		}
		vtysh_config_dump (fp);
		return CMD_SUCCESS;
	}
	else
	{
		vtysh_config_write (vty);
#if 0
		
				vtysh_show_running_config();
			vtysh_show_running_config();
#else
		
		for (i = 0; i < vector_active (cmdvec); i++)
			if ((node = vector_slot (cmdvec, i)) && node->func )
				{
					if ((*node->func) (vty)){
						vty_out (vty, "!%s", VTY_NEWLINE);
					vtysh_config_dump (fp);
					}
				}
		
		
#endif
			for (i = 0; i < VTYSH_INDEX_MAX; i++)
			ret = vtysh_client_config (&vtysh_client[i], line);
			vtysh_config_dump (fp);
			
#if 0
			/*
			vtysh_static_arp_show_running_config();
			vtysh_config_dump(fp);
		*/
		
			vtysh_wcpss_show_running_config(); 
				vtysh_client_config (&vtysh_client[0], "show interface wireless_config"); 	
			vtysh_config_dump (fp); 
			vtysh_wcpss_show_running_config_end();	
			vtysh_config_dump (fp); 
#endif 
		
		
			if (!vty->fp && vtysh_pager_name && fp)
			{
				fflush (fp);
				if (pclose (fp) == -1)
			{
				perror ("pclose");
				exit (1);
			}
				fp = NULL;
			}
			return CMD_SUCCESS;


	}

}

DEFUN (vtysh_integrated_config,
       vtysh_integrated_config_cmd,
       "service integrated-vtysh-config",
       "Set up miscellaneous service\n"
       "Write configuration into integrated file\n")
{
  vtysh_writeconfig_integrated = 1;
  return CMD_SUCCESS;
}

DEFUN (test_config,
       test_cmd,
       "test file",
       NO_STR
       "Write configuration into integrated file\n")
{
	FILE *fp1,*fp2;
	int len;
	fp1 = fopen("/mnt/test","wx");
	if(!fp1)
	{
		vty_out(vty,"fopen fp1 error %s\n",safe_strerror(errno));
		return 1;
	}
	len = fprintf (fp1, "12345678fp1\n");
	
	if(len<=0)
	{
		vty_out(vty,"fprintf error line %d %s\n",__LINE__,safe_strerror(errno));
	}
	fp2 = fopen("/mnt/test","wx");
	
	if(!fp2)
	{
		vty_out(vty,"fopen fp2 error %s\n",safe_strerror(errno));
		fclose(fp1);
		return 1;
	}
	
	len = fprintf (fp2, "12345678fp2\n");
	
	if(len<=0)
	{
		vty_out(vty,"fprintf error line %d %s\n",__LINE__,safe_strerror(errno));
	}
	len = fprintf (fp1, "987654321fp1\n");
	if(len<=0)
	{
		vty_out(vty,"fprintf error line %d %s\n",__LINE__,safe_strerror(errno));

	}

	len = fprintf (fp2, "987654321fp2\n");
	if(len<=0)
	{
		vty_out(vty,"fprintf error line %d %s\n",__LINE__,safe_strerror(errno));

	}
	
	len = fprintf (fp1, "11111111111111fp1\n");
	if(len<=0)
	{
		vty_out(vty,"fprintf error line %d %s\n",__LINE__,safe_strerror(errno));

	}
	fclose(fp1);
	fclose(fp2);
  return CMD_SUCCESS;
}


DEFUN (no_vtysh_integrated_config,
       no_vtysh_integrated_config_cmd,
       "no service integrated-vtysh-config",
       NO_STR
       "Set up miscellaneous service\n"
       "Write configuration into integrated file\n")
{
  vtysh_writeconfig_integrated = 0;
  return CMD_SUCCESS;
}
static int
write_config_integrated(void)
{
  u_int i;
  int ret;
  char line[] = "write terminal\n";
  FILE *fp;
  char *integrate_sav = NULL;
  int fd;
  struct vty *file_vty;
  struct cmd_node *node;
  char *config_file_tmp = NULL;
	vector cmdvec = get_cmdvec();

  integrate_sav = malloc (strlen (integrate_default) +
			  strlen (CONF_BACKUP_EXT) + 1);
  strcpy (integrate_sav, integrate_default);
  strcat (integrate_sav, CONF_BACKUP_EXT);

  fprintf (stdout,"Building Configuration...\n");

  fp = fopen (integrate_sav, "w");
  
  if (fp == NULL)
    {
    	if(errno == EEXIST)
    	{
			fprintf (stdout,"%% Another user is saving configuration.\n Please wait a few minutes and save again.\n");
			return CMD_WARNING;

		}
      fprintf (stdout,"%% Can't open configuration file %s.\n",
	       integrate_default);
      return CMD_WARNING;
    }
#if 0	
  vtysh_config_write(vty);
  vtysh_show_running_config();

  for (i = 0; i < VTYSH_INDEX_MAX; i++)
    ret = vtysh_client_config (&vtysh_client[i], line);
  vtysh_config_dump (fp);  
  vtysh_static_arp_show_running_config();
  vtysh_config_dump(fp);
#ifdef _D_WCPSS_
	  vtysh_wcpss_show_running_config(); 
	  vtysh_client_config (&vtysh_client[0], "show interface wireless_config"); 
	  vtysh_config_dump (fp);  	  
	  vtysh_wcpss_show_running_config_end(); 	  
	  vtysh_config_dump (fp);  	  
#endif 
#else
for (i = 0; i < HIDDENDEBUG_NODE; i++)
	if ((node = vector_slot (cmdvec, i)) && node->func )
		{
			if ((*node->func) (vty))
			{
				vty_out(vty,"Module %s[%d] error \n",node->node_name,node->node);
				fclose (fp);
				unlink(integrate_sav);
				return CMD_WARNING;
				
			}
		}
	
	vtysh_config_dump (fp);
	
for (i = 0; i < VTYSH_INDEX_MAX; i++)
{
	ret = vtysh_client_config (&vtysh_client[i], line);
	if(ret == CMD_SUCCESS)
		vtysh_config_dump (fp);
	else
	{
		vty_out(vty,"Module rtmd error \n");
		fclose (fp);
		unlink(integrate_sav);
		return CMD_WARNING;

	}

}

#ifdef _D_WCPSS_
for (i = HIDDENDEBUG_NODE; i < EBR_NODE1; i++)
	if ((node = vector_slot (cmdvec, i)) && node->func )
	{
		if ((*node->func) (vty))
		{
			vty_out(vty,"Module %s[%d] error \n",node->node_name,node->node);
			fclose (fp);
			unlink(integrate_sav);
			return CMD_WARNING;
		}
	
	
	}
	vtysh_config_dump (fp);
	for (i = EBR_NODE1; i < vector_active(cmdvec); i++)
		if ((node = vector_slot (cmdvec, i)) && node->func )
		{
			if ((*node->func) (vty))
			{
				vty_out(vty,"Module %s[%d] error \n",node->node_name,node->node);
				fclose (fp);
				unlink(integrate_sav);
				return CMD_WARNING;
			}
		
		
		}

	vtysh_config_dump (fp);
#endif
	ret = vtysh_client_config (&(vtysh_client[VTYSH_INDEX_ZEBRA]), "write terminal iproute");
	if(ret != CMD_SUCCESS)
	{
		fclose (fp);
		return CMD_WARNING;

	}
	vtysh_config_dump (fp);
#endif

  fclose (fp);
  
  /* Move current configuration file to backup config file. */
  unlink (integrate_default);
  if(rename ( integrate_sav,integrate_default)<0)
  {
  	free (integrate_sav);
  	fprintf(stdout,"Update config file error(%d),please check it\n",errno);
  	return CMD_WARNING;
  }
  free (integrate_sav);
#if 0  
  /*
	config_file_tmp = XMALLOC (MTYPE_TMP, strlen (integrate_default) + 8);
	sprintf (config_file_tmp, "%s.XXXXXX", integrate_default);
  */ 
	/* Open file to configuration write. */
  
	fd = open (integrate_default,O_APPEND|O_RDWR);
	if (fd < 0)
	  {
		vty_out (vty, "Can't open configuration file %s.%s", integrate_default,
			 VTY_NEWLINE);
		return CMD_SUCCESS;
  
	  }
  
  ret = lseek(fd,0,SEEK_END);
  if(ret<0)
  {
      fprintf (stdout,"%% Can't seek configuration file %s.\n",
	       integrate_default);
      return CMD_SUCCESS;

  }
	/* Make vty for configuration file. */
	file_vty = vty_new ();
	file_vty->fd = fd;
	file_vty->type = VTY_FILE;
  
	/* Config file header print. */
	vty_out (file_vty, "!\n! \n!	 ");
	vty_time_print (file_vty, 1);
	vty_out (file_vty, "!\n");
  
	for (i = 0; i < vector_active (cmdvec); i++)
	  if ((node = vector_slot (cmdvec, i)) && node->func)
		{
	  if ((*node->func) (file_vty))
		vty_out (file_vty, "!\n");
		}
	vty_close (file_vty);
  
#endif
  if (chmod (integrate_default, CONFIGFILE_MASK) != 0)
    {
    	fprintf(stdout,"Save config file error(%d),please check it\n",errno);
      syslog(LOG_ERR,"%% Can't chmod configuration file %s: %s (%d)\n", 
	integrate_default, safe_strerror(errno), errno);
      return CMD_WARNING;
    }
  fprintf (stdout,"[OK]\n");

  return CMD_SUCCESS;
}
extern void (*dcli_sync_file)(const char* file, int syn_to_blk);
int is_WriteConfig=0;

DEFUN (vtysh_write_memory,
       vtysh_write_memory_cmd,
       "write memory",
       "Write running configuration to memory, network, or terminal\n"
       "Write configuration to the file (same as write file)\n")
{
  int ret = CMD_SUCCESS;
  char line[] = "write memory\n";
  u_int i;
  char sor_cmd[256];
  #if 0
  char *configfile_name=NULL;
  #endif
 /* char *temp=argv[0];*/
  char *temp=(char *)argv[0];/*gujd: 2012-2-23, pm 5:20 . In order to decrease the warning when make img . */
 
	char file_path[128];
	char file_name[64];
	time_t timep;
	struct tm *p;

	time(&timep);
	p=gmtime(&timep);/*CID 14559 (#1 of 1): returned_pointer: Pointer "p" returned by "gmtime(&timep)" is never used*/
	sprintf(file_name,"Temp_config.conf");
	sprintf(file_path,"/mnt/%s",file_name);
	unlink(file_name);
	
	  config_fd=fopen(file_path,"w"); 
	  if(NULL == config_fd)
	  {
		  vty_out(vty,"system error[21]\n");
//		  return CMD_WARNING;
	  }

  
  if(1==argc)
  {
  	if(strlen(argv[0])>VTYSH_MAX_FILE_LEN)
  	{
  			vty_out(vty,VTYSH_CONFIG_NAME_LONG_STR,VTYSH_MAX_FILE_LEN);
			return CMD_WARNING;
				
  	}
	if(!((temp=strstr(temp,".conf"))&&!(*(temp+5))))//if temp does not has ".conf" and ".conf" in the end then..
	{
		vty_out(vty,VTYSH_CONFIG_NAME_WITHOUT_CONF_STR);
		return CMD_WARNING;
	}
  }
  ret=system("cp /mnt/conf_xml.conf /mnt/conf_xml.conf.bak > /dev/null 2> /dev/null\n");
  if(-1==ret)
  	return CMD_WARNING;
/* If integrated Quagga.conf explicitely set. */
  if (vtysh_writeconfig_integrated)
  {
  		is_WriteConfig = 1;
		ret = write_config_integrated();
		is_WriteConfig = 0;
		if(CMD_SUCCESS  == ret )
		{
			ret = system("srvsave.sh");
			ret = WEXITSTATUS(ret);
			if(argc)
			{	
			

			#if VTYSH_WRITE_WITHOUT_CONF_EXECUTE
				if((temp=strstr(temp,".conf"))&&!(*(temp+5)))//if temp has ".conf" and ".conf" in the end then..
						{
							configfile_name = (char*) malloc (strlen(argv[0])+1);
						strcpy (configfile_name,argv[0]);
					}else{
						configfile_name = (char*) malloc (strlen(argv[0])+strlen(".conf")+1);
						strcpy (configfile_name,argv[0]);
						strcat (configfile_name,".conf");
						vty_out(vty,"The config file name does not end with \".conf\",save to \"%s\"\n",configfile_name);
						}
			#endif
				char cmd[256];
				memset(cmd,0,256);
			#if VTYSH_WRITE_WITHOUT_CONF_EXECUTE
				sprintf(cmd,"cd /mnt;cp conf_xml.conf ./%s;sor.sh cp %s 100\n",configfile_name,configfile_name);
			#else
				sprintf(cmd,"cd /mnt;cp conf_xml.conf ./%s;sor.sh cp %s 100\n",argv[0],argv[0]);
			#endif
				ret = system (cmd);
				ret = WEXITSTATUS(ret);
				vty_out(vty,"DONE(%d)\n",ret);
			#if VTYSH_WRITE_WITHOUT_CONF_EXECUTE
				free(configfile_name);
			#endif
			}
		}else{
			vty_out(vty,"System can't write configure\n");
			}
	
	if(0!=ret)
	{
		
		return CMD_WARNING;
	}
	
#if 0
	sprintf(sor_cmd,"cd /mnt;sor.sh cp rtsuit/ 100\n");
	ret = system (sor_cmd);
	ret = WEXITSTATUS(ret);
#else
	/*sor.sh not support rtsuit/*.conf	*/
	/* gujd:2013-03-22, am, 11:06. Change for not copy rtmd.conf~ , ospf.conf~ and so on to SD card .Only copy xxxx.conf */
	memset(sor_cmd,0,256);
	sprintf(sor_cmd,"cd /mnt;sor.sh cp rtsuit/rtmd.conf 30\n");
	ret = system (sor_cmd);
	ret = WEXITSTATUS(ret);
	if(ret!=0)
	{
		syslog(LOG_ERR,"cp /mnt/rtsuit/rtmd.conf failed !\n");
	}
	
	#if 0 /*At present , no rip*/
	memset(sor_cmd,0,256);
	sprintf(sor_cmd,"cd /mnt;sor.sh cp rtsuit/ripd.conf 30\n");
	ret = system (sor_cmd);
	ret = WEXITSTATUS(ret);
	if(ret!=0)
	{
		syslog(LOG_ERR,"cp /mnt/rtsuit/ripd.conf failed !\n");
	}
	#endif
	
	memset(sor_cmd,0,256);
	sprintf(sor_cmd,"cd /mnt;sor.sh cp rtsuit/ospfd.conf 30\n");
	ret = system (sor_cmd);
	ret = WEXITSTATUS(ret);
	if(ret!=0)
	{
		syslog(LOG_ERR,"cp /mnt/rtsuit/ospfd.conf failed !\n");
	}
#endif

	if(dcli_sync_file != NULL)
	{

		(*dcli_sync_file)("/mnt/conf_xml.conf",1);
		if(1 == argc)
		{
			char temp[64];
			memset(temp,0,64);
			sprintf(temp,"/mnt/%s",argv[0]);
			(*dcli_sync_file)((char*)argv[0],1);
			
		}
		
	}else{
		syslog(LOG_ERR,"dcli_sync_file is NULL\n");
	}


	/*write config file*/	
	
	fclose(config_fd);
	config_fd=-1;
	char cmd2[256]; 
	memset(cmd2,0,256); 
	sprintf(cmd2,"cd /mnt;sor.sh cp %s 200 > /dev/null 2> /dev/null",file_name);

	ret=system(cmd2);
	
	if(-1==ret) 
	{
	vty_out(vty,"system error[20]\n");
/*	return CMD_WARNING;	*/
		}
	

	/*write config file*/

	char cmd1[256];
	memset(cmd1,0,256);
/*sprintf(cmd1,"sudo mount /blk;sudo cp -r /var/run/config /blk;sudo umount /blk;");*/
	sprintf(cmd1,"cp -r /var/run/config /mnt > /dev/null 2> /dev/null; cd /mnt;sor.sh cp config 200 > /dev/null 2> /dev/null");
	ret=system(cmd1);
	if(-1==ret){
		return CMD_WARNING;

	}

	return ret;
  }
return CMD_SUCCESS;
}
DEFUN (vtysh_show_bootconfig,
	vtysh_show_bootconfig_cmd,
	"show bootconfig [BOOTCONFIGNAME]",
	SHOW_STR
	"Show the config file\n"
	"Show the Contents of the config file\n")
{
	int ret = CMD_SUCCESS;
	char cmd[64];
	memset(cmd,0,64);
	if(1==argc)
  	{
  		/*char *temp=argv[0];*/
  		char *temp=(char *)argv[0];/*gujd: 2012-2-23, pm 5:20 . In order to decrease the warning when make img . */
  		if(strlen(argv[0])>VTYSH_MAX_FILE_LEN)
  		{
  			vty_out(vty,VTYSH_CONFIG_NAME_LONG_STR,VTYSH_MAX_FILE_LEN);
			return CMD_WARNING;
				
  		}
		if(!((temp=strstr(temp,".conf"))&&!(*(temp+5))))//if temp does not has ".conf" and ".conf" in the end then..
		{
			vty_out(vty,VTYSH_CONFIG_NAME_WITHOUT_CONF_STR);
			return CMD_WARNING;
		}
  	}
	if(!argc)
		{
			ret = system("cd /mnt;ls *.conf|grep -v '^cli.conf$'|grep -v '^conf_xml.conf$'\n");
			ret = WEXITSTATUS(ret);
		}else{
			sprintf(cmd,"/mnt/%s",argv[0]);
			if(0!=access(cmd,F_OK)||0==strcmp(argv[0],"cli.conf"))
			{
				vty_out(vty,"WARNING: FILE NAME ERROR\n");
				return CMD_WARNING;
			}
			sprintf(cmd,"more /mnt/%s\n",argv[0]);
			ret = system(cmd);
			ret = WEXITSTATUS(ret);
			}
	if(0!=ret)
		return CMD_WARNING;
	return ret;
}

DEFUN (vtysh_del_bootconfig,
	vtysh_del_bootconfig_cmd,
	"delete bootconfig BOOTCONFIGNAME",
	"Delete the config file\n"
	"Delete the bootconfig file\n"
	"Delete the config file of BOOTCONFIGNAME\n"
	)
{
	int ret = CMD_SUCCESS;
	/*char* temp= argv[0];*/
	char* temp= (char *)argv[0];/*gujd: 2012-2-23, pm 5:20 . In order to decrease the warning when make img . */
	char cmd[128];
	memset(cmd,0,128);
	if(strlen(argv[0])>VTYSH_MAX_FILE_LEN)
	{
			vty_out(vty,VTYSH_CONFIG_NAME_LONG_STR,VTYSH_MAX_FILE_LEN);
			return CMD_WARNING;
		
	}
	if(!((temp=strstr(temp,".conf"))&&!(*(temp+5))))//if temp does not has ".conf" and ".conf" in the end then..
	{
		vty_out(vty,VTYSH_CONFIG_NAME_WITHOUT_CONF_STR);
		return CMD_WARNING;
	}
	sprintf(cmd,"/mnt/%s",argv[0]);
	if(0!=access(cmd,F_OK)||0==strcmp(argv[0],"conf_xml.conf")||0==strcmp(argv[0],"cli.conf"))
	{
		vty_out(vty,"WARNING: FILE NAME ERROR\n");
		return ret;
	}
	sprintf(cmd,"/usr/bin/sor.sh rm %s 10;rm /mnt/%s > /dev/null\n",argv[0],argv[0]);
	ret = system (cmd);
	ret = WEXITSTATUS(ret);
	if(0!=ret)
		return CMD_WARNING;
	return ret;
}

DEFUN (vtysh_set_bootconfig,
	vtysh_set_bootconfig_cmd,
	"set bootconfig BOOTCONFIGNAME",
	"Set system configuration\n"
	"Set bootconfig file\n"
	"Config file name\n"
	)
{
	int ret = CMD_SUCCESS;
	/*char* temp= argv[0];*/
	char* temp= (char *)argv[0];/*gujd: 2012-2-23, pm 5:20 . In order to decrease the warning when make img . */
	char cmd[512];
	memset(cmd,0,512);
	if(strlen(argv[0])>VTYSH_MAX_FILE_LEN)
	{
			vty_out(vty,VTYSH_CONFIG_NAME_LONG_STR,VTYSH_MAX_FILE_LEN);
			return CMD_WARNING;
		
	}
	if(!((temp=strstr(temp,".conf"))&&!(*(temp+5))))//if temp does not has ".conf" and ".conf" in the end then..
	{
		vty_out(vty,VTYSH_CONFIG_NAME_WITHOUT_CONF_STR);
		return CMD_WARNING;
	}
	sprintf(cmd,"/mnt/%s",argv[0]);
	if(0!=access(cmd,F_OK)||0==strcmp(argv[0],"conf_xml.conf")||0==strcmp(argv[0],"cli.conf"))
	{
		vty_out (vty, "WARNING: FILE NAME ERROR\n");
		return  CMD_WARNING;
	}
	sprintf(cmd,"cd /mnt;cp %s ./conf_xml.conf;sor.sh cp conf_xml.conf 10 > /dev/null\n",argv[0]);
	ret = system (cmd);
	ret = WEXITSTATUS(ret);
	if(0 != ret)
		return CMD_WARNING;
	return ret;
}


DEFUN (vtysh_earse_memory,
       vtysh_earse_memory_cmd,
       "erase",
       "Erase start configure\n")
{
	int ret = CMD_SUCCESS;
	char cmd[64];
	char cmd1[128];
	memset(cmd,0,64);
	sprintf(cmd,"earse.sh %s \n",integrate_default);
	system(cmd);
	system("rm -f /var/run/static_arp_file");
	
	/*gujd: 2013-03-22, am 11:11. Add for erase the rtsuit config file when use vty commad erase.
	In order to avoid the SD card out of memory.*/	
#if 1
	memset(cmd1,0,128);
	sprintf(cmd1,"rm %s -f > /dev/null 2 >/dev/null\n",RTSUIT_CONFIG_MNT);
	system(cmd1);
	
	memset(cmd1,0,128);
	sprintf(cmd1,"sor.sh rm %s 30 > /dev/null\n",RTSUIT_CONFIG_BLK);
	system(cmd1);
#endif

	return ret;
}

ALIAS (vtysh_write_memory,
       vtysh_copy_runningconfig_startupconfig_cmd,
       "copy running-config startup-config",  
       "Copy from one file to another\n"
       "Copy from current system configuration\n"
       "Copy to startup configuration\n")

ALIAS (vtysh_write_memory,
       vtysh_write_file_cmd,
       "write file",
       "Write running configuration to memory, network, or terminal\n"
       "Write configuration to the file (same as write memory)\n")

ALIAS (vtysh_write_memory,
       vtysh_write_cmd,
       "write [CONFIGFILENAME]",
       "Write running configuration to memory, network, or terminal\n"
       "Config file name\n")

ALIAS (vtysh_write_terminal,
       vtysh_show_running_config_cmd,
       "show running-config",
       SHOW_STR
       "Current operating configuration\n")

DEFUN (vtysh_terminal_length,
       vtysh_terminal_length_cmd,
       "terminal length <0-512>",
       "Set terminal line parameters\n"
       "Set number of lines on a screen\n"
       "Number of lines on screen (0 for no pausing)\n")
{
  int lines;
  char *endptr = NULL;
  char default_pager[10];

  lines = strtol (argv[0], &endptr, 10);
  if (lines < 0 || lines > 512 || *endptr != '\0')
    {
      vty_out (vty, "length is malformed%s", VTY_NEWLINE);
      return CMD_WARNING;
    }

  if (vtysh_pager_name)
    {
      free (vtysh_pager_name);
      vtysh_pager_name = NULL;
    }

  if (lines != 0)
    {
      snprintf(default_pager, 10, "more -%i", lines);
      vtysh_pager_name = strdup (default_pager);
    }

  return CMD_SUCCESS;
}

DEFUN (vtysh_terminal_no_length,
       vtysh_terminal_no_length_cmd,
       "terminal no length",
       "Set terminal line parameters\n"
       NO_STR
       "Set number of lines on a screen\n")
{
  if (vtysh_pager_name)
    {
      free (vtysh_pager_name);
      vtysh_pager_name = NULL;
    }

  vtysh_pager_init();
  return CMD_SUCCESS;
}

DEFUN (vtysh_show_daemons,
       vtysh_show_daemons_cmd,
       "show daemons",
       SHOW_STR
       "Show list of running daemons\n")
{
  u_int i;

  for (i = 0; i < VTYSH_INDEX_MAX; i++)
    if ( vtysh_client[i].fd >= 0 )
      vty_out(vty, " %s", vtysh_client[i].name);
  vty_out(vty, "%s", VTY_NEWLINE);

  return CMD_SUCCESS;
}

/* Execute command in child process. */
static int
execute_command (const char *command, int argc, const char *arg1,
		 const char *arg2,const char *arg3)
{
  int ret;
  pid_t pid;
  int status;

  /* Call fork(). */
  pid = fork ();

  if (pid < 0)
    {
      /* Failure of fork(). */
      syslog(LOG_ERR, "Can't fork: %s\n", safe_strerror (errno));
      exit (1);
    }
  else if (pid == 0)
    {
      /* This is child process. */
      switch (argc)
	{
	case 0:
	  ret = execlp (command, command, (const char *)NULL);
	  break;
	case 1:
	  ret = execlp (command, command, arg1, (const char *)NULL);
	  break;
	case 2:
		ret = execlp (command, command, arg1, arg2, (const char *)NULL);
		break;
	case 3:
		ret = execlp (command, command, arg1, arg2, arg3, (const char *)NULL);
		break;
	}

      /* When execlp suceed, this part is not executed. */
      syslog(LOG_ERR, "Can't execute %s: %s\n", command, safe_strerror (errno));
 /*     exit (1);*/
    }
  else
    {
      /* This is parent. */
      execute_flag ++;
      ret = wait4 (pid, &status, 0, NULL);
      execute_flag --;
    }
  return 0;
}
/*DEFUN(show_route_mvdrv_entry_cmd_func,
	show_route_mvdrv_entry_cmd,
	"show drvroute A.B.C.D A.B.C.D",
	SHOW_STR
	"Route table infomation on MARVELL driver \n"
	"IP address. eg:192.168.1.0\n"
	"IP mask length. eg:255.255.255.0\n"
)
{
	unsigned int DIP,masklen;

	if(argc != 2)
	{
		vty_out(vty,"parameter number is error\n");
		return CMD_SUCCESS;
	}
	if(1 != inet_atoi(argv[0],&DIP))
	{
		vty_out(vty,"can't get ip address\n");
		return CMD_SUCCESS;

	}
	masklen = get_ip_masklen(argv[1]);


	dcli_route_show_drvroute(vty,DIP,masklen);
	return CMD_SUCCESS;
}
*/

/*DEFUN(show_route_mvdrv_entry1_cmd_func,
		show_route_mvdrv_entry1_cmd,
		"show drvroute A.B.C.D/M",
		SHOW_STR
		"Route table infomation on MARVELL driver \n"
		"IP address and masklen. eg:192.168.1.0/24\n"
	)
{
	unsigned int DIP,masklen;
	unsigned char ipaddrstr[17];
	int addrlen;

	if(argc != 1)
	{
		vty_out(vty,"parameter number is error\n");
		return CMD_SUCCESS;
	}
	addrlen=strchr(argv[0],'/')-argv[0];
	memcpy(ipaddrstr,argv[0],addrlen);
	ipaddrstr[addrlen]='\0';
	
	if(1 != inet_atoi(ipaddrstr,&DIP))
	{
		vty_out(vty,"can't get ip address\n");
		return CMD_SUCCESS;

	}
	masklen = atoi(argv[0]+addrlen+1);
	dcli_route_show_drvroute(vty,DIP,masklen);
	return CMD_SUCCESS;
}
*/

/*DEFUN (vtysh_ping,
       vtysh_ping_cmd,
       "ping WORD",
       "Send echo messages\n"
       "Ping destination address or hostname\n")
{
  execute_command ("ping", 1, argv[0], NULL,NULL);
  return CMD_SUCCESS;
}

ALIAS (vtysh_ping,
       vtysh_ping_ip_cmd,
       "ping ip WORD",
       "Send echo messages\n"
       "IP echo\n"
       "Ping destination address or hostname\n")

*/
/*DEFUN (vtysh_ping_c_arg,
       vtysh_ping_count_cmd,
       "ping WORD -c  COUNT",
       "Send echo messages\n"
       "Ping destination address or hostname\n"
	   "The numer of ping count\n"
	   "The count\n"
		)
{
	char buf[10] = "-c";
	execute_command("ping",3,argv[0],buf,argv[1]);
	return CMD_SUCCESS;
}*/

/*DEFUN (vtysh_ping_t_arg,
       vtysh_ping_time_cmd,
       "ping WORD -t TTL",
       "Send echo messages\n"
       "Ping destination address or hostname\n"
	   "The ttl number of send packets\n"
	   "Ttl\n")
{
	char buf[10] = "-t";
	execute_command("ping",3,argv[0],buf,argv[1]);
	return CMD_SUCCESS;
}

*/
/*DEFUN (vtysh_traceroute,
       vtysh_traceroute_cmd,
       "traceroute WORD",
       "Trace route to destination\n"
       "Trace route to destination address or hostname\n")
{
  	execute_command ("traceroute", 2, "-I", argv[0], NULL);
  return CMD_SUCCESS;
}*/
/*DEFUN (vtysh_traceroute_udp,
       vtysh_traceroute_udp_cmd,
       "traceroute WORD udp",
       "Trace route to destination\n"
       "Trace route to destination address or hostname\n"
       "Trace route to destination with udp packet\n")
{
  	execute_command ("traceroute", 1, argv[0], NULL, NULL);
  return CMD_SUCCESS;
}

ALIAS (vtysh_traceroute,
       vtysh_traceroute_ip_cmd,
       "traceroute ip WORD",
       "Trace route to destination\n"
       "IP trace\n"
       "Trace route to destination address or hostname\n")
ALIAS (vtysh_traceroute_udp,
	   vtysh_traceroute_ip_udp_cmd,
	   "traceroute ip WORD udp",
	   "Trace route to destination\n"
	   "IP trace\n"
	   "Trace route to destination address or hostname\n"
	   "Trace route to destination with udp packet\n")
*/
/*#ifdef HAVE_IPV6
DEFUN (vtysh_ping6,
       vtysh_ping6_cmd,
       "ping ipv6 WORD",
       "Send echo messages\n"
       "IPv6 echo\n"
       "Ping destination address or hostname\n")
{
  execute_command ("ping6", 1, argv[0], NULL, NULL);
  return CMD_SUCCESS;
}
*/
/*DEFUN (vtysh_traceroute6,
       vtysh_traceroute6_cmd,
       "traceroute ipv6 WORD",
       "Trace route to destination\n"
       "IPv6 trace\n"
       "Trace route to destination address or hostname\n")
{
  execute_command ("traceroute6", 1, argv[0], NULL,NULL);
  return CMD_SUCCESS;
}
#endif
*/

/*DEFUN (vtysh_telnet,
       vtysh_telnet_cmd,
       "telnet WORD",
       "Open a telnet connection\n"
       "IP address or hostname of a remote system\n")
{
  execute_command ("telnet", 1, argv[0], NULL,NULL);
  return CMD_SUCCESS;
}
*/
/*DEFUN (vtysh_telnet_port,
       vtysh_telnet_port_cmd,
       "telnet WORD PORT",
       "Open a telnet connection\n"
       "IP address or hostname of a remote system\n"
       "TCP Port number\n")
{
  execute_command ("telnet", 2, argv[0], argv[1],NULL);
  return CMD_SUCCESS;
}*/

/*DEFUN (vtysh_ssh,
       vtysh_ssh_cmd,
       "ssh WORD",
       "Open an ssh connection\n"
       "[user@]host\n")
{
  execute_command ("ssh", 1, argv[0], NULL,NULL);
  return CMD_SUCCESS;
}
*/
DEFUN (vtysh_start_shell,
       vtysh_start_shell_cmd,
       "start-shell",
       "Start UNIX shell\n")
{

//  execute_command2("bash","-bash", 0, NULL, NULL, NULL);
  system("startbash.sh");
  return CMD_SUCCESS;
}

DEFUN (vtysh_shell_exec,
       vtysh_shell_exec_cmd,
       "shell-exec WORD",
       "Execute UNIX shell command.\n"
       "The command that need to be executed")
{

	system(argv[0]);
  return CMD_SUCCESS;
}


DEFUN (vtysh_start_bash,
       vtysh_start_bash_cmd,
       "start-shell bash",
       "Start UNIX shell\n"
       "Start bash\n")
{
  execute_command ("bash", 0, NULL, NULL,NULL);
  return CMD_SUCCESS;
}

DEFUN (vtysh_start_zsh,
       vtysh_start_zsh_cmd,
       "start-shell zsh",
       "Start UNIX shell\n"
       "Start Z shell\n")
{
  execute_command ("zsh", 0, NULL, NULL, NULL);
  return CMD_SUCCESS;
}

void sync_hostname_fun()
{	
	void (*dcli_init_func)();	/*houxx:2012-6-13 sync hostname*/
	char *error;
	void (*dcli_sync_hostname)(void) = NULL;

	  dcli_sync_hostname = dlsym(dcli_dl_handle,"sync_hostname");
	  if ((error = dlerror()) != NULL) {
		  printf("Run without print_msg_form_vtysh be called.\n");
		  fputs(error,stderr);
	  }
	  (*dcli_sync_hostname)();
}

/**********************************************************************/
/*gujd:2013-03-01, am 10:27. Add set hostname independent between every board.*/  
void sync_hostname_fun_v2(int slot_ID)
{	
	void (*dcli_init_func)();
	char *error;
	void (*dcli_sync_hostname_v2)(int slotid) = NULL;

	  dcli_sync_hostname_v2 = dlsym(dcli_dl_handle,"sync_hostname_v2");
	  if ((error = dlerror()) != NULL) {
		  printf("Run without print_msg_form_vtysh be called.\n");
		  fputs(error,stderr);
	  }
	  
	  (*dcli_sync_hostname_v2)(slot_ID);
	  
}

/*gujd: 2013-02-28 , am 4:01 . Add func for judge Product type .*/
int
fetch_product_slot_count(char *product_name)
{
	int slot_count = 0;
	
	if(!product_name)
	{
		syslog(LOG_NOTICE,"product name is NULL.\n");
		return -1;
	}
	if(strncmp(product_name, AX7605I_PRODUCT_NAME, sizeof(AX7605I_PRODUCT_NAME))==0)/*7605i*/
	{
		slot_count = 3;
	}
	else if(strncmp(product_name, AX8603_PRODUCT_NAME, sizeof(AX8603_PRODUCT_NAME))==0)/*8603*/
	{
		slot_count = 3;
	}
	else if(strncmp(product_name, AX8606_PRODUCT_NAME, sizeof(AX8606_PRODUCT_NAME))==0)/*8606*/
	{
		slot_count = 6;
	}
	else if(strncmp(product_name, AX8610_PRODUCT_NAME, sizeof(AX8610_PRODUCT_NAME))==0)/*8610*/
	{
		slot_count = 10;
	}
	else if(strncmp(product_name, AX8800_PRODUCT_NAME, sizeof(AX8800_PRODUCT_NAME))==0)/*8800*/
	{
		slot_count = 14;
	}
	else/*enlarge future*/
	{
		syslog(LOG_NOTICE,"unkown product type.\n");
	}
	
	return slot_count;
	
}

int
get_product_slot_count()
{
	FILE *fd = NULL;
	char product_name[64]={0};
	int slot_count = 0;
	
	fd = fopen(PRODUCT_NAME_FILE,"r");
	if(NULL == fd)
	{
		syslog(LOG_NOTICE,"fopen /dbm/product/name failed.\n");
		fclose(fd);
		return -1;
	}
	fscanf(fd,"%s",product_name);
	/*syslog(LOG_NOTICE,"product name is %s .\n",product_name);*/
	fclose(fd);
	
	fd = fopen("/dbm/product/active_master_slot_id", "r");
	if (fd == NULL)
	{		
		syslog(LOG_NOTICE,"fopen /dbm/product/active_master_slot_id failed.\n");
		fclose(fd);
		return -1;
	}
	fscanf(fd, "%d", &active_master_slot_id);
	fclose(fd);

	slot_count = fetch_product_slot_count(product_name);		
	return slot_count;	
	
}

struct host_name *
read_hostname_from_file(int slot, struct host_name *Hostname)
{
	int ret = 0;
	FILE *fp = NULL;
	char *default_name = HOSTNAME_DEFAULT;
	int length = strlen(default_name);
	char str[HOSTNAME_MAX_LEN+1] = {0};
	char filename[128] = {0};
	
	/*the file is "/dbm/product/slot/slotX/host_name"*/
	sprintf(filename,"%sslot%d%s",PRODUCT_SLOT_DIR,slot,HOSTNAME_FILE);
	
	ret = access(filename, 0);/**to check the file is exist or not ?**/
	if(ret < 0)    /**if not, creat it**/
	 {
	  	fp = fopen(filename,"w+");
		if(!fp)
		{
			syslog(LOG_NOTICE,"fopen file (%s) failed.\n",filename);
			return NULL;
		}
		/*length + 1 is '\0' */
		memcpy(str,default_name,length);
		str[length]= '\n';
		fwrite(str, length+1, 1,fp);
		fclose(fp);
		memcpy(Hostname->Host_name,str,length+1);

		return Hostname;
	 }
	else
	{
		/*the switch of distribute system*/
		 fp = fopen(filename,"r");
		 if(fp == NULL)
		 {
			 syslog(LOG_NOTICE,"fopen file (%s) failed.\n",filename);
			 return NULL;
		 }
		 fscanf(fp ,"%s",Hostname->Host_name);
		 fclose(fp);
		 return Hostname;
			
		}

}

int
write_hostname_to_file(const char *word, int slot)
{
	int ret = 0;
	FILE *fp = NULL;
	int length = strlen(word);
	char str[HOSTNAME_MAX_LEN+1] = {0};	
	char filename[128] = {0};
	
	sprintf(filename,"%sslot%d%s",PRODUCT_SLOT_DIR,slot,HOSTNAME_FILE);
	
	fp = fopen(filename,"w+");
	if(!fp)
	{
		syslog(LOG_NOTICE,"fopen file (%s) failed.\n",filename);
		return -1;
	}
	/*length + 1 is '\0' */
	memcpy(str,word,length);
	str[length]= '\n';
	fwrite(str, length+1, 1,fp);
	fclose(fp);
	
	return 0;
}

int
write_word_to_file(const char *word, const char *file_path)
{
	int ret = 0;
	FILE *fp = NULL;
	int length = strlen(word);
	char str[HOSTNAME_MAX_LEN+1] = {0};	
	
	/*sprintf(filename,"%sslot%d%s",PRODUCT_SLOT_DIR,slot,HOSTNAME_FILE);*/
	
	fp = fopen(file_path,"w+");
	if(!fp)
	{
		syslog(LOG_NOTICE,"fopen file (%s) failed.\n",file_path);
		return -1;
	}
	/*length + 1 is '\0' */
	memcpy(str,word,length);
	str[length]= '\n';
	fwrite(str, length+1, 1,fp);
	fclose(fp);
	
	return 0;
}

int
read_int_vlaue_from_file(const char *file_path)
{
	FILE *fp = NULL;
	int value = -1;
		
	 if(file_path== NULL)
	 {
		return -1;
	 }
	 fp = fopen(file_path,"r");
	 if(fp == NULL)
	 {
		 syslog(LOG_NOTICE,"fopen file (%s) failed.\n",file_path);
		 return -1;
	 }
	 fscanf(fp ,"%d",&value);
	 fclose(fp);
	 return value;
	
}


int
vtysh_get_every_board_hostname_list_init()
{
	int i ;
	int slot_count = 0;
	struct hostname *hostname_tmp = NULL;
	int ret = 0;

	/*host is global varrible*/
	host.host_name_list = list_new();

	slot_count = get_product_slot_count();
	if(slot_count <= 0)
	{
		syslog(LOG_NOTICE,"get slot count failed.\n");
		return -1;
	}

	for(i=1; i <= slot_count; i++)
	{
		struct host_name *Hostname;
		Hostname = XCALLOC(MTYPE_HOSTNAME,sizeof(struct host_name));
		if(active_master_slot_id == i)/*active master copy host name*/
		{
			int length = strlen(host.name);
			memcpy(Hostname->Host_name, host.name, length);
			Hostname->Host_name[length] = '\0';
			ret = write_hostname_to_file(Hostname->Host_name, i);
			if(ret < 0)
			{
				syslog(LOG_NOTICE,"write hostname from_file failed.\n");
				list_free(host.host_name_list);
				XFREE(MTYPE_HOSTNAME,Hostname);
				return -1;
			}
		 }
		else
		{
			hostname_tmp = read_hostname_from_file( i,Hostname);
			if(!hostname_tmp)
			{
				syslog(LOG_NOTICE,"read hostname from_file failed.\n");
				list_free(host.host_name_list);
				XFREE(MTYPE_HOSTNAME,Hostname);
				return -1;
			}
		 }
		Hostname->slotid = i ;/*slot*/
		Hostname->flag = 0;/*default is global*/
		
		listnode_add (host.host_name_list, Hostname);
		
		
	}
	

	return 0;
}

int
hostname_list_init()
{
	int i ;
	int slot_count = 0;

	/*host is global varrible*/
	host.host_name_list = list_new();

	slot_count = get_product_slot_count();
	if(slot_count <= 0)
	{
		syslog(LOG_NOTICE,"get slot count failed.\n");
		return -1;
	}

	for(i=1; i <= slot_count; i++)
	{
		struct host_name *Hostname;
		Hostname = XCALLOC(MTYPE_HOSTNAME,sizeof(struct host_name));
		if(active_master_slot_id == i)/*active master copy host name*/
		{
			int length = strlen(host.name);
			memcpy(Hostname->Host_name, host.name, length);
			Hostname->Host_name[length] = '\0';
			}
		else
		{
			int length = strlen(HOSTNAME_DEFAULT);
		 	memcpy(Hostname->Host_name,HOSTNAME_DEFAULT,length);/*other baord default  is SYSTEM*/
			Hostname->Host_name[length] = '\0';
			}
		Hostname->slotid = i ;/*slot*/
		Hostname->flag = 0;/*default is global*/
		
		listnode_add (host.host_name_list, Hostname);
		
		
	}
	

	return slot_count;
}

void
hostname_list_free(void)
{
  	struct listnode *node = NULL , *nextnode = NULL;
	struct host_name *Hostname = NULL;
		
	for(ALL_LIST_ELEMENTS (host.host_name_list, node, nextnode, Hostname))
	{
		listnode_delete(host.host_name_list, Hostname);
		XFREE(MTYPE_HOSTNAME,Hostname);
     }
	return;	
}

#ifdef DISTRIBUT
/* Hostname configuration support set every board independent.*/
DEFUN (vtysh_config_indepent_hostname, 
       vtysh_hostname_independent_cmd,
       "hostname slot ID WORD",
       "Set system's network name\n"
       "Support set one board\n"
       "The board slot id\n"
       "This system's network name\n")
{
	char cmd[128]={0};
	char* hostname = (char *)argv[1];
	char tmp;
	int i=0,len = strlen(hostname);
	int slotid = 0, slot_count =0;
	int ret = 0;

	if(argc < 2)
	{
		vty_out(vty, "Bad parameter!%s", VTY_NEWLINE);
		return CMD_WARNING;
	}
	
	slotid = atoi(argv[0]);
	
	if(len > HOSTNAME_MAX_LEN)
	{
		vty_out (vty, "Please specify string lenth <= 64 %s", VTY_NEWLINE);
		return CMD_WARNING;
	}
  if (!isalpha((int) *argv[1]))
    {
      vty_out (vty, "Please specify string starting with alphabet%s", VTY_NEWLINE);
      return CMD_WARNING;
    }
  tmp = *hostname;
  for(i = 0;i<len ;i++)
  {
	  tmp = *(hostname+i);
	  if( (tmp>='0' && tmp<='9')||(tmp>='a' &&tmp<='z')
			||(tmp>='A' &&tmp<='Z')||(tmp=='-')||(tmp=='.'))
	  {
		  continue;
	  }
	  else
	  {
		  vty_out (vty, "Please specify string with alphabet or number%s", VTY_NEWLINE);
		  return CMD_WARNING;
	  }
  }
  slot_count = get_product_slot_count();
  if(slotid > slot_count)
  	{
	  vty_out (vty, "Please input a right slot again %s", VTY_NEWLINE);
	  return CMD_WARNING;
  		
  	}
  if(slotid == active_master_slot_id)/*active master. */
  {
  	struct listnode *node = NULL , *nextnode = NULL;
	struct host_name *Hostname = NULL;
	
  	if(host.name)
      XFREE (MTYPE_HOST, host.name);
	host.name = XSTRDUP (MTYPE_HOST, argv[1]);
	sprintf(cmd,"hostname.sh %s",argv[1]);
	system(cmd);
	
	for(ALL_LIST_ELEMENTS (host.host_name_list, node, nextnode, Hostname))
	{
		if(Hostname->slotid == active_master_slot_id)
		{
			int length = strlen(host.name);
			/*delete before*/
			memset(Hostname->Host_name,0,HOSTNAME_MAX_LEN);
			/*copy new*/
			memcpy(Hostname->Host_name,host.name,length);
			Hostname->Host_name[length] = '\0';
			SET_FLAG(Hostname->flag, HOSTNAEM_LOCAL);
			
			/*vty_out (vty, "Hostname[%s],hostname[%s].%s",Hostname->Host_name,host.name, VTY_NEWLINE);*/
			ret = write_hostname_to_file(Hostname->Host_name,Hostname->slotid);
			if(ret < 0)
			{
				vty_out (vty, "Write active master hostname file failed.%s", VTY_NEWLINE);
				return CMD_WARNING;
			}
			vty_out (vty, "Only active master set hostname sucess.%s", VTY_NEWLINE);
			
			write_word_to_file("0",HOSTNAME_SCOPE_FLAG_FILE);
			return CMD_SUCCESS;
			
		 }
	
     }
  }
  else/*other boards .*/
  {
  	/*struct listnode *node = NULL , *nextnode = NULL;*/
  	struct listnode *node = NULL, *nextnode = NULL;
	struct host_name *Hostname = NULL;
	
	/*use a new file under /var/run/other_hostname*/
	for(ALL_LIST_ELEMENTS (host.host_name_list, node, nextnode, Hostname))/*search bingo slot*/
	{
		if(Hostname->slotid == slotid)
		{
			int length = strlen(argv[1]);
			/*delete before*/
			memset(Hostname->Host_name,0,HOSTNAME_MAX_LEN);
			/*copy new, argv[1]*/
			memcpy(Hostname->Host_name, argv[1], length);
			Hostname->Host_name[length] = '\0';
			SET_FLAG(Hostname->flag, HOSTNAEM_LOCAL);
			break;
		}
	}
	ret = write_hostname_to_file(Hostname->Host_name,slotid);/*write file*/
	if(ret < 0)
	{
		
		vty_out (vty, "Set slot (%d) hostname failed .%s",slotid, VTY_NEWLINE);
		return CMD_WARNING;
	}
	/*sync*/
	sync_hostname_fun_v2(slotid);

	vty_out (vty, "Set slot (%d) hostname sucess.%s",slotid, VTY_NEWLINE);
	
	write_word_to_file("0",HOSTNAME_SCOPE_FLAG_FILE);
	return CMD_SUCCESS;
	
   }
  
}


/* Hostname delete support set every board independent.*/
DEFUN (vtysh_no_config_indepent_hostname, 
       vtysh_no_hostname_independent_cmd,
       "no hostname slot ID",
       "Delete system's network name\n"
       "Support delete one board\n"
       "The board slot id\n")
{
	char cmd[128]={0};
	char* hostname = (char *)argv[1];
	char tmp;
	int i=0,len = strlen(hostname);
	int slotid = 0, slot_count =0;
	int ret = 0;
	
	slotid = atoi(argv[0]);

	slot_count = get_product_slot_count();
  if(slotid > slot_count)
  	{
	  vty_out (vty, "Please input a right slot again %s", VTY_NEWLINE);
	  return CMD_WARNING;
  		
  	}
  if(slotid == active_master_slot_id)/*active master. */
  {
  	struct listnode *node = NULL , *nextnode = NULL;
	struct host_name *Hostname = NULL;
	
  	if(host.name)
      XFREE (MTYPE_HOST, host.name);
	host.name = XSTRDUP (MTYPE_HOST, HOSTNAME_DEFAULT);
	sprintf(cmd,"hostname.sh %s",HOSTNAME_DEFAULT);
	system(cmd);
	
	for(ALL_LIST_ELEMENTS (host.host_name_list, node, nextnode, Hostname))
	{
		if(Hostname->slotid == active_master_slot_id)
		{
			int length = strlen(HOSTNAME_DEFAULT);
			/*delete before*/
			memset(Hostname->Host_name,0,HOSTNAME_MAX_LEN);
			/*copy new*/
			memcpy(Hostname->Host_name,HOSTNAME_DEFAULT,length);
			Hostname->Host_name[length] = '\0';
			SET_FLAG(Hostname->flag, HOSTNAEM_LOCAL);
			ret = write_hostname_to_file(Hostname->Host_name,Hostname->slotid);
			if(ret < 0)
			{
				vty_out (vty, "Delete active master hostname file failed.%s", VTY_NEWLINE);
				return CMD_WARNING;
			}
			/*break;*/
			vty_out (vty, "Only active master delete hostname sucess.%s", VTY_NEWLINE);
			return CMD_SUCCESS;
		 }
	
	
     }
  }
  else/*other boards .*/
  {
  	/*struct listnode *node = NULL , *nextnode = NULL;*/
  	struct listnode *node = NULL, *nextnode = NULL;
	struct host_name *Hostname = NULL;
	
	/*use a new file under /var/run/other_hostname*/
	for(ALL_LIST_ELEMENTS (host.host_name_list, node, nextnode, Hostname))/*search bingo slot*/
	{
		if(Hostname->slotid == slotid)
		{
			int length = strlen(HOSTNAME_DEFAULT);
			/*delete before*/
			memset(Hostname->Host_name,0,HOSTNAME_MAX_LEN);
			/*copy new, argv[1]*/
			memcpy(Hostname->Host_name, HOSTNAME_DEFAULT, length);
			Hostname->Host_name[length] = '\0';
			SET_FLAG(Hostname->flag, HOSTNAEM_LOCAL);
			break;
		}
	}
	ret = write_hostname_to_file(Hostname->Host_name,slotid);/*write file*/
	if(ret < 0)
	{
		
		vty_out (vty, "Delete slot (%d) hostname failed .%s",slotid, VTY_NEWLINE);
		return CMD_WARNING;
	}
	/*sync*/
	sync_hostname_fun_v2(slotid);

	vty_out (vty, "Delete slot (%d) hostname sucess.%s",slotid, VTY_NEWLINE);
	return CMD_SUCCESS;
	
   }
  
}
#endif
/****************************Add hostname independent end.********************************************/

/* Hostname configuration */
DEFUN (vtysh_config_hostname, 
       vtysh_hostname_cmd,
       "hostname WORD",
       "Set system's network name\n"
       "This system's network name\n")
{
	/*char* cmd[128];*/
	char cmd[128]={0};/*gujd: 2012-2-23, pm 5:30 . In order to decrease the warning when make img . */
	
	/*char* hostname = argv[0];*/
	char* hostname = (char *)argv[0];/*gujd: 2012-2-23, pm 5:30 . In order to decrease the warning when make img . */
	char tmp;
	int i=0,len = strlen(hostname);

	if(len>64)
	{
		vty_out (vty, "Please specify string lenth <= 64 %s", VTY_NEWLINE);
		return CMD_WARNING;
	}
  if (!isalpha((int) *argv[0]))
    {
      vty_out (vty, "Please specify string starting with alphabet%s", VTY_NEWLINE);
      return CMD_WARNING;
    }
  tmp = *hostname;
  for(i = 0;i<len ;i++)
  {
	  tmp = *(hostname+i);
	  if( (tmp>='0' && tmp<='9')||(tmp>='a' &&tmp<='z')
			||(tmp>='A' &&tmp<='Z')||(tmp=='-')||(tmp=='.'))
	  {
		  continue;
	  }
	  else
	  {
		  vty_out (vty, "Please specify string with alphabet or number%s", VTY_NEWLINE);
		  return CMD_WARNING;
	  }
  }

  if (host.name)
    XFREE (MTYPE_HOST, host.name);
    
  host.name = XSTRDUP (MTYPE_HOST, argv[0]);
  sprintf(cmd,"hostname.sh %s",argv[0]);
  system(cmd);
  sync_hostname_fun();/*added by houxx 2012-6-13*/
  
  /*gujd:2013-03-01, am 10:27. Add set hostname independent for show run.*/	
  write_word_to_file("1",HOSTNAME_SCOPE_FLAG_FILE);
  
  return CMD_SUCCESS;
}

DEFUN (vtysh_config_no_hostname, 
       vtysh_no_hostname_cmd,
       "no hostname [HOSTNAME]",
       NO_STR
       "Reset system's network name\n"
       "Host name of this router\n")
{
  	struct listnode *node = NULL, *nextnode = NULL;
	struct host_name *Hostname = NULL;
	int ret = 0;

  if (host.name)
    XFREE (MTYPE_HOST, host.name);
  host.name = XSTRDUP (MTYPE_HOST, "SYSTEM");
  system("hostname.sh SYSTEM");
  sync_hostname_fun();/*added by houxx 2012-6-13*/
  
  /*gujd:2013-03-01, am 10:27. Add set hostname independent between every board.*/	
  #if 1
  write_word_to_file("0",HOSTNAME_SCOPE_FLAG_FILE);
  for(ALL_LIST_ELEMENTS (host.host_name_list, node, nextnode, Hostname))/*search bingo slot*/
  {
		int length = strlen(HOSTNAME_DEFAULT);
		/*delete before*/
		memset(Hostname->Host_name,0,HOSTNAME_MAX_LEN);
		/*copy new, argv[1]*/
		memcpy(Hostname->Host_name, HOSTNAME_DEFAULT, length);
		Hostname->Host_name[length] = '\0';
		SET_FLAG(Hostname->flag, HOSTNAEM_LOCAL);
		ret = write_hostname_to_file(Hostname->Host_name,Hostname->slotid);/*write file*/
		if(ret < 0)
		{
			
			vty_out (vty, "Delete one slot (%d) hostname failed .%s",Hostname->slotid, VTY_NEWLINE);
			return CMD_WARNING;
		}
		/*sync*/
	/*	sync_hostname_fun_v2(slotid);*/
		
		/*vty_out (vty, "Delete one slot (%d) hostname sucess.%s",Hostname->slotid, VTY_NEWLINE);*/
	}
 	
   #endif
  return CMD_SUCCESS;
}

DEFUN (show_process,
       show_process_cmd,
       "show process [PROCESS_NAME]",
       SHOW_STR
       "Show cpu process\n"
       "Show cpu process by process name\n")
{
	char cmd[256]={0};
	if (0 == argc)
	{
		sprintf(cmd,"ps -e");
	}else{
		sprintf(cmd,"ps -e | grep %s | grep -v \"grep\"",argv[0]);
	}

	system(cmd);
		
  return CMD_SUCCESS;
}

DEFUN (show_process_by_bsd,
       show_process_by_bsd_cmd,
       "show process bsd [PROCESS_NAME] ",
       SHOW_STR
       "Show cpu process\n"
       "Show cpu process with BSD format\n"
       "Show cpu process by process name wity BSD format\n")
{
	char cmd[256]={0};
	if (0 == argc)
	{
		sprintf(cmd,"ps -aux");
	}else{
		sprintf(cmd,"ps -aux | grep %s |grep -v \"grep\"",argv[0]);
	}

	system(cmd);
		
  return CMD_SUCCESS;
}
DEFUN (show_process_count,
       show_process_count_cmd,
       "show process count [PROCESS_NAME] ",
       SHOW_STR
       "Show cpu process\n"
       "Show cpu process number\n"
       "Display the number of process name\n")
{
	char cmd[256]={0};
	if (0 == argc)
	{
		sprintf(cmd,"ps -e |grep -v \"grep\"| wc -l");
	}else{
		sprintf(cmd,"ps -e | grep %s |grep -v \"grep\"| wc -l",argv[0]);
	}

	system(cmd);
		
  return CMD_SUCCESS;
}


DEFUN (show_process_all,
       show_process_all_cmd,
       "show process all",
       SHOW_STR
       "Show cpu process\n"
       "Show all cpu process\n")
{
  system ("htop");
  return CMD_SUCCESS;
}

DEFUN (hidden_debug_exit_cmd_func,
       hidden_debug_exit_cmd,
       "exit",
       "Exit hidden mode\n")
{
	vty->node = save_node;
	return CMD_SUCCESS;
}

DEFUN(vtysh_enable_cmd_func,
	vtysh_enable_cmd_autelan,
	"vtysh enable",
	"enter vtysh command line\n" )
{
	vtysh_enable_flags = 1;
	return CMD_SUCCESS;
	
}

#if 0
DEFUN(vtysh_disable_cmd_func,
	vtysh_disable_cmd_autelan,
	"vtysh disable",
	"Cannot enter vtysh command line\n" )
{
	vtysh_enable_flags = 0;
	return CMD_SUCCESS;
	
}
#endif

#if 0/*dongshu for del cli_syslog cmd*/
int set_cli_syslog = 0;
int get_cli_syslog_str(void)
{
	FILE *fp=NULL;
	char *ptr=NULL;
	int ret=0;

	fp = fopen(VTYSH_CLI_LOG_CONFIG_FILE,"r");
	
	if(!fp)
		return 0;
	ptr=malloc(8);
	if(!ptr)
		{
		fclose(fp);
		return 0;
	}
	memset(ptr,0,8);
	fgets(ptr,8,fp);
	ret = atoi(ptr);
	free(ptr);
	fclose(fp);
	return ret;
	
}
int set_cli_syslog_str(int num)
{
	FILE * fp=NULL;
	char *ptr=NULL;
	int i=0;

	fp = fopen(VTYSH_CLI_LOG_CONFIG_FILE,"w");
	if(!fp)
		return -1;

	ptr=malloc(8);
	if(!ptr)
		{
		fclose(fp);
		return -1;
	}
	sprintf(ptr,"%d\n",num);
	fputs(ptr,fp);

	free(ptr);
	fclose(fp);
	return 0;

}

DEFUN (set_vtysh_cli_log_func,
       set_vtysh_cli_log_func_cmd,
       "set cli-log (on|off)",
       "Set system configuration\n"
       "Set cli syslog\n"
       "Set cli syslog on\n"
       "Set cli syslog off\n")
{
	if(!strncmp(argv[0],"on",2))
	{
		set_cli_syslog = 1;
		
	}
	else
	{
		set_cli_syslog = 0;
	}
	set_cli_syslog_str(set_cli_syslog);
  return CMD_SUCCESS;
}

DEFUN (show_vtysh_cli_log_func,
       show_vtysh_cli_log_func_cmd,
       "show cli-log ",
       "Show system configuration\n"
       "Show cli syslog\n")
{
		if(set_cli_syslog==1)
			vty_out(vty,"The system cli_log is on\r\n");
		else			
			vty_out(vty,"The system cli_log is off\r\n");
  	return CMD_SUCCESS;
}
#endif/*dongshu end*/


int set_idle_time_init()
{
	FILE* fp=NULL;
	char ptr[64];
	int get_idle=0;

	memset(ptr,0,64);
	fp = fopen(VTYSH_IDLE_TIMEOUT_CONFIG_FILE,"r");
	
	if(!fp)
	{
		idle_time = IDLE_TIME_DEFAULT;
		idle_time_rem = idle_time;
		return -1;
	}
	while(fgets(ptr,64,fp))	
	{

		if(!strncmp(ptr,"idle_timeout ",13))
		{
			get_idle=1;
			break;

		}
	}
	fclose(fp);
	if(get_idle)
		idle_time = atoi(ptr+13);
	else
		idle_time = IDLE_TIME_DEFAULT;
		
	idle_time_rem = idle_time;
	return idle_time;
}

int set_idle_time(int minutes)
{
	FILE* fp=NULL;
	char ptr[64];

	memset(ptr,0,64);
	fp = fopen(VTYSH_IDLE_TIMEOUT_CONFIG_FILE,"w");
	if(!fp)
		return -1;
	
	idle_time=minutes;
	sprintf(ptr,"idle_timeout %d\n",minutes);
	fputs(ptr,fp);
	fclose(fp);
	idle_time_rem = idle_time;
	return 0;
}

DEFUN (set_idle_time_func,
       set_idle_time_func_cmd,
       "idle-timeout  <0-3600>",
	   "Set idle timeout value\n"
	   "Idle timeout value in minutes (set to 0 will disable idle-timeout)\n")
{
	int ret;
	ret = set_idle_time(atoi(argv[0]));
	if(ret)
	{
		vty_out(vty,"Set idle timeout value ERROR!");
		return CMD_WARNING;
	}
	
	return CMD_SUCCESS;
}

DEFUN (show_idle_time_func,
       show_idle_time_func_cmd,
       "show idle-timeout",
	   SHOW_STR
	   "Idle timeout value in minutes\n")
{
	set_idle_time_init();

	vty_out(vty,"Idle time out is set to %d minutes.",idle_time);
	return CMD_SUCCESS;
}

DEFUN (no_set_idle_time_func,
       no_set_idle_time_func_cmd,
       "no idle-timeout",
	   NO_STR
	   "Set Idle timeout default value(10 minutes)\n")
{
	int ret;
	ret = set_idle_time(IDLE_TIME_DEFAULT);
	if(ret)
	{
		vty_out(vty,"Set idle timeout value ERROR!");
		return CMD_WARNING;
	}
	
	return CMD_SUCCESS;
}
extern 	struct termios bktty;
void show_input_echo(void)
{
	struct termios savetty;
	int ret;

	tcgetattr(fileno(stdout),&savetty);
	savetty.c_lflag |= ECHO;
	memcpy(&bktty,&savetty,sizeof(savetty));
	ret = tcsetattr(fileno(stdout),TCSANOW,&savetty);

}


void hide_input_echo(void)
{

	struct termios savetty;
	int ret;
	tcgetattr(fileno(stdout),&savetty);
	savetty.c_lflag &= ~ECHO;

	memcpy(&bktty,&savetty,sizeof(savetty));

	ret = tcsetattr(fileno(stdout),TCSANOW,&savetty);
	

}

int set_ripd_socket_buffer(unsigned int entry)
{
	FILE* confp=NULL;
	
	/*gujd: 2012-02-09: am11:30 . In order to decrease the warning when make img . For initialization every variable separate  .*/
#if 0
	char* buf = NULL, ptr = NULL;
#else
	char* buf = NULL;
	char* ptr = NULL;
#endif
	char tmp[128];
	int ret=-1,buflen=0;


	memset(tmp,0,128);	
	confp = fopen(RIPD_SOCKET_BUF_FILE,"w");
	if (!confp )
	{
		syslog(LOG_ERR,"Can't open %s file\n",RIPD_SOCKET_BUF_FILE);
		return ret;
	}

	buflen = entry*42;/*42=41600/(40*25)*/
	sprintf(tmp,"%s%d\n",RIPD_MAX_SOCKET_BUF,buflen);
	ret = fputs(tmp,confp);
	if(EOF == ret)
		{
		syslog(LOG_ERR,"Write file %s error\n",RIPD_SOCKET_BUF_FILE);
		fclose(confp);
		return -1;
	}
	fclose(confp);
	return ret;	
		
}

DEFUN (rip_max_socket_buffer,
       rip_max_socket_buffer_cmd,
       "rip max entry <100-5000>",
       "RIP routes\n"
	   "Max entry value\n"
       "Set rip max entry value\n"
       "Set rip max entry value <100-5000>.default 1000\n")
{
	int ret,max_rip_entry  = atoi(argv[0]);

//	vty_out(vty,"max_rip_entry is %d \n",max_rip_entry);
	
	if(max_rip_entry>0)
	{
		ret=set_ripd_socket_buffer(max_rip_entry);
		if(ret==-1)
		{
			vty_out(vty,"Set entry value is error\n");
			return CMD_WARNING;
		}
	}
	else
		{
		vty_out(vty,"The rip entry is error\n");
		return CMD_WARNING;
	}
  	return CMD_SUCCESS;
}

static void
vtysh_install_default (enum node_type node)
{
  install_element (node, &config_list_cmd);
}

/* Making connection to protocol daemon. */
static int
vtysh_connect (struct vtysh_client *vclient)
{
  int ret;
  int sock, len;
  struct sockaddr_un addr;
  struct stat s_stat;
  struct	 timeval   recv_timeval;

  /* Stat socket to see if we have permission to access it. */
  ret = stat (vclient->path, &s_stat);
  if (ret < 0 && errno != ENOENT)
    {
      syslog(LOG_NOTICE, "vtysh_connect(%s): stat = %s\n", 
		vclient->path, safe_strerror(errno)); 
      EXIT(1);
    }
  
  if (ret >= 0)
    {
      if (! S_ISSOCK(s_stat.st_mode))
	{
	  syslog(LOG_NOTICE, "vtysh_connect(%s): Not a socket\n",
		   vclient->path);
      EXIT(1);
	}
      
    }

  sock = socket (AF_UNIX, SOCK_STREAM, 0);
  if (sock < 0)
    {
#ifdef DEBUG
      syslog(LOG_NOTICE, "vtysh_connect(%s): socket = %s\n", vclient->path,
	      safe_strerror(errno));
#endif /* DEBUG */
      return -1;
    }
#if 0  
  recv_timeval.tv_sec	=	30;
  recv_timeval.tv_usec	 =	 0;
  
  setsockopt(sock,   SOL_SOCKET,	SO_RCVTIMEO,   &recv_timeval,	sizeof(recv_timeval));
  set_nonblocking(sock);
#endif

  memset (&addr, 0, sizeof (struct sockaddr_un));
  addr.sun_family = AF_UNIX;
  strncpy (addr.sun_path, vclient->path, strlen (vclient->path));
#ifdef HAVE_SUN_LEN
  len = addr.sun_len = SUN_LEN(&addr);
#else
  len = sizeof (addr.sun_family) + strlen (addr.sun_path);
#endif /* HAVE_SUN_LEN */

  ret = connect (sock, (struct sockaddr *) &addr, len);
  if (ret < 0)
    {
#ifdef DEBUG
      syslog(LOG_ERR, "vtysh_connect(%s): connect = %s\n", vclient->path,
	      safe_strerror(errno));
#endif /* DEBUG */
      close (sock);
      return -1;
    }
  vclient->fd = sock;

  return 0;
}

int
vtysh_connect_all(const char *daemon_name)
{
  u_int i;
  int rc = 0;
  int matches = 0;

  for (i = 0; i < VTYSH_INDEX_MAX; i++)
    {
      if (!daemon_name || !strcmp(daemon_name, vtysh_client[i].name))
	{
	  matches++;
	  if (vtysh_connect(&vtysh_client[i]) == 0)
	    rc++;
	  /* We need direct access to ripd in vtysh_exit_ripd_only. */
	  if (vtysh_client[i].flag == VTYSH_RIPD)
	    ripd_client = &vtysh_client[i];
	}
    }
  if (!matches)
    syslog(LOG_NOTICE, "Error: no daemons match name %s!\n", daemon_name);
  return rc;
}

/* To disable readline's filename completion. */
static char *
vtysh_completion_entry_function (const char *ignore, int invoking_key)
{
  return NULL;
}
static int syslog_start=1;
int
vtysh_rl_stop (void)/*CID 11360 (#1 of 1): Missing return statement (MISSING_RETURN)*/
{
	if(syslog_start)
	  {
		system ("syslog_stop.sh");
		syslog_start=0;
	}  
	return 0;
}
int
vtysh_rl_start (void)
{
	if(!syslog_start)
	  {
		system ("syslog_start.sh");
		syslog_start=1;
	
	}
	return 0;
}

void
vtysh_readline_init (void)
{
  /* readline related settings. */
  rl_bind_key ('?', (Function *) vtysh_rl_describe);
/* rl_bind_key ('~', (Function *) vtysh_rl_stop);
  rl_bind_key ('!', (Function *) vtysh_rl_start);*/
  rl_completion_entry_function = vtysh_completion_entry_function;
  rl_attempted_completion_function = (CPPFunction *)new_completion;
  /* do not append space after completion. It will be appended
   * in new_completion() function explicitly. */
  rl_completion_append_character = '\0';
}

int distribute_info_init_once = 0;
int slot_id;
int is_active_master;
int is_master;

char *
vtysh_prompt (void)
{
  static struct utsname names;
  static char buf[100];
  char hostname_buf[64]={0};
  const char*hostname;
  extern struct host host;
  int rindex = 0;
  hostname = host.name;
  
  //get host slot id
  if(1 != distribute_info_init_once)
  {
  	FILE *fd = NULL;

	fd = fopen("/dbm/local_board/slot_id", "r");
	if (fd == NULL)
	{
		slot_id = -2;
	}else{
		fscanf(fd, "%d", &slot_id);/*
									CID 10326 (#3 of 3): Unchecked return value from library (CHECKED_RETURN).
									check_return: Calling function "fscanf(fd, "%d", &is_master)" without checking return value. 
									This library function may fail and return an error code
									*/
		fclose(fd);
		fd = NULL;
	}
	
	fd = fopen("/dbm/local_board/is_active_master", "r");
	if (fd == NULL)
	{
		slot_id = -3;
	}else{
		fscanf(fd, "%d", &is_active_master);
		
		fclose(fd);
		fd = NULL;
	}
	
	fd = fopen("/dbm/local_board/is_master", "r");
	if (fd == NULL)
	{
		slot_id = -4;
	}else{
		fscanf(fd, "%d", &is_master);
		
		fclose(fd);
		fd = NULL;
	}
  }

  if (!hostname)
    {
      if (!names.nodename[0])
	uname (&names);
	  
      hostname = names.nodename;
    }
  if(1 != is_master)
  //set hostname such as: SYSTEM slot_1>
  {
 	 sprintf(hostname_buf,"%s(%d)",hostname,slot_id);
	  
	  hostname = hostname_buf;
  }else{

	if(1 != is_active_master)
	{
		
		sprintf(hostname_buf,"%s-MS",hostname);

	}else{

		sprintf(hostname_buf,"%s-MA",hostname);

	}
	//hostname = hostname_buf;
  }
	switch (vty->node)
	  {
	  case HANSI_NODE:
		  snprintf (buf, sizeof buf, cmd_prompt (vty->node), hostname, vty->slotindex,vty->index);
		  break;
	  case LOCAL_HANSI_NODE:
		  snprintf (buf, sizeof buf, cmd_prompt (vty->node), hostname, vty->slotindex,vty->index);
		  break;
      case SNMP_SLOT_NODE:	
	  	  snprintf (buf, sizeof buf, cmd_prompt (vty->node), hostname, vty->slotindex);
	  	  break;
	  case HANSI_EAG_NODE:
      case LOCAL_HANSI_EAG_NODE:
	  case HANSI_PDC_NODE:
      case LOCAL_HANSI_PDC_NODE:
	  case HANSI_RDC_NODE:
      case LOCAL_HANSI_RDC_NODE:
	  case HANSI_IU_NODE:			/*add for femto*/
	  case HANSI_IUH_NODE:
	  case LOCAL_HANSI_IU_NODE:
	  case LOCAL_HANSI_IUH_NODE:
	  	  snprintf (buf, sizeof buf, cmd_prompt (vty->node), hostname, vty->slotindex,vty->index);
	  	  break;
#ifdef _D_WCPSS_
	  case WLAN_NODE:
	  case WTP_NODE:
	  case SECURITY_NODE:
	  case WQOS_NODE:
	  case ACIPLIST_NODE:
	  case EBR_NODE:
	  case AP_GROUP_NODE:
	  case AP_GROUP_WTP_NODE:
	  case AP_GROUP_RADIO_NODE:
		  snprintf (buf, sizeof buf, cmd_prompt (vty->node), hostname, vty->index);
		  break;
	  case RADIO_NODE:
	  	  /*rindex = vty->index;*/
	  	  rindex =(int) vty->index;/*gujd: 2012-02-23, pm 5:49. In order to decrease the warning when make img .*/
		  snprintf (buf, sizeof buf, cmd_prompt (vty->node), hostname, rindex/4,rindex%4);		
		  break;
	  case	  HANSI_SECURITY_NODE:
	  case	  HANSI_ACIPLIST_NODE:
	  case	  HANSI_WLAN_NODE:			  /* WLAN node */
	  case	  HANSI_WTP_NODE:
	  case	  HANSI_WQOS_NODE:		  /*wireless qos node.*/
	  case	  HANSI_EBR_NODE:
	  case    HANSI_CWTUNNEL_NODE:
	  case	  HANSI_AP_GROUP_NODE:	  
	  case 	  HANSI_AP_GROUP_WTP_NODE:
	  case	  HANSI_AP_GROUP_RADIO_NODE:
	  case	  HANSI_POOL_NODE:	
	  case	  HANSI_POOLV6_NODE:	
	  case	  HANSI_AC_GROUP_NODE:		/*wireless ac_group_node*/
	  case	  HANSI_PPPOE_DEVICE_NODE:			/* new pppoe node */
		  snprintf (buf, sizeof buf, cmd_prompt (vty->node), hostname, vty->slotindex,vty->index, vty->index_sub);
		  break;		
	  case	  HANSI_RADIO_NODE:
	  	 /* rindex = vty->index_sub;*/
	  	  rindex =(int) vty->index_sub;/*gujd: 2012-02-23, pm 5:49. In order to decrease the warning when make img .*/
		  snprintf (buf, sizeof buf, cmd_prompt (vty->node), hostname, vty->slotindex,vty->index,rindex/4,rindex%4);		
		  break;
	
 	case	LOCAL_HANSI_SECURITY_NODE:
 	case	LOCAL_HANSI_ACIPLIST_NODE:
 	case	LOCAL_HANSI_WLAN_NODE:			/* WLAN node */
 	case	LOCAL_HANSI_WTP_NODE:
 	case	LOCAL_HANSI_WQOS_NODE:		/*wireless qos node.*/
 	case	LOCAL_HANSI_EBR_NODE:
 	case	LOCAL_HANSI_CWTUNNEL_NODE:
 	case	LOCAL_HANSI_AP_GROUP_NODE:	
 	case	LOCAL_HANSI_AP_GROUP_WTP_NODE:
 	case	LOCAL_HANSI_AP_GROUP_RADIO_NODE:
	case	LOCAL_HANSI_POOL_NODE:	
	case	LOCAL_HANSI_POOLV6_NODE:
	case	LOCAL_HANSI_AC_GROUP_NODE:		/*wireless ac_group_node*/
	case	LOCAL_HANSI_PPPOE_DEVICE_NODE:			/* new pppoe node */
 		snprintf (buf, sizeof buf, cmd_prompt (vty->node), hostname, vty->slotindex,vty->index, vty->index_sub);
 		break;		  
 	case	LOCAL_HANSI_RADIO_NODE:
 		/*rindex = vty->index_sub;*/
		rindex =(int) vty->index_sub;/*gujd: 2012-02-23, pm 5:49. In order to decrease the warning when make img .*/
 		snprintf (buf, sizeof buf, cmd_prompt (vty->node), hostname, vty->slotindex,vty->index,rindex/4,rindex%4); 	  
 		break;
#endif    
	  default:
		  snprintf (buf, sizeof buf, cmd_prompt (vty->node), hostname);
		break;
	  }


  return buf;
}


void
vtysh_init_vty (void)
{
  /* Make vty structure. */
  vty = vty_new ();
  vty->type = VTY_SHELL;
  vty->node = VIEW_NODE;


  /* Initialize commands. */
  cmd_init (0);

  /* Install nodes. */
//  install_node (&bgp_node, NULL, "BGP_NODE");
  install_node (&rip_node, NULL, "RIP_NODE");
  install_node (&interface_node, NULL, "INTERFACE_NODE");
  install_node (&rmap_node, NULL, "RMAP_NODE");
  install_node (&zebra_node, NULL, "ZEBRA_NODE");
#if 1
	install_node (&config_node, vtysh_config_write, "CONFIG_NODE");
#else
install_node (&config_node, NULL, "CONFIG_NODE");
#endif
//  install_node (&bgp_vpnv4_node, NULL);
//  install_node (&bgp_ipv4_node, NULL);
//  install_node (&bgp_ipv4m_node, NULL);
/* #ifdef HAVE_IPV6 */
//  install_node (&bgp_ipv6_node, NULL);
//  install_node (&bgp_ipv6m_node, NULL);
/* #endif */
  install_node (&ospf_node, NULL, "OSPF_NODE");
/* #ifdef HAVE_IPV6 */
//  install_node (&ripng_node, NULL);
//  install_node (&ospf6_node, NULL);
/* #endif */
  install_node (&hidden_debug_node, NULL, "HIDDENDEBUG_NODE");

  install_node (&keychain_node, NULL, "KEYCHAIN_NODE");
  install_node (&keychain_key_node, NULL, "KEYCHAIN_KEY_NODE");
//  install_node (&isis_node, NULL);
  install_node (&vty_node, NULL, "VTY_NODE");
  vtysh_install_default (VIEW_NODE);
  vtysh_install_default (ENABLE_NODE);
  vtysh_install_default (CONFIG_NODE);
//  vtysh_install_default (BGP_NODE);
  vtysh_install_default (RIP_NODE);
  vtysh_install_default (INTERFACE_NODE);
  vtysh_install_default (RMAP_NODE);
#if 0
  vtysh_install_default (ZEBRA_NODE);
#endif
//  vtysh_install_default (BGP_VPNV4_NODE);
//  vtysh_install_default (BGP_IPV4_NODE);
// vtysh_install_default (BGP_IPV4M_NODE);
//  vtysh_install_default (BGP_IPV6_NODE);
//  vtysh_install_default (BGP_IPV6M_NODE);
  vtysh_install_default (OSPF_NODE);
//  vtysh_install_default (RIPNG_NODE);
//  vtysh_install_default (OSPF6_NODE);
//  vtysh_install_default (ISIS_NODE);
  vtysh_install_default (KEYCHAIN_NODE);
  vtysh_install_default (KEYCHAIN_KEY_NODE);
  vtysh_install_default (VTY_NODE);
  vtysh_install_default (HIDDENDEBUG_NODE);
  
  install_element (VIEW_NODE, &vtysh_enable_cmd);
  install_element (ENABLE_NODE, &vtysh_config_terminal_cmd);
	/* start - added by zhengbo 2011-09-19*/
	install_element(CONFIG_NODE, &config_terminal_passwd_cmd);
	install_element(CONFIG_NODE, &no_config_terminal_passwd_cmd);
	/* end - added by zhengbo 2011-09-19*/
  install_element (ENABLE_NODE, &vtysh_disable_cmd);

  /* "exit" command. */
  install_element (VIEW_NODE, &vtysh_exit_all_cmd);
  install_element (VIEW_NODE, &vtysh_quit_all_cmd);
  install_element (CONFIG_NODE, &vtysh_exit_all_cmd);
   install_element (CONFIG_NODE, &vtysh_quit_all_cmd); 
  install_element (ENABLE_NODE, &vtysh_exit_all_cmd);
  install_element (ENABLE_NODE, &vtysh_quit_all_cmd);
  install_element (RIP_NODE, &vtysh_exit_ripd_cmd);
  install_element (RIP_NODE, &vtysh_quit_ripd_cmd);
//  install_element (RIPNG_NODE, &vtysh_exit_ripngd_cmd);
//  install_element (RIPNG_NODE, &vtysh_quit_ripngd_cmd);
  install_element (OSPF_NODE, &vtysh_exit_ospfd_cmd);
  install_element (OSPF_NODE, &vtysh_quit_ospfd_cmd);
//  install_element (OSPF6_NODE, &vtysh_exit_ospf6d_cmd);
//  install_element (OSPF6_NODE, &vtysh_quit_ospf6d_cmd);
//  install_element (BGP_NODE, &vtysh_exit_bgpd_cmd);
//  install_element (BGP_NODE, &vtysh_quit_bgpd_cmd);
//  install_element (BGP_VPNV4_NODE, &vtysh_exit_bgpd_cmd);
//  install_element (BGP_VPNV4_NODE, &vtysh_quit_bgpd_cmd);
//  install_element (BGP_IPV4_NODE, &vtysh_exit_bgpd_cmd);
//  install_element (BGP_IPV4_NODE, &vtysh_quit_bgpd_cmd);
//  install_element (BGP_IPV4M_NODE, &vtysh_exit_bgpd_cmd);
//  install_element (BGP_IPV4M_NODE, &vtysh_quit_bgpd_cmd);
//  install_element (BGP_IPV6_NODE, &vtysh_exit_bgpd_cmd);
//  install_element (BGP_IPV6_NODE, &vtysh_quit_bgpd_cmd);
//  install_element (BGP_IPV6M_NODE, &vtysh_exit_bgpd_cmd);
//  install_element (BGP_IPV6M_NODE, &vtysh_quit_bgpd_cmd);
//  install_element (ISIS_NODE, &vtysh_exit_isisd_cmd);
//  install_element (ISIS_NODE, &vtysh_quit_isisd_cmd);
  install_element (KEYCHAIN_NODE, &vtysh_exit_ripd_cmd);
  install_element (KEYCHAIN_NODE, &vtysh_quit_ripd_cmd);
  install_element (KEYCHAIN_KEY_NODE, &vtysh_exit_ripd_cmd);
  install_element (KEYCHAIN_KEY_NODE, &vtysh_quit_ripd_cmd);
  install_element (RMAP_NODE, &vtysh_exit_rmap_cmd);
  install_element (RMAP_NODE, &vtysh_quit_rmap_cmd);
  install_element (VTY_NODE, &vtysh_exit_line_vty_cmd);
  install_element (VTY_NODE, &vtysh_quit_line_vty_cmd);

  /* "end" command. */
  extern struct cmd_element vtysh_end_all_cmd ;
  install_element (CONFIG_NODE, &vtysh_end_all_cmd);
  install_element (ENABLE_NODE, &vtysh_end_all_cmd);
  install_element (RIP_NODE, &vtysh_end_all_cmd);
//  install_element (RIPNG_NODE, &vtysh_end_all_cmd);
  install_element (OSPF_NODE, &vtysh_end_all_cmd);
//  install_element (OSPF6_NODE, &vtysh_end_all_cmd);
//  install_element (BGP_NODE, &vtysh_end_all_cmd);
//  install_element (BGP_IPV4_NODE, &vtysh_end_all_cmd);
//  install_element (BGP_IPV4M_NODE, &vtysh_end_all_cmd);
//  install_element (BGP_VPNV4_NODE, &vtysh_end_all_cmd);
//  install_element (BGP_IPV6_NODE, &vtysh_end_all_cmd);
//  install_element (BGP_IPV6M_NODE, &vtysh_end_all_cmd);
//  install_element (ISIS_NODE, &vtysh_end_all_cmd);
  install_element (KEYCHAIN_NODE, &vtysh_end_all_cmd);
  install_element (KEYCHAIN_KEY_NODE, &vtysh_end_all_cmd);
  install_element (RMAP_NODE, &vtysh_end_all_cmd);
  install_element (VTY_NODE, &vtysh_end_all_cmd);

  install_element (INTERFACE_NODE, &interface_desc_cmd);
  install_element (INTERFACE_NODE, &no_interface_desc_cmd);
  
  install_element (INTERFACE_NODE, &interface_desc_local_cmd);

  install_element (INTERFACE_NODE, &vtysh_end_all_cmd);
  install_element (INTERFACE_NODE, &vtysh_exit_interface_cmd);
  install_element (INTERFACE_NODE, &vtysh_quit_interface_cmd);
  install_element (CONFIG_NODE, &router_rip_cmd);
#ifdef HAVE_IPV6
//  install_element (CONFIG_NODE, &router_ripng_cmd);
//  install_element (CONFIG_NODE, &router_ospf6_cmd);
#endif
  install_element (CONFIG_NODE, &router_ospf_cmd);
#ifdef HAVE_IPV6
//  install_element (CONFIG_NODE, &router_ospf6_cmd);
#endif
 // install_element (CONFIG_NODE, &router_isis_cmd);
//  install_element (CONFIG_NODE, &router_bgp_cmd);
//  install_element (BGP_NODE, &address_family_vpnv4_cmd);
//  install_element (BGP_NODE, &address_family_vpnv4_unicast_cmd);
//  install_element (BGP_NODE, &address_family_ipv4_unicast_cmd);
//  install_element (BGP_NODE, &address_family_ipv4_multicast_cmd);
#ifdef HAVE_IPV6
//  install_element (BGP_NODE, &address_family_ipv6_cmd);
//  install_element (BGP_NODE, &address_family_ipv6_unicast_cmd);
#endif
//  install_element (BGP_VPNV4_NODE, &exit_address_family_cmd);
//  install_element (BGP_IPV4_NODE, &exit_address_family_cmd);
//  install_element (BGP_IPV4M_NODE, &exit_address_family_cmd);
//  install_element (BGP_IPV6_NODE, &exit_address_family_cmd);
//  install_element (BGP_IPV6M_NODE, &exit_address_family_cmd);
  install_element (CONFIG_NODE, &key_chain_cmd);
  install_element (CONFIG_NODE, &route_map_cmd);
  //install_element (CONFIG_NODE, &vtysh_line_vty_cmd);
  install_element (KEYCHAIN_NODE, &key_cmd);
  install_element (KEYCHAIN_NODE, &key_chain_cmd);
  install_element (KEYCHAIN_KEY_NODE, &key_chain_cmd);
 /* 
  install_element (CONFIG_NODE, &vtysh_interface_cmd);
  install_element (CONFIG_NODE, &vtysh_no_interface_cmd);
*/
  install_element (ENABLE_NODE, &vtysh_show_running_config_cmd);
  install_element (VIEW_NODE, &vtysh_show_running_config_cmd);
//  install_element (ENABLE_NODE, &vtysh_copy_runningconfig_startupconfig_cmd);
//  install_element (ENABLE_NODE, &vtysh_write_file_cmd);
  install_element (ENABLE_NODE, &vtysh_write_cmd);

  /* "write terminal" command. */
//  install_element (ENABLE_NODE, &vtysh_write_terminal_cmd);
		install_element (ENABLE_NODE, &vtysh_write_terminal_cmd1);

/* deleted bye flash for delete command 
  "service integrated-vtysh-config" and 
  "no service integrated-vtysh-config"
  install_element (CONFIG_NODE, &vtysh_integrated_config_cmd);
  install_element (CONFIG_NODE, &no_vtysh_integrated_config_cmd);
*/
  /* "write memory" command. */
//  install_element (ENABLE_NODE, &vtysh_write_memory_cmd);
  install_element (ENABLE_NODE, &vtysh_earse_memory_cmd);

  install_element (VIEW_NODE, &vtysh_terminal_length_cmd);
  install_element (ENABLE_NODE, &vtysh_terminal_length_cmd);
  install_element (VIEW_NODE, &vtysh_terminal_no_length_cmd);
  install_element (ENABLE_NODE, &vtysh_terminal_no_length_cmd);
//  install_element (VIEW_NODE, &vtysh_show_daemons_cmd);
//  install_element (ENABLE_NODE, &vtysh_show_daemons_cmd);
   install_element (HIDDENDEBUG_NODE, &vtysh_show_daemons_cmd);

  /*gjd : add for Distribute System*/  	
  install_element (HIDDENDEBUG_NODE, &vtysh_enable_cmd_autelan);
/*  install_element (HIDDENDEBUG_NODE, &vtysh_disable_cmd_autelan);*/

  //install_element (VIEW_NODE, &vtysh_ping_cmd);
  //install_element(VIEW_NODE,&vtysh_ping_count_cmd);
  //install_element(VIEW_NODE,&vtysh_ping_time_cmd);
  //install_element (VIEW_NODE, &vtysh_ping_ip_cmd);
  //install_element (VIEW_NODE, &vtysh_traceroute_cmd);
  //install_element (VIEW_NODE, &vtysh_traceroute_udp_cmd);
  //install_element (VIEW_NODE, &vtysh_traceroute_ip_cmd);
  //install_element (VIEW_NODE, &vtysh_traceroute_ip_udp_cmd);
#ifdef HAVE_IPV6
 // install_element (VIEW_NODE, &vtysh_ping6_cmd);
 // install_element (VIEW_NODE, &vtysh_traceroute6_cmd);
#endif
  //install_element (VIEW_NODE, &vtysh_telnet_cmd);
  //install_element (VIEW_NODE, &vtysh_telnet_port_cmd);
  //install_element (VIEW_NODE, &vtysh_ssh_cmd);
  //install_element (ENABLE_NODE, &vtysh_ping_cmd);
	//install_element(ENABLE_NODE,&vtysh_ping_count_cmd);
	//install_element(ENABLE_NODE,&vtysh_ping_time_cmd);
	//install_element (ENABLE_NODE, &vtysh_ping_ip_cmd);
	//install_element (ENABLE_NODE, &vtysh_traceroute_cmd);
	//install_element (ENABLE_NODE, &vtysh_traceroute_udp_cmd);
	//install_element (ENABLE_NODE, &vtysh_traceroute_ip_cmd);
	//install_element (ENABLE_NODE, &vtysh_traceroute_ip_udp_cmd);
#ifdef HAVE_IPV6
	//install_element (ENABLE_NODE, &vtysh_ping6_cmd);
	//install_element (ENABLE_NODE, &vtysh_traceroute6_cmd);
#endif
	//install_element (ENABLE_NODE, &vtysh_telnet_cmd);
	//install_element (ENABLE_NODE, &vtysh_telnet_port_cmd);
	//install_element (ENABLE_NODE, &vtysh_ssh_cmd);
	
	//install_element (CONFIG_NODE, &vtysh_ping_cmd);
	//install_element(CONFIG_NODE,&vtysh_ping_count_cmd);
	//install_element(CONFIG_NODE,&vtysh_ping_time_cmd);
	//install_element (CONFIG_NODE, &vtysh_ping_ip_cmd);
	
	//install_element (CONFIG_NODE, &vtysh_traceroute_cmd);
	//install_element (CONFIG_NODE, &vtysh_traceroute_udp_cmd);
	//install_element (CONFIG_NODE, &vtysh_traceroute_ip_udp_cmd);
#ifdef HAVE_IPV6
	//install_element (CONFIG_NODE, &vtysh_ping6_cmd);
	//install_element (CONFIG_NODE, &vtysh_traceroute6_cmd);
#endif
	//install_element (CONFIG_NODE, &vtysh_telnet_cmd);
	//install_element (CONFIG_NODE, &vtysh_telnet_port_cmd);
	//install_element (CONFIG_NODE, &vtysh_ssh_cmd);
	
	//install_element (INTERFACE_NODE, &vtysh_ping_cmd);
	//install_element(INTERFACE_NODE,&vtysh_ping_count_cmd);
	//install_element(INTERFACE_NODE,&vtysh_ping_time_cmd);
	//install_element (INTERFACE_NODE, &vtysh_ping_ip_cmd);
	//install_element (INTERFACE_NODE, &vtysh_traceroute_udp_cmd);
	//install_element (INTERFACE_NODE, &vtysh_traceroute_ip_udp_cmd);
#ifdef HAVE_IPV6
	//install_element (INTERFACE_NODE, &vtysh_ping6_cmd);
	//install_element (INTERFACE_NODE, &vtysh_traceroute6_cmd);
#endif
	//install_element (INTERFACE_NODE, &vtysh_telnet_cmd);
	//install_element (INTERFACE_NODE, &vtysh_telnet_port_cmd);
	//install_element (INTERFACE_NODE, &vtysh_ssh_cmd);
  install_element (HIDDENDEBUG_NODE, &vtysh_start_shell_cmd);
  install_element (HIDDENDEBUG_NODE, &vtysh_shell_exec_cmd);
  install_element (HIDDENDEBUG_NODE, &hidden_debug_exit_cmd);
  //install_element (HIDDENDEBUG_NODE, &vtysh_ping_cmd);
  //install_element(HIDDENDEBUG_NODE,&vtysh_ping_count_cmd);
  //install_element(HIDDENDEBUG_NODE,&vtysh_ping_time_cmd);
  //install_element (HIDDENDEBUG_NODE, &vtysh_ping_ip_cmd);
  //install_element (HIDDENDEBUG_NODE, &vtysh_traceroute_udp_cmd);
  //install_element (HIDDENDEBUG_NODE, &vtysh_traceroute_ip_udp_cmd);
#ifdef HAVE_IPV6
  //install_element (HIDDENDEBUG_NODE, &vtysh_ping6_cmd);
 // install_element (HIDDENDEBUG_NODE, &vtysh_traceroute6_cmd);
#endif
  //install_element (HIDDENDEBUG_NODE, &vtysh_telnet_cmd);
  //install_element (HIDDENDEBUG_NODE, &vtysh_telnet_port_cmd);
  //install_element (HIDDENDEBUG_NODE, &vtysh_ssh_cmd);
  install_element (HIDDENDEBUG_NODE, &rip_max_socket_buffer_cmd);


//  install_element (ENABLE_NODE, &vtysh_start_bash_cmd);
//  install_element (ENABLE_NODE, &vtysh_start_zsh_cmd);
  install_element (VIEW_NODE, &vtysh_show_memory_cmd);
  install_element (ENABLE_NODE, &vtysh_show_memory_cmd);

  install_element (HIDDENDEBUG_NODE, &vtysh_show_memory_cmd);
  /* Logging */
//  install_element (ENABLE_NODE, &vtysh_show_logging_cmd);
//  install_element (VIEW_NODE, &vtysh_show_logging_cmd);
//  install_element (CONFIG_NODE, &vtysh_log_stdout_cmd);
//  install_element (CONFIG_NODE, &vtysh_log_stdout_level_cmd);
//  install_element (CONFIG_NODE, &no_vtysh_log_stdout_cmd);
//  install_element (CONFIG_NODE, &vtysh_log_file_cmd);
//  install_element (CONFIG_NODE, &vtysh_log_file_level_cmd);
//  install_element (CONFIG_NODE, &no_vtysh_log_file_cmd);
//  install_element (CONFIG_NODE, &no_vtysh_log_file_level_cmd);
//  install_element (CONFIG_NODE, &vtysh_log_monitor_cmd);
//  install_element (CONFIG_NODE, &vtysh_log_monitor_level_cmd);
//  install_element (CONFIG_NODE, &no_vtysh_log_monitor_cmd);
//  install_element (CONFIG_NODE, &vtysh_log_syslog_cmd);
//  install_element (CONFIG_NODE, &vtysh_log_syslog_level_cmd);
//  install_element (CONFIG_NODE, &no_vtysh_log_syslog_cmd);
//  install_element (CONFIG_NODE, &vtysh_log_trap_cmd);
//  install_element (CONFIG_NODE, &no_vtysh_log_trap_cmd);
//  install_element (CONFIG_NODE, &vtysh_log_facility_cmd);
//  install_element (CONFIG_NODE, &no_vtysh_log_facility_cmd);
//  install_element (CONFIG_NODE, &vtysh_log_record_priority_cmd);
//  install_element (CONFIG_NODE, &no_vtysh_log_record_priority_cmd);

  	install_element(ENABLE_NODE,&vtysh_show_bootconfig_cmd);
	install_element(ENABLE_NODE,&vtysh_del_bootconfig_cmd);
	install_element(ENABLE_NODE,&vtysh_set_bootconfig_cmd);
#if 0
  install_element (CONFIG_NODE, &vtysh_service_password_encrypt_cmd);
  install_element (CONFIG_NODE, &no_vtysh_service_password_encrypt_cmd);
  install_element (CONFIG_NODE, &vtysh_password_cmd);
  install_element (CONFIG_NODE, &vtysh_password_text_cmd);
  install_element (CONFIG_NODE, &vtysh_enable_password_cmd);
  install_element (CONFIG_NODE, &vtysh_enable_password_text_cmd);
  install_element (CONFIG_NODE, &no_vtysh_enable_password_cmd);
#endif
  install_element (CONFIG_NODE, &vtysh_no_hostname_cmd);
  install_element (CONFIG_NODE, &vtysh_hostname_cmd);
  install_element (ENABLE_NODE, &vtysh_no_hostname_cmd);
  install_element (ENABLE_NODE, &vtysh_hostname_cmd);
  
  /*gujd:2013-03-01, am 10:27. Add set hostname independent between every board.*/  
#ifdef DISTRIBUT
  install_element (CONFIG_NODE, &vtysh_hostname_independent_cmd);
  install_element (ENABLE_NODE, &vtysh_hostname_independent_cmd);
  install_element (CONFIG_NODE, &vtysh_no_hostname_independent_cmd);
  install_element (ENABLE_NODE, &vtysh_no_hostname_independent_cmd);
 #endif 

  install_element (ENABLE_NODE, &show_process_cmd);
  install_element (VIEW_NODE, &show_process_cmd);
  install_element (ENABLE_NODE, &show_process_all_cmd);
  install_element (VIEW_NODE, &show_process_all_cmd);
  install_element (HIDDENDEBUG_NODE, &show_process_cmd);
  install_element (HIDDENDEBUG_NODE, &show_process_all_cmd);

  install_element (ENABLE_NODE, &show_process_count_cmd);
  install_element (HIDDENDEBUG_NODE, &show_process_count_cmd);
  install_element (VIEW_NODE, &show_process_count_cmd);
  
  install_element (ENABLE_NODE, &show_process_by_bsd_cmd);
  install_element (HIDDENDEBUG_NODE, &show_process_by_bsd_cmd);
  install_element (VIEW_NODE, &show_process_by_bsd_cmd);

#if 0 /*dongshu for del cli_syslog cmd*/
  install_element (ENABLE_NODE, &set_vtysh_cli_log_func_cmd);  
  install_element (CONFIG_NODE, &set_vtysh_cli_log_func_cmd);

  install_element (ENABLE_NODE, &show_vtysh_cli_log_func_cmd);  
  install_element (CONFIG_NODE, &show_vtysh_cli_log_func_cmd);
#endif /*end*/
  install_element (CONFIG_NODE, &set_idle_time_func_cmd);
  install_element (CONFIG_NODE, &no_set_idle_time_func_cmd);
  install_element (ENABLE_NODE, &set_idle_time_func_cmd);
  install_element (ENABLE_NODE, &no_set_idle_time_func_cmd);
  install_element (VIEW_NODE, &show_idle_time_func_cmd);
  install_element (CONFIG_NODE, &show_idle_time_func_cmd);
  install_element (ENABLE_NODE, &show_idle_time_func_cmd);

  
}
