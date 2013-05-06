#ifndef _AC_SAMPLE_DEF_H
#define _AC_SAMPLE_DEF_H

typedef struct acsample_conf_t{
	int				sample_on;
	unsigned int		statistics_time;
	unsigned int		sample_interval;
	unsigned long	resend_interval;
	unsigned int		reload_config_interval;
	int				over_threshold_check_type;	//not use
	int				clear_check_type;				//not use
	int				threshold;
	
}acsample_conf;






struct cpu_stat_t{
    unsigned long long user;
    unsigned long long nice;
    unsigned long long system;
    unsigned long long idle;
    unsigned long long iowait;
    unsigned long long irq;
    unsigned long long softirq;
    unsigned long long unknown;
    
    unsigned long long used;
    unsigned long long total;
};

typedef struct cpu_in{
	unsigned int cpu_cont;
	double cpu_info[16];
	double cpu_average;
}cpu_in_t;


/*sample config file*/
#define ACSAMPLE_CONF_PATH	"/opt/services/option/acsampleconf_option"


#define DEFAULT_TIMEOUT 	500		//ms
#define SE_XML_ACSAMPLE_TIMEOUT					"timeout"

#define SE_XML_ACSAMPLE_SWITCH					"switch"
#define SE_XML_ACSAMPLE_STATIC_TIME				"statistic_time"
#define SE_XML_ACSAMPLE_INTERVAL					"sample_interval"
#define SE_XML_ACSAMPLE_RESEND_INTERVAL			"resend_interval"
#define SE_XML_ACSAMPLE_RELOAD_INTERVAL			"config_reload_interval"
#define SE_XML_ACSAMPLE_OVER_THRESHOLD_TYPE		"overthreshold_type"
#define SE_XML_ACSAMPLE_CLEAR_TYPE				"clear_type"
#define SE_XML_ACSAMPLE_THRESHOLD				"threshold"





/*sample global param*/
#define AC_SAMPLE_PARAM_TYPE_INTERVAL           				1
#define AC_SAMPLE_PARAM_TYPE_STATISTICS         				2
#define AC_SAMPLE_PARAM_TYPE_RESEND_INTERVAL    			3
#define AC_SAMPLE_PARAM_TYPE_CONFIG_RELOAD_INTERVAL	4
#define AC_SAMPLE_PARAM_TYPE_SHOKET_TIMOUT				5
#define AC_SAMPLE_PARAM_TYPE_SWITCH						6
#define AC_SAMPLE_PARAM_TYPE_THRESHOLD					7
//#define AC_SAMPLE_PARAM_TYPE_TIMEOUT						8


#define SERVER_REACH_THRESHOLD		1
#define SERVER_NOT_REACH_THRESHOLD 	2
#define DEFAULT_HALF_THRESHOLD		50
#define SIXTY_THRESHOLD		60

/*syslog level name*/
#define LOG_LEVEL_NAME_DEBUG    "debug"

/*sample limit*/
#define MIN_SAMPLE_INTERVAL     5
#define MIN_STATISTICS_TIME     MIN_SAMPLE_INTERVAL


/*sample name*/
#define SAMPLE_NAME_CPURT				"cpurt"
#define SAMPLE_NAME_CPU					"cpu"
#define SAMPLE_NAME_MEMUSAGE			"memusage"
#define SAMPLE_NAME_DHCPUSE				"dhcpusage"
#define SAMPLE_NAME_TMP					"temperature"
#define SAMPLE_NAME_DROPRATE 			"droprate"
#define SAMPLE_NAME_MAX_USER			"maxuser"
#define SAMPLE_NAME_RADIUS_REQ_SUC		"radiusserver"
#define SAMPLE_NAME_IP_POOL				"ippool"

#define SAMPLE_NAME_PORTER_NOT_REACH 	"porterserver"
#define SAMPLE_NAME_RADIUS_AUTH			"radiusauth"
#define SAMPLE_NAME_RADIUS_COUNT		"radiuscount"

#define SAMPLE_NAME_DHCP_IPPOOL			"dhcpexhaust"
#define SAMPLE_NAME_FINE_SYN_ATTACK		"synattack"

#define SAMPLE_NAME_INTERFACE_FLOW		"interfaceflow"


#define AC_SAMPLE_ARRAY		{SAMPLE_NAME_CPU,SAMPLE_NAME_MEMUSAGE,SAMPLE_NAME_TMP,SAMPLE_NAME_DHCPUSE}

#define AC_SAMPLE_SECOND_ARRAY  {SAMPLE_NAME_DROPRATE,\
									SAMPLE_NAME_MAX_USER,\
									SAMPLE_NAME_RADIUS_REQ_SUC,\
									SAMPLE_NAME_PORTER_NOT_REACH,\
									SAMPLE_NAME_RADIUS_AUTH,\
									SAMPLE_NAME_RADIUS_COUNT,\
									SAMPLE_NAME_FINE_SYN_ATTACK,\
									SAMPLE_NAME_INTERFACE_FLOW}

/*sample state*/
#define SAMPLE_OFF	0
#define SAMPLE_ON	1

/*instance state*/
#define SAMPLE_INSTANCE_STANDBY     0
#define SAMPLE_INSTANCE_ACTIVE      1

/*signal state*/
#define NOT_OVER_THRESHOLD      0		/*for trap clear*/
#define OVER_THRESHOLD_FLAG     1		/*for trap*/


/*all service state*/
#define SAMPLE_SERVICE_STATE_ON		1
#define SAMPLE_SERVICE_STATE_OFF	0

/* sample tipc type*/
#define SAMPLE_TIPC_TYPE	(0x7002)

/*sample defualt  configuration*/
#define DEFAULT_SAMPLE_INTERVAL_CPURT		1	/*modified by stephen for cpu rt stat*/
#define DEFAULT_STATISTICS_TIME_CPURT		60	/*modified by stephen for cpu rt stat*/			

#define DEFAULT_SAMPLE_INTERVAL				5	/*default 5 for  guangdong china mobile*/
#define DEFAULT_STATISTICS_TIME				300 /*default 300 for  guangdong china mobile*/
#define DEFAULT_RESEND_INTERVAL				0	//from 1.3.11 trap-helper can do resend stop acsample resend
#define DEFAULT_CPU_THRESHOLD				80
#define DEFAULT_BAND_THRESHOLD 				50
#define DEFAULT_DROP_THRESHOLD				50
#define DEFAULT_IPPOOL_RATE_THRESHOLD		50
#define DEFAULT_RADIUS_REQ_SUC_THRESHOLD		50
#define DEFAULT_ONLINE_USER_THRESHOLD		50
#define DEFAULT_MEM_THRESHOLD				75
#define DEFAULT_TMP_THRESHOLD				65
#define DEFAULT_SERVER_STATE				SAMPLE_SERVICE_STATE_OFF
#define DEFAULT_CONFIG_RELOAD_INTERVAL		0
#define DEFAULT_THRESHOLD						75
#define DEFAULT_RADIUS_AUTH_STATISTICS_TIME	1800
#define DEFAULT_RADIUS_ACCT_STATISTICS_TIME	1800
#define DEFAULT_PORTAL_STATISTIC_TIME			900
#define DEFAULT_RADIUS_AUTH_SAMPLE_INTERVAL	600
#define DEFAULT_RADIUS_ACCT_SAMPLE_INTERVAL	600
#define DEFAULT_PORTAL_SAMPLE_INTERVAL		300
#define DEFAULT_SAMPLE_SERVER_THRESHOLD		90
#define NORM_STAT_THRESHOLD 		1
#define FIND_ATTACK_THRESHOLD    	2
#define DEFAULT_ZERO_THRESHOLD	0


/*syslog  level*/
#define LOG_EMERG_MASK			(LOG_MASK(LOG_EMERG))
#define LOG_ALERT_MASK			(LOG_MASK(LOG_ALERT))
#define LOG_CRIT_MASK			(LOG_MASK(LOG_CRIT))
#define LOG_ERR_MASK			(LOG_MASK(LOG_ERR))
#define LOG_WARNING_MASK		(LOG_MASK(LOG_WARNING))
#define LOG_NOTICE_MASK			(LOG_MASK(LOG_NOTICE))
#define LOG_INFO_MASK			(LOG_MASK(LOG_INFO))
#define LOG_DEBUG_MASK			(LOG_MASK(LOG_DEBUG))
#define LOG_USER_MASK			(LOG_MASK(LOG_USER))
#define LOG_AT_LEAST_ALERT		LOG_EMERG_MASK|LOG_ALERT_MASK
#define LOG_AT_LEAST_CRIT		LOG_AT_LEAST_ALERT|LOG_CRIT_MASK
#define LOG_AT_LEAST_ERR		LOG_AT_LEAST_CRIT|LOG_ERR_MASK
#define LOG_AT_LEAST_WARNING	LOG_AT_LEAST_ERR|LOG_WARNING_MASK
#define LOG_AT_LEAST_NOTICE		LOG_AT_LEAST_WARNING|LOG_NOTICE_MASK
#define LOG_AT_LEAST_INFO		LOG_AT_LEAST_NOTICE|LOG_INFO_MASK
#define LOG_AT_LEAST_DEBUG		LOG_AT_LEAST_INFO|LOG_DEBUG_MASK
#define LOG_AT_LEAST_USER		LOG_AT_LEAST_DEBUG|LOG_USER_MASK

/*sample mode*/
#define SET_DEFAULT_SAMPLE_MODE 		0
#define SAMPLE_MODE_SINGLE			0
#define SAMPLE_MODE_MULTI				1

/*subsample flag*/
#define SUBSAMPLE_CLEAR 			0
#define SUBSAMPLE_CANCELED		0
#define SUBSAMPLE_DO				1


#endif

