#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "pppoe_def.h"
#include "pppoe_priv_def.h"

#include "pppoe_buf.h"


/* max size if oxffff */
struct pppoe_buf *
pbuf_alloc(uint32 size) {
	struct pppoe_buf *pbuf;

	if (size > 0xffff)
		return NULL;

	pbuf = (struct pppoe_buf *)malloc(sizeof(struct pppoe_buf ) + size);
	if (!pbuf)
		return NULL;
	
	pbuf->head = pbuf->data = pbuf->tail = (unsigned char *)pbuf + sizeof(struct pppoe_buf);
	pbuf->end = pbuf->head + size;
	pbuf->len = 0;

	return pbuf;
}

void
pbuf_free(struct pppoe_buf *pbuf) {
	PPPOE_FREE(pbuf);
}

