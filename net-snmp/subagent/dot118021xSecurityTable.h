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
* dot118021xSecurityTable.c
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

#ifndef DOT118021XSECURITYTABLE_H
#define DOT118021XSECURITYTABLE_H

/* function declarations */
void init_dot118021xSecurityTable(void);
void initialize_table_dot118021xSecurityTable(void);
Netsnmp_Node_Handler dot118021xSecurityTable_handler;
Netsnmp_First_Data_Point  dot118021xSecurityTable_get_first_data_point;
Netsnmp_Next_Data_Point   dot118021xSecurityTable_get_next_data_point;

/* column number definitions for table dot118021xSecurityTable */
       #define COLUMN_SECURITY8021XINDEX		1
       #define COLUMN_SECURITY8021XENCRYTYPE		2
       #define COLUMN_SECURITY8021XAUTHIP		3
       #define COLUMN_SECURITY8021XAUTHPORT		4
       #define COLUMN_SECURITY8021XAUTHKEY		5
       #define COLUMN_SECURITY8021XRADIUSIP		6
       #define COLUMN_SECURITY8021XRADIUSPORT		7
       #define COLUMN_SECURITY8021XRADIUSKEY		8
       #define COLUMN_SECURITY8021XHOSTIP		9
       #define COLUMN_SECURITY8021XRADIUSSERVER		10
       #define COLUMN_SECURITY8021XRADIUSINFOUPDATETIME		11
       #define COLUMN_SECURITY8021XREAUTHTIME		12
       #define COLUMN_SECURITY8021XID		13
	   #define SECURITY802MIN COLUMN_SECURITY8021XINDEX
	   #define SECURITY802MAX COLUMN_SECURITY8021XID
#endif /* DOT118021XSECURITYTABLE_H */
