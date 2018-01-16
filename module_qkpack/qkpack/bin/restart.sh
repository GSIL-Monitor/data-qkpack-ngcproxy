#!/bin/bash

ngproxyDirectoryName='/opt/data-qkpack-ngproxy/'

BIN=$ngproxyDirectoryName"bin/nginx"
CONF=$ngproxyDirectoryName"conf/nginx.conf.online"
PID=`ps -ef|grep $BIN |grep -v grep|grep -v rsync | awk '{print $2}'`

if [ x"$PID" != x ];then
	kill $PID;
	echo "[data-qkpack-ngproxy] kill $PID"
fi

cd $ngproxyDirectoryName

rm -rf   /opt/data-qkpack-ngproxy/logs/access.log
mkfifo   /opt/data-qkpack-ngproxy/logs/access.log

rm -rf   /opt/data-qkpack-ngproxy/logs/access_proxy.log
mkfifo   /opt/data-qkpack-ngproxy/logs/access_proxy.log

mkdir -p  /opt/data-qkpack-ngproxy/logs/access/
mkdir -p  /opt/data-qkpack-ngproxy/logs/access_proxy/
mkdir -p /opt/data-qkpack-ngproxy/logs/qkpack/

nohup cat /opt/data-qkpack-ngproxy/logs/access.log | /usr/local/cronolog/sbin/cronolog  /opt/data-qkpack-ngproxy/logs/access/access_%Y%m%d.log  &
nohup cat /opt/data-qkpack-ngproxy/logs/access_proxy.log | /usr/local/cronolog/sbin/cronolog  /opt/data-qkpack-ngproxy/logs/access_proxy/access_proxy_%Y%m%d.log  &


$BIN -c $CONF -p `pwd` 2>error.out &
sleep 1

PID=`ps -ef|grep $CONF |grep -v grep|grep -v rsync|awk '{print $2}'`
if [ x"$PID" != x ];then
 	echo "[data-qkpack-ngproxy] $BIN -c $CONF $PID restart success."
else
	echo "[data-qkpack-ngproxy] $BIN -c $CONF restart failure."
fi
