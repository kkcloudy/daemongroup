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
* ACConfigFile.c
*
*
* CREATOR:
* autelan.software.wireless-control. team
*
* DESCRIPTION:
* wid module
*
*
*******************************************************************************/


#include "CWAC.h"
#include "ACDbus.h"

#include "wcpss/wid/WID.h"
#include <libxml/tree.h>
#include <libxml/xpath.h>
#include <sys/types.h>
#include <unistd.h>
#include <sys/wait.h>
#ifdef DMALLOC
#include "../dmalloc-5.5.0/dmalloc.h"
#endif
//added by weiay 20080618
#define AP_MODEL "</AP_SW_MODEL>"
#define AP_VERSION_NAME "</AP_SW_VERSION_NAME>"
#define AP_VERSION_PATH "</AP_SW_VERSION_IMAGE_PATH>"
#define AP_RADIO_NUM "</AP_RADIO_NUM>"
#define AP_RADIO_POWER "</AP_RADIO_POWER>"
#define AP_BSS_NUM "</AP_BSS_NUM>"
#define AP_RADIO_TYPE  "</AP_RADIO_TYPE>"
#define AP_SW_CODE "</AP_SW_CODE>"

#define ACPROTOCOL "IPv6"

const char *CW_CONFIG_VERSION_FILE = "/etc/version/wtpcompatible";
const char *CW_CONFIG_VERSION_FILE_FIRST = "/mnt/wtp/wtpcompatible";
const char *CW_CONFIG_VERSION_FILE_XML = "/etc/version/wtpcompatible.xml";
const char *CW_CONFIG_VERSION_FILE_XML_FIRST = "/mnt/wtp/wtpcompatible.xml";

const char *CW_BLACK_OUI_LIST_FILE_XML = "/etc/version/blackouilist.xml";
const char *CW_BLACK_OUI_LIST_FILE_XML_FIRST = "/mnt/wtp/blackouilist.xml";
const char *CW_WHITE_OUI_LIST_FILE_XML = "/etc/version/whiteouilist.xml";
const char *CW_WHITE_OUI_LIST_FILE_XML_FIRST = "/mnt/wtp/whiteouilist.xml";
/*mahz add for path of apimg.xml for temporary use*/
char *CW_CONFIG_APVER_FILE = "/etc/version/apimg";
char *CW_CONFIG_APVER_FILE_FIRST = "/mnt/wtp/apimg";
char *CW_CONFIG_APVER_FILE_XML = "/etc/version/apimg.xml";
char *CW_CONFIG_APVER_FILE_XML_FIRST = "/mnt/wtp/apimg.xml";

const char *CW_11N_RATE_FILE = "/etc/version/11nratetable.xml";//add by book, 2010-10-20
const char *CW_CONFIG_FILE = "config.ac";

CWBool CWConfigFileInitLib() {
	gConfigValuesCount = 11;

	CW_CREATE_ARRAY_ERR(gConfigValues, gConfigValuesCount, CWConfigValue, return CWErrorRaise(CW_ERROR_OUT_OF_MEMORY, NULL););
	
	gConfigValues[0].type = CW_INTEGER;
	gConfigValues[0].code = "</AC_HW_VERSION> ";
	gConfigValues[0].value.int_value = 0;
	
	gConfigValues[1].type = CW_INTEGER;
	gConfigValues[1].code = "</AC_SW_VERSION> ";
	gConfigValues[1].value.int_value = 0;
	
	gConfigValues[2].type = CW_INTEGER;
	gConfigValues[2].code = "</AC_MAX_STATIONS> ";
	gConfigValues[2].value.int_value = 0;
	
	gConfigValues[3].type = CW_INTEGER;
	gConfigValues[3].code = "</AC_MAX_WTPS> ";
	gConfigValues[3].value.int_value = 0;
	
	gConfigValues[4].type = CW_STRING;
	gConfigValues[4].code = "</AC_SECURITY> ";
	gConfigValues[4].value.str_value = NULL;
	
	gConfigValues[5].type = CW_STRING;
	gConfigValues[5].code = "</AC_NAME> ";
	gConfigValues[5].value.str_value = NULL;
	
	gConfigValues[6].type = CW_STRING_ARRAY;
	gConfigValues[6].code = "<AC_MCAST_GROUPS>";
	gConfigValues[6].endCode = "</AC_MCAST_GROUPS>";
	gConfigValues[6].value.str_array_value = NULL;
	gConfigValues[6].count = 0;
	
	gConfigValues[7].type = CW_INTEGER;
	gConfigValues[7].code = "</AC_FORCE_MTU> ";
	gConfigValues[7].value.int_value = 0;
	
	gConfigValues[8].type = CW_STRING;
	gConfigValues[8].code = "</AC_LEV3_PROTOCOL> ";
	gConfigValues[8].value.str_value = NULL;

	gConfigValues[9].type = CW_INTEGER;
	gConfigValues[9].code = "</AC_LOG_FILE_ENABLE> ";
	gConfigValues[9].value.int_value = 0;

	gConfigValues[10].type = CW_INTEGER;
	gConfigValues[10].code = "</AC_LOG_FILE_SIZE> ";
	gConfigValues[10].value.int_value = DEFAULT_LOG_SIZE;
	
	return CW_TRUE;
}


CWBool CWConfigFileInitLib1() {
	gConfigValuesCount = 11;
	int len;
	CW_CREATE_ARRAY_ERR(gConfigValues, gConfigValuesCount, CWConfigValue, return CWErrorRaise(CW_ERROR_OUT_OF_MEMORY, NULL););

		{
			
		}
	//parse hw_version
	{
		char Hw_version[] = "/devinfo/product_name";
		char Hw_version_name[DEFAULT_LEN];
		memset(Hw_version_name,0,DEFAULT_LEN);
		if(read_ac_info(Hw_version,Hw_version_name) == 1)
		{
			//not find 
			gConfigValues[0].type = CW_INTEGER;
			gConfigValues[0].code = "</AC_HW_VERSION> ";
			gConfigValues[0].value.int_value = 123456;
		}
		else
		{	
			//find info
			gConfigValues[0].type = CW_STRING;;
			gConfigValues[0].code = "</AC_HW_VERSION> ";
			
			len = strlen(Hw_version_name);
			gConfigValues[0].value.str_value = (char*)WID_MALLOC(len+1);
			if (NULL == gConfigValues[0].value.str_value)
			{
				goto label_gconfigvalues;
			}
			memset(gConfigValues[0].value.str_value,0,len+1);
			memcpy(gConfigValues[0].value.str_value,Hw_version_name,len);
		}
	}
	//gConfigValues[0].type = CW_INTEGER;
	//gConfigValues[0].code = "</AC_HW_VERSION> ";
	//gConfigValues[0].value.int_value = 123456;
	
	//parse sw_version
	{
		char Sw_version[] = "/devinfo/software_name";
		char Sw_version_name[DEFAULT_LEN];
		memset(Sw_version_name,0,DEFAULT_LEN);
		if(read_ac_info(Sw_version,Sw_version_name) == 1)
		{
			//not find 
			gConfigValues[1].type = CW_INTEGER;
			gConfigValues[1].code = "</AC_SW_VERSION> ";
			gConfigValues[1].value.int_value = 123456;
		}
		else
		{	
			//find info
			gConfigValues[1].type = CW_STRING;;
			gConfigValues[1].code = "</AC_SW_VERSION> ";
			
			len = strlen(Sw_version_name);
			gConfigValues[1].value.str_value = (char*)WID_MALLOC(len+1);
			if (NULL == gConfigValues[1].value.str_value)
			{
				goto lable_gconfigvalues_str_value;
			}
			memset(gConfigValues[1].value.str_value,0,len+1);
			memcpy(gConfigValues[1].value.str_value,Sw_version_name,len);
		}
	}
	//gConfigValues[1].type = CW_INTEGER;
	//gConfigValues[1].code = "</AC_SW_VERSION> ";
	//gConfigValues[1].value.int_value = 123456;

	//parse AC_MAX_WTPS
	{
		int wtpnum=0;
		char *wtpnum_char;
		char wtp_path[] = "/devinfo/maxwtpcount";
		char wtp_count[DEFAULT_LEN];
		memset(wtp_count,0,DEFAULT_LEN);
		if(read_ac_info(wtp_path,wtp_count) == 1)
		{
			//not find 
			gConfigValues[3].type = CW_INTEGER;
			gConfigValues[3].code = "</AC_MAX_WTPS> ";
			gConfigValues[3].value.int_value = WTP_NUM;//sz change 1023 from 1024,because of bug autelan-1000
		}
		else
		{	
			//find info
			gConfigValues[3].type = CW_INTEGER;;
			gConfigValues[3].code = "</AC_MAX_WTPS> ";

			len = strlen(wtp_count);
			wtpnum_char = (char*)WID_MALLOC(len+1);
			if (NULL == wtpnum_char)
			{
				goto label_wtpnum_char;
			}
			memset(wtpnum_char,0,len+1);
			memcpy(wtpnum_char,wtp_count,len);

			wtpnum = atoi(wtpnum_char);
			gConfigValues[3].value.int_value = wtpnum;
			CW_FREE_OBJECT_WID(wtpnum_char);
		}
	}
	
	gConfigValues[2].type = CW_INTEGER;
	gConfigValues[2].code = "</AC_MAX_STATIONS> ";
	//gConfigValues[2].value.int_value = 1024;
	gConfigValues[2].value.int_value = (gConfigValues[3].value.int_value)*32;
	//gConfigValues[3].type = CW_INTEGER;
	//gConfigValues[3].code = "</AC_MAX_WTPS> ";
	//gConfigValues[3].value.int_value = 1023;//sz change 1023 from 1024,because of bug autelan-1000
	
	gConfigValues[4].type = CW_STRING;
	gConfigValues[4].code = "</AC_SECURITY> ";
	len = strlen("X509_CERTIFICATE");
	gConfigValues[4].value.str_value = (char*)WID_MALLOC(len+1);
	if (NULL == gConfigValues[4].value.str_value)
	{
		goto label_wtpnum_char;
	}
	memset(gConfigValues[4].value.str_value,0,len+1);
	memcpy(gConfigValues[4].value.str_value,"X509_CERTIFICATE",len);

	//parse ac name
	{
		char ac_name_path[] = "/devinfo/enterprise_name";
		char ac_name[DEFAULT_LEN];
		memset(ac_name,0,DEFAULT_LEN);
		if(read_ac_info(ac_name_path,ac_name) == 1)
		{
			//not find 
			gConfigValues[5].type = CW_STRING;
			gConfigValues[5].code = "</AC_NAME> ";
			len = strlen("Autelan AC");
			gConfigValues[5].value.str_value = (char*)WID_MALLOC(len+1);
			if (NULL == gConfigValues[5].value.str_value)
			{
				goto label_configvalues5_str_value;
			}
			memset(gConfigValues[5].value.str_value,0,len+1);
			memcpy(gConfigValues[5].value.str_value,"Autelan AC",len);
		}
		else
		{	
			//find info
			gConfigValues[5].type = CW_STRING;
			gConfigValues[5].code = "</AC_NAME> ";
			
			len = strlen(ac_name);
			gConfigValues[5].value.str_value = (char*)WID_MALLOC(len+1);
			if (NULL == gConfigValues[5].value.str_value)
			{
				goto label_configvalues5_str_value;
			}
			memset(gConfigValues[5].value.str_value,0,len+1);
			memcpy(gConfigValues[5].value.str_value,ac_name,len);
		}
	}
	//gConfigValues[5].type = CW_STRING;
	//gConfigValues[5].code = "</AC_NAME> ";
	//gConfigValues[5].value.str_value = NULL;	
	//len = strlen("Autelan AC");
	//gConfigValues[5].value.str_value = (char*)malloc(len+1);
	//memset(gConfigValues[5].value.str_value,0,len+1);
	//memcpy(gConfigValues[5].value.str_value,"Autelan AC",len);
	
	gConfigValues[6].type = CW_STRING_ARRAY;
	gConfigValues[6].code = "<AC_MCAST_GROUPS>";
	gConfigValues[6].endCode = "</AC_MCAST_GROUPS>";
	gConfigValues[6].value.str_array_value = NULL;
	gConfigValues[6].count = 0;
	
	gConfigValues[7].type = CW_INTEGER;
	gConfigValues[7].code = "</AC_FORCE_MTU> ";
	gConfigValues[7].value.int_value = 500;
	
	gConfigValues[8].type = CW_STRING;
	gConfigValues[8].code = "</AC_LEV3_PROTOCOL> ";
	//gConfigValues[8].value.str_value = NULL;	
	len = strlen(ACPROTOCOL);
	gConfigValues[8].value.str_value = (char*)WID_MALLOC(len+1);
	if (NULL == gConfigValues[8].value.str_value)
	{
		goto label_configvalues8_str_value;
	}
	memset(gConfigValues[8].value.str_value,0,len+1);
	memcpy(gConfigValues[8].value.str_value,ACPROTOCOL,len);

	gConfigValues[9].type = CW_INTEGER;
	gConfigValues[9].code = "</AC_LOG_FILE_ENABLE> ";
	gConfigValues[9].value.int_value = 1;

	gConfigValues[10].type = CW_INTEGER;
	gConfigValues[10].code = "</AC_LOG_FILE_SIZE> ";
	gConfigValues[10].value.int_value = DEFAULT_LOG_SIZE;
	
	return CW_TRUE;
label_configvalues8_str_value:
	CW_FREE_OBJECT_WID(gConfigValues[5].value.str_value );
label_configvalues5_str_value:
	CW_FREE_OBJECT_WID(gConfigValues[4].value.str_value);
label_wtpnum_char:
	CW_FREE_OBJECT_WID(gConfigValues[1].value.str_value);
lable_gconfigvalues_str_value:
	if (gConfigValues[0].value.str_value)
	CW_FREE_OBJECT_WID(gConfigValues[0].value.str_value);
label_gconfigvalues:
	CW_FREE_OBJECT_WID(gConfigValues);
	return CW_FALSE;
}

CWBool CWParseConfigFile1() {

	if(!(CWConfigFileInitLib1())) return CW_FALSE;
		
	#ifdef CW_DEBUGGING
		{
			int i;
			for(i = 0; i < gConfigValuesCount; i++) {
				if(gConfigValues[i].type == CW_INTEGER) {
					wid_syslog_debug_debug(WID_DEFAULT,"%s%d", gConfigValues[i].code, gConfigValues[i].value.int_value);
				}
			}
		}
		wid_syslog_debug_debug(WID_DEFAULT,"*** Config File END ***");
	#endif
	
	return CWConfigFileDestroyLib();
}

CWBool CWDistroyConfigVersionInfoXML(CWConfigVersionInfo ** VersionInfo)
{
	CWConfigVersionInfo *confignode;
	CWConfigVersionInfo *confignode1;
	confignode = *VersionInfo;
	while(confignode != NULL){
		confignode1 = confignode->next;
		CW_FREE_OBJECT_WID(confignode->str_ap_code);
		CW_FREE_OBJECT_WID(confignode->str_ap_version_name);
		CW_FREE_OBJECT_WID(confignode->str_ap_model);
		CW_FREE_OBJECT_WID(confignode->str_oem_version);
		CW_FREE_OBJECT_WID(confignode->str_ap_version_path);
		CW_FREE_OBJECT_WID(confignode);
		confignode = confignode1;
	}
	return CW_TRUE;
}

CWBool CWParseConfigVersionInfoXML(CWConfigVersionInfo ** VersionInfo)
{
	//printf("config info path /mnt/wtp/wtpcompatible.xml is used\n");
	*VersionInfo = NULL;
	gModelCount = 0;
	int i =0;
	
	xmlDocPtr doc;
	xmlNodePtr cur;
	xmlNodePtr tmp_cur;
	xmlChar *key;

	doc = xmlReadFile(CW_CONFIG_VERSION_FILE_XML_FIRST,"utf-8",256);
	if(doc == NULL)
	{
		fprintf(stderr,"Does not open the file %s,",CW_CONFIG_VERSION_FILE_XML_FIRST);
		fprintf(stderr,"may be no such file!\n");

		
		doc = xmlReadFile(CW_CONFIG_VERSION_FILE_XML,"utf-8",256);
		if(doc == NULL)
		{
			fprintf(stderr,"Does not open the file %s,",CW_CONFIG_VERSION_FILE_XML);
			fprintf(stderr,"may be no such file!\n");
			return CWParseConfigVersionInfo();
		}
	}

	cur = xmlDocGetRootElement(doc);
	if(cur == NULL)
	{
		fprintf(stderr,"empty document\n");
		xmlFreeDoc(doc);
		return CWParseConfigVersionInfo();
		
	}
	
	if (xmlStrcmp(cur->name, (const xmlChar *) "config"))
	{
		fprintf(stderr,"document of the wrong type, root node != config");
		xmlFreeDoc(doc);
		return CWParseConfigVersionInfo();
	}


	cur = cur->xmlChildrenNode;

	while(cur != NULL)
	{
		if (xmlStrcmp(cur->name, (const xmlChar *) "body") == 0)
		{
			CWConfigVersionInfo *confignode = NULL;
			CW_CREATE_OBJECT_ERR_WID(confignode, CWConfigVersionInfo, return CWErrorRaise(CW_ERROR_OUT_OF_MEMORY, NULL););	
			memset(confignode,0,sizeof(CWConfigVersionInfo));
			confignode->eth_num = 1;  //fengwenchao add 20110407
			tmp_cur = cur->xmlChildrenNode;
			
			while(tmp_cur != NULL)
			{
				//printf("%s\n",tmp_cur->name);
				if(!strcmp((char*)tmp_cur->name,"ap_sw_model"))
				{

					key = xmlNodeListGetString(doc,tmp_cur->xmlChildrenNode,1);
					if(key != NULL)
					{
						CW_CREATE_STRING_ERR_WID(confignode->str_ap_model,strlen((char*)(key)),return CWErrorRaise(CW_ERROR_OUT_OF_MEMORY, NULL););
						strcpy(confignode->str_ap_model,(char*)(key));
						xmlFree(key);
					}

				}
				#if 0
				else if(!strcmp((char*)tmp_cur->name,"ap_sw_version_name"))
				{
					key = xmlNodeListGetString(doc,tmp_cur->xmlChildrenNode,1);
					if(key != NULL)
					{
						CW_CREATE_STRING_ERR_WID(confignode->str_ap_version_name,strlen((char*)(key)),return CWErrorRaise(CW_ERROR_OUT_OF_MEMORY, NULL););
						strcpy(confignode->str_ap_version_name,(char*)(key));
						xmlFree(key);
					}	
		
				}			
				else if(!strcmp((char*)tmp_cur->name,"ap_oem_version_name"))
				{
					key = xmlNodeListGetString(doc,tmp_cur->xmlChildrenNode,1);
					if(key != NULL)
					{
						CW_CREATE_STRING_ERR_WID(confignode->str_oem_version,strlen((char*)(key)),return CWErrorRaise(CW_ERROR_OUT_OF_MEMORY, NULL););
						strcpy(confignode->str_oem_version,(char*)(key));
						xmlFree(key);
					}	
		
				}
				else if(!strcmp((char*)tmp_cur->name,"ap_sw_version_image_path"))
				{
					key = xmlNodeListGetString(doc,tmp_cur->xmlChildrenNode,1);
					if(key != NULL)
					{
						CW_CREATE_STRING_ERR_WID(confignode->str_ap_version_path,strlen((char*)(key)),return CWErrorRaise(CW_ERROR_OUT_OF_MEMORY, NULL););
						strcpy(confignode->str_ap_version_path,(char*)(key));
						xmlFree(key);
					}	
		
				}
				#endif
				/*fengwenchao add 20110407*/
				else if(!strcmp((char*)tmp_cur->name,"ap_eth_num"))
				{
					key = xmlNodeListGetString(doc,tmp_cur->xmlChildrenNode,1);
					if(key != NULL)
					{
						confignode->eth_num = atoi((char*)(key));

						xmlFree(key);
					}	
		
				}
				/*fengwenchao add end*/
				else if(!strcmp((char*)tmp_cur->name,"ap_radio_num"))
				{
					key = xmlNodeListGetString(doc,tmp_cur->xmlChildrenNode,1);
					if(key != NULL)
					{
						confignode->radio_num = atoi((char*)(key));
						xmlFree(key);
					}
					tmp_cur = tmp_cur->next;
					for(i=0; i<(confignode->radio_num); i++)
					{
						int tag = 0;
						while(tmp_cur){
							if((!strcmp((char*)tmp_cur->name,"ap_radio_type")))
							{
								if(tag == 1){
									break;
								}
								confignode->radio_info[i].radio_id = i;
								tag = 1;
								key = xmlNodeListGetString(doc,tmp_cur->xmlChildrenNode,1);
								if(key != NULL)
								{
									confignode->radio_info[i].radio_type = atoi((char*)(key));
									xmlFree(key);
								}

								tmp_cur = tmp_cur->next;
							}
							else if(!strcmp((char*)tmp_cur->name,"ap_radio_power"))
							{
								key = xmlNodeListGetString(doc,tmp_cur->xmlChildrenNode,1);
								if(key != NULL)
								{
									confignode->radio_info[i].txpower = atoi((char*)(key));
									xmlFree(key);
								}
								
								tmp_cur = tmp_cur->next;
							}
							else if(!strcmp((char*)tmp_cur->name,"ap_radio_chainmask"))//zhangshu add
							{
								key = xmlNodeListGetString(doc,tmp_cur->xmlChildrenNode,1);
								if(key != NULL)
								{
									confignode->radio_info[i].chainmask_num = atoi((char*)(key));
									xmlFree(key);
								}
								
								tmp_cur = tmp_cur->next;
							}
							else if(!strcmp((char*)tmp_cur->name,"ap_bss_num"))
							{
								key = xmlNodeListGetString(doc,tmp_cur->xmlChildrenNode,1);
								if(key != NULL)
								{
									confignode->radio_info[i].bss_count= atoi((char*)(key));
									xmlFree(key);
								}
								tmp_cur = tmp_cur->next;
							}
							else if(!strcmp((char*)tmp_cur->name,"ap_extern_flag"))
							{
								key = xmlNodeListGetString(doc,tmp_cur->xmlChildrenNode,1);
								if(key != NULL)
								{
									//confignode->radio_info[i].extern_flag = atoi((char*)(key));
									/*zhanglei delete parse ap_extern_flag for requirement-131*/
									xmlFree(key);
								}
								tmp_cur = tmp_cur->next;
							}else{
								break;
							}
							
						}
						
					}
                    /* book add for xml accode del, 2011-11-15 */
					confignode->bss_num = L_BSS_NUM;
						
					confignode->next = NULL;
					confignode->ischanged = CW_FALSE;//default value is 0
					confignode->ismodelchanged = CW_FALSE;
					
					if((confignode->str_ap_model != NULL))
					{
						if(*VersionInfo == NULL)
						{
							*VersionInfo = confignode;
							gModelCount++;
						}
						else
						{
							confignode->next = *VersionInfo;
							*VersionInfo = confignode;
							gModelCount++;
						}
					}else{
						CW_FREE_OBJECT_WID(confignode->str_ap_model);
						CW_FREE_OBJECT_WID(confignode);
					}
				}	
				else
				{
					//printf("003 error node\n");
				}
                if(tmp_cur != NULL)
				    tmp_cur = tmp_cur->next;
				#if 0
				if(!strcmp((char*)tmp_cur->name,"ap_code_flag")){					
					key = xmlNodeListGetString(doc,tmp_cur->xmlChildrenNode,1);
					if(key != NULL)
					{
						confignode->apcodeflag = atoi((char*)(key));
						xmlFree(key);
					}
				}
				if(!strcmp((char*)tmp_cur->name,"ap_sw_code"))
				{
					key = xmlNodeListGetString(doc,tmp_cur->xmlChildrenNode,1);
					if(key != NULL)
					{
						CW_CREATE_STRING_ERR_WID(confignode->str_ap_code,strlen((char*)(key)),return CWErrorRaise(CW_ERROR_OUT_OF_MEMORY, NULL););
						strcpy(confignode->str_ap_code,(char*)(key));
						xmlFree(key);

						confignode->bss_num = L_BSS_NUM;
						
						confignode->next = NULL;
						confignode->ischanged = CW_FALSE;//default value is 0
						confignode->ismodelchanged = CW_FALSE;
						
						if((confignode->str_ap_code != NULL)
							&&(confignode->str_ap_version_name != NULL)
							&&(confignode->str_ap_model != NULL)
							&&(confignode->str_oem_version != NULL)
							&&(confignode->str_ap_version_path != NULL)
						){
							if(*VersionInfo == NULL)
							{
								*VersionInfo = confignode;
								gModelCount++;
							}
							else
							{
								confignode->next = *VersionInfo;
								*VersionInfo = confignode;
								gModelCount++;
							}
						}else{
							CW_FREE_OBJECT_WID(confignode->str_ap_code);
							CW_FREE_OBJECT_WID(confignode->str_ap_version_name);
							CW_FREE_OBJECT_WID(confignode->str_ap_model);
							CW_FREE_OBJECT_WID(confignode->str_oem_version);
							CW_FREE_OBJECT_WID(confignode->str_ap_version_path);
							CW_FREE_OBJECT_WID(confignode);
						}
					}
				}
				#endif
			}		
		}
		
		cur = cur->next;
	}
	
	xmlFreeDoc(doc);
		
	CWConfigVersionInfo *pnode = *VersionInfo;
	while(pnode != NULL)
	{
		//printf("*** model:<%s> name:<%s> path:<%s> radionum:<%d> bssnum:<%d>***\n",pnode->str_ap_model,pnode->str_ap_version_name,pnode->str_ap_version_path,pnode->radio_num,pnode->bss_num);
		wid_syslog_debug_debug(WID_DEFAULT,"*** model:<%s> radionum:<%d> bssnum:<%d>***\n",pnode->str_ap_model,pnode->radio_num,pnode->bss_num);
		pnode = pnode->next;
		//printf("##### configuration file parse success######\n");
	}
	//printf("******gConfigVersionInfo info end********\n");

	return CW_TRUE;
}

//added by weiay 20080618
CWBool CWParseConfigVersionInfo()
{
	gConfigVersionInfo = NULL;
	gModelCount = 0;
	int i;
	
	FILE *fp = fopen(CW_CONFIG_VERSION_FILE_FIRST, "rb");
	if (fp == NULL)
	{
		//printf("config info path /mnt/wtp/wtpcompatible is disable we use /etc/version/wtpcompatible\n");
		fp = fopen(CW_CONFIG_VERSION_FILE, "rb");
		if (fp == NULL)
		{
			//printf("*** we cann't open config_version.ac file ***\n");
			wid_syslog_crit("*** we cann't open config_version.ac file ***");
			return CW_TRUE;// also return success
		}
	}
	else
	{
		//printf("config info path /mnt/wtp/wtpcompatible is enable \n");
	}

	char *line = NULL;
	while((line = CWGetCommand(fp)) != NULL)
	{
		//printf("*** Parsing model (%s) ***\n", line);
		
		wid_syslog_debug_debug(WID_DEFAULT,"*** Parsing model (%s) ***",line);
		if(!strncmp(line, AP_MODEL, strlen(AP_MODEL)))
		{
			char *myLine_model = line + strlen(AP_MODEL);

			CWConfigVersionInfo *confignode = NULL;
			CW_CREATE_OBJECT_ERR_WID(confignode, CWConfigVersionInfo, return CWErrorRaise(CW_ERROR_OUT_OF_MEMORY, NULL););	
			confignode->str_oem_version = NULL;
			CW_CREATE_STRING_ERR_WID(confignode->str_ap_model,strlen(myLine_model),return CWErrorRaise(CW_ERROR_OUT_OF_MEMORY, NULL););

			strcpy(confignode->str_ap_model,myLine_model);

			CW_FREE_OBJECT_WID(line);//parse ap model
			
			////////////////////////////////////////////////////
			
			line = CWGetCommand(fp);//parse version name
			//printf("*** Parsing name(%s) ***\n", line);
				
			if(!strncmp(line, AP_VERSION_NAME, strlen(AP_VERSION_NAME)))
			{
				char *myLine_name = line + strlen(AP_VERSION_NAME);

				CW_CREATE_STRING_ERR_WID(confignode->str_ap_version_name,strlen(myLine_name),return CWErrorRaise(CW_ERROR_OUT_OF_MEMORY, NULL););

				strcpy(confignode->str_ap_version_name,myLine_name);

				CW_FREE_OBJECT_WID(line);//parse version name
				
					
				line = CWGetCommand(fp);//parse verision image path  start
				//printf("*** Parsing path(%s) ***\n", line);
				wid_syslog_debug_debug(WID_DEFAULT,"*** Parsing path (%s) ***",line);
				if(!strncmp(line, AP_VERSION_PATH, strlen(AP_VERSION_PATH)))
				{

					char *myLine_path = line + strlen(AP_VERSION_PATH);

					CW_CREATE_STRING_ERR_WID(confignode->str_ap_version_path,strlen(myLine_path),return CWErrorRaise(CW_ERROR_OUT_OF_MEMORY, NULL););

					strcpy(confignode->str_ap_version_path,myLine_path);

					CW_FREE_OBJECT_WID(line);//parse Radio num
					
					line = CWGetCommand(fp);//parseRadio num
					//printf("*** Parsing radio num(%s) ***\n", line);
					if(!strncmp(line, AP_RADIO_NUM, strlen(AP_RADIO_NUM)))
					{
						char *myLine_radio = line + strlen(AP_RADIO_NUM);
						confignode->radio_num = atoi(myLine_radio);

						if(confignode->radio_num > L_RADIO_NUM)
						{
							confignode->radio_num = L_RADIO_NUM;
						}

						CW_FREE_OBJECT_WID(line);//parse Radio num
						
						//printf("*** Parsing bss num(%s) ***\n", line);
						for(i=0; i<confignode->radio_num; i++)
						{
							line = CWGetCommand(fp);//parseRadio num
							if(!strncmp(line, AP_RADIO_TYPE, strlen(AP_RADIO_TYPE)))
							{
								char *myLine_type = line + strlen(AP_RADIO_TYPE);
								confignode->radio_info[i].radio_type = atoi(myLine_type);

								CW_FREE_OBJECT_WID(line);

								line = CWGetCommand(fp);
								if(!strncmp(line, AP_BSS_NUM, strlen(AP_BSS_NUM)))
								{
									char *myLine_bss = line + strlen(AP_BSS_NUM);
									confignode->radio_info[i].bss_count = atoi(myLine_bss);

									if(confignode->radio_info[i].bss_count > L_BSS_NUM)
									{
										confignode->radio_info[i].bss_count = L_BSS_NUM;
									}

									CW_FREE_OBJECT_WID(line);//parse Radio num	

									//wid_syslog_debug_debug("*** model:%s name:%s path:%s radionum:%d bssnum:%d***\n",confignode->str_ap_model,confignode->str_ap_version_name,confignode->str_ap_version_path,confignode->radio_num,confignode->bss_num);
								}
								else
								{
									CW_FREE_OBJECT_WID(confignode->str_ap_model);
									CW_FREE_OBJECT_WID(confignode->str_ap_version_name);
									CW_FREE_OBJECT_WID(confignode->str_ap_version_path);
									CW_FREE_OBJECT_WID(confignode);
									CW_FREE_OBJECT_WID(line);//parse ap model

									wid_syslog_debug_debug(WID_DEFAULT,"####### wtpcompatible error 004-bss err#########\n");
									break;
								}
							}
							else
							{
								CW_FREE_OBJECT_WID(confignode->str_ap_model);
								CW_FREE_OBJECT_WID(confignode->str_ap_version_name);
								CW_FREE_OBJECT_WID(confignode->str_ap_version_path);
								CW_FREE_OBJECT_WID(confignode);
								CW_FREE_OBJECT_WID(line);//parse ap model

								wid_syslog_debug_debug(WID_DEFAULT,"####### wtpcompatible error 004-bss err#########\n");
								break;
							}

							confignode->radio_info[i].radio_id = i;

						}
						
						line = CWGetCommand(fp);//apcode
						if(!strncmp(line, AP_SW_CODE, strlen(AP_SW_CODE)))
						{
							char *myLine_apcode = line + strlen(AP_SW_CODE);
							
							CW_CREATE_STRING_ERR_WID(confignode->str_ap_code,strlen(myLine_apcode),return CWErrorRaise(CW_ERROR_OUT_OF_MEMORY, NULL););
							
							strcpy(confignode->str_ap_code,myLine_apcode);
							
							CW_FREE_OBJECT_WID(line);//parse Radio num
							confignode->bss_num = L_BSS_NUM;
							
							confignode->next = NULL;
							confignode->ischanged = CW_FALSE;//default value is 0
							confignode->ismodelchanged = CW_FALSE;
						}
						//printf("*** model:<%s> code:<%s> name:<%s> path:<%s> radionum:<%d> bssnum:<%d>***\n",confignode->str_ap_model,confignode->str_ap_code,confignode->str_ap_version_name,confignode->str_ap_version_path,confignode->radio_num,confignode->bss_num);
						if(gConfigVersionInfo == NULL)
						{
							gConfigVersionInfo = confignode;
							gModelCount++;
						}
						else
						{
							confignode->next = gConfigVersionInfo;
							gConfigVersionInfo = confignode;
							gModelCount++;
						}			


						
					}
					else
					{
						CW_FREE_OBJECT_WID(confignode->str_ap_model);
						CW_FREE_OBJECT_WID(confignode->str_ap_version_name);
						CW_FREE_OBJECT_WID(confignode->str_ap_version_path);
						CW_FREE_OBJECT_WID(confignode);
						CW_FREE_OBJECT_WID(line);//parse ap model

						wid_syslog_debug_debug(WID_DEFAULT,"####### wtpcompatible error 003-radio err#########\n");
					}
					
				}
				else
				{
					CW_FREE_OBJECT_WID(confignode->str_ap_model);
					CW_FREE_OBJECT_WID(confignode->str_ap_version_name);
					CW_FREE_OBJECT_WID(confignode);
					CW_FREE_OBJECT_WID(line);//parse ap model

					wid_syslog_debug_debug(WID_DEFAULT,"####### wtpcompatible error 002-version path err#########\n");
				}
				
			}
			else
			{
				CW_FREE_OBJECT_WID(confignode->str_ap_model);
				CW_FREE_OBJECT_WID(confignode);
				CW_FREE_OBJECT_WID(line);//parse ap model
				wid_syslog_debug_debug(WID_DEFAULT,"####### wtpcompatible error 001-version name err#########\n");
			}

		}
	}

	//printf("******gConfigVersionInfo info start********\n");
	wid_syslog_debug_debug(WID_DEFAULT,"******gConfigVersionInfo info start********");
	CWConfigVersionInfo *pnode = gConfigVersionInfo;
	while(pnode != NULL)
	{
		//printf("*** model:<%s> name:<%s> path:<%s> radionum:<%d> bssnum:<%d>***\n",pnode->str_ap_model,pnode->str_ap_version_name,pnode->str_ap_version_path,pnode->radio_num,pnode->bss_num);
		wid_syslog_debug_debug(WID_DEFAULT,"*** model:<%s> name:<%s> path:<%s> radionum:<%d> bssnum:<%d>***\n",pnode->str_ap_model,pnode->str_ap_version_name,pnode->str_ap_version_path,pnode->radio_num,pnode->bss_num);
		pnode = pnode->next;
		//printf("##### configuration file parse success######\n");
	}
	//printf("******gConfigVersionInfo info end********\n");
	wid_syslog_debug_debug(WID_DEFAULT,"******gConfigVersionInfo info end********");
	fclose(fp);
	return CW_TRUE;
	
}
CWBool CWSaveConfigVersionInfo()
{
	FILE *fp = fopen(CW_CONFIG_VERSION_FILE, "w+b");
	if (fp == NULL)
	{
		//printf("*** we cann't open config_version.ac file ***\n");
		wid_syslog_crit("******we cann't open config_version.ac file********");
		return CW_TRUE;// also return success
	}
	
	CWConfigVersionInfo *pnode = gConfigVersionInfo;
	char str[256];
	memset(str,0,256);
	
	while(pnode != NULL)
	{
		snprintf(str, 256, "</AP_SW_MODEL>%s\n</AP_SW_VERSION_NAME>%s\n</AP_SW_VERSION_IMAGE_PATH>%s\n</AP_RADIO_NUM>%d\n</AP_BSS_NUM>%d\n\n",pnode->str_ap_model,pnode->str_ap_version_name,pnode->str_ap_version_path,pnode->radio_num,pnode->bss_num);

		fwrite(str, sizeof(char), strlen(str), fp);
		
		pnode = pnode->next;
		memset(str,0,256);
	}
	fclose(fp);
	
	return CW_TRUE;

}

CWBool CWConfigFileDestroyLib() {
	int  i;
	int len = 0;
	// save the preferences we read
	if(gConfigValues[0].type == CW_STRING)//hw
	{
		len = strlen(gConfigValues[0].value.str_value);
		gACHWVersion_char = (char *)WID_MALLOC(len+1);
		if (NULL == gACHWVersion_char)
		{
			return CW_FALSE;
		}
		memset(gACHWVersion_char,0,len+1);
		memcpy(gACHWVersion_char,gConfigValues[0].value.str_value,len);
	}
	else if(gConfigValues[0].type == CW_INTEGER)
	{
		gACHWVersion = gConfigValues[0].value.int_value;
	}
	//gACHWVersion = gConfigValues[0].value.int_value;
	
	if(gConfigValues[1].type == CW_STRING)//sw
	{
		len = strlen(gConfigValues[1].value.str_value);
		gACSWVersion_char = (char *)WID_MALLOC(len+1);
		if (NULL == gACSWVersion_char)
		{
			CW_FREE_OBJECT_WID(gACHWVersion_char);
			return CW_FALSE;
		}
		memset(gACSWVersion_char,0,len+1);
		memcpy(gACSWVersion_char,gConfigValues[1].value.str_value,len);
	}
	else if(gConfigValues[1].type == CW_INTEGER)
	{
		gACSWVersion = gConfigValues[1].value.int_value;
	}
	//gACSWVersion = gConfigValues[1].value.int_value;
	
	gLimit = gConfigValues[2].value.int_value;
	//gMaxWTPs = gConfigValues[3].value.int_value;
	if(gConfigValues[4].value.str_value != NULL && !strcmp(gConfigValues[4].value.str_value, "PRESHARED")) {
		gACDescriptorSecurity = CW_PRESHARED;
	} else { // default
		gACDescriptorSecurity = CW_X509_CERTIFICATE;
	}
	if(gConfigValues[5].value.str_value != NULL) {
		CW_CREATE_STRING_FROM_STRING_ERR(gACName, (gConfigValues[5].value.str_value), return CWErrorRaise(CW_ERROR_OUT_OF_MEMORY, NULL););
		//	CW_FREE_OBJECT(gACName);
	}
	
	CW_CREATE_ARRAY_ERR(gMulticastGroups, gConfigValues[6].count, char*, return CWErrorRaise(CW_ERROR_OUT_OF_MEMORY, NULL););
	
	for(i = 0; i < gConfigValues[6].count; i++) {
		CW_CREATE_STRING_FROM_STRING_ERR(gMulticastGroups[i], (gConfigValues[6].value.str_array_value)[i], return CWErrorRaise(CW_ERROR_OUT_OF_MEMORY, NULL););
	}
	
	gMulticastGroupsCount = gConfigValues[6].count;
	CW_PRINT_STRING_ARRAY(gMulticastGroups, gMulticastGroupsCount);
	
	gCWForceMTU = gConfigValues[7].value.int_value;
	
	if(gConfigValues[8].value.str_value != NULL && !strcmp(gConfigValues[8].value.str_value, "IPv6")) {
		gNetworkPreferredFamily = CW_IPv6;
	} else { // default
		gNetworkPreferredFamily = CW_IPv4;
	}
	
	for(i = 0; i < gConfigValuesCount; i++) {
		if(gConfigValues[i].type == CW_STRING) {
			CW_FREE_OBJECT_WID(gConfigValues[i].value.str_value);
		} else if(gConfigValues[i].type == CW_STRING_ARRAY) {
			CW_FREE_OBJECTS_ARRAY((gConfigValues[i].value.str_array_value), gConfigValues[i].count);
		}
	}

	gEnabledLog=gConfigValues[9].value.int_value;
	gMaxLogFileSize=gConfigValues[10].value.int_value;
	
	CW_FREE_OBJECT_WID(gConfigValues);
	
	return CW_TRUE;
}

int wid_oui_mac_format_check(char* str,int len){
	int i = 0;
	unsigned int result = 0;
	char c = 0;
	if( 8 != len){
	   return -1;
	}
	for(;i<len;i++) {
		c = str[i];
		if((2 == i)||(5 == i)){
			if((':'!=c)&&('-'!=c)&&('.'!=c))
				return -1;
		}
		else if((c>='0'&&c<='9')||
			(c>='A'&&c<='F')||
			(c>='a'&&c<='f'))
			continue;
		else {
			result = -1;
			return result;
		}
    }
	if((str[2] != str[5])){
		result = -1;
		return result;
	}
	return result;
}
 int wid_parse_oui_mac_addr(char* input,unsigned char* oui_mac) 
 {
	int i = 0;
	char cur = 0,value = 0;
	if((NULL == input)||(NULL == oui_mac)) return -1;
	if(-1 == wid_oui_mac_format_check(input,strlen(input))) {
        wid_syslog_err("%s:oui_mac_format_error!\n",__func__);
		return -1;
	}	
	for(i = 0; i < OUI_LEN;i++) {
		cur = *(input++);
		if((cur == ':') ||(cur == '-')||(cur == '.')){
			i--;
			continue;
		}
		if((cur >= '0') &&(cur <='9')) {
			value = cur - '0';
		}
		else if((cur >= 'A') &&(cur <='F')) {
			value = cur - 'A';
			value += 0xa;
		}
		else if((cur >= 'a') &&(cur <='f')) {
			value = cur - 'a';
			value += 0xa;
		}
		oui_mac[i] = value;
		cur = *(input++);	
		if((cur >= '0') &&(cur <='9')) {
			value = cur - '0';
		}
		else if((cur >= 'A') &&(cur <='F')) {
			value = cur - 'A';
			value += 0xa;
		}
		else if((cur >= 'a') &&(cur <='f')) {
			value = cur - 'a';
			value += 0xa;
		}
		oui_mac[i] = (oui_mac[i]<< 4)|value;
	}
	return 0;
} 
 CWBool CWFreeOuiInfoList(CWOUIInfo **OuiInfoList)
 {
	 CWOUIInfo *curOuiInfoNode;
	 CWOUIInfo *tmpOuiInfoNode;
	 curOuiInfoNode = *OuiInfoList;
	 while(curOuiInfoNode != NULL){
		 tmpOuiInfoNode = curOuiInfoNode->next;
		 CW_FREE_OBJECT_WID(curOuiInfoNode->oui_mac);
	     CW_FREE_OBJECT_WID(curOuiInfoNode);
		 curOuiInfoNode = tmpOuiInfoNode;
	 }
	 return CW_TRUE;
 }
 void addOuiMacToXml(xmlDocPtr doc, xmlNodePtr cur, unsigned char *oui_mac_buff){ 
        wid_syslog_debug_debug(WID_DBUS,"oui_mac_buff:%s\n",oui_mac_buff);
	    xmlNewTextChild (cur, NULL,(unsigned char*)"oui_mac", oui_mac_buff); 
		return; 
 }
  CWBool CWAddWhiteOuiInfoToXml(unsigned char* oui_mac){
	xmlDocPtr  doc; 
	xmlNodePtr cur; 
	xmlNodePtr tmp_cur; 
	xmlNodePtr newnode; 
	//xmlAttrPtr newattr; 
	xmlChar *key;
	unsigned int readFileFlag = 0; //0--read file which at /mnt/wtp
	//unsigned int ret = 0;
	char oui_mac_buff[128] = {0};
	char buff[128] = {0};
	int sysState=0;
	int sysErrorCode = 0;
	doc = xmlReadFile(CW_WHITE_OUI_LIST_FILE_XML_FIRST,"utf-8",256);
	if(doc == NULL){
		 wid_syslog_info("Cant open the file %s",CW_WHITE_OUI_LIST_FILE_XML_FIRST);
		 if(doc==NULL)
		 doc = xmlReadFile(CW_WHITE_OUI_LIST_FILE_XML,"utf-8",256);
		 readFileFlag = 1;
		 if(doc == NULL){
		   wid_syslog_info("Cant open the file %s",CW_WHITE_OUI_LIST_FILE_XML);
		   return CW_FALSE;//also return success
		 }
	 }
	cur = xmlDocGetRootElement(doc);
	if(cur == NULL){
		 wid_syslog_crit("empty document\n");
		 xmlFreeDoc(doc);
		 return CW_FALSE;
	 }
	if (xmlStrcmp(cur->name, (const xmlChar *) "OUI_WHITELIST")){
			  wid_syslog_crit("document of the wrong type,the root node != OUI_WHITELIST\n");
			  xmlFreeDoc(doc);
			  return CW_FALSE;
	 }
	newnode = xmlNewTextChild (cur, NULL, (unsigned char*)"oui_body", NULL); 
	cur = cur->xmlChildrenNode; 
	sprintf(oui_mac_buff, OUIMACSTR, OUIMAC2STR(oui_mac));//16 to char
	while (cur != NULL) { 
		   if ((!xmlStrcmp(cur->name, (const xmlChar *)"oui_body"))){ 
                 tmp_cur = cur->xmlChildrenNode;
				 while(tmp_cur!=NULL){
				   if(!strcmp((char*)tmp_cur->name,"oui_mac")){
					    key = xmlNodeListGetString(doc,tmp_cur->xmlChildrenNode,1);
					    if(key != NULL){
                             if(strcasecmp((char*)key,oui_mac_buff)==0){
                                 wid_syslog_debug_debug(WID_DBUS,"oui_mac %s has already in whitelist.xml",oui_mac_buff);
                                 xmlFree(key);
								 return CW_FALSE;
                               }
						  xmlFree(key); 
                    }
				 }
					 tmp_cur = tmp_cur->next;
				}
				if(cur->next==NULL){
				       addOuiMacToXml (doc, cur, (unsigned char*)oui_mac_buff); 
			   	 }
		   } 
	   cur = cur->next; 
     }
	if(readFileFlag == 0){
	   xmlSaveFormatFile (CW_WHITE_OUI_LIST_FILE_XML_FIRST, doc, 1); 
	   sprintf(buff, "sudo mount /blk && sudo cp /mnt/wtp/whiteouilist.xml /blk/wtp && sudo umount /blk");
	}
	else{
	   xmlSaveFormatFile (CW_WHITE_OUI_LIST_FILE_XML, doc, 1); 
	   sprintf(buff, "sudo mount /blk && sudo cp /etc/version/whiteouilist.xml /blk/wtp && sudo umount /blk");
	}
     xmlFreeDoc(doc); 
	 sysState = system(buff);
	 sysErrorCode = WEXITSTATUS(sysState);
    if(sysErrorCode != 0){
       wid_syslog_debug_debug(WID_DBUS,"System cmd error,error code %d\n",sysErrorCode);
	}
	if(sysState != 0 )
    {
        wid_syslog_debug_debug(WID_DBUS,"System sysState error,error code %d\n",sysState);
	}
	 return CW_TRUE;
 }
 CWBool CWAddBlackOuiInfoToXml(unsigned char* oui_mac){
	xmlDocPtr  doc; 
	xmlNodePtr cur; 
	xmlNodePtr tmp_cur; 
	xmlNodePtr newnode; 
	//xmlAttrPtr newattr; 
	xmlChar *key;
	unsigned int readFileFlag = 0; //0--read file which at /mnt/wtp
	//unsigned int ret = 0;
	char oui_mac_buff[128] = {0};
	char buff[128] = {0};
	int sysState=0;
	int sysErrorCode = 0;
	doc = xmlReadFile(CW_BLACK_OUI_LIST_FILE_XML_FIRST,"utf-8",256);
	if(doc == NULL){
		 wid_syslog_info("Cant open the file %s",CW_BLACK_OUI_LIST_FILE_XML_FIRST);
		 doc = xmlReadFile(CW_BLACK_OUI_LIST_FILE_XML,"utf-8",256);
		 readFileFlag = 1;
		 if(doc == NULL){
		   wid_syslog_info("Cant open the file %s",CW_BLACK_OUI_LIST_FILE_XML);
		   return CW_FALSE;//also return success
		 }
	 }
	cur = xmlDocGetRootElement(doc);
	if(cur == NULL){
		 wid_syslog_crit("empty document\n");
		 xmlFreeDoc(doc);
		 return CW_FALSE;
	 }
	if (xmlStrcmp(cur->name, (const xmlChar *) "OUI_BLACKLIST")){
			  wid_syslog_crit("document of the wrong type,the root node != OUI_BLACKLIST\n");
			  xmlFreeDoc(doc);
			  return CW_FALSE;
	 }
	newnode = xmlNewTextChild (cur, NULL, (unsigned char*)"oui_body", NULL); 
	cur = cur->xmlChildrenNode; 
	sprintf(oui_mac_buff, OUIMACSTR, OUIMAC2STR(oui_mac));//16 to char
	while (cur != NULL) { 
		   if ((!xmlStrcmp(cur->name, (const xmlChar *)"oui_body"))){ 
                 tmp_cur = cur->xmlChildrenNode;
				 while(tmp_cur!=NULL){
				   if(!strcmp((char*)tmp_cur->name,"oui_mac")){
					    key = xmlNodeListGetString(doc,tmp_cur->xmlChildrenNode,1);
					    if(key != NULL){
                             if(strcasecmp((char*)key,oui_mac_buff)==0){
                                 wid_syslog_debug_debug(WID_DBUS,"oui_mac %s has already in blacklist.xml",oui_mac_buff);
                                 xmlFree(key);
								 return CW_FALSE;
                               }
						   xmlFree(key); 
                    }
				 }
					 tmp_cur = tmp_cur->next;
				}
				if(cur->next==NULL){
				       addOuiMacToXml (doc, cur, (unsigned char*)oui_mac_buff); 
			   	 }
		   } 
	   cur = cur->next; 
     }
	if(readFileFlag == 0){
	   xmlSaveFormatFile (CW_BLACK_OUI_LIST_FILE_XML_FIRST, doc, 1); 
	   sprintf(buff, "sudo mount /blk && sudo cp /mnt/wtp/blackouilist.xml /blk/wtp && sudo umount /blk");
	}
	else{
	   xmlSaveFormatFile (CW_BLACK_OUI_LIST_FILE_XML, doc, 1); 
	   sprintf(buff, "sudo mount /blk && sudo cp /etc/version/blackouilist.xml /blk/wtp && sudo umount /blk");
	}
     xmlFreeDoc(doc); 
	 sysState = system(buff);
	 sysErrorCode = WEXITSTATUS(sysState);
    if(sysErrorCode != 0){
       wid_syslog_debug_debug(WID_DBUS,"System cmd error,error code %d\n",sysErrorCode);
	}
	if(sysState != 0 )
    {
        wid_syslog_debug_debug(WID_DBUS,"System sysState error,error code %d\n",sysState);
	}
	 return CW_TRUE;
 }
 int CWDelWhiteOuiInfoFromXml(unsigned char* oui_mac){
		 xmlDocPtr doc; 
		 xmlNodePtr cur; 
		 xmlNodePtr cur_key; 
		 xmlNodePtr cur_tmp;
		 xmlNodePtr cur_tmp_key;
		 unsigned int readFileFlag = 0; //0--read file which at /mnt/wtp
		 unsigned int oui_mac_flag = 0; //0--not mattch 1--mattch  
		 char oui_mac_buff[128] = {0};
		 unsigned int ret = 0;
		 xmlChar *key;
		char buff[128] = {0};
		int sysState=0;
		int sysErrorCode = 0;
		 doc = xmlReadFile(CW_WHITE_OUI_LIST_FILE_XML_FIRST,"utf-8",256);
		if(doc == NULL){
			  wid_syslog_info("Cant open the file %s",CW_WHITE_OUI_LIST_FILE_XML_FIRST);
			  doc = xmlReadFile(CW_WHITE_OUI_LIST_FILE_XML,"utf-8",256);
			  readFileFlag = 1;
			  if(doc == NULL){
				wid_syslog_info("Cant open the file %s",CW_WHITE_OUI_LIST_FILE_XML);
				return -1;
			  }
		  }
		 cur = xmlDocGetRootElement(doc);
		 if(cur == NULL){
			  wid_syslog_crit("empty document\n");
			  xmlFreeDoc(doc);
			  return -1;
		  }
		 if (xmlStrcmp(cur->name, (const xmlChar *) "OUI_WHITELIST")){
				   wid_syslog_crit("document of the wrong type,the root node != OUI_WHITELIST\n");
				   xmlFreeDoc(doc);
				   return -1;
		  }
		 cur = cur->xmlChildrenNode; 
		 sprintf(oui_mac_buff, OUIMACSTR, OUIMAC2STR(oui_mac));//16 to char
		 while (cur != NULL) { 
		   cur_tmp = cur->next;  
		   if ((!xmlStrcmp(cur->name, (const xmlChar *)"oui_body"))){ 
				   cur_key = cur->xmlChildrenNode;
					while(cur_key != NULL){
					  cur_tmp_key = cur_key->next;
						if ((!xmlStrcmp(cur_key->name, (const xmlChar *)"oui_mac"))) { 
							key = xmlNodeListGetString(doc, cur_key->xmlChildrenNode, 1); 
							   if(key != NULL){
									if(strcasecmp(oui_mac_buff,(char*)key) == 0){
										   oui_mac_flag = 1;
										   ret = 1;
										   xmlFree(key); 
										   break;
									}
									xmlFree(key); 
							  }
					   } 
					   cur_key = cur_tmp_key; 
				 }
		   }
		   if(oui_mac_flag){
			   oui_mac_flag = 0;
			   wid_syslog_debug_debug(WID_DBUS,"Release oui_mac: %s Successful\n",oui_mac_buff);
			   xmlUnlinkNode(cur);
			   xmlFreeNode(cur);
		   }
		   cur = cur_tmp;
		 } 
	 if(readFileFlag == 0){
		xmlSaveFormatFile (CW_WHITE_OUI_LIST_FILE_XML_FIRST, doc, 1); 
		sprintf(buff, "sudo mount /blk && sudo cp /mnt/wtp/whiteouilist.xml /blk/wtp && sudo umount /blk");
	 }
	 else{
		xmlSaveFormatFile (CW_WHITE_OUI_LIST_FILE_XML, doc, 1); 
		sprintf(buff, "sudo mount /blk && sudo cp /etc/version/whiteouilist.xml /blk/wtp && sudo umount /blk");
	 }
	 xmlFreeDoc(doc); 
	 sysState = system(buff);
	 sysErrorCode = WEXITSTATUS(sysState);
	 if(sysErrorCode != 0){
	     wid_syslog_debug_debug(WID_DBUS,"System cmd error,error code %d\n",sysErrorCode);
	  }
	 if(sysState != 0 ){
	     wid_syslog_debug_debug(WID_DBUS,"System sysState error,error code %d\n",sysState);
	 }
	if(ret == 0){
		wid_syslog_debug_debug(WID_DBUS,"oui_mac %s has not in whiteouilist.xml",oui_mac_buff);
	 }
	 return ret;
  }
 int CWDelBlackOuiInfoFromXml(unsigned char* oui_mac){
		xmlDocPtr doc; 
		xmlNodePtr cur; 
        xmlNodePtr cur_key; 
        xmlNodePtr cur_tmp;
        xmlNodePtr cur_tmp_key;
		unsigned int readFileFlag = 0; //0--read file which at /mnt/wtp
	    unsigned int oui_mac_flag = 0; //0--not mattch 1--mattch  
		char oui_mac_buff[128] = {0};
		unsigned int ret = 0;
		xmlChar *key;
		char buff[128] = {0};
		int sysState=0;
		int sysErrorCode = 0;
		doc = xmlReadFile(CW_BLACK_OUI_LIST_FILE_XML_FIRST,"utf-8",256);
       if(doc == NULL){
			 wid_syslog_info("Cant open the file %s",CW_BLACK_OUI_LIST_FILE_XML_FIRST);
			 doc = xmlReadFile(CW_BLACK_OUI_LIST_FILE_XML,"utf-8",256);
			 readFileFlag = 1;
			 if(doc == NULL){
			   wid_syslog_info("Cant open the file %s",CW_BLACK_OUI_LIST_FILE_XML);
			   return -1;
			 }
		 }
		cur = xmlDocGetRootElement(doc);
		if(cur == NULL){
			 wid_syslog_crit("empty document\n");
			 xmlFreeDoc(doc);
			 return -1;
		 }
		if (xmlStrcmp(cur->name, (const xmlChar *) "OUI_BLACKLIST")){
				  wid_syslog_crit("document of the wrong type,the root node != OUI_BLACKLIST\n");
				  xmlFreeDoc(doc);
				  return -1;
		 }
        cur = cur->xmlChildrenNode; 
		sprintf(oui_mac_buff, OUIMACSTR, OUIMAC2STR(oui_mac));//16 to char
		while (cur != NULL) { 
          cur_tmp = cur->next;  
          if ((!xmlStrcmp(cur->name, (const xmlChar *)"oui_body"))){ 
                  cur_key = cur->xmlChildrenNode;
	               while(cur_key != NULL){
	                 cur_tmp_key = cur_key->next;
				       if ((!xmlStrcmp(cur_key->name, (const xmlChar *)"oui_mac"))) { 
		                   key = xmlNodeListGetString(doc, cur_key->xmlChildrenNode, 1); 
		                      if(key != NULL){
							       if(strcasecmp(oui_mac_buff,(char*)key) == 0){
			                              oui_mac_flag = 1;
										  ret = 1;
										  xmlFree(key); 
			                              break;
								   }
								xmlFree(key); 
						     }
					  } 
				      cur_key = cur_tmp_key; 
			    }
          }
          if(oui_mac_flag){
              oui_mac_flag = 0;
              wid_syslog_debug_debug(WID_DBUS,"Release oui_mac: %s Successful\n",oui_mac_buff);
              xmlUnlinkNode(cur);
              xmlFreeNode(cur);
          }
          cur = cur_tmp;
        } 
    if(readFileFlag == 0){
	   xmlSaveFormatFile (CW_BLACK_OUI_LIST_FILE_XML_FIRST, doc, 1); 
	   sprintf(buff, "sudo mount /blk && sudo cp /mnt/wtp/blackouilist.xml /blk/wtp && sudo umount /blk");
	}
	else{
	   xmlSaveFormatFile (CW_BLACK_OUI_LIST_FILE_XML, doc, 1); 
	   sprintf(buff, "sudo mount /blk && sudo cp /etc/version/blackouilist.xml /blk/wtp && sudo umount /blk");
	}
    xmlFreeDoc(doc); 
	sysState = system(buff);
	 sysErrorCode = WEXITSTATUS(sysState);
	 if(sysErrorCode != 0){
	     wid_syslog_debug_debug(WID_DBUS,"System cmd error,error code %d\n",sysErrorCode);
	  }
	 if(sysState != 0 ){
	     wid_syslog_debug_debug(WID_DBUS,"System sysState error,error code %d\n",sysState);
	 }
	if(ret == 0){
         wid_syslog_debug_debug(WID_DBUS,"oui_mac %s has not in blackouilist.xml",oui_mac_buff);
	}
    return ret;
 }
 CWBool CWParseWhiteOuiInfoXML(CWOUIInfo **WhiteOuiInfoList){
	 *WhiteOuiInfoList = NULL;
	 gWhiteOuiNum = 0;
	 unsigned int ret = 0;
	 xmlDocPtr doc;
	 xmlNodePtr cur;
	 xmlNodePtr tmp_cur;
	 xmlChar *key;
	 doc = xmlReadFile(CW_WHITE_OUI_LIST_FILE_XML_FIRST,"utf-8",256);
	 if(doc == NULL){
		 wid_syslog_info("Cant open the file %s",CW_WHITE_OUI_LIST_FILE_XML_FIRST);
		 doc = xmlReadFile(CW_WHITE_OUI_LIST_FILE_XML,"utf-8",256);
		 if(doc == NULL){
		  wid_syslog_info("Cant open the file %s",CW_WHITE_OUI_LIST_FILE_XML);
		   return CW_TRUE;//also return success
		 }
	 }
	 cur = xmlDocGetRootElement(doc);
	 if(cur == NULL){
		 wid_syslog_crit("empty document\n");
		 xmlFreeDoc(doc);
		 return CW_TRUE;
	 }
	 if (xmlStrcmp(cur->name, (const xmlChar *) "OUI_WHITELIST")){
			  wid_syslog_crit("document of the wrong type,the root node != OUI_WHITELIST\n");
			  xmlFreeDoc(doc);
			  return CW_TRUE;
		 }
		 cur = cur->xmlChildrenNode;
		 while(cur != NULL){
			 if(xmlStrcmp(cur->name, (const xmlChar *) "oui_body") == 0){
				 CWOUIInfo *whiteOuiNode = NULL;
				 CW_CREATE_OBJECT_ERR_WID(whiteOuiNode, CWOUIInfo, return CWErrorRaise(CW_ERROR_OUT_OF_MEMORY, NULL););  
				 memset(whiteOuiNode,0,sizeof(CWOUIInfo));
				 tmp_cur = cur->xmlChildrenNode;
				 while(tmp_cur != NULL){
				 if(!strcmp((char*)tmp_cur->name,"oui_mac"))
				 {
					 key = xmlNodeListGetString(doc,tmp_cur->xmlChildrenNode,1);
					 if(key != NULL)
					 {
						 CW_CREATE_STRING_ERR_UNSIGNED(whiteOuiNode->oui_mac,strlen((char*)(key)),return CWErrorRaise(CW_ERROR_OUT_OF_MEMORY, NULL););
						 ret = wid_parse_oui_mac_addr((char*)(key),whiteOuiNode->oui_mac);
						 if(ret == -1){
						   wid_syslog_err("%s parse oui mac erro ret =%d",__func__,ret);
						   CW_FREE_OBJECT_WID(whiteOuiNode->oui_mac);
						   CW_FREE_OBJECT_WID(whiteOuiNode);
						   xmlFree(key);
						   return CW_TRUE;
						 }
						 xmlFree(key);
						 if((whiteOuiNode != NULL)
							 &&(whiteOuiNode->oui_mac != NULL)
						 ){
							 if(*WhiteOuiInfoList == NULL)
							 {
								 *WhiteOuiInfoList = whiteOuiNode;
								 gWhiteOuiNum++;
							 }
							 else
							 {
								 whiteOuiNode->next = *WhiteOuiInfoList;
								 *WhiteOuiInfoList = whiteOuiNode;
								 gWhiteOuiNum++;
							 }
						 }else{
							 wid_syslog_crit("Error:whiteOuiNode->oui_mac == NULL\n");
							 CW_FREE_OBJECT_WID(whiteOuiNode->oui_mac);
							 CW_FREE_OBJECT_WID(whiteOuiNode);
						 }
					 }
				 }
				 else
				 {
					 wid_syslog_crit("Error:wrong type ,can not find oui mac!!\n");
				 }
					 tmp_cur = tmp_cur->next;
				}
			 }
			 cur = cur->next;
		 }
		 xmlFreeDoc(doc);
         return CW_TRUE;
 }
CWBool CWParseBlackOuiInfoXML(CWOUIInfo **BlackOuiInfoList){
	*BlackOuiInfoList = NULL;
	gblackOuiNum = 0;
	unsigned int ret = 0;
	xmlDocPtr doc;
	xmlNodePtr cur;
	xmlNodePtr tmp_cur;
	xmlChar *key;
	doc = xmlReadFile(CW_BLACK_OUI_LIST_FILE_XML_FIRST,"utf-8",256);
	if(doc == NULL){
        wid_syslog_info("Cant open the file %s",CW_BLACK_OUI_LIST_FILE_XML_FIRST);
		doc = xmlReadFile(CW_BLACK_OUI_LIST_FILE_XML,"utf-8",256);
		if(doc == NULL){
         wid_syslog_info("Cant open the file %s",CW_BLACK_OUI_LIST_FILE_XML);
          return CW_TRUE;//also return success
		}
	}
	cur = xmlDocGetRootElement(doc);
	if(cur == NULL){
        wid_syslog_crit("empty document\n");
		xmlFreeDoc(doc);
		return CW_TRUE;
	}
	if (xmlStrcmp(cur->name, (const xmlChar *) "OUI_BLACKLIST")){
			 wid_syslog_crit("document of the wrong type,the root node != OUI_BLACKLIST\n");
			 xmlFreeDoc(doc);
			 return CW_TRUE;
	    }
		cur = cur->xmlChildrenNode;
		while(cur != NULL){
			if(xmlStrcmp(cur->name, (const xmlChar *) "oui_body") == 0){
				CWOUIInfo *blackOuiNode = NULL;
				CW_CREATE_OBJECT_ERR_WID(blackOuiNode, CWOUIInfo, return CWErrorRaise(CW_ERROR_OUT_OF_MEMORY, NULL););	
				memset(blackOuiNode,0,sizeof(CWOUIInfo));
			    tmp_cur = cur->xmlChildrenNode;
				while(tmp_cur != NULL){
                if(!strcmp((char*)tmp_cur->name,"oui_mac"))
				{
					key = xmlNodeListGetString(doc,tmp_cur->xmlChildrenNode,1);
					if(key != NULL)
					{
						CW_CREATE_STRING_ERR_UNSIGNED(blackOuiNode->oui_mac,strlen((char*)(key)),return CWErrorRaise(CW_ERROR_OUT_OF_MEMORY, NULL););
						ret = wid_parse_oui_mac_addr((char*)(key),blackOuiNode->oui_mac);
						if(ret == -1){
                          wid_syslog_err("%s parse oui mac erro ret =%d",__func__,ret);
						  CW_FREE_OBJECT_WID(blackOuiNode->oui_mac);
						  CW_FREE_OBJECT_WID(blackOuiNode);
						  xmlFree(key);
						  return CW_TRUE;
						}
						xmlFree(key);
                        if((blackOuiNode != NULL)
							&&(blackOuiNode->oui_mac != NULL)
						){
							if(*BlackOuiInfoList == NULL)
							{
								*BlackOuiInfoList = blackOuiNode;
								gblackOuiNum++;
							}
							else
							{
								blackOuiNode->next = *BlackOuiInfoList;
								*BlackOuiInfoList = blackOuiNode;
								gblackOuiNum++;
							}
						}else{
						    wid_syslog_crit("Error:blackOuiNode->oui_mac == NULL\n");
							CW_FREE_OBJECT_WID(blackOuiNode->oui_mac);
							CW_FREE_OBJECT_WID(blackOuiNode);
						}
					}
				}
				else
				{
					wid_syslog_crit("Error:wrong type ,can not find oui mac!!\n");
				}
					tmp_cur = tmp_cur->next;
               }
			}
			cur = cur->next;
		}
	    xmlFreeDoc(doc);
		return CW_TRUE;
}


/*
** set g11nRateTable memory to 0
** book add, 2011-10-20
*/
void CWBeZero11nRateTable()
{
    int i;
    for(i = 0; i < NRATE_TABLE_LEN; i++)
    {
        g11nRateTable[i].count = 0;
        g11nRateTable[i].rate_info_list = NULL;
    }
    return;
}


/*
** get hash result of 11n rate
** book add, 2011-10-20
*/
int CWHash11nRate(int rate)
{
    wid_syslog_info("call CWHash11nRate\n");
    return (rate % NRATE_HASH_VAL);
}


/*
** init 11n rate table
** book add, 2011-10-20
*/
CWBool CWInit11nRateTable()
{
    //wid_syslog_info("call CWInit11nRateTable\n");
    CWBeZero11nRateTable();
    int index = 0;


    xmlDocPtr doc;
	xmlNodePtr cur;
	xmlNodePtr tmp_cur;
	xmlChar *key;
	int gcount = 0;

	doc = xmlReadFile(CW_11N_RATE_FILE,"utf-8",256);
	if(doc == NULL)
	{
		wid_syslog_crit("Does not open the file %s,",CW_11N_RATE_FILE);
		wid_syslog_crit("may be no such file!\n");
	}

	cur = xmlDocGetRootElement(doc);
	if(cur == NULL)
	{
		wid_syslog_crit("empty document\n");
		xmlFreeDoc(doc);
		return CW_FALSE;
	}
	
	if (xmlStrcmp(cur->name, (const xmlChar *) "config"))
	{
		wid_syslog_crit("document of the wrong type, root node != config");
		xmlFreeDoc(doc);
		return CW_FALSE;
	}

	cur = cur->xmlChildrenNode;

	while(cur != NULL)
	{
		if (xmlStrcmp(cur->name, (const xmlChar *) "body") == 0)
		{
			struct n_rate_list *rateList = NULL;
		    rateList = (struct n_rate_list *)WID_MALLOC(sizeof(struct n_rate_list));
		    //CW_CREATE_OBJECT_ERR(rateList, (struct n_rate_list), return CWErrorRaise(CW_ERROR_OUT_OF_MEMORY, NULL););
			rateList->next = NULL;
			tmp_cur = cur->xmlChildrenNode;
			
			while(tmp_cur != NULL)
			{
				//printf("%s\n",tmp_cur->name);
				if(!strcmp((char*)tmp_cur->name,"rate"))
				{
					key = xmlNodeListGetString(doc,tmp_cur->xmlChildrenNode,1);
					if(key != NULL)
					{
						rateList->rate_info.rate = atoi((char*)key);
						//wid_syslog_info("rate = %d\n",rateList->rate_info.rate);
						xmlFree(key);
					}
				}
				else if(!strcmp((char*)tmp_cur->name,"stream_number"))
				{
					key = xmlNodeListGetString(doc,tmp_cur->xmlChildrenNode,1);
					if(key != NULL)
					{
						rateList->rate_info.stream_num = atoi((char*)key);
						//wid_syslog_info("stream_num = %d\n",rateList->rate_info.stream_num);
						xmlFree(key);
					}	
				}			
				else if(!strcmp((char*)tmp_cur->name,"mcs"))
				{
					key = xmlNodeListGetString(doc,tmp_cur->xmlChildrenNode,1);
					if(key != NULL)
					{
						rateList->rate_info.mcs = atoi((char*)key);
						//wid_syslog_info("mcs = %d\n",rateList->rate_info.mcs);
						xmlFree(key);
					}	
				}
				else if(!strcmp((char*)tmp_cur->name,"cwmode"))
				{
					key = xmlNodeListGetString(doc,tmp_cur->xmlChildrenNode,1);
					if(key != NULL)
					{
						rateList->rate_info.cwmode = atoi((char*)key);
						//wid_syslog_info("cwmode = %d\n",rateList->rate_info.cwmode);
						xmlFree(key);
					}	
				}
				else if(!strcmp((char*)tmp_cur->name,"guard_interval"))
				{
					key = xmlNodeListGetString(doc,tmp_cur->xmlChildrenNode,1);
					if(key != NULL)
					{
						rateList->rate_info.guard_interval = atoi((char*)key);
						//wid_syslog_info("gi = %d\n",rateList->rate_info.guard_interval);
						xmlFree(key);
					}	
				}
				else
				{}

				tmp_cur = tmp_cur->next;
		    }
		    index = CWHash11nRate(rateList->rate_info.rate);
			//wid_syslog_info("index = %d\n",index);
			rateList->next = g11nRateTable[index].rate_info_list;
			g11nRateTable[index].rate_info_list = rateList;
			g11nRateTable[index].count++;
			gcount++;
		}
		cur = cur->next;
    }

    xmlFreeDoc(doc);
#if 0
	/* for test only */
	struct n_rate_list *p = NULL;
	int i = 0;
	int tcount = 0;
	wid_syslog_info("gcount = %d\n",gcount);
	wid_syslog_info("print g11nRateTable:\n");
	for(i = 0; i < 32; i++)
	{
	    tcount += g11nRateTable[i].count;
	    wid_syslog_info("i = %d, count = %d\n",i,g11nRateTable[i].count);
	    if(g11nRateTable[i].count == 0)
	        continue; 
	    p = g11nRateTable[i].rate_info_list;
	    while(p)
	    {
	        wid_syslog_info("index=%d, rate=%d, streamnum=%d, mcs=%d, cwmode=%d, gi=%d\n",\
	        index,p->rate_info.rate,p->rate_info.stream_num,p->rate_info.mcs,p->rate_info.cwmode,p->rate_info.guard_interval);
	        p = p->next;
	    }
	}
    wid_syslog_info("tcount = %d\n",tcount);
#endif    
    return CW_TRUE;
}

/*mahz add for parsing apimg.xml file,
	and global variable gConfigVerInfo is used to point to the list,
	which is built after reading apimg.xml file*/
CWBool CWParseApimgXML(CWConfigVersionInfo_new ** VersionInfo, char *model_name)
{
	/*gConfigVerInfo should be initialized as NULL and never be vlaued NULL again until the list is dstroyed*/
	//*VersionInfo = NULL;
	//gModelCount = 0;
	int i =0;
	unsigned char model_parsed = 0;
	
	xmlDocPtr doc;
	xmlNodePtr cur;
	xmlNodePtr tmp_cur;
	xmlChar *key;

	doc = xmlReadFile(CW_CONFIG_APVER_FILE_XML_FIRST,"utf-8",256);
	if(doc == NULL)
	{
		fprintf(stderr,"Does not open the file %s,",CW_CONFIG_APVER_FILE_XML_FIRST);
		fprintf(stderr,"may be no such file!\n");

		
		doc = xmlReadFile(CW_CONFIG_APVER_FILE_XML,"utf-8",256);
		if(doc == NULL)
		{
			fprintf(stderr,"Does not open the file %s,",CW_CONFIG_APVER_FILE_XML);
			fprintf(stderr,"may be no such file!\n");
			return 0;
			//return CWParseConfigVersionInfo();
			/*we don't consider the situation that the apimg.xml doesn't exist,
			if this happens,return error to remind the operator of parse result*/
		}
	}

	cur = xmlDocGetRootElement(doc);
	if(cur == NULL)
	{
		fprintf(stderr,"empty document\n");
		xmlFreeDoc(doc);
		return 0;
	}
	
	if (xmlStrcmp(cur->name, (const xmlChar *) "config"))
	{
		fprintf(stderr,"document of the wrong type, root node != config");
		xmlFreeDoc(doc);
		return 0;
	}


	cur = cur->xmlChildrenNode;

	while(cur != NULL)
	{
		if (xmlStrcmp(cur->name, (const xmlChar *) "body") == 0)
		{
			CWConfigVersionInfo_new *confignode = NULL;
			CW_CREATE_OBJECT_ERR_WID(confignode, CWConfigVersionInfo_new, return CWErrorRaise(CW_ERROR_OUT_OF_MEMORY, NULL););	
			memset(confignode,0,sizeof(CWConfigVersionInfo_new));
			confignode->next = NULL;
			tmp_cur = cur->xmlChildrenNode;
			
			while(tmp_cur != NULL)
			{
				if(!strcmp((char*)tmp_cur->name,"ap_sw_model"))
				{
					key = xmlNodeListGetString(doc,tmp_cur->xmlChildrenNode,1);
					if(key != NULL)
					{
						/*compare parsed model with input model here, go to next node if they are different*/
						if(strcmp((char*)key,model_name)){
							//cur = cur->next; //this will be excuted outside this while
							CW_FREE_OBJECT_WID(confignode);
							break;
						}
						CW_CREATE_STRING_ERR_WID(confignode->str_ap_model,strlen((char*)(key)),return CWErrorRaise(CW_ERROR_OUT_OF_MEMORY, NULL););
						memset(confignode->str_ap_model,0,strlen((char*)(key)));
						strcpy(confignode->str_ap_model,(char*)(key));
						xmlFree(key);
					}
				}
				else if(!strcmp((char*)tmp_cur->name,"ap_code_num"))
				{
					key = xmlNodeListGetString(doc,tmp_cur->xmlChildrenNode,1);
					if(key != NULL)
					{
						confignode->code_num = atoi((char*)(key));
						xmlFree(key);
					}
					tmp_cur = tmp_cur->next;
					for(i=0; i<(confignode->code_num); i++)
					{
						CWCodeInfo *codenode = NULL;
						CW_CREATE_OBJECT_ERR_WID(codenode, CWCodeInfo, return CWErrorRaise(CW_ERROR_OUT_OF_MEMORY, NULL););	
						memset(codenode,0,sizeof(CWCodeInfo));
						codenode->next = NULL;
					
						int tag = 0;
						while(tmp_cur){
							if((!strcmp((char*)tmp_cur->name,"ap_sw_version_name")))
							{
								if(tag == 1){
									break;
								}
								tag = 1;
								
								key = xmlNodeListGetString(doc,tmp_cur->xmlChildrenNode,1);
								if(key != NULL)
								{
									CW_CREATE_STRING_ERR_WID(codenode->str_ap_version_name,strlen((char*)(key)),return CWErrorRaise(CW_ERROR_OUT_OF_MEMORY, NULL););
									memset(codenode->str_ap_version_name,0,strlen((char*)(key)));
									strcpy(codenode->str_ap_version_name,(char*)(key));
									xmlFree(key);
								}	

								tmp_cur = tmp_cur->next;
							}
							else if(!strcmp((char*)tmp_cur->name,"ap_img"))
							{
								key = xmlNodeListGetString(doc,tmp_cur->xmlChildrenNode,1);
								if(key != NULL)
								{
									CW_CREATE_STRING_ERR_WID(codenode->str_ap_version_path,strlen((char*)(key)),return CWErrorRaise(CW_ERROR_OUT_OF_MEMORY, NULL););
									memset(codenode->str_ap_version_path,0,strlen((char*)(key)));
									strcpy(codenode->str_ap_version_path,(char*)(key));
									xmlFree(key);
								}	
								
								tmp_cur = tmp_cur->next;
							}
							else if(!strcmp((char*)tmp_cur->name,"ap_sw_code"))
							{
								key = xmlNodeListGetString(doc,tmp_cur->xmlChildrenNode,1);
								if(key != NULL)
								{
									CW_CREATE_STRING_ERR_WID(codenode->str_ap_version_code,strlen((char*)(key)),return CWErrorRaise(CW_ERROR_OUT_OF_MEMORY, NULL););
									memset(codenode->str_ap_version_code,0,strlen((char*)(key)));
									strcpy(codenode->str_ap_version_code,(char*)(key));
									xmlFree(key);
								}	
								
								tmp_cur = tmp_cur->next;

								/*The list of code info is built here,which requires the order of code info in apimg.xml must be right*/
								if((codenode->str_ap_version_name != NULL)
									&&(codenode->str_ap_version_path != NULL)
									&&(codenode->str_ap_version_code != NULL)
								){
									if(confignode->code_info == NULL)
									{
										confignode->code_info = codenode;
									}
									else
									{
										codenode->next = confignode->code_info;
										confignode->code_info = codenode;
									}
								}else{
									CW_FREE_OBJECT_WID(codenode->str_ap_version_name);
									CW_FREE_OBJECT_WID(codenode->str_ap_version_path);
									CW_FREE_OBJECT_WID(codenode->str_ap_version_code);
									CW_FREE_OBJECT_WID(codenode);
								}
							}
							else{
								break;
							}
						}
					}
					
					if(confignode->str_ap_model != NULL){
						if(*VersionInfo == NULL)
						{
							*VersionInfo = confignode;
							ApimgModelCount++;
						}
						else
						{
							confignode->next = *VersionInfo;
							*VersionInfo = confignode;
							ApimgModelCount++;
						}
					}else{
						/*should the codenode be freed here?*/
						CW_FREE_OBJECT_WID(confignode->str_ap_model);
						CW_FREE_OBJECT_WID(confignode);
					}
					
					model_parsed = 1;
				}	
				else
				{
					//printf("003 error node\n");
				}
                if(tmp_cur != NULL)
				    tmp_cur = tmp_cur->next;
			}		
		}
		/*if this model is parsed successfully,there is no need to continue*/
		if(model_parsed == 1)
			break;
		cur = cur->next;
	}
	/*this is the situation that model doesn't exist in apimg.xml*/
	if(model_parsed == 0){
		wid_syslog_debug_debug(WID_DEFAULT,"the model doesn't exist in apimg.xml\n"); //for test
		return 0;
	}
	
	xmlFreeDoc(doc);
		
	CWConfigVersionInfo_new *pnode = *VersionInfo;
	while(pnode != NULL)
	{
		CWCodeInfo *code_info = pnode->code_info;
		wid_syslog_debug_debug(WID_DEFAULT,"*** model:<%s>   codenum:<%d> ***\n",pnode->str_ap_model,pnode->code_num);
		while(code_info != NULL){
			wid_syslog_debug_debug(WID_DEFAULT,"*** name:<%s> path:<%s> code:<%s> ***\n",code_info->str_ap_version_name,code_info->str_ap_version_path,code_info->str_ap_version_code);
			code_info = code_info->next;
		}
		pnode = pnode->next;
	}

	return 1;
}

