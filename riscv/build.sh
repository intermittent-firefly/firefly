#!/bin/bash

set -ex

RISCV_DIR=$(pwd)

cp firefly-rocket/csrc/* rocket-chip/src/main/resources/csrc/
cp firefly-rocket/emulator/* rocket-chip/emulator/
cp firefly-rocket/scala/* rocket-chip/src/main/scala/system/

cd rocket-chip/emulator
make
make debug
cd $RISCV_DIR
cp rocket-chip/emulator/emulator-* ./../bin/
