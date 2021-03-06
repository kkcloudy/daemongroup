/*
 * Note: this file originally auto-generated by mib2c using
 *       version : 15899 $ of $
 *
 * $Id:$
 */
/** @ingroup interface: Routines to interface to Net-SNMP
 *
 * \warning This code should not be modified, called directly,
 *          or used to interpret functionality. It is subject to
 *          change at any time.
 * 
 * @{
 */
/*
 * *********************************************************************
 * *********************************************************************
 * *********************************************************************
 * ***                                                               ***
 * ***  NOTE NOTE NOTE NOTE NOTE NOTE NOTE NOTE NOTE NOTE NOTE NOTE  ***
 * ***                                                               ***
 * ***                                                               ***
 * ***       THIS FILE DOES NOT CONTAIN ANY USER EDITABLE CODE.      ***
 * ***                                                               ***
 * ***                                                               ***
 * ***       THE GENERATED CODE IS INTERNAL IMPLEMENTATION, AND      ***
 * ***                                                               ***
 * ***                                                               ***
 * ***    IS SUBJECT TO CHANGE WITHOUT WARNING IN FUTURE RELEASES.   ***
 * ***                                                               ***
 * ***                                                               ***
 * *********************************************************************
 * *********************************************************************
 * *********************************************************************
 */
#ifndef DOT11WTPWIRELESSAUTHTABLE_INTERFACE_H
#define DOT11WTPWIRELESSAUTHTABLE_INTERFACE_H

#ifdef __cplusplus
extern "C" {
#endif


#include "dot11WtpWirelessAuthTable.h"


/* ********************************************************************
 * Table declarations
 */

/* PUBLIC interface initialization routine */
void _dot11WtpWirelessAuthTable_initialize_interface(dot11WtpWirelessAuthTable_registration * user_ctx,
                                    u_long flags);
void _dot11WtpWirelessAuthTable_shutdown_interface(dot11WtpWirelessAuthTable_registration * user_ctx);

dot11WtpWirelessAuthTable_registration *
dot11WtpWirelessAuthTable_registration_get( void );

dot11WtpWirelessAuthTable_registration *
dot11WtpWirelessAuthTable_registration_set( dot11WtpWirelessAuthTable_registration * newreg );

netsnmp_container *dot11WtpWirelessAuthTable_container_get( void );
int dot11WtpWirelessAuthTable_container_size( void );

    dot11WtpWirelessAuthTable_rowreq_ctx * dot11WtpWirelessAuthTable_allocate_rowreq_ctx(void *);
void dot11WtpWirelessAuthTable_release_rowreq_ctx(dot11WtpWirelessAuthTable_rowreq_ctx *rowreq_ctx);

int dot11WtpWirelessAuthTable_index_to_oid(netsnmp_index *oid_idx,
                            dot11WtpWirelessAuthTable_mib_index *mib_idx);
int dot11WtpWirelessAuthTable_index_from_oid(netsnmp_index *oid_idx,
                              dot11WtpWirelessAuthTable_mib_index *mib_idx);

/*
 * access to certain internals. use with caution!
 */
void dot11WtpWirelessAuthTable_valid_columns_set(netsnmp_column_info *vc);


#ifdef __cplusplus
}
#endif

#endif /* DOT11WTPWIRELESSAUTHTABLE_INTERFACE_H */
/** @} */
