#ifndef _PPPOE_PRIV_DEF_H
#define _PPPOE_PRIV_DEF_H

#ifndef likely
#define likely(x)	__builtin_expect(!!(x), 1)
#define unlikely(x)	__builtin_expect(!!(x), 0)
#endif

#define PPPOE_FREE(obj_name)	{ if (obj_name) { free((obj_name)); (obj_name) = NULL; } }
#define PPPOE_CLOSE(sk)			{ if ((sk) > 0) { close((sk)); (sk) = -1; } }
#define ARRAY_SIZE(arr) 		(sizeof(arr) / sizeof((arr)[0]))

#ifndef HIPQUAD
#define HIPQUAD(addr)	\
		((addr) >> 24) & 0xff,	\
		((addr) >> 16) & 0xff,	\
		((addr) >> 8) & 0xff,	\
		(addr) & 0xff
#endif


#define GETCHAR(cp)		(*(cp))
#define GETSHORT(cp) 	((*(cp) << 8) + *((cp) + 1))
#define GETLONG(cp)		((*(cp) << 24) +			\
						(*((cp) + 1) << 16) +		\
						(*((cp) + 2) << 8) +		\
						*((cp) + 3))

#define PUTCHAR(cp, c) {		\
	*(cp)++ = (c) & 0xff;		\
}

#define PUTSHORT(cp, s) {		\
	*(cp)++ = ((s) >> 8) & 0xff;		\
	*(cp)++ = (s) & 0xff;	\
}

#define PUTLONG(cp, l) {		\
	*(cp)++ = ((l) >> 24) & 0xff; 	\
	*(cp)++ = ((l) >> 16) & 0xff;		\
	*(cp)++ = ((l) >> 8) & 0xff;		\
	*(cp)++ = (l) & 0xff;	\
}

typedef enum {
	SESSOPT_EXIT,
	SESSOPT_DISTREM,
	SESSOPT_LCPCONFREQ,
	SESSOPT_LCPECHOREQ,
	SESSOPT_LCPTERMREQ,
	SESSOPT_CCPCONFREQ,
	SESSOPT_IPCPCONFREQ,
	SESSOPT_IPAPPLY,
	SESSOPT_IPRECOVER,
	SESSOPT_IPREGISTER,
	SESSOPT_IPUNREGISTER,
	SESSOPT_AUTHPRE,
	SESSOPT_AUTHREQ,
	SESSOPT_AUTHSUCCESS,
	SESSOPT_AUTHFAIL,
	SESSOPT_ACCTSTART,
	SESSOPT_ACCTSTOP,
	SESSOPT_ACCTSTOPV2,
	SESSOPT_ACCTUPDATE,
	SESSOPT_ONLINESYNC,
	SESSOPT_OFFLINESYNC,
	SESSOPT_UPDATESYNC,
	SESSOPT_NUMS,
} sessionOptType;

enum {
	THREAD_EXTIME_NONE,
	THREAD_EXTIMER1 = 1,
	THREAD_EXTIMER2,
	THREAD_EXTIMER3,
};

enum {
	INSTANCE_SESSONLINE_SYNC = 1,
	INSTANCE_SESSOFFLINE_SYNC,
	INSTANCE_SESSUPDATE_SYNC,
	INSTANCE_SESSCLEAR_SYNC,
	INSTANCE_SESSSYNC_REQUEST,
	INSTANCE_SESSSYNC_FINISHED,
	INSTANCE_BACKUP_FINISHED,		
};

typedef unsigned char 		uint8;
typedef unsigned short 		uint16;
typedef unsigned int		uint32;

typedef struct thread_bus		thread_bus_t;
typedef struct tbus_connection	tbus_connection_t;

typedef struct session_opt		session_opt_t;
typedef struct session_struct	session_struct_t;
typedef struct pppoe_sessions	pppoe_sessions_t;

typedef struct radius_config	radius_config_t;
typedef struct radius_struct	radius_struct_t;

typedef struct manage_config	manage_config_t;
typedef struct pppoe_manage		pppoe_manage_t;

typedef struct discover_config	discover_config_t;
typedef struct discover_struct	discover_struct_t;

typedef struct control_config	control_config_t;
typedef struct control_struct	control_struct_t;

typedef int (*methodFunc)(void *, void *);
typedef void (*tbusDataFree)(void *);
typedef int (*sessOptFunc) (session_struct_t *, void *);

/* True if Ethernet address is broadcast or multicast */
#define NOT_UNICAST(e)		((e[0] & 0x01) != 0)
#define BROADCAST(e)		((e[0] & e[1] & e[2] & e[3] & e[4] & e[5]) == 0xFF)
#define NOT_BROADCAST(e)	((e[0] & e[1] & e[2] & e[3] & e[4] & e[5]) != 0xFF)
#define NO_VIRTUALMAC(e)	((e[0] | e[1] | e[2] | e[3] | e[4] | e[5]) == 0)

/* PPPoE codes */
#define CODE_PADI		0x09
#define CODE_PADO		0x07
#define CODE_PADR		0x19
#define CODE_PADS		0x65
#define CODE_PADT		0xA7

#define CODE_SESS		0x00

/* PPPoE Tags */
#define TAG_END_OF_LIST				0x0000
#define TAG_SERVICE_NAME			0x0101
#define TAG_AC_NAME					0x0102
#define TAG_HOST_UNIQ				0x0103
#define TAG_AC_COOKIE				0x0104
#define TAG_VENDOR_SPECIFIC			0x0105
#define TAG_RELAY_SESSION_ID		0x0110
#define TAG_SERVICE_NAME_ERROR		0x0201
#define TAG_AC_SYSTEM_ERROR			0x0202
#define TAG_GENERIC_ERROR			0x0203


/* Random seed for cookie generation */
#define SEED_LEN 	16
#define MD5_LEN		16
#define COOKIE_LEN	(MD5_LEN + sizeof(pid_t)) /* Cookie is 16-byte MD5 + PID of server */

/* PPP codes */
#define PPP_IPCP	0x8021	/* IP Control Protocol */
#define PPP_CCP		0x80fd	/* Compression Control Protocol */
#define PPP_LCP		0xc021	/* Link Control Protocol */
#define PPP_PAP		0xc023	/* Password Authentication Protocol */
#define PPP_CHAP	0xc223	/* Cryptographic Handshake Auth. Protocol */
#define PPP_EAP		0xc227	/* Extensible Authentication Protocol */


/*
 *  CP (LCP, IPCP, etc.) codes.
 */
#define CONFREQ		1	/* Configuration Request */
#define CONFACK		2	/* Configuration Ack */
#define CONFNAK		3	/* Configuration Nak */
#define CONFREJ		4	/* Configuration Reject */
#define TERMREQ		5	/* Termination Request */
#define TERMACK		6	/* Termination Ack */
#define CODEREJ		7	/* Code Reject */

/*
 * LCP-specific packet types (code numbers).
 */
#define PROTREJ		8	/* Protocol Reject */
#define ECHOREQ		9	/* Echo Request */
#define ECHOREP		10	/* Echo Reply */
#define DISCREQ		11	/* Discard Request */
#define IDENTIF		12	/* Identification */
#define TIMEREM		13	/* Time Remaining */

/*
 * Values for chap code field.
 */
#define CHAP_CHALLENGE	1
#define CHAP_RESPONSE	2
#define CHAP_SUCCESS	3
#define CHAP_FAILURE	4

/**/
#define STATE_NONE		0
#define STATE_RECVACK	0x1
#define STATE_SENDACK	0x2


/* Value used as data for CI_CALLBACK option */
#define CBCP_OPT		6	/* Use callback control protocol */

/*
 * LCP Options.
 */
#define CI_VENDOR			0	/* Vendor Specific */
#define CI_MRU				1	/* Maximum Receive Unit */
#define CI_ASYNCMAP			2	/* Async Control Character Map */
#define CI_AUTHTYPE			3	/* Authentication Type */
#define CI_QUALITY			4	/* Quality Protocol */
#define CI_MAGICNUMBER		5	/* Magic Number */
#define CI_PCOMPRESSION		7	/* Protocol Field Compression */
#define CI_ACCOMPRESSION 	8	/* Address/Control Field Compression */
#define CI_FCSALTERN		9	/* FCS-Alternatives */
#define CI_SDP				10	/* Self-Describing-Pad */
#define CI_NUMBERED			11	/* Numbered-Mode */
#define CI_CALLBACK			13	/* callback */
#define CI_MRRU				17	/* max reconstructed receive unit; multilink */
#define CI_SSNHF			18	/* short sequence numbers for multilink */
#define CI_EPDISC			19	/* endpoint discriminator */
#define CI_MPPLUS			22	/* Multi-Link-Plus-Procedure */
#define CI_LDISC			23	/* Link-Discriminator */
#define CI_LCPAUTH			24	/* LCP Authentication */
#define CI_COBS				25	/* Consistent Overhead Byte Stuffing */
#define CI_PREFELIS			26	/* Prefix Elision */
#define CI_MPHDRFMT			27	/* MP Header Format */
#define CI_I18N				28	/* Internationalization */
#define CI_SDL				29	/* Simple Data Link */

/*
 *  IPCP Options.
 */
#define CI_ADDRS			1	/* IP Addresses */
#define CI_COMPRESSTYPE		2	/* Compression Type */
#define CI_ADDR				3

#define CI_MS_DNS1			129	/* Primary DNS value */
#define CI_MS_WINS1			130	/* Primary WINS value */
#define CI_MS_DNS2			131	/* Secondary DNS value */
#define CI_MS_WINS2			132	/* Secondary WINS value */


/*
 * Length of each type of configuration option (in octets)
 */
#define CILEN_VOID		2
#define CILEN_CHAR		3
#define CILEN_SHORT		4	/* CILEN_VOID + 2 */
#define CILEN_CHAP		5	/* CILEN_VOID + 2 + 1 */
#define CILEN_LONG		6	/* CILEN_VOID + 4 */
#define CILEN_LQR		8	/* CILEN_VOID + 2 + 4 */
#define CILEN_CBCP		3

#define CILEN_COMPRESS	4	/* min length for compression protocol opt. */
#define CILEN_VJ		6	/* length for RFC1332 Van-Jacobson opt. */
#define CILEN_ADDR		6	/* new-style single address option */
#define CILEN_ADDRS		10	/* old-style dual address option */


#define AUTH_NONE	0x0
#define AUTH_PAP	0x1
#define AUTH_CHAP	0x2
#define AUTH_EAP	0x4

/*
 * CHAP digest codes.
 */
#define CHAP_MD5			0x5
#define CHAP_MICROSOFT		0x80
#define CHAP_MICROSOFT_V2	0x81

/* bitmask of supported algorithms */
#define MDTYPE_MICROSOFT_V2	0x1
#define MDTYPE_MICROSOFT	0x2
#define MDTYPE_MD5			0x4
#define MDTYPE_NONE			0x0

/* Return the digest alg. ID for the most preferred digest type. */
#define CHAP_DIGEST(mdtype) \
	(((mdtype) & MDTYPE_MD5)? CHAP_MD5: \
	((mdtype) & MDTYPE_MICROSOFT_V2)? CHAP_MICROSOFT_V2: \
	((mdtype) & MDTYPE_MICROSOFT)? CHAP_MICROSOFT: \
	0)

/* Return the bit flag (lsb set) for our most preferred digest type. */
#define CHAP_MDTYPE(mdtype)	(((mdtype) ^ ((mdtype) - 1)) & (mdtype))

/* Return the bit flag for a given digest algorithm ID. */
#define CHAP_MDTYPE_D(digest) \
	(((digest) == CHAP_MICROSOFT_V2)? MDTYPE_MICROSOFT_V2: \
	((digest) == CHAP_MICROSOFT)? MDTYPE_MICROSOFT: \
	((digest) == CHAP_MD5)? MDTYPE_MD5: \
	0)

/* Can we do the requested digest? */
#define CHAP_CANDIGEST(mdtype, digest) \
	(((digest) == CHAP_MICROSOFT_V2)? (mdtype) & MDTYPE_MICROSOFT_V2: \
	((digest) == CHAP_MICROSOFT)? (mdtype) & MDTYPE_MICROSOFT: \
	((digest) == CHAP_MD5)? (mdtype) & MDTYPE_MD5: \
	0)


#endif
