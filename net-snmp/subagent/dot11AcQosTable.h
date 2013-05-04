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
* dot11AcQosTable.h
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

#ifndef DOT11ACQOSTABLE_H
#define DOT11ACQOSTABLE_H

/* function declarations */
void init_dot11AcQosTable(void);
void initialize_table_dot11AcQosTable(void);
Netsnmp_Node_Handler dot11AcQosTable_handler;
Netsnmp_First_Data_Point  dot11AcQosTable_get_first_data_point;
Netsnmp_Next_Data_Point   dot11AcQosTable_get_next_data_point;

/* column number definitions for table dot11AcQosTable */
       #define COLUMN_DOT11QOSTRAFFICCLASS		1
       #define COLUMN_DOT11QOSAIFS		2
       #define COLUMN_DOT11QOSCWMIN		3
       #define COLUMN_DOT11QOSCWMAX		4
       #define COLUMN_DOT11QOSTXOPLIM		5
       #define COLUMN_DOT11AVGRATE		6
       #define COLUMN_DOT11MAXDEGREE		7
       #define COLUMN_DOT11POLICYPRI		8
       #define COLUMN_DOT11RESSHOVEPRI		9
       #define COLUMN_DOT11RESGRABPRI		10
       #define COLUMN_DOT11MAXPALLEL		11
       #define COLUMN_DOT11BANDWIDTH		12
       #define COLUMN_DOT11BANDWIDTHSCALE		13
       #define COLUMN_DOT11USEFLOWEQTQUEUE		14
       #define COLUMN_DOT11SINGLEFLOWMAXQUEUE		15
       #define COLUMN_DOT11FLOWAVGRATE		16
       #define COLUMN_DOT11FLOWMAXDEGREE		17
       #define COLUMN_DOT11USEWREDPOLICY		18
       #define COLUMN_DOT11USETRAFFICSHAPING		19
	   #define COLUMN_QOSSVCPKTLOSSRATIO        20
	   #define COLUMN_PKTLOSSRATIO              21
	   #define COLUMN_SVCLOSS                   22
	   #define COLUMN_QUEAVGLEN                 23
	   #define COLUMN_PUTTHROUGHRATIO           24
	   #define COLUMN_DROPRATIO                 25
	   #define COLUMN_VOICEEXCEEDMAXUSERSREQUEST 26
	   #define COLUMN_VIDEOEXCEEDMAXUSERSREQUEST 27
	   #define DOT11ACQOSMIN COLUMN_DOT11QOSTRAFFICCLASS
	   #define DOT11ACQOSMAX COLUMN_VIDEOEXCEEDMAXUSERSREQUEST
#endif /* DOT11ACQOSTABLE_H */
