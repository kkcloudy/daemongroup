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
* dot11ConfigPskTable.h
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

#ifndef DOT11CONFIGPSKTABLE_H
#define DOT11CONFIGPSKTABLE_H

/* function declarations */
void init_dot11ConfigPskTable(void);
void initialize_table_dot11ConfigPskTable(void);
Netsnmp_Node_Handler dot11ConfigPskTable_handler;
Netsnmp_First_Data_Point  dot11ConfigPskTable_get_first_data_point;
Netsnmp_Next_Data_Point   dot11ConfigPskTable_get_next_data_point;

/* column number definitions for table dot11ConfigPskTable */
       #define COLUMN_PSKSSID		1
       #define COLUMN_PSKCIPHERKEYVALUE		2
       #define COLUMN_PSKCIPHERKEYCHARTYPE		3
	   #define CONFIGPSKMIN        COLUMN_PSKSSID
	   #define CONFIGPSKMAX        COLUMN_PSKCIPHERKEYCHARTYPE
#endif /* DOT11CONFIGPSKTABLE_H */
