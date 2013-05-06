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

S_IMG=/mnt/logo/5624_03.jpg
D_IMG=/opt/www/htdocs/images/new5000/5624_03.jpg

S_IMG_7K=/mnt/logo/7000news_06.jpg
D_IMG_7K=/opt/www/htdocs/images/panel/7000news_06.jpg

S_IMG_BG=/mnt/logo/indexbg.jpg
D_IMG_BG=/opt/www/htdocs/images/indexbg.jpg

S_IMG_LG=/mnt/logo/logo.jpg
D_IMG_LG=/opt/www/htdocs/images/logo.jpg

if [ -f $S_IMG ] 
  then
        cp $S_IMG $D_IMG
fi 

if [ -f $S_IMG_7K ] 
  then
        cp $S_IMG_7K $D_IMG_7K
fi 

if [ -f $S_IMG_BG ] 
  then
        cp $S_IMG_BG $D_IMG_BG
fi 


if [ -f $S_IMG_LG ] 
  then
        cp $S_IMG_LG $D_IMG_LG
fi 