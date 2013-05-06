#!/bin/bash
PATH=/var/run/MCore.txt
PATH2=/var/run/MCore2.txt
if [ $# -eq 4 ]; then
        /bin/ps -e -o pid,cmd |/bin/grep "$1 $3 $4"> $PATH
        /bin/sed -e 's/\/opt\/bin\/'"$1"' '"$3"' '"$4"'//g' $PATH  > $PATH2
elif [ $# -eq 2 ]; then
        /bin/ps -e -o pid,cmd |/bin/grep $1> $PATH
        /bin/sed -e 's/\/opt\/bin\/'"$1"'//g' $PATH  > $PATH2
else
        exit 2
fi

while read LINE
do
  /usr/bin/taskset -p $2 $LINE  
done<$PATH2
