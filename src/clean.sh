#!/bin/sh
set -e
. ./config.sh

for PROJECT in $PROJECTS; do
  (cd $PROJECT && $MAKE clean)
done

rm -rf sysroot/usr/lib sysroot/usr/include
rm -rf sysroot/boot/kfs-1.bin
