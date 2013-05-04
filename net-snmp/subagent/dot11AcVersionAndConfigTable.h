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
* dot11AcVersionAndConfigTable.h
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

#ifndef DOT11ACVERSIONANDCONFIGTABLE_H
#define DOT11ACVERSIONANDCONFIGTABLE_H

/* function declarations */
void init_dot11AcVersionAndConfigTable(void);
void initialize_table_dot11AcVersionAndConfigTable(void);
Netsnmp_Node_Handler dot11AcVersionAndConfigTable_handler;
Netsnmp_First_Data_Point  dot11AcVersionAndConfigTable_get_first_data_point;
Netsnmp_Next_Data_Point   dot11AcVersionAndConfigTable_get_next_data_point;

/* column number definitions for table dot11AcVersionAndConfigTable */
       #define COLUMN_LOADFLAG				1
       #define COLUMN_FILENAME				2
       #define COLUMN_FILETYPE				3
       #define COLUMN_ESTIMATEFILESIZE		4
       #define COLUMN_TRANSPROTOCOL			5
       #define COLUMN_SERVERADDR				6
       #define COLUMN_SERVERPORT				7
       #define COLUMN_SERVERUSERNAME		8
       #define COLUMN_SERVERPASSWD			9
       #define COLUMN_TRANSSTATUS			10
       #define COLUMN_TOTALBYTES				11
       #define COLUMN_TRANSFERBYTES			12
       #define COLUMN_FAILREASON				13
	#define COLUMN_VERSIONMIN				COLUMN_LOADFLAG
	#define COLUMN_VERSIONMAX				COLUMN_FAILREASON
#endif /* DOT11ACVERSIONANDCONFIGTABLE_H */
