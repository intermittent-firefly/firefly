#!/bin/bash

set -e

mkdir -p ./bin

# Install dependencies
sudo apt install -y scons python3 python3-pip
pip3 install matplotlib

# Build NVMain
cd NVmain
sed -i 's/from gem5_scons import Transform/#from gem5_scons import Transform/g' SConscript
scons --build-type=fast
cd ./..
cp NVmain/nvmain.fast bin/

cd riscv
./build.sh
cd ./..
