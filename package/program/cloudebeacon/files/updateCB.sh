#!/bin/sh

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


cp -rfpP /tmp/rootfs/* /
/etc/init.d/startup restart

