/* db.c

   Persistent database management routines for DHCPD... */

/*
 * Copyright (c) 2004-2007,2009 by Internet Systems Consortium, Inc. ("ISC")
 * Copyright (c) 1995-2003 by Internet Software Consortium
 *
 * Permission to use, copy, modify, and distribute this software for any
 * purpose with or without fee is hereby granted, provided that the above
 * copyright notice and this permission notice appear in all copies.
 *
 * THE SOFTWARE IS PROVIDED "AS IS" AND ISC DISCLAIMS ALL WARRANTIES
 * WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF
 * MERCHANTABILITY AND FITNESS.  IN NO EVENT SHALL ISC BE LIABLE FOR
 * ANY SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES
 * WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER IN AN
 * ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT
 * OF OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.
 *
 *   Internet Systems Consortium, Inc.
 *   950 Charter Street
 *   Redwood City, CA 94063
 *   <info@isc.org>
 *   https://www.isc.org/
 *
 * This software has been written for Internet Systems Consortium
 * by Ted Lemon in cooperation with Vixie Enterprises and Nominum, Inc.
 * To learn more about Internet Systems Consortium, see
 * ``https://www.isc.org/''.  To learn more about Vixie Enterprises,
 * see ``http://www.vix.com''.   To learn more about Nominum, Inc., see
 * ``http://www.nominum.com''.
 */

#include "dhcpd.h"
#include <ctype.h>
#include <errno.h>

#ifndef __AX_PLATFORM__
extern unsigned int dhcp_optimize_enable;
#endif

#ifdef __AX_PLATFORM__
#include <sqlite3.h>


static sqlite3 *dhcp_lease_db = NULL;
#define DHCP_LEASE_SQLITE3_DBNAME					"/mnt/dhcplease.db"
#define DHCP_LEASE_HASH_TABLE_SIZE    256
#define DHCP_LEASE_MAC_ADD_LEN        6                        /* length of mac address			*/

static char *zLeaseTbl = "\
CREATE TABLE dhcpLeaseDb(\
IP INTEGER NOT NULL UNIQUE,\
MAC VARCHAR(12) NOT NULL UNIQUE,\
STARTS INTEGER,\
ENDS INTEGER,\	CLTT INTEGER,\
BINDSTATE INTEGER,\	NEXTBINDSTATE INTEGER,\
PRIMARY KEY(IP)\
);";

static char *zDropTbl = "drop TABLE dhcpSnpDb;";


unsigned int dhcp_lease_db_create
(
	sqlite3 *db,
	char *sql
)
{
	int rc = 0;
    char * zErrMsg = 0;

    if (!sql || !db) {
		return 1;
    }

    rc = sqlite3_exec(db, sql, 0, 0, &zErrMsg);
	if (rc == SQLITE_OK) {
		log_debug("create dhcpSnpDb success.\n");
	}else if (rc == SQLITE_ERROR) {
		log_error("%s, rc %x.\n", zErrMsg ? zErrMsg : "", rc);
	}else {
		log_error("%s, rc %x!\n", zErrMsg ? zErrMsg : "", rc);
		return 1;
	}

    return 0;
}

void dhcp_lease_db_close
(
	sqlite3 *db
)
{
	int rc = 0;

    if (!db) {
		log_error("close db with null message!\n");
		return ;
    }

    rc = sqlite3_close(db);
    if (SQLITE_OK != rc) {
		log_error("close %s fail, rc %x!\n", DHCP_LEASE_SQLITE3_DBNAME, rc);
		return ;
	}

	log_debug("close %s success.\n", DHCP_LEASE_SQLITE3_DBNAME);

    return ;
}

unsigned int dhcp_lease_db_open
(
	sqlite3 **db
)
{
	int rc = 0;
	sqlite3 *tmp_db = NULL;

	rc = sqlite3_open(DHCP_LEASE_SQLITE3_DBNAME, &tmp_db);
	if (rc != SQLITE_OK || tmp_db == NULL) {
		dhcp_lease_db_close(tmp_db);
		log_error("open %s error, rc %x\n", DHCP_LEASE_SQLITE3_DBNAME, rc);
		return 1;
	}

	*db = tmp_db;
	log_debug("open %s %p success.\n", DHCP_LEASE_SQLITE3_DBNAME, *db);

	return 0;
}

unsigned int dhcp_lease_tbl_initialize
(
	void
)
{
	unsigned int ret = 0;

	/*memset(g_DHCP_Snp_Hash_Table, 0, NPD_DHCP_SNP_HASH_TABLE_SIZE * 4);*/

	/* open dhcp snooping db */
	ret = dhcp_lease_db_open(&dhcp_lease_db);
	if (0 != ret) {
		log_error("open DHCP-Snooping db error, ret %x", ret);
		return 1;
	}

	log_debug("open DHCP-Snooping db %s %p success.\n",
							DHCP_LEASE_SQLITE3_DBNAME, dhcp_lease_db);

	/* create dhcp snooping table */
	ret = dhcp_lease_db_create(dhcp_lease_db, zLeaseTbl);
	if (0 != ret) {
		log_error("create DHCP-Snooping db error, ret %x", ret);
		return 1;
	}

	return 0;
}

unsigned int dhcp_lease_db_insert
(
	sqlite3 *db,
	struct lease *item 
)
{
	int rc = 0;
	char * zErrMsg = NULL;
	char zInsertItem[1024] = {0};
	char *bufPtr = NULL;
	int loglength = 0;
	unsigned int ipaddr = 0;
	char chaddr[6];
	
    if (!item || !db) {
		return 1;
	}
	memcpy(&ipaddr, item->ip_addr.iabuf, 4);
	memcpy(chaddr, item->hardware_addr.hbuf, 6);

	bufPtr = zInsertItem;

	loglength += sprintf(bufPtr, "INSERT INTO dhcpLeaseDb VALUES('");
	bufPtr = zInsertItem + loglength;
	
	loglength += sprintf(bufPtr, "%d", ipaddr);
	bufPtr = zInsertItem + loglength;

	loglength += sprintf(bufPtr, ",");
	bufPtr = zInsertItem + loglength;

	loglength += sprintf(bufPtr, "%02x", chaddr[0]);
	bufPtr = zInsertItem + loglength;
	loglength += sprintf(bufPtr, "%02x", chaddr[1]);
	bufPtr = zInsertItem + loglength;
	loglength += sprintf(bufPtr, "%02x", chaddr[2]);
	bufPtr = zInsertItem + loglength;
	loglength += sprintf(bufPtr, "%02x", chaddr[3]);
	bufPtr = zInsertItem + loglength;
	loglength += sprintf(bufPtr, "%02x", chaddr[4]);
	bufPtr = zInsertItem + loglength;
	loglength += sprintf(bufPtr, "%02x", chaddr[5]);
	bufPtr = zInsertItem + loglength;

	loglength += sprintf(bufPtr, "',");
	bufPtr = zInsertItem + loglength;


	loglength += sprintf(bufPtr, "%d", item->starts);
	bufPtr = zInsertItem + loglength;

	loglength += sprintf(bufPtr, ",");
	bufPtr = zInsertItem + loglength;

	loglength += sprintf(bufPtr, "%d", item->ends);
	bufPtr = zInsertItem + loglength;

	loglength += sprintf(bufPtr, ",");
	bufPtr = zInsertItem + loglength;

	loglength += sprintf(bufPtr, "%d", item->cltt);
	bufPtr = zInsertItem + loglength;

	loglength += sprintf(bufPtr, ",");
	bufPtr = zInsertItem + loglength;

	loglength += sprintf(bufPtr, "%d", item->binding_state);
	bufPtr = zInsertItem + loglength;

	loglength += sprintf(bufPtr, ",");
	bufPtr = zInsertItem + loglength;

	loglength += sprintf(bufPtr, "%d", item->next_binding_state);
	bufPtr = zInsertItem + loglength;

	loglength += sprintf(bufPtr, ");");
	bufPtr = zInsertItem + loglength;

	log_debug("%s\n", zInsertItem);

	rc = sqlite3_exec(db, zInsertItem, 0, 0, &zErrMsg);
	if (rc != SQLITE_OK) {
		return rc;
	}
	
	return ISC_R_SUCCESS;
}

unsigned int dhcp_lease_db_delete
(
	sqlite3 *db,
	struct lease *item 
)
{
	int rc = 0;
    char * zErrMsg = NULL;

	char zDeleteItem[1024] = {0};
	char *bufPtr = NULL;
	int loglength = 0;
	unsigned int ipaddr = 0;

    if (!item || !db) {
		log_error("delete db with null message\n");
		return 1;
    }
	
	memcpy(&ipaddr, item->ip_addr.iabuf, 4);

	/* delete the item from dhcp snp db*/
	bufPtr = zDeleteItem;

	loglength += sprintf(bufPtr, "DELETE FROM dhcpLeaseDb WHERE IP='");
	bufPtr = zDeleteItem + loglength;

	loglength += sprintf(bufPtr, "%d", ipaddr);
	bufPtr = zDeleteItem + loglength;

	loglength += sprintf(bufPtr, "';");
	bufPtr = zDeleteItem + loglength;

	log_debug("%s\n", zDeleteItem);

    rc = sqlite3_exec(db, zDeleteItem, 0, 0, &zErrMsg);
	if (rc != SQLITE_OK) {
		log_error("%s%s, rc %x!\n",
								zErrMsg ? "error:" : "",
								zErrMsg ? zErrMsg : "",
								rc);
		return 1;
	}

	log_debug("delete from dhcpSnpDb success.\n");

	return 0;
}

unsigned int dhcp_lease_db_drop
(
	sqlite3 *db,
	char *sql
)
{
	int rc = 0;
	char * zErrMsg = 0;

	if (!sql || !db) {
		log_error("drop db with null message\n");
		return 1;
	}

	rc = sqlite3_exec(db, sql, 0, 0, &zErrMsg);
	if (rc != SQLITE_OK) {
		log_error("%s, rc %x!\n", zErrMsg ? zErrMsg : "", rc);
		return 1;
	}

	log_debug("drop table dhcpSnpDb success.\n");

	return 0;
}

unsigned int dhcp_lease_db_update
(
	sqlite3 *db,
	struct lease *item 
)
{
	int rc = 0;
    char * zErrMsg = NULL;

	char zUpdateItem[1024] = {0};
	char *bufPtr = NULL;
	int loglength = 0;
	unsigned int ipaddr = 0;
	char chaddr[6];

    if (!item || !db) {
		log_error("sql update with null message\n");
		return 1;
    }
	memcpy(&ipaddr, item->ip_addr.iabuf, 4);
	memcpy(chaddr, item->hardware_addr.hbuf, 6);

	/* update the item to dhcp snp db
	bufPtr = zUpdateItem;

	loglength += sprintf(bufPtr, "UPDATE dhcpLeaseDb SET ");
	bufPtr = zUpdateItem + loglength;
	
	loglength += sprintf(bufPtr, "MAC=");
	bufPtr = zUpdateItem + loglength;

	loglength += sprintf(bufPtr, "%02x", chaddr[0]);
	bufPtr = zUpdateItem + loglength;
	loglength += sprintf(bufPtr, "%02x", chaddr[1]);
	bufPtr = zUpdateItem + loglength;
	loglength += sprintf(bufPtr, "%02x", chaddr[2]);
	bufPtr = zUpdateItem + loglength;
	loglength += sprintf(bufPtr, "%02x", chaddr[3]);
	bufPtr = zUpdateItem + loglength;
	loglength += sprintf(bufPtr, "%02x", chaddr[4]);
	bufPtr = zUpdateItem + loglength;
	loglength += sprintf(bufPtr, "%02x", chaddr[5]);
	bufPtr = zUpdateItem + loglength;

	loglength += sprintf(bufPtr, ",STARTS=");
	bufPtr = zUpdateItem + loglength;

	loglength += sprintf(bufPtr, "%d", item->sys_escape);
	bufPtr = zUpdateItem + loglength;

	loglength += sprintf(bufPtr, ",ENDS=");
	bufPtr = zUpdateItem + loglength;

	loglength += sprintf(bufPtr, "%d", item->lease_time);
	bufPtr = zUpdateItem + loglength;
	
	loglength += sprintf(bufPtr, ",ENDS=");
	bufPtr = zUpdateItem + loglength;

	loglength += sprintf(bufPtr, "%d", item->lease_time);
	bufPtr = zUpdateItem + loglength;

	loglength += sprintf(bufPtr, ",CLTT=");
	bufPtr = zUpdateItem + loglength;

	loglength += sprintf(bufPtr, "%d", item->lease_time);
	bufPtr = zUpdateItem + loglength;

	loglength += sprintf(bufPtr, ",BINDSTATE=");
	bufPtr = zUpdateItem + loglength;

	loglength += sprintf(bufPtr, "%d", item->lease_time);
	bufPtr = zUpdateItem + loglength;
	
	loglength += sprintf(bufPtr, ",NEXTBINDSTATE=");
	bufPtr = zUpdateItem + loglength;

	loglength += sprintf(bufPtr, "%d", item->lease_time);
	bufPtr = zUpdateItem + loglength;

	loglength += sprintf(bufPtr, " WHERE IP='");
	bufPtr = zUpdateItem + loglength;
	
	loglength += sprintf(bufPtr, "%d", item->ip_addr);
	bufPtr = zUpdateItem + loglength;

	loglength += sprintf(bufPtr, "';");
	bufPtr = zUpdateItem + loglength;
*/	
	log_debug("%s\n", zUpdateItem);

    rc = sqlite3_exec(db, zUpdateItem, 0, 0, &zErrMsg);
	if (rc != SQLITE_OK) {
		log_error("%s%s, rc %x!\n",
								zErrMsg ? "error:" : "",
								zErrMsg ? zErrMsg : "",
								rc);
		return 1;
	}

	log_debug("update to dhcpSnpDb success.\n");

	return 0;
}

unsigned int dhcp_lease_db_query
(
	sqlite3 *db,
	char *sql,
	unsigned int *count,
	struct lease array[]
)
{
	int rc = 0;
	int i = 0;
	int j = 0;
	int nRow = 0;
	int nColumn = 0;
	char **azResult = NULL;
	char *zErrMsg = NULL;

    if (!sql || !db || !array) {
		log_error("sql query with null message\n");
		return 1;
    }

	log_debug("%s\n", sql);
	log_debug("bef select:\n");

	rc = sqlite3_get_table(db, sql, &azResult, &nRow, &nColumn, &zErrMsg);
	if (rc != SQLITE_OK && rc != SQLITE_NOTFOUND) {
		log_debug("error select:\n");
		log_error("%s%s, rc %x!\n",
								zErrMsg ? "error:" : "",
								zErrMsg ? zErrMsg : "",
								rc);
		return 1;
	}

	log_debug("after select:\n");

	/* for debug */
    log_debug("query result %d row(s) %d column(s):\n#",nRow, nColumn);
	for (i = 0; i <= nRow; i++) {
		for(j = 0; j < nColumn; j++) {
			log_debug("%-12s ", azResult[i * nColumn + j]);
		}
		log_debug("#\n#");
    }
	
	for (i = 1; i <= nRow; i++) {
		/* BINDTYPE, IP, MAC, LESAE, VLAN, PORT */
		/**************************************************************
		array[i - 1].bind_type  = strtoul(azResult[i * nColumn + 0], (char **)NULL, 10);
		array[i - 1].ip_addr    = strtoul(azResult[i * nColumn + 1], (char **)NULL, 10);
		memcpy((char *)(array[i-1].chaddr), azResult[i * nColumn + 2], 12);
		array[i - 1].lease_time = strtoul(azResult[i * nColumn + 3], (char **)NULL, 10);
		array[i - 1].vlanId     = strtoul(azResult[i * nColumn + 4], (char **)NULL, 10);
		array[i - 1].ifindex    = strtoul(azResult[i * nColumn + 5], (char **)NULL, 10);

	    log_debug("bind %d ip %3d.%3d.%3d.%3d mac %c%c%c%c%c%c%c%c%c%c%c%c lease %d vlanid %d ifindex %d\n",
						array[i - 1].bind_type,
						(array[i - 1].ip_addr>>24) & 0xff, (array[i - 1].ip_addr>>16) & 0xff,
						(array[i - 1].ip_addr>>8) & 0xff, array[i - 1].ip_addr & 0xff,
						array[i-1].chaddr[0], array[i-1].chaddr[1], array[i-1].chaddr[2], array[i-1].chaddr[3], 
						array[i-1].chaddr[4], array[i-1].chaddr[5], array[i-1].chaddr[6], array[i-1].chaddr[7], 
						array[i-1].chaddr[8], array[i-1].chaddr[9], array[i-1].chaddr[10], array[i-1].chaddr[11], 
						array[i - 1].lease_time,
						array[i - 1].vlanId, array[i - 1].ifindex);
						****/
    }

	*count = nRow;

    sqlite3_free_table(azResult);

	log_debug("query from dhcpSnpDb success.\n");

	return 0;
}

#endif

static isc_result_t write_binding_scope(FILE *db_file, struct binding *bnd,
					char *prepend);

FILE *db_file;

static int counting = 0;
static int count = 0;
TIME write_time;
int lease_file_is_corrupt = 0;

/* Write a single binding scope value in parsable format.
 */

static isc_result_t
write_binding_scope(FILE *db_file, struct binding *bnd, char *prepend) {
	char *s;

	if ((db_file == NULL) || (bnd == NULL) || (prepend == NULL))
		return ISC_R_INVALIDARG;

	if (bnd->value->type == binding_data) {
		if (bnd->value->value.data.data != NULL) {
			s = quotify_buf(bnd->value->value.data.data,
					bnd->value->value.data.len, MDL);
			if (s != NULL) {
				errno = 0;
				fprintf(db_file, "%sset %s = \"%s\";",
					prepend, bnd->name, s);
				if (errno)
					return ISC_R_FAILURE;

				dfree(s, MDL);
			} else {
			    return ISC_R_FAILURE;
			}
		}
	} else if (bnd->value->type == binding_numeric) {
		errno = 0;
		fprintf(db_file, "%sset %s = %%%ld;", prepend,
			bnd->name, bnd->value->value.intval);
		if (errno)
			return ISC_R_FAILURE;
	} else if (bnd->value->type == binding_boolean) {
		errno = 0;
		fprintf(db_file, "%sset %s = %s;", prepend, bnd->name,
			bnd->value->value.intval ? "true" : "false");
		if (errno)
			return ISC_R_FAILURE;
	} else if (bnd->value->type == binding_dns) {
		log_error("%s: persistent dns values not supported.",
			  bnd->name);
	} else if (bnd->value->type == binding_function) {
		log_error("%s: persistent functions not supported.",
			  bnd->name);
	} else {
		log_fatal("%s: unknown binding type %d", bnd->name,
			  bnd->value->type);
	}

	return ISC_R_SUCCESS;
}

/* Write the specified lease to the current lease database file. */

int write_lease (lease)
	struct lease *lease;
{
	int errors = 0;
	const char *tval;

	if (dhcp_optimize_enable) {
		return 1;
	}
	/* If the lease file is corrupt, don't try to write any more leases
	   until we've written a good lease file. */
	if (lease_file_is_corrupt)
		if (!new_lease_file ())
			return 0;

	if (counting)
		++count;
	errno = 0;
#ifndef __AX_PLATFORM__
	if(lease->seek) {
		if(fseek(db_file, lease->seek, SEEK_SET) == -1){
			log_error("fseek error!\n");
		}
	}
	else {
		lease->seek = ftell(db_file);
		if(fseek(db_file, 0, SEEK_END) == -1){
			log_error("fseek error!\n");
		}
	}
#endif

	fprintf (db_file, "lease %s {", piaddr (lease -> ip_addr));
	if (errno) {
		++errors;
	}

	if (lease->starts &&
	    ((tval = print_time(lease->starts)) == NULL ||
	     fprintf(db_file, "\n  starts %s", tval) < 0))
		++errors;

	if (lease->ends &&
	    ((tval = print_time(lease->ends)) == NULL ||
	     fprintf(db_file, "\n  ends %s", tval) < 0))
		++errors;
/*
	if (lease->tstp &&
	    ((tval = print_time(lease->tstp)) == NULL ||
	     fprintf(db_file, "\n  tstp %s", tval) < 0))
		++errors;

	if (lease->tsfp &&
	    ((tval = print_time(lease->tsfp)) == NULL ||
	     fprintf(db_file, "\n  tsfp %s", tval) < 0))
		++errors;

	if (lease->atsfp &&
	    ((tval = print_time(lease->atsfp)) == NULL ||
	     fprintf(db_file, "\n  atsfp %s", tval) < 0))
		++errors;
*/
	if (lease->cltt &&
	    ((tval = print_time(lease->cltt)) == NULL ||
	     fprintf(db_file, "\n  cltt %s", tval) < 0))
		++errors;

	if (fprintf (db_file, "\n  binding state %s;",
		 ((lease -> binding_state > 0 &&
		   lease -> binding_state <= FTS_LAST)
		  ? binding_state_names [lease -> binding_state - 1]
		  : "abandoned")) < 0)
                ++errors;

/*	if (lease -> binding_state != lease -> next_binding_state)*/
		if (fprintf (db_file, "\n  next binding state %s;",
			 ((lease -> next_binding_state > 0 &&
			   lease -> next_binding_state <= FTS_LAST)
			  ? (binding_state_names
			     [lease -> next_binding_state - 1])
			  : "abandoned")) < 0)
                        ++errors;
/*
	if (lease->flags & RESERVED_LEASE)
		if (fprintf(db_file, "\n  reserved;") < 0)
                        ++errors;

	if (lease->flags & BOOTP_LEASE)
		if (fprintf(db_file, "\n  dynamic-bootp;") < 0)
                        ++errors;
*/
	/* If this lease is billed to a class and is still valid,
	   write it out. 
	if (lease -> billing_class && lease -> ends > cur_time) {
		if (!write_billing_class (lease -> billing_class)) {
			log_error ("unable to write class %s",
				   lease -> billing_class -> name);
			++errors;
		}
	}*/

	if (lease -> hardware_addr.hlen) {
		errno = 0;
		fprintf (db_file, "\n  hardware %s %s;",
			 hardware_types [lease -> hardware_addr.hbuf [0]],
			 print_hw_addr (lease -> hardware_addr.hbuf [0],
					lease -> hardware_addr.hlen - 1,
					&lease -> hardware_addr.hbuf [1]));
		if (errno)
			++errors;
	}
	
/*	if (lease -> uid_len) {
		s = quotify_buf (lease -> uid, lease -> uid_len, MDL);
		if (s) {
			errno = 0;
			fprintf (db_file, "\n  uid \"%s\";", s);
			if (errno)
				++errors;
			dfree (s, MDL);
		} else
			++errors;
	}

	if (lease->scope != NULL) {
	    for (b = lease->scope->bindings; b; b = b->next) {
		if (!b->value)
			continue;

		if (write_binding_scope(db_file, b, "\n  ") != ISC_R_SUCCESS)
			++errors;
	    }
	}

	if (lease -> agent_options) {
	    struct option_cache *oc;
	    struct data_string ds;
	    pair p;

	    memset (&ds, 0, sizeof ds);
	    for (p = lease -> agent_options -> first; p; p = p -> cdr) {
	        oc = (struct option_cache *)p -> car;
	        if (oc -> data.len) {
	    	errno = 0;
	    	fprintf (db_file, "\n  option agent.%s %s;",
	    		 oc -> option -> name,
	    		 pretty_print_option (oc -> option, oc -> data.data,
				      		oc -> data.len, 1, 1));
	    	if (errno)
		    ++errors;
	        }
	    }
	}
	
	if (lease -> client_hostname &&
	    db_printable((unsigned char *)lease->client_hostname)) {
		s = quotify_string (lease -> client_hostname, MDL);
		if (s) {
			errno = 0;
			fprintf (db_file, "\n  client-hostname \"%s\";", s);
			if (errno)
				++errors;
			dfree (s, MDL);
		} else
			++errors;
	}
	if (lease -> on_expiry) {
		errno = 0;
		fprintf (db_file, "\n  on expiry%s {",
			 lease -> on_expiry == lease -> on_release
			 ? " or release" : "");
		write_statements (db_file, lease -> on_expiry, 4);
		fprintf (db_file, "\n  }");
		if (errno)
			++errors;
	}
	if (lease -> on_release && lease -> on_release != lease -> on_expiry) {
		errno = 0;
		fprintf (db_file, "\n  on release {");
		write_statements (db_file, lease -> on_release, 4);
		fprintf (db_file, "\n  }");
		if (errno)
			++errors;
	}
*/
	errno = 0;
	fputs ("\n}\n", db_file);
	if (errno)
		++errors;
	
#ifndef __AX_PLATFORM__
/*
	char *buf;
	int i = 0;
	lease->seekend = ftell(db_file);
	if (lease->seekend < end) {
		i = end - lease->seekend;
		buf = malloc(i);
		memset(buf, 0, i);
log_debug("22 seek i %d \n", i);	
		fseek(db_file, -i, SEEK_END);
		fgets(buf, sizeof(buf), db_file);
log_debug("2233 seek cur %s \n", buf);		
		fseek(db_file, -i, SEEK_END);	
		while(i--) {
			if (buf[i] == '\n') {
				fputs ("\n", db_file);
			}
			else {				
				fputs (" ", db_file);
			}
		}
	}
	*/
	fseek(db_file, 0, SEEK_END);
#endif

	if (errors) {
		log_debug ("write_lease: unable to write lease %s",
		      piaddr (lease -> ip_addr));
		lease_file_is_corrupt = 1;
        }

	return !errors;
}

int write_host (host)
	struct host_decl *host;
{
	int errors = 0;

	/* If the lease file is corrupt, don't try to write any more leases
	   until we've written a good lease file. 
	if (lease_file_is_corrupt)
		if (!new_lease_file ())
			return 0;

	if (!db_printable((unsigned char *)host->name))
		return 0;

	if (counting)
		++count;

	errno = 0;
	fprintf (db_file, "host %s {", host -> name);
	if (errno)
		++errors;

	if (host -> flags & HOST_DECL_DYNAMIC) {
		errno = 0;
		fprintf (db_file, "\n  dynamic;");
		if (errno)
			++errors;
	}

	if (host -> flags & HOST_DECL_DELETED) {
		errno = 0;
		fprintf (db_file, "\n  deleted;");
		if (errno)
			++errors;
	} else {
		if (host -> interface.hlen) {
			errno = 0;
			fprintf (db_file, "\n  hardware %s %s;",
				 hardware_types [host -> interface.hbuf [0]],
				 print_hw_addr (host -> interface.hbuf [0],
						host -> interface.hlen - 1,
						&host -> interface.hbuf [1]));
			if (errno)
				++errors;
		}
		if (host -> client_identifier.len) {
			int i;
			errno = 0;
			if (db_printable_len (host -> client_identifier.data,
					      host -> client_identifier.len)) {
				fprintf (db_file, "\n  uid \"%.*s\";",
					 (int)host -> client_identifier.len,
					 host -> client_identifier.data);
				if (errno)
					++errors;
			} else {
				fprintf (db_file,
					 "\n  uid %2.2x",
					 host -> client_identifier.data [0]);
				if (errno)
					++errors;
				for (i = 1;
				     i < host -> client_identifier.len; i++) {
					errno = 0;
					fprintf (db_file, ":%2.2x",
						 host ->
						 client_identifier.data [i]);
					if (errno)
						++errors;
				}

                                errno = 0;
				fputc (';', db_file);
				if (errno)
					++errors;
			}
		}
		
		memset (&ip_addrs, 0, sizeof ip_addrs);
		if (host -> fixed_addr &&
		    evaluate_option_cache (&ip_addrs, (struct packet *)0,
					   (struct lease *)0,
					   (struct client_state *)0,
					   (struct option_state *)0,
					   (struct option_state *)0,
					   &global_scope,
					   host -> fixed_addr, MDL)) {
		
			errno = 0;
			fprintf (db_file, "\n  fixed-address ");
			if (errno)
				++errors;
			for (i = 0; i < ip_addrs.len - 3; i += 4) {

				errno = 0;
				fprintf (db_file, "%u.%u.%u.%u%s",
					 ip_addrs.data [i] & 0xff,
					 ip_addrs.data [i + 1] & 0xff,
					 ip_addrs.data [i + 2] & 0xff,
					 ip_addrs.data [i + 3] & 0xff,
					 i + 7 < ip_addrs.len ? "," : "");
				if (errno)
					++errors;
			}

			errno = 0;
			fputc (';', db_file);
			if (errno)
				++errors;
		}

		if (host -> named_group) {
			errno = 0;
			fprintf (db_file, "\n  group \"%s\";",
				 host -> named_group -> name);
			if (errno)
				++errors;
		}

		if (host -> group &&
		    (!host -> named_group ||
		     host -> group != host -> named_group -> group) &&
		    host -> group != root_group) {
			errno = 0;
			write_statements (db_file,
					  host -> group -> statements, 8);
			if (errno)
				++errors;
		}
	}

	errno = 0;
	fputs ("\n}\n", db_file);
	if (errno)
		++errors;

	if (errors) {
		log_info ("write_host: unable to write host %s",
			  host -> name);
		lease_file_is_corrupt = 1;
	}
	*/

	return !errors;
}

int write_group (group)
	struct group_object *group;
{
	int errors = 0;

	/* If the lease file is corrupt, don't try to write any more leases
	   until we've written a good lease file. */
	if (lease_file_is_corrupt)
		if (!new_lease_file ())
			return 0;

	if (!db_printable((unsigned char *)group->name))
		return 0;

	if (counting)
		++count;

	errno = 0;
	fprintf (db_file, "group %s {", group -> name);
	if (errno)
		++errors;

	if (group -> flags & GROUP_OBJECT_DYNAMIC) {
		errno = 0;
		fprintf (db_file, "\n  dynamic;");
		if (errno)
			++errors;
	}

	if (group -> flags & GROUP_OBJECT_STATIC) {
		errno = 0;
		fprintf (db_file, "\n  static;");
		if (errno)
			++errors;
	}

	if (group -> flags & GROUP_OBJECT_DELETED) {
		errno = 0;
		fprintf (db_file, "\n  deleted;");
		if (errno)
			++errors;
	} else {
		if (group -> group) {
			errno = 0;
			write_statements (db_file,
					  group -> group -> statements, 8);
			if (errno)
				++errors;
		}
	}

	errno = 0;
	fputs ("\n}\n", db_file);
	if (errno)
		++errors;

	if (errors) {
		log_error ("write_group: unable to write group %s",
			  group -> name);
		lease_file_is_corrupt = 1;
	}

	return !errors;
}

/*
 * Write an IA_NA and the options it has.
 */
int
write_ia_na(const struct ia_na *ia_na) {
	struct iaaddr *iaaddr;
	struct binding *bnd;
	int i;
	char addr_buf[sizeof("ffff:ffff:ffff:ffff:ffff:ffff.255.255.255.255")];
	const char *binding_state;
	const char *tval;
	char *s;
	int fprintf_ret;

	/* 
	 * If the lease file is corrupt, don't try to write any more 
	 * leases until we've written a good lease file. 
	 */
	if (lease_file_is_corrupt) {
		if (!new_lease_file()) {
			return 0;
		}
	}

	if (counting) {
		++count;
	}

	
	s = quotify_buf(ia_na->iaid_duid.data, ia_na->iaid_duid.len, MDL);
	if (s == NULL) {
		goto error_exit;
	}
	fprintf_ret = fprintf(db_file, "ia-na \"%s\" {\n", s);
	dfree(s, MDL);
	if (fprintf_ret < 0) {
		goto error_exit;
	}
	for (i=0; i<ia_na->num_iaaddr; i++) {
		iaaddr = ia_na->iaaddr[i];

		inet_ntop(AF_INET6, &iaaddr->addr, addr_buf, sizeof(addr_buf));
		if (fprintf(db_file, "  iaaddr %s {\n", addr_buf) < 0) {
			goto error_exit;
		}
		if ((iaaddr->state <= 0) || (iaaddr->state > FTS_LAST)) {
			log_fatal("Unknown ia_na state %d at %s:%d", 
				  iaaddr->state, MDL);
		}
		binding_state = binding_state_names[iaaddr->state-1];
		if (fprintf(db_file, "    binding state %s;\n", 
			    binding_state) < 0) {
			goto error_exit;
		}

		/* Note that from here on out, the \n is prepended to the
		 * next write, rather than appended to the current write.
		 */
		tval = print_time(iaaddr->valid_lifetime_end_time);
		if (tval == NULL) {
			goto error_exit;
		}
		if (fprintf(db_file, "    ends %s", tval) < 0) {
			goto error_exit;
		}

		/* Write out any binding scopes: note that 'ends' above does
		 * not have \n on the end!  We want that.
		 */
		if (iaaddr->scope != NULL)
			bnd = iaaddr->scope->bindings;
		else
			bnd = NULL;

		for (; bnd != NULL ; bnd = bnd->next) {
			if (bnd->value == NULL)
				continue;

			/* We don't do a regular error_exit because the
			 * lease db is not corrupt in this case.
			 */
			if (write_binding_scope(db_file, bnd,
						"\n    ") != ISC_R_SUCCESS)
				goto error_exit;
				
		}

		if (fprintf(db_file, "\n  }\n") < 0)
                        goto error_exit;
	}
	if (fprintf(db_file, "}\n\n") < 0)
                goto error_exit;

	fflush(db_file);
	return 1;

error_exit:
	log_error("write_ia_na: unable to write ia-na");
	lease_file_is_corrupt = 1;
	return 0;
}

#ifdef DHCPv6
/*
 * Put a copy of the server DUID in the leases file.
 */
int
write_server_duid(void) {
	struct data_string server_duid;
	char *s;
	int fprintf_ret;

	/*
	 * Only write the DUID if it's been set.
	 */
	if (!server_duid_isset()) {
		return 1;
	}

	/* 
	 * If the lease file is corrupt, don't try to write any more 
	 * leases until we've written a good lease file. 
	 */
	if (lease_file_is_corrupt) {
		if (!new_lease_file()) {
			return 0;
		}
	}

	/*
	 * Get a copy of our server DUID and convert to a quoted string.
	 */
	memset(&server_duid, 0, sizeof(server_duid));
	copy_server_duid(&server_duid, MDL);
	s = quotify_buf(server_duid.data, server_duid.len, MDL);
	data_string_forget(&server_duid, MDL);
	if (s == NULL) {
		goto error_exit;
	}

	/*
	 * Write to the leases file.
	 */
	fprintf_ret = fprintf(db_file, "server-duid \"%s\";\n\n", s);
	dfree(s, MDL);
	if (fprintf_ret < 0) {
		goto error_exit;
	}

	/*
	 * Check if we actually managed to write.
	 */
	fflush(db_file);
	return 1;

error_exit:
	log_error("write_server_duid: unable to write server-duid");
	lease_file_is_corrupt = 1;
	return 0;
}
#endif /* DHCPv6 */

#if defined (FAILOVER_PROTOCOL)
int write_failover_state (dhcp_failover_state_t *state)
{
	/*

	if (lease_file_is_corrupt)
		if (!new_lease_file ())
			return 0;

	errno = 0;
	fprintf (db_file, "\nfailover peer \"%s\" state {", state -> name);
	if (errno)
		++errors;

	tval = print_time(state->me.stos);
	if (tval == NULL ||
	    fprintf(db_file, "\n  my state %s at %s",
		    (state->me.state == startup) ?
		    dhcp_failover_state_name_print(state->saved_state) :
		    dhcp_failover_state_name_print(state->me.state),
		    tval) < 0)
		++errors;

	tval = print_time(state->partner.stos);
	if (tval == NULL ||
	    fprintf(db_file, "\n  partner state %s at %s",
		    dhcp_failover_state_name_print(state->partner.state),
		    tval) < 0)
		++errors;

	if (state -> i_am == secondary) {
		errno = 0;
		fprintf (db_file, "\n  mclt %ld;",
			 (unsigned long)state -> mclt);
		if (errno)
			++errors;
	}

        errno = 0;
	fprintf (db_file, "\n}\n");
	if (errno)
		++errors;

	if (errors) {
		log_info ("write_failover_state: unable to write state %s",
			  state -> name);
		lease_file_is_corrupt = 1;
		return 0;
	}

*/	
	return 1;
}

#endif

int db_printable (s)
	const unsigned char *s;
{
	int i;
	for (i = 0; s [i]; i++)
		if (!isascii (s [i]) || !isprint (s [i])
		    || s [i] == '"' || s [i] == '\\')
			return 0;
	return 1;
}

int db_printable_len (s, len)
	const unsigned char *s;
	unsigned len;
{
	int i;

	for (i = 0; i < len; i++)
		if (!isascii (s [i]) || !isprint (s [i]) ||
		    s [i] == '"' || s [i] == '\\')
			return 0;
	return 1;
}

static int print_hash_string(FILE *fp, struct class *class)
{
	int i;

	for (i = 0 ; i < class->hash_string.len ; i++)
		if (!isascii(class->hash_string.data[i]) ||
		    !isprint(class->hash_string.data[i]))
			break;

	if (i == class->hash_string.len) {
		if (fprintf(fp, " \"%.*s\"", (int)class->hash_string.len,
			    class->hash_string.data) <= 0) {
			log_error("Failure writing hash string: %m");
			return 0;
		}
	} else {
		if (fprintf(fp, " %2.2x", class->hash_string.data[0]) <= 0) {
			log_error("Failure writing hash string: %m");
			return 0;
		}
		for (i = 1 ; i < class->hash_string.len ; i++) {
			if (fprintf(fp, ":%2.2x",
				    class->hash_string.data[i]) <= 0) {
				log_error("Failure writing hash string: %m");
				return 0;
			}
		}
	}

	return 1;
}


isc_result_t
write_named_billing_class(const void *key, unsigned len, void *object)
{
	const unsigned char *name = key;
	struct class *class = object;

	if (class->flags & CLASS_DECL_DYNAMIC) {
		numclasseswritten++;
		if (class->superclass == 0) {
			if (fprintf(db_file, "class \"%s\" {\n", name) <= 0)
				return ISC_R_IOERROR;
		} else {
			if (fprintf(db_file, "subclass \"%s\"",
				    class->superclass->name) <= 0)
				return ISC_R_IOERROR;
			if (!print_hash_string(db_file, class))
				return ISC_R_IOERROR;
			if (fprintf(db_file, " {\n") <= 0)
				return ISC_R_IOERROR;
		}

		if ((class->flags & CLASS_DECL_DELETED) != 0) {
			if (fprintf(db_file, "  deleted;\n") <= 0)
				return ISC_R_IOERROR;
		} else {
			if (fprintf(db_file, "  dynamic;\n") <= 0)
				return ISC_R_IOERROR;
		}
	
		if (class->lease_limit > 0) {
			if (fprintf(db_file, "  lease limit %d;\n",
				    class->lease_limit) <= 0)
				return ISC_R_IOERROR;
		}

		if (class->expr != 0) {
			if (fprintf(db_file, "  match if ") <= 0)
				return ISC_R_IOERROR;

                        errno = 0;                                       
			write_expression(db_file, class->expr, 5, 5, 0);
                        if (errno)
                                return ISC_R_IOERROR;

			if (fprintf(db_file, ";\n") <= 0)
				return ISC_R_IOERROR;
		}

		if (class->submatch != 0) {
			if (class->spawning) {
				if (fprintf(db_file, "  spawn ") <= 0)
					return ISC_R_IOERROR;
			} else {
				if (fprintf(db_file, "  match ") <= 0)
					return ISC_R_IOERROR;
			}

                        errno = 0;
			write_expression(db_file, class->submatch, 5, 5, 0);
                        if (errno)
                                return ISC_R_IOERROR;

			if (fprintf(db_file, ";\n") <= 0)
				return ISC_R_IOERROR;
		}
	
		if (class->statements != 0) {
                        errno = 0;
			write_statements(db_file, class->statements, 8);
                        if (errno)
                                return ISC_R_IOERROR;
		}

		/* XXXJAB this isn't right, but classes read in off the
		   leases file don't get the root group assigned to them
		   (due to clone_group() call). */
		if (class->group != 0 && class->group->authoritative != 0) {
                        errno = 0;
			write_statements(db_file, class->group->statements, 8);
                        if (errno)
                                return ISC_R_IOERROR;
                }

		if (fprintf(db_file, "}\n\n") <= 0)
			return ISC_R_IOERROR;
	}

	if (class->hash != NULL) {	/* yep. recursive. god help us. */
		/* XXX - cannot check error status of this...
		 * foo_hash_foreach returns a count of operations completed.
		 */
		class_hash_foreach(class->hash, write_named_billing_class);
	}

	return ISC_R_SUCCESS;
}

void write_billing_classes ()
{
	struct collection *lp;
	struct class *cp;

	for (lp = collections; lp; lp = lp -> next) {
	    for (cp = lp -> classes; cp; cp = cp -> nic) {
		if (cp -> spawning && cp -> hash) {
		    class_hash_foreach (cp -> hash, write_named_billing_class);
		}
	    }
	}
}

/* Write a spawned class to the database file. */

int write_billing_class (class)
	struct class *class;
{
	int errors = 0;

	if (lease_file_is_corrupt)
		if (!new_lease_file ())
			return 0;

	if (!class -> superclass) {
		errno = 0;
		fprintf (db_file, "\n  billing class \"%s\";", class -> name);
		return !errno;
	}

	if (fprintf(db_file, "\n  billing subclass \"%s\"",
		    class -> superclass -> name) < 0)
		++errors;

	if (!print_hash_string(db_file, class))
                ++errors;

	if (fprintf(db_file, ";") < 0)
                ++errors;

	class -> dirty = !errors;
	if (errors)
		lease_file_is_corrupt = 1;

	return !errors;
}

/* Commit leases after a timeout. */
void commit_leases_timeout (void *foo)
{
	commit_leases ();
}

/* Commit any leases that have been written out... */

int commit_leases ()
{
#ifndef __AX_PLATFORM__
	if (dhcp_optimize_enable) {
		return 1;
	}
#endif	
	/* Commit any outstanding writes to the lease database file.
	   We need to do this even if we're rewriting the file below,
	   just in case the rewrite fails. */
	if (fflush (db_file) == EOF) {
		log_error("commit_leases: unable to commit: %m");
		return 0;
	}
	if (fsync (fileno (db_file)) < 0) {
		log_error("commit_leases: unable to commit: %m");
		return 0;
	}

	/* If we haven't rewritten the lease database in over an
	   hour, rewrite it now.  (The length of time should probably
	   be configurable. */
	if (count && cur_time - write_time > 3600) {
		count = 0;
		write_time = cur_time;
		new_lease_file ();
	}
	return 1;
}

void db_startup (testp)
	int testp;
{
	isc_result_t status;

#if defined (TRACING)
	if (!trace_playback ()) {
#endif
		/* Read in the existing lease file... */
		status = read_conf_file (path_dhcpd_db,
					 (struct group *)0, 0, 1);
		/* XXX ignore status? */
#if defined (TRACING)
	}
#endif

#if defined (TRACING)
	/* If we're playing back, there is no lease file, so we can't
	   append it, so we create one immediately (maybe this isn't
	   the best solution... */
	if (trace_playback ()) {
		new_lease_file ();
	}
#endif
	if (!testp) {
		db_file = fopen (path_dhcpd_db, "a");
		if (!db_file) {
//			log_fatal ("Can't open %s for append.", path_dhcpd_db);
			log_error("Can't open %s for append.", path_dhcpd_db);
		}
//		expire_all_pools (); do it later
#if defined (TRACING)
		if (trace_playback ())
			write_time = cur_time;
		else
#endif
			time(&write_time);
		new_lease_file ();
	}

#if defined(REPORT_HASH_PERFORMANCE)
	log_debug("Host HW hash:   %s", host_hash_report(host_hw_addr_hash));
	log_debug("Host UID hash:  %s", host_hash_report(host_uid_hash));
	log_debug("Lease IP hash:  %s",
		 lease_ip_hash_report(lease_ip_addr_hash));
	log_debug("Lease UID hash: %s", lease_id_hash_report(lease_uid_hash));
	log_debug("Lease HW hash:  %s",
		 lease_id_hash_report(lease_hw_addr_hash));
#endif
}

int new_lease_file ()
{
	char newfname [512];
	char backfname [512];
	TIME t;
	int db_fd;
	int db_validity;
	FILE *new_db_file;

	/* Make a temporary lease file... */
	time(&t);

	db_validity = lease_file_is_corrupt;

	/* %Audit% Truncated filename causes panic. %2004.06.17,Safe%
	 * This should never happen since the path is a configuration
	 * variable from build-time or command-line.  But if it should,
	 * either by malice or ignorance, we panic, since the potential
	 * for havoc is high.
	 */
	if (snprintf (newfname, sizeof newfname, "%s.%d",
		     path_dhcpd_db, (int)t) >= sizeof newfname)
		log_fatal("new_lease_file: lease file path too long");

	db_fd = open (newfname, O_WRONLY | O_TRUNC | O_CREAT, 0664);
	if (db_fd < 0) {
		log_error ("Can't create new lease file: %m");
		return 0;
	}
	if ((new_db_file = fdopen(db_fd, "w")) == NULL) {
		log_error("Can't fdopen new lease file: %m");
		close(db_fd);
		goto fdfail;
	}

	/* Close previous database, if any. */
	if (db_file)
		fclose(db_file);
	db_file = new_db_file;

	errno = 0;
	fprintf (db_file, "# The format of this file is documented in the %s",
		 "dhcpd.leases(5) manual page.\n");
	if (errno)
		goto fail;

	fprintf (db_file, "# This lease file was written by isc-dhcp-%s\n\n",
		 PACKAGE_VERSION);
	if (errno)
		goto fail;

	/* At this point we have a new lease file that, so far, could not
	 * be described as either corrupt nor valid.
	 */
	lease_file_is_corrupt = 0;

	/* Write out all the leases that we know of... */
	counting = 0;
	if (!write_leases ())
		goto fail;

#if defined (TRACING)
	if (!trace_playback ()) {
#endif
	    /* %Audit% Truncated filename causes panic. %2004.06.17,Safe%
	     * This should never happen since the path is a configuration
	     * variable from build-time or command-line.  But if it should,
	     * either by malice or ignorance, we panic, since the potential
	     * for havoc is too high.
	     */
	    if (snprintf (backfname, sizeof backfname, "%s~", path_dhcpd_db)
			>= sizeof backfname)
		log_fatal("new_lease_file: backup lease file path too long");

	    /* Get the old database out of the way... */
	    if (unlink (backfname) < 0 && errno != ENOENT) {
		log_error ("Can't remove old lease database backup %s: %m",
			   backfname);
		goto fail;
	    }
	    if (link(path_dhcpd_db, backfname) < 0) {
		if (errno == ENOENT) {
			log_error("%s is missing - no lease db to backup.",
				  path_dhcpd_db);
		} else {
			log_error("Can't backup lease database %s to %s: %m",
				  path_dhcpd_db, backfname);
			goto fail;
		}
	    }
#if defined (TRACING)
	}
#endif
	
	/* Move in the new file... */
	if (rename (newfname, path_dhcpd_db) < 0) {
		log_error ("Can't install new lease database %s to %s: %m",
			   newfname, path_dhcpd_db);
		goto fail;
	}

	counting = 1;
	return 1;

      fail:
	lease_file_is_corrupt = db_validity;
      fdfail:
	unlink (newfname);
	return 0;
}

int group_writer (struct group_object *group)
{
	if (!write_group (group))
		return 0;
	if (!commit_leases ())
		return 0;
	return 1;
}
