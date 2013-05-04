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
* dot11SharedSecurityTable.h
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

#ifndef DOT11SHAREDSECURITYTABLE_H
#define DOT11SHAREDSECURITYTABLE_H

/* function declarations */
void init_dot11SharedSecurityTable(void);
void initialize_table_dot11SharedSecurityTable(void);
Netsnmp_Node_Handler dot11SharedSecurityTable_handler;
Netsnmp_First_Data_Point  dot11SharedSecurityTable_get_first_data_point;
Netsnmp_Next_Data_Point   dot11SharedSecurityTable_get_next_data_point;

/* column number definitions for table dot11SharedSecurityTable */
       #define COLUMN_SECURITYSHAREDINDEX		1
       #define COLUMN_SECURITYSHAREDENCRYTYPE		2
       #define COLUMN_SECURITYSHAREDKEYINPUTTYPE		3
       #define COLUMN_SECURITYSHAREDKEY		4
       #define COLUMN_SECURITYSHAREDEXTENSIBLEAUTH		5
       #define COLUMN_SECURITYSHAREDAUTHIP		6
       #define COLUMN_SECURITYSHAREDAUTHPORT		7
       #define COLUMN_SECURITYSHAREDAUTHKEY		8
       #define COLUMN_SECURITYSHAREDRADIUSIP		9
       #define COLUMN_SECURITYSHAREDRADIUSPORT		10
       #define COLUMN_SECURITYSHAREDRADIUSKEY		11
       #define COLUMN_SECURITYSHAREDHOSTIP		12
       #define COLUMN_SECURITYSHAREDRADIUSSERVER		13
       #define COLUMN_SECURITYSHAREDRADIUSINFOUPDATETIME		14
       #define COLUMN_SECURITYSHAREDREAUTHTIME		15
       #define COLUMN_SECURITYSHAREDID		16
	   #define SECURITYSHAREDMIN COLUMN_SECURITYSHAREDINDEX
	   #define SECURITYSHAREDMAX COLUMN_SECURITYSHAREDID
#endif /* DOT11SHAREDSECURITYTABLE_H */
