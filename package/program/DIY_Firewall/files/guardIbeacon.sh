#!/bin/sh

Program="update"

ProgramPath="/usr/ibeacon/bin/"

while true
do
	for i in ${Program} 
	do
		File="`ps | grep ${i} | grep -v grep`"
		if [ "${File}" = "" ]
		then
			cd ${ProgramPath}
			echo ${i} running ...
			./${i} &
			cd -
		fi
		sleep 1
	done
	sleep 5
done


