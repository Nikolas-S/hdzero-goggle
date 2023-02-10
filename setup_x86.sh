#!/bin/bash
set -e

TOOLCHAIN_URL="https://toolchains.bootlin.com/downloads/releases/toolchains/armv7-eabihf/tarballs/armv7-eabihf--musl--stable-2018.02-2.tar.bz2"

sudo apt install -y bzip2
sudo apt install -y mtd-utils
rm -rf mkapp/bin
mkdir mkapp/bin
cp /usr/bin/md5sum mkapp/bin/
cp /usr/sbin/mkfs.jffs2 mkapp/bin/

rm -rf toolchain
rm -rf toolchain_x86

echo "Extracting toolchain..."
mkdir toolchain
wget -qO- "$TOOLCHAIN_URL" | tar xj --strip-components=1 -C toolchain

rm -rf build && mkdir build
cmake . -DCMAKE_BUILD_TYPE=Release -DCMAKE_TOOLCHAIN_FILE=toolchain/share/buildroot/toolchainfile.cmake -Bbuild
