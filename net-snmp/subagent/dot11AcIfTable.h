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
* dot11AcIfTable.h
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

#ifndef DOT11ACIFTABLE_H
#define DOT11ACIFTABLE_H

/* function declarations */
void init_dot11AcIfTable(void);
void initialize_table_dot11AcIfTable(void);
Netsnmp_Node_Handler dot11AcIfTable_handler;
Netsnmp_First_Data_Point  dot11AcIfTable_get_first_data_point;
Netsnmp_Next_Data_Point   dot11AcIfTable_get_next_data_point;

/* column number definitions for table dot11AcIfTable */
       #define COLUMN_IFINDEX				1
       #define COLUMN_IFDESCR				2
       #define COLUMN_IFTYPE				3
       #define COLUMN_IFMTU				4
       #define COLUMN_IFSPEED				5
       #define COLUMN_IFPHYSADDRESS		6
       #define COLUMN_IFADMINSTATUS		7
       #define COLUMN_IFOPERSTATUS		8
       #define COLUMN_IFLASTCHANGE		9
	#define COLUMN_ACIFMIN				COLUMN_IFINDEX	
	#define COLUMN_ACIFMAX				COLUMN_IFLASTCHANGE
#endif /* DOT11ACIFTABLE_H */
