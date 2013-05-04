#ifndef __DCLI_QOS_H__
#define __DCLI_QOS_H__

#define QOS_ERROR_NONE	-1
#define QOS_SUCCESS			        (QOS_ERROR_NONE+1)
#define QOS_FAIL                    (QOS_ERROR_NONE+2)
#define QOS_BAD_PARAM               (QOS_ERROR_NONE+3)
#define QOS_PROFILE_EXISTED			(QOS_ERROR_NONE + 4)
#define QOS_PROFILE_NOT_EXISTED 	(QOS_ERROR_NONE + 5)
#define QOS_POLICY_EXISTED			(QOS_ERROR_NONE + 6)
#define QOS_POLICY_NOT_EXISTED 		(QOS_ERROR_NONE + 7)
#define QOS_POLICY_MAP_BIND		    (QOS_ERROR_NONE + 8)
#define QOS_POLICER_NOT_EXISTED 	(QOS_ERROR_NONE + 9)
#define QOS_COUNTER_NOT_EXISTED 	(QOS_ERROR_NONE + 10)
#define QOS_POLICY_MAP_PORT_WRONG   (QOS_ERROR_NONE + 11)
#define QOS_POLICER_USE_IN_ACL      (QOS_ERROR_NONE + 12)
#define QOS_TRAFFIC_NO_INFO         (QOS_ERROR_NONE + 13)
#define QOS_PROFILE_IN_USE         (QOS_ERROR_NONE + 14)
#define QOS_POLICER_CBS_BIG         (QOS_ERROR_NONE + 15)
#define QOS_POLICER_CBS_LITTLE         (QOS_ERROR_NONE + 16)
#define QOS_NO_MAPPED         (QOS_ERROR_NONE + 17)

#define QOS_POLICER_ENABLE		1
#define QOS_POLICER_DISABLE 	0
#define QOS_BAD_PTR					(QOS_ERROR_NONE + 100)
#define QOS_SCH_GROUP_IS_SP	1024		

void dcli_qos_init();

extern int dcli_str2ulong
(
	char *str,
	unsigned int *value
);

typedef struct
{
	unsigned int flag;
	unsigned int profileIndex;
}DCLI_QOS_REMAP_SHOW_STC;

#endif

