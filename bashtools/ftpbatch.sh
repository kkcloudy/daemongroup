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
# ftpbatch.sh
#
# CREATOR:
# autelan.software.xxx. team
# 
# DESCRIPTION: 
#     
#############################################################################

# Arguments
# $1: A.B.C.D
# $2: src
# $3: usrname
# $4: passwd 

# Exit Status
# 0 	No problems occurred.  	
# 1 	Download error 
# 2 	Directory exist 
# 3 	Empty Directory

dir_mnt=/mnt/eag_html/$2
dir_opt=/opt/eag/www
dir_eag=/mnt/eag_html

if [[ ! -d $dir_eag ]];then
	sudo mkdir $dir_eag
fi 
   
if [[ ! -d $dir_mnt ]];then   
	sudo mkdir $dir_mnt  
fi

if [[ -d $dir_opt/$2 ]];then
	exit 2
else
	cd $dir_opt
fi

wget -r -T 5 -A htm,html,js,css,jpg,png,bmp --ftp-user=$3 --ftp-password=$4 ftp://$1/$2/ -nH 2>/dev/null

if [[ ! $? -eq 0 ]];then
        sudo rm -rf $dir_opt/$2
        exit 1 
else
	count=$( ls -l $dir_opt/$2 | wc -l )

	if [[ $count -gt 1 ]]; then
		sudo cp -rf $dir_opt/$2/* $dir_mnt
		sudo sor.sh cp eag_html/* 120
	else
	   sudo rm -rf $dir_opt/$2
	   exit 3
	fi
fi

exit 0
