#!/bin/bash

# make clean

make ARCH=arm64 rockchip_linux_defconfig
make ARCH=arm64 gdk850.img -j8 LOCALVERSION=-yanzi

exit 0

