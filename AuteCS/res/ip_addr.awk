BEGIN{
   FS="\n[ ]+"
   RS="(^|[^ a-z:])[0-9]+:[ ]"
   ORS="\n"
   OFS=" "
}
{
   if($1 !~ "lo:*")
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
          print substr($1,0,POS1-3)" "substr($3,0,POS2-2) 
      }
   }
}
END{
}
