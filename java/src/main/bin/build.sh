#!/usr/bin/env bash

#brew tap phinze/homebrew-cask
#brew install brew-cask
#brew cask install virtualbox
#brew install boot2docker
#brew install docker
#boot2docker delete
#boot2docker init
#boot2docker up

# After running boot2docker up this is printed out and it should be the same for everyone
#export DOCKER_HOST=tcp://192.168.59.103:2376
#export DOCKER_CERT_PATH=~/.boot2docker/certs/boot2docker-vm
#export DOCKER_TLS_VERIFY=1

docker run -v $(pwd):/vowpal_wabbit 32bit/ubuntu:14.04 /bin/bash -c "\
apt-get install -qq software-properties-common; \
apt-get update; \
apt-get install -qq g++ make libboost-all-dev default-jdk; \
export JAVA_HOME=/usr/lib/jvm/java-7-openjdk-i386; \
cd /vowpal_wabbit; \
make clean; \
make; \
mv java/target/vw_jni.lib java/target/vw_jni.Ubuntu.14.i386.lib"

docker run -v $(pwd):/vowpal_wabbit ubuntu:14.04 /bin/bash -c "\
apt-get install -qq software-properties-common; \
apt-get update; \
apt-get install -qq g++ make libboost-all-dev default-jdk; \
export JAVA_HOME=/usr/lib/jvm/java-7-openjdk-amd64; \
cd /vowpal_wabbit; \
make clean; \
make; \
mv java/target/vw_jni.lib java/target/vw_jni.Ubuntu.14.amd64.lib"

docker run -v $(pwd):/vowpal_wabbit centos:7 /bin/bash -c "\
yum install -q -y gcc-c++ make boost-devel zlib-devel java-1.7.0-openjdk-devel; \
export JAVA_HOME=/usr/lib/jvm/java-1.7.0-openjdk; \
cd /vowpal_wabbit; \
make clean; \
make; \
mv java/target/vw_jni.lib java/target/vw_jni.Red_Hat.7.amd64.lib"

cat Makefile | perl -pe 's/-fPIC/-fpermissive -fPIC/g' > Makefile.permissive

docker run -v $(pwd):/vowpal_wabbit centos:6 /bin/bash -c "\
yum update -q -y; \
yum install -q -y wget which boost-devel zlib-devel java-1.7.0-openjdk-devel; \
cd /etc/yum.repos.d; \
wget http://people.centos.org/tru/devtools-2/devtools-2.repo; \
yum install -q -y devtoolset-2-gcc devtoolset-2-gcc-c++ devtoolset-2-binutils; \
ln -s /opt/rh/devtoolset-2/root/usr/bin/* /usr/local/bin/; \
export JAVA_HOME=/usr/lib/jvm/java-1.7.0-openjdk.x86_64; \
cd /vowpal_wabbit; \
make clean; \
make -f Makefile.permissive; \
mv java/target/vw_jni.lib java/target/vw_jni.Red_Hat.6.amd64.lib"

rm Makefile.permissive

make clean
make
mv java/target/vw_jni.lib java/target/vw_jni.$(uname -s).$(uname -m).lib

cd java
mvn clean package
