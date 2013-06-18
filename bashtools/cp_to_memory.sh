#!/bin/sh

TEMPORARY_DIRECTORY="/home/admin/temp_directory"
STORAGE_SIZE_LIMIT=900
SOFT_VERSION_COUNT_MAX=5
CURRENT_DIRECTORY=`pwd`

for dev_path in $(cat /etc/fstab | awk '{print $1}' | sed '/^#/d') 
do
	echo $dev_path

	#get the storage size has been used
	STORAGE_USED_SIZE=`df --block-size=MB $dev_path | awk '{print $3}' | grep -o -i "[0-9]\{1,\}"`
	
	#echo "STORAGE_USED_SIZE = $STORAGE_USED_SIZE MB"
	#echo "STORAGE_SIZE_LIMIT = $STORAGE_SIZE_LIMIT MB"

	if [ $STORAGE_USED_SIZE -gt $STORAGE_SIZE_LIMIT ]; then
		echo "STORAGE_USED_SIZE = $STORAGE_USED_SIZE MB"
		echo "bigger than $STORAGE_SIZE_LIMIT MB"
	else
		#get the free mem size
		free_mem_size=`free -m | grep "Mem" | awk '{print $4}'`
		
		echo "free_mem_size = $free_mem_size" 
		#is the free mem size is double then  used storage size
		if [ `expr $free_mem_size / 2` -gt $STORAGE_USED_SIZE ]; then 
			echo "free mem is bigger enough"
			
			#find out is the storage been mounted
			if [ `sudo mount -l | grep "$dev_path" | wc -l` -gt 0 ]; then
				mount_flag=1
			else
				mount_flag=0
			fi
			
			#if the storage is not mounted then mount it
			if [ $mount_flag -eq 0 ]; then
				if ! `sudo mount /blk`; then
					echo "mount /blk failed"
					exit 1
				fi	
			fi

			#if the software version number is more than SOFT_VERSION_COUNT_MAX
			if [ `ls -l /blk/*.IMG | wc -l` -gt $SOFT_VERSION_COUNT_MAX ]; then
				echo "there are so many version files,please delete some useless version files"
				echo "keep the version file not more than $SOFT_VERSION_COUNT_MAX"
				exit 2
			else
				echo "version file check ok"
				cd
				if [ -d $TEMPORARY_DIRECTORY ]; then
					echo "$TEMPORARY_DIRECTORY already exist at `pwd`"
					echo "please rename the exist directory"
					cd -
				else
					echo "$TEMPORARY_DIRECTORY not exist at `pwd`"
					#make dir for temporary
					if ! `mkdir $TEMPORARY_DIRECTORY`; then
						echo "create directory $TEMPORARY_DIRECTORY failed"
						cd $CURRENT_DIRECTORY	
						exit 3
					fi
					
					echo "copy files from storage to memory,please waite for few minutes "
					echo "and please keep the power not down"
					#copy file from the storage to the temporary directory
                       if ! `pkill sad.sh`; then
                            echo "pkill sad.sh failed"   
                            rm -rf $TEMPORARY_DIRECTORY	
                            echo "delete temp_directory successful"
                            cd $CURRENT_DIRECTORY
                            exit 4
                       else 
                            echo "pkill sad.sh successful"
                       fi 
					
					if ! `cp -r /blk/*  $TEMPORARY_DIRECTORY/`; then
						echo "copy files from storarge failed"
						rm -rf $TEMPORARY_DIRECTORY	
						cd $CURRENT_DIRECTORY	
						exit 5
					else 
					    echo "copy files from storarge to memory successful"
					fi
				fi										
				
			fi
		else
			echo "free mem is not enough to sort the storage"
			cd $CURRENT_DIRECTORY
			exit 6
		fi
	fi
done

cd $CURRENT_DIRECTORY	
