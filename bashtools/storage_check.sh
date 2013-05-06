#!/bin/sh

TEMPORARY_DIRECTORY="temp_directory"
STORAGE_SIZE_LIMIT=256
SOFT_VERSION_COUNT_MAX=4
CURRENT_DIRECTORY=`pwd`

for dev_path in $(cat /etc/fstab | awk '{print $1}' | sed '/^#/d') 
do
	#echo $dev_path

	#get the storage size has been used
	STORAGE_USED_SIZE=`df --block-size=MB $dev_path | grep $dev_path | awk '{print $3}' | grep -o -i "[0-9]\{1,\}"`

	if [ $STORAGE_USED_SIZE -gt $STORAGE_SIZE_LIMIT ]; then
		echo $STORAGE_USED_SIZE
		echo "bigger than $(STORAGE_SIZE_LIMIT)MB"
	else
		#get the free mem size
		free_mem_size=`free | grep "Mem" | awk '{print $4}'`
		#is the free mem size is double then the used storage size
		if [ `expr $free_mem_size / 2048` -gt $STORAGE_USED_SIZE ]; then 
			#echo "free mem is bigger enough"
			
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
					exit
				fi	
			fi

			#if the software version number is more than SOFT_VERSION_COUNT_MAX
			if [ `ls -l /blk/*.IMG | wc -l` -gt $SOFT_VERSION_COUNT_MAX ]; then
				echo "there are so many version files,please delete some useless version files"
				echo "keep the version file not more than $SOFT_VERSION_COUNT_MAX"
				exit
			else
				#echo "version file check ok"
				cd
				if [ -d $TEMPORARY_DIRECTORY ]; then
					echo "$TEMPORARY_DIRECTORY allready exist at `pwd`"
					echo "please rename the exist directory"
					cd -
				else
					#echo "$TEMPORARY_DIRECTORY not exist at `pwd`"
					#make dir for temporary
					if ! `mkdir $TEMPORARY_DIRECTORY`; then
						echo "create directory $TEMPORARY_DIRECTORY failed"
						cd $CURRENT_DIRECTORY	
						exit
					fi
					
					echo "sorting the storage,please waite for few minutes "
					echo "and please keep the power not down"
					#copy file from the storage to the temporary directory
					if ! `cp -r /blk/* ./$TEMPORARY_DIRECTORY/`; then
						echo "copy files from storarge failed"
						rm -rf $TEMPORARY_DIRECTORY	
						cd $CURRENT_DIRECTORY	
						exit
					fi
					
					#umount the storage device and ready to format it
					if ! `sudo umount -l /blk`; then
						echo "umount storage failed"
						rm -rf $TEMPORARY_DIRECTORY	
						cd $CURRENT_DIRECTORY	
						exit
					fi
					
					if ! `sudo mkfs.vfat $dev_path >& /dev/null`; then
						echo "sort the storage failed"
						echo "original files at `pwd`/$TEMPORARY_DIRECTORlY"
						echo "please operate the left operation manually"
						cd $CURRENT_DIRECTORY	
						exit
					fi
					
					#mount the storage and ready to copy the files back to it
					if ! `sudo mount /blk`; then
						echo "mount /blk failed"
						echo "original files at `pwd`/$TEMPORARY_DIRECTORlY"
						echo "please operate the left operation manually"
						cd $CURRENT_DIRECTORY	
						exit
					fi

					if ! `cp $TEMPORARY_DIRECTORY/* /blk/`; then
						echo "copy file back to the storage failed"
						echo "original files at `pwd`/$TEMPORARY_DIRECTORlY"
						echo "please operate the left operation manually"
						cd $CURRENT_DIRECTORY	
						exit
					else
						#write the files really to the storage device
						if ! `sudo umount -l /blk`; then
							echo "write the storage failed"
							echo "original files at `pwd`/$TEMPORARY_DIRECTORlY"
							echo "please operate the left operation manually"
							cd $CURRENT_DIRECTORY	
							exit
						else
							rm -rf $TEMPORARY_DIRECTORY	
							echo "sort succeed"
							if [ $mount_flag -eq 1 ]; then
								if ! `sudo mount /blk`; then
									echo "mount the storage failed"
									cd $CURRENT_DIRECTORY	
									exit
								fi
							fi
							cd $CURRENT_DIRECTORY	
						fi	
					fi
				fi
			fi
		else
			echo "free mem is not enough to sort the storage"
			cd $CURRENT_DIRECTORY	
		fi
	fi
done

cd $CURRENT_DIRECTORY	
