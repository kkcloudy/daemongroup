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
* dot11WidDetectHistoryTable.h
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

#ifndef DOT11WIDDETECTHISTORYTABLE_H
#define DOT11WIDDETECTHISTORYTABLE_H

/* function declarations */
void init_dot11WidDetectHistoryTable(void);
void initialize_table_dot11WidDetectHistoryTable(void);
Netsnmp_Node_Handler dot11WidDetectHistoryTable_handler;
Netsnmp_First_Data_Point  dot11WidDetectHistoryTable_get_first_data_point;
Netsnmp_Next_Data_Point   dot11WidDetectHistoryTable_get_next_data_point;

/* column number definitions for table dot11WidDetectHistoryTable */
       #define COLUMN_DEVICEID					1
       #define COLUMN_DEVICEMAC					2
       #define COLUMN_ATTACKTYPE				3
       #define COLUMN_FRAMETYPE					4
       #define COLUMN_ATTACKCOUNT				5
       #define COLUMN_FIRSTATTACK				6
       #define COLUMN_LASTATTACK				7
	   #define COLUMN_ROGSTAMONAPNUM			8
	   #define COLUMN_ROGSTAACCBSSID			9
	   #define COLUMN_ROGSTAMAXSIGSTRENGTH		10
	   #define COLUMN_ROGSTACHANNEL				11
	   #define COLUMN_ROGSTAADHOCSTATUS			12
	   #define COLUMN_ROGSTAATTACKSTATUS		13
	   #define COLUMN_ROGSTATOIGNORE			14
	   #define COLUMN_WIDDETECTMIN	COLUMN_DEVICEID
	   #define COLUMN_WIDDETECTMAX	COLUMN_ROGSTATOIGNORE
#endif /* DOT11WIDDETECTHISTORYTABLE_H */
