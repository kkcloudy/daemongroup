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
* dot11AdministratorTable.h
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

#ifndef DOT11ADMINISTRATORTABLE_H
#define DOT11ADMINISTRATORTABLE_H

/* function declarations */
void init_dot11AdministratorTable(void);
void initialize_table_dot11AdministratorTable(void);
Netsnmp_Node_Handler dot11AdministratorTable_handler;
Netsnmp_First_Data_Point  dot11AdministratorTable_get_first_data_point;
Netsnmp_Next_Data_Point   dot11AdministratorTable_get_next_data_point;

/* column number definitions for table dot11AdministratorTable */
       #define COLUMN_ADMINISTRATORID		1
       #define COLUMN_ADMINISTRATORNAME		2
       #define COLUMN_ADMINISTRATORPASSWORD		3	   
	   #define COLUMN_ADMINISTRATORPRIVILEGE		 4
	   #define COLUMN_ADMINISTRATORROWSTATUS		 5
	   #define ADMIN_MIN COLUMN_ADMINISTRATORID
	   #define ADMIN_MAX COLUMN_ADMINISTRATORROWSTATUS
#endif /* DOT11ADMINISTRATORTABLE_H */
