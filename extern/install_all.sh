#!/bin/bash

installDir='/usr/local'
sourceDir='/opt/src/data-qkpack-ngproxy/extern/'

boostSourceFile='boost_1_53_0.tar.gz'
boostSourceDirectoryName='boost_1_53_0'

libeventSourceFile='libevent-2.0.22-stable.tar.gz'
libeventDirectoryName='libevent-2.0.22-stable'

log4cppSourceFile='log4cpp-1.2.0.tar.gz'
log4cppDirectoryName='log4cpp'

snappySourceFile='snappy-master.zip'
snappyDirectoryName='snappy-master'

yajlSourceFile='yajl-master-patch.zip'
yajlDirectoryName='yajl-master'

#path
cronologPath=/usr/local/cronolog

echo "[info] install boost begin..."

#install boost
cd $sourceDir
tar -xvf $boostSourceFile
cd boost_1_53_0
./bootstrap.sh
./bjam  install
ldconfig

echo "[info] install libevent begin..."

#install libevent
cd $sourceDir
tar -xvf $libeventSourceFile
cd $libeventDirectoryName
./configure
make && make install

echo "[info] install log4cpp begin..."

#install log4cpp
cd $sourceDir
tar -xvf $log4cppSourceFile
cd $log4cppDirectoryName
./configure
make && make install

echo "[info] install snappy begin..."

#install snappy
cd $sourceDir
unzip $snappySourceFile
cd $snappyDirectoryName
./autogen.sh
./configure
make && make install

echo "[info] install yajl begin..."

#install yajl
yum install cmake -y

cd $sourceDir
unzip $yajlSourceFile
cd $yajlDirectoryName
./configure
make && make install

echo "[info] install cronolog begin..."

#install cronolog
tar zxvf cronolog-1.6.2.tar.gz
cd cronolog-1.6.2
./configure --prefix=/usr/local/cronolog
make
make install


echo "[info] install is ok end..."
