#!/bin/bash

ngproxyDirectoryName='/opt/data-qkpack-ngproxy/'

BIN=$ngproxyDirectoryName"bin/nginx"
CONF=$ngproxyDirectoryName"conf/nginx.conf.online"

cd $ngproxyDirectoryName
$BIN -s reload -c $CONF -p `pwd` 2>error.out &
sleep 1

PID=`ps -ef|grep $CONF |grep -v grep|grep -v rsync|awk '{print $2}'`
if [ x"$PID" != x ];then
 	echo "[data-qkpack-ngproxy] $BIN -c $CONF $PID reload success."
else
	echo "[data-qkpack-ngproxy] $BIN -c $CONF reload failure."
fi
