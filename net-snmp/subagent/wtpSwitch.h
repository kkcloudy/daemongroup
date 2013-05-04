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
* wtpSwitch.h
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
#ifndef WTPSWITCH_H
#define WTPSWITCH_H

/* function declarations */
void init_wtpSwitch(void);
Netsnmp_Node_Handler handle_wtpRogueSwtich;
Netsnmp_Node_Handler handle_wtpStatisticsSwitch;
Netsnmp_Node_Handler handle_wtpFloodAttackDetectSwitch;
Netsnmp_Node_Handler handle_wtpSpoofAttackDetectSwitch;
Netsnmp_Node_Handler handle_wtpWeakIVAttackDetectSwitch;
Netsnmp_Node_Handler handle_wtpCleanAttackHistorySwitch;
Netsnmp_Node_Handler handle_wtpCleanAttackStatisticalSwitch;
#endif /* WTPSWITCH_H */
