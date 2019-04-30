#!/bin/sh

cd u-boot-2010.06
make ARCH=arm CROSS_COMPILE=arm-himix100-linux-
cp u-boot.bin ../
cd ..
./mkboot.sh reginfo.bin uboot-liteos.bin
cp uboot-liteos.bin /work/tftp

