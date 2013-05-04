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
* dot11WtpSignalTable.h
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

#ifndef DOT11WTPSIGNALTABLE_H
#define DOT11WTPSIGNALTABLE_H

/* function declarations */
void init_dot11WtpSignalTable(void);
void initialize_table_dot11WtpSignalTable(void);
Netsnmp_Node_Handler dot11WtpSignalTable_handler;
Netsnmp_First_Data_Point  dot11WtpSignalTable_get_first_data_point;
Netsnmp_Next_Data_Point   dot11WtpSignalTable_get_next_data_point;

/* column number definitions for table dot11WtpSignalTable */
       #define COLUMN_WTPALLSIGNALPACK		1
       #define COLUMN_WTPRXSIGNALPACK		2
       #define COLUMN_WTPTXSIGNALPACK		3
	   #define COLUMN_WTPDATATYPE			4
	   #define WTPSIGNALMIN COLUMN_WTPALLSIGNALPACK
	   #define WTPSIGNALMAX COLUMN_WTPDATATYPE
#endif /* DOT11WTPSIGNALTABLE_H */
