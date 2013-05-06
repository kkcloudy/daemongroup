#ifndef _DCLI_WID_WLAN_H
#define _DCLI_WID_WLAN_H

#define WLAN_IF_NAME_LEN 20

/*------------------------liuzhenhua append begin---------------------*/
/*2010-05-20*/
struct SSIDConfigInfo {
	
	long  wlanCurrID;		//WLANID索引
	char *NewSSIDName;		//WLANID名称 
	long NewSSIDEnabled;		//WLAN是否启用 {0,1}=wlan.status{1,x}
	long NewSSIDHidden;		//WLAN是否隐藏 wlan.HideESSid
	long NewStaIsolate;		//二层隔离是否开启

	long NewDot11Auth;		//802.11鉴权
	long Newsecurity;		//安全类型
	long NewAuthenMode;		//认证模式

	long NewSecurityCiphers;	//加密类型
	long NewEncrInputType;		//密钥输入类型
	char *NewSecurityKEY;		//密钥值
	long NewExtensibleAuth;		//扩展认证值
	char *NewAuthIP;	//认证服务器IP
	long NewAuthPort;		//认证服务器端口
	char *NewAuthSharedSecret;	//认证服务器密钥值
	char *NewAcctIP;	//计费服务器IP
	long NewAcctPort;		//计费服务器端口
	char *NewAcctSharedSecret;	//计费服务器密钥值
	long NewVlanId;			//VLAN标识
	long NewMaxSimultUsers;		//最大支持的终端数量wlan.wlan_traffic_limit;
	long NewStaUplinkMaxRate;	//终端上行最大速率 wlan.wlan_traffic_limit;
	long NewStaDwlinkMaxRate;	//终端下行最大速率wlan.wlan_send_traffic_limit
	long SSIDRowStatus;		//创建列  1

	unsigned int SecurityID; // append for High efficiency
	unsigned int sta_aged;
	unsigned int authentication_aged;
	char *asip;
	long cert_type;
	char *as_path;
	char *ae_path;
	struct SSIDConfigInfo *next;
};
/*------------------------liuzhenhua append begin---------------------*/

void dcli_wlan_init(void);
int parse_char_ID(char* str,unsigned char* ID);
void CheckWIDIfPolicy(char *whichinterface, unsigned char wlan_if_policy);

#define		DCLI_FORMIB_FREE_OBJECT(obj_name)				{if(obj_name){free((obj_name)); (obj_name) = NULL;}}

struct ConfigWapiInfo {
	unsigned char  WlanId;		       //WAPI SSID
	unsigned char  SecurityID;              //创建的安全策略的ID
	char *WAPIASIPAddress; //Wapi IP地址
	unsigned int WapiCipherKeyCharType;    //wapi加密类型
	
		
    struct ConfigWapiInfo *next;
	struct ConfigWapiInfo *ConfigWapiInfo_list;
	struct ConfigWapiInfo *ConfigWapiInfo_last;
};
/*table 21-2*/
struct Sub_Sta_WtpWAPIPerformance {
	unsigned char *staMacAddr;					//station的MAC地址
	int sta_seq;
	unsigned int wtpWapiVersion;					//端站的WAPI版本号
	unsigned char wtpWapiControlledPortStatus;			//鉴权控制端口的状态
	unsigned char wtpWapiSelectedUnicastCipher[4];			//选择的单播加密套件
	
	unsigned int wtpWapiWPIReplayCounters;			//由于响应机制而丢弃的WPI?MPDU数量
	unsigned int wtpWapiWPIDecryptableErrors;		//由于密钥无效而丢弃的WPI?MPDU数量
	unsigned int wtpWapiWPIMICErrors;			//由于MIC校验失败而丢弃的WPI?MPDU数量
	unsigned int wtpWapiWAISignatureErrors;		//签名错误的WAI数据包的数量
	unsigned int wtpWapiWAIHMACErrors;			//鉴权密钥校验失败的WAI数据包的数量
	unsigned int wtpWapiWAIAuthResultFailures;		//WAI鉴权结果失败的数量
	unsigned int wtpWapiWAIDiscardCounters;		//WAI数据包丢弃的数量
	unsigned int wtpWapiWAITimeoutCounters;		//WAI数据包超时的数量
	unsigned int wtpWapiWAIFormatErrors;			//WAI数据包格式错误的数量
	unsigned int wtpWapiWAICertificateHandshakeFailures;	//WAI认证握手失败的数量
	unsigned int wtpWapiWAIUnicastHandshakeFailures;	//WAI单播密钥协商失败的数量
	unsigned int wtpWapiWAIMulticastHandshakeFailures;     //WAI多播密钥失败的数量
	
			
    struct Sub_Sta_WtpWAPIPerformance *next;
	struct Sub_Sta_WtpWAPIPerformance *Sub_Sta_WtpWAPIPerformance_list;
	struct Sub_Sta_WtpWAPIPerformance *Sub_Sta_WtpWAPIPerformance_last;
	
};

/*table 21*/
struct WtpWAPIPerformanceStatsInfo {
	unsigned char  WlanId;
	unsigned int SecurityType;
	unsigned int wlan_bss_num;			//for showing
	int wlan_total_sta_num ;			//for showing
	unsigned char  SecurityID;              //创建的安全策略的ID

	struct Sub_Sta_WtpWAPIPerformance *Sub_Sta_WtpWAPIPerformance_head;
    struct WtpWAPIPerformanceStatsInfo *next;
	struct WtpWAPIPerformanceStatsInfo *WtpWAPIPerformanceStatsInfo_list;
	struct WtpWAPIPerformanceStatsInfo *WtpWAPIPerformanceStatsInfo_last;
	
};

struct WtpWAPIExtendConfigInfo {
								
	unsigned char  WapiWlanID;				//Wapi安全策略绑定的WLANID（索引）				//
	unsigned char  SecurityID;				//WLAN绑定的安全策略ID								//
	unsigned int wtpWapiConfigVersion;			//支持的WAPI 最高版本号						//默认1
	unsigned int SecurityType;			//for showting     

	unsigned char wtpWapiControlledAuthenControlenabled; //是否启用鉴权								//
	char *wtpWapiControlledPortControl; 	//端口的控制类型							//默认"auto"
	unsigned int wtpWapiWPIOptionImplement; 		//是否支持WAPI							//1
	
	unsigned int wtpWapiWPIPreauthImplemented;		//是否支持WAPI预鉴权							//
	unsigned int wtpWapiEnabled;				//是否启用WAPI										//
	unsigned int wtpWapiPreauthEnabled; 		//是否启用WAPI预鉴权								//
	
	unsigned int wtpWapiUnicastKeysSupported;	//支持的WAPI 单播密钥数				//默认2
	unsigned char wtpWapiUnicastRekeyMethod; 		//单播密钥更新机制								//
	unsigned int wtpWapiUnicastRekeyTime;		//单播密钥有效时间									//
	unsigned int wtpWapiUnicastRekeyPackets;	//单播密钥有效的数据包数量						//
	char wtpWapiMulticastCipher[4];			//多播加密套件													//
	unsigned char wtpWapiMulticastRekeyMethod;		//组播密钥更新机制								//
	unsigned int wtpWapiMulticastRekeyTime;	//组播密钥有效时间										//
	unsigned int wtpWapiMulticastRekeyPackets; //组播密钥有效的数据包数量							//
	unsigned char wtpWapiMulticastRekeyStrict;		//是否在终端离开BSS 后更新组播密钥		//
	
	char *wtpWapiPSKValue;				//PSK值
	char *wtpWapiPSKPassPhrase; 		//用于生成PSK的 pass-phrase
	unsigned int wtpWapiCertificateUpdateCount;	//鉴权握手的重试次数									//
	unsigned int wtpWapiMulticastUpdateCount;	//MSK 握手的重试次数									//
	unsigned int wtpWapiUnicastUpdateCount;	//单播密钥握手的重试次数								//
	unsigned int wtpWapiMulticastCipherSize;	//组播密钥的bit长度						//默认512
	unsigned int wtpWapiBKLifetime;		//广播密钥(BK)的有效期										//
	unsigned int wtpWapiBKReauthThreshold; 	//BK重新鉴权的门限时间									//
	unsigned int wtpWapiSATimeout; 		//建立安全连接的最大时长									//

	
	char wtpWapiAuthSuiteSelected[4]; 		//选择的AKM									//默认
	char *wtpWapiUnicastCipherSelected; 	//上一个单播选择的加密类型			//默认
	char *wtpWapiMulticastCipherSelected;		//上一个组播选择的加密类型		//默认
	unsigned char wtpWapiBKIDUsed[16];				//上一个使用的BK ID								//默认
	char wtpWapiAuthSuiteRequested[4];		//上一个AKM 请求的选择					//默认
	char *wtpWapiUnicastCipherRequested;		//上一个单播加密请求的选择		//默认
	char *wtpWapiMulticastCipherRequested;		//上一个组播加密请求的选择		//默认
	
			
	struct WtpWAPIExtendConfigInfo *next;
	struct WtpWAPIExtendConfigInfo *WtpWAPIExtendConfigInfo_list;
	struct WtpWAPIExtendConfigInfo *WtpWAPIExtendConfigInfo_last;
};

struct UnicastInfo {

	unsigned char  SecurityID;                    //创建的安全策略ID
	unsigned char  UnicastWlanID;                 //单播Wlan ID
	unsigned int SecurityType;					//策略类型
	char *NewUnicastCipher;              //单播密码		//mib加默认的
	long NewUnicastCipherEnabled;        //单播密码生效//已算出
	unsigned long NewUnicastCipherSize;  //单播密码长度//512
		
    struct UnicastInfo *next;
	struct UnicastInfo *UnicastInfo_list;
	struct UnicastInfo *UnicastInfo_last;
};

/*table 28-2*/
struct Sub_BssWAPIPerformanceStatsInfo {	
	unsigned char *wtpMacAddr;					//AP的MAC地址
	unsigned char *wtpBssCurrID;					//AP的bssID

	unsigned char bss_id;					//for showwting
	unsigned char wlan_id;					//for showwting
	unsigned int wtp_id;					//for showwting
	
	unsigned int bssWapiControlledPortStatus;			//鉴权控制端口的状态			//默认 0
	unsigned char bssWapiSelectedUnicastCipher[4];			//选择的单播加密套件		//mib 默认自己加{0x00, 0x14, 0x72, 0x01};
	
	unsigned int bssWapiWPIReplayCounters;			//由于响应机制而丢弃的WPI?MPDU数量
	unsigned int bssWapiWPIDecryptableErrors;		//由于密钥无效而丢弃的WPI?MPDU数量
	unsigned int bssWapiWPIMICErrors;			//由于MIC校验失败而丢弃的WPI?MPDU数量
	unsigned int bssWapiWAISignatureErrors;		//签名错误的WAI数据包的数量
	unsigned int bssWapiWAIHMACErrors;			//鉴权密钥校验失败的WAI数据包的数量
	unsigned int bssWapiWAIAuthResultFailures;		//WAI鉴权结果失败的数量
	unsigned int bssWapiWAIDiscardCounters;		//WAI数据包丢弃的数量
	unsigned int bssWapiWAITimeoutCounters;		//WAI数据包超时的数量
	unsigned int bssWapiWAIFormatErrors;			//WAI数据包格式错误的数量
	unsigned int bssWapiWAICertificateHandshakeFailures;	//WAI认证握手失败的数量
	unsigned int bssWapiWAIUnicastHandshakeFailures;	//WAI单播密钥协商失败的数量
	unsigned int bssWapiWAIMulticastHandshakeFailures;	//WAI多播密钥失败的数量
										    
    struct Sub_BssWAPIPerformanceStatsInfo *next;
	struct Sub_BssWAPIPerformanceStatsInfo *Sub_BssWAPIPerformanceStatsInfo_list;
	struct Sub_BssWAPIPerformanceStatsInfo *Sub_BssWAPIPerformanceStatsInfo_last;
};
/*table 28*/
struct BssWAPIPerformanceStatsInfo {	
	unsigned char WlanID;
	unsigned char  SecurityID;
	unsigned int SecurityType;	
	int wlan_bss_num;

	
	struct Sub_BssWAPIPerformanceStatsInfo	*Sub_BssWAPIPerformanceStatsInfo_head;
    struct BssWAPIPerformanceStatsInfo *next;
	struct BssWAPIPerformanceStatsInfo *BssWAPIPerformanceStatsInfo_list;
	struct BssWAPIPerformanceStatsInfo *BssWAPIPerformanceStatsInfo_last;
};

#define		DCLI_FORMIB_FREE_OBJECT(obj_name)				{if(obj_name){free((obj_name)); (obj_name) = NULL;}}
#endif

