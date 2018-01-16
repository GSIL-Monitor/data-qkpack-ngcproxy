#!/bin/bash

ngproxyDirectoryName='/opt/data-qkpack-ngproxy/'

BIN=$ngproxyDirectoryName"bin/nginx"
PID=`ps -ef|grep $BIN |grep -v grep|grep -v rsync| awk '{print $2}'`

if [ x"$PID" != x ];then
	kill $PID;
	echo "[data-qkpack-ngproxy]] kill $PID success"
else
	echo "[data-qkpack-ngproxy] stoped already"	
fi
