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
* ws_secondary_container.c
*
*
* CREATOR:
* autelan.software.Network Dep. team
* zhouym@autelan.com
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
*$Source: /rdoc/AuteCS/cgic205/ws_secondary_container.c,v $
*$Author: zhouyanmei $
*$Date: 2010/02/09 07:16:54 $
*$Revision: 1.22 $
*$State: Exp $
*$Modify:	$
*$Log $
*
*/
	
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include "ws_list_container.h"
#include "ws_sndr_cfg.h"//这个配置文件决定了编译的时候需要将哪些模块编译到页面
#include "ws_secondary_container.h"
#include "cgic.h"
#include "ws_ec.h"

/******************************************************
二级页面的公共容器对象，所有二级页面，只需要提供每个模块相应的api后，
就能让其在二级页面概述中显示出来。
使用了　 list container  公共的链表容器。
**********************************************************/

#define sec 60
#define test(buff,buff_size,cmd)\
	{\
		FILE *fp;\
		fp = popen( cmd,"r" );\
		if( NULL != fp ){\
			memset(buff,0,sizeof(buff));\
			fgets( buff, buff_size, fp );\
			pclose(fp);\
		}\
	}

//创建一个二级页面的item
STSndrItem *create_sndr_module_item()
{
	STSndrItem *pRet=NULL;
	
	NEW_ELEMENT( pRet, STSndrItem, NULL );//结构体内部没有分配空间，这里不需要设置释放函数，最后一个参数为NULL
	
	return pRet;
}

//删除一个二级页面的item
int release_sndr_module_item( STSndrItem *this )
{
	DELETE_ELEMENT( this );
	return 0;
}

//设置lable name
int SI_set_label_name( STSndrItem *this, char *name )
{
	if( NULL == this || NULL == name ) return -1;
	
	if( strlen(name) > sizeof(this->st_sndr_label.szlabelname)-1 )
	{
		return  -1;	
	}
	
	strcpy( this->st_sndr_label.szlabelname, name );
	
	return 0;
}

//设置img
int SI_set_label_img( STSndrItem *this, char *img )
{
	if( NULL == this || NULL == img ) return -1;

	if( strlen(img) > sizeof(this->st_sndr_label.szlabelimgname)-1 )
	{
		return  -1;	
	}
	
	strcpy( this->st_sndr_label.szlabelimgname, img );
	
	return 0;		
}
//设置url
int SI_set_label_url( STSndrItem *this, char *url )
{
	if( NULL == this || NULL == url ) return -1;

	if( strlen(url) > sizeof(this->st_sndr_label.szlabelurl)-1 )
	{
		return  -1;	
	}
	
	strcpy( this->st_sndr_label.szlabelurl, url );
	
	return 0;		
}
//设置font
int SI_set_label_font( STSndrItem *this, char *font )
{
	if( NULL == this || NULL == font ) return -1;

	if( strlen(font) > sizeof(this->st_sndr_label.szlabelfont)-1 )
	{
		return  -1;	
	}
	
	strcpy( this->st_sndr_label.szlabelfont, font );
	
	return 0;		
}

//设置summary title
int SI_set_summary_title( STSndrItem *this, char *title )
{
	if( NULL == this || NULL == title ) return -1;

	if( strlen(title) > sizeof(this->st_sndr_summary.szsummarytitle)-1 )
	{
		return  -1;	
	}
	
	strcpy( this->st_sndr_summary.szsummarytitle, title );
	
	return 0;
}
//设置summary key
int SI_set_summary_key( STSndrItem *this, char *key )
{
	if( NULL == this || NULL == key ) return -1;

	if( strlen(key) > sizeof(this->st_sndr_summary.szsummarykey)-1 )
	{
		return  -1;	
	}
	
	strcpy( this->st_sndr_summary.szsummarykey, key );
	
	return 0;	
}

//设置summary key info
int SI_set_summary_keyinfo( STSndrItem *this, char *keyinfo )
{
	if( NULL == this || NULL == keyinfo ) return -1;

	if( strlen(keyinfo) > sizeof(this->st_sndr_summary.szsummarykeyinfo)-1 )
	{
		return  -1;	
	}
	
	strcpy( this->st_sndr_summary.szsummarykeyinfo, keyinfo );
	
	return 0;			
}

//设置summary key value
int SI_set_summary_keyvalue( STSndrItem *this, char *keyvalue )
{
	if( NULL == this || NULL == keyvalue ) return -1;

	if( strlen(keyvalue) > sizeof(this->st_sndr_summary.szsummarykeyvalue)-1 )
	{
		return  -1;	
	}
	
	strcpy( this->st_sndr_summary.szsummarykeyvalue, keyvalue );
	
	return 0;			
}
//设置summary key font
int SI_set_summary_font( STSndrItem *this, char *font )
{
	if( NULL == this || NULL == font ) return -1;

	if( strlen(font) > sizeof(this->st_sndr_summary.szsummaryfont)-1 )
	{
		return  -1;	
	}
	
	strcpy( this->st_sndr_summary.szsummaryfont, font );
	
	return 0;			
}

//设置callback  非必须
int SI_set_show_callback( STSndrItem *this, EX_SHOW_CALL_BACK callback )
{
	if( NULL == this || NULL == callback ) return -1;
	
	this->ex_show_call_back = callback;
	return 0;
}

int MC_setPageCallBack( STSndrContainer *me, EX_SHOW_CALL_BACK_N callback,void *callback_param)
{
	if( NULL == me || NULL == callback )
	{
		return -1;	
	}
	me->callback_content = callback;
	me->callback_param = callback_param;
	return 0;
}
int MC_setPageCallBack_z( STSndrContainer *me, EX_SHOW_CALL_BACK_NZ callback,void *callback_param_z)
{
	if( NULL == me || NULL == callback )
	{
		return -1;	
	}
	me->callback_content_z= callback;
	me->callback_param_z = callback_param_z;
	return 0;
}

//创建一个二级页面的容器
STSndrContainer *create_sndr_module_container()
{
	STSndrContainer *pRet=NULL;
	
	NEW_CONTAINER(pRet,STSndrContainer,release_sndr_module_container );
	
	return pRet;
}

//销毁一个二级页面容器
int release_sndr_module_container( STSndrContainer *this)
{
//	DELETE_CONTAINER( this );
	return 0;
}


//添加 item到container
int SC_add_item( STSndrContainer *this, STSndrItem *p_item )
{
	return add_element( this, p_item, -1 );
}


//输出每个item的summary代码的函数，每个元素的html通常是一个<tr>包含的内容。
//这个是函数是设置到list_container的回调函数。在遍历list_container中的元素的时候，由container自己调用的函数。
int SI_writeEachLableHtml( STSndrItem *this )
{
	int flag=-1;
	FILE *ffp=this->fp;
	if( NULL == this )
	{
		return -1;	
	}
    /////////////////////////左边框里的带图片的条目 写到一个table 里	 

	 if(strcmp(this->st_sndr_label.szlabelimgname,"null")!=0)
	 {
	 //fprintf(ffp,"<tr height=7><td></td><td></td></tr>");
     fprintf(ffp,"<tr>");
	 flag=if_show_sndr_icon();
	 if(flag==1)
	 {
     	fprintf(ffp,"<td width=52 align=center height=43 >");	 
	 /////根据一个函数的返回值要决定是否要显示图片
	 	if(this->st_sndr_label.szlabelimgname == NULL)
	 		fprintf(ffp,"");
	 	else 
	 		fprintf(ffp,"<img src=%s width=43 height=43 />",this->st_sndr_label.szlabelimgname); //链接图片	
	 
	 /////end
     	fprintf(ffp,"</td>");    
	
     	fprintf(ffp,"<td align=left width=101 style='word-break:keep-all;border: thin #0000FF'><a href='%s' target=mainFrame class=qu ><font id=%s>%s</font>"\
     		"</a></td>",this->st_sndr_label.szlabelurl,this->st_sndr_label.szlabelfont,this->st_sndr_label.szlabelname); //链接命令
	 }
	 else
	 {	
     	fprintf(ffp,"<td width=12 height=22>&nbsp;</td>"\
     		"<td align=left><a href='%s' target=mainFrame class=qu><font id=%s>%s</font>"\
     		"</a></td>",this->st_sndr_label.szlabelurl,this->st_sndr_label.szlabelfont,this->st_sndr_label.szlabelname); //链接命令
	 }
       
     fprintf(ffp,"</tr>");
     fprintf(ffp,"<tr>"\
       "<td colspan=2 align=center>&nbsp;</td>"\
     "</tr>");
	/////////////////////////
    }
	return 0;	
}


//输出每个item的label代码的函数，通常是一个<tr>包含的内容。
//这个是函数是设置到list_container的回调函数。
int SI_writeEachSummaryHtml( STSndrItem *this )
{
   	FILE *ffp=this->fp;
	if( NULL == this )
	{
		return -1;	
	}
 
////////////////////////////////////////////右边边框中的内容,可调用函数wp_contrl.c
    fprintf(ffp,"<tr>"\
     "<td colspan=3 style=\"border-bottom:1px solid black; padding-top:25px\"><font id=yingwen_summary2>%s</font></td>",this->st_sndr_summary.szsummarytitle);
    fprintf(ffp,"</tr>"\
    "<tr align=left style=\"padding-top:10px; padding-left:10px\">"\
     "<td width=300><font id=%s3>%s</font></td>",this->st_sndr_summary.szsummaryfont,this->st_sndr_summary.szsummarykeyinfo);
     fprintf(ffp,"<td width=150><font id=%s4>%s</font></td>",this->st_sndr_summary.szsummaryfont,this->st_sndr_summary.szsummarykeyvalue);	
     fprintf(ffp,"<td width=150>%s</td>",this->st_sndr_summary.szsummarykey);
    fprintf(ffp,"</tr>");
///////////////////////////////////////////          
         
	  		

	return 0;
}

//将container 输出为页面,包含框架内容,按照一个页面来写
int SC_writeHtml(STSndrContainer *this )
{  
    FILE *ffp=this->fp;
    
    //html的头,但是也牵涉到title名字和引用的js等(页面暂时不需要)
    int flag=-1;
	flag=this->flag;
	if(flag==1)
		{
      	cgiHeaderContentType("text/html");
        fprintf(ffp,"<html xmlns=\"http://www.w3.org/1999/xhtml\"><head>");
        fprintf(ffp,"<meta http-equiv=refresh content=%d url=wp_sysmagic.cgi?UN=%s charset=gb2312>",sec,this->encry);
        fprintf(ffp,"<title></title>");
        fprintf(ffp,"<link rel=stylesheet href=/style.css type=text/css>"\
        "</head>"\
        "<script language=javascript>"\
      	"var   d=%d;",sec+1);
      	  fprintf(ffp,"function	 DispTime()"\
      	  "{"\
      			"if(d>0)"\
      			"{"\
      			"window.Form1.DateTime.value=--d;"\
      			"setTimeout(DispTime,1000);"\
      			"}"\
      	  "}"\
      	 "</script>"\
        "<body onload=DispTime()>"\
        "<form id=Form1 method=post runat=server>"\
        "<div align=center>"\
        "<table id=plant width=976 border=0 cellpadding=0 cellspacing=0>"\
        "<tr>"\
        "<td width=8 align=left valign=top background=/images/di21.jpg><img src=/images/youce6.jpg width=8 height=15/></td>"\
          "<td width=61 align=left valign=bottom background=/images/di21.jpg>&nbsp;</td>"\
      	"<td width=143 align=left valign=bottom background=/images/di21.jpg>&nbsp;</td>"\
      	"<td width=690 align=right valign=bottom background=/images/di21.jpg>&nbsp;</td>"\
          "<td width=74 align=right valign=top background=/images/di21.jpg><img src=/images/youce5.jpg width=31 height=15/></td>"\
        "</tr>"\
        "<tr>"\
        "<td colspan=5 align=center valign=middle><table width=976 border=0 cellpadding=0 cellspacing=0 bgcolor=#f0eff0>"\
            "<tr>"\
              "<td width=12 align=left valign=top background=/images/di888.jpg>&nbsp;</td>"\
              "<td width=948>"\
                "<table width=947 border=0 cellpadding=0 cellspacing=0>"\
      			"<tr valign=top>"\
                  "<td width=171>");
      }    
	 else
     {
        cgiHeaderContentType("text/html");
        fprintf(ffp,"<html xmlns=\"http://www.w3.org/1999/xhtml\"><head>");
        fprintf(ffp,"<meta http-equiv=Content-Type content=text/html; charset=gb2312>");
        fprintf(ffp,"<title></title>");
        fprintf(ffp,"<link rel=stylesheet href=/style.css type=text/css>"\
        "</head>"\
        "<body>"\
        "<form>"\
        "<div align=center>"\
        "<table width=976 border=0 cellpadding=0 cellspacing=0>"\
        "<tr>"\
        "<td width=8 align=left valign=top background=/images/di21.jpg><img src=/images/youce6.jpg width=8 height=15/></td>"\
          "<td width=61 align=left valign=bottom background=/images/di21.jpg>&nbsp;</td>"\
      	"<td width=143 align=left valign=bottom background=/images/di21.jpg>&nbsp;</td>"\
      	"<td width=690 align=right valign=bottom background=/images/di21.jpg>&nbsp;</td>"\
          "<td width=74 align=right valign=top background=/images/di21.jpg><img src=/images/youce5.jpg width=31 height=15/></td>"\
        "</tr>"\
        "<tr>"\
        "<td colspan=5 align=center valign=middle><table width=976 border=0 cellpadding=0 cellspacing=0 bgcolor=#f0eff0>"\
            "<tr>"\
              "<td width=12 align=left valign=top background=/images/di888.jpg>&nbsp;</td>"\
              "<td width=948>"\
                "<table width=947 border=0 cellpadding=0 cellspacing=0>"\
      			"<tr valign=top>"\
                  "<td width=171>");
     }

    //左边的矩形框的框架
	 /******************************Fillet rectangular top********************************/		  
		  fprintf(ffp,"<table style=\"table-layout: fixed\" height=28 cellspacing=0 cellpadding=0 width=160 border=0>"\
		  "<tbody>"\
		  "<tr height=3 width=%s>","100%");
		  fprintf(ffp,"<td><table style=\"table-layout: fixed\" height=3 cellspacing=0 cellpadding=0 width=%s border=0>","100%");
		  fprintf(ffp,"<tbody>"\
		  "<tr height=1>"\
		  "<td width=1></td>"\
		  "<td width=1></td>"\
		  "<td width=1></td>"\
		  "<td bgcolor=#3f3f3f></td>"\
		  "<td width=1></td>"\
		  "<td width=1></td>"\
		  "<td width=1></td>"\
		  "</tr>"\
		  "<tr height=1>"\
		  "<td></td>"\
		  "<td bgcolor=#3f3f3f colspan=2></td>"\
		  "<td bgcolor=#ffffff></td>"\
		  "<td bgcolor=#3f3f3f colspan=2></td>"\
		  "<td></td>"\
		  "</tr>"\
		  "<tr height=1>"\
		  "<td></td>"\
		  "<td bgcolor=#3f3f3f></td>"\
		  "<td bgcolor=#ffffff colspan=3></td>"\
		  "<td bgcolor=#3f3f3f></td>"\
		  "<td></td>"\
		  "</tr>"\
		  "</tbody>"\
		  "</table></td>"\
		  "</tr>"\
		  "<tr>"\
		  "<td><table style=\"table-layout: fixed\" height=%s cellspacing=0 cellpadding=0 border=0>","100%");
		  fprintf(ffp,"<tbody>"\
		  "<tr>"\
		  "<td width=1 bgcolor=#3f3f3f></td>"\
		  "<td bgcolor=#ffffff>");
	      /****************************end Fillet rectangular top******************************/
    fprintf(ffp,"<table width=160 border=0 align=left bgcolor=#ffffff cellpadding=0 cellspacing=0>");
    fprintf(ffp,"<tr>"\
       "<td colspan=2 align=center>&nbsp;</td>"\
     "</tr>");
	
	//printf("element num = %d\n", get_element_num(this));
	//下面的调用输出label部分的html代码。
	set_proc_all_func( this, SI_writeEachLableHtml);
	proc_all_ele_in_cont( this );
	
	

     //右边的矩形框的框架
	      fprintf(ffp,"</table>\n");
	      fprintf(ffp,"</td>");
		  fprintf(ffp,"<td width=1 bgcolor=#3f3f3f></td>"\
		  "</tr>"\
		  "</tbody>"\
		  "</table></td>"\
		  "</tr>"\
		  "<tr height=3 width=%s>","100%");
		  fprintf(ffp,"<td><table style=\"table-layout: fixed\" height=3 cellspacing=0 cellpadding=0 width=%s border=0>","100%");
		  fprintf(ffp,"<tbody>"\
		  "<tr height=1>"\
		  "<td width=1></td>"\
		  "<td width=1 bgcolor=#3f3f3f></td>"\
		  "<td width=1 bgcolor=#ffffff></td>"\
		  "<td bgcolor=#ffffff></td>"\
		  "<td width=1 bgcolor=#ffffff></td>"\
		  "<td width=1 bgcolor=#3f3f3f></td>"\
		  "<td width=1></td>"\
		  "</tr>"\
		  "<tr height=1>"\
		  "<td></td>"\
		  "<td bgcolor=#3f3f3f colspan=2></td>"\
		  "<td bgcolor=#ffffff></td>"\
		  "<td bgcolor=#3f3f3f colspan=2></td>"\
		  "<td></td>"\
		  "</tr>"\
		  "<tr height=1>"\
		  "<td colspan=3></td>"\
		  "<td bgcolor=#3f3f3f></td>"\
		  "<td colspan=3></td>"\
		  "</tr>"\
		  "</tbody>"\
		  "</table></td>"\
		  "</tr>"\
		  "</tbody>"\
		  "</table>");	
	      /****************************end Fillet rectangular bottom******************************/

		  fprintf(ffp,"</td>"\
			"<td width=10>&nbsp;</td>"\
			"<td width=766>");

		  /******************************Fillet rectangular top********************************/		  		  
		  fprintf(ffp,"<table style=\"table-layout: fixed\" height=28 cellspacing=0 cellpadding=0 width=766 border=0>"\
		  "<tbody>"\
		  "<tr height=3 width=%s>","100%");
		  fprintf(ffp,"<td><table style=\"table-layout: fixed\" height=3 cellspacing=0 cellpadding=0 width=%s border=0>","100%");
		  fprintf(ffp,"<tbody>"\
		  "<tr height=1>"\
		  "<td width=1></td>"\
		  "<td width=1></td>"\
		  "<td width=1></td>"\
		  "<td bgcolor=#3f3f3f></td>"\
		  "<td width=1></td>"\
		  "<td width=1></td>"\
		  "<td width=1></td>"\
		  "</tr>"\
		  "<tr height=1>"\
		  "<td></td>"\
		  "<td bgcolor=#3f3f3f colspan=2></td>"\
		  "<td bgcolor=#ffffff></td>"\
		  "<td bgcolor=#3f3f3f colspan=2></td>"\
		  "<td></td>"\
		  "</tr>"\
		  "<tr height=1>"\
		  "<td></td>"\
		  "<td bgcolor=#3f3f3f></td>"\
		  "<td bgcolor=#ffffff colspan=3></td>"\
		  "<td bgcolor=#3f3f3f></td>"\
		  "<td></td>"\
		  "</tr>"\
		  "</tbody>"\
		  "</table></td>"\
		  "</tr>"\
		  "<tr>"\
		  "<td><table style=\"table-layout: fixed\" height=%s cellspacing=0 cellpadding=0 border=0>","100%");
		  fprintf(ffp,"<tbody>"\
		  "<tr>"\
		  "<td width=1 bgcolor=#3f3f3f></td>"\
		  "<td bgcolor=#ffffff>");
	      /****************************end Fillet rectangular top******************************/
    if(this->callback_content == NULL)
    {
    fprintf(ffp,"<table width=766 height=100%% align=center border=0 bgcolor=#ffffff cellpadding=0 cellspacing=0>\n"\
    "<tr>\n"\
      "<td width=766 align=left valign=top style=\"padding-left:10px; padding-top:10px\">\n"\
      "<table width=700 border=0 cellspacing=0 cellpadding=0>\n"\
      "<tr height=30>"
       "<td id=sec style=\"border-bottom:2px solid #163871\"><font id=%s1>%s</font></td>", search(this->lpublic,"menu_summary"),search( this->lpublic, "summary" ));
      fprintf(ffp,"</tr>\n"\
      "<tr>\n"\
        "<td style=\"padding-left:30px; padding-top:20px\"><table width=600 border=0 cellspacing=0 cellpadding=0>\n");
	//下面的调用输出summary部分的html代码。
	if(this->callback_content_z != NULL)
	{
      this->callback_content_z(this);
	}
	set_proc_all_func( this, SI_writeEachSummaryHtml );
	proc_all_ele_in_cont( this );
	  fprintf( ffp, "</table>\n" );
		fprintf( ffp, "</td>\n"\
        "</tr>\n"\
        "</table>\n"\
        "</td>\n"\
	    "</tr>\n");
		fprintf(ffp,"<tr height=15><td></td></tr>"\
        "</table>");
     }
	 else  
	 {
		if(this->callback_content_z != NULL)
		{
	      this->callback_content_z(this);
		}
     	this->callback_content(this);
	 }
	
	
    //矩形框的底部和总框架的结束
    
	
 /******************************Fillet rectangular bottom********************************/		  
		  fprintf(ffp,"</td>"\
		  "<td width=1 bgcolor=#3f3f3f></td>"\
		  "</tr>"\
		  "</tbody>"\
		  "</table></td>"\
		  "</tr>"\
		  "<tr height=3 width=%s>","100%");
		  fprintf(ffp,"<td><table style=\"table-layout: fixed\" height=3 cellspacing=0 cellpadding=0 width=%s border=0>","100%");
		  fprintf(ffp,"<tbody>"\
		  "<tr height=1>"\
		  "<td width=1></td>"\
		  "<td width=1 bgcolor=#3f3f3f></td>"\
		  "<td width=1 bgcolor=#ffffff></td>"\
		  "<td bgcolor=#ffffff></td>"\
		  "<td width=1 bgcolor=#ffffff></td>"\
		  "<td width=1 bgcolor=#3f3f3f></td>"\
		  "<td width=1></td>"\
		  "</tr>"\
		  "<tr height=1>"\
		  "<td></td>"\
		  "<td bgcolor=#3f3f3f colspan=2></td>"\
		  "<td bgcolor=#ffffff></td>"\
		  "<td bgcolor=#3f3f3f colspan=2></td>"\
		  "<td></td>"\
		  "</tr>"\
		  "<tr height=1>"\
		  "<td colspan=3></td>"\
		  "<td bgcolor=#3f3f3f></td>"\
		  "<td colspan=3></td>"\
		  "</tr>"\
		  "</tbody>"\
		  "</table></td>"\
		  "</tr>"\
		  "</tbody>"\
		  "</table>");	
	      /****************************end Fillet rectangular bottom******************************/

		  fprintf(ffp,"</td>"\
			"</tr>"\
          "</table>"\
        "</td>"\
        "<td width=15 background=/images/di999.jpg>&nbsp;</td>"\
      "</tr>"\
    "</table></td>"\
  "</tr>"\
  "<tr>"\
    "<td colspan=3 align=left valign=top background=/images/di777.jpg><img src=/images/di555.jpg width=61 height=62/></td>"\
    "<td align=left valign=top background=/images/di777.jpg>&nbsp;</td>"\
    "<td align=left valign=top background=/images/di777.jpg><img src=/images/di666.jpg width=74 height=62/></td>"\
  "</tr>"\
"</table>"\
"</div>"\
"</form>"\
"</body>"\
"</html>");
return 0;
}


//辅助创建二级页面的函数
STSndrContainer *create_sndr_module_container_helper( STPubInfoForItem *p_pubinfo, STSCCreateHelper pstSCCreateHelper[], int num )
{
	int i;
	STSndrContainer *pstSCRet=NULL;
	STSndrItem *pstSICur=NULL;
	
	for( i=0; i<num; i++ )
	{
		if( NULL != pstSCCreateHelper[i].fill_data_api )
		{
			pstSICur = create_sndr_module_item();
			
			if( NULL == pstSICur )
			{
				break;
			}
			
			if( 0 == pstSCCreateHelper[i].fill_data_api( p_pubinfo,pstSICur) )
			{
				if( NULL == pstSCRet )	
				{
					pstSCRet = create_sndr_module_container();
					if( NULL == pstSCRet )
					{
						break;
					}
				}
				SC_add_item( pstSCRet, pstSICur );
			}
			else
			{
				//printf("error!\n");
				release_sndr_module_item(pstSICur);
			}
		}
	}
	
	return pstSCRet;
}

//是否显示链接图片 /devinfo/snmp_sys_oid  4526 是的话: flag=0  否则 flag=1
int if_show_sndr_icon()
{
int flag = 1 ;
char content[20];
memset(content,0,20);

char cmd[256]="";        	
sprintf( cmd, "cat /devinfo/enterprise_snmp_oid" );
test(content,sizeof(content),cmd);
int num;
num=strtoul(content,0,10);
if(num==4526)
	flag=0;

return flag;
}


