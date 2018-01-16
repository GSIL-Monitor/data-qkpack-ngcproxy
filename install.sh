#!/bin/bash

installDir='/opt/'
sourceDir='/opt/src/'

ngproxySourceDirectoryName='data-qkpack-ngproxy/'
qkpackSourceDirectoryName=$sourceDir$ngproxySourceDirectoryName"module_qkpack/qkpack/"

echo "[info] install data-qkpack-ngproxy begin..."

chmod 755 configure
./configure  --add-module=module_qkpack/ --with-http_realip_module  --with-http_stub_status_module
make

cd $qkpackSourceDirectoryName
mkdir -p lib
mkdir -p logs

make clean
make

mkdir -p $installDir$ngproxySourceDirectoryName

cp -r $qkpackSourceDirectoryName"bin" $installDir$ngproxySourceDirectoryName
cp -r $qkpackSourceDirectoryName"conf" $installDir$ngproxySourceDirectoryName
cp -r $qkpackSourceDirectoryName"logs" $installDir$ngproxySourceDirectoryName
cp -r $qkpackSourceDirectoryName"lib" $installDir$ngproxySourceDirectoryName

cp -r $sourceDir$ngproxySourceDirectoryName"objs/nginx" $installDir$ngproxySourceDirectoryName"bin/"

cp -r $sourceDir$ngproxySourceDirectoryName"module_qkpack/sysctl.conf" /etc/
sysctl -p


echo "[info] install data-qkpack-ngproxy end..."
