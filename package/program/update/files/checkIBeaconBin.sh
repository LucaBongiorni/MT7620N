#!/bin/sh

#查看文件的格式是否正确
#格式正确，输出 "returnVal 0"
#格式错误，输出 "returnVal -1"
#文件没下载，输出 "returnVal -2"，正常不会出现这个，执行这个程序前应该检测过了


updateConfFile="/usr/ibeacon/conf/ibeaconUpdate.conf"
updateFile="`cat ${updateConfFile} | grep "UpdateFilePath" | cut -d'=' -f2`"
if [ ! -f ${updateFile} ] ; then
	echo "returnVal -2"
	exit 1
fi


checkBin="`sysupgrade -T ${updateFile}`"
if [ -z "${checkBin}" ]; then
	echo "returnVal 0"
else
	cd /tmp
	tar -zxvf ${updateFile}
	if [ -d "/tmp/updateRoot" ]; then
		echo "returnVal 0"
	else
		echo "returnVal -1"
	fi
fi


