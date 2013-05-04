#ifndef _PPPOE_MEM_CACHE_H
#define _PPPOE_MEM_CACHE_H

#define CACHE_MAX_NAMELEN			32
#define CACHE_MAX_BLKNUM			128
#define CACHE_DEFAULT_ITEMNUM		512


typedef struct mem_cache mem_cache_t;

void *mem_cache_alloc(mem_cache_t *cache);
void mem_cache_free(mem_cache_t *cache, void *data);

mem_cache_t *mem_cache_create(char *name, 
			unsigned int max_blk, unsigned int max_empty, 
			unsigned int item_size, unsigned int item_num);
int mem_cache_destroy(mem_cache_t **cache);


#endif
