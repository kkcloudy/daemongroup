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
*$Source: /rdoc/AuteCS/cgic205/ws_list_container.c,v $
*$Author: chensheng $
*$Date: 2010/02/22 06:48:37 $
*$Revision: 1.3 $
*$State: Exp $
*$Modify:	$
*$Log $
*
*/
#include "ws_list_container.h"
#include "string.h"
#include "stdlib.h"
#include "stdio.h"



/***************************************************************
*USEAGE:	add an element to container.
*Param: 	p_c->a pointer to a container. 可以是指向一个list_container_base派生类的指针
		p_e->a pointer to a element. 可以是指向一个list_element_base的派生类的指针
		idx->position that you want to insert.如果该值为负数,则放在链表末尾,如果该idx超过了当前总个数,则返回失败.0表示放在第一个.
*Return:	0 ->  success
		other -> error
*Auther:	shao jun wu
*Date:		2009-3-9 14:15
*Modify:	(include modifyer,for what resease,date)
****************************************************************/
int add_element_f( list_container_base *p_c, list_element_base *p_e, int idx )
{
	list_element_base *p_e_insert_prev;
	
	if( NULL == p_c || NULL == p_e )
	{
		//debugprint("p_c or p_e is NULL!\n");
		return LC_ERR_POINTER_NULL;	
	}
	
	if( NULL == p_c->head )
	{
		p_c->head = p_e;
	}
	else if( 0 == idx  )
	{
		p_e->next = p_c->head;
		p_c->head = p_e;
	}
	else
	{
		if( idx < 0 )
		{
			get_last_element( p_c, &p_e_insert_prev );
		}
		else
		{
			get_element_by_idx( p_c, idx-1, &p_e_insert_prev );

		}
		
		if( NULL == p_e_insert_prev )//没有找到当前索引,返回错误.
		{
			return LC_ERR_THIS_IDX_NULL;
		}		
		
		p_e->next = p_e_insert_prev->next;
		p_e_insert_prev->next = p_e;
	}
	//debugprint("add item ok p_e = %x\n", p_e);
	return RTN_OK;
}


/***************************************************************
*USEAGE:	add an element to container.by cmp.按照从element_compare函数的返回小到大的顺序将元素插入到链表。
*Param: 	p_c->a pointer to a container. 可以是指向一个list_container_base派生类的指针。
		p_e->a pointer to a element. 可以是指向一个list_element_base的派生类的指针。
		idx->position that you want to insert.如果该值为负数,则放在链表末尾,如果该idx超过了当前总个数,则返回失败.0表示放在第一个。
*Return:	0 ->  success
		other -> error
*Auther:	shao jun wu
*Date:		2009-3-9 16:51
*Modify:	(include modifyer,for what resease,date)
****************************************************************/
int add_element_by_cmp_f( list_container_base *p_c, list_element_base *p_e )
{
	list_element_base *p_e_insert_prev;

	if( NULL == p_c || NULL == p_e )
	{
		//debugprint("p_c or p_e is NULL!\n");
		return LC_ERR_POINTER_NULL;	
	}
	
	if( NULL == p_c->element_compare )
	{
		return LC_ERR_CB_NULL;
	}
	
	if( NULL == p_c->head )
	{
		p_c->head = p_e;
	}
	else if( p_c->element_compare( p_e , p_c->head ) < 0 )//插入到head
	{
		p_e->next = p_c->head;
		p_c->head = p_e;
	}
	else
	{
		for( p_e_insert_prev = p_c->head; p_e_insert_prev->next; p_e_insert_prev = p_e_insert_prev->next )
		{
			if( p_c->element_compare( p_e_insert_prev, p_e ) <= 0 && 
				p_c->element_compare( p_e_insert_prev->next, p_e ) > 0 )
			{
				break;
			}			
		}
		p_e->next = p_e_insert_prev->next;
		p_e_insert_prev->next = p_e;		
	}
	return RTN_OK;
}


/***************************************************************
*USEAGE:	delete an element from container. you must no the element's pointer.
*Param: 	p_c->a pointer to a container. 可以是指向一个list_container_base派生类的指针
		p_e->a pointer to a element. 可以是指向一个list_element_base的派生类的指针,如果这个元素还没有添加到container中则返回LC_ERR_NO_THIS_ELE.
*Return:	0 ->  success
		other -> error
*Auther:	shao jun wu
*Date:		2009-3-9 14:20
*Modify:	(include modifyer,for what resease,date)
****************************************************************/
int del_element_f( list_container_base *p_c, list_element_base *p_e )
{
	list_element_base *p_e_prev;
	list_element_base *p_e_for_free;
	
	if( NULL == p_c || NULL == p_e )
	{
		//debugprint("p_c or p_e is NULL!\n");
		return LC_ERR_POINTER_NULL;	
	}
	
	for( p_e_prev = p_c->head; (p_e_prev != NULL)&&(p_e_prev->next!=p_e); p_e_prev = p_e_prev->next);
	
	if( NULL==p_e_prev )
	{
		//debugprint("this element is no in container! \n");
		return LC_ERR_NO_THIS_ELE;	
	}
	
	//先将element从container中断开,再释放指针
	p_e_for_free = p_e_prev->next;
	p_e_prev->next = p_e_for_free->next;
	
	DELETE_ELEMENT( p_e_for_free );
	
	return RTN_OK;	
}

/***************************************************************
*USEAGE:	得到最后一个元素.外部不要使用。
*Param: 	p_c->a pointer to a container. 可以是指向一个list_container_base派生类的指针
*Return:	porinter to an element base， which is the last element in the container.
*Auther:	shao jun wu
*Date:		2009-3-9 14:23
*Modify:	(include modifyer,for what resease,date)
****************************************************************/
int get_last_element_f( list_container_base *p_c, list_element_base **pp_e_base_get )
{
	list_element_base *p_ret;
	if( NULL == p_c || NULL == pp_e_base_get )
	{
		return LC_ERR_POINTER_NULL;	
	}
		
	for( p_ret = p_c->head; (p_ret && p_ret->next!=NULL); p_ret=p_ret->next );
	
	*pp_e_base_get = p_ret;
	
	return RTN_OK;
}



/***************************************************************
*USEAGE:	根据索引号，得到一个元素。
*Param: 	p_c->a pointer to a container. 可以是指向一个list_container_base派生类的指针
		idx->the index that you want to get.if it is lager than the cont of the element, it will return NULL.
*Return:	porinter to an element base， which is the last element in the container.
*Auther:	shao jun wu
*Date:		2009-3-9 14:23
*Modify:	(include modifyer,for what resease,date)
****************************************************************/
int get_element_by_idx_f( list_container_base *p_c, int idx, list_element_base **pp_e_base_get  )
{
	int i;
	list_element_base *p_ret;
	
	if( NULL == p_c  || NULL == pp_e_base_get )
	{
		return LC_ERR_POINTER_NULL;
	}
	
	if( index < 0 )
	{
		return LC_ERR_INDEX;	
	}
			
	for( i=0,p_ret=p_c->head; (p_ret && i<idx); p_ret=p_ret->next,i++);
	
	*pp_e_base_get = p_ret;
	
	return RTN_OK;
}




/********************************
查找一个节点,如果是派生类，需要对输入的参数进行强制类型转换,返回索引号，如果返回小于0，表示没有找到
pp_get得到
注意,这里的p_e_search,可能并不是container中的元素,这个函数需要调用cmp函数来比较两个对象的大小.
//暂时不用,先不实现.
*******************************/
int search_element_f( list_container_base *p_c, list_element_base *p_e_search, list_element_base **pp_e_get )
{
	return 0;
}



/***************************************************************
*USEAGE:	得到container中元素的个数
*Param: 	p_c->a pointer to a container. 可以是指向一个list_container_base派生类的指针
*Return:	the num of the elements in the container
*Auther:	shao jun wu
*Date:		2009-3-9 14:27
*Modify:	(include modifyer,for what resease,date)
****************************************************************/
int get_element_num_f( list_container_base *p_c )
{
	int iret;
	list_element_base *p_e_cur;
	
	for(iret=0,p_e_cur=p_c->head; p_e_cur; iret++,p_e_cur=p_e_cur->next );
	
	return iret;
}

/***************************************************************
*USEAGE:	得到某个元素的索引值
*Param: 	p_c->a pointer to a container. 可以是指向一个   list_container_base派生类  的指针
		p_e->a pointer to an element. 可以是指向一个  list_element_base派生类 的指针
*Return:	the index of this element in the container
*Auther:	shao jun wu
*Date:		2009-3-9 14:27
*Modify:	(include modifyer,for what resease,date)
****************************************************************/
int get_idx_of_element_f( list_container_base *p_c, list_element_base *p_e )
{
	int iret;
	list_element_base *p_e_cur;
	
	for( iret=0,p_e_cur=p_c->head; p_e_cur!=p_e; iret++,p_e_cur=p_e_cur->next );
	
	return iret;
}


/***************************************************************
*USEAGE:	得到某个元素的下一个元素，不建议外部调用。
*Param: 	p_e_cur->a pointer to an element. 可以是指向一个  list_element_base派生类 的指针
*Return:	a pointer to an element.
*Auther:	shao jun wu
*Date:		2009-3-9 14:29
*Modify:	(include modifyer,for what resease,date)
****************************************************************/
int get_next_element_f( list_element_base *p_e_cur, list_element_base **pp_e_base_get )
{
	if( NULL == p_e_cur || NULL == pp_e_base_get )
	{
		return LC_ERR_POINTER_NULL;	
	}
	
	*pp_e_base_get = p_e_cur->next;
	return RTN_OK;
}

/***************************************************************
*USEAGE:	设置element比较的callback函数。当需要对container中的元素进行排序的时候，需要设置该变量。
*Param: 	p_c->a pointer to a container. 可以是指向一个   list_container_base派生类  的指针
		ele_cmp_func-> function pointer .ELE_CMP_FUNC define as:
				int (*ELE_CMP_FUNC)( list_element_base *p1, list_element_base *p2 );
				so you should define a function like this:
				int ELE_CMP_FUNC( list_element_base *p1, list_element_base *p2 );
				参数需要指定为 相应的派生类的指针。
				注意，这个callback函数的两个参数都是 element指针.
*Return:	a pointer to an element.
*Auther:	shao jun wu
*Date:		2009-3-9 14:29
*Modify:	(include modifyer,for what resease,date)
****************************************************************/
int set_element_cmp_func_f( list_container_base *p_c, ELE_CMP_FUNC ele_cmp_func )
{
	if( NULL == p_c || NULL == ele_cmp_func  )
	{
		//debugprint("p_c  or  ele_cmp_func is NULL!\n");
		return LC_ERR_POINTER_NULL;
	}
	
	p_c->element_compare = ele_cmp_func ;
	
	return RTN_OK;	
};


/***************************************************************
*USEAGE:	设置处理每个元素的callback函数。
*Param: 	p_c->a pointer to a container. 可以是指向一个   list_container_base派生类  的指针
		proc_all_ele_func-> function pointer .PROC_ALL_ELE_FUNC define as:
				typedef int (*PROC_ALL_ELE_FUNC)( list_element_base* this );
				so you should define a function like this:
				int PROC_ALL_ELE_FUNC( list_element_base* this );
				参数需要指定为   相应的派生类   的指针。
				注意，这个callback函数的参数是 element， 不是 container。
*Return:	0
*Auther:	shao jun wu
*Date:		2009-3-9 14:29
*Modify:	(include modifyer,for what resease,date)
****************************************************************/
int set_proc_all_func_f( list_container_base *p_c, PROC_ALL_ELE_FUNC proc_all_ele_func )
{
	if( NULL == p_c || NULL == proc_all_ele_func )
	{
		//debugprint( "p_c  or proc_all_ele_func is NULL!\n" );
		return LC_ERR_POINTER_NULL;
	}
	
	p_c->proc_element = proc_all_ele_func;
	
	return RTN_OK;
}


/***************************************************************
*USEAGE:	设置处理cotainer中每个元素，使用的是上一个函数指定的callback函数。 这个函数主要的作用就是将链表遍历的方法统一.
		如果没有设置 proc_all_func的回调函数，则会返回LC_ERR_CB_NULL错误。
*Param: 	p_c->a pointer to a container. 可以是指向一个   list_container_base派生类  的指针
*Return:	0 ->  success
		other -> error
*Auther:	shao jun wu
*Date:		2009-3-9 14:29
*Modify:	(include modifyer,for what resease,date)
****************************************************************/
int proc_all_ele_in_cont_f( list_container_base *p_c )
{
	list_element_base *p_e;
	int err=RTN_OK;
	
	if( NULL == p_c )
	{
		//debugprint( "p_c   is NULL!\n" );
		return 	LC_ERR_POINTER_NULL;
	}
	
	if( NULL == p_c->proc_element )
	{
		//debugprint( "didn't set call back func for    proc_element!\n" );
		return LC_ERR_CB_NULL;	
	}
	
	//遍历每个元素,并对每个元素都调用
	for( p_e = p_c->head; p_e; p_e=p_e->next )
	{
		err = p_c->proc_element( p_e );
		if( err != RTN_OK )
		{
			break;
		}
	}
	
	return err;
}


/***************************************************************
*USEAGE:	可以对链表中某些特殊的对象，才进行处理。
		这个函数会在遍历所有的element是，将element与p_e_key进行比较，如果比较返回的值为0，才会调用proc_all_func函数来处理对象。
		如果没有设置element_compare回调函数，则会返回LC_ERR_CB_NULL错误。
		如果没有设置 proc_element的回调函数，则会返回LC_ERR_CB_NULL错误。
*Param: 	p_c->a pointer to a container. 可以是指向一个   list_container_base派生类  的指针
*Return:	0 ->  success
		other -> error
*Auther:	shao jun wu
*Date:		2009-3-9 14:29
*Modify:	(include modifyer,for what resease,date)
****************************************************************/
int proc_eles_in_cont_by_key_f( list_container_base *p_c, list_element_base *p_e_key )
{
	list_element_base *p_e_cur;
	int err=RTN_OK;
	
	if( NULL == p_c )
	{
		//debugprint( "p_c   is NULL!\n" );
		return 	LC_ERR_POINTER_NULL;
	}
	
	if( NULL == p_c->proc_element || NULL == p_c->element_compare )
	{
		return LC_ERR_CB_NULL;
	}
	
	//遍历每个元素,并将每个元素与key进行比较，当返回的是0时，才调用proc函数
	for( p_e_cur = p_c->head; p_e_cur; p_e_cur=p_e_cur->next )
	{
		if( 0 != p_c->element_compare(p_e_cur,p_e_key) )
		{
			continue;
		}
		err = p_c->proc_element( p_e_cur );
		if( err != RTN_OK )
		{
			break;
		}
	}
	
	return err;		
}



/***************************************************************
*USEAGE:	对container中的元素进行排序
		如果没有设置element_compare回调函数，则会返回LC_ERR_CB_NULL错误。

*Param: 	p_c->a pointer to a container. 可以是指向一个   list_container_base派生类  的指针
*Return:	0 ->  success
		other -> error
*Auther:	shao jun wu
*Date:		2009-3-9 14:29
*Modify:	(include modifyer,for what resease,date)
****************************************************************/
int sort_elements_f( list_container_base *p_c )
{
	list_element_base *p_e_cur;
	list_element_base *p_head;
	int iret=RTN_OK;
	
	if( NULL == p_c )
	{
		return 	LC_ERR_POINTER_NULL;
	}
	
	if( NULL == p_c->element_compare )
	{
		return LC_ERR_CB_NULL;	
	}
	
	//排序
	//
	p_head = p_c->head;
	p_c->head = NULL;
	
	//每次将head从链表中取下来，放到container中，同时将head指向下一个,起始和每次循环要做的事情是一样的，
	p_e_cur=p_head;
	while( p_e_cur )
	{
		p_head=p_head->next;
		p_e_cur->next=NULL;//将当前的元素从原来的链表中断掉。
		iret = add_element_by_cmp( p_c, p_e_cur );
		if( iret != RTN_OK )
		{
			break;	
		}
		p_e_cur=p_head;
	}
	
	return iret;
}






























/*************************************************************************
下面用于该模块的单独调试,同时可以作为模块的一个使用说明!

重要,每个人都需要看懂是怎么用的, 尽可能理解其意义.

使用的基本流程:
1、定义：派生对象（在定义自己需要处理的数据结构体时，将相应的base类放在结构体的开头）。（为什么要放在开头？不放在开头行吗？）
2、定义：container和elemnet对象的release函数。（如果定义的结构体中没有需要释放的指针，也可以不定义）
3、定义：处理每个元素的函数指针。如果需要可以定义多个函数
4、创建：使用NEW_CONTAINER创建一个container对象。传入相应的release函数，如果没有就传递NULL
5、创建：使用NEW_ELEMENT创建element对象，根据需要创建多个，对需要赋值的地方进行赋值，并将元素添加到container中。
6、应用：调用proc_all_ele_in_cont函数，遍历所有对象。

7、应用：如果需要处理特殊的element对象，可以设置一个key，并设置cmp函数，将满足key的对象cmp时返回0，然后调用proc_eles_in_cont_by_key
8、应用：如果需要排序，可以设置cmp函数（这个函数中处理排序的关键字），排序时，会将比较返回的较小的对象排在前面。
**************************************************************************/
#if __DEBUG
#if 0//上传cvs时,必须把这里设置为0,否则可能出现多个main函数，编译不过。	设置为1可以单独调试该模块.
//继承element base 和 container base, 
typedef struct {
	LEBase ebase;//继承element 的基类
	int aaa;
	char *bbb;
}ST_E_Test;


typedef struct {
	LCBase cbase; 
	int xxx;
	char yyy[42];
}ST_C_Test;




//定义扩展的错误处理宏.必须基于  LC_ERR_EXTEND_BASE
#define LCTEST_ERR_MENBER_BBB_NULL	LC_ERR_EXTEND_BASE-1




int e_test_release_self( struct element_base_t *this )//定义element的 release函数
{
	ST_E_Test *p_e_test;//继承派生类
	int err = RTN_OK;
	
	p_e_test = (ST_E_Test *)this;
	if( NULL == this )
	{
		return LC_ERR_POINTER_NULL;	
	}
	
	if( NULL == p_e_test->bbb )
	{
		err = LCTEST_ERR_MENBER_BBB_NULL; 	
	}
	else
	{
		free( p_e_test->bbb );
	}
	
	free( p_e_test );
		
	return err;
}


//定义每个元素的处理函数.
int proc_element_test( ST_E_Test *this )
{
	if( NULL != this )
	{
		printf( "proc func bbb = %s         ", this->bbb );
		printf( " aaa = %d\n", this->aaa );	
	}
	return 0;
}

//定义每个元素的处理函数2.使用set_proc_all_func来设置.
int proc_element_test2( ST_E_Test *this )
{
	if( NULL != this )
	{
		printf( "second proc func bbb = %s         ", this->bbb );
		printf( "aaa = %d\n", this->aaa );
		//比如,如果要对container中的所有元素做表格的化,你可以在这里输出一个tr.也就是每一个表格的一行.
	}
	return 0;		
}


//定义比较函数，这里需要注意，当返回一个错误编号时，可能被调用的函数判定为 p_e1 < p_e2
int element_cmp_test( ST_E_Test *p_e1, ST_E_Test *p_e2 )
{
	int iRet;
	
	if( NULL == p_e1 || NULL == p_e2 )
	{
		return LC_ERR_POINTER_NULL;	
	}
	
	
	return (p_e1->aaa<p_e2->aaa)?0:1;	
}


int element_cmp_test2( ST_E_Test *p_e1, ST_E_Test *p_e2 )
{
	int iRet;
	
	if( NULL == p_e1 || NULL == p_e2 )
	{
		return LC_ERR_POINTER_NULL;	
	}
	
	return -(p_e1->aaa-p_e2->aaa);
}



int main() 
{
	ST_C_Test *p_container;//注意这里申明的是一个ST_C_Test的变量,与之前写的<编程原则>中的shap的示例不一样,为什么?有什么作用?
	ST_E_Test *p_element;
	ST_E_Test *p_element_key;
	int i;
	
	//下面需要尤其的注意,不能将container和element搞混淆了,如果使用的是NEW_CONTAINER,第二个参数不能写成element的派生类了.
	NEW_CONTAINER(p_container,ST_C_Test,NULL);//因为结构体中没有指针需要释放,所以这里的release可以制定为NULL;
	if( NULL == p_container )
	{
		//debugprint( "err  for malloc container!\n" );
		return -1;
	}
	
	for( i=0;i<10;i++ )//一个循环完成所有element的初始化
	{
		char test[20];
		NEW_ELEMENT(p_element,ST_E_Test,e_test_release_self);//因为结构体中有一个指针,可能需要释放,所以在这里定义了e_test_release_self函数.
		sprintf( test, "test bbb!  %d", i );
		
		//strdup为bbb指针分配了空间,需要free来释放, free写在了e_test_release_self 这个回调函数里面.这样,在销毁container的时候就能自动的调用了.
		p_element->bbb = strdup(test);
		p_element->aaa = (i%5)*10;
		
		add_element( p_container, p_element, -1 );//-1将元素加到最后.
	}
	
	set_proc_all_func( p_container, proc_element_test );
	
	proc_all_ele_in_cont( p_container );
	
	
	del_element(p_container,p_element);//test,   删除最后一个节点.
	

	set_proc_all_func( p_container, proc_element_test2 );//更改了回调函数.
		
	proc_all_ele_in_cont( p_container );
	
	//设置了比较函数，并且设置了key，只打印 aaa<5的对象
	NEW_ELEMENT( p_element_key, ST_E_Test, e_test_release_self );//这个对象并没有添加到container中，需要在使用完后独立释放。
	p_element_key->aaa = 5;
	set_element_cmp_func( p_container, element_cmp_test );
	proc_eles_in_cont_by_key( p_container, p_element_key );
	
	
	
	//设置比较函数，将element按照aaa的降序排列，然后打印所有
	//将aaa按照从大到小的顺序排列。
	//前面设置的比较函数已经不能满足新的要求了，重新设置cmp函数。
	set_element_cmp_func( p_container, element_cmp_test2 );
	sort_elements( p_container );//排序
	proc_all_ele_in_cont( p_container );//处理每一个元素。
	
	
	DELETE_CONTAINER( p_container );
	
	DELETE_ELEMENT( p_element_key );//这个key只是用来过滤的，没有放在container中，单独释放。
	
	return 0;
}

#endif
#endif

