#ifndef DCLI_SEAGENT_H
#define DCLI_SEAGENT_H

extern void dcli_se_agent_init(void);
#define IP_FMT(m)	\
				((uint8_t*)&(m))[0], \
				((uint8_t*)&(m))[1], \
				((uint8_t*)&(m))[2], \
				((uint8_t*)&(m))[3]

#define IPV6_FMT(m)	\
				((uint16_t*)&(m))[0], \
				((uint16_t*)&(m))[1], \
				((uint16_t*)&(m))[2], \
				((uint16_t*)&(m))[3]

#define MAC_FMT(m)  \
				((uint8_t*)(m))[0], \
				((uint8_t*)(m))[1], \
				((uint8_t*)(m))[2], \
				((uint8_t*)(m))[3], \
				((uint8_t*)(m))[4], \
				((uint8_t*)(m))[5]

#define PROTO_STR(t)  ((t) == 0x6 ? "TCP" : ((t) == 0x11 ? "UDP" : "Unknown"))

#define DCLI_IPPORT_STRING_MAXLEN	(strlen("255.255.255.255:65535"))
#define DCLI_IPPORT_STRING_MINLEN   (strlen("0.0.0.0:0"))

#define DCLI_IP_STRING_MAXLEN	    (strlen("255.255.255.255"))
#define DCLI_IP_STRING_MINLEN       (strlen("0.0.0.0"))
#define DCLI_PORT_STRING_MAXLEN     (strlen("65535"))
#define DCLI_PORT_STRING_MINLEN     (strlen("0"))

#define MAX_PORT  65535   /*TCP or UDP  maximum port number*/


/*********************** Error  message **********************************************/
#define AGENT_NO_RESPOND_STR         "%% Failed:agent is not responding\n"
#define WRITE_FAIL_STR               "%% Failed:communication failure with agent module\n"
#define CMD_PARAMETER_ERROR          "%% Failed:command parameter format error\n"
#define COMMAND_FAIL_STR             "%% Failed:command failed\n"
#define CMD_PARAMETER_NUM_ERROR      "%% Failed:command parameters number error!\n"

typedef enum
{
    CVMX_POW_TAG_TYPE_ORDERED   = 0L,   /**< Tag ordering is maintained */
    CVMX_POW_TAG_TYPE_ATOMIC    = 1L,   /**< Tag ordering is maintained, and at most one PP has the tag */
    CVMX_POW_TAG_TYPE_NULL      = 2L,   /**< The work queue entry from the order
                                            - NEVER tag switch from NULL to NULL */
    CVMX_POW_TAG_TYPE_NULL_NULL = 3L    /**< A tag switch to NULL, and there is no space reserved in POW
                                            - NEVER tag switch to NULL_NULL
                                            - NEVER tag switch from NULL_NULL
                                            - NULL_NULL is entered at the beginning of time and on a deschedule.
                                            - NULL_NULL can be exited by a new work request. A NULL_SWITCH load can also switch the state to NULL */
} cvmx_pow_tag_type_t;

#endif
