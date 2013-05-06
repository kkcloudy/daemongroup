#ifndef __ACCAPI_DBA_H__
#define __ACCAPI_DBA_H__


struct dba_result {
	unsigned int module_type;
	int result;
	unsigned int len;	/* length of data */
	void *data;	
};

typedef struct dba_result dba_result_t;

/* dba_result.module */
#define PPPOE_SNOOPING_KMOD		0x01
#define DBA_KMOD				0x02
#define DHCP_OPTION82_KMOD		0x04

/* dba_result.result */
#define DBA_CONSUMED			0x01	/* skb is consumed by module */
#define DBA_PASS				0x02	/* module do nothing to skb */
#define DBA_ERROR				0x04	/* */
#define DBA_HANDLED				0x08	/* module handle skb, go on */

#endif
