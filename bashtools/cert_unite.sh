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
# eag_init
#
# CREATOR:
# autelan.software.xxx. team
# 
# DESCRIPTION: 
#     
#############################################################################

source vtysh_start.sh
UNITE_FILE=./user.cer
TMP_FILE=./user.cer.tmp
#UNITE_FILE=user.cer
#TMP_FILE=user.cer.tmp

usage()
{
	echo "Usage: cert_unite srcascert srcapcert [dstcert]"
	exit 0
}

( [ ! $# -eq 3 ] && [ ! $# -eq 2 ] ) && usage 

if [ $# -eq 3 ];then
	UNITE_FILE="$3"
fi

sed 's/BEGIN CERTIFICATE/BEGIN USER CERTIFICATE/g' $2 > $TMP_FILE
sed 's/END CERTIFICATE/END USER CERTIFICATE/g' $TMP_FILE > $UNITE_FILE 
echo "" >> $UNITE_FILE
sed 's/BEGIN CERTIFICATE/BEGIN ASU CERTIFICATE/g' $1 >> $UNITE_FILE
sed 's/END CERTIFICATE/END ASU CERTIFICATE/g' $UNITE_FILE > $TMP_FILE
mv $TMP_FILE $UNITE_FILE




