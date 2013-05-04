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
* dot11OpenSecurityTable.h
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

#ifndef DOT11OPENSECURITYTABLE_H
#define DOT11OPENSECURITYTABLE_H

/* function declarations */
void init_dot11OpenSecurityTable(void);
void initialize_table_dot11OpenSecurityTable(void);
Netsnmp_Node_Handler dot11OpenSecurityTable_handler;
Netsnmp_First_Data_Point  dot11OpenSecurityTable_get_first_data_point;
Netsnmp_Next_Data_Point   dot11OpenSecurityTable_get_next_data_point;

/* column number definitions for table dot11OpenSecurityTable */
       #define COLUMN_SECURITYOPENINDEX		1
       #define COLUMN_SECURITYOPENENCRYTYPE		2
       #define COLUMN_SECURITYOPENKEYINPUTTYPE		3
       #define COLUMN_SECURITYOPENKEY		4
       #define COLUMN_SECURITYOPENEXTENSIBLEAUTH		5
       #define COLUMN_SECURITYOPENAUTHIP		6
       #define COLUMN_SECURITYOPENAUTHPORT		7
       #define COLUMN_SECURITYOPENAUTHKEY		8
       #define COLUMN_SECURITYOPENRADIUSIP		9
       #define COLUMN_SECURITYOPENRADIUSPORT		10
       #define COLUMN_SECURITYOPENRADIUSKEY		11
       #define COLUMN_SECURITYOPENHOSTIP		12
       #define COLUMN_SECURITYOPENRADIUSSERVER		13
       #define COLUMN_SECURITYOPENRADIUSINFOUPDATETIME		14
       #define COLUMN_SECURITYOPENREAUTHTIME		15
       #define COLUMN_SECURITYOPENID		16
	   #define SECURITYOPENMIN COLUMN_SECURITYOPENINDEX
	   #define SECURITYOPENMAX COLUMN_SECURITYOPENID
#endif /* DOT11OPENSECURITYTABLE_H */
