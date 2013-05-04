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
* dot11AcDeviceInfo.h
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

#ifndef DOT11ACDEVICEINFO_H
#define DOT11ACDEVICEINFO_H

/* function declarations */
void init_dot11AcDeviceInfo(void);
Netsnmp_Node_Handler handle_acDeviceName;
Netsnmp_Node_Handler handle_acManufacturers;
Netsnmp_Node_Handler handle_acDeviceSN;
Netsnmp_Node_Handler handle_acDeviceType;
Netsnmp_Node_Handler handle_acDeviceHWVersion;
Netsnmp_Node_Handler handle_acMemoryType;
Netsnmp_Node_Handler handle_acMemoryCapacity;
Netsnmp_Node_Handler handle_acMemRTUsage;
Netsnmp_Node_Handler handle_acMemPeakUsage;
Netsnmp_Node_Handler handle_acMemAvgUsage;
Netsnmp_Node_Handler handle_acMemUsageThreshhd;
Netsnmp_Node_Handler handle_acCPUType;
Netsnmp_Node_Handler handle_acCPURTUsage;
Netsnmp_Node_Handler handle_acCPUPeakUsage;
Netsnmp_Node_Handler handle_acCPUAvgUsage;
Netsnmp_Node_Handler handle_acCPUusageThreshhd;
Netsnmp_Node_Handler handle_acFlashType;
Netsnmp_Node_Handler handle_acFlashCapacity;
Netsnmp_Node_Handler handle_acFlashResidualSpace;
Netsnmp_Node_Handler handle_acWorkTemperature;
Netsnmp_Node_Handler handle_acWtpCPUusageThreshhd;
Netsnmp_Node_Handler handle_acWtpMemusageThreshhd;
Netsnmp_Node_Handler handle_acCPUFrequency;

#endif /* DOT11ACDEVICEINFO_H */
