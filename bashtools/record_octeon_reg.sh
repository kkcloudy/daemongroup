#!/bin/sh
#
###########################################################################
#
#              Copyright (C) Autelan Technology
#
#This software file is owned and distributed by Autelan Technology 
#
############################################################################
#THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND 
#ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED 
#WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE 
#DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE LIABLE FOR 
#ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES 
#(INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; 
#LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON 
#ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT 
#(INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS 
#SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
##############################################################################
#
# record_octeon_reg
#
# CREATOR:
# autelan.software.xxx. team
# 
# DESCRIPTION: 
#     
#############################################################################

export PATH=/bin:/sbin:/usr/bin:/usr/sbin:/opt/bin

board_type=`cat /proc/product_info/board_type`
core_num=0

if [ $board_type -eq 0 ] ; then
	core_num=16
	echo "POW_PP_GRP_MSKX" >> $1
	bmutil -r -a 0x8001670000000000 >> $1
	bmutil -r -a 0x8001670000000008 >> $1
	bmutil -r -a 0x8001670000000010 >> $1
	bmutil -r -a 0x8001670000000018 >> $1
	bmutil -r -a 0x8001670000000020 >> $1
	bmutil -r -a 0x8001670000000028 >> $1
	bmutil -r -a 0x8001670000000030 >> $1
	bmutil -r -a 0x8001670000000038 >> $1
	bmutil -r -a 0x8001670000000040 >> $1
	bmutil -r -a 0x8001670000000048 >> $1
	bmutil -r -a 0x8001670000000050 >> $1
	bmutil -r -a 0x8001670000000058 >> $1
	bmutil -r -a 0x8001670000000060 >> $1
	bmutil -r -a 0x8001670000000068 >> $1
	bmutil -r -a 0x8001670000000070 >> $1
	bmutil -r -a 0x8001670000000078 >> $1
	echo "CIU_INTX_EN0" >> $1
	bmutil -r -a 0x8001070000000200 >> $1
	bmutil -r -a 0x8001070000000210 >> $1
	bmutil -r -a 0x8001070000000220 >> $1
	bmutil -r -a 0x8001070000000230 >> $1
	bmutil -r -a 0x8001070000000240 >> $1
	bmutil -r -a 0x8001070000000250 >> $1
	bmutil -r -a 0x8001070000000260 >> $1
	bmutil -r -a 0x8001070000000270 >> $1
	bmutil -r -a 0x8001070000000280 >> $1
	bmutil -r -a 0x8001070000000290 >> $1
	bmutil -r -a 0x80010700000002a0 >> $1
	bmutil -r -a 0x80010700000002b0 >> $1
	bmutil -r -a 0x80010700000002c0 >> $1
	bmutil -r -a 0x80010700000002d0 >> $1
	bmutil -r -a 0x80010700000002e0 >> $1
	bmutil -r -a 0x80010700000002f0 >> $1
	bmutil -r -a 0x8001070000000300 >> $1
	bmutil -r -a 0x8001070000000310 >> $1
	bmutil -r -a 0x8001070000000320 >> $1
	bmutil -r -a 0x8001070000000330 >> $1
	bmutil -r -a 0x8001070000000340 >> $1
	bmutil -r -a 0x8001070000000350 >> $1
	bmutil -r -a 0x8001070000000360 >> $1
	bmutil -r -a 0x8001070000000370 >> $1
	bmutil -r -a 0x8001070000000380 >> $1
	bmutil -r -a 0x8001070000000390 >> $1
	bmutil -r -a 0x80010700000003a0 >> $1
	bmutil -r -a 0x80010700000003b0 >> $1
	bmutil -r -a 0x80010700000003c0 >> $1
	bmutil -r -a 0x80010700000003d0 >> $1
	bmutil -r -a 0x80010700000003e0 >> $1
	bmutil -r -a 0x80010700000003f0 >> $1
	bmutil -r -a 0x8001070000000400 >> $1
	echo "CIU_INTX_SUM0" >> $1
	bmutil -r -a 0x8001070000000000 >> $1
	bmutil -r -a 0x8001070000000008 >> $1
	bmutil -r -a 0x8001070000000010 >> $1
	bmutil -r -a 0x8001070000000018 >> $1
	bmutil -r -a 0x8001070000000020 >> $1
	bmutil -r -a 0x8001070000000028 >> $1
	bmutil -r -a 0x8001070000000030 >> $1
	bmutil -r -a 0x8001070000000038 >> $1
	bmutil -r -a 0x8001070000000040 >> $1
	bmutil -r -a 0x8001070000000048 >> $1
	bmutil -r -a 0x8001070000000050 >> $1
	bmutil -r -a 0x8001070000000058 >> $1
	bmutil -r -a 0x8001070000000060 >> $1
	bmutil -r -a 0x8001070000000068 >> $1
	bmutil -r -a 0x8001070000000070 >> $1
	bmutil -r -a 0x8001070000000078 >> $1
	bmutil -r -a 0x8001070000000080 >> $1
	bmutil -r -a 0x8001070000000088 >> $1
	bmutil -r -a 0x8001070000000090 >> $1
	bmutil -r -a 0x8001070000000098 >> $1
	bmutil -r -a 0x80010700000000a0 >> $1
	bmutil -r -a 0x80010700000000a8 >> $1
	bmutil -r -a 0x80010700000000b0 >> $1
	bmutil -r -a 0x80010700000000b8 >> $1
	bmutil -r -a 0x80010700000000c0 >> $1
	bmutil -r -a 0x80010700000000c8 >> $1
	bmutil -r -a 0x80010700000000d0 >> $1
	bmutil -r -a 0x80010700000000d8 >> $1
	bmutil -r -a 0x80010700000000e0 >> $1
	bmutil -r -a 0x80010700000000e8 >> $1
	bmutil -r -a 0x80010700000000f0 >> $1
	bmutil -r -a 0x80010700000000f8 >> $1
	bmutil -r -a 0x8001070000000100 >> $1
fi

if [ $board_type -eq 2 ] ; then
	core_num=4
	echo "POW_PP_GRP_MSKX" >> $1
	bmutil -r -a 0x8001670000000000 >> $1
	bmutil -r -a 0x8001670000000008 >> $1
	bmutil -r -a 0x8001670000000010 >> $1
	bmutil -r -a 0x8001670000000018 >> $1
	echo "CIU_INTX_EN0" >> $1
	bmutil -r -a 0x8001070000000200 >> $1
	bmutil -r -a 0x8001070000000210 >> $1
	bmutil -r -a 0x8001070000000220 >> $1
	bmutil -r -a 0x8001070000000230 >> $1
	bmutil -r -a 0x8001070000000240 >> $1
	bmutil -r -a 0x8001070000000250 >> $1
	bmutil -r -a 0x8001070000000260 >> $1
	bmutil -r -a 0x8001070000000270 >> $1
	bmutil -r -a 0x8001070000000400 >> $1
	echo "CIU_INTX_SUM0" >> $1
	bmutil -r -a 0x8001070000000000 >> $1
	bmutil -r -a 0x8001070000000008 >> $1
	bmutil -r -a 0x8001070000000010 >> $1
	bmutil -r -a 0x8001070000000018 >> $1
	bmutil -r -a 0x8001070000000020 >> $1
	bmutil -r -a 0x8001070000000028 >> $1
	bmutil -r -a 0x8001070000000030 >> $1
	bmutil -r -a 0x8001070000000038 >> $1
	bmutil -r -a 0x8001070000000100 >> $1
fi

if [ $board_type -eq 3 ] || [ $board_type -eq 4 ] ; then
	core_num=12
	echo "POW_PP_GRP_MSKX" >> $1
	bmutil -r -a 0x8001670000000000 >> $1
	bmutil -r -a 0x8001670000000008 >> $1
	bmutil -r -a 0x8001670000000010 >> $1
	bmutil -r -a 0x8001670000000018 >> $1
	bmutil -r -a 0x8001670000000020 >> $1
	bmutil -r -a 0x8001670000000028 >> $1
	bmutil -r -a 0x8001670000000030 >> $1
	bmutil -r -a 0x8001670000000038 >> $1
	bmutil -r -a 0x8001670000000040 >> $1
	bmutil -r -a 0x8001670000000048 >> $1
	bmutil -r -a 0x8001670000000050 >> $1
	bmutil -r -a 0x8001670000000058 >> $1
	echo "CIU_INTX_EN0" >> $1
	bmutil -r -a 0x8001070000000200 >> $1
	bmutil -r -a 0x8001070000000210 >> $1
	bmutil -r -a 0x8001070000000220 >> $1
	bmutil -r -a 0x8001070000000230 >> $1
	bmutil -r -a 0x8001070000000240 >> $1
	bmutil -r -a 0x8001070000000250 >> $1
	bmutil -r -a 0x8001070000000260 >> $1
	bmutil -r -a 0x8001070000000270 >> $1
	bmutil -r -a 0x8001070000000280 >> $1
	bmutil -r -a 0x8001070000000290 >> $1
	bmutil -r -a 0x80010700000002a0 >> $1
	bmutil -r -a 0x80010700000002b0 >> $1
	bmutil -r -a 0x80010700000002c0 >> $1
	bmutil -r -a 0x80010700000002d0 >> $1
	bmutil -r -a 0x80010700000002e0 >> $1
	bmutil -r -a 0x80010700000002f0 >> $1
	bmutil -r -a 0x8001070000000300 >> $1
	bmutil -r -a 0x8001070000000310 >> $1
	bmutil -r -a 0x8001070000000320 >> $1
	bmutil -r -a 0x8001070000000330 >> $1
	bmutil -r -a 0x8001070000000340 >> $1
	bmutil -r -a 0x8001070000000350 >> $1
	bmutil -r -a 0x8001070000000360 >> $1
	bmutil -r -a 0x8001070000000370 >> $1
	bmutil -r -a 0x8001070000000400 >> $1
	echo "CIU_INTX_SUM0" >> $1
	bmutil -r -a 0x8001070000000000 >> $1
	bmutil -r -a 0x8001070000000008 >> $1
	bmutil -r -a 0x8001070000000010 >> $1
	bmutil -r -a 0x8001070000000018 >> $1
	bmutil -r -a 0x8001070000000020 >> $1
	bmutil -r -a 0x8001070000000028 >> $1
	bmutil -r -a 0x8001070000000030 >> $1
	bmutil -r -a 0x8001070000000038 >> $1
	bmutil -r -a 0x8001070000000040 >> $1
	bmutil -r -a 0x8001070000000048 >> $1
	bmutil -r -a 0x8001070000000050 >> $1
	bmutil -r -a 0x8001070000000058 >> $1
	bmutil -r -a 0x8001070000000060 >> $1
	bmutil -r -a 0x8001070000000068 >> $1
	bmutil -r -a 0x8001070000000070 >> $1
	bmutil -r -a 0x8001070000000078 >> $1
	bmutil -r -a 0x8001070000000080 >> $1
	bmutil -r -a 0x8001070000000088 >> $1
	bmutil -r -a 0x8001070000000090 >> $1
	bmutil -r -a 0x8001070000000098 >> $1
	bmutil -r -a 0x80010700000000a0 >> $1
	bmutil -r -a 0x80010700000000a8 >> $1
	bmutil -r -a 0x80010700000000b0 >> $1
	bmutil -r -a 0x80010700000000b8 >> $1
	bmutil -r -a 0x8001070000000100 >> $1
fi

if [ $core_num -gt 0 ] ; then
	echo "POW_WQ_INT_CNTX" >> $1
	bmutil -r -a 0x8001670000000100 >> $1
	bmutil -r -a 0x8001670000000108 >> $1
	bmutil -r -a 0x8001670000000110 >> $1
	bmutil -r -a 0x8001670000000118 >> $1
	bmutil -r -a 0x8001670000000120 >> $1
	bmutil -r -a 0x8001670000000128 >> $1
	bmutil -r -a 0x8001670000000130 >> $1
	bmutil -r -a 0x8001670000000138 >> $1
	bmutil -r -a 0x8001670000000140 >> $1
	bmutil -r -a 0x8001670000000148 >> $1
	bmutil -r -a 0x8001670000000150 >> $1
	bmutil -r -a 0x8001670000000158 >> $1
	bmutil -r -a 0x8001670000000160 >> $1
	bmutil -r -a 0x8001670000000168 >> $1
	bmutil -r -a 0x8001670000000170 >> $1
	bmutil -r -a 0x8001670000000178 >> $1
	echo "POW_WQ_INT_THRX" >> $1
	bmutil -r -a 0x8001670000000080 >> $1
	bmutil -r -a 0x8001670000000088 >> $1
	bmutil -r -a 0x8001670000000090 >> $1
	bmutil -r -a 0x8001670000000098 >> $1
	bmutil -r -a 0x80016700000000a0 >> $1
	bmutil -r -a 0x80016700000000a8 >> $1
	bmutil -r -a 0x80016700000000b0 >> $1
	bmutil -r -a 0x80016700000000b8 >> $1
	bmutil -r -a 0x80016700000000c0 >> $1
	bmutil -r -a 0x80016700000000c8 >> $1
	bmutil -r -a 0x80016700000000d0 >> $1
	bmutil -r -a 0x80016700000000d8 >> $1
	bmutil -r -a 0x80016700000000e0 >> $1
	bmutil -r -a 0x80016700000000e8 >> $1
	bmutil -r -a 0x80016700000000f0 >> $1
	bmutil -r -a 0x80016700000000f8 >> $1
	echo "POW_WQ_INT" >> $1
	bmutil -r -a 0x8001670000000200 >> $1
	echo "FPA_QUEX_AVAILABLE" >> $1
	bmutil -r -a 0x8001180028000098 >> $1
	bmutil -r -a 0x80011800280000a0 >> $1
	bmutil -r -a 0x80011800280000a8 >> $1
	bmutil -r -a 0x80011800280000b0 >> $1
	bmutil -r -a 0x80011800280000b8 >> $1
	bmutil -r -a 0x80011800280000c0 >> $1
	bmutil -r -a 0x80011800280000c8 >> $1
	bmutil -r -a 0x80011800280000d0 >> $1
fi
