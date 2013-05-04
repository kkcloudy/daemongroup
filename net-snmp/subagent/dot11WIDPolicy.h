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
* dot11WIDPolicy.h
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

#ifndef DOT11ATTACK_H
#define DOT11ATTACK_H

/* function declarations */
void init_dot11WIDPolicy(void);
Netsnmp_Node_Handler handle_widsFloodInterval;
Netsnmp_Node_Handler handle_widsBlackListThreshold;
Netsnmp_Node_Handler handle_widsBlackListDuration;
Netsnmp_Node_Handler handle_widsFloodDetectOnOff;
Netsnmp_Node_Handler handle_SSIDFilterSwitch;
Netsnmp_Node_Handler handle_BSSIDFilterSwitch;
Netsnmp_Node_Handler handle_RogueAPCountermeasureSwitch;
Netsnmp_Node_Handler handle_RogueAPCountermeasureMode;

#endif /* DOT11ATTACK_H */

