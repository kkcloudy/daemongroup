#ifndef _RADIUS_DEF_H
#define _RADIUS_DEF_H

#define RADIUS_PACKET_HEADSIZE		20
#define RADIUS_PACKET_SIZE			4096

/* radius limits*/
#define RADIUS_MD5LEN				16	/* Length of MD5 hash */
#define RADIUS_AUTHLEN				16	/* RFC 2865: Length of authenticator */
#define RADIUS_PWSIZE				128	/* RFC 2865: Max 128 octets in password */
#define RADIUS_ATTR_VLEN			253
#define RADIUS_PASSWORD_LEN			16
#define RADIUS_CHAP_CHAL_LEN		16

struct radius_packet {
	uint8 code;
	uint8 id;
	uint16 length;
	uint8 authenticator[RADIUS_AUTHLEN];
	uint8 data[0];
} __attribute__ ((packed));

struct radius_attr {
	uint8 t;
	uint8 l;
	union {
		uint32 i;
		uint8 t[RADIUS_ATTR_VLEN];
		struct {
			uint32 i;
			uint8 t;
			uint8 l;
			union {
				uint32 i;
				uint8 t[RADIUS_ATTR_VLEN - 4];
			} v;
		} vv;
	} v;
} __attribute__ ((packed));

/* Radius packet types */
#define RADIUS_CODE_ACCESS_REQUEST				1
#define RADIUS_CODE_ACCESS_ACCEPT				2
#define RADIUS_CODE_ACCESS_REJECT				3
#define RADIUS_CODE_ACCOUNTING_REQUEST			4
#define RADIUS_CODE_ACCOUNTING_RESPONSE			5
#define RADIUS_CODE_ACCESS_CHALLENGE			11
#define RADIUS_CODE_STATUS_SERVER				12
#define RADIUS_CODE_STATUS_CLIENT				13
#define RADIUS_CODE_DISCONNECT_REQUEST			40
#define RADIUS_CODE_DISCONNECT_ACK				41
#define RADIUS_CODE_DISCONNECT_NAK				42
#define RADIUS_CODE_COA_REQUEST					43
#define RADIUS_CODE_COA_ACK						44
#define RADIUS_CODE_COA_NAK						45
#define RADIUS_CODE_STATUS_REQUEST				46
#define RADIUS_CODE_STATUS_ACCEPT				47
#define RADIUS_CODE_STATUS_REJECT				48

/* Radius attributes */
#define RADIUS_ATTR_USER_NAME					1	/* string */
#define RADIUS_ATTR_USER_PASSWORD				2	/* string (encrypt) */
#define RADIUS_ATTR_CHAP_PASSWORD				3	/* octets */
#define RADIUS_ATTR_NAS_IP_ADDRESS				4	/* ipaddr */
#define RADIUS_ATTR_NAS_PORT					5	/* integer */
#define RADIUS_ATTR_SERVICE_TYPE				6	/* integer */
#define RADIUS_ATTR_FRAMED_PROTOCOL				7	/* integer */
#define RADIUS_ATTR_FRAMED_IP_ADDRESS			8	/* ipaddr */
#define RADIUS_ATTR_FRAMED_IP_NETMASK			9	/* ipaddr */
#define RADIUS_ATTR_FRAMED_ROUTING				10	/* integer */
#define RADIUS_ATTR_FILTER_ID					11	/* string */
#define RADIUS_ATTR_FRAMED_MTU					12	/* integer */
#define RADIUS_ATTR_FRAMED_COMPRESSION			13	/* integer */
#define RADIUS_ATTR_LOGIN_IP_HOST				14	/* ipaddr */
#define RADIUS_ATTR_LOGIN_SERVICE				15	/* integer */
#define RADIUS_ATTR_LOGIN_TCP_PORT				16	/* integer */
#define RADIUS_ATTR_REPLY_MESSAGE				18	/* string */
#define RADIUS_ATTR_CALLBACK_NUMBER				19	/* string */
#define RADIUS_ATTR_CALLBACK_ID					20	/* string */
#define RADIUS_ATTR_FRAMED_ROUTE				22	/* string */
#define RADIUS_ATTR_FRAMED_IPX_NETWORK			23	/* ipaddr */
#define RADIUS_ATTR_STATE						24	/* octets */
#define RADIUS_ATTR_CLASS						25	/* octets */
#define RADIUS_ATTR_VENDOR_SPECIFIC				26	/* octets */
#define RADIUS_ATTR_SESSION_TIMEOUT				27	/* integer */
#define RADIUS_ATTR_IDLE_TIMEOUT				28	/* integer */
#define RADIUS_ATTR_TERMINATION_ACTION			29	/* integer */
#define RADIUS_ATTR_CALLED_STATION_ID			30	/* string */
#define RADIUS_ATTR_CALLING_STATION_ID			31	/* string */
#define RADIUS_ATTR_NAS_IDENTIFIER				32	/* string */
#define RADIUS_ATTR_PROXY_STATE					33	/* octets */
#define RADIUS_ATTR_LOGIN_LAT_SERVICE			34	/* string */
#define RADIUS_ATTR_LOGIN_LAT_NODE				35	/* string */
#define RADIUS_ATTR_LOGIN_LAT_GROUP				36	/* octets */
#define RADIUS_ATTR_FRAMED_APPLETALK_LINK		37	/* integer */
#define RADIUS_ATTR_FRAMED_APPLETALK_NETWORK	38	/* integer */
#define RADIUS_ATTR_FRAMED_APPLETALK_ZONE		39	/* string */
#define RADIUS_ATTR_ACCT_STATUS_TYPE			40	/* integer */
#define RADIUS_ATTR_ACCT_DELAY_TIME				41	/* integer */
#define RADIUS_ATTR_ACCT_INPUT_OCTETS			42	/* integer */
#define RADIUS_ATTR_ACCT_OUTPUT_OCTETS			43	/* integer */
#define RADIUS_ATTR_ACCT_SESSION_ID				44	/* string */
#define RADIUS_ATTR_ACCT_AUTHENTIC				45	/* integer */
#define RADIUS_ATTR_ACCT_SESSION_TIME			46	/* integer */
#define RADIUS_ATTR_ACCT_INPUT_PACKETS			47	/* integer */
#define RADIUS_ATTR_ACCT_OUTPUT_PACKETS			48	/* integer */
#define RADIUS_ATTR_ACCT_TERMINATE_CAUSE		49	/* integer */
#define RADIUS_ATTR_ACCT_MULTI_SESSION_ID		50	/* string */
#define RADIUS_ATTR_ACCT_LINK_COUNT				51	/* integer */
#define RADIUS_ATTR_ACCT_INPUT_GIGAWORDS		52	/* integer */
#define RADIUS_ATTR_ACCT_OUTPUT_GIGAWORDS		53	/* integer */
#define RADIUS_ATTR_EVENT_TIMESTAMP				55	/* date */
#define RADIUS_ATTR_CHAP_CHALLENGE				60	/* string */
#define RADIUS_ATTR_NAS_PORT_TYPE				61	/* integer */
#define RADIUS_ATTR_PORT_LIMIT					62	/* integer */
#define RADIUS_ATTR_LOGIN_LAT_PORT				63	/* integer */
#define RADIUS_ATTR_ACCT_TUNNEL_CONNECTION		68	/* string */
#define RADIUS_ATTR_ARAP_PASSWORD				70	/* string */
#define RADIUS_ATTR_ARAP_FEATURES				71	/* string */
#define RADIUS_ATTR_ARAP_ZONE_ACCESS			72	/* integer */
#define RADIUS_ATTR_ARAP_SECURITY				73	/* integer */
#define RADIUS_ATTR_ARAP_SECURITY_DATA			74	/* string */
#define RADIUS_ATTR_PASSWORD_RETRY				75	/* integer */
#define RADIUS_ATTR_PROMPT						76	/* integer */
#define RADIUS_ATTR_CONNECT_INFO				77	/* string */
#define RADIUS_ATTR_CONFIGURATION_TOKEN			78	/* string */
#define RADIUS_ATTR_EAP_MESSAGE					79	/* string */
#define RADIUS_ATTR_MESSAGE_AUTHENTICATOR		80	/* octets */
#define RADIUS_ATTR_ARAP_CHALLENGE_RESPONSE		84	/* string # 10 octets */
#define RADIUS_ATTR_ACCT_INTERIM_INTERVAL		85	/* integer */
#define RADIUS_ATTR_NAS_PORT_ID					87	/* string */
#define RADIUS_ATTR_FRAMED_POOL					88	/* string */
#define RADIUS_ATTR_NAS_IPV6_ADDRESS			95	/* octets (IPv6) */
#define RADIUS_ATTR_FRAMED_INTERFACE_ID			96	/* octets # 8 octets */
#define RADIUS_ATTR_FRAMED_IPV6_PREFIX			97	/* octets ??? */
#define RADIUS_ATTR_LOGIN_IPV6_HOST				98	/* octets (IPv6) */
#define RADIUS_ATTR_FRAMED_IPV6_ROUTE			99	/* string */
#define RADIUS_ATTR_FRAMED_IPV6_POOL			100	/* string */
#define RADIUS_ATTR_ERROR_CAUSE					101	/* integer */
#define RADIUS_ATTR_DIGEST_RESPONSE				206	/* string */
#define RADIUS_ATTR_DIGEST_ATTRIBUTES			207	/* octets  ??? */

/* Radius server type */
#define RADIUS_SERVICE_TYPE_LOGIN					1
#define RADIUS_SERVICE_TYPE_FRAMED					2
#define RADIUS_SERVICE_TYPE_CALLBACK_LOGIN			3
#define RADIUS_SERVICE_TYPE_CALLBACK_FRAME			4
#define RADIUS_SERVICE_TYPE_OUTBOUND				5
#define RADIUS_SERVICE_TYPE_ADMINISTRATIVE			6
#define RADIUS_SERVICE_TYPE_NAS_PROMPT				7
#define RADIUS_SERVICE_TYPE_AUTHENTICATE_ONLY		8
#define RADIUS_SERVICE_TYPE_CALLBACK_NAS_PROMPT		9

/*Radius fram protocol */
#define RADIUS_FRAMED_PROTOCOL_PPP					1
#define RADIUS_FRAMED_PROTOCOL_SLIP					2
#define RADIUS_FRAMED_PROTOCOL_ARA					3
#define RADIUS_FRAMED_PROTOCOL_GANDALF				4
#define RADIUS_FRAMED_PROTOCOL_XYLOGICS				5


/* Radius status type */
#define RADIUS_STATUS_TYPE_START					1
#define RADIUS_STATUS_TYPE_STOP						2
#define RADIUS_STATUS_TYPE_INTERIM_UPDATE			3
#define RADIUS_STATUS_TYPE_ACCOUNTING_ON			7
#define RADIUS_STATUS_TYPE_ACCOUNTING_OFF			8

/* Radius nas port type */
#define RADIUS_NAS_PORT_TYPE_VIRTUAL				5
#define RADIUS_NAS_PORT_TYPE_WIRELESS_802_11		19
#define RADIUS_NAS_PORT_TYPE_WIRELESS_UMTS			23

/* Radius terminate cause */
#define RADIUS_TERMINATE_CAUSE_USER_REQUEST			1
#define RADIUS_TERMINATE_CAUSE_LOST_CARRIER			2
#define RADIUS_TERMINATE_CAUSE_LOST_SERVICE			3
#define RADIUS_TERMINATE_CAUSE_IDLE_TIMEOUT			4
#define RADIUS_TERMINATE_CAUSE_SESSION_TIMEOUT		5
#define RADIUS_TERMINATE_CAUSE_ADMIN_RESET			6
#define RADIUS_TERMINATE_CAUSE_ADMIN_REBOOT			7
#define RADIUS_TERMINATE_CAUSE_PORT_ERROR			8
#define RADIUS_TERMINATE_CAUSE_NAS_ERROR			9
#define RADIUS_TERMINATE_CAUSE_NAS_REQUEST			10
#define RADIUS_TERMINATE_CAUSE_NAS_REBOOT			11
#define RADIUS_TERMINATE_CAUSE_PORT_UNNEEDED		12
#define RADIUS_TERMINATE_CAUSE_PORT_PREEMPTED		13
#define RADIUS_TERMINATE_CAUSE_PORT_SUSPEND			14
#define RADIUS_TERMINATE_CAUSE_SERVICE_UNAVAILABLE	15
#define RADIUS_TERMINATE_CAUSE_CALLBACK				16
#define RADIUS_TERMINATE_CAUSE_USER_ERROR			17
#define RADIUS_TERMINATE_CAUSE_HOST_REQUEST			18

/* Radius error cause */
#define RADIUS_ERROR_CAUSE_201		201 /* Residual Session Context Removed */
#define RADIUS_ERROR_CAUSE_402		402 /* Missing Attribute */
#define RADIUS_ERROR_CAUSE_503		503 /* Session Context Not Found */

/* vendor  attr - WISPR */
#define RADIUS_VENDOR_WISPR								14122
#define RADIUS_ATTR_WISPR_LOCATION_ID					1
#define RADIUS_ATTR_WISPR_LOCATION_NAME					2	/* string */
#define RADIUS_ATTR_WISPR_LOGOFF_URL					3	/* string */
#define RADIUS_ATTR_WISPR_REDIRECTION_URL				4	/* string */
#define RADIUS_ATTR_WISPR_BANDWIDTH_MIN_UP				5	/* integer */
#define RADIUS_ATTR_WISPR_BANDWIDTH_MIN_DOWN			6	/* integer */
#define RADIUS_ATTR_WISPR_BANDWIDTH_MAX_UP				7	/* integer */
#define RADIUS_ATTR_WISPR_BANDWIDTH_MAX_DOWN			8	/* integer */
#define RADIUS_ATTR_WISPR_SESSION_TERMINATE_TIME		9	/* string */
#define RADIUS_ATTR_WISPR_SESSION_TERMINATE_END_OF_DAY	10	/* string */
#define RADIUS_ATTR_WISPR_BILLING_CLASS_OF_SERVICE		11	/* string */

/* vendor  attr - HUAWEI */
#define RADIUS_VENDOR_HUAWEI						2011
#define RADIUS_ATTR_HUAWEI_AVE_BANDWIDTH_MAX_DOWN	2	/* integer */
#define RADIUS_ATTR_HUAWEI_AVE_BANDWIDTH_MAX_UP		5	/* integer */

/* authentic types */
#define RADIUS_AUTHENTIC_TYPE_RADIUS		1
#define RADIUS_AUTHENTIC_TYPE_LOCAL			2
#define RADIUS_AUTHENTIC_TYPE_REMOTE		3


#endif
