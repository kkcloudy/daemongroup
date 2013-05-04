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
* AsdCtrlIface.c
*
*
* CREATOR:
* autelan.software.WirelessControl. team
*
* DESCRIPTION:
* asd module
*
*
*******************************************************************************/

#include "includes.h"


#include <sys/un.h>
#include <sys/stat.h>

#include "asd.h"
#include "circle.h"
#include "config.h"
#include "ASD8021XOp.h"
#include "ASDWPAOp.h"
#include "ASDRadius/radius_client.h"
#include "ASD80211Op.h"
#include "ASDCtrlIface.h"
#include "ASDStaInfo.h"
#include "ASDAccounting.h"
#include "wcpss/wid/WID.h"
#include "wcpss/asd/asd.h"


struct wpa_ctrl_dst {
	struct wpa_ctrl_dst *next;
	struct sockaddr_un addr;
	socklen_t addrlen;
	int debug_level;
	int errors;
};


static int asd_ctrl_iface_attach(struct asd_data *wasd,
				     struct sockaddr_un *from,
				     socklen_t fromlen)
{
	struct wpa_ctrl_dst *dst;

	dst = os_zalloc(sizeof(*dst));
	if (dst == NULL)
		return -1;
	os_memcpy(&dst->addr, from, sizeof(struct sockaddr_un));
	dst->addrlen = fromlen;
	dst->debug_level = MSG_INFO;
	dst->next = wasd->ctrl_dst;
	wasd->ctrl_dst = dst;
	wpa_hexdump(MSG_DEBUG, "CTRL_IFACE monitor attached",
		    (u8 *) from->sun_path, fromlen);
	return 0;
}


static int asd_ctrl_iface_detach(struct asd_data *wasd,
				     struct sockaddr_un *from,
				     socklen_t fromlen)
{
	struct wpa_ctrl_dst *dst, *prev = NULL;

	dst = wasd->ctrl_dst;
	while (dst) {
		if (fromlen == dst->addrlen &&
		    os_memcmp(from->sun_path, dst->addr.sun_path, fromlen) ==
		    0) {
			if (prev == NULL)
				wasd->ctrl_dst = dst->next;
			else
				prev->next = dst->next;
			os_free(dst);
			wpa_hexdump(MSG_DEBUG, "CTRL_IFACE monitor detached",
				    (u8 *) from->sun_path, fromlen);
			return 0;
		}
		prev = dst;
		dst = dst->next;
	}
	return -1;
}


static int asd_ctrl_iface_level(struct asd_data *wasd,
				    struct sockaddr_un *from,
				    socklen_t fromlen,
				    char *level)
{
	struct wpa_ctrl_dst *dst;

	asd_printf(ASD_DEFAULT,MSG_DEBUG, "CTRL_IFACE LEVEL %s", level);

	dst = wasd->ctrl_dst;
	while (dst) {
		if (fromlen == dst->addrlen &&
		    os_memcmp(from->sun_path, dst->addr.sun_path, fromlen) ==
		    0) {
			wpa_hexdump(MSG_DEBUG, "CTRL_IFACE changed monitor "
				    "level", (u8 *) from->sun_path, fromlen);
			dst->debug_level = atoi(level);
			return 0;
		}
		dst = dst->next;
	}

	return -1;
}


static int asd_ctrl_iface_sta_mib(struct asd_data *wasd,
				      struct sta_info *sta,
				      char *buf, size_t buflen)
{
	int len, res, ret;

	if (sta == NULL) {
		ret = os_snprintf(buf, buflen, "FAIL\n");
		if (ret < 0 || (size_t) ret >= buflen)
			return 0;
		return ret;
	}

	len = 0;
	ret = os_snprintf(buf + len, buflen - len, MACSTR "\n",
			  MAC2STR(sta->addr));
	if (ret < 0 || (size_t) ret >= buflen - len)
		return len;
	len += ret;

	res = ieee802_11_get_mib_sta(wasd, sta, buf + len, buflen - len);
	if (res >= 0)
		len += res;
	res = wpa_get_mib_sta(sta->wpa_sm, buf + len, buflen - len);
	if (res >= 0)
		len += res;
	res = ieee802_1x_get_mib_sta(wasd, sta, buf + len, buflen - len);
	if (res >= 0)
		len += res;

	return len;
}


static int asd_ctrl_iface_sta_first(struct asd_data *wasd,
					char *buf, size_t buflen)
{
	return asd_ctrl_iface_sta_mib(wasd, wasd->sta_list, buf, buflen);
}


static int asd_ctrl_iface_sta(struct asd_data *wasd,
				  const char *txtaddr,
				  char *buf, size_t buflen)
{
	u8 addr[ETH_ALEN];
	int ret;

	if (hwaddr_aton(txtaddr, addr)) {
		ret = os_snprintf(buf, buflen, "FAIL\n");
		if (ret < 0 || (size_t) ret >= buflen)
			return 0;
		return ret;
	}
	return asd_ctrl_iface_sta_mib(wasd, ap_get_sta(wasd, addr),
					  buf, buflen);
}


static int asd_ctrl_iface_sta_next(struct asd_data *wasd,
				       const char *txtaddr,
				       char *buf, size_t buflen)
{
	u8 addr[ETH_ALEN];
	struct sta_info *sta;
	int ret;

	if (hwaddr_aton(txtaddr, addr) ||
	    (sta = ap_get_sta(wasd, addr)) == NULL) {
		ret = os_snprintf(buf, buflen, "FAIL\n");
		if (ret < 0 || (size_t) ret >= buflen)
			return 0;
		return ret;
	}		
	return asd_ctrl_iface_sta_mib(wasd, sta->next, buf, buflen);
}


static int asd_ctrl_iface_new_sta(struct asd_data *wasd,
				      const char *txtaddr)
{
	u8 addr[ETH_ALEN];
	struct sta_info *sta;

	asd_printf(ASD_DEFAULT,MSG_DEBUG, "CTRL_IFACE NEW_STA %s", txtaddr);

	if (hwaddr_aton(txtaddr, addr))
		return -1;

	sta = ap_get_sta(wasd, addr);
	if (sta)
		return 0;

	asd_printf(ASD_DEFAULT,MSG_DEBUG, "Add new STA " MACSTR " based on ctrl_iface "
		   "notification", MAC2STR(addr));
	sta = ap_sta_add(wasd, addr, 1);
	if (sta == NULL)
		return -1;

	asd_new_assoc_sta(wasd, sta, 0);
	accounting_sta_get_id(wasd, sta);
	return 0;
}


static void asd_ctrl_iface_receive(int sock, void *circle_ctx,
				       void *sock_ctx)
{
	struct asd_data *wasd = circle_ctx;
	char buf[256];
	int res;
	struct sockaddr_un from;
	socklen_t fromlen = sizeof(from);
	char *reply;
	const int reply_size = 4096;
	int reply_len;

	res = recvfrom(sock, buf, sizeof(buf) - 1, 0,
		       (struct sockaddr *) &from, &fromlen);
	if (res < 0) {
		perror("recvfrom(ctrl_iface)");
		return;
	}
	buf[res] = '\0';
	wpa_hexdump_ascii(MSG_DEBUG, "RX ctrl_iface", (u8 *) buf, res);

	reply = os_zalloc(reply_size);
	if (reply == NULL) {
		sendto(sock, "FAIL\n", 5, 0, (struct sockaddr *) &from,
		       fromlen);
		return;
	}

	os_memcpy(reply, "OK\n", 3);
	reply_len = 3;

	if (os_strcmp(buf, "PING") == 0) {
		os_memcpy(reply, "PONG\n", 5);
		reply_len = 5;
	} else if (os_strcmp(buf, "MIB") == 0) {
		reply_len = ieee802_11_get_mib(wasd, reply, reply_size);
		if (reply_len >= 0) {
			res = wpa_get_mib(wasd->wpa_auth, reply + reply_len,
					  reply_size - reply_len);
			if (res < 0)
				reply_len = -1;
			else
				reply_len += res;
		}
		if (reply_len >= 0) {
			res = ieee802_1x_get_mib(wasd, reply + reply_len,
						 reply_size - reply_len);
			if (res < 0)
				reply_len = -1;
			else
				reply_len += res;
		}
		if (reply_len >= 0) {
			res = radius_client_get_mib(wasd->radius,
						    reply + reply_len,
						    reply_size - reply_len);
			if (res < 0)
				reply_len = -1;
			else
				reply_len += res;
		}
	} else if (os_strcmp(buf, "STA-FIRST") == 0) {
		reply_len = asd_ctrl_iface_sta_first(wasd, reply,
							 reply_size);
	} else if (os_strncmp(buf, "STA ", 4) == 0) {
		reply_len = asd_ctrl_iface_sta(wasd, buf + 4, reply,
						   reply_size);
	} else if (os_strncmp(buf, "STA-NEXT ", 9) == 0) {
		reply_len = asd_ctrl_iface_sta_next(wasd, buf + 9, reply,
							reply_size);
	} else if (os_strcmp(buf, "ATTACH") == 0) {
		if (asd_ctrl_iface_attach(wasd, &from, fromlen))
			reply_len = -1;
	} else if (os_strcmp(buf, "DETACH") == 0) {
		if (asd_ctrl_iface_detach(wasd, &from, fromlen))
			reply_len = -1;
	} else if (os_strncmp(buf, "LEVEL ", 6) == 0) {
		if (asd_ctrl_iface_level(wasd, &from, fromlen,
						    buf + 6))
			reply_len = -1;
	} else if (os_strncmp(buf, "NEW_STA ", 8) == 0) {
		if (asd_ctrl_iface_new_sta(wasd, buf + 8))
			reply_len = -1;
	} else {
		os_memcpy(reply, "UNKNOWN COMMAND\n", 16);
		reply_len = 16;
	}

	if (reply_len < 0) {
		os_memcpy(reply, "FAIL\n", 5);
		reply_len = 5;
	}
	//qiuchen
	if((sendto(sock, reply, reply_len, 0, (struct sockaddr *) &from, fromlen))<0)
		asd_printf(ASD_DEFAULT,MSG_DEBUG,"func: %s,sendto error!\n",__func__);
	os_free(reply);
}


static char * asd_ctrl_iface_path(struct asd_data *wasd)
{
	char *buf;
	size_t len;

	if (wasd->conf->ctrl_interface == NULL)
		return NULL;

	len = os_strlen(wasd->conf->ctrl_interface) +
		os_strlen(wasd->conf->iface) + 2;
	buf = os_zalloc(len);
	if (buf == NULL)
		return NULL;

	os_snprintf(buf, len, "%s/%s",
		    wasd->conf->ctrl_interface, wasd->conf->iface);
	buf[len - 1] = '\0';
	return buf;
}


int asd_ctrl_iface_init(struct asd_data *wasd)
{
	struct sockaddr_un addr;
	int s = -1;
	char *fname = NULL;

	wasd->ctrl_sock = -1;

	if (wasd->conf->ctrl_interface == NULL)
		return 0;

	if (mkdir(wasd->conf->ctrl_interface, S_IRWXU | S_IRWXG) < 0) {
		if (errno == EEXIST) {
			asd_printf(ASD_DEFAULT,MSG_DEBUG, "Using existing control "
				   "interface directory.");
		} else {
			perror("mkdir[ctrl_interface]");
			goto fail;
		}
	}

	if (wasd->conf->ctrl_interface_gid_set &&
	    chown(wasd->conf->ctrl_interface, 0,
		  wasd->conf->ctrl_interface_gid) < 0) {
		perror("chown[ctrl_interface]");
		return -1;
	}

	if (os_strlen(wasd->conf->ctrl_interface) + 1 +
	    os_strlen(wasd->conf->iface) >= sizeof(addr.sun_path))
		goto fail;

	s = socket(PF_UNIX, SOCK_DGRAM, 0);
	if (s < 0) {
		perror("socket(PF_UNIX)");
		goto fail;
	}

	os_memset(&addr, 0, sizeof(addr));
	addr.sun_family = AF_UNIX;
	fname = asd_ctrl_iface_path(wasd);
	if (fname == NULL)
		goto fail;
	os_strlcpy(addr.sun_path, fname, sizeof(addr.sun_path));
	if (bind(s, (struct sockaddr *) &addr, sizeof(addr)) < 0) {
		perror("bind(PF_UNIX)");
		goto fail;
	}

	if (wasd->conf->ctrl_interface_gid_set &&
	    chown(fname, 0, wasd->conf->ctrl_interface_gid) < 0) {
		perror("chown[ctrl_interface/ifname]");
		goto fail;
	}

	if (chmod(fname, S_IRWXU | S_IRWXG) < 0) {
		perror("chmod[ctrl_interface/ifname]");
		goto fail;
	}
	os_free(fname);

	wasd->ctrl_sock = s;
	circle_register_read_sock(s, asd_ctrl_iface_receive, wasd,
				 NULL);

	return 0;

fail:
	if (s >= 0)
		close(s);
	if (fname) {
		unlink(fname);
		os_free(fname);
	}
	return -1;
}


void asd_ctrl_iface_deinit(struct asd_data *wasd)
{
	struct wpa_ctrl_dst *dst, *prev;

	if (wasd->ctrl_sock > -1) {
		char *fname;
		circle_unregister_read_sock(wasd->ctrl_sock);
		close(wasd->ctrl_sock);
		wasd->ctrl_sock = -1;
		fname = asd_ctrl_iface_path(wasd);
		if (fname)
			unlink(fname);
		os_free(fname);

		if (wasd->conf->ctrl_interface &&
		    rmdir(wasd->conf->ctrl_interface) < 0) {
			if (errno == ENOTEMPTY) {
				asd_printf(ASD_DEFAULT,MSG_DEBUG, "Control interface "
					   "directory not empty - leaving it "
					   "behind");
			} else {
				perror("rmdir[ctrl_interface]");
			}
		}
	}

	dst = wasd->ctrl_dst;
	while (dst) {
		prev = dst;
		dst = dst->next;
		os_free(prev);
	}
}


void asd_ctrl_iface_send(struct asd_data *wasd, int level,
			     char *buf, size_t len)
{
	struct wpa_ctrl_dst *dst, *next;
	struct msghdr msg;
	int idx;
	struct iovec io[2];
	char levelstr[10];

	dst = wasd->ctrl_dst;
	if (wasd->ctrl_sock < 0 || dst == NULL)
		return;

	os_snprintf(levelstr, sizeof(levelstr), "<%d>", level);
	io[0].iov_base = levelstr;
	io[0].iov_len = os_strlen(levelstr);
	io[1].iov_base = buf;
	io[1].iov_len = len;
	os_memset(&msg, 0, sizeof(msg));
	msg.msg_iov = io;
	msg.msg_iovlen = 2;

	idx = 0;
	while (dst) {
		next = dst->next;
		if (level >= dst->debug_level) {
			wpa_hexdump(MSG_DEBUG, "CTRL_IFACE monitor send",
				    (u8 *) dst->addr.sun_path, dst->addrlen);
			msg.msg_name = &dst->addr;
			msg.msg_namelen = dst->addrlen;
			if (sendmsg(wasd->ctrl_sock, &msg, 0) < 0) {
				fprintf(stderr, "CTRL_IFACE monitor[%d]: ",
					idx);
				perror("sendmsg");
				dst->errors++;
				if (dst->errors > 10) {
					asd_ctrl_iface_detach(
						wasd, &dst->addr,
						dst->addrlen);
				}
			} else
				dst->errors = 0;
		}
		idx++;
		dst = next;
	}
}

