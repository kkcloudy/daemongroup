#ifndef __DCLI_ACL_H__
#define __DCLI_ACL_H__

#define STANDARD_ACL_RULE 0
#define EXTENDED_ACL_RULE 1
#define MAX_IP_STRLEN 16

#define ACCESS_PORT_TYPE 0
#define ACCESS_VID_TYPE  1

#define ACL_TRUE	1
#define ACL_FALSE   0

#define ALIAS_NAME_SIZE 		0x15
#define ALIAS_NAME_LEN_ERROR	0x1
#define ALIAS_NAME_HEAD_ERROR	(ALIAS_NAME_LEN_ERROR+1)
#define ALIAS_NAME_BODY_ERROR	(ALIAS_NAME_LEN_ERROR+2)
#define TIME_SPLIT_DASH 	'/'
#define TIME_SPLIT_SLASH	':'

#define NPD_ACL_RULE_SHOWRUN_CFG_SIZE (100*1024)

#define ACL_DIRECTION_INGRESS 0
#define ACL_DIRECTION_EGRESS  1
#define ACL_DIRECTION_TWOWAY  2

#define ACL_TIME_NAME_EXIST    1
#define ACL_TIME_NAME_NOTEXIST 2
#define ACL_TIME_PERIOD_NOT_EXISTED 3
#define ACL_TIME_PERIOD_EXISTED 4
#define MAX_EXT_RULE_NUM	500

#define ACL_GROUP_ERROR_NONE	0
#define ACL_GROUP_SUCCESS			(ACL_GROUP_ERROR_NONE)
#define ACL_GROUP_EXISTED			(ACL_GROUP_ERROR_NONE + 1)
#define ACL_GROUP_NOT_EXISTED 		(ACL_GROUP_ERROR_NONE + 16)
#define ACL_GROUP_PORT_NOTFOUND		(ACL_GROUP_ERROR_NONE + 3)
#define ACL_GROUP_RULE_NOTEXISTED	(ACL_GROUP_ERROR_NONE + 4)
#define ACL_GROUP_RULE_EXISTED		(ACL_GROUP_ERROR_NONE + 5)
#define NPD_DBUS_ACL_ERR_GENERAL	(ACL_GROUP_ERROR_NONE + 6)
#define ACL_GROUP_PORT_BINDED    	(ACL_GROUP_ERROR_NONE + 7)
#define ACL_GROUP_NOT_SHARE         (ACL_GROUP_ERROR_NONE + 8)
#define ACL_GLOBAL_NOT_EXISTED      (ACL_GROUP_ERROR_NONE + 9)
#define ACL_GLOBAL_EXISTED          (ACL_GROUP_ERROR_NONE + 10)
#define ACL_SAME_FIELD				(ACL_GROUP_ERROR_NONE + 11)
#define ACL_EXT_NO_SPACE			(ACL_GROUP_ERROR_NONE + 12)
#define ACL_GROUP_NOT_BINDED			(ACL_GROUP_ERROR_NONE + 13)
#define ACL_GROUP_VLAN_BINDED           (ACL_GROUP_ERROR_NONE + 14)
#define ACL_GROUP_EGRESS_ERROR          (ACL_GROUP_ERROR_NONE + 17)
#define EGRESS_ACL_GROUP_RULE_EXISTED  (ACL_GROUP_ERROR_NONE + 18)
#define ACL_GROUP_EGRESS_NOT_SUPPORT         (ACL_GROUP_ERROR_NONE + 19)
#define ACL_GROUP_WRONG_INDEX		 (ACL_GROUP_ERROR_NONE + 20)
#define ACL_ON_PORT_DISABLE         (ACL_GROUP_ERROR_NONE + 21)
#define ACL_POLICER_ID_NOT_SET      (ACL_GROUP_ERROR_NONE + 22)
#define ACL_GROUP_SAME_ID		    (ACL_GROUP_ERROR_NONE + 23)
#define ACL_RULE_TIME_NOT_SUPPORT         (ACL_GROUP_ERROR_NONE + 24)
#define ACL_RULE_INDEX_ERROR             (ACL_GROUP_ERROR_NONE + 25)
#define ACL_GROUP_INDEX_ERROR             (ACL_GROUP_ERROR_NONE + 26)
#define ACL_MIRROR_USE					(ACL_GROUP_ERROR_NONE + 27)
#define ACL_RULE_EXT_ONLY					(ACL_GROUP_ERROR_NONE + 28)
#define ACL_UNBIND_FRIST					(ACL_GROUP_ERROR_NONE + 29)
#define ACL_ADD_EQUAL_RULE					(ACL_GROUP_ERROR_NONE + 30)
#define ACL_RANGE_NOT_EXIST					(ACL_GROUP_ERROR_NONE + 31)
#define ACL_UDP_VLAN_RULE_ENABLE				(ACL_GROUP_ERROR_NONE + 32)
#define ACL_PORT_NOT_SUPPORT_BINDED				(ACL_GROUP_ERROR_NONE + 33)

#define ACL_NORMAL_MALLOC_ERROR_NONE	 (ACL_GROUP_ERROR_NONE + 100)

#define ACL_ANY_PORT NPD_ACL_RULE_SHOWRUN_CFG_SIZE
#define ACL_ANY_PORT_CHAR 255

		
void dcli_acl_init(void);

struct ipv6addr {
	unsigned char ipbuf [16];
};

typedef struct
{
    unsigned char       arEther[6];
}ETHERADDR;
int dcli_checkPoint(char *ptr);

int dcli_str2ulong
(
	char *str,
	unsigned int *value
);
unsigned long dcli_ip2ulong
(
	char *str
);
int ip_address_format2ulong
(
	char ** buf,
	unsigned long *ipAddress,
	unsigned int * mask
);

unsigned int ip_long2str
(
	unsigned long ipAddress,
	unsigned char **buff
);
int timeRange_time_check_illegal
(	 
	unsigned int sm,
	unsigned int sd,
	unsigned int sh,
	unsigned int smt	
);
int timeRange_time_hour_check_illegal
(	 
	unsigned int sh,
	unsigned int sm 
);
int timeRange_name_legal_check
(
	char* str,
	unsigned int len
);

int timeRange_absolute_deal
 (
 	char *str,
 	unsigned int *startyear,
 	unsigned int *startmonth,
 	unsigned int *startday,
 	unsigned int *starthour,
 	unsigned int *startminute
 );
int timeRange_time_deal
(
	char *str,
	unsigned int *hour,
	unsigned int *minute
);

#endif
