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
* ws_secondary_container.h
*
*
* CREATOR:
* autelan.software.Network Dep. team
* shaojw@autelan.com
*
* DESCRIPTION:
* function for web
*
*
***************************************************************************/
#ifndef _WS_SECONDARY_CONTAINER_H
#define _WS_SECONDARY_CONTAINER_H

#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include "ws_list_container.h"
#include "ws_sndr_cfg.h"//这个配置文件决定了编译的时候需要将哪些模块编译到页面

#define _DEBUG_SNDR_CONTAINER	0


#define MAX_LABLE_STR_LEN		32
#define MAX_LABLE_URL_LEN		128
#define MAX_LABLE_IMG_NAME_LEN	32
#define MAX_SUMMARY_TITLE_LEN	32
#define MAX_SUMMARY_KEY			32
#define MAX_SUMMARY_KEY_INFO	32
#define MAX_SUMMARY_KEY_VALUE	32


typedef struct secondary_module_label{
	char szlabelname[MAX_LABLE_STR_LEN];
	char szlabelurl[MAX_LABLE_URL_LEN];
	char szlabelimgname[MAX_LABLE_IMG_NAME_LEN];	
	char szlabelfont[MAX_LABLE_STR_LEN];
}t_sndr_label;


typedef struct secondary_module_summary{
	char szsummarytitle[MAX_SUMMARY_TITLE_LEN];
	char szsummarykey[MAX_SUMMARY_KEY];
	char szsummarykeyinfo[MAX_SUMMARY_KEY_INFO];
	char szsummarykeyvalue[MAX_SUMMARY_KEY_VALUE];
	char szsummaryfont[MAX_SUMMARY_KEY_VALUE];
}t_sndr_summary;

typedef struct secondary_module_item {
	LEBase			st_list_element_base;//继承链表容器的元素基类,
	t_sndr_label	st_sndr_label;
	t_sndr_summary 	st_sndr_summary;
	int (*ex_show_call_back)( struct secondary_module_item *this );//非必须，面板会用到。
	FILE *fp;
}t_sndr_item,STSndrItem;

typedef int (*EX_SHOW_CALL_BACK)( struct secondary_module_item *this );


typedef int (*EX_SHOW_CALL_BACK_N)( struct secondary_module_container *this );
typedef int (*EX_SHOW_CALL_BACK_NZ)( struct secondary_module_container *this );


typedef struct secondary_module_container{
	LCBase			st_list_container_base;//继承链表容器的容器基类。
	//在这里存放与页面框架显示相关的变量。比如框架需要使用txt文件的指针。等。
	FILE	*fp;//通常设置为cgiOut.	
	void *lpublic;
	void *local;
	char encry[50];
	unsigned int pid;
	EX_SHOW_CALL_BACK_N callback_content;
	void *callback_param;
	EX_SHOW_CALL_BACK_NZ callback_content_z;
	void *callback_param_z;
	int flag;
	
}t_sndr_module_container,STSndrContainer;

int MC_setPageCallBack( STSndrContainer *me, EX_SHOW_CALL_BACK_N callback,void *callback_param);
int MC_setPageCallBack_z( STSndrContainer *me, EX_SHOW_CALL_BACK_NZ callback,void *callback_param_z);

typedef struct {
	void *public;
	void *local;
	char encry[50];
	int plotid;
}STPubInfoForItem;




//创建一个二级页面的item
STSndrItem *create_sndr_module_item();
//删除一个二级页面的item
int release_sndr_module_item( STSndrItem *this );
//设置lable name
int SI_set_label_name( STSndrItem *this, char *name );
//设置img
int SI_set_label_img( STSndrItem *this, char *img );
//设置url
int SI_set_label_url( STSndrItem *this, char *url );
//设置encry
int SI_set_label_encry( STSndrItem *this, char *encry );
//设置font
int SI_set_label_font( STSndrItem *this, char *font );

//设置summary title
int SI_set_summary_title( STSndrItem *this, char *title );
//设置summary key
int SI_set_summary_key( STSndrItem *this, char *key );
//设置summary key info
int SI_set_summary_keyinfo( STSndrItem *this, char *keyinfo );
//设置summary key value
int SI_set_summary_keyvalue( STSndrItem *this, char *keyvalue );
//设置summary key font
int SI_set_summary_font( STSndrItem *this, char *font );

//设置callback  非必须
int SI_set_show_callback( STSndrItem *this, EX_SHOW_CALL_BACK callback );

//创建一个二级页面的容器
STSndrContainer *create_sndr_module_container();

//销毁一个二级页面容器
int release_sndr_module_container( STSndrContainer *this);

//添加 item到container
int SC_add_item( STSndrContainer *this, STSndrItem *p_item );

//将container 输出为页面
int SC_writeHtml( STSndrContainer *this);

//是否显示图片，前后要修改整体效果
int if_show_sndr_icon();

//每个二级模块都要提供一个如下接口的函数来完成ｉｔｅｍ数据的获得。
//lpublic  llocal为两个txt的语言链表，按理说应该有函数内部自己去打开文件获得链表的，考虑到如果container中会打开这些文件并取得链表，就直接传到下层，节约程序执行时间。如果除了这两个链表还使用了其它的，就需要函数自己去打开了。
//e.g. int fill_summary_data_valn( void *public, void *local, STSndrItem *p_item );
//e.g. int fill_summary_data_port( void *public, void *local, STSndrItem *p_item );
typedef int (*FILL_SUMMARY_DATA)( STPubInfoForItem *p_pubinfo, STSndrItem *p_item );

//辅助创建二级页面的结构及函数
//这里将helper定义为一个结构体，是因为将来可能还会向helper中添加属性。
typedef struct sc_create_helper {
	FILL_SUMMARY_DATA fill_data_api;
}STSCCreateHelper;

STSndrContainer *create_sndr_module_container_helper( STPubInfoForItem *p_pubinfo, STSCCreateHelper pstSCCreateHelper[], int num );

#endif
