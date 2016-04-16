#!/bin/sh

#查看文件的格式，通过对升级的固件进行检测查看升级方法
#第二个参数必须是版本号
if [ $# -ne 1 ]; then
	echo "execute error: no arguments"
	exit 1
fi

beaconConf="/usr/ibeacon/conf/"
updateConfFile="/usr/ibeacon/conf/ibeaconUpdate.conf"
updateFile="`cat ${updateConfFile} | grep "UpdateFilePath" | grep -v grep | cut -d'=' -f2`"

local UPDATE=1
#检测是否要升级
checkBin="`sysupgrade -T ${updateFile}`"
if [ -z "${checkBin}" ]
then
    UPDATE=2
else
	echo "update filesystem"
	#覆盖，更新版本号，重新启动程序，此时检测脚本已经将文件解压
	cd /tmp
	echo ${updateFile}
	tar -zxvf ${updateFile}
	if [ -d "/tmp/updateRoot" ]; then
	    UPDATE=2
    fi
fi

if [ "$UPDATE" == "1" ]; then
  	echo "111111111111111111111111111"
	exit
fi



#GuardPro="testshell.sh"
GuardPro="guardIbeacon.sh"
UpdatePro="update"
CloudbeaconPro="cloudbeacon"

#关闭程序
GuardProPid="`ps | grep \"$GuardPro\" | grep -v grep | cut -d' ' -f1`"
if [ -n "${GuardProPid}" ]
then
	echo ${GuardProPid}
	kill -9 ${GuardProPid}
else
    GuardProPid="`ps | grep \"$GuardPro\" | grep -v grep | cut -d' ' -f2`"
    if [ -n "${GuardProPid}" ]; then
        echo ${GuardProPid}
        kill -9 ${GuardProPid}
    fi
fi
cloudbeaconPid="`ps | grep cloudbeacon | grep -v grep | cut -d' ' -f1`"
if [ -n "${cloudbeaconPid}" ]
then
	echo ${cloudbeaconPid}
	kill -9 ${cloudbeaconPid}
else
    cloudbeaconPid="`ps | grep cloudbeacon | grep -v grep | cut -d' ' -f2`"
    if [ -n "${cloudbeaconPid}" ]; then
        echo ${cloudbeaconPid}
        kill -9 ${cloudbeaconPid}
    fi
fi
updatePid="`ps | grep \"update\b\" | grep -v grep | grep -v updateIbeacon | cut -d' ' -f1`"
if [ -n "${updatePid}" ] 
then
	echo ${updatePid}
	kill -9 ${updatePid}
else
    updatePid="`ps | grep \"update\b\" | grep -v grep | grep -v updateIbeacon | cut -d' ' -f2`"
    if [ -n "${updatePid}" ]; then
        echo ${updatePid}
        kill -9 ${updatePid}
    fi
fi


checkBin="`sysupgrade -T ${updateFile}`"
if [ -z "${checkBin}" ]
then
	echo "update kernel and filesystem"
	#1、更新版本号
	sed -i -e "s#UpdateVersion=.*#UpdateVersion=$1#" ${updateConfFile}
	#2、备份配置信息
	echo "${beaconConf}" >> /etc/sysupgrade.conf
	sysupgrade -b /tmp/back.tar.gz
	#3、执行更新命令
	sysupgrade -f /tmp/back.tar.gz ${updateFile}
	#4、重新启动
	reboot
else
	echo "update filesystem"
	#覆盖，更新版本号，重新启动程序，此时检测脚本已经将文件解压
	cd /tmp
	echo ${updateFile}
	tar -zxvf ${updateFile}
	if [ -d "/tmp/updateRoot" ]; then
		cp -rfpP /tmp/updateRoot/* /
		sed -i -e "s#UpdateVersion=.*#UpdateVersion=$1#" ${updateConfFile}
	fi

	#重启程序
	/etc/init.d/startup restart

	#删除文件
	rm -rf /tmp/updateRoot
	rm -rf ${updateFile}
	/usr/ibeacon/tool/afterUpdate.sh
fi


