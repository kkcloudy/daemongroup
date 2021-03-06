/*
 * Note: this file originally auto-generated by mib2c using
 *       version : 14170 $ of $
 *
 * $Id:$
 */
#ifndef DOT11DHCPIPV6USEAGETABLE_H
#define DOT11DHCPIPV6USEAGETABLE_H

#ifdef __cplusplus
extern "C" {
#endif


/** @addtogroup misc misc: Miscellaneous routines
 *
 * @{
 */
#include <net-snmp/library/asn1.h>

/* other required module components */
    /* *INDENT-OFF*  */
config_add_mib(DOT11-AC-MIB)
config_require(DOT11-AC-MIB/dot11DHCPIpv6UseageTable/dot11DHCPIpv6UseageTable_interface)
config_require(DOT11-AC-MIB/dot11DHCPIpv6UseageTable/dot11DHCPIpv6UseageTable_data_access)
config_require(DOT11-AC-MIB/dot11DHCPIpv6UseageTable/dot11DHCPIpv6UseageTable_data_get)
config_require(DOT11-AC-MIB/dot11DHCPIpv6UseageTable/dot11DHCPIpv6UseageTable_data_set)
    /* *INDENT-ON*  */

/* OID and column number definitions for dot11DHCPIpv6UseageTable */
#include "dot11DHCPIpv6UseageTable_oids.h"

/* enum definions */
#include "dot11DHCPIpv6UseageTable_enums.h"

/* *********************************************************************
 * function declarations
 */
void init_dot11DHCPIpv6UseageTable(void);
void shutdown_dot11DHCPIpv6UseageTable(void);

/* *********************************************************************
 * Table declarations
 */
/**********************************************************************
 **********************************************************************
 ***
 *** Table dot11DHCPIpv6UseageTable
 ***
 **********************************************************************
 **********************************************************************/
/*
 * DOT11-AC-MIB::dot11DHCPIpv6UseageTable is subid 8 of dot11Service.
 * Its status is Current.
 * OID: .1.3.6.1.4.1.31656.6.1.2.6.8, length: 12
*/
/* *********************************************************************
 * When you register your mib, you get to provide a generic
 * pointer that will be passed back to you for most of the
 * functions calls.
 *
 * TODO:100:r: Review all context structures
 */
    /*
     * TODO:101:o: |-> Review dot11DHCPIpv6UseageTable registration context.
     */
typedef netsnmp_data_list dot11DHCPIpv6UseageTable_registration;

/**********************************************************************/
/*
 * TODO:110:r: |-> Review dot11DHCPIpv6UseageTable data context structure.
 * This structure is used to represent the data for dot11DHCPIpv6UseageTable.
 */
/*
 * This structure contains storage for all the columns defined in the
 * dot11DHCPIpv6UseageTable.
 */
typedef struct dot11DHCPIpv6UseageTable_data_s {
    
        /*
         * DHCPIpv6poolName(2)/DisplayString/ASN_OCTET_STR/char(char)//L/A/w/e/R/d/H
         */
   char   DHCPIpv6poolName[255];
size_t      DHCPIpv6poolName_len; /* # of char elements, not bytes */
    
        /*
         * DHCPIpv6PoolUseage(3)/INTEGER/ASN_INTEGER/long(long)//l/A/w/e/r/d/h
         */
   long   DHCPIpv6PoolUseage;
    
} dot11DHCPIpv6UseageTable_data;


/*
 * TODO:120:r: |-> Review dot11DHCPIpv6UseageTable mib index.
 * This structure is used to represent the index for dot11DHCPIpv6UseageTable.
 */
typedef struct dot11DHCPIpv6UseageTable_mib_index_s {

        /*
         * Ipv6DHCPUSEID(1)/INTEGER/ASN_INTEGER/long(long)//l/A/w/e/r/d/h
         */
   long   Ipv6DHCPUSEID;


} dot11DHCPIpv6UseageTable_mib_index;

    /*
     * TODO:121:r: |   |-> Review dot11DHCPIpv6UseageTable max index length.
     * If you KNOW that your indexes will never exceed a certain
     * length, update this macro to that length.
*/
#define MAX_dot11DHCPIpv6UseageTable_IDX_LEN     1


/* *********************************************************************
 * TODO:130:o: |-> Review dot11DHCPIpv6UseageTable Row request (rowreq) context.
 * When your functions are called, you will be passed a
 * dot11DHCPIpv6UseageTable_rowreq_ctx pointer.
 */
typedef struct dot11DHCPIpv6UseageTable_rowreq_ctx_s {

    /** this must be first for container compare to work */
    netsnmp_index        oid_idx;
    oid                  oid_tmp[MAX_dot11DHCPIpv6UseageTable_IDX_LEN];
    
    dot11DHCPIpv6UseageTable_mib_index        tbl_idx;
    
    dot11DHCPIpv6UseageTable_data              data;

    /*
     * flags per row. Currently, the first (lower) 8 bits are reserved
     * for the user. See mfd.h for other flags.
     */
    u_int                       rowreq_flags;

    /*
     * TODO:131:o: |   |-> Add useful data to dot11DHCPIpv6UseageTable rowreq context.
     */
    
    /*
     * storage for future expansion
     */
    netsnmp_data_list             *dot11DHCPIpv6UseageTable_data_list;

} dot11DHCPIpv6UseageTable_rowreq_ctx;

typedef struct dot11DHCPIpv6UseageTable_ref_rowreq_ctx_s {
    dot11DHCPIpv6UseageTable_rowreq_ctx *rowreq_ctx;
} dot11DHCPIpv6UseageTable_ref_rowreq_ctx;

/* *********************************************************************
 * function prototypes
 */
    int dot11DHCPIpv6UseageTable_pre_request(dot11DHCPIpv6UseageTable_registration * user_context);
    int dot11DHCPIpv6UseageTable_post_request(dot11DHCPIpv6UseageTable_registration * user_context,
        int rc);

    int dot11DHCPIpv6UseageTable_rowreq_ctx_init(dot11DHCPIpv6UseageTable_rowreq_ctx *rowreq_ctx,
                                   void *user_init_ctx);
    void dot11DHCPIpv6UseageTable_rowreq_ctx_cleanup(dot11DHCPIpv6UseageTable_rowreq_ctx *rowreq_ctx);


    dot11DHCPIpv6UseageTable_rowreq_ctx *
                  dot11DHCPIpv6UseageTable_row_find_by_mib_index(dot11DHCPIpv6UseageTable_mib_index *mib_idx);

extern oid dot11DHCPIpv6UseageTable_oid[];
extern int dot11DHCPIpv6UseageTable_oid_size;


#include "dot11DHCPIpv6UseageTable_interface.h"
#include "dot11DHCPIpv6UseageTable_data_access.h"
#include "dot11DHCPIpv6UseageTable_data_get.h"
#include "dot11DHCPIpv6UseageTable_data_set.h"

/*
 * DUMMY markers, ignore
 *
 * TODO:099:x: *************************************************************
 * TODO:199:x: *************************************************************
 * TODO:299:x: *************************************************************
 * TODO:399:x: *************************************************************
 * TODO:499:x: *************************************************************
 */

#ifdef __cplusplus
}
#endif

#endif /* DOT11DHCPIPV6USEAGETABLE_H */
/** @} */
