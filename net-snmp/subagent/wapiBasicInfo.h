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
* wapiBasicInfo.h
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
#ifndef WAPIBASICINFO_H
#define WAPIBASICINFO_H

/* function declarations */
void init_wapiBasicInfo(void);
Netsnmp_Node_Handler handle_wapiConfigVersion;
Netsnmp_Node_Handler handle_wapiOptionImplemented;
Netsnmp_Node_Handler handle_wapiPreauthImplemented;
Netsnmp_Node_Handler handle_wapiPreauthEnabled;
Netsnmp_Node_Handler handle_wapiUnicastKeysSupported;
Netsnmp_Node_Handler handle_wapiMulticastCipher;
Netsnmp_Node_Handler handle_wapiMulticastRekeyStrict;
Netsnmp_Node_Handler handle_wapiMulticastCipherSize;
Netsnmp_Node_Handler handle_wapiBKLifetime;
Netsnmp_Node_Handler handle_wapiBKReauthThreshold;
Netsnmp_Node_Handler handle_wapiSATimeout;
Netsnmp_Node_Handler handle_wapiUnicastCipherSelected;
Netsnmp_Node_Handler handle_wapiMulticastCipherSelected;
Netsnmp_Node_Handler handle_wapiUnicastCipherRequested;
Netsnmp_Node_Handler handle_wapiMulticastCipherRequested;

#endif /* WAPIBASICINFO_H */
