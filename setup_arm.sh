#!/bin/bash
#
# Sets up toolchain for arm64

set -e

URL_ARM_TOOLCHAIN="https://developer.arm.com/-/media/Files/downloads/gnu/11.2-2022.02/binrel/gcc-arm-11.2-2022.02-aarch64-arm-none-linux-gnueabihf.tar.xz"
URL_x86_TOOLCHAIN="https://toolchains.bootlin.com/downloads/releases/toolchains/armv7-eabihf/tarballs/armv7-eabihf--musl--stable-2018.02-2.tar.bz2"

sudo apt install -y bzip2
sudo apt install -y mtd-utils
rm -rf mkapp/bin
mkdir mkapp/bin
cp /usr/bin/md5sum mkapp/bin/
cp /usr/sbin/mkfs.jffs2 mkapp/bin/

rm -rf toolchain
rm -rf toolchain_x86

echo "Starting toolchain config..."       
mkdir toolchain
mkdir toolchain_x86
       
echo "Downloading and extracting ARM toolchain..."
wget -qO- "$URL_ARM_TOOLCHAIN" | tar xJ --strip-components=1 -C toolchain
        
echo "Downloading and extracting x86 toolchain..."
wget -qO- "$URL_x86_TOOLCHAIN" | tar xj --strip-components=1 -C toolchain_x86


echo "Doing despicable things..."
cp toolchain_x86/share/* toolchain/share -r
cp -r toolchain_x86/arm-buildroot-linux-musleabihf/sysroot toolchain/arm-none-linux-gnueabihf
cp -r toolchain_x86/arm-buildroot-linux-musleabihf/lib/* toolchain/arm-none-linux-gnueabihf/lib/
cp -r toolchain_x86/lib/* toolchain/lib/  #this might not be needed
rm -rf toolchain/lib/gcc/arm-none-linux-gnueabihf/11.2.1/*
cp -r toolchain_x86/lib/gcc/arm-buildroot-linux-musleabihf/6.4.0/* toolchain/lib/gcc/arm-none-linux-gnueabihf/11.2.1/
cp -r toolchain_x86/include/* toolchain/include/

# replace toolchain names in cmake files
cd toolchain/share/buildroot/
find . -type f -exec sed -i 's/arm-buildroot-linux-musleabihf/arm-none-linux-gnueabihf/g' {} +
cd ../../../

echo "Cleaning up..."
rm -rf toolchain_x86

rm -rf build && mkdir build
cmake . -DCMAKE_TOOLCHAIN_FILE=toolchain/share/buildroot/toolchainfile.cmake -Bbuild

