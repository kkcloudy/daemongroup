#ifndef _MANAGE_TRANSPORT_H_
#define _MANAGE_TRANSPORT_H_

/*
 * The standard MANAGE domains.  
 */
#define MANAGE_TIPC_DOMAIN	"tipc"
#define MANAGE_NETLINK_DOMAIN	"netlink"


typedef struct manage_transport_s manage_transport;
typedef struct manage_tdomain_s   manage_tdomain;

struct manage_transport_s {

	int 			sock;

	u_long		flags;

	/** transport addr**/
	void			*addr;
	
	unsigned long 	msgMaxSize;
		
	int (*f_recv)		(struct manage_transport_s *, void *,
						size_t, void **, size_t *);	
	int (*f_send)		(struct manage_transport_s *, void *,
						size_t, void *, size_t );
	int (*f_close)		(struct manage_transport_s *);

	/*  This callback is only necessary for stream-oriented transports.  */
	int (*f_accept)	(struct manage_transport_s *);
};

struct manage_tdomain_s {
    	char *name;

	manage_transport *(*f_create_from_chunk) (const u_char *, size_t, u_long);

	struct manage_tdomain_s *next;
};


void manage_transport_free(manage_transport *t);

void manage_tdomain_init(void);

int manage_tdomain_register(manage_tdomain *n);

manage_transport *manage_transport_open(const char *name, u_long flags,
											const u_char *chunk, const size_t chunk_size) ;


#endif
