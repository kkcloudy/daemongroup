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
* dot11WPA2ESecurityTable.h
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

#ifndef DOT11WPA2ESECURITYTABLE_H
#define DOT11WPA2ESECURITYTABLE_H

/* function declarations */
void init_dot11WPA2ESecurityTable(void);
void initialize_table_dot11WPA2ESecurityTable(void);
Netsnmp_Node_Handler dot11WPA2ESecurityTable_handler;
Netsnmp_First_Data_Point  dot11WPA2ESecurityTable_get_first_data_point;
Netsnmp_Next_Data_Point   dot11WPA2ESecurityTable_get_next_data_point;

/* column number definitions for table dot11WPA2ESecurityTable */
       #define COLUMN_SECURITYWPA2EINDEX		1
       #define COLUMN_SECURITYWPA2EENCRYTYPE		2
       #define COLUMN_SECURITYWPA2EAUTHIP		3
       #define COLUMN_SECURITYWPA2EAUTHPORT		4
       #define COLUMN_SECURITYWPA2EAUTHKEY		5
       #define COLUMN_SECURITYWPA2ERADIUSIP		6
       #define COLUMN_SECURITYWPA2ERADIUSPORT		7
       #define COLUMN_SECURITYWPA2ERADIUSKEY		8
       #define COLUMN_SECURITYWPA2EHOSTIP		9
       #define COLUMN_SECURITYWPA2ERADIUSSERVER		10
       #define COLUMN_SECURITYWPA2ERADIUSINFOUPDATETIME		11
       #define COLUMN_SECURITYWPA2EREAUTHTIME		12
       #define COLUMN_SECURITYWPA2EID		13
	   #define SECURITYWPA2EMIN COLUMN_SECURITYWPA2EINDEX
	   #define SECURITUWPA2EMAX COLUMN_SECURITYWPA2EID
#endif /* DOT11WPA2ESECURITYTABLE_H */
