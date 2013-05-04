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
* dot11AcStats.h
*
*
* CREATOR:
* autelan.software.Network Dep. team
*
* DESCRIPTION:
* 
*
*
*******************************************************************************/

#ifndef DOT11ACSTATS_H
#define DOT11ACSTATS_H

/* function declarations */
void init_dot11AcStats(void);
Netsnmp_Node_Handler handle_acNumAPInAC;
Netsnmp_Node_Handler handle_acOnlineNumAPInAC;
Netsnmp_Node_Handler handle_acUsersonAC;
Netsnmp_Node_Handler handle_acUplinkDataThroughput;
Netsnmp_Node_Handler handle_acDownlinkDataThroughput;
Netsnmp_Node_Handler handle_acPortThroughput;
Netsnmp_Node_Handler handle_acInACRoamingSuccRate;
Netsnmp_Node_Handler handle_acOutACRoamingSuccRate;
Netsnmp_Node_Handler handle_acAuthReqCnt;
Netsnmp_Node_Handler handle_acAuthSucCnt;
Netsnmp_Node_Handler handle_acAuthFailCnt;
Netsnmp_Node_Handler handle_acAuthRejectCnt;
Netsnmp_Node_Handler handle_acAuthAttemptCnt;
Netsnmp_Node_Handler handle_acStaAllReassoc;
Netsnmp_Node_Handler handle_acStatus;
Netsnmp_Node_Handler handle_acStandbySwitch;
Netsnmp_Node_Handler handle_acBandWidthAvgUtilization;
Netsnmp_Node_Handler handle_acLostPktsAvgUtilization;
Netsnmp_Node_Handler handle_acMaxStaNum;
Netsnmp_Node_Handler handle_acRadiusReqSuccRate;
Netsnmp_Node_Handler handle_acOutACRoamingInSuccRate;
Netsnmp_Node_Handler handle_acOutACRoamingOutSuccRate;
Netsnmp_Node_Handler handle_acPortalLogoffnum;
Netsnmp_Node_Handler handle_acChallengeRecReqnum;
Netsnmp_Node_Handler handle_acChallengeResReqnum;
Netsnmp_Node_Handler handle_acAuthRecReqnum;
Netsnmp_Node_Handler handle_acSendRadiusReqnum;
Netsnmp_Node_Handler handle_acRecRadiusResReqnum;
Netsnmp_Node_Handler handle_acRecRadiusReqSuccnum;
Netsnmp_Node_Handler handle_acAuthResReqnum;
Netsnmp_Node_Handler handle_acSendRadiusOfflineReqnum;
Netsnmp_Node_Handler handle_acRecRadiusOfflineResnum;
Netsnmp_Node_Handler handle_acAcctReqnum;
Netsnmp_Node_Handler handle_acAcctReqSuccnum;
Netsnmp_Node_Handler handle_acLogoffRate;
Netsnmp_Node_Handler handle_acAssociatedTotalUserNum;
Netsnmp_Node_Handler handle_acAuthTimeoutNum;
Netsnmp_Node_Handler handle_acChallengeTimeoutNum;
Netsnmp_Node_Handler handle_acChallengeRejectNum;
Netsnmp_Node_Handler handle_acChallengeBusyNum;
Netsnmp_Node_Handler handle_acAuthPasswordMissingNum;
Netsnmp_Node_Handler handle_acAuthUnknownTypeNum;
Netsnmp_Node_Handler handle_acAuthBusyNum;
Netsnmp_Node_Handler handle_acAuthDisorderNum;
Netsnmp_Node_Handler handle_acPortalErrcode0Num;
Netsnmp_Node_Handler handle_acPortalErrcode1Num;
Netsnmp_Node_Handler handle_acPortalErrcode2Num;
Netsnmp_Node_Handler handle_acPortalErrcode3Num;
Netsnmp_Node_Handler handle_acPortalErrcode4Num;
Netsnmp_Node_Handler handle_acMacAuthCuruserNum;
Netsnmp_Node_Handler handle_acMacAuthErrLogoffNum;
Netsnmp_Node_Handler handle_acMacAuthReqNum;
Netsnmp_Node_Handler handle_acMacAuthReqSuccNum;
Netsnmp_Node_Handler handle_acMacAuthReqFailNum;
Netsnmp_Node_Handler handle_acAuthFreeCuruserNum;
Netsnmp_Node_Handler handle_acAuthFreeErrLogoffNum;

Netsnmp_Node_Handler handle_acAssocAuthCuruserNum;
Netsnmp_Node_Handler handle_acAssocAuthErrLogoffNum;
Netsnmp_Node_Handler handle_acAssocAuthReqNum;
Netsnmp_Node_Handler handle_acAssocAuthReqSuccNum;
Netsnmp_Node_Handler handle_acAssocAuthReqFailNum;
Netsnmp_Node_Handler handle_assocAuthOnlineUserNum;
Netsnmp_Node_Handler handle_assocAuthUserLostConnectionCnt;
Netsnmp_Node_Handler handle_assocAuthReqCnt;
Netsnmp_Node_Handler handle_assocAuthSucCnt;
Netsnmp_Node_Handler handle_assocAuthReqFailCnt;
Netsnmp_Node_Handler handle_autoAuthOnlineUserNum;
Netsnmp_Node_Handler handle_autoAuthUserLostConnectionCnt;

#endif /* DOT11ACSTATS_H */
