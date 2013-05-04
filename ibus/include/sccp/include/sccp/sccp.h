/*
 * SCCP management code
 *
 * (C) 2009, 2010 by Holger Hans Peter Freyther <zecke@selfish.org>
 *
 * All Rights Reserved
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License along
 * with this program; if not, write to the Free Software Foundation, Inc.,
 * 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.
 *
 */

#ifndef SCCP_H
#define SCCP_H

#include <stdlib.h>

#include <sys/socket.h>
#include <sys/types.h>

#include "sccp_types.h"
#include "../../../../../accapi/iuh/Iuh.h"

#include <osmocore/linuxlist.h>

#define	IU_FREE_OBJECT(obj_name)				{if(obj_name){free((obj_name)); (obj_name) = NULL;}}

#define IU_HNB_DEFAULT_NUM     (1024)
#define IU_HNB_MAX_UE_NUM              (16)
#define IU_UE_MAX_NUM         (IU_HNB_DEFAULT_NUM * IU_HNB_MAX_UE_NUM)
#define IU_MAX_INS_NUM		(16)

#define IU_IMSI_LEN 8
#define IU_CONTEXTID_LEN 3

struct msgb;
struct sccp_system;

unsigned int start_sccp_con;

/*local msc paras*/
extern struct cn_config home_gateway_msc ;
/*local sgsn paras*/
extern struct cn_config home_gateway_sgsn ;
/*remote msc paras*/
extern struct cn_config global_msc_parameter;
/*remote sgsn paras*/
extern struct cn_config global_sgsn_parameter;


extern int gNi;
extern int gCnDomain;

enum {
	SCCP_CONNECTION_STATE_NONE,
	SCCP_CONNECTION_STATE_REQUEST,
	SCCP_CONNECTION_STATE_CONFIRM,
	SCCP_CONNECTION_STATE_ESTABLISHED,
	SCCP_CONNECTION_STATE_RELEASE,
	SCCP_CONNECTION_STATE_RELEASE_COMPLETE,
	SCCP_CONNECTION_STATE_REFUSED,
	SCCP_CONNECTION_STATE_SETUP_ERROR,
};

struct sockaddr_sccp {
	sa_family_t	sccp_family;		/* AF_SCCP in the future??? */
	uint8_t	sccp_ssn;		/* subssystem number for routing */

	/* TODO fill in address indicator... if that is ever needed */

	/* optional gti information */
	uint8_t *gti;
	int gti_len;

	/* any of SCCP_TITLE_IND_* */
	uint8_t gti_ind;

	int use_poi;
	uint8_t poi[2];

	/* not sure about these */
	/* uint8_t    sccp_class; */
};

/*
 * parsed structure of an address
 */
struct sccp_address {
	struct sccp_called_party_address    address;
	uint8_t			    ssn;
	uint8_t			    poi[2];

	uint8_t			    *gti_data;
	int			    gti_len;
};

struct sccp_optional_data {
	uint8_t			    data_len;
	uint8_t			    data_start;
};

struct sccp_connection {
	/* public */
	void *data_ctx;
	void (*data_cb)(struct sccp_connection *conn, struct msgb *msg, unsigned int len);

	void *state_ctx;
	void (*state_cb)(struct sccp_connection *, int old_state);

	struct sccp_source_reference source_local_reference;
	struct sccp_source_reference destination_local_reference;
	/*add for connect with iuh*/
	unsigned char contextid[IU_CONTEXTID_LEN];	
/*    CS_Domain = 0,
    PS_Domain = 1*/
	unsigned int domainid;
	
	int connection_state;

	/* private */
	/* list of active connections */
	struct llist_head list;
	struct sccp_system *system;
	int incoming;
};


/* book add, 2011-12-21 */
typedef struct IuUE{
	uint32_t UEID;
	unsigned char IMSI[IU_IMSI_LEN];
	unsigned char ContextID[IU_CONTEXTID_LEN];
	struct IuUE *ue_next;
}Iu_UE;

extern Iu_UE **IU_UE;

/**
 * Create a new socket. Set your callbacks and then call bind to open
 * the connection.
 */
struct sccp_connection *sccp_connection_socket(void);

/* generic sock addresses */
extern const struct sockaddr_sccp sccp_ssn_ranap;

/* helpers */
uint32_t sccp_src_ref_to_int(struct sccp_source_reference *ref);
struct sccp_source_reference sccp_src_ref_from_int(uint32_t);

struct msgb *sccp_create_refuse(struct sccp_source_reference *src_ref, int cause, uint8_t *data, int length);
struct msgb *sccp_create_cc(struct sccp_source_reference *src_ref, struct sccp_source_reference *dst_ref);
struct msgb *sccp_create_rlsd(struct sccp_source_reference *src_ref, struct sccp_source_reference *dst_ref, int cause);
struct msgb *sccp_create_dt1(struct sccp_source_reference *dst_ref, uint8_t *data, uint8_t len);
struct msgb *sccp_create_udt(int _class, const struct sockaddr_sccp *sock_sender,
			     const struct sockaddr_sccp *sock_target, uint8_t *data, int len);

/**
 * Below this are helper functions and structs for parsing SCCP messages
 */
struct sccp_parse_result {
	struct sccp_address called;
	struct sccp_address calling;

	/* point to the msg packet */
	struct sccp_source_reference *source_local_reference;
	struct sccp_source_reference *destination_local_reference;

	/* data pointer */
	int data_len;
};

/*
**  parameters of paging message
*/

typedef enum {
    PAGING_AREA_NOTHING,
    PAGING_AREA_LAI,
    PAGING_AREA_RAI
} PagingAreaPresent;


struct PagingAreaId{
    PagingAreaPresent present;
    union{
        IUH_LAI lai;
        IUH_RAI rai;
    }choice;
};

typedef enum {
    TERMINATING_CONVERSATIONAL_CALL = 0,
    TERMINATING_STREAMING_CALL = 1,
    TERMINATING_INTERACTIVE_CALL = 2,
    TERMINATING_BACKGROUND_CALL = 3,
    TERMINATING_LOW_PRIORITY_SIGNALLING = 4,
	/*
	 * Enumeration is extensible
	 */
	TERMINATING_HIGH_PRIORITY_SIGNALLING = 5
} PagingCause;

typedef enum {
    NON_SEARCHING = 0,
    SEARCHING = 1
} NonSearchingIndication;

typedef struct globalCnId {
	char	    plmnid[PLMN_LEN];
	long        cnid;
} GlobalCNID;


typedef struct sccp_ranap_paging {
    CNDomain    CnDomainID;
    char        IMSI[IMSI_LEN];
    char        TMSI[TMSI_LEN];
    struct PagingAreaId paging_area_id;
    PagingCause paging_cause;
    NonSearchingIndication non_searching_indication;
    long        drx_cycle_length;
    GlobalCNID global_cn_id;
} PAGING_PARAS;


/*msc config informaion*/
struct cn_config{
	unsigned int primary_ip;
	unsigned int secondary_ip;
	unsigned short primary_port;
	unsigned short secondary_port;
	unsigned int point_code;
	unsigned char connect_mode;
	unsigned char multi_switch;
};



/**
 * system functionality to implement on top of any other transport layer:
 *   call sccp_system_incoming for incoming data (from the network)
 *   sccp will call outgoing whenever outgoing data exists
 *   The conn is NULL for UDT and other messages without a connection
 */
int sccp_system_init(void (*outgoing)(struct sccp_connection *conn, struct msgb *data, void *ctx), void *context);
int sccp_system_incoming(struct msgb *data);
void ranap_sccp_system(void);

/**
 * Send data on an existing connection
 */
int sccp_connection_write(struct sccp_connection *connection, struct msgb *data);
int sccp_connection_send_it(struct sccp_connection *connection);
int sccp_connection_close(struct sccp_connection *connection, int cause);
int sccp_connection_free(struct sccp_connection *connection);

/**
 * internal.. 
 */
int sccp_connection_force_free(struct sccp_connection *conn);


/**
 * Open the connection and send additional data
 */
int sccp_connection_connect(struct sccp_connection *conn,
			    const struct sockaddr_sccp *sccp_called,
			    struct msgb *data);

/**
 * mostly for testing purposes only. Set the accept callback.
 * TODO: add true routing information... in analogy to socket, bind, accept
 */
int sccp_connection_set_incoming(const struct sockaddr_sccp *sock,
				 int (*accept_cb)(struct sccp_connection *connection, void *data),
				 void *user_data);

/**
 * Send data in terms of unit data. A fixed address indicator will be used.
 */
int sccp_write(struct msgb *data,
	       const struct sockaddr_sccp *sock_sender,
	       const struct sockaddr_sccp *sock_target, int class);
int sccp_set_read(const struct sockaddr_sccp *sock,
		  int (*read_cb)(struct msgb *msgb, unsigned int, void *user_data),
		  void *user_data);

/*
 * helper functions for the nat code
 */
int sccp_determine_msg_type(struct msgb *msg);
int sccp_parse_header(struct msgb *msg, struct sccp_parse_result *result);

/*
 * osmocore logging features
 */
void sccp_set_log_area(int log_area);

int do_sccp_connection(char *data, int len);
int rcv_iuh_send2sigtran(char* msg, int len);

int local_socket_init();

int sccp_decode_Relocation_Request(const char *buf, const int size, char *imsi, CNDomain *domain);

#endif
