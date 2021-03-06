/*
 * Note: this file originally auto-generated by mib2c using
 *       version : 1.18.2.1 $ of : mfd-data-get.m2c,v $ 
 *
 * $Id:$
 */
/* standard Net-SNMP includes */
#include <net-snmp/net-snmp-config.h>
#include <net-snmp/net-snmp-includes.h>
#include <net-snmp/agent/net-snmp-agent-includes.h>

/* include our parent header */
#include "dot11WtpWirelessIfConfigTable.h"


/** @defgroup data_get data_get: Routines to get data
 *
 * TODO:230:M: Implement dot11WtpWirelessIfConfigTable get routines.
 * TODO:240:M: Implement dot11WtpWirelessIfConfigTable mapping routines (if any).
 *
 * These routine are used to get the value for individual objects. The
 * row context is passed, along with a pointer to the memory where the
 * value should be copied.
 *
 * @{
 */
/**********************************************************************
 **********************************************************************
 ***
 *** Table dot11WtpWirelessIfConfigTable
 ***
 **********************************************************************
 **********************************************************************/
/*
 * dot11WtpWirelessIfConfigTable is subid 4 of wtpInterface.
 * Its status is Current.
 * OID: .1.3.6.1.4.1.31656.6.1.1.3.4, length: 12
*/

/* ---------------------------------------------------------------------
 * TODO:200:r: Implement dot11WtpWirelessIfConfigTable data context functions.
 */

/*---------------------------------------------------------------------
 * DOT11-WTP-MIB::dot11WtpInfoEntry.wtpMacAddr
 * wtpMacAddr is subid 1 of dot11WtpInfoEntry.
 * Its status is Current, and its access level is ReadOnly.
 * OID: .1.3.6.1.4.1.31656.6.1.1.1.1.1.1
 * Description:
AP mac.
 *
 * Attributes:
 *   accessible 1     isscalar 0     enums  0      hasdefval 0
 *   readable   1     iscolumn 1     ranges 0      hashint   0
 *   settable   0
 *
 *
 * Its syntax is Dot11BaseWtpIdTC (based on perltype OCTETSTR)
 * The net-snmp type is ASN_OCTET_STR. The C type decl is char (char)
 * This data type requires a length.  (Max )
 */
/**
 * map a value from its original native format to the MIB format.
 *
 * @retval MFD_SUCCESS         : success
 * @retval MFD_ERROR           : Any other error
 *
 * @note parameters follow the memset convention (dest, src).
 *
 * @note generation and use of this function can be turned off by re-running
 * mib2c after adding the following line to the file
 * default-node-wtpMacAddr.m2d :
 *   @eval $m2c_node_skip_mapping = 1@
 *
 * @remark
 *  If the values for your data type don't exactly match the
 *  possible values defined by the mib, you should map them here.
 *  Otherwise, just do a direct copy.
 */
int
wtpMacAddr_dot11WtpWirelessIfConfigTable_map(char **mib_wtpMacAddr_val_ptr_ptr, size_t *mib_wtpMacAddr_val_ptr_len_ptr, char *raw_wtpMacAddr_val_ptr, size_t raw_wtpMacAddr_val_ptr_len, int allow_realloc)
{
    int converted_len;

    netsnmp_assert(NULL != raw_wtpMacAddr_val_ptr);
    netsnmp_assert((NULL != mib_wtpMacAddr_val_ptr_ptr) && (NULL != mib_wtpMacAddr_val_ptr_len_ptr));
    
    DEBUGMSGTL(("verbose:dot11WtpWirelessIfConfigTable:wtpMacAddr_map","called\n"));
    
    /*
     * TODO:241:r: |-> Implement wtpMacAddr non-integer mapping
     * it is hard to autogenerate code for mapping types that are not simple
     * integers, so here is an idea of what you might need to do. It will
     * probably need some tweaking to get right.
     */
    /*
     * if the length of the raw data doesn't directly correspond with
     * the length of the mib data, set converted_len to the
     * space required.
     */
    converted_len = raw_wtpMacAddr_val_ptr_len; /* assume equal */
    if((NULL == *mib_wtpMacAddr_val_ptr_ptr) || (*mib_wtpMacAddr_val_ptr_len_ptr < converted_len)) {
        if(! allow_realloc) {
            snmp_log(LOG_ERR,"not enough space for value mapping\n");
            return SNMP_ERR_GENERR;
        }
        *mib_wtpMacAddr_val_ptr_ptr = realloc( *mib_wtpMacAddr_val_ptr_ptr, converted_len * sizeof(**mib_wtpMacAddr_val_ptr_ptr));
        if(NULL == *mib_wtpMacAddr_val_ptr_ptr) {
            snmp_log(LOG_ERR,"could not allocate memory\n");
            return SNMP_ERR_GENERR;
        }
    }
    *mib_wtpMacAddr_val_ptr_len_ptr = converted_len;
    memcpy( *mib_wtpMacAddr_val_ptr_ptr, raw_wtpMacAddr_val_ptr, converted_len );

    return MFD_SUCCESS;
} /* wtpMacAddr_map */

/*---------------------------------------------------------------------
 * DOT11-WTP-MIB::dot11WtpWirelessIfEntry.wtpWirelessIfIndex
 * wtpWirelessIfIndex is subid 1 of dot11WtpWirelessIfEntry.
 * Its status is Current, and its access level is ReadOnly.
 * OID: .1.3.6.1.4.1.31656.6.1.1.3.3.1.1
 * Description:
Wireless interface index.
 *
 * Attributes:
 *   accessible 1     isscalar 0     enums  0      hasdefval 0
 *   readable   1     iscolumn 1     ranges 0      hashint   0
 *   settable   0
 *
 *
 * Its syntax is INTEGER (based on perltype INTEGER)
 * The net-snmp type is ASN_INTEGER. The C type decl is long (long)
 */
/**
 * map a value from its original native format to the MIB format.
 *
 * @retval MFD_SUCCESS         : success
 * @retval MFD_ERROR           : Any other error
 *
 * @note parameters follow the memset convention (dest, src).
 *
 * @note generation and use of this function can be turned off by re-running
 * mib2c after adding the following line to the file
 * default-node-wtpWirelessIfIndex.m2d :
 *   @eval $m2c_node_skip_mapping = 1@
 *
 * @remark
 *  If the values for your data type don't exactly match the
 *  possible values defined by the mib, you should map them here.
 *  Otherwise, just do a direct copy.
 */
int
wtpWirelessIfIndex_dot11WtpWirelessIfConfigTable_map(long *mib_wtpWirelessIfIndex_val_ptr, long raw_wtpWirelessIfIndex_val)
{
    netsnmp_assert(NULL != mib_wtpWirelessIfIndex_val_ptr);
    
    DEBUGMSGTL(("verbose:dot11WtpWirelessIfConfigTable:wtpWirelessIfIndex_map","called\n"));
    
    /*
     * TODO:241:o: |-> Implement wtpWirelessIfIndex mapping.
     * If the values for your data type don't exactly match the
     * possible values defined by the mib, you should map them here.
     */
    (*mib_wtpWirelessIfIndex_val_ptr) = raw_wtpWirelessIfIndex_val;

    return MFD_SUCCESS;
} /* wtpWirelessIfIndex_map */


/**
 * set mib index(es)
 *
 * @param tbl_idx mib index structure
 *
 * @retval MFD_SUCCESS     : success.
 * @retval MFD_ERROR       : other error.
 *
 * @remark
 *  This convenience function is useful for setting all the MIB index
 *  components with a single function call. It is assume that the C values
 *  have already been mapped from their native/rawformat to the MIB format.
 */
int
dot11WtpWirelessIfConfigTable_indexes_set_tbl_idx(dot11WtpWirelessIfConfigTable_mib_index *tbl_idx, char *wtpMacAddr_val_ptr,  size_t wtpMacAddr_val_ptr_len, long wtpWirelessIfIndex_val)
{
    DEBUGMSGTL(("verbose:dot11WtpWirelessIfConfigTable:dot11WtpWirelessIfConfigTable_indexes_set_tbl_idx","called\n"));

    /* wtpMacAddr(1)/Dot11BaseWtpIdTC/ASN_OCTET_STR/char(char)//L/A/w/e/r/d/h */
     tbl_idx->wtpMacAddr_len = sizeof(tbl_idx->wtpMacAddr);
    /*
     * make sure there is enough space for wtpMacAddr data
     */
    if ((NULL == tbl_idx->wtpMacAddr) ||
        (tbl_idx->wtpMacAddr_len < (wtpMacAddr_val_ptr_len * sizeof(tbl_idx->wtpMacAddr[0])))) {
        snmp_log(LOG_ERR,"not enough space for value\n");
        return MFD_ERROR;
    }
    tbl_idx->wtpMacAddr_len = wtpMacAddr_val_ptr_len * sizeof(tbl_idx->wtpMacAddr[0]);
    memcpy( tbl_idx->wtpMacAddr, wtpMacAddr_val_ptr, tbl_idx->wtpMacAddr_len );
    
    /* wtpWirelessIfIndex(1)/INTEGER/ASN_INTEGER/long(long)//l/A/w/e/r/d/h */
    tbl_idx->wtpWirelessIfIndex = wtpWirelessIfIndex_val;
    

    return MFD_SUCCESS;
} /* dot11WtpWirelessIfConfigTable_indexes_set_tbl_idx */

/**
 * @internal
 * set row context indexes
 *
 * @param reqreq_ctx the row context that needs updated indexes
 *
 * @retval MFD_SUCCESS     : success.
 * @retval MFD_ERROR       : other error.
 *
 * @remark
 *  This function sets the mib indexs, then updates the oid indexs
 *  from the mib index.
 */
int
dot11WtpWirelessIfConfigTable_indexes_set(dot11WtpWirelessIfConfigTable_rowreq_ctx *rowreq_ctx, char *wtpMacAddr_val_ptr,  size_t wtpMacAddr_val_ptr_len, long wtpWirelessIfIndex_val)
{
    DEBUGMSGTL(("verbose:dot11WtpWirelessIfConfigTable:dot11WtpWirelessIfConfigTable_indexes_set","called\n"));

    if(MFD_SUCCESS != dot11WtpWirelessIfConfigTable_indexes_set_tbl_idx(&rowreq_ctx->tbl_idx
                                   , wtpMacAddr_val_ptr, wtpMacAddr_val_ptr_len
                                   , wtpWirelessIfIndex_val
           ))
        return MFD_ERROR;

    /*
     * convert mib index to oid index
     */
    rowreq_ctx->oid_idx.len = sizeof(rowreq_ctx->oid_tmp) / sizeof(oid);
    if(0 != dot11WtpWirelessIfConfigTable_index_to_oid(&rowreq_ctx->oid_idx,
                                    &rowreq_ctx->tbl_idx)) {
        return MFD_ERROR;
    }

    return MFD_SUCCESS;
} /* dot11WtpWirelessIfConfigTable_indexes_set */


/*---------------------------------------------------------------------
 * DOT11-WTP-MIB::dot11WtpWirelessIfConfigEntry.wtpWirelessIfBeaconIntvl
 * wtpWirelessIfBeaconIntvl is subid 1 of dot11WtpWirelessIfConfigEntry.
 * Its status is Current, and its access level is ReadWrite.
 * OID: .1.3.6.1.4.1.31656.6.1.1.3.4.1.1
 * Description:
Beacon frame interval.(Range25 - 1000ms)
 *
 * Attributes:
 *   accessible 1     isscalar 0     enums  0      hasdefval 0
 *   readable   1     iscolumn 1     ranges 1      hashint   0
 *   settable   1
 *
 * Ranges:  25 - 1000;
 *
 * Its syntax is INTEGER (based on perltype INTEGER)
 * The net-snmp type is ASN_INTEGER. The C type decl is long (long)
 */
/**
 * map a value from its original native format to the MIB format.
 *
 * @retval MFD_SUCCESS         : success
 * @retval MFD_ERROR           : Any other error
 *
 * @note parameters follow the memset convention (dest, src).
 *
 * @note generation and use of this function can be turned off by re-running
 * mib2c after adding the following line to the file
 * default-node-wtpWirelessIfBeaconIntvl.m2d :
 *   @eval $m2c_node_skip_mapping = 1@
 *
 * @remark
 *  If the values for your data type don't exactly match the
 *  possible values defined by the mib, you should map them here.
 *  Otherwise, just do a direct copy.
 */
int
wtpWirelessIfBeaconIntvl_map(long *mib_wtpWirelessIfBeaconIntvl_val_ptr, long raw_wtpWirelessIfBeaconIntvl_val)
{
    netsnmp_assert(NULL != mib_wtpWirelessIfBeaconIntvl_val_ptr);
    
    DEBUGMSGTL(("verbose:dot11WtpWirelessIfConfigTable:wtpWirelessIfBeaconIntvl_map","called\n"));
    
    /*
     * TODO:241:o: |-> Implement wtpWirelessIfBeaconIntvl mapping.
     * If the values for your data type don't exactly match the
     * possible values defined by the mib, you should map them here.
     */
    (*mib_wtpWirelessIfBeaconIntvl_val_ptr) = raw_wtpWirelessIfBeaconIntvl_val;

    return MFD_SUCCESS;
} /* wtpWirelessIfBeaconIntvl_map */

/**
 * Extract the current value of the wtpWirelessIfBeaconIntvl data.
 *
 * Set a value using the data context for the row.
 *
 * @param rowreq_ctx
 *        Pointer to the row request context.
 * @param wtpWirelessIfBeaconIntvl_val_ptr
 *        Pointer to storage for a long variable
 *
 * @retval MFD_SUCCESS         : success
 * @retval MFD_SKIP            : skip this node (no value for now)
 * @retval MFD_ERROR           : Any other error
 */
int
wtpWirelessIfBeaconIntvl_get( dot11WtpWirelessIfConfigTable_rowreq_ctx *rowreq_ctx, long * wtpWirelessIfBeaconIntvl_val_ptr )
{
   /** we should have a non-NULL pointer */
   netsnmp_assert( NULL != wtpWirelessIfBeaconIntvl_val_ptr );


    DEBUGMSGTL(("verbose:dot11WtpWirelessIfConfigTable:wtpWirelessIfBeaconIntvl_get","called\n"));

    netsnmp_assert(NULL != rowreq_ctx);

/*
 * TODO:231:o: |-> Extract the current value of the wtpWirelessIfBeaconIntvl data.
 * set (* wtpWirelessIfBeaconIntvl_val_ptr ) from rowreq_ctx->data
 */
    (* wtpWirelessIfBeaconIntvl_val_ptr ) = rowreq_ctx->data.wtpWirelessIfBeaconIntvl;

    return MFD_SUCCESS;
} /* wtpWirelessIfBeaconIntvl_get */

/*---------------------------------------------------------------------
 * DOT11-WTP-MIB::dot11WtpWirelessIfConfigEntry.wtpWirelessIfDtimIntvl
 * wtpWirelessIfDtimIntvl is subid 2 of dot11WtpWirelessIfConfigEntry.
 * Its status is Current, and its access level is ReadWrite.
 * OID: .1.3.6.1.4.1.31656.6.1.1.3.4.1.2
 * Description:
DTIM interval.
 *
 * Attributes:
 *   accessible 1     isscalar 0     enums  0      hasdefval 0
 *   readable   1     iscolumn 1     ranges 1      hashint   0
 *   settable   1
 *
 * Ranges:  1 - 15;
 *
 * Its syntax is INTEGER (based on perltype INTEGER)
 * The net-snmp type is ASN_INTEGER. The C type decl is long (long)
 */
/**
 * map a value from its original native format to the MIB format.
 *
 * @retval MFD_SUCCESS         : success
 * @retval MFD_ERROR           : Any other error
 *
 * @note parameters follow the memset convention (dest, src).
 *
 * @note generation and use of this function can be turned off by re-running
 * mib2c after adding the following line to the file
 * default-node-wtpWirelessIfDtimIntvl.m2d :
 *   @eval $m2c_node_skip_mapping = 1@
 *
 * @remark
 *  If the values for your data type don't exactly match the
 *  possible values defined by the mib, you should map them here.
 *  Otherwise, just do a direct copy.
 */
int
wtpWirelessIfDtimIntvl_map(long *mib_wtpWirelessIfDtimIntvl_val_ptr, long raw_wtpWirelessIfDtimIntvl_val)
{
    netsnmp_assert(NULL != mib_wtpWirelessIfDtimIntvl_val_ptr);
    
    DEBUGMSGTL(("verbose:dot11WtpWirelessIfConfigTable:wtpWirelessIfDtimIntvl_map","called\n"));
    
    /*
     * TODO:241:o: |-> Implement wtpWirelessIfDtimIntvl mapping.
     * If the values for your data type don't exactly match the
     * possible values defined by the mib, you should map them here.
     */
    (*mib_wtpWirelessIfDtimIntvl_val_ptr) = raw_wtpWirelessIfDtimIntvl_val;

    return MFD_SUCCESS;
} /* wtpWirelessIfDtimIntvl_map */

/**
 * Extract the current value of the wtpWirelessIfDtimIntvl data.
 *
 * Set a value using the data context for the row.
 *
 * @param rowreq_ctx
 *        Pointer to the row request context.
 * @param wtpWirelessIfDtimIntvl_val_ptr
 *        Pointer to storage for a long variable
 *
 * @retval MFD_SUCCESS         : success
 * @retval MFD_SKIP            : skip this node (no value for now)
 * @retval MFD_ERROR           : Any other error
 */
int
wtpWirelessIfDtimIntvl_get( dot11WtpWirelessIfConfigTable_rowreq_ctx *rowreq_ctx, long * wtpWirelessIfDtimIntvl_val_ptr )
{
   /** we should have a non-NULL pointer */
   netsnmp_assert( NULL != wtpWirelessIfDtimIntvl_val_ptr );


    DEBUGMSGTL(("verbose:dot11WtpWirelessIfConfigTable:wtpWirelessIfDtimIntvl_get","called\n"));

    netsnmp_assert(NULL != rowreq_ctx);

/*
 * TODO:231:o: |-> Extract the current value of the wtpWirelessIfDtimIntvl data.
 * set (* wtpWirelessIfDtimIntvl_val_ptr ) from rowreq_ctx->data
 */
    (* wtpWirelessIfDtimIntvl_val_ptr ) = rowreq_ctx->data.wtpWirelessIfDtimIntvl;

    return MFD_SUCCESS;
} /* wtpWirelessIfDtimIntvl_get */

/*---------------------------------------------------------------------
 * DOT11-WTP-MIB::dot11WtpWirelessIfConfigEntry.wtpWirelessIfShtRetryThld
 * wtpWirelessIfShtRetryThld is subid 3 of dot11WtpWirelessIfConfigEntry.
 * Its status is Current, and its access level is ReadWrite.
 * OID: .1.3.6.1.4.1.31656.6.1.1.3.4.1.3
 * Description:
The total times that retry times when frame length less than RTS threshold  .
 *
 * Attributes:
 *   accessible 1     isscalar 0     enums  0      hasdefval 0
 *   readable   1     iscolumn 1     ranges 1      hashint   0
 *   settable   1
 *
 * Ranges:  1 - 15;
 *
 * Its syntax is INTEGER (based on perltype INTEGER)
 * The net-snmp type is ASN_INTEGER. The C type decl is long (long)
 */
/**
 * map a value from its original native format to the MIB format.
 *
 * @retval MFD_SUCCESS         : success
 * @retval MFD_ERROR           : Any other error
 *
 * @note parameters follow the memset convention (dest, src).
 *
 * @note generation and use of this function can be turned off by re-running
 * mib2c after adding the following line to the file
 * default-node-wtpWirelessIfShtRetryThld.m2d :
 *   @eval $m2c_node_skip_mapping = 1@
 *
 * @remark
 *  If the values for your data type don't exactly match the
 *  possible values defined by the mib, you should map them here.
 *  Otherwise, just do a direct copy.
 */
int
wtpWirelessIfShtRetryThld_map(long *mib_wtpWirelessIfShtRetryThld_val_ptr, long raw_wtpWirelessIfShtRetryThld_val)
{
    netsnmp_assert(NULL != mib_wtpWirelessIfShtRetryThld_val_ptr);
    
    DEBUGMSGTL(("verbose:dot11WtpWirelessIfConfigTable:wtpWirelessIfShtRetryThld_map","called\n"));
    
    /*
     * TODO:241:o: |-> Implement wtpWirelessIfShtRetryThld mapping.
     * If the values for your data type don't exactly match the
     * possible values defined by the mib, you should map them here.
     */
    (*mib_wtpWirelessIfShtRetryThld_val_ptr) = raw_wtpWirelessIfShtRetryThld_val;

    return MFD_SUCCESS;
} /* wtpWirelessIfShtRetryThld_map */

/**
 * Extract the current value of the wtpWirelessIfShtRetryThld data.
 *
 * Set a value using the data context for the row.
 *
 * @param rowreq_ctx
 *        Pointer to the row request context.
 * @param wtpWirelessIfShtRetryThld_val_ptr
 *        Pointer to storage for a long variable
 *
 * @retval MFD_SUCCESS         : success
 * @retval MFD_SKIP            : skip this node (no value for now)
 * @retval MFD_ERROR           : Any other error
 */
int
wtpWirelessIfShtRetryThld_get( dot11WtpWirelessIfConfigTable_rowreq_ctx *rowreq_ctx, long * wtpWirelessIfShtRetryThld_val_ptr )
{
   /** we should have a non-NULL pointer */
   netsnmp_assert( NULL != wtpWirelessIfShtRetryThld_val_ptr );


    DEBUGMSGTL(("verbose:dot11WtpWirelessIfConfigTable:wtpWirelessIfShtRetryThld_get","called\n"));

    netsnmp_assert(NULL != rowreq_ctx);

/*
 * TODO:231:o: |-> Extract the current value of the wtpWirelessIfShtRetryThld data.
 * set (* wtpWirelessIfShtRetryThld_val_ptr ) from rowreq_ctx->data
 */
    (* wtpWirelessIfShtRetryThld_val_ptr ) = rowreq_ctx->data.wtpWirelessIfShtRetryThld;

    return MFD_SUCCESS;
} /* wtpWirelessIfShtRetryThld_get */

/*---------------------------------------------------------------------
 * DOT11-WTP-MIB::dot11WtpWirelessIfConfigEntry.wtpWirelessIfLongRetryThld
 * wtpWirelessIfLongRetryThld is subid 4 of dot11WtpWirelessIfConfigEntry.
 * Its status is Current, and its access level is ReadWrite.
 * OID: .1.3.6.1.4.1.31656.6.1.1.3.4.1.4
 * Description:
The total times that retry times when frame length over RTS threshold  .
 *
 * Attributes:
 *   accessible 1     isscalar 0     enums  0      hasdefval 0
 *   readable   1     iscolumn 1     ranges 1      hashint   0
 *   settable   1
 *
 * Ranges:  1 - 15;
 *
 * Its syntax is INTEGER (based on perltype INTEGER)
 * The net-snmp type is ASN_INTEGER. The C type decl is long (long)
 */
/**
 * map a value from its original native format to the MIB format.
 *
 * @retval MFD_SUCCESS         : success
 * @retval MFD_ERROR           : Any other error
 *
 * @note parameters follow the memset convention (dest, src).
 *
 * @note generation and use of this function can be turned off by re-running
 * mib2c after adding the following line to the file
 * default-node-wtpWirelessIfLongRetryThld.m2d :
 *   @eval $m2c_node_skip_mapping = 1@
 *
 * @remark
 *  If the values for your data type don't exactly match the
 *  possible values defined by the mib, you should map them here.
 *  Otherwise, just do a direct copy.
 */
int
wtpWirelessIfLongRetryThld_map(long *mib_wtpWirelessIfLongRetryThld_val_ptr, long raw_wtpWirelessIfLongRetryThld_val)
{
    netsnmp_assert(NULL != mib_wtpWirelessIfLongRetryThld_val_ptr);
    
    DEBUGMSGTL(("verbose:dot11WtpWirelessIfConfigTable:wtpWirelessIfLongRetryThld_map","called\n"));
    
    /*
     * TODO:241:o: |-> Implement wtpWirelessIfLongRetryThld mapping.
     * If the values for your data type don't exactly match the
     * possible values defined by the mib, you should map them here.
     */
    (*mib_wtpWirelessIfLongRetryThld_val_ptr) = raw_wtpWirelessIfLongRetryThld_val;

    return MFD_SUCCESS;
} /* wtpWirelessIfLongRetryThld_map */

/**
 * Extract the current value of the wtpWirelessIfLongRetryThld data.
 *
 * Set a value using the data context for the row.
 *
 * @param rowreq_ctx
 *        Pointer to the row request context.
 * @param wtpWirelessIfLongRetryThld_val_ptr
 *        Pointer to storage for a long variable
 *
 * @retval MFD_SUCCESS         : success
 * @retval MFD_SKIP            : skip this node (no value for now)
 * @retval MFD_ERROR           : Any other error
 */
int
wtpWirelessIfLongRetryThld_get( dot11WtpWirelessIfConfigTable_rowreq_ctx *rowreq_ctx, long * wtpWirelessIfLongRetryThld_val_ptr )
{
   /** we should have a non-NULL pointer */
   netsnmp_assert( NULL != wtpWirelessIfLongRetryThld_val_ptr );


    DEBUGMSGTL(("verbose:dot11WtpWirelessIfConfigTable:wtpWirelessIfLongRetryThld_get","called\n"));

    netsnmp_assert(NULL != rowreq_ctx);

/*
 * TODO:231:o: |-> Extract the current value of the wtpWirelessIfLongRetryThld data.
 * set (* wtpWirelessIfLongRetryThld_val_ptr ) from rowreq_ctx->data
 */
    (* wtpWirelessIfLongRetryThld_val_ptr ) = rowreq_ctx->data.wtpWirelessIfLongRetryThld;

    return MFD_SUCCESS;
} /* wtpWirelessIfLongRetryThld_get */

/*---------------------------------------------------------------------
 * DOT11-WTP-MIB::dot11WtpWirelessIfConfigEntry.wtpWirelessIfMaxRxLifetime
 * wtpWirelessIfMaxRxLifetime is subid 5 of dot11WtpWirelessIfConfigEntry.
 * Its status is Current, and its access level is ReadWrite.
 * OID: .1.3.6.1.4.1.31656.6.1.1.3.4.1.5
 * Description:
Received data packets lifetime(units ms).
 *
 * Attributes:
 *   accessible 1     isscalar 0     enums  0      hasdefval 0
 *   readable   1     iscolumn 1     ranges 0      hashint   0
 *   settable   1
 *
 *
 * Its syntax is UNSIGNED32 (based on perltype UNSIGNED32)
 * The net-snmp type is ASN_UNSIGNED. The C type decl is u_long (u_long)
 */
/**
 * map a value from its original native format to the MIB format.
 *
 * @retval MFD_SUCCESS         : success
 * @retval MFD_ERROR           : Any other error
 *
 * @note parameters follow the memset convention (dest, src).
 *
 * @note generation and use of this function can be turned off by re-running
 * mib2c after adding the following line to the file
 * default-node-wtpWirelessIfMaxRxLifetime.m2d :
 *   @eval $m2c_node_skip_mapping = 1@
 *
 * @remark
 *  If the values for your data type don't exactly match the
 *  possible values defined by the mib, you should map them here.
 *  Otherwise, just do a direct copy.
 */
int
wtpWirelessIfMaxRxLifetime_map(u_long *mib_wtpWirelessIfMaxRxLifetime_val_ptr, u_long raw_wtpWirelessIfMaxRxLifetime_val)
{
    netsnmp_assert(NULL != mib_wtpWirelessIfMaxRxLifetime_val_ptr);
    
    DEBUGMSGTL(("verbose:dot11WtpWirelessIfConfigTable:wtpWirelessIfMaxRxLifetime_map","called\n"));
    
    /*
     * TODO:241:o: |-> Implement wtpWirelessIfMaxRxLifetime mapping.
     * If the values for your data type don't exactly match the
     * possible values defined by the mib, you should map them here.
     */
    (*mib_wtpWirelessIfMaxRxLifetime_val_ptr) = raw_wtpWirelessIfMaxRxLifetime_val;

    return MFD_SUCCESS;
} /* wtpWirelessIfMaxRxLifetime_map */

/**
 * Extract the current value of the wtpWirelessIfMaxRxLifetime data.
 *
 * Set a value using the data context for the row.
 *
 * @param rowreq_ctx
 *        Pointer to the row request context.
 * @param wtpWirelessIfMaxRxLifetime_val_ptr
 *        Pointer to storage for a u_long variable
 *
 * @retval MFD_SUCCESS         : success
 * @retval MFD_SKIP            : skip this node (no value for now)
 * @retval MFD_ERROR           : Any other error
 */
int
wtpWirelessIfMaxRxLifetime_get( dot11WtpWirelessIfConfigTable_rowreq_ctx *rowreq_ctx, u_long * wtpWirelessIfMaxRxLifetime_val_ptr )
{
   /** we should have a non-NULL pointer */
   netsnmp_assert( NULL != wtpWirelessIfMaxRxLifetime_val_ptr );


    DEBUGMSGTL(("verbose:dot11WtpWirelessIfConfigTable:wtpWirelessIfMaxRxLifetime_get","called\n"));

    netsnmp_assert(NULL != rowreq_ctx);

/*
 * TODO:231:o: |-> Extract the current value of the wtpWirelessIfMaxRxLifetime data.
 * set (* wtpWirelessIfMaxRxLifetime_val_ptr ) from rowreq_ctx->data
 */
    (* wtpWirelessIfMaxRxLifetime_val_ptr ) = rowreq_ctx->data.wtpWirelessIfMaxRxLifetime;

    return MFD_SUCCESS;
} /* wtpWirelessIfMaxRxLifetime_get */



/** @} */
