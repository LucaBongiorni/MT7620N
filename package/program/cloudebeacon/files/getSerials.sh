#!/bin/sh
filePath="/usr/ibeacon/conf/ssid.conf"

#grep "^cb" "$filePath" | cut -d'=' -f2
cat ${filePath}
