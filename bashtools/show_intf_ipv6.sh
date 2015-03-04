#!/bin/sh 
###########################################################################
#
#Copyright (C) Autelan Technology
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
#
#
# CREATOR:
# autelan.software.team
#
# DESCRIPTION:
#
#############################################################################
DIR="/proc/net/dev_snmp6/"
DIR_RUN="/var/run/"
#ls $DIR > filename.txt
if [ -s temp.txt ]
then
	rm -rf temp.txt
fi
find ${DIR} -name "*" -print >> temp.txt
sed -i '1d' temp.txt

if [ -s data1.txt ]
then
	rm -rf data1.txt
fi
i=0
while read eth
do
	sed -n '/Ip6*/p' ${eth} >> data1.txt
	let i+=1
	FILE_NUM=`expr $i + 0`
done < temp.txt
sed -i 's/\/proc\/net\/dev_snmp6\///g'  temp.txt
cut -f2 data1.txt |tr -s "\n" > data2.txt
var=`cat data2.txt |wc -l`
echo "all elements num: $var"
ELEMENT_NUM=`expr ${var} / $FILE_NUM`
echo "all files num: $ELEMENT_NUM"

awk -vnvar="$ELEMENT_NUM" '{if(0==NR%nvar)printf("%s\n",$0);else printf("%s\t",$0)}'  data2.txt > data3.txt

awk '{printf "%s:\t",$1;getline < "data3.txt"; printf "%s\n",$0}' temp.txt  > data.txt 
#paste -d ":" filename.txt data3.txt >data.txt
mv data.txt ${DIR_RUN}
rm -rf data1.txt data2.txt data3.txt temp.txt 
	

