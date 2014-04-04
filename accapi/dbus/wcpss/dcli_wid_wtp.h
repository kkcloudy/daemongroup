#ifndef DCLI_WID_WTP_H
#define DCLI_WID_WTP_H

/*nl add for mib showting wtp  infromation begin*/
/*------------------------------------------------*/
#define		DCLI_FORMIB_FREE_OBJECT(obj_name)				{if(obj_name){free((obj_name)); (obj_name) = NULL;}}

/* for mib showting wtp basic infromation*/
struct WtpBasicInfo{
    int wtpCurrID;
    unsigned char *wtpMacAddr;				//AP的MAC地址
    char *wtpModel;				//AP模式	
    char *wtpDevName;				//AP名称
    char *wtpPosInfo;				//AP位置信息
    char *wtpProduct;				//AP制造商
    char *wtpDevTypeNum;			//AP类型
    char *wtpVersionInfo;			//AP软件版本
    unsigned long wtpUpTime;			//AP运行时间
    unsigned long wtpOnlineTime;		//AP上线时间
    char *wtpSysSoftName;			//AP软件名称
    char *wtpSysVersion;			//AP硬件版本
    char *wtpSeriesNum;				//AP SN
    char *wtpSysSoftProductor;			//AP软件制造商
    char *wtpDomain;				//AP Domian
    long wtpSysRestart;				//AP系统重启
    long wtpSysReset;				//AP 系统重置
    long wtpColdReboot;				//AP冷启动
    unsigned long  Mib_wtpUpTime;			//AP启动运行时间长度
    unsigned int ElectrifyRegisterCircle;
	unsigned int acNeighbordeadTimes;
    struct WtpBasicInfo *next;
    struct WtpBasicInfo *WtpBasicInfo_list;
    struct WtpBasicInfo *WtpBasicInfo_last;
	unsigned char *longitude;
	unsigned char *latitude;
	unsigned char power_mode;
	unsigned char *manufacture_date;
	unsigned char forward_mode;
};
/*for mib showting wtp collect infromation*/
struct WtpCollectInfo{
    int  wtpCurrID;				
    unsigned char *wtpMacAddr;
    unsigned int wtpRtCollectOnOff;			//AP 实时采集开关
    unsigned int CPURTUsage;				//AP CPU实时利用率
    unsigned char MemRTUsage;				//AP内存实时利用率info.memoryuse
    unsigned int wtpAssocTimes;		//AP 关联总次数
    unsigned int wtpAssocFailtimes;		//AP 关联失败次数
    unsigned int wtpReassocTimes;		//AP重关联次数
    unsigned int wtpIfIndiscardPkts;		//AP 有线端口接收的丢弃包数exten_info.rx_drop
    unsigned long long  wtpStaTxBytes;		//AP上行端口有线接受的总字节数
    unsigned long long  wtpStaRxBytes;		//AP 下行端口有线发送的总字节数

    unsigned int wtpReassocFailure;		//AP重关联失败次数//20100721   //copy from 1.2 by zhangshu  0909
    unsigned int wtpSuccAssociatedNum;	//assoc successfully times, xiaodawei add, 20110418
    unsigned char wtpMomentCollectSwith;/*AP collect switch for lixiang's mib table by nl 20100823*/ //copy from 1.2 by zhangshu  0909
    
    struct WtpCollectInfo *next;
	struct WtpCollectInfo *WtpCollectInfo_list;
    struct WtpCollectInfo *WtpCollectInfo_last;
};
/*for mib showting WtpParaInfo*/

struct WtpParaInfo{
	int wtpCurrID;
	unsigned char *wtpMacAddr;
	char *wtpModel;				
	unsigned char *wtpCurBssid;			//AP当前的BSSID
	char *wtpGateAddr;		//AP网关地址
	char *wtpNetElementCode;		//AP网元编码
	char *wtpAddrMask;		//AP地址掩码
	unsigned char wtp_ipv6_ip_prefix;	//AP ipv6 prefix
	unsigned char wtpRemoteRestartFun;		//AP远程重启功能
	unsigned int wtpReceiverSignalPWL;		//AP接收信号强度
	unsigned char *wtpMacConApAc;		//AP的MAC地址
	unsigned char wtpState;				//AP状态
	char *wtpIP;			//AP的IP地址//20100714
	char *wtp_ipv6_ip;		//AP ipv6 ip
	char *wtpName;			//AP的名字//20100722
	unsigned char wtpDefenceDOSAttack;		//AP防DOS攻击
	unsigned char wtpIGMPSwitch;			//AP的IGMP开关
	unsigned char wtpCurrAPMode;			//AP工作模式（包含正常、监控、半监控）
	
	unsigned int wtpCurBssidNumber;					//ssid number of one wtp  
	// copy from 1.2omc by zhangshu   2010-09-09
	unsigned char total_if_num; /*add for mib require total if num : wifi_num + eth_num*/

	char *wtpIfType;			//AP支持的接口类型
	long wtpWorkMode;			//AP工作模式
	unsigned int wtpBridgingWorkMode;		//AP桥工作模式
	unsigned int wtpsupport11b;			//AP是否支持802.11B
	unsigned int wtpsupport11g;			//AP是否支持802.11G

	//xiaodawei transplant from 2.0 for telecom test, 20110302
	time_t add_time;
	time_t ElectrifyRegisterCircle;
	time_t imagadata_time;
	time_t config_update_time;
	time_t starttime;
	//END
	struct WtpParaInfo *next;
	struct WtpParaInfo *WtpParaInfo_list;
	struct WtpParaInfo *WtpParaInfo_last;
};

/* book modify, 2011-1-19 */
struct WtpWirelessIfstatsInfo_radio{

	unsigned char wtpWirelessIfIndex;				//AP无线接口索引
	unsigned char wirelessIfUpdownTimes;				//AP无线接口UpDown次数

	unsigned int wirelessIfChStatsUplinkUniFrameCnt;		//无线信道上行单播的帧数
	unsigned int wirelessIfChStatsDwlinkUniFrameCnt;		//无线信道下行单播的帧数
	unsigned int wirelessIfUpChStatsFrameNonUniFrameCnt;	//无线信道上行非单播的帧数
	unsigned int wirelessIfDownChStatsFrameNonUniFrameCnt;	//无线信道下行非单播帧数

	unsigned int wirelessIfApChStatsFrameFragRate;		//信道上帧的分段速率
	unsigned int wirelessIfChStatsNumStations;			//使用该信道的终端数

	unsigned int wirelessIfRxMgmtFrameCnt;			//接收到的管理帧的数量
	unsigned int wirelessIfRxCtrlFrameCnt;			//接收到的控制帧的数量
	unsigned int wirelessIfRxDataFrameCnt;			//接收到的数据帧的数量
	unsigned int wirelessIfRxAuthenFrameCnt;			//接收到的认证帧的数量
	unsigned int wirelessIfRxAssociateFrameCnt; 		//接收到的关联帧的数量
	unsigned int wirelessIfTxMgmtFrameCnt;			//发送的管理帧的数量
	unsigned int wirelessIfTxCtrlFrameCnt;			//发送的控制帧的数量
	unsigned int wirelessIfTxDataFrameCnt;			//发送的数据帧的数量
	unsigned int wirelessIfTxAuthenFrameCnt;			//发送的认证帧的数量
	unsigned int wirelessIfTxAssociateFrameCnt; 		//发送的关联帧的数量


	unsigned int sub_wirelessIfRxDataPkts;				//add to sub radio c1
	unsigned int sub_wirelessIfTxDataPkts;
	unsigned int sub_wirelessIfChStatsFrameErrorCnt;
	unsigned long sub_wirelessIfRxErrPkts;				//xiaodawei add rx error pkts, 20110406
	unsigned long sub_wirelessIfTxDropPkts;				//xiaodawei add tx drop pkts, 20110406
	unsigned long sub_wirelessIfRxDropPkts;				//xiaodawei add rx drop pkts, 20110406
	unsigned long long  sub_wirelessIfDwlinkDataOctets;
	unsigned long long  sub_wirelessIfUplinkDataOctets;
	unsigned int sub_wirelessIfChStatsMacFcsErrPkts;
	unsigned int sub_wirelessIfChStatsMacDecryptErrPkts;
	unsigned int sub_wirelessIfChStatsMacMicErrPkts;
	unsigned int sub_wirelessIfChStatsPhyErrPkts;			//add to sub radio c9

	unsigned char radio_cur_snr ;				//snr 强度nl 20100723
	unsigned char radio_max_snr;				//snr max强度nl20100723
	unsigned char radio_min_snr;				//snr min强度nl20100723
	double radio_aver_snr;				//snr aver强度nl20100723

    char radio_cur_snr2 ;				//snr 强度nl 20100907
	char radio_max_snr2;				//snr max强度nl 20100907
	char radio_min_snr2;				//snr min强度nl 20100907
	
	/*add for new mib requirement 20100809*/  //zhangshu copy from 1.2omc 0909
	unsigned int sub_rx_pkt_mgmt ;  // packets received of management
	unsigned int sub_tx_pkt_mgmt ; // packets transtmitted of management
	unsigned long long sub_rx_mgmt ;
	unsigned long long sub_tx_mgmt ;
	unsigned long long sub_total_rx_bytes ;
	unsigned long long sub_total_tx_bytes ;
	unsigned long long sub_total_rx_pkt ;
	unsigned long long sub_total_tx_pkt ;

    /* book modify, 2011-5-19 */
	unsigned int wirelessIfTxSignalPkts;			//发送的信令包数
    unsigned int wirelessIfRxSignalPkts;			//接收的信令包数
    unsigned int wirelessIfChStatsDwlinkTotRetryPkts;		//无线信道下行重传的包数	//only wtp
    unsigned int wirelessIfChStatsFrameRetryCnt;		//信道下行重传的帧数				//only wtp
	
	struct WtpWirelessIfstatsInfo_radio *next;
	struct WtpWirelessIfstatsInfo_radio *WtpWirelessIfstatsInfo_radio_list;
	struct WtpWirelessIfstatsInfo_radio *WtpWirelessIfstatsInfo_radio_last;
};

struct WtpWirelessIfstatsInfo {		//aaaa
    int wtpCurrID;
    unsigned char *wtpMacAddr;
	unsigned char wtp_radio_num;
    unsigned char wirelessIfAvgRxSignalStrength;			//AP无线接口接收的信号平均强度	//nnnn
    unsigned char wirelessIfHighestRxSignalStrength;		//AP无线接口接收信号的最大强度	//nnnn
    unsigned char wirelessIfLowestRxSignalStrength;		//AP无线接口接收信号的最低强度		//nnnn
    unsigned char wifiExtensionInfoReportswitch;		/*set ap extension infomation switch (enable|disable)*/	//nnnn
    double 	math_wirelessIfAvgRxSignalStrength;			//AP无线接口接收的信号算数平均强度	//nnnn

    char wirelessIfAvgRxSignalStrength2;			//AP无线接口接收的信号平均强度	//add 2010-09-07
    char wirelessIfHighestRxSignalStrength2;		//AP无线接口接收信号的最大强度	//add 2010-09-07
    char wirelessIfLowestRxSignalStrength2;		//AP无线接口接收信号的最低强度		//add 2010-09-07
	
    unsigned int wirelessIfTxDataPkts;				//发送的数据包数//	add to sub radio
    unsigned int wirelessIfRxDataPkts;				//接收的数据包数//	add to sub radio
    unsigned long long wirelessIfUplinkDataOctets;			//发送的数据包字节数//	add to sub radio
    unsigned long long wirelessIfDwlinkDataOctets;			//接收的数据包字节数//	add to sub radio

	unsigned int wirelessIfChStatsPhyErrPkts;			//信道接收的物理层错包数//	add to sub radio
    unsigned int wirelessIfChStatsMacFcsErrPkts;		//信道接收的FCS MAC错包数//	add to sub radio
    unsigned int wirelessIfChStatsMacMicErrPkts;		//信道接收的MIC MAC错包数//	add to sub radio
    unsigned int wirelessIfChStatsMacDecryptErrPkts;		//信道接收的解密失败的MAC错包数//	add to sub radio
    unsigned int wirelessIfChStatsFrameErrorCnt;		//信道接收的错帧数				//	add to sub radio

	unsigned int asd_radio_num;						//asd 端radio num
    
	struct WtpWirelessIfstatsInfo_radio *wireless_sub_radio_head;
	struct Neighbor_AP_ELE *neighbor_wtp;   //fengwenchao add 20110521
	int neighbor_ap_count;   //fengwenchao add 20110521
    struct WtpWirelessIfstatsInfo *next;
	struct WtpWirelessIfstatsInfo *WtpWirelessIfstatsInfo_list;
	struct WtpWirelessIfstatsInfo *WtpWirelessIfstatsInfo_last;
};

struct WtpDeviceInfo {
	unsigned char  *wtpMacAddr;										
	unsigned char *wtpModel;
	int wtpCurrID;
	
	unsigned char wtpCPUType;				//AP CPU类型 	// change type to unsigned char
	unsigned char wtpMemoryType;				//AP内存类型// change type to unsigned char

	unsigned int wtpCPUusageThreshhd;			//AP CPU利用率阈值	//
	unsigned int wtpMemUsageThreshhd;			//AP内存利用率阈值//
	
	unsigned int  wtpCPURTUsage;				//AP CPU实时利用率		3w
	unsigned long long wtpMemoryCapacity;		//AP内存容量			//3w
	unsigned int  wtpMemRTUsage;				//AP内存实时利用率	//3w
	unsigned long long wtpFlashCapacity;				//AP Flash容量		//3w
	unsigned int wtpFlashRemain;				//AP Flash剩余空间		//3w
	unsigned char wtpWayGetIP;				//AP获取IP方式				//3w
	unsigned char wtpWorkTemp;				//AP工作温度				//3w

	unsigned int wtpCPUPeakUsage;				//AP CPU峰值利用率
	unsigned int  wtpCPUAvgUsage;				//AP CPU平均利用率
	unsigned int  wtpMemPeakUsage;				//AP内存峰值利用率
	unsigned int  wtpMemAvgUsage;				//AP内存平均利用率

	unsigned char cpuType_str[WTP_TYPE_DEFAULT_LEN];				//nl add 20100728
	unsigned char flashType_str[WTP_TYPE_DEFAULT_LEN];				//nl add 20100728
	unsigned char memType_str[WTP_TYPE_DEFAULT_LEN];				//nl add 20100728

	char *wtpCPUProcessAbility;			//AP CPU处理能力			//mib自己加
	char *wtpFlashType;				//AP Flash类型						//默认
	char *WtpIP;				//AP的IP地址					//ip
	char *wtp_ipv6_ip;			//AP ipv6 ip
	unsigned int cpu_collect_average;
/////////////////////////
	
	struct WtpDeviceInfo *next;
	struct WtpDeviceInfo *WtpDeviceInfo_list;
	struct WtpDeviceInfo *WtpDeviceInfo_last;
};	


struct WtpDataPktsInfo {
    unsigned  char *wtpMacAddr;
    int wtpCurrID;
    unsigned long long wtpWirelessMacRecvAllByte;			//无线侧MAC层接收的总的byte数		//
    unsigned long long wtpWirelessMacSendAllByte;			//无线侧MAC层发送的总的byte数		//
    unsigned int wtpWirelessMacRecvPack;			//无线侧mac层接收的包数				//
    unsigned int wtpWirelessMacRecvWrongPack;			//无线侧mac层接收的错包数		//
    unsigned long long wtpWiredMacRecvRightByte;			//有线侧MAC层接收的总的byte数					//
    unsigned long long wtpWiredMacSendAllByte;			//有线侧MAC层发送的总的byte数					//
    unsigned int wtpWiredMacRecvWrongPack;			//有线侧MAC层接收的错包数						//
    unsigned int wtpDropPkts;					//无线侧MAC层发送的丢包数				//
    unsigned int wtpWirelessMacSendPack;			//无线侧MAC层发送的包数				//
    unsigned int wtpWirelessMacSendErrPack; 			//无线侧MAC层发送的错包数		//
    unsigned int wtpWirelessMacRecvRightPack;			//无线侧MAC层接收的正确的包数//
    unsigned int wtpWirelessMacSendRightPack;			//无线侧MAC层发送的正确的包数	//
    unsigned int wtpWiredMacRecvPack;				//有线侧MAC层接收的包数							//
    unsigned int wtpWiredMacRecvRightPack;			//有线侧MAC层接收的正确的包数					//
    unsigned int wtpWiredMacSendPack;				//有线侧MAC层发送的包数							//
    unsigned int wtpWiredMacSendErrPack;			//有线侧MAC层发送的错包数						//
    unsigned int wtpWiredMacSendRightPack;			//有线侧MAC层发送的正确的包数					//
    unsigned int wtpWirelessradioRecvBytes;			//
    unsigned long long wtpWirelesscoreRecvBytes;																		//
    unsigned long long wtpWirelessRecvFlowByte;		//无线侧接收的流量字节数（AP重启不会清零）
    unsigned long long wtpWirelessSendFlowByte;		//无线侧发送的流量字节数（AP重启不会清零）
    
    struct WtpDataPktsInfo *next;
	struct WtpDataPktsInfo *WtpDataPktsInfo_list;
	struct WtpDataPktsInfo *WtpDataPktsInfo_last;
};

struct WtpStatsInfo {
    int wtpCurrID;
    unsigned char *wtpMacAddr;
	unsigned char wtpRadioCount;						//add for radio count
	char *wtpModel;									//AP模式
	unsigned char wtpBwlanNum;							//add for wtpBwlanNum  for showing wlan infor
    unsigned int wtpConfigBSSIDNum;				//配置的BSSID总数
    unsigned char wtpSSIDNum;					//SSID总数				//
	unsigned char wtpSupportSSID[WLAN_NUM] ; 		//should make 
    unsigned int wtpAllowConUsrMnt;				//AP允许接入的用户数

	/*station information*/
  	unsigned int wtpMountConUsrTimes;			//用户接入次数
    unsigned int wtpMountConSuccTimes;			//assoc success times
    unsigned int wtpMountReConSuccTimes;		//reassco success times
    unsigned int wtpMountConFailTimes;			//用户接入失败次数
    unsigned int wtpMountReConFailTimes;			//reassoc failure times
    unsigned int wtpMountConReasTimes;			//用户重关联次数
    unsigned int wtpOnlineUsrNum;				//AP在线用户数
    unsigned int wtpStatsDisassociated;		//由于终端异常而导致断开连接的次数
    unsigned int wtpAllStaAssTime;			//AP接入总时间

	/*station connect information*/	
    unsigned int wtpUsrRequestConnect;			//用户总的请求连接数
    unsigned int wtpResponseUsrConnect;		//AP响应用户连接数
    unsigned int wtpLoginSuccNum;			//成功接入次数
    unsigned int wtpLessResourceRefuseNewUsrMount;		//资源不足而导致拒绝接入的新用户个数
	unsigned int wtpResponseUsrSuccConnect;	//用户请求连接成功数
	unsigned int wtpResponseUsrFailConnect;	//用户请求连接失败数

    float wtpUsrWirelessResoUseRate;//用户侧（无线侧）资源可用率//mib 加wtpOnlineUsrNum/wtpAllowConUsrMnt
    unsigned char total_if_num;         //zhangshu copy from 1.2, 2010-09-14
   /*fengwenchao add 20111125 for GM-3*/
    unsigned int heart_time_avarge;		//AC向该AP发送心跳的平均延迟时间								
    unsigned int heart_lose_pkt;			//AC丢失该AP心跳响应总包数							
    unsigned int heart_transfer_pkt;		//AC向该AP发送心跳请求总包数						
   /*fengwenchao add end*/
    unsigned int is_refuse_lowrssi;	 /*fengwenchao add for chinamobile-177.20111122*/	
    struct WtpStatsInfo  *next;
	struct  WtpStatsInfo *WtpStatsInfo_list;
	struct  WtpStatsInfo *WtpStatsInfo_last;
};

struct WtpWlanStatsInfo_wlan{
	unsigned char wlan_id;

	unsigned int wtpAttachedFlowThreshold;			 //流量负载均衡阈值
	unsigned int wtpUsrNumAppendThreshold; 		 //用户负载均衡阈值
	unsigned char wtpLoadBalance;				 //负载均衡
	unsigned char *wirelessSSID;					 //无线SSID
	unsigned char wtpSsidBroadcast; 			 //SSID是否广播
    unsigned char wlanSecID;
	char *wlan_essid;

	/*security infor*/
	unsigned char * wtpConRadiusServerIP;			//Radius server IP
    unsigned int wtpConRadiusServerPort;			//Radius server 端口号
    unsigned int wtpEAPAuthenSupport;				//EAP 模块是否开启
    unsigned int wtpConfSecurMech;				//当前配置的安全策略


	unsigned int wtpSsidCurrOnlineUsrNum;			//SSID当前在线用户数//asd
    unsigned int wtpAcAppointDistOnlineUsrNum;			//指定域在线用户数//asd

	unsigned char wlanL2Isolation;				//xiaodawei add isolation for wlan, 20110304
	
	/*fengwenchao add 20110426 for dot11WlanDataPktsTable*/
	/*-----------------------------------------------*/
	unsigned int ethernetRecvCorrectFrames;                    //网络侧（有线侧）MAC层接收到的正确数据帧数目
	unsigned long long ethernetRecvCorrectBytes;            //网络侧（有线侧）MAC层接收到的数据正确数据包字节数
	unsigned long long ethernetSendCorrectBytes;           //网络侧（有线侧）MAC层接发送出去的数据正确数据包字节数
	/*-----------------------------------------------*/
	
	struct WtpWlanStatsInfo_wlan *next;
	struct WtpWlanStatsInfo_wlan *WtpWlanStatsInfo_wlan_list;
	struct WtpWlanStatsInfo_wlan *WtpWlanStatsInfo_wlan_last;
};


struct WtpWlanStatsInfo {
	/*set ap statistics enable*/
    int wtpCurrID;
    unsigned char *wtpMacAddr;
 	unsigned char wtpBwlanNum;							//add for wtpBwlanNum  for showing wlan infor
 	unsigned char wtpSupportSSID[WLAN_NUM] ; 		//should make 
    unsigned char wtpL2Insulate;					//二层隔离是否开启		//f

    long wtpSSIDPriority;				//SSID 优先级			//mib自己加，为默认1

	struct WtpWlanStatsInfo_wlan *WtpWlanStatsInfo_wlan_head;
    struct WtpWlanStatsInfo *next;
	struct WtpWlanStatsInfo *WtpWlanStatsInfo_list;
	struct WtpWlanStatsInfo *WtpWlanStatsInfo_last;

};

struct SSIDStatsInfo_sub_wlan{
    unsigned char wlanCurrID;
	/*should input update wtp bss pakets infomation*/
	/*------------------------------------------*/
    unsigned long long SSIDChStatsDwlinkTotRetryPkts;	//SSID下行重传包数		//wtp f
    unsigned long long SSIDChStatsUplinkUniFrameCnt;			//SSID上行单播帧数//wtp f
    unsigned long long SSIDChStatsDwlinkUniFrameCnt;			//SSID下行单播帧数//wtp f
    unsigned long long SSIDUpChStatsFrameNonUniFrameCnt;		//SSID上行非单播帧数//wtp f
    unsigned long long SSIDDownChStatsFrameNonUniFrameCnt;		//SSID下行非单播帧数//wtp f
    
    unsigned long long SSIDDwlinkTotFrameCnt;		//SSID下行的总帧数//wtp f
    unsigned long long SSIDUplinkTotFrameCnt;		//SSID上行的总帧数//wtp f
	/*------------------------------------------*/

	unsigned int SSIDRxCtrlFrameCnt;			//SSID接收的控制帧数
    unsigned int SSIDRxDataFrameCnt;			//SSID接收的数据帧数
    unsigned int SSIDRxAuthenFrameCnt;			//SSID接收的认证帧数
    unsigned int SSIDRxAssociateFrameCnt;		//SSID接收的关联帧数
    unsigned int SSIDTxCtrlFrameCnt;			//SSID发送的控制帧数
    unsigned int SSIDTxDataFrameCnt;			//SSID发送的数据帧数
    unsigned int SSIDTxAuthenFrameCnt;			//SSID发送的认证帧数
    unsigned int SSIDTxAssociateFrameCnt;		//SSID发送的关联帧数
/*fengwenchao add 20110127*/
	unsigned int SSIDDwErrPkts;                 //SSID下行的错误包数
	unsigned int SSIDDwDropPkts;				//SSID下行的总丢帧数
	unsigned int SSIDDwTotErrFrames;			//SSID下行的总错帧数
	unsigned int SSIDUpErrPkts;					//SSID上行的错误包数
	unsigned int SSIDUpDropPkts;				//SSID上行的总丢帧数
	unsigned int SSIDUpTotErrFrames;			//SSID上行的总错帧数
/*fengwenchao add end*/    
    unsigned int SSIDApChStatsNumStations;			//使用该信道的终端数
    unsigned int SSIDAccessTimes;				//access times
    
	/*table 34*/
	/*--------------------------------------*/
	/*在wtp 结点下输入update wtp bss pakets infomation*/
	unsigned char *wtpSSIDName;
	unsigned int wtpSSIDMaxLoginUsr;
	unsigned char wtpSSIDState;
	unsigned char wtpSSIDSecurityPolicyID;
	unsigned char wtpSSIDLoadBalance;
	char *wtpSSIDESSID;
	unsigned char *WlanName;
	unsigned int vlanid;	//xiaodawei add vlanid 20101028
	/*--------------------------------------*/

	/*for dot11RadioWlanTable   fengwenchao add 20110331*/ 
	/*--------------------------------------*/
	unsigned char traffic_limit_able;   //速率限制状态
	unsigned int send_traffic_limit;    //速率限制门限
	/*--------------------------------------*/

	struct SSIDStatsInfo_sub_wlan *next;
	struct SSIDStatsInfo_sub_wlan *SSIDStatsInfo_sub_wlan_list;
	struct SSIDStatsInfo_sub_wlan *SSIDStatsInfo_sub_wlan_last;

/*fengwenchao add 20110127*/
	unsigned long long wl_up_flow;  //无线上行端口流量
    unsigned long long wl_dw_flow;  //无线下行端口流量
    unsigned long long ch_dw_pck;		   // 信道下行总包数
    unsigned long long ch_dw_los_pck;    //信道下行总的丢包数
    unsigned long long ch_dw_mac_err_pck;  //信道下行MAC错包数
    unsigned long long ch_dw_resend_pck;  //信道下行重传包数
    unsigned long long ch_up_frm;		   //信道上行总的帧数
    unsigned long long ch_dw_frm;    //信道下行总的帧数
    unsigned long long ch_dw_err_frm;	   //信道下行错帧数
    unsigned long long ch_dw_los_frm;  //信道下行总的丢帧数
    unsigned long long ch_dw_resend_frm;  //信道下行总的重传帧数
    unsigned long long ch_up_los_frm;   //信道上行总的丢帧数                    fengwenchao modify 20110224
    unsigned long long ch_up_resend_frm;  //信道上行总的重传帧数           fengwenchao modify 20110224

	unsigned int WlanradioRecvSpeed;  //抽样时间内无线上行端口速率
	unsigned int WlanradioSendSpeed;  //抽样时间内无线下行端口速率
/*fengwenchao add end*/
};

//mahz add 2011.11.9 for GuangZhou Mobile
struct WtpStationinfo {
	unsigned int	wtpid;
	char 		 *wtpMacAddr;				//AP的MAC地址	
	unsigned int no_auth_sta_num;			/*免认证接入用户数*/
	unsigned int assoc_auth_sta_num;		/*关联认证接入用户数*/
	
	unsigned int no_auth_accessed_total_time;		/*免认证用户连接AP总时长*/
	unsigned int assoc_auth_accessed_total_time;	/*关联认证用户连接AP 总时长*/
	
	unsigned int no_auth_sta_abnormal_down_num;		/*异常掉线次数*/
	unsigned int assoc_auth_sta_abnormal_down_num;

	unsigned int assoc_auth_req_num;
	unsigned int assoc_auth_succ_num;
	unsigned int assoc_auth_fail_num;
	//qiuchen
	unsigned int assoc_auth_online_sta_num;			/* WEP/PSK associate auth  (SHARE:WEP) */
	unsigned int auto_auth_online_sta_num;			/* SIM/PEAP */
	unsigned int all_assoc_auth_sta_total_time;		/*all WEP/PSK sta online time */
	unsigned int auto_auth_sta_total_time;			/* SIM/PEAP sta online time */
	unsigned int assoc_auth_sta_drop_cnt;		/* WEP/PSK assoc auth sta drop count */
	unsigned int auto_auth_sta_drop_cnt;			/* SIM/PEAP sta drop count */
	unsigned int weppsk_assoc_req_cnt, weppsk_assoc_succ_cnt, weppsk_assoc_fail_cnt;	
	unsigned int auto_auth_req_cnt, auto_auth_suc_cnt, auto_auth_fail_cnt;
	unsigned int auto_auth_resp_cnt;	/* radius response auth count */
    struct WtpStationinfo *next;
};

#if 0 //for old version
struct SSIDStatsInfo {
    unsigned char *wtpMacAddr;
    int wtpCurrID;
	unsigned char wtpBwlanNum;
 	unsigned char wtpSupportSSID[WLAN_NUM] ; 		//should make ]
 	unsigned char wtpBssNum;
	
    unsigned int SSIDTxSignalPkts;			//SSID发送的信令包数	//wtp f
    unsigned int SSIDRxSignalPkts;			//SSID接收的信令包数	//wtp f
    
    unsigned int SSIDTxDataPkts;			//SSID发送的数据包数//wtp f
    unsigned int SSIDRxDataPkts;			//SSID接收的数据包数//wtp f
    unsigned long long SSIDUplinkDataOctets;			//SSID接收的字节数//wtp f
    unsigned long long SSIDDwlinkDataOctets;			//SSID发送的字节数//wtp f
    unsigned int SSIDApChStatsFrameFragRate;			//信道上帧的分段速率//wtp f

	
    struct SSIDStatsInfo_sub_wlan *SSIDStatsInfo_sub_wlan_head;
    struct SSIDStatsInfo *next;
	struct SSIDStatsInfo *SSIDStatsInfo_list;
	struct SSIDStatsInfo *SSIDStatsInfo_last;
};
#endif
/* zhangshu copy from 1.2,2010-09-13 */
/*zhaoruijia,20100805,添加Radioid结点到wtp下*/
struct SSIDStatsInfo_Radioid_info{
    
  unsigned int radioId;
  unsigned char radioBwlanNum;
  unsigned char RadioidSupportSSID[WLAN_NUM];
  MixedGreenfieldParameter Mixed_Green_Field;   //fengwenchao add 20110331
  struct SSIDStatsInfo_sub_wlan     *SSIDStatsInfo_sub_wlan_head;
  struct SSIDStatsInfo_Radioid_info *next;
  struct SSIDStatsInfo_Radioid_info *SSIDStatsInfo_Radioid_info_list;
  struct SSIDStatsInfo_Radioid_info *SSIDStatsInfo_Radioid_info_last;

};
struct SSIDStatsInfo {
    unsigned char *wtpMacAddr;
    int wtpCurrID;
	//unsigned char wtpBwlanNum;
	unsigned char wtpBwlanRadioNum;//wtp 下可用的radio数
 	unsigned char wtpSupportSSID[WLAN_NUM] ; 		//should make ]
 	unsigned int wtpSupportRadioId[L_RADIO_NUM];
 	unsigned char wtpBssNum;
	
    unsigned int SSIDTxSignalPkts;			//SSID发送的信令包数	//wtp f
    unsigned int SSIDRxSignalPkts;			//SSID接收的信令包数	//wtp f
    
    unsigned int SSIDTxDataPkts;			//SSID发送的数据包数//wtp f
    unsigned int SSIDRxDataPkts;			//SSID接收的数据包数//wtp f
    unsigned long long SSIDUplinkDataOctets;			//SSID接收的字节数//wtp f
    unsigned long long SSIDDwlinkDataOctets;			//SSID发送的字节数//wtp f
    unsigned int SSIDApChStatsFrameFragRate;			//信道上帧的分段速率//wtp f

	
   // struct SSIDStatsInfo_sub_wlan *SSIDStatsInfo_sub_wlan_head;
    struct SSIDStatsInfo_Radioid_info *SSIDStatsInfo_Radioid_info_head;
    struct SSIDStatsInfo *next;
	struct SSIDStatsInfo *SSIDStatsInfo_list;
	struct SSIDStatsInfo *SSIDStatsInfo_last;
};


struct WtpIfInfo_sub_info{
	/* should input set ap interface infomation report switch enable to get the data*/
	unsigned char	wtpIfIndex;		  //AP有线接口索引						//ethindex
	unsigned int	wtpIfSpeed;		  //AP有线接口速率						//rate *1000
	unsigned int    wtpMTU;           //AP有线接口最大传输单元          fengwenchao add 20110127 for XJDEV-32 from 2.0
	unsigned char	wtpIfPhyAddr[6];	  //AP有线接口物理地址
	char *wtpIfIntro;				  //AP有线接口描述
	char *wtpIfName;				  //AP有线接口名称
	unsigned char	wtpIfType; 				  //AP有线接口类型				//type
	unsigned char 	wtpifinfo_report_switch;
	unsigned char	wtpIfAdminStatus;			  //AP有线接口状态管理	//state
	unsigned char	wtpIfOperStatus;			  //AP有线接口当前状态	//state

	unsigned int    wtpIfUplinkRealtimeRate; //AP有线接口上行实时速率
	unsigned int    wtpIfDownlinkRealtimeRate; //AP有线接口下行实时速率
	time_t state_time;
	time_t wtpIfLastChange;			  //AP有线接口当前状态持续时长// mib 从state_time 计算出

	struct WtpIfInfo_sub_info *next;
	struct WtpIfInfo_sub_info *WtpIfInfo_sub_info_list;
	struct WtpIfInfo_sub_info *WtpIfInfo_sub_info_last;
	
};

struct WtpIfnameInfo {
	int wtpCurrID;			  //AP当前ID
	unsigned char *wtpMacAddr;                 //AP的MAC地址
	unsigned char *wtpModel;
	unsigned char wtpIfIndexNum;
	unsigned int wtpMTU;                      //AP有线接口最大传输数据单元
	
	struct WtpIfInfo_sub_info *WtpIfInfo_sub_info_head;
    struct WtpIfnameInfo *next;
	struct WtpIfnameInfo *WtpIfnameInfo_list;
	struct WtpIfnameInfo *WtpIfnameInfo_last;
	
};

struct Sub_RadioParaInfo{
	unsigned char wtpRadLocalID;                 //Radio 的Local ID
	unsigned int  wtpRadCurrID;		    //Radio 的ID

	
	unsigned short wtpFrequencyHopTimes;          //由于避免信道干扰而导致频率改变的次数
	unsigned short wtpFreHopDetectTime;           //由于避免信道干扰而导致频率改变经过的时长
	
	unsigned char wtpConfigLongRetransThreshold; //长重传阈值
	unsigned short wtpMessageNeafThreshold;       //信息分片阈值
	unsigned short wtpRTSThreshold;               //RTS阈值
	unsigned short wtpSignalAveIntensity;         //平均信号强度
	
	unsigned char wtpPreambleLen;                //preamble长度

	struct Sub_RadioParaInfo *next;
	struct Sub_RadioParaInfo *Sub_RadioParaInfo_list;
	struct Sub_RadioParaInfo *Sub_RadioParaInfo_last;

};

struct WtpRadioParaInfo {
	int wtpCurrID;                         //AP的ID
	unsigned char *wtpMacAddr;		    //AP的MAC地址
	unsigned char wtpRadioNum;

	long wtpAntAPlus;                   //天线增益（802.11a）
	long wtpAntBGPlus;                  //天线增益（802.11b/g）
	char wtpSignalPathMode;            //AP信道模式
	unsigned char wtpSignalSNR;                 //信号信噪比	//wtp
	

	struct Sub_RadioParaInfo *Sub_RadioParaInfo_head;
    struct WtpRadioParaInfo *next;
    struct WtpRadioParaInfo *WtpRadioParaInfo_list;
    struct WtpRadioParaInfo *WtpRadioParaInfo_last;
};

struct WtpEthPortInfo {
	int wtpCurrID;             	 //AP当前ID
	unsigned char *wtpMacAddr;                //AP的MAC地址
	
	unsigned int  wtpWirelessUpPortRate;      //采样周期内上行无线接口速率
	unsigned int  wtpWirelessDownPortRate;    //采样周期内下行无线接口速率
	
	unsigned char wtpWirelessUpPortUDTimes;   //采样周期内上行无线接口UpDown次数
	unsigned char wtpWirelessDownPortUDTimes; //采样周期内下行无线接口UpDown次数
	
	unsigned int wtpUplinkDataThroughput;    //以太网指定的上行数据流量
	unsigned int wtpDownlinkDataThroughput;  //以太网指定的下行数据流量
	unsigned char wtpPhyInterMnt;             //AP物理接口总数	
	
    struct WtpEthPortInfo *next;
	struct WtpEthPortInfo *WtpEthPortInfo_list;
	struct WtpEthPortInfo *WtpEthPortInfo_last;

};

struct Sub_RadioStatsInfo{

	unsigned char wtpRadLocalID;		//Radio的Local ID
	unsigned int  wtpRadCurrID;		//Radio 的ID	
	
	unsigned short wtpCurSendPower;		//AP发射功率
	unsigned short wtpRecvPower;		//AP接收功率
	
	unsigned char wtpCurTakeFreqPoint;	//AP当前占用频段	//mib 自己加
	unsigned char wtpCurConfChannel;		//AP当前配置的信道
	unsigned int radio_type;	//算出wtpProtoMode用的
	long wtpProtoMode;		//AP协议模式	//由radio type得到mib自己加
	
	unsigned int wtpTerminalConRate;	//终端接入速率	
	char *wtpConfigAvailSpeed;      //有效速率集	//mib加默认值
	char *wtpConfigForceSpeed;      //强制速率集	//mib 加默认值
	long wtpRTSCTSEnable;           //是否使用RTS/CTS		//默认1
	
	unsigned short wtpconfigBeaconFrameBlank; //配置的beacon帧间隔

	struct Sub_RadioStatsInfo *next;
	struct Sub_RadioStatsInfo *Sub_RadioStatsInfo_list;
	struct Sub_RadioStatsInfo *Sub_RadioStatsInfo_last;

};


struct RadioStatsInfo {
	int wtpCurrID; 
	unsigned char *wtpMacAddr;		//AP的MAC地址
	unsigned char wtp_radio_num;
	unsigned int wtpPowerManage;		//AP功率管理				//nnnn
	unsigned char wtpSampleTime;	//AP采样时长 mib need *100		//nnnn
	
	struct Sub_RadioStatsInfo *Sub_RadioStatsInfo_head;
    struct RadioStatsInfo *next;
    struct RadioStatsInfo *RadioStatsInfo_list;
	struct RadioStatsInfo *RadioStatsInfo_last;
};
/*16-2*/
struct Sub_WtpConfigRadioInfo{
	unsigned char wtpRadLocalID;	//Radio的Local ID		//
	unsigned int  wtpRadCurrID;	//Radio ID			//
	unsigned int radioType;		//Radio类型
	unsigned int radioWlanNum;

	unsigned char radioChannel;	//Radio的信道				//
	unsigned short radioTxPower;	//Radio的发射功率	//
	unsigned int radioSpeed;	//Radio速率				//同radioMaxSpeed
	
	unsigned short radioBeacon;	//beacon				//
	unsigned short radioBeaf;		//分片门限	//
	unsigned char radioDtim;		//Dtim门限		//
	unsigned short radioRTS;		//RTS门限		//
	
	unsigned char radioLeadCode;	//前导码		//mib加判断
	unsigned char radioShortRetry;	//Short retry		//
	unsigned char radioLongRetry;	//Long retry		//
	unsigned char radioService;	//Radio服务			//
	
	unsigned int radioMaxSpeed;	//Radio最大速率				//
	unsigned char radioMaxFlow;	//最大流量		//
	unsigned char radioBindWlan[WLAN_NUM];	//绑定的wlan						
	unsigned char radioDelbindWlan[WLAN_NUM];	//Radio删除绑定的wlan  同radioDelbindWlan
	unsigned int radioBindQos;	//Radio绑定Qos					//   
	unsigned int radioDelbindQos;  //Radio删除绑定的Qos		//同radioDelbindQos

	u_int16_t guardinterval;			/* Huang Leilei add for snmp-Hu Pingxin: 2013-3-15 AXSSZFI-1323 */
	u_int16_t cwmode;					/* Huang Leilei add for snmp-Hu Pingxin: 2013-3-15 AXSSZFI-1323*/
	unsigned char Mixed_Greenfield;	/* Huang Leilei add for snmp-Hu Pingxin: 2013-3-15 AXSSZFI-1323*/
	unsigned char AmpduAble;			/* Huang Leilei add for snmp-Hu Pingxin: 2013-3-15 AXSSZFI-1323*/
	unsigned char AmsduAble;			/* Huang Leilei add for snmp-Hu Pingxin: 2013-3-15 AXSSZFI-1323*/
	u_int32_t mcs_count;				/* Huang Leilei add for snmp-Hu Pingxin: 2013-3-15 AXSSZFI-1323*/
	char mcs_list[L_BSS_NUM];			/* Huang Leilei add for snmp-Hu Pingxin: 2013-3-15 AXSSZFI-1323*/
	u_int32_t	Radio_Type;				/*a/b/g/n*/
		
	struct Sub_WtpConfigRadioInfo *next;
	struct Sub_WtpConfigRadioInfo * Sub_WtpConfigRadioInfo_list;
	struct Sub_WtpConfigRadioInfo * Sub_WtpConfigRadioInfo_last;
	u_int8_t	radio_work_role;
	unsigned char radio_channel_use_rate;
	unsigned int radio_channel_change_counter;
	
	unsigned int radio_channel_width;
	int radio_noise;
};
/*16*/
struct WtpConfigRadioInfo {
	int wtpCurrID;                         //AP的ID
	unsigned char *wtpMacAddr;	//AP的MAC地址
	unsigned char wtp_radio_num;
	
	struct Sub_WtpConfigRadioInfo *Sub_WtpConfigRadioInfo_head;
    struct WtpConfigRadioInfo *next;
	struct WtpConfigRadioInfo * WtpConfigRadioInfo_list;
	struct WtpConfigRadioInfo * WtpConfigRadioInfo_last;

	
};
/*17*/
struct UsrLinkInfo {
	int  wtpCurrID;			  //AP的当前ID
	unsigned char *wtpMacAddr; 			  //AP的MAC地址
	
	unsigned int wtpReasonInvalidFailLinkTimes;	  //由于无效而导致连接失败的次数
	unsigned int wtpReasonTimeOutFailLinkTimes;	  //由于超时而导致连接失败的次数
	unsigned int wtpReasonRefuseFailLinkTimes;	  //由于拒绝而导致连接失败的次数
	unsigned int wtpReasonOthersFailLinkTimes;        //由于其他原因而导致连接失败的次数
	
	unsigned int wtpSolutionLinksVerifyLinkTimes;     //解关联验证次数
	unsigned int wtpReasonUsrLeaveVerfiyLinkTimes;    //由于用户离开而解关联的次数
	unsigned int wtpReasonLackAbilityVerifyLinkTimes; //由于能力不足而解关联的次数
	unsigned int wtpReasonExceptionVerifyLinkTimes;   //由于异常而解关联的次数
	unsigned int wtpReasonOtherVerfyLinkTimes;        //其他原因解关联的次数
	
	unsigned int wtpStaOnlineTime;	          //所有station在线总时长				//mib *100
	unsigned long long wtpStaNewTotalOnlineTime;	  //所有station在线总时长之和//mib *100
		
    struct UsrLinkInfo *next;
	struct UsrLinkInfo *UsrLinkInfo_list;
	struct UsrLinkInfo *UsrLinkInfo_last;
};

/*19-1 book modify, 2011-1-19 */
struct WiredIfStatsInfo{

    unsigned char wtpIfIndex;               //有线接口索引
    unsigned char wtpWiredififUpDwnTimes;   //有线接口UpDown次数
    
	unsigned int wtpWiredifInUcastPkts;     //有线接口接收的单播包数		//
	unsigned int wtpWiredififInNUcastPkts;  //有线接口接收的非单播包数	//
	unsigned int wtpWiredififInPkts;        //有线接口接收的总字节数
	unsigned int wtpWiredififInDiscardPkts; //有线接口接收的丢包数
	unsigned int wtpWiredififInErrors;      //有线接口接收的错包数
	unsigned int wtpWiredifInBcastPkts;
	unsigned int wtpWiredififOutUcastPkts;  //有线接口发送的单播包数		//
	unsigned int wtpWiredififOutNUcastPkts; //有线接口发送的非单播包数	//
	unsigned int wtpWiredififOutPkts;       //有线接口发送的总字节数
	unsigned int wtpWiredififOutDiscardPkts;//有线接口发送的丢包数
	unsigned int wtpWiredififOutErrors;     //有线接口发送的错报数	
	unsigned int wtpWiredifOutBcastPkts;

	
	unsigned long long wtpWiredififInOctets;      //有线接口接收字节数
	unsigned long long wtpWiredififOutOctets;     //有线接口发送字节数

	unsigned long long rx_sum_bytes ;  // 新加    （接口收到的总字节数）20100910
   	unsigned long long tx_sum_bytes ;  // 新加    （接口发出的总字节) 		20100910

   	struct WiredIfStatsInfo *next;
};

/*19-2 */
struct WtpWiredIfStatsInfo {
	/*SYSTEM(config-wtp 1)# set ap extension infomation switch enable
	 SYSTEM(config)# set ap statistics enable*/
	int wtpCurrID;	                         //AP当前ID
	unsigned char *wtpMacAddr;                        //Ap的MAC地址
	unsigned char wtpWireIfNum;				//add for showing

	struct WiredIfStatsInfo *EthInfo;
		
    struct WtpWiredIfStatsInfo *next;
    struct WtpWiredIfStatsInfo *WtpWiredIfStatsInfo_list;
	struct WtpWiredIfStatsInfo *WtpWiredIfStatsInfo_last;
	
};

/*20-2*/
struct Sub_WtpWirelessIfInfo {
	/*"set ap interface infomation report switch (enable|disable)",*/

	unsigned char  wtpWirelessIfIndex;                      //AP无线接口索引
	unsigned int g_radio_id;
	char *wtpWirelessIfDescr;                      //AP无线接口描述									//mib 加
	unsigned char radio_has_bss;					//这个radio有没有bss
	unsigned int surport_rate_num;					//for showting
	
	unsigned char wtpWirelessIfType;                       //AP无线接口类型				//mib默认41
	unsigned int wtpWirelessIfSpeed;              //AP无线接口速率
	unsigned char *wtpWirelessIfPhysAddress;                //AP无线接口物理地址		//bssid	
	unsigned char wtpWirelessIfAdminStatus;                 //AP无线接口状态管理				//
	unsigned char wtpWirelessIfOperStatus;                  //AP无线接口当前状态					//state 				
	unsigned int wtpWirelessIfLastChange;         //AP无线接口当前状态持续时长					//已扩大100	
	unsigned char wtpWirelessIfRadioChannelAutoSelectEnable;//AP无线接口是否开启自动信道选择//
	unsigned char wtpWirelessIfRadioChannelConfig;          //AP无线接口信道配置					//
	unsigned int wtpWirelessIfDiversitySelectionRx;        //AP无线是否支持分集模式			//默认0
	unsigned int wtpWirelessIfCurrRadioModeSupport;        //AP当前radio支持的模式				//
	int wtpWirelessIfTransmitSpeedConfig[20];        //AP无线接口传输速率配置	
	unsigned short wtpWirelessIfPwrAttValue;                 //AP无线接口功率衰减配置			//
	unsigned char wtpWirelessIfPowerMgmtEnable;             //AP无线接口是否开启自动功率控制//
	unsigned short wtpWirelessIfTxPwrStep;// AP txpower step
			
    struct Sub_WtpWirelessIfInfo *next;
	struct Sub_WtpWirelessIfInfo *Sub_WtpWirelessIfInfo_list;
	struct Sub_WtpWirelessIfInfo *Sub_WtpWirelessIfInfo_last;
};

/*20*/
struct WtpWirelessIfInfo {
	
	int wtpCurrID;                                //AP当前ID				//wtp
	unsigned char *wtpMacAddr;	                       //AP的MAC地址		//wtp
	unsigned char wifi_num;
	unsigned char radio_count;		//xiaodawei add radiocount from wtpcompatible.xml, 20110124
	unsigned char *wtpModel;
	unsigned int wtpWirelessIfMTU;
	unsigned int wtpWirelessIfPwrAttRange;                 //AP无线接口功率衰减范围				//wtp
	unsigned int wtpWirelessIfAntennaGain;                 //AP无线接口天线增益					//wtp
	unsigned int wtpWirelessIfMaxStationNumPermitted;      //AP无线接口允许接入的最大station数	//wtp
	unsigned int wtpWirelessIfMaxTxPwrLvl;                //AP无线接口最大发射功率				//wtp
	
	struct Sub_WtpWirelessIfInfo *Sub_WtpWirelessIfInfo_head;
    struct WtpWirelessIfInfo *next;
	struct WtpWirelessIfInfo *WtpWirelessIfInfo_list;
	struct WtpWirelessIfInfo *WtpWirelessIfInfo_last;
};

/*liuzhenhua append 2010-05-28-*/
struct WtpStaInfo {
	
	char 		 *wtpMacAddr;				//AP的MAC地址	
	char 		 *wtpTerminalMacAddr;			//端站的MAC地址
	long 		  wtpStaIp;					//端站的IP
	long 		  wtp_sta_realip;	//real ip (in Centralized authentication local forwarding)/* yjl 2014-2-28 */
	unsigned char forward_mode;    //0:local 1:tunnel 2 tunnel auth, local forward            /* yjl 2014-2-28 */
	struct in6_addr wtpStaIp6;					//端站的IP6地址	
	unsigned long wtpWirelessClientSNR;		//无线端站的信噪比
	unsigned long long wtpSendTerminalPackMount;		//发送到端站的总包数//xiaodawei modify, change 32bits to 64bits, 20110309
	unsigned long long wtpSendTerminalDataPackMount;	//data pkts sent to terminal, xiaodawei add, 20110309
	unsigned long long wtpSendTerminalByteMount;		//发送到端站的总字节数//xiaodawei modify, change 32bits to 64 bits, 20110104
	unsigned long long wtpTerminalRecvPackMount;		//从端站接收的总包数//xiaodawei modify, change 32bits to 64bits, 20110309
	unsigned long long wtpTerminalRecvDataPackMount;	//data pkts received from terminal, xiaodawei add, 20110309
	unsigned long long wtpTerminalRecvByteMount;		//从端站接收的总字节数//xiaodawei modify, change 32bits to 64 bits, 20110104
	unsigned long wtpTerminalResPack;		//端站重传报数
	unsigned long wtpTerminalResByte;		//端站重传字节数
	unsigned long wtpTerminalRecvWrongPack;		//端站接收的错包数
	unsigned long wtpMacTermAddrUsrOnlineTime;	//基于mac地址的用户在线时长
	unsigned long long wtpMacTermAddrUsrSendSpd;			//基于mac地址的用户发送速率	//xiaodawei modify, change 32bits to 64bits, 20110309
	unsigned long long wtpMacTermAddrUsrRecvSpd;			//基于MAC地址的用户接受速率	//xiaodawei modify, change 32bits to 64bits, 20110309
	unsigned long long wtpMacTermAddrUsrAllThroughput;		//基于mac地址的用户总流量		//xiaodawei modify, change 32bits to 64bits, 20110309
	char*		  wtpMacTermAPReceivedStaSignalStrength;	//AP从端站接收到的当前信号强度
	unsigned long long wtpMacTermStaTxFragmentedPkts;		//端站发送的总的分片包数//xiaodawei modify, change 32bits to 64bits, 20110309
	unsigned long long wtpMacTermAPTxFragmentedPkts;		//AP发送给端站的总的分片包数//xiaodawei modify, change 32bits to 64bits, 20110309
	unsigned int MAXofRateset;	/* 终端与AP刚关联时根据双方能力而协商的无线速率集中的最高速率 */
	struct WID_WTP_STA_INFO wtp_sta_statistics_info;
	long 		  wtpBelongAPID;				//所属的AP的IP地址
	char 		 *wtpterminalaccesstime;			//终端接入时长
	unsigned int wtpterminalaccesstime_int;
	char*        wtpName;		
	unsigned int	security_type;		//mahz add 2011.11.10
	unsigned char *identity;	/* sta name */	//qiuchen
	unsigned int wtpterminalaccesstime_int_sys;
    struct WtpStaInfo *next;
	time_t sta_online_time;//qiuchen add it 2012.10.31
	time_t sta_access_time;
	time_t sta_online_time_new;
};

/*------------------------------------------------*/
struct Wtp_TerminalInfo {
    int staqSecID;
    int staqWlanID;
    int  staqRadioID; 
	
    char *wtpTerminalMacAddr;				
    long wtpEndWMMSta;					//端站的WMM属性
    unsigned int wtpStaIPAddress;			//端站IP地址
	struct in6_addr wtpStaIP6Address;	        //ipv6 address    
    unsigned int wtpStaRadioMode;				//端站的无线模式
    long wtpStaRadioChannel;				//端站的信道
    long wtpAPTxRates;					//端站当前接入速率
    long wtpAPRxRates;					//sta tx rates, xiaodawei add, 20110103, ap report 0 now
    long wtpStaPowerSaveMode;				//端站保护模式
    long wtpStaVlanId;					//端站所接入的vlan的ID
    char *wtpStaSSIDName;				//SSID 名称
    long wtpStaAuthenMode;				//鉴权模式
    long wtpStaSecurityCiphers;				//端站加密类型
    long wtpAutenModel;					//端站认证模式
    int wtpEndStaID;					//端站所属apid
    unsigned int encryption_type;		//encryption_type  add for mib showting
    struct Wtp_TerminalInfo *next;
};

struct WtpTerminalInfo{
	int wtpCurrID;
	unsigned char *wtpMacAddr;
	int sta_num;    //Wtp_TerminalInfo entries number in terminalInfo_list;
	struct Wtp_TerminalInfo * terminalInfo_list;
	struct WtpTerminalInfo *next;
};
/*liuzhenhua append 2010-05-26*/
struct WlanDataPktsInfo{
		
	long  wlanCurrID;			//Wlan当前ID
	unsigned long long wtpSsidSendTermAllByte;		//指定SSID AP发送到终端的字节数
	unsigned long wtpSsidRecvTermAllPack;		//指定SSID AP从终端接收的包数
	unsigned long long wtpSsidRecvTermAllByte;		//指定SSID AP从终端接收的字节数	
	unsigned long long wtpSsidWirelessMacRecvDataRightByte;	/*xiaodawei modify,20101116, 指定SSID MAC层接收的正确的数据字节数*/
	unsigned long long wtpSsidWirelessMacSendDataRightByte;	/*xiaodawei modify,20101116, 指定SSID MAC层发送的正确的数据字节数*/
	unsigned long wtpSsidWiredMacRecvDataWrongPack;	//指定SSID MAC层接收的错误的数据包数
	unsigned long wtpNetWiredRecvPack;		//指定SSID 网络测（有线侧）接收的包数
	unsigned long wtpUsrWirelessMacRecvDataPack;/*xiaodawei modify,20101116, 无线侧MAC层收到的数据包数*/
	unsigned long wtpUsrWirelessMacSendDataPack;/*xiaodawei modify,20101116, 无线侧MAC层发送的数据包数*/
	unsigned long wtpNetWiredSendPack;		//指定SSID 网络侧（有线侧）发送的包数
	unsigned long WtpWirelessSendFailPkts;		//指定SSID 无线侧发送失败的包数
	unsigned long wtpWirelessResendPkts; 	//指定SSID 无线侧总的重传包数
	char *wtpWirelessWrongPktsRate; 	//指定SSID 无线侧错包率
	unsigned long wtpWirelessSendBroadcastMsgNum;	//指定SSID 无线侧发送的广播包数
	unsigned long wtpStaUplinkMaxRate;		//指定SSID station上行最大速率
	unsigned long wtpStaDwlinkMaxRate;		//指定SSID station下行最大速率
	unsigned long wtpNetWiredRecvErrPack;		//指定SSID 网络侧（有线侧）接收的错包数
	unsigned long wtpNetWiredRecvRightPack;		//指定SSID 网络侧（有线侧）接收的正确包数
	unsigned long long wtpNetWiredRecvByte;		//指定SSID 网络侧（有线侧）接收的字节数
	unsigned long long wtpNetWiredSendByte;		//指定SSID 网络侧（有线侧）发送的字节数
	unsigned long wtpNetWiredSendErrPack;		//指定SSID 网络侧（有线侧）发送的错包数
	unsigned long wtpNetWiredSendRightPack;		//指定SSID 网络侧（有线侧）发送的正确包数
	unsigned long wtpSsidSendDataAllPack;		//制定SSID 网络侧（有线侧）发送的所有包数
	char *wtpNetWiredRxWrongPktsRate;	//指定SSID 网络侧（有线侧）接收的错误包百分比
	char *wtpNetWiredTxWrongPktsRate;	//指定SSID 网络侧（有线侧）发送的错误包百分比
	unsigned int wtpSsidTxDataDropPkts;
	unsigned int wtpSsidRxDataDropPkts;
	
	struct WlanDataPktsInfo *next;
};
struct WtpWlanDataPktsInfo {
	long  wtpCurrID;			//AP当前ID
	char *wtpMacAddr;           //AP的MAC地址
	unsigned int wlan_num;    //numbers of entries in wlan_list
	struct WlanDataPktsInfo* wlan_list;
	
	struct WtpWlanDataPktsInfo* next;
};




struct WtpWlanRadioInfo {
    unsigned char wlanid;
	unsigned int WlanradioRecvSpeed ;//抽样时间内无线上行端口流量
    unsigned int WlanradioSendSpeed ;//抽样时间内无线下行端口流量
    struct WtpWlanRadioInfo* next;
};

/*26 27 31*/
/*set ap extension infomation switch (enable|disable)*/
struct NewWtpWirelessIfInfo {	
	int  wtpCurrID;					//ap id						//
	unsigned char  NewapWirelessIfIndex;			//无线接口	//
	unsigned int wtpRadCurrID;				//radio id				//
	unsigned char wtpRadLocalID; 	//radio local id   //for showing
	unsigned char *wtpMacAddr;	     //AP的MAC地址		//
	
	unsigned short NewwtpWirelessIfBeaconIntvl;		//beacon 间隔		//					//t26,t31							
	unsigned char NewwtpWirelessIfDtimIntvl;			//DTIM			//					//t26,t31
	unsigned char NewwtpWirelessIfShtRetryThld;		//ShortRetry		//					//t26,t31
	unsigned char NewwtpWirelessIfLongRetryThld;		//LongRetry		//					//t26,t31
	unsigned int NewwtpWirelessIfMaxRxLifetime;	//Received data packets lifetime(units ms)			//t26
	unsigned char NewWtpPreambleLen;				//前导码			//					//t26
	unsigned short NewRtsThreshold;				//分片					//					//t26
	unsigned short NewFragThreshlod;				//分片				//					//t26

	unsigned int wtpWiredIfInMulticastPkts;	//有线接口接收的组播包数				//t27
	unsigned int wtpWiredIfOutMulticastPkts;	//有线接口发送的组播包数			//t27
	unsigned int wtpWirelessIfMaxRxLifetime;	//AP接收数据包生存期					//t31

    struct NewWtpWirelessIfInfo *next;
	struct NewWtpWirelessIfInfo *NewWtpWirelessIfInfo_list;
	struct NewWtpWirelessIfInfo *NewWtpWirelessIfInfo_last;
};

struct NewWtpWirelessIfstatsInfo_radio{

	unsigned char wtpWirelessIfIndex;				//AP无线接口索引
	unsigned char wirelessIfUpdownTimes;				//AP无线接口UpDown次数

	unsigned int wirelessIfChStatsUplinkUniFrameCnt;		//无线信道上行单播的帧数
	unsigned int wirelessIfChStatsDwlinkUniFrameCnt;		//无线信道下行单播的帧数
	unsigned int wirelessIfUpChStatsFrameNonUniFrameCnt;	//无线信道上行非单播的帧数
	unsigned int wirelessIfDownChStatsFrameNonUniFrameCnt;	//无线信道下行非单播帧数

	unsigned int wirelessIfApChStatsFrameFragRate;		//信道上帧的分段速率
	unsigned int wirelessIfChStatsNumStations;			//使用该信道的终端数

	unsigned int wirelessIfRxMgmtFrameCnt;			//接收到的管理帧的数量
	unsigned int wirelessIfRxCtrlFrameCnt;			//接收到的控制帧的数量
	unsigned int wirelessIfRxDataFrameCnt;			//接收到的数据帧的数量
	unsigned int wirelessIfRxAuthenFrameCnt;			//接收到的认证帧的数量
	unsigned int wirelessIfRxAssociateFrameCnt; 		//接收到的关联帧的数量
	unsigned int wirelessIfTxMgmtFrameCnt;			//发送的管理帧的数量
	unsigned int wirelessIfTxCtrlFrameCnt;			//发送的控制帧的数量
	unsigned int wirelessIfTxDataFrameCnt;			//发送的数据帧的数量
	unsigned int wirelessIfTxAuthenFrameCnt;			//发送的认证帧的数量
	unsigned int wirelessIfTxAssociateFrameCnt; 		//发送的关联帧的数量

	unsigned int radioWlanNum;
	unsigned char radioBindWlan[WLAN_NUM];	//绑定的wlan	
	struct NewWtpWirelessIfstatsInfo_radio *next;
	struct NewWtpWirelessIfstatsInfo_radio *NewWtpWirelessIfstatsInfo_radio_list;
	struct NewWtpWirelessIfstatsInfo_radio *NewWtpWirelessIfstatsInfo_radio_last;
};

struct NewWtpWirelessIfstatsInfo {
    int wtpCurrID;
    unsigned char *wtpMacAddr;
	unsigned char wtp_radio_num;
    unsigned char wirelessIfAvgRxSignalStrength;			//AP无线接口接收的信号平均强度
    unsigned char wirelessIfHighestRxSignalStrength;		//AP无线接口接收信号的最大强度
    unsigned char wirelessIfLowestRxSignalStrength;		//AP无线接口接收信号的最低强度
    unsigned char wifiExtensionInfoReportswitch;		/*set ap extension infomation switch (enable|disable)*/
	
    unsigned int wirelessIfTxSignalPkts;			//发送的信令包数
    unsigned int wirelessIfRxSignalPkts;			//接收的信令包数
    unsigned int wirelessIfTxDataPkts;				//发送的数据包数//
    unsigned int wirelessIfRxDataPkts;				//接收的数据包数//
    unsigned long long wirelessIfUplinkDataOctets;			//发送的数据包字节数//
    unsigned long long wirelessIfDwlinkDataOctets;			//接收的数据包字节数//

    unsigned int wirelessIfChStatsDwlinkTotRetryPkts;		//无线信道下行重传的包数

	unsigned int wirelessIfChStatsPhyErrPkts;			//信道接收的物理层错包数//
    unsigned int wirelessIfChStatsMacFcsErrPkts;		//信道接收的FCS MAC错包数//
    unsigned int wirelessIfChStatsMacMicErrPkts;		//信道接收的MIC MAC错包数//
    unsigned int wirelessIfChStatsMacDecryptErrPkts;		//信道接收的解密失败的MAC错包数//
    unsigned int wirelessIfChStatsFrameErrorCnt;		//信道接收的错帧数				//
    unsigned int wirelessIfChStatsFrameRetryCnt;		//信道下行重传的帧数

	unsigned int asd_radio_num;						//asd 端radio num
	struct NewWtpWirelessIfstatsInfo_radio *NewWtpWirelessIfstatsInfo_radio_head;

    struct NewWtpWirelessIfstatsInfo *next;
	struct NewWtpWirelessIfstatsInfo *NewWtpWirelessIfstatsInfo_list;
	struct NewWtpWirelessIfstatsInfo *NewWtpWirelessIfstatsInfo_last;
};

struct 	Sub_RogueAPInfo
{
	int wtpID;
	int  rogueAPIndex;		//非法AP的索引
	unsigned char rogueAPMac[MAC_LEN];		//非法AP的MAC
	
	short rogueAPRate;		//非法AP的速率
	unsigned char rogueAPChannel;		//非法AP所在信道
	
	unsigned char rogueAPRssi;		//非法AP的Rssi
	unsigned char rogueAPNoise;		//非法AP噪声
	unsigned char rogueAPBeaconInterval;	//非法APbeacon间隔
	
	unsigned short rogueAPCapability;		//非法AP能力
	unsigned char rogueAPEssid[ESSID_DEFAULT_LEN+1];		//非法AP ESSID
	char *rogueAPElemInfo;		//非法AP网元信息
	
	struct Sub_RogueAPInfo *next;
	struct Sub_RogueAPInfo *Sub_RogueAPInfo_list;
	struct Sub_RogueAPInfo *Sub_RogueAPInfo_last;
};


/*table 29 dot11RogueAPTable*/
struct RogueAPInfo {	

	int wtpCurrID;
	unsigned int rogue_ap_num;
	unsigned char *wtpMacAddr;		//AP的MAC地址		
	
	struct Sub_RogueAPInfo *Sub_RogueAPInfo_head;
    struct RogueAPInfo *next;
    struct RogueAPInfo *RogueAPInfo_list;
    struct RogueAPInfo *RogueAPInfo_last;
};

/*table 30-2*/
struct Sub_SecurityMechInfo {	
	unsigned char  wlanSecID;				//无限安全策略ID
	unsigned char  wlanCurrID;				//wlan ID
	char *wtpWirelessSecurMechName;			//安全策略名称
	unsigned char wtpWirelessSecurMechID;			//安全策略ID
	unsigned int wtpWirelessSecurMechType;			//安全策略类型
	unsigned int wtpWirelessSecurMechEncryType;		//安全策略加密类型

	char *wtpWirelessSecurMechSecurPolicyKey;	//安全策略密钥
	int wtpWirelessSecurMechKeyInputType;		//安全策略输入密钥
	unsigned int wlan_num;						//for showwing

    struct Sub_SecurityMechInfo *next;
	struct Sub_SecurityMechInfo *Sub_SecurityMechInfo_list;
	struct Sub_SecurityMechInfo *Sub_SecurityMechInfo_last;
	
};

/*table 30 dot11SecurityMechTable*/
struct SecurityMechInfo {	
    int wtpCurrID;
	unsigned char *wtpMacAddr;				//AP的MAC地址                
	unsigned int wlan_num;						//for showwing

	struct Sub_SecurityMechInfo *Sub_SecurityMechInfo_head;
    struct SecurityMechInfo *next;
	struct SecurityMechInfo *SecurityMechInfo_list;
	struct SecurityMechInfo *SecurityMechInfo_last;
	
};
/*table 33 dot11ConjunctionTable*/
struct WtpInfor{
    int wtpCurrID;
	unsigned char *wtpMacAddr;				//AP的MAC地址          
	
	unsigned int acc_tms;						//e1
	unsigned int auth_tms;						//e2
	unsigned int repauth_tms;					//e3
	
	unsigned int auth_success_num ; 			//b1
	unsigned int auth_fail_num ; 
	unsigned int auth_invalid_num ; 
	unsigned int auth_timeout_num ; 
	unsigned int auth_refused_num ; 			//b5
	
	unsigned int auth_others_num ; 				//b6
	unsigned int assoc_req_num ; 
	unsigned int assoc_resp_num ; 
	unsigned int assoc_invalid_num ; 
	unsigned int assoc_timeout_num ; 			//b10

	unsigned int assoc_success_num;			//assoc success times
	
	unsigned int assoc_refused_num ; 			//b11
	unsigned int assoc_others_num ; 
	unsigned int reassoc_request_num ; 
	unsigned int reassoc_success_num ; 
	unsigned int reassoc_invalid_num ; 			//b15
	
	unsigned int reassoc_timeout_num ; 
	unsigned int reassoc_refused_num ; 
	unsigned int reassoc_others_num ; 
	unsigned int identify_request_num ; 
	unsigned int identify_success_num ; 		//b20
	
	unsigned int abort_key_error_num ; 			//b21
	unsigned int abort_invalid_num ; 
	unsigned int abort_timeout_num ; 
	unsigned int abort_refused_num ; 
	unsigned int abort_others_num ; 			//b25
	
	unsigned int deauth_request_num ; 			//b26
	unsigned int deauth_user_leave_num  ;
	unsigned int deauth_ap_unable_num ;
	unsigned int deauth_abnormal_num ; 
	unsigned int deauth_others_num ; 			//b30
	
	unsigned int disassoc_request_num ; 		//b31
	unsigned int disassoc_user_leave_num ; 
	unsigned int disassoc_ap_unable_num ; 
	unsigned int disassoc_abnormal_num ; 
	unsigned int disassoc_others_num ; 			//b35

	unsigned int rx_mgmt_pkts ;					//c1
	unsigned int tx_mgmt_pkts ;
	unsigned int rx_ctrl_pkts ;
	unsigned int tx_ctrl_pkts ;					//c4
	
	unsigned int rx_data_pkts ;					//c5
	unsigned int tx_data_pkts ;
	unsigned int rx_auth_pkts ;
	unsigned int tx_auth_pkts ;					//c8

	unsigned long long wtp_total_online_time ;	/*	xm0703*///a1a2
	unsigned int num_assoc_failure ; /*	xm0703*/	//z1

	//zhanglei add 20101007
	unsigned int rx_assoc_norate; /*因终端不支持基本速率集要求的所有速率而关联失败的总次数*/
	unsigned int rx_assoc_capmismatch;	  /*由不在802.11标准制定范围内的原因而关联失败的总次数*/
	unsigned int assoc_invaild;   /*未知原因而导致关联失败的总次数*/
	unsigned int reassoc_deny;	  /*由于之前的关联无法识别与转移而导致重新关联失败的总次数*/

	//mahz add 2011.5.5
	unsigned int num_assoc_record, num_reassoc_record, num_assoc_failure_record, num_reassoc_failure_record;
	unsigned int assoc_success_record, reassoc_success_record, assoc_req_record, assoc_resp_record;
	unsigned int auth_success_record, auth_fail_record,usr_auth_tms_record,ac_rspauth_tms_record;
	unsigned long long total_ap_flow_record;
	//
	
	struct WtpInfor *WtpInfor_list;
	struct WtpInfor *WtpInfor_last;
	struct WtpInfor *next;
	
};
/*nl add for mib showting wtp  infromation end*/
/*xdw add for show ap network information of all wtp, 20101215*/
struct WtpNetworkInfo {	
    unsigned int WTPID;
	unsigned char *WTPMAC;
	char * WTPIP;

	unsigned int ap_mask_new;	//ap mask
	unsigned int ap_gateway;	//ap gateway
	unsigned int ap_dnsfirst;	//ap first dns
	unsigned int ap_dnssecend;	//ap second dns
	
    struct WtpNetworkInfo *next;
	struct WtpNetworkInfo *WtpNetworkInfo_list;
	struct WtpNetworkInfo *WtpNetworkInfo_last;
};
struct WtpAthStatisticInfo{
    long wtpCurrID;   //AP当前ID
    unsigned char *wtpMacAddr;
    long  wtpwirelessifindex; //radio_local_ID
    long wlanid;
    unsigned int wirelessIfUpdownTimes;//radio上下时间合
    unsigned int wirelessIfTxSignalPkts; //发送的信令包数  apstatsinfo[i].tx_pkt_mgmt
    unsigned int wirelessIfRxSignalPkts; //接收的信令包数  apstatsinfo[i].rx_pkt_mgmt
 
    unsigned int wirelessIfTxDataPkts; //发送的数据包数//         apstatsinfo[i].tx_packets 
    unsigned int wirelessIfRxDataPkts; //接收的数据包数//         apstatsinfo[i].rx_packets 
    unsigned long long wirelessIfUplinkDataOctets; //发送的数据包字节数//apstatsinfo[i].tx_bytes 
    unsigned long long wirelessIfDwlinkDataOctets; //接收的数据包字节数//apstatsinfo[i].rx_bytes 
    unsigned int  wirelessIfChStatsDwlinkTotRetryPkts; //下行重传的包数//apstatsinfo[i].tx_pkt_retry 
    unsigned int  wirelessIfChStatsFrameRetryCnt;	//apstatsinfo[i].tx_pkt_retry*103%
    unsigned int  wirelessifUplinkUniPktCnt;// 上行单播的包数 //apstatsinfo[i].rx_unicast 
    unsigned int  wirelessifDwlinkUniPktCnt ;//下行单播的包数 apstatsinfo[i].rx_unicast 
    unsigned int  wirelessifUpNonUniPktCnt  ;//上行非单播的包数 apstatsinfo[i].rx_multicast 因为多播包数已经包含了广播包数
    unsigned int  wirelessifDownNonUniPktCnt;//滦蟹堑ゲサ陌?
    unsigned int  wirelessifFrameFragRate ;//帧的分段速率 B
    unsigned int  wirelessifRxCtrlFrameCnt ;//接收到的控制帧的数量 B
    unsigned int  wirelessifRxDataFrameCnt ;//接收到的数据帧的数量 B
    unsigned int  wirelessifRxAuthenFrameCnt ;//接收到的认证帧的数量 B
    unsigned int  wirelessifRxAssociateFrameCnt ;//接收到的关联帧的数量 B
    unsigned int  wirelessifTxCtrlFrameCnt ;//发送的控制帧的数量 B
    unsigned int  wirelessifTxDataFrameCnt;//发送的数据帧的数量 B
    unsigned int  wirelessifTxAuthenFrameCnt ;//发送的认证帧的数量  B
    unsigned int  wirelessifTxAssociateFrameCnt ; //发送的关联帧的数量 B
    unsigned int  wirelessIfSuccAssociatedNum;//associated successfully times, xiaodawei add, 20110418
    unsigned int  wirelessIfAssociatedUserNum;
    unsigned int  wirelessifNumStations; //当前连接到该SSID的终端数 、、黄涛
    unsigned int  wirelessIfTxMgmtFrameCnt; //发送的管理帧apstatsinfo[i].tx_pkt_mgmt ,zhaoruijia,20100903
    unsigned int  wirelessIfRxMgmtFrameCnt;//接收的管理帧 apstatsinfo[i].rx_pkt_mgmt  ,zhaoruijia,20100903
    unsigned int  wirelessifUplinkCtrlOctets;//发送的管理包字节数 apstatsinfo[i].tx_mgmt 
    unsigned int  wirelessifDownlinkCtrlOctets;//接收的管理包字节数 apstatsinfo[i].rx_mgmt 
    unsigned int wirelessIfChStatsMacFcsErrPkts; // 	信道接收的FCS MAC错包数apstatsinfo[i].ast_rx_crcerr
	unsigned int wirelessIfChStatsMacMicErrPkts;//	信道接收的MIC MAC错包数apstatsinfo[i].ast_rx_badmic
	unsigned int wirelessIfChStatsMacDecryptErrPkts;//	信道接收的解密失败的MAC错包数apstatsinfo[i].ast_rx_badcrypt
	unsigned int wirelessIfChStatsFrameErrorCnt; //信道接收的错帧数 apstatsinfo[i].tx_errors

	unsigned int wirelessIfChStatsPhyErrPkts;  //信道接收的物理层错包数  fengwenchao add 20101228
	unsigned char wirelessIfLowestRxSignalStrength ;    //AP无线接口接收信号的最低强度 fengwenchao add 20101228
	unsigned char wirelessIfHighestRxSignalStrength;    //AP无线接口接收信号的最大强度 fengwenchao add 20101228
	double wirelessIfAvgRxSignalStrength ;              // AP无线接口接收的信号平均强度fengwenchao add 20101228

	char wirelessIfHighestRxSignalStrength2;			//AP无线接口接收信号的最大强度 fengwenchao add 20101228
	char wirelessIfLowestRxSignalStrength2;	           //AP无线接口接收信号的最低强度 fengwenchao add 20101228
	
	
	/*for dot11WtpStatisticsTable ,dot11WtpChannelTable fengwenchao add 20110330*/
	/*----------------------------------------------------------------*/
   	//unsigned int wtpRxPacketLossRate;           //入丢包率
	//unsigned int wtpTxPacketLossRate;			//出丢包率
	unsigned int wtpTotalRx_Drop;               //ath口rx_drop的总和
	unsigned int wtpTotalTx_Drop;               //ath口tx_drop的总和
	unsigned int wtpTotalRx_Pkt;                //ath口rx_pkt的总和
	unsigned int wtpTotalTx_Pkt;                //ath口tx_pkt的总和
	unsigned int wtpDownBandwidthUtilization;   //下行带宽利用率
	unsigned int wtpUpBandwidthUtilization;   	//上行带宽利用率
	unsigned int wtpReceiveRate;                //AP上行速率
	unsigned int wtpSendRate;                   //AP下行速率
	unsigned int sta_num;                       //接入用户数量
	unsigned char monitor_time;                 //监控时长
	unsigned int ast_rx_phyerr;                 //物理层错包数
	unsigned int tx_packets;
	unsigned int tx_errors;
	unsigned int tx_retry;
	/*---------------------------------------------------------------*/


	/*for dot11WtpExtensionTable,dot11WtpWidStatisticsTable,dot11WtpBssIDNumTable  fengwenchao add 20110503*/
	/*----------------------------------------------------------------*/
	unsigned char wtpExtensionUsed;    //AP扩展信息开关
	unsigned int wtpFloodAttackTimes;        //泛洪攻击次数
	unsigned int wtpSpoofingAttackTimes;   //Spoof攻击次数
	unsigned int wtpWeakIVAttackTimes;     //WeakIV攻击次数
	unsigned int BssIDTatolNum;              //BSSID总数
	/*----------------------------------------------------------------*/
	unsigned long long wirelessIfDwlinkTotFrameCnt;	//tx_pkt_unicast+tx_pkt_multicast, tx_pkt_multicast(ap report)=multicast+broadcast 
	unsigned long long wirelessIfUplinkTotFrameCnt;	//rx_pkt_unicast+rx_pkt_multicast,rx_pkt_multicast(ap report)=multicast+broadcast
   	//qiuchen add for Guangdong mobile v3.2-1
   	unsigned int AllApUserOnlineTime;
	unsigned int APUserLostConnectionCnt;
	unsigned int APAuthReqCnt;
	unsigned int APAuthSucCnt;
	unsigned int APAuthFailCnt;
	struct WtpAthStatisticInfo	*next;
};

/*------------------------------------------------*/

struct WtpList{
	int WtpId;
	char FailReason;
	int WtpStaNum;
	//unsigned char wtpMacAddr[6];
	struct WtpList *next;
	struct WtpList *WtpList_list;
	struct WtpList *WtpList_last;
};

struct WtpOUIInfo_{
   unsigned char* oui_mac;
   unsigned int   oui_num;
   struct WtpOUIInfo_ *oui_list;
   struct WtpOUIInfo_ *oui_last;
   struct WtpOUIInfo_ *next;
};
typedef struct WtpOUIInfo_ WtpOUIInfo;
#endif

