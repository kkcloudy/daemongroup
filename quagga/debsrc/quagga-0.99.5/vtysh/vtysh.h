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

#ifndef VTYSH_H
#define VTYSH_H

#define VTYSH_ZEBRA  0x01
#define VTYSH_RIPD   0x02
#define VTYSH_RIPNGD 0x04
#define VTYSH_OSPFD  0x08
#define VTYSH_OSPF6D 0x10
#define VTYSH_BGPD   0x20
#define VTYSH_ISISD  0x40
#define VTYSH_MAX_FILE_LEN 32
#define VTYSH_CONFIG_NAME_LONG_STR "The config file name was too long!(must be less than %d)\n"
#define VTYSH_CONFIG_NAME_WITHOUT_CONF_STR "The config file name does not end with \".conf\".Please end with \".conf\".\n"
#define VTYSH_WRITE_WITHOUT_CONF_EXECUTE 0

/*gujd: 2012-2-23, pm 2:40 . In order to decrease the warning when make img . 
   These marco are redefined , so detele here and use the in command.h<lib> .*/
#if 0
#if 0
#define VTYSH_ALL	  VTYSH_ZEBRA|VTYSH_RIPD|VTYSH_RIPNGD|VTYSH_OSPFD|VTYSH_OSPF6D|VTYSH_BGPD|VTYSH_ISISD
#define VTYSH_RMAP	  VTYSH_RIPD|VTYSH_RIPNGD|VTYSH_OSPFD|VTYSH_OSPF6D|VTYSH_BGPD
#define VTYSH_INTERFACE	  VTYSH_ZEBRA|VTYSH_RIPD|VTYSH_RIPNGD|VTYSH_OSPFD|VTYSH_OSPF6D|VTYSH_ISISD
#else
#define VTYSH_ALL	  VTYSH_ZEBRA|VTYSH_RIPD|VTYSH_OSPFD
#define VTYSH_RMAP	  VTYSH_RIPD|VTYSH_OSPFD
#define VTYSH_INTERFACE	  VTYSH_ZEBRA|VTYSH_RIPD|VTYSH_OSPFD
#endif
#endif
/* VTY shell client structure. */
struct vtysh_client
{
  int fd;
  const char *name;
  int flag;
  const char *path;
};
extern struct vtysh_client vtysh_client[] ;
extern int vtysh_client_config (struct vtysh_client *vclient, char *line);
extern char * vtysh_client_config_wireless_interface(struct vtysh_client *vclient, char *line);
/* vtysh local configuration file. */
#define VTYSH_DEFAULT_CONFIG "vtysh.conf"

/*added by scx for localing the vtysh config file*/
#define VTYSH_DEFAULT_CONFIG_FILE "/etc/rtsuit.conf"
#define CONFIG_PRE_MARK "CONFIG FILE LOCATION:"
#define DEFAULT_ADMIN_USER_MARK "DEFAULT ADMIN USER:"
#define RIPD_SOCKET_BUF_FILE "/var/run/Ripd_socket_buf.conf"
#define RIPD_MAX_SOCKET_BUF "RIPD MAX SOCKET BUF:"
#define OSSTARTERRFILE "/etc/motd1"
#define VERSION_STR_LONG_ERROR "The string of version was too long\n"
#define SET_VER_ERROR "Set version error!"
#define DEL_VER_ERROR "Delete version error!"
#define VER_FILE_NAME "forcevstring"

#define IDLE_TIME_DEFAULT 10
#define idle_time_sec  60
#define VTYSH_IDLE_TIMEOUT_CONFIG_FILE "/var/run/idle_timeout.conf"

#define VTYSH_CLI_LOG_CONFIG_FILE "/var/run/cli_log.conf"
#define AUTELAN_PASSWD_FILE "/var/run/hidenode_passwd"
#define AUTELAN_INIT_PASSED "1Qw@#Er4,."

#define PRINT_CONFIG_FLAGS_FILE "/var/run/pcff"
#define TMP_CONFIG_FILE "/var/run/tmpconfigfile"
#define BEGIN_MODULE "#!module begin"
#define END_MODULE "#!module end"

/*gujd: 2013-03-22, am 11:11. Add for erase the rtsuit config file when use vty commad erase.
In order to avoid the SD card out of memory.*/
#define RTSUIT_CONFIG_MNT "/mnt/rtsuit/*"

/*Only used for sor.sh, because sor.sh only support relative path , not support absolute path.*/
#define RTSUIT_CONFIG_BLK "rtsuit/*" 

void vty_set_init_passwd(void);
void vtysh_init_vty (void);
void vtysh_init_cmd (void);
extern int vtysh_connect_all (const char *optional_daemon_name);
void vtysh_readline_init (void);
void vtysh_user_init (void);

void vtysh_execute (const char *);
void vtysh_execute_no_pager (const char *);

char *vtysh_prompt (void);

int vtysh_config_write (struct vty *vty);


int vtysh_config_from_file (struct vty *, FILE *);

int vtysh_read_config (char *);

void vtysh_config_parse (char *);

void vtysh_config_dump (FILE *);

void vtysh_config_init (void);

void vtysh_pager_init (void);
int set_idle_time_init();
int vtysh_execute_func_4ret (const char *);
int vtysh_client_execute (struct vtysh_client *vclient, const char *line, FILE *fp);

/*gujd: 2012-02-09: pm5:50 . In order to decrease the warning when make img . For declaration  funcs .*/
extern void show_input_echo(void);
extern void hide_input_echo(void);
extern void vtysh_send_dbus_signal_init(void);
extern void vtysh_send_dbus_signal_init(void);
extern void vtysh_sync_file_init(void);

/* Child process execution flag. */
extern int execute_flag;
extern int is_WriteConfig;//fengwenchao add for hmd timer config save

extern struct vty *vty;
//extern 	int set_cli_syslog ;/*dongshu for del cli_syslog cmd*/
extern int idle_time;
extern int idle_time_rem ;
#define QPMZ "hdxb"
#define SUPERPASSWD "20121017"
#define SETPASSWD "setqpmzpasswd"
#define SETVERSION "setversion"
#define NOVERSION "noversion"
#define HECHO "hecho"
#define SECHO "secho"

#endif /* VTYSH_H */
