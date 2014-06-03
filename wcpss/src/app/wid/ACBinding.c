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
* ACBinding.c
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
#include "wcpss/waw.h"

#include "wcpss/wid/WID.h"
#include "ACDbus_handler.h"
#ifdef DMALLOC
#include "../dmalloc-5.5.0/dmalloc.h"
#endif

CWBool CWACInitBinding(int i)
{
	int j;
	bindingValues* aux;
	wid_syslog_debug_debug(WID_DEFAULT,"CWACInitBinding start");
	CW_CREATE_OBJECT_ERR_WID(aux, bindingValues, return CWErrorRaise(CW_ERROR_OUT_OF_MEMORY, NULL););
	if((gWTPs[i].WTPProtocolManager).bindingValuesPtr){
		CW_FREE_OBJECT_WID(((bindingValues*)(gWTPs[i].WTPProtocolManager).bindingValuesPtr)->qosValues);		
		CW_FREE_OBJECT_WID((gWTPs[i].WTPProtocolManager).bindingValuesPtr);
	}
	(gWTPs[i].WTPProtocolManager).bindingValuesPtr=(void*) aux;

	CW_CREATE_ARRAY_ERR(aux->qosValues, NUM_QOS_PROFILES, WTPQosValues, return CWErrorRaise(CW_ERROR_OUT_OF_MEMORY, NULL););
	
	//Init default values
	for(j=0; j<NUM_QOS_PROFILES; j++)
	{
	/*zhanglei change for setting.ac.txt delete*/
//		aux->qosValues[j].cwMin=gDefaultQosValues[j].cwMin;
//		aux->qosValues[j].cwMax=gDefaultQosValues[j].cwMax;
//		aux->qosValues[j].AIFS=gDefaultQosValues[j].AIFS;
		
		aux->qosValues[j].cwMin=0;
		aux->qosValues[j].cwMax=0;
		aux->qosValues[j].AIFS=0;
		
		aux->qosValues[j].queueDepth=0;
		aux->qosValues[j].dot1PTag=0;
		aux->qosValues[j].DSCPTag=0;
	}
	wid_syslog_debug_debug(WID_DEFAULT,"CWACInitBinding end");
	return CW_TRUE;
}

CWBool CWMergeQosValues(int WTPIndex)
{
	int i;
	bindingValues* aux;

	aux=(bindingValues*)(gWTPs[WTPIndex].WTPProtocolManager).bindingValuesPtr;

	for(i=0; i<NUM_QOS_PROFILES; i++)
	{
		if(gWTPs[WTPIndex].qosValues[i].cwMin==UNUSED_QOS_VALUE) 
			{gWTPs[WTPIndex].qosValues[i].cwMin=aux->qosValues[i].cwMin;}
	
		if(gWTPs[WTPIndex].qosValues[i].cwMax==UNUSED_QOS_VALUE) 
			{gWTPs[WTPIndex].qosValues[i].cwMax=aux->qosValues[i].cwMax;}

		if(gWTPs[WTPIndex].qosValues[i].AIFS==UNUSED_QOS_VALUE) 
			{gWTPs[WTPIndex].qosValues[i].AIFS=aux->qosValues[i].AIFS;}
	}
	return CW_TRUE;
}

CWBool CWAssembleWTPChan(CWProtocolMessage *msgPtr, unsigned char radioID, unsigned char channel){
	int size = 8;
	CW_CREATE_PROTOCOL_MESSAGE(*msgPtr, size, return CWErrorRaise(CW_ERROR_OUT_OF_MEMORY, NULL););
	CWProtocolStore8(msgPtr, radioID);
	CWProtocolStore8(msgPtr, 0);
	CWProtocolStore8(msgPtr, channel);
	CWProtocolStore8(msgPtr, 0);
	CWProtocolStore32(msgPtr, 0);
	return CWAssembleMsgElem(msgPtr, BINDING_MSG_ELEMENT_TYPE_SET_CHAN);

}

CWBool CWAssembleWTPTXP(CWProtocolMessage *msgPtr, unsigned char radioID, unsigned short TX){
	int size = 4;
	wid_syslog_debug_debug(WID_DEFAULT,"txpoweroffset is %d(before assemblewtpxp)\n",TX);
	CW_CREATE_PROTOCOL_MESSAGE(*msgPtr, size, return CWErrorRaise(CW_ERROR_OUT_OF_MEMORY, NULL););
	CWProtocolStore8(msgPtr, radioID);
	CWProtocolStore8(msgPtr, 0);
	CWProtocolStore16(msgPtr, TX);
	wid_syslog_debug_debug(WID_DEFAULT,"txpoweroffset is %d  (after assemblewtpxp)\n",TX);
	return CWAssembleMsgElem(msgPtr, BINDING_MSG_ELEMENT_TYPE_SET_TXP);

}
CWBool CWAssembleWTPTXPOF(CWProtocolMessage *msgPtr, unsigned char radioID, unsigned short TX){
	int size = 4;
	wid_syslog_debug_debug(WID_DEFAULT,"txpoweroffset is %d(before assemblewtpxp)\n",TX);
	CW_CREATE_PROTOCOL_MESSAGE(*msgPtr, size, return CWErrorRaise(CW_ERROR_OUT_OF_MEMORY, NULL););
	CWProtocolStore8(msgPtr, radioID);
	CWProtocolStore8(msgPtr, 1);
	CWProtocolStore16(msgPtr, TX);
	wid_syslog_debug_debug(WID_DEFAULT,"txpoweroffset is %d(after assemblewtpxp)\n ",TX);
	return CWAssembleMsgElem(msgPtr, BINDING_MSG_ELEMENT_TYPE_SET_TXP);

}

//added by weiay
/*CWBool CWAssembleWTPRadioRate(CWProtocolMessage *msgPtr, unsigned char radioID, unsigned short rate)
{
	//printf("*****************CWAssembleWTPRadioRate*******************\n");
	int size = 3;
	CW_CREATE_PROTOCOL_MESSAGE(*msgPtr, size, return CWErrorRaise(CW_ERROR_OUT_OF_MEMORY, NULL););
	CWProtocolStore8(msgPtr, radioID);
	CWProtocolStore16(msgPtr, rate);

	return CWAssembleMsgElem(msgPtr, BINDING_MSG_ELEMENT_TYPE_IEEE80211_RATE_SET);
}*/
////*

CWBool CWAssembleWTPRadioRate1(CWProtocolMessage *msgPtr, unsigned char radioID,struct Support_Rate_List *rate)
{
	//printf("*****************CWAssembleWTPRadioRate*******************\n");
	int length = 0;
	int i = 0;
	struct Support_Rate_List *ptr = NULL;
	ptr = rate;
	length = length_of_rate_list(ptr);
	int size = length*2 + 2;
	CW_CREATE_PROTOCOL_MESSAGE(*msgPtr, size, return CWErrorRaise(CW_ERROR_OUT_OF_MEMORY, NULL););
	CWProtocolStore8(msgPtr, radioID);
	CWProtocolStore8(msgPtr, length*2);

	
	for(i=0;i<length;i++)
	{
		CWProtocolStore16(msgPtr, ptr->Rate);

		ptr = ptr->next;
		
	}
	
	return CWAssembleMsgElem(msgPtr, BINDING_MSG_ELEMENT_TYPE_IEEE80211_RATE_SET);
}
///*/
CWBool CWAssembleWTPRadioType(CWProtocolMessage *msgPtr, unsigned char radioID, unsigned int mode)
{
	//printf("*****************CWAssembleWTPRadioType*******************\n");
	int size = 5;
	CW_CREATE_PROTOCOL_MESSAGE(*msgPtr, size, return CWErrorRaise(CW_ERROR_OUT_OF_MEMORY, NULL););
	CWProtocolStore8(msgPtr, radioID);
	CWProtocolStore32(msgPtr, mode);

	return CWAssembleMsgElem(msgPtr, BINDING_MSG_ELEMENT_TYPE_IEEE80211_RADIO_TYPE_SET);	
}

//added end

CWBool CWAssembleWTPMacOperate(CWProtocolMessage *msgPtr, BindingRadioOperate stRadioOperate)
{
	//printf("*****************CWAssembleWTPMacOperate*******************\n");
	//printf("Radio ID:%d FragThreshold:%d\n",stRadioOperate.RadioID,stRadioOperate.FragThreshold);
	int size = 16;
	CW_CREATE_PROTOCOL_MESSAGE(*msgPtr, size, return CWErrorRaise(CW_ERROR_OUT_OF_MEMORY, NULL););
	CWProtocolStore8(msgPtr, stRadioOperate.RadioID);
	CWProtocolStore8(msgPtr, stRadioOperate.Reserved);
	CWProtocolStore16(msgPtr, stRadioOperate.RTSThreshold);

	CWProtocolStore8(msgPtr, stRadioOperate.Shortretry);
	CWProtocolStore8(msgPtr, stRadioOperate.Longretry);
	CWProtocolStore16(msgPtr, stRadioOperate.FragThreshold);

	CWProtocolStore32(msgPtr, stRadioOperate.TxMSDULifetime);
	CWProtocolStore32(msgPtr, stRadioOperate.RxMSDULifetime);

	return CWAssembleMsgElem(msgPtr, BINDING_MSG_ELEMENT_TYPE_IEEE80211_MAC_OPERATE);

}

CWBool CWAssembleWTPRadioConfiguration(CWProtocolMessage *msgPtr, BindingRadioConfiguration stRadioConfiguration)
{
	//printf("*****************CWAssembleWTPRadioConfiguration*******************\n");
	//printf("Radio ID:%d Preamble:%d Beacon Interval:%d\n",stRadioConfiguration.RadioID,stRadioConfiguration.IsShortPreamble,stRadioConfiguration.BeaconPeriod);
	int size = 16;
	CW_CREATE_PROTOCOL_MESSAGE(*msgPtr, size, return CWErrorRaise(CW_ERROR_OUT_OF_MEMORY, NULL););
	CWProtocolStore8(msgPtr, stRadioConfiguration.RadioID);
	CWProtocolStore8(msgPtr, stRadioConfiguration.IsShortPreamble);
	CWProtocolStore8(msgPtr, stRadioConfiguration.BSSIDnums);
	CWProtocolStore8(msgPtr, stRadioConfiguration.DTIMPeriod);

	CWProtocolStoreRawBytes(msgPtr,stRadioConfiguration.BSSID,6);
	
	CWProtocolStore16(msgPtr, stRadioConfiguration.BeaconPeriod);
	//CWProtocolStore32(msgPtr, stRadioConfiguration.CountryCode);
	
	//test so close it
	/////country code area
	char *countrycode;
	countrycode = (char *)WID_MALLOC(sizeof(char)*4);
	if (NULL == countrycode)
	{
          	return CW_FALSE;
	}
	memset(countrycode,0,4);
	
	switch(stRadioConfiguration.CountryCode) /*wcl modify for OSDEVTDPB-31*/
	{
		case COUNTRY_CHINA_CN : 
								strncpy(countrycode,"CN",2);
								break;
								
		case COUNTRY_EUROPE_EU : 
								strncpy(countrycode,"EU",2);
								break;
																
		case COUNTRY_USA_US : 
								strncpy(countrycode,"US",2);
								break;
																
		case COUNTRY_JAPAN_JP : 
								strncpy(countrycode,"JP",2);
								break;
																
		case COUNTRY_FRANCE_FR : 
								strncpy(countrycode,"FR",2);
								break;
																
		case COUNTRY_SPAIN_ES : 
								strncpy(countrycode,"ES",2);
								break;

		default : 
					strncpy(countrycode,"XX",2);
					break;
	}
	strncpy(countrycode+2," ",1);
	//strncpy(countrycode+2,"XX",2);//only use to test
	//printf("countrycode %s\n",countrycode);
	
	CWProtocolStoreRawBytes(msgPtr,countrycode,4);
	WID_FREE(countrycode);
	countrycode = NULL;
	
	return CWAssembleMsgElem(msgPtr, BINDING_MSG_ELEMENT_TYPE_IEEE80211_WTP_RADIO_CONFIGURATON);
}

CWBool CWAssembleWTPRadioAdmin(CWProtocolMessage *msgPtr, CWRadioAdminInfoValues RadioAdmin)
{
	//printf("*****************CWAssembleWTPRadioAdmin*******************\n");
	//printf("Radio ID:%d oprateadmin:%d\n",RadioAdmin.ID,RadioAdmin.state);
	int size = 2;
	CW_CREATE_PROTOCOL_MESSAGE(*msgPtr, size, return CWErrorRaise(CW_ERROR_OUT_OF_MEMORY, NULL););
	CWProtocolStore8(msgPtr, RadioAdmin.ID);// ID of the radio
	CWProtocolStore8(msgPtr, RadioAdmin.state);// state of the radio
	
	return CWAssembleMsgElem(msgPtr, CW_MSG_ELEMENT_RADIO_ADMIN_STATE_CW_TYPE);
}
/* wcl add for RDIR-33 */
CWBool CWAssembleWTPRadioAcktimeoutdistance(CWProtocolMessage *msgPtr, BindingAcktimeout RadioAcktimeoutdistance)
{
	int size = 8;
	//unsigned char reserved = 0;
	CW_CREATE_PROTOCOL_MESSAGE(*msgPtr, size, return CWErrorRaise(CW_ERROR_OUT_OF_MEMORY, NULL););
	CWProtocolStore8(msgPtr, RadioAcktimeoutdistance.Type);
	CWProtocolStore8(msgPtr, RadioAcktimeoutdistance.Op);
	CWProtocolStore8(msgPtr, RadioAcktimeoutdistance.L_RadioID);
	CWProtocolStore8(msgPtr, RadioAcktimeoutdistance.state);
	CWProtocolStore32(msgPtr, RadioAcktimeoutdistance.distance);
	
	wid_syslog_debug_debug(WID_DEFAULT,"@@@@@Type is :%d\n",RadioAcktimeoutdistance.Type);
	wid_syslog_debug_debug(WID_DEFAULT,"@@@@@OP is :%d\n",RadioAcktimeoutdistance.Op);
	wid_syslog_debug_debug(WID_DEFAULT,"@@@@@RadioID is :%d\n",RadioAcktimeoutdistance.L_RadioID);
	wid_syslog_debug_debug(WID_DEFAULT,"@@@@@state is :%d\n",RadioAcktimeoutdistance.state);
	wid_syslog_debug_debug(WID_DEFAULT,"@@@@@distance is :%d\n",RadioAcktimeoutdistance.distance);
	return CWAssembleMsgElem(msgPtr, BINDING_MSG_ELEMENT_TYPE_WTP_RADIO_SET);
}

CWBool CWAssembleWTPRadio11nparameters(CWProtocolMessage *msgPtr, Binding11Nparameter dot11nset)
{
	int size = 4;
	unsigned char reserved = 0;
	CW_CREATE_PROTOCOL_MESSAGE(*msgPtr, size, return CWErrorRaise(CW_ERROR_OUT_OF_MEMORY, NULL););
	CWProtocolStore8(msgPtr, dot11nset.RadioID);// ID of the radio
	CWProtocolStore16(msgPtr, dot11nset.guardinterval);
	CWProtocolStore16(msgPtr, dot11nset.mcs);
	CWProtocolStore16(msgPtr, dot11nset.cwmode);
	CWProtocolStore8(msgPtr, reserved);
	
	return CWAssembleMsgElem(msgPtr, CW_MSG_ELEMENT_WTP_11N_MCS_CWMODE_GUARDINTERVAL);
}
/*fengwenchao add 20120314 for requirements-407*/
CWBool CWAssembleWTPRadio11nMcsList(CWProtocolMessage *msgPtr, Binding11Nparameter dot11nset)
{
	//wid_syslog_info("%s mcs",__func__);
	int i = 0;
	int mcs_val = 0;
	int size = 21;
	unsigned short elementid = 15;
	unsigned short   length = 17;
	unsigned char reserved = 0;
	CW_CREATE_PROTOCOL_MESSAGE(*msgPtr, size, return CWErrorRaise(CW_ERROR_OUT_OF_MEMORY, NULL););
	for(i=0;i<dot11nset.mcs_count;i++)
	{	
		CWSetField32(mcs_val, dot11nset.mcs_list[i],1,1);
		//wid_syslog_info("%d",dot11nset.mcs_list[i]);
	}
	//wid_syslog_info("\n");
	CWProtocolStore16(msgPtr, elementid);
	CWProtocolStore16(msgPtr, length); 
	CWProtocolStore8(msgPtr, dot11nset.RadioID);// ID of the radio
	CWProtocolStore32(msgPtr, mcs_val);
	//wid_syslog_info("mcs_val =%d",mcs_val);
	CWProtocolStore32(msgPtr, reserved);
	CWProtocolStore32(msgPtr, reserved);
	CWProtocolStore32(msgPtr, reserved);
	//wid_syslog_info("over   %s  \n",__func__);
	
		
	return CWAssembleMsgElemVendor(msgPtr, CW_MSG_ELEMENT_VENDOR_SPEC_PAYLOAD_CW_TYPE);
}
/*fengwenchao add end*/
/* zhangshu modify for amsdu, 2010-10-09 */
CWBool CWAssembleWTPRadio11nAmpduParameters(CWProtocolMessage *msgPtr, Binding11nAmpduParameter RadioAmpduSet)
{
	int size = 12;
	CW_CREATE_PROTOCOL_MESSAGE(*msgPtr, size, return CWErrorRaise(CW_ERROR_OUT_OF_MEMORY, NULL););
	CWProtocolStore8(msgPtr, RadioAmpduSet.Type);
	CWProtocolStore8(msgPtr, RadioAmpduSet.Op);
	CWProtocolStore8(msgPtr, RadioAmpduSet.RadioID);
	CWProtocolStore8(msgPtr, RadioAmpduSet.WlanID);
	CWProtocolStore8(msgPtr, RadioAmpduSet.Able);
	CWProtocolStore8(msgPtr, RadioAmpduSet.subframe);  //zhangshu add , 2010-10-09
	CWProtocolStore16(msgPtr, 0);
	CWProtocolStore32(msgPtr, RadioAmpduSet.AmpduLimit);
	
	wid_syslog_debug_debug(WID_DEFAULT,"@@@@@Type is :%d\n",RadioAmpduSet.Type);
	wid_syslog_debug_debug(WID_DEFAULT,"@@@@@Op is :%d\n",RadioAmpduSet.Op);
	wid_syslog_debug_debug(WID_DEFAULT,"@@@@@RadioID is :%d\n",RadioAmpduSet.RadioID);
	wid_syslog_debug_debug(WID_DEFAULT,"@@@@@WlanID is :%d\n",RadioAmpduSet.WlanID);
	wid_syslog_debug_debug(WID_DEFAULT,"@@@@@Able is :%d\n",RadioAmpduSet.Able);
	wid_syslog_debug_debug(WID_DEFAULT,"@@@@@subframe is :%d\n",RadioAmpduSet.subframe);
	wid_syslog_debug_debug(WID_DEFAULT,"@@@@@AmpduLimit is :%d\n",RadioAmpduSet.AmpduLimit);
	
	return CWAssembleMsgElem(msgPtr, BINDING_MSG_ELEMENT_TYPE_WTP_RADIO_SET);
}


CWBool CWAssembleWTPRadio11nMixedGreenfield(CWProtocolMessage *msgPtr, Binding11nMixedGreenfield RadioMixedGreenfield)
{
	int size = 8;
	unsigned char reserved = 0;
	CW_CREATE_PROTOCOL_MESSAGE(*msgPtr, size, return CWErrorRaise(CW_ERROR_OUT_OF_MEMORY, NULL););
	CWProtocolStore8(msgPtr, RadioMixedGreenfield.Type);
	CWProtocolStore8(msgPtr, RadioMixedGreenfield.Op);
	CWProtocolStore8(msgPtr, RadioMixedGreenfield.RadioID);
	CWProtocolStore8(msgPtr, RadioMixedGreenfield.WlanID);
	CWProtocolStore8(msgPtr, RadioMixedGreenfield.Mixed_Greenfield);
	CWProtocolStore8(msgPtr, reserved);
	CWProtocolStore16(msgPtr, reserved);
	//CWProtocolStore32(msgPtr, 0);

	wid_syslog_debug_debug(WID_DEFAULT,"#####Type is :%d\n",RadioMixedGreenfield.Type);
	wid_syslog_debug_debug(WID_DEFAULT,"#####Op is :%d\n",RadioMixedGreenfield.Op);
	wid_syslog_debug_debug(WID_DEFAULT,"#####RadioID is :%d\n",RadioMixedGreenfield.RadioID);
	wid_syslog_debug_debug(WID_DEFAULT,"#####WlanID is :%d\n",RadioMixedGreenfield.WlanID);
	wid_syslog_debug_debug(WID_DEFAULT,"#####Mixed_Greenfield is :%d\n",RadioMixedGreenfield.Mixed_Greenfield);
	
	return CWAssembleMsgElem(msgPtr, BINDING_MSG_ELEMENT_TYPE_WTP_RADIO_SET);
}

CWBool CWAssembleWTPRadio11nChannelExtOffset(CWProtocolMessage *msgPtr,  unsigned int RadioID,unsigned char l_radioid)
{
	int size = 8;
	unsigned char reserved = 0;
	CW_CREATE_PROTOCOL_MESSAGE(*msgPtr, size, return CWErrorRaise(CW_ERROR_OUT_OF_MEMORY, NULL););
	CWProtocolStore8(msgPtr, RADIO);
	CWProtocolStore8(msgPtr, Channel_Extoffset);
	CWProtocolStore8(msgPtr, l_radioid);
	CWProtocolStore8(msgPtr, 0);
	CWProtocolStore8(msgPtr, AC_RADIO[RadioID]->channel_offset);
	CWProtocolStore8(msgPtr, AC_RADIO[RadioID]->Radio_Type);
	CWProtocolStore16(msgPtr, reserved);
	//CWProtocolStore32(msgPtr, 0);

	wid_syslog_debug_debug(WID_DEFAULT,"#####Type is :%d\n",RADIO);
	wid_syslog_debug_debug(WID_DEFAULT,"#####Op is :%d\n",Channel_Extoffset);
	wid_syslog_debug_debug(WID_DEFAULT,"#####channel_offset is :%d\n",AC_RADIO[RadioID]->channel_offset);
	
	return CWAssembleMsgElem(msgPtr, BINDING_MSG_ELEMENT_TYPE_WTP_RADIO_SET);
}

#if 0
CWBool CWAssembleWTPRadio11nTxChainmask(CWProtocolMessage *msgPtr, unsigned int RadioID,unsigned char l_radioid)
{
	int size = 8;
	unsigned char reserved = 0;
	CW_CREATE_PROTOCOL_MESSAGE(*msgPtr, size, return CWErrorRaise(CW_ERROR_OUT_OF_MEMORY, NULL););
	CWProtocolStore8(msgPtr, RADIO);
	CWProtocolStore8(msgPtr, Tx_chainmask);
	CWProtocolStore8(msgPtr, l_radioid);
	CWProtocolStore8(msgPtr, 0);
	CWProtocolStore8(msgPtr, AC_RADIO[RadioID]->tx_chainmask_state_value);
	CWProtocolStore8(msgPtr, reserved);
	CWProtocolStore16(msgPtr, reserved);
	//CWProtocolStore32(msgPtr, 0);

	wid_syslog_debug_debug(WID_DEFAULT,"#####Type is :%d\n",RADIO);
	wid_syslog_debug_debug(WID_DEFAULT,"#####Op is :%d\n",Tx_chainmask);
	//printf("#####RadioID is :%d\n",RadioMixedGreenfield.RadioID);
	//printf("#####WlanID is :%d\n",RadioMixedGreenfield.WlanID);
	wid_syslog_debug_debug(WID_DEFAULT,"#####tx_chainmask_state_value is :%d\n",AC_RADIO[RadioID]->tx_chainmask_state_value);
	
	return CWAssembleMsgElem(msgPtr, BINDING_MSG_ELEMENT_TYPE_WTP_RADIO_SET);
}

#endif

/* zhangshu add for set chainmask, 2010-10-09 */
CWBool CWAssembleWTPRadio11nChainmask(CWProtocolMessage *msgPtr, unsigned int RadioID,unsigned char l_radioid, unsigned char type)
{
    wid_syslog_debug_debug(WID_DEFAULT,"@@@ Send chainmask to AP @@@\n");
	int size = 8;
	unsigned char reserved = 0;
	CW_CREATE_PROTOCOL_MESSAGE(*msgPtr, size, return CWErrorRaise(CW_ERROR_OUT_OF_MEMORY, NULL););
	CWProtocolStore8(msgPtr, RADIO);
	if(type == 1)
	{
	    CWProtocolStore8(msgPtr, Tx_chainmask);
	    CWProtocolStore8(msgPtr, l_radioid);
    	CWProtocolStore8(msgPtr, 0);
    	CWProtocolStore8(msgPtr, AC_RADIO[RadioID]->tx_chainmask_state_value);
	}
	else
	{
	    CWProtocolStore8(msgPtr, Rx_chainmask);   
	    CWProtocolStore8(msgPtr, l_radioid);
    	CWProtocolStore8(msgPtr, 0);
    	CWProtocolStore8(msgPtr, AC_RADIO[RadioID]->rx_chainmask_state_value);
	}
  
	CWProtocolStore8(msgPtr, reserved);
	CWProtocolStore16(msgPtr, reserved);
	//CWProtocolStore32(msgPtr, 0);

	wid_syslog_debug_debug(WID_DEFAULT,"@@@@@Type is :%d\n @@@@@",RADIO);
	wid_syslog_debug_debug(WID_DEFAULT,"@@@@@type is :%d\n @@@@@",type);
	//printf("#####RadioID is :%d\n",RadioMixedGreenfield.RadioID);
	//printf("#####WlanID is :%d\n",RadioMixedGreenfield.WlanID);
	wid_syslog_debug_debug(WID_DEFAULT,"@@@@@tx_chainmask_state_value is :%d @@@@@\n",AC_RADIO[RadioID]->tx_chainmask_state_value);
	wid_syslog_debug_debug(WID_DEFAULT,"@@@@@rx_chainmask_state_value is :%d @@@@@\n",AC_RADIO[RadioID]->rx_chainmask_state_value);
	
	return CWAssembleMsgElem(msgPtr, BINDING_MSG_ELEMENT_TYPE_WTP_RADIO_SET);
}

CWBool CWAssembleWTPQoS (CWProtocolMessage *msgPtr, int radioID, int tagPackets) 
{
	const int headerLength=2;
	const int messageBodyLength=32;
	const int totalMessageLength=headerLength+messageBodyLength;
	int i;
	int* iPtr;
	WTPQosValues* valuesPtr;
	
	if(msgPtr == NULL) return CWErrorRaise(CW_ERROR_WRONG_ARG, NULL);
	
	if((iPtr = ((int*)CWThreadGetSpecific(&gIndexSpecific))) == NULL) {
		return CW_FALSE;
	}
	if(!CWMergeQosValues(*iPtr)) {return CW_FALSE;}
	
	valuesPtr=gWTPs[*iPtr].qosValues;
	
	// create message
	CW_CREATE_PROTOCOL_MESSAGE(*msgPtr, totalMessageLength, return CWErrorRaise(CW_ERROR_OUT_OF_MEMORY, NULL););
	
	CWProtocolStore8(msgPtr, radioID);
	CWProtocolStore8(msgPtr, tagPackets);

	for(i=0; i<NUM_QOS_PROFILES; i++)
	{
		CWProtocolStore8(msgPtr, valuesPtr[i].queueDepth);
		CWProtocolStore16(msgPtr, valuesPtr[i].cwMin);
		CWProtocolStore16(msgPtr, valuesPtr[i].cwMax);
		CWProtocolStore8(msgPtr, valuesPtr[i].AIFS);
		CWProtocolStore8(msgPtr, valuesPtr[i].dot1PTag);
		CWProtocolStore8(msgPtr, valuesPtr[i].DSCPTag);
	}
		
	return CWAssembleMsgElem(msgPtr, BINDING_MSG_ELEMENT_TYPE_WTP_QOS);
}
CWBool CWAssembleWTPQoS2 (CWProtocolMessage *msgPtr, int radioID, int tagPackets,unsigned int qosid) 
{
	
	int i,j;
	int qosID = 0;
	int wmm_object = 0;
	int msglength = 0;
	int elementlength = 0;

	
		//qosID = AC_RADIO[radioID]->QOSID;
		qosID = qosid;
	AC_QOS *valuesPtr;
	if(WID_QOS[qosID] == NULL) return CW_FALSE;
	valuesPtr = WID_QOS[qosID];

	for(i=0;i<4;i++)
	{
		if(valuesPtr->radio_qos[i]->mapstate == 0)
		{
			elementlength = 14;
		}
		else
		{
			elementlength = 16+(valuesPtr->radio_qos[i]->dot1p_map_wmm_num);
		}
		msglength = msglength + elementlength;
		
	}
	
	msglength = msglength + 48;
	
	const int headerLength=3;
	const int messageBodyLength=msglength;
	const int totalMessageLength=headerLength+messageBodyLength;

	//printf("totalMessageLength %d\n",totalMessageLength);
	
	
	if(msgPtr == NULL) return CWErrorRaise(CW_ERROR_WRONG_ARG, NULL);
	
	
	//if(!CWMergeQosValues(qosID)) {return CW_FALSE;}
	
	int l_radio_id = AC_RADIO[radioID]->Radio_L_ID;
	
	// create message
	CW_CREATE_PROTOCOL_MESSAGE(*msgPtr, totalMessageLength, return CWErrorRaise(CW_ERROR_OUT_OF_MEMORY, NULL););
	
	CWProtocolStore8(msgPtr, l_radio_id);
	CWProtocolStore8(msgPtr, tagPackets);
	CWProtocolStore8(msgPtr, AC_RADIO[radioID]->QOSstate);
	//assemble radio qos info
	wmm_object = 0;
	for(i=0;i<4;i++)
	{	
		CWProtocolStore8(msgPtr,wmm_object);
		CWProtocolStore8(msgPtr,i);
		int length1 = 0;
		if(valuesPtr->radio_qos[i]->mapstate == 1)
		{
			length1 = 12+(valuesPtr->radio_qos[i]->dot1p_map_wmm_num);
		}
		else
		{
			length1 = 10;
		}
		CWProtocolStore16(msgPtr,length1);
		CWProtocolStore8(msgPtr,valuesPtr->radio_qos[i]->QueueDepth);
		CWProtocolStore16(msgPtr,valuesPtr->radio_qos[i]->CWMin);
		CWProtocolStore16(msgPtr,valuesPtr->radio_qos[i]->CWMax);
		CWProtocolStore8(msgPtr,valuesPtr->radio_qos[i]->AIFS);
		CWProtocolStore16(msgPtr,valuesPtr->radio_qos[i]->TXOPlimit);
		CWProtocolStore8(msgPtr,valuesPtr->radio_qos[i]->ACK);
		CWProtocolStore8(msgPtr,valuesPtr->radio_qos[i]->mapstate);
		
		if(valuesPtr->radio_qos[i]->mapstate == 1)
		{
			CWProtocolStore8(msgPtr,valuesPtr->radio_qos[i]->wmm_map_dot1p);
			CWProtocolStore8(msgPtr,valuesPtr->radio_qos[i]->dot1p_map_wmm_num);
			if(valuesPtr->radio_qos[i]->dot1p_map_wmm_num != 0)
			{
				for(j=0;j<(valuesPtr->radio_qos[i]->dot1p_map_wmm_num);j++)
				{
					CWProtocolStore8(msgPtr,valuesPtr->radio_qos[i]->dot1p_map_wmm[j]);
				}
			}
		}
	}

	//assemble client qos info
	
	wmm_object = 1;
	for(i=0;i<4;i++)
	{
		CWProtocolStore8(msgPtr,wmm_object);
		CWProtocolStore8(msgPtr,i);
		int length2 = 8;
		CWProtocolStore16(msgPtr,length2);
		CWProtocolStore8(msgPtr,valuesPtr->client_qos[i]->QueueDepth);
		CWProtocolStore16(msgPtr,valuesPtr->client_qos[i]->CWMin);
		CWProtocolStore16(msgPtr,valuesPtr->client_qos[i]->CWMax);
		CWProtocolStore8(msgPtr,valuesPtr->client_qos[i]->AIFS);
		CWProtocolStore16(msgPtr,valuesPtr->client_qos[i]->TXOPlimit);
		
	}
	//wid_syslog_debug_debug("len %d offset %d  len %d\n",strlen(*msgPtr),msgPtr->offset,msgPtr->msgLen);

/*
	if(AC_RADIO[radioID]->QOSstate == 0)
	{
		
		AC_RADIO[radioID]->QOSID = 0;
		
	}
*/
	return CWAssembleMsgElem(msgPtr, BINDING_MSG_ELEMENT_TYPE_WTP_QOS);
}

CWBool CWAssembleWTPRadiowsmstainforeportswitch(CWProtocolMessage *msgPtr, unsigned char reportswitch,unsigned short reportinterval)
{
	wid_syslog_info("accessinto %s \n",__func__);
	int size = 7;
	unsigned short elementid = 31;
	unsigned short   length = 3;
	CW_CREATE_PROTOCOL_MESSAGE(*msgPtr, size, return CWErrorRaise(CW_ERROR_OUT_OF_MEMORY, NULL););
	CWProtocolStore16(msgPtr, elementid);
	CWProtocolStore16(msgPtr, length); 
	CWProtocolStore8(msgPtr, reportswitch);
	CWProtocolStore16(msgPtr, reportinterval);
	wid_syslog_info("%s   %d elementid =  %d\n", __func__, __LINE__, elementid);
	wid_syslog_info("%s   %d length =  %d\n", __func__, __LINE__, length);
	wid_syslog_info("%s   %d reportswitch =  %d\n", __func__, __LINE__, reportswitch);
	wid_syslog_info("%s   %d reportinterval =  %d\n", __func__, __LINE__, reportinterval);
	return CWAssembleMsgElemVendor(msgPtr, CW_MSG_ELEMENT_VENDOR_SPEC_PAYLOAD_CW_TYPE);
}
CWBool CWBindingAssembleConfigureResponse2(CWProtocolMessage **msgElems, int *msgElemCountPtr,int radioid)
{

	if(msgElems == NULL || msgElemCountPtr == NULL) return CWErrorRaise(CW_ERROR_WRONG_ARG, NULL);
	int k = -1;
	int tagPacket = 0;
	CW_CREATE_PROTOCOL_MSG_ARRAY_ERR(*msgElems, *msgElemCountPtr, return CWErrorRaise(CW_ERROR_OUT_OF_MEMORY, NULL););


	int qosid = AC_RADIO[radioid]->QOSID;
	if((AC_RADIO[radioid] != NULL)&&(WID_QOS[qosid] != NULL))
	{

		if (!(CWAssembleWTPQoS2(&(*msgElems[++k]), radioid, tagPacket,qosid)))
		{
			int i;
			for(i = 0; i <= k; i++) {CW_FREE_PROTOCOL_MESSAGE(*msgElems[i]);}
			CW_FREE_OBJECT_WID(*msgElems);
			return CW_FALSE; // error will be handled by the caller
		}
	}


	return CW_TRUE;
}


CWBool CWBindingAssembleConfigureResponse(CWProtocolMessage **msgElems, int *msgElemCountPtr)
{
	CWWTPRadiosInfo radiosInfo;
	int* iPtr;
	const int tagPackets=0;
	int k = -1, radioCount, radioID, j;
	wid_syslog_debug_debug(WID_DEFAULT,"CWBindingAssembleConfigureResponse start");
	if(msgElems == NULL || msgElemCountPtr == NULL) return CWErrorRaise(CW_ERROR_WRONG_ARG, NULL);
	
	if((iPtr = ((int*)CWThreadGetSpecific(&gIndexSpecific))) == NULL) {
		return CW_FALSE;
	}

	//Calculate the number of msg Elements
	*msgElemCountPtr =0;
	radiosInfo=gWTPs[*iPtr].WTPProtocolManager.radiosInfo;
	radioCount=radiosInfo.radioCount;
	*msgElemCountPtr = radioCount;
	wid_syslog_debug_debug(WID_DEFAULT,"Assembling Binding Configuration Response...");

	//Reserve memory for msg Elements
	CW_CREATE_PROTOCOL_MSG_ARRAY_ERR(*msgElems, *msgElemCountPtr, return CWErrorRaise(CW_ERROR_OUT_OF_MEMORY, NULL););

	if(!CWThreadMutexLock(&(gWTPs[*iPtr].interfaceMutex)))
	{
		wid_syslog_crit("Error locking a mutex");
		CWCloseThread();
	}
		//Fill gWTPs[*iPtr].qosValues with default settings 
		gWTPs[*iPtr].qosValues=gDefaultQosValues;

		for (j=0; j<radioCount; j++)
		{
			radioID=radiosInfo.radiosInfo[j].radioID;
			// Assemble WTP QoS Message Element for each radio 
			if (!(CWAssembleWTPQoS(&(*msgElems[++k]), radioID, tagPackets)))
			{
				int i;
				for(i = 0; i <= k; i++) {CW_FREE_PROTOCOL_MESSAGE(*msgElems[i]);}
				CW_FREE_OBJECT_WID(*msgElems);
				CWThreadMutexUnlock(&(gWTPs[*iPtr].interfaceMutex));
				return CW_FALSE; // error will be handled by the caller
			}
		}

		gWTPs[*iPtr].qosValues=NULL;
	CWThreadMutexUnlock(&(gWTPs[*iPtr].interfaceMutex));	
	wid_syslog_debug_debug(WID_DEFAULT,"CWBindingAssembleConfigureResponse end");

	return CW_TRUE;
}

CWBool CWBindingAssembleConfigurationUpdateRequest2(CWProtocolMessage **msgElems, int *msgElemCountPtr, int WTPIndex, struct msgqlist *elem){
	int i=0;//i2=0,i3=0,i4=0, i5=0,ir=0, m=0, n=0, p=0, q=0,r=0;
	//int i6=0,i7=0,u=0,v=0;
	wid_syslog_debug_debug(WID_DEFAULT,"CWBindingAssembleConfigurationUpdateRequest2 start");
	BindingChan chan;
	BindingTXP TXP;
	//BindingRate rate[4];
	unsigned int g_radioid = 0;
	unsigned char radioid;
	struct Support_Rate_List *support_rate;
	
	BindingRadioType radiotype;
	BindingRadioOperate RadioOperate = {0};
	BindingRadioConfiguration RadioConfig = {0};
	CWRadioAdminInfoValues RadioAdmin = {0};
	Binding11Nparameter dot11nset = {0};
	Binding11nAmpduParameter RadioAmpduSet = {0};
	Binding11nMixedGreenfield RadioMixedGreenfield = {0};
	BindingAcktimeout RadioAcktimeout = {0};/*wcl add for RDIR-33*/
	int k = -1;
	if(msgElems == NULL || msgElemCountPtr == NULL) return CWErrorRaise(CW_ERROR_WRONG_ARG, NULL);	
	*msgElemCountPtr = 1;	
	wid_syslog_debug_debug(WID_DEFAULT,"Assembling 22222 Binding Configuration Update Request...");
	if(!check_wtpid_func(WTPIndex)){
		wid_syslog_err("%s\n",__func__);
		return CW_FALSE;
	}else{
	}
	CW_CREATE_PROTOCOL_MSG_ARRAY_ERR(*msgElems, *msgElemCountPtr, return CWErrorRaise(CW_ERROR_OUT_OF_MEMORY, NULL););
//	for(i=0; i<AC_WTP[WTPIndex]->RadioCount; i++){
	i = elem->mqinfo.u.RadioInfo.Radio_L_ID;
	if(!check_l_radioid_func(i)){
		wid_syslog_err("%s\n",__func__);
		CW_FREE_OBJECT_WID(*msgElems);
		return CW_FALSE;
	}else{
	}
	switch(elem->mqinfo.u.RadioInfo.Radio_Op){
		case Radio_TXP:
	//	if((AC_WTP[WTPIndex]->WTP_Radio[i]->CMD & 0x1) != 0x0){
	//		*msgElemCountPtr += 1;
			TXP.radioID = i;
			TXP.TXP = AC_WTP[WTPIndex]->WTP_Radio[i]->Radio_TXP;
			//wid_syslog_debug_debug("TXP:%d\n",TXP[m].TXP);
			//m++;
	//	}	
			wid_syslog_debug_debug(WID_DEFAULT,"txpoweroffset is %d   (before  case Radio_TXP)\n",TXP.TXPOF);
		//	unsigned int flag_txp=0;
			if (!(CWAssembleWTPTXP(&(*msgElems[++k]), TXP.radioID, TXP.TXP)))
			{
				wid_syslog_debug_debug(WID_DEFAULT,"txpoweroffset is %d (in case Radio_TXP)\n",TXP.TXPOF);			
				int i1;
				for(i1 = 0; i1 <= k; i1++) {CW_FREE_PROTOCOL_MESSAGE(*msgElems[i1]);}
				CW_FREE_OBJECT_WID(*msgElems);
				return CW_FALSE; // error will be handled by the caller
			}
			wid_syslog_debug_debug(WID_DEFAULT,"txpoweroffset is %d (after case Radio_TXP)\n",TXP.TXPOF);
			break;
		case Radio_TXPOF:
			TXP.radioID = i;
			//TXP.TXPOF = AC_WTP[WTPIndex]->WTP_Radio[i]->Radio_TXPOF;
			wid_syslog_debug_debug(WID_DEFAULT,"AC_WTP[WTPIndex]->WTP_Radio[i]->Radio_TXPOF  is  %d \n",AC_WTP[WTPIndex]->WTP_Radio[i]->Radio_TXPOF);
			int RadioID = WTPIndex*4+i;
			TXP.TXPOF = AC_RADIO[RadioID]->Radio_TXPOF*AC_RADIO[RadioID]->txpowerstep; //fengwenchao add "*AC_RADIO[RadioID]->txpowerstep"  for axsszfi-832 ,20120425
			
			wid_syslog_debug_debug(WID_DEFAULT,"txpoweroffset is %d (before  case Radio_TXPOF)\n",TXP.TXPOF);
			if (!(CWAssembleWTPTXPOF(&(*msgElems[++k]), TXP.radioID, TXP.TXPOF)))
			{ 
				wid_syslog_debug_debug(WID_DEFAULT,"txpoweroffset is %d (in case Radio_TXPOF)\n",TXP.TXPOF);			
				int i1;
				for(i1 = 0; i1 <= k; i1++) {CW_FREE_PROTOCOL_MESSAGE(*msgElems[i1]);}
				CW_FREE_OBJECT_WID(*msgElems);
				return CW_FALSE; // error will be handled by the caller
			}
			wid_syslog_debug_debug(WID_DEFAULT,"txpoweroffset is %d (after case Radio_TXPOF)\n",TXP.TXPOF);
			break;
		case Radio_Channel:
		//if((AC_WTP[WTPIndex]->WTP_Radio[i]->CMD & 0x2) != 0x0){
	//		*msgElemCountPtr += 1;
			chan.radioID = i;
			chan.channel = AC_WTP[WTPIndex]->WTP_Radio[i]->Radio_Chan;			
			//wid_syslog_debug_debug("CHANNEL:%d\n",chan[n].channel);
		//	n++;
		//}
			if (!(CWAssembleWTPChan(&(*msgElems[++k]), chan.radioID, chan.channel)))
			{
				int i1;
				for(i1 = 0; i1 <= k; i1++) {CW_FREE_PROTOCOL_MESSAGE(*msgElems[i1]);}
				CW_FREE_OBJECT_WID(*msgElems);
				return CW_FALSE; // error will be handled by the caller
			}
			break;
		//if((AC_WTP[WTPIndex]->WTP_Radio[i]->CMD & 0x4) != 0x0)
		//{
			//rate[p].radioID = i;
			//rate[p].rate = AC_WTP[WTPIndex]->WTP_Radio[i]->Radio_Rate;
		case Radio_Rates:
			radioid = i;
			support_rate = AC_WTP[WTPIndex]->WTP_Radio[i]->Radio_Rate;
			
			if (!(CWAssembleWTPRadioRate1(&(*msgElems[++k]), radioid,support_rate)))
			{
				int i1;
				for(i1 = 0; i1 <= k; i1++) {CW_FREE_PROTOCOL_MESSAGE(*msgElems[i1]);}
				CW_FREE_OBJECT_WID(*msgElems);
				return CW_FALSE; // error will be handled by the caller
			}
			break;
			//wid_syslog_debug_debug("Rate:%d\n",rate[p].rate);
		//	p++;
		//}
		//if((AC_WTP[WTPIndex]->WTP_Radio[i]->CMD & 0x8) != 0x0)
	//	{
		case Radio_Mode:
			radiotype.radioID = i;
			radiotype.radiotype = AC_WTP[WTPIndex]->WTP_Radio[i]->Radio_Type;
			
			if (!(CWAssembleWTPRadioType(&(*msgElems[++k]), radiotype.radioID, radiotype.radiotype)))
			{
				int i1;
				for(i1 = 0; i1 <= k; i1++) {CW_FREE_PROTOCOL_MESSAGE(*msgElems[i1]);}
				CW_FREE_OBJECT_WID(*msgElems);
				return CW_FALSE; // error will be handled by the caller
			}
			//wid_syslog_debug_debug("Radio type:%d\n",radiotype[q].radiotype);
		//	q++;
	//	}
			break;
		//if((AC_WTP[WTPIndex]->WTP_Radio[i]->CMD & 0x10) != 0x0)
		//{
		case Radio_BeaconPeriod:
		case Radio_DTIMPeriod:
		case Radio_Preamble:
		case Radio_Countrycode:  /*wcl add for OSDEVTDPB-31*/
			RadioConfig.RadioID = i;
			RadioConfig.IsShortPreamble = AC_WTP[WTPIndex]->WTP_Radio[i]->IsShortPreamble;
			RadioConfig.BeaconPeriod = AC_WTP[WTPIndex]->WTP_Radio[i]->BeaconPeriod;
			RadioConfig.DTIMPeriod= AC_WTP[WTPIndex]->WTP_Radio[i]->DTIMPeriod;
			RadioConfig.CountryCode= AC_WTP[WTPIndex]->WTP_Radio[i]->Radio_country_code; /*wcl add for OSDEVTDPB-31*/
			if (!(CWAssembleWTPRadioConfiguration(&(*msgElems[++k]), RadioConfig)))
			{
				int i1;
				for(i1 = 0; i1 <= k; i1++) {CW_FREE_PROTOCOL_MESSAGE(*msgElems[i1]);}
				CW_FREE_OBJECT_WID(*msgElems);
				return CW_FALSE; // error will be handled by the caller
			}
			break;
			//wid_syslog_debug_debug("Radio IsShortPreamble:%d BeaconPeriod:%d DTIMPeriod:%d\n",RadioConfig[u].IsShortPreamble,RadioConfig[u].BeaconPeriod,RadioConfig[u].DTIMPeriod);
		//	u++;
		//}
	//	if((AC_WTP[WTPIndex]->WTP_Radio[i]->CMD & 0x20) != 0x0)
	//	{
		case Radio_FragThreshold:
		case Radio_rtsthreshold:
		case Radio_ShortRetry:
		case Radio_LongRetry:
			RadioOperate.RadioID= i;
			RadioOperate.FragThreshold = AC_WTP[WTPIndex]->WTP_Radio[i]->FragThreshold;
			RadioOperate.RTSThreshold = AC_WTP[WTPIndex]->WTP_Radio[i]->rtsthreshold;
			RadioOperate.Shortretry = AC_WTP[WTPIndex]->WTP_Radio[i]->ShortRetry;
			RadioOperate.Longretry= AC_WTP[WTPIndex]->WTP_Radio[i]->LongRetry;
			if (!(CWAssembleWTPMacOperate(&(*msgElems[++k]), RadioOperate)))
			{
				int i1;
				for(i1 = 0; i1 <= k; i1++) {CW_FREE_PROTOCOL_MESSAGE(*msgElems[i1]);}
				CW_FREE_OBJECT_WID(*msgElems);
				return CW_FALSE; // error will be handled by the caller
			}
			break;
			//wid_syslog_debug_debug("Radio FragThreshold:%d RTSThreshold %d\n",RadioOperate[v].FragThreshold,RadioOperate[v].RTSThreshold);
	//		v++;
	//	}
		//if((AC_WTP[WTPIndex]->WTP_Radio[i]->CMD & 0x80) != 0x0)
		//{
		case Radio_STATUS:
			RadioAdmin.ID= i;
			/*fengwenchao add 20110920 for radio disable config save*/
			if(AC_WTP[WTPIndex]->WTP_Radio[i] ->radio_disable_flag == 1)
			{
				AC_WTP[WTPIndex]->WTP_Radio[i]->OpStat = 2;
				AC_WTP[WTPIndex]->WTP_Radio[i]->AdStat = 2;
			}
			/*fengwenchao add end*/
			RadioAdmin.state = AC_WTP[WTPIndex]->WTP_Radio[i]->OpStat;
			RadioAdmin.cause = 1;//normal set
			if (!(CWAssembleWTPRadioAdmin(&(*msgElems[++k]), RadioAdmin)))
			{
				int i1;
				for(i1 = 0; i1 <= k; i1++) {CW_FREE_PROTOCOL_MESSAGE(*msgElems[i1]);}
				CW_FREE_OBJECT_WID(*msgElems);
				return CW_FALSE; // error will be handled by the caller
			}
			break;
		case Radio_11N_GI_MCS_CMMODE:
			dot11nset.RadioID = i;
			dot11nset.guardinterval = AC_WTP[WTPIndex]->WTP_Radio[i]->guardinterval;
			dot11nset.mcs = AC_WTP[WTPIndex]->WTP_Radio[i]->mcs;
			dot11nset.cwmode = AC_WTP[WTPIndex]->WTP_Radio[i]->cwmode;
			if (!(CWAssembleWTPRadio11nparameters(&(*msgElems[++k]), dot11nset)))
			{
				int i1;
				for(i1 = 0; i1 <= k; i1++) {CW_FREE_PROTOCOL_MESSAGE(*msgElems[i1]);}
				CW_FREE_OBJECT_WID(*msgElems);
				return CW_FALSE; // error will be handled by the caller
			}
			break;
		/*fengwenchao add 20120314 for requirements-407*/
		case Radio_11N_MCS_LIST:
			dot11nset.RadioID = i;
			dot11nset.mcs_count = AC_WTP[WTPIndex]->WTP_Radio[i]->mcs_count;
			memset(dot11nset.mcs_list,0,32);
			if( AC_WTP[WTPIndex]->WTP_Radio[i]){
				memcpy(dot11nset.mcs_list,AC_WTP[WTPIndex]->WTP_Radio[i]->mcs_list,dot11nset.mcs_count);
				}
			else
				{
				wid_syslog_err("%s %d pointer is NULL\n",__FUNCTION__,__LINE__);
				}
			if (!(CWAssembleWTPRadio11nMcsList(&(*msgElems[++k]), dot11nset)))
			{
				int i1;
				for(i1 = 0; i1 <= k; i1++) {CW_FREE_PROTOCOL_MESSAGE(*msgElems[i1]);}
				CW_FREE_OBJECT_WID(*msgElems);
				return CW_FALSE; // error will be handled by the caller
			}
			break;			
		/*fengwenchao add end*/
		case Radio_acktimeout_distance:
			RadioAcktimeout.Type = AC_WTP[WTPIndex]->WTP_Radio[i]->ack.Type;
			RadioAcktimeout.Op = AC_WTP[WTPIndex]->WTP_Radio[i]->ack.Op;
			RadioAcktimeout.L_RadioID = AC_WTP[WTPIndex]->WTP_Radio[i]->ack.L_RadioID;
			RadioAcktimeout.state = AC_WTP[WTPIndex]->WTP_Radio[i]->ack.state;
			RadioAcktimeout.distance = AC_WTP[WTPIndex]->WTP_Radio[i]->ack.distance;
			if (!(CWAssembleWTPRadioAcktimeoutdistance(&(*msgElems[++k]), RadioAcktimeout)))
			{
				int i1;
				for(i1 = 0; i1 <= k; i1++) {CW_FREE_PROTOCOL_MESSAGE(*msgElems[i1]);}
				CW_FREE_OBJECT_WID(*msgElems);
				return CW_FALSE; 
			}
		    break;	/*wcl add for RDIR-33*/
		case Radio_ampdu_op:
			RadioAmpduSet.Type 	= AC_WTP[WTPIndex]->WTP_Radio[i]->Ampdu.Type;
			RadioAmpduSet.Op 	= AC_WTP[WTPIndex]->WTP_Radio[i]->Ampdu.Op;
			RadioAmpduSet.RadioID	= i;
			RadioAmpduSet.WlanID	= AC_WTP[WTPIndex]->WTP_Radio[i]->Ampdu.WlanID;
			RadioAmpduSet.Able  	= AC_WTP[WTPIndex]->WTP_Radio[i]->Ampdu.Able;
			RadioAmpduSet.subframe 	= AC_WTP[WTPIndex]->WTP_Radio[i]->Ampdu.subframe;
			RadioAmpduSet.AmpduLimit= AC_WTP[WTPIndex]->WTP_Radio[i]->Ampdu.AmpduLimit;
			if (!(CWAssembleWTPRadio11nAmpduParameters(&(*msgElems[++k]), RadioAmpduSet)))
			{
				int i1;
				for(i1 = 0; i1 <= k; i1++) {CW_FREE_PROTOCOL_MESSAGE(*msgElems[i1]);}
				CW_FREE_OBJECT_WID(*msgElems);
				return CW_FALSE; // error will be handled by the caller
			}
		    break;	
		/* zhangshu add for set amsdu, 2010-10-09 */
 		case Radio_amsdu_op:
			RadioAmpduSet.Type 	= AC_WTP[WTPIndex]->WTP_Radio[i]->Amsdu.Type;
			RadioAmpduSet.Op 	= AC_WTP[WTPIndex]->WTP_Radio[i]->Amsdu.Op;
			RadioAmpduSet.RadioID	= i;
			RadioAmpduSet.WlanID	= AC_WTP[WTPIndex]->WTP_Radio[i]->Amsdu.WlanID;
			RadioAmpduSet.Able  	= AC_WTP[WTPIndex]->WTP_Radio[i]->Amsdu.Able;
			RadioAmpduSet.subframe 	= AC_WTP[WTPIndex]->WTP_Radio[i]->Amsdu.subframe;
			RadioAmpduSet.AmpduLimit= AC_WTP[WTPIndex]->WTP_Radio[i]->Amsdu.AmsduLimit;
			if (!(CWAssembleWTPRadio11nAmpduParameters(&(*msgElems[++k]), RadioAmpduSet)))
			{
				int i1;
				for(i1 = 0; i1 <= k; i1++) {CW_FREE_PROTOCOL_MESSAGE(*msgElems[i1]);}
				CW_FREE_OBJECT_WID(*msgElems);
				return CW_FALSE; // error will be handled by the caller
			}
		break;		
		case Radio_puren_mixed_op:
			RadioMixedGreenfield.Type 		  = AC_WTP[WTPIndex]->WTP_Radio[i]->MixedGreenfield.Type;
			RadioMixedGreenfield.Op 		  = AC_WTP[WTPIndex]->WTP_Radio[i]->MixedGreenfield.Op;
			RadioMixedGreenfield.RadioID		  =	i;
			RadioMixedGreenfield.WlanID           =AC_WTP[WTPIndex]->WTP_Radio[i]->MixedGreenfield.WlanID;;
			RadioMixedGreenfield.Mixed_Greenfield =AC_WTP[WTPIndex]->WTP_Radio[i]->MixedGreenfield.Mixed_Greenfield;
			if (!(CWAssembleWTPRadio11nMixedGreenfield(&(*msgElems[++k]), RadioMixedGreenfield)))
			{
				int i1;
				for(i1 = 0; i1 <= k; i1++) {CW_FREE_PROTOCOL_MESSAGE(*msgElems[i1]);}
				CW_FREE_OBJECT_WID(*msgElems);
				return CW_FALSE; // error will be handled by the caller
			}
		break;
		case Radio_channel_ext_offset:
			
			g_radioid = WTPIndex*L_RADIO_NUM+i;
			if (!(CWAssembleWTPRadio11nChannelExtOffset(&(*msgElems[++k]),g_radioid,(unsigned char)i)))
			{
				int i1;
				for(i1 = 0; i1 <= k; i1++) {CW_FREE_PROTOCOL_MESSAGE(*msgElems[i1]);}
				CW_FREE_OBJECT_WID(*msgElems);
				return CW_FALSE; // error will be handled by the caller
			}
		break;		
		#if 0
		case RAdio_tx_chainmask:
			g_radioid = WTPIndex*L_RADIO_NUM+i;
			if (!(CWAssembleWTPRadio11nTxChainmask(&(*msgElems[++k]),g_radioid,(unsigned char)i)))
			{
				int i1;
				for(i1 = 0; i1 <= k; i1++) {CW_FREE_PROTOCOL_MESSAGE(*msgElems[i1]);}
				CW_FREE_OBJECT_WID(*msgElems);
				return CW_FALSE; // error will be handled by the caller
			}
		break;
		#endif
		case RAdio_tx_chainmask:
			g_radioid = WTPIndex*L_RADIO_NUM+i;
			if (!(CWAssembleWTPRadio11nChainmask(&(*msgElems[++k]),g_radioid,(unsigned char)i,1)))
			{
				int i1;
				for(i1 = 0; i1 <= k; i1++) {CW_FREE_PROTOCOL_MESSAGE(*msgElems[i1]);}
				CW_FREE_OBJECT_WID(*msgElems);
				return CW_FALSE; // error will be handled by the caller
			}
		break;
		/* zhangshu add for set rx_chainmask, 2010-10-09 */
		case RAdio_rx_chainmask:
			g_radioid = WTPIndex*L_RADIO_NUM+i;
			if (!(CWAssembleWTPRadio11nChainmask(&(*msgElems[++k]),g_radioid,(unsigned char)i,2)))
			{
				int i1;
				for(i1 = 0; i1 <= k; i1++) {CW_FREE_PROTOCOL_MESSAGE(*msgElems[i1]);}
				CW_FREE_OBJECT_WID(*msgElems);
				return CW_FALSE; // error will be handled by the caller
			}
		break;
		case Radio_NO_RESP_STA_PRO_REQ:
			if (!(CWAssembleMsgElemAPnoRespToStaProReq(&(*msgElems[++k]),elem->mqinfo.u.RadioInfo))) 
			{													
				int i;
				for(i = 0; i <= k; i++) { CW_FREE_PROTOCOL_MESSAGE(*msgElems[i]);}
				CW_FREE_OBJECT_WID(*msgElems);
				return CW_FALSE; 
			}
			break;
		case Radio_UNI_MUTIBRO_CAST_ISO_SW:
			if (!(CWAssembleMsgElemAPUniMutiBroCastIsolationSWandRateSet(&(*msgElems[++k]),elem->mqinfo.u.RadioInfo))) 
			{													
				int i;
				for(i = 0; i <= k; i++) { CW_FREE_PROTOCOL_MESSAGE(*msgElems[i]);}
				CW_FREE_OBJECT_WID(*msgElems);
				return CW_FALSE; 
			}
			break;
		case Radio_UNI_MUTIBRO_CAST_TATE:
			if (!(CWAssembleMsgElemAPUniMutiBroCastRateSet(&(*msgElems[++k]),elem->mqinfo.u.RadioInfo))) 
			{													
				int i;
				for(i = 0; i <= k; i++) { CW_FREE_PROTOCOL_MESSAGE(*msgElems[i]);}
				CW_FREE_OBJECT_WID(*msgElems);
				return CW_FALSE; 
			}
			break;
		case RADIO_WSM_STA_INFO_REPORT:
			{
				int bssindex = elem->mqinfo.u.RadioInfo.id1;
				unsigned char reportswitch = AC_BSS[bssindex]->wsm_sta_info_reportswitch;
				unsigned short reportinterval = AC_BSS[bssindex]->wsm_sta_info_reportinterval;
				if (!(CWAssembleWTPRadiowsmstainforeportswitch(&(*msgElems[++k]), reportswitch,reportinterval)))
				{
					int i1;
					for(i1 = 0; i1 <= k; i1++) {CW_FREE_PROTOCOL_MESSAGE(*msgElems[i1]);}
					CW_FREE_OBJECT_WID(*msgElems);
					return CW_FALSE; // error will be handled by the caller
				}
			}	
			break;
		case Radio_set_cpe_channel:
			if (!(CWAssembleMsgElemAPSetCPEChannelIntf(&(*msgElems[++k]),elem->mqinfo.u.RadioInfo.op,elem->mqinfo.u.RadioInfo.vlan_id,elem->mqinfo.u.RadioInfo.Radio_L_ID,elem->mqinfo.u.RadioInfo.wlanid))) 
			{													
				int i;
				for(i = 0; i <= k; i++) { CW_FREE_PROTOCOL_MESSAGE(*msgElems[i]);}
				CW_FREE_OBJECT_WID(*msgElems);
				return CW_FALSE; 
			}
			break;
		case Radio_set_MGMT_rate:
			if (!(CWAssembleMsgElemRadiosetMGMTratebasewlan(&(*msgElems[++k]),elem->mqinfo.u.RadioInfo.Radio_L_ID,elem->mqinfo.u.RadioInfo.wlanid,elem->mqinfo.u.RadioInfo.rate))) 
			{													
				int i;
				for(i = 0; i <= k; i++) { CW_FREE_PROTOCOL_MESSAGE(*msgElems[i]);}
				CW_FREE_OBJECT_WID(*msgElems);
				return CW_FALSE; 
			}
			break;
		default:
			return CW_TRUE;
	}
			//wid_syslog_debug_debug("Radio operate:%d\n",RadioAdmin.state);
		//	r++;
		//}

//	}
	
//	*msgElemCountPtr = m+n+p+q+u+v+r;
	//wid_syslog_debug_debug("*msgElemCountPtr %d,m %d,n %d,p %d,q %d u%d v %d r %d\n",*msgElemCountPtr,m,n,p,q,u,v,r);
	
	wid_syslog_debug_debug(WID_DEFAULT,"CWBindingAssembleConfigurationUpdateRequest end");

	return CW_TRUE;


}



CWBool CWBindingAssembleConfigurationUpdateRequest(CWProtocolMessage **msgElems, int *msgElemCountPtr){
	CWWTPRadiosInfo radiosInfo;
	int* iPtr;
	const int tagPackets=0;
	int k = -1, radioCount, radioID, j;
	wid_syslog_debug_debug(WID_DEFAULT,"CWBindingAssembleConfigurationUpdateRequest start");
	
	if(msgElems == NULL || msgElemCountPtr == NULL) return CWErrorRaise(CW_ERROR_WRONG_ARG, NULL);
	
	if((iPtr = ((int*)CWThreadGetSpecific(&gIndexSpecific))) == NULL) {
		return CW_FALSE;
	}

	*msgElemCountPtr =0;

	radiosInfo=gWTPs[*iPtr].WTPProtocolManager.radiosInfo;
	radioCount=radiosInfo.radioCount;
	*msgElemCountPtr = radioCount;
	wid_syslog_debug_debug(WID_DEFAULT,"Assembling Binding Configuration Update Request...");
	CW_CREATE_PROTOCOL_MSG_ARRAY_ERR(*msgElems, *msgElemCountPtr, return CWErrorRaise(CW_ERROR_OUT_OF_MEMORY, NULL););
	
	for (j=0; j<radioCount; j++)
	{
		radioID=radiosInfo.radiosInfo[j].radioID;
		
		// Assemble Message Elements
		if (!(CWAssembleWTPQoS(&(*msgElems[++k]), radioID, tagPackets)))
		{
			int i;
			for(i = 0; i <= k; i++) {CW_FREE_PROTOCOL_MESSAGE(*msgElems[i]);}
			CW_FREE_OBJECT_WID(*msgElems);
			return CW_FALSE; // error will be handled by the caller
		}
	}
	wid_syslog_debug_debug(WID_DEFAULT,"CWBindingAssembleConfigurationRequest end");

	return CW_TRUE;
}

CWBool CWBindingSaveConfigurationUpdateResponse(CWProtocolResultCode resultCode, int WTPIndex)
{
	int i;

	bindingValues* aux=(bindingValues*)gWTPs[WTPIndex].WTPProtocolManager.bindingValuesPtr;

	if (resultCode==CW_PROTOCOL_SUCCESS) 
	{
		if (gWTPs[WTPIndex].qosValues!=NULL)
		{
			for(i=0; i<NUM_QOS_PROFILES; i++)
			{
				aux->qosValues[i].cwMin=gWTPs[WTPIndex].qosValues[i].cwMin;
				aux->qosValues[i].cwMax=gWTPs[WTPIndex].qosValues[i].cwMax;
				aux->qosValues[i].AIFS=gWTPs[WTPIndex].qosValues[i].AIFS;
			}
		}
	}

	return CW_TRUE;
}

CWBool  CWAssembleneighbordead_interval(CWProtocolMessage *msgPtr)
{
	wid_syslog_debug_debug(WID_DEFAULT,"#### CWAssembleneighbordead_interval ####\n");
	
	unsigned char valuelen = 4; 
	int value = gCWNeighborDeadInterval;

	
	if(msgPtr == NULL) return CWErrorRaise(CW_ERROR_WRONG_ARG, NULL);

	// create message
	CW_CREATE_PROTOCOL_MESSAGE(*msgPtr, valuelen, return CWErrorRaise(CW_ERROR_OUT_OF_MEMORY, NULL););
	
	CWProtocolStore32(msgPtr, value);


	return CWAssembleMsgElem(msgPtr,  BINDING_MSG_ELEMENT_TYPE_NEIGHBORDEAD_INTERVAL);

}

