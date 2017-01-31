#!/bin/bash
set -e

if [ ! -d vpo_lib ]; then
  echo "Run this script from the top level."
  exit 1
fi

echo "Creating source package...."

DEST=visexp-X.XX-source

mkdir $DEST

#
#  Source code
#
mkdir $DEST/src
cp -av src/*.[chr]* $DEST/src
cp -av Makefile* $DEST/
cp -av make_* $DEST/

mkdir $DEST/vpo_lib
cp -av vpo_lib/*.[chr]* $DEST/vpo_lib

mkdir $DEST/misc
cp -av misc/*.* $DEST/misc
mkdir $DEST/misc/debian
cp -av misc/debian/* $DEST/misc/debian

mkdir $DEST/obj_linux
mkdir $DEST/obj_lx32
mkdir $DEST/obj_win32

mkdir $DEST/obj_linux/libvpo
mkdir $DEST/obj_lx32/libvpo
mkdir $DEST/obj_win32/libvpo

#
#  Data files
#
cp example.wad $DEST

#
#  Documentation
#
cp -av *.txt $DEST

mkdir $DEST/doc
cp -av doc/*.* $DEST/doc

#
# all done
#
echo "------------------------------------"
echo "tar czf visexp-X.XX-source.tar.gz visexp-X.XX-source"

