#!/bin/sh

set -e

. ./install_headers.sh

for PROJECT in $PROJECTS
do
  (cd "$PROJECT" && DESTDIR="$SYSROOT" $MAKE install)
done
