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
* ws_license.c
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
#include "ws_license.h"
#include <libxml/parser.h>
#include <libxml/tree.h>
#include <libxml/xpathInternals.h>
#include <string.h>
#include <sys/wait.h>
#include <unistd.h>
/*#include "cgic.h"*/
#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>

#include "ws_y3des.h"  //引入加密函数

//根据所发的license 文件来判断相应功能是否开放
int get_license_state( int module )
{
    return  module;    
}

int get_license_state_new( int module ,char *Src)
{

	//获取内容处理，默认的是全局开
	int flag=-1,op_ret=-1;

	int filesize=1024,op_flag=-1;	
	op_flag=get_max_size_file(LICENSE_FILE);
	if(op_flag != 0)
	filesize=op_ret+200;

	char *zstring=(char *)malloc(filesize);
	memset(zstring,0,filesize);

	strcpy(zstring,Src);

	if( strcmp(Src,"")!=0 && strlen(Src) > 0 )
	{
		op_ret=encry_oid();
	}
	else
	{
		if(get_decrypt_content()!=NULL)
		{
			strcpy(zstring,get_decrypt_content());
			op_ret=encry_oid();
		}
		else
		op_ret=1;

	}
	if(op_ret==0)
	{

		if(module==SYSTEM_ITEM)
		{
			if(strstr(zstring,"system:0")==NULL)
			flag=1;
			else
			flag=0;
		} 
		else if(module==CONTRL_ITEM)
		{
			if(strstr(zstring,"contrl:0")==NULL)
			flag=1;
			else
			flag=0;
		}              
		else if(module==WLAN_ITEM)
		{
			if(strstr(zstring,"wlan:0")==NULL)
			flag=1;
			else
			flag=0;
		}
		else if(module==AUTH_ITEM)
		{
			if(strstr(zstring,"auth:0")==NULL)
			flag=1;
			else
			flag=0;
		}
		else if(module==FIREWALL_ITEM)
		{
			if(strstr(zstring,"firewall:0")==NULL)
			flag=1;
			else
			flag=0;
		}
		else if(module==SYS_ITEM)
		{
			if(strstr(zstring,"sysinfo:0")==NULL)
			flag=1;
			else
			flag=0;
		}
		else if(module==HELP_ITEM)
		{
			if(strstr(zstring,"help:0") ==NULL)
			flag=1;
			else
			flag=0;
		}
	}
	else
	flag=0;

	free(zstring);
	return flag;

}

//获取并根据密钥加密本设备机器码，并进行比较，相同返回0，不同返回1
int encry_oid()
{
	int flag=-1;
	FILE *fp;
	int iLen=0;

	int filesize=1024,op_ret=-1;	
	op_ret=get_max_size_file(LICENSE_TEMP);
	if(op_ret != 0)
	filesize=op_ret+200;


	char *content=(char *)malloc(filesize);
	char  *zone=(char *)malloc(filesize);  //文本中总的内容   
	char *key=(char *)malloc(filesize);
	char *anylength=(char *)malloc(filesize);  //加密后的机器码的内容

	memset(content,0,filesize);
	memset(zone,0,filesize);
	memset(key,0,filesize);
	memset(anylength,0,filesize);

	char buff[128];
	memset(buff,0,128);


	char key2[50];
	memset(key2,0,50);



	//通过一个函数来得到设备的机器码
	char srcmc[128];
	memset(srcmc,0,128);
	strcpy(srcmc,get_machine_code());

	if((fp=fopen(LICENSE_TEMP,"r"))==NULL)   
	{  
		free(content);
		free(zone);
		free(anylength);
		free(key);
		return -2;  //不能打开此文件
	}   

	while(!feof(fp))   
	{   
		fgets(buff,128,fp);   
		strcat(zone,buff);
	}   
	fclose(fp);

	/////对文本进行切割

	char *p;
	p=strtok(zone,"<");
	if(p)  
	{
		memcpy(anylength,p,strlen(p));
		p=strtok(NULL,"<");   	 
		memcpy(key,p,strlen(p));
	}

	memcpy(key2,key,strlen(key)<8?strlen(key):8);
	ymDES2_InitializeKey(key2,0);


	char *bitsCiphertextAnyLength = NULL,*hexCiphertextAnyLength=NULL;     
	iLen=(strlen(srcmc) % 8 == 0 ? strlen(srcmc) << 3 : ((strlen(srcmc)>>3) + 1) << 6);


	bitsCiphertextAnyLength = (char *)malloc( (iLen+10)*sizeof(char) );
	hexCiphertextAnyLength  = (char *)malloc( (iLen+10)>>2 );

	memset(bitsCiphertextAnyLength,0,(iLen+10)*sizeof(char));
	memset(hexCiphertextAnyLength,0,(iLen+10)>>2);

	if( NULL == bitsCiphertextAnyLength )
	{
		free(content);
		free(zone);
		free(anylength);
		free(key);
		free(bitsCiphertextAnyLength);
		free(hexCiphertextAnyLength);
		return -2;
	}
	if( NULL == hexCiphertextAnyLength )
	{
		free(content);
		free(zone);
		free(anylength);
		free(key);
		free(bitsCiphertextAnyLength);
		free(hexCiphertextAnyLength);
		return -3;
	}

	ymDES2_EncryptAnyLength(srcmc,strlen(srcmc),0);	

	ymDES2_Bytes2Bits(ymDES2_GetCiphertextAnyLength(),bitsCiphertextAnyLength,iLen);
	ymDES2_Bits2Hex(hexCiphertextAnyLength,bitsCiphertextAnyLength,iLen);	

	if(strcmp(anylength,hexCiphertextAnyLength)==0)
	flag=0;

	free(bitsCiphertextAnyLength);
	free(hexCiphertextAnyLength);
	free(content);
	free(zone);
	free(anylength);
	free(key);

	return flag;
}

//首次加密后的内容分块输出,开放功能为明文
char * get_decrypt_content()
{
	int iLen = 0;
	FILE *mc;

	int filesize=1024,op_ret=-1;	
	op_ret=get_max_size_file(LICENSE_FILE);
	if(op_ret != 0)
	filesize=op_ret+200;


	char *content=(char *)malloc(filesize);
	char  *zone=(char *)malloc(filesize);  //文本中总的内容   
	char *key=(char *)malloc(filesize);
	char *anylength=(char *)malloc(filesize);  //加密后的机器码的内容

	memset(content,0,filesize);
	memset(zone,0,filesize);
	memset(key,0,filesize);
	memset(anylength,0,filesize);

    char dd[filesize];
	memset(dd,0,filesize);

	char buff[128];
	memset(buff,0,128);

	char key2[50];
	memset(key2,0,50);


	if((mc=fopen(LICENSE_FILE,"r"))==NULL)   
	{  
		free(content);
		free(zone);
		free(anylength);
		free(key);
		return NULL;  //不能打开此文件
	}   

	while(!feof(mc))   
	{   
		fgets(buff,128,mc);   
		strcat(zone,buff);
	}   
	fclose(mc);

	/////对文本进行切割，防止文件中没有分隔符	  
	if(strstr(zone,">")!=NULL)
	{
		char *p;
		p=strtok(zone,">");
		if(p)  
		{    
			memcpy(anylength,p,strlen(p));
			p=strtok(NULL,">");  			 
			memcpy(key,p,strlen(p));

		}    
	}
	else
	{
		free(content);
		free(zone);
		free(anylength);
		free(key);
		return NULL;
	}




	//对文件进行首次的解密,防止key2非值
	FILE *fp3 = fopen(LICENSE_TEMP,"w+");

	char *bitsCiphertextAnyLength = NULL;    

	memcpy(key2,key,strlen(key)<8?strlen(key):8);
	ymDES2_InitializeKey(key2,0);

	iLen = ((strlen(anylength)>>2) + (strlen(anylength) % 4 == 0 ? 0 : 1))<<4;
	bitsCiphertextAnyLength = (char *)malloc( (iLen+10)*sizeof(char) );
	memset(bitsCiphertextAnyLength,0,(iLen+10)*sizeof(char));

	if( NULL == bitsCiphertextAnyLength )
	{
		free(content);
		free(zone);
		free(anylength);
		free(key);
		free(bitsCiphertextAnyLength);
		if(fp3 != NULL)
		{
			fclose(fp3);
		}
		return NULL;
	}
	

	ymDES2_Hex2Bits(anylength,bitsCiphertextAnyLength,iLen);	  
	ymDES2_Bits2Bytes( ymDES2_GetCiphertextAnyLength(),bitsCiphertextAnyLength,iLen);


	//解密任何长度的字符串，存放到明文区
	ymDES2_DecryptAnyLength(ymDES2_GetCiphertextAnyLength(),iLen>>3,0);

	//将机器码(加密后)存放到一个临时文件中，与本设备的机器码加密后想比较，始终不出现机器码的明文
	memset(content,0,filesize);

	/////再次进行切割取出等号中间的内容
	if(strstr(ymDES2_GetPlaintextAnyLength(),"==")!=NULL)
	{
		char *q;  
		q=strtok(ymDES2_GetPlaintextAnyLength(),"==");
		if(q)     	
		{
			memcpy(content,q,strlen(q));
			q=strtok(NULL,"=="); 
			memcpy(dd,q,strlen(q));
		}
	}
	else
	{
		free(content);
		free(zone);
		free(anylength);
		free(key);
		free(bitsCiphertextAnyLength);
		if(fp3 != NULL)
		{
			fclose(fp3);
		}
		return NULL;
	}

	if(fp3 != NULL)
	{
		fwrite(content,strlen(content),1,fp3);		  
		fclose(fp3);
	}

	free(content);
	free(zone);
	free(anylength);
	free(key);
	free(bitsCiphertextAnyLength);  //注意防止发生问题

	return dd;
}

//get machine code 
char *get_machine_code()
{
static char mc[128];
strcpy(mc,"001F641C0001");
return mc;
}

//get file max size
int get_max_size_file(char *fpath)
{
	FILE *fp;
	char cmd[128];
	memset(cmd,0,128);

	int size=0;

	sprintf(cmd,"wc -c %s",fpath);

	char buff[128];
	memset(buff,0,128);

	if(access(fpath,0)==0)
	{
		fp=popen(cmd,"r");
		if(fp != NULL)
		{
			fgets( buff, sizeof(buff), fp );  //很重要 ，不然与条目不匹配
		}


		char *p;

		p=strtok(buff," ");
		if(p)
		size=atoi(p);

		if(fp != NULL)
		{
			pclose(fp);
		}
	}

	return size;
}

