/* cgicTempDir is the only setting you are likely to need
	to change in this file. */

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
* ws_ec.h
*
*
* CREATOR:
* autelan.software.Network Dep. team
* qiaojie@autelan.com
* shaojw@autelan.com
* tangsq@autelan.com
* zhouym@autelan.com
*
* DESCRIPTION:
*
*
*
*******************************************************************************/


#include<stdio.h>                        /*输入输出头文件*/
#include<string.h>                       /*字符分类头文件*/
#include<stdlib.h>                       /*动态存储分配头文件*/

#ifndef _WS_EC_H
#define _WS_EC_H

#define LN 128                            /*存放单个变量的临时数组长度*/
#define File_Len 30                            /*上传文件名的最大长度*/

#define LONG_SORT 600
#define SHORT_SORT 120

#define STR_NOT_FOND	"string no found"			/*can also defined as  NULL*/




struct list                              /*链表结构*/
{
//  char val[LN];                           /*存放变量及其对应值*/
	struct list *next;                     /*指向下一节点的指针*/
	char val[0];					/*存放变量及其对应值,这个数组的长度是根据实际的长度来malloc的，长度为LN的整数被*/
};                                /*le指向英文链表的头指针,lc指向中文链表的头指针*/

int equal(char *des,char *src);        /*比较字符串src与字符串des的前sizeof(src)个字符是否相等，相等返回1，不相等返回0*/
int load(struct list *l);               /*分隔文件fp中的变量，存入链表l*/
char *search(struct list *l,char *v);    /*在链表l中查找变量v，返回指向变量的值的指针，没找到返回NULL*/
void release(struct list *l);            /*遍历链表l，释放空间*/
char *ser_var(char *v);                /*在文件en-ch.txt中查找变量var的值*/
char *getFPath(char *v);  /*根据所给字符串查找它对应的文件路径，成功返回指向文件路径字符串的指针，失败返回NULL*/
char * readproductID();
int getfpath(const char *v, char *fpath );
struct list * get_chain_head(char *file_name); /*file_name表示资源文件的名字，返回链表的头*/





char *replace_url(char *strbuf, char *sstr, char *dstr);



#endif

