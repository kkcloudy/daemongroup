#ifndef _PPPOE_BUF_H
#define _PPPOE_BUF_H

#define DEFAULT_BUF_SIZE	4096
#define PBUF_FREE(pbuf) { if (pbuf) { pbuf_free((pbuf)); (pbuf) = NULL; } }

struct pppoe_buf {
	uint32 len;
	unsigned char *head, *data, *tail, *end;
};


static inline unsigned int 
pbuf_headroom(const struct pppoe_buf *pbuf) {
	return pbuf->data - pbuf->head;
}

static inline int 
pbuf_tailroom(const struct pppoe_buf *pbuf){
	return pbuf->end - pbuf->tail;
}

static inline struct pppoe_buf *
pbuf_init(struct pppoe_buf *pbuf) {
	pbuf->data = pbuf->tail = pbuf->head;
	pbuf->len = 0;
	return pbuf;
}

static inline unsigned char * 
pbuf_reserve(struct pppoe_buf *pbuf, uint32 len) {
	pbuf->data += len;
	pbuf->tail += len;
	return pbuf->data;
}


static inline unsigned char *
pbuf_put(struct pppoe_buf *pbuf, uint32 len) {
	unsigned char *tmp = pbuf->tail;
	pbuf->tail += len;
	pbuf->len += len;
	return tmp;
}

static inline unsigned char *
pbuf_push(struct pppoe_buf *pbuf, uint32 len) {
	pbuf->data -= len;
	pbuf->len  += len;
	return pbuf->data;
}

static inline int
pbuf_trim(struct pppoe_buf *pbuf, uint32 len) {
	if (unlikely(len > pbuf->len)) 
		return PPPOEERR_ELENGTH;

	pbuf->len = len;
	pbuf->tail = pbuf->data + len;
	return PPPOEERR_SUCCESS;
}

static inline unsigned char *
pbuf_pull(struct pppoe_buf *pbuf, uint32 len) {
	pbuf->len -= len;
	return pbuf->data += len;
}

static inline int 
pbuf_may_pull(struct pppoe_buf *pbuf, uint32 len) {
	if (likely(len <= pbuf->len))
		return PPPOEERR_SUCCESS;

	return PPPOEERR_EINVAL;
}

/* max size if oxffff */
struct pppoe_buf *pbuf_alloc(uint32 size) ;
void pbuf_free(struct pppoe_buf *pbuf);


#endif

