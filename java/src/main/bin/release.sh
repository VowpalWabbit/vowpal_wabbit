#!/usr/bin/env bash

# The goal of this is to emulate the maven release plugin with the added
# steps necessary

if (( $# < 1 )); then
    echo "Usage: $0 <tag> (<gpg_key>)"
    echo "This program will do a maven release and requires the tag/
    The gpg key is optional and if not supplied a prompt will appear requesting it."
    exit 1
fi

tag=$1
if (( $# > 1)); then
    gpg_key=$2
fi

url=$(cat java/pom.xml | pcregrep -M "<scm>(\n|.)*<url>(.*)</url>(\n|.)*</scm>" | grep "<url>" | sed "s/<\/*url>//g")

checkout_dir="java/target/checkout"
rm -rf $checkout_dir
checkout="git clone $url $checkout_dir"

echo $checkout
$checkout
cd $checkout_dir

branch="git checkout $tag"
echo $branch
$branch

chmod 755 java/src/main/bin/build.sh
java/src/main/bin/build.sh

# This will make sure that the cross platform build script made all the required libraries for the different supported OSs
if [ ! -f java/target/vw_jni.Ubuntu.12.amd64.lib ] ||
    [ ! -f java/target/vw_jni.Ubuntu.14.amd64.lib ] ||
    [ ! -f java/target/vw_jni.Ubuntu.14.i386.lib ] ||
    [ ! -f java/target/vw_jni.Red_Hat.6.amd64.lib ] ||
    [ ! -f java/target/vw_jni.Red_Hat.7.amd64.lib ] ||
    [ ! -f java/target/vw_jni.Darwin.x86_64.lib ]; then
    echo "Not all libraries built, failing!"
    exit 1
fi

cd java

version=$(grep AC_INIT ../configure.ac | cut -d '[' -f 3 | cut -d ']' -f 1)

perl -pi -e "\$a=1 if (!\$a && s/<version>.*/<version>$version<\/version>/);" pom.xml

mvn_cmd="mvn clean deploy -P release-sign-artifacts"
if [ ! -z $gpg_key ]; then
    mvn_cmd="$mvn_cmd -Dgpg.passphrase=$gpg_key"
fi

echo $mvn_cmd
$mvn_cmd
