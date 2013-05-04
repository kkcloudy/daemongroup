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
* dot11ConfigSecurityTable.h
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

#ifndef DOT11CONFIGSECURITYTABLE_H
#define DOT11CONFIGSECURITYTABLE_H

/* function declarations */
void init_dot11ConfigSecurityTable(void);
void initialize_table_dot11ConfigSecurityTable(void);
Netsnmp_Node_Handler dot11ConfigSecurityTable_handler;
Netsnmp_First_Data_Point  dot11ConfigSecurityTable_get_first_data_point;
Netsnmp_Next_Data_Point   dot11ConfigSecurityTable_get_next_data_point;

/* column number definitions for table dot11ConfigSecurityTable */
       #define COLUMN_SECURITYTYPE					1
       #define COLUMN_ENCRYPTIONTYPE				2
       #define COLUMN_ENCRYPTIONINPUTTYPEKEY		3
       #define COLUMN_SECURITYKEY					4
       #define COLUMN_EXTENSIBLEAUTH				5
       #define COLUMN_AUTHIP						6
       #define COLUMN_AUTHPORT						7
       #define COLUMN_AUTHSHAREDSECRET				8
       #define COLUMN_ACCTIP						9
       #define COLUMN_ACCTPORT						10
       #define COLUMN_ACCTSHAREDSECRET				11
       #define COLUMN_HOSTIP						12
       #define COLUMN_RADIUSSERVER					13
       #define COLUMN_ACCTINTERIMINTERVAL			14
       #define COLUMN_EAPREAUTHPERIOD				15
       #define COLUMN_WAPIASIP						16
       #define COLUMN_WAPIASTYPE					17
       #define COLUMN_WAPIASCERTIFICATIONPATH		18
	   #define COLUMN_WAPIISINSTALLEDASCER			19
       #define COLUMN_WAPIUNICASTREKEYMETHOD		20
       #define COLUMN_WAPIMULTICASTREKEYMETHOD		21
       #define COLUMN_WAPIUNICASTREKEYTIME			22
       #define COLUMN_WAPIUNICASTREKEYPACKETS		23
       #define COLUMN_WAPIMULTICASTREKEYTIME		24
       #define COLUMN_WAPIMULTICASTREKEYPACKETS		25
	   #define COLUMN_PSKVALUE						26
	   #define COLUMN_PSKPASSPHRASE                 27
	   #define COLUMN_RADIUSAUTHSERVERIPADD         28
	   #define COLUMN_RADIUSAUTHSERVERPORT			29
	   #define COLUMN_RADIUSAUTHSERVERSHAREDKEY     30
	   #define COLUMN_RADIUSACCSERVERIPADD			31
	   #define COLUMN_RADIUSACCSERVERPORT			32
	   #define COLUMN_RADIUSACCSERVERSHAREDKEY		33
	   #define COLUMN_WAPIAECERTIFICATIONPATH		34 
	   #define COLUMN_HYBRIDAUTHSWITCH				41
	   #define COLUMN_SEMIN							COLUMN_SECURITYTYPE	
	   #define COLUMN_SEMAX							COLUMN_HYBRIDAUTHSWITCH
#endif /* DOT11CONFIGSECURITYTABLE_H */
