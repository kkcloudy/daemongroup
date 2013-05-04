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
* CWConfigFile.h
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


#ifndef __CAPWAP_CWConfigFile_HEADER__
#define __CAPWAP_CWConfigFile_HEADER__
 
#define RADIO_TOTAL_COUNT 4
#define NRATE_TABLE_LEN  32     //book add, 2011-10-20
#define NRATE_HASH_VAL  29     //book add, 2011-10-20
#define BATCH_UPGRADE_AP_NUM	32

typedef char **CWStringArray;

typedef struct {
	enum {
		CW_INTEGER,
		CW_STRING,
		CW_STRING_ARRAY
	} type;
	
	union {
		int int_value;
		char *str_value;
		char **str_array_value;
	} value;
	
	char *code;
	char *endCode;
	
	int count;
} CWConfigValue;

struct radio_info_type{
	char radio_type;
	char radio_id;
	char bss_count;
	char txpower;
	char chainmask_num; //zhangshu add 2010-11-24
	char reserved2;
	unsigned short reserved3;
	unsigned int extern_flag;
};

struct CWConfigVersionInfo_
{
	char* str_ap_model; //for oem change
	char *str_ap_version_name;
	char *str_ap_version_path;
	unsigned char eth_num;  //fengwenchao add 20110407
	unsigned char radio_num;
	unsigned char bss_num;
	int ischanged;
	int ismodelchanged;
	struct radio_info_type radio_info[RADIO_TOTAL_COUNT];
	char * str_ap_code;// for model match
	char * str_oem_version;
	int	apcodeflag;
	struct CWConfigVersionInfo_ *next;
};

typedef struct CWConfigVersionInfo_ CWConfigVersionInfo;

struct CWOUIInfo_{
   unsigned char* oui_mac;
   struct CWOUIInfo_ *next;
};
typedef struct CWOUIInfo_ CWOUIInfo;

/*mahz add struct of code_info and struct of version_info , which is read from apimg.xml file*/
struct code_info_type{
	char *str_ap_version_name;
	char *str_ap_version_path;
	char *str_ap_version_code;
	struct code_info_type *next;
};
typedef struct code_info_type CWCodeInfo;

struct CWConfigVersionInfo_new_
{
	char *str_ap_model;
	char *tar_file_name;
	unsigned char code_num;
	CWCodeInfo *code_info;

	struct CWConfigVersionInfo_new_ *next;
};
typedef struct CWConfigVersionInfo_new_ CWConfigVersionInfo_new;

extern CWConfigVersionInfo_new *gConfigVerInfo;
extern unsigned char gupdateControl;

extern CWConfigVersionInfo *gConfigVersionInfo;
extern struct n_rate_table g11nRateTable[NRATE_TABLE_LEN];  //book add, 2011-10-20
extern CWConfigVersionInfo *gConfigVersionUpdateInfo[BATCH_UPGRADE_AP_NUM];
extern CWConfigValue *gConfigValues;
extern int gConfigValuesCount;
extern int gModelCount;
extern int ApimgModelCount;
extern unsigned char gupdateCountOneTime;
extern CWOUIInfo               *gBlackOuiInfoList;
extern CWOUIInfo               *gWhiteOuiInfoList;
extern int                        gblackOuiNum;
extern int                        gWhiteOuiNum;
extern int                        gOuiListType;/*0---none,1---use black oui list ,2---use white oui list*/



CWBool CWParseConfigFile();
CWBool CWParseConfigVersionInfo();//added by weiay 20080618
CWBool CWParseConfigVersionInfoXML(CWConfigVersionInfo **VersionInfo);//added by weianying 20090408
CWBool CWDistroyConfigVersionInfoXML(CWConfigVersionInfo **VersionInfo);
CWBool CWSaveConfigVersionInfo();
CWBool CWParseConfigFile1();
char * CWGetCommand(FILE *configFile);
CWBool CWConfigFileInitLib(void);
CWBool CWConfigFileInitLib1(void);
CWBool CWConfigFileDestroyLib(void);

CWBool CWParseBlackOuiInfoXML(CWOUIInfo **BlackOuiInfoList);
CWBool CWParseWhiteOuiInfoXML(CWOUIInfo **WhiteOuiInfoList);
CWBool CWFreeOuiInfoList(CWOUIInfo **OuiInfoList);
CWBool CWAddBlackOuiInfoToXml(unsigned char* oui_mac);
CWBool CWAddWhiteOuiInfoToXml(unsigned char* oui_mac);
int CWParseDelBlackOuiInfoFromXml(unsigned char* oui_mac);
int CWDelWhiteOuiInfoFromXml(unsigned char* oui_mac);
void CWBeZero11nRateTable();
int CWHash11nRate(int rate);
CWBool CWInit11nRateTable();//add by book, 2011-10-20
CWBool CWParseApimgXML(CWConfigVersionInfo_new ** VersionInfo, char *model_name);
#endif
