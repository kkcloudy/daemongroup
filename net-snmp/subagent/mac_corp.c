
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "mac_corp.h"


#define MAC_CORP_FILE_PATH	"/opt/www/htdocs/mac_corp.txt"
//#define MAC_CORP_FILE_PATH	"mac_corp.txt"

#define MAX_LINE_LEN	1024
#define MAX_LINE_NUM	20000
#define CMP_MAC_HEADER_LEN	8

static int line_num=0;
static char *line[MAX_LINE_NUM]={NULL};
static char *content=NULL;

int init_mac_corp_data()
{
	FILE *fp = NULL;
	int cbuff_len = 0;
	char *temp=NULL;
	
	if( NULL != content )
	{
		return 0;
	}
	memset( line, 0, sizeof(line) );
	line_num = 0;
	
	fp = fopen( MAC_CORP_FILE_PATH, "r" );
	if( NULL == fp )
	{
		return -1;
	}
	
	fseek( fp, 0, SEEK_END );
	cbuff_len = ftell(fp);
	if( cbuff_len <= 0 )
	{
		fclose( fp );
		fp = NULL;
		return -2;
	}
	
	cbuff_len += 10;
	content = (char *)malloc(cbuff_len);/**/
	if( NULL == content )
	{
		fclose( fp );
		fp = NULL;
		return -3;
	}
	memset( content, 0, cbuff_len );
	
	fseek( fp, 0, SEEK_SET );
	if( fread( content, 1, cbuff_len-1, fp ) <= 0 )
	{
		fclose( fp );
		fp = NULL;
		return -4;	
	}
	
	fclose( fp );
	fp = NULL;
	
	/*format line to line array!*/
	temp = content;
	line_num = 0;
	if( NULL != temp && *temp != 0 )
	{
		line[line_num] = temp;
		line_num++;
		do{
			temp++;
			if( *temp == 0x0a || *temp == 0x0d )
			{
				*temp = 0;
				temp++;
				if( *temp == 0x0a || *temp == 0x0d )
				{
					*temp = 0;
					temp++;		
				}
				line[line_num] = temp;
				line_num++;
			}
		}while( *temp != 0 );
	}
	else
	{
		return -5;	
	}
	
	return 0;
}




int get_corp_by_mac( char *mac, char *corp, int buff_len )
{
	int min,max=0;
	int middle=0;
	int cmp=0;
	int getline = -1;
	char *temp = NULL;
	
	if( NULL == mac || NULL == corp || buff_len <= 0)
	{
		return -1;
	}
	/*使用二分查找,因为默认的数据是排序的*/
	min = 0;
	max = line_num-1;
	middle = (min + max)/2;
	while( min != middle )
	{
		cmp = strncmp( mac, line[middle], CMP_MAC_HEADER_LEN );
		if(  cmp < 0 )
		{
			max = middle;
		}
		else if( cmp > 0 )
		{
			min = middle;	
		}
		else
		{
			getline = middle;
			break;
		}
		middle = (min + max)/2;
	}
	
	if( getline < 0 )
	{
		if( strncmp( mac, line[min], CMP_MAC_HEADER_LEN ) ==  0 )
		{
			getline = min;
		}
		else if ( strncmp( mac, line[max], CMP_MAC_HEADER_LEN ) ==  0 )
		{
			getline = max;
		}
	}
	else if( strncmp( mac, line[getline], CMP_MAC_HEADER_LEN ) !=  0 )	/*绝对查找，最后需要比较一次*/
	{
		getline = -1;	
	}
	
	if( getline >= 0 && getline < line_num )
	{
		memset( corp, 0, sizeof(buff_len) );
		temp = strchr( line[getline], '#' );
		if(temp)
		{
			temp++;
			strncpy( corp, temp , buff_len-1 );
		}
	}
	
	return getline;
}

int destroy_mac_corp_data()
{
	if( NULL != content )
	{
		free( content );
	}

	content = NULL;
	return 0;
}

#if 0
int main()
{
	char corp[1024]="";
	
	init_mac_corp_data();

	memset( corp, 0, sizeof(corp) );
	get_corp_by_mac( "00-00-00-AA-BB-CC", corp, sizeof(corp) );
	printf( "00-00-00-AA-BB-CC corp = %s\n", corp );
	
	memset( corp, 0, sizeof(corp) );
	get_corp_by_mac( "00-00-01-AA-BB-CC", corp, sizeof(corp) );
	printf( "00-00-01 corp = %s\n", corp );
		
	memset( corp, 0, sizeof(corp) );
	get_corp_by_mac( "00-F0-01-AA-BB-CC", corp, sizeof(corp) );
	printf( "00-F0-01 corp = %s\n", corp );

	memset( corp, 0, sizeof(corp) );
	get_corp_by_mac( "F0-00-01-AA-BB-CC", corp, sizeof(corp) );
	printf( "F0-00-01 corp = %s\n", corp );


	memset( corp, 0, sizeof(corp) );
	get_corp_by_mac( "FC-FA-F7-AA-BB-CC", corp, sizeof(corp) );
	printf( "FC-FA-F7 corp = %s\n", corp );
	

	memset( corp, 0, sizeof(corp) );
	get_corp_by_mac( "FC-FB-F8-AA-BB-CC", corp, sizeof(corp) );
	printf( "FC-FA-F8 corp = %s\n", corp );
	
	memset( corp, 0, sizeof(corp) );
	get_corp_by_mac( "FC-FB-FB-AA-BB-CC", corp, sizeof(corp) );
	printf( "FC-FB-FB-AA-BB-CC corp = %s\n", corp );
	


	memset( corp, 0, sizeof(corp) );
	get_corp_by_mac( "00-10-B6-AA-BB-CC", corp, sizeof(corp) );
	printf( "00-10-B6-AA-BB-CC corp = %s\n", corp );	
	

	memset( corp, 0, sizeof(corp) );
	get_corp_by_mac( "40-61-8E-AA-BB-CC", corp, sizeof(corp) );
	printf( "40-61-8E-AA-BB-CC corp = %s\n", corp );	


	memset( corp, 0, sizeof(corp) );
	get_corp_by_mac( "18-FC-9F-AA-BB-CC", corp, sizeof(corp) );
	printf( "18-FC-9F-AA-BB-CC corp = %s\n", corp );
	
	destroy_mac_corp_data();
	
	
	
	printf("\n\n\n");
	memset( corp, 0, sizeof(corp) );
	get_corp_by_mac_ext( "18-FC-9F-AA-BB-CC", corp, sizeof(corp) );
	printf( "18-FC-9F-AA-BB-CC corp = %s\n", corp );
	
	printf("\n\n\n");
	memset( corp, 0, sizeof(corp) );
	get_corp_by_mac_ext( "FC-FB-FB-AA-BB-CC", corp, sizeof(corp) );
	printf( "FC-FB-FB-AA-BB-CC corp = %s\n", corp );
	
	printf("\n\n\n");
	memset( corp, 0, sizeof(corp) );
	get_corp_by_mac_ext( "00-00-00-AA-BB-CC", corp, sizeof(corp) );
	printf( "00-00-00-AA-BB-CC corp = %s\n", corp );		
}

#endif
