#!/bin/sh
#
# Build all binaries under directory "build-bin".
#
# This should be invoked at "..", top of the source archive.
#
# argument: any configure options except "--enable-setup=..." is allowed.
# 
JULIUS_VERSION=4.0RC1

######################################################################

mkdir build-bin
dir=`pwd`

# make julius and other tools with default setting
./configure $* --bindir=${dir}/build-bin
make
make install.bin

# make julius/julian with another setting
rm ${dir}/build-bin/julius
cd julius
make install.bin INSTALLTARGET=julius-${VERSION}

# julius-standard
make distclean
./configure $* --bindir=${dir}/build-bin --enable-setup=standard
make install.bin INSTALLTARGET=julius-${VERSION}-std

# finished
cd ..
make distclean
strip build-bin/*
ls build-bin
echo '###### FINISHED ######'
