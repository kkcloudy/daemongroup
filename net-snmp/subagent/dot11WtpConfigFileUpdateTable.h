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
* dot11WtpConfigFileUpdateTable.h
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

#ifndef DOT11WTPCONFIGFILEUPDATETABLE_H
#define DOT11WTPCONFIGFILEUPDATETABLE_H

/* function declarations */
void init_dot11WtpConfigFileUpdateTable(void);
void initialize_table_dot11WtpConfigFileUpdateTable(void);
Netsnmp_Node_Handler dot11WtpConfigFileUpdateTable_handler;
Netsnmp_First_Data_Point  dot11WtpConfigFileUpdateTable_get_first_data_point;
Netsnmp_Next_Data_Point   dot11WtpConfigFileUpdateTable_get_next_data_point;

/* column number definitions for table dot11WtpConfigFileUpdateTable */
       //#define COLUMN_WTPVERSIONFILEUPDATE		1
       #define COLUMN_WTPCONFIGFILEUPDATE		1
	   #define COLUMN_FILEMIN		COLUMN_WTPCONFIGFILEUPDATE
	   #define COLUMN_FILEMAX		COLUMN_WTPCONFIGFILEUPDATE
#endif /* DOT11WTPCONFIGFILEUPDATETABLE_H */
