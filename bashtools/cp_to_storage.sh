 #!/bin/sh

TEMPORARY_DIRECTORY="/home/admin/temp_directory"
MYFILE="/home/admin/temp_directory/conf_xml.conf"
CURRENT_DIRECTORY=`pwd`
VERSION_NUM_IN_CARD=3
PARTION_MOUNT="/tmp_dir/"
BOUNDARY=500

#mount the storage and ready to copy the files back to it
if ! `sudo mount /blk`; then
	echo "mount /blk failed"
	echo "original files at `pwd`/$TEMPORARY_DIRECTORY"
	echo "please operate the left operation manually"
	cd $CURRENT_DIRECTORY	
fi

variable=`du -sh $TEMPORARY_DIRECTORY | awk '{print $1}' | grep -o -i "[0-9]\{1,\}"`
if [ $variable -lt $BOUNDARY ]; then
	 if ! `cp -r $TEMPORARY_DIRECTORY/*  /blk/`; then
	     echo "copy file back to the storage failed"
         echo "original files at `pwd`/$TEMPORARY_DIRECTORY"
         echo "please operate the left operation manually"
         cd $CURRENT_DIRECTORY
         exit 7
     fi
else
     #put the current version to the first partion       
         cur_img=`/opt/bin/vtysh -c "show system boot_img" | awk '{print $6}'`         
         CUR_IMG=$(echo $cur_img | tr '[a-z]' '[A-Z]')           
           items=`ls -l $TEMPORARY_DIRECTORY/ | grep -i img | awk '{print $9}'`
           for item in $items
           do            
           ITEM=$(echo $item | tr '[a-z]' '[A-Z]')             
           if [ "$ITEM" = "$CUR_IMG" ]; then                
              cp $TEMPORARY_DIRECTORY/$item  /blk/
              echo "copy the current version to the storage card"
              flag=1              
              break
           else 
              flag=0
           fi
           done     
         
     #put the other two the latest version in /blk/    
     if [ $flag  -eq  1 ]; then       
          if [ `ls -l $TEMPORARY_DIRECTORY/  | grep -i img | wc -l` -gt  $VERSION_NUM_IN_CARD ]; then
            versions=`ls -lrt $TEMPORARY_DIRECTORY/ | grep -i img | tail -n  2 | awk '{print $9}'`
            for  version in $versions
            do          
            VERSION=$(echo $version | tr '[a-z]' '[A-Z]')           
            if [ "$VERSION" !=  "$CUR_IMG" ]; then                 
                if ! `cp  $TEMPORARY_DIRECTORY/$version  /blk/`; then
                     echo "copy the other two the latest version to the first partion failed"
                     cd $CURRENT_DIRECTORY
                fi
            else
               vers=`ls -lrt $TEMPORARY_DIRECTORY/ | grep -i img | tail -n  3| awk '{print $9}'`
               for ver in $vers
               do
               VER=$(echo $ver | tr '[a-z]' '[A-Z]') 
               if [ "$VER" != "$CUR_IMG" ]; then
                  cp  $TEMPORARY_DIRECTORY/$ver  /blk/                 
               fi    
               done
            fi
            done
            
          else 
            versions=`ls -lrt $TEMPORARY_DIRECTORY/ | grep -i img | awk '{print $9}'`
            for  version in $versions
            do        
            if ! `cp   $TEMPORARY_DIRECTORY/$version  /blk`;  then
                 echo "When version less than 3 copy to the first partion is not successful"
                 cd $CURRENT_DIRECTORY                
            fi 
            done         
          fi   
     else      
           if [ `ls -l $TEMPORARY_DIRECTORY/  | grep -i img | wc -l` -gt  $VERSION_NUM_IN_CARD ]; then
            versions=`ls -lrt $TEMPORARY_DIRECTORY/ | grep -i img | tail -n  3 | awk '{print $9}'`
            for  version in $versions
            do        
             if ! `cp  $TEMPORARY_DIRECTORY/$version  /blk/`; then
                  echo "copy the other two the latest version to the first partion failed"
                  cd $CURRENT_DIRECTORY
             fi             
            done           
          else 
            versions=`ls -lrt $TEMPORARY_DIRECTORY/ | grep -i img | awk '{print $9}'`
            for  version in $versions
            do        
            if ! `cp   $TEMPORARY_DIRECTORY/$version  /blk`;  then
                 echo "When version less than 3 copy to the first partion is not successful"
                 cd $CURRENT_DIRECTORY                
            fi 
            done         
           fi           
     fi
fi 

#Copy configuration files in the first partition
if [ -f $MYFILE ]; then
     if ! `cp  $MYFILE  /blk`; then
       echo "copy conf_xml.conf failed"
       cd $CURRENT_DIRECTORY       
     fi
else
     echo "conf_xml.conf doesn't exit"
fi

if [ -d  $TEMPORARY_DIRECTORY/patch ]; then  
  cp -r  $TEMPORARY_DIRECTORY/patch  /blk
else 
  echo "patch doesn't exit"
fi

if [ -d  $TEMPORARY_DIRECTORY/critlog ]; then 
  cp  -r  $TEMPORARY_DIRECTORY/critlog  /blk
else
  echo "critlog doesn't exit"  
fi

if [ -d  $TEMPORARY_DIRECTORY/nvram_log ]; then 
  cp  -r  $TEMPORARY_DIRECTORY/nvram_log  /blk
else
  echo "nvram_log doesn't exit"  
fi

if [ -d  $TEMPORARY_DIRECTORY/wtp ]; then 
  cp  -r  $TEMPORARY_DIRECTORY/wtp  /blk
else
  echo "wtp doesn't exit"  
fi

if [ -d  $TEMPORARY_DIRECTORY/earlystart ]; then 
  cp  -r  $TEMPORARY_DIRECTORY/earlystart /blk
else
  echo "earlystart doesn't exit"  
fi

if [ -d  $TEMPORARY_DIRECTORY/snapshot ]; then 
  cp  -r  $TEMPORARY_DIRECTORY/snapshot /blk
else
  echo "snapshot doesn't exit"  
fi

if [ -f $TEMPORARY_DIRECTORY/newconf ]; then 
  cp  $TEMPORARY_DIRECTORY/newconf  /blk
else
  echo "newconf doesn't exit "
fi

if [ -f $TEMPORARY_DIRECTORY/bootlog.sem ]; then 
  cp  $TEMPORARY_DIRECTORY/bootlog.sem  /blk
else
  echo "bootlog.sem doesn't exit "
fi

if [ -f $TEMPORARY_DIRECTORY/runosver ]; then 
  cp  $TEMPORARY_DIRECTORY/runosver  /blk
else
  echo "runosver doesn't exit "
fi

if [ -f $TEMPORARY_DIRECTORY/softreboot ]; then 
  cp  $TEMPORARY_DIRECTORY/softreboot  /blk
else
  echo "softreboot doesn't exit "
fi

if [ -f $TEMPORARY_DIRECTORY/forcestring ]; then 
  cp  $TEMPORARY_DIRECTORY/forcestring  /blk
else
  echo "forcestring doesn't exit "
fi

if [ -f $TEMPORARY_DIRECTORY/vstring ]; then 
  cp  $TEMPORARY_DIRECTORY/vstring  /blk
else
  echo "vstring doesn't exit "
fi

if [ -f $TEMPORARY_DIRECTORY/devinfo ]; then 
  cp  $TEMPORARY_DIRECTORY/devinfo  /blk
else
  echo "devinfo doesn't exit "
fi
#Create the second partition of hardpoints
if [ -d $PARTION_MOUNT ]; then
	echo "$PARTION_MOUNT allready exist at /"
    echo "please rename the exist directory"
	cd -
	
else
#echo "$PARTION_MOUNT not exist at /"
#make dir for temporary
	 if ! `mkdir $PARTION_MOUNT`; then
		 echo "create directory $PARTION_MOUNT failed"
		 cd $CURRENT_DIRECTORY	
		 exit 8
     fi
					
	 if ! `sudo mount /dev/sda2  $PARTION_MOUNT`; then
	     echo "mount $PARTION_MOUNT failed"
         cd $CURRENT_DIRECTORY	
	     exit  9
     fi
     #get Get the current version of the system and deposited in the second  partition     
         cur_img=`/opt/bin/vtysh -c "show system boot_img" | awk '{print $6}'`      
         CUR_IMG=$(echo $cur_img | tr '[a-z]' '[A-Z]')                 
            items=`ls -l $TEMPORARY_DIRECTORY/ | grep -i img | awk '{print $9}'`
            for item in $items
            do 
            ITEM=$(echo $item | tr '[a-z]' '[A-Z]')               
            if [ "$ITEM" = "$CUR_IMG" ]; then 
                cp $TEMPORARY_DIRECTORY/$item  $PARTION_MOUNT
                echo "copy the current version to the storage card"  
                flag=1                
                break
             else
                flag=0
            fi
            done        
#put the  other two the latest version in /tmp_dir     
      if [ $flag -eq 1 ]; then
           if [ `ls -l $TEMPORARY_DIRECTORY/ | grep -i img | wc -l` -gt  $VERSION_NUM_IN_CARD ]; then
              versions=`ls -lrt $TEMPORARY_DIRECTORY/ | grep -i img | tail -n  2 | awk '{print $9}'`
              for  version in $versions
              do               
              VERSION=$(echo $version | tr '[a-z]' '[A-Z]')
              if [ "$VERSION" !=  "$CUR_IMG" ]; then                   
                  if ! `cp  $TEMPORARY_DIRECTORY/$version  $PARTION_MOUNT `; then
                       echo "copy the other two the latest version to the second  partion failed"
                       cd $CURRENT_DIRECTORY
                  fi
              else
                  vers=`ls -lrt $TEMPORARY_DIRECTORY/ | grep -i img | tail -n  3| awk '{print $9}'`
                  for ver in $vers
                  do
                  VER=$(echo $ver | tr '[a-z]' '[A-Z]') 
                  if [ "$VER" != "$CUR_IMG" ]; then
                     cp  $TEMPORARY_DIRECTORY/$ver  $PARTION_MOUNT                
                  fi    
                  done           
                  
              fi
              done
            
          else 
              versions=`ls -lrt $TEMPORARY_DIRECTORY/ | grep -i img | awk '{print $9}'`
              for  version in $versions
              do        
              if ! `cp   $TEMPORARY_DIRECTORY/$version  $PARTION_MOUNT `;  then
                  echo "When version less than 3 copy to the second  partion is not successful"
                  cd $CURRENT_DIRECTORY                
              fi 
              done   
          fi
      else
           if [ `ls -l $TEMPORARY_DIRECTORY/  | grep -i img | wc -l` -gt  $VERSION_NUM_IN_CARD ]; then
            versions=`ls -lrt $TEMPORARY_DIRECTORY/ | grep -i img | tail -n  3 | awk '{print $9}'`
            for  version in $versions
            do        
             if ! `cp  $TEMPORARY_DIRECTORY/$version  $PARTION_MOUNT`; then
                  echo "copy the other two the latest version to the second partion failed"
                  cd $CURRENT_DIRECTORY
             fi            
            done            
           else 
              versions=`ls -lrt $TEMPORARY_DIRECTORY/ | grep -i img | awk '{print $9}'`
              for  version in $versions
              do        
              if ! `cp   $TEMPORARY_DIRECTORY/$version $PARTION_MOUNT`;  then
                   echo "When version less than 3 copy to the second  partion is not successful"
                   cd $CURRENT_DIRECTORY                
              fi 
              done         
           fi   
      fi
fi   
#Copy configuration files in the first partition
if [ -f $MYFILE ]; then
     cp  $MYFILE  $PARTION_MOUNT
else
    echo "conf_xml.conf doesn't exit"
fi
#Unloading hardpoints
if ! `sudo umount -l /blk`; then
	echo "Unloading the first partition failure,Please manually delete"
	echo "original files at `pwd`/$TEMPORARY_DIRECTORY"
	echo "Please hand unloading"
	cd $CURRENT_DIRECTORY		
fi	

if ! `sudo umount -l $PARTION_MOUNT`; then
	echo "Unloading the secongd partition failure,Please manually delete"
	echo "original files at `pwd`/$TEMPORARY_DIRECTORY"
    echo "Please hand unloading"
    cd $CURRENT_DIRECTORY	 
fi

if ! `rm -rf $PARTION_MOUNT`; then
    echo "Delete the second partition of hardpoints failure,Please manually delete"
    cd $CURRENT_DIRECTORY         
fi    

if ! `rm -rf $TEMPORARY_DIRECTORY`; then
    echo "Delete temporary folder failure,Please manually delete"
    echo "Please manually delete"
    cd $CURRENT_DIRECTORY			
else 
    echo "sort succeed"
    cd $CURRENT_DIRECTORY	
fi

#recover sad.sh
if ! `rm -rf /var/run/sad/*`; then
    echo "rm -rf /var/run/sad/* failed,Please manually delete"
    cd $CURRENT_DIRECTORY  
else
    sudo sad.sh &
    echo "sad.sh is  recover"        
fi	


cd $CURRENT_DIRECTORY	
