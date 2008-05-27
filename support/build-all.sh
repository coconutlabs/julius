#!/bin/sh
#
# Build all binaries under directory "build-bin".
#
# This should be invoked at "..", top of the source archive.
#
# argument: any configure options except "--enable-setup=..." is allowed.
# 
JULIUS_VERSION=4.0.2

######################################################################

mkdir build
dir=`pwd`

# make julius and other tools with default setting
./configure --prefix=${dir}/build $*
make
make install

# make julius with another setting
rm ${dir}/build/bin/julius
cd julius
make install.bin INSTALLTARGET=julius-${JULIUS_VERSION}

# standard
cd ../libjulius
make distclean
./configure --prefix=${dir}/build --enable-setup=standard $*
make
cd ../julius
make clean
make install.bin INSTALLTARGET=julius-${JULIUS_VERSION}-std

# GMM-VAD
cd ../libjulius
make distclean
./configure --prefix=${dir}/build --enable-gmm-vad $*
make
cd ../julius
make clean
make install.bin INSTALLTARGET=julius-${JULIUS_VERSION}-gmm-vad

# Decoder-VAD
cd ../libjulius
make distclean
./configure --prefix=${dir}/build --enable-decoder-vad --enable-power-reject $*
make
cd ../julius
make clean
make install.bin INSTALLTARGET=julius-${JULIUS_VERSION}-decoder-vad

# finished
cd ..
make distclean
strip build/bin/*
echo '###### FINISHED ######'
