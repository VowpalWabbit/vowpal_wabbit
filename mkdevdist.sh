#!/bin/bash

DESTDIR=/tmp/dev_dist_vw
echo "Checking out git repo to $DESTDIR"
git clone https://github.com/JohnLangford/vowpal_wabbit.git $DESTDIR

cd $DESTDIR
sh autogen.sh

VERSION=$(cat vowpalwabbit/config.h | grep PACKAGE_VERSION | cut -d '"' -f 2)

echo Making Distribution for version $VERSION
make dist

FNAME=$(ls vowpal_wabbit-*.tar.gz)
mv $FNAME /tmp

git checkout releases
git rm *
mv /tmp/$FNAME .
tar -zxvf $FNAME
git add vowpal_wabbit-*
VDATE=$VERSION-$(date +%Y-%m-%d-%H-%M)
git add *
git commit -m "added version $VDATE files"
git push origin releases

