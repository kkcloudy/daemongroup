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
* ACBinding.h
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

#ifndef __CAPWAP_ACBinding_HEADER__
#define __CAPWAP_ACBinding_HEADER__

CWBool CWACInitBinding(int i);
CWBool CWBindingAssembleConfigureResponse(CWProtocolMessage **msgElems, int *msgElemCountPtr);
CWBool CWBindingAssembleConfigurationUpdateRequest(CWProtocolMessage **msgElems, int *msgElemCountPtr);
CWBool CWBindingAssembleConfigurationUpdateRequest2(CWProtocolMessage **msgElems, int *msgElemCountPtr, int WTPIndex, struct msgqlist *elem);
CWBool CWBindingSaveConfigurationUpdateResponse(CWProtocolResultCode resultCode, int WTPIndex);

CWBool CWAssembleWTPChan(CWProtocolMessage *msgPtr, unsigned char radioID, unsigned char channel);

CWBool CWAssembleWTPTXP(CWProtocolMessage *msgPtr, unsigned char radioID, unsigned short TXP);
CWBool CWAssembleWTPTXPOF(CWProtocolMessage *msgPtr, unsigned char radioID, unsigned short TXP);

//added by weiay 20080714
//CWBool CWAssembleWTPRadioRate(CWProtocolMessage *msgPtr, unsigned char radioID, unsigned short rate);
CWBool CWAssembleWTPRadioRate1(CWProtocolMessage *msgPtr, unsigned char radioID,struct Support_Rate_List *rate);
CWBool CWAssembleWTPRadioType(CWProtocolMessage *msgPtr, unsigned char radioID, unsigned int mode);
CWBool CWAssembleWTPMacOperate(CWProtocolMessage *msgPtr, BindingRadioOperate stRadioOperate);
CWBool CWAssembleWTPRadioConfiguration(CWProtocolMessage *msgPtr, BindingRadioConfiguration stRadioConfiguration);//20080722
CWBool CWAssembleWTPRadioAdmin(CWProtocolMessage *msgPtr, CWRadioAdminInfoValues RadioAdmin);
CWBool CWAssembleWTPQoS2(CWProtocolMessage * msgPtr,int radioID,int tagPackets,unsigned int qosid);
CWBool  CWAssembleneighbordead_interval(CWProtocolMessage *msgPtr);
CWBool CWAssembleWTPRadio11nparameters(CWProtocolMessage *msgPtr, Binding11Nparameter dot11nset);

#endif
