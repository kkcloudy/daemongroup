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
* capture.c
*
*
* CREATOR:
* autelan.software.Network Dep. team
*
* DESCRIPTION:
*
*
*
*******************************************************************************/

/*
* Copyright (c) 2008,Autelan - AuteCS
* All rights reserved.
*
*$Source: /rdoc/accapi/nm/public/ws_list_container.h,v $
*$Author: shaojunwu $
*$Date: 2010/05/31 11:51:50 $
*$Revision: 1.1 $
*$State: Exp $
*$Modify:	$
*$Log $
*
*/
/*
list  container 是一个公共的容器对象，其封装了所有的链表操作。

*/
#ifndef _LIST_CONTAINER_H
#define _LIST_CONTAINER_H


#define __DEBUG	0	//上传cvs时必须为0

#if __DEBUG
//多参数宏的定义,用的时候不是很多,但有时候很有用.
#define debugprint(...) printf("%s  %d   ",__FILE__,__LINE__);printf(stderr,__VA_ARGS__)
#else
#define debugprint(...)
#endif


/******************************
定义基类的数据类型，一个container，一个element
在调用的地方,统一使用 LEBase  LCBase 这两个类型定义
*******************************/
typedef struct element_base_t {
	struct element_base_t *next;
	int (*release_self)( struct element_base_t *this );//用于释放派生出的对象中malloc的空间.同时要释放this指针
}list_element_base, LEBase;
//定义LEBase和LCBase是为了方便使用,要求在派生类中引用该结构时必须使用这个定义.

typedef int (*REL_ELE_SELF_FUNC )( struct element_base_t *this );


typedef struct list_container_t {
	list_element_base ele_base;//当需要将container作为一个element，放在另一个container中时需要使用。用于二维链表。通常的不用这个成员。
	list_element_base *head;
	int (*release_self)( struct list_container_t *this );//这个函数只需要释放派生类中分配的空间,基类中的空间以及链表都不由他来释放.同时要求释放this指针
	int (*element_compare)( list_element_base *p1, list_element_base *p2 );//用于排序，或者查找。就目前我们的系统而言，大多数用不着。
	int (*proc_element)( list_element_base *this );//这里处理的是container中的每一个元素,所以参数传递的是element的指针.
}list_container_base, LCBase;

typedef int (*RELS_CONT_SELF_FUNC)( list_container_base *this );
typedef int (*ELE_CMP_FUNC)( list_element_base *p1, list_element_base *p2 );
typedef int (*PROC_ALL_ELE_FUNC)( list_element_base* this );




/****************************************
定义错误号,外部定义错误号时，从LC_ERR_EXTEND_BASE开始
*****************************************/
#define RTN_OK			0
#define LC_ERR_NONE		-1
#define LC_ERR_MALLOC_FAILED	-2
#define LC_ERR_POINTER_NULL	-3
#define LC_ERR_NO_THIS_ELE	-4
#define LC_ERR_CB_NULL		-5
#define LC_ERR_THIS_IDX_NULL	-6
#define LC_ERR_INDEX		-7

#define LC_ERR_EXTEND_BASE	-1000


/*************************************
创建一个container  type是     派生类    的类型，初始化完成后,head指针指向null.
**********************************/
#define NEW_CONTAINER(_p,_type,_release)	{\
	_p=(_type*)malloc(sizeof(_type));\
	if( NULL != _p )\
	{\
		list_container_base *p_tmp=(list_container_base *)_p;\
		memset( p_tmp,0,sizeof(_type));\
		p_tmp->release_self = (RELS_CONT_SELF_FUNC)_release;\
	}\
}

/***********************************
释放一个container，
包括其中的所有元素
************************************/
#define DELETE_CONTAINER(p)	{\
	if( NULL != p )\
	{\
		list_container_base *p_container_base;\
		list_element_base *p_head,*p_temp;\
		p_container_base = (list_container_base *)p;\
		p_head = p_container_base->head;\
		/*debugprint("p_head = %x\n", p_head );*/\
		while( NULL != p_head )\
		{\
			/*debugprint("relese element in container p_head=%x\n", p_head);*/\
			p_temp = p_head;\
			p_head = p_head->next;\
			if( NULL != p_temp->release_self )\
			{\
				p_temp->release_self( p_temp );\
			}\
			else\
			{\
				free( p_temp );\
			}\
			p_temp = NULL;\
		}\
		if( NULL != p_container_base->release_self)\
		{\
			p_container_base->release_self( p_container_base );\
		}\
		else\
		{\
			free( p );\
		}\
	}\
}
	

/*************************************
创建一个element,所有的派生对象都用这个宏来创建
************************************/
#define NEW_ELEMENT(_p,_type,_release)	{\
	_p = (_type*)malloc(sizeof(_type));\
	if( NULL != _p )\
	{\
		list_element_base *p_tmp = (list_element_base *)_p;\
		memset( p_tmp, 0, sizeof(_type));\
		p_tmp->release_self = (REL_ELE_SELF_FUNC)_release;\
	}\
}
	
/**********************************
删除一个element,
************************************/
#define DELETE_ELEMENT(_p) {\
	if( NULL != _p )\
	{\
		list_element_base *p_e_base;\
		p_e_base = (list_element_base *)_p;\
		if( NULL !=  p_e_base->release_self )\
		{\
			p_e_base->release_self(p_e_base);\
		}\
		else\
		{\
			free(p_e_base);\
		}\
	}\
}







/************************************
添加一个元素
p_c   : container 的实例
p_e   ： element的实例
index ：     添加到的位置，如果为0，放在第一个，如果小于0，放在最后，如果index超过了当前的最大个数，放在最后。
***********************************/
int add_element_f( list_container_base *p_c, list_element_base *p_e, int idx );
//做下面这样的宏定义是为了方便大家调用,没必要在每个地方都进行强制类型转换了.
#define add_element(p_c,p_e,idx)	add_element_f( (list_container_base *)p_c, (list_element_base *)p_e,idx )


/************************************
添加一个元素,按照顺序插入到链表，从小到大，你需要先设置element_compare 函数。
p_c   : container 的实例
p_e   ： element的实例
index ：     添加到的位置，如果为0，放在第一个，如果小于0，放在最后，如果index超过了当前的最大个数，放在最后。
***********************************/
int add_element_by_cmp_f( list_container_base *p_c, list_element_base *p_e );
#define add_element_by_cmp( p_c, p_e )	add_element_by_cmp_f( (list_container_base *)p_c, (list_element_base *)p_e )

/*********************************
根据节点的指针 删除一个节点, 如果制定节点的指针不存在，就返回<0  删除成功返回0
***********************************/
int del_element_f( list_container_base *p_c, list_element_base *p_e );
#define del_element( p_c,p_e )		del_element_f( (list_container_base*)p_c, (list_element_base *)p_e )




/********************************
查找一个节点,如果是派生类，需要对输入的参数进行强制类型转换,返回索引号，如果返回小于0，表示没有找到
pp_get得到
*******************************/
int search_element_f( list_container_base *p_c, list_element_base *p_e_search, list_element_base **pp_e_get );
#define search_element(p_c,p_e_sharch,pp_e_get)	search_element_f((list_container_base *)p_c,(list_element_base *)p_e_search, (list_element_base **)pp_e_get )
/**********************************
得到container中元素的个数
**********************************/
int get_element_num_f( list_container_base *p_c );
#define get_element_num(p_c)	get_element_num_f((list_container_base *)p_c)

/**********************************
根据元素指针，得到索引号。
***********************************/
int get_idx_of_element_f( list_container_base *p_c, list_element_base *p_e );
#define get_idx_of_element(p_c,p_e)	get_idx_of_element_f((list_container_base *)p_c,(list_element_base *)p_e)



/*********************************
设置元素的比较函数.
************************************/
int set_element_cmp_func_f( list_container_base *p_c, ELE_CMP_FUNC ele_cmp_func );
#define set_element_cmp_func(p_c,ele_cmp_func)	set_element_cmp_func_f((list_container_base *)p_c,(ELE_CMP_FUNC)ele_cmp_func)


/*********************************************************************
设置处理所有container中每个元素的函数回调函数
********************************************************************/
int set_proc_all_func_f( list_container_base *p_c, PROC_ALL_ELE_FUNC proc_all_ele_func );
#define set_proc_all_func( p_c, func )	set_proc_all_func_f( (list_container_base *)p_c, (PROC_ALL_ELE_FUNC)func )


/***************************************************
处理所有的元素, 调用的处理函数是 set_proc_all_func设置的 回调函数.
******************************************************/
int proc_all_ele_in_cont_f( list_container_base *p_c );
#define proc_all_ele_in_cont( p_c )	proc_all_ele_in_cont_f( (list_container_base *)p_c )


/*****************************************************
处理按照key进行比较，相等的对象才调用处理函数
******************************************************/
int proc_eles_in_cont_by_key_f( list_container_base *p_c, list_element_base *p_e_key );
#define proc_eles_in_cont_by_key(p_c,p_e_key)	proc_eles_in_cont_by_key_f((list_container_base *)p_c,(list_element_base *)p_e_key )

/******************************************************
对所有元素进行排序。
********************************************************/
int sort_elements_f( list_container_base *p_c );
#define sort_elements(p_c)	sort_elements_f( (list_container_base *)p_c )


/******************************************
得到下一个元素
因为在派生新的类时，没有next指针，next指针是在基类中定义的，所以有个专用函数来获得下一个。
所以在实际操作的时候，都不要用next来取得下一个，统一使用这个函数.
在调用这个函数,将返回值赋值给一个派生类的指针变量时,会有warning,需要强制转换成派生类的类型,以去除warning
*******************************************/

int get_next_element_f( list_element_base *p_e_cur, list_element_base **pp_e_base_get );
#define get_next_element(p_e_cur,pp_e_base_get)	get_next_element_f((list_element_base *)p_e_cur,(list_element_base **)pp_e_base_get)

/*****************************
得到最后一个元素
使用注意事项: 如上
********************************/
int get_last_element_f( list_container_base *p_c, list_element_base **pp_e_base_get );
#define get_last_element(p_c,pp_e_base_get)	get_last_element_f((list_container_base *)p_c,(list_element_base **)pp_e_base_get)

/***********************************
根据index，得到一个节点,如果index < 0或者超过范围返回NULL
************************************/
int get_element_by_idx_f( list_container_base *p_c, int idx, list_element_base **pp_e_base_get );
#define get_element_by_idx(p_c,idx,pp_e_base_get)	get_element_by_idx_f((list_container_base *)p_c,idx,(list_element_base **)pp_e_base_get);




#endif
