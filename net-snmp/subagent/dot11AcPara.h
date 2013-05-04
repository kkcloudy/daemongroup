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
* dot11AcPara.h
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

//#ifndef DOT11ACPARA_H
//#define DOT11ACPARA_H

/* function declarations */
void init_dot11AcPara(void);
Netsnmp_Node_Handler handle_acIpAddress;
Netsnmp_Node_Handler handle_acNetMask;
Netsnmp_Node_Handler handle_acNetElementCode;
Netsnmp_Node_Handler handle_acAuthenMothedsupp;
Netsnmp_Node_Handler handle_acMacAddress;
Netsnmp_Node_Handler handle_acConPortalURL;
Netsnmp_Node_Handler handle_acPortalServerPort;
Netsnmp_Node_Handler handle_acMaxPortalOnlineUsers;
Netsnmp_Node_Handler handle_acPortalOnlineUsers;
Netsnmp_Node_Handler handle_acMaxPPPoEOnlineUsers;
Netsnmp_Node_Handler handle_acPPPoEOnlineUsers;
Netsnmp_Node_Handler handle_acCertifServerType;
Netsnmp_Node_Handler handle_acConASServerIP;
Netsnmp_Node_Handler handle_acFirstConDNSServer;
Netsnmp_Node_Handler handle_acSeconConDNSServer;
Netsnmp_Node_Handler handle_acTrapDesIPAddress;
Netsnmp_Node_Handler handle_acSNMPPort;
Netsnmp_Node_Handler handle_SysGWAddr;
Netsnmp_Node_Handler handle_acTrapPort;
//#endif /* DOT11ACPARA_H */
