/*
 * Note: this file originally auto-generated by mib2c using
 *       version : 1.32.2.3 $ of : mfd-top.m2c,v $
 *
 * $Id:$
 */
#ifndef DOT11BSSIDWAPIPROTOCOLCONFIGTABLE_H
#define DOT11BSSIDWAPIPROTOCOLCONFIGTABLE_H

#ifdef __cplusplus
extern "C" {
#endif


/** @defgroup misc misc: Miscelaneous routines
 *
 * @{
 */
#include <net-snmp/library/asn1.h>

/* other required module components */
    /* *INDENT-OFF*  */
config_require(DOT11-WTP-MIB/dot11BSSIDWAPIProtocolConfigTable/dot11BSSIDWAPIProtocolConfigTable_interface);
config_require(DOT11-WTP-MIB/dot11BSSIDWAPIProtocolConfigTable/dot11BSSIDWAPIProtocolConfigTable_data_access);
config_require(DOT11-WTP-MIB/dot11BSSIDWAPIProtocolConfigTable/dot11BSSIDWAPIProtocolConfigTable_data_get);
config_require(DOT11-WTP-MIB/dot11BSSIDWAPIProtocolConfigTable/dot11BSSIDWAPIProtocolConfigTable_data_set);
    /* *INDENT-ON*  */

/* OID and column number definitions for  */
#include "dot11BSSIDWAPIProtocolConfigTable_oids.h"

/* enum definions */
#include "dot11BSSIDWAPIProtocolConfigTable_enums.h"
#include "ws_dbus_list_interface.h"

/* *********************************************************************
 * function declarations
 */
void init_dot11BSSIDWAPIProtocolConfigTable(void);

/* *********************************************************************
 * Table declarations
 */
/**********************************************************************
 **********************************************************************
 ***
 *** Table dot11BSSIDWAPIProtocolConfigTable
 ***
 **********************************************************************
 **********************************************************************/
/*
 * dot11BSSIDWAPIProtocolConfigTable is subid 2 of wtpWAPI.
 * Its status is Current.
 * OID: .1.3.6.1.4.1.31656.6.1.1.10.2, length: 12
*/
/* *********************************************************************
 * When you register your mib, you get to provide a generic
 * pointer that will be passed back to you for most of the
 * functions calls.
 *
 * TODO:100:r: Review all context structures
 */
    /*
     * TODO:101:o: |-> Review dot11BSSIDWAPIProtocolConfigTable registration context.
     */
typedef netsnmp_data_list * dot11BSSIDWAPIProtocolConfigTable_registration_ptr;

/**********************************************************************/
/*
 * TODO:110:r: |-> Review dot11BSSIDWAPIProtocolConfigTable data context structure.
 * This structure is used to represent the data for dot11BSSIDWAPIProtocolConfigTable.
 */
/*
 * This structure contains storage for all the columns defined in the
 * dot11BSSIDWAPIProtocolConfigTable.
 */
typedef struct dot11BSSIDWAPIProtocolConfigTable_data_s {
   
   dbus_parameter parameter;
    
        /*
         * wapiControlledAuthControl(1)/TruthValue/ASN_INTEGER/long(u_long)//l/A/w/E/r/d/h
         */
   u_long   wapiControlledAuthControl;
    
        /*
         * wapiControlledPortControl(2)/INTEGER/ASN_INTEGER/long(u_long)//l/A/w/E/r/d/h
         */
   u_long   wapiControlledPortControl;
    
        /*
         * wapiEnabled(3)/TruthValue/ASN_INTEGER/long(u_long)//l/A/W/E/r/d/h
         */
   u_long   wapiEnabled;
    
        /*
         * wapiCertificateUpdateCount(4)/UNSIGNED32/ASN_UNSIGNED/u_long(u_long)//l/A/W/e/r/d/h
         */
   u_long   wapiCertificateUpdateCount;
    
        /*
         * wapiMulticastUpdateCount(5)/UNSIGNED32/ASN_UNSIGNED/u_long(u_long)//l/A/W/e/r/d/h
         */
   u_long   wapiMulticastUpdateCount;
    
        /*
         * wapiUnicastUpdateCount(6)/UNSIGNED32/ASN_UNSIGNED/u_long(u_long)//l/A/W/e/r/d/h
         */
   u_long   wapiUnicastUpdateCount;
    
        /*
         * wapiAuthSuiteSelected(7)/INTEGER/ASN_INTEGER/long(long)//l/A/w/e/r/d/h
         */
   long   wapiAuthSuiteSelected;
    
        /*
         * wapiAuthSuiteRequested(8)/INTEGER/ASN_INTEGER/long(long)//l/A/w/e/r/d/h
         */
   long   wapiAuthSuiteRequested;
    
} dot11BSSIDWAPIProtocolConfigTable_data;


/* *********************************************************************
 * TODO:115:o: |-> Review dot11BSSIDWAPIProtocolConfigTable undo context.
 * We're just going to use the same data structure for our
 * undo_context. If you want to do something more efficent,
 * define your typedef here.
 */
typedef dot11BSSIDWAPIProtocolConfigTable_data dot11BSSIDWAPIProtocolConfigTable_undo_data;

/*
 * TODO:120:r: |-> Review dot11BSSIDWAPIProtocolConfigTable mib index.
 * This structure is used to represent the index for dot11BSSIDWAPIProtocolConfigTable.
 */
typedef struct dot11BSSIDWAPIProtocolConfigTable_mib_index_s {

        /*
         * wtpMacAddr(1)/DisplayString/ASN_OCTET_STR/char(char)//L/A/w/e/R/d/H
         */
        /** 128 - 1(other indexes) - oid length(14) = 112 */
   char   wtpMacAddr[112];
   size_t      wtpMacAddr_len;

        /*
         * wtpBssCurrID(1)/DisplayString/ASN_OCTET_STR/char(char)//L/A/w/e/R/d/H
         */
        /** 128 - 1(other indexes) - oid length(14) = 112 */
   char   wtpBssCurrID[112];
   size_t      wtpBssCurrID_len;


} dot11BSSIDWAPIProtocolConfigTable_mib_index;

    /*
     * TODO:121:r: |   |-> Review dot11BSSIDWAPIProtocolConfigTable max index length.
     * If you KNOW that your indexes will never exceed a certain
     * length, update this macro to that length.
     *
     * BE VERY CAREFUL TO TAKE INTO ACCOUNT THE MAXIMUM
     * POSSIBLE LENGHT FOR EVERY VARIABLE LENGTH INDEX!
     * Guessing 128 - col/entry(2)  - oid len(12)
*/
#define MAX_dot11BSSIDWAPIProtocolConfigTable_IDX_LEN     114


/* *********************************************************************
 * TODO:130:o: |-> Review dot11BSSIDWAPIProtocolConfigTable Row request (rowreq) context.
 * When your functions are called, you will be passed a
 * dot11BSSIDWAPIProtocolConfigTable_rowreq_ctx pointer.
 */
typedef struct dot11BSSIDWAPIProtocolConfigTable_rowreq_ctx_s {

    /** this must be first for container compare to work */
    netsnmp_index        oid_idx;
    oid                  oid_tmp[MAX_dot11BSSIDWAPIProtocolConfigTable_IDX_LEN];
    
    dot11BSSIDWAPIProtocolConfigTable_mib_index        tbl_idx;
    
    dot11BSSIDWAPIProtocolConfigTable_data              data;
    dot11BSSIDWAPIProtocolConfigTable_undo_data       * undo;
    unsigned int                column_set_flags; /* flags for set columns */


    /*
     * flags per row. Currently, the first (lower) 8 bits are reserved
     * for the user. See mfd.h for other flags.
     */
    u_int                       rowreq_flags;

    /*
     * implementor's context pointer (provided during registration)
     */
    dot11BSSIDWAPIProtocolConfigTable_registration_ptr dot11BSSIDWAPIProtocolConfigTable_reg;

    /*
     * TODO:131:o: |   |-> Add useful data to dot11BSSIDWAPIProtocolConfigTable rowreq context.
     */
    
    /*
     * storage for future expansion
     */
    netsnmp_data_list             *dot11BSSIDWAPIProtocolConfigTable_data_list;

} dot11BSSIDWAPIProtocolConfigTable_rowreq_ctx;

typedef struct dot11BSSIDWAPIProtocolConfigTable_ref_rowreq_ctx_s {
    dot11BSSIDWAPIProtocolConfigTable_rowreq_ctx *rowreq_ctx;
} dot11BSSIDWAPIProtocolConfigTable_ref_rowreq_ctx;

/* *********************************************************************
 * function prototypes
 */
    int dot11BSSIDWAPIProtocolConfigTable_pre_request(dot11BSSIDWAPIProtocolConfigTable_registration_ptr user_context);
    int dot11BSSIDWAPIProtocolConfigTable_post_request(dot11BSSIDWAPIProtocolConfigTable_registration_ptr user_context);

    int dot11BSSIDWAPIProtocolConfigTable_check_dependencies(dot11BSSIDWAPIProtocolConfigTable_rowreq_ctx * rowreq_ctx); 
    int dot11BSSIDWAPIProtocolConfigTable_commit(dot11BSSIDWAPIProtocolConfigTable_rowreq_ctx * rowreq_ctx);

extern oid dot11BSSIDWAPIProtocolConfigTable_oid[];
extern int dot11BSSIDWAPIProtocolConfigTable_oid_size;


#include "dot11BSSIDWAPIProtocolConfigTable_interface.h"
#include "dot11BSSIDWAPIProtocolConfigTable_data_access.h"
#include "dot11BSSIDWAPIProtocolConfigTable_data_get.h"
#include "dot11BSSIDWAPIProtocolConfigTable_data_set.h"

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

#endif /* DOT11BSSIDWAPIPROTOCOLCONFIGTABLE_H */
