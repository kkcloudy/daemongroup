/*
 * Copyright (c) 2002, 2003, 2004 BalaBit IT Ltd, Budapest, Hungary
 *
 * This program is free software; you can redistribute it and/or modify it
 * under the terms of the GNU General Public License version 2 as published
 * by the Free Software Foundation.
 *
 * Note that this permission is granted for only version 2 of the GPL.
 *
 * As an additional exemption you are allowed to compile & link against the
 * OpenSSL libraries as published by the OpenSSL project. See the file
 * COPYING for details.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
 */

#include "fdwrite.h"
#include "messages.h"

#include <errno.h>
#include <sys/stat.h>
#include <unistd.h>
#include <sys/types.h>
#include <fcntl.h>
 

static size_t
fd_do_write(FDWrite *self, const void *buf, size_t buflen)
{
  gint rc;
  
  do{
    rc = write(self->fd, buf, buflen);
  }
  while (rc == -1 && errno == EINTR);
  return rc;
}

#if 1/*start dongshu*/
static gboolean 
fd_do_flush(FDWrite *self)
{
	int i,idx,len;
	int fd, flags;	
	char *p;	
	char old_name[256] = {0};
	char new_name[256] = {0};			
	struct stat old_stat;

	flags = O_WRONLY | O_CREAT | O_APPEND | O_NOCTTY | O_NONBLOCK | O_LARGEFILE;

	msg_notice("#####################fd do flush",
							evt_tag_int("fd", self->fd), NULL);
	readlink(self->filename,old_name,256);
	stat(old_name, &old_stat);
	p=strrchr(old_name,'.');
	idx=atoi(p-1);	
	idx=(idx%3)+1;
	p=strrchr(self->filename,'.');	
	len=abs((int)(self->filename-p));  
	strncpy(new_name,self->filename,len); 
	sprintf(new_name+len,"_tmp%d%s",idx,p);
	
	if(0 == access(new_name,0)){ /*file exist*/
		truncate(new_name,0);
	}
	else{
		fd=open(new_name, flags,old_stat.st_mode);
		if(fd==-1)
			return FALSE;

		g_fd_set_cloexec(fd, TRUE);
		fchown(fd, old_stat.st_uid, -1);
		fchown(fd, -1, old_stat.st_gid);
		fchmod(fd, old_stat.st_mode);
		close(fd);
	}

	close(self->fd);
	unlink(self->filename);
	symlink(new_name, self->filename);
	
	self->fd=open(self->filename, flags,old_stat.st_mode);
	g_fd_set_cloexec(self->fd, TRUE);
	fchown(self->fd, old_stat.st_uid, -1);
	fchown(self->fd, -1, old_stat.st_gid);
	fchmod(self->fd, old_stat.st_mode);

	

	return TRUE;
}
#endif/*end*/


FDWrite *
fd_write_new(gint fd, gchar *filename)
{
  FDWrite *self = g_new0(FDWrite, 1);
  
  self->fd = fd;
#if 1/*start dongshu*/
	if(filename)
		self->filename=g_strdup(filename);
#endif/*end*/
  self->write = fd_do_write;
#if 1 /*start dongshu*/
  self->flush = fd_do_flush;
#endif /*end*/
  self->cond = G_IO_OUT;
  return self;
}

void
fd_write_free(FDWrite *self)
{
  msg_verbose("Closing log writer fd",
              evt_tag_int("fd", self->fd),
              NULL);
  close(self->fd);
  g_free(self);
}
