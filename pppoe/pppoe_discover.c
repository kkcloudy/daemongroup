
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <pthread.h>
#include <arpa/inet.h>
#include <errno.h>

#include <sys/ioctl.h>
#include <linux/if_packet.h>
#include <linux/if_arp.h>

#include "pppoe_def.h"
#include "pppoe_priv_def.h"
#include "pppoe_interface_def.h"
#include "kernel/if_pppoe.h"
#include "radius_def.h"

#include "md5.h"
#include "pppoe_util.h"
#include "pppoe_log.h"
#include "pppoe_list.h"
#include "pppoe_buf.h"
#include "pppoe_thread.h"
#include "notifier_chain.h"
#include "pppoe_backup.h"
#include "pppoe_ippool.h"
#include "pppoe_netlink.h"
#include "pppoe_session.h"
#include "pppoe_manage.h"

#include "pppoe_discover.h"


typedef enum {
	DIS_SERVICE,
	DIS_RALAYID,
	DIS_HOSTUNIQ,
	DIS_ACCOOKIE,
	DIS_TAGNUMS,
} RecvTag;

struct discover_config {
	/* discover public config, from device config */
	char base_ifname[IFNAMSIZ];
	char sname[PPPOE_NAMELEN];	

	/* manage private config, need show running */		
	uint8 virtualMac[ETH_ALEN];	/* this is  */
};

struct discover_info {
	uint8 hwaddr[ETH_ALEN];
	uint8 cookie_seed[SEED_LEN];
	char hname[PPPOE_NAMELEN];
};

struct discover_struct {
	int sk;
	struct pppoe_buf	*pbuf;
	
	discover_config_t	*config;
	pppoe_manage_t		*manage;
	thread_master_t		*master;
	thread_struct_t 	*thread;

	struct discover_info info;	
};

static inline int
_recv_packet(int sk, struct pppoe_buf *pbuf) {
	char errbuf[128];
	ssize_t length;

	do {
		length = recv(sk, pbuf->data, pbuf->end - pbuf->data, 0);
		if (length < 0) {
			if (EINTR == errno) 
				continue;

			if (EAGAIN == errno || EWOULDBLOCK == errno) 
				return PPPOEERR_EAGAIN;

			pppoe_log(LOG_WARNING, "socket %d recv failed: %s\n", 
							sk, strerror_r(errno, errbuf, sizeof(errbuf)));
			return PPPOEERR_ERECV;			
		}
	} while (length < 0);

	pbuf->len = length;
	pbuf->tail = pbuf->data + pbuf->len;
	return PPPOEERR_SUCCESS;
}

static inline int
_send_packet(int sk, struct pppoe_buf *pbuf) {
	char errbuf[128];
	ssize_t length;

	do {
		length = send(sk, pbuf->data, pbuf->tail - pbuf->data, 0);
		if (length < 0) {
			if (EINTR == errno) 
				continue;

			if (EAGAIN == errno || EWOULDBLOCK == errno) 
				return PPPOEERR_EAGAIN;

			pppoe_log(LOG_WARNING, "socket %d send failed: %s\n", 
							sk, strerror_r(errno, errbuf, sizeof(errbuf)));
			return PPPOEERR_ESEND;			
		}
	} while (length < 0);
		
	return PPPOEERR_SUCCESS;
}


static inline void
genCookie(unsigned char const *peerEthAddr, unsigned char const *myEthAddr,
				unsigned char const *seed, unsigned char *cookie) {
	struct MD5Context ctx;
	pid_t pid = getpid();

	MD5Init(&ctx);
	MD5Update(&ctx, peerEthAddr, ETH_ALEN);
	MD5Update(&ctx, myEthAddr, ETH_ALEN);
	MD5Update(&ctx, seed, SEED_LEN);
	MD5Final(cookie, &ctx);
	memcpy(cookie + MD5_LEN, &pid, sizeof(pid));
}

static int
parsePacket(struct pppoe_buf *pbuf, struct discover_tag *tag) {
	unsigned char *cur_tag = pbuf->data;
	uint16 tag_type, tag_len;
	int len = pbuf->len;

	/* Do some sanity checks on packet */
	if (len > PPPOE_MTU - 6) { /* 6-byte overhead for PPPoE header */
		pppoe_log(LOG_WARNING, "PPPoE packet length (%d) error", len);
		return PPPOEERR_ELENGTH;
	}

	/* Step through the tags */
	while (len) {
		if (len < TAG_HDR_SIZE) {
			pppoe_log(LOG_WARNING, "packet length (%d) error \n", len);
			return PPPOEERR_ELENGTH;
		}
		
		tag_type = GETSHORT(cur_tag);
		tag_len = GETSHORT(cur_tag + 2);
		
		if (TAG_END_OF_LIST == tag_type) {
			return PPPOEERR_SUCCESS;
		}

		if ((tag_len + TAG_HDR_SIZE) > len) {
			pppoe_log(LOG_WARNING, "tag length (%d) error\n", tag_len);
			return PPPOEERR_ELENGTH;
		}
		
		pppoe_token_log(TOKEN_DISCOVER, "tag type 0x%04x, len %d\n", tag_type, tag_len);

		switch(tag_type) {
			case TAG_SERVICE_NAME:
				memcpy(&tag[DIS_SERVICE], cur_tag, tag_len + TAG_HDR_SIZE);
				break;
				
			case TAG_RELAY_SESSION_ID:
				memcpy(&tag[DIS_RALAYID], cur_tag, tag_len + TAG_HDR_SIZE);
				break;
				
			case TAG_HOST_UNIQ:
				memcpy(&tag[DIS_HOSTUNIQ], cur_tag, tag_len + TAG_HDR_SIZE);
				break;

			case TAG_AC_COOKIE:
				memcpy(&tag[DIS_ACCOOKIE], cur_tag, tag_len + TAG_HDR_SIZE);
				break;
		}
		
		cur_tag += TAG_HDR_SIZE + tag_len;
		len -= TAG_HDR_SIZE + tag_len;
	}
	
	return PPPOEERR_SUCCESS;
}

static int
sendPADT(int sk, struct pppoe_buf *pbuf, 
			unsigned char *dest, unsigned char *source,
			uint32 sid, char *mess){
	struct pppoe_packet *pack
		= (struct pppoe_packet *)pbuf_put(pbuf_init(pbuf), sizeof(struct pppoe_packet));

	/* Construct a PADT packet */
	memcpy(pack->ethHdr.h_dest, dest, ETH_ALEN);
	memcpy(pack->ethHdr.h_source, source, ETH_ALEN);
	pack->ethHdr.h_proto = htons(ETH_P_PPP_DISC);
	pack->phdr.ver = 1;
	pack->phdr.type = 1;
	pack->phdr.code = CODE_PADT;
	pack->phdr.sid = htons(sid);

	if (mess) {
		struct discover_tag *tag;
		uint32 len = strlen(mess);
		
		if (pbuf_tailroom(pbuf) < (TAG_HDR_SIZE + len))
			return PPPOEERR_ENOMEM;

		tag = (struct discover_tag *)pbuf_put(pbuf, len + TAG_HDR_SIZE);
		tag->type = htons(TAG_GENERIC_ERROR);
		tag->length = htons(len);
		memcpy(tag->payload, mess, len);
	}
	
	pack->phdr.length = htons(pbuf->len - sizeof(struct pppoe_packet));
	return _send_packet(sk, pbuf);
}

static void
sendErrorPADS(int sk, struct pppoe_buf *pbuf,
					unsigned char *dest, unsigned char *source,
					struct discover_tag *recv_tag,
					int errorTag, char *errorMsg) {
	struct pppoe_packet *pack
		= (struct pppoe_packet *)pbuf_put(pbuf_init(pbuf), sizeof(struct pppoe_packet));
	struct discover_tag *tag;
	uint32 elen = strlen(errorMsg);

	memcpy(pack->ethHdr.h_dest, dest, ETH_ALEN);
	memcpy(pack->ethHdr.h_source, source, ETH_ALEN);
	pack->ethHdr.h_proto = htons(ETH_P_PPP_DISC);
	pack->phdr.ver = 1;
	pack->phdr.type = 1;
	pack->phdr.code = CODE_PADS;
	pack->phdr.sid = 0;
	
	if (pbuf_tailroom(pbuf) < (TAG_HDR_SIZE + elen))
		return;

	tag = (struct discover_tag *)pbuf_put(pbuf, TAG_HDR_SIZE + elen);
	tag->type = htons(errorTag);
	tag->length = htons(elen);
	memcpy(tag->payload, errorMsg, elen);
		
	if (recv_tag[DIS_HOSTUNIQ].type) {
		uint32 hlen = ntohs(recv_tag[DIS_HOSTUNIQ].length) + TAG_HDR_SIZE;
		if (pbuf_tailroom(pbuf) < hlen)
			return;

		memcpy(pbuf_put(pbuf, hlen), &recv_tag[DIS_HOSTUNIQ], hlen);
	}

	pack->phdr.length = htons(pbuf->len - sizeof(struct pppoe_packet));
	_send_packet(sk, pbuf);
}

static inline int 
__discover_session_exit(session_struct_t *sess, void *arg) {
	discover_struct_t *discover = (discover_struct_t *)arg;
	sendPADT(discover->sk, sess->pbuf, sess->mac, discover->info.hwaddr, sess->sid, NULL);
	sess->state = SESSION_DISTERM;
	return PPPOEERR_SUCCESS;
}

static inline void 
_discover_session_setup(discover_struct_t *discover, session_struct_t *sess) {
	memcpy(sess->sname, discover->config->sname, PPPOE_NAMELEN);
}


static int
processPADT(discover_struct_t *discover, struct pppoe_packet *pack, struct pppoe_buf *pbuf) {
	session_struct_t *sess;

	pppoe_token_log(TOKEN_DISCOVER, "recv PADT packet, ethHdr %02X:%02X:%02X:%02X:%02X:%02X\n", 
								pack->ethHdr.h_source[0], pack->ethHdr.h_source[1],
								pack->ethHdr.h_source[2], pack->ethHdr.h_source[3],
								pack->ethHdr.h_source[4], pack->ethHdr.h_source[5]);


	/* Ignore PADR's not directed at us */
	if (memcmp(pack->ethHdr.h_dest, discover->info.hwaddr, ETH_ALEN)) {
		pppoe_log(LOG_WARNING, "dest address is not AC\n");
		return PPPOEERR_EADDR;
	}
	
	/* Ignore PADI's which don't come from a unicast address */
	if (NOT_UNICAST(pack->ethHdr.h_source)) {
		pppoe_log(LOG_WARNING, "packet from non-unicast source address");
		return PPPOEERR_EADDR;
	}

	sess = __manage_sess_get_by_sid(discover->manage, ntohs(pack->phdr.sid));
	if (!sess) {
		pppoe_token_log(TOKEN_DISCOVER, "session %u is not exist\n", ntohs(pack->phdr.sid));
		return PPPOEERR_ENOEXIST;
	}

	if (memcmp(sess->mac, pack->ethHdr.h_source, ETH_ALEN)) {
		pppoe_token_log(TOKEN_DISCOVER, "session %u mac is not match "
										"%02X:%02X:%02X:%02X:%02X:%02X\n", ntohs(pack->phdr.sid),
										pack->ethHdr.h_source[0], pack->ethHdr.h_source[1],
										pack->ethHdr.h_source[2], pack->ethHdr.h_source[3],
										pack->ethHdr.h_source[4], pack->ethHdr.h_source[5]);
		return PPPOEERR_ENOEXIST;
	}

	sendPADT(discover->sk, pbuf, sess->mac, discover->info.hwaddr, sess->sid, "Recv PADT");
	session_termcause_setup(sess, RADIUS_TERMINATE_CAUSE_USER_REQUEST);
	session_opt_unregister(sess, SESSOPT_LCPTERMREQ);
	session_opt_unregister(sess, SESSOPT_DISTREM);
	manage_sess_exit(sess);
	return PPPOEERR_SUCCESS;
}

static int
processPADR(discover_struct_t *discover, struct pppoe_packet *pack, struct pppoe_buf *pbuf) {
	struct discover_tag recv_tag[DIS_TAGNUMS];
	struct discover_info *info = &discover->info;
	discover_config_t *config = discover->config;
	session_struct_t *session;
	unsigned char cookie[COOKIE_LEN];
	uint32 slen;

	pppoe_token_log(TOKEN_DISCOVER, "recv PADR packet, ethHdr %02X:%02X:%02X:%02X:%02X:%02X\n", 
									pack->ethHdr.h_source[0], pack->ethHdr.h_source[1],
									pack->ethHdr.h_source[2], pack->ethHdr.h_source[3],
									pack->ethHdr.h_source[4], pack->ethHdr.h_source[5]);

	/* Ignore PADR's not directed at us */
	if (memcmp(pack->ethHdr.h_dest, discover->info.hwaddr, ETH_ALEN)) {
		pppoe_log(LOG_WARNING, "dest address is not AC\n");
		return PPPOEERR_EADDR;
	}
	
	/* Ignore PADI's which don't come from a unicast address */
	if (NOT_UNICAST(pack->ethHdr.h_source)) {
		pppoe_log(LOG_WARNING, "packet from non-unicast source address");
		return PPPOEERR_EADDR;
	}

	session = __manage_sess_get_by_mac(discover->manage, pack->ethHdr.h_source);
	if (session) {
		if (session->state > SESSION_INIT) {
			pppoe_log(LOG_WARNING, "Client %02X:%02X:%02X:%02X:%02X:%02X is already create session\n",
									pack->ethHdr.h_source[0], pack->ethHdr.h_source[1],
									pack->ethHdr.h_source[2], pack->ethHdr.h_source[3],
									pack->ethHdr.h_source[4], pack->ethHdr.h_source[5]);
			return PPPOEERR_EEXIST;
		}

		pppoe_token_log(TOKEN_DISCOVER, "session %u PADO need retransport\n", session->sid);	
	}

	/* parse recv tag */
	memset(recv_tag, 0, sizeof(recv_tag));	
	if (parsePacket(pbuf, recv_tag)) {
		pppoe_log(LOG_WARNING, "parse packet fail!\n");
		return PPPOEERR_EINVAL;
	}

	if (recv_tag[DIS_RALAYID].type) {
		pppoe_log(LOG_WARNING, "pppoe server not support session replay");
		/* Drop it -- do not send error PADS */
		return PPPOEERR_EINVAL;
	}

	/* check PADR cookie */
	if (!recv_tag[DIS_ACCOOKIE].type ||
		recv_tag[DIS_ACCOOKIE].length != htons(COOKIE_LEN)) {
		/* Drop it -- do not send error PADS */
		return PPPOEERR_EINVAL;
	}

	genCookie(pack->ethHdr.h_source, info->hwaddr, info->cookie_seed, cookie);
	if (memcmp(recv_tag[DIS_ACCOOKIE].payload, cookie, COOKIE_LEN)) {
		/* Drop it -- do not send error PADS */
		return PPPOEERR_EINVAL;
	}

	/* Check service name */
	if (!recv_tag[DIS_SERVICE].type) {
		pppoe_log(LOG_WARNING, "Received packet with no SERVICE_NAME tag");

		/* need send error PADS */
		sendErrorPADS(discover->sk, pbuf, pack->ethHdr.h_source, info->hwaddr, recv_tag,
						TAG_SERVICE_NAME_ERROR, "PPPoE: Server: No service name tag");
		return PPPOEERR_EINVAL;
	}

	if ((slen = ntohs(recv_tag[DIS_SERVICE].length)) > 0) {
		if (slen != strlen(config->sname) ||
			memcmp(config->sname, recv_tag[DIS_SERVICE].payload, slen)) {
			pppoe_log(LOG_WARNING, "service %s is not support\n",
									recv_tag[DIS_SERVICE].payload);

			/* need send error PADS */
			sendErrorPADS(discover->sk, pbuf, pack->ethHdr.h_source, info->hwaddr, recv_tag,
							TAG_SERVICE_NAME_ERROR, "PPPoE: Server: Not support service name");
			return PPPOEERR_ENOEXIST;
		}
	} 

	if (!session) {
		session = manage_sess_init(discover->manage, pack->ethHdr.h_source, info->hwaddr);
		if (!session) {
			pppoe_log(LOG_WARNING, "session alloc fail\n");
			sendErrorPADS(discover->sk, pbuf, pack->ethHdr.h_source, info->hwaddr, recv_tag,
							TAG_AC_SYSTEM_ERROR, "PPPoE: Server: Session timeout");
			return PPPOEERR_ENOMEM;
		}

		_discover_session_setup(discover, session);			
	}	

	/* Construct a PADR packet */
	pack = (struct pppoe_packet *)pbuf_put(pbuf_init(pbuf), sizeof(struct pppoe_packet));
	memcpy(pack->ethHdr.h_dest, pack->ethHdr.h_source, ETH_ALEN);
	memcpy(pack->ethHdr.h_source, info->hwaddr, ETH_ALEN);
	pack->ethHdr.h_proto = htons(ETH_P_PPP_DISC);
	pack->phdr.ver = 1;
	pack->phdr.type = 1;
	pack->phdr.code = CODE_PADS;
	pack->phdr.sid = htons(session->sid);
	
	if (config->sname[0]) {
		struct discover_tag *tag;

		slen = strlen(config->sname);
		if (pbuf_tailroom(pbuf) < (TAG_HDR_SIZE  + slen))
			goto room_err;

		tag = (struct discover_tag *)pbuf_put(pbuf, slen + TAG_HDR_SIZE);
		tag->type = htons(TAG_SERVICE_NAME);
		tag->length = htons(slen);
		memcpy(tag->payload, config->sname, slen);
	}

	if (recv_tag[DIS_HOSTUNIQ].type) {
		int hlen = ntohs(recv_tag[DIS_HOSTUNIQ].length) + TAG_HDR_SIZE;
		if (pbuf_tailroom(pbuf) < hlen)
			goto room_err;

		memcpy(pbuf_put(pbuf, hlen), &recv_tag[DIS_HOSTUNIQ], hlen);
	}

	pack->phdr.length = htons(pbuf->len - sizeof(struct pppoe_packet));
	pppoe_token_log(TOKEN_DISCOVER, "PADS packag is ready\n");
	return _send_packet(discover->sk, pbuf);

room_err:
	pppoe_log(LOG_WARNING, "PADS packet mem less\n");
	sendErrorPADS(discover->sk, pbuf, pack->ethHdr.h_source, info->hwaddr, recv_tag,
					TAG_AC_SYSTEM_ERROR, "PPPoE: Server: memory less");
	session_opt_perform(session, SESSOPT_EXIT);	
	return PPPOEERR_ENOMEM;
}

static int
processPADI(discover_struct_t *discover, struct pppoe_packet *pack, struct pppoe_buf *pbuf) {
	struct discover_tag recv_tag[DIS_TAGNUMS], *tag;
	struct discover_info *info = &discover->info;
	discover_config_t *config = discover->config;
	uint32 acname_len;

	pppoe_token_log(TOKEN_DISCOVER, "recv PADI packet, ethHdr %02X:%02X:%02X:%02X:%02X:%02X\n", 
									pack->ethHdr.h_source[0], pack->ethHdr.h_source[1],
									pack->ethHdr.h_source[2], pack->ethHdr.h_source[3],
									pack->ethHdr.h_source[4], pack->ethHdr.h_source[5]);


	/* Ignore PADI's which don't come from a unicast address */
	if (NOT_UNICAST(pack->ethHdr.h_source)) {
		pppoe_log(LOG_WARNING, "packet from non-unicast source address");
		return PPPOEERR_EADDR;
	}


	/* If number of sessions per MAC is limited, check here and don't
	   send PADO if already max number of sessions. */
	if (__manage_sess_get_by_mac(discover->manage, pack->ethHdr.h_source)) {
		pppoe_log(LOG_WARNING, "Client %02X:%02X:%02X:%02X:%02X:%02X is already create session\n",
								pack->ethHdr.h_source[0], pack->ethHdr.h_source[1],
								pack->ethHdr.h_source[2], pack->ethHdr.h_source[3],
								pack->ethHdr.h_source[4], pack->ethHdr.h_source[5]);
		return PPPOEERR_EEXIST;
	}
	
	memset(recv_tag, 0, sizeof(recv_tag));
	if (parsePacket(pbuf, recv_tag)) {
		pppoe_log(LOG_WARNING, "parse packet fail!\n");
		return PPPOEERR_EINVAL;
	}


	if (recv_tag[DIS_RALAYID].type) {
		pppoe_log(LOG_WARNING, "pppoe server not support session replay");
		/* Drop it, do not send PADO */
		return PPPOEERR_EINVAL;
	}

		
	/* If PADI specified non-default service name, */
	/* and we do not offer that service, DO NOT send PADO */
	if (recv_tag[DIS_SERVICE].type) {		
		uint32 slen = ntohs(recv_tag[DIS_SERVICE].length);
		if (slen && (slen != strlen(config->sname) || 
			memcmp(config->sname, recv_tag[DIS_SERVICE].payload, slen))) {
			/*need set service name '\0', Otherwise*/
			recv_tag[DIS_SERVICE].payload[slen] = '\0';
			pppoe_log(LOG_WARNING, "service %s is not support\n",
								recv_tag[DIS_SERVICE].payload);
			return PPPOEERR_ENOEXIST;
		}
	} 

	/* Construct a PADO packet */
	pack = (struct pppoe_packet *)pbuf_put(pbuf_init(pbuf), sizeof(struct pppoe_packet));
	memcpy(pack->ethHdr.h_dest, pack->ethHdr.h_source, ETH_ALEN);
	memcpy(pack->ethHdr.h_source, info->hwaddr, ETH_ALEN);
	pack->ethHdr.h_proto = htons(ETH_P_PPP_DISC);
	pack->phdr.ver = 1;
	pack->phdr.type = 1;
	pack->phdr.code = CODE_PADO;
	pack->phdr.sid = 0;

	/* discover AC_NAME tag*/	
	acname_len = strlen(info->hname);			/* need copy from config */
	if (pbuf_tailroom(pbuf) < (acname_len + TAG_HDR_SIZE))
		goto room_err;

	tag = (struct discover_tag *)pbuf_put(pbuf, acname_len + TAG_HDR_SIZE);
	tag->type = htons(TAG_AC_NAME);
	tag->length = htons(acname_len);
	memcpy(tag->payload, info->hname, acname_len);

	/* discover SERVICE_NAME tag*/	
	if (config->sname[0]) {
		uint32 slen = strlen(config->sname);
		if (pbuf_tailroom(pbuf) < (TAG_HDR_SIZE + slen)) {
			goto room_err;
		}

		tag = (struct discover_tag *)pbuf_put(pbuf, slen + TAG_HDR_SIZE);
		tag->type = htons(TAG_SERVICE_NAME);
		tag->length = htons(slen);
		memcpy(tag->payload, config->sname, slen);
	} else {
		if (pbuf_tailroom(pbuf) < TAG_HDR_SIZE)
			goto room_err;

		tag = (struct discover_tag *)pbuf_put(pbuf, TAG_HDR_SIZE);
		tag->type = htons(TAG_SERVICE_NAME);
		tag->length = 0;
	} 

	/* Generate a cookie */
	if (pbuf_tailroom(pbuf) < (TAG_HDR_SIZE + COOKIE_LEN))
		goto room_err;

	tag = (struct discover_tag *)pbuf_put(pbuf, TAG_HDR_SIZE + COOKIE_LEN);
	tag->type = htons(TAG_AC_COOKIE);
	tag->length = htons(COOKIE_LEN);	
	genCookie(pack->ethHdr.h_dest, info->hwaddr, info->cookie_seed, tag->payload);
		
	/* return hostuniq */
	if (recv_tag[DIS_HOSTUNIQ].type) {
		uint32 hlen = ntohs(recv_tag[DIS_HOSTUNIQ].length) + TAG_HDR_SIZE;
		if (pbuf_tailroom(pbuf) < hlen)
			goto room_err;

		memcpy(pbuf_put(pbuf, hlen), &recv_tag[DIS_HOSTUNIQ], hlen);
	}

	pack->phdr.length = htons(pbuf->len - sizeof(struct pppoe_packet));
	pppoe_token_log(TOKEN_DISCOVER, "PADO packag is ready\n");
	
	return _send_packet(discover->sk, pbuf);

room_err:
	pppoe_log(LOG_WARNING, "packet room is not enough\n");
	return PPPOEERR_ENOMEM;
}


static int
discover_process(thread_struct_t *thread) {
	discover_struct_t *discover = thread_get_arg(thread);
	struct pppoe_buf *pbuf = pbuf_init(discover->pbuf);
	struct pppoe_packet *pack;
	int ret;
	
	if (PPPOEERR_SUCCESS != 
		(ret = _recv_packet(discover->sk, pbuf))) {
		pppoe_log(LOG_WARNING, "recv packet failed\n");
		return ret;
	}

	if (unlikely(pbuf_may_pull(pbuf, sizeof(struct pppoe_packet)))) {
		pppoe_log(LOG_WARNING, "packet length(%d) error\n", pbuf->len);
		return PPPOEERR_ELENGTH;
	}

	pack = (struct pppoe_packet *)pbuf->data;
	pbuf_pull(pbuf, sizeof(struct pppoe_packet));
	
	if (unlikely(pbuf_trim(pbuf, ntohs(pack->phdr.length)))) {
		pppoe_log(LOG_WARNING, "PPPoE hdr length(%d) error", ntohs(pack->phdr.length));
		return PPPOEERR_ELENGTH;
	}

	/* Sanity check on packet */
	if (pack->phdr.ver != 1 || pack->phdr.type != 1) {
		pppoe_log(LOG_WARNING, "PPPoE ver or type field (%d) (%d)", 
					pack->phdr.ver, pack->phdr.type);
		return PPPOEERR_EINVAL;
	}

	pppoe_token_log(TOKEN_DISCOVER, "recv discover packet, code = 0x%02x\n", pack->phdr.code);
	
	switch(pack->phdr.code) {
		case CODE_PADI:
			ret = processPADI(discover, pack, pbuf);
			break;
			
		case CODE_PADR:
			ret = processPADR(discover, pack, pbuf);
			break;
			
		case CODE_PADT:
			/* kill session */
			ret = processPADT(discover, pack, pbuf);
			break;
			
		case CODE_SESS:
		case CODE_PADO:
		case CODE_PADS:
			/* Ignore PADO and PADS totally */
			break;
			
		default:
			pppoe_log(LOG_WARNING, "recv unknow packet type(%d)\n", pack->phdr.code);
			return PPPOEERR_EINVAL;
	}

	return ret;
}

static inline void 
discover_setup(discover_struct_t *discover) {
	memset(discover->info.hname, 0, sizeof(discover->info.hname));
	gethostname(discover->info.hname, sizeof(discover->info.hname) -1);
	manage_sessions_opt_register(discover->manage, SESSOPT_DISTREM, discover, __discover_session_exit);	
}

static inline void
discover_unsetup(discover_struct_t *discover) {
	manage_sessions_opt_unregister(discover->manage, SESSOPT_DISTREM);
}

static int
discover_sock_init(discover_struct_t *discover) {
	discover_config_t *config = discover->config;
	struct sockaddr_ll saddr;
	struct ifreq ifr;	
	int ret, optval = 1;

	if (unlikely(!config->base_ifname[0]))
		return PPPOEERR_EINVAL;
	
	discover->sk = socket(PF_PACKET, SOCK_RAW, htons(ETH_P_PPP_DISC));	/* init discover socket */
	if (discover->sk < 0) {
		if (EPERM == errno) {
			pppoe_log(LOG_ERR, "Cannot create raw socket -- must be run as root.\n");
		}
		pppoe_log(LOG_ERR, "creat discover socket fail\n");
		ret = PPPOEERR_ESOCKET;
		goto error;
	}
	pppoe_token_log(TOKEN_DISCOVER, "create discover socket is %d\n", discover->sk);
	
	/* set discover socket opt */
	if (setsockopt(discover->sk, SOL_SOCKET, SO_BROADCAST, &optval, sizeof(optval)) < 0) {
		pppoe_log(LOG_ERR, "discover socket setsockopt fail\n");
		ret = PPPOEERR_ESOCKET;
		goto error1;
	}

	set_nonblocking(discover->sk);	

	/* get pppoe base interface hwaddr */
	memset(&ifr, 0, sizeof(ifr));
	strncpy(ifr.ifr_name, config->base_ifname, sizeof(ifr.ifr_name));
	pppoe_token_log(TOKEN_DISCOVER, "listen interface %s\n", ifr.ifr_name);
	if (ioctl(discover->sk, SIOCGIFHWADDR, &ifr) < 0) {
		pppoe_log(LOG_ERR, "ioctl(SIOCGIFHWADDR): get interface hwaddr failed\n");
		ret = PPPOEERR_ESYSCALL;
		goto error1;
	}

	/* base interface is need ether */
	if (ifr.ifr_hwaddr.sa_family != ARPHRD_ETHER) {
		pppoe_log(LOG_ERR, "interface %s is not Ethernet\n", config->base_ifname);
		ret = PPPOEERR_EADDR;
		goto error1;
	}

	if (NO_VIRTUALMAC(config->virtualMac)) {
		/* base interface hwaddr can not be broadcast/multicast MAC address */
		if (NOT_UNICAST(discover->info.hwaddr)) {
			pppoe_log(LOG_ERR, "interface %s has broadcast/multicast MAC address\n", config->base_ifname);
			ret = PPPOEERR_EADDR;
			goto error1;
		}
		
		memcpy(discover->info.hwaddr, ifr.ifr_hwaddr.sa_data, ETH_ALEN);
	} else {
		/* pppoe set virtual Mac */
		memcpy(discover->info.hwaddr, config->virtualMac, ETH_ALEN);
	}
	pppoe_token_log(TOKEN_DISCOVER, "discover hwaddr %02X:%02X:%02X:%02X:%02X:%02X\n", 
									discover->info.hwaddr[0], discover->info.hwaddr[1],
									discover->info.hwaddr[2], discover->info.hwaddr[3],
									discover->info.hwaddr[4], discover->info.hwaddr[5]);

	/* discover socket bind base interface  */
	memset(&saddr, 0, sizeof(saddr));
	saddr.sll_family = AF_PACKET;
	saddr.sll_protocol = htons(ETH_P_PPP_DISC);
	if (ioctl(discover->sk, SIOCGIFINDEX, &ifr) < 0) {
		pppoe_log(LOG_ERR, "ioctl(SIOCFIGINDEX): get interface index failed\n");
		ret = PPPOEERR_ESYSCALL;
		goto error1;
	}
	saddr.sll_ifindex = ifr.ifr_ifindex;
	pppoe_token_log(TOKEN_DISCOVER, "bind addr ifindex %d\n", saddr.sll_ifindex);

	if (bind(discover->sk, (struct sockaddr *)&saddr, sizeof(saddr)) < 0) {
		pppoe_log(LOG_ERR, "discover socket bind sockaddr failed\n");
		ret = PPPOEERR_EBIND;
		goto error1;
	}

	return PPPOEERR_SUCCESS;
	
error1:
	PPPOE_CLOSE(discover->sk);
error:
	return ret;
}

#if 0
static inline void
discover_cookie_seed_init(discover_struct_t *discover) {
	FILE *fp;
	
	/* Initialize our random cookie.  Try /dev/urandom; if that fails, use PID and rand() */
	fp = fopen("/dev/urandom", "r");
	if (fp) {
		unsigned int seed;
		fread(&seed, 1, sizeof(seed), fp);
		srand(seed);
		fread(discover->info.cookie_seed, 1, SEED_LEN, fp);
		fclose(fp);
	} else {
		int i;
		srand((uint32)getpid() * (uint32)time_sysup());
		discover->info.cookie_seed[0] = getpid() & 0xFF;
		discover->info.cookie_seed[1] = (getpid() >> 8) & 0xFF;
		for ( i = 2; i < SEED_LEN; i++) {
			discover->info.cookie_seed[i] = (rand() >> (i % 9)) & 0xFF;
		}
	}
}
#endif

int
pppoe_discover_start(discover_struct_t *discover) {
	if (unlikely(!discover))
		return PPPOEERR_EINVAL;

	if (discover_sock_init(discover)) {
		pppoe_log(LOG_ERR, "discover socket init failed\n");
		return PPPOEERR_ESOCKET;
	}

	rand_seed_init(discover->info.cookie_seed, SEED_LEN);

	discover->thread 
		= thread_add_read(discover->master, discover_process, discover, discover->sk);
	if (!discover->thread) {
		pppoe_log(LOG_ERR, "discover thread add read failed\n");
		return PPPOEERR_ESOCKET;
	}
	
	pppoe_log(LOG_INFO, "disocover start sucess\n");
	return PPPOEERR_SUCCESS;
}

int
pppoe_discover_stop(discover_struct_t *discover) {
	if (unlikely(!discover))
		return PPPOEERR_EINVAL;

	THREAD_CANCEL(discover->thread);
	PPPOE_CLOSE(discover->sk);	
	pppoe_log(LOG_INFO, "disocover stop sucess\n");
	return PPPOEERR_SUCCESS;
}

discover_struct_t * 
pppoe_discover_init(thread_master_t *master,
						pppoe_manage_t *manage, 
						discover_config_t *config){
	discover_struct_t *discover;

	if (unlikely(!config || !master || !manage))
		return NULL;

	discover = (discover_struct_t *)malloc(sizeof(discover_struct_t));
	if (unlikely(!discover)) {
		pppoe_log(LOG_ERR, "malloc pppoe discover fail\n");
		return NULL;
	}
	
	memset(discover, 0, sizeof(*discover));
	discover->config = config;
	discover->master = master;
	discover->manage = manage;
	discover->sk = -1;

	discover->pbuf = pbuf_alloc(DEFAULT_BUF_SIZE);
	if (unlikely(!discover->pbuf)) {
		pppoe_log(LOG_ERR, "malloc pppoe pbuf fail\n");
		PPPOE_FREE(discover);
		return NULL;
	}

	discover_setup(discover);
	
	pppoe_log(LOG_INFO, "disocover init sucess\n");
	return discover;
}

void
pppoe_discover_destroy(discover_struct_t **discover) {
	if (unlikely(!discover || !(*discover)))
		return ;
	
	discover_unsetup(*discover);
	PBUF_FREE((*discover)->pbuf);
	PPPOE_FREE(*discover);
	pppoe_log(LOG_INFO, "disocover destroy sucess\n");	
}

int
pppoe_discover_config_virtual_mac(discover_config_t *config, uint8 *virtualMac) {
	if (unlikely(!virtualMac))
		return PPPOEERR_EINVAL;

	if (!NO_VIRTUALMAC(config->virtualMac)) {
		if (NOT_UNICAST(config->virtualMac)) {
			pppoe_log(LOG_WARNING, "virtual mac is non-unicast\n");
			return PPPOEERR_EADDR;
		}
	}

	memcpy(config->virtualMac, virtualMac, ETH_ALEN);
	return PPPOEERR_SUCCESS;	
}

int 
pppoe_discover_show_running_config(discover_config_t *config, char *cmd, uint32 len) {
	char *cursor = cmd;

	if (!NO_VIRTUALMAC(config->virtualMac)) {
		cursor += snprintf(cursor, len - (cursor - cmd), 
						" virtual mac %02X:%02X:%02X:%02X:%02X:%02X\n", 
						config->virtualMac[0], config->virtualMac[1],
						config->virtualMac[2], config->virtualMac[3],
						config->virtualMac[4], config->virtualMac[5]);
	}

	return cursor - cmd;
}

void
pppoe_discover_config_setup(discover_config_t *config, char *base_ifname, char *sname) {
	strncpy(config->base_ifname, base_ifname, sizeof(config->base_ifname) - 1);
	strncpy(config->sname, sname, sizeof(config->sname) - 1);
}

int
pppoe_discover_config_init(discover_config_t **config) {
	if (unlikely(!config))
		return PPPOEERR_EINVAL;

	*config = (discover_config_t *)malloc(sizeof(discover_config_t));
	if (unlikely(!(*config))) {
		pppoe_log(LOG_ERR, "pppoe discover config alloc fail\n");
		return PPPOEERR_ENOMEM;
	}
	
	memset(*config, 0, sizeof(discover_config_t));
	return PPPOEERR_SUCCESS;
}

void
pppoe_discover_config_exit(discover_config_t **config) {
	if (unlikely(!config))
		return;

	PPPOE_FREE(*config);
}


