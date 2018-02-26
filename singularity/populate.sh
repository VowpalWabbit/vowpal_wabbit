#!/bin/bash
#
# A rudimentary script to populate a set of Singularity files
# for different development environments
#
set -eu

for DEBRELEASE in stable unstable; do
  for ARCH in amd64 i386; do
     export DEBRELEASE ARCH
     eval "cat <<EOF
$(cat "Singularity-template")
EOF
" >| Singularity.debian-$DEBRELEASE-$ARCH
  done
done

