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
* ws_conf_engine.c
*
*
* CREATOR:
* autelan.software.Network Dep. team
* 
*
* DESCRIPTION: 
*
*
*******************************************************************************/
/*
* Copyright (c) 2008,Autelan - AuteCS
* All rights reserved.
*
*$Source: /rdoc/AuteCS/cgic205/portal/ws_conf_engine.c,v $
*$Author: zhouyanmei $
*$Date: 2010/02/23 06:07:59 $
*$Revision: 1.6 $
*$State: Exp $
*
*
*/


#include "ws_conf_engine.h"
#include <stdlib.h>
#include <string.h>
/***************************************************************
*USEAGE:	装载配置文件
*Param:		file_path  ->  配置文件的路径
			pt_conf_item ->所有的配置的数组，这个数组是在上层定义的一个全局变量，当有新的配置需要添加到文件中是，只需要在数组中添加好默认配置就可以了 。
			max_num ->  item 数组的个数
*Return:	0 -> 装载成功
			!0 -> 失败*			
*Auther:shao jun wu
*Date:2009-1-16 17:00:20
*Modify:(include modifyer,for what resease,date)
****************************************************************/
//装载conf文件
t_conf_item *find_conf_item_by_name( char *name, t_conf_item pt_conf_item[], int max_num )
{
	int i=0;
	
	for( i=0; i<max_num; i++ )
	{
		if( strlen(name)==strlen(pt_conf_item[i].conf_name) && strcmp( name, pt_conf_item[i].conf_name ) == 0 )
		{
			return &(pt_conf_item[i]);
		}
	}
	return NULL;
}

#define MIN(a,b) ((a)>(b))?(b):(a)
	
int load_conf_file( char *file_path, t_conf_item pt_conf_item[], int max_num )
{
	FILE *fp;
	int i;
	char line[MAX_CONF_NAME_LEN+MAX_CONF_VALUE_LEN+20]="";
	char *div;
	t_conf_item *p_cur_item;
	char name[MAX_CONF_NAME_LEN+1];
		
	fp = fopen( file_path, "r" );
	if( NULL == fp )
	{
		//fprintf( stderr, "load_conf_file    file %s open err!\n", file_path );
		return RTN_ERR_FILE_OPEN;	
	}	

	//fprintf( stderr, "load_conf_file    file %s open ok!\n", file_path );
	while( !feof(fp) )
	{
		fgets( line, sizeof(line), fp )	;
		
		//fprintf( stderr, "load_conf_file    get line = %s\n", line );
		for(i=0;i<strlen(line);(0x0a==line[i]||0x0d==line[i])?line[i]=0:0,i++);//去掉末尾的回车换行符号
		div=strchr( line, '=' );//
		if( NULL == div )
		{
			continue;	
		}
		memset( name, 0, sizeof(name) );
		strncpy( name, line, MIN(div-line,sizeof(name)) );
		//fprintf( stderr, "load_conf_file    name = %s\n", name );
		p_cur_item = find_conf_item_by_name( name, pt_conf_item, max_num );
		if( NULL == p_cur_item )
		{
			continue;	
		}
		//fprintf( stderr, "load config %s = %s\n", p_cur_item->conf_name, div+1 );
		strncpy( p_cur_item->conf_value, div+1, sizeof(p_cur_item->conf_value) );
	}
	
	
	return 0;
}


/***************************************************************
*USEAGE:	保存配置文件
*Param:		file_path  ->  配置文件的路径
			pt_conf_item ->所有的配置的数组，这个数组是在上层定义的一个全局变量，当有新的配置需要添加到文件中是，只需要在数组中添加好默认配置就可以了 。
			max_num ->  item 数组的个数
*Return:	0 -> 保存成功
			!0 -> 失败			
*Auther:shao jun wu
*Date:2009-1-16 17:00:20
*Modify:(include modifyer,for what resease,date)
****************************************************************/
//保存conf文件
int save_conf_file( char *file_path, t_conf_item pt_conf_item[], int max_num )
{
	int i;
	FILE *fp;
	char line[MAX_CONF_NAME_LEN+MAX_CONF_VALUE_LEN+20]="";
	
	fp = fopen( file_path, "w+" );
	if( NULL == fp )
	{
		//fprintf( stderr, "save_conf_file    file %s open err!\n", file_path );
		return RTN_ERR_FILE_OPEN;	
	}
	
	for( i=0; i<max_num; i++ )
	{
		memset( line, 0, sizeof(line) );
		if( strlen(pt_conf_item[i].conf_name)==0 || strlen(pt_conf_item[i].conf_value)==0 )
		{
			break;
		}
		snprintf( line,	sizeof(line), "%s=%s",pt_conf_item[i].conf_name,pt_conf_item[i].conf_value );
		//fprintf( stderr, "save_conf_file line=%s\n", line );
		fprintf( fp, "%s\n", line );
	}
	
	fclose( fp );
	fp=NULL;
	
	return 0;	
}


/***************************************************************
*USEAGE:	开启服务
*Param:		
*Return:	0 -> 开启成功
			!0 -> 失败			
*Auther:shao jun wu
*Date:2009-2-4 17:10:22
*Modify:(include modifyer,for what resease,date)
****************************************************************/
static int docommand( char *command )
{
	int status;
	int iRet;
	
	status = system(command);
	iRet = WEXITSTATUS(status);	
	
	//fprintf( stderr, "docommandxxx %s  ret=%d\n", command, iRet);
	return iRet;
}

int eag_services_start()
{
	return docommand( SCRIPT_FILE_PATH" start >/dev/null 2>&1" );
}

/***************************************************************
*USEAGE:	关闭eag服务
*Param:		
*Return:	0 -> 关闭成功
			!0 -> 失败			
*Auther:shao jun wu
*Date:2009-2-4 17:10:41
*Modify:(include modifyer,for what resease,date)
****************************************************************/
int eag_services_stop( )
{
	return docommand( SCRIPT_FILE_PATH" stop >/dev/null 2>&1" );
}


/***************************************************************
*USEAGE:	重启eag服务
*Param:		
*Return:	0 -> 关闭成功
			!0 -> 失败			
*Auther:shao jun wu
*Date:2009-2-4 17:10:41
*Modify:(include modifyer,for what resease,date)
****************************************************************/
int eag_services_restart()
{
	return docommand( SCRIPT_FILE_PATH" restart >/dev/null 2>&1" );
}

/***************************************************************
*USEAGE:	获得服务状态
*Param:		
*Return:	0 -> 未开启
			!0 -> 开启		
*Auther:shao jun wu
*Date:2009-2-4 17:23:30
*Modify:(include modifyer,for what resease,date)
****************************************************************/
int eag_services_status()
{
	FILE *fp;
	char status[12];
	int iRet=0;
	
	fp = fopen( EAG_STATUS_FILE, "r" );
	if( NULL != fp )
	{
		
		fgets( status, sizeof(status), fp );
		if( strncmp( status, "start", strlen("start") ) == 0 )
		{
			iRet = 1;	
		}
		fclose( fp );
	}

	return iRet;
}
/***********************************/
//author :liutao
/********************************/
int idle_tick_status()
{
	
	char * cmd [256];
	int iRet=0;
	FILE *fp;
	char status[12];
	strcpy(cmd,"cat /opt/services/conf/eag_conf.conf|grep \"HS_STATUS_KICK\"|awk 'BEGIN {FS=\"=\"}{print $2}'");
	fprintf(stderr,"cmd===%s\n",cmd);
	if( (fp=popen(cmd,"r")) !=NULL)
		{
			fgets(status,sizeof(status),fp);
			fprintf(stderr,"status===%s\n",status);
			if( strncmp( status, "start", strlen("start") ) == 0 )
				{
					iRet = 1;	
				}
		pclose( fp );
		}
	
	fprintf(stderr,"iRet===%d\n",iRet);
	return iRet;

}

