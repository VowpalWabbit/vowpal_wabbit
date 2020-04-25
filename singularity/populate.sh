#!/bin/bash
#
# A rudimentary script to populate a set of Singularity files
# for different development environments
#
set -eu

for DEBRELEASE in unstable; do
  for ARCH in amd64 i386; do
     if [ $ARCH == amd64 ]; then
         DOCKERARCHPREFIX=""  # it is the default one
     else
         DOCKERARCHPREFIX="$ARCH/"
     fi
     export DEBRELEASE ARCH DOCKERARCHPREFIX
     eval "cat <<EOF
$(cat "template")
EOF
" >| Singularity.debian-$DEBRELEASE-$ARCH
  done
done

