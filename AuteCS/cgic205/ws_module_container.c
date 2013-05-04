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
* ws_module_container.c
*
*
* CREATOR:
* autelan.software.Network Dep. team
* tangsq@autelan.com
*
* DESCRIPTION:
* function for web
*
*
***************************************************************************/
/*
* Copyright (c) 2008,Autelan - AuteCS
* All rights reserved.
*
*$Source: /rdoc/AuteCS/cgic205/ws_module_container.c,v $
*$Author: zhouyanmei $
*$Date: 2010/02/09 07:16:54 $
*$Revision: 1.9 $
*$State $
**Modify :tangsiqi 2008-12-24
*$Log $
*
*/
#include "ws_module_container.h"
#include "string.h"
#include "ctype.h"
#include "cgic.h"



#define LABEL_HEIGHT "25"
/*****************************************************************
static not for api
*****************************************************************/
/***************************************************************
*NAME:create_str_copy
*USEAGE:to create a copy of a string.
*Param: src -> the pointer of the string which you want to copy.
*Return:	string of the copy.
*Auther:shao jun wu
*Date:2008-11-3 14:09:51
*Modify:(include modifyer,for what resease,date)
****************************************************************/
static char *create_str_copy( char *src )
{
	char *ret=NULL;
	
	if( NULL == src )
	{
		return NULL;	
	}
	
	ret = (char*)malloc( strlen(src) + 1 );
	if( NULL == ret )
	{
		return NULL;	
	}
	
	strcpy( ret, src );
	
	return ret;
}

/***************************************************************
*NAME:get_ch_eng_cut_num
*USEAGE:to create a copy of a string.
*Param: src -> the pointer of the string which you want to copy.
*Return:	string of the copy.
*Auther:shao jun wu
*Date:2008-11-3 14:09:51
*Modify:(include modifyer,for what resease,date)
****************************************************************/
static unsigned int get_ch_eng_cut_num( unsigned char *szStr )
{
	unsigned int ret=0;
	unsigned char *temp =szStr;
	int pre_flag = 0;
	int temp_flag=0;
	
	pre_flag = -1;
	for( ; *temp != '\0'; )
	{
		if( (*temp<0x80) && (!isdigit(*temp)) && (!isalpha(*temp)) )
		{
			temp++;
			continue;
		}

		temp_flag = (*temp>0x80)?((temp+=2)&&1):((temp+=1)&&0);
		
		if( temp_flag != pre_flag )
		{
			ret++;
			pre_flag = temp_flag;
		}
	}
	
	return ret;		
}

/***************************************************************
*NAME:getFontInfo
*USEAGE:to get the font information about the lable name
*Param: des -> the output str
		src -> the input str
*Return:	0 -> success
			others -> failure
*Auther:shao jun wu
*Date:2008-11-3 14:09:51
*Modify:(include modifyer,for what resease,date)
****************************************************************/
static int getFontInfo( unsigned char *des, unsigned char *src, char *ch, char *eng )
{
	int pre_flag = 0;
	int temp_flag=0;
	
	if( NULL == src || strlen((const char*)src) == 0 || NULL == des )
	{
		return -1;	
	}
	
	strcpy( (char*)des, "<span id='" );
	if( *src > 0x80 )
	{
		strcat( (char*)des, ch );
	}
	else
	{
		strcat( (char*)des, eng );
	}
	strcat( (char*)des, "'>" );
	des += strlen((char*)des);

	pre_flag=(*src>0x80)?1:0;
	for( ; *src != '\0'; )
	{
		
		temp_flag = (*src>0x80)?1:0;
		
		if( (*src<0x80) && (!isdigit(*src)) && (!isalpha(*src)) )
			temp_flag = pre_flag;
			

		if( temp_flag != pre_flag )
		{
			strcat( (char*)des, "</span>" );
			pre_flag = temp_flag;
			if( temp_flag == 0 && *src != '\0' )
			{
				strcat( (char*)des, "<span id='" );
				strcat( (char*)des, eng );
				strcat( (char*)des, "'>" );
			}
			else if( *src != '\0' )
			{
				strcat( (char*)des, "<span id='" );
				strcat( (char*)des, ch );
				strcat( (char*)des, "'>" );

			}
			des += strlen((char*)des);
		}
		if( temp_flag == 0 || ( (*src<0x80) && (!isdigit(*src)) && (!isalpha(*src))  ) )
		{
			if( *src == 0x20 )
			{
				strcat( (char*)des, "&nbsp;" );
				des += strlen((char*)des);
				src++;
			}
			else
			{
				*des++ = *src++;
			}
		}
		else
		{
			*des++ = *src++;
			*des++ = *src++;
		}
	}	
	strcat( (char*)des, "</span>" );

	return 0;
}

/*****************************************************************
apis of Label obj
*****************************************************************/

/***************************************************************
*NAME:LB_create_label
*USEAGE:To create  a  struct STLabel,and return it's addres
*Param:
*Return:A struct pointer of STLabel
*Auther:shao jun wu
*Date:2008-11-3 13:56:36
*Modify:(include modifyer,for what resease,date)
****************************************************************/
STLabel *LB_create_label()
{
	STLabel *pstLabel=NULL;
	
	pstLabel = (STLabel*)malloc(sizeof(STLabel));
	if( NULL == pstLabel )
	{
		return NULL;	
	}
	
	memset( pstLabel, 0, sizeof(STLabel) );
	
	return pstLabel;
}


/***************************************************************
*NAME:LB_setLabelName
*USEAGE:to set the Label's name,which while show  on the  html page
*Param:		me -> the reference of the Label
*			name -> the name that set to the Label
*Return:	0 -> success
			others -> failure
*Auther:shao jun wu
*Date:2008-11-3 14:04:59
*Modify:(include modifyer,for what resease,date)
****************************************************************/
int LB_setLabelName( STLabel *me, char *name )
{
	char *temp=NULL;
	int len;
	
	if( NULL == me || NULL == name )	
	{
		return -1;	
	}
	
//添加font的信息
	//计算有多少个中英文的分割;
	
	len = strlen(name) + get_ch_eng_cut_num((unsigned char *)name)*29 + 16;//最后再留一点空白
	temp = (char*)malloc( len );
	if( NULL == temp )
	{
		return -1;	
	}
	memset( temp, 0, len );
	
	//每个分割将使得字符传加长 "<span id=yingwen_san></span>":28 和 "<span id=zhongwen_san></span>":29中最长的一个
	getFontInfo( (unsigned char *)temp, (unsigned char *)name, "zhongwen_san", "yingwen_san" );
	
	if( NULL != me->name )
	{
		free( me->name );
	}
	
	me->name = temp;
	
	return 0;
}


/***************************************************************
*NAME:LB_setLabelUrl
*USEAGE:to set the Label's url,while it connect to.
*Param:		me -> the reference of the Label
*			url -> the url that set to the Label
*Return:	0 -> success
			others -> failure
*Auther:shao jun wu
*Date:2008-11-3 14:05:56
*Modify:(include modifyer,for what resease,date)
****************************************************************/
int LB_setLabelUrl( STLabel *me, char *url )
{
	char *temp=NULL;
	
	if( NULL == me || NULL == url )	
	{
		return -1;	
	}
	
	temp = create_str_copy(url);
	if( NULL == temp )
	{
		return -1;	
	}
	
	if( NULL != me->url )
	{
		free( me->url );
	}
	
	me->url = temp;
	
	return 0;
}

/***************************************************************
*NAME:LB_changelabelName_Byindex
*USEAGE:to set the Label's name by index,which while show  on the  html page
*Param:		me -> the reference of the Label
*			name -> the name that set to the Label by index
*Return:	0 -> success
			others -> failure
*Auther:tangsiqi
*Date:2008-12-19 14:04:59
*Modify:(include modifyer,for what resease,date)
****************************************************************/
int LB_changelabelName_Byindex( STModuleContainer *me, char * LabelName, int index)
{
	int step=1;
	if( NULL == me || NULL == LabelName )
	{
		return -1;
	}

	STLabel *pstLabelLast = me->pstLabelRoot;
		
	for( ; pstLabelLast->next != NULL && step < index; pstLabelLast = pstLabelLast->next,step++ );
	
	memset(pstLabelLast->name,0,sizeof(pstLabelLast->name));
	LB_setLabelName(pstLabelLast,LabelName);
	return 0;
}

/***************************************************************
*NAME:LB_changelabelUrl_Byindex
*USEAGE:to set the Label's Url by index,which while show  on the  html page
*Param:		me -> the reference of the Label
*			name -> the url that set to the Label by index
*Return:	0 -> success
			others -> failure
*Auther:tangsiqi
*Date:2008-12-19 14:04:59
*Modify:(include modifyer,for what resease,date)
****************************************************************/
int LB_changelabelUrl_Byindex( STModuleContainer *me , char * url, int index)
{
	int step=1;
	if( NULL == me || NULL == url )
	{
		return -1;	
	}

	STLabel *pstLabelLast = me->pstLabelRoot;
		
	for( ; pstLabelLast->next != NULL && step < index; pstLabelLast = pstLabelLast->next,step++ );

	memset(pstLabelLast->url,0,sizeof(pstLabelLast->url));
	LB_setLabelUrl(pstLabelLast,url);
	return 0;
}


/***************************************************************
*NAME:LB_destroy_label
*USEAGE:destroy the label after use
*Param:		me -> the pointer of the Label's reference
*Return:	0 -> success
*			others -> failure
*Auther:shao jun wu
*Date:2008-11-3 14:28:07
*Modify:(include modifyer,for what resease,date)
****************************************************************/
int LB_destroy_label( STLabel **me )
{
	if( NULL == me || NULL == *me )
	{
		return -1;	
	}
	
	if( NULL != (*me)->name )
		free( (*me)->name );
	if( NULL != (*me)->url )
		free( (*me)->url );
		
	free( *me );
	
	(*me) = NULL;
	
	return 0;
	
}




/*****************************************************************
apis of module_container obj
*****************************************************************/
/***************************************************************
*NAME:MC_create_module_container
*USEAGE:create a module container
*Param:	
*Return:	the reference of the module container
*Auther:shao jun wu
*Date:2008-11-3 14:36:35
*Modify:(include modifyer,for what resease,date)
****************************************************************/
STModuleContainer *MC_create_module_container()
{
	STModuleContainer *pstModuleContainer=NULL;
	
	pstModuleContainer = (STModuleContainer*)malloc(sizeof(STModuleContainer));
	if( NULL == pstModuleContainer )
	{
		return NULL;	
	}
	
	memset( pstModuleContainer, 0, sizeof(STModuleContainer) );
	
	return pstModuleContainer;
}


/***************************************************************
*NAME:MC_destroy_func_container
*USEAGE:destroy a module container
*Param:		the reference of a container's pointer
*Return:	0 -> sucess
			others -> failure
*Auther:shao jun wu
*Date:2008-11-3 14:36:35
*Modify:(include modifyer,for what resease,date)
****************************************************************/
int MC_destroy_module_container( STModuleContainer **me )
{
	int i=0;
	int delState = 0;
	
	if( NULL == me || NULL == (*me))
	{
		return 0;	
	}	
	
	for( i=0; i<KEY_DOMAIN_MUN; i++ )
	{
		if( NULL != (*me)->domains[i] )
		{
			free( (*me)->domains[i] );
			(*me)->domains[i] = NULL;
		}	
	}
	
//释放所有的label
	for( ;( (*me)->ulAllLabelNum != 0 && 0 == delState ); delState = MC_delLabelByIndex(*me, 0) );

	if( 0 == delState )
	{
		free( *me );
		*me = NULL;
	}
	
	return delState;
}

/***************************************************************
*NAME:MC_setOutPutFileHandle
*USEAGE:	设置输出句柄.
*Param:		me -> the reference of a container's pointer
			outputFileHandle - >output file handle
*Return:	0 -> sucess
			others -> failure
*Auther:shao jun wu
*Date:2008-11-3 16:22:56
*Modify:(include modifyer,for what resease,date)
****************************************************************/
int MC_setOutPutFileHandle( STModuleContainer* me, FILE *outputFileHandle )
{
	if( NULL == me || NULL == outputFileHandle )
	{
		return -1;	
	}
	
	me->outputFileHandle = outputFileHandle;
	
	return 0;
}

/***************************************************************
*NAME:MC_setModuleContainerDomainValue
*USEAGE:destroy a module container
*Param:		me -> the reference of a container's pointer
			key -> the key that you want to set
			value -> the value that you want to set
*Return:	0 -> sucess
			others -> failure
*Auther:shao jun wu
*Date:2008-11-3 14:36:35
*Modify:(include modifyer,for what resease,date)
****************************************************************/
int MC_setModuleContainerDomainValue( STModuleContainer* me, ENKeyOfDomain key,char *value )
{
	char *temp=NULL;
	
	if( NULL == me || key >= KEY_DOMAIN_MUN || key < 0 || NULL == value )
	{
		return -1;	
	}
	
	temp = create_str_copy(value);
	if( NULL == temp )
	{
		return -2;	
	}
		
	if( NULL != me->domains[key] )
	{
		free( me->domains[key] );
	}
	
	me->domains[key] = temp;
		
	return 0;
}

/***************************************************************
*NAME:MC_setActiveLabel
*USEAGE:设置当前活动的标签
*Param:		me -> the reference of a container's pointer
			ulIndex -> the active label's index
*Return:	0 -> sucess
			others -> failure
*Auther:shao jun wu
*Date:2008-11-3 14:43:13
*Modify:(include modifyer,for what resease,date)
****************************************************************/
int MC_setActiveLabel( STModuleContainer* me, unsigned int ulIndex )
{
	if( NULL == me || ulIndex > me->ulAllLabelNum )
	{
		return -1;	
	}
	
	me->ulActiveLabelIndex = ulIndex;
	
	return 0;
}



/***************************************************************
*NAME:getLabelByIndex
*USEAGE:根据index得到一个Label
*Param:		me -> the reference of a STModuleContainer
			ulIndex -> the label's index that you want to get
*Return:	a reference of the label
*Auther:shao jun wu
*Date:2008-11-3 14:45:01
*Modify:(include modifyer,for what resease,date)
****************************************************************/
STLabel *MC_getLabelByIndex( STModuleContainer *me, unsigned int ulIndex )
{
	unsigned int i=0;
	STLabel *ret=NULL;
	
	if( NULL == me || ulIndex >= me->ulAllLabelNum )
	{
		return NULL;	
	}
	
	ret = me->pstLabelRoot;
	for( i = ulIndex; (i > 0 && ret != NULL ); i-- )
	{
		ret = ret->next;
	}
	
	return ret;
}


/***************************************************************
*NAME:MC_addLabel
*USEAGE:添加Label到container
*Param:		me -> the reference of a STModuleContainer
			pstLabel -> the reference of a STLabel which you want to add
*Return:	0 -> sucess
			others -> failure
*Auther:shao jun wu
*Date:2008-11-3 14:55:43
*Modify:(include modifyer,for what resease,date)
****************************************************************/
int MC_addLabel( STModuleContainer *me, STLabel *pstLabel )
{
	if( NULL == me || NULL == pstLabel )
	{
		return -1;	
	}
	
	if( NULL == me->pstLabelRoot )
	{
		me->pstLabelRoot = pstLabel;
		me->ulAllLabelNum = 1;
	}
	else
	{
		STLabel *pstLabelLast = me->pstLabelRoot;
		
		for( ; pstLabelLast->next != NULL; pstLabelLast = pstLabelLast->next );
		
		pstLabelLast->next = pstLabel;
		me->ulAllLabelNum ++;
	}

	return 0;	
}

/***************************************************************
*NAME:MC_delLabelByIndex
*USEAGE:根据index删除一个label
*Param:		me -> the reference of a STModuleContainer
			pstLabel -> the reference of a STLabel which you want to add
*Return:	0 -> sucess
			others -> failure
*Auther:shao jun wu
*Date:2008-11-3 14:55:43
*Modify:(include modifyer,for what resease,date)
****************************************************************/
int MC_delLabelByIndex( STModuleContainer *me, unsigned int ulIndex )
{
	STLabel *pstLabelPrevOfDel = NULL;
	STLabel *pstLabelDel = NULL;
	unsigned int i=0;
	
	if( NULL == me || ulIndex >= me->ulAllLabelNum )
	{
		return -1;
	}
	
	if( NULL == me->pstLabelRoot )
	{
		return -2;	
	}
	
	if( 0 == ulIndex )
	{
		pstLabelDel = me->pstLabelRoot;
		me->pstLabelRoot = pstLabelDel->next;

	}
	else
	{
		i = ulIndex-1;
		pstLabelPrevOfDel = me->pstLabelRoot;
		for( ; ( i>0 && NULL!=pstLabelPrevOfDel ); i--,pstLabelPrevOfDel=pstLabelPrevOfDel->next );
		
		if( NULL == pstLabelPrevOfDel )// if this happen, there may be has a fatal error!
		{
			return -3;
		}
		
		pstLabelDel = pstLabelPrevOfDel->next;
		pstLabelPrevOfDel->next = pstLabelDel->next;		
	}
	
	LB_destroy_label( &pstLabelDel );
	me->ulAllLabelNum--;
	
	return 0;
}

/***************************************************************
*NAME:		MC_setPrefixOfPageCallBack
*USEAGE: 	设置页面 prefix部分的代码输出的函数，设置的这个函数输出的是在html页面中，title标签之后，body标签之前的内容，通常都是
			MATA，js，css等内容。
			call back 函数的定义typedef int (*MC_CALLBACK)(void *);
			the callback function must return 0,otherwise i will think it has some fatal error, and then do not write flowing html code.
*Param:		me -> the reference of a STModuleContainer
			callback -> the function 
			callback_param -> the param that you want to pass to the callback function.it could be NULL if you has no param want to pass.
*Return:	0 -> sucess
			others -> failure
*Auther:shao jun wu
*Date:2008-11-3 15:47:40
*Modify:(include modifyer,for what resease,date)
****************************************************************/
int MC_setPrefixOfPageCallBack( STModuleContainer *me, MC_CALLBACK callback, void *callback_param )
{
	if( NULL == me || NULL == callback )
	{
		return -1;	
	}
	
	me->prefix_of_page = callback;
	me->prefix_callback_param = callback_param;
	return 0;
}

/***************************************************************
*NAME:		MC_setContentOfPageCallBack  
*USEAGE: 	设置页面 主要代码 部分的输出的函数，设置的这个函数输出的是在html页面中，title标签之后，body标签之前的内容，通常都是
			MATA，js，css等内容。
			call back 函数的定义typedef int (*MC_CALLBACK)(void *);
			the callback function must return 0,otherwise container will think it has some fatal error, and then do not write flowing html code.
*Param:		me -> the reference of a STModuleContainer
			callback -> the function 
			callback_param -> the param that you want to pass to the callback function.it could be NULL if you has no param want to pass.
*Return:	0 -> sucess
			others -> failure
*Auther:shao jun wu
*Date:2008-11-3 15:47:36
*Modify:(include modifyer,for what resease,date)
****************************************************************/
int MC_setContentOfPageCallBack( STModuleContainer *me, MC_CALLBACK callback, void *callback_param )
{
	if( NULL == me || NULL == callback )
	{
		return -1;	
	}
	
	me->content_of_page = callback;
	me->content_callback_param = callback_param;
	return 0;
}


/***************************************************************
*NAME:MC_writeHtml
*USEAGE: 	输出页面代码
*Param:		me -> the reference of a STModuleContainer
*Return:	0 -> sucess
			others -> failure
*Auther:shao jun wu
*Date:2008-11-3 14:55:43
*Modify:(include modifyer,for what resease,date)
****************************************************************/
int MC_writeHtml( STModuleContainer *me )
{
	register FILE *fp;
	int i=0;
	STLabel *pstLabel=NULL;
	
	fp = me->outputFileHandle;
	if( NULL == me || NULL == fp )
		return -1;
	
	cgiHeaderContentType("text/html");
	
	fprintf( fp,"<html xmlns=\"http://www.w3.org/1999/xhtml\">\n"\
				"	<head>\n"\
				"		<meta content=\"MSHTML 6.00.6000.16735\" name='GENERATOR' />\n"\
				"		<meta http-equiv=Content-Type content=text/html; charset=gb2312\n />"\
				"		<meta http-equiv='pragma' content='no-cache' />\n"\
				"		<meta http-equiv='Cache-Control' content='no-cache,   must-revalidate' />\n"\
				"		<meta http-equiv='expires' content='Wed,   26   Feb   1997   08:21:57   GMT' />\n" );
	
	
	fprintf( fp,"		<title>%s</title>\n", (NULL==me->domains[PAGE_TITLE])?"":(me->domains[PAGE_TITLE]) );
	//fprintf( fp,"		<link rel='stylesheet' href='/style.css' type='text/css' />\n");	
	fprintf(fp, "<link rel=stylesheet href=/style.css type=text/css>\n");

	//////////////////////
    fprintf(fp, "<style type=text/css>\n"\
  				"#div1{ width:62px; height:18px; border:1px solid #666666; background-color:#f9f8f7;}\n"\
    			"#div2{ width:60px; height:15px; padding-left:5px; padding-top:3px}\n"\
   				"#link{ text-decoration:none; font-size: 12px}\n"\
   				"</style>\n");
	fprintf(fp,	"<script type=\"text/javascript\">\n"\
				"function popMenu(objId)\n"\
				"{\n"\
			   	"	var obj = document.getElementById(objId);\n"\
			   	"	if( null == obj )\n"\
			   	"	{\n"\
			   	"		return;\n"\
			   	"	}\n"\
			   	"	if (obj.style.display == 'none')\n"\
			   	"	{\n"\
				"		obj.style.display = 'block';\n"\
			    "	}\n"\
			   	"	else\n"\
			   	"	{\n"\
				"		obj.style.display = 'none';\n"\
			   	"	}\n"\
		  	 	"}\n"\
		   		"</script>\n");
	//////////////////////
#if 1
	if( NULL != me->prefix_of_page )
	{
		if( 0 != me->prefix_of_page( me->prefix_callback_param ) )
		{
			return -1;	
		}
	}
#endif	
	fprintf( fp,"	</head>\n" );
	fprintf( fp,"	<body onresize = 'js_check_lable_bottom_height();'>\n" );
	
	//form info
	fprintf( fp,"		<form " );
	if( me->domains[FORM_ACTION] != NULL && strlen( me->domains[FORM_ACTION] ) > 0 )
	{
		fprintf( fp, "action='%s' ", me->domains[FORM_ACTION] );
	}	
	if( me->domains[FORM_ENCTYPE] != NULL && strlen( me->domains[FORM_ENCTYPE] ) > 0 
		&& ( strcmp( me->domains[FORM_ENCTYPE], "application/x-www-form-urlencoded" )==0 ||
			 strcmp( me->domains[FORM_ENCTYPE], "multipart/form-data" )==0 ||
			 strcmp( me->domains[FORM_ENCTYPE], "text/plain" )==0 )
		)
	{
		fprintf( fp, "enctype='%s' ", me->domains[FORM_ENCTYPE] );
	}
	if( me->domains[FORM_ONSIBMIT] != NULL && strlen( me->domains[FORM_ONSIBMIT] ) > 0 )
	{
		fprintf( fp, "onsubmit='%s' ", me->domains[FORM_ONSIBMIT] );
	}
	if( me->domains[FORM_METHOD] != NULL && strlen( me->domains[FORM_METHOD] ) > 0 
		&& ( strcmp( me->domains[FORM_METHOD], "post" ) == 0
			 || strcmp( me->domains[FORM_METHOD], "get" ) )
		)
	{
		fprintf( fp, "method='%s'>\n", me->domains[FORM_METHOD] );
	}
	else
	{
		fprintf( fp, "method='post'>\n" );	
	}
	
	
	//table frame begin
	fprintf( fp,"		<div align='center'>\n" );
	
	//public hidden input
	if( NULL!=me->domains[PUBLIC_INPUT_ENCRY] && strlen(me->domains[PUBLIC_INPUT_ENCRY])>0 )
	    fprintf( fp, "			<input type=hidden name=UN value=%s />", me->domains[PUBLIC_INPUT_ENCRY]);

  	fprintf( fp,"			<table width='976' border='0' cellpadding='0' cellspacing='0'>\n"\
  				"				<tr>\n"\
    			"					<td width='8' align='left' valign='top' background='/images/di22.jpg'><img alt='' src='/images/youce4.jpg' width='8' height='30' /></td>\n"\
    			"					<td width='51' align='left' valign='bottom' background='/images/di22.jpg'><img alt='' src='/images/youce33.jpg' width='37' height='24'/></td>\n" );
{
	char temp[256] = "";
	getFontInfo( (unsigned char *)temp, (unsigned char *)((NULL==me->domains[MODULE_TITLE])?"default":me->domains[MODULE_TITLE]), "titlech", "titleen" );
   	fprintf( fp,"					<td width='153' align='left' valign='bottom' background='/images/di22.jpg'>%s</td>\n", temp );
}
    fprintf( fp,"					<td width='692' align='right' valign='bottom' background='/images/di22.jpg'>\n" );

	fprintf( fp, "<table cellSpacing=0 cellPadding=0 width=130 border=0>\n"\
				"  <tbody>\n"\
				"  <tr>\n" );
    if( NULL != me->domains[BTN_OK_SUBMIT_NAME] && strlen(me->domains[BTN_OK_SUBMIT_NAME]) > 0 )
    {
		fprintf( fp, "	<td valign=bottom align=left width=62><input id=but style='background-image:url(/images/%s)' type=submit name=%s value='' /></td>\n",
						(NULL==me->domains[BTN_OK_IMG])? "ok-ch.jpg":me->domains[BTN_OK_IMG],
						me->domains[BTN_OK_SUBMIT_NAME] );
	}
	else
	{
		fprintf( fp, "	<td valign=bottom align=left width=62><a href='%s' target='mainFrame'><img src='/images/%s' border='0' alt='' width=62 height=20 border=0 /></a></td>\n",
						(NULL==me->domains[BTN_OK_URL])?"wp_sysmagic.cgi":me->domains[BTN_OK_URL], 
						(NULL==me->domains[BTN_OK_IMG])?"ok-ch.jpg":me->domains[BTN_OK_IMG] );
	}
	fprintf( fp, "	<td valign=bottom align=left width=62><a href='%s' target=mainFrame><img src='/images/%s'  alt='' width=62 height=20 border=0 /></a></td>\n",
					(NULL==me->domains[BTN_CANCEL_URL])?"wp_sysmagic.cgi":me->domains[BTN_CANCEL_URL], 
					(NULL==me->domains[BTN_CANCEL_IMG])?"cancel-ch.jpg":me->domains[BTN_CANCEL_IMG] );


	fprintf( fp, "  </tr></tbody></table>\n" );

    fprintf( fp,"					</td>\n");
    //end of ok-cancel
	    			
	fprintf( fp,"					<td width='74' align='right' valign='top' background='/images/di22.jpg'><img alt='' src='/images/youce3.jpg' width='31' height='30' /></td>\n"\
  				"				</tr>\n");
  				
	fprintf( fp,"				<tr>\n"\
				"					<td colspan='5' align='center' valign='middle'>\n" );
	fprintf( fp,"						<table width='976' border='0' cellpadding='0' cellspacing='0' bgcolor='#f0eff0'>\n"\
				"							<tr>\n"\
				"								<td width='12' align='left' valign='top' background='/images/di888.jpg'>&nbsp;</td>\n"\
				"								<td width='948'>\n"\
				"									<table width='947' border='0' cellspacing='0' cellpadding='0'>\n"\
				"										<tr height='4' valign='bottom'>\n"\
				"											<td width='120'>&nbsp;</td>\n"\
				"											<td width='827' valign='bottom'><img alt='' src='/images/bottom_05.gif' width='827' height='4' /></td>\n"\
				"										</tr>\n"\
				"										<tr>\n"\
				"											<td>\n"\
				"												<table width='120' border='0' cellspacing='0' cellpadding='0'>\n" );
	fprintf( fp,"													<tr height='%s'><td id='tdleft'>&nbsp;</td></tr>\n", 
																		(NULL==me->domains[LABLE_TOP_HIGHT])?LABEL_HEIGHT:me->domains[LABLE_TOP_HIGHT]);

//循环输出每个label
	for( pstLabel = me->pstLabelRoot,i=0; pstLabel !=NULL; i++,pstLabel=pstLabel->next )
	{
		if( i == me->ulActiveLabelIndex )
		{
			fprintf( fp,"													<tr height='26'>\n"\
                    	"														<td align='left' id='tdleft' background='/images/bottom_bg.gif' style='border-right:0'>\n"\
																					"%s</td></tr>\n", pstLabel->name );   /*突出显示*/
		}
		else
		{
			fprintf( fp,"													<tr height='%s'>\n"\
					  	"														<td align='left' id='tdleft'>\n"\
					  	"															<a href='%s' target='mainFrame' class='top'>%s</a></td></tr>",LABEL_HEIGHT, pstLabel->url, pstLabel->name );
		}
	}
	//输出一个tr,控制高度
	fprintf( fp,"													<tr id='label_bottom_height_ctrl' height='%s'><td align='left' id='tdleft'>&nbsp;</td></tr>\n", 
																				(NULL==me->domains[LABLE_BOTTOM_HIGHT])?"1":me->domains[LABLE_BOTTOM_HIGHT]);
	fprintf( fp,"												</table>\n" );
	fprintf( fp,"											</td>\n" );
	fprintf( fp,"											<td align='left' style='background-color:#ffffff; border-right:1px solid #707070; padding-left:30px'>\n" );
	
	//content
	fprintf( fp,"												<div id='main_content' onresize='js_check_lable_bottom_height();' >\n" );
	
	//输出content
	if( NULL != me->content_of_page )
	{
		if( 0 != me->content_of_page( me->content_callback_param ) )
		{
			return -1;	
		}
	}	
	
	fprintf( fp,"												</div>\n" );
	fprintf( fp,"											</td>\n"\
				"										</tr>\n"\
				"								<tr height='4' valign='top'>\n"\
				"									<td width='120' height='4' align='right' valign='top'><img alt='' src='/images/bottom_07.gif' width='1' height='10' /></td>\n"\
				"									<td width='827' height='4' valign='top' bgcolor='#FFFFFF'><img alt='' src='/images/bottom_06.gif' width='827' height='15' /></td>\n"\
				"								</tr>\n"\
				"							</table>\n"\
				"						</td>\n"\
				"						<td width='15' background='/images/di999.jpg'>&nbsp;</td>\n"\
				"					</tr>\n"\
				"				</table>\n"\
				"			</td>\n"\
				"		</tr>\n"\
				"		<tr>\n"\
				"			<td colspan='3' align='left' valign='top' background='/images/di777.jpg'><img alt='' src='/images/di555.jpg' width='61' height='62' /></td>\n"\
				"			<td align='left' valign='top' background='/images/di777.jpg'>&nbsp;</td>\n"\
				"			<td align='left' valign='top' background='/images/di777.jpg'><img alt='' src='/images/di666.jpg' width='74' height='62' /></td>\n"\
				"		</tr>\n"\
				"	</table>\n"\
				"</div>\n"\
				"</form>\n"\
				"</body>\n" );

	
#if 1	
	//控制label bottom高度的js函数.
	fprintf( fp,"<script type='text/javascript'>\n"\
				"	function js_check_lable_bottom_height()\n"\
				"	{\n"\
				"       //popMenu('main_content');\n"\
				"		var content_height = document.getElementById( 'main_content' ).offsetHeight;\n"\
				"		//alert( content_height );\n" );
	fprintf( fp,"		var label_height_toset = content_height - %d*%s - %s + %s*2;\n",me->ulAllLabelNum,LABEL_HEIGHT,me->domains[LABLE_TOP_HIGHT], LABEL_HEIGHT );
	fprintf( fp,"		if( label_height_toset <= 0 )\n"\
				"			label_height_toset = 1;\n"\
				"		document.getElementById( 'label_bottom_height_ctrl' ).style.height = label_height_toset;\n"\
				"	}\n"\
				"	js_check_lable_bottom_height();\n"\
				"</script>\n" );
#endif				
	
	fprintf( fp,"</html>\n" );

	return 0;
}

