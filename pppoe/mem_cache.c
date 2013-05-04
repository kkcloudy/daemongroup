
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "pppoe_def.h"
#include "pppoe_priv_def.h"

#include "pppoe_log.h"
#include "pppoe_list.h"

#include "mem_cache.h"

struct mem_item {
	void *priv;
	unsigned char data[0];
};

struct mem_blk {
	unsigned int free_num;
	void *frist, *free, *recover;
	struct list_head next;
	unsigned char data[0];
};

struct mem_cache {
	unsigned int max_empty;
	unsigned int max_blk;

	unsigned int blk_num;
	unsigned int empty_num;
		
	unsigned int item_size;
	unsigned int item_num;
	
	struct list_head full_head;
	struct list_head partial_head;
	struct list_head empty_head;

	char name[CACHE_MAX_NAMELEN];
};

static struct list_head cache_head = LIST_HEAD_INIT(cache_head);


static inline struct mem_blk *
__get_free_item_blk(mem_cache_t *cache) {
	if (!list_empty(&cache->partial_head)) {
		return list_entry(cache->partial_head.next, struct mem_blk, next);
	}

	if (!list_empty(&cache->empty_head)) {
		return list_entry(cache->empty_head.next, struct mem_blk, next);
	}

	return NULL;
}

static inline void *
__blk_alloc_item(struct mem_blk *blk, unsigned int item_size) {
	struct mem_item *item;
	
	if (blk->recover) {
		item = blk->recover;
		blk->recover = item->priv;
		goto out;
	}

	item = blk->free;
	blk->free = (void *)item + item_size;

out:
	item->priv = blk;
	blk->free_num--;
	return (void *)(item->data);
}

static inline void
__blk_free_item(struct mem_blk *blk, struct mem_item *item) {
	item->priv = blk->recover;
	blk->recover = item;
	blk->free_num++;
}


static inline struct mem_blk *
mem_blk_create(unsigned int item_size, unsigned int item_num) {
	struct mem_blk *blk;
	unsigned int blk_mem_size 
		= sizeof(struct mem_blk) + item_size * (item_num + 1);
	
	blk = (struct mem_blk *)malloc(blk_mem_size);
	if (unlikely(!blk)) 
		return NULL;

	memset(blk, 0, sizeof (struct mem_blk));
	blk->free_num = item_num;
	blk->frist = blk->free = (struct mem_item *)(blk->data - 
				((unsigned int)(&blk->data) & (item_size - 1)) + item_size);	/* mem blk struct Alignment */

	return blk;
}

static inline void
mem_blk_destroy(struct mem_blk **blk) {
	PPPOE_FREE(*blk);
}

void *
mem_cache_alloc(mem_cache_t *cache) {
	struct mem_blk *blk;

	if (unlikely(!cache))
		return NULL;
	
	blk = __get_free_item_blk(cache);
	if (!blk) {		
		if (cache->max_blk == cache->blk_num)
			return NULL;

		blk = mem_blk_create(cache->item_size, cache->item_num);
		if (unlikely(!blk))
			return NULL;
		
		if (1 == blk->free_num) {
			list_add(&blk->next, &cache->full_head);
		} else{
			list_add(&blk->next, &cache->partial_head);
		}

		cache->blk_num++;
		goto out;
	}

	if (blk->free_num == cache->item_num) {
		list_del(&blk->next);	
		cache->empty_num--;
		
		if (1 == blk->free_num) {
			list_add(&blk->next, &cache->full_head);
		} else{
			list_add(&blk->next, &cache->partial_head);
		}
		goto out;
	} 

	if (1 == blk->free_num) {
		list_del(&blk->next);
		list_add(&blk->next, &cache->full_head);
	}

out:
	return __blk_alloc_item(blk, cache->item_size);	
}

void
mem_cache_free(mem_cache_t *cache, void *data) {
	struct mem_item *item;
	struct mem_blk *blk;
	
	if (unlikely(!cache || !data))
		return;

	item = data - sizeof(struct mem_item);
	blk = item->priv;
	
	__blk_free_item(blk, item);
	if (blk->free_num == cache->item_num) {
		list_del(&blk->next);
		
		if (cache->empty_num == cache->max_empty) {
			cache->blk_num--;
			mem_blk_destroy(&blk);
			return ;
		}

		blk->free_num = cache->item_num;
		blk->free = blk->frist;
		blk->recover = NULL;
		
		list_add(&blk->next, &cache->empty_head);
		cache->empty_num++;
	} else if (1 == blk->free_num) {
		list_del(&blk->next);
		list_add(&blk->next, &cache->partial_head);
	}
}
	
mem_cache_t *
mem_cache_create(char *name, 
			unsigned int max_blk, unsigned int max_empty, 
			unsigned int item_size, unsigned int item_num) {
	mem_cache_t *cache;
	int i;	

	if (unlikely(!name || max_blk > CACHE_MAX_BLKNUM 
		|| (max_blk && max_empty > max_blk)))
		return NULL;

	cache = (struct mem_cache *)malloc(sizeof(struct mem_cache));
	if (unlikely(!cache)) {
		pppoe_log(LOG_WARNING, "mem_cache_create: malloc cache fail\n");
		return NULL;
	}
	
	memset(cache, 0, sizeof(struct mem_cache));
	strncpy(cache->name, name, sizeof(cache->name) - 1);
	cache->item_size = item_size + sizeof(struct mem_item);
	cache->item_num = item_num ? : CACHE_DEFAULT_ITEMNUM;;
	cache->max_blk = max_blk ? : CACHE_MAX_BLKNUM;
	cache->max_empty = max_empty;

	INIT_LIST_HEAD(&cache->full_head);
	INIT_LIST_HEAD(&cache->partial_head);
	INIT_LIST_HEAD(&cache->empty_head);

	for (i = 0; i < cache->max_empty; i++) {
		struct mem_blk *blk
			= mem_blk_create(cache->item_size, cache->item_num);
		if (likely(blk)) {
			list_add_tail(&blk->next, &cache->empty_head);
			cache->blk_num++;
			cache->empty_num++;
		}
	}

	return cache;
}

int
mem_cache_destroy(mem_cache_t **cache) {
	struct list_head *pos, *n;

	if (!cache || !(*cache))
		return PPPOEERR_EINVAL;

	if (!list_empty(&(*cache)->full_head)
		|| !list_empty(&(*cache)->partial_head)) {
		pppoe_log(LOG_ERR, "mem cache %s is not cleaned\n", (*cache)->name);
		return PPPOEERR_EINVAL;
	}

	list_for_each_safe(pos, n, &(*cache)->empty_head) {
		struct mem_blk *blk
			= list_entry(pos, struct mem_blk, next);
		mem_blk_destroy(&blk);
	}
	
	PPPOE_FREE(*cache);
	return PPPOEERR_SUCCESS;
}



#if 0
#include <sys/time.h>

int 
main(int argc, char **argv) {
	struct test_cache {
		unsigned char mac[6];
		unsigned int ipaddr;
		char ifname[15];
		unsigned int sid;
	};

	int i;
	struct timeval tv1, tv2;
	struct mem_cache *cache;
	struct test_cache **entry;
	unsigned int entry_num = 20000;

	entry = (struct test_cache **)calloc(entry_num + 1, sizeof(struct test_cache *));
	if (!entry) {
		printf("entry cache create fail\n");
		return -1;
	}

	cache = mem_cache_create("test_name", 0, 6, sizeof(struct test_cache), 512);
	if (!cache) {
		printf("mem cache create fail\n");
		return -1;
	}
	printf("blk item is %d\n", cache->item_num);


	gettimeofday(&tv1, NULL);

	int times = 0;
	for (; times < 10; times++) {
		for (i = 0; i < entry_num ; i++) {
			entry[i] = malloc(sizeof(struct test_cache));
	//		entry[i] = mem_cache_alloc(cache);
			memset(entry[i], 0, sizeof(struct test_cache));
			snprintf(entry[i]->ifname, sizeof(struct test_cache), "entry %u", i);
		}
		for (i = entry_num - 1; i >= 0 ; i--) {
			free(entry[i]);
//			printf("free %s\n", entry[i]->ifname);
//			mem_cache_free(cache, entry[i]);
		}
	}

	gettimeofday(&tv2, NULL);
	printf("aaaaaaause time is %lu\n", (tv2.tv_sec - tv1.tv_sec) * 1000000 + (tv2.tv_usec - tv1.tv_usec));

	printf("cache->empty_num = %d\n", cache->empty_num);
	printf("cache->max_blk = %u, cache->blk_num = %u\n", cache->max_blk, cache->blk_num);
	mem_cache_destroy(&cache);

	printf("end test\n");
	
	return 0;
}

#endif
