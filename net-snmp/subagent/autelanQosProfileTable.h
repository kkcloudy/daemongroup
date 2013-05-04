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
*autelanQosProfileTable.c
*
*
* CREATOR:
* autelan.software.Network Dep. team
*
* DESCRIPTION:
* complete wireless qos infomation table
*
*
*******************************************************************************/

#ifndef AUTELANQOSPROFILETABLE_H
#define AUTELANQOSPROFILETABLE_H

/* function declarations */
void init_autelanQosProfileTable(void);
void initialize_table_autelanQosProfileTable(void);
Netsnmp_Node_Handler autelanQosProfileTable_handler;
Netsnmp_First_Data_Point  autelanQosProfileTable_get_first_data_point;
Netsnmp_Next_Data_Point   autelanQosProfileTable_get_next_data_point;

/* column number definitions for table autelanQosProfileTable */
       #define COLUMN_QOSPROFILEINDEX           1
       #define COLUMN_QOSPROFILENAME            2
       #define COLUMN_QOSRADIOBESTEFFORTDEPTH           3
       #define COLUMN_QOSRADIOBACKGROUNDDEPTH           4
       #define COLUMN_QOSCLIENTBESTEFFORTDEPTH          5
       #define COLUMN_QOSCLIENTBACKGROUNDDEPTH          6
       #define COLUMN_QOSWMMBESTEFFORTMAPPRIORITY               7
       #define COLUMN_QOSWMMBACKGROUNDMAPPRIORITY               8

       #define QOSPROFILETABLE_MIN_COLUMN		COLUMN_QOSPROFILEINDEX
       #define QOSPROFILETABLE_MAX_COLUMN		COLUMN_QOSWMMBACKGROUNDMAPPRIORITY
#endif /* AUTELANQOSPROFILETABLE_H */
