#! /bin/sh

times=10000

if [ $# == 1 ]
then
	times=$1
fi

cat /dev/null > bigconf.conf

for((i=1;i<=$times;i++))
do
	echo [group_$i]
done >> bigconf.conf
