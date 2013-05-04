#ifndef _RADIUS_PACKET_H
#define _RADIUS_PACKET_H

int radius_packet_init(struct pppoe_buf *pbuf, uint32 *seed, uint8 code);

int radius_addattr(struct pppoe_buf *pbuf,
			uint8 type, uint32 vendor_id, uint8 vendor_type,
			uint32 value, uint8 *data, uint16 dlen);

int
radius_getattr(struct pppoe_buf *pbuf, struct radius_attr **attr,
				uint8 type, uint32 vendor_id, uint8 vendor_type, int instance);

int radius_add_userpasswd(struct pppoe_buf *pbuf,
							uint8 *pwd, uint16 pwdlen,
							char *secret, size_t secretlen);

int radius_hmac_md5(struct radius_packet *pack,
				char *secret, size_t secretlen, uint8 *dst);

int radius_acctreq_authenticator(struct radius_packet *pack,
								char *secret, size_t secretlen);

int radius_authresp_authenticator(struct radius_packet *pack,
						uint8 *req_auth, char *secret, size_t secretlen);

int radius_reply_check(struct radius_packet *pack,
					struct radius_packet *pack_req,
					char *secret, size_t secretlen);

int radius_request_check(struct radius_packet *pack,
						char *secret, size_t secretlen);

#endif /* _RADIUS_PACKET_H */

