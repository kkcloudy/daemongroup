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
* dot11WPAESecurityTable.h
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

#ifndef DOT11WPAESECURITYTABLE_H
#define DOT11WPAESECURITYTABLE_H

/* function declarations */
void init_dot11WPAESecurityTable(void);
void initialize_table_dot11WPAESecurityTable(void);
Netsnmp_Node_Handler dot11WPAESecurityTable_handler;
Netsnmp_First_Data_Point  dot11WPAESecurityTable_get_first_data_point;
Netsnmp_Next_Data_Point   dot11WPAESecurityTable_get_next_data_point;

/* column number definitions for table dot11WPAESecurityTable */
       #define COLUMN_SECURITYWPAEINDEX		1
       #define COLUMN_SECURITYWPAEENCRYTYPE		2
       #define COLUMN_SECURITYWPAEAUTHIP		3
       #define COLUMN_SECURITYWPAEAUTHPORT		4
       #define COLUMN_SECURITYWPAEAUTHKEY		5
       #define COLUMN_SECURITYWPAERADIUSIP		6
       #define COLUMN_SECURITYWPAERADIUSPORT		7
       #define COLUMN_SECURITYWPAERADIUSKEY		8
       #define COLUMN_SECURITYWPAEHOSTIP		9
       #define COLUMN_SECURITYWPAERADIUSSERVER		10
       #define COLUMN_SECURITYWPAERADIUSINFOUPDATETIME		11
       #define COLUMN_SECURITYWPAEREAUTHTIME		12
       #define COLUMN_SECURITYWPAEID		13
	   #define SECURITYWPAEMIN COLUMN_SECURITYWPAEINDEX
	   #define SECURITYWPAEMAX COLUMN_SECURITYWPAEID
#endif /* DOT11WPAESECURITYTABLE_H */
