/*******************************************************************************
Copyright (C) Autelan Technology


This software file is owned and distributed by Autelan Technology 
********************************************************************************


THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND 
ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED 
WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE 
DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE LIABLE FOR 
ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES 
(INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; 
LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON 
ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT 
(INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS 
SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
********************************************************************************
*	RCSfile:	hashtable.c
*
*	Author: 	shaojunwu
*
*	Revision:	1.00
*
*	Date:	2009-10-22 19:58:07
*
*******************************************************************************/
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <stdarg.h>
#include "trap-util.h"
#include "nm_list.h"
#include "trap-def.h"
#include "trap-list.h"
#include "hashtable.h"
#include "trap-descr.h"
#include "trap-data.h"	//need #include "trap-descr.h"
//#include "trap-resend.h"  //need #include "trap-data.h"


#define _HASHTABLE_LOCAL_DEBUG 0 /*it must be set to 0 before put to cvs!!!!!!!*/

#if _HASHTABLE_LOCAL_DEBUG

	#include <stdio.h>
	
	#define debug_hashtable 	printf
	#define error_hashtable 	printf
#else
	#include <syslog.h>
	#include <stdarg.h>
	#include <time.h>
	#include "trap-util.h"
	#define debug_hashtable(format,args...)		TRAP_TRACE_LOG(LOG_DEBUG,format,##args)
	#define error_hashtable(format,args...)		TRAP_TRACE_LOG(LOG_DEBUG,format,##args)
#endif

extern unsigned int hash_value_count(const char *buff, unsigned int len,  unsigned int hash_size)
{
	if (NULL == buff || 0 == len || 0 == hash_size)
	{
		error_hashtable("create_hash_table  error input!\n");		
		return 0;
	}
	debug_hashtable("key is (%s)",buff); 
	unsigned int uint32_hash_value = 0;
		
	while (4 <= len)
	{
		uint32_hash_value += *((unsigned int*)buff);
		buff += 4;
		len -= 4;
	}

	while (0 < len)
	{
		uint32_hash_value += (*buff) << ((len-1)*8);
		buff++;
		len--;
	}

	debug_hashtable("value is (%u) hash_value is (%d)\n", uint32_hash_value, uint32_hash_value%hash_size);

	return uint32_hash_value%hash_size;
}

extern int hashtable_create_table( hashtable **ht, unsigned int hash_num )
{
	hashtable *table=NULL;
	unsigned int size = 0;
	unsigned int i = 0;
	
	if (0 == hash_num || NULL == ht)
	{
		error_hashtable("create_hash_table  error input!\n");
		return TRAP_ERROR;	
	}

	size = sizeof(hashtable) + sizeof(struct hlist_head)*hash_num;
	
	table = (hashtable *)TRAP_MALLOC(size);
	if( NULL == table)
	{
		error_hashtable( "appconn_db_create malloc error!\n" );
		return TRAP_ERROR;	
	}
	
	memset(table, 0, sizeof(hashtable));
	
	table->hash_num = hash_num;	
	
	for (i=0; i<hash_num; i++)
	{
		INIT_HLIST_HEAD(&(table->head_nodes[i]));
	}
		
	*ht = table;
	return TRAP_OK;		
}

extern int hashtable_destroy_table( hashtable **ht )
{	
	if( NULL == ht || NULL == *ht )
	{
		error_hashtable("hashtable_destroy_table input error!\n" );
		return TRAP_ERROR;	
	}
	
	/* just free the table!!! the nodes are still there!!!*/
	eag_free(*ht);
	
	*ht = NULL;
	
	return TRAP_OK;
}

extern int hashtable_add_node(hashtable *ht, const void *key, unsigned int len, struct hlist_node *node)
{	
	if( NULL == ht || NULL == key || 0 == len || NULL == node )
	{
		error_hashtable( "hashtable_add_obj input error!\n" );
		return TRAP_ERROR;
	}

	unsigned int hash_value = 0;

	if ( NULL == ht->hash_value_count_func ){
		error_hashtable("hash_value_count_func is null erro!\n");
		return TRAP_ERROR;
	}
	
	hash_value = ht->hash_value_count_func(key, len, ht->hash_num);
		
	hlist_add_head(node, &(ht->head_nodes[hash_value]));
	
	return TRAP_OK;	
}

extern int hashtable_check_add_node(hashtable *ht, const void *key, unsigned int len, struct hlist_node *node)
{	
	if( NULL == ht || NULL == key || 0 == len || NULL == node )
	{
		error_hashtable("hashtable_add_obj input error!\n" );
		return TRAP_ERROR;
	}

	unsigned int hash_value = 0;
	int ret = 0;
	
	hash_value = hash_value_count(key, len, ht->hash_num);
	
	/* check is it in the list */
	ret = is_entry_in_hlist(node, &(ht->head_nodes[hash_value]));
	if (1 == ret)
	{
		error_hashtable("hashtable_add_obj error! node allready in it\n" );		
		return TRAP_ERROR;
	}
	hlist_add_head(node, &(ht->head_nodes[hash_value]));
	
	return TRAP_OK;	
}

extern int hashtable_del_node(struct hlist_node *node)
{
	if (NULL == node )
	{
		error_hashtable( "hashtable_del_node input error!\n" );
		return TRAP_ERROR;
	}

	unsigned int hash_value = 0;
			
	hlist_del(node);
	
	return TRAP_OK;	
}

extern struct hlist_head * hashtable_get_hash_list(hashtable *ht, const void *key, unsigned int len)
{
	if( NULL == ht || NULL == key || 0 == len)
	{
		error_hashtable( "hashtable_get_hash_list input error!\n" );
		return NULL;
	}

	unsigned int hash_value = 0;
	hash_value=ht->hash_value_count_func(key, strlen(key), ht->hash_num);
		
	return &(ht->head_nodes[hash_value]);		
}

/********************************************************
*method:   main 
*usage:    gcc hashtable.c dblnklst.c -o hashtest
*param:    
*return:   
*author:   shaojunwu
*create time:   	2009-10-23 9:44:26
*last changed:(include change contant, author, time!!!!)
*
*********************************************************/


#if _HASHTABLE_LOCAL_DEBUG
#include <mcheck.h> 


struct element_tst_t{
	int i;
	char buff[128];
};

#define ELE_NUM 	512

int main()
{
	hashtable *table1=NULL;
	dblnklst *lsttest=NULL;
	int i;
	struct element_tst_t *eles[ELE_NUM];
	
	struct element_tst_t *ele_get=NULL;
	dblnklst_iterator itr;
	dblnklst_node *node;
	
	mtrace();
	
	memset( eles, 0, sizeof( eles ));

	if( 0 != hashtable_create(&table1, 512) )
	{
		printf("create hash_table_err! table1\n");	
		return -1;
	}
	
	if( 0 != dblnklst_create( &lsttest ) )
	{
		
		hashtable_destroy(&table1);
		return -1;	
	}

	printf("create hashtable =%p\n", table1 );
	/*add element*/
	for( i = 0; i<ELE_NUM; i++ )
	{
		eles[i] = (struct element_tst_t *)eag_malloc(sizeof(struct element_tst_t));
		if( NULL == eles[i] )
		{
			printf("eag_malloc element error!\n");
			return -1;
		}
		memset( eles[i], 0, sizeof(struct element_tst_t) );
		eles[i]->i = i;
		memset( eles[i]->buff, 0, sizeof(eles[i]->buff));
		memset( eles[i]->buff, i%10+'0', i%5+5 );
		hashtable_add_obj( table1, eles[i]->buff, strlen(eles[i]->buff) , eles[i] );
		dblnklst_add_obj( lsttest, eles[i] );
	}
	
	/*find element*/
	hashtable_del_obj( table1, eles[88]->buff, strlen(eles[88]->buff), eles[88] );
	hashtable_del_obj( table1, eles[108]->buff, strlen(eles[108]->buff), eles[108] );
	hashtable_del_obj( table1, eles[218]->buff, strlen(eles[218]->buff), eles[218] );
	hashtable_del_obj( table1, eles[58]->buff, strlen(eles[58]->buff), eles[58] );
	itr = hashtable_find_key( table1, eles[88]->buff, strlen(eles[88]->buff) );
	if( NULL == itr )
	{
		printf("not find %s   it's error!", eles[88]->buff );
	}
	else
	{
		for( node=DBLNKLST_ITR_FIRST(itr); !DBLNKLST_ITR_ISEND(itr); node=DBLNKLST_ITR_NEXT(itr) )
		{
			ele_get=node->obj;
			printf("ele_get = %p  eles[%d].buff = %s\n", ele_get, ele_get->i, ele_get->buff );
		}
		DBLNKLST_ITR_DESTROY(itr);/*itr is create by hashtable_find_key, it should destroy by DBLNKLST_ITR_DESTROY, otherwise no need to destroy*/
	}
	
	hashtable_destroy( &table1 );
	itr = DBLNKLST_GET_ITR(lsttest);
	for( node=DBLNKLST_ITR_FIRST(itr); !DBLNKLST_ITR_ISEND(itr); node=DBLNKLST_ITR_NEXT(itr) )
	{
		ele_get=node->obj;
		printf("ele_get = %p  eles[%d].buff = %s\n", ele_get, ele_get->i, ele_get->buff );
	}
	dblnklst_destroy( &lsttest );
	for( i = 0; i<ELE_NUM; i++ )
	{
		eag_free( eles[i] );
		eles[i] = NULL;
	}
	muntrace();
	return 0;	
}

#endif
