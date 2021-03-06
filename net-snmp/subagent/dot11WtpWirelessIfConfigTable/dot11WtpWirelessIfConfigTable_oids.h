/*
 * Note: this file originally auto-generated by mib2c using
 *  : generic-table-oids.m2c,v 1.10 2004/10/08 23:39:17 rstory Exp $
 *
 * $Id:$
 */
#ifndef DOT11WTPWIRELESSIFCONFIGTABLE_OIDS_H
#define DOT11WTPWIRELESSIFCONFIGTABLE_OIDS_H

#ifdef __cplusplus
extern "C" {
#endif


/* column number definitions for table dot11WtpWirelessIfConfigTable */
//#define DOT11WTPWIRELESSIFCONFIGTABLE_OID              1,3,6,1,4,1,31656,6,1,1,3,4
#define COLUMN_WTPWIRELESSIFBEACONINTVL		1
#define COLUMN_WTPWIRELESSIFDTIMINTVL		2
#define COLUMN_WTPWIRELESSIFSHTRETRYTHLD		3
#define COLUMN_WTPWIRELESSIFLONGRETRYTHLD		4
#define COLUMN_WTPWIRELESSIFMAXRXLIFETIME		5

#define DOT11WTPWIRELESSIFCONFIGTABLE_MIN_COL		COLUMN_WTPWIRELESSIFBEACONINTVL
#define DOT11WTPWIRELESSIFCONFIGTABLE_MAX_COL		COLUMN_WTPWIRELESSIFMAXRXLIFETIME

    /*
     * change flags for writable columns
     */
#define FLAG_WTPWIRELESSIFBEACONINTVL       (0x1 << 0)
#define FLAG_WTPWIRELESSIFDTIMINTVL       (0x1 << 1)
#define FLAG_WTPWIRELESSIFSHTRETRYTHLD       (0x1 << 2)
#define FLAG_WTPWIRELESSIFLONGRETRYTHLD       (0x1 << 3)
#define FLAG_WTPWIRELESSIFMAXRXLIFETIME       (0x1 << 4)

#define FLAG_MAX_DOT11WTPWIRELESSIFCONFIGTABLE 5

    

#ifdef __cplusplus
}
#endif

#endif /* DOT11WTPWIRELESSIFCONFIGTABLE_OIDS_H */
