#ifndef _WS_SNMPD_MANUAL_H
#define _WS_SNMPD_MANUAL_H

/* lixiang modify at 2012-02-15 
 * change ifname size from 10 to 32
 */
#define MIB_IFNAME_SIZE    32 

struct mib_acif_stats {
	char ifname[MIB_IFNAME_SIZE];

    unsigned int acIfInNUcastPkts;
	unsigned int acIfInDiscardPkts;
	unsigned int acIfInErrors;	
    unsigned int acIfInMulticastPkts;
    
	unsigned int acIfOutDiscardPkts;
	unsigned int acIfOutErrors;
	unsigned int acIfOutNUcastPkts;
    unsigned int acIfOutMulticastPkts;

    struct mib_acif_stats *next;
};
        

#endif
