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
# autelan.software.liutao. team
# 
# DESCRIPTION: 
#    
#############################################################################

BEGIN{
   FS="\n[ ]+"
   RS="(^|\n)[0-9]+:[ ]"
   ORS="\n"
   OFS=" "
}
{
   if($1 !~ "lo:*")
   {
	if($1~/eth.*/ || $1~/vlan.*/ || $1~/ebr.*/ )
	{
     	POS1=index($1,"<")
     	if(NF>=3 && $3~"inet*")
     	{
        
	  sub(/inet /,"",$3)
          POS2=index($3,"brd")
          if(POS2 == 0)
          {
             POS2=index($3,"scope")
          }      
          gsub(/\//," ",$3)
          if($1~/UP/)
		{
                	print substr($1,0,POS1-3)" "substr($3,0,POS2-2) 
             	}
             
	 }
  	 }
	}
}
END{
}
